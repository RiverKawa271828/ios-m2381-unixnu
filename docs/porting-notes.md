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
3. ~~IPSW 段照抄 sheng 系~~ **已改**：用户拍板用手机镜像（不用 iPadOS）——
   m2381 的 `[ipsw]` = iPhone10,3 / 16.7.16 / 20H392 / d22ap，见 §4。
   sheng 系 fetch 组件清单（KernelCache/DeviceTree/StaticTrustCache/
   Cryptex…）与 hash 对拍方式仍照抄结构。
4. **安全设计沿用**：`[MEASURE]` 类值未填时构建 preflight 直接失败，而不是写
   错盘。
5. **playbook 套用**：nx729j 的 `PORTING-NX729J.md` / `GUIDE-从零开始.md` /
   `CHECKLIST-勾选清单.md` / `STATUS-适配进度.md` 几乎是现成的 m2381 施工
   文档模板，P1 起逐条翻译成 m2381 版。

## 3. m2381 实测值（2026-10-23，devsh 只读取证，nura OS 在跑）

可直接填 board.toml 的 `[DEVICE]` 值（详细表见 `docs/STATE.md` §2）：

```toml
[userdata]
first_lba = 2330472        # sysfs start 18,643,776 × 512B ÷ 4096
last_lba  = 123871226      # first + 121,540,755 − 1（972,326,040 × 512B ÷ 4096）
lba_bytes = 4096           # sda logical_block_size = 4096（与 nx729j 同款陷阱）
```

- 分区：boot_b = 96MiB（100,663,296B）@ sde43；recovery_a = 100MiB
  （104,857,600B）@ sde27；userdata ≈463.7GiB @ sda22（= nura root）。
- 四槽读回 sha 已记录（`docs/FLASHING.md` §6）；**boot_b 现役是 nura 件**
  （`af574775…`），旧 "r34 shim `339b8480`" 记录作废。
- 待补：`fastboot getvar partition-size:userdata` + `current-slot` 复核
  （刷机窗口做）；m2381 stock fastboot 包（`gpt_main0.bin`/`rawprogram0.xml`）
  作 `[FW]` 侧佐证。

## 4. 待办（并入 PLAN P0/P1）

- [ ] 通读上游 `docs/porting/` 五篇
- [ ] 通读 nx729j 的 PORTING / GUIDE / CHECKLIST，抽成 m2381 版骨架
- [ ] 读 pong 的 userdata 镜像构建器与 negroni README 的 ESP 流程
- [ ] 收集 m2381 的 stock fastboot 包 / GPT 备份（`[FW]` 类值来源）
- [ ] 实机测量：`fastboot getvar partition-size:userdata` 等（刷机窗口做，
  读回纪律照 `docs/FLASHING.md`）
- [ ] 下载 20H392 后 sha 对拍 `ece1194f…`，放 `research/ipsw/`
- [ ] P1 验证上游管线对 20H392 kernelcache 的 donor/bindings 支持
- [ ] 确认 m2381 调试 UART 引出与 USB CDC 可用性
