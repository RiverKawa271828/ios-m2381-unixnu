# 致主会话：arm64e 专项会话安排（2026-10-24，用户拍板）

> 来源：侧聊会话（Gate C 对策讨论）。用户决定：**另开独立会话专门研究
> arm64e 原生内核**；主会话继续门 A rebinder 主线。本信 = 背景论证 +
> 已核实事实 + 专项会话任务书 + 主会话待办。

## 一句话

arm64e 原生内核**可行（源码+硬件都支持）但不是当前推荐**：门 C 的首选
假设是"PAuth 关闭透传"（arm64 内核不启用 EnIA/EnIB/EnDA → 组件里全部
PAC 指令退化 NOP，无签名指针天然可用）；arm64e 只作专项研究对象，不挡
门 A/B/C 任何进度。

## 背景：Gate C 三对策重新排序（本侧聊论证）

1. **透传假设（新，升为首选）**：ARM 规范（FEAT_PAuth，DDI 0487）规定
   `SCTLR_EL1.EnIA/EnIB/EnDA` = 0 时 `PACIASP`→NOP、`BRAA`/`BLRAA`→普通
   `BR`/`BLR`、`LDRAA`→普通 load。21H16 组件存储值本来就是**无签名裸地
   址**（fileset KC 设计 = 加载时再签），所以 arm64 内核只要从不启用
   PAuth，组件全部验签消费点直通——**零签名工作**。
2. **对策①'（垫底一）**：arm64 内核开已知 keys + prelink 路径加运行时
   再签名 hook（UniXNU 已有自定 prelink 钩子先例：sparse-index /
   security-init 版本符号）。
3. **对策②（垫底二）**：QARMA 离线预签名（QEMU 有现成 QARMA 实现可
   移植；要求固定加载地址）。
4. **arm64e 原生内核（专项研究主题）**：把门 C 做难而不是做没了——
   arm64e 内核必须开 PAuth（自己代码全是 PAC 序言），组件 GOT 就真的
   需要有效签名，还得复刻 Apple 的"加载时再签"；且偏离 fuxi/nx729j
   全链实证（他们全是 arm64）。唯一硬收益（原生吃 A12+ 供体）已被
   rebinder 拿到。

## 已核实事实（专项会话直接引用，勿重复验证）

- **源码支持**：工作树 `out/feeds/xnu-0b4b910d`（xnu-10063.141.1，即
  Apple 出厂 A12+ arm64e 内核的同一份）：
  - `makedefs/MakeInc.def:243/245`：`ARCH_FLAGS_ARM64 = -arch arm64e`；
    `:268`：无 device map 时 arm64 默认回退 `ARCH_STRING = arm64e`；
    `:859-864`：arm64e 内核链接专属 flags。
  - `osfmk/arm64/` PAC 全套在源：`asm.h`、`cswitch.s`、`pcb.c`、
    `machine_routines.c`（APIAKey/ptrauth 上下文管理）。
- **硬件**：SM8550 = Cortex-X3/A715/A710/A510（Armv9.0），FEAT_PAuth +
  QARMA 在。**FEAT_FPAC / FEAT_PACM 的存在性未确认**（专项会话查；
  FPAC 只在"实际发生验签失败"时 fault，透传路径无影响）。
- **供体侧**：21H16 = arm64e fileset KC（t8020）；存储值无签名 + 仅带
  key/diversity 选择子；IA key 占 357,971 / DA 2,116；addrDiv=1 占
  335,662（总数 419,323，格式已全量破译，见 PLAN-P3-CI §M）。
- **未做**：组件 GOT 消费点形态抽查（braa/blraa/ldraa 与 metadata
  key/diversity 的对应、有无 PAC 之外的非常规依赖）——见下"REA"。
- **工具**：REA = `/home/ri/work/meizu20/s5-rea`（Ghidra 12.1.4 +
  rea-cli，`source env.sh` 后 `rea` 可用；**用户点名要用，别只手搓
  python**）。

## 专项会话任务书

**目标**：判定"arm64e 原生 UniXNU 内核"作为 m2381 基座的工程可行性，
产出 go/no-go 报告（建议落 `docs/PLAN-arm64e-feasibility.md`）。

**范围**（离线为主，不刷机、不推送、不动 m2381 主线分支/board/kit/CI）：
1. **构建面**：在 UniXNU 树上试编 arm64e 内核（ARCH_STRING 回退已默认
   arm64e，重点=UniXNU 增量的 .s/driver 胶水在 PAC ABI 下是否干净；
   CI macos.yml xcodebuild 链路是否需要改）。
2. **自举面**：PAC key 编程时序（pexpert/osfmk 早期 boot 哪一步、Writable
   keys 假设）、Mu UEFI DarwinBoot → arm64e 内核交接要不要改、SM8550
   上 SCTLR 生效路径。
3. **组件配对面**：arm64e 内核下 21H16 组件 GOT 再签名的机制——读
   xnu 源的 prelink/kext 指针处理路径 + 用 REA 逆 21H16 cache 里
   chained-fixups 消费者（Apple 的签名语义：key 选择、diversity/blend
   公式）。
4. **硬件面**：SM8550 FEAT_FPAC/PACM/EPAC 细节核对（TRM/内核日志）。

**明确不做**：不改主线 CI/board.toml/NativePrelink kit 的现役行为；
不刷机；commit 不 push（红线 8）。

## 主会话待办（本信到达时执行）

1. 把"透传假设"记进 `PLAN-P3-CI.md` §L 门 C 定案：对策排序改为
   **透传 > ①' 运行时再签 > QARMA 预签 > arm64e（专项会话）**，并注明
   本信为依据。
2. 两个廉价验证（主线顺手做或留给专项会话）：
   a. grep UniXNU 内核 + boot shim 的 SCTLR_EL1 设置，确认 EnIA/EnIB/
      EnDA 恒为 0；
   b. REA 抽查 21H16 组件 GOT 消费点形态（braa 立即数 vs metadata
      diversity 的对应关系）。
3. 主线照旧推进：门 A rebinder（注意：侧聊已发现 21H16 元数据
   `_PrelinkExecutableLoadAddr` 解析出非段内怪值、顶层无 LC_SYMTAB——
   fileset KC 的组件发现要走 `LC_FILESET_ENTRY`（cmd 0x2b）file 偏移，
   元数据 load addr 字段语义待重验；kernel 目标映射的符号源也要重找，
   这两项是门 A rebinder 的前置修正）。
4. CI run `40057239716`（原生内核 artifact）继续跟进，runner 容量守护
   `/tmp/m2381-ci-retry.sh` 在跑。
