# m2381 硬件常量（SM8550 / 魅族 20 Classic）

> 来源：stock FDT（`docs/hardware/stock-fdt.dts`，`model = "Meizu M2381(kalama MTP)"`）、
> nura 主线 DTS（`docs/hardware/sm8550-meizu-20.dts`）、r34 实测记录
> （`docs/archive/NOTES-m2381.md`）、fuxi 的 UEFI 内存图（`reference/uefi-fuxi`）。
> 只列 B2 会用到的值；更细的寄存器/面板序列查档案。

## 1. SoC / 基本

| 项 | 值 |
|---|---|
| SoC | Qualcomm SM8550（kalama），8 核：1×X3 + 2×A715 + 2×A710 + 3×A510 |
| 内存 | 16GB LPDDR5X（8 段 /memory，约 15.7GB 可用） |
| 存储 | UFS 4.0 512GB，Samsung KLUFG8RHHD-B0G1 |
| 显示 | 1080×2400 BOE AMOLED，cmd-mode DSC；stock `qcom,mdss_dsi_meizu_amoled_boe_cmd`（dis_id 4DB501） |
| 触摸 | Goodix GT9916（berlin 族），GENI I²C |
| 按键 | PMK8550 PON（pwrkey + resin/vol-down） |
| 充电/PMIC | PMK8550 + SPMI（B2 暂不碰） |

## 2. 中断 / 定时器

| 项 | 值 |
|---|---|
| GICD | `0x17100000`，size `0x10000` |
| GICR | `0x17180000`，PE stride `0x40000`，区大小 `0x200000`（必须覆盖 8 核；旧版只给 0x100000 会挂副核） |
| VTIMER | INTID **27**（PPI11）；PTIMER 28（PPI12） |
| GIC 安全态 | `GICD_CTLR.DS=1`（单安全态） |
| SGI 页 | 写 SGI 优先级前先置 `GICD_CTLR.bit8`（nASSGIreq）；写后 RWP |
| 时间基 | 19,200,000 Hz（ADT `/cpus/timebase-frequency`） |

## 3. 显示

| 项 | 值 |
|---|---|
| ABL 帧缓冲 | `0xb8000000`，size `0x2b00000`（43MiB），32bpp，stride `4320` |
| 面板 | 1080×2400，cmd-mode BOE AMOLED，DSC |
| UEFI | `SimpleFbDxe` 直接在该帧缓冲上发布 GOP；不跑 vendor DisplayDxe |
| Kernel | 旧 r34 的 `BOARD_FB_BASE = 0xb8000000`；AppleCLCD CPU copy 路线 |

> nura 的 splash_region 与 fuxi 的 Display Reserved 完全同址；ABL 已把面板点亮，
> 所以 B2 的 UEFI 菜单不需要自己初始化 DSI/panel。

## 4. 存储 / UFS

| 项 | 值 |
|---|---|
| UFS 控制器 | `0x1d84000`（UFSHCI 4.0），size `0x3000` |
| UFS PHY | `0x1d80000` |
| ICE | `0x1d88000`（B2 不用） |
| 设备 | Samsung KLUFG8RHHD-B0G1，512GB，UFS 4.0 |
| 策略 | 沿用 ABL 已拉起的链路；内核 ios-sweet UFS 驱动直接挂 `rd=ufsroot0` |

## 5. 触摸 / 按键（B2 先不驱动，记录备阶段 3）

| 项 | 值 |
|---|---|
| 触摸控制器 | Goodix GT9916（berlin 族） |
| 总线 | GENI I²C4 `0xa90000`，400kHz |
| 地址 | `0x5d` |
| IRQ | tlmm25，edge-falling |
| Reset | tlmm24，active-low |
| 电源 | avdd `l14b_3p2`，vddio `l12b_1p8` |
| 分辨率 | 1080×2400 |
| 按键 | PMK8550 PON：`pon_pwrkey`（power）、`pon_resin`（vol-down） |

## 6. 内存图（关键窗口）

### 6.1 /memory（stock FDT 与 nura DTS 同）

```
0x0000000080000000  0x0000000000e00000   (14MiB)
0x00000000811d0000  0x0000000056e30000   (~1.39GiB)
0x00000000d8140000  0x0000000000020000
0x00000000d8800000  0x0000000000000000
0x00000000e3bb0000  0x000000001c450000   (~453MiB)
0x0000000880000000  0x0000000038700000   (903MiB high)
0x00000008c0000000  0x0000000140000000   (5GiB high)
0x0000000a00000000  0x0000000200000000   (8GiB high)
```

- **13GiB 连续高段**：`0x8c0000000` … `0xc0000000`（5GiB + 8GiB 首尾相接）。
- DarwinBoot 窗口最大 8GiB；B2 取 **3–4GiB**，基址 `0x8c0000000`。
- 低 4GB 内有大量 carve-out；不要试图在低 4GB 找 3GiB 连续窗。

### 6.2 Reserved（stock FDT，节选）

| 节点 | 地址/大小 |
|---|---|
| gunyah | `0x80000000+0xa00000`（no-map） |
| xbl_dt_log | `0x81a00000+0x260000` |
| aop_cmd_db / aop_config | `0x81c60000+0x20000` / `0x81c80000+0x74000` |
| chipinfo | `0x81cf4000+0x1000` |
| smem | `0x81d00000+0x200000` |
| mpss（PIL） | `0x89800000` 起（大块） |
| ramoops（stock） | `0xa5100000+0x200000`（在 Demura 区，每 boot 被覆盖；nura 改到 `0xa8000000+0x200000`） |
| mzversion | `0xa6000000+0x1000` |
| bootloader log | `0xa7605000+0x8000`（与 fuxi Log Buffer 同址） |
| splash_region | `0xb8000000+0x2b00000` |
| disp_rdump | `0xb8000000+0x800000` |

### 6.3 UEFI 参考布局（fuxi / SM8550 平台惯例）

| 区域 | 地址/大小 |
|---|---|
| UEFI FD 窗口 | `0xA7000000+0x100000`（reserved） |
| UEFI FD | `0xA7100000+0x300000`（3MiB） |
| FD reserved | `0xA7400000+0x200000` |
| UEFI stack | `0xA760D000+0x40000` |
| FV region | `0xA7AD9000+0x400000` |
| Kernel 窗 | `0xA8000000+0x10000000`（256MiB） |
| Display Reserved | `0xB8000000+0x02B00000` |

> 这些值与 m2381 stock FDT 的 bootloader log / display / Mu “Kernel” reserv 对得上；
> B2 直接用 `0xA7100000/0x300000` 作为 FD 基址/大小，风险是 ABL 是否把该 4MiB
> 窗口当 UEFI 专用；nura 的 Mu 链已在同址工作，先按此做，失败再回退。

## 7. ABL 行为（Flyme 12.6 / v4）

- boot.img **只有 kernel**；init_boot.img 是 generic ramdisk；vendor_boot.img 是
  vendor ramdisk + DTB + bootconfig。
- ABL 把 gzip kernel 解到 **`0x80080000`**；解压缓冲上限
  `DtLoad - K ≈ 0x0537E000 ≈ 83.5MB`。
- **忽略 boot.img 的 ramdisk**（本机实证：shim 只看到 stock 5.7MB initramfs）。
- arm64 header `image_size`（偏移 `0x10`）会被校验；`text_offset` 只在特殊分支用。
- 若 `[K+0x38] == ARMd`，校验 appended DTB 结束地址 ≤ DtLoad。
- 没有 3MB/size 门限；staging 窗口 `[0x80080000, ~0x85600000)` 在
  ExitBootServices 前可读。
- 结论：B2 的 boot.img = `gzip(BootShim + FV)`，约 3MB，远低于 83.5MB；
  **不要** appended DTB，**不要**依赖 boot.img ramdisk。

## 8. 分区

| 分区 | 大小 | B2 用途 |
|---|---|---|
| boot_b | 96MiB（100,663,296B） | BootShim + UEFI FV（约 3MB） |
| recovery_a | 100MiB | 100MiB FAT32 ESP（UniXNU.efi + kernel + ADT + toml） |
| userdata | 12GB | CI 重建 iOS root（fd43ee70）；尾部 32MiB = SHLOG 取证窗（只读，FLASHING §7） |

> `fastboot getvar current-slot` 必须 = b；`fastboot boot` 在本机是死路。
