# artifacts — 本地资产（不入 git）

**只在本地**；不要把 Apple IPSW/Apple 件提交或外发。状态按 2026-10-23
路线重置（原生 UniXNU 移植）标注。

## images — 回滚件（保留）

| 文件 | 大小 | sha256 | 用途 |
|---|---|---|---|
| `images/mu-r69-97cdea20.img` | 1,110,016 B | `97cdea208214e655e055d815bdcc307d00d8990fd371abeebabf4c31c14326f6` | boot_b → nura Mu |
| `images/esp-recovery-v71.img.gz` | 13,940,788 B | `6fe7811a4c26002858fdfaa7fc3e9d03d72a77b90943e70e3196bfa76eaf649e` | recovery_a → nura ESP 模板 |
| `images/shim-boot-m2381.img` | 5,734,400 B | `339b8480b71c40ec940772623e16b15875256624073e9533a45280ea8bc3b32e` | boot_b → r34 iOS16（2026-10-23 自 `legacy/sweet/images/` 移入） |

- `esp-recovery-v71.img.gz` 是 **nura 的 Linux ESP**；首次刷 recovery_a 前
  先读回确认现状。

## vendor — OEM UEFI DXE（路线 A 仍要用；本地 blob，不外发）

| 文件 | 大小 | sha256 |
|---|---|---|
| `vendor/meizu20-uefi-mods/UFSDxe.efi` | 135,168 B | `e9df76b4abe3dec1b9f434f99a9ca0d12c5868708e742b73a3393ae3b5ebac09` |
| `vendor/meizu20-uefi-mods/UFSDxe-meizu20-157assertNOP.efi` | 135,168 B | `8203aa89b37b384a0c3b57a0bec5442215fc4c2c530f6e5fcb40a867f9bfa853` |
| `vendor/meizu20-uefi-mods/DiskIoDxe.efi` | 45,056 B | — |
| `vendor/meizu20-uefi-mods/PartitionDxe.efi` | 53,248 B | — |
| `vendor/meizu20-uefi-mods/Fat.efi` | 65,536 B | — |
| `vendor/inner-fv-A.bin` | 3,960,840 B | `0db60160dec91a3e522a625d4445905fa7fadacc806e1a875903a289af4ebc4e` |

- 来源：本机 stock UEFI 分区抽取。m2381Pkg 的 FV 要按 Binary 模块加
  `UFSDxe` + `DiskIo`/`Partition`/`Fat`；先试 stock `UFSDxe`，命中 157
  assert 再换 `…assertNOP`。
- nura 的 `UfsDeviceInitDxe` 源码在 `../docs/archive/nura-uefi/`，只在 OEM
  UFSDxe 的 fDeviceInit 失败时参考。

## kernel / adt — 已删除（2026-10-23）

B2 死库存（16K 内核 Mach-O/zip、旧 ADT）已按用户指令删除；sha 记录见 git
历史的本文件旧版。路线 A 的内核/ADT 走上游 unixnu 构建流程。

## 注意

- 读回/刷写前的备份放 `../backups/`（gitignore）。
