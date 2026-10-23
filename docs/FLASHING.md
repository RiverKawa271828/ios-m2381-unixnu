# 刷写 / 读回 / 回滚（m2381）

> 只碰 **boot_b + recovery_a + userdata**。xbl/abl/tz/modem/dtbo/super/persist 永不碰。
> 任何刷写前先读回、先对拍；刷写时长不算证据，dd/读回 sha 才算。

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

## 3. 刷写（具体镜像待设备仓/UEFI 产出后补）

路线 A 的刷写流程等 `devices/m2381/` 出第一版镜像后补全。原则不变：

- 先刷可回退的那个槽（当前 = recovery_a，boot_b 保留 r34 shim）；
- 每槽刷完立即读回 sha 对拍；
- 刷完不要 `fastboot boot`（死路）；直接 `fastboot reboot` 让 ABL 走 boot_b。

## 4. 回滚

### 4.1 回 nura（boot_b = Mu，recovery_a = nura ESP）

```bash
# boot_b: nura 的 Mu
fastboot flash boot_b artifacts/images/mu-r69-97cdea20.img
# recovery_a: 解压 nura ESP v71
gunzip -c artifacts/images/esp-recovery-v71.img.gz > /tmp/esp-v71.img
fastboot flash recovery_a /tmp/esp-v71.img
fastboot set_active b
fastboot reboot
```

### 4.2 回 r34 iOS16（boot_b = r34 shim）

```bash
fastboot flash boot_b artifacts/images/shim-boot-m2381.img
fastboot set_active b
fastboot reboot
```

> r34 shim 的 boot_b 现状 sha `339b8480…`；userdata 还是 r34 root 时可直接回。
> 如果 userdata 已被换掉，r34 shim 也起不来；先确认 userdata 身份。

### 4.3 只回 recovery_a

```bash
fastboot flash recovery_a /tmp/esp-v71.img
```

boot_b 不动；Mu 会重新扫描 ESP。

## 5. 红线

1. userdata 抹除/覆盖前必须用户拍板；当前 r34 root 没有本地完整镜像。
2. 不在 boot_b / recovery_a 之外动手；不碰 super、dtbo、persist、modem。
3. 每次刷写前 `getvar current-slot` = b；刷后读回 sha。
4. 不连续盲试；任一门上机失败，先回滚再分析。
5. iOS IPSW / Apple 件只本地使用。

## 6. 当前设备状态（2026-10-22，未动）

| 分区 | 记录值 | 备注 |
|---|---|---|
| boot_b | r34 shim，sha `339b8480…` | 上机前读回确认 |
| recovery_a | 未读回 | nura ESP v71 只是模板 |
| userdata | r34 iPhone X 16.7.16 root，sha `7aaab96f…` | 12GB；无本地完整镜像 |

回滚件（都在 `artifacts/images/`）：

- `artifacts/images/mu-r69-97cdea20.img`（sha `97cdea20…`）
- `artifacts/images/esp-recovery-v71.img.gz`（sha `6fe7811a…`）
- `artifacts/images/shim-boot-m2381.img`（sha `339b8480…`；2026-10-23 从
  已删除的 `legacy/sweet/images/` 移入）
