# ios-m2381-unixnu

魅族 20 Classic（m2381 / SM8550）的 iOS 启动工作区，改用 **UniXNU / DarwinBoot**
启动链，保留现有 iPhone X iOS 16.7.16 root 与 m2381 16K ios-sweet 内核。

上游 = [UniXNU/UniXNU](https://github.com/UniXNU/UniXNU)（remote `upstream`，
pin `b7b6a6a`）；本仓 `m2381` 分支 = 上游 main + 工作区提交，跟随上游 =
`git rebase upstream/main`。设备模板 = `hanzi123-bilp/{uefi-fuxi,unixnu-fuxi}`。
不向 upstream push；详见 [docs/UPSTREAM.md](docs/UPSTREAM.md)。

- 当前方案：**原生 UniXNU 移植**（2026-10-23 起）——对齐上游 fuxi/sheng 模式，
  `unixnu-m2381` 设备仓（UniXNU 4K 内核 + iPadOS 17.7.11 OS 件）+ `m2381Pkg`
  （Mu-Silicium UEFI）。旧 B2 路线（复用 16K 内核 + 现有 root）已废弃归档。
- 旧 ios-sweet 路线已退役，笔记/评估只作史料放在 `docs/archive/`；回滚件在
  本地 `artifacts/images/`。
- GitHub：`origin` = `RiverKawa271828/ios-m2381-unixnu`；分支 `m2381` 是工作区，
  `main` 保留为 UniXNU 上游旧镜像。建议在 GitHub Settings → Branches 把默认分支
  设为 `m2381`。
- 还没有上机；下一步是读上游 `docs/porting/`，然后派生设备仓与 UEFI 包。
- 云端构建：本仓当前没有自己的 workflow（旧 16K 内核 CI 已随 B2 删除）；
  上游带来的 3 个 workflow 建议在 Actions 页禁用，见 [docs/CI.md](docs/CI.md)。

入口：

| 文件 | 内容 |
|---|---|
| [AGENTS.md](AGENTS.md) | 项目约束、目标布局、红线、下一步 |
| [docs/PLAN-unixnu.md](docs/PLAN-unixnu.md) | 当前路线（原生移植）计划与阶段 |
| [docs/STATE.md](docs/STATE.md) | 当前状态、资产清单、开放问题 |
| [docs/UPSTREAM.md](docs/UPSTREAM.md) | 上游 pin、rebase 同步流程、remote 策略 |
| [docs/FLASHING.md](docs/FLASHING.md) | 刷写、读回、回滚 |
| [docs/HARDWARE-m2381.md](docs/HARDWARE-m2381.md) | 硬件常量与地址 |
| [docs/CI.md](docs/CI.md) | 云端构建说明 |
| [docs/MIGRATION.md](docs/MIGRATION.md) | 从旧工作区搬了什么、没搬什么 |
| [docs/archive/](docs/archive/) | 旧 sweet 笔记、B2 评估与启动链（史料） |

本地资产（不入 git）：

- `artifacts/images/`：回滚件三件（nura Mu / nura ESP 模板 / r34 shim）
- `artifacts/vendor/`：OEM UEFI DXE blobs（m2381Pkg FV 要用）
- `artifacts/{kernel,adt}/`：B2 遗留的 16K 内核与旧 ADT（死库存，可删）
- `reference/`：UniXNU、uefi-fuxi、unixnu-fuxi 等只读克隆
- `research/`：旧 iPhone X IPSW 与 16K 重建套件（已无用，可删）
