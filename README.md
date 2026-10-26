# ios-m2381-unixnu

魅族 20 Classic（m2381 / SM8550）的 **UniXNU 原生移植**工作区：UniXNU 4K
内核（donor = iPad7,1 / iPadOS 17.7.11 `21H461`，fuxi/nx729j 同款）+
`unixnu-m2381` 设备仓 + `m2381Pkg`（Mu-Silicium UEFI）。

上游 = [UniXNU/UniXNU](https://github.com/UniXNU/UniXNU)（remote `upstream`，
fetch-only；现 pin `16e592a`）。本仓 `m2381` 分支 = 上游 main + 工作区提交，
跟随上游 = rebase（[docs/UPSTREAM.md](docs/UPSTREAM.md)）；不向 upstream push。

- **现状（2026-10-27，r50 后）**：iOS 链全链上机（UEFI → XNU → launchd →
  SpringBoard 起跑），当前墙 = SpringBoard displayConfiguration 断言；
  下轮 = 上游同步 + 解法调研，见 [docs/STATE.md](docs/STATE.md) §7。
- 旧 ios-sweet / B2 路线已退役，史料在 `docs/archive/`（含 STATE 轮次史）；
  回滚件**只**在 nura 仓 `/home/ri/work/nura-meizu-m2381/artifacts/`
  （本仓 `artifacts/images/` 已于 2026-10-26 删除）。
- GitHub：`origin` = `RiverKawa271828/ios-m2381-unixnu`，默认分支 = `m2381`
  （`main` 已删，fork 对比 = m2381 vs 上游 main，永远干净）。
- 云端构建：上游 `macos.yml`（workflow_dispatch）= 内核/userdata/payload
  正式车道，设备仓走 orphan 分支 `device-m2381`，见 [docs/CI.md](docs/CI.md)；
  日常出件在本机 Linux。

入口：

| 文件 | 内容 |
|---|---|
| [AGENTS.md](AGENTS.md) | 项目约束、目标布局、红线、下一步 |
| [docs/STATE.md](docs/STATE.md) | 现势状态、方法资产、轮次索引、工作单 |
| [docs/PLAN-unixnu.md](docs/PLAN-unixnu.md) | 路线计划与阶段状态 |
| [docs/UPSTREAM.md](docs/UPSTREAM.md) | 上游 pin、rebase 同步流程、remote 策略 |
| [docs/FLASHING.md](docs/FLASHING.md) | 刷写、读回、回滚（三件卡规矩） |
| [docs/HARDWARE-m2381.md](docs/HARDWARE-m2381.md) | 硬件常量与地址 |
| [docs/CI.md](docs/CI.md) | 云端构建说明 |
| [docs/m2381-bringup.md](docs/m2381-bringup.md) | P0 施工骨架（历史；donor 值以 board.toml 为准） |
| [docs/archive/](docs/archive/) | 史料：STATE 轮次史（§0.0–0.34）、B2/sweet、完工计划、迁移记录 |

本地资产（不入 git）：

- `artifacts/vendor/`：OEM UEFI DXE blobs（m2381Pkg FV 备选）
- `reference/`：上游/模板克隆（清单与 pin 见 docs/UPSTREAM.md §1）
- `research/`：21H461 IPSW、KDK 15.8.2、ipsw CLI
- `out/m2381/`：现役件（kernel 三件、平铺 payload r49/r50、821ab7c0 回滚件）
- `temp/`：符号化/取证工具链与各轮日志窗（方法论见 docs/STATE.md §4）
