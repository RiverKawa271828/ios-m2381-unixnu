# 任务书：iOS 16 用户态在骁龙 8550（SM8550）上的 GUI 死锁独立分析

> 给 DeepSeek：请只做**分析和方案设计**，不要写操作步骤的执行细节（动手由我方完成）。
> 你拿到的证据全部来自内核级日志环 + 线程采样器 + 符号化（ipsw/DSC a2s），可信度高。
> 答复请按文末「期望输出」的六问逐条组织。

## 一、项目背景（一段话）

在魅族 20 Classic（SM8550，16GB LPDDR5X，UFS 4.0）上启动**真实的 iPhone X（iPhone10,3）
iOS 16.7.16（20H392）用户态**，内核为移植到高通平台的 xnu-8796.141.3（Darwin 22.6.0）。
启动链：ABL → shim（Linux Image 头伪装）→ XNU 内核 → UFS 根盘（userdata 分区上的 HFS+，
由 IPSW 重建，无 GPU、无基带、CPU 软渲染）。同源项目已在两台别的机型上**到达桌面**：
SM7150（sweet，iOS 16 xnu-8796）与 SM6225（topaz，home screen + 触摸 + 按键 + Wi-Fi 全通）。
我们这台：内核 17 分钟窗零 panic 稳定运行，launchd/守护进程大规模起动，
**但 GUI 永远黑屏（日志滚完后黑屏）**。

## 二、已验证打通的部分（不需要你再怀疑方向）

- 内核平台移植（GIC/PSCI/timer/SMP 8 核/UFS root/HFS+/AMFI/sandbox）全部工作，
  17min 窗零 panic。
- 显示管线五段全通：surface create → lock/unlock → SwapBegin/End → swap → CPU 拷贝入
  面板内存；合成器已 present 过首帧（黑 = surface 内容为空，非拷贝问题）。
- AKS（AppleKeyStore）sel 35 正常应答 0x0；IOKit 访问控制对 suggestd 等守护进程正常放行。
- launchd 正常 exec 守护进程群（每窗 ~2400 次 exec、180+ 个不同进程名）。

## 三、卡点：证据链（三个 17 分钟取证窗，结论一致）

### 3.1 现象

每次启动：内核/守护进程日志滚动 → 黑屏。SpringBoard 起来、走 11 发 AppleKeyStore 调用
（sel 0→35→7→35→17×6→7，全部成功返回 0x0），然后主线程**整窗静止**（8 次采样 × 120s
间隔同一栈），backboardd 画了黑根帧后 idle。GUI 永远拿不到内容。

### 3.2 死锁栈（符号化实锤，逐帧跨窗全同）

- **SpringBoard 主线程（后期）**：`__os_state_request_for_self` block → `dispatch_sync`
  → 等 LS unfair lock（前期栈 = CCSModuleRepository init → LSApplicationRecord init）。
  LS 线程吊在文档化的 `LAUNCH_SERVICES_IS_RESEEDING` 同步 XPC 上，永不回话。
- **backboardd**：一线程吊在同一个 reseed XPC。
- **lsd（服务端全景）**：
  - t1 = `MIInstalledInfoGatherer _populatePlaceholderRecordValues →
    MIInstallationIdentity → GenerateBuiltInAppInstallSessionID → libMobileGestalt →
    dispatch_semaphore_wait`（seeding 工作者，等 MobileGestalt 信号量）；
  - t2 = `+[LSHRNSupport deviceConfiguredForHRN] → initMAECopyActivationRecord →
    -[MadGate copyActivationRecord:] → _copyActivationRecord →
    xpc_connection_send_message_with_reply_sync`（**同步 XPC 永等，全程 15 分钟不退**）；
  - t3/t5 = `-LSDatabaseBlockingFetchClient getServerStoreBlockingWithCompletionHandler: →
    LAUNCH_SERVICES_CLIENTS_ARE_WAITING_FOR_THE_DATABASE_TO_FINISH_SEEDING__ →
    dispatch_group_wait`（正在吊死 SB/bbd 的那条请求的处理现场）；
  - 主线程 journal recovery 曾吊 dispatch_sync，但中后期自行逃出转 runloop（非死结）。
- 因果链：**lsd 的 seeding 里有一发同步 XPC 向 mobileactivationd 要激活记录 → 对端崩溃
  循环永远无法应答（XPC 无超时）→ seeding 永不收尾 → LS 数据库锁不放开 →
  SpringBoard/backboardd 全部吊死 → GUI 黑屏。**

### 3.3 对端：mobileactivationd 崩溃循环（环内原文，最全一次）

```
CODE SIGNING: cs_invalid_page(0x1004d0000): p=1247[mobileactivation] final status 0x22000000, allowing (remove VALID) page
CRASH mobileactivation[1247] sig 5 code 0x600000001 sub 0x1006435bc slide 0x36d84000 mh 0x100484000
CRASH mobileactivation pc 0x1006435bc lr 0x1006435bc sp 0x16f97b390 fp 0x16f97b3a0 far 0x10079c000
CRASH mobileactivation bt 0x1004d4630 0x1db3a8344 0x0
CRASH mobileactivation reason ns 2 code 0x5: (none)
Process[1247] crashed: mobileactivation. Too many corpses being created.
```

- 每窗 exec ~97-102 次（≈10 秒一次，launchd 节流节奏），**每次 exec 必崩**（崩 4 行/次）。
- 签名放行是我们自己打的 AMFI 补丁（`allowing (remove VALID)`，为了放行一个 8 字节二进制
  热补丁）——补丁页加载成功后，进程在 Swift `brk` 陷阱上自杀（SIGTRAP/EXC_BREAKPOINT）：
  pc=lr=mh+0x1bf5bc，回溯帧 mh+0x50630（已反编译确认 = 一个 Swift 大函数的**错误清理尾声**，
  全函数 ~13 处守卫跳入该尾声；当初只补了 2 个块首入口，打地鼠失败后 parked）。
- 每次 exec 前都有 `exec xpcproxy`（= launchd 是父进程，标准 job spawn 路径）。
- 同拍数据点：bluetoothd / fairplayd.H2 / coresymbolicationd 每窗也**恰好** exec 97 次，
  但零崩溃记录（正常退出循环）。

### 3.4 附加现象（可能同根，可能无关）

一批守护进程 SIGABRT 崩溃循环（每窗各 ~12 次）：**accountsd**（188 行 CRASH，量最大）、
chronod、dmd、liveactivitiesd、ScreenTimeAgent、cloudd、mmaintenanced、privacyaccountd、
profiled。accountsd 崩栈与 mobileactivationd 不同（sig 6，bt 10 帧，经
CoreServices/UIFoundation 层）。全部发生在 lsd 墙之后（它们是 seeding 后期起动的那批）。

## 四、已尝试的修复与查证（r33 手术）

**思路**：mobileactivationd 的 LaunchDaemon plist（`/System/Library/LaunchDaemons/
com.apple.mobileactivationd.plist`）**无 RunAtLoad、无 KeepAlive**（纯 on-demand），
其起动只能由 MachServices 名字的客户端连接触发。把 **Label 和两个 MachServices 键改名**
（`com.apple.mobileactivationd`→`…q`、`…d.lockdown`→`…d.lockdowq`，镜像里精确 2 字节
补丁，plistlib 完整解析验证通过），期望：launchd 不再注册旧端口 → MadGate 的同步 XPC
立即 error（fast-fail）→ seeding 收尾 → 链条解锁；副作用 = 无人能再拉起该 job，崩溃
循环消失（`exec mobileactivationd` 归零 = 上机直接证据）。

**镜像查证（四重，全部干净）**：
1. r33 simg 与上一版 r29 simg 全图 cmp **恰差这 2 个字节**；
2. 7z 抽出该 plist → plistlib 解析 ✓：Label/MachServices 全部已改名、ProgramArguments
   未动、无 RunAtLoad/KeepAlive；
3. launchd 全域内容级扫描：/System/Library/{LaunchDaemons, LaunchAgents,
   NanoLaunchDaemons, NanoLaunchDaemonsAltAccount, DeveloperModeLaunchDaemons} +
   /Library/LaunchDaemons + Caches + /private/var（497MB 全抽，/var 是 symlink）——
   引用 "mobileactivation" 的只有：改名后的 plist + 两份 jetsamproperties 配额表
   （按 label 列内存条目，被动，不起动不注册）；二进制路径串
   `/usr/libexec/mobileactivationd` 无第二引用方；
4. dyld 共享缓存在镜像里已清零（二进制全部实文件化，无缓存内副本）。

## 五、当前矛盾（请你重点分析的核心）

r33 出件后又跑了**第三把 17 分钟窗，崩溃循环 97 次照旧，全部栈与 Census 统计同构**
（mobileactivationd exec 102/99/97，其余进程数均在噪声内）。但这一把刷进去的 userdata
**是 r29 还是 r33 已不可判**：
- 刷完 iOS 后又刷了救援 Linux（2G sparse）用于读日志环，分区 2.6GB 处的判别字节被清零；
- 两版镜像全图只差那 2 字节，无任何其它在体标记可读；
- 宿主侧无刷写日志可查。

**两个并存假说**：
- **A**：又刷成了 r29（与上轮同版；两文件同目录同名一族只差 2 字节，tab 补全一位之差
  即中招；本轮未做刷前 sha 对拍确认）。
- **B**：r33 真上了机，而 iOS 16 launchd 存在我们没找到的机制仍然起动这个 job
  （Label + MachServices 都已改名、无 RunAtLoad/KeepAlive、全域无第二声明方——
  四重查证下我们找不到任何起动路径）。

## 六、已知参照与约束

- **topaz（SM6225，同版本用户态）带着同样的 mobileactivationd 崩溃循环上了桌面**——
  崩溃循环本身并不必然闸死 GUI；他们的 lsd seeding 显然没有卡在同一发同步 XPC 上。
  这是「绕过 mobileactivationd 可行」的最强旁证。
- 每次刷 userdata = 抹掉救援 Linux，救援 Linux = 读日志环的唯一通道（环在分区尾部，
  sparse 刷入不覆盖）。所以「刷 iOS → 跑 → 刷救援 Linux → 读环」是固定循环，
  iOS 上机期间无任何 shell/网络通道（只有屏幕和内核日志环）。
- 我们能做：任意改根盘文件/plist/二进制（本地重签 + 信任缓存重建已打通）、重建镜像、
  打内核补丁、内核级采样/打印。不能做：Apple 签名、真基带/激活服务器流程（无 modem）。
- rootfs 是全新解包的 iPhone X 16.7.16（/private/var 也是镜像内的初始态，无激活记录、
  无移动用户数据）。

## 七、请你回答的六问

1. **假说 B 的机制排查**：iOS 16 的 launchd/libxpc 里，还有什么路径可以起动/保持一个
   job，是其 plist 的 Label 与 MachServices 全部改名后仍然生效的？重点帮我们排查：
   LaunchEvents（com.apple.xpc.activity）在全新 /var 上的首次触发语义；launchd 是否存在
   服务名/label 缓存；jetsam 表与 job 生命周期有无联动；lockdownd 或其它守护进程用
   bundle 路径/endpoint 而非服务名拉起 peer 的可能。
2. **kill-switch 方案排序**：在「可任意改根盘 + 本地重签」的约束下，你认为让
   `MadGate copyActivationRecord` 不再永等的最佳手段是什么？请对以下候选给出行-hasard
   评估与排序，并指出各自的风险：
   a. 修好崩溃本体（把 Swift 错误清理尾声整体废除/替换为函数 epilogue，需要定函数边界；
      或者找到它 fatal 的第一因——激活记录缺失？MobileGestalt 某应答缺失？）；
   b. 把 `/usr/libexec/mobileactivationd` 换成 stub（常驻不退、应答空记录/错误）——
      关键问题：MadGate 对「连接成功但对端应答错误/空」是返回 nil 继续走，还是也有
      永等分支？
   c. 补丁 lsd 本体：让 `+[LSHRNSupport deviceConfiguredForHRN]` 返回 NO（HRN 是什么？
      这条路径在桌面机 topaz 上是否也会走到？）；
   d. 直接预置假激活记录（/var/root/Library/Lockdown/activation_records 之类路径），
      让 MadGate 走缓存命中而不问 mobileactivationd——已知社区有「离线激活记录」实践，
      iOS 16.7/iPhone X 的记录文件结构是否可用？lsd 的查询是否先于 MadGate 查缓存？
   e. 其它你想到的手段（plist Disabled=true 对 on-demand 连接的实际语义；
      xpc_reply 超时注入；内核层对这条 XPC 做应答伪造等）。
3. **假说检验设计**：下一把刷窗如何**在环里直接证明**刷进去的是哪一版镜像？（我们已
   计划体内标记——例如 r34 把二进制换成带独特行为的版本让环里留下不可混淆的痕迹；
   请评估哪种标记最可靠且工程量最小。）
4. **~8 守护进程 SIGABRT 波**（accountsd/chronod/dmd/liveactivitiesd/ScreenTimeAgent/
   cloudd/mmaintenanced/privacyaccountd/profiled）：在「移植的全新 rootfs、无激活、
   无基带、无 GPU」的背景下，最可能的共同根因是什么？给出排查顺序（哪些最先修、
   哪些可以无视）。
5. **lsd seeding 全链复核**：t1 卡 `MIInstalledInfoGatherer → libMobileGestalt →
   dispatch_semaphore_wait`——这个信号量由谁放？它和 t2 的 MadGate 调用是什么顺序/依赖
   关系？如果 MadGate fast-fail 成功，t1 会自然收尾吗？还是 MobileGestalt 这关是
   独立的第二道墙（我们在裸机上 MobileGestalt 的部分应答是补过的）？
6. **审我们的判据树**：①`exec mobileactivationd` = 0 且 cs_invalid_page = 0 → 手术
   生效，看 lsd/SB 新栈；②SB 走出 LS 墙出现新栈 → 解锁成立等首帧；③lsd 换新卡点 →
   采样定位下一环；④exec = 0 但栈仍同 → MadGate 失败后另有依赖。这个树有什么漏洞或
   遗漏的分支？

## 八、期望输出

- 六问逐条作答；结论先行、证据/推理随后；
- 方案类回答给出「推荐排序 + 每项的前置条件/风险/回滚」；
- 如需更多数据（环原文、反汇编、某个栈的全帧），直接点名，我们下窗补采。

—— 证据附件可按需提供：三个窗的日志环解析全文（各 ~6000 行）、全部符号化栈、
mobileactivationd 反汇编片段、镜像 diff 清单。
