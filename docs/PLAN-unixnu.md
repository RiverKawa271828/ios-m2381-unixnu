# 计划 — m2381 原生 UniXNU 移植（2026-10-23 起）

> 前一版计划（B2/B1/A 三路线评估）已归档：`docs/archive/PLAN-unixnu-b2-era.md`；
> B2 启动链细节：`docs/archive/BOOT-CHAIN-b2.md`。

## 1. 决策

- **废弃**：复用 16K ios-sweet 内核 + iPhone X 16.7.16 root 的 B2/B1 路线。
  旧 sweet 路线实机跑不通，不再背旧内核/旧镜像的包袱。
- **新目标**：m2381 = 上游原生 UniXNU 移植，与 fuxi/sheng 同构——UniXNU 4K
  内核 + **iPhone10,3 / iOS 16.7.16 `20H392`** 的 OS 件（用户 2026-10-23
  拍板：手机镜像、不用 iPadOS；与 negroni 的 userland 同版本、与 nuwa 同
  身份）+ `unixnu` 构建树全流程。
  **【已被 2026-10-24 换配取代】** donor = **iPad7,1 / iPadOS 17.7.11
  `21H461`**（同代配对修正，fuxi/nx729j 同款；见 AGENTS.md 顶部说明），
  20H392 资产保留回溯。
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

> **状态（2026-10-27）**：P0–P4 全部完成（launchd r25 起、SpringBoard 起跑
> 至 r50），现处 P5 常态轮。当前主墙 = SpringBoard displayConfiguration
> 断言（`docs/STATE.md` §1/§7）。

## 4. 从旧阶段带来的实证约束（B2 遗产，沿用前需在新流程下复验）

1. **ABL**：v4；gunzip 到 `0x80080000`、解压上限 ~83.5MB；**忽略 boot.img
   ramdisk**；只校验 image_size / appended DTB；无 CmdBoot（`fastboot boot`
   死路）。⇒ fuxi 的 192MiB 内嵌 payload 形态在 m2381 不可用，payload 走
   recovery_a ESP（**已实证：M2381ESP，r26 起每轮现役**）。
2. **内存/窗口**：FD `0xA7100000+0x300000`；内核窗 `0xA8000000+0x10000000`；
   Display `0xB8000000+0x2B00000`；bootloader log `0xA7605000+0x8000`；高段
   DRAM `0x8c0000000` 起 13GB 连续（fuxi UEFI 默认只加 4GB 以下，必须改）。
3. **UFS**：控制器 `0x1d84000`；`uefi-fuxi` FV 不带 UFS DXE，OEM
   `UFSDxe/DiskIo/Partition/Fat` blobs 在 `artifacts/vendor/`（assert 变体
   备用）。
4. **分区**：boot_b 96MiB / recovery_a 100MiB / userdata 12GB；回滚件已
   移出本仓（2026-10-26 删 `artifacts/images/`），回滚链只在 nura 仓
   artifacts（红线 6）。
5. **板级**：显示 cmd-mode DSC、Goodix GT9916 触摸、PMK8550 按键——详见
   `AGENTS.md` §5 与 `docs/HARDWARE-m2381.md`。

## 5. 明确不做

- 不再维护/重建 16K 内核；不跑旧 shim；不保留 sweet 补丁树为施工面。
- 不向 upstream push；Apple 件不入 git。

## 6. 下阶段执行计划（2026-10-23 定；设备在线、数据豁免已给）

前置已就绪：设备 nura OS 在线（devsh USB-net，root 可用）、分区几何与四槽
sha 已实测（`docs/STATE.md` §2）、参照设备仓齐（`docs/porting-notes.md`）、
iPadOS 21H461 IPSW 下载中。

| # | 任务 | 产出/门槛 |
|---|---|---|
| 1 | ~~通读 `docs/porting/` 五篇 + fuxi.md + nx729j playbook~~ **完成（2026-10-23）** | `docs/m2381-bringup.md`（施工骨架：值源/构建序列/阶段表/风险） |
| 2 | 收集 m2381 stock fastboot 包（`gpt_main0.bin`/`rawprogram0.xml`） | userdata 几何 `[FW]` 侧佐证（现在只有 sysfs/GPT 源） |
| 3 | 派生 `devices/m2381/unixnu-m2381/board.toml`（ipsw/userdata/defconfig/归档构建器） | unixnu 构建树能吃进 m2381 |
| 4 | 确认构建主机方案：Linux x86_64 交叉 or 上游 macOS CI or 容器 | kernel 步骤在本仓跑通（P1 门槛） |
| 5 | 派生 `devices/m2381/uefi/m2381Pkg` + UFSDxe 集成 | Linux 本机出 FV（P2 门槛） |
| 6 | 桌面镜像（boot 固件 + payload + userdata 件）+ 哈希对拍 | 全部校验通过，不刷机（P3） |
| 7 | 上机：先 recovery_a 后 boot_b；四道门逐个过（P4） | UEFI 菜单 → UniXNU → XNU banner → launchd |

- 设备当前是 nura 现役件在跑，刷写顺序与回滚纪律照 `docs/FLASHING.md`；
  第一次上机前先在 fastboot 窗口补 `getvar current-slot`/
  `partition-size:userdata` 复核。
- 任务 1（读文档）与任务 2（找 stock 包）可并行，互不依赖。
