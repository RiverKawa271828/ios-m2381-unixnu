# STATE — 2026-10-22（新工作区落地）

## 1. 结论/决策

- **旧 ios-sweet 路线废弃**（用户 2026-10-22 拍板）。旧工作区
  `/home/ri/work/ios-meizu-m2381` 保留但标记 `DEPRECATED.md`。
- **当前方案 = B2**：UniXNU/DarwinBoot 启动链 + 现有 iPhone X 16.7.16 root +
  m2381 16K ios-sweet 内核；内核/ADT 放 recovery_a 的 100MiB FAT32 ESP，
  boot_b 只放约 3MB 的 BootShim+UEFI 固件。
- A（UniXNU 4K 内核 + iPadOS 17.7.11 + 新 userdata）只作后续升级；本阶段
  不换 iOS 镜像、不动 userdata。
- 不做多系统管理；nura 与 iOS 各有各的镜像，需要时整槽重刷。

## 2. 设备现状（本轮**未**动设备）

| 分区 | 现状 | 证据/备注 |
|---|---|---|
| boot_b | r34 shim `shim-boot-m2381.img`，sha256 `339b8480b71c40ec940772623e16b15875256624073e9533a45280ea8bc3b32e` | 旧仓 r34 记录；上机前仍应读回确认 |
| recovery_a | **未在本轮读取**；nura ESP v71 只是模板，不等于现状 | 首次刷前必须读回/确认 |
| userdata | r34 iOS16 root `iOSRoot-s-r34.simg`，sha256 `7aaab96f…`（旧记录） | 12GB；本地**没有**完整镜像；抹前必须用户拍板/先拉回 |

- 回滚 boot_b：`artifacts/images/mu-r69-97cdea20.img`（nura 的 Mu 启动）。
- 回滚 recovery_a：`artifacts/images/esp-recovery-v71.img.gz`（nura ESP 模板，
  不是 iOS ESP；刷之前先解压/确认）。
- 回滚到 r34 iOS：`legacy/sweet/images/shim-boot-m2381.img`（boot_b）+
  现有 userdata。
- 设备在同一时刻只允许一个 fastboot 通道；本仓与 nura 侧刷写互斥。

## 3. 资产（本地，不入 git）

| 路径 | 大小 | sha256 |
|---|---|---|
| `artifacts/kernel/kernel.development.m2381` | 17,208,544 B | `012f596fa93fd74b251198a5622df92b2d7558e6250df1459e4a4ebdc055dacb` |
| `artifacts/kernel/xnu-kernel.zip` | 59,849,381 B | `a71a44f29b5628e0d08dd05d8e632fdcf7499b319b83457481693c7b387b67dc` |
| `artifacts/adt/m2381-sweet-13g.adt` | 12,624 B | `c85b01f025b6b16654c5d75bd1f81f32890606c51b527d8f4b59f7be125f6fe0` |
| `artifacts/adt/m2381-old.adt` | 12,620 B | `7c4a6e7c2a512c118a96fcd07f5a5ee31a62e15e3b94f8657b470f3bdd4960a4` |
| `artifacts/images/mu-r69-97cdea20.img` | 1,110,016 B | `97cdea208214e655e055d815bdcc307d00d8990fd371abeebabf4c31c14326f6` |
| `artifacts/images/esp-recovery-v71.img.gz` | 13,940,788 B | `6fe7811a4c26002858fdfaa7fc3e9d03d72a77b90943e70e3196bfa76eaf649e` |
| `artifacts/vendor/meizu20-uefi-mods/` | 6.5 MB | OEM UEFI DXE 集合（vendor blob，不入 git） |
| `artifacts/vendor/meizu20-uefi-mods/UFSDxe.efi` | 135,168 B | `e9df76b4abe3dec1b9f434f99a9ca0d12c5868708e742b73a3393ae3b5ebac09` |
| `artifacts/vendor/meizu20-uefi-mods/UFSDxe-meizu20-157assertNOP.efi` | 135,168 B | `8203aa89b37b384a0c3b57a0bec5442215fc4c2c530f6e5fcb40a867f9bfa853` |
| `research/ipsw/iPhone10,3,iPhone10,6_16.7.16_20H392_Restore.ipsw` | 6,171,035,648 B | `ece1194f7e233e9109c5790a8d23a4564a10a254e15ec5b09978a343265568fb` |
| `research/ipsw/*.dmg` + `BuildManifest.plist` | 13 GB | IPSW 抽取件（Apple 本地） |
| `research/legacy-sweet/` | 296 MB | 16K 内核重建套件（xnu-8796/buildtree/patches 脚本） |
| `research/bin/ipsw` | 104 MB | ipsw CLI（DSC/IM4P 抽取） |
| `legacy/sweet/images/shim-boot-m2381.img` | 5,734,400 B | `339b8480…` |
| `legacy/sweet/images/Image.gz` | 5,723,780 B | — |
| `legacy/sweet/images/payload-r34.bin` | 16,599,584 B | — |

- `artifacts/kernel/kernel.development.m2381` 的 `SHA256SUMS` 与 zip 内一致。
- `research/ipsw-x/d22ap.adt`（182K）是 `tools/mkadt.py` 的输入，保持该相对路径。
- `research/` 全部在 `.gitignore`；IPSW/DMG 是 Apple 版权件，只本地使用、不外发。

## 4. 当前没做/不能声称的

- 没有构建 `m2381Pkg`、没有构建 `UniXNU.efi`、没有做 ESP、没有做 boot.img、
  没有刷机。
- 没有验证 `UniXNU.efi` 的最终大小；nura ESP 里的 `BOOTAA64.EFI`
  13,885,952B 是 **nura 的 Linux kernel**，不是 UniXNU，不能拿来当 UniXNU.efi。
- 没有验证 100MiB ESP 在 Mu 下的可读性（FAT32 512B 扇区/2 扇区簇是 nura 实测
  形态，理论上 Mu FatDxe 可读）。
- **没有验证 Mu 能看到 recovery_a**：`uefi-fuxi`/`uefi-sheng` 的 FV 不带 UFS
  DXE；OEM `UFSDxe.efi` + `DiskIoDxe`/`PartitionDxe`/`Fat.efi` 已放在
  `artifacts/vendor/meizu20-uefi-mods/`，但还没集成进 m2381Pkg FV。这是 B2 的
  第一道门；过不了就转 B1（payload 进 boot_b kernel，不需要 UFS 读盘）。
- 没有验证高段 DRAM 在 m2381 UEFI GCD 中的可分配性；`FuxiBootMemoryDxe`
  默认只加 4GB 以下，必须改。
- 没有验证 ABL 的 v4 路径对“kernel 里只有 BootShim+FV”的接受度；按 nura RE，
  9 项检查里只校验 image_size/appended DTB，风险主要在 FD 拷贝目标
  `0xA7100000` 与 cache 维护。

## 5. 下一步（桌面 spike，先不刷机）

1. **UEFI 包**：从 `reference/uefi-fuxi` 复制 `fuxiPkg` → `m2381Pkg`，改
   DSC/FDF/DEC、FD base/size、SimpleFbDxe、FuxiBootMemoryDxe + 高段 DRAM、
   FuxiKeyInputDxe、UniXNUDxe + DarwinBoot、BootShim DTB 保存；加 OEM
   `UFSDxe`/`DiskIoDxe`/`PartitionDxe`/`Fat.efi`（`artifacts/vendor/`）。
2. **ADT**：`mkadt.py --m2381` 加 `darwinboot-memory-size`、`dram-base/size`
   对齐 3–4GiB 窗；`owned_memory_size` 同步。
3. **DarwinBoot**：打 guacamole 的 32MiB 同余 patch；确认 16K 内核的
   Kernel.Base 低位处理。
4. **ESP**：100MiB FAT32，放 UniXNU.efi、toml、kernel、ADT；`fsck.fat`。
5. **boot.img**：BootShim+FV 3MiB，gzip，v4，空 ramdisk；image_size 正确。
6. **桌面判据**：`unzip -t`/`fsck.fat`/`mkbootimg --unpack` 对拍；sha 记录；
   不刷机。

## 6. 开放问题

- **stock DTB 选型**：用 `docs/hardware/stock-live-fdt.dtb`（ABL 运行时，774K）
  还是用 `stock-fdt.dts` 重编译？前者更接近真机，后者可裁剪；UEFI 侧
  `/memory` 的 8 段与 reserved-memory 必须与 GCD 对上。
- **高段 DRAM**：13GB 连续段如何加入 GCD；是否需要 `ADD_ABOVE_4GB` 变体；
  加多少（3–4GiB）才既够 DarwinBoot 又避开 reserved。
- **UniXNU.efi 构建**：Linux 上 EDK2 CLANGPDB 能否直接出；大小；是否需要
  macOS。当前计划 Linux 构建 UEFI，只有内核重建才需要 macOS。
- **DarwinBoot 子模块坏 pin**：`reference/unixnu` 的 gitlink
  `d099354d0eab34df5088ca1ba89c9c88bc3260bb` 在上游不存在；用
  `reference/darwinboot` HEAD `85332b3` 或 guacamole 的补丁版本。
- **BootShim DTB 保存**：Mu-Silicium 的 BootShim 是否已保存 x0；若没有，
  要加 HOB/全局变量。
- **recovery_a 现状**：首次刷前读回，确认不是 nura 正在用的 ESP；若是，
  nura 侧重刷自己的镜像。
- **ESP 的 `unixnu.toml` schema**：以 guacamole 的实际 toml 为准，先复制再改
  路径/标题/超时；不要凭记忆写。
- **按键**：FuxiKeyInputDxe 的 SPMI SID/键码是否与 m2381 PMK8550 一致。
- **显示**：SimpleFbDxe 在 `0xb8000000` 上出菜单是否与 ABL 的 splash 重叠/
  花屏；先上机看，必要时改 stride/格式。
