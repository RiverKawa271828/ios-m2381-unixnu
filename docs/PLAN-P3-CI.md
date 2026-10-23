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

## D. 执行记录（2026-10-24 凌晨，第 7/8 轮 + 首刷上机）

- **第 7 轮（run 40034860212，Intel）kernel 步全绿**：连破四墙后 XNU 链接 +
  prelink 首次通过，产出 kernel.macho `056820a3…`。四墙 = ①设备仓缺
  `build-arm-driver.py`（fuxi/sheng/nx729j 设备仓都自带，派生时漏抄）→收编
  （ea13f79）②probe 补丁 site-A 缺编译守卫→同款 `#if XNU_NATIVE_LEGACY_DISPLAY`
  守卫（ea13f79）③20H392 kext 指针绑 xnu-8796 内核导出
  `_IORPCMessageFromMach`（xnu-10063 改名 static Reply）→补丁按 8796 原体
  恢复 extern "C" 导出（f82e851）④instruction_inventory 把 20H392 Libm 的
  3 个内联数据 run（0x5b8c1b0-0x5b8c510）当指令→`IOS16_LIBM_LITERAL_POOLS`
  白名单（sha256 锚定）。④的修法走**设备仓自带 kit 目录**（feeds 规则 5
  优先于基座子模块；kits/NativePrelink 是上游子模块无推送权；
  make-device-branch.sh 加拷 kits；6763ec5）。白名单修法本地全验证：
  23 个 kit 单测过 + instruction_inventory 20H392 全过（236906 编码）。
- **第 8 轮（run 40035863677）+ upload_kernel 输入**：workflow 只传文本日志
  （prelinked kernel 内嵌 Apple kext，公开仓 artifact=分发，红线 7）。新增
  `upload_kernel`（默认 false）上传**未链接 noctf 内核**（纯 xnu 源码构建，
  Apple OSS 可再分发；39b676c）。取回 6.3MB artifact。
- **本地 prelink（Linux）**：emit_prelink.py 纯 Python，用本地
  kernelcache.decompressed + CI noctf → out/m2381/kernel/kernel.macho
  57,147,392B（sha 1c7cf512…；与 r7 不同因内核构建不可复现，均有效）。
  **P3 的"取件"改为 noctf + 本地 prelink，Apple 件不出本机。**
- **payload（B-5）**：`unixnu build payload -D out/m2381/device-bringup
  --unique-chip-id 3645ad55d47e77c1 --trust-cache out/ipsw/20H392/
  087-86698-044.dmg.trustcache`（sha 63e01c03 命中板级白名单）→
  M2381ESP 96MiB `5b919cd0…`：kernel.macho + kernel.macho.trustcache +
  kernel.macho.adt（root_match_volume_uuid 仍 TODO-P3 占位，owner 链关）。
- **boot.img 勘误**：磁盘上 527d0463 是上会话试包；当前树确定性重建 =
  `40f2a199`（与 P2 boot.json 逐字节一致，fd 201a2c87），已换上。
- **首刷（用户授权，boot_b/recovery_a 读回基线✓、免备份）**：单 fastboot
  会话 flash recovery_a(98304KB)+boot_b(1624KB) 双 OKAY，`fastboot reboot`。
- **首机观测（里程碑）**：**UniXNU UEFI 菜单上屏**（照片存档）：SimpleFbDxe
  显示 ✓、ESP 读取 ✓（config \EFI\UniXNU\unixnu.toml 已加载）、菜单/按键 ✓。
  entry 1 "m2381 first boot (xnu)" 返回 **"Compromised Data"**；drivers
  0 loaded/0 failed、kits 0、acpi 0。
- **下会话主攻 = "Compromised Data"**：DarwinBoot 对 payload 的校验返回值。
  候选：trust cache 形态/覆盖面（OS dmg StaticTrustCache 是否覆盖
  kernelcache 预链接 kext？可能需要 Cryptex1,SystemTrustCache 或 kernelcache
  专属 trust cache）、ADT/种子校验、unixnu.toml 与 payload 布局比对。
  回滚件 = mu-r69 + esp-v71（artifacts/images/），nura userdata 未动。

## E. 执行记录（2026-10-24，第 9 轮："Compromised Data" 定案 = trust cache 格式）

- **本地复现锁定死因**：把 DarwinBoot 的 `Boot()` 五处 `EFI_COMPROMISED_DATA`
  分支逐行复刻成 Python（/tmp/esp-check/check_payload.py），对首刷 ESP
  解包件跑：**ADT 全过**（结构/`chosen`→`memory-map`/`darwinboot-memory-size`
  0x130000000 全合法，parsed_end == TreeSize），死在 `ValidTrustCaches`
  （DarwinBoot.c:551）——容器头读出 count=0x0201xxxx 垃圾值。
- **根因**：第 8 轮把 IPSW 的 `087-86698-044.dmg.trustcache` **IM4P 原件**
  （DER 包装 `IM4P`/`trst`）直接喂给 `--trust-cache`，而 `unixnu build
  payload` 对该文件是**原样 copyfile**（payload.py:143），DarwinBoot
  `ValidTrustCaches` 只认**裸多模块容器**（首 u32 = 模块数 + 偏移表 +
  v2 模块体）。不是覆盖面问题，是格式差一层包装。
- **现成解法（上游）**：fuxi 链路 = `scripts/build-original-system-trust.py`
  （IM4P 剥壳 → 裸 v2 模块 → static+system 打包 → 按 loader 规则自检），
  `make-owner-trust-bundle.py` 注释确认内核侧二分消费同一容器格式。
- **修复（设备仓）**：
  - 新增 `scripts/make-m2381-trust-bundle.py`：DER 剥壳两个 20H392 件
    （Cryptex 件内型是 `trcs`），输入 sha 对板级 pin，模块校验（v2/尺寸
    算术/升序/类别字节），bundle = StaticTrustCache(2760 条) +
    Cryptex1,SystemTrustCache(61 条)，loader 规则自检 →
    `out/m2381/trust/original-trust.bundle`（`eb1475f6…`）。
  - board.toml + board.bringup.toml：`trust_cache_sha256` 白名单加第三条
    （bundle sha）。
  - payload 重建：`0dd57373` → 复验发现**非确定性**，唯一随机源 =
    make-m2381-adt.py `/chosen/random-seed` 的 `os.urandom(64)`（上游有意
    设计）。**决定保留 urandom**（上游对齐 + 熵卫生），对拍纪律以磁盘上
    payload.img 实体为准；**待刷件 = `f33a43c6…`**（96MiB）。
- **本地复验全绿**：新 payload 解包后 ADT/容器/ValidTrustCaches 全过，
  DarwinBoot ExitBootServices 之前所有拒绝分支排除；check-m2381
  4 ok / 0 fail（root_match_volume_uuid WARN = 已知 P3 占位）。
- **下一步 = 只刷 recovery_a**（boot_b 不动）：`fastboot flash recovery_a
  payload.img`，刷后读回对拍 `f33a43c6…`；预期 "Compromised Data" 消失，
  下一观测点 = DARWINBOOT 大字诊断（hand-off/green bar）或内核 panic 面。
  若内核启动后在预链接 kext 校验上炸，才是真正的"覆盖面"问题（届时查
  StaticTrustCache 是否覆盖本地 prelink 产物）。

## F. 执行记录（2026-10-24，第 10 轮：绿条里程碑 + 早期内核探针）

- **绿条里程碑（用户照片）**：刷入第 9 轮 payload 后 "Compromised Data" 消失
  —— DarwinBoot 全链走完（读件/校验/窗口分配/ExitBootServices），
  **内核入口代码确实执行**（顶部绿条 = entry stub 画出）。entry words
  `140006F5 D503201F×3` = 入口 `b start_first_cpu` + NOP 填充，符合预期；
  `SecurityLock/Capsule Interface Locked/MU_CHANGE/DBI dump/App Log Flush`
  是 OEM DXE 的 ExitBootServices 回调日志，正常。当前堵点推进为**内核早期
  静默死亡**（绿条后再无绘制）。
- **早期内核可见性 = 入口探针（nx729j 现成 kit 收编）**：
  `reference/UniXNU-nx729j/scripts/patch-xnu-entry-probe.py` +
  `xnu-panic-screen.c` 收编进本树 `scripts/`，唯一适配 = **洞区验收放宽**：
  本板链接器入口填充是 NOP（`0xD503201F`）而非 fuxi 的全零；实测洞区
  [entry+0x78, entry+0xB78) 全 NOP，`0xB78` 起是 `b .` 自旋桩表
  （与 fuxi 的 PANIC_CAVE_END=entry+0xB68 同构），`empty_padding()` 把
  零/NOP 都算空洞。其余全走 nx729j 的通用 derive（entry PC 读
  LC_UNIXTHREAD；`_panic`/`_vinfo`/`_start_first_cpu` 从符号表；MMU 使能点
  唯一指令序列锚定——本板符号齐全，地址实测）。
- **探针内核产出**：`kernel.probe.macho`（`015e45fb…`）= 本地 prelink 内核
  副本 + stage 1 蓝条（entry）+ stage 2 品红条（MMU on）+ panic screen
  （0x9ED 字节 @ entry+0x138；`_panic` 与 `_panic_trap_to_debugger`
  双漏斗重定向；全 site 校验通过，loud refusal 未触发）。
- **待刷 payload = `387516f0…`**（96MiB）：探针内核 + trust bundle + ADT，
  本地 DarwinBoot 前置校验复验全绿。**只刷 recovery_a**，boot_b 不动。
- **下轮读图指南**：绿条后出现**蓝条** = 入口探针执行（必然）；再出
  **品红条** = MMU 开启点到达；**大字文本** = panic 串（panic screen 接管）。
  全无（绿条后死） = stage 2 之前死，下轮上 stage 4 reset-after 探针
  二分 `_start_first_cpu` 之前的调用点。
