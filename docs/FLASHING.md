# 刷写 / 读回 / 回滚（m2381）

> 只碰 **boot_b + recovery_a + userdata**。xbl/abl/tz/modem/dtbo/super/persist 永不碰。
> 任何刷写前先读回、先对拍；刷写时长不算证据，dd/读回 sha 才算。

## 0. 连接方法（2026-10-23 实测）

设备跑 nura OS 时走 **USB 网络（NCM）**，不是 adb：

```bash
ip -o -4 addr show | grep 172.16.42          # 宿主侧接口（实测 enp104s0f3u1，172.16.42.2）
N=/home/ri/work/nura-meizu-m2381/tools
python3 $N/devsh.py 'uname -a; id'           # shell（设备 172.16.42.1:23，root）
python3 $N/devpull.py <设备路径> <本地路径>    # 拉文件
python3 $N/devpush.py <本地路径> <设备路径>    # 推文件
```

- `devsh.py` 用 SO_BINDTODEVICE 绑接口，绕开宿主 TUN 策略路由。
- **重启进 bootloader 会断开 USB 网会话**；fastboot 操作前先把要读/写的
  东西整理好，刷完 `fastboot reboot` 回 nura 再继续。
- 设备侧是 root shell，读分区直接 `sha256sum /dev/disk/by-partlabel/<名>`。

## 1. 前置检查

```bash
fastboot getvar current-slot          # 必须是 b
fastboot getvar partition-size:boot_b
fastboot getvar partition-size:recovery_a
fastboot getvar partition-size:userdata
```

- 本机 boot_b = 96MiB；recovery_a = 100MiB；userdata = 463.6 GiB（旧档"12GB"
  是陈旧记录，2026-10-30 对照 board.toml `[userdata]` 的 [DEVICE] 双测几何
  纠正：sda22 = 972,326,040 × 512 B；23F84 的 userdata-root 件 13.98 GB 刷入
  无碍）。
- 同一时刻只允许一个 fastboot 通道；本仓与 nura 侧刷写互斥。
- `fastboot boot` 是死路（魅族 ABL 裁剪 CmdBoot），只能 flash。

## 2. 读回（刷后对拍）

**iOS 链现态（无 adb）= 用取证 reader 读回（r55e 实证）**：刷完重启后
菜单选 reader，shell 里定位分区并 hash（busybox sha256sum 在）：

```bash
python3 ../UniXNU-m2381/scripts/acm-reader.py cmd \
  'grep PARTNAME=recovery_a /sys/class/block/*/uevent'   # 找设备名（如 /dev/sde27）
python3 ../UniXNU-m2381/scripts/acm-reader.py cmd \
  'dd if=/dev/sde27 bs=1M count=96 2>/dev/null | sha256sum'   # == 镜像 sha 即实证
```

设备在 Flyme / nura 下且有 root 时（全分区镜像读回）：

```bash
adb shell su -c 'dd if=/dev/block/by-name/boot_b of=/data/local/tmp/boot_b.before.img'
adb pull /data/local/tmp/boot_b.before.img
adb shell su -c 'dd if=/dev/block/by-name/recovery_a of=/data/local/tmp/recovery_a.before.img'
adb pull /data/local/tmp/recovery_a.before.img
sha256sum boot_b.before.img recovery_a.before.img
```

- userdata 完整读回 = 12GB；除非要换 root/抹盘，不要拉。若要做，先确认磁盘空间
  （本机 254GB 空闲）和用户拍板。
- 读回件存 `backups/`（不入 git），文件名带日期/分区。

## 3. 刷写（iOS 链现役形态）

**常规迭代轮 = 只刷 recovery_a**（当轮 payload；boot_b/userdata 未被动）；
**三件全刷**（boot_b + recovery_a + userdata）只在 iOS 链被 nura 覆盖后
触发的恢复轮（r44 两次踩坑定的规矩），按当轮"刷写卡"执行：

- **发卡前先确认设备现态**：走过 nura-min 读回流程后 iOS 链已被覆盖，
  "只刷 recovery_a" 快捷卡只在 iOS 链未被 nura 覆盖时合法（r44 两次踩坑）。
- payload 只认**平铺路径** `out/m2381/payload-rNN-<sha8>.img`（`--out`
  双层路径歧义已在 r49 作废，旧路径文件保留作回滚件不覆盖）；
  卡上必带**刷前 sha256 门**（`sha256sum` 输出须以卡上 sha 前 8 开头）。
- 刷前 `fastboot --set-active=b` 重置 ABL 重试预算（同槽连败耗尽会自动
  落 fastboot；getvar current-slot 必须 = b）。
- 现役件（2026-10-28 r62 刷写后，sha 前 8）：boot_b = `289614ba`
  （无SB 版，ENABLE_SECUREBOOT=0）+ recovery_a = **r62 `0854ea5e`
  （已刷：set_active b / sha 门 / flash OKAY 实录；读回对拍待补——
  下会话 reader 首 act）** + userdata =
  `fd43ee70`
  （out/m2381/ci-artifacts/userdata-root.img）。现势详见
  `docs/STATE.md` §2。
- 每槽刷完立即读回 sha 对拍（§2；iOS 链态走 reader）；刷完
  `fastboot reboot`（`fastboot boot` 死路），让 ABL 走 boot_b。

## 4. 回滚

> **回滚件唯一所在 = nura 仓 `/home/ri/work/nura-meizu-m2381/artifacts/`**
> （mu-r62…r91、esp v46+ 全套）。本仓 `artifacts/images/` 已于 2026-10-26
> 删除（红线 6），下文旧路径一并作废。

### 4.1 回 nura（boot_b = Mu，recovery_a = nura ESP）

```bash
N=/home/ri/work/nura-meizu-m2381/artifacts
fastboot flash boot_b $N/mu-r91-350386e0.img
gunzip -c $N/esp-recovery-v71.img.gz > /tmp/esp-v71.img
fastboot flash recovery_a /tmp/esp-v71.img
fastboot set_active b
fastboot reboot
```

### 4.2 回 r34 iOS16（历史态，已失效）

> r34 shim 件已随本仓 `artifacts/images/` 删除；且 userdata 已换 iOS root
> （fd43ee70），r34 shim 起不来的前提已成立。此节仅留档，回 r34 需先重建
> 其 userdata（无预案 = 视为不可回）。

### 4.3 只回 recovery_a

```bash
fastboot flash recovery_a /tmp/esp-v71.img
```

boot_b 不动；Mu 会重新扫描 ESP。

## 5. 红线

1. userdata 数据保全豁免（用户 2026-10-23）：现 userdata = nura root，无须
   保留；但每次实际写入仍按本文件流程（读回、对拍、可回退优先）。
2. 不在 boot_b / recovery_a / userdata 之外动手；不碰 super、dtbo、persist、modem。
3. 每次刷写前 `getvar current-slot` = b；刷后读回 sha。
4. 不连续盲试；任一门上机失败，先回滚再分析。
5. iOS IPSW / Apple 件只本地使用。

## 6. 设备分区映射（2026-10-23 devsh 实测；映射关系不变，现役件见 docs/STATE.md §2）

| 分区 | 设备 | sha256（整分区读回） |
|---|---|---|
| boot_a | sde13 | `5f9f1503…8b11e30`（nura） |
| **boot_b** | sde43 | `af574775…26a300f`（**现役启动槽**，nura） |
| **recovery_a** | sde27 | `0a102220…3a38653f`（现役 nura ESP） |
| recovery_b | sde56 | `20bcd203…e316f43`（nura） |
| userdata | sda22 | nura root（ext4，`root=/dev/sda22`），≈464GiB |

- 旧记录"boot_b = r34 shim `339b8480…`"**作废**：设备早已被 nura 侧整槽重刷。
- 回滚件**只**在 nura 仓 `/home/ri/work/nura-meizu-m2381/artifacts/`
  （本仓 `artifacts/images/` 已于 2026-10-26 删除）；完全恢复 nura 需要
  nura 侧重刷（含 userdata）。
- userdata 4K LBA（实机 GPT 三源对拍，权威值在设备仓 `board.toml`）：
  first=2,330,472 / last=123,871,226。

## 7. 取证 reader 循环（**r55e 实测立通**，无需 nura-min）

> payload 内置 `type = "linux"` entry（forensic）：UniXNU 菜单选
> "m2381 forensic log reader" → LoadImage 一个 Linux EFI-stub 内核 PE +
> LoadFile2 initrd → 只读 dd 出 userdata 尾部 32MiB SHLOG 窗 → USB CDC
> ACM serial shell → 宿主拉回。
> **全程只读 UFS、不挂载任何文件系统、不刷任何分区。**
> r55e 首验实录（2026-10-27）：lsusb 出 `1d6b:0104` → init 报
> `userdata=sda22 … magic=SHLOG001` → 32MiB 窗拉回；同 shell 里
> `dd if=/dev/sde27 bs=1M count=96 | sha256sum` = 刷写读回对拍
> （recovery_a 前 96MiB sha == payload 镜像 sha，§2 纪律闭环）。

### 构件（../UniXNU-m2381/forensic/）

| 件 | 出处 | sha 前 8 |
|---|---|---|
| forensic-kernel.efi | nura ESP v71 BOOTAA64.EFI（pmOS r91 vmlinuz-efi，**m2381 实机验证过**：UART/USB/UFS/configfs 全通） | `4aad92b3` |
| forensic-initrd.img | busybox 1.36.1 (Ubuntu arm64 static) + init（fork 上游 uefi-fuxi package/linux-log）+ rebootbl（自编译 aarch64 static） | `402fb431`（r55e；**cpio 必须 13 字段**——newc 头 110 字节含尾部 check、namesize 含 NUL，12 字段版本内核拒包 "damaged header"，即 r55d 根因） |

### 出件（payload 命令，2026-10-27 r56 废轮教训入法）

```bash
# 三 flag 缺一不可：缺 --data-ramdisk = ADT 无 PackedRAMDisk 映射
# （手术救不了）→ Data mount ENOENT panic；缺 --trust-cache = ESP 无
# 信任缓存 sidecar。forensic reader 已由 payload.py 集成（board.toml
# [payload] forensic=true），add-forensic-entry.py 仅存手术用。
UNIXNU_UNIQUE_CHIP_ID=3645ad55d47e77c1 ./unixnu build payload \
    -D ../UniXNU-m2381 \
    --trust-cache out/m2381/trust/original-trust.bundle \
    --data-ramdisk ../UniXNU-m2381/.cache/firmware/owner-data-fixture/scratch-data.img
# 刷卡前对 ESP 验四样：scratch-data.img 在 / toml 有 initrd 行 /
# trustcache 在 / ADT 有 PackedRAMDisk（r56 = 四缺三翻车实录）
# 平铺出件照旧（payload-rNN-<sha8>.img）
```

### 宿主一次性准备（串口权限持久化，用户已装 2026-10-27）

```bash
echo 'SUBSYSTEM=="tty", ATTRS{idVendor}=="1d6b", ATTRS{idProduct}=="0104", MODE="0666"' \
  | sudo tee /etc/udev/rules.d/99-m2381-reader.rules
sudo udevadm control --reload-rules && sudo udevadm trigger
```

### 读窗循环（iOS 引导断电后；工具 = ../UniXNU-m2381/scripts/acm-reader.py）

1. 进 fastboot（电源+音量减；iOS 侧 `sheng-autoreset` 到点自落）；
2. `fastboot reboot`（或直接 ABL 默认）→ UniXNU 菜单 → 选
   **"m2381 forensic log reader"**（xnu 仍是 default，3 秒超时不动它）；
3. 宿主确认通道并拉窗（判定只看宿主，屏幕黑不阻塞）：
   ```bash
   lsusb | grep 1d6b:0104                      # 通道立起判据
   python3 ../UniXNU-m2381/scripts/acm-reader.py probe
   #   → m2381 forensic log reader 7.3.0 / usb: acm on a600000.usb
   #   → userdata=sda22 sectors=… log=33554432 bytes magic=SHLOG001
   python3 ../UniXNU-m2381/scripts/acm-reader.py \
       fetchchunk out/m2381/reader-<轮次>-log.bin
   #   设备端 sha + 分块拉（2MiB×16，每块新会话）+ 宿主对拍；
   #   r56 实测单流 fetch 在 ~13MiB 处必断（ACM gadget 长流卡死，
   #   两连复现），fetchchunk 16/16 块 + SHA MATCH 31s——断流直接换它，
   #   勿反复重试单流
   python3 ../UniXNU-m2381/scripts/read-m2381-ufs-log.py \
       out/m2381/reader-<轮次>-log.bin --all    # parser 已守 header 计数契约
   ```
   `cmd 'CMDS' [timeout]` = 任意只读 shell（如读回对拍：先
   `grep PARTNAME=recovery_a /sys/class/block/*/uevent` 找设备名再
   `dd … | sha256sum`）。
4. iOS 引导时长标准（BootLog.c：**每 2s 写一个 8KiB 槽**，slot_count
   ≈509 → 约 **17 分钟写满回卷**，回卷后最早的槽（fixups/applied 证据
   所在）被覆盖）：**跑 8–15 分钟、屏幕形态稳定后再加 2–3 分钟、长按
   硬断电**；不要超过 ~17 分钟。
5. 退出：reader shell 里 `/bin/rebootbl` 会重启但**不落 ABL fastboot**
   （我们链上实测；只回菜单/ABL 默认）——无人值守自动循环才需要 nura
   侧完整进 fastboot 方法，常规人工 = 电源+音量减。
6. 通道失效回退 = nura-min 老路（§6），通道常青。

### 红线不变

- 只读 userdata 尾部；不写不挂载；fastboot 单通道照旧。
- kernel PE 是 nura 的 pmOS 内核：**只用于取证 reader，不是 iOS 链的
  一部分**；iOS 引导永远走菜单 default（xnu entry）。
