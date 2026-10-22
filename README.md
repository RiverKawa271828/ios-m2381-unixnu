# ios-m2381-unixnu

魅族 20 Classic（m2381 / SM8550）的 iOS 启动工作区，改用 **UniXNU / DarwinBoot**
启动链，保留现有 iPhone X iOS 16.7.16 root 与 m2381 16K ios-sweet 内核。

- 当前方案：**B2** — boot_b 只放 Mu-Silicium UEFI 固件；recovery_a 做成 100MiB
  FAT32 ESP，放 UniXNU.efi、`unixnu.toml`、`kernel.macho`、ADT；userdata 不动。
- 旧 ios-sweet 路线已废弃，源码/笔记只作史料放在 `legacy/sweet/` 和
  `docs/archive/`。
- 还没有上机；下一步是桌面 spike（UEFI 包 + ESP + boot.img）。

入口：

| 文件 | 内容 |
|---|---|
| [AGENTS.md](AGENTS.md) | 项目约束、B2 布局、红线、下一步 |
| [docs/PLAN-unixnu.md](docs/PLAN-unixnu.md) | 完整评估、B2/B1/A 路线、证据 |
| [docs/STATE.md](docs/STATE.md) | 当前状态、资产哈希、开放问题 |
| [docs/BOOT-CHAIN.md](docs/BOOT-CHAIN.md) | 启动链、内存窗、ESP、构建步骤 |
| [docs/FLASHING.md](docs/FLASHING.md) | 刷写、读回、回滚 |
| [docs/HARDWARE-m2381.md](docs/HARDWARE-m2381.md) | 硬件常量与地址 |
| [docs/MIGRATION.md](docs/MIGRATION.md) | 从旧工作区搬了什么、没搬什么 |

本地资产（不入 git）：

- `artifacts/kernel/kernel.development.m2381`（17,208,544B，sha256
  `012f596f…`）
- `artifacts/images/mu-r69-97cdea20.img`、`esp-recovery-v71.img.gz`
- `reference/`：UniXNU、uefi-fuxi、UniXNU-guacamole 等只读克隆
- `research/ipsw-x/d22ap.adt`：`tools/mkadt.py` 的输入
