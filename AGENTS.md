# ios-m2381-unixnu — 魅族 20 Classic（m2381 / SM8550）iOS via UniXNU

> **2026-10-22 新工作区。旧 ios-sweet 路线 `/home/ri/work/ios-meizu-m2381` 已废弃**
> （其 `DEPRECATED.md` 指向这里）。旧代码/旧笔记只作硬件史料，留在本仓
> `legacy/sweet/` 与 `docs/archive/`，不再作为施工面。当前方案 = **B2（ESP 启动链）**。
>
> 设备：魅族 20 Classic = **m2381**，SM8550（kalama MTP 参考布局），16GB LPDDR5X，
> 1080×2400 BOE AMOLED（cmd-mode DSC），UFS 4.0 512GB。
>
> **上游（2026-10-23 起跟随）**：基座 = `UniXNU/UniXNU`（remote `upstream`，
> pin `b7b6a6a`；`m2381` 分支 = 上游 main + 工作区提交，同步 =
> `git rebase upstream/main`）；设备模板 = `hanzi123-bilp/uefi-fuxi` +
> `unixnu-fuxi`。详见 `docs/UPSTREAM.md`；`reference/` 只读，不 push upstream。

## 0. 一句话

用 **Mu-Silicium UEFI + UniXNU + DarwinBoot** 启动**现有 iPhone X iOS 16.7.16 root +
m2381 16K ios-sweet 内核**，**不换 iOS 镜像**。内核/ADT 放 **recovery_a 的 100MiB FAT32
ESP**，boot_b 只放约 3MB 的 BootShim+UEFI 固件。A（UniXNU 4K 内核 + iPadOS 17.7.11）
是后续升级，不是当前目标。

## 1. 为什么这条路成立

- **guacamole 先例**（OnePlus 7 Pro，SM8150）：保留 ios-sweet 16K 内核 + iPhone X
  16.7.16 root，只换 UniXNU/DarwinBoot 启动链，**已上机到 iOS 用户态**。
  其 kernel 17,206,448B、ADT 12,636B、payload 64MiB FAT、boot.img 72,302,592B。
  我们的 kernel 17,208,544B、ADT 12,624B，体积几乎相同。
- **同 SoC + 同 UEFI 布局**：m2381 的 stock FDT model 是
  `"Meizu M2381(kalama MTP)"`，UEFI FD 窗口 `0xA7000000/0xA7100000+3MiB`、
  BootShim 的 256MiB 内核窗 `0xA8000000`、Display Reserved
  `0xB8000000+0x2B00000`、bootloader log `0xA7605000+0x8000` 全部与 fuxi/nuwa/sheng
  同址；显示 1080×2400、ABL 帧缓冲 `0xb8000000` 与 fuxi 完全一致。
- **ABL 的真实约束**（nura `docs/BOOT-CHAIN-RE.md` §1.3/§5.2/§5.3/§5.4 +
  本仓旧 `shim/build.sh:22-27` 实证）：Flyme 12.6 的 v4 boot.img **只放 kernel**，
  ramdisk 在 init_boot；魅族 ABL **完全忽略 boot.img 的 ramdisk**。ABL 把 gzip kernel
  解到 `0x80080000`，解压缓冲上限约 **83.5MB**，只校验 arm64 header 的 `image_size`
  与（若存在）appended DTB；没有 3MB 门限。**所以不能走 guacamole 的
  “payload 当 boot ramdisk”路线**，但可以走 ESP。
- **recovery_a 不是 Android recovery**：nura 的“recovery 三重死”说的是 ABL 把
  recovery 槽纳入正常启动校验链；本方案里 ABL **从不**引导 recovery_a，是 Mu 的
  `MsBootPolicy` 把它当 FAT 卷扫描。两者不是一回事。100MiB ESP 足够放
  UniXNU.efi + 17MB kernel + ADT。

## 2. B2 布局（当前方案）

```
boot_b (96MiB, Android v4 boot.img):
  kernel   = gzip(BootShim + UEFI FV 3MiB)
  ramdisk  = 空占位 cpio（ABL 忽略，只为 mkbootimg 格式）
  ABL gunzip -> 0x80080000；BootShim 把 FV 拷到 0xA7100000，dc cvac/ic iallu，br

recovery_a (100MiB FAT32 ESP, label M2381ESP；Mu 扫描，ABL 不引导):
  /EFI/BOOT/BOOTAA64.EFI        = 构建出的 UniXNU.efi（Mu 的 fallback boot）
  /EFI/UniXNU/UniXNU.efi        = 同一份（UniXNUDxe 的 Boot#### 用；可选）
  /EFI/UniXNU/unixnu.toml       = 启动菜单 + Payloads 路径
  /Payloads/kernel.macho        = kernel.development.m2381 (17,208,544B)
  /Payloads/kernel.macho.adt    = m2381 ADT (约 12.6KB)
  （无 trust cache、无 Data seed：内核是 ios-sweet 16K，root 在 userdata）

userdata (12GB, 现 r34 iPhone X 16.7.16 root):
  不动；内核 rd=ufsroot0 直接挂现有 root
```

**boot_b 不需要 17MB kernel，也不需要 64MiB payload**：ESP 绕开了
ABL 的 83.5MB 解压上限和 boot_b 96MiB 上限。boot.img 只有约 3MB 固件，
留足余量。

## 3. 需要新建/移植的东西

1. **`m2381Pkg`（UEFI）**：从 `reference/uefi-fuxi/Platforms/Xiaomi/fuxiPkg` 派生。
   - FD 基址 `0xA7100000`、大小 `0x300000`（3MiB，在 `0xA7000000` 4MiB 窗口内）。
   - `SimpleFbDxe` 直接在 ABL 帧缓冲 `0xb8000000` 上发布 GOP。
   - `FuxiBootMemoryDxe`：DT 驱动；把 `/memory` 中未占用的 DRAM 加入 GCD。
     **fuxi 版只加 4GB 以下（ADD_BELOW 4GB），m2381 必须额外把
     `0x8c0000000` 起的 13GB 连续高段加进 GCD**，否则 DarwinBoot 找不到
     3–4GiB 连续窗。
   - `FuxiKeyInputDxe`（PMK8550 PON 电源/音量键）先沿用，键位/SPMI SID 按
     m2381 校。
   - **UFS 块设备（B2 硬前提）**：`uefi-fuxi`/`uefi-sheng` 的 FV 里**没有**
     UFS DXE（只有 MassStorage app），Mu 默认看不到 recovery_a。本机 stock
     UEFI 的 OEM `UFSDxe.efi`（132K）+ `DiskIoDxe.efi` / `PartitionDxe.efi` /
     `Fat.efi` 已放在 `artifacts/vendor/meizu20-uefi-mods/`（本地 vendor blob，
     不入 git/不外发）；FV 里按 Binary 模块加入。`UFSDxe-meizu20-157assertNOP.efi`
     是 nura 的去 assert 变体；先试 stock，assert 再换。nura 的
     `UfsDeviceInitDxe` 源码在 `docs/archive/nura-uefi/`，只在 OEM UFSDxe 的
     fDeviceInit 失败时用。
   - FV 里加 `UniXNUPkg/Application/UniXNU/UniXNU.inf`、
     `UniXNUPkg/Driver/UniXNUDxe/UniXNUDxe.inf`、`DarwinBoot.inf`。
   - `BootShim` 保存 ABL 传入的 x0 DTB（HOB 或全局变量），供
     FuxiBootMemoryDxe/DarwinBoot 使用。
2. **`unixnu-m2381`（设备仓）**：从 `reference/unixnu-fuxi` 派生。
   - `board.toml`：SoC SM8550；B2 的设备身份 = **iPhone X（iPhone10,3 /
     iOS 16.7.16 20H392 / d22ap / chip 0x8015）**。`[ipsw]` 只是模板占位，
     B2 **不跑** `unixnu kernel` / `unixnu userdata`，**不需要 iPad IPSW**；
     kernel 用现有 Mach-O，ADT 用 `tools/mkadt.py`。不要照抄 fuxi 模板的
     `iPad7,1 / 21H461`。
   - `[payload]` 改为 **ESP 内容**而不是 boot.img ramdisk；`loader = firmware`；
     `size_mib` 不再受 96MiB 限制（ESP 100MiB）。
   - `[bootimg.image]` `header_version = 4`、`base 0`、`kernel_offset 0x8000`、
     `pagesize 4096`；**不要 ramdisk_offset / append_dtb**。
   - 不启用 `native-ufs-root`、`sheng-pmgr`、Data seed、trust cache：UFS/root
     由 ios-sweet 内核自己管。
   - kernel args 用旧 r34 那套（`debug=0xc serial=3 cpus=8 keepsyms=1
     rd=ufsroot0 ufsrw=1 rootrw=1 launchd_unsecure_cache=1 ...`），先原样，
     启动成功后再逐项调。
3. **ADT**：`tools/mkadt.py --m2381` 基础上补：
   - `/chosen/darwinboot-memory-size` = 3–4GiB（DarwinBoot 要求 256MiB–8GiB、
     2MiB 对齐）。
   - `/chosen/dram-base` = `0x8c0000000`、`/chosen/dram-size` = 同一窗大小
     （`arm_init.c:643-658` 没有这两个节点会 panic；`arm_vm_init.c:1872-1887`
     用 boot args 的 physBase/memSize）。
   - `owned_memory_size` 与窗口一致；不要再用旧的 13GiB。
   - 32MiB 同余：DarwinBoot 的 `AllocateWindow` 用
     `ALIGN_VALUE(PhysicalStart, 32MiB) + (Kernel.Base & (32MiB-1))`；guacamole
     的 darwinboot patch 就是为此。m2381 必须打同一补丁。
4. **ESP 制作**：100MiB FAT32（`mformat -C -F -T 204800 -h 16 -s 63 ...`，
   512B 扇区/2 扇区簇），label `M2381ESP`；`mcopy` 上述文件；`fsck.fat`。
5. **boot.img**：`gzip(BootShim + FV)`，arm64 header `image_size` = shim+FV，
   `mkbootimg --header_version 4 --ramdisk 空cpio`；不要 appended DTB。
6. **上机门**：先只刷 boot_b + recovery_a，不碰 userdata；门 =
   UniXNU 菜单 → DarwinBoot → XNU banner → launchd。

## 4. 红线（从 nura/旧仓继承）

1. 只碰 **boot_b + recovery_a + userdata**；xbl/abl/tz/modem/dtbo/super/persist
   永不碰。
2. **抹 userdata = 抹 Flyme 用户数据 + 现有 iOS root**；动 userdata 前必须用户拍板。
3. **fastboot 单通道**：本仓与 nura 侧任何刷写互斥；刷前 `getvar current-slot`
   必须 = b。
4. **dd 读回才算证据**；刷写时长不算。每轮刷完读回 sha 对拍。
5. **fastboot boot 是死路**（魅族 ABL 裁剪 CmdBoot），只能 `flash boot_b` /
   `flash recovery_a`。
6. 回滚件：`artifacts/images/mu-r69-97cdea20.img`（boot_b）、
   `artifacts/images/esp-recovery-v71.img.gz`（recovery_a 模板）、
   `legacy/sweet/images/shim-boot-m2381.img`（r34 boot_b 现状）。
7. iOS IPSW/Apple 件只本地使用，不入仓不外发。
8. 每轮独立可停；commit 不 push，推送/发布窗口 = 用户拍板。

## 5. 关键硬件常量（m2381，全部实机/FDT 实证）

| 项 | 值 |
|---|---|
| SoC | SM8550 kalama，8 核（1×X3 + 2×A715 + 2×A710 + 3×A510） |
| RAM | 16GB LPDDR5X；低 4GB 有大量 carve-out；高段 `0x8c0000000` 起 13GB 连续 |
| 显示 | 1080×2400 BOE AMOLED cmd-mode DSC；FB `0xb8000000` size `0x2b00000` stride 4320 |
| GIC | GICD `0x17100000`；GICR `0x17180000` stride `0x40000` 区 `0x200000`；VTIMER 27 / PTIMER 28；DS=1；SGI 页优先级写前先置 `GICD_CTLR.bit8` |
| UFS | 控制器 `0x1d84000`（UFSHCI 4.0）、PHY `0x1d80000`、ICE `0x1d88000`；Samsung KLUFG8RHHD-B0G1 512GB |
| 触摸 | Goodix GT9916（berlin 族），GENI I2C4 `0xa90000` 400kHz，addr `0x5d`，irq tlmm25 edge-falling，reset tlmm24 active-low，avdd l14b_3p2 / vddio l12b_1p8 |
| 按键 | PMK8550 PON：pwrkey + resin(vol-down)；SPMI |
| 分区 | boot_b 96MiB；recovery_a 100MiB；userdata 12GB（r34 root） |
| ABL | v4；gunzip 到 `0x80080000`；解压上限约 83.5MB；忽略 boot.img ramdisk；校验 image_size / appended DTB |
| UEFI 窗口 | FD `0xA7100000+0x300000`；内核窗 `0xA8000000+0x10000000`；Display `0xB8000000+0x2B00000`；bootloader log `0xA7605000+0x8000` |

更细的地址/证据见 `docs/HARDWARE-m2381.md` 与 `docs/hardware/` 下的
stock FDT / nura DTS。

## 6. 目录

```
AGENTS.md                 本文件
README.md                 入口
docs/PLAN-unixnu.md       完整评估 + B2/B1/A 方案
docs/STATE.md             当前状态、资产、下一步、开放问题
docs/BOOT-CHAIN.md        B2 启动链、内存窗、ESP 布局、构建步骤
docs/UPSTREAM.md          上游 pin、更新流程、remote 策略
docs/FLASHING.md          刷写/读回/回滚纪律
docs/HARDWARE-m2381.md    硬件常量与地址
docs/hardware/            stock FDT/DTS、nura DTS、DTB（只读参考）
docs/archive/             旧 sweet 笔记/交接（只作历史）
reference/                UniXNU/uefi-fuxi/guacamole 等克隆（gitignore）
research/ipsw/            iPhone X 16.7.16 IPSW + 抽取 DMG（gitignore）
research/legacy-sweet/    16K 内核重建套件 xnu-8796/buildtree（gitignore）
research/bin/ipsw         ipsw 抽取工具（gitignore）
research/ipsw-x/d22ap.adt mkadt.py 的输入（gitignore）
tools/                    mkadt.py / adt.py / remotezip.py
artifacts/kernel/         内核 zip + kernel.development.m2381（gitignore）
artifacts/adt/            旧 ADT 样本（gitignore）
artifacts/images/         回滚/ESP 模板（gitignore）
devices/m2381/            m2381Pkg / unixnu-m2381 未来落地处
legacy/sweet/             废弃 sweet 源码/补丁/镜像（只作史料）
```

## 7. 下一步（按顺序）

1. 建 `devices/m2381/` 骨架：从 `reference/uefi-fuxi` 派生 `m2381Pkg`；
   从 `reference/unixnu-fuxi` 派生设备仓。
2. 先解决 **高段 DRAM 进 GCD** + **DarwinBoot 32MiB 同余** + **ADT
   darwinboot-memory-size** 三件。
3. 做 ESP（100MiB FAT32）与 boot.img（3MB 固件），桌面验证文件/哈希。
4. 只刷 boot_b + recovery_a，按门 1–4 上机；userdata 不动。
5. 若 ESP 路线在 Mu 侧卡住，退回 **B1**（64MiB payload 嵌 kernel 成员，
   `legacy/sweet` 的 shim/build.sh 机制可复用），但 B2 优先。

> **禁止**再把 ios-sweet 的 84 补丁树当施工面；需要旧硬件结论时查
> `docs/archive/NOTES-m2381.md`，需要旧 shim 机制时查 `legacy/sweet/shim/`。
