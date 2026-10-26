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

- 本机 boot_b = 96MiB；recovery_a = 100MiB；userdata = 12GB。
- 同一时刻只允许一个 fastboot 通道；本仓与 nura 侧刷写互斥。
- `fastboot boot` 是死路（魅族 ABL 裁剪 CmdBoot），只能 flash。

## 2. 读回（刷前必做）

设备在 Flyme / nura 下且有 root 时：

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

## 3. 刷写（iOS 链现役形态，r49/r50 定谳后的卡片格式）

每轮刷写 = **三件全刷**（boot_b + recovery_a + userdata），按当轮"刷写卡"执行：

- **发卡前先确认设备现态**：走过 nura-min 读回流程后 iOS 链已被覆盖，
  "只刷 recovery_a" 快捷卡只在 iOS 链未被 nura 覆盖时合法（r44 两次踩坑）。
- payload 只认**平铺路径** `out/m2381/payload-rNN-<sha8>.img`（`--out`
  双层路径歧义已在 r49 作废，旧路径文件保留作回滚件不覆盖）；
  卡上必带**刷前 sha256 门**（`sha256sum` 输出须以卡上 sha 前 8 开头）。
- 现役件（2026-10-27，sha 前 8）：boot_b = `a1d51f04`
  （devices/m2381/unixnu-m2381/build/m2381/boot/boot.img）+ recovery_a =
  当轮 payload + userdata = `fd43ee70`
  （out/m2381/ci-artifacts/userdata-root.img）。现势详见 `docs/STATE.md` §2。
- 每槽刷完立即读回 sha 对拍；刷完 `fastboot reboot`（`fastboot boot` 死路），
  让 ABL 走 boot_b。

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
- 回滚件（`artifacts/images/`）都是**历史版本**，不是现役镜像；完全恢复
  nura 需要 nura 侧重刷（含 userdata）。
- 几何与 board.toml 实测值见 `docs/STATE.md` §2；userdata 4K LBA：
  first=2,330,472 / last=123,871,226。
