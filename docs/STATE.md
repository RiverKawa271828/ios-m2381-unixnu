# STATE — 现势状态（m2381，2026-10-28 更新）
## 0. 现势指针（每轮首更，2026-11-05 会话五十五·墙 55 定谳 + 手术 r56 出件待刷版）

- **★会话五十五 = 墙 55 定谳（自设门，非 donor 墙）+ fips_mode 撤除 + 外科手术 r56 出件待刷（2026-11-05）**：
  **r55 首刷判读**：t=1s，stage 至 mapping_adjust 后 panic =
  `source corecrypto verification requires complete default POST
  @prng_random.c:665`，crc `0x27f71c47` 零纠正命中（plain
  crc32("message @file:line") 实证）。**病灶 = 我们自己的
  `source-corecrypto14-bootstrap.patch` 门**（STARTUP SYSCTL
  RANK_LAST，在 provider start 之前）：`PE_parse_boot_argn("fips_mode")
  && mode != 0 → panic`——s54 加的 `fips_mode=4`（FIPS_MODE_FLAG_DISABLE）
  正是该门禁止项（系列契约 = no skipped POST；PRNG 上下文登记依赖 POST
  完成态）。**r55 比 r54 死得更早**：fipspost_post 根本没跑，s54 四槽
  重绑（0x84ac980→0xaefbfdc 等）仍未上机验证。修法 = **board.toml 撤
  `fips_mode=4`**（设备仓，连同其"kpis bl retarget 在哈希段内 HMAC 永
  不匹配"的过时注释一并改写：emitter 换道后 TEXT_WORD_PATCHES 已清空、
  FIPS 哈希段 [0xac12c00,0xac818b0) diff=0，完整 HMAC 可过；DISABLE 只
  是防 KAT 未知的保险，被门判非法）。
  **r56 出件遇硬阻**：Linux out 树 kernel = r52 旧件 cfbe9b25；s54 两
  次 scp（temp/s54/kernel.macho=`3d158c4d`、kernel-r55.macho=`e64448dd`）
  走本地 payload 步骤封印后 HMAC 均为 `61f9ca2a…` ≠ r55 实际封印件
  `b2f48b39…`，且 0x3eeb64c..0x3eebfdb ~1.4KB 指令级差异（编译级）——
  **r55 的父内核 X = 未回传第三状态**（Mac payload 出件时间早于两次
  scp，X 仅存于 Mac out 树）。**外科手术定案**：对
  `payload-r55-dd51ff60.img` 全图 FAT 手术，仅重写 unixnu.toml 活动簇
  （dir entry 簇 94598 = 0x5D29800，` fips_mode=4` 移除 562→550B + 尺寸
  字段）——全图 diff 166B 全部落在 toml 簇 + size 字段内，**内核
  （封印件 787778cb）、ADT、Data seed、HMAC 报告与实机在跑 r55 逐字
  节相同**。四门验证（mtools 读回 / 7z 逐件 / 全图 diff 定位 / sha）。
  **r56 出件**：`out/m2381/payload-r56-94a096c3.img`（99 MiB，sha256
  `94a096c3f52bf242…40e1cc7`；payload.json 已写手术 provenance）。board
  已改，**后续正式出件必须重走 Mac（rsync 后本地树重出）或 Mac scp 新
  kernel**。屏幕显 args 与 loader 实际一致（内核内嵌 args 串随封印件保
  留 r55 原文 = 判读时屏显即实证）。**预期推进面**：门过 → donor
  corecrypto kext start → **fipspost_post 完整 POST 首次实测**
  （mach_absolute_time 槽 + HMAC 段 diff=0 应过；KAT 面 = s54 挂账未知
  墙，踩到按哨兵流程定名）→ PRNG 登记 → PE_init_iokit → AMFI（站表
  0x924e77c/ef1c/ef5c shim 预案在案）。成功旗标不变：vfsinit →
  root/cryptex mount → SHLOG state=2。回退：recovery_a ← r55
  `dd51ff60`（当轮在跑件）/ r54 `df033828` 二级。


- **★会话五十四 = 墙 54 判读定谳 + 四 records + r55 出件待刷（2026-11-05）**：
  **r54 首刷**（crc `0x196bb370` 零纠正一次命中 len=124B，t=1s，stage 到
  mapping adjust）= from `0xac1536c` → bl `0xac15368` → thunk `0xac819b0` →
  auth-GOT 槽 `0x84ac980`（rebase `0x8011|0x3ecaf24`）→ donor **`0xaecef24`
  = `_mach_absolute_time`**；caller `0xac15330` = corecrypto kext
  **`fipspost_post(fips_mode=0, mach_header=ki->address)`**（x1 =
  `0x7dc74e0` = corecrypto entry 基址；`static corecrypto_kext_pmach_header`
  记忆化体与 fipspost_post.c:106 逐句同源）——首调 =
  `cc_absolute_time()`（cc_absolute_time.h CC_KERNEL 分支）。
  **方法修正**：s53 "余 6 桩零调用点"系线性反汇编数据池失步漏扫（thunk 簇
  0xac818b0..0xac81a80 在线扫失步区）；word 级重扫 = 7 桩目标 bl 站
  **125/1/1/1/1/1/0**。**四 reviewed records（144→148，设备仓 62935f9）**：
  ①`0xaecef24→_mach_absolute_time`（125 站 = 最大剩余 import 面）；②
  `0xad43134→_lck_mtx_alloc_init`（槽夹逼 lck_grp_alloc_init/mtx_assert +
  KT_LCK_MTX 分配体 + caller pacda 签回锁指针）；③`0xad43db4→
  _IOLockTryLock`（槽夹逼 IOLockLock/IOLockUnlock + casa-percpu 体形）；
  ④`0xb42b4a0→_register_crypto_functions`（register_crypto.h:284，**翻案
  s53 "无孪生"**——源码级孪生在，noctf 实证）。**撤 wall-53 kpis-publish
  NOP**（TEXT_WORD_PATCHES 清空）：实测 emitter 机制 = bl 保持指 donor
  thunk、槽重绑 native → **FIPS 哈希段 [0xac12c00,0xac818b0) 文本 diff = 0
  （donor 逐位还原）**。**board.toml kernel_args + `fips_mode=4`**
  （FIPS_MODE_FLAG_DISABLE）：完整 HMAC 虽已可过，POST KAT 面仍有未知墙，
  DISABLE 让 fipspost_post 过 mach_absolute_time 后立即 CCERR_OK
  （PE_parse_boot_argn r53 已绑可解析）。**留桩 3**（故意）：
  0xb441a24（1 参 kalloc_data 形态 flags 0x31000 烘死，我们 2 参无安全
  孪生）/ 0xae9bd20（零直调）/ 0xb441b54（死 thunk）。
  **r55 出件**：`out/m2381/payload-r55-dd51ff60.img`（payload `dd51ff60` /
  kernel `e64448dd`，99 MiB，payload.json kernel_args 含 fips_mode=4 实
  证）；stubbed 1024→**1002**、corecrypto 7→**3**；四槽重绑字节级实证
  （0x84ac980→0xaefbfdc / 958→0xaceffe8 / 970→0xacf0368 / 9e8→0xb3fdb18，
  verify-r55.py 四门）；Mac 全链（defconfig→tree→kernel→payload）BUILD
  EXIT=0（~/unixnu/s54-build.log）。
- **★下轮工作单 = 会话五十五 r55 首刷判读（runbook）**：0. 收屏（屏态永
  非判决）。1. 判读主线：a. unmapped-call hit → `verify-panic-crc.py` 对
  拍 → from−4 = bl 站 → 对号 kernel.json（stubbed 1002 面，donor 坐标）
  → 三证定名（word 级 thunk 扫描法，勿线扫）→ 通道。b. **预期推进面**：
  fipspost DISABLE 过 → corecrypto_kext_start 尾段（kpis publish 已通/
  sysctl_register_oid 31+13 面/register_and_init_prng 已配/cc_kext_random
  _init）→ 新墙按站分流。c. FIPS POST Failed（不该发生：fips_mode=4 在；
  若现 = boot-arg 解析问题，查 ADT chosen 传递）。d. AMFI No-PMGR（t≈32s
  IORTC 税面）→ PENDING 站表 0x924e77c/0x924ef1c/0x924ef5c → shim 三条目
  → r56。2. 成功旗标不变：vfsinit → root/cryptex mount → SHLOG state=2。
  3. 回退：recovery_a ← r54 `df033828`；w42fix `3fe101ba` 二级。
- **★会话五十三对齐 = payload-r54 出件（2026-11-05，已刷已判 = 墙 54）**：
  `out/m2381/payload-r54-df033828.img`（99 MiB，sha256 `df033828fd9fec1f…`
  双端同；内嵌 kernel sha `2221b0d2…`）。本轮 = "跟着上游对齐"落地：
  上游 24A446 = **同源 xnu-12377.121.6**（Darwin 26 源码未发布，谁都无法
  源码换代）+ series 逐字同 + kit 同 commit b1888c1；**真缺口 = Mac 从未
  构建 RELEASE fileset reference**（26.5 车道 session 43 同坑重演，参照件
  一直是 Linux 建的）。修复链：①正确通道 = `XNU_KERNEL_CONFIGS=RELEASE
  ./unixnu kernel --skip-prelink`（builder 带 defconfig 环境，我两次绕过
  builder 手搓 = 丢 `XNU_KERNEL_CHAINED_FIXUPS`=1 → ld-classic 缺失假象）
  ②baseline UBSan stub 缺 `__ubsan_handle_negate_overflow_minimal`（nm 扫
  组件档实测三缺一，已补）③CROSS_CLANG/DARWIN_TRIPLE 两个 Linux 专属变量
  Mac 上需显式补（homebrew llvm + arm64e-apple-ios27.0）。
  **r54 emit 基线（reference 元素首秀）**：reference-symbol 1489 +
  reference-order 4334 anchors，functions 8215→**9468**，stubbed
  1262→**1024**，missing 320→**286**；regression policy 已按新观测面重生成
  （missing 149/stubbed 1024 sites，IOAVFamily 0xb92ab20 = reference 重配
  后如实缺失，旧面疑似误配）。字节复核：NOP 0xac726e8 在位 + kprintf 槽
  0x84ac948→0xb514cb8（reviewed 绑定在 reference 重配下幸存）。提交：主
  树 baseline+policy+STATE，设备仓 kit 本轮未动（fb8af03 仍为头）。
- **r53（无 reference 过渡件）**：payload-r53-47859083 已出已验证，被
  r54 取代，留档不刷。
- **会话五十三判读档（墙 53，已执行移档）**：r52 首刷 = corecrypto kprintf
  槽 0x84ac948 漏配全案 + kpis-publish NOP（本轮已翻案撤销）→ 全文在
  `docs/archive/STATE-history.md`（2026-11-05 会话五十四移档）与记忆
  `s53-r52-firstflash-unmapped-wall`。
- **换道出件完成（2026-11-05，会话五十二）**：os_profile →
  `ipados-27.0.1-24A446` 全链走通，**payload-r52 已出待刷**：
  `out/m2381/payload-r52-773d5578.img`（payload sha8 `773d5578`，kernel
  `cfbe9b25`，99 MiB ESP，Mac 出件后 Linux 拉回 sha 逐位同）；
  check-m2381 **4 ok 0 fail**；emit 报告 missing 320 / stubbed 1262 =
  政策基线吻合，adapter branches 0。首刷判读 = **会话五十三**（recovery_a
  only + `set_active b`；回退一级 = `payload-w42fix-3fe101ba.img`
  `6436442d`）。
- **★对账重大修正（板级层）**：上游实证 24A446 车道 = **arch series +
  UniXNU-sheng 板级 kernel patches（10 条）+ 板级 package capture 面**——
  侦察档"arch 68 条全有/零 we-only"口径漏了板级层。**已收编 4 条自洽件**
  （source-storage-runtime + latency-trace/exit-latency/unnest-trace 三
  hooks，定义 sheng_user_fault/exit/unnest_hook 指针）；**bringup 主件
  (2006 行) 未收编**：其 kern_exec 面在 sheng 24A446 树上重写
  （AGXMetalG17G/壁纸 27 变体），与我们 23F84 时代 zzy-* patch 同功能区
  fork 分歧，patch 级不可落（git apply/-3/patch --fuzz=3 三级 fallback 全
  试，kern_exec hunks #4-8/#11 真冲突）——**fork merge 挂账首刷证据后做**。
  prepare 新增 `git apply -3` → `patch -p1 --fuzz=3` 两级 fallback
  （3e6b3ca/378a922）。
- **步 3 终版结论（7 条 we-only）**：六条退役维持（23F84 墙修 ≠ sheng
  板级修），**prng 保留**（Xcode 27.2 转换税 + 0xbcad 24A446 三站同构实证
  donor-va ≈ 0xaf73560/0xaf74ef8/0xaf75868）。TEXT_WORD_PATCHES 清空（4 条
  23F84 钉全失效）。
- **verified-root-24A446 政策对（换 donor 隐性硬前置）**：Mac userdata 出
  件（AEA 链 = blacktop `ipsw fw aea --key` + `/usr/bin/aea decrypt`；ipsw
  CLI 3.1.741 已 brew 装机），OS 9,613,344,768B sha `8a12cdad` ✓ /
  SystemOS 6,016,729,088B sha `d5b4873d` ✓ = profile [root] pins 逐位同；
  政策对 = 通用化 build-owner-policies-local.py（窗口/pins 自
  userdata-root.json + profile 读），RSA 全块 **OS 586,752 + SystemOS
  367,232 全 PASS**。
- **emit 布局定谳**：24A446 kernel-entry __DATA 窗 0x1e8000（23F84 =
  0x22c000），native __DATA 放置其内；越界 0x1c000（撞 Libm 等四条
  retained __DATA @0xba1c000）→ **bp 采样环 4096→1024**（省 120KB）。
  ESP volume 96→99 MiB（24A446 payload 92 MiB 顶 95% 门）。
- **iOS27 源码面补齐**：BootProcessProbe/Worker/BootUiTraceProbe +
  BootLatencyTrace 家族 verbatim 收编；BootWatchdogPolicy/BootDiagExport
  双载 → build-boot-progress.py 门扩展 + TelemetryDefs 预处理门；
  ShengLatencyHooks.c 只留 native_gpu_role_thread（sheng 车道也无定义 =
  Apple ld 绑零；我们 ld64 拒 weak-undefined）。
- **参考链**：kit gitlink `b1888c12` ✓；RELEASE 参照件 Mac `5a794da8` ✓；
  trust bundle `dc655bcb` 双端 ✓；IPSW `fa93b653` 双端 ✓；kernelcache
  `38e6d9fa` ✓（profile 硬门）。
- **未推**：主树 `139c7c5` `beebdd8` `c97e6a2` `556b847` `d4766d1`
  `3e6b3ca` `378a922` `886eeee`；设备仓 `1464ec1`…`52d8e01`（12 笔）。
  Mac = rsync/scp 全量同步 ✓。
- **★首刷前提前项（2026-11-05 收队轮）**：①步 0 闭合——w42fix 屏墙
  donor 0xac8f594 定名 = **ktriage register-subsystem-strings 族**（范围门
  [0xa,0x12] = subsystem id；native `_ktriage_register_subsystem_strings`
  0x7834074 同形），24A446 AMFI 2661 bl 站扫描**零复现**（此墙新车道不
  在）。②wall-42 24A446 站表备妥（PENDING）：AMFI dev-mode 函数
  **0x924e7b0**（0x498B，/arm-io/soc-misc@0x924e800 + ApplePMGRNub
  @0x924e92c 同函数）三 caller 站 **0x924e77c/0x924ef1c/0x924ef5c**，与
  23F84 三站同构；首刷若撞 No-PMGR assert = TEXT_WORD_PATCHES 三站→
  shim 一重建一刷。③bringup fork-merge 侦察定谳——36 段中 35 段 per-file
  可落但实跑**多文件真分歧**（exception.c/iokit_rpc.c/pmap.c/proc_reg.h/
  vm_dyld_pager.c/sched_clutch/vm_param.h = 我们自己的 4k/sandbox 链改同
  行），patch 级不可落；**series 保持 r52 基线**（bringup-core 两次摘除，
  6bd09d3）；merge = 首 刷证据后按缺失面逐文件手工工程（设备仓
  e479869 普查账）。
- **★下轮工作单 = 会话五十三 r52 首刷判读（runbook，2026-11-05 定稿）**：
  0. **工具前置**（判读开始前一次）：`scp` Mac 侧 24A446 三件回 Linux
     判读工作区（`out/m2381/ipados-27.0.1-24A446/m2381/kernel/kernel.macho`
     + `tree/build/xnu/ios-obj/DEVELOPMENT_ARM64_BCM2837/mach.development.
     bcm2837.unstripped.noctf` + `kernel/kernel.json`）；`tools/xnu27/kc.py`
     路径参数化（DON/OUR/NAT 现钉 23F84，SLIDE `0x1f000000000` 不变——
     13 跑 record 验证同值），判读站坐标 = donor 空间口径。
  1. **收屏**：刷机结果屏摄（挂 = panic 屏；进 = 启动面/黑屏，黑屏不阻塞
     ——屏态永非判决）。
  2. **判读主线（分流）**：
     a. `verify-panic-crc.py` 对拍（assert 配方 `"MSG" @%s:%d` 已校准；
        24A446 PANIC_TEXT 布局/映射若变 = 会话 49 屏映射法重对 LC）；
     b. panic 站解码 `kc.py wall SITE`（donor 空间；屏地址归属先对 LC
        判 native/donor——49 课）；
     c. **AMFI "No PMGR?"（第一预期，预案已备）**：ConfigurationSettings
        :388 同族 → 启用 PENDING 站表 `0x924e77c/0x924ef1c/0x924ef5c` →
        `_bl_to_native('unixnu_amfi_devmode_config')` 三条目入
        TEXT_WORD_PATCHES → 一重建一刷（r53b）；
     d. unmapped-call 哨兵墙 → 对号 kernel.json missing 面（320 站基线
        已录）定名分流；
     e. 新墙 → 三证定名 → 通道选择（内核目标=fixture / kext 站=TWP）
        → spec → 一重建一刷。
  3. **成功分支旗标**：native pac-keys → IORTC 税（t≈32s，clock_initialize
        _calendar 面 = 23F84 墙 41 后继形态待验）→ vfsinit → root/cryptex
        mount → **SHLOG 窗 state=2 = 大捷旗标**（24A446 root layout 才
        开窗）→ launchd/SpringBoard 站。
  4. **判读后工程**：AMFI 站表启用出件；bringup fork-merge 按缺失面
     证据逐文件立项（设备仓 e479869 普查账）；PMGR 包重取证（仅当
     AMFI 等待面实锤需要 nub 路径）。

## 1. 一句话现状（**2026-10-31 会话二十四收档：23F84 fileset 车道，matcher v11 判别子通道落地，战略拍板 = 关键路径优先**）：

**车道**：本机 Linux 全链出件（`./unixnu kernel|payload -D
../UniXNU-m2381`），donor = iPad17,1-4 / iPadOS 26.5.2 `23F84`
（xnu-12377.122.8 fileset KC，298 entries），native = 自建 xnu-12377
（arm64e PAC）。**内核已活**：每次刷机内核都在执行，panic 屏 =
fileset 未配调用的报告哨兵（fileset-unmapped-call.patch），现卡在
**AMFI early_security_init（root mount 之前）**。

**账本**：stubbed 654 目标/4848 站 → **601/4298**（会话二十四，
matcher v11 `pair_family_by_vtable_slot` = 槽判别子签名通道，69 配对
全语义自洽，设备仓 d0b94c0）；kext-facing named 1685→1736。
**战略拍板（2026-10-31 用户定"只想起内核→进桌面→完善驱动"）：
关键路径优先，不再追账本清零**——只修"到 root mount 为止"实际执行
路径上的墙，一刷验收整段；账本其余降级为背景（B.2 续单挂起）。

**最近墙**（按调用序出队；判读惯例 = CRC 验真→payload ESP 提
kernel→站点反汇编→batchid/vtslots 夹逼→三重验证）：第十六堵 =
SecureDTLookupEntry（已配）；会话二十四末新屏 = **0xafc09d8 =
SecureDTGetProperty**（9 站，AMFI /chosen 属性读取；donor 独立函数、
native 内联无独立符号，需 ~30 行 shim）。规矩：**关键路径上的坐标
当场修，路径外一律只记坐标**。

**出件**：kernel `18affaa2` / payload `936cd505`
（`out/m2381/payload-strncasecmp-936cd505.img`，fixture 166 笔；
ledger 3082→**3048 站**/847 目标 = 墙 21 −34 精确）**待刷**。
刷写红线不变：只 recovery_a；boot_b `d0200a6b` /
userdata `fd43ee70` 不动。下会话工作单 = §7b（墙 21 判读单已毕，
待刷后按调用序出队）。

## 2. 设备现状（2026-10-31 = **iOS 链现役**）

| 分区 | 件 | sha 前 8 | 状态 |
|---|---|---|---|
| boot_b | 本仓 UEFI（23F84 线配套） | `d0200a6b` | **现役**（不动） |
| recovery_a | payload vtslot | `2b8b8258` | **现役**（会话二十四已刷；会话二十五出件 `e5e9556f` 待刷换防） |
| userdata | CI 重建 iOS root | `fd43ee70` | **现役**（未动；源件 = `out/m2381/ci-artifacts/userdata-root.img` 8.8G，sha 同，勿删） |

- **读窗 = forensic reader**（专用件 `payload-reader-0f1013f4.img`；
  23F84 payload 本体 forensic=false 无 reader——ESP 超 recovery_a 已摘
  读者三件）。流程：fastboot → 菜单选 "m2381 forensic log reader" →
  `acm-reader.py probe/cmd/fetchchunk` → `read-m2381-ufs-log.py`。
  recovery_a 节点 = **sde27**（勿按 sda 猜）；ACM 通道推大文本会丢
  字节，一切比对走设备端 sha。
- **发卡规矩**：①发卡前确认设备现态（nura 态 = 三件全刷；iOS 态 =
  只刷 recovery_a）；②payload 只认平铺路径
  `out/m2381/payload-<名>-<sha8>.img`；③卡上必带刷前 sha256sum 门，
  dd 读回对拍才算证据。
- **ABL 重试预算**：同槽连败数次自动落 fastboot；恢复 =
  `fastboot --set-active=b`（留在 b、重置预算）。

## 3. 现役件与资产

- **构建**：`./unixnu kernel|payload -D ../UniXNU-m2381 -j16`（-D 在
  子命令后）；改 defconfig 必重跑 defconfig；改设备脚本必重跑 tree
  （staged kit = 设备仓 `kits/XnuPrelink` 的拷贝，改 matcher 后
  tree+kernel 即生效，map 按 matcher_signature 自动失效重建）。
- **payload 四元**：kernel `876a14c7` / trust =
  `out/m2381/ipados-26.5.2-23F84/m2381/original-trust-23F84.bundle`
  （ebeaf7fb）/ 种子 =
  `out/m2381/ci-artifacts/verified-root-grow/owner-data-fixture/scratch-data.img`
  （d04a0153，勿删）/ `--unique-chip-id 3645ad55d47e77c1`。kernel_args
  = `-v serial=3 debug=0x14e kt_fixed_zones=120 -no_slto_panic
  use_contiguous_hint=0 native-source-storage=1 native-ufs-root=1
  noidle=1 boot-progress=2 sheng-log=1`。panic screen + CRC 行上岗
  （zlib 0xEDB88320 over PANIC_TEXT 消息副本，判读先复算再钻）。
- **配对**：matcher v11（设备仓 `kits/XnuPrelink/fileset_match.py`，
  单测 27 例）；fixtures = 同目录 `fixtures/fileset-23F84-symbols.json`
  （137 函数 sha 钉）；批次工具 = `temp/batch-fixtures/`（batchid.py
  夹逼 / vtslots.py 判别子签名）。RELEASE 参照件已内建于 kernel 步。
- **donor**：iPad17,1-4 / iPadOS 26.5.2 `23F84`（IPSW 本地已核验
  `out/ipsw/23F84/`，全 zip sha f7f55fa3…，**留着勿删**）。
- **device repo**：`../UniXNU-m2381`（os_profile = ipados-26.5.2-23F84）；
  **UEFI**：`../uefi-m2381`。
- **诊断工具**：设备仓 `scripts/re-tools/`（cachesym/symbolicate/
  disasm…；cache 目录 `UNIXNU_CACHE_EXTRACT` 可覆盖，23F84 用户态
  符号化时需从 SystemOS DMG 重建 cache-extract，步骤见 memory
  dyld-cache-symbolication）。
- **阶段②预告（显示/驱动）**：=y LegacyDisplay 车道组件 = 主树
  `drivers/GopFramebuffer` + 设备仓 `scripts/build-native-legacy-display.py`
  （21H461 期施工，换代重生成见 docs/PLAN-display-lane.md）；GPU 固件
  已按 m2381 pin 落设备仓 `.cache/firmware/m2381-gpu/`（nura 侧供给
  实证，zap/sqe 与 sheng 件不同勿混用）。

## 4. 方法资产（反复要用的）

1. **fixup 惰性应用判活器（r49 定谳）**：text_fixup 挂在
   `vm_page_validate_cs_mapped_slow`，含目标地址的页首次签名校验时才
   patch ⇒ **applied 标记 = 函数真在执行路径；零应用 = 不在路径**。
   不应用不等于修复失败，可能只是目标函数没被执行。
2. **dyld 缓存符号化**：CRASH 行 `+N` 去滑；**先读 REASON 行**（SIGABRT
   的 PC 停在 abort 链，给不了抛点）；bt 帧序内→外，live 判据 =
   **bl-target 校验**（LR 落点必须是真调用指令——r47 修正 r46 的"陈旧帧"
   误判：位置不预示真伪，bl-target 才是判活器）。工具已收编入设备仓
   **`devices/m2381/unixnu-m2381/scripts/re-tools/`**（cachesym /
   symbolicate / verifychain / disasm / find-string-xref + README 坑
   表；cache 目录默认 `temp/cache-extract`，`UNIXNU_CACHE_EXTRACT`
   可覆盖）；全步骤见 memory `dyld-cache-symbolication`。
3. **chained-fixups 指针编码**：用户态 `__DATA_CONST` 存储值 =
   `VA | 0x400_00000000`（裸指针搜索必零）。
4. **扫描器坑**：ADD 立即数掩码必须 `0xFF800000 == 0x91000000`
   （0x7F800000 丢 bit31）；xref 必须**全区扫**（跨段引用 >±24MB，
   r57 实证 XBApplicationController 断言块距其 CFString 池 >400MB）；
   字节-pattern 扫描**不可把 rd/rn 位写死**（`add x4,x4,#0x928` 的
   低字节 = rd/rn≠0，pattern 若按 `0000` 编码全零命中，r57 实证）；
   **regex 必须逐字节 re.escape**（未转义时 0x24 变 `$` 锚点静默零
   命中，r59 实证——修正后先拿已知引用点验证扫描器再信零结果）；
   re-tools `find-string-xref` 的 rn==adrp_rd 判定本身没错（真引用
   生成器满足），错在窗口——大跨度引用一律手写全区扫。
5. **CFString 定位路径**：REASON 串 → 裸串/CFString 候选 →
   `bl _objc_msgSend$handleFailure…` 桩（各二进制私有）→ 断言站点
   x6/x7 串回读。
6. **defconfig 铁律**：改 defconfig 后必须手动重跑 defconfig 命令。
7. **bt 用户帧可信度分代（r52 定谳）**：=n 时代 bt 用户帧多为陈旧帧
   （r50 实锤，live 链只有 terminate 机器）；=y 环境 r52 的 bt 用户帧
   首次全活（bl-target 三帧全过）。**无论哪代，bl-target 验活必做**；
   去滑先核 N 值（r48 帧列表曾有 0x10000 转录滑差 → 符号落 HomeUI
   垃圾 = 红旗）。
8. **r73 断言-串定位三坑（全部当日付费）**：①`find-string-xref.py`
   的 needle = 串尾紧随 NUL——短语式子串搜索恒 0 命中；找串本体要
   用 Cache 直搜（data.find 不带 NUL 要求）。②xref 全区扫必须重叠式
   判映射（`hi<=addr or lo>=addr+size` 跳过）；按映射起点写
   `addr >= lo` 过滤会把起点在 lo 之前但覆盖 lo 之后代码的整段截掉
   （r73 因此漏扫 .25 全段）。③引用模型三件套 = adrp 配对（相邻或
   宽窗，**add 判定掩码必须 0xFF800000，见 §4.9**）/ 绝对槽（低 48
   位掩码 + bit42 tag；r64 链格式：高位 16 = next-delta）/ i32 相
   对指针——**"三者全零 ⇒ raiser 在缓存外"这类推断必须先过阳性对
   照**（r74 教训：掩码 bug 下三者全零是假的）。**一切地址算术必须 python**（r73 一
   天手算错两次：imm12 0x368 vs 实际 0x478；b1d15f9000 去滑
   0x1cf3039000 vs 实际 0x1c379d000）；name→VA 反查走 symbolicate46
   同款解析（dec-symbols 头 @0x48 → 6×u32 → nlist 二分）。
9. **r74 add 掩码三犯 + 阳性对照铁律（当日最贵）**：①adrp 消费者的
   add 判定必须 `(w2 & 0xFF800000) == 0x91000000`（或
   `== 0x11000000` 用 0x7F800000）——`(w2 & 0x7F800000) ==
   0x91000000` **恒假**（bit31 被 0x7F800000 丢掉）；这正是 §4.4
   r57 已入档的同款坑第三次复发，r74 当天从首个脚本传染到全部
   xref/全缓存/阳性对照扫描，制造出"raiser 在缓存外"的假结论直到
   正面 disasm 真抛点才证伪。②**零引用结论必须先过阳性对照**：拿
   一条"确知当轮在触发"的断言串（r74 用 BaseBoard 的
   NSParameterAssert 格式串 CFString，PB 断言每轮必经）走同一管
   线——阳性对照零引用 = 工具箱有洞，任何"缓存外/死拷贝"推断全部
   作废。③正面 disasm 已知抛点函数（r72 钉死的 once block）是终
   审手段：机制用眼睛看，不靠扫描器间接推断。


## 5. 开放问题

- **root mount 前关键路径**（阶段①主战场）：AMFI early_security_init
  调用图上的 stubbed 目标（首个 = SecureDTGetProperty shim，§7）；其后
  apfs / IOStorageFamily root-mount 段未走到过，可能有新坐标。
- **root mount 后**：用户态（信任缓存/Data 卷/launchd）无先例区；
  donor 用户态与 donor 内核配套（23F84 全套），理论顺于老 20H392
  跨代错配，但纯探索。
- **B.2 续单（非关键路径，阶段②前按需）**：co-slot 群一致性通道
  （先配 IOUserClient::init 0xaf4e544 → 伴随槽锁类 → 收编 Dispatch×48
  + init×47×2）；大函数残墙放宽（0xaeb8eb0×190 copyProperty 候选、
  0xa95ca30×127、0xaf6236c×92）；0xafc1b20 PE 小壳身份。
- **GPU 接装**（阶段②）：固件/OPP/bus 表已备（§8 会话十八续档），
  缺 votes 头/配方去 xcrun/DTS/defconfig——显示破墙后主题化。

## 6. 不能声称的

- root mount 未通过：仍卡 AMFI early_security_init（panic 哨兵按调用
  序出队）。用户态未到达：信任缓存/Data 卷/launchd 全部未实测。
- 桌面/显示：=y LegacyDisplay 车道停留在 21H461 期（一级已实机实证但
  main display 注册链未通），未在 23F84 车道重验。
- matcher v11 通道只覆盖纯转发壳族；账本 601 目标大多数未配且**不构成
  当前目标**（关键路径优先拍板）。

## 9. 工作单 = 会话三十九（Mac 件首刷判读）+ §10 工作单 = 会话四十（crypto-funcs-unpack v2）——均已执行，全文移 `docs/archive/STATE-history.md`（2026-11-03 会话四十三按文档卫生常规移档）。

执行档副本：§9G = Mac 件首刷墙 30 判读（判读全文在 archive + git commit）；§10.X = crypto-funcs-unpack v2 出件（payload 24f0a941，已刷已判 = 墙 30 过，见 10.Y）。

## 7~10.Y.10 批量移档（2026-11-04 会话四十五收队，文档卫生常规）：§7（会话二十七
工作单）、§8 车道判读档案（会话十三→二十四）、§7a~§7i + §7j/§7k（会话二十八~
三十八工作单与执行档）、§8'（会话三十六工作单）、§10.Y~§10.Y.10（会话四十一~
四十五全部开工单与执行档）均已执行，全文移 `docs/archive/STATE-history.md`
（2026-11-04 批次，原样保留）。正文只留现势区（§0~§6）、§9 存根与本轮
10.Y.11。

## 10.Y.11 会话四十五续 = 墙 36 首刷判读 + 墙 37 施工档（2026-11-04，
kernel `6d4a9f19`/payload `e6b06399` 待刷，屏摄设备仓
records/mackernel-w36fix-b9e733be-firstboot-20261104-020733.jpg）

- **墙 36 修复真机生效（fixture 判读通过）**：旧报点
  `0x…977dd00` 不再报；stage 轨迹保持全亮（PE_lockdown_iokit → … →
  kernel do post - done）；pages=0x125b07 / free=0x1251c7（+1 同水位
  = 走过旧墙后即刻撞下一站）；hit #1。
- **消息定稿**：crc `0x1fa14f06` 一次精确命中（591,872 候选唯一命中，
  **零纠正**；配方先复现上轮 0xee584f8d 自校通过）。x0 =
  `0xfffffff7863fe090`（f7 前缀，非上轮 f8 heap 段）；x1 =
  `0xf5db14b0097807a0`（PAC tag 指针）。
- **新墙 37 = 同族 unmapped-call**：报点 LR `0x…977fefc`（bl 站
  donor `0x977fef8`）；caller = **CoreAnalyticsFamily** start 装配
  大函数（`0x977fd14..0x97802ac`，1432B；与旧墙 36 caller 同 kext
  不同函数且更晚）；target = donor **`0x0af0f77c` =
  `IOTimerEventSource::timerEventSource(OSObject*, Action)`**（二参
  工厂重载；j 重载已排除：常量 1 作 init timeout + caller 只传 2 参）；
  构造链叙事完整：基类 ctor（墙 36 已修）→ workLoop → commandGate →
  timer 工厂（墙 37）。未配成因 = donor 体内联差异致 body-hash 失败
  且无字符串/vtable 证据 = matcher 正常漏网（非"已配却报"）。
- **红利面 = 321 直调 bl + 100 fixup 槽（约 100 entry，
  **最大单笔 fixture**；榜：IOAccessoryManager 26/AppleHPM 19/
  ApplePearlSEPDriver 19/AppleSPU 10…）；CoreAnalyticsFamily 自身
  4 站，同 caller 内还有 0x977ff40/0x977ff88 两站（钉名后同函数继续
  推进，预期行为）。
- **墙 37 fixture 落位**（设备仓 `bdd72fd`）：↔ native
  `__ZN18IOTimerEventSource16timerEventSourceEP8OSObjectPFvS1_PS_E`；
  native 地址 = Mac noctf（7307aecf）`0xfffffff007aab9e0` / Linux
  `0x07aeedbc`；四门双车道全过（donor 区间 size=284 sha 一致；双
  reviewed；make_fixture check 255/255）。native 形态逐项同构（0x60
  typed alloc/vptr 0xcda1/instanceConstructed 内联/vtable[+0x140]
  PAC key 0x5f4/失败路 0x3a87）。
- **Mac 重建 + 静态门**（增量重编 ~4min，ninja 热缓存；**本轮零锚
  漂移**：native 未变，reviewed 0 drift）：kernel `6d4a9f19`/payload
  `e6b06399`，存档 `out/m2381/payload-w37fix-6d4a9f19.img` + kernel
  同名存档。w35-gate 双件 PASS（resign 0/dispatch 371）；map 9896 对
  （+6：reviewed +1 + auto 回收 5；kext_targets_named 2048→2051）。
- **站点重定向 + 红利静态对账**：新件 bl `0x…977fef8` →
  `0x0af1f9e0` = native timerEventSource 终局地址（noctf 原体骨架
  0x60B 归一化逐位同）；直调 bl 实换靶 26 站（哨兵 1228→1203）；
  closure_pointers_unmapped 314→**311**（−3）、kext_pointers_rebound
  47493→47496（+3）；branches/kept 持平 = 零附带漂移。其余 321−26
  直调站在被 drop 的 entry 内。
- **回退**：recovery_a ← `payload-w36fix-b9e733be.img`（ce3df35f）；
  b63ad26e 二级；abe3ec85 三级。

## 10.Y.12（会话四十六，已执行移档）→ 全文在 `docs/archive/STATE-history.md`

## 10.Y.14 会话四十八 = 墙 40 定谳 + 一字补丁修复出件（2026-11-04，Mac ssh 全程，未刷机）

- **判读**：屏摄（IMG_20261104_061101）= PAC failure from kernel with
  IA key while branching to x8 at pc `0xfffffff0084ca4fc`，lr
  `0x62df8a700aeea59c`（签名值），esr `0x72000000`，stage 全亮维持
  （kernel do post - done 后），pages `0x125b08`/free `0x1249c8`。
  CRC 复算 = 多行 PAC 新格式未覆盖（见 §0①），**结构验证法**定谳：
  donor `0x84ca4fc: blraa x8, x17` 与屏报逐字吻合。
- **机制（全链实证）**：`start()+0x48` = `adrp g 页; ldr [g,#0xfd8];
  add x9,x8,#0x2c0; ldr x8,[x8,#0x2c0]; mov x17,x9; movk x17,#0x3c68;
  blraa x8,x17`。donor 全镜像扫描：g 全局唯一读者 = 本站、表页
  donor 文件值全零（BSS）、零 fixup、零写入者（kernel+kext 全部
  298 entry fixups 解析零命中）——**表 = 混合内核的 native
  __DATA_CONST**（kernel entry = donor text + 我们 data）。final 页
  节点 = auth IA + addrDiv、div=0x4529、target=final `0xaee785c` =
  noctf `0x7a7385c` = `__ZN9IOService4openEPS_jPv` twin（donor
  `0xaec90cc`）。donor 读者标签 `0x3c68` vs 我方槽 div `0x4529` →
  FPAC。两标签在 donor movk 站点各 254 处 = 工具链标签漂移
  （donor 23F84 clang vs 我们 26.5.2 构建）。**移位 +0x10 假设三读者
  独立吻合但被否决**（会自断我们自己的读者）；语义重配对挂账。
- **修法**：kit `TEXT_WORD_PATCHES` 一字通道（旧字钉死 fail-fast）+
  首条目 `0x84ca4f8: 0xF2E78D11→0xF2E8A531`（movk imm16
  0x3c68→0x4529）。单测 10+11 OK。scp Mac → kernel 重建（tree 自动
  重暂存，**无需手动 tree**，本轮实证）→ 补丁字 fo `0x14c64f8`
  字节级验真。静态门 resign **0** / dispatch **371** 持平 PASS。
- **出件**：kernel `96422b08`（candidate）/payload `a9bf924f`，双存档
  `out/m2381/payload-w40fix-96422b08.img` + kernel 同名。回退链见 §0。
- **挂账**：①PAC 格式 CRC 转录验真（PANIC_TEXT 副本 + \t/\n→空格）
  ②移植表 per-slot 语义重配对 ③同族审计（final 表页 div 清单已记
  §0②）④ApplePMGR 21 targets。

## 10.Y.13 会话四十七 = 墙 38 首刷判读 + 墙 39 定名 + 池批量 18 笔清仓（2026-11-04，
payload `2f31a6bd` 全门验讫待刷；屏摄判读现场 = 本节判读部分）

- **墙 38 真机通关（fixture 判读通过）**：旧报点 `0x…84ca3bc`（ctor
  簇 IOCPU C2 克隆）未复现；本轮回扫 ctor 簇 0x84ca200..0x84ca4b4
  实测 5 直调站 `0x84ca268/29c/3b8/424/47c` 全打 `0x0af3432c`
  （= fixture 覆盖面精确兑现，含 GOT 槽共 6 站全灭）；stage 轨迹
  全亮维持；pages 0x125b07 / free 0x1249c7（比 w37 boot 再深
  ~2046 页 wired，前沿继续推进）；hit #1。
- **消息定稿 + CRC 配方再校准**：先以墙 38 已知对（139B /
  `0x8a2bf81e`）复算配方确认无尾换行；本轮报文 113B →
  **`0x50d36e66` 零纠正命中**（屏显 17 位 = 9 前导零补齐）。
  `from 0xfffffff0084ca524 (x0 0x1 x1 0x10001)` 全真——x0/x1 为
  残寄存器（无参 getter 忽略实参），与签名自洽。
- **新墙 39 = 同族 unmapped-call**：报点 LR `0x…84ca524`（bl 站
  donor `0x84ca520`）；caller = **`AppleARMCPU::start`**（donor
  `0x84ca4b4..0x84cad7c`，2248B；ctor 在 0x84ca4b4 前已完整通过）；
  target = donor **`0x0a963a34` = `ml_get_cpu_count`**（16B getter，
  读 `topology_info.num_cpus`）。定名三证：①体形家族互证：同 TU
  donor `_ml_get_boot_cpu_number 0xa963a44` = `boot_cpu->cpu_id`
  解引、`_ml_get_cpu_number 0xa963a58` = phys_id 循环返下标 i，
  与 osfmk/arm64/machine_routines.c:1391/1415 逐句同；②KDK
  26.5.2_25F84 地址序 `cpu_count→boot_cpu_number→cpu_number` 与
  donor 逐位同；③调用点 `str w0,[x19,#0xec]` + 同函数尾部
  last-CPU 屏障 `OSIncrementAtomic()+1 == [x19+0xec]` = CPU 数
  语义闭环。
- **关键路径清障（start() 全 callee 对账）**：mapped 12 / **unmapped
  6**。顺手定名 3 个（全部 machine_routines.c 同 TU，KDK 尺寸/
  地址序双夹逼）：`0xa963aa0 = ml_get_cluster_number`（0x6c 循环
  体返 `cpus[i].cluster_id`@+8；结构体 osfmk/arm/cpu_topology.h
  字段序钉死）；`0xa963bc4 = ml_get_max_cpu_number`（16B 读
  `[g+0x280]` = base+8 = max_cpu_id；base = g+0x278 由 cpus/boot_cpu
  双锚反推）。**挂账 1 个 = `0xaedab0c`**（3 站）：0xbc 包装器
  `gObj->vt+0xb8(arg0, arg1?:[g0xb376cf0])` + tpidr 跨 CPU 计数 +
  尾调 `vt+0x2a0(0)`；donor map 邻名 = IOServicePH::systemPowerChange
  但 native 全镜像 csel 指纹扫描零孪生（donor 122.8 版本增量），
  不硬凑——它是 last-CPU 屏障（start() 尾 0x84cad54/6c 双 bl 夹
  sched_cpu_init_completed），SMP 全核起来才触发，下轮墙照带
  x0/x1 定名。
- **池批量 18 笔 fixtures 全落位**（256→274，四个 spec 批次，
  子代理三路扇出 + 主线 1 路）：
  - **ml 族 ×3**（主线，见上）：`0xa963a34→_ml_get_cpu_count`、
    `0xa963aa0→_ml_get_cluster_number`、`0xa963bc4→_ml_get_max_cpu_number`。
  - **vtable 槽位法 ×4**（子代理，4/4 复核 EXACT）：`0xaf32ddc→
    __ZN5IOCPU11setPropertyEPK8OSSymbolP8OSObject`、`0xaf32dc8→
    __ZNK5IOCPU11getPropertyEPK8OSSymbol`、`0xaf32d0c→
    __ZNK5IOCPU19serializePropertiesEP11OSSerialize`、`0xaf32bdc→
    __ZN5IOCPU13setPropertiesEP8OSObject`（各 2 fixup 槽 0 bl；
    ptrauth diversifier 活代码交叉验证；IOCPU.h:77-80 声明序
    对读）。
  - **IOPlatformIO/IODT ×3**（子代理）：`0xaf329cc→
    __ZN12IOPlatformIOC2EPK11OSMetaClass`（3 站）、**`0xaf329f8→
    __ZN12IOPlatformIOD2Ev`（翻案：16B = 死 vptr 毒化
    `mov x8,#-1; str x8,[x0]`，非 C1 thunk；native D2 = 尾调
    super no-op 链，-1 只写不读，重定向安全）**、`0xaf31f80→
    __ZN18IODTPlatformExpertC2EPK11OSMetaClass`（6 站）。
  - **IOCPUInterruptController 簇 ×8**（子代理）：`0xaf34670→
    __ZN24IOCPUInterruptControllernwEm`（kalloc.type operator new，
    **zone 名串 "site.IOCPUInterruptController" + kt_size 0xb8**
    铁证；关键路径：start() 0x84ca818 `mov w0,#0xb8` 前置）、
    `0xaf346e0→__ZN24IOCPUInterruptControllerC1Ev`（0x84ca820，
    Itanium new-X 双连）、`0xaf34770→_IOCPURunPlatformQuiesceActions`
    （队列 +0x20）、`0xaf347f8→_IOCPURunPlatformActiveActions`
    （队列 +0x30；donor 无 hibernate bl = LTO 消除，配置性形态差）、
    `0xaf32aec→getMachProcessor / 0xaf32af8→getCPUGroupSize /
    0xaf32b28→getCPUGroup`（vt 槽 +0x5b0/+0x5a8/+0x5a0 声明序夹逼）、
    **`0xaf3435c→__ZN5IOCPUD2Ev`（再翻案：C1 假设推翻——唯一 bl 站
    0x84ca2e8 在 `AppleARMCPU::D0Ev` deleting-dtor 序列内（bl dtor
    → `_OSObject_typed_operator_delete(ktv,this,0xf8)` 尾调，主线
    独立复核成立），另有 0x84ca2c8/2d0 两条 8B D1/D2 veneer**）。
  - **池身份定谳（背景发现）**：`0xaf32xxx–0xaf34xxx` 整簇不是匿名
    kext 池，是 **donor kernel-proper 的 iokit 代码**（IOCPU.cpp +
    IOPlatformActions.cpp，exec 段尾部），native 同源同版全部有
    twin 符号——解释了本批为何全部一次命中。上轮"super =
    OSObject 级 gMetaClass 0xb3771e8"修正：**0xb3771e8 = IOService
    gMetaClass**，IOPlatformIO/IOCPU 共同 super = IOService
    （IOPlatformIO.h:42）。
- **★工具坑（本轮踩实）**：`make_fixture.py` 的权威区间依赖 kit
  的 `kc_image`/`fileset_match.Side`，无 `PYTHONPATH=
  UniXNU-m2381/kits/XnuPrelink` 时静默降级 end-scan，本机算出
  `0xaf32af8` size=60 ≠ matcher 函数表 48 → Mac `anchor_reviewed`
  当场拒件（`Reviewed function bytes changed`）= fail-fast 设计
  有效。修复 = 摘 18 笔带 PYTHONPATH 重入（17/18 区间本就一致，
  仅 0xaf32af8 60→48）。§0 有备忘。
- **Mac 重建 + 静态门**（kernel+payload 全链 BUILD_EXIT=0）：kernel
  `2f31a6bd`（candidate_sha256）/payload `426ae6ce`，双存档
  `out/m2381/payload-w39fix-2f31a6bd.img` + kernel 同名（本地已拉
  回同 sha）。**native 本轮有布局漂移（noctf 整体 −0x193F4）**，
  map 全部从本 build noctf 重推导内部自洽；map reviewed **274/274
  全绑**（+18 精确）；functions named 9896→**9914**；kext-facing
  named 2051→**2065**；unmapped closure 311→**308**；stubbed
  functions 521。**站点重定向字节级实测 ×5**：bl 0x84ca520→
  `0xa9accb8`（= ml_get_cpu_count 终局，`bti c; adrp; ldr
  w0,[g,#0x664]; ret` 逐句同构）+ cluster_number/max_cpu_number/
  icu new/icu C1 全验，终局镜像里 topology 双偏移 num_cpus@0x664 /
  max_cpu_id@0x668 相邻对上。静态门 `gate-s47.py`（新写，w37 件
  校准）：resign **0** / dispatch 对 **371** 持平 PASS。
- **回退**：recovery_a ← `payload-w38fix-b6bfe74b.img`（c4cb22fb，
  已证通关墙 36~38）；e6b06399 二级；ce3df35f 三级。

## 11. 换道工作单 = 会话五十一：os_profile → ipados-27.0.1-24A446
（2026-11-05 用户拍板；侦察对账档 = 记忆 `sheng-24a446-recon-ledger`；
上游实证实录 = sheng porting.toml：SpringBoard PID 30 / backboardd 61 /
wifid 49 / 触控 / WiFi device-verified，视觉呈现未验）

**会话五十二排程（2026-11-05 定）**：本单步 0~7 一次跑完（换道施工全链
出件，payload-r52 待刷）；首刷+判读 = 会话五十三。**步 0（可选，
不阻塞）**：w42fix 屏摄轻判读——decode `0xa5b8db4` 站 bl → 目标 →
23F84 map 定名被调内核函数；donor→内核按名绑定，该函数名大概率 =
24A446 首刷第一面同型墙，提前备 fixture/spec 省 one round。**Mac 侧
准备**：24A446 IPSW 需上 Mac（`scp` 过 LAN 或 Mac 直接下载同 CDN URL），
替换/并存 `UNIXNU_IPSW` 指向（23F84 件保留勿覆盖，回退车道要用）。

**IPSW 到货复验（已毕 2026-11-05）**：
`research/ipsw/iPad17,1,iPad17,2,iPad17,3,iPad17,4_27.0.1_24A446_Restore.ipsw`
—— 字节数 11,654,176,243 与 CDN Content-Length 逐位同；Restore.plist =
24A446/27.0.1 ✓；`kernelcache.release.ipad17`（24 MB）在位，raw 成员
`802b495b…`，profile pin `38e6d9fa…` = 解码后 sha（由
`scripts/unixnu/kernel.py:334` 在 fetch/kernel 步硬门验）；整包 sha256
`fa93b653cc03c80ee5009302545479cb3b0f7bbaaa43f43a25e70cc328575b7a` 存档。

**步骤**：

1. **profile 切换**：设备仓 `board.toml` `os_profile = "ipados-27.0.1-24A446"`；
   IPSW 本地喂 `--ipsw research/ipsw/…ipsw` 或 `UNIXNU_IPSW`。铁律：改
   board 必重跑 `./unixnu defconfig m2381_defconfig`。
2. **fetch + sha 门**：`./unixnu fetch`（KernelCache / DeviceTree /
   StaticTrustCache / Cryptex1,SystemTrustCache；`kernelcache_sha256`
   硬门必过，不过即停）。
3. **we-only 8 patch 逐条 24A446 重验**（对 24A446 donor kernelcache 找
   同站/同病灶，结论 = 保留/改站/退役，逐条入 porting.toml）：
   `fileset-amfi-devmode-shim`（预计退役换步 4 板级方案）、
   `prng-getentropy-donor-discriminator`、`crypto-funcs-probe`、
   `crypto-funcs-unpack`、`mac-policy-ops-donor-resign`、
   `sysctl-handler-donor-resign`、`kext-entry-donor-pptr`、
   `fileset-read-erandom-shim`。
4. **板级 PMGR 包（m2381 PlatformPower，第一版）**：抄 sheng
   `package/ShengPlatformPower.cpp` 模式——`lookup("ApplePMGR")` /
   `lookup("ApplePMGRNub")` 注册仿制 metaclass + `serviceMatching`
   personality 发布 nub 面（AMFI `waitForMatchingService` 命中即安）；
   PMIC/Rpmh 实质功率管理后续轮补。放设备仓 `package/` + 构建登记。
5. **kit**：gitlink 保持 `b1888c12`（= sheng startup-pipeline tip，含
   fileset emitter function/data maps + 24A446 startup import/abs-branch
   修复）+ 本地 matcher/TEXT_WORD_PATCHES 提交；RELEASE 参照件必须
   `--lto`（sheng 教训：packaging success ≠ boot compatibility；
   corecrypto kprintf rebind 即此翻车）。
6. **ADT/identity 平移**：iPad17,3/J820AP 不变；`/chosen`
   `kernel-ctrr-to-be-enabled=0` 双保险属性随迁（墙 41 对策）；24A446
   ADT 结构差异由 payload 步自检报。
7. **构建链**：defconfig → fetch → tree → kernel（xnu-12377.121.6，
   darwin26 系列 4 条 OS patch 自动进 series）→ firmware → payload
   （三红线显式旗标）→ 刷 recovery_a only（boot_b/userdata 不动）→
   `set_active b` → reboot。
8. **首刷判读预期**：AMFI 静默（板级 nub 或 shim 命中）→ t≈32s IORTC
   税 → vfsinit → root/cryptex mount → **SHLOG 窗若 state=2 开始写 =
   大捷旗标**（落盘通道 root mount 后才开）→ launchd/SpringBoard 站。
   判读 = 屏摄；新墙照旧分流（定名三证 → 通道选择 → spec → 一重建一刷）。
9. **回退链**：recovery_a ← `payload-w42fix-3fe101ba.img`（`6436442d`）
   一级；w41fix 二级；w40fix 三级。
10. **风险**：①23F84 地址簿 fixture（311 closure）全量作废重做，新
    emitter 吸收大头；②`--allow-missing` 语义仅限 sheng 已容忍面
    （2 data refs + 24 trap refs 级别），超出即按墙处理不放过；③24A446
    的 `early_security_init` AMFI wrapper 地址与 `roots` 清单以 profile
    pin 为准（`0xfffffe00092551ac`），若 mismatch 先查 profile 再查件。
