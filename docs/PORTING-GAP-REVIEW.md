# 官方移植文档合规差距报告（2026-10-27，对照 unixnu.com/zh/porting/ 全 19 页）

> 范围 = 官方 porting 指南四个板块（准备/硬件/驱动/验证发布）逐页对照
> 我们的现役施工面（devices/m2381 + m2381Pkg + profiles）。结论先行：
> **骨架高度合规**（我们就是按 porting/ 五篇+上游模式建的），主要差距
> 集中在「验证与发布」板块的自动化（这正是 r55 取证 reader 补的）与
> 若干文档化义务。

## 1. 逐板块判定

### 准备移植 ✅ 合规
- **steps / migration / device-repository**：unixnu-m2381 = schema v2
  （soc_source=sm8550 + os_profile=ipados-17.7.11-21H461），结构/合并
  顺序（arch→soc→os→device）与 profiles.py 实现一致；orphan 分支
  device-m2381 供 CI ✅。
- **soc-support**：消费上游 UniXNU-sm8550-common feed（43da0d56），
  未自创 SoC 层 ✅。
- **components**：组件引用（component:GopFramebuffer 等）走上游组件化
  机制，含 archives pin（storage 静态 pin 先例）✅。

### 描述硬件 ✅ 合规（一处文档债）
- **device-tree**：dts/m2381.dts + v2 生成器（sm8550.dtsi 导出），
  stock FDT/DTS 留档 docs/hardware ✅。
- **uefi-firmware**：m2381Pkg = shengPkg 骨架 + mars blob + OEM UFSDxe
  Binary 集成，FD 窗口/PCD 全实测值 ✅。
- **dual-system-partition**：boot_b/recovery_a/userdata 布局 + board.toml
  LBA 实测 ✅。
- ⚠️ **文档债**：board.toml 里大量 `[DEVICE]`/实测注释很好，但官方
  **records（适配记录）**我们没有对应物——见 §2.4。

### 适配驱动 ⚠️ 部分落后（计划内）
- **drivers（适配顺序）**：官方顺序 = 存储→平台→显示→输入→GPU→网络
  →音频。我们实际走到了「显示」深层（=y 车道一级过，SB 用户态推进
  中），输入（触摸 GT9916 有硬件常量未上车）、GPU（ADRENO=n 计划内）、
  Wi-Fi（未开始）——**符合官方「按序适配」精神，无跳级** ✅。
- **platform/ufs/display/input/gpu/network/power**：每域的关键机制
  （NativeUfs、GopFramebuffer =y、LEGACY_DISPLAY 车道、BootReset、
  telemetry 条件化）都用上游原生机制，零 fork 主树 ✅；m2381 fork 面
  全部在设备仓（BootLog/BootProgress/BootReset/LegacyDisplayProbe）=
  官方期望的「board glue 不回上游」形态 ✅。
- **xnu/source-and-patches**：58 补丁全走 patches/xnu（rebase 随上游），
  text_fixups 走 /chosen 正道 ✅。

### 验证与发布 ⚠️ 差距集中区（r55 正在补）
- **building**：unixnu 全步骤 Linux 跑通（PR#100 后），CI macos.yml ✅。
- **bring-up**：启动验证顺序我们按 panic screen→绿蓝条→logo→SB 推进
  实践着，但没有显式的「检查点表」——records 缺位的另一面。
- **log-capture（本次重点）**：官方四方法对照：
  | 官方方法 | 我们的状态 |
  |---|---|
  | ①UEFI AdvancedLogDxe 二维码导出 | ❌ 未集成（AdvancedLogDxe 不在 m2381 FV；诊断页 BootProgress 已覆盖大部分需求，但固件期日志无导出通道） |
  | ②UART/屏幕拍照 | ⚠️ 屏幕照片在用（用户拍诊断页/logo）；UART 物理口未接 |
  | ③持久日志 + Linux reader | ✅ BootLog.c SHLOG 窗全在用；**r55 起内置 forensic reader（= 官方方法③的标准形态，fork package/linux-log）**，脱离 nura-min |
  | ④fastboot boot 手动/自动循环 | ❌ m2381 ABL 裁 CmdBoot，`fastboot boot` 死路（红线 5）；自动循环改用 sheng-autoreset（BootReset PMIC cell，待实测） |
- **records**：❌ 无官方格式的适配记录页——git commit message +
  STATE-history 实质承担了同等职责，但官方期望一份 per-device 可浏览
  记录（下步可从 STATE.md 派生）。
- **documentation（发布文档）**：README/docs 齐但未对齐官方 device
  page 格式（如 sheng 示例页）——非阻塞。

## 2. 行动项（按优先级）

1. ✅ **r55 forensic reader**（本报告同轮落地）：方法③补齐，循环里
   nura-min 出局。
2. **sheng-autoreset 实测**：r56 起 kernel_args 试加
   `sheng-autoreset=900`，看 [reset] 段 PMK8550 SDAM cell 探测——
   方法④的自动循环在我们的引导约束下的等价物。
3. **AdvancedLogDxe 集成**（中优先）：固件期（PE handoff 前）日志导出
   通道；需 AdvancedLoggerPkg 依赖进 m2381Pkg + tools=true。
4. **records 页**（低优先，文档债）：从 STATE §8 轮次索引派生一份
   per-device 适配记录（官方 records 格式：版本/参数/检查点/复位/
   输出/未解决错误五列）。
5. **bring-up 检查点表**（低优先）：把 r43→r53 已经验证过的启动检查点
   （panic screen / 绿蓝条 / 诊断页 / logo / SB 断言层）固化成表，
   放 docs/m2381-bringup.md。

## 3. 与官方形态的已知刻意偏离（非差距）

- **payload 载体 = recovery_a ESP（100MiB）**，非官方默认 boot 镜像
  内嵌——m2381 ABL 83.5MB gunzip 窗量化定死（AGENTS §0），官方
  dual-system-partition 页明确支持外置 payload 卷形态 ✅。
- **donor = iPad7,1 21H461**（同代配对拍板），设备身份走 ADT fixups
  （J120AP/product-id）——官方 device-tree 页的正道形态 ✅。
- **LEGACY_DISPLAY=y + GopFramebuffer**：上游主树驱动 + 组件化，=n
  时代的 displayless fixup 族换代退役——正是官方 drivers/display 页
  描述的终态路径 ✅。
