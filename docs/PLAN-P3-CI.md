# P3 执行计划 — CI 内核 + payload ESP + userdata（2026-10-23 定，用户已授权）

> 前置状态：P1/P2 完成（commits `aacdfc4`…`a6c71f4`，本地）；FV + boot.img
> 本机已产出；**唯一堵点 = kernel.macho 只能 macOS CI**。本计划分两段：
> **A 段 = 用户一次性操作（约 5 分钟，网页/终端）**；**B 段 = 会话内自动
> 执行**。推送窗口已获用户授权（2026-10-23）。

## A. 用户一次性操作（下会话开工前或会话开头做完）

1. **登录 gh**（解锁 push / dispatch / secrets）：
   ```bash
   gh auth login        # 选 github.com → HTTPS → 浏览器授权
   ```
2. **设置 CI secret**（只需一次；值 = P1 定死的软件种子）：
   ```bash
   gh secret set UNIXNU_UNIQUE_CHIP_ID -R RiverKawa271828/ios-m2381-unixnu -b "3645ad55d47e77c1"
   ```
3. 无须新建设备仓库：设备仓以 orphan 分支 **`device-m2381`** 推到 origin
   （根 = 设备仓四件：board.toml / configs / scripts / package），
   CI dispatch 参数 `device_repository=RiverKawa271828/ios-m2381-unixnu`
   `device_ref=device-m2381`。分支由
   `devices/m2381/unixnu-m2381/scripts/make-device-branch.sh` 生成
   （首版已验证：`e78b281`）。若用户更想要独立库，备选 = 网页建
   `unixnu-m2381` 私有库后改 dispatch 参数。

## B. 会话内自动流程（我执行）

1. **推送**（授权已给；m2381 是 fast-forward，device-m2381 是新分支）：
   ```bash
   git push origin m2381
   ./devices/m2381/unixnu-m2381/scripts/make-device-branch.sh   # 重建纳入最新提交
   git push origin device-m2381
   ```
2. **dispatch CI**（steps=kernel userdata payload 一次跑全；~4-8h）：
   ```bash
   gh workflow run macos.yml -R RiverKawa271828/ios-m2381-unixnu \
     -f device_repository=RiverKawa271828/ios-m2381-unixnu \
     -f device_ref=device-m2381 -f defconfig=m2381_defconfig \
     -f steps="kernel userdata payload"
   ```
   - IPSW 走 board.toml 的 Apple URL（远端 range 请求，不需上传 6GB）。
   - payload 步吃 `UNIXNU_UNIQUE_CHIP_ID` secret。
   - **预期失败点**：20H392 的 prelink 三值还是 21H461 占位（slide /
     early_security_init / text_fixups）——prelink 指针绑定核对会大声
     失败。见 B-3 提前 derive，CI 失败即用本机值重试。
3. **等 CI 期间本机 derive 20H392 prelink 值**（`out/ipsw/20H392/
   kernelcache.decompressed` 已在）：
   - slide：跟随上游内核构建约定（fuxi/nx729j 均 `-0x1000000`，与镜像
     代次无关——先沿用，绑定核对是裁决）。
   - early_security_init = AMFI kext 的 slide 后地址：解 20H392
     kernelcache 的 Mach-O kext 预链接段，取
     `com.apple.driver.AppleMobileFileIntegrity` 的 `__TEXT` vm 地址 +
     slide；形态应为 `0xfffffff0…`。
   - text_fixups：20H392 的 dyld_shared_cache 无 fixup 先例时**留空**
     （nx729j 的四条全是 21H461 偏移；negroni 走源码内核未用）。P4 用户
     态卡点再补。
4. **取件**：`gh run watch` → artifacts `kernel.macho`（+ userdata 件）→
   放 `out/m2381/` 对应路径。
5. **payload ESP 打包**（本机）：
   ```bash
   ./unixnu build payload --unique-chip-id 3645ad55d47e77c1 \
     --trust-cache out/ipsw/20H392/…trustcache（bundle 形态待核 CI 产物）
   ```
   → `M2381ESP` FAT 96MiB → 待刷 recovery_a 的镜像。UUID 占位解除：
   从 userdata 件读 20H392 Data 卷 UUID 回填 board.toml +
   `CONFIG_XNU_OWNER_DATA_VOLUME_UUID`，check-m2381.py 收紧回 FAIL。
6. **桌面全量校验**（不刷机）：boot.img（已就绪 `40f2a199…`）+ ESP +
   userdata 哈希对拍；预检全绿。
7. 更新 STATE / porting-notes；commit 不推送（推送 = 已授权的 A 段 +
   B-1 范围，后续 commit 仍走拍板）。

## 风险与回退

| 风险 | 对策 |
|---|---|
| CI prelink 失败（20H392 值错） | B-3 本机 derive；slide 绑定核对是 loud failure，改值重跑（每轮 4-8h，尽量一次到位） |
| macos.yml 拒私有 device 分支 | github.token 同仓可读自己的私有分支；不行再 UNIXNU_DEVICE_TOKEN |
| kernel 步成功但 userdata 步磁盘不足 | steps 拆两轮（先 kernel，后 userdata payload） |
| dispatch 参数与 workflow 版本漂移 | rebase 后以树内 macos.yml 输入定义为准 |
| 20H392 AMFI 地址解析方法不通 | 退化：CI 用 fuxi 值跑（预期 prelink 报错信息会给出期望地址线索，比盲derive快） |

## 明确不做（本计划内）

- 不刷机（P4，等 P3 三件齐 + 哈希对拍 + 用户拍板）。
- 不动 nura 现役分区（boot_b/recovery_a 首刷前再走 FLASHING 纪律）。
