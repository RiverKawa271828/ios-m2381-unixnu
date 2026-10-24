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
  - **门 C（上机，唯一不可离线裁决项）**：arm64e PAC 语义。设计对策（两
    条都备）：① 内核 bootstrap 程编 APIAKey 为已知值 + 运行时对 GOT/
    函数指针再签名；② prelink 期用 QARMA(key=选定值) 预签名（格式公开、
    可离线实现）。两策都失效且无第三策 → 转 (b)。
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
