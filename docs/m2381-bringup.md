# m2381 施工骨架（P0 产出，2026-10-23；P1 会话修订同日）

> **历史定位（2026-10-27 复核标注）**：本文 = P0/P1 时代的施工骨架记录，
> 阶段表（§5）与取证手段引用仍有效；但文中**所有 donor 相关税值已被取代
> 两次**（20H392 → 21H16 → 现役 **iPad7,1 / 21H461**）：§2 `[ipsw]` 行、
> §4 的 `ipsw fetch` 路径、AMFI 值（现役 = `0xfffffff005ab4ae0`）均以
> 设备仓 `board.toml` / `docs/STATE.md` 为准，勿按本文执行。平台级结论
> （shengPkg 骨架 + mars blob、ESP 载荷窗口算术、无 CmdBoot）全部成立。

> 输入：上游 `docs/porting/` 五篇 + `docs/devices/fuxi.md` + nx729j 三份
> playbook（`reference/UniXNU-nx729j/`）。本文只记 m2381 的结论与差异，通用
> 流程不重复，出处见上。
>
> **P1 修订（2026-10-23 晚）**：① P2 基座改为 shengPkg 骨架 + mars blob
> （§0/§3，评估记录见 `docs/archive/STATE-history.md` §1/§5）；② 内核步只能在 macOS（xcodebuild
> + iphoneos SDK，本机实测），Linux 侧到该缺口为止全链路已跑通；③ payload
> 走 ESP 的依据从"忽略 ramdisk"升级为窗口算术（~85MB 装不下 192MiB ramdisk
> 载荷，RdLoad 负值必拒载）。

## 0. 结论

- **最重的两块上游已做完**：SM8550 的 XNU 驱动链（sheng 实机全通过：UFS/
  DPU/GMU/Adreno740/PMGR）+ ESP 载荷形态（`shengPkg`/`nx729jPkg` 的
  `<BOARD>ESP` FAT 卷就是上游标准做法，boot.img 只装 BootShim+FV）——
  m2381 的 ABL 约束恰好被这个形态天然满足，且是**硬数学**：gunzip 窗
  [0x80080000, 0x85580000) 共 ~85MB，fuxi/sheng 式 192MiB ramdisk 载荷
  在此窗内 RdLoad 为负必拒载（BOOT-CHAIN-RE §5.3），ESP 路线无悬念。
- **m2381 真正要做的四件事**（P1 修订基座后）：
  | # | 事项 | 量 |
  |---|---|---|
  | 1 | UEFI `m2381Pkg`：**shengPkg 骨架派生**（UniXNU 全链在 FV 的唯一同 SoC 模板；fuxiPkg 是半程态不能当母版）+ **Device-Binaries `@036ba9f7` mars blob**（nura 现役链实证的 UFSDxe/时钟/RPMh 胶水；注意 mars ≠ infiniti）+ nura m2381Pkg 常量（MemoryMapLib/m2381.toml/FdtBlob） | 中 |
  | 2 | 设备仓 delta：board.toml / defconfig / make-m2381-adt.py / check-m2381.py / package/unixnu.toml | 小（**P1 已完成**） |
  | 3 | Goodix GT9916 触摸驱动（与 nx729j 同款，但其为 SPI、m2381 为 I2C4@0x5d，传输层要改） | 大（后置，P4+） |
  | 4 | 内核 CI：设备仓推独立库 → macos.yml（kernel 步 = xcodebuild，只能 macOS）；prelink 三值已随 21H16 换配本机 derive（2026-10-24） | 中（等推送拍板） |
- **m2381 特有红线**：魅族 ABL 无 `CmdBoot` ⇒ **没有 nx729j 那条 "fastboot
  boot 临时启动" 保命通道**。首刷前核对 nura 侧镜像版本：全套在
  `/home/ri/work/nura-meizu-m2381/artifacts/`（mu-r62…r91、esp-recovery
  v46+），与实测现役 sha（boot_b `af574775…` / recovery_a `0a102220…`）
  对拍；启动链逆向全集 = nura `docs/BOOT-CHAIN-RE.md`（§3.4 m2381Pkg
  终形态、§5 ABL 约束、§6.3 ramoops 0xa5100000）。

## 1. 落地方式（照 nx729j 方式一：fork + delta 覆盖）

```
devices/m2381/
  unixnu-m2381/            # fork reference/unixnu-fuxi（同为 fuxiPkg 系）再覆盖：
    board.toml             # 值源见 §2
    configs/m2381_defconfig # 全 SoC 级开关，先与 sheng/fuxi 相同
    scripts/make-m2381-adt.py   # 从 make-nx729j-adt.py 改（身份+面板几何）
    scripts/check-m2381.py      # 预检：board.toml 一致性 + UEFI 内存映射 vs DTB 保留区
    scripts/collect-m2381.py    # 实机采集（devsh 路线，替代 nx729j 的 adb）
    package/unixnu.toml         # 菜单
  uefi/m2381Pkg/           # 从 reference/uefi-fuxi 的 fuxiPkg 派生
```

nx729j 经验：`sheng-` 前缀内核参数（`sheng-pmgr=2`、`sheng-touch=4`、
`sheng-gpu-*`）**保持原名**——解析代码在 sheng 套件里，与机型名无关。

## 2. board.toml 值源清单

| 段 | 值 | 源 |
|---|---|---|
| `[board]` | name=m2381, soc=SM8550, kit=m2381 | 定 |
| `[ipsw]` | iPhone11,2（XS）/ 17.7 / 21H16 / d321ap（2026-10-24 换配，同代配对）；sha `6fd757be…`（本地已核对） | 定；chip_id 0x8020 / board_id 0x0e 已回填 |
| `[userdata]` | first_lba=2330472, last_lba=123871226, lba_bytes=4096 | [DEVICE] 2026-10-23 devsh 实测 |
| `[firmware]` | config=m2381；dsc=`Platforms/Meizu/m2381Pkg/m2381.dsc`；fd=…/SILICIUM_UEFI.fd | 定；FD 窗口 0xA7100000+0x300000 [DEVICE] |
| `[bootimg.image]` | header_version=4（Flyme v4）、base=0、kernel_offset=0x8000、pagesize=4096 | 旧 sweet 实证；append_dtb 按 stock 要求核 [TODO] |
| `[payload]` | volume=M2381ESP（≤11 字符）、loader=firmware、dram_size=0x400000000（16GB，同 nx729j）、panic_screen=true、timeout=3 | 定 |
| `[payload] kernel_args` | 初版：`-v serial=3 debug=0x14e boot-progress=1` + sheng 系参数原样 | 定（初版） |
| `[payload.adt]` | tool=make-m2381-adt.py；unique_chip_id=required；model/compatible=`meizu,m2381`；artwork-device-subtype=2400；artwork-scale-factor=3（1080/3=360pt） | [DTB]/推算 |
| `[[bootimg.dtb_node]]` | 预计**不需要**追加 splash_region：stock FDT 已有 cont_splash_region 0xb8000000+0x2b00000（与 nx729j 同款判断）；以预检 vs DTB 比对为准 | [DTB] 待核 |

[TODO]（P1 更新）：`unique-chip-id` **已定**（软件种子 `3645ad55d47e77c1`，
设备无可暴露硬件序列号，history §1）；stock fastboot 包本地未找到，改用
**实机 GPT 直读**闭环几何（三源吻合，值在设备仓 `board.toml`），stock 包
留作以后第四源；
刷机窗口 `fastboot getvar current-slot` + `partition-size:userdata` 形式
复核仍留。另 [TODO P3] root_match_volume_uuid / OWNER_DATA_VOLUME_UUID、
已 derive（2026-10-24）：slide `-0x1000000`、AMFI early init `0xfffffff008f3bd18`（三重指纹法）；text_fixups 留待 P4。

## 3. m2381Pkg 要点（P1 修订：shengPkg 骨架 + mars blob）

1. 基座 = `reference/uefi-sheng` 的 shengPkg（UniXNUDxe/DarwinBoot/菜单全
   编进 FV，`SHENG_VENDOR_HW=1` 时加载 Binaries blob 族）；m2381Pkg =
   Platforms/Meizu/m2381Pkg overlay，板级常量从 nura fork（`reference/
   uefi-m2381-nura`）搬：MemoryMapLib（FD/Kernel 窗/Display/TZApps 全表
   实证）、m2381.toml（header v4/gzip/append_dtb=false/Gunyah WDT off）、
   FdtBlob/sm8550-meizu-20.dtb。
2. blob = Device-Binaries `@036ba9f7` 的 **mars** 集（nura 实证；
   mars/infiniti 的 UFSDxe.efi 哈希不同，不可混用）；本机 OEM blobs
   （`artifacts/vendor/`）为备选。sheng blob 公开仓没有（只有 nuwa）。
3. FD `0xA7100000+0x300000`（与 sheng 同窗）；`requires_kernel_header=true`
   + `disable_gunyah_watchdog=true`。**FD 保持 3MB**：FD>15MB 死因是
   BootShim 拷贝跨 0xA8000000 栅栏（BOOT-CHAIN-RE §5.7），不扩。
4. SimpleFbDxe 在 ABL 帧缓冲 `0xb8000000`（1080×2400 stride 4320）。
5. 按键 PMK8550 PON（pwrkey+resin），SID/键码与 sheng 对比后定；出 FV 后
   grep 复核 FV 内无 KEP/Winload（nura 旧雷，现源码已无）。

## 4. 构建与刷写序列

```bash
# Linux 本机（P1 全部实测通过，到 xcodebuild 缺口为止）：
./unixnu defconfig m2381_defconfig -D devices/m2381/unixnu-m2381
./unixnu fetch
./unixnu ipsw fetch --ipsw research/ipsw/iPhone11,2,iPhone11,4,iPhone11,6_17.7_21H16_Restore.ipsw
#   Linux 无 /usr/bin/compression_tool：kernelcache 预置
#   out/ipsw/21H16/…/kernelcache.release.…（research/bin/ipsw kernel dec
#   产出，sha 须 = board.toml pin）即跳过解码。

# kernel 步 = macOS 专用（xcodebuild + iphoneos SDK，本机实测卡点）：
#   GitHub Actions macos.yml dispatch：device_repository=<m2381 设备仓>，
#   device_ref=main，defconfig=m2381_defconfig，steps=kernel。
#   前提：设备仓独立成库推送（用户拍板）+ UNIXNU_UNIQUE_CHIP_ID secret
#   （= 3645ad55d47e77c1）。CI 产 kernel.macho 拷回本机再走后续。

./unixnu build package           # Linux
./unixnu build firmware --uefi <uefi-sheng+m2381Pkg overlay>   # P2 门槛：Linux 出 FV
./unixnu build payload --unique-chip-id 3645ad55d47e77c1 --trust-cache <bundle> --data-ramdisk <seed>
./unixnu bootimg --uefi <同上> --dtb <m2381.dtb>
python3 scripts/check-m2381.py --uefi <同上>   # 预检挡 TODO 值与不一致
```

刷写（与 nx729j 的差异点）：

- **无 `fastboot boot`**。首刷顺序 = 备份 nura → `flash recovery_a`（ESP，
  不影响 nura 启动）→ `flash boot_b`（固件，此步起 nura 不可启动，直到回滚
  或 iOS 成功）→ 每步 dd 读回对拍。
- userdata 镜像（`./unixnu build userdata` → sparse）在四门过门后单独刷，
  豁免已给、流程照 `docs/FLASHING.md`。

## 5. bring-up 阶段表（上游七阶段 × m2381 取证）

| 阶段 | 判据 | 卡住先看 |
|---|---|---|
| 1 固件 | 串口出 UEFI 输出（无 fastboot boot，直接看刷入结果） | FD 窗口、BootShim 头、ABL 是否收 v4 镜像 |
| 2 菜单 | `UNIXNU:` 行 + GOP 画面 | 帧缓冲基址/stride、UFS DXE 是否枚举到 ESP |
| 3 payload | 读到 `M2381ESP` + `unixnu.toml` | FAT 标签 ≤11 字符、分区路径 |
| 4 DarwinBoot | `DARWINBOOT:` 行、EL1、fb 参数 | ADT sidecar、trust cache、memory-map 占位 |
| 5 内核入口 | XNU 版本行/颜色条/panic 屏 | boot args、ADT 六大节点、NVRAM 代理 |
| 6 存储+root | UFS 链速日志、OS/SystemOS 窗口、root UUID 匹配 | `root_match_volume_uuid`、窗口偏移、容量 |
| 7 userland | launchd → Setup/SpringBoard | skip-setup 工具、Data 卷、`boot-progress=2` |

每阶段的取证手段见 `docs/porting-notes.md` §5（串口/屏幕/DT panic region）。

## 6. 风险表

| 风险 | 等级 | 对策 |
|---|---|---|
| iPhone 镜像+SM8550 无实机先例 | 中 | 上游官方第二目标（requirements.md）；negroni 证 userland、sheng 证 SoC 链；不行退 nuwa 源码内核路线 |
| ABL 85MB 窗 / 无 CmdBoot | 已消解 | 窗口算术定死 ESP 载荷（192MiB ramdisk 必拒载）；刷写纪律替代临时启动 |
| 触摸 GT9916 I2C 驱动开发 | 大（后置） | nx729j `drivers/NativeGoodixTouch` 规格起步；P4 前 UI 用按键+串口 |
| 内核构建主机 | 已定案 | kernel 步只能 macOS（xcodebuild，P1 实测）；CI = macos.yml，设备仓推送等拍板 |
| arm64e donor（21H16）上机风险 | 中 | NativePrelink 已放宽 subtype 门 + 源段包含解析（kit 26 测试绿 + 真件干跑）；PAC 指针语义与 prelink 报告 = CI/上机门终审 |
| 面板 cmd-mode DSC 初始化 | 中 | stock FDT 有完整 init 序列，照 `build-native-legacy-display.py` 输入格式转换（nx729j 同款待办） |

## 7. 立即行动（P1 已完成 2026-10-23；下一步 = `docs/STATE.md` §7 工作单）

1. ~~消化 nura 资产~~ **完成**（现役镜像对拍 ✓、P2 基座评估 → shengPkg+
   mars、ABL 约束逐条对回源文档并新增 KEP/栅栏/kernel-log 三条）。
2. ~~`devices/m2381/unixnu-m2381` 骨架~~ **完成**（board.toml/defconfig/
   菜单/check-m2381.py，几何三源对拍）。
3. ~~`make-m2381-adt.py` 初版~~ **完成**（iPhone 身份 + d321ap 真值，2026-10-24 随换配更新）。
4. ~~unique-chip-id~~ **已定**（软件种子 `3645ad55d47e77c1`）；stock 包
   改由实机 GPT 直读闭环。
5. ~~构建环境实测~~ **完成**：Linux 到 xcodebuild 缺口全绿；kernel = CI。
