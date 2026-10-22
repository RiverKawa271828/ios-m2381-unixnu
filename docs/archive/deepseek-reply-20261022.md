# GUI 黑屏死锁：六问答复 v2（含反汇编实证）

> 本版取代 v1。新增的是**实证**而不是推测：mobileactivationd 的崩溃第一因已经从二进制反汇编里
> 挖出来了，MadGate 的应答路径也确认了。反汇编工装 = `ipsw macho disass`（`.re/` 下的
> 中间产物可复现）。
> 一句话：**崩溃和激活记录无关——是容器路径（system group）取不到。**

---

## 0. 本轮五个实证（先看这个）

| # | 结论 | 证据 |
|---|---|---|
| 1 | **崩溃第一因 = `container_system_group_path_for_identifier()` 返回 NULL** → `fatalError("Failed to copy container path(s).")` → `os_crash` + `brk #1` | mobileactivationd `main` 反汇编：0x4F828–0x4F834 四个 `cbz` 全部跳 0x5062C → `bl 0x1BF56C`（该函数的格式化串就是这句）；崩溃 pc `0x1BF5BC` 正是该函数里的 `brk`，bt `0x50630` 正是 0x5062C 那条 `bl` 的返回地址 |
| 2 | **根盘里根本没有 `/private/var/containers`**（也没有 `/private/var/mobile/Containers`） | 全卷 listing 321,389 条（`rings/iosroot-list.txt`）零命中；`private/var` 下 29 个目录里没有 containers |
| 3 | **MadGate 对错误应答是安全的 fast-fail**：NSXPCConnection 同步代理 + `synchronousRemoteObjectProxyWithErrorHandler:`，错误进 block 后返回 nil+NSError，**没有重试、没有 trap** | `-[MadGate copyActivationRecord:]` 反汇编（0x1C90CF99C）；`-[MadGate init]`（0x1C90CDB90）用 `initWithMachServiceName:_kMAApplicationID options:0x1000` |
| 4 | **`deviceConfiguredForHRN` 的真身 = 一个 os_feature flag**：`_os_feature_enabled_impl("DeviceManagementClient","HRN")`；**flag 为真时函数直接返回 true，根本不调 MadGate** | `+[LSHRNSupport deviceConfiguredForHRN]` 反汇编（0x181C258B0，0x181C2595C 处） |
| 5 | **还有第二个独立 fatal**在同一函数里：`setiopolicy_np(IOPOL_TYPE_VFS_ALLOW_LOW_SPACE_WRITES=9, SCOPE_PROCESS, ON)` 在内核里要求 entitlement `com.apple.private.vfs.allow-low-space-writes`，没有就 EPERM → 同样进 fatal 链 | 二进制反汇编 0x4F78C；内核 `kern_resource.c:2400/2427` |

**由此推出最重要的一条判读**：r33（改名手术）在**代码层是成立的**——服务名没注册时 MadGate 会立刻
拿到 error 并返回 nil。r33b 窗里它却吊了 15 分钟，说明**那一窗服务名仍然是注册着的**。
所以版本身份的结论被反向钉死：**那一窗跑的仍是带旧名字的定义**（假说 A，或 B 里的"旧定义仍在"）。
这比"分不清 A/B"强得多：**只要看到 MadGate 永等，就说明手术没在设备上生效。**

---

## 1. 崩溃第一因：容器路径（新）

### 1.1 反汇编链（可逐条复核）

`main` 从文件偏移 0x4F728 开始（`macho disass --vaddr 0x10004f728`），启动顺序是：

```
0x4F780  setiopolicy_np(9, 0, 1)          ; IOPOL_TYPE_VFS_ALLOW_LOW_SPACE_WRITES
0x4F790  cbnz w0, 0x50624                  ; 非 0 -> fatal（EPERM 会走到这里）
0x4F7A4  [NSString UTF8String]             ; "com.apple.mobileactivationd"
0x4F7A8  set_user_dir_suffix(...)
0x4F7AC  tbz  w0,#0, 0x50628               ; 0 -> fatal
0x4F7B8  signal(30, SIG_IGN)
0x4F7BC  bl 0x34218                        ; 返回 NSString（系统组容器路径 1）
0x4F7DC  bl 0x342C8                        ; 返回 NSString（系统组容器路径 2）
0x4F7FC  bl 0x342C8                        ; 返回 NSString（系统组容器路径 3）
0x4F81C  bl 0x342C8                        ; 返回 NSString（系统组容器路径 4）
0x4F828  cbz x26, 0x5062C                  ; ← 任一为 NULL 就 fatal
0x4F82C  cbz x19, 0x5062C
0x4F830  cbz x20, 0x5062C
0x4F834  cbz x0,  0x5062C
...
0x4F944  tbnz w27,#0, 0x50614              ; NSURL getResourceValue 失败（r29 的 8B 补丁点之一）
0x4F9B8  tbnz w23,#0, 0x5061C              ; NSURL setResourceValue 失败（另一处）
0x4FA14  bl getpwuid_r; cbnz w0, 0x50630    ; passwd 查询失败
```

- `0x342C8` 内部直接调 `container_system_group_path_for_identifier()`（`macho disass --vaddr 0x1000342c8`）。
  也就是说这四个值 = **该 daemon 的 system group 容器路径**（对应它 entitlements 里的
  `com.apple.security.system-groups`）。
- fatal 帮助函数 `0x1BF56C` 的串就是 **"Failed to copy container path(s)."**（`macho disass --vaddr 0x1001bf56c`）。
- 崩溃现场 `pc=0x1BF5BC` 是该函数末尾的 `brk #1`；`bt=0x50630` 是 0x5062C 那条 `bl` 的返回地址。
  **0x5062C 这个入口在全函数里只有一个来源：0x4F828–0x4F834 那四个容器空指针检查。**
  ⇒ 崩溃 = 容器路径拿到 NULL，确凿。

### 1.2 为什么"修 fatal 尾声"注定失败

r29 的 8 字节补丁（tbz→tbnz）打在 **0x4F944 / 0x4F9B8**，那是 NSURL 资源读写的两个守卫，
位于容器检查**之后**。进程在 0x4F828 就已经死了，永远走不到那两条。这解释了：
- 补丁"确认上机"（cs_invalid_page 出现、bt 从 0x50674 变到 0x50630）却**毫无效果**；
- 每次 bt 都停在容器这一站，因为它是这条启动序列里第一个真正失败的判据。

### 1.3 为什么容器路径是 NULL

三条候选，按可能性排：

1. **镜像里没有容器树**：`/private/var/containers` 整个不存在（证据见 §0 #2）。
   `container_system_group_path_for_identifier` 走 containermanagerd，后者要么按需建，
   要么已经有一份 group 映射；在全空的 /var 上，这一步失败非常自然。
2. **entitlements 没被内核看见**：system group 的解析要读进程的
   `com.apple.security.system-groups`。我们的 AMFI 是自研实现（0055 号补丁 0035），
   entitlements 来自 **XML blob**，且只在 `vnode_check_signature` 的"可写副本"分支里 attach。
   若 attach 没发生（或 blob 走的只读重验证分支），查询就全空——**同时也解释了
   setiopolicy_np 为什么 EPERM**（那一条也是 entitlement 门）。
3. containermanagerd 自身没就绪（它只 exec 了 1 次，环里无崩溃记录，但没采到它的栈）。

**判别方法极便宜**：boot-arg 加 **`sweet_amfi_debug=1`**（`0035` 里预留的 Tunable）。
它会为每个签名打一行 `SWEET AMFI: <identity>: N entitlements (...)`；
若 mobileactivationd 显示 0 或没有这一行 → 候选 2 成立；若显示正常数量 → 候选 1/3。

> 顺带：二进制里确实带 XML entitlement
> `<key>com.apple.private.vfs.allow-low-space-writes</key>` 与
> `com.apple.security.system-groups` 相关串——所以**文件本身是有权限的**，
> 问题在"内核有没有把它们挂到进程上"。

---

## 2. Q1 假说 B：用 MadGate 反汇编把范围再收窄

### 2.1 新的事实

`-[MadGate copyActivationRecord:]` **不是**裸 XPC，而是：

1. `-[MadGate init]`：`NSXPCConnection initWithMachServiceName:_kMAApplicationID options:0x1000`，
   接口 = `MobileActivationProtocol`。服务名常量 `_kMAApplicationID` = `com.apple.mobileactivationd`。
2. 调用点：`synchronousRemoteObjectProxyWithErrorHandler:` → 代理上
   `copyActivationRecordWithCompletionBlock:`。
3. 错误路径：error handler 收到 `NSError`，方法返回 nil 并回填 error。**无重试、无断言。**

结论：**服务名未注册 → 立即 error → 返回 nil**。r33 的设计在代码层是对的。
反过来说——**r33b 窗里 MadGate 吊了整窗，只能说明那一窗服务名还注册着**。
这把"分不清 A/B"变成了一条硬判据（见 §0）。

### 2.2 B 还剩哪些机制

既然"永等"要求服务名注册着，B 就只剩**"另有一份带旧 MachServices 的定义"**这一类：

| 机制 | 判定 |
|---|---|
| **launchd job cache**（`launchd_cache_loader` 装入内核/持久化的 job 定义快照） | **仍为头号**。环里确有 `exec launchd_cache_loader`。硬复位从不给它刷新的机会，旧定义可长期驻留 |
| `.xpc` bundle 隐式注册（Info.plist 的 `CFBundleIdentifier` 即服务名） | 次之。你们的扫描没覆盖 `/System/Library/XPC/` 与 app 内 `XPCServices/`；390 个 raw 命中未按路径枚举 |
| 某进程直接 exec 路径 | 基本排除：每次 exec 前都有 `exec xpcproxy`（launchd 的跳板） |
| LaunchEvents | 排除：86400s 周期 × 17min 出不来 97 次 |
| jetsam 表 | 排除：被动配额表，不起动不注册 |
| KeepAlive/RunAtLoad | plist 里没有；97 次循环是 launchd 的崩溃节流（"Too many corpses"） |

**判读要点**：97 次/10.5s 的节奏 = launchd 对同一 job 的 crash throttle。plist 无 KeepAlive 还能
这样重启，只有两种解释：launchd 攥着未交付的 demand 反复重试，或有客户端每 10s 重发。
两者都要求"服务名仍注册着"——与 §2.1 的结论一致。

---

## 3. Q2 kill-switch 排序（按新证据重排）

### 3.1 排序

| 序 | 方案 | 为什么排这里 | 前置 | 风险 | 回滚 |
|---|---|---|---|---|---|
| **S1** | **修容器环境**：让 `container_system_group_path_for_identifier` 拿得到路径 | 这是**真正的根因**；修好后守护进程自己就不崩，不再需要任何 kill-switch；很可能同时清掉 SIGABRT 波 | 先加 `sweet_amfi_debug=1` 判 entitlements；采 containermanagerd 栈 | 容器树/DB 是闭源格式，若必须手搓映射，工量不可控 | 纯数据，可逆 |
| **S2** | **原地 stub**（保持 `/usr/libexec/mobileactivationd` 路径，服务名恢复原名，check-in 后回字典） | 对 A/B、对崩溃根因**全部免疫**；是唯一不依赖"先修好什么"的方案 | 知道请求/应答键名（可从 `MobileActivationProtocol` 反汇编取） | stub 需本地签名+信任缓存；sandbox 可能禁写 console（用 exec 名做标记即可） | 换回原二进制重签 |
| **S3** | **lsd 侧绕过**：让 HRN 门不再触发 MadGate | 不依赖守护进程是否能跑起来 | 见 3.2：优先走 **feature flag 数据层**，其次 patch | 改 lsd 需重签；语义上把设备当成"HRN 已配置" | 换回原 dylib |
| **S4** | **patch 守护进程启动序列**：NOP 掉 0x4F828–0x4F834 四条 `cbz`（外加 0x4F790/0x4F7AC/0x4FA18 三条） | 5–6 条指令，机械可行 | 无 | 进程带着 NULL 路径继续跑，可能在下一次用到容器时崩；治标不治本 | 换回原二进制 |
| **S5** | 假激活记录 | —— | —— | 与本崩溃**无关**：崩在容器检查，还没走到记录逻辑 | —— |
| **S6** | 改名 / `Disabled` | 只作为**诊断实验** | —— | 若旧定义来自 cache/bundle，删 plist 声明无效 | 换镜像 |

### 3.2 S3 的两个具体落点（反汇编给出）

`+[LSHRNSupport deviceConfiguredForHRN]` 的真实逻辑（0x181C258B0）：

```
hrnLock
if (cached) return cached
if (featureFlagOverride 已设) flag = (override == 1)
else flag = _os_feature_enabled_impl("DeviceManagementClient", "HRN")
if (flag) return true;                  // ← 根本不去问激活记录
if (activationRecordOverride == 0)      // 默认值
    rec = MAECopyActivationRecordWithError()   // ← 就是这里吊死
if (rec) configured = ((DeviceConfigurationFlags & 0x21) == 0x21)
```

- **数据层（首选）**：`_os_feature_enabled_impl` 读
  `/System/Library/FeatureFlags/` 下的 Domain plist（本机存在
  `Domain/DeviceManagementClient.plist`，390B）。把这枚 `HRN` flag 打开 →
  `deviceConfiguredForHRN` 直接返回 true，**零代码、零重签**。
  需要先确认 release 构建认可的是哪一个 set（Disclosure/Profile），
  读一下那个 390B 的 plist 和 `FeatureFlags/Global.plist` 即可。
- **代码层（次选）**：把 `initMAECopyActivationRecord` 的入口改成 `mov x0,#0; ret`，
  或把 0x181C2597C 的 `cbnz w0` 改成无条件跳（强制走 flag=true 分支）。
  后者只影响这一个门。
- 顺带：类里还有 `+[LSHRNSupport setActivationRecordOverride:]` /
  `setActivationRecordOverrideNil`——说明 Apple 自己也留了"覆盖"入口，
  但它们只能进程内调用，不能从外部设置。

### 3.3 对 S2（stub）的修订

现在知道真实守护进程**根本起不来**（容器路径缺失），stub 的价值反而更高：

- stub 必须在 check-in **之后**立刻可用，不做任何可能失败的初始化（不碰容器、不碰 passwd）。
- 应答用**字典**（`{ActivationRecord: <空>, Error: <码>}`），不要用 `XPC_TYPE_ERROR` 对象。
- 服务名恢复原名（`com.apple.mobileactivationd` 和 `…d.lockdown`），让所有客户端正常连上。
- 它同时是**体内标记**：用一个唯一名字（如 `/usr/libexec/mobileactivationd-r34`）就能在
  `exec %s` 行里证明"这一窗刷的是 r34"。

---

## 4. Q3 体内标记（结论不变，补一条）

- 首选仍是 **`exec <name>` 名标记**：加一个 RunAtLoad 的 `/usr/libexec/r34mark`（拷现成小二进制），
  环里出现 `exec r34mark` = 镜像身份实锤。零代码、零重签、不受 sandbox 写 console 的限制。
- 次选是 stub 自己的路径名（见 3.3），与 kill-switch 合并做。
- 金标准：内核启动时直接读 userdata 的判别字节并 `printf`——要一轮 CI，但从此永久解决。
- 另外：环里的 `boot-args = ...` 行来自 boot.img 的 cmdline，**不能**用来判 userdata 版本；
  但 `CRASH ... slide 0x...` 是共享缓存滑动值，同 boot 相同、跨 boot 随机，可当"本窗指纹"。

---

## 5. Q4 SIGABRT 波：重判为"很可能同根"

新证据把这个问题从"LS 墙的下游"改判为**"容器基础设施缺失"的另一个面**：

- `/private/var/containers` 不存在 → 任何调用 `container_copy_path` /
  `container_system_group_path_for_identifier` 的 daemon 都会拿到 NULL；
  iOS 里这类失败的典型处理就是 `fatalError`/`assert` → SIGABRT。
- accountsd / chronod / dmd / liveactivitiesd / cloudd / ScreenTimeAgent / profiled
  都属于"要容器"的一批（账号库、Duet、CloudKit、配置库都在自己的容器里）。
- 这也解释了为什么它们全部在 LS 墙之后才出现：它们是 seeding 后期才起动的那批，
  起动时容器服务已被前面的事情拖住。

**排查顺序（修订）**：
1. 先按 §3.1 S1 修容器/entitlements，然后看这 8 个是否自己归零——一次实验同时验证两个问题。
2. 若残留，只符号化 accountsd 的 pc/bt（10 帧，现成配方），看 assertion 文本。
3. 采 `containermanagerd` 与 `installd` 的栈（它们已在 sweetsample 名单里，但三窗都没报过栈）。

---

## 6. Q5 lsd seeding 复核（更新）

- t1 的 `dispatch_semaphore_wait` 在 **libMobileGestalt** 内部；它和 t2 的 MadGate 调用是
  两条不同的子任务，没有证据表明 t1 等 t2。
- **但新的怀疑**：`MIInstalledInfoGatherer → MIInstallationIdentity` 这条路本身要查容器/
  安装身份；如果容器基础设施是坏的，t1 也可能因此卡住（栈顶的 libMobileGestalt
  可能只是它顺手调的一个答案查询）。所以"修好容器"可能同时解开 t1。
- 若 t2 被 S1/S3 解开后 t1 还在原地：按老办法做——取等待点返回地址，反汇编 libMobileGestalt，
  找同库 `dispatch_semaphore_signal` 的调用者，确认信号者是内部线程还是外部通知。
- **不要假设** "MadGate fast-fail ⇒ seeding 收尾"。t1 是独立的第二道墙的概率不低。

---

## 7. Q6 判据树（重写，适配新方案）

**Gate 0 环新鲜度**：首行 banner + `slide` 值；`exec` 总数应在 ~2000 量级。

**Gate 1 镜像身份**：`exec r34mark`（或 stub 的唯一路径名）出现。
- 不出现 → 本窗作废，先查刷写通道，不看任何后续判据。

**Gate 2 守护进程活性**：
- 若仍用真 daemon：`CRASH mobileactivation` 计数应 = 0；若仍崩，看 pc 是否仍是 0x1BF5BC
  （容器）——若变成 0x1BF3BC/0x1BF290 等，说明换了一个守卫，按新消息走。
- 若用 stub：`exec` 出现且无 CRASH，且环里应有 stub 的 check-in 迹象（或直接把应答行为
  设计成可观测，例如首次应答时打印一行到 /dev/console——sandbox 允许的话）。

**Gate 3 主墙**：lsd t2 不再停在 `MadGate copyActivationRecord`：
- 若 t2 消失/返回 → 主墙解开；
- 若 lsd 出现 CRASH → 错误应答路径有问题（回 S2 改字典形状）。

**Gate 4 第二道墙**：t1 仍在 `libMobileGestalt sem` → 独立墙，走 §6 的 RE。

**Gate 5 LS 解锁**：SB t0 离开 `os_state_request`，bbd 离开 reseed。

**Gate 6 首帧内容**：swap 行 + 非零 surface。

**新增分支（原树没有）**：
- **手术生效但 SB 换新等待** = 进展，按新栈走；
- **stub 被拉起但没人连** = lsd 没走到 HRN 路径（另一种解锁），但仍要确认 `getServerStoreBlocking`
  的客户已返回；
- **daemon 不再崩但容器相关调用仍在别处失败** = 根因没修干净，回 S1。

---

## 8. 下窗数据请求（按性价比）

1. **`sweet_amfi_debug=1` 一窗**：看 mobileactivationd 是否 attach 到 entitlements（决定 S1 走向）。
2. **containermanagerd / installd 的采样栈**（已在名单，三窗都没报）。
3. **`/System/Library/FeatureFlags/Domain/DeviceManagementClient.plist` + `Global.plist` 内容**
   （390B 小文件，用 apfs-fuse 从 os.dmg 里直接读，或设备侧读）→ 决定 S3 数据层是否可行。
4. **390 个 `mobileactivation` raw 命中按路径枚举**（判 `.xpc` bundle 隐患）。
5. **`launchd_cache_loader` 的缓存文件路径与内容**（判 launchd cache）。
6. **`MobileActivationProtocol` 的方法/键名**（写 stub 用；`ipsw dyld disass --image MobileActivation`）。
7. accountsd 的 pc/bt 符号化（若容器修好后仍有 SIGABRT）。

---

## 9. 决策摘要

1. **崩溃不是激活问题**：是 `container_system_group_path_for_identifier` 返回 NULL。
   任何针对"激活记录 / MobileGestalt / NSURL 资源"的补丁都不会有用。
2. **r33 的 fast-fail 设计是对的**；r33b 的永等反过来证明那一窗服务名仍注册
   —— 先解决"镜像/定义身份"，再谈 kill-switch。
3. **最短路径**：先用 `sweet_amfi_debug=1` 判 entitlements，再决定
   "修容器（S1）" 还是 "stub（S2）"；lsd 侧尽量走 **HRN feature flag 数据层**（S3）。
4. **SIGABRT 波与守护进程崩溃很可能同根**（容器），一次实验可以同时验证。
