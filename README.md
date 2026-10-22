# ios-m2381-unixnu

魅族 20 Classic（m2381 / SM8550）的 iOS 启动工作区，改用 **UniXNU / DarwinBoot**
启动链，保留现有 iPhone X iOS 16.7.16 root 与 m2381 16K ios-sweet 内核。

上游 = [UniXNU/UniXNU](https://github.com/UniXNU/UniXNU)（remote `upstream`，pin
`e95b37d`）；设备模板 = `hanzi123-bilp/{uefi-fuxi,unixnu-fuxi}`。本仓是 m2381
下游移植，不向 upstream push；详见 [docs/UPSTREAM.md](docs/UPSTREAM.md)。

- 当前方案：**B2** — boot_b 只放 Mu-Silicium UEFI 固件；recovery_a 做成 100MiB
  FAT32 ESP，放 UniXNU.efi、`unixnu.toml`、`kernel.macho`、ADT；userdata 不动。
- 旧 ios-sweet 路线已废弃，源码/笔记只作史料放在 `legacy/sweet/` 和
  `docs/archive/`。
- GitHub：`origin` = `RiverKawa271828/ios-m2381-unixnu`；分支 `m2381` 是工作区，
  `main` 保留为 UniXNU 上游镜像。建议在 GitHub Settings → Branches 把默认分支
  设为 `m2381`。
- 还没有上机；下一步是桌面 spike（UEFI 包 + ESP + boot.img）。
- 云端构建：`.github/workflows/xnu-16k.yml` 保留旧 16K 内核的 macOS 流水线；
  B2 用现成 kernel，不依赖它。

入口：

| 文件 | 内容 |
|---|---|
| [AGENTS.md](AGENTS.md) | 项目约束、B2 布局、红线、下一步 |
| [docs/PLAN-unixnu.md](docs/PLAN-unixnu.md) | 完整评估、B2/B1/A 路线、证据 |
| [docs/STATE.md](docs/STATE.md) | 当前状态、资产哈希、开放问题 |
| [docs/BOOT-CHAIN.md](docs/BOOT-CHAIN.md) | 启动链、内存窗、ESP、构建步骤 |
| [docs/UPSTREAM.md](docs/UPSTREAM.md) | 上游 pin、更新流程、remote 策略 |
| [docs/FLASHING.md](docs/FLASHING.md) | 刷写、读回、回滚 |
| [docs/HARDWARE-m2381.md](docs/HARDWARE-m2381.md) | 硬件常量与地址 |
| [docs/MIGRATION.md](docs/MIGRATION.md) | 从旧工作区搬了什么、没搬什么 |
| [docs/CI.md](docs/CI.md) | 云端构建：`xnu-16k` workflow、产物、UEFI CI 待办 |

本地资产（不入 git）：

- `artifacts/kernel/kernel.development.m2381`（17,208,544B，sha256
  `012f596f…`）
- `artifacts/images/mu-r69-97cdea20.img`、`esp-recovery-v71.img.gz`
- `reference/`：UniXNU、uefi-fuxi、UniXNU-guacamole 等只读克隆
- `research/ipsw/`：iPhone X 16.7.16 IPSW + 抽取 DMG（sha `ece1194f…`）
- `research/legacy-sweet/`：16K 内核重建套件（xnu-8796/buildtree）
- `research/ipsw-x/d22ap.adt`：`tools/mkadt.py` 的输入
