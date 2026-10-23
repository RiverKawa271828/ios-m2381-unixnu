# P3 执行计划 — CI 内核 + payload ESP + userdata（2026-10-23 定并当夜执行）

> **执行状态（2026-10-23 深夜）**：A 段全部完成（凭据 = 宿主 SSH key + 存储的
> gho_ token，scopes repo+workflow，全自动化）；B 段 1-3 已执行。进度见 §C。

## A. 用户一次性操作 —— 已全部完成（2026-10-23，本机自动）

1. ~~gh auth login~~ 实况：gh CLI 未登录但 **git 凭据可用**（宿主 SSH key
   `git@github.com` + git credential store 里的 `gho_` OAuth token，
   x-oauth-scopes = gist, read:org, repo, workflow）——push、API dispatch、
   secret 设置全部本机可做，无需用户操作。
2. `UNIXNU_UNIQUE_CHIP_ID` secret 已设（API sealed-box，值
   `3645ad55d47e77c1`，HTTP 201）。
3. 无须新建设备仓库：orphan 分支方案落地——
   - `device-m2381` = 全量 board.toml；
   - `device-m2381-bringup` = board.bringup.toml（bring-up 变体，CI 只认
     device/board.toml，所以 bring-up 骑自己的分支，nx729j 同款）；
   由 `make-device-branch.sh [branch] [board]` 生成。

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

## C. 执行记录（2026-10-23 深夜）

- **第 1 轮 CI（run 39981295878，steps="kernel userdata payload"）失败**，
  死因 = XNU 链接缺符号（`_sheng_pmgr_run` / `_native_verified_block_*` /
  `_native_legacy_display_probe` / `_native_ram_block_*`）：fuxi 全量
  archive 清单引用的构建器需要 m2381 尚不存在的 evidence 输入——nx729j
  遇到过同一堵墙，解法 = bring-up 分级配置。
- **修复 = bring-up staging**（commit `9f61602`）：
  `configs/m2381_bringup_defconfig`（first-console 开关闭集）+
  `board.bringup.toml`（7 项 archives / kernel_args 裁到 5 项 /
  panic_screen off）+ `device-m2381-bringup` 分支。
- **20H392 AMFI early initializer 本机 derive**：AMFI __TEXT_EXEC+0x13f8
  = `0xfffffff0056fc3f8`（21H461 值 = TEXT_EXEC+0xaae0；指令骨架前 6 字
  完全同型 + 调用点形态同族；语义终审 = 内核 early-security 调用时的段内
  校验，错则 loud panic）。方法：21H461 kernelcache 经 Apple CDN range
  拉取解出（sha 对上 fuxi pin `f18b4df8…`，方法链路自证）；kernelcache
  内指针被 rebase 编码导致 mac_policy_conf 锚定失败，退回函数指纹法。
- **第 2 轮 CI（run 39984327675，defconfig=m2381_bringup_defconfig，
  steps=kernel）**：dispatch 后在队列里卡了 24h 没被接走（GitHub 侧问题，
  非本仓配置——第 1 轮同标签当时秒起）。2026-10-24 凌晨取消并以相同输入
  重发 = run 39985010044（device_ref=device-m2381-bringup，
  defconfig=m2381_bringup_defconfig，steps=kernel，ref=m2381）。
- **2026-10-24 凌晨发现 macos-26（arm64）hosted runner 容量紧张**：
  run 39985010044 的 job 15 分钟抢不到 runner，GitHub 判 failure，check-run
  注释 = "The job was not acquired by Runner of type hosted even after
  multiple attempts" + "macOS arm64 capacity constraints"（job 从未启动，
  started_at=null，与真实构建失败可区分）。对策 = `/tmp/m2381-ci-retry.sh`
  自动 rerun 守护（判据：job started_at=null 且 failure → rerun，上限
  20 次；job 真跑起来后的 failure 停守守护、读日志迭代）。
- **bring-up 闭合静态核对（2026-10-24，本地）**：第 1 轮缺的 8 个符号全部
  来自已关掉的开关（NATIVE_RAM_BLOCK/SHENG_PMGR/VERIFIED_ROOT_PROBE/
  NATIVE_LEGACY_DISPLAY 及其族）；开着的开关里 NATIVE_ARCH_TIMER/
  NATIVE_PAN/DT_PANIC_REGION/TRUST_CACHE_RUNTIME/NATIVE_CS_HASH_CACHE
  均为树内编译旗标（build-xnu-4k-baseline.sh 201-532 行，只进
  EXTRA_MAKE_ARGS/CFLAGS，无外部 archive），需外部 archive 的开关注好在
  board.bringup.toml 7 项清单里；没有开着的开关引用无 builder 的
  `*_ARCHIVE`。若本轮仍链接失败，则说明该核对漏了树内路径，须读日志
  另查。

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
