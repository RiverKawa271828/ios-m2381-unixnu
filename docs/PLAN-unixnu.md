# 上游切换评估：ios-sweet → UniXNU（m2381 / SM8550）

- 日期：2026-10-22
- 触发：用户提出把跟踪上游换成 <https://github.com/UniXNU/UniXNU>，理由“毕竟是同一个 soc”（m00001）
- 范围：只做评估与资料核对；**没有改任何工程文件、没有刷机、没有 push**
- **2026-10-22 落地：本文件已迁入新工作区 `docs/PLAN-unixnu.md`；B2 为当前主线，施工细节见 `AGENTS.md`、`docs/BOOT-CHAIN.md`、`docs/STATE.md`。**
- 结论先行：**技术上成立，且比“同一个 SoC”更强——m2381 的 stock FDT 就是高通 kalama MTP 参考板，低 4GB 的 UEFI 保留区与 fuxi（小米 13）逐项同址；建议分两步：先走 B（只换 UniXNU 启动链，保留现有 iPhone X 16.7.16 镜像/内核，guacamole 已实证），A（4K + iPadOS 17.7.11 全量）作为后续可选；不是原地 rebase，也不要在第一周抹 userdata。**

---

## 0. 一句话结论

**分两层换：先换启动链，再决定是否换内核/镜像。** 理由不是“SoC 相同”这种泛泛的复用，而是三条硬证据：

1. **同一套 Qualcomm SM8550 板级/UEFI 布局**：m2381 stock FDT 里的 bootloader log 在 `0xA7605000+0x8000`，与 uefi-fuxi 的 `MemoryMapLib` 里 “Log Buffer 0xA7605000+0x8000” 完全同址；FD 窗口 `0xA7000000`、FD `0xA7100000+0x300000`、Kernel `0xA8000000+0x10000000`、Display `0xB8000000+0x2B00000` 也逐项相同。
2. **同一块面板量级**：1080×2400 AMOLED、stride 4320、splash/连续扫描区 `0xb8000000+0x2b00000`；nuwa（小米 13）就是 1080×2400，我们的 shim 已经在这块 framebuffer 上出图，SimpleFbDxe 的 GOP 路径可直用。
3. **UniXNU 的机制正好绕开 r34 的死结**：它不修改 Apple kernelcache 里的 kext，而是从 Apple 源码构建 4K XNU，再用 NativePrelink 把原版 kext 原样搬进来；root 用原版 iPadOS + trust cache + owner-root。r34 的 launchd 签名缓存遮蔽、mad fatal 都属于我们这套“16K 改造 root + 自签/改 kext”路径上的问题，sheng 上没有出现（是否在 m2381 上消失仍需上机实证，不能打包票）。

**关键区分（上一版没讲清，导致“必须换 iOS 镜像”的误读）**：UniXNU 有两层——

- **① 启动链层**：BootShim → Mu-Silicium UEFI → UniXNU → DarwinBoot。它只负责“把任意一个 XNU Mach-O + ADT 放进内存并按 Apple boot args 跳过去”，**与内核页大小/根盘版本解耦**。这一层可以直接启动我们现有的 16K iOS16 内核和 iPhone X 16.7.16 根盘。`UniXNU-guacamole` 已经这么做并硬件实证（见 §5.1）。
- **② 内核/用户态层**：4K 源码 XNU + NativePrelink + iPadOS 17.7.11 + trust cache + owner-root。这一层才是“换 iOS 镜像”的那一层，也是解决 r34 死结的机制所在。

所以：**“用他的启动链启动我们的镜像”成立；“用他的 4K 内核跑我们的 iOS16 镜像”不成立**（内核版本/页大小/prelink 都对不上）。本轮先做①，②留作后续可选。

**用户 10-22 拍板（B2 的前提）**：不做多系统管理；nura 是 nura 的镜像，iOS 是 iOS 的镜像，切换时整盘重刷。因此 `recovery_a` 可以**专用**为 iOS/UniXNU 的 100MiB FAT32 ESP，不必保留 nura 的 Linux 内核；`boot_b` 只放 Mu/UniXNU 固件。这样 payload 既不走 boot.img ramdisk、也不占 96MiB，而是走 ESP——即下文的 **B2**，作为主路线；B1（payload 内嵌 kernel 成员）只在“绝不碰 recovery”时保留。

代价也要说清楚：**全量 A 才是新 port，不是 rebase。** 现有 84 件 ios-sweet 补丁、iOS 16 根盘、r34 的 launchd/mad 调试成果，在②下基本退役，只保留为 fallback/hybrid 与硬件知识；到桌面的现实预期是 **3–6 周**（含 spike）。**B（只做①）** 的代价是：mad/launchd 仍在 kernel/root 侧，启动链本身不解决；也拿不到 4K/GPU 的长期收益；但 1–2 周可上机；B2 的 boot_b 只有 ~3MB 固件，payload 17MB 内核 + ADT 放 recovery_a 的 100MiB ESP，96MiB 完全不紧张。

---

## 1. 为什么“同一个 SoC”这次真的成立

先看 m2381 的 stock FDT。它的根节点写的是：

```
model = "Meizu M2381(kalama MTP) meizu,board-id = <0x1 0xf>";
compatible = "qcom,kalama-mtp", "qcom,kalama", "qcom,mtp";
qcom,msm-id = <0x218 0x20000 0x207 0x20000 0x207 0x10000 0x218 0x10000>;
qcom,board-id = <0x10008 0x00>;
meizu,board-id = <0x01 0x0f>;
meizu,chip-version = "M2381 MP";
```

即 **高通 kalama MTP 参考板 + 魅族覆盖**，不是魅族自己另起的一套内存布局（model 字符串把 meizu board-id 也拼进去了，是原厂 FDT 的写法，不影响判断）。证据文件：[stock-fdt.dts](/home/ri/work/meizu20/meizu20-m1/flyme-forensics-20261019/fdt/stock-fdt.dts)。

| 项目 | m2381 stock FDT / nura DTS | fuxi（小米 13）/ sheng | 判断 |
|---|---|---|---|
| 板型 | kalama MTP，board-id `0x10008` | fuxi 同为 kalama/SM8550 | 同参考板 |
| UEFI FD 窗口 | `0xA7000000` 4MiB 窗口（由 bootloader log 反推） | FD Reserved `0xA7000000+0x100000`，UEFI FD `0xA7100000+0x300000` | 同址 |
| bootloader log | `0xA7605000+0x8000`（stock `bootloader_mem`） | `Log Buffer 0xA7605000+0x8000` | **逐字节同址** |
| Kernel / Mu Reserv | `0xA8000000+0x10000000`（nura 的 ramoops 就放在这个洞） | `Kernel 0xA8000000+0x10000000` | 同址 |
| Display / splash | `0xB8000000+0x2B00000`，1080×2400，stride 4320 | `Display Reserved 0xB8000000+0x02B00000`；nuwa 1080×2400 | 同址同分辨率 |
| Demura | `0xA2A80000+0x2B00000`（stock/Nura 都引用） | `Display Demura 0xA2A80000+0x2B00000` | 同址 |
| UFS | 控制器 `0x1d84000`，PHY `0x1d80000`，ICE `0x1d88000` | SM8550 同 IP | 同 SoC |
| GPU | Adreno 740 | sheng 的 NativeAdreno A740 | 同 GPU |
| PMIC / 按键 | PMK8550 PON（pwrkey/resin） | fuxi `FuxiKeyInputDxe` 走 SPMI PMIC keys | 同 PMIC 家族 |
| GIC | `0x17100000` / `0x17180000` | APSS `0x17000000+0x2000000` | 同地址段 |

差异只有：**RAM 16GB（8 段，其中 `0x8C0000000` 起 13GiB 连续高段）、UFS 512GB、触摸 GT9916、手机而非平板**。这些差异全部落在“板级数据/单个驱动”层面，不落在 UEFI 平台骨架层面。

> 换句话说：fuxi 能当模板，不是因为“都是 SM8550”，而是因为 **m2381 和 fuxi 共用同一套 Qualcomm UEFI 低 4GB 保留区、同一块 1080×2400 面板、同一代 PMIC**。这三点才是真正省时间的部分。

---

## 2. UniXNU 与我们当前路线的本质差异

| 维度 | 当前 ios-sweet 16K 路线 | UniXNU 4K 路线 |
|---|---|---|
| 内核来源 | 改 Apple kernelcache / kext 二进制（84 件补丁） | 从 Apple 源码 `xnu-10063.141.1` 构建（BCM2837/4K 配置） |
| 页大小 | 16 KiB（A11 风格） | 4 KiB（ARM64 标准；硬件两者都支持，shim 探针 tg16=1 已证） |
| 原版 kext | 尽量替换/改写 | NativePrelink 把原版 kext 段搬进新内核，代码不动、只重绑指针 + slide |
| 签名 / AMFI | 自签、改 kext、launchd cache 补丁 | 原版 kext + Static/System trust cache + `amfi-only-platform-code`；DarwinBoot 校验 trust cache |
| root | 改造的 iOS 16 root + 大量 tweak | 原版 iPadOS 17.7.11 的 OS/SystemOS 窗口 + Data seed + owner-root；内核按卷 UUID 挂 root |
| 启动链 | shim → XNU 直跳 | BootShim → Mu UEFI → UniXNU 菜单 → DarwinBoot → XNU |
| 当前墙 | r34：launchd 缓存遮蔽 LaunchDaemons + mad fatal | sheng 已到桌面；没有我们这两个改造引入的问题 |
| 维护性 | 每次 iOS 版本/补丁要重做 | 上游活跃、同 SoC 设备仓可复用，4K 栈是长期方向 |

**关键判断**：r34 的 mad fatal 和 launchd 缓存问题，根因在“我们改了原版组件 + 改了 root + 16K 兼容层”，不在硬件。UniXNU 的机制上不走这条路，所以**有很高概率自然消失**；但 m2381 是第一次上这套栈，仍要按“未验证”对待。

---

## 3. 可以直接复用 / 必须新做

### 3.1 直接复用（这是“换”的最大底气）

**UEFI 侧（uefi-fuxi 已是一个真实验证过 v3 启动的小米 13 板包）**
- `reference/uefi-fuxi/Platforms/Xiaomi/fuxiPkg/`：`DeviceBuild.py`、`Drivers/FuxiBootMemoryDxe`、`Drivers/FuxiKeyInputDxe`、`Library/MemoryMapLib`、`fuxi.dsc/fdf/dec`。
- `FuxiBootMemoryDxe` 是 DT 驱动的：从 `/memory` 加未占用 DRAM（`ADD_LIMIT 1GiB`、`ADD_BELOW 4GB`），并把 `/chosen linux,initrd-start/end` 发布成只读块设备，让 payload FAT 变成 UEFI 卷；**没有把板级地址写死**。
- `SimpleFbDxe` 直接在 ABL 留下的 framebuffer 上发布 GOP，不加载 vendor 显示驱动；我们在同一块 `0xb8000000` 上用 shim 出过图，路径已实证。
- `KailuaPkg`（SM8550）、`qrd8550`/`nuwa` 的板包结构、`uefi-sheng` 的 Mu 补丁（如 GIC spurious 0x105）都可参照。

**XNU / 设备无关层**
- `ArmDTPlatform`、`ArmCpu`、`NativeUfs`（`qcom,sm8550-ufshc`）、`NativeAdreno`（A740）、`NativeMobileDisplay`/`NativeLegacyDisplay`、`NativeHidInput`、`NativeAES`、`NativeGeniSpi`、`NativeRamBlock`、`NativeVirtioBlock`。
- `NativePrelink`、`VerifiedRoot`、trust cache、owner-root、`NativeCSHashCache`、`NativePan` 等整套 4K/安全/挂载机制，全部设备无关。
- `ShengPlatformPower.cpp` 其实是**通用 stub**：加载 `ApplePMGR`，检查 class size，所有方法 `unavailable(slot)` → panic；不碰具体时钟/电源寄存器。`sheng-pmgr=2` 可以直接沿用，真正要验证的是 UFS 电源是否已由 ABL 拉起。

**构建/镜像脚本**
- `unixnu-fuxi/scripts/make-fuxi-adt.py` → 改 `make-m2381-adt.py`；`build-sheng-payload.py`、`package-sheng-boot-v4.py`、`build-userdata-root-image.py`、`seed-data-skip-setup.py`、`prepare-owner-data-fixture.py`（qemu 仓）都是现成模板。
- `unixnu` 的 `bootimg` 支持 `header_version` 覆盖；我们 ABL 已验证 v4（shim/mu 都是 v4），fuxi 实测 v3，二选一即可，v4 优先。

### 3.2 必须新做

1. **uefi-m2381 板包**：从 fuxiPkg 改名，`MemoryMapLib` 按 m2381 stock FDT 校准低区保留项：
   - `gunyah 0x80000000+0xa00000`、`xbl_dt_log 0x81a00000+0x260000`、`aop_cmd_db 0x81c60000+0x20000`、`aop_config 0x81c80000+0x74000`、`chipinfo 0x81cf4000+0x1000`、`smem 0x81d00000+0x200000`、`mpss 0x89800000`（PIL）、`mzversion 0xa6000000+0x1000`；
   - UEFI 自有区沿用 fuxi：FD `0xA7100000+0x300000`、CPU vectors `0xA7600000`、log `0xA7605000+0x8000`、stack `0xA760D000+0x40000`、FV `0xA7AD9000+0x400000`、Kernel `0xA8000000+0x10000000`、Display `0xB8000000+0x2B00000`；
   - 16GB 高段（`0x8_80000000`、`0x8_c0000000`、`0xa_00000000`）在 Stage 1 不放进 MemoryMapLib，交给 HOB/GCD，和 fuxi 的处理一致。
2. **DTB**：从 [stock-fdt.dts](/home/ri/work/meizu20/meizu20-m1/flyme-forensics-20261019/fdt/stock-fdt.dts) 编译（或用 `stock-fdt.bin` 的 DTB 表）；`[[bootimg.dtb_node]]` 加 `/reserved-memory/splash_region`（stock 已有 `splash_region`，注意去重）；ABL 靠它保留 splash/面板。
3. **board.toml m2381 值**：iPad7,1 iPadOS 17.7.11 21H461、`chip_id 207`、`owned_memory_size 0x200000000`（8GiB，需 GCD 支持；失败退 4GiB）、`root_match_volume_uuid`（新生成）、`cpu_regs 0,0x100,...,0x700`、`userdata first_lba/last_lba`（**待从设备 GPT 读**）、`splash_region 0xb8000000/0x2b00000`、`unique_chip_id`（本地私存，不入仓）。
4. **触摸 GT9916**：UniXNU 基座**没有**触摸驱动（`NativeHidInput`/`NativeGeniSpi` 只是总线框架）；sheng 的是 Novatek NT36532E（SPI，平板），fuxi 仓也没有 Goodix/GT9916。我们的 GT9916 走 GENI I2C（`i2c4 0xa90000`、addr `0x5d`、irq `tlmm25`、reset `tlmm24`），需要新写或从 ios-sweet/nura 移植。UEFI 阶段可先只用电音量/电源键（FuxiKeyInputDxe）导航。
5. **iPadOS 17.7.11 用户态**：4K 内核是 `xnu-10063`，与现有 iPhone X 16.7.16 root 不兼容；需要下载 iPad7,1 17.7.11 IPSW（~7GB）、生成新 trust cache、Data seed、userdata。IPSW 是 Apple 版权件，只本地使用、不入仓不外发。

---

## 4. 硬约束与量化风险

### 4.1 boot_b 只有 96MiB（B1/A 的风险；B2 规避）

**B2 把 payload 放 recovery_a 的 ESP，boot_b 只装 `gzip(BootShim+FV)` ≈ 3MB，所以本节只约束 B1（payload 内嵌 kernel）与 A（全量 payload）。** 下面数字留作 B1/A 的预算。

`boot_b = 100,663,296 B`（[NOTES-m2381.md:650](NOTES-m2381.md#L650)）。UniXNU 默认 payload 是 **192MiB**（sheng/fuxi），那是给平板/大 boot 分区的预算，不能照抄。

guacamole 的实测给了换算基准：

- payload 64MiB → `boot.img = 72,302,592 B`，即 **overhead ≈ 8.3MB**（gzip shim+FV + DTB + header）。
- 96MiB boot → payload ≤ **~87MiB**；FAT32 内容 ≤95% → **≤ ~82MiB**。

payload 内容 = `kernel.macho` + ADT（约 12KB）+ trust cache + Data seed（+ 可选 GPU addon）。按估算：

| 组合 | kernel | trust cache | Data seed | 合计 | 82MiB 内？ |
|---|---:|---:|---:|---:|---|
| 乐观 | 25MB | 10MB | 32MB | 67MB | ✅ |
| 中性 | 35MB | 15MB | 32MB | 82MB | 边缘 |
| 悲观 | 50MB | 15MB | 32MB | 97MB | ❌ |

**kernel.macho 的真实大小目前没有公开数据**：UniXNU 的 macOS workflow 成功 run（39650589805）存在，但匿名下载日志/产物返回 403；网页/新闻也没有尺寸。**这是必须在 spike 里先测的第一个数。**

缓解手段（按优先级）：
1. 不装 GPU addon；Data seed 用 32MiB dense（qemu 的 fixture），`sheng-data-persist=2` 让写穿到 userdata，不必在 payload 里塞 1GiB seed；
2. 减 prelink roots（只保留启动/显示/输入/UFS 必需）；
3. FAT16 payload（EDK2 FatDxe 支持；`mformat` 不加 `-F`），可把最小 payload 从 64MiB 降下来；
4. 最后手段：用 guacamole 式 **hybrid**——保留 ios-sweet 16K 内核（17.2MB，已验证）+ UniXNU 启动链，先验证 boot chain；
5. 极端情况：在 payload 里压缩 kernel、DarwinBoot 解压（当前未实现，不建议先做）。

### 4.2 Meizu ABL 只从 boot_b 启动，且 **v4 的 boot ramdisk 不被取件**（关键修正）

UniXNU 文档/guacamole 默认把 payload 放在 **boot.img ramdisk**，再 `fastboot boot`；这套在魅族 ABL 上不成立：

- 我们的 ABL 是 stock QcomModulePkg，boot header v4 分组交付：`boot.img` 只有 kernel，ramdisk 在 `init_boot`，vendor ramdisk + DTB 在 `vendor_boot`（nura [BOOT-CHAIN-RE.md](/home/ri/work/nura-meizu-m2381/docs/BOOT-CHAIN-RE.md) §1.3、§5.2：`0x6d0c` 解析 v0-v2；v3/v4 走 vendor_boot 取件）。
- 我们的 shim 构建脚本已把这条钉成注释：`For M2381 the GKI ABL ignores the boot.img ramdisk entirely (initrd comes from init_boot -- proven on-device: the shim saw only the stock 5.7MB initramfs)`（[shim/build.sh:22-27](shim/build.sh#L22-L27)）。所以 **payload 放在 boot ramdisk 里 = UniXNU 永远看不到它**；不是“可能被拒”，是取件路径不同。
- 可行替代（我们已在用）：**payload 嵌进 kernel 成员**。shim 的 payload 在 `_payload_start`（16K 对齐），`image_size` 改成含 payload 的实际解压长度；ABL 只对 kernel 成员做 gunzip → `0x80080000` → 跳，ramdisk 留空 cpio。
- B1（备选）的落地：kernel 成员 = `gzip(BootShim + UEFI FV(3MB) + 64MiB payload FAT)`；BootShim 把 FV 拷到 `0xA7100000` 后跳；**首选同时把 payload 拷到 `0xA8000000`**（Mu「Kernel」保留区，256MiB 空洞），由 **M2381PayloadDxe** 以固定地址/BPB 发布成 FAT 卷；`FuxiBootMemoryDxe` 的 `linux,initrd-start/end` 路径用不上。这样既绕开 staging 在 DXE/EBS 后的可读性未知，也避免 DarwinBoot 把 payload 当空闲 DRAM 分配掉（FuxiBootMemoryDxe 加 DRAM 时把 `0xA8000000..0xAC000000` 排除/预留）。
- ABL 的硬限制（nura RE §5.3/§5.4）：解压缓冲 `DtLoad-K ≈ 83.5MB`；`3MB FV + 64MB payload = 67MB` 够；`image_size [K+0x10]` 只校验不消费，写 FD+shim 或实际 67MB 都行（建议 67MB，与真实解压长度一致）；不要 append DTB（DTB 在 FV/由 ABL x0 传入）；九项校验没有“payload 大小”门。**真正要实测的是：staging 区（0x80080000 起）在 UEFI DXE/EBS 后能否稳定读**——nura 记录过 Mu DXE 读 `0x803808B8` 的 external abort；因此首选 BootShim 先拷到 `0xA8000000` 再跳，staging 直读只作为对照。
- 另一个保守选项是 v2 header + 真 ramdisk（ABL 解析 v0-v2 会取 ramdisk），但离开我们已实证的 v4 路径，优先级低于 kernel-embed。

### 4.2.1 recovery_a = iOS 专用 ESP（B2，用户 10-22 拍板的主路线）

nura 的“内核放 recovery”不是 Android recovery 内核，而是 **recovery_a 上的 100MiB FAT32 ESP**：`\EFI\BOOT\BOOTAA64.EFI` = Linux 内核，Mu（boot_b）的 MsBootPolicy 扫所有 FAT 卷找到它。ABL 只启动 boot_b，不启动 recovery；ABL 的 recovery 校验链不在这条路上。这和我们红线里的“recovery 三重死”（把 boot.img 格式的 recovery 交给 ABL）是两件事。

既然不做多系统（nura/iOS 各自镜像，切换整盘重刷），recovery_a 就不再需要保留 nura 的 Linux 内核，直接做成 **iOS/UniXNU 专用 ESP**：

- ESP = recovery_a 100MiB，格式化为 FAT32（label `M2381ESP`，mformat 几何照 nura：`-T 204800 -h 16 -s 63 -H 0 -r 0 -c 2`）。内容：
  - `\EFI\BOOT\BOOTAA64.EFI` = `UniXNU.efi`（让 Mu 的 fallback 直接起 UniXNU，不依赖 Boot#### 顺序）；
  - `\EFI\UniXNU\unixnu.toml`；
  - `Payloads/kernel.macho` = `kernel.development.m2381`（17,208,544B）；
  - `Payloads/kernel.macho.adt`（12,636B）。
- `boot_b` 只装 `gzip(BootShim + FV)`（~3MB）+ 空 ramdisk；ABL 的 96MiB、83.5MB、v4 ramdisk 取件问题全部不涉及。
- nura 的 Mu（自带 stock UFSDxe）已证明这种 ESP 可读；但 **uefi-fuxi / uefi-sheng 的 FV 默认没有 UFS DXE**，m2381Pkg 必须加 stock `UFSDxe` + `DiskIoDxe`/`PartitionDxe`/`Fat.efi`（`artifacts/vendor/meizu20-uefi-mods/`，见 `docs/BOOT-CHAIN.md` §7.1.1），否则 Mu 看不到 recovery_a。这是 B2 的第一道门，先过它再谈 UniXNU 菜单。
- 现役 nura ESP 的实测（`BOOTAA64.EFI` 13,885,952B、free 90,138,624B）只作备份参考；B2 直接重刷 recovery_a，不保留 nura 文件。回 nura = 刷 nura 的 boot_b + recovery_a + userdata（整盘重刷）。
- 与 B1（payload 内嵌 kernel 成员）比较：B2 无尺寸/ABL 风险、不需要 `M2381PayloadDxe`、boot_b ~3MB；代价是 recovery_a 专用。**用户已拍板接受，所以 B2 为主，B1 仅作“绝不碰 recovery”的备选。**

### 4.3 抹 userdata（红线 #2）

现 r34 的 iOS 16 root 只在设备 userdata 上，本地没有 `iOSRoot-s*.simg`（nura artifacts 只有 mu-r69/ESP 镜像）。全量路线要刷新的 iPadOS userdata，等于退役它。动 userdata 前必须用户拍板；可先 `adb exec-out su dd` 拉 12GB 备份，或明确接受退役。

### 4.4 构建宿主：kernel/userdata 必须 macOS

- `unixnu build kernel` 用 xcodebuild + iphoneos SDK；`userdata`/`seed` 用 hdiutil → 只能在 macOS 或 GitHub macOS runner。
- 本容器能做的：UEFI 固件（EDK2 CLANGPDB）、payload（mtools）、bootimg（dtc/mkbootimg）、DTB/ADT 脚本、尺寸核算。
- 需要用户的 fork + GitHub Actions 跑 macOS workflow（现有 ios-sweet 的 CI 经验可复用）；不 push、不跑，等拍板。

### 4.5 上游成熟度（要有“自己修粗糙边”的预期）

- `reference/unixnu` 把 `drivers/DarwinBoot` 的 gitlink 指到 `d099354...`，该 commit 在 `UniXNU/DarwinBoot` 上游不存在（HEAD `85332b3`）→ 新 clone 的 `git submodule update` 会失败；需改 pin 或本地路径。不是战略问题，但说明“开箱即用”要打折扣。
- `unixnu-fuxi` 的 README 还是 sheng 的，触摸表是 Novatek（平板值），PMGR/触摸未验证；它是**模板仓**，不是已验证的手机 port。
- sheng 设备 README 的支持表：Display/Touch/UFS ✅，GPU ❌；但 10-22 新闻称 Metal 已打通（Adreno 740），设备 README 未同步。GPU 按“加分项/不确定”处理。

### 4.6 首次启动窗口（DarwinBoot）

- `darwinboot-memory-size` 要求 256MiB–8GiB、2MiB 对齐；DarwinBoot 在 UEFI 内存图里找 `EfiConventionalMemory` 连续段，按 32MiB 同余取址（`ALIGN_VALUE(PhysicalStart, 32MiB) + (Kernel.Base & (32MiB-1))`）。
- kernel + ADT + ramdisk（Data seed）+ trust cache 都复制进这个窗口，`Used ≤ WindowSize - 32MiB`。
- 我们 13GiB 连续高段，8GiB 理论可分配；但 `FuxiBootMemoryDxe`/`ShengBootMemoryDxe` 只把 **4GB 以下** 的 DRAM 加进 GCD（`ADD_LIMIT 1GiB`、`ADD_BELOW 4GB`），高段要靠 HOB/GCD 已映射。sheng 的 4.75GiB 窗口证明其固件可以，m2381 必须实测；失败时 panic 屏会打 `NO WINDOW ...` + `FREE ...` 行，很好判读。
- 16GB 机器用 8GiB 窗口会“浪费”8GB，但首启足够；后续可再加高段。

---

## 5. 三条路线对比

| 路线 | 内容 | 到桌面预期 | 优点 | 风险 |
|---|---|---|---|---|
| **A 全量切换（后续可选）** | 4K UniXNU + iPadOS 17.7.11 + 新 userdata + 新设备仓 | 3–6 周（含 spike） | 机制上绕开 r34 死结；同 SoC 模板成熟；4K 栈可长期维护；GPU 有路线 | 96MiB payload、ABL/initrd 实测、新 port、触摸/PMGR、需抹 userdata、macOS CI |
| **B2 混合 · ESP（主路线，推荐先做）** | boot_b = Mu/UniXNU 固件（~3MB）；recovery_a = iOS 专用 100MiB FAT32 ESP（UniXNU.efi + toml + kernel/ADT）；userdata = 现有 16K iOS16 root | 1–2 周 | boot_b 只有 ~3MB，96MiB/83.5MB/v4 ramdisk 全部绕开；ESP 100MiB 放 17.2MB kernel 只占 ~18%；UEFI/ESP 可在 Linux 做 | 不解决 mad/launchd（kernel/root 侧）；recovery_a 专用（nura 需整盘重刷，已拍板）；UFS DXE（vendor 二进制）需集成；高段 DRAM/窗口需实测 |
| **B1 混合 · kernel 内嵌（备选）** | boot_b = Mu/UniXNU 固件 + 64MiB payload 内嵌 kernel 成员 | 1–2 周 | 绝不碰 recovery；仍保留现有 root | boot_b 67MB / 83.5MB / ABL 读入缓冲需实测；比 B2 多一个 payload 发布 DXE |
| **C 不换** | 继续 r35（mobileactivationd 原地 stub 化） | 1–2 周（乐观，不保证） | 最快可能到桌面；不动大结构 | 16K 改造栈长期脆弱；launchd 缓存/签名类问题会反复；每次升级重做 |

**推荐先 B2，后 A；B1 仅作“绝不碰 recovery”的备选。** B2 先把 UniXNU 启动链在我们机器上跑通，且不动 iOS 镜像/userdata；B2 上机后再看 mad/launchd 是否仍在——若仍在，A 是“换内核/根盘”的解法，不是 B 的替代；若 B2 + r35 已到桌面，A 可推迟。C 保留为“B/A 都卡住且时间紧”的兜底。

### 5.1 先例：guacamole 已经用 UniXNU 启动链启动“我们的镜像”

`wochgn/UniXNU-guacamole`（OnePlus 7 Pro，SM8150）是本轮最直接的证据：

- 它**保留 ios-sweet 的 16K 内核**（`kernel.development.guacamole`，17,206,448 B）和 **iPhone X 16.7.16 (20H392) 根盘**；UniXNU/DarwinBoot 只替换启动链。iOS 镜像不换、不拷进 payload。
- payload = **64MiB FAT32**（32MiB 因 FAT32 最小区群数不够、EDK2 FatDxe 拒绝）；内容 = kernel Mach-O + 12,636 B ADT；**没有 trust cache、没有 Data seed**（内核自带 SWEET AMFI，root 在手机 UFS 上）。
- **注意：guacamole 的 payload 是从 boot.img ramdisk 进的（OnePlus ABL 取 ramdisk）；魅族 ABL v4 不取 boot ramdisk（见 §4.2），所以 B 不能照搬这一步：B2 把 payload 放 recovery_a 的 ESP（推荐，用户已允许），B1 才把 payload 嵌进 kernel 成员。**
- `boot.img` = **72,302,592 B**；r15 硬件实证：UniXNU 菜单 → DarwinBoot → XNU → iOS 用户态上屏。
- 三个关键补丁：`unixnu-guacamole.patch`（config-volume/卷根/Aloha shim header）、`darwinboot-guacamole.patch`（**16K 内核窗口按 32MiB 同余**：`ALIGN_VALUE(PhysicalStart, SIZE_32MB) + (Kernel.Base & (SIZE_32MB-1))`）、`aloha-guacamole.patch`（DTB 保住的 BootShim、ShengBootMemoryDxe、INFs）。
- 3GiB `owned_memory_size` 解决 `arm_vm_init` 为 0x2400000 framebuffer 预留 early I/O 页表不足；我们 FB 0x2B00000，3–4GiB 也应够（16K 内核的窗口基数必须与 32MiB 对齐，这是硬条件）。
- 我们的 `kernel.development.m2381` = 17,208,544 B，与 guacamole 17,206,448 B 几乎同尺寸；`tools/mkadt.py --m2381` 已给 dev-fused 身份/`unique-chip-id`/`dram-base`，只差 `darwinboot-memory-size` 和窗口/ADT 对齐。
- 结论：**B 不是理论可行，是别人已经在我们同一代 XNU（8796）+ 同一 iOS16 镜像上跑通过的路径**；m2381 要做的是把 Aloha/SM8150 的启动链件换成 Mu-Silicium/SM8550 的对应件。

不推荐“原地把 UniXNU 的 4K 补丁塞进我们 16K 树”：两套补丁目标版本/页大小不同（8796/16K vs 10063/4K），混用等于同时背两个技术债。

---

## 6. Spike 计划

### 6.0 B2 方案 spike（推荐先做，1–2 周，不抹 userdata）

目标：boot_b = Mu-Silicium/UniXNU 固件（~3MB）；recovery_a = iOS 专用 100MiB FAT32 ESP，放 `UniXNU.efi` + `unixnu.toml` + `kernel.development.m2381` + ADT；userdata 仍是 iPhone X 16.7.16 root。**不做多系统；nura 要回来时整盘重刷 nura 镜像。**

**B2 不用 iPadOS**：B2 的 kernel 是现有 m2381 16K Mach-O（`xnu-8796` / iOS 16.7.16 20H392），root 是设备上的 iPhone X 16.7.16；`unixnu kernel` / `unixnu userdata` 两步完全不跑，`[ipsw]` 只作模板占位。iPadOS 17.7.11 只属于 A（4K 源码 XNU + NativePrelink 那条线），不要把它带进 B2。

1. **桌面 UEFI（3–5 天）**：以 `uefi-fuxi` 为模板建 `uefi-m2381`：`fuxiPkg` → `m2381Pkg`，保留 `SimpleFbDxe`（ABL framebuffer 0xB8000000/0x2B00000）、`FuxiBootMemoryDxe`（把高段 0x8c0000000 起的 13GiB 连续段加进 GCD；不再需要 payload 预留）、按键驱动；DSC/FDF 把 `UniXNUPkg/Application/UniXNU`、`UniXNUDxe`、`DarwinBoot` 从注释状态打开。不需要 `M2381PayloadDxe`。**另加 OEM UFSDxe + DiskIo/Partition/Fat（`artifacts/vendor/meizu20-uefi-mods/`），先单独验证 Mu 能列出 `M2381ESP`**；没有它 Mu 看不到 recovery_a。
2. **移植 guacamole 三补丁**：DarwinBoot 的 32MiB 同余（16K 内核硬条件）；BootShim 保存 x0 DTB（Mu BootShim 若已保存则跳过）；UniXNU 的 config-volume/卷根修正；`aloha-guacamole.patch` 里 Aloha 专用的 shim header 检查在 Mu-Silicium 上不适用，按实际 header 形态裁剪。
3. **设备仓 + ESP（1–2 天）**：以 `unixnu-fuxi` 为模板建 `unixnu-m2381`；`[payload.adt]` 包一层 `tools/mkadt.py --m2381`，补 `darwinboot-memory-size`（3–4GiB，2MiB 对齐）并让 ADT 的 `dram-base=0x8c0000000`、`dram-size` 与 DarwinBoot 窗口一致；构建 `UniXNU.efi`、`unixnu.toml`、`kernel.macho`（17,208,544B）、`kernel.macho.adt`（12,636B）。建 100MiB FAT32 ESP（label `M2381ESP`，几何照 nura 的 `-T 204800 -h 16 -s 63 -H 0 -r 0 -c 2`）：
   - `\EFI\BOOT\BOOTAA64.EFI` = `UniXNU.efi`
   - `\EFI\UniXNU\unixnu.toml`
   - `Payloads/kernel.macho`、`Payloads/kernel.macho.adt`
   boot.img：kernel = `gzip(BootShim + FV)`，ramdisk = 空 cpio，header v4、pagesize 4096、`abl_base=0x80000000`；**不装 payload、不 append DTB**。
4. **尺寸**：boot.img ~3–4MB；ESP 100MiB，kernel+ADT 占 ~18%；ABL 的 83.5MB/96MiB 全部无关。
5. **上机（2–4 天，刷 boot_b + recovery_a）**：先看 UniXNU 菜单（可先只放 `UniXNU.efi` + 最小 toml），再带 kernel/ADT 看 DarwinBoot → XNU banner → launchd。失败判读：菜单不出现（Mu fallback / UFS DXE 未加载 / ESP FAT / `BOOTAA64.EFI` 路径）；`NO WINDOW`（高段 DRAM 未进 GCD / 窗口不对）；ADT 缺 `darwinboot-memory-size`；XNU 早期 panic。
6. **回滚**：回 nura = 刷 nura 的 boot_b + recovery_a + userdata（整盘重刷）；回 r34 = boot_b shim + 现有 userdata。B2 不动 userdata。
7. **B2 成功后**：mad/launchd 仍在 → 继续 r35（root 侧）或评估 A；B2 只换启动链，不解决 kernel/root 侧。

**B1（备选，不碰 recovery）**：boot_b 的 kernel 成员 = `gzip(BootShim + FV + 64MiB payload FAT)`；BootShim 把 payload 拷到 `0xA8000000`，新增 `M2381PayloadDxe` 发布；`image_size` ≈ 67MB（< 83.5MB）；boot_b 需先实测 ABL 的读入缓冲能否接受 ~30MB 的 boot.img；其余同 B2。

### 6.1 A 方案 spike（B 不够时再做，3–6 周）

### S0（桌面，3–5 天）：UEFI + 镜像，不上机

1. 本地建两个仓（先不 push）：
   - `reference/unixnu-m2381`：从 `unixnu-fuxi` 复制，改 `[board]`/`[payload]`/`[payload.adt]`/`[userdata]`/`[bootimg]`；
   - `reference/uefi-m2381`：从 `uefi-fuxi` 复制 `fuxiPkg` → `m2381Pkg`，改 `m2381.dsc/fdf/dec`、`DeviceBuild.py`、`MemoryMapLib`、`FuxiBootMemoryDxe`/按键驱动改名。
2. `make-m2381-adt.py`（从 fuxi 脚本改）；`stock-fdt.dts` 编译 DTB + splash 节点去重；`unique_chip_id` 本地生成。
3. Linux 构建：UEFI FD（3MiB）、payload（先用占位 kernel 测 FAT/尺寸）、bootimg v4；检查 shim ARM64 header、FD 大小、gzip、DTB 附加、boot.img 总尺寸。
4. 产出 `out/m2381/boot/boot.img` + 尺寸报告（FD/shim/payload/boot.img 各多少字节）。

**G1**：boot.img ≤96MiB；FD ≤3MiB；本地解包对拍通过。

### S1（macOS CI，2–4 天）：测 kernel / payload 真实尺寸

5. 用户 fork UniXNU + m2381 设备仓；`workflow_dispatch` 跑 `steps=kernel`（m2381 defconfig：从 `fuxi_defconfig` 复制，保留 `NATIVE_UFS`/`NATIVE_ADRENO`/`ArmDTPlatform`/`NativeMobileDisplay` 等）；记录 `kernel.macho` 字节数。
6. 跑 `steps=kernel userdata payload`（iPad 17.7.11 IPSW；trust cache；32MiB seed；`--skip-setup`）；记录 payload/boot.img 尺寸。

**G2**：`kernel + trust + seed ≤ ~82MiB` 且 boot.img ≤96MiB。若超，按 §4.1 缓解（去 GPU addon/减 roots/换 FAT16/缩 seed），再测。

### S2（上机，3–7 天）：启动链验证，仍不碰 userdata

7. 刷 boot_b（G1 的 UEFI + G2 的 payload）：
   - 先只刷 UEFI+menu（payload 可先空/占位），确认 UniXNU 菜单上屏；
   - 再刷带真 kernel 的 boot.img，看 DarwinBoot → XNU banner/panic。
8. 读屏/串口：`NO WINDOW`、trust cache 校验、DTB 解析、`AdtNode` 报错都要能定位。

**G3**：XNU 起、ADT 被接受、至少能看到 NativeUfs/root 尝试。通过才进 S3。

### S3（全量，1–3 周）：userdata + root + 桌面

9. 生成 iPadOS 17.7.11 userdata 镜像（OS + SystemOS 窗口）+ 32MiB seed + `sheng-data-persist=2` + skip-setup；**用户批准后**刷 userdata（先备份或确认退役 iOS16 root）。
10. UFS root → launchd → SpringBoard；逐项调 PMGR/UFS 电源、Data seed、Setup 跳过、显示缩放。
11. 触摸 GT9916 移植；按键；GPU 后置。

---

## 7. 需要用户拍板的事

1. **路线**：B2（推荐，已拍板不做多系统）/ B1（绝不碰 recovery 的备选）/ 直接 A / 继续 C。
2. **B2 需要确认**：允许把 recovery_a 格式化为 iOS 专用 100MiB FAT32 ESP（nura 要回来时整盘重刷）；boot_b 刷 Mu/UniXNU 固件；**userdata 不动**。
3. **B2 的桌面 spike 是否现在做**：建 `uefi-m2381`/`unixnu-m2381` 两个本地仓 + 100MiB ESP + ~3MB boot.img；全部可在 Linux 完成，不需要 macOS CI。
4. **B1** 只在“绝不碰 recovery”时做；A 的 macOS workflow 只在转 A 时需要。
5. **userdata 的处理时机**：B2 不动；A 的 S3 才需要。

在拍板前，设备保持现状：boot_b = r34 shim（`shim-boot-m2381.img`，339b8480），userdata = r34 iOS16 root（`iOSRoot-s-r34.simg`，7aaab96f）；随时可回 mu-r69。**本评估没有动过设备。**

---

## 8. 证据文件索引

| 内容 | 路径 |
|---|---|
| m2381 stock FDT（内存/保留区/面板/bootargs） | [/home/ri/work/meizu20/meizu20-m1/flyme-forensics-20261019/fdt/stock-fdt.dts](/home/ri/work/meizu20/meizu20-m1/flyme-forensics-20261019/fdt/stock-fdt.dts) |
| nura 主线 DT（m2381 硬件真值，含 ramoops/触摸/UFS/PMIC） | [/home/ri/work/linux-mobile-ports/arch/arm64/boot/dts/qcom/sm8550-meizu-20.dts](/home/ri/work/linux-mobile-ports/arch/arm64/boot/dts/qcom/sm8550-meizu-20.dts) |
| fuxi 低 4GB 内存图（28 区，uefiplatLA.cfg 逐项） | [MemoryMapLib.c](reference/uefi-fuxi/Platforms/Xiaomi/fuxiPkg/Library/MemoryMapLib/MemoryMapLib.c) |
| fuxi 启动内存驱动（DT 驱动 + initrd 块设备） | [FuxiBootMemoryDxe.c](reference/uefi-fuxi/Platforms/Xiaomi/fuxiPkg/Drivers/FuxiBootMemoryDxe/FuxiBootMemoryDxe.c) |
| fuxi 板包（DSC/FDF/按键/构建） | [fuxiPkg](reference/uefi-fuxi/Platforms/Xiaomi/fuxiPkg) |
| fuxi 设备仓（board.toml/ADT/payload 模板） | [unixnu-fuxi](reference/unixnu-fuxi) |
| DarwinBoot（窗口分配/32MiB 同余/trust cache/ADT） | [DarwinBoot.c](reference/darwinboot/DarwinBootPkg/Driver/DarwinBoot.c) |
| sheng 设备仓（已到桌面，官方 SM8550 参考） | [unixnu-sheng](reference/unixnu-sheng) |
| boot_b 96MiB 与启动链记录 | [NOTES-m2381.md:650](NOTES-m2381.md#L650) |

---

## 9. 附：m2381 与 fuxi 关键地址对照

| 区域 | m2381（stock FDT / nura DTS） | fuxi（uefi-fuxi） | 说明 |
|---|---|---|---|
| FD Reserved | `0xA7000000` 4MiB 窗口（推断） | `0xA7000000+0x100000` | 同 |
| UEFI FD | `0xA7100000+0x300000`（推断） | `0xA7100000+0x300000` | 同 |
| bootloader log | `0xA7605000+0x8000` | `0xA7605000+0x8000` | **同址实证** |
| Kernel / Mu | `0xA8000000+0x10000000` | `0xA8000000+0x10000000` | 同 |
| Display / splash | `0xB8000000+0x2B00000` | `0xB8000000+0x02B00000` | 同 |
| Demura | `0xA2A80000+0x2B00000` | `0xA2A80000+0x2B00000` | 同 |
| gunyah | `0x80000000+0xa00000` | `NOMAP 0x80000000+0x01200000` | 尺寸不同，按 m2381 校准 |
| xbl_dt_log | `0x81a00000+0x260000` | `XBL DT 0x81A00000+0x40000` | 尺寸不同 |
| aop_cmd_db | `0x81c60000+0x20000` | `AOP 0x81C00000+0xA0000` | 起点/尺寸不同 |
| smem | `0x81d00000+0x200000` | `SMEM 0x81D00000+0x200000` | 同 |
| chipinfo | `0x81cf4000+0x1000` | （fuxi 表未单列） | 按 m2381 加 |
| mpss / PIL | `0x89800000` 起（大区） | `PIL Reserved 0x89800000+0x19280000` | 同起点 |
| ramoops | stock `0xa5100000+0x200000`；nura 移到 `0xa8000000+0x200000` | （fuxi 表无） | stock 地址落在 Demura，会被 ABL 覆写；用 nura 的地址 |

> 结论：**UEFI 自有区逐项同址，低区保留项按 m2381 stock FDT 替换即可**；这就是“fuxi 板包改个名就能当模板”的技术依据，也是这次切换风险可控的原因。

---

## 10. 附：与 r34/r35 的关系

- r34 的结论（launchd 签名缓存遮蔽磁盘 LaunchDaemons、mad 52/52 崩在 stock stub line321）说明：问题不在 AMFI/硬件，而在“改造过的 root + 自签/改 kext + launchd cache”这条路径。
- r35 的“mobileactivationd 原地换已签名即退体”仍是最短路径到桌面（如果成功），但它只是绕过 mad，不解决这条路径的结构性问题。
- 换 UniXNU 后，r34/r35 的**调试结论仍然有价值**（证明硬件、AMFI、容器机制都没问题），但**补丁/root 产物不再复用**；保留旧树与 mu-r69，随时可回。
- 两件事不要混：**这次评估不建议立刻停 r35**；如果用户选择 A，r35 可以冻结为 fallback，等 G1/G2 结果再决定是否继续投入。
