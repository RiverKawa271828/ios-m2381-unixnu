# 移植笔记 — m2381 原生 UniXNU（P0，2026-10-23 起）

参照设备选自 unixnu.com/devices/，选型原则：**同 SoC（SM8550）优先**，其次同
方法（ESP 载荷 / userdata 重建 / 直板机 bring-up）。各仓都在 `reference/`
（只读，不入 git）。

## 1. 参照设备清单

### 第一梯队：SM8550 同 SoC

| 设备 | 本地仓库 | 从它拿什么 |
|---|---|---|
| **sheng**（小米平板6S Pro；官方支持，硬件全通过：显示/触摸/UFS/GPU） | `UniXNU-sheng`、`uefi-sheng`（已有） | board.toml 基线、defconfig、touch driver；SM8550 板级值的最终裁决参照 |
| **fuxi**（小米13，直板） | `unixnu-fuxi`、`uefi-fuxi`（已有） | 设备仓/UEFI 包的派生母版（stage A 最小内核 + stage B 全量 19 archives） |
| **nx729j**（红魔8 Pro，直板） | `UniXNU-nx729j` @ `06d7e5d`（2026-10-23 克隆） | **方法论最全**：board.toml 值标注体系（`[DTB]`/`[TODO]`/`[MEASURE]`/`[DEVICE]`）、`scripts/collect-nx729j.py` 实机测量流、userdata 几何三源对拍、lba_bytes=4096、NativeUfs 进设备仓 + SMMU stream 组回收 + owner bundle 签根策略；附 PORTING/GUIDE/CHECKLIST/STATUS 全套文档 |
| **vermeer**（Redmi K70，直板） | `UniXNU-vermeer`（已有） | sheng→直板的差异基线；mansar1337 版是源码 XNU 构建基线 |
| **nuwa**（小米13 Pro，直板） | `UniXNU-nuwa` @ `4d4571b`（2026-10-23 克隆） | iOS 16 / iPhone10,3（d22ap）身份 + xnu-8796 源码内核 CI 链通（65,348 指针绑定核对）；板级值未建立，只作管线参照 |

### 第二梯队：方法参照（不同 SoC）

| 设备 | 本地仓库 | 从它拿什么 |
|---|---|---|
| **negroni**（一加10 Pro，SM8450） | `UniXNU-negroni` @ `f812002`（2026-10-23 克隆） | **ESP 载荷先例**：64MiB NEGRONIESP 载 kernel+ADT，iPhone X 16.7.16 root 上机到 SpringBoard（CPU 渲染）；README 记 build/flash/boot 命令。UEFI 仓 `Mu-Silicum-negroni` 未克隆，需要时再取 |
| **pong**（Nothing Phone 2，SM8475） | `UniXNU-pong` @ `51911de`、`uefi-pong` @ `cdd581f`（2026-10-23 克隆） | **直板机全流程**：iOS 16.6.1 到主屏、显示/触摸/按键通、原生 UFS+APFS 接通、**主机侧 userdata root 镜像构建器**；`uefi-pong` = 完整 Mu 树（pongPkg，可作 m2381Pkg 的第二直板 UEFI 参照） |

其余（umi / oneplus13t / giulia / hummer / pandora / myron 等）见分支内
`docs/devices/*.md`，需要时再克隆。

## 2. 从 nx729j board.toml 直接学到的（对 m2381 立即可用）

1. **值标注体系**：`[DTB]` = 从 stock ABL 交给 payload 的 DTB 读（m2381 有
   `docs/hardware/stock-live-fdt.dtb` 可填）；`[TODO]`/`[MEASURE]` = 必须实机
   测，由 `scripts/collect-<board>.py` 收集；`[DEVICE]` = 已测（记日期+方法+
   两次一致）。
2. **userdata 几何三源对拍**：① stock fastboot 包（`gpt_main0.bin` 分区表 +
   `rawprogram0.xml` start_sector 互证）；② `fastboot getvar
   partition-size:userdata`；③ 驱动读实机 GPT（native-ufs-probe）。三源一致
   才填 `[DEVICE]`。**注意 UFS 可能是 4KiB 逻辑块**（nx729j 实测 lba_bytes=4096）。
3. **IPSW 段照抄 sheng 系**：iPad7,1 / 17.7.11 / 21H461 / j120ap / chip
   0x8011 / board_id 12——m2381 与它们同目标镜像，这段不需要改。
4. **安全设计沿用**：`[MEASURE]` 类值未填时构建 preflight 直接失败，而不是写
   错盘。
5. **playbook 套用**：nx729j 的 `PORTING-NX729J.md` / `GUIDE-从零开始.md` /
   `CHECKLIST-勾选清单.md` / `STATUS-适配进度.md` 几乎是现成的 m2381 施工
   文档模板，P1 起逐条翻译成 m2381 版。

## 3. 待办（并入 PLAN P0/P1）

- [ ] 通读上游 `docs/porting/` 五篇
- [ ] 通读 nx729j 的 PORTING / GUIDE / CHECKLIST，抽成 m2381 版骨架
- [ ] 读 pong 的 userdata 镜像构建器与 negroni README 的 ESP 流程
- [ ] 收集 m2381 的 stock fastboot 包 / GPT 备份（`[FW]` 类值来源）
- [ ] 实机测量：`fastboot getvar partition-size:userdata` 等（读回纪律照
  `docs/FLASHING.md`）
