# ios-m2381-unixnu — 魅族 20 Classic（m2381 / SM8550）iOS via UniXNU

> **2026-10-24 donor 换配（用户拍板，两步）**：根因 = 内核源码 xnu-10063.141.1
> （iOS 17.6 代）与 20H392（16 代）全程跨代配对（entropy 墙 + AppleImage4
> mix-n-match 墙同根因）。先换 iPhone XS `21H16`（手机身份）后发现其
> kernelcache 是 **fileset KC/chained fixups、无 KASLR 表**，prelink kit
> 需开发重绑定器才能吃；遂按拍板**先上 fuxi 同款 `21H461` iPad donor**
> （iPad7,1 / 17.7.11 / j120ap / t8011，经典 arm64 kernelcache，机制全实证，
> 同代配对修复照常生效）；XS/fileset-KC 重绑定器为后续主题。20H392 与
> 21H16 的资产与记录都保留作回溯。
>
> **2026-10-23 路线重置（用户拍板）**：B2（复用 16K ios-sweet 内核 + 现有
> iPhone X 16.7.16 root）废弃——旧 sweet 路线实机跑不通，不再复用旧内核与旧
> 镜像；旧阶段只带**经验与方法**。现方向 = **原生 UniXNU 设备移植**（对齐
> 上游 fuxi/sheng 模式：UniXNU 4K 内核 + IPSW root + unixnu 设备仓 +
> Mu-Silicium UEFI）。B2 评估与启动链已归档到 `docs/archive/`；`legacy/`
> 已从 git 移除（历史可查）。
>
> 设备：魅族 20 Classic = **m2381**，SM8550（kalama MTP 参考布局），16GB LPDDR5X，
> 1080×2400 BOE AMOLED（cmd-mode DSC），UFS 4.0 512GB。
>
> **上游（跟随）**：基座 = `UniXNU/UniXNU`（remote `upstream`，pin `16e592a`；
> `m2381` 分支 = 上游 main + 工作区提交，同步 = `git rebase upstream/main`，
> 详见 `docs/UPSTREAM.md`）。移植指南 = 上游树 `docs/porting/` 五篇；
> 设备模板 = `reference/unixnu-fuxi` + `reference/uefi-fuxi`。

## 0. 一句话

把 m2381 做成**上游原生的 UniXNU 移植**：`unixnu-m2381` 设备仓（board.toml
驱动 unixnu 构建树，从 **iPad7,1 / iOS 17.7.11 `21H461`** 取内核与 OS 件；
fuxi/nx729j 同款 donor（同代配对历程见顶部换配说明））+ `m2381Pkg`（Mu-Silicium
UEFI）。boot_b 放 UEFI 固件；payload 载体受本机 ABL 实证约束（gunzip
≤ ~83.5MB、忽略 boot ramdisk），**已实证走 recovery_a 的 100MiB ESP**
（r26 起每轮现役）；userdata 由 IPSW 件重建为 iOS root（数据豁免已给，
见红线 2）。

## 1. 为什么这条路成立

- **同 SoC 原生先例**：fuxi（Xiaomi 13）= SM8550，是上游原生移植（4K 内核 +
  NativeUfs/NativeAdreno/NativeMetal 等），设备仓与 UEFI 包都有现成母版；
  sheng（SM8550 平板）为第二参照；umi/pandora/vermeer/nuwa 等社区移植可查。
- **机制全在上游树里**：`unixnu` 构建驱动 + `docs/porting/`（bring-up /
  device-repository / device-tree / uefi-firmware）+ `patches/xnu` + 全套
  driver 子模块；分支挂在上游 main 之下，rebase 即可吃到 GPU 驱动等大更新。
- **m2381 硬件实证已在手**：stock FDT/DTS（`docs/hardware/`）、分区布局、
  OEM UEFI DXE blobs（`artifacts/vendor/`）、ABL 行为 RE 结论（§5、
  `docs/HARDWARE-m2381.md`）——这是旧阶段留下的核心复用资产。

## 2. 目标布局（草案，读完 docs/porting/ 后修正）

```
boot_b     = Mu-Silicium UEFI（BootShim + FV，gzip 后 ~3MB 级）
recovery_a = 100MiB FAT32 ESP：UniXNU.efi / kernel / ADT / trust cache /
             data seed 等 payload（m2381 ABL 装不下 fuxi 式 192MiB 内嵌
             payload；ESP 前提 = FV 集成 OEM UFSDxe，见 §3）
userdata   = IPSW 重建的 iOS root（OS + Cryptex1,SystemOS 窗口，走
             board.toml [userdata] LBA 流程；LBA 必须从 m2381 实机 GPT 读）
```

- ~~若 UFSDxe 集成后 Mu 仍读不到 recovery_a，退路：payload 压进 ABL 83.5MB
  限内、或上游若有 UFS 直读机制则改走之~~ **已实证无恙**：M2381ESP 每轮
  正常挂载读通，退路未启用。
- 旧 16K 内核 / iPhone X 16.7.16 root 与本路线**无关**，仅作为回滚终点的
  设备现状存在。

## 3. 需要新建/派生的东西

1. **`devices/m2381/unixnu-m2381`（设备仓）**：从 `reference/unixnu-fuxi`
   派生，结构按上游 `docs/porting/device-repository.md`。
   - `board.toml`：SoC SM8550；`[ipsw]` = **iPad7,1 / 17.7.11 / 21H461 /
     j120ap**（fuxi/nx729j 同款 donor，2026-10-24 拍板；sha256 到货复验后
     与 fuxi 值对拍）；`[userdata]` first/last_lba
     **已实测**（实机 GPT 三源对拍，值在设备仓 `board.toml`），不许抄 fuxi；`[kernel]` defconfig 从
     `fuxi_defconfig` 派生再改板级差异。
2. **`devices/m2381/uefi/m2381Pkg`（UEFI）**：从
   `reference/uefi-fuxi/Platforms/Xiaomi/fuxiPkg` 派生，按
   `docs/porting/uefi-firmware.md`。
   - FD `0xA7100000+0x300000`；SimpleFbDxe 在 ABL 帧缓冲 `0xb8000000`；
     按键 PMK8550 PON（键位/SPMI SID 按 m2381 校）；高段 DRAM
     （`0x8c0000000` 起）加入 GCD（fuxi 版默认只加 4GB 以下）；FV 按 Binary
     模块加 OEM `UFSDxe` + `DiskIo`/`Partition`/`Fat`（`artifacts/vendor/`）。
3. **ADT/DeviceTree**：按 `docs/porting/device-tree.md` 走上游流程；
   `tools/adt.py`、`tools/mkadt.py` 留作参考（其 d22ap 输入已无用）。

## 4. 红线（继承 + 更新）

1. 只碰 **boot_b + recovery_a + userdata**；xbl/abl/tz/modem/dtbo/super/
   persist 永不碰。
2. **userdata 数据豁免（用户 2026-10-23）**：无须保留设备数据，随时可刷；
   现 userdata = nura root。每次实际写入仍按 `docs/FLASHING.md`（读回、
   对拍、可回退优先）。
3. **fastboot 单通道**：本仓与 nura 侧任何刷写互斥；刷前 `getvar
   current-slot` 必须 = b。
4. **dd 读回才算证据**；刷写时长不算。每轮刷完读回 sha 对拍。
5. **fastboot boot 是死路**（魅族 ABL 裁 CmdBoot），只能 `flash`。
6. **回滚件不在本仓**（2026-10-26 用户拍板删除本仓 `artifacts/images/` 旧
   回滚件与已刷参考件）：回滚链只在 nura 仓
   `/home/ri/work/nura-meizu-m2381/artifacts/`（mu-r62…r91、esp v46+ 全套）。
7. Apple IPSW/件只本地使用，不入仓不外发。
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
| ABL | v4；gunzip 到 `0x80080000`；解压上限约 83.5MB；忽略 boot.img ramdisk；校验 image_size / appended DTB；无 CmdBoot |
| UEFI 窗口 | FD `0xA7100000+0x300000`；内核窗 `0xA8000000+0x10000000`；Display `0xB8000000+0x2B00000`；bootloader log `0xA7605000+0x8000` |

更细的地址/证据见 `docs/HARDWARE-m2381.md` 与 `docs/hardware/` 下的
stock FDT / nura DTS。

## 6. 目录

```
AGENTS.md                 本文件
README.md                 入口
docs/PLAN-unixnu.md       当前路线（原生移植）计划
docs/porting-notes.md     参照设备清单、实测值、取证方法（P0）
docs/m2381-bringup.md     m2381 施工骨架：值源、构建/刷写序列、阶段表（P0 产出）
docs/STATE.md             当前状态、资产、下一步、开放问题
docs/UPSTREAM.md          上游 pin、rebase 同步流程、remote 策略
docs/FLASHING.md          刷写/读回/回滚纪律
docs/CI.md                云端构建说明（当前无本仓 workflow）
docs/HARDWARE-m2381.md    硬件常量与地址
docs/hardware/            stock FDT/DTS、DTB（只读参考）
docs/archive/             旧 sweet 笔记、B2 评估与启动链、STATE 轮次史（§0.0–0.34）、完工计划（史料）
reference/                UniXNU/uefi-fuxi/unixnu-fuxi 等克隆（gitignore）
research/                 IPSW 等本地大件（gitignore；旧 iPhone X IPSW 已无用）
tools/                    adt.py / mkadt.py / remotezip.py（留作参考）
artifacts/                本地回滚件 / 死库存 / vendor blobs（gitignore）
devices/m2381/            unixnu-m2381 设备仓 + m2381Pkg 落地处
```

## 7. 下一步（2026-10-27 r55 组件完成；工作单 = `docs/STATE.md` §7）

1. ~~上游同步/调研/拍板~~ ✅ r51；~~=y 施工~~ ✅ r51（d1c0645）；
   ~~r52 读窗定谳~~ ✅（=y 一级全过，BannerKit 抛点钉死）；~~r53~~ ✅
   （fixup 11 生效 applied=6，SB 推进 BLSHBacklightHost brk 新墙）。
2. **r55 待刷（用户实测；设备现态 = iOS 链 r54，未刷回 nura）**：
   payload `fb2c5dd1` = r54 fixup 12（BLSHBacklightHost 入口返 nil，
   裸件 1ef5eb63）+ **forensic 内置换证 reader**（菜单 "m2381 forensic
   log reader"，type=linux，FLASHING §7）。**只刷 recovery_a**（iOS
   链未被 nura 覆盖；三件全刷规矩只在被 nura 覆盖后触发）。判读 =
   applied=7 + BLSHBacklightHost 换墙 + reader 首验。
3. **读窗纪律（r53 教训入法）**：SHLOG 窗跨引导不清零，parser 已守
   header 计数契约（slots_written 截断）；判读前先看 header rounds。
   读窗走 forensic reader（USB ACM）；nura-min 只作 reader 失败回退
   （届时只刷 recovery_a=nura ESP 即可，boot_b 不动）。
4. **合规差距报告**：`docs/PORTING-GAP-REVIEW.md`（对照官方 porting
   全 19 页；行动项 = sheng-autoreset 试投 / AdvancedLogDxe / records
   页 / bring-up 检查点表）。
5. **独立主题（不阻塞）**：chronod sig5 崩溃循环（次优先）；XS/fileset-KC
   重绑定器（换代 fileset-KC 情形转正）；arm64e 专项；GPU/触摸主题化
   （=y 显示破墙后 ADRENO=y 叠加）。

> **P1 关键定案**（细节见 `docs/archive/STATE-history.md` §1）：P2 基座 = shengPkg 骨架 + mars blob
> （fuxiPkg 是半程态不可当母版）；payload = recovery_a ESP（85MB gunzip 窗
> 装不下 192MiB ramdisk 载荷，量化定死）；kernel 步只能 macOS（xcodebuild），
> Linux 侧全链路已跑通；unique-chip-id = `3645ad55d47e77c1`（软件种子）。

> **禁止**再把 ios-sweet 的 84 补丁树、旧 shim、16K 内核当施工面；需要旧
> 结论时查 `docs/archive/`（含 `PLAN-unixnu-b2-era.md`、`BOOT-CHAIN-b2.md`、
> `NOTES-m2381.md`）。
