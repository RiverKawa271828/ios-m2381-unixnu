# B2 启动链（m2381）

> 目标：ABL → boot_b 的 BootShim+Mu UEFI → recovery_a ESP 上的 UniXNU.efi →
> DarwinBoot → 现有 m2381 16K ios-sweet 内核 → 现有 userdata iOS16 root。
> **不换 iOS 镜像；boot_b 不放 payload；userdata 不动。**

## 1. 总图

```
XBL
 └─ ABL（v4 boot.img，只认 kernel，忽略 ramdisk）
     └─ gunzip kernel -> 0x80080000
         └─ BootShim（ARM64 header + FV 拷贝）
             ├─ 把 UEFI FV 拷到 0xA7100000（3MiB）
             ├─ dc cvac / ic iallu
             └─ br -> Mu-Silicium UEFI
                 ├─ SimpleFbDxe：GOP 建在 ABL 帧缓冲 0xb8000000
                 ├─ FuxiBootMemoryDxe：/memory -> GCD；加高段 DRAM；发布块设备
                 ├─ FuxiKeyInputDxe：PMK8550 PON -> SimpleTextIn
                 ├─ UniXNUDxe：注册 UniXNU.efi 为 Boot####
                 ├─ MsBootPolicy：扫描 FAT 卷 -> \EFI\BOOT\BOOTAA64.EFI
                 └─ UniXNU.efi
                     ├─ 扫描卷 -> \EFI\UniXNU\unixnu.toml
                     ├─ 菜单/超时
                     └─ DarwinBoot
                         ├─ 校验 Mach-O、ADT、窗口
                         ├─ 分配 3–4GiB 窗（0x8c0000000 高段）
                         ├─ 拷 kernel/ADT/args；无 ramdisk/trust cache
                         ├─ ExitBootServices、MMU off、EL1
                         └─ XNU start.s
                             ├─ arm_init：读 ADT dram-base/size
                             ├─ arm_vm_init：physBase/memSize
                             ├─ UFS -> rd=ufsroot0
                             └─ launchd
```

## 2. boot_b：只有固件

- 内容：`Image.gz = gzip(BootShim + UEFI FV)`。
- `BootShim` 的 arm64 header：`text_offset = 0xA7100000`（FD_BASE），
  `image_size = sizeof(BootShim)+sizeof(FV)`，`flags = 0`；ABL 在
  `[K+0x38] == ARMd` 时校验 `image_size <= DtLoad-K`（约 83.5MB）。
- BootShim 做的事：把紧跟其后的 FV 从 `0x80080000` 附近拷到 `0xA7100000`，
  `dc cvac`/`ic iallu`，`br`。
- **ramdisk**：空占位 cpio（仅 mkbootimg 格式要求）；ABL 不会把它交给任何人。
- **不要** appended DTB（ABL 若看到会校验结束地址；Mu 用 x0 传入的 DTB）。
- boot.img 约 3MB，boot_b 96MiB 无压力。

## 3. recovery_a：100MiB FAT32 ESP

```
/EFI/BOOT/BOOTAA64.EFI        UniXNU.efi（构建产物）
/EFI/UniXNU/UniXNU.efi        同一份（可选；给 UniXNUDxe）
/EFI/UniXNU/unixnu.toml       菜单 + Payloads 路径
/Payloads/kernel.macho        kernel.development.m2381（17,208,544B）
/Payloads/kernel.macho.adt    m2381 ADT（约 12.6KB）
```

- 标签 `M2381ESP`；512B 扇区、2 扇区/簇（nura ESP 实测形态，Mu FatDxe 可读）。
- 100MiB 容量：UniXNU.efi（预计 1–3MB）+ kernel 17MB + ADT 13KB，余量充足。
- **不**放 trust cache、**不**放 Data seed：内核是 ios-sweet 16K，root 在 userdata。
- ABL 从不引导 recovery_a；“recovery 三重死”是 ABL recovery 启动路径，与本方案无关。

## 4. UniXNU 配置

- `unixnu.toml` 以 `reference/UniXNU-guacamole` 的实际文件为模板改：
  - 标题/菜单项；
  - `Payloads/kernel.macho`、`kernel.macho.adt` 的相对路径；
  - 超时 3s；
  - 不启用 GPU addon / Data seed / trust cache。
- UniXNU 的 `Config.c` 会扫描所有 `SimpleFileSystem` 卷找
  `\EFI\UniXNU\unixnu.toml`（`package/UniXNUPkg/Library/UniXNULib/Config.c`），
  所以 ESP 不要求是“启动卷”。
- 若 UniXNUDxe 在 FV 里自动注册了 Boot####，菜单会出现；没有的话
  `\EFI\BOOT\BOOTAA64.EFI` 的 fallback 也能进。

## 5. DarwinBoot 窗口

- ADT `/chosen/darwinboot-memory-size` = 3–4GiB（256MiB–8GiB，2MiB 对齐）。
- DarwinBoot 扫描 UEFI memory map 的 `EfiConventionalMemory`，取
  `Candidate = ALIGN_VALUE(PhysicalStart, 32MiB) + (Kernel.Base & (32MiB-1))`，
  `AllocatePages(AllocateAddress, EfiLoaderCode, ...)`。
- 16K 内核的 bootstrap 用 32MiB L2 块，所以**必须**做这个同余对齐
  （guacamole 的 `darwinboot-guacamole.patch`）。
- 窗口内依次放 kernel 段、boot args、ADT、ramdisk（无）、trust cache（无）；
  `Args->MemorySize = WindowSize`、`PhysicalBase = Address`。
- XNU `arm_vm_init` 用 `physBase/memSize` 管内核 RAM；ADT `dram-base/size`
  必须与窗口一致，否则 `arm_init` 会按 13GiB 记账/越界。

### 5.1 高段 DRAM 进 GCD

fuxi 的 `FuxiBootMemoryDxe` 只把 `/memory` 中 **4GB 以下**未占用段加入 GCD
（`ADD_LIMIT 1GiB`、`ADD_BELOW 4GB`、`MIN_PIECE_SIZE 16MB`）。m2381 的
3–4GiB 连续窗在 `0x8c0000000`，必须扩展：

- 方案 A：在 `FuxiBootMemoryDxe` 里加 `ADD_ABOVE_4GB` 分支，把
  `/memory` 中 `0x8c0000000+0x140000000`（或用户指定的 3–4GiB 子段）加入 GCD。
- 方案 B：在 DSC 里换一个 m2381 专用 memory driver，直接按地址加。
- 只加窗口所需大小（3–4GiB），不要一次把 13GiB 全给 UEFI；UEFI 只需要
  能分配窗口 + 自身堆。
- 加入 GCD 前要确保这段没有 reserved-memory 子节点；/memory 的
  `0x8c0000000` 段在 stock FDT 里没有嵌套 carve-out。

## 6. ADT

`tools/mkadt.py --m2381` 生成，需补/改：

| 字段 | 值 |
|---|---|
| `dram-base` | `0x8c0000000` |
| `dram-size` | 窗口大小（如 `0x100000000` = 4GiB） |
| `darwinboot-memory-size` | 同窗口大小 |
| `owned_memory_size` | 同窗口大小（不要旧的 13GiB） |
| `unique-chip-id` | 现有 dev-fused 值（与 r34 一致） |
| 其他 | random-seed、boot-manifest-hash、crypto-hash-method、display-scale、chip-id 0x8015 保持 |

- `arm_init.c:643-658` 没有 `/chosen/dram-base`/`dram-size` 会 panic。
- `arm_vm_init.c:1872-1887` 用 boot args 的 `physBase`/`memSize`（DarwinBoot 填）。
- 旧 ADT（`artifacts/adt/m2381-sweet-13g.adt`，sha `c85b01f0…`）只作对照，
  **不要**直接用于 B2。

## 7. 构建步骤（桌面）

### 7.1 UEFI

```bash
# 从 uefi-fuxi 派生（不要直接改 reference/）
cp -a reference/uefi-fuxi/Platforms/Xiaomi/fuxiPkg devices/m2381/uefi/m2381Pkg
# 改 m2381Pkg/{m2381.dsc,m2381.fdf,m2381.dec}、DeviceBuild.py、MemoryMapLib
# - FD base 0xA7100000, size 0x300000
# - SimpleFbDxe / FuxiBootMemoryDxe(+高段) / FuxiKeyInputDxe
# - UniXNUPkg/Application/UniXNU、UniXNUPkg/Driver/UniXNUDxe、DarwinBoot
# - BootShim 保存 x0 DTB
# 构建（Linux CLANGPDB；具体命令以 uefi-fuxi 的 DeviceBuild.py 为准）
```

产物：`Build/m2381Pkg/.../FV/SILICIUM_UEFI.fd`，约 3MiB。

#### 7.1.1 UFS 块设备（B2 硬前提）

- `uefi-fuxi` / `uefi-sheng` 的 FV 里**没有** UFS DXE（只有 `MassStorage`
  app），Mu 默认**看不到 recovery_a**。不补这一层，ESP 里的 UniXNU.efi 根本
  不会被扫到。
- 加入本机 stock UEFI 的 OEM 二进制（`artifacts/vendor/meizu20-uefi-mods/`，
  本地 vendor blob，**不入 git、不外发**）：
  - `UFSDxe.efi`（132K，sha `e9df76b4…`）：先试 stock；若命中 nura 记录的
    157 assert，换 `UFSDxe-meizu20-157assertNOP.efi`（sha `8203aa89…`）。
  - `DiskIoDxe.efi`（40K）、`PartitionDxe.efi`（52K）、`Fat.efi`（64K）；
    `UFSDxe.depex` 2B（无 deps）。
- 若 OEM UFSDxe 的 fDeviceInit 不过，再考虑 nura 的 `UfsDeviceInitDxe`
  （`docs/archive/nura-uefi/UfsDeviceInitDxe-r23bg.c`）。它骑在 OEM UFSDxe
  已建好的 UTRL 列表上，**不自己发布 BlockIo/BDS**；只在最坏情况用。
- 验证点：Mu 启动后能枚举 `recovery_a` 的 FAT 卷（label `M2381ESP`），
  UniXNU 菜单能看到它；看不到就不要继续 B2，回 B1。
- 另：fuxi.dsc 里 `DarwinBoot.inf` 与 `FuxiBootMemoryDxe.inf` 是注释掉的，
  m2381Pkg 必须打开（见 7.2 之后的说明）。

### 7.2 ESP

```bash
dd if=/dev/zero of=out/esp.img bs=1M count=100
mformat -i out/esp.img -F -T 204800 -h 16 -s 63 -H 0 -r 0 -c 2 -v M2381ESP ::
mmd   -i out/esp.img ::/EFI ::/EFI/BOOT ::/EFI/UniXNU ::/Payloads
mcopy -i out/esp.img UniXNU.efi        ::/EFI/BOOT/BOOTAA64.EFI
mcopy -i out/esp.img UniXNU.efi        ::/EFI/UniXNU/UniXNU.efi
mcopy -i out/esp.img unixnu.toml       ::/EFI/UniXNU/unixnu.toml
mcopy -i out/esp.img kernel.macho      ::/Payloads/kernel.macho
mcopy -i out/esp.img kernel.macho.adt  ::/Payloads/kernel.macho.adt
fsck.fat -n out/esp.img
```

### 7.3 boot.img

```bash
# BootShim 编译时 -DFD_BASE=0xA7100000 -DFD_SIZE=0x300000 -DREQUIRES_KERNEL_HEADER=1
# compile_shim 会写 text_offset / image_size
cat BootShim.bin UEFI.fd > Image
# 如 compile_shim 没写 image_size，手工在 Image+0x10 写 uint64(len(Image))
gzip -9 -c Image > Image.gz
# 空 cpio
mkbootimg.py --header_version 4 --pagesize 4096 \
  --kernel Image.gz --ramdisk empty.cpio --output out/boot.img
```

- `image_size` 必须 = `sizeof(BootShim)+sizeof(FV)`，且 ≤ 83.5MB。
- 不要 `--dtb`/append DTB；不要 `ramdisk_offset`。
- 用 `unpack_bootimg` 反解对拍 kernel/ramdisk。

### 7.4 验证

- `unzip -t` / `fsck.fat` / `gunzip -t`；
- `sha256sum` 记录；
- 不刷机。

## 8. 上机门

1. boot_b 3MB 固件 → Mu 菜单（SimpleFbDxe，1080×2400）。
2. UniXNU 读到 ESP 的 toml，显示菜单/超时。
3. DarwinBoot 打印窗口分配（成功/NOWINDOW），加载 kernel。
4. XNU banner / UFS root / launchd。

任何一门失败，先回滚 boot_b（mu-r69）或恢复 recovery_a（nura ESP），不要连续试。

## 9. B1 回退（仅当 ESP 路线在 Mu 侧走不通）

- 64MiB FAT payload 嵌进 kernel 成员：
  `gzip(BootShim + FV + payload)`；BootShim 把 payload 留在 `0x80080000` 附近
  或拷到 `0xA8000000`；用 UniXNU 的 RAM-disk/块设备驱动发布 payload。
- 预算：67MB < 83.5MB；boot_b 96MiB 仍够。
- 具体机制可复用 `legacy/sweet/shim/build.sh:22-27` 的
  “payload 嵌 kernel 尾部 + image_size”做法，但要换成 FAT payload + Mu 驱动。
- B1 是 fallback，不先做。
