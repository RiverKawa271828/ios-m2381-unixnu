# GLM r34 窗报告：宿主侧 RE（A1–A6）+ r34 出件

> 任务书 = `handoff/glm-brief-r34.md`；上游分析 = `handoff/deepseek-reply-20261022.md`。
> 本报告只陈述结论与证据；与 v2 分析的冲突单独列出（§3）。
> 状态：A1–A6 完成；r34 userdata 手术 = 撤补丁已落副本，其余四项待 rw 挂载后由
> `.re/r34-surgery.py` 执行；boot 镜像已出件。

---

## 0. 一页判读（给下窗直接用）

- **崩溃第一因成立**，且直接缺口已定位：镜像里 `/private/var/containers/Shared/SystemGroup/`
  是**空目录**，而 mobileactivationd 的 entitlements 要三个 system group 容器
  （`systemgroup.com.apple.mobileactivationd` / `systemgroup.com.apple.pisco.suinfo` /
  `systemgroup.com.apple.regulatory_images`）。r34 带上三个预置骨架。
- **假说 B 机制实锤**：`/System/Library/xpc/launchd.plist`（795KB，带 `.sig`，cdhash 过
  AMFI 信任缓存校验）= launchd 的任务缓存，内含 mobileactivationd **原始定义**。
  r33 只改磁盘 plist 对它零作用。**但** r34 的 r34mark 也是磁盘 plist——若缓存权威且
  不合并磁盘扫描，Gate 1 会失败；这个失败本身就是重要判读（见 §5 Gate 表）。
- **A1 有解**（零代码）：`os_feature` 的 plist SLOWPATH 可用
  `FEATUREFLAGS_ENABLED` 环境变量强制走 + `Global.plist` 注入 `HRN/Enabled=true`。
  已把两件都带进 r34；若 lsd plist 也被缓存遮蔽，则 envvar 不生效，Global.plist 单独
  只在「SHM 表为空」的设备上起作用——下窗用 AMFI/行为判据区分。

## 1. 任务 A 结论（每项：结论 → 证据）

### A1（HRN feature flag）：**数据层可行，方案如下（已入 r34）**

- 三个 plist 原文已读（挂载直读，全文存 `.re/out/a1-plists.txt`）：
  - `Domain/DeviceManagementClient.plist`（390B）：只有 `ESSO`/`ESSOTest` 两旗，
    `DevelopmentPhase=FeatureComplete`。**无 HRN**。
  - `Global.plist`（1489B）：六个域（SiriUI/AppKit/Messages/SpringBoard/InCallService/Kettle），
    条目形状 = `<域>{<旗>{Enabled:true}}`。**无 DeviceManagementClient 域**。
  - `GlobalDisclosures.plist`（2465B）：UUID→`Disclosed:true` 的披露表，23 条。
- **机制（反汇编 libsystem_featureflags，证据链）**：
  - `_os_feature_enabled_impl` @0x1c2646b90：先查全局 SHM 表
    （`__os_feature_table_once` @0x1c2647bec：`shm_open("com.apple.featureflags.shm")` +
    fstat + mmap，表空 = NULL）；表命中直接返回，**不读 plist**。
  - 表为 NULL、或环境变量 `FEATUREFLAGS_ENABLED`/`FEATUREFLAGS_DISABLED` 存在
    （`__os_feature_enabled_envvar_check_once` @0x1c2647d08，getenv 两者之一非空即置位）
    → 尾调 `__os_feature_enabled_SLOWPATH`（@0x1c2647d64）。
  - SLOWPATH 按 `__os_feature_search_paths` 逐路径读
    `GlobalDisclosures.plist` → `GlobalDisclosureOverrides.plist` →
    `ProfilesDisclosures.plist` → `SettingsDisclosures.plist` →
    **`Domain/<域>.plist`**（键序：`DisclosureRequired` → `Enabled` →
    `DevelopmentPhase`，值例 `FeatureComplete`）→ `Global.plist` → `Profiles.plist` →
    `Settings.plist`，结果缓存在 `__slow_cache`。
  - 调用方证据：`+[LSHRNSupport deviceConfiguredForHRN]` @0x181C258B0
    （CoreServices.framework）在 0x181C2595C `bl __os_feature_enabled_impl_stub`
    （stub @0x186d87ff4 → `__os_feature_enabled_impl` @0x1c2646b90）。
- **r34 方案**（两件都上，互补）：
  1. `Global.plist` 注入 `DeviceManagementClient → { HRN → { Enabled: true } }`
     ——形状与现有真实生效条目（如 SpringBoard/PictureInPicture）完全一致；
  2. `com.apple.lsd.system.plist` 加
     `EnvironmentVariables = { FEATUREFLAGS_ENABLED: "DeviceManagementClient/HRN" }`
     ——强制 lsd 进程走 SLOWPATH（绕过 SHM 表遮蔽）。
- **残余不确定**（报告不隐瞒）：envvar 值格式未 RE（仅确认"存在性"触发 slowpath）；
  lsd plist 若被 launchd 缓存遮蔽则 envvar 投递不到。两件的失效模式都是"仍为 false"，
  不会更坏。

### A2（entitlements 全表）：**完成，全文在 `.re/out/a2-entitlements.txt`**

- 来源：`.re/mobileactivationd.macho`（= 镜像内 `/usr/libexec/mobileactivationd`，
  2,383,376B，与 HFS 记录尺寸一致）的 CMS SuperBlob slot 5（XML）+ slot 7（DER），两槽一致。
- **`com.apple.security.system-group-containers`（3 项）**：
  `systemgroup.com.apple.mobileactivationd`、`systemgroup.com.apple.pisco.suinfo`、
  `systemgroup.com.apple.regulatory_images`。
- `com.apple.private.vfs.allow-low-space-writes = true`（在表内——setiopolicy_np 的
  EPERM 与否只取决于内核 AMFI 是否把 blob 挂上，Gate 2 裁决）。
- `com.apple.security.system-container = true`；`seatbelt-profiles = [mobileactivationd]`；
  `com.apple.private.MobileGestalt.AllowedProtectedKeys` = 11 键（WifiAddress…UniqueDeviceID，
  对 t1 MobileGestalt 墙后续有用）；keychain-access-groups = lockdown-identities / mfiaccessory。

### A3（containermanagerd 解析路径）：**daemon 中介，非客户端直扫；骨架方案可行（已入 r34）**

- 客户端链（反汇编 libsystem_containermanager.dylib，DSC 抽取件）：
  `container_system_group_path_for_identifier`（0x1c4c2d08c，activity 包装）→
  `___container_system_group_path_for_identifier_block_invoke`（0x1c4c2c4f4）→
  `__container_legacy_lookup`（0x1c4c4b6ac：组 container_query[class=0xd]、
  identifiers、`_container_query_operation_set_flags`、query[0x68]=3[查+建]）→
  `_container_query_get_single_result` → **XPC 同步**。失败时 invoke 里路径 NULL →
  错误码 0x35（53），对应 fatal。
- 服务名（dylib 字符串 + 调用点）：`com.apple.containermanagerd`（用户态）/
  `com.apple.containermanagerd.system`（root 态）。
- **launchd 配置实证**（挂载直读两份 plist）：
  - `com.apple.containermanagerd.plist`：`RunAtLoad=true`，`--runmode=normal`
    （feature flag `containermanagerd/user_system_split` 未开时的分支），注册
    `com.apple.containermanagerd` + `.upcall`（HostSpecialPort 25）。
  - `com.apple.containermanagerd.system.plist`：
    `Disabled = {#IfFeatureFlagDisabled: containermanagerd/user_system_split → true}`
    ——**release 上 `.system` 守护是被禁的**，system-group 查询由主 daemon 接。
- **镜像现状（修正 v2 事实 #2）**：`/private/var/containers` **存在半套骨架**（早前窗口
  预置）：`Data/System/D90C94C4-F94E-4FCF-8A28-A93BE3846D8E/`（metadata bplist：
  `MCMContainerType=System`，`MCMMetadataIdentifier=com.apple.mobileactivationd`，
  `MCMMetadataVersion=1`；内含 `Library/internal/data_ark.plist`：ActivationState=
  Activated 伪装件）——但 **`Shared/SystemGroup/` 为空目录**。entitlements 要的三个组
  一个都没有 = 客户端拿 NULL 的直接缺口。
- **离线预置判定：可行（扫描型映射）**。依据：①客户端库自带
  `_container_paths_enumerate_containers_at` / `_container_paths_copy_container_from_path`
  （枚举目录 + 读 `.com.apple.mobile_container_manager.metadata.plist`，键族
  MCMMetadataUUID/ContentClass/Identifier/UserIdentity/Persona——真实设备上 group 容器
  目录名是每台随机 UUID，必为扫描映射而非推导）；②daemon 链接同一
  libsystem_containermanager + ContainerManagerSystem/User.framework，沿用同一套
  路径模板（`containers/Shared/SystemGroup`、`Containers/Shared/AppGroup` 等在库内字符串）。
- **r34 骨架**（写入 Shared/SystemGroup/，0755 root，metadata 0644 bplist）：
  三个 `<uuid>/`，每个 metadata = `MCMContainerType=SystemGroup` +
  `MCMMetadataIdentifier=<组名>` + `MCMMetadataUUID=<uuid>` + `MCMMetadataVersion=1`
  （键族取镜像现有先例 + UUID 补充；MCMMetadataContentClass 值格式未实证，宁缺勿猜）。
  UUID：5A34E7D2-9C41-4F6B-A1D8-3E2C7B90D411 / 6F25C8E3-AD52-4A7C-B2E9-4D1F8C03E522 /
  7C36D9F4-BE63-4B8D-C3FA-5E2A9D14F633。
- **未闭合环节（下窗判据覆盖）**：主 daemon r33b 窗只 exec 1 次且无 CRASH 行——它可能
  起来后在等、也可能干净退出。骨架若格式对、daemon 活，Gate 3 应看到 mobileactivationd
  换守卫或不再崩；若 daemon 死，骨架无效，下一步 RE ContainerManagerSystem.framework
  的启动依赖。

### A4（390 命中按路径枚举）：**完成，444 命中全映射（`.re/out/a4-hits.txt`）**

- 全图扫描（4K 块经 HFS extent 映射到文件）：444 处 `mobileactivation`，**除已改名 plist
  外的声明方只有一处 = `/System/Library/xpc/launchd.plist`（7 命中，含 mobileactivationd
  原始条目）**。无 `/System/Library/XPC/` 声明、无 `.app/XPCServices/` 声明。
- 其余命中皆为内容引用（daemon 二进制本体 35、Migrator 19、 jetsamproperties D22/D221
  各 3=被动、ams/akd/lockdownd 等客户端、AppSignatures/AppPlaceholders 拷贝、
  HFS+ Private Data 的 iNode 硬链接体）。
- **`.xpc` bundle 隐式注册嫌疑：排除**。

### A5（launchd job cache）：**命中——B 的机制与文件定位**

- `launchd_cache_loader`（72KB，PROJECT libxpc-2462.140.4.702.1）反汇编主流程：
  `kern.bootargs` sysctl → 含 `launchd_unsecure_cache=` 时用其后随值作缓存路径
  （"Using unsecure cache: %s"），否则默认
  **`/System/Library/xpc/launchd.plist`** + `.sig`；
  SecStaticCode + 分离签名 + `SecStaticCodeCheckValidity` →
  **AMFI userclient 校验 cdhash 是否在信任缓存**（"cdhash: %s is trusted"）→
  "Sending validated cache to launchd"。
- 镜像内该文件存在（795,373B，CNID 307515；`.sig` 19,006B），**plist 内容含
  `LaunchDaemons` 全库，其中 `/System/Library/LaunchDaemons/com.apple.mobileactivationd.plist`
  条目为原始定义**（Label `com.apple.mobileactivationd` + 原 MachServices）。
- ⇒ **r33 改名对缓存零作用**（缓存 cdhash 未动，照常被信任装载）。r34 不碰它
  （任务书禁令），B 的解法（删/换缓存 = 签名失效风险，或 stub 化）留给 r35 拍板。
- 我们 cmdline 里的 `launchd_unsecure_cache=1`：boot-arg 值=路径，"1" 是不存在路径——
  加载失败后的回退语义未 RE 完（失败分支在 0x100002884，未走查）。

### A6（MobileActivationProtocol 形状，为 stub 备）：**完成**

- 协议方法面（daemon 侧 `MobileActivationDaemon` 全表，`.re/out/a6-a2s.txt`）：
  `copyActivationRecordWithCompletionBlock:`、`handleActivationInfo:withCompletionBlock:`、
  `handleActivationInfoWithSession:activationSignature:completionBlock:`、
  `createTunnel1ActivationInfo:options:withCompletionBlock:`、
  `getActivationStateWithCompletionBlock:`、`requestDeviceReactivationWithCompletionBlock:`、
  `deactivateDeviceWithCompletionBlock:`、`isDeviceBrickedWithCompletionBlock:`、
  `unbrickDeviceWithCompletionBlock:`、`copyPCRTTokenWithCompletionBlock:`、
  `copyDCRTWithCompletionBlock:`/`issueDCRT:withCompletionBlock:`、
  `copyUCRTVersionInfoWithCompletionBlock:`/`issueUCRT:withCompletionBlock:`、
  `copyRTCResetCountWithCompletionBlock:`、`copyAttestationDictionaryWithCompletionBlock:options:completion:`、
  `copyLegacyDeviceIdentityWithCompletionBlock:`/`deleteLegacyDeviceIdentityWithCompletionBlock:`、
  `issueClientCertificateLegacy:WithCompletionBlock:`、`isInFieldCollectedWithCompletionBlock:`、
  `copyAutomaticTimeEnabledWithCompletion:`、`copyMonotonicClockWithCompletionBlock:`、
  `updateBasebandTicket:baaCertData:baaIntermediateCert:options:withCompletionBlock:`、
  会话族（sessionHelloMessage/handleSessionResponse:withError:/generateSession/…）。
- 应答键（daemon 字符串）：`ActivationRecord`、`FactoryActivationRecord`；
  MadGate 侧经 `synchronousRemoteObjectProxyWithErrorHandler:` 消费。stub 回
  `{ActivationRecord: <空记录>, Error: <码>}` 字典即可满足 copy 调用；精确键序建议
  r35 前补一刀 MadGate 解码侧反汇编（MobileActivation.framework 在 DSC 外，需从
  根盘刻出）。

## 2. 任务 B 出件

### 已完成

1. **撤 8 字节补丁**：`iOSRoot-raw-r34.img` 内 `/usr/libexec/mobileactivationd`
   （raw 基址 0x206f37000）@0x4F944 `3700669b→3600669b`、@0x4F9B8 `37006337→36006337`
   （tbnz→tbz，清 bit24），双写双验（`.re/out/revert-log.txt`）。
2. **boot 镜像**：`shim/out/shim-boot-m2381.img` = **`339b8480…`**（5,734,400B）。
   内核 = r31 CI 件 `012f596f…`（/tmp/r31/kernel.development.m2381 实测同 sha），
   ADT = 现役 `c85b01f0…`（未再生）；cmdline = r32 权威串（自 shim-boot-r32.img
   解压提取，394B，含 sweetscan/sweetfb 参数）+ ` sweet_amfi_debug=1`（413B），
   回验通过（`.re/out/r34-cmdline-verify.txt`）。payload = `4561673d…`（16,599,584B，
   与 r31 payload 同尺寸 = cmdline-only）。

### 待 rw 挂载后执行（`.re/r34-surgery.py`，一次跑完自动验证）

3. `/usr/libexec/r34mark`（= `vm_stat` 拷贝，Apple 签名原样，只读打印即退，环中从未
   出现）+ `/System/Library/LaunchDaemons/com.r34.mark.plist`（RunAtLoad）。
4. 容器骨架三件（见 A3）。
5. `Global.plist` 注入 HRN（见 A1）。
6. `com.apple.lsd.system.plist` 加 `FEATUREFLAGS_ENABLED`（见 A1）。
7. **不碰**：com.apple.mobileactivationd.plist（r33 改名保持原样）、
   `/System/Library/xpc/launchd.plist(.sig)`、内核。

### 出件参数

- `fsck.hfsplus -f -y` → `-n` 复验：**两遍全绿**（"The volume iOSRoot appears to be OK"，
  `.re/out/fsck-r34.txt`）。
- **simg→raw 回程对拍**：`iOSRoot-s-r34.simg` unsparse 后与手术原图 `cmp` 全等
  （ROUNDTRIP_IDENTICAL）。
- **手术 toolbox 教训（记录给后续）**：`containers/Shared/SystemGroup` 三层目录的盘上
  uid/gid 字段原值 = 0，而本镜像 hfsplus 驱动的属主约定 = **0x80000**（Apple 全系目录
  均此值）——uid 0 在驱动眼里是 nobody(65534)，root 也被拒 mkdir/chmod/chown 全 EPERM。
  修复 = umount 后直接改三条目录记录的 @32/@36（0→0x00080000，等长写，带签名断言+
  回读验证，`.re/r34-fixperm.py` + `.re/out/fixperm-log.txt`）。新种子目录由驱动创建，
  落盘 uid 一致。
- 全图 4K diff（基线 = r33 raw `0750ae6f…`；完整表在 `.re/out/r34-diff.txt`）：
  **626 块 / 23 段**，全部归因：
  | 段（4K 块） | 归属 |
  |---|---|
  | 0 | 卷头（writeCount/mountCount） |
  | 61, 65 | 分配位图（新块登记） |
  | 183682–185212、1761648–1763342 各段 | 目录 B-tree（新增 r34mark/plist/骨架条目 + 三条属主补丁记录 @0x1ae1cc7ee/0x1ae809d90/0x1ae809fec + 节点改写） |
  | 224644–224687、1966889–1967042 | 驱动新分配块（新文件数据 + 元数据） |
  | 308558 | `/System/Library/FeatureFlags/Global.plist` |
  | 635261 | `/System/Library/LaunchDaemons/com.apple.lsd.system.plist` |
  | 2125623–2126067 | `/usr/libexec/mobileactivationd`（驱动整文件重写；内容 = 原版 + 8 字节 tbnz→tbz 还原 ⇒ **字节回到 stock，CDHash 与原版一致，r33 的 cs_invalid_page 噪音源随之消失**） |
- **基线对拍**：`iOSRoot-raw.img`（= r33 simg 97342037 的未稀疏源）sha `0750ae6f…`，
  与副本 `iOSRoot-raw-r34.img` 手术基线一致。

### 出件 sha（设备窗对拍用）

```
userdata: iOSRoot-s-r34.simg  sha256 = 7aaab96f5ce236f7628d90e7a045a4ff26c58924403ac6c170ed14aa3dc4848c  (12,145,451,600B)
boot:     shim-boot-m2381.img sha256 = 339b8480b71c40ec940772623e16b15875256624073e9533a45280ea8bc3b32e  (5,734,400B)
```

基线复核：`iOSRoot-s-r33.simg` 实测 sha = `97342037cf8c…`，与交接单一致 ✓。

⚠ **boot 镜像换新**：r34 的 `sweet_amfi_debug=1` 走 cmdline ⇒ boot 必须换新件
`339b8480…`（内核 012f596f 与 ADT c85b01f0 均未动，payload `4561673d…` 与 r31 同尺寸
= cmdline-only）。**不要**再刷 b8cddc98（那是 r32 的 cmdline，没有 AMFI 调试行）。

## 3. 与 v2 分析的冲突/修正

1. **「镜像里没有 /private/var/containers」不成立**（v2 §0 #2）：容器树存在（半套，
   早前窗口预置，含 data_ark 伪装件），缺的是 `Shared/SystemGroup/` 下的三个组。
   判读不变（仍是容器 NULL 致命），但 r34 的修复是"补全骨架"而非"从零建树"。
2. **r33b「改名无效」的解释不是单选题**：除了假说 A（整对重刷），现在有实锤的
   **签名缓存机制**（A5）——即使 r33 镜像送达，磁盘改名也可能被缓存遮蔽。
   r34mark（Gate 1）+ 缓存内无 r34mark 条目（新文件）的组合恰好能裁决：
   r34mark 出现 ⇒ 磁盘声明仍被合并（A 假说成分大）；不出现 ⇒ 缓存权威遮蔽（B 实锤）。
3. **「 MadGate 永等 ⇒ 服务名仍注册」** 在缓存遮蔽下同样成立，但注册源可能是缓存
   而非磁盘 plist——判读时不要把它单独当 A 的证据。

## 4. 设备窗判据表（用户执行；Gate 编号沿 brief §4）

| Gate | 判据 | 读数方法 | 解读 |
|---|---|---|---|
| 1 | `exec r34mark` 出现 | 环 grep | 否 → 本窗作废查刷写通道；**另**：不出现且其它变化（AMFI 行）出现 ⇒ 缓存权威实锤（r35 走 stub/拆缓存） |
| 2 | `SWEET AMFI: mobileactivationd: N entitlements` | 环 grep AMFI | N≥20 = entitlements 挂载正常（S1 走容器向）；N=0/无行 = AMFI 挂载缺失（S1 转修 AMFI） |
| 3 | `CRASH mobileactivation` 计数与 pc | 环 grep CRASH | 0 = 容器修复生效；仍崩但 pc ≠ 0x1BF5BC+slide = 换守卫（按新 fatal 走）；仍 0x1BF5BC = 骨架未匹配/daemon 未答（下一步 RE ContainerManagerSystem 启动依赖） |
| 4 | lsd t2 是否仍吊 MadGate | SMP lsd 栈 | 不吊 = 主墙开（若经 HRN 门 = A1 kill-switch 生效；若经 fast-fail = 服务名解析变化）；仍吊 = daemon 崩溃循环未断 |
| 5 | 容器相关 SIGABRT 波（accountsd/chronod/dmd 等 15-16×者） | 环 CRASH 计数 | 同比下降 = 容器根因面修复的旁证；不降 = 骨架只救了 mobileactivationd 或没救 |
| 6 | r34mark 若出现：有无 `SWEET AMFI: r34mark: N` 行 | 环 grep | 有 = 标记件也过了签名链（双保险身份证据） |

环境补充判据：环首行 slide 指纹（同 boot 相同）；`boot-args` 行来自 boot.img（r34 应含
sweet_amfi_debug=1——它**能**当 boot 版本身份用，与 userdata 身份判据分开）。

## 5. 遗留/建议（不改变 r34 方案）

1. `launchd_unsecure_cache=1` 的语义 = 缓存**路径**；r35 可实验改成明确不存在的绝对
   路径或直接删该旗，观察 launchd 是否回退磁盘扫描（一枚 boot-arg 的实验，零镜像改动）。
2. r35 若走 stub：A6 方法面 + 应答键已备，缺 MadGate 解码侧一刀反汇编。
3. `MobileActivation.framework` 不在 DSC（A6 用的是 daemon 本体符号）；从根盘刻出即可。
4. MCMMetadataContentClass 的合法值未实证；若 r34 骨架不被认，下一步先 RE daemon 的
   metadata 读取函数再补键。
