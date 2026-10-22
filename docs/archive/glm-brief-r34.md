# GLM 接手任务书：r34 出件（宿主侧 RE + 镜像构建）

> 上游分析 = `handoff/deepseek-reply-20261022.md`（v2，含反汇编实证）。
> 本任务书只列**要做的事和通过标准**。设计判断已经做完，不要重新论证方向。
> 设备刷写由用户执行；你产出镜像 + 一份可复核的报告。

---

## 0. 当前已确认的事实（不要重复验证）

1. mobileactivationd 的崩溃第一因 = `container_system_group_path_for_identifier()` 返回 NULL
   → `fatalError("Failed to copy container path(s).")` → `os_crash` + `brk #1`。
   反汇编证据：`main` @ 0x4F728，四条 `cbz` @ 0x4F828–0x4F834 → 0x5062C → `0x1BF56C`；
   崩溃 pc 0x1BF5BC、bt 0x50630 与此精确对应。
2. 根盘镜像初始态**没有 `/private/var/containers`**（全卷 321,389 条 listing 零命中）。
3. MadGate 走 NSXPCConnection 同步代理 + errorHandler，服务未注册时快速返回 nil。
   **MadGate 永等 ⇒ 那一窗服务名仍注册着**（旧定义仍在）。
4. `deviceConfiguredForHRN` = `os_feature_enabled_impl("DeviceManagementClient","HRN")`；
   flag 为真时直接返回 true，不调 MadGate。
5. 第二条潜在 fatal：`setiopolicy_np(9,0,1)` 需要 entitlement
   `com.apple.private.vfs.allow-low-space-writes`。

## 1. 禁止事项

- **不要**再改 LaunchDaemon plist 的 Label/MachServices（r33 已证明对 B 无效）。
- **不要**重建内核，除非任务 A3 明确要求（`sweet_amfi_debug` 是 Tunable，走 cmdline 即可）。
- **不要**在没有 `sha256sum` 对拍 + 体内标记的情况下出件。
- **不要**改 reference/ 下的两个只读克隆。

---

## 2. 任务 A：宿主侧 RE（无设备，按顺序做）

每项都要产出「结论 + 证据路径/地址」，不接受"看起来像"。

### A1（最高优先）HRN feature flag 能否在 release 构建里打开
- 用 `apfs-fuse` 挂 `~/mkiosroot-work/os.dmg`（配方见 `tools/mkiosroot.sh`），读：
  - `/System/Library/FeatureFlags/Domain/DeviceManagementClient.plist`（390B）
  - `/System/Library/FeatureFlags/Global.plist`
  - `/System/Library/FeatureFlags/GlobalDisclosures.plist`
- 判定：`HRN` 在哪个 set 里（Development/Disclosure/Profile/GlobalDisclosure），
  release 构建（xnu-8796.141.3，非 internal）是否会读它。
- **通过标准**：给出一个具体的 plist 修改方案（文件 + 键路径 + 值），或明确判死"数据层不可行"。
  这一项决定 r34 是否带零代码 kill-switch。

### A2 mobileactivationd 的 entitlements 全表
- 从根盘镜像里的 `/usr/libexec/mobileactivationd` 的代码签名提取 XML entitlements
  （原始 raw 镜像偏移 0x206f37000；或从 os.dmg 直接取）。
- 重点列出 `com.apple.security.system-groups` 的值（容器路径就是按它解析的）。
- **通过标准**：列出全部 key，特别是 system-groups 数组 + `com.apple.private.vfs.allow-low-space-writes`。

### A3 containermanagerd 的 system group 解析路径
- 反汇编/strings `containermanagerd` 与 `libsystem_containermanager`，
  确认 `container_system_group_path_for_identifier` 是：
  (a) 扫 `/private/var/containers/Shared/SystemGroup/<uuid>/.com.apple.mobile_container_manager.metadata.plist`，
  还是 (b) 需要一份数据库/索引。
- 若是 (a)：给出一份「离线预置容器骨架」方案（目录 + metadata plist 的最小字段），
  用 A2 拿到的 group id 生成。这是最有希望让 daemon **原生跑起来**的修复。
- **通过标准**：能/不能离线预置的明确结论 + 最小字段清单。

### A4 按路径枚举 390 个 `mobileactivation` raw 命中
- 之前只做了归类。要输出「偏移 → 所属文件路径」，重点看
  `/System/Library/XPC/`、`.app/Contents/XPCServices/`、`/System/Library/Caches/`、`/private/var/db/`。
- **通过标准**：是否存在除已改名 plist 之外的第二个声明方。

### A5 launchd job cache
- `strings`/反汇编 `/usr/libexec/launchd_cache_loader`，找出它读的缓存文件路径；
  在镜像里确认该文件是否存在、是否含 `mobileactivationd` 的旧 label/MachServices。
- **通过标准**：路径 + 是否命中，二选一。

### A6 MobileActivationProtocol 的协议形状（为 stub 备）
- `ipsw dyld disass <cache> --image MobileActivation`（或按符号）取
  `MobileActivationProtocol` 的方法名与 `copyActivationRecordWithCompletionBlock:` 的应答结构。
- **通过标准**：请求/应答的键名清单，够写一个「回空记录」的 stub。

---

## 3. 任务 B：r34 出件（必须包含）

基线 = r33 的 userdata simg（`iOSRoot-s-r33.simg 97342037`），在其上做增量：

1. **体内标记**（必做）
   - 新建 `/usr/libexec/r34mark`（拷一个现成小二进制，不写代码）+
     `/System/Library/LaunchDaemons/com.r34.mark.plist`（`RunAtLoad=true`）
   - 判据：环里出现 `exec r34mark`。
2. **cmdline 增量**（必做）：`sweet_amfi_debug=1`。不重建内核。
3. **撤掉 8 字节补丁**（必做）：把 mobileactivationd 恢复为原字节
   （两处 tbnz→tbz @ file 0x4F944 / 0x4F9B8）。目的是净化变量、减少 cs_invalid_page 噪音。
4. **kill-switch 候选**（按 A 的结论二选一，能两个都带更好）：
   - A1 可行 → 写 feature flag plist（零代码）；
   - A3 可行 → 预置 system group 容器骨架（数据）。
   若两者都判死 → r34 不带 kill-switch，纯诊断窗（可接受，但要在报告里写明）。
5. 出件后：`img2simg`（4K 块，注意 ulimit -v 2G 防 OOM）→ sha256 → 写进交接单。

**通过标准**：镜像 sha、与基线的全图 diff 清单（精确到偏移）、每项改动的 7z/plist 回验结果。

---

## 4. 设备窗（用户执行，你只给清单）

1. 刷前 `sha256sum` 对拍交接单里的 sha，**复合命令**里带文件名，不接受"目录里同名一族"。
2. flash boot_b（沿用 `b8cddc98`）+ userdata r34 → `set_active b` → 开机。
3. ≥9.5min 窗 → pwr+vol↓ → 刷 mu-r69 + nura-min → 读环。
4. 判据（按顺序）：
   - **Gate 1**：`exec r34mark` 出现？否 → 本窗作废，查刷写通道。
   - **Gate 2**：`SWEET AMFI: mobileactivationd: N entitlements` 的 N 是多少？
     N=0 或无此行 → entitlement 没挂上（S1 走"修 AMFI 挂载"）。
   - **Gate 3**：`CRASH mobileactivation` 计数；若仍崩，pc 是否仍 0x1BF5BC？
   - **Gate 4**：lsd t2 是否还吊在 `MadGate copyActivationRecord`？不吊 → 主墙解开，看 SB。
   - **Gate 5**：`/private/var/containers` 相关容器调用是否仍失败（看 daemon 是否换了 fatal 点）。

---

## 5. 交付物

- `handoff/glm-report-r34.md`：A1–A6 的结论（带证据）+ r34 镜像 sha + diff 清单 + 设备窗判据结果表。
- 不要改设计方向；如果某项结论与 v2 分析冲突，把冲突写进报告，不要自行改方案。
