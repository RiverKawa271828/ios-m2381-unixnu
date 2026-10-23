# m2381 施工骨架（P0 产出，2026-10-23）

> 输入：上游 `docs/porting/` 五篇 + `docs/devices/fuxi.md` + nx729j 三份
> playbook（`reference/UniXNU-nx729j/`）。本文只记 m2381 的结论与差异，通用
> 流程不重复，出处见上。

## 0. 结论

- **最重的两块上游已做完**：SM8550 的 XNU 驱动链（sheng 实机全通过：UFS/
  DPU/GMU/Adreno740/PMGR）+ ESP 载荷形态（`shengPkg`/`nx729jPkg` 的
  `<BOARD>ESP` FAT 卷就是上游标准做法，boot.img 只装 BootShim+FV）——
  m2381 的 ABL 约束（83.5MB、忽略 ramdisk）恰好被这个形态天然满足。
- **m2381 真正要做的四件事**：
  | # | 事项 | 量 |
  |---|---|---|
  | 1 | UEFI `m2381Pkg`：fuxiPkg 派生 + **从 uefi-sheng 的 shengPkg/nx729jPkg 移植 UFS 源码 DXE**（同 SoC；OEM UFSDxe blob 降为备选） | 中 |
  | 2 | 设备仓 delta：board.toml / defconfig / make-m2381-adt.py / check-m2381.py / package/unixnu.toml | 小 |
  | 3 | Goodix GT9916 触摸驱动（与 nx729j 同款，但其为 SPI、m2381 为 I2C4@0x5d，传输层要改） | 大（后置，P4+） |
  | 4 | 实测补齐：unique-chip-id、stock fastboot 包、`fastboot getvar` 复核 | 小 |
- **m2381 特有红线**：魅族 ABL 无 `CmdBoot` ⇒ **没有 nx729j 那条 "fastboot
  boot 临时启动" 保命通道**。首刷前必须先用 devpull 把 nura 的
  boot_b/recovery_a 整分区备份到本地 `backups/`（现在只有 sha 没有镜像）。

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
| `[ipsw]` | iPhone10,3 / 16.7.16 / 20H392 / d22ap；sha `ece1194f…`（本地 `research/ipsw/` 已就位并核对） | 定；chip_id/board_id 从 20H392 BuildManifest 读 [TODO] |
| `[userdata]` | first_lba=2330472, last_lba=123871226, lba_bytes=4096 | [DEVICE] 2026-10-23 devsh 实测 |
| `[firmware]` | config=m2381；dsc=`Platforms/Meizu/m2381Pkg/m2381.dsc`；fd=…/SILICIUM_UEFI.fd | 定；FD 窗口 0xA7100000+0x300000 [DEVICE] |
| `[bootimg.image]` | header_version=4（Flyme v4）、base=0、kernel_offset=0x8000、pagesize=4096 | 旧 sweet 实证；append_dtb 按 stock 要求核 [TODO] |
| `[payload]` | volume=M2381ESP（≤11 字符）、loader=firmware、dram_size=0x400000000（16GB，同 nx729j）、panic_screen=true、timeout=3 | 定 |
| `[payload] kernel_args` | 初版：`-v serial=3 debug=0x14e boot-progress=1` + sheng 系参数原样 | 定（初版） |
| `[payload.adt]` | tool=make-m2381-adt.py；unique_chip_id=required；model/compatible=`meizu,m2381`；artwork-device-subtype=2400；artwork-scale-factor=3（1080/3=360pt） | [DTB]/推算 |
| `[[bootimg.dtb_node]]` | 预计**不需要**追加 splash_region：stock FDT 已有 cont_splash_region 0xb8000000+0x2b00000（与 nx729j 同款判断）；以预检 vs DTB 比对为准 | [DTB] 待核 |

[TODO] 测量项：`unique-chip-id`（nura OS 下找读取路径）、stock fastboot 包
（gpt_main0/rawprogram 佐证）、刷机窗口 `fastboot getvar current-slot` +
`partition-size:userdata` 三源复核。

## 3. m2381Pkg 要点

1. FD `0xA7100000+0x300000`（ABL 拷入窗，旧实证）；`boot_shim.requires_kernel_header=true`。
2. SimpleFbDxe 在 ABL 帧缓冲 `0xb8000000`（1080×2400 stride 4320）。
3. **UFS 源码移植**：从 `reference/uefi-sheng` 的 shengPkg/nx729jPkg 拿 UFS
   DXE（控制器同为 `0x1d84000`）；OEM `UFSDxe` blobs（`artifacts/vendor/`）
   只作源码路线卡死时的备选。
4. `FuxiBootMemoryDxe` 加高段 DRAM `0x8c0000000` 起（fuxi 版默认只加 4GB 下）。
5. 按键 PMK8550 PON（pwrkey+resin），SID/键码与 fuxi 对比后定。

## 4. 构建与刷写序列

```bash
./unixnu defconfig m2381_defconfig -D devices/m2381/unixnu-m2381
./unixnu fetch
./unixnu ipsw fetch --ipsw research/ipsw/iPhone10,3,iPhone10,6_16.7.16_20H392_Restore.ipsw
./unixnu build kernel            # Linux 可跑（nx729j §5.4）；卡住再评估 CI/macOS
./unixnu build package
./unixnu build firmware --uefi devices/m2381/uefi
./unixnu build payload --unique-chip-id <hex> --trust-cache <bundle> --data-ramdisk <seed>
./unixnu bootimg --uefi devices/m2381/uefi --dtb <m2381.dtb>
python3 scripts/check-m2381.py --uefi devices/m2381/uefi   # 预检挡两类致命错
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
| ABL 83.5MB / 无 CmdBoot | 已消解 | ESP 载荷形态天然合规；刷写纪律替代临时启动 |
| 触摸 GT9916 I2C 驱动开发 | 大（后置） | nx729j `drivers/NativeGoodixTouch` 规格起步；P4 前 UI 用按键+串口 |
| 内核构建主机不确定 | 小 | nx729j 明确"Linux 亦可"；P1 实测 |
| 面板 cmd-mode DSC 初始化 | 中 | stock FDT 有完整 init 序列，照 `build-native-legacy-display.py` 输入格式转换（nx729j 同款待办） |

## 7. 立即行动（P1 开工顺序）

1. **备份 nura**：devpull boot_b/recovery_a 整分区 → `backups/`（只有 sha
   没有镜像，首刷前必须补）。
2. 起 `devices/m2381/unixnu-m2381` 骨架（fork fuxi + §1 delta 文件）。
3. `make-m2381-adt.py` 初版（从 nx729j 改身份/面板/DRAM）。
4. nura OS 下找 `unique-chip-id` 读取路径；收集 stock fastboot 包。
5. 构建环境实测：Linux 跑通 `build kernel`。
