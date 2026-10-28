# ios-m2381-unixnu — 魅族 20 Classic（m2381 / SM8550）iOS via UniXNU

> **2026-10-29 拆分整理（用户拍板"按上游要求来拆"+"把uefi也整进去"）**：
> ①设备仓 `devices/m2381/unixnu-m2381` 拆出独立仓
> **[UniXNU-m2381](https://github.com/RiverKawa271828/UniXNU-m2381)**（本地
> `../UniXNU-m2381`；`git subtree split` 保 93 提交全史 tip `2f06128c`；
> keys/*.pem 本地随迁仍不入仓；独立实跑 check-m2381 = 4 ok 0 fail）；
> ②m2381Pkg 收编新 UEFI 仓 **`../uefi-m2381`**（分支 main；基座 = 上游
> uefi-sheng `sheng@0eb4514` 完整克隆 + overlay 提交 `5297334`：m2381Pkg/
> BootShim/Silicon 三处板级改动/Binaries pin→`036ba9f7` mars；**实证来源 =
> nura 链**——nura 首把 Mu 跑通，unixnu 侧 Mu 树本就产自 nura 树，现役 FV
> 构建树 = 上游 uefi-sheng 基座 + overlay；旧 RiverKawa271828/Mu-Silicium
> fork = nura 期档案封存不再使用），sync.sh 与 `devices/m2381/` 整体退役；
> ③三仓收拢于
> 工作区根 `/home/ri/work/unixnu-meizu`（老主树路径留 shim 符号链接）。
> 构建 = 主树 `./unixnu -D ../UniXNU-m2381`（固件加 `--uefi ../uefi-m2381`）；
> CI dispatch 改指新仓 `device_repository=RiverKawa271828/UniXNU-m2381,
> device_ref=main`，旧 orphan `device-m2381` 冻结回退至新仓 CI 首绿。
> 历史 docs 里 `devices/m2381/…` 路径一律指新仓对应路径。
>
> **2026-10-29 donor 二次反转（用户拍板"不搞了，换ios16的版本"）**：=y 主屏
> 注册链攻坚（r78a/r78b sel6 刀两轮未落地）停车道；donor 回退
> **iPhone10,3 / iOS 16.7.16 / 20H392**（=本次换配前的原配对，资产保留
> 正是为今天）。A11/t8015 = 经典 arm64 kernelcache，**无需 fileset 重绑
> 定器**，纯配置级换道。理由：r35–r77 墙链全部是 21H461 十七代用户态墙，
> 16.7.16 配对下整体蒸发；跨代风险（entropy/Image4）回到已驯服状态
> （entropy 守卫仍在树内、绿蓝条曾全通、prelink 三值已 derive）。
> 21H461 与 =y 车道资产全数保留作回溯。施工单 = `docs/STATE.md` §8。
>
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

把 m2381 做成**上游原生的 UniXNU 移植**：设备仓 `UniXNU-m2381`（2026-10-29
起为独立仓 `../UniXNU-m2381`；board.toml 驱动 unixnu 构建树，从
**iPad7,1 / iOS 17.7.11 `21H461`** 取内核与 OS 件；
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
2. **m2381Pkg（UEFI，现居 `../uefi-m2381/Platforms/Meizu/m2381Pkg`，
   2026-10-29 收编）**：从
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
| 分区 | boot_b 96MiB；recovery_a 100MiB；userdata 12GB（现 = CI 重建 iOS root，尾部 32MiB = SHLOG 取证窗） |
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
artifacts/                本地回滚件 / 死库存 / vendor blobs（gitignore）
../UniXNU-m2381/          设备仓（2026-10-29 拆出；porting.toml 移植记录 +
                          records/ 实据 + scripts/re-tools 全套）
../uefi-m2381/            UEFI 仓（= 上游 uefi-sheng sheng@0eb4514 + overlay
                          提交；m2381Pkg 真相源；devices/m2381 已退役）
```

## 7. 下一步（**r77 已刷已判 + 战略转 =y（2026-10-29 凌晨）**）：

**四连击达成 = #30 实锤生效**（窗 078a8227 208/509 uptime=417s，
对拍 ✓）：scale 断言 0 出现 + applied 25→26；**墙链五连单调后移**
（nil-set→workspace 门→position→bounds→scale 全灭，每轮 applied
+1）；PB CRASH 133→8 暴跌；回归六项全绿。**新墙 = SB sig11 ×172
once 域毒化果**（`-[WGCarouselListViewController
_updateCarouselEffect]` 读 `___sharedInstance.onceToken+0x8` =
0x4000000000000018 毒化标记当指针；全链符号化验活 =
CFXRegistrationPost→rootFolderDidChange→newAvocadoWidgetList…→
_updateCarouselEffect——**SB 已深达今天视图 widget 列表装载**）；
果非因（r72 同款机制）→ 本轮不出 fixup。**战略拍板 = 修复继续但
换主攻**：用户态 fixup 降支路（供给族遇一堵修一堵），**主菜转 =y
main display 注册链攻坚**（displayless 是根因，换 iPhone donor 墙
同形重长，=y 接通则供给族成片消失）；止损线 = =y 攻坚 2–3 会话，
若 =y 卡死且供给族又积 5+ 堵则评估中间路线 B（保内核换 iPhone
donor 21H16，fileset-KC 重绑定器已破译待实现）。**取证双铁律照
旧**；判读纪律含修正掩码 + 阳性对照（§4.9）。工作单 =
`docs/STATE.md` §7）

## 8. 子代理协作（2026-10-29 用户拍板）

1. **主会话定谳制**：判读定谳、拍板、出件、刷写只由主会话做；子代理
   只承担可复核的采集/对拍/扫描/检索（侦察类）工作。
2. **证据三件套验收**：子代理结论必须附**原文行 + `路径:行号`**；无
   证据的结论不入档、不引用、不转述进 STATE/commit。
3. **零命中须过阳性对照**（同 §4.9）：子代理报"不存在/零引用/缓存外"
   类结论，必须用一条已知在案的阳性样本走同一管线复测；复测不过则
   该结论作废。
4. **并行分派**：相互独立的侦察线（如日志矩阵提取 / 源码对照 / 返回
   码反查）一次并行派出，主会话不串行等待；有依赖的线拆开等前序出
   结果再派。
5. **权限最小化**：侦察默认只读（Explore 类）；子代理不碰设备、不刷
   写、不改状态文件与 commit；红线 7（Apple 件不外发）对子代理同
   样生效，本地大件只给路径让它自己读。
6. **大件转述不转贴**：子代理只回结论+关键行，不回贴大文件全文；主
   会话需要时按其给的路径自取。
7. **分歧处理**：子代理结论与在案判读冲突时，先交叉核验（找原文、
   复跑管线），不直接采信也不直接丢弃；核验结果落档后才算数。
