# ios-m2381-unixnu

魅族 20 Classic（m2381 / SM8550）的 **UniXNU 原生移植**工作区：UniXNU 4K
内核（donor = iPad7,1 / iPadOS 17.7.11 `21H461`，fuxi/sheng 同款）+
独立设备仓 [UniXNU-m2381](https://github.com/RiverKawa271828/UniXNU-m2381)
（2026-10-29 自本仓 subtree split 拆出，本地 `../UniXNU-m2381`）+ UEFI 仓
[uefi-m2381](https://github.com/RiverKawa271828/uefi-m2381)（待建仓首推；
基座 = 上游 uefi-sheng + nura 遗产 overlay）。三仓收拢于工作区根
`/home/ri/work/unixnu-meizu`。

## 进度（2026-10-28，r64 = 缓存全局审计轮已出件 `54b70ad7` 待刷）

**全链真机在跑**：UEFI → XNU → launchd → 200 进程用户态（configd /
sleepd / mDNSResponder / akd … 全生态），**IOMFB 真 swap 上屏**
（1080×2400 @ GOP 0xb8000000），**SpringBoard 已画
boot progress 菊花、装载应用库、深入 wallpaper UI 构造**。五轮连破
（displayIdentity → XB 断言 → UIScreen SIGSEGV → traitCollection nil →
unspecified scale）；r61 换墙 = UIKit 缓存全局垃圾族 SIGSEGV——用户
拍板转体系路线。**r62 调研定谳**：四过墙板（asteroids 桌面 /
oneplus13t 无 GPU 锁屏 / pong 主屏 / negroni SB+touch）无一携带显示
族 fixup，答案模板 = **AppleKeyStore 替身 + sheng-vsync + client 探针
关闭**。r62 已按模板出件、已刷、读回对拍 ✓（sde27）并**判读定谳**
（`0854ea5e`，2026-10-28）：**体系三件实锤生效**（AKS opens 3→18
calls 6→49 / vsync hz=120 start+refresh / 探针客户端消失，C0=
IOMFB_FDR_Loader 实为真实消费者）；applied=67 未跌破 14 = 环境未愈
合；旧墙链全灭（r57–r60 五墙封死确认）；**换墙 = SB sig11 ×520 垃
圾控制流**（unslid PC 0x1822F5FAC = cstring 区野跳，0xf00/0xf1 族升
级）。**r63**（`04eb0533`，已刷已判）：fixup16 全指定 fallback TC 真
执行（applied 67→69）但垃圾族未愈，死点漂回 r61 点——链钉死 = bgTC
缓存槽 0x1f48ce5e0 被写 0xf1，病根在缓存全局槽写路径。**r64 =
缓存全局审计**（`54b70ad7` 已出件待刷）：静态腿 = 21H461 缓存
slide-v2 链格式破译 + UIKitCore __DATA_DIRTY 全链枚举 5410 槽——四
病灶槽文件态全 0，毒在运行时写路径；动态腿 = UIKitCacheProbe 内核探
针（2s 轮询 SpringBoard __DATA_DIRTY 全窗，ext 行 X4-X7 t 序列）；
判读主菜 = 首垃圾 tick/槽址/值形态（工作单 = docs/STATE.md §7）。

- **轮次索引**：[docs/STATE.md §8](docs/STATE.md)（r43→r64，每轮
  payload sha + 定谳一句话）；全量判定档案在
  [docs/archive/STATE-history.md](docs/archive/STATE-history.md)
  §0.24–0.41 与各轮 commit message。
- **移植记录（官方格式）**：设备仓
  [porting.toml](https://github.com/RiverKawa271828/UniXNU-m2381/blob/main/porting.toml)（13 领域，
  官方检查器已过）+
  [records/](https://github.com/RiverKawa271828/UniXNU-m2381/tree/main/records)
  实据档案；格式要求见
  [unixnu.com/zh/porting/records](https://unixnu.com/zh/porting/records)。
- **SM8550 横向坐标**：组织母版 sheng（平板 6S Pro）全实现；社区
  SM8550（fuxi/nuwa/vermeer/nx729j）均无真机运行记录——r55e 轮的
  SpringBoard 起跑 + 真显示 swap 已与官方表第二梯队头部的 negroni
  （SM8450）/ pong（SM8475）同级或更深；GPU 对标 tb128fu（Adreno 610
  已渲染）。
- **取证闭环**：每轮判读不靠屏幕——payload 内置 Linux forensic
  reader（只读 ACM shell），宿主 `acm-reader.py fetchchunk` 拉取
  userdata 尾部 32MiB SHLOG 窗（BootLog 每 2s 一槽），含 applied
  计数、swap timeline、崩溃链 PC/LR/bt（带 dyld slide 可直接对
  donor 符号表符号化）。流程 =
  [docs/FLASHING.md §7](docs/FLASHING.md)。

## 上游关系

上游 = [UniXNU/UniXNU](https://github.com/UniXNU/UniXNU)（remote
`upstream`，fetch-only；现 pin `16e592a`）。本仓 `m2381` 分支 = 上游
main + 工作区提交，跟随上游 = rebase（[docs/UPSTREAM.md](docs/UPSTREAM.md)）；
不向 upstream push。`main` 分支已删，fork 对比永远干净。
姊妹仓（同组织）= [UniXNU-sheng](https://github.com/UniXNU/UniXNU-sheng)
（SM8550 平板母版，机制全实证）。

## 入口

| 文件 | 内容 |
|---|---|
| [AGENTS.md](AGENTS.md) | 项目约束、目标布局、红线、下一步 |
| [docs/STATE.md](docs/STATE.md) | 现势状态、方法资产、轮次索引、工作单 |
| [docs/PLAN-unixnu.md](docs/PLAN-unixnu.md) | 路线计划与阶段状态 |
| [docs/UPSTREAM.md](docs/UPSTREAM.md) | 上游 pin、rebase 同步流程、remote 策略 |
| [docs/FLASHING.md](docs/FLASHING.md) | 刷写、读回、回滚；取证 reader 循环（§7） |
| [docs/HARDWARE-m2381.md](docs/HARDWARE-m2381.md) | 硬件常量与地址 |
| [docs/CI.md](docs/CI.md) | 云端构建说明 |
| [docs/porting-notes.md](docs/porting-notes.md) | 参照设备清单、实测值、取证方法 |
| [UniXNU-m2381](https://github.com/RiverKawa271828/UniXNU-m2381)（本地 `../UniXNU-m2381`） | 设备仓（2026-10-29 拆出独立仓；入口 = 其 README.md） |
| [docs/archive/](docs/archive/) | 史料：STATE 轮次史（§0.0–0.41）、B2/sweet、完工计划 |

旧 ios-sweet / B2 路线已退役，史料只在 `docs/archive/`；回滚件**只**
在 nura 仓 `/home/ri/work/nura-meizu-m2381/artifacts/`（本仓
`artifacts/images/` 已于 2026-10-26 删除）。

## 本地资产（不入 git）

- `artifacts/vendor/`：OEM UEFI DXE blobs（m2381Pkg FV 备选）
- `reference/`：上游/模板克隆（清单与 pin 见 docs/UPSTREAM.md §1）
- `research/`：21H461 IPSW、KDK 15.8.2、ipsw CLI
- `out/m2381/`：现役件（平铺 payload，现役 r62 `0854ea5e` 已刷已验
  已判；kernel 三件、ci-artifacts/userdata-root.img；各轮取证拉窗件
  reader-*-log.bin）
- `temp/`：符号化/取证工具与各轮日志窗（方法 =
  [记忆 dyld-cache-symbolication] 与 docs/STATE.md §4）
