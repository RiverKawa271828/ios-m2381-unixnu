# STATE — 2026-10-23（路线重置）

## 1. 结论/决策

- **B2/B1 废弃（用户 2026-10-23 拍板）**：旧 ios-sweet 16K 内核 + iPhone X
  16.7.16 root 复用路线实机跑不通，不再复用旧内核/旧镜像；旧阶段只带经验与
  方法。B2 评估 → `docs/archive/PLAN-unixnu-b2-era.md`；B2 启动链 →
  `docs/archive/BOOT-CHAIN-b2.md`；`legacy/` 已从 git 移除（内容在 git
  历史），r34 回滚 shim 移到 `artifacts/images/shim-boot-m2381.img`。
- **现方向 = 原生 UniXNU 设备移植**（对齐上游 fuxi/sheng 模式）：UniXNU 4K
  内核 + iPad7,1 / iPadOS 17.7.11 `21H461` IPSW root + `unixnu-m2381` 设备仓
  + `m2381Pkg`（Mu-Silicium UEFI）。母版 = `reference/unixnu-fuxi` +
  `reference/uefi-fuxi`（fuxi = 小米13，同 SM8550）；指南 = 上游
  `docs/porting/`。
- 分支已对齐上游：`m2381` = `upstream/main`（`b7b6a6a`）+ 工作区提交，同步 =
  `git rebase upstream/main`（见 `docs/UPSTREAM.md` §2）。

## 2. 设备现状（未动）

| 分区 | 现状 | 证据/备注 |
|---|---|---|
| boot_b | r34 shim `shim-boot-m2381.img`，sha256 `339b8480b71c40ec940772623e16b15875256624073e9533a45280ea8bc3b32e` | 旧仓 r34 记录；上机前仍应读回确认 |
| recovery_a | **未在本轮读取**；nura ESP v71 只是模板，不等于现状 | 首次刷前必须读回/确认 |
| userdata | r34 iOS16 root `iOSRoot-s-r34.simg`，sha256 `7aaab96f…`（旧记录） | 12GB；本地**没有**完整镜像；路线 A 会重建它，抹前必须拍板 |

- 回滚件（本地，`artifacts/images/`）：`mu-r69-97cdea20.img`（→nura boot_b）、
  `esp-recovery-v71.img.gz`（→nura recovery_a 模板）、
  `shim-boot-m2381.img`（→r34 iOS boot_b）。
- 设备同一时刻只允许一个 fastboot 通道；本仓与 nura 侧刷写互斥。

## 3. 资产（本地，不入 git）

| 路径 | 状态 |
|---|---|
| `artifacts/images/{mu-r69-97cdea20.img, esp-recovery-v71.img.gz, shim-boot-m2381.img}` | **回滚件，保留** |
| `artifacts/vendor/meizu20-uefi-mods/`（UFSDxe 等 5 件）、`inner-fv-A.bin` | **路线 A 仍要用**（m2381Pkg FV 的 UFS/文件系统支持） |
| `artifacts/kernel/{kernel.development.m2381, xnu-kernel.zip}`、`artifacts/adt/*.adt` | **死库存**（B2 遗留的 16K 内核/ADT，路线 A 不用；可删） |
| `research/ipsw/`（iPhone X 16.7.16 IPSW+DMG ~13GB）、`research/legacy-sweet/`（16K 重建套件） | **已无用**（路线 A 用 iPad7,1 21H461，上游工具可远程 range 读，不必本地全量）；可删 |
| `reference/` 全部克隆 | 参照保留；`reference/unixnu` 降级为干净构建树 |

## 4. 当前没做/不能声称的

- 没有设备仓、没有 m2381Pkg、没有构建任何镜像、没有刷机。
- 没有通读上游 `docs/porting/` 全文；AGENTS §2 布局仍是草案。
- ESP 载荷在 Mu 下的可见性（OEM UFSDxe 集成进 m2381Pkg FV）未验证——B2 的
  第一道门，在路线 A 下依旧成立。
- 没有验证 m2381 实机 userdata 的 GPT LUN/LBA 读取方式。
- 上游 3 个 workflow（docs/macos/terminal）是否已在 GitHub UI 禁用未确认。

## 5. 下一步

1. 通读上游 `docs/porting/` 五篇 + `docs/devices/fuxi.md`，校准目标布局。
2. 派生 `devices/m2381/unixnu-m2381`：`board.toml` 优先。
3. 派生 `devices/m2381/uefi/m2381Pkg`：FD/显示/按键/高段 DRAM + UFSDxe。
4. 桌面构建 spike + 哈希校验，不刷机。
5. 上机门：UEFI 菜单 → UniXNU → XNU banner → launchd；userdata 重建前拍板。

## 6. 开放问题

- **payload 载体**：ESP（recovery_a）vs 压进 ABL 83.5MB 限内；上游原生流程
  的 payload 交付方式读完 porting 文档后对齐。
- **UFSDxe**：OEM blob 在 m2381Pkg FV 的集成方式与 fDeviceInit 行为；
  assert 变体何时换上。
- **userdata 重建** = 抹盘；是否先 12GB 拉回镜像；r34 root 的善后。
- **实机 GPT 读取**：m2381 userdata 所在 LUN 与 first/last_lba 的获取路径
  （adb root / fastboot）。
- **kernel defconfig 板级差异**：与 fuxi 的 PMIC/按键/显示/触摸差异清单。
- **显示**：cmd-mode DSC 面板在 UEFI 的最小点灯（SimpleFbDxe 用 ABL 帧缓冲）。
- **按键**：PMK8550 SPMI SID/键码与 fuxi 是否一致。
