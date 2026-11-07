# STATE — 现势状态（m2381，2026-10-28 更新）
## 0. 现势指针（每轮首更，2026-11-07 会话七十五·r76 = 墙76 AMFI No-PMGR 三站 shim 轮：**payload-w76fix-a1fc030e 待刷（recovery_a 单件，kernel 2f856b18）**；静态门全绿，三轮 CRC 挂账清零）

- **★★★★会话七十四 = r73 判读深读+账本手术+出件（2026-11-07 晨，用户令"开始r73"；Linux 分析+Mac 出件，源码零改动）**。
  **⓪①ABI 深读定谳：墙73 靶 donor `0xb3d6498`（44B 叶件）= `OSObject::OSObject()`**——`bti c; adrp/add/add → __ZTV8OSObject+0x10`（=0x7f077d8，`__DATA_CONST.__const`；vtable 槽位 div 序列 0x3771/0x56b7/0xbaa2… 与 OSObject 虚函数集 release(int)/getRetainCount/retain/release/serialize/getMetaClass/isEqualTo/taggedRetain/taggedRelease×2/Dispatch/init/free 逐位对上，map body-hash 交叉命名）；`movk x17,#0xcda1,lsl#48` 修饰符=obj 高 16 位换判别子；`str w8,[x0,#8]`=32 位 retainCount=1。**x1 全程不用**（r72 推断实证）。源码锚 OSObject.cpp:427（xnu-3199e8bc）= retainCount=1 单行体。**kernel 内 0 直呼 bl**——内核构造器全内联惯用语（native 1468 站同款），out-of-line 件只作为 kext 导出面存在：donor 全宇宙 109 kext GOT 槽，**存活 entry 内 12 槽 = r72 stub 报告"12站"精确来源**（工具=temp/s74-*.py，槽清单=temp/s74-kext-got-imports.json）。
  **②native 对应物实锤：r72 hybrid `__ZN8OSObjectC2Ev`@0xb3aba98**（pacibsp + bl 0xb3a8d04=OSMetaClassBase-ctor 内联件装 __ZTV15OSMetaClassBase → 自装 __ZTV8OSObject 0x7e4b188 → refcount=1 → retab）——**s72"native 全内联无 out-of-line"系误判**（record[152] 在 r72 emit 已成功解析该符号；vtable 槽位 div 与 donor 完全同构双向验证）。
  **★★★0xb3d385c 大翻案 = `OSMetaClass::~OSMetaClass()`（非 OSObject ctor）**：vtable=__ZTV11OSMetaClass（slot0/1=D1/D0，vtable 通道已命名）；体读 reserved=[this+8]（活对象字段——真 3-arg ctor=0xb3d2cbc 404B 存 x1/x2/x3+IOMallocType(reserved@8)+stalled 数组+OSKextLog），析构序=sStalled 摘除+memmove+IOFreeType(reserved)+[this]=-1；源码锚 OSMetaClass.cpp 析构器。**record[152]（r59 期）把 23F84 代的 OSObject-ctor 身份错罩到 24A446 同 VA 的 dtor 上（漏 vtable 检查），sha 恰按本代体算故 anchor_reviewed 全绿**——292 kext 导入实为编译器析构引用（运行时~不执行，故 r72 潜伏未炸），25 存活槽在 r72 被误绑到 native OSObjectC2。
  **③修法 = 零 shim、纯账本两笔**（设备仓 9302de7；s56-shim 预案作废——其前提"native 无 out-of-line 件"被证伪）：新增 record `0xb3d6498→__ZN8OSObjectC2Ev`（44B sha 629cfcc7…）+ record[152] native_symbol→`__ZN11OSMetaClassD1Ev`（体 sha 不变仍绿）。kit 单测 46/46。
  **④Mac 出件**（tree→prelink→payload；源码零改动=参照件 448c84b7 复用，map 缓存按 evidence 自动失效）：**stub 37→25 站/14 kext 精确兑现**（helper 12 站全消；余=ARMPlatform×7/Image4×5/IOGPU×2/各×1；missing 2 不变）；map v12 functions 17001=+1（0xb3d6498[reviewed]→0x79efa98、0xb3d385c[reviewed]→0x7c95964）；**槽级实证：CoreAnalytics 槽 0x827c610→0xb3aba98（反汇编=OSObjectC2 逐指令同款），12 helper 槽全绑同靶，25 dtor 槽全绑 OSMetaClassD1Ev@0xb651964**。kernel **9b9f57e0**（96MiB）/payload **f38d11cc**（99MiB）双端 sha 逐位同。
  **★r74 工作单 = 首刷判读**：预期墙73 死（CoreAnalytics t≈2s 面过、boot 后移）；下一墙=余 25 站首个命中（候选序照 23 靶预注册：Image4×4/ARMPlatform×7/0xad94128 等）；**留意 vnode_put/vnode_hold 修正（0e38039）首次实跑**（r72 死于 VFS 前，r73 若推进到 root mount 即验证）+ dtor 重绑（25 槽）无回归。**余单 = 账本 23F84 遗留 record 审计**（record[152] 同族风险：凡换道前入账且按 VA 重算过 sha 的 record 需补 vtable 复验——本轮实证此类错可潜伏整条链）。回退=recovery_a ← payload-r72-5a92ea2b。
  **★★★r73 首刷判读（2026-11-07 屏摄 IMG_20261107_072740/072753，用户报"跑得最久"）**：**t=30s 中途屏 + t=31s panic —— iOS27 车道史上最长 boot**（r72=2s）。**墙73 消实锤**：CoreAnalytics 面过（r72 的 OSObject ctor stub 未复现），余 25 stub 站全未踩；stage 行 `+28s bsd_init: calling IOKitInitializeTime?`（30s IORTC 开机税照挂账）+ 内核 console 深入（IONVRAM unserialize/ephemeral-data-mode、vm swapout/swapfile/GC 线程、lockdown_mode nvram 读取）；PC 采样 23-30s 停 0xb59f23c/238 微跳（IOKitInitializeTime 轮询内，非挂死）；MEM free4K=1197116 与 panic 屏 free=0x12443c 逐位互证。**新墙 panic：`SourceStorage: root policy handoff or authentication failed` @SourceStorage.cpp:102——性质换档：非 stub 哨兵，是自家 SourceStorage 组件的 policy 强制门**（native-source-storage=1 车道史上第一次跑到）。CRC 0x2960355a 未精配（消息缓冲格式未钉死，pages/free 交叉验证已足信，精配挂账）。
  **★★★墙74 根因定谳（全链尸检）**：`native_source_root_policies_load()` 读 `/chosen/memory-map.RootPolicies`（16B {phys,len}）→ 复制锁页 → `VROOTB01` 信封逐 policy `vr_open/vr_open_header` 对编译内置 `verified_root_key[395]` 验签。链条逐环核验：kernel 步 `build-sheng-source-storage.py` 已合成信封（VROOTB01/v1/count=2/30.5MB 全量表）+ 同源 key（RootAnchor.c，trusted-key sha 74e60348=Mac `.cache/firmware/verified-root-24A446` pair，verification.json 的 image_sha256 8a12cdad=userdata-root.json OS 窗口 sha 逐位同）——**kernel/pair 侧自洽**；UEFI DarwinBoot 读 `<KernelPath>.vroot` 交接（缺文件+DT 有属性=固件层拒启，r73 进了内核=两缺）；**根因=board.toml [payload] 从无 `root_policies` 键** → .vroot 未上 ESP、DT 无属性 → 内核 getProperty=NULL → panic。s52 备忘"政策对=换 donor 硬前置"的最后一步（payload 接线）从未落地，此前 71 轮全死在 t≤2s 故从未暴露。
  **★r74 修复 = 三件套接线（零内核改动）**：①board.toml [payload] `root_policies="{tree}/build/sheng-source-storage/root-policies.bin"`（设备仓 6ba3b84）；②**全量表 30.5MB 塞不进 99MiB ESP**（kernel 已占 91.4MiB）→ lane 转 **tables-in-userdata 模式**：kernel 步 env `XNU_SOURCE_STORAGE_TABLES=userdata`（bundle 缩成 2×544B 签名头 =1152B）+ userdata 重建带 `--object OS.vroot=<pair>/OS/manifest.bin --object SystemOS.vroot=<pair>/SystemOS/manifest.bin`（NativeUfs 从根镜像 v2 objects 读全表对签名头验签，RootLayout.h/NativeUfs.cpp:1668 既有通路）；③Mac 重跑三步：**kernel sha 9b9f57e0 逐位不变**（env 只改 bundle，重验证 ✓）+ userdata 新件 **e01c27ae**（15,662,579,712B=旧+31.5MB objects，OS.vroot 18,776,608B@0x3a3b00000 + SystemOS.vroot 11,751,968B@0x3a4d00000）+ payload **7db186c0**（ESP 实测 VROOTB01/ver1/count2/total1152 在位）。**r74 判读预期**：SourceStorage 过门→UFS start→Data disk→root mount 首跑（vnode_put/vnode_hold 修正（0e38039）首次实跑窗口）→ 新墙=余 25 stub 站首中或 NativeUfs root step 报点。刷写 = **双件**：userdata ← e01c27ae + recovery_a ← 7db186c0（boot_b 289614ba 不动）；回退 = recovery_a ← payload-r73-f38d11cc + userdata ← b280f63e。
  **★★★r74 首刷判读（2026-11-07 屏摄 IMG_20261107_081812，单屏 panic）**：t=31s，**墙74 死实锤——`.vroot` 信封真机全链走通**（UEFI 读 kernel.macho.vroot→DT RootPolicies→内核复制锁页→vr_open_header×2 对内置 key 验签全过，SourceStorage.cpp:102 未复现，verified-root 机制首次硬件兑现）。新墙 **NativeUfs.cpp:402**：`NATIVE_UFS smmu: conflicting UFS stream mapping slot=2 smr=80000060 s2cr=2`（crc 0xa8b5d477 未精配，同 PANIC_TEXT 格式挂账）。仍是自家栈（SMMU 规划器），余 25 stub 站仍未踩。
  **★★★墙75 根因（UEFI 侧考古，用户指路 /home/ri/work/meizu20/mu-uefi）**：m2381Pkg 的 UFS 栈 = **`Binaries/mars/QcomPkg/Drivers/UFSDxe`（Qualcomm 二进制）**+标准 DiskIo/Partition/Fat（DXE.inc:49），ESP 读取全程走它 → 它给 stream 0x60（DT `iommus=&apps_smmu 0x60`）自建了 SMMU 映射 **SMR2（精确）→S2CR2 TYPE=TRANSLATE CBNDX=2** 且 handoff 无人拆（Silicium 的 ArmSmmuDetachLib 有链接无调用点，Kailua 跳过名单只有 MDP 流）；内核 SmmuTakeover planner 按 2026-10-17 测量（"handoff 后无 0x60 条目"）设计 → 旧口径判 final conflict。
  **★r75 修法 = planner 收编（设备仓 6d29329）**：自家 stream 的独占精确单一映射 = 占住该 SMR 槽（retaken 锁防后续空闲槽覆盖），configure 按序（先 S2CR 后 SMR，quiesced 无窗口）把它改指向保留 bank；wildcard/重复映射仍 fatal；**顺带修掉暗坑**：重试路径曾在活低槽下方分配更高槽（SMMU 最低索引组胜出=静默错映射）——现任何独占自家流映射一律收编；脏保留 bank（异常 VMID/CBA2R）从"拒绝"改"弃置另收编"（不碰可疑 VMID=更安全）。单测 +3 case 全绿（固件映射收编/改写顺序/弃置语义）。**"拿 UEFI 侧 UFS 驱动来用"的定谳**：UFSDxe=二进制不可复用源码，收编其运行时现场=正确姿势。
  **★r75 出件**：kernel **c30e85b7** / payload **7c5c39e5**（recovery_a **单件**，userdata e01c27ae 不变）；回退 = recovery_a ← 7db186c0。
  **★★★r75 首刷判读（2026-11-07 屏摄 IMG_20261107_085140/085335）**：t=31s，**墙75 死实锤——SMMU 收编生效**：NativeUfs 全链过（SMMU retake→UFS start→Data disk→root policy），中途屏 PC 采样 0xb59f244/248（IOKitInitializeTime 轮询，r74 的 23c/238 平移 8B=内核重排正常）。新墙 = **`"AMFI: No PMGR?\n" @ConfigurationSettings.cpp:392`**（crc 0xf063fb5b）= **23F84 车道 wall-42 同族老对手**（当年 :388，s50 定谳：waitForMatchingService("ApplePMGRNub") 30s 整超时→NULL→panic；SM8550 无 PMGR 硬件+matching 从未跑必炸；31s = IORTC 税+30s 等待双定时器对齐）。**修复史勘定（用户问"ios16/17 修过吧"）**：修于 23F84 车道 s50（11-04）；iOS16 车道不同架构没跑 donor AMFI，iOS17 车道止步 wall41 未到此。
  **★★★墙76 第一刀已侦察（r76 工作单）**：①**shim 侧不在现役 series**——`fileset-amfi-devmode-shim.patch`（`unixnu_amfi_devmode_config` 返回 0=benign absence，mac_base.c+Mach.exports，41 行）只挂在 23F84 series，24A446 series 仅 5 条未带 → **第一动作 = series 加回**；②24A446 donor AMFI 站点普查已毕：ConfigurationSettings 函数簇 = AMFI `__TEXT_EXEC 0x924e5ac-0x9251994`（117 个 adrp 引用 cstring 页 0x763e000），串表=`"AMFI: No AppleSocMisc?\n" @%s:%d`@0x763e3b1 + `ApplePMGRNub`@0x763e3c2/0x3d6 + `AMFI: [ApplePMGRNub] latched BPR\n` + **`"AMFI: No PMGR?\n" @%s:%d`@0x763e3f8（assert 式单行格式——引号+字面\n+@file:line 都是消息本体）** + `skipping PMGRAON latch due to AVP` + `/arm-io/soc-misc`；三件套=devmode latch+soc-misc 探针+BPR 兄弟（23F84 的 :367/:372/:388 同构，行号平移）。③绑定口径=当前 v12 pipeline 的 kext bl→native shim（23F84 用 TEXT_WORD_PATCHES 三站 bl→shim 的等价物；改站不改头毁 LR 的坑照抄警示）。
  **★CRC 欠账定性（三轮 0x2960355a/0xa8b5d477/0xf063fb5b）**：`boot_progress_putc`=console 流式逐字符捕获（zzz-boot-progress.patch 的 putc 钩子），缓冲=滚动窗口含可能的前文；"AMFI: No PMGR"消息=assert 式**单行**（非 msg\n@loc 两行式）——CRC 精配需读 BootProgress.c `bp_char`/panic 副本逻辑定缓冲字节序，r76 一条龙关账。
  **★r76 工作单（定稿）**：①r75 判读已毕（本节）；②series 加回 amfi-devmode-shim + AMFI 三站绑定（普查坐标见上）→出件；③PANIC_TEXT 格式钉死+三轮 CRC 回算；④时间允许：账本 23F84 遗留 record 审计（按入账时间戳筛换道前子集补 vtable 复验）。大分区议题冻结等上游。刷写 = recovery_a 单件（userdata e01c27ae 现役不动）；回退 = recovery_a ← 7c5c39e5。


## 1. 一句话现状（**2026-10-31 会话二十四收档：23F84 fileset 车道，matcher v11 判别子通道落地，战略拍板 = 关键路径优先**）：

**★★★r76 施工全档（2026-11-07 会话七十五，用户令"r76开工"；照 §0 定稿工作单四项全毕）**：
  **①三站侦察定谳**：24A446 donor AMFI 把 23F84 的 getter 簇**合并成一个组合函数 LATCH `0xfffffe000924e7b0`（988B）**——soc-misc 探针 + ApplePMGRNub 30s 等待 + BPR latch + 两个编译器拆出的 assert 助手（`"No AppleSocMisc?"`@0x926feec / `"No PMGR?"`@0x926ff1c，行号常量 0x188=392 在助手体内钉死）；全 kext 分支面扫描 = **恰好 3 个 bl 站**（`0x924e77c` word `0x9400000D` / `0x924ef1c` word `0x97FFFE25` / `0x924ef5c` word `0x97FFFE15`，三路 imm26 解码同靶 LATCH；站1 与 23F84 站1 逐位同字 = 编码巧合）；零 fixup/零函数指针/零 b 尾调（recon5 全扫）。**②施工**：series 加回 `fileset-amfi-devmode-shim.patch`（主树；本地 dry-run 81 patches 全绿 + Mac prepare 应用实证）+ 设备仓 kit `TEXT_WORD_PATCHES` 三条目 `_bl_to_native('_unixnu_amfi_devmode_config')`（23F84 配方逐位复刻）；kit 单测全绿。**③出件链（两坑现形）**：参照件重编裸跑 exit 2 = **`XNU_KERNEL_CHAINED_FIXUPS=1` env 缺失**（MakeInc.def:1127 else 分支要 ld-classic+threaded_starts，Xcode 27.2 双双没有；s72"CHAINED_FIXUPS env"老坑第三次，s74 会话环境里带着所以成功）——带上后 exit 0；payload 首跑拒收 121MiB>99MiB = **r74 的 `XNU_SOURCE_STORAGE_TABLES=userdata` 是 env-only 接线**（board.toml 无此键），重跑 kernel 步漏带 → .vroot 全量表 30.5MB 复活进 ESP → 补带后 root-policies.bin 回 1152B。kernel 步重发 noctf sha 不变（16f25c93）→ prelink 重发 **kernel.macho sha 逐位同 2f856b18**（emit 确定性实证）。**④静态门**：emit `text_word_patches: 3`（old words fail-fast 未触发）+ **站点字节级验证**：三站 bl → `0xfffffff00b5185c0`，靶体与 noctf shim（`pacibsp; mov w0,#0; retab` 12B）逐字节 IDENTICAL，delta = **0x39bc000** = s58 期 native 放置常量精确吻合。**出件**：kernel `2f856b18`（91.4MiB）/ payload `a1fc030e`（99MiB），存档 `out/m2381/payload-w76fix-a1fc030e.img` + kernel 同名。**刷写 = recovery_a 单件**（userdata e01c27ae 现役不动）；回退 = recovery_a ← `7c5c39e5`。
  **★★★③ CRC 欠账清零（三轮零纠正）**：**格式总定谳 = `osfmk/kern/panic_call.h:46`（XNU_KERNEL_PRIVATE）`panic` 是宏**：每个调用编译成 `(panic)(fmt " @%s:%d", ##__VA_ARGS__, __FILE_NAME__, __LINE__)`——PANIC_TEXT 副本 = 宏展开字节（fmt + `" @"` + 基名 + `":"` + 行号，无尾换行；assert 式串里字面反斜杠-n 保持两字符；putc 过滤 c∈[9,126] 且 c<32 变空格，`g.log=0` 后 pages/crc 行不入副本）。回算：墙74 `SourceStorage: root policy handoff or authentication failed @SourceStorage.cpp:102`（82B）= `2960355a` ✓；墙75 `NATIVE_UFS smmu: conflicting UFS stream mapping slot=2 smr=80000060 s2cr=2 @NativeUfs.cpp:402`（93B）= `a8b5d477` ✓；墙76 `"AMFI: No PMGR?\n" @ConfigurationSettings.cpp:392`（49B）= `f063fb5b` ✓。`verify-panic-crc.py` 扩 `--msg` 通道 + 五对校准全过（w38/w39 旧对回归 ✓）。
  **★④ 账本 23F84 遗留 record 审计（layer-1）**：`Ported from` 子集 = 39 条，**零 ctor/dtor 身份**（record[152] 风险族不在其中；本体 r73 已带 vtable 复验修毕）、体全可解、const 引用 7 条皆为全局/zone 面；邻接抽查 `_enable/_disable_developer_mode` = 镜像 setter 体（唯一差异 `[sp]`=1/0）语义一致 → **无新误绑**。逐函数三证降为背景（哨兵会说话）。
  **★r77 工作单 = 首刷判读**：预期墙76 死（AMFI 30s PMGR 等待链整体消失——三站重定向后 LATCH 永不执行，t≈31s 的 assert 不再复现；devmode 走 shim 0=benign absence 与 23F84 s50 实证同语义）；boot 推进到 root mount 之后的新组件；新墙 = 余 25 stub 站首中（候选序照 23 靶预注册）或 root-mount 后首站。**挂账**：①`XNU_SOURCE_STORAGE_TABLES=userdata` env-only 接线固化（board.toml/profile 键 + builder 读取，防再丢）②大分区议题冻结等上游 ③CRC 精配虽清零，未来新消息族照 `panic_call.h` 宏口径直算。判读通道 = 屏摄（SHLOG 窗在 root mount 后才开，本次大概率仍屏摄）。

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

## 2. 设备现状（2026-11-07 更新 = **iOS 链现役，24A446 混合线**）

| 分区 | 件 | sha 前 8 | 状态 |
|---|---|---|---|
| boot_b | 本仓 UEFI（24A446 线配套） | `289614ba` | **现役**（不动） |
| recovery_a | payload vtslot | `7db186c0`（r74） | **现役**（r74 已刷已判=墙75 SMMU 收编前夜）；**r75 `7c5c39e5` 待刷** |
| userdata | 24A446 tables-in-userdata 件 | `e01c27ae` | **现役**（r74 已刷，+OS/SystemOS.vroot objects，15,662,579,712B）；r75 不动 |

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
