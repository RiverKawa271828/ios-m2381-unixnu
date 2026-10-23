# 计划 — m2381 原生 UniXNU 移植（2026-10-23 起）

> 前一版计划（B2/B1/A 三路线评估）已归档：`docs/archive/PLAN-unixnu-b2-era.md`；
> B2 启动链细节：`docs/archive/BOOT-CHAIN-b2.md`。

## 1. 决策

- **废弃**：复用 16K ios-sweet 内核 + iPhone X 16.7.16 root 的 B2/B1 路线。
  旧 sweet 路线实机跑不通，不再背旧内核/旧镜像的包袱。
- **新目标**：m2381 = 上游原生 UniXNU 移植，与 fuxi/sheng 同构——UniXNU 4K
  内核 + iPad7,1 / iPadOS 17.7.11 `21H461` 的 OS 件 + `unixnu` 构建树全流程。
- **方法**：母版 = `reference/unixnu-fuxi`（设备仓）+ `reference/uefi-fuxi`
  （UEFI 包），指南 = 上游 `docs/porting/`；m2381 特有的硬件实证（§4）作为
  差异清单叠加，而不是另起炉灶。

## 2. 路线骨架

1. **设备仓 `unixnu-m2381`**：`board.toml` 描述 SoC、IPSW、userdata 布局、
   kernel defconfig、归档构建器清单（照 fuxi 起步）。
2. **UEFI `m2381Pkg`**：Mu-Silicium 平台包（FD/显示/按键/内存/UFS），产出
   BootShim+FV 的 boot 镜像。
3. **镜像**：boot_b 固件 + payload 载体（ESP 或内嵌，见 §4-1）+ userdata
   重建件；桌面校验后再上机。
4. **上机门**：UEFI 菜单 → UniXNU → XNU banner → launchd → 触摸/显示修好。

## 3. 阶段

| 阶段 | 内容 | 门槛 |
|---|---|---|
| P0 | 通读 `docs/porting/` 五篇 + `docs/devices/fuxi.md`；校准布局草案 | 笔记落 `docs/porting-notes.md`（已建：参照设备清单 + nx729j 方法论） |
| P1 | 设备仓骨架：`board.toml`（ipsw / 实机 userdata LBA / defconfig） | unixnu 构建树能吃进 m2381 并跑通 kernel 步骤 |
| P2 | `m2381Pkg`：FD、SimpleFb、按键、高段 DRAM、UFSDxe 集成 | Linux 本机出 FV；Mu 在 FV 里枚举到 UFS 卷 |
| P3 | 桌面镜像：boot 固件 + payload 件 + userdata 重建件；哈希对拍 | 不刷机完成全部校验 |
| P4 | 上机：只刷 boot_b + recovery_a（userdata 最后） | 四道门逐个过；任一失败即回滚 |
| P5 | 常态：跟随上游 rebase；触摸/显示/GPU 等主题化推进 | upstream behind 归零常态化 |

## 4. 从旧阶段带来的实证约束（B2 遗产，沿用前需在新流程下复验）

1. **ABL**：v4；gunzip 到 `0x80080000`、解压上限 ~83.5MB；**忽略 boot.img
   ramdisk**；只校验 image_size / appended DTB；无 CmdBoot（`fastboot boot`
   死路）。⇒ fuxi 的 192MiB 内嵌 payload 形态在 m2381 大概率不可用，payload
   优先走 recovery_a ESP（前提 = UFSDxe 集成）；读完 porting 文档定案。
2. **内存/窗口**：FD `0xA7100000+0x300000`；内核窗 `0xA8000000+0x10000000`；
   Display `0xB8000000+0x2B00000`；bootloader log `0xA7605000+0x8000`；高段
   DRAM `0x8c0000000` 起 13GB 连续（fuxi UEFI 默认只加 4GB 以下，必须改）。
3. **UFS**：控制器 `0x1d84000`；`uefi-fuxi` FV 不带 UFS DXE，OEM
   `UFSDxe/DiskIo/Partition/Fat` blobs 在 `artifacts/vendor/`（assert 变体
   备用）。
4. **分区**：boot_b 96MiB / recovery_a 100MiB / userdata 12GB；回滚件齐
   （`artifacts/images/`）。
5. **板级**：显示 cmd-mode DSC、Goodix GT9916 触摸、PMK8550 按键——详见
   `AGENTS.md` §5 与 `docs/HARDWARE-m2381.md`。

## 5. 明确不做

- 不再维护/重建 16K 内核；不跑旧 shim；不保留 sweet 补丁树为施工面。
- 不在 userdata 拍板前做任何破坏性操作。
- 不向 upstream push；Apple 件不入 git。
