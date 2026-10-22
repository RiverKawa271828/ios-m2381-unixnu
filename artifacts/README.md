# artifacts — 本地资产（不入 git）

这些文件是 B2 的输入/回滚件，**只在本地**；不要把 Apple IPSW/Apple 件提交或外发。

## kernel

| 文件 | 大小 | sha256 |
|---|---|---|
| `kernel/kernel.development.m2381` | 17,208,544 B | `012f596fa93fd74b251198a5622df92b2d7558e6250df1459e4a4ebdc055dacb` |
| `kernel/xnu-kernel.zip` | 59,849,381 B | `a71a44f29b5628e0d08dd05d8e632fdcf7499b319b83457481693c7b387b67dc` |

- `kernel.development.m2381` 是 r34 的 16K ios-sweet 内核（Mach-O），B2 的
  ESP payload 就用它。
- zip 里的 `SHA256SUMS` 与上面一致；zip 还带 dSYM/link 目录，B2 不用。

## ADT

| 文件 | 大小 | sha256 | 用途 |
|---|---|---|---|
| `adt/m2381-sweet-13g.adt` | 12,624 B | `c85b01f025b6b16654c5d75bd1f81f32890606c51b527d8f4b59f7be125f6fe0` | r34 ADT，只作对照 |
| `adt/m2381-old.adt` | 12,620 B | `7c4a6e7c2a512c118a96fcd07f5a5ee31a62e15e3b94f8657b470f3bdd4960a4` | 旧样本 |

B2 的 ADT 要用 `tools/mkadt.py --m2381` 重新生成（加
`darwinboot-memory-size`、3–4GiB 窗口、`dram-base/size` 对齐）。

## images

| 文件 | 大小 | sha256 | 用途 |
|---|---|---|---|
| `images/mu-r69-97cdea20.img` | 1,110,016 B | `97cdea208214e655e055d815bdcc307d00d8990fd371abeebabf4c31c14326f6` | boot_b 回滚到 nura |
| `images/esp-recovery-v71.img.gz` | 13,940,788 B | `6fe7811a4c26002858fdfaa7fc3e9d03d72a77b90943e70e3196bfa76eaf649e` | recovery_a 回滚模板（nura ESP） |
| `../legacy/sweet/images/shim-boot-m2381.img` | 5,734,400 B | `339b8480b71c40ec940772623e16b15875256624073e9533a45280ea8bc3b32e` | boot_b 回滚到 r34 |

## vendor（OEM UEFI DXE，本地 blob，不入 git、不外发）

| 文件 | 大小 | sha256 |
|---|---|---|
| `vendor/meizu20-uefi-mods/UFSDxe.efi` | 135,168 B | `e9df76b4abe3dec1b9f434f99a9ca0d12c5868708e742b73a3393ae3b5ebac09` |
| `vendor/meizu20-uefi-mods/UFSDxe-meizu20-157assertNOP.efi` | 135,168 B | `8203aa89b37b384a0c3b57a0bec5442215fc4c2c530f6e5fcb40a867f9bfa853` |
| `vendor/meizu20-uefi-mods/DiskIoDxe.efi` | 45,056 B | — |
| `vendor/meizu20-uefi-mods/PartitionDxe.efi` | 53,248 B | — |
| `vendor/meizu20-uefi-mods/Fat.efi` | 65,536 B | — |
| `vendor/inner-fv-A.bin` | 3,960,840 B | `0db60160dec91a3e522a625d4445905fa7fadacc806e1a875903a289af4ebc4e` |

- 来源：本机 stock UEFI 分区抽取（nura `meizu20-uefi-mods` 与
  `uefi_a-extract`）。`UFSDxe` 是 **vendor 二进制**，不要提交/外发。
- Mu 要看到 `recovery_a`，FV 必须带 `UFSDxe` + `DiskIo`/`Partition`/`Fat`；
  先试 stock `UFSDxe`，命中 157 assert 再换 `…assertNOP`。
- nura 的 `UfsDeviceInitDxe` 源码在 `../docs/archive/nura-uefi/`，只在 OEM
  UFSDxe 的 fDeviceInit 失败时参考。

## 注意

- `esp-recovery-v71.img.gz` 是 **nura 的 Linux ESP**，不是 B2 的 iOS ESP；
  B2 要新建 100MiB FAT32 `M2381ESP`。
- 没有 userdata 的完整本地镜像；r34 root 只在设备上。
- 读回/刷写前的备份放 `../backups/`（gitignore）。
