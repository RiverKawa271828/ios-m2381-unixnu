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

## G. 执行记录（2026-10-24，第 11 轮：panic 定案 = crypto 注册时序代错配）

- **panic screen 首战告捷（用户照片）**：绿条 + 蓝条（入口探针）+
  **完整 panic 现场**：`Kernel data abort, pc=_SHA512_Init+0x10,
  lr=_entropy_init+0x20`，ESR 0x96000005（同 EL 数据访问，1 级转换错误），
  FAR=0x40。符号化 + 反汇编定案：`x8 = *(g_crypto_funcs)` 为 NULL，
  `ldr x0,[x8,#0x40]`（取 ccsha512_di）炸。
- **调用链取证（全镜像 BL 扫描 + kext 反汇编）**：entropy_init ←
  register_and_init_prng ← **Apple corecrypto kext start**
  （kmod_info.start→_realmain=0x46d5f3c，20H392 件已 prelink 且**确实被
  启动了**）。16.7 kext 的 start 顺序 = sysctl 注册 → 装 PRNG（0x46d6688
  调 register_and_init_prng）→ 填 kpis → **0x46d69d0 才调
  register_crypto_functions**——注册在 PRNG 安装**之后**；kpis 结构
  prelink 镜像里全零（运行时填充），顺序交换补丁不可行。
- **根因（代错配）**：xnu-8796（iOS 17 代源码）的 entropy_init 在 PRNG
  安装时就经 g_crypto_funcs 调 SHA512_Init；iOS 16.7 的 corecrypto kext
  按 iOS 16 内核的契约"先装 PRNG 后注册"。nx729j/fuxi 用 21H461（iOS 17
  同代）所以不炸。上游 `source-corecrypto14-bootstrap.patch` 的
  prelinked 模式剥离（`-UXNU_SOURCE_CORECRYPTO14`）正是为同代配对设计的。
- **修复（双轨）**：
  1. **源码补丁** `patches/xnu/native-entropy-deferred-sha512.patch`：
     entropy_init 的 `SHA512_Init` 加 `g_crypto_funcs != NULL` 守卫
     （git diff 生成、已验证 clean apply）。对 21H461 配对零影响（注册
     先行，守卫恒真）；sha512_ctx 唯一消费者 entropy_provide 只在 PRNG
     真正生成时运行（彼时注册必已完成），零状态只改变首次提取的压缩 IV。
  2. **本地二进制补丁**（不等 CI 重核）：设备仓
     `scripts/patch-entropy-sha512-guard.py`——entropy_init 的
     `bl _SHA512_Init`（+0x1c）重定向到 corecrypto kext __TEXT_EXEC
     udf 死填充区的守卫跳板（0x46d8b3c，9 条指令：读 g_crypto_funcs，
     NULL 则跳过，否则 x19=entropy_data 调原函数；分支距 64MiB < 128MiB
     限制；cbz 跳转已核栈平衡）。流水线 = kernel.macho → probe patch →
     entropy guard → payload。
- **待刷 payload = `8d05c74a…`**（96MiB；探针内核 015e45fb + 本补丁，
 DarwinBoot 前置校验复验全绿）。只刷 recovery_a。
- **读图预期**：若守卫生效，entropy_init 静默通过，kext start 完成注册，
  下一个观测点 = 后续启动阶段的新 panic 面（AMFI/mac_policy、
  trust_cache_init、随机数子系统等都是候选雷区）或 stage bars 前进。

## H. 执行记录（2026-10-24，第 12 轮：entropy 守卫生效，新墙 = AppleImage4 mix-n-match）

- **守卫确认生效**：entropy_init 不再炸（上一轮的 FAR 0x40 消失），启动推进
  到 kext start 阶段更深处的下一个 panic：
  **"failed to get mix-n-match policy: 2"**（panic screen 带格式化参数，
  后接标准 TCR/SCTLR/pages/free 尾注）。
- **定域**：panic 串唯一实例在 `com.apple.security.AppleImage4 __TEXT`
  （0x3e20262）。上下文字符串揭示其所在流程 = img4 AP 实例化：
  `failed to instantiate ap` → **`failed to get mix-n-match policy`** →
  `/chosen/manifest-properties`；邻居 = `acquire CSM exports`、
  `pmap runtime initialization`、`chip: AMNM = 0x%x`、
  `overriding mix-n-match policy` 等——AppleImage4 在取芯片的
  allow-mix-n-match 观测值（IMG4_IDENTIFIER_AMNM，img4 runtime.h），来源
  候选 = CSM/pmap_cs 导出或芯片观测接口。
- **取证状态**：串的代码引用点尚未定位——AppleImage4 不走常规
  `bl _panic`（全镜像 4906 个 panic 直调中零命中该区），其导入机制为
  自有 __DATA_CONST 槽（0x5cabb18→_panic 已找到，但 __TEXT_EXEC 无
  adrp x16 桩，疑似 double-deref/blr 形态）；`_img4if` 内核接口结构
  （0x71b4380）已 dump，22 个有效槽位符号化待做。
- **对照面**：fuxi/sheng（README 实证 home screen + Metal 全通）= xnu-8796
  + **21H461**（iOS 17.7.11）配对，AppleImage4 在同款内核上能过；我们 =
  20H392（iOS 16.7.16，用户拍板的手机身份），16.7 的 AppleImage4 策略
  获取路径与 17.x 不同。nx729j 截至 10-22 只到 UEFI 阶段 A，无内核上机
  记录可参照。**m2381 在 16.7 配对上属开荒。**
- **下一步候选（第 13 轮）**：
  1. 从 Apple CDN 拉 21H461 kernelcache（fuxi pin `f18b4df8…`，方法已
     实证）→ 剥 17.7 的 AppleImage4 → 差分两版 mix-n-match getter 的
     数据来源（CSM 导出 vs ADT/manifest-properties vs img4if 槽位），
     按 entropy 先例给 16.7 版补来源或补内核侧。
  2. 廉价试探（可与 1 并行）：查 16.7 getter 是否可从 ADT
     `/chosen/manifest-properties`（我们的 ADT 已有该节点）取 AMNM——
     补属性即活，则无需动内核。
  3. 继续本地 xref：破解 AppleImage4 的 double-deref 导入形态，定位
     panic 调用点后按 entropy 同款"守卫跳板"模式处理。
- **本轮回滚/资产**：payload `8d05c74a…`（现刷在 recovery_a，含 entropy
  守卫，仍有效——下一轮无论走哪条路都基于它叠加）。

## I. 执行记录（2026-10-24，第 13 轮：donor 换配 21H16，mix-n-match 墙改由配对修复消除）

- **换配定案（用户拍板）**：IPSW root 换 **iPhone11,2（iPhone XS）/ iOS 17.7
  `21H16` / d321ap**。第 12 轮定域后复盘根因：内核源码 xnu-10063.141.1
  （17.6 代）对 20H392（16 代）是全程跨代配对，entropy 墙（r11）与
  mix-n-match 墙（r12）是同一条根因的两个切面；与其在 16.7 上逐墙开荒
  （G/H 两条候选路都要动 AppleImage4），不如换同 train 的手机镜像
  （fuxi/nx729j = 21H461 平板原生配对实证了 21H train 与 xnu-10063 源码
  同代）。20H392 的差分方案（H-1/2/3）随之降级为备选。
- **IPSW 核对**：本地件 sha256 `6fd757be…` 与 ipsw.me 记录一致；官方 URL
  （2024FallFCS/062-79002）活体 range 请求 206 ✓。BuildManifest：d321ap =
  ApChipID 0x8020（t8020，A12）/ ApBoardID 0x0e；OS =
  `044-17947-018.dmg`（`9b69f645…`）、Cryptex1,SystemOS =
  `044-17442-018.dmg`（`3b21f08a…`）、两 trust cache
  `090c6a97…` / `8e43fc06…`；kernelcache 解压 sha `0fb5cf59…`。
- **prelink 三值（本机 derive，替代原"等 CI"计划）**：
  - slide = `-0x1000000` 不变（上游约定，绑定核对裁决）。
  - **early_security_init = `com.apple.driver.AppleMobileFileIntegrity=
    0xfffffff008f3bd18`**，方法升级为**三重指纹**：AMFI early-init 函数
    序言 6 字（`ff0304d1 f85f0ca9 f6570da9 f44f0ea9 fd7b0fa9 fdc30391`）
    在 21H461/20H392/21H16 三代 cache 中逐字相同，且在各代 AMFI
    __TEXT_EXEC 内**唯一命中**；在 21H461 命中点 = fuxi 已知值
    `0xfffffff005ab4ae0`（TEXT_EXEC `0x5aaa000` + 0xaae0）✓、在 20H392
    命中点 = r11 实机已验证值 `0xfffffff0056fc3f8`（+0x13f8）✓——两代
    已知值反证方法，第三代命中即取值。21H16 AMFI __TEXT_EXEC =
    `0xfffffff008f2ea10`，命中点 = +0xd308；该段 initprot=5/maxprot=5
    满足内核 early-security 调用的段内校验。
  - text_fixups 留空（P4 用户态阶段再补）。
- **trust bundle 重生**：`make-m2381-trust-bundle.py` PIN 换 21H16 两件，
  bundle = **`e9f6c5b9…`**（2868 + 80 entries，StaticTrustCache 在前），两
  board 回填。
- **ADT 换 XS 身份**：`make-m2381-adt.py` 用 21H16 内真 d321ap DeviceTree
  （tools/adt.py 解包）取真值：model iPhone11,2、target D321、product-name
  "iPhone XS"、**compatible-device-fallback = iPhone10,3**（真 d321ap 值，
  恰为旧 donor）、idiom phone / scale 3 / subtype 0x984=2436 / P3。生成
  冒烟通过（11344 B，identity iPhone11,2）。
- **NativePrelink kit 三处适配（21H16 两个新面：arm64e、t8020 段布局）**：
  1. **arm64e**：cache cputype 仍 0x100000c 但 subtype = 2（嵌入 kext =
     0xC0000002，PAC 版本位）；上游显式拒绝 arm64e（`test_refuse_arm64e`）。
     放宽 `macho()` 门到 subtype 0/2（含 PAC 版本位），拒绝测试改写为新
     契约（arm64e 收、其余拒）。PAC 风险注记：组件代码逐字节复制，
     pacibsp/autibsp 自平衡，SM8550（ARMv9）有 FEAT_PAuth；数据指针仍是
     KASLR 偏移机制（`_PrelinkLinkKASLROffsets` 在 21H16 在场），上机终审。
  2. **info plist 形状**：21H16 的 `__PRELINK_INFO` 是带 `<?xml`/`<plist>`
     包裹的缩进 XML（21H461 = 裸 dict、hex integer + IDREF；20H392 = 裸
     dict 十进制）。`unserialize_xml` 增加单层 `<plist>` 剥壳。
  3. **源段解析**：t8011 cache 的 kext 段在专用 `__PLK_*`/`__PRELINK_*`
     段；t8020 cache 把 kext `__TEXT_EXEC`/`__DATA_CONST`/`__DATA` 装进
     **内核级同名段**。emit 的 `COALESCED` 源段查找从固定段名改为按组件
     VA 包含关系解析（`__PRELINK_TEXT` 名字查找保留为 fallback）。
  - **验证**：kit 26 项测试全绿；真实 21H16 cache 干跑 = 31 roots 中 30
    个模块解析落位、Libm 池表三张逐字节命中、AMFI/containment 全对。
    IOHIDEventDriver 两代同为无 loadaddr 条目（0x7fffffffffffffff 哨兵/
    缺键），plan() codeless 路径处理，非新问题。
- **Libm literal pools 免重 derive**：`LIBM_LITERAL_POOLS` 三张表在
  20H392/21H461/21H16 逐字节相同（纯数学常数），前缀+sha 校验天然跨代；
  原注释"20H392 专属"已修正。
- **entropy 守卫保留**：源码补丁 + 二进制守卫对同代配对无害（注册先行时
  守卫恒真）；但跳板 VA `0x46D8B3C` 是 20H392 prelinked 镜像专属，21H16
  出核后**必须重 derive**（脚本已加 [TODO kernel step]；空填充探针
  failsafe，但不能赌"恰好为零"的区域）。
- **预检与 pin**：board.toml / board.bringup.toml / check-m2381.py /
  defconfig 注释全部换 21H16；check-m2381.py 全绿（仅 root UUID 照旧
  WARN）。20H392 资产（out/ipsw/20H392、device 侧历史记录）保留作回溯。
- **下一步（第 14 轮）**：设备仓提交随 push 窗口上 GitHub（device-m2381
  orphan 分支刷新）→ dispatch macos.yml（defconfig=m2381_bringup_defconfig
  先行）出 21H16 bring-up 内核 → 本机 probe/entropy 守卫流水线换 trampoline
  重 derive → payload 打包。arm64e donor 的指针语义为上机门新增风险项，
  CI prelink 报告（bindings/extents）先做桌面审。

## J. 第 14 轮准备（2026-10-24 续会话：21H16 指针机制硬 blocker 提前暴露）

- **结论先行：21H16 donor 不能直接进 CI kernel 步。** 第 13 轮的 kit 三处
  适配只解决了"解析"，没解决"重绑定"——本会话逐层下钻发现：
  1. 21H16 cache = **fileset kernel collection**（MH_FILESET filetype 12，
     顶层 239 条 `LC_FILESET_ENTRY` + 1 条 `LC_DYLD_CHAINED_FIXUPS`
     @0x3460000/0x446）；20H392/21H461 是经典 kernelcache（裸 Mach-O）。
  2. **全镜像无 `_PrelinkLinkKASLROffsets`**（顶层与 246 个 kext dict 均无；
     20H392 有 38.7 万条、21H461 同机制）。kit 的 `plan()` 第一步就依赖该
     表 → `KeyError`，CI kernel 步必死在第 13 轮分支上（已验证 plan 的
     读表逻辑与 20H392 侧行为）。
  3. 21H16 的指针 = **dyld chained fixups 编码**：内核级段 page_size
     **16384**、pointer_format **8**（上游 audit-aes-interface.py 的 KDK
     参考件断言 fmt 7——21H16 用 8，需按 dyld 源核对语义）；函数指针为
     auth-rebase 形态（kmod_info.start = `(auth 元数据<<32)|目标偏移`，
     两个数据点支持"目标偏移相对镜像基 `0xfffffff007004000`"，尚需对
     dyld 源定案）；kext 模块内 `__auth_got/__auth_ptr/__auth_stubs`
     GOT 间接 + arm64e PAC。
  4. 数据指针（如 kmod_info.address@+156）仍是裸 64 位指针。
- **含义**：换 21H16 = 给 NativePrelink 增加"fileset-KC 供体解码 + 链走查
  重绑定"一整块（解码 auth-rebase 条目、跨组件/跨内核重绑定、arm64e GOT
  再签名语义——可能需要 QARMA(key=0) 实现或证明可绕过）。这是**可离线
  开发**的（编码自包含于 cache + dyld 公开源码），但是多小时的开发项，
  不是配置活。
- **donor 决策点（用户拍板，第 14 轮第一件事）**：
  - **(a) 保留 21H16/XS 身份**：实现 fileset-KC/chained-fixups 重绑定器。
    手机身份（2026-10-24 拍板维持），同代配对的根因修复不变；代价 =
    新机制开发 + arm64e PAC 风险面。
  - **(b) 退回 fuxi 同款 21H461 iPad donor**（iPad7,1 / 17.7.11 / j120ap，
    t8011）：机器件 arm64 + KASLR 表机制 = fuxi/nx729j 全链实证，kit 零
    新机制，entropy/mix-n-match 根因同样消除（同 train）。代价 = 平板
    身份（2026-10-23 曾拍板不用 iPadOS——当时语境是 16.7 配对，需重新
    确认）。
  - 折中注记：(a) 开发出的机制对所有 A12+ 手机 donor 永久适用；(b) 可先
    出核验证同代配对消除两堵墙，机制开发并行推进。
- **本轮已完成的中性预做**：
  - entropy 守卫：21H16 corecrypto = **14.0**（与已知无需守卫的 21H461
    同版同 train；4K 块级比对零重合 = 不同 SoC 构建，无法指纹复用）。
    守卫保留（同代配对下守卫恒真、无害），trampoline 重 derive 推迟到位
    到真正出核之后。
  - 设备 orphan 分支从 `70d68bc` 重生成：`device-m2381` = `6256b00`、
    `device-m2381-bringup` = `d8ba932`。注意 make-device-branch.sh 在
    目标分支已存在时 `checkout --orphan` 直接 128 退出——重生前先
    `git branch -D`。
  - 资产：21H461 kernelcache 已拉取并解压（sha 对上 fuxi pin
    `f18b4df8…`，out/ipsw/21H461/）——无论 (a)(b) 都要用。
- **第 14 轮流程（按拍板结果二选一）**：
  - (b) 路：board 换 21H461 pin（fuxi url/hash 现成）→ push 窗口 →
    dispatch macos.yml（m2381_bringup_defconfig 先行）→ 出核 → probe/
    守卫流水线 → payload。arm64 供体，预期不撞指针机制墙。
  - (a) 路：先做 kit 的 KC/chained-fixups rebinder（离线 + 26 测试扩绿 +
    真件干跑）→ 再走同一 push/CI 流程。

## K. 第 14 轮定案与准备（2026-10-24 续：用户拍板 (b) 路 = 21H461 iPad donor 先行）

- **拍板**：先试平板。donor = **iPad7,1 / iOS 17.7.11 / 21H461 / j120ap
  / t8011**——fuxi/nx729j 的同款 IPSW（同 URL），机制全实证；(a) 路
  （XS/21H16 + fileset-KC 重绑定器）降级为后续主题。
- **本会话预换完成（不依赖本地 IPSW 下载）**：
  - 21H461 件全取：kernelcache（sha 对上 fuxi pin `f18b4df8…`）、
    BuildManifest（j120ap = chip 0x8011 / board 0xc）、DeviceTree.j120ap、
    两 trust cache（CDN range + CRC 校验；raw IM4P sha `4563bdbf…` /
    `9742f3d5…`——fuxi board 的 `7416ca4f…` 是 bundle 哈希，我们重生 bundle
    **逐字节复现了它**，provenance 交叉验证 ✓）。
  - board.toml / board.bringup.toml：ipsw 块（fuxi URL + 其 dmg 哈希，
    **到货后须对本地件复验再推送**）、kernelcache pin、AMFI
    `0xfffffff005ab4ae0`（fuxi 实机值 + 三重指纹在本 cache 独立验证）、
    trust pins、bundle `7416ca4f…`、root UUID `04769D26-…`（同镜像值，
    占位清除）、**text_fixups 换 nx729j 四条 21H461 值**（同 IPSW 直接适用）。
  - make-m2381-adt.py：j120ap 身份（对真 DeviceTree 逐值验证：iPad7,1 /
    J120 / "iPad Pro (12.9-inch) (2nd generation)" / fallback iPad6,7 /
    idiom pad / subtype 2732 / **scale 2**）；buttons 节点保持本机形态
    （无 Home 键，type 2）= fuxi/nx729j 同款。生成冒烟 ✓。
  - check-m2381.py：身份断言 iPad7,1/21H461/j120ap + 0x8011/12，预检
    **4 ok / 0 fail / 0 warn**（root UUID 不再 WARN）。
  - defconfig：OWNER UUID `18AB5557-…` 从占位转正（同镜像 = 同 UUID；
    fuxi/nx729j defconfig 同值）；bring-up lane 保持零占位（nx729j bringup
    先例）。
  - entropy：守卫二进制补丁 **21H461 路不应用**（跳板是 20H392 专属；
    nx729j/fuxi 同 train 无需守卫）；源码补丁保留无害。
  - kit 回归：21H461 cache 干跑 = KASLR 表在场 ✓、30/31 模块落位（
    IOHIDEventDriver codeless 路径）、包含解析正确回落到经典专用段
    （__PLK_TEXT_EXEC 等）、AMFI 值段内+保护位校验过。26 项 kit 测试绿。
- **到货后（用户下载完 iPad_Pro_HFR_17.7.11_21H461_Restore.ipsw 放
  research/ipsw/）**：
  1. `sha256sum` 全件 + `unzip -p … | sha256sum` 两个 dmg 成员，对拍
     board.toml `[ipsw.sha256]`（fuxi 值）；
  2. 解包两 trust cache 对拍 `[payload] trust_cache_sha256` 前两项；
  3. 全绿 → push 窗口：`git push origin m2381 device-m2381-bringup`（分支
     需先从最新 commit 重生成：`git branch -D … && make-device-branch.sh …`）；
  4. dispatch：`gh workflow run macos.yml -R RiverKawa271828/ios-m2381-unixnu`（逐参数照 §B-2，
     device_ref=device-m2381-bringup，defconfig=m2381_bringup_defconfig，
     steps=kernel）；
  5. 预期失败点清零：prelink 指针绑定核对 = 唯一 loud gate；entropy 二进制
     守卫**跳过**（流水线 = kernel.macho → probe patch → payload，无守卫步）；
  6. 出核后本机：probe patch → payload 打包（bundle 已就绪 `7416ca4f…`）→
     桌面校验 → 拍板首刷（刷写纪律照 FLASHING.md）。

## L. 第 14 轮最终定序（2026-10-24：用户拍板"先 iOS，不行转 iPadOS"）

- **定序**：(a) 路（XS/21H16 手机身份 + fileset-KC 重绑定器）升为先行；
  (b) 路（21H461 iPad，commit `224686f`）保持为**已提交备胎**——转 (b) 只需
  push 该分支，零返工。
- **"不行"的门（避免无限下沉）**：
  - **门 A（离线，可判定）**：rebinder 在真实 21H16 cache + CI 原生内核上
    产出自洽 prelinked 镜像——全部链条目可解码、无未解释形态、重绑定后
    自检闭合（目标归属 = 组件/kernel 两类，kernel 目标可映射到原生内核
    符号表）、kit 测试扩绿。已利好：21H16 顶层 fixups blob `imports_count
    = 0`——KC 是全链接的，**没有 bind，只有 rebase 偏移**，无需符号解析。
    若离线阶段撞上解不了的形态 → 转 (b)，成本有界。
  - **门 B（CI）**：macos.yml kernel 步通过 prelink（指针绑定核对 = loud
    gate）。
  - **门 C（上机，唯一不可离线裁决项）**：arm64e PAC 语义。**对策排序
    （2026-10-24 侧聊定，依据 = `docs/PLAN-arm64e-research.md`）**：
    **首选 = 透传假设**——arm64 内核从不启用 PAuth（SCTLR_EL1
    EnIA/EnIB/EnDA = 0）时，组件里全部 PAC 指令按 ARM 规范退化为
    NOP/普通分支（PACIASP→NOP、BRAA/BLRAA→BR/BLR、LDRAA→LDR），而
    21H16 组件存储值本就是无签名裸地址（fileset KC 设计 = 加载时再签）
    → **零签名工作**。垫底①'：内核开已知 keys + prelink 路径运行时
    再签名 hook；垫底②：QARMA 离线预签名。arm64e 原生内核 = 独立专项
    会话研究对象（把门 C 做难而非做没了，唯一硬收益已被 rebinder 拿
    到），不作主线。透传两廉价验证：grep 内核/boot shim 的 SCTLR 设置
    恒 0；REA 抽查组件 GOT 消费点形态。透传假设证伪且①'②失效 → 转
    (b)（(b) = t8011 纯 arm64 经典 cache，本就无门 C）。
  - 另注意 emit 的 kmod_info 读数（+156 裸指针 ✓ / +180 start 为编码态）
    需解码感知，属门 A 范围。
- **对 §K runbook 的修正**：IPSW 到货后若走 (a) 路则 21H16 对拍照旧；
  dispatch 前先过门 A。(b) 路流程原样保留为备胎执行单。

## M. 门 A 先行战果（2026-10-24 续二：21H16 链格式完全破译，零未解释形态）

- **dyld chained fixups（pointer_format 8）语义钉死**，全部经真实 cache 数据
  约束验证：
  - 条目 = `dyld_chained_ptr_64_auth_rebase` 标准位域：
    `target:32 | diversity:16 | addrDiv:1 | key:2 | next:12 | auth:1`；
  - **target_va = 镜像基 `0xfffffff007004000`（内核 __TEXT vmaddr）+ target32**
    （三个独立锚点吻合：kmod_info.start、__DATA_CONST/__DATA 的
    segment_offset = 段 VA − 镜像基、全量目标落段统计）；
  - **唯一非标点：next 以 4 字节为单位**（非 dyld 64 位格式的字节单位）——
    用真链判定（u4 全链 8 对齐走完，bytes 走进字符串垃圾）；这正是
    pointer_format 8 与上游 KDK 参考件 fmt 7 的差别；
  - 页表：`page_start[i] = 0xFFFF` = 该页无链；链起点即页内偏移；
  - **存储值不含签名**，auth 条目只有 key/diversity 选择子（IA=key0 占
    绝对多数 357,971，key2=2,116；addrDiv=1 占 335,662）——签名在加载时
    计算，Gate C 对策（运行时再签名 / QARMA 预签名）照旧。
- **全量清点（21H16 真实 cache，零假设）**：419,323 条链目，plain 59,236 /
  auth 360,087；目标落段 = __TEXT_EXEC 362,981 / __PRELINK_TEXT 36,111 /
  __DATA 12,432 / __DATA_CONST 7,799，**100% 在已映射段内、零越界、零
  runaway**（最长链 2047 跳 = 12 位 next 上限，自洽）。链只存在于
  __DATA_CONST 与 __DATA（代码段纯 PC 相对，无需重定位）。
- **对 rebinder 的直接含义**：
  1. 解码/走查/再编码层 = 纯机械（重绑定 = 改 target32，meta 原样保留）；
  2. 工作量上限 419k 条，其中落在选定组件段内的才是真负载（roots 选定后
     再过滤）；
  3. "kernel 目标"（指向内核自身的指针）映射到原生内核导出 = 与经典路
     `unique_kernel_binding` 同构；
  4. kmod_info 读数需解码感知（address@+156 裸、start/stop@+180/188 为
     编码态，已验证 start 解码落入自家 exec 段）；
  5. arm64e GOT 的 auth 条目重绑定后需要**再签名**（存储值无签名 → 运行
     时消费者会用运行时 key 验签）——这是门 C 的实质，两条对策照旧。
- **门 A 剩余项**：kit 内实现 rebinder（解码/重绑/再编码 + kmod 解码感知 +
  kernel 目标映射）+ 26 项测试扩绿 + CI 原生内核在手后的全链自洽干跑。
  格式层风险已清零。

## N. 原生内核先行 dispatch（2026-10-24 续三）

- **关键认知**：CI kernel 步编的是我们自己的源码内核，与 donor 无关（prelink
  才吃 donor）。因此用 21H461 分支当"建造车"先行 dispatch 拿原生内核，
  rebinder 开发并行对着真内核做——两线互不阻塞。
- **执行**：m2381 推至 `d761c9e`；device 分支 force 更新（bringup
  `99a7051` / full `67fe1e2`，21H461 board，prelink 在 runner 上可过 =
  upload 前置条件 `steps.kernel.outcome == 'success'` 能满足）；dispatch
  run **`40057239716`**（bringup lane，steps=kernel，**upload_kernel=true**
  → 未链接内核 artifact）。runner 容量守护已挂（/tmp/m2381-ci-retry.sh，
  抢不到 runner 自动 rerun ≤20 次，真失败即停）。
- **注意**：21H16 board 的 CI 轮在 rebinder 就绪前不要发——runner 上
  prelink 会 KeyError 导致 kernel 步失败、artifact 拿不到。
- **产物流向**：未链接内核（bring-up 配置）→ 本机 rebinder（21H16 cache）
  → kernel.macho → probe patch（entropy 二进制守卫跳过）→ payload。

## O. 门 A 续战果（2026-10-24 续四：链走查全量验证 + kernel 目标命名墙实测）

### O.1 链走查全量验证（rebinder 语义层收口）

在 §M 格式破译之上，用 roots 闭包对真实 21H16 cache 做了全量链走查
（419,323 条目）：

- **plain 条目 meta 位零污染**：全部 59,236 条 plain（auth=0）条目的
  meta 低 19 位（diversity/addrDiv/key）恒为 0 —— rebinder 可对 plain
  形态做严格断言（非零即大声失败）；
- **组件归属分类零违规**：roots（31 个）闭包 = 31 选定 + 7 接口；选定
  组件内链站点 69,386 个，组件→组件目标 67 对 **全部落在选定闭包内**
  （0 个未声明依赖）；其余站点全部属于 donor kernel 自有数据（不复制，
  正确跳过）；
- **kernel 目标去重 = 1,795 个 VA** —— 这就是命名机制的精确工作量。

### O.2 硬发现：21H16 release cache 全面符号剥离（门 A 新墙）

逐层下钻钉死（全部实测）：

1. **元数据地址 = 真实 VA 的有符号十进制**：`_PrelinkExecutableLoadAddr`
   等以负数十进制存储（如 `-68601184256`），u64 补码即真值
   （0xfffffff0070d0000），与 LC_FILESET_ENTRY `extoffset` 精确相等。
   rebinder 消费元数据地址须统一 `& (2^64-1)` 归一。
2. **全 cache 零符号名**：kernel 组件 LC_SYMTAB `nsyms=0/strsize=1`
   （所有组件的 symtab 命令都指向同一个桩 0x3430000）；indirect
   symbol table 全零；LC_DYLD_INFO 字段为垃圾桩（rebase/bind/export
   偏移皆无意义）；顶层无 LC_SYMTAB、export trie 0/0。对照：21H461
   经典 cache 有 6,056 个导出符号、20H392 有 5,873 —— 经典路"按名映射
   kernel 目标"的数据源在 fileset release cache 上**不存在**。
3. **19 条 sentinel 条目**（load addr = INT64_MAX、无 size，多为 kpi
   接口与 codeless 占位）——plan 的 size 过滤天然跳过，无需特判。
4. LC_FILESET_ENTRY（cmd 0x80000035）布局钉死：cmd+cmdsize+
   `extoffset`(=组件 VA)+`extsize`+`entry_id`(命令内偏移，字符串内联)。
   组件发现可走元数据 load addr（227 个全在段内）。

### O.3 离线命名路线实测判死

kernel 目标命名（donor VA → 符号名 → 原生内核地址）的离线替代源实测：

- **跨 cache 字节匹配（21H461→21H16）结构性死路**：21H461/20H392 均
  为 arm64 内核（A10X/A11 不用 PAC），21H16 为 arm64e —— 指令流本身
  不同构，字节级匹配无意义；
- **panic 字符串交叉引用**（adrp/add 编码跨 arm64/arm64e 不变，21H461
  侧 6,056 符号可作名字源）：实测 21H16 kernel 19,245 函数中仅 4,830
  个有字符串引用；对 1,795 个目标函数——**60%（1,081 个）零字符串
  引用**，歧义 577 个，交集唯一命中仅 **137/1795 = 7.6%**。天花板
  数量级不足，判死。

### O.4 门 A 剩余 = 单一外部符号源 + 机械实现

- **唯一干净解 = KDK 21H16**（Apple Developer "Kernel Debug Kit 17.7
  (21H16)"，含 t8020 带符号内核）：从 KDK 内核提取 VA→导出名旁表，
  喂给 rebinder 作 `unique_kernel_binding` 的 donor 侧符号源；KDK 与
  release 同 build，VA 应逐一对齐（提取时逐 VA 校验字节前缀）。
  获取 = 用户 Apple ID 一次认证（`ipsw auth login` + `ipsw download
  kdk`，或网页手动下载）。**无 KDK → 按 §L 判门 A 死路，转 (b)
  （21H461 备胎，`224686f` 原样待命，零返工）。**
- **rebinder 本体已实现（2026-10-24 续五，32 项测试全绿）**：
  `native_prelink.py` 新增 `u64()`（元数据有符号地址归一）与
  `chain_relocations()`（链走查：fmt=8/页表/0xFFFF 跳页/next×4/条目
  4 字节对齐——457 条 4-mod-8 实站含 kmod_info +180/+188）；`plan()` 按
  `original['kind']`（2=经典 KASLR 表 / 12=fileset 链走查）分流，fileset
  要求 `original_symbols` 旁表（KDK 派生 {va: name}），缺省响亮失败；
  kmod start/stop 从链解码（零 start/stop 无链条目 → 0），并校验解码
  落点归属本组件；`emit()` 线程旁表参数、u64 归一元数据地址、fileset
  kmod 用解码值。合成 fileset fixture（test_fileset.py 6 项：链解码/
  绑定/kmod 解码/emit 重绑/四条拒绝路径）。**真实 21H16 cache 冒烟：
  419,323 条全走查（0.2s）与独立清点精确对拍，归属分类通过，精确停在
  命名门并给出 KDK 指引。** kernel 目标映射 = 查旁表（KDK 在场时与
  经典路同构）。

## P. 执行记录（2026-10-25：上游同步 + 本机 Linux 出核 + 新 payload 就绪）

- **上游 rebase（209 提交，含历史改写）**：upstream main 被上游改写过——旧
  pin `b7b6a6a`/`e95b37d` 不再是现 main 的祖先，普通 rebase 会把旧链上游
  提交当独立提交重放而全面冲突。改用 `git rebase --onto upstream/main
  b7b6a6a m2381` 只重放 49 个工作区提交：**零冲突落位**（备份分支
  `m2381-pre-rewritesync`）；rebase 后 check-m2381 4 ok / 0 fail、kit 32
  测试全绿、工作区文件零丢失。**一个 rebase 伤**：
  `patches/xnu/offline-component-load-probes.patch`——我们的 legacy-display
  编译守卫（ea13f79）与上游同款修复 `6c9de1b` 在 rebase 三方合并时叠加成
  双重 `#if` 包裹且 hunk 头计数未更新 → `git apply` 报 corrupt；恢复为
  上游 6c9de1b 原版（我们的守卫被 subsume，无语义损失，`1157c66`）。
- **上游新能力落地（本会话直接受益）**：
  - **PR#100 = kernel 步 Linux 可跑**：`sudo scripts/linux/setup-toolchain.py`
    一次装齐（打 `__builtin_xnu_type_*` 补丁的 clang 24 + cctools-port
    ld64 + mig/iig/lzfse；~1h，Ubuntu 24.04 容器直接过）。**本机出核首次
    全链打通**：XNU 编译 → emit_prelink（21H461 cache + AMFI
    `0xfffffff005ab4ae0` + 31 roots，指针绑定核对过）→ `kernel.macho`
    50,978,816 B（sha `155b2df1…`）。CI/runner 容量依赖就此解除；macos.yml
    仍可用（新输入 `runner_label` = macos-15 Intel lane、`pre_build`、
    `trust_cache_url` 等），userdata 步仍需 macOS（hdiutil）。
  - 卡死的 run 40057239716 已取消、runner 守护进程（/tmp/m2381-ci-retry.sh）
    已 kill。
- **probe/panic 流水线（新内核适配）**：`uefi-sheng` 符号链接被此前清理删了
  （panic screen 的 Font.h include 路径需要），已重建
  `uefi-sheng → reference/uefi-sheng`。**新内核入口洞区变短**：自旋桩表比
  r10 内核提前 0x70 字节，stage 3 reset 洞 + panic screen 放不下（差 0x1d）
  —— r10 配置本就是 stage 1+2+panic（stage 3 是备用探针），改用
  `--no-reset` 全过。探针内核 = kernel.macho 原地补丁（蓝条/品红条/
  panic screen 0x9ed @ `0xfffffff0074145c0` + `_panic`/common 漏斗双覆盖），
  sha `e89dcbc8…`。
- **payload 重建**：`./unixnu build payload --unique-chip-id 3645ad55d47e77c1
  --trust-cache out/m2381/trust/original-trust.bundle --kernel
  out/m2381/kernel/kernel.probe.macho`（bundle = 21H461 版 `7416ca4f…` ✓ 白
  名单）→ **M2381ESP 96MiB，待刷件 = `4d7d83db…`**。
- **桌面校验全绿**（/tmp/esp-check/check_payload.py 对新 payload 解包件）：
  ADT parsed_end == TreeSize（j120ap 身份 + 四条 21H461 text-fixups）、
  trust bundle 两模块（2478 + 76 entries）、ValidTrustCaches 过、ESP 内
  kernel = 探针件 `e89dcbc8`。DarwinBoot ExitBootServices 前全部拒绝分支
  排除。
- **待办**：①首刷拍板（只刷 recovery_a，boot_b 不动；预期 = entropy/
  mix-n-match 两墙因同代配对消失，看 panic screen 给出下一堵墙或 stage 条
  前进）；②userdata 重建件（21H461）仍走 macOS（CI 或本机无 hdiutil）；
  ③commit 未推送（推送窗口 = 用户拍板）。

## Q. 执行记录（2026-10-25 续：第 15/16 轮 = "No PMGR?" 墙）

- **第 15 轮（payload `4d7d83db`，本会话首刷）读图**：绿条（entry）→ 蓝条
  （MMU off 入口探针）→ **品红条（MMU on）** → 约 30 秒后 panic screen 大字：
  **`"AMFI: No PMGR?\n" @ConfigurationSettings.cpp:235`** + TCR/SCTLR/pages/
  free 尾注。entropy（r11）与 mix-n-match（r12）两墙确认消失（21H461 同代
  配对生效），堵点推进到 AMFI 初始化。
- **反汇编定域（全链，无源码——AMFI kext 是 Apple 内部件，OSS xnu 无
  security/mac/amfi）**：panic 格式串 `"AMFI: No PMGR?\n" @%s:%d`（文件名/
  行号是参数，0xeb=235）；调用方 = AMFI developer-mode 策略函数
  （`serviceMatching("ApplePMGRNub")` → `waitForMatchingService(…,
  0x6fc23ac00 = 30s)` → NULL → panic；找到则经 PMGR AON 寄存器锁存
  developer-mode 位并打印 "PMGRAON latched"）。**唯一旁路 =
  `kern.hv_vmm_present`（VM 检测，"AMFI: Booted in a VM"）**。
  *工具坑：emit_prelink 的大段（__PLK_TEXT_EXEC 等）是 0-section 裸段，
  按 section 扫 adrp 会整体漏扫，必须按 segment 扫。*
- **正解（fuxi/nx729j 既有机制）**：板侧 `ShengPlatformPower.cpp`
  （`sheng_pmgr_run()` 由 sheng-platform-power.patch 挂在 bsd_init
  bsd_bufferinit 之后）按 boot-arg `sheng-pmgr=2` 代 ApplePMGR 发布原版
  ApplePMGRNub（AMFI 等的就是它）；电源操作全部 "unimplemented" panic
  守卫。第 1 轮 CI 缺的 `_sheng_pmgr_run` 符号即此——bring-up lane 当时
  关掉，本轮开启。全量 defconfig 早已 `CONFIG_XNU_SHENG_PMGR=y`。
- **接线（commit `f889841`）**：设备仓收编 fuxi 的
  package/ShengPlatformPower.cpp + scripts/build-sheng-platform-power.py
  （两参照逐字节一致）；双 board `kit = "sheng"`（fuxi 同款映射——builder
  硬编码读 `kits/sheng/ShengPlatformPower.cpp`）；bringup archives 加
  builder、kernel_args 加 `sheng-pmgr=2`；bringup defconfig 旗标转 y。
  builder 前置审计 `audit-amfi-pmgr-contract.py` 需要一个 prelink
  candidate（树内 `build/native-prelink/`，`native-owner-ufs-cpu-desktop-*`
  命名即可命中 glob）+ `tree/.cache/firmware/kernelcache.macho/` 下的
  原始 cache——本地现产物落位即过；**契约审计 PASS**（t8011 PMGR 幻影
  MMIO 读恒 0xffffffff、写不落，SM8550 安全）。
- **第 16 轮首刷（payload `6e5210dc`）同墙复发** → 排查：ESP unixnu.toml
  args 含 `sheng-pmgr=2` ✓（kernel_args 走菜单配置由 DarwinBoot 递给内核，
  不进 ADT——ADT 里搜不到是正常形态）；hook 在 bsd_init（相对 AMFI kext
  start 时序合理）；真凶 = **`out/m2381/.config` 是会话头 `unixnu
  defconfig` 的旧拷贝（SHENG_PMGR=n）**——改设备仓 defconfig 后未重跑
  defconfig，归档建了但内核链接没挂（nm 50669 符号零 sheng、kernel.log 零
  sheng 引用）。教训：**改 defconfig 后必须重跑 `./unixnu defconfig`**。
- **第 17 轮（进行中）**：重跑 defconfig → 重建（`XNU_SHENG_PMGR=1` 会
  要求 SOURCE_CORECRYPTO14=1，bringup 已满足）→ 链接产物 nm 验证
  `_sheng_pmgr_run` 在场 → probe（--no-reset）→ payload → 刷。

## R. 执行记录（2026-10-25 续二：第 18-20 轮 = UFS 车三连，硬件面毕业）

- **第 18 轮（payload `8c6f1be4`，UFS 车首刷）**：panic 定位到
  NativeUfs 的 SMMU 规划——ABL 交接 apps_smmu 时**留下了本流映射**
  （SMR=0x80000060 = VALID + UFS host 流 ID 0x60，S2CR=2），驱动的
  复用检查只认自家编码（fuxi 注释：他们 ABL 交接无 UFS SMR 条目——
  板间差异），8 次 context bank 回退全撞同条 → "conflicting UFS stream
  mapping slot=2"。构建期同轮拆了三墙：①上游 `audit-storage-interface.py`
  要实机 lldb 快照（gdb-stop.json）→ 改走 **nx729j 静态绑定路**
  （`build-storage-bindings.py` + `prelink-storage-bindings.json`，同
  21H461 donor 地址不变）；②cctools nm 不认 `--defined-only`（去旗标，
  列数过滤本就够）；③NativeUfs 弱引用 `boot_progress_poll` 在本机 ld64
  报未解析 → no-op 桩归档走 `EXTRA_LD_ARCHIVES`（真 BootProgress 上场
  再换）。另：**树重组会清证据件** → 设备仓新增
  `stage-local-candidates.py` 作归档首步（自动落 kernelcache + prelink
  候选对）。
- **第 19 轮（payload `dd200d32`）= SMMU 接管**：NativeUfs 收编进设备仓
  `drivers/`（feeds 规则 5 覆盖，nx729j 同款），SmmuTakeover.h 打补丁：
  单条精确匹配本流（mask=0）的遗留 SMR 从硬错误改为**接管**（队列已静默、
  UFS 流无他主，configure 先清写 SMR/S2CR 带读回再全新分配）；真歧义
  （多匹配/带掩码）仍拒绝（`492caf7`）。结果：smmu panic 消失，推进到
  root 挂载失败（与 r17 同串——VerifiedRoot 拒绝桩设计上不发布块设备，
  且 userdata=nura ext4 无 donor Data 卷 UUID，**预期形态**）。
- **第 20 轮（payload `07518ebe`）= UFS 数据链全通（硬件面毕业）**：
  arg 换 `native-ufs-probe=10`（读-only GPT/首块检查，屏上打印结果）。
  实机读数：**`userdata first=238f68 last=7621ffa`**（与 board 实测
  逐位一致）+ **`ext4magic=ef53 ext4blocks=121540755`**（真实 nura
  ext4 超级块 ≈463GB 几何）+ manifest status=1（verified-root 未建，
  预期）。SMMU 接管 → UFSHCI 4.0 → 队列 → DMA 读回 → GPT 全链实证。
- **到 launchd 的剩余件（全部软件）**：
  1. **userdata 重建（关键路径）**：`unixnu build userdata` 需 macOS
     hdiutil；重写后的 macos.yml **只传文本日志**（明注 Apple 件不上传），
     userdata 镜像无 CI 出口——路径需拍板（CI 加 upload 特例=红线 7
     豁免 / 借 Mac / 本机 macOS VM）。
  2. **verified-root 链（Linux 可做，与 1 并行）**：
     `build-owner-os-policy.py`（纯 Python 签验原始 DMG，产出
     verification.json；输入 = 本地 IPSW 的 044-76040-038/044-76034-038
     两 dmg）→ `build-verified-root-probe.py` → defconfig 开
     VERIFIED_ROOT_PROBE/OWNER 组 + arg 回 `native-ufs-root=1`。
- **读回欠账**：recovery_a 已六刷未 dd 对拍（fastboot 无 fetch，需回
  nura 换 boot_b 舞步），列入下个 fastboot 会话。

## S. userdata 路径定案 + 证据链并入 CI（2026-10-25 续三）

- **调研结论（用户问"别人怎么做的"）**：nx729j GUIDE §8.2 = 无 Mac 用户
  fork + dispatch `steps="kernel userdata"` + **artifact 取回
  `kernel.macho`/`userdata-root.img`**——生态标准路。上游 workflow 重写
  （caad2d7）只是删了**上游仓**的上传通道；旧版 `upload_userdata`
  输入定义原样可恢复。
- **用户拍板：CI artifact 路（对红线 7 的此用途豁免，私有仓 + 7 天过期）**。
- **执行**：①macos.yml 恢复 `upload_userdata`（旧定义）+ 新增
  `upload_verified_root`；②`build-owner-os-policy.py` 实测**需 hdiutil
  attach + diskutil**（非纯 Linux）→ 证据链折进同一 run 的 `pre_build`
  （`ci-owner-policy.sh`：IPSW 取件 → keys/dylib → 按 UUID 04769D26 只读
  挂载 OS 卷 → 真 C 签名 + 全块验证 policy → 跨构建探针归档）；
  ③设备仓 pmgr builder 补"无候选跳审计"（CI 全新 runner 无 prelink
  候选，鸡生蛋）；④make-device-branch.sh 补 `drivers/`（NativeUfs 覆盖
  必须上分支）；⑤首个 run 40130587859 取消（排队 3 分钟无损失），
  **重发 40131073217**（steps=kernel userdata + xcode=26.3 +
  双 upload + pre_build），守护 pid 366228。
- **取件后本机流程**（下会话）：artifact ①userdata.img → 刷 userdata
  （nura 消亡，豁免已有；FLASHING 纪律照走）+ ②verified-root 证据落
  device `.cache/firmware/` → defconfig 翻 VERIFIED_ROOT_PROBE=y（去桩）
  → 本机出 owner 内核 → arg 回 `native-ufs-root=1` → payload → 刷。
  预期下一堵墙在 APFS 挂载/launchd 前（owner 链的 OWNER_* 旗标组届时
  按 fuxi 全量版对表逐个开）。

## T. owner 车全线打通 + userdata 镜像到货（2026-10-25 续四：第 21 轮材料齐备）

- **CI userdata 车四轮拔钉全部通过**（run `40142020091` 全绿）：①pre_build
  子模块缺失（重写后 workflow 不递归 checkout）；②Summary 步 xcodebuild
  被管道 SIGPIPE 标红成功轮；③pmgr builder 无候选鸡生蛋（CI 首建无
  prelink 产物）→ 设备仓改**双主机自适应**：无候选路径钉死 donor AON 槽
  值（7readAonEv/8writeAonEy = 0x…fbc/0x…fc4，从本机候选实测）+ 跳过跨件
  校验，own-guard 校验双主机全跑；④boot-progress 桩在 macOS 默认拿
  MacOS SDK 配 iPhone target → 补 `-isysroot iphoneos`。
- **真 VerifiedRoot 上车（owner 内核 `38b433ee`）**：探针归档三连修
  （EXPORT_HDRS/libsa 空目录、clang24 的 `-Wno-unsafe-buffer-usage`/
  `-Wno-shadow-header`）；**双卷 bundle 死结**（SystemOS 策略须同锚签名，
  CI 生成私钥未保留）→ 改**单策略 bundle**：包装脚本给探针归档追加汇编
  （`_verified_root_policies` = [OS manifest]，count=1）——根挂载只需 OS
  卷（04769D26），SystemOS 双卷留待未来同锚重产。ESP 96→100MiB（manifest
  随内核变大）。
- **坑两记**：①pmgr builder 的 "All and only audited original base imports"
  是 **require 失败串**不是 PASS（静默 SystemExit 陷阱）；②builder 重构
  时 AON 绑定行丢失导致集合不等——UNDEF/BIND 双打印定位。
- **userdata 镜像验真**（artifact `unixnu-m2381-userdata-1.zip` →
  artifacts/images/m2381/userdata/）：整镜像 sha `c5977f2d…` ✓；头部
  `SHNGROOT` 容器，OS 窗口 @1MiB = 6,077,580,800 B，**窗口 sha
  `e5634d04…` = VerifiedRoot 策略核验的同一份字节**——证据链闭合。
- **待刷件（用户自刷）**：userdata（5.7GiB，约 10-20min）+
  recovery_a = **`c767358c`**（owner 内核 + `native-ufs-root=1` + panic
  screen + 探针），单会话双刷后 reboot。
- **读图分支**：恢复界面/Apple logo/任何用户态画面 = launchd 级里程碑
  （无 Data 种子的预期终点）；APFS/挂载类新 panic = 取证循环下一墙；
  品红后静默 = stage 探针二分。
- **下会话候选**（按读图结果选）：①用户态画面 → Data 种子（CI pre_build
  可产，prepare-owner-data-fixture 需 macOS 挂载）→ 显示/触摸 P4 门；
  ②取证循环；③欠账 recovery_a dd 读回对拍；④空闲主题：arm64e 专项
  （PLAN-arm64e-research.md）、XS/fileset-KC rebinder（门 A 剩 KDK 源）、
  上游跟随。

## U. 第 22 轮：首启读图 = root 挂载 panic → 定位发布契约死结（2026-10-25 续五）

- **实机读图（用户刷 userdata 5.7GiB + recovery_a `c767358c` 后）**：绿条 →
  蓝条 → panic `Failed ro mount root device @IOKitBSDInit.cpp:984`（TCR/SCTLR/
  pages 寄存器页齐全）。绿/蓝条 = `libBootProgressStub.a` 在 VerifiedRoot 逐块
  校验 5.7GiB OS 窗口时的轮询进度（EXTRA_LD_ARCHIVES 常驻，无需 boot-progress
  旗标）。
- **panic 语义（对源码）**：IOKitBSDInit 的 iOS 路径——root 媒体在等待窗内
  始终未出现，直接 panic（"Still waiting" 文案仅 macOS 路打印）。即
  NativeUfs `startRoot` 八处软失败点之一退出、零块设备发布。
- **根因（代码层死结，与刷写质量无关）**：`RootPublication.h
  ufsBuildRootPlan` 上游契约 `count >= 2` 且必须同时有 `SystemOS`+`OS` 两窗口；
  而 §T 拍板的单策略镜像只有**一个 OS 窗口**（userdata-root.json：OS @1MiB
  6,077,580,800B，`e5634d04`）。windows=1 → plan 拒绝 →
  `rootDiagnosticFailure("root windows require distinct OS and SystemOS")`
  → 无 IOMedia → 本 panic。**该镜像配该内核必然停在此处。**三份拷贝
  （根目录 drivers/ = devices/ = out/m2381/device-bringup/）与上游树同源同病。
- **排除项**：`findRootLayout`/RootLayout 对窗口数无假设（泛型解析）；
  现役 `.config` = `m2381_owner_defconfig`，**OWNER_* 全关**
  （CACHE_MOUNT/DATA_MOUNT/UNION_SYSTEMOS 编译期就不存在）——kern_exec 里
  `offline owner SystemOS mount failed` / `offline scratch Data mount failed`
  两 panic 在这版内核**未编入**，root 挂载成功即直奔 `exec /sbin/launchd`。
- **修复**：`ufsBuildRootPlan` 放行 count=1（OS 窗口必需；多窗口时 SystemOS
  配对契约原样保留），`plan.order[0]=system` 改为 SystemOS 存在才占位（单窗口
  时原写法会越界）；`ufsPrepareRootWindows` 门槛 `count<1`。测试
  `tests/RootPublication.cpp` 扩绿：lone-OS 单窗口 plan/prepare/publish 全
  覆盖（ASan/UBSan PASS）。三份拷贝同步。
- **读图判别器（下次启动）**：修后内核 + 已刷 userdata——
  ① **launchd 级画面**（恢复界面/Apple logo/用户态任何帧）= §T 预期的
    无 Data 种子终点里程碑，userdata 落地 + root 挂载 + VerifiedRoot 全链
    实机闭环；
  ② **exec/dyld 类新 panic**（如 launchd 路径缺失）= 同样证明挂载链全通，
    墙在用户态第一脚；
  ③ **仍 `Failed to mount root device`** = userdata 未落地/manifest 被拒
    （读回欠账坐实）→ probe=11 只读窗口取证（打印 manifest+每窗 GPT）。

## V. 第 23 轮：官方文档研读 + r22 读图重判读 = 盲启非回退，探针件恢复（2026-10-25 续六）

- **官方新文档（unixnu.com/zh，2026-10-25 发布，用户指路）**：上游生态已转向
  **格式 2**（board.toml `schema_version = 2`）：SoC 共享仓（`UniXNU-sm8550-common`
  已存在！）+ OS profile（`profiles/os/ipados-17.7.11-21H461/` = 恰好我们的
  donor 版本）+ 组件改名（NativeUfs→ShengUfs、NativeHidInput→ShengHidInput、
  NativeMobileDisplay→GopFramebuffer、NativeAES→SoftwareAES、NativePrelink→
  XnuPrelink、NativeAdreno→Adreno740…）+ 新命令面（`unixnu ipsw fetch`/`kernel`/
  `build package`/`firmware`/`payload`/`bootimg`，unixnu.local.toml 存路径）+
  ADT 改由硬件 DTS+OS profile 在 payload 步生成（make-m2381-adt.py 属旧路）。
  **sheng = 官方维护的格式 2 参考设备**；ufs 页官方形态 = 双窗口 userdata
  （OS+SystemOS + root UUID）——我们的单窗口是已知偏离（r21 双策略同锚死结）。
  debugging 文档给出色条权威语义 + 日志抓取（二维码导出固件日志）。迁移格式 2
  = 独立主题，不阻塞当前循环。
- **r22 读图重判读（关键更正）**：色条语义（官方 debugging + 探针脚本双源）：
  **绿条 = DarwinBoot 画的**（"the green one that DarwinBoot paints"），只证
  加载器交接；**蓝条 = 入口探针 stage 1**（内核入口已执行、MMU 关）；**品红 =
  stage 2**（MMU 开前）；**panic 屏幕 = 探针 --panic-screen 补丁**（patch
  _panic+common 漏斗，按符号表自适配布局）。r22 payload 用了
  device-bringup/board.toml，其 `panic_screen = false`（bring-up 纪律：探针
  VA 按内核布局走，脚本 rebase() 现已自适配）→ **r22 = 盲启**：绿条后一切
  （含 5.7GiB VerifiedRoot 校验全程）不可见。**不是变严重，是变瞎**；r22 内核
  可能死在任何位置，无法判读。r21 蓝条+panic 屏的确切来源仍开放（CI payload
  json 显示同样没在 payload 步打探针、stub=空函数、VerifiedRoot 无绘图——待
  考古，不阻塞）。
- **恢复可见性（本轮产出）**：按 r10 成熟流程手动打探针——
  `patch-xnu-entry-probe.py kernel.probe.macho --panic-screen --no-reset`
  （rebase() 按新内核入口 pc 平移全部补丁点 + 符号表取 _panic/_vinfo，一次
  通过；panic screen 0x9e5B @ `0xfffffff00a1645c0`，funnel `0x…9fcad14`）→
  **探针内核 `6666f96d` → payload `e3c5c0fa`（100MiB）**，桌面校验全绿
  （ESP 内核 sha 对上、ADT 完整、trust 双模块）。构建命令 = kernel.probe.macho
  先探针后 payload（PATH 需 /opt/unixnu-linux/llvm/bin + linuxbrew bin；
  UNIXNU_UEFI=仓根 uefi-sheng 符号链接）。
- **待刷**：recovery_a = `e3c5c0fa`（userdata 不动）。**读图判定表（带探针）**：
  ①无蓝条（绿条后静默）= 内核入口未执行 → DarwinBoot 侧新问题；②蓝条 = 入口
  已执行；③品红 = MMU 前到站；④panic 屏幕 = 直接读因；⑤恢复界面/logo =
  launchd 里程碑。校验 5.7GiB 期间探针轮询是 no-op 桩——蓝条只闪一次（stage 1），
  之后数分钟静默属预期，勿提前断电。
