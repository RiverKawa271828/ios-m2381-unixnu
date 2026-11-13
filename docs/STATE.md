# STATE — 现势状态（m2381，2026-10-28 更新）
## 0. 现势指针（每轮首更，2026-11-13 会话一百零二=**★★★★★★★★★★新墙一锅端出件=payload-r112-be7f2f87（kernel af2f055b，Mac 链）双修待刷：watchdogd=disabled.plist arg、containermanagerd=MCM 数据库目录骨架（cache RE 定谳）**）：

**★★★★★★★★★★r112 出件判读定谳全轮（2026-11-13 会话一百零二；用户令"新墙开工+一锅端+子代理打杂≤3线程"）**：
- **watchdogd 墙定谳（工具 RE 翻案）**：主嫌疑"emit missing kext"是误读——AppleARMWatchdogTimer **完整在 r111 内核**（LC_FILESET_ENTRY+PRELINK_INFO+personality 逐位验证，load 0xfffffff007180a90）；真根因 = **ADT 无 arm-io `wdt` 节点**（arm-io 只有 intc/ufshc/ufsphy 三导出）→ personality `IONameMatch="wdt,s5l8960x"` 永不 match → 驱动永不 start → `"IOWatchdog"` 服务从未发布 → watchdogd `IOServiceGetMatchingService("IOWatchdog")` NULL → brk#1 自杀循环。**"emit missing 2"记账勘误**：= 2 个未配对悬空 GOT 指针（owner(site)→unmatched target；0xfffffe00070446a8 = 根内核 __TEXT.__const 零 hook 槽，紧邻 version[]；非缺失 kext、非本墙因果）。20H392 先例=NOTES.md:122/134/155（sweet 0040 IOWatchdog 注册制）+21H461 r36-r43 disabled.plist 制。
- **修①（arg-only）**：board.toml kernel_args + `sheng-disable-watchdogd=1`（BootWatchdogPolicy 写 disabled.plist com.apple.watchdogd=true；机制已编进内核：符号 0xfffffff00b5940e8 在 r111 内核实证；时序 owner_data_prepare 先于 load_init_program 逐 hunk 验证；先例=20H392 r43 实机+sheng Pad6S Pro 生产同 arg 同 SoC）。
- **containermanagerd 墙定谳（24A446 dyld cache 首次全量符号化，scratch-r112/REPORT-cache-symbolication.md）**：cache 自 userdata-root.img SystemOS 窗口抽取（Rave24A446 SystemCryptex /System/Library/Caches/com.apple.dyld/，84 文件；**cache 在 cryptex 不在 OS 卷**；imagesText 字段序=uuid+load+textSize+pathOffset 自证 10/10；.symbols/.77.dyldlinkedit ZBM 经 eafer/libzbitmap 按组并行 0.8s 解码；nlist 与 17.7 同构）。死链符号化：**PC 0x206fdbc04 = `+[MCMSQLiteDB openDBWithURL:queue:error:]+0x33c`**（brk#1=noreturn helper 异常返回后编译器兜底）← `-[MCMLibraryRepair fixAndRetryIfPermissionsErrorWithURL:…]` ← MCMCodeSigningMapping codeSignMapping（dispatch barrier-sync）← containermanagerd_start XPC block：**sqlite3_open_v2(READWRITE|CREATE|EXRESCODE) 失败** → "Failed to initialize database, err=%d xerr=%d" → 终结。**缺因 = `/private/var/root/Library/MobileContainerManager/`**（DB=containers.sqlite3；CFString 常量 'containers'/'sqlite3' 与 `file:///…/Replace/`、`/Delete/` 逐字互证；daemon=root home 拼装；OS 卷无数据侧模板；LibraryRepair 只修权限不建目录）。
- **修②（内核+arg）**：BootWatchdogPolicy.c 新增 `sheng_container_db_prepare`（`sheng-container-db=1` 门）：launchd exec 前 mkdir 骨架 /private/var/root/Library/MobileContainerManager（+PendingUpdates/Replace/Delete）+ …/Caches/com.apple.containermanagerd/System/Dead，root 0755，sqlite 自建 DB。census 定谳：真 spawn→crash 循环全窗仅 watchdogd(6/3@84)+containermanagerd(8/3@86+W0 p137)；Sandbox "no … container path found" 六处=cm 死亡下游；DEADLINE EXCEEDED=BootProgress.c:280 自绘横幅，成因链=两 daemon `_PanicOnConsecutiveCrash{deadline:60}`（plist 实读）。
- **出件（Mac 链，payload=out/m2381/payload-r112-be7f2f87.img sha be7f2f8738968b5d…/103809024B；kernel af2f055b→密封 7bb29a96；ADT 2fb4f35d 新种子）**：静态门绿（missing 同款 2/stubbed 22/骨架串×1/双 arg 在 cmdline）。**管线两坑入账**：①本地 Linux emit 车道对本地 native matcher 大面积配对失败（stubbed 1245 vs 基线 22；fileset-reference LTO 通道不在本地）——内核出件今后只用 Mac 链（r105 起老问题本轮坐实）；②Mac kernel step 必带 `XNU_SOURCE_STORAGE_TABLES=userdata`（漏带 → root-policies.bin 30.5MB 全表 → payload 121MiB 溢出 99 上限；带之 → 1152B 双头）。r111 实为 Mac 出件（本地 kernel.json=11-11 是 r109 件）。
- **旁支入账**：disk1(OwnerData/RamBlock 支撑)"device is not ready"+fsync EBUSY ino(249) 风暴 179.7-181s = RamBlock"store 写失败一次全停写"语义（NativeRamBlock.cpp:109），触发因未定性→G3 观察；kqworkloops "too many"=软限通知发送失败(0x10000004) 噪声非墙；AKS st=-536870174=**kIOReturnNotPermitted**（换算入账）；watchdogd 备刀坐实未启用=`kern.hv_vmm_present` 置位走 VM 干净退出分支（+0x59e0）。
- **r112 判读预注册+刷写卡=out/m2381/reader-r111/R112-CARD.md**（G1 watchdogd 零崩+policy stage；G2 cm 零崩+skeleton ready 行；G2x 备刀 disable-jobs；G3 disk1 风暴再现=独立墙；G4 回归六项；G5 boot 深于 158s+横幅消失）。取证通道：recovery_a=reader 现态不变，刷 payload-r112 单件到 recovery_a，8-15min 硬断电拉窗。**主树/设备仓提交待推**（主树 s102 轮+设备仓 board.toml/BootWatchdogPolicy.c 两文件）。cache 抽取物留存 scratch-r112/（7.3GB，dmg.raw 已删可重建）。



**★★★★★★★★★★r111 判读首刷（2026-11-13 会话一百下半，用户刷 kernel 40ac5f4f/payload 3a4bf72e 后真机回报）**：**四代 entcopy 墙死透三证**：①`ret=14` 全窗 **0 次**（旧墙死点 72s/95s 消失）；②entval/entconv/conv 探针行全 **0**（F1/F2/value 转换全程健康——守卫修法生效，`_unixnu_kernel_stack_shift`=15 在役）；③`-l` 探针行 ret=13×**336** = **benign 空容器码**（M 的 0xd=0 元素返回，K `cmp #0xd` 容忍→正常控制流；L 探针打印条件 ret≠0 连带收录，非死亡行）；ret=2（守卫假 trip）0 次。**boot 世界全面展开**：158s/page-clock、procs=91/spawn=598、**published=2**（s75 期恒 0→大捷旗标）、UFS requests=163619 verified4K=463605(1810MiB) failures=0、launchd p1/backboardd p65/**SpringBoard p29**/runningboardd/lsd 全在跑（spawn census 至 profiled@155s，87 名进程，xpcproxy 308/120@78）。**新墙（下会话判读+修复对象，用户拍板移交）= watchdogd+containermanagerd spawn→brk(sig5) 循环 → launchd watchdog DEADLINE EXCEEDED（诊断屏，OS 继续记录）**：
- **watchdogd ×3 pid（57/118/140）**：CRASH sig5 PC=0x10056abe8/0x100b2bbe8/0x100af1be8（ESR f2000001 INSN d4200020=brk，各 pid 不同 slide）；p140 现场串证 `connect_to_kext() unexpectedly failed - unable to lookup watchdog KEXT service`（x0=0x100af539c=镜像内 cstring，image0=0x100ae3000=/usr/libexec/watchdogd → off 0xE1BE8）；p140 bt = 0x100ae8ad4→0x100aed630→**0x182fe75b8（cache 区第三帧=IOKit/共享库）**。**主嫌疑①：AppleARMWatchdogTimer kext 服务 lookup 失败——emit missing 2 之一恰 = com.apple.driver.AppleARMWatchdogTimer (0xfffffff0070446a8)**（r109 起挂账）；msgbuf 无该 kext start 痕迹；20H392 车道 watchdogd 先例见 memory watchdogd-wall-bootprogress（r77-r80，boot-progress 期第 30 墙修）。
- **containermanagerd ×3 pid（75/107/132）全部同点 PC=0x209b3bc04（dyld cache 区）** = 确定性逻辑死（cache 内共享库函数）；W0 SEGFAIL p137 sc=209b3bc04 同串；主嫌疑②：与 `[cryptos] stage=0 status=-1`（cryptex 未挂）关联。
- 死亡窗口时序：179.7-180.65s disk1 "device is not ready"×N + `apfs_fsync_internal_ext cluster_push() failed with 16 on ino(249)`（先于/伴随 DEADLINE，疑似 datavolume 空闲面，下会话定性）；X0 AKS st=-536870174（kIOReturn 负值，待换算）；EXIT 217 nonzero67 crashsig5 drop3；`A files=-1 mount=0/-1` 恒定（addon mount 面，车道一贯）。
- **判读前置（下会话开工单）**：①watchdogd 二进制符号化 = 独立二进制**不在共享缓存**，需 SystemOS 树（.aea 在 out/ipsw/24A446/ 重解，配方=temp-s99/scan_dmg_ents.py 期；extract 已删）；②containermanagerd 0x209b3bc04 = cache 地址符号化需 **24A446 cache-extract 重建**（UNIXNU_CACHE_EXTRACT 配方，memory dyld-cache-symbolication）；③去滑规则（CRASH 行+N=shared region slide；bt 帧=高32位标记+低32位offset，memory dyld-cache-symbolication）；④旧先例对照 = 20H392 watchdogd-wall-bootprogress 记忆（wall30 scale 断言修法）+ s77-r79 判读惯例；⑤窗弹药 = out/m2381/reader-r111/（log.bin sha **80e85380** 双端对拍绿，79 槽鲜窗 uptime=155s state=2，parsed-all 9610 行/parsed-msg 1432 行，屏摄 IMG_20261113_195227.jpg+转录同目录）。

**s100 收队段（上轮遗留，流程性内容仍有效）**：提取流程（acm-reader probe→fetchchunk→parse --msg）、挂账（主树未推 711+本会话判读新增、设备仓 8、AMFI unresolved 导入 0xad3e8d8、end7 字段挂账）、出件链管线三坑备忘照旧；回退锚 = r106 `8a0c089d`（r111 = 现役件，勿回退）。

**★★★s100 收队交接段（2026-11-13，历史记录——①②③流程已由 r111 刷写+判读完成兑毕）**：①设备现态 = r110-rev2（ff2cf2d1）在刷 → **已由 r111（3a4bf72e）接替并判读毕**。②刷写卡流程（已执行毕）。③判读序（已执行毕，结果见 §0 r111 判读首刷段）。④**判读弹药** = temp-s99/（INTEL-s99.md + ce-model.md + fsck-ents-blob.txt + re_value_path.py 本会话判读脚本 + v5-probe.patch 现役探针源）——**extract/ 37GB 已删**（.aea 原件在 out/ipsw/24A446/ 可重解，scan-*.json 已抢救回 temp-s99/ 顶层）；Mac 侧出件记录 = ~/unixnu/r111-chain.log。⑤**挂账**：主树未推 711（origin/m2381 基线起；本会话新增 2：dfec0a4c 判读+施工 / 2a56c526 amend 含出件号+管线坑）、设备仓 8（1ab43bf=kit 两站+fixture sites 条目；KeyStoreProbe.cpp 旧 M 非本会话）；AMFI 唯一 unresolved 导入 donor 0xad3e8d8 仍挂；end7=[H+0x30]=0x620ed6480 真字段挂账（两轮均 << TLV 区，非本 TLV end）；r109"holder 回指"判读作废（f stackcopy 布局勘误见 §0 上段）。⑥**出件链管线三坑（备忘）**：hunk 头 new-count 含上下文行（`git apply --numstat` 验真）；生成 hunk 必语义读回；build 期禁 checkout 清树。⑦回退锚 = r106 `8a0c089d`；r110-rev2 = ff2cf2d1。

**★★★★★★★★★★s100 判读定谳（本会话，全静态 RE 闭环）**：r110-rev2 窗三行硬结果（`-l` ret=14/conv 三族全缺席/cont=pacia(0x9263a90)）指向 value 面。本会话把 value 面静态端到端拆完——**全链排除**：K(0x92638d8) 分派各 case 调的 handler 槽全解（dict→+0x30 壳 0x9264b4c→stub 0x92713f8→0xb3df420；seq→+0x28 壳 0x9264b58→0x9271468→0xb3d834c；string→+0x18 壳 0x9264bc4→0x92714d8=**F1 同函数**（r110 实证活）；bool→+0x8 壳 0x9264be0→0x92715a8→0xb3d921c=OSBoolean 单例 csel 永不 NULL；int→+0x10→0xb409490；data→+0x38 壳 0x9264b38→0x9271458→0xb3dc3bc）；cb1/cb2/C1/M/bool 助手 0x9262ef4 全体反汇编干净；**我们侧 GOT 七槽实绑逐位读出件 kernel 验证全健康**（bool=单例形逐位同 donor、dict/seq/data/int/string 同形、F1/F2 与 r109 reviewed 对账同）→ **转换器链零嫌疑**。**f 布局破案（勘误 r109"holder 回指"）**：f(0x9263780) 把 record 字段整体 +8 拷栈（CEType@0/TLV@8/total@0x10/tag@0x18/content@0x20/clen@0x28），`-l` 的 container=stackcopy+8，c28=[sp+0x30]=f 的 saved x20=holder 指针——回指=读出界伪影。**唯一活口 = M 的栈守卫假 trip**：M(0x926245c) 开场 `sp & ((1<<*GOT[0x8136458])-1) < 0x80 → ret 2`；donor 绑开机写 14 的页移位全局（init 0xb5a1d68 `mov w8,#0xe; str`），16K 页内核栈基对齐故不假；**我们 auto 通道把同槽绑到无名 0x7DFA120（__DATA_CONST+0x16120，文件值 0、TEXT_EXEC/PRELINK_TEXT 零写者=运行时垃圾）**，4K 内核栈起于 32K 对齐分配 +8KiB，donor 14 掩码在固定调用深度假 out-of-stack → 内层 M ret 2 → K=0 → cb1 release → 14——**sheng 车道实证过的同一堵墙，s52 拆 bring-up 时留案"first boot actually trips the AMFI entitlements stack check 才再推导"，现条件成立**。end7=[H+0x30]=0x620ed6480 为真字段（两轮均 << TLV 区，非本 TLV end，挂账）。

**★★★★★★★★★★s100 施工+出件全毕（2026-11-13 会话一百，r111 待刷）**：①主树 `patches/xnu-12377/kernel-stack-shift-guard.patch`（sheng bring-up stack.c 两 hunk 拆独立：`unixnu_kernel_stack_shift` 变量 + stack_init 整分配对齐 `roundup_pow2(stack+ptoa(3))`=15）+ series 条目；②probe patch 加 `_unixnu_amfi_entval_probe1/2`（blade-5/6：包 cb1 bl K @0x9263af8 word 0x97FFFF78 / cb2 bl K @0x9263b6c word 0x97FFFF5B，x0/x1 透传，ret==0 打印元素记录头 type/tag/ptr/len+**guard 变量实值**+victim+flush）+ exports 两行；③设备仓 fixture `sites[0]` 恢复 s52 条目（GOT 0x8136458→`_unixnu_kernel_stack_shift`）+ kit 两新站位；kit 六测试文件全绿。④**r111 出件 = kernel `40ac5f4f` / payload `3a4bf72e`**（`out/m2381/payload-r111-3a4bf72e.img`，103,809,024B，双端 sha 同；主树 959b6c30/设备仓 1ab43bf）。**静态门全绿**：GOT 0x8136458 新靶 0xfffffff007ded808=unixnu_kernel_stack_shift（写者=stack_init 尾 `rbit+clz`=ctzl 逐位验证，文件值 0=SECURITY_READ_ONLY_LATE 正常）、7 站位字全改写（blade-5 bl→0xB51ED54/probe1、blade-6 bl→0xB51EDD4/probe2 邻位合理）、emit 报告 text_word_patches=9（3 devmode+6 blades）、stubbed/missing 无回归（missing 2 同 r109）。**途中三坑（出件链管线级）**：⑤**unified-diff hunk 头 new-count=新侧总行数含上下文行**（3 ctx+381 add=**384**），三轮 379/380/381 全因数纯 + 行错——git apply 按 new-count 截断写入，多余尾行被静默丢（`--numstat` 一眼验真）；⑥generator 行序 bug：赋值行插到 `}` 后（h2_old[4:6]+post 次序错）→ stack.c 编译错 implicit int——round-trip 验 apply 不够，**必须语义读回**；⑦build 期间勿跑 `git checkout --` 清树（与 prepare 竞态，第一次"复现"是假象）。**刷写卡 = recovery_a 单件 ← `out/m2381/payload-r111-3a4bf72e.img`（sha 门 3a4bf72e），dd 读回对拍，set-active b，reboot，~3min 看屏 → 屏摄 → 硬断电 → 刷 reader 拉窗（recovery_a ← payload-reader-0f1013f4.img）**。**r111 判读预期**：守卫假 trip 定谳成立 ⇒ 内层 M 不再假 ret 2 ⇒ `-l`/v3/entval 全静默（probe 全冷路径）+ boot 越过 72s 墙进新组件/新墙；仍死同墙+entval 有行 ⇒ 行内 type= 值类定死元素级（bool=5/seq=2/string=4）→ blob 损坏面开刀；`guard=` 行（若出现）直报 shift 实值（预期 15）。

**★s99 死亡链全解（主会话+RE 子代理双路互证，全部 24A446 donor 反汇编实证）**：F(0x925d014 = `amfi->OSEntitlements_asdict` 实现，panic "failed to create OSDictionary copy: 14" @OSEntitlements.cpp:863)→G(0x9264c90 holder→context)→H(0x9263780 tag→CEType，位图 0x381 罚 {0,7,8,9}，**K 返 0 → H `mov w0,#0xe` = 第三个 14 源**)→K(0x92638d8 CEType switch [record+0]∈{1..6}；case-1=dict：slot_0x30 建容器+bl L)→L桩(0x9262690 门=[x0+0x10]∈{SET,DICT}，罚路径=0xa)→M(0x926245c 栈守卫 (sp&((1<<*g))-1)<0x80→2；ccder 校验+TLV 迭代，空容器返 13)→L体(0x92626fc pair 拆解 key/value TLV+value tag→CEType)→**callback 0x9263a90（s98 记忆的 "0x92630a90"=转写多零）**。**14 全区仅 3 条 mov w0,#0xe**（子代理全区 MOVZ 扫描）：cb1 0x9263b30（三源：key tag≠0xc / key 符号化 NULL / **value 递归 K 返 0→release→14**——s99 早期"value 失败非致命"判读作废）+ cb2 0x9263b90（序列递归）+ H 0x92638c4。**CE 错误对象 = {string,code 1..6}，14≠任何 CE 码** = AMFI 查询机内部"本元素不满足"码。

**★s99 关键转换器解构**：runtime 工厂表 = AMFI __DATA_CONST 静态对象 donor 0x8132e30（auth-rebase 槽指回 AMFI 内部壳簇 0x9264a00-0x9264be8）；**key 转换壳 0x9264b64 = 恰好两连调**：F1=`OSString::withCString(data,len)`（GOT[0x8135dd8]→donor 0xB410A94，Mac r109 map **reviewed**→我们 native 0x7a26f04）→释放→F2=`OSSymbol::withString(str)`（GOT[0x8135de8]→0xB4128a8，**reviewed**→0x7a28664）→**返回值=最终 key 对象**。**r109 GOT 绑定逐位验证正确**（emitter chain 格式 decode：auth=1 key=0 addrdiv=1；修正算术后两站目标相对差=0x1760=noctf 内 twin 相对差——非 stub 非 misbind）⇒ **NULL = 我们 native 实现真实返回**。F1 若返 NULL 会 panic 在 withString 的 [x+0xc] flags 读——干净 ret=14 ⇒ F1 成功、**NULL 来自 OSSymbol::withString→withCString，内部出口仅 kalloc_data(23,KHEAP_DATA_PRIVATE) 与 MetaClass::alloc**（pool/metaclass/site-tag 静态全健康）。桥真相（子代理 Q1）：CE 桥 = amfi_t 内嵌字段（amfi 全局 0x7f02a88 运行时注册，静态零）——F/G/H/K/L/M 非桥槽，AMFI 静态直呼+pacia 回调。

**★s99 两判读作废（探针伪影）**：①**r109 -l 行 cont=0/out=0x18 = probe v4 inline asm clobber 表漏 x1/x2**（编译器把 cont/out 常驻 x1/x2 跨 blr，打印=M 迭代器遗留寄存器；out=0x18 恰=首 key TLV 尺寸 24B=走查死在第一元素）——A18"非 fall-through 入口"判读作废（子代理 Q5 独立证明 donor 静态上该指令 x1=pacia(0x9263a90)/x2=sp 无条件确定，全区零入向分支）；②**post c8=0x1bf = H 建记录时填的 tlv_len 静态字段，非游标**——"游标走完全程"推断作废。**r106"存活"真相：每轮 boot 都死于第一个致命 entcopy（r106=114.4s SpringBoard 落地后），无非确定性魔法。**

**★r110 = probe v5（三刀，冷路径零热开销）**：blade-3 = 转换壳内 `bl F1`（donor 0x9264b80 word 0x94003256）→`_unixnu_amfi_entconv_probe1`：F1=NULL 时打印 key 字节+KHEAP_DATA_PRIVATE 堆健康；blade-4 = `bl F2`（donor 0x9264b88 word 0x9400325c）→`_unixnu_amfi_entconv_probe2`：x0=NULL 入口判别行（F1-handed-NULL）+F2=NULL 时打印 OSString flags/string 指针+堆健康；probe_l clobber 修复（x1/x2 改 "+r" in-out）→ -l 行恢复可信。**判读分裂树（r110）**：`f2=NULL ostr=...` 行 = key 符号化 NULL（预期主路径）→ 看 heap 行定位 zone 面；`f1=NULL` 行 = OSString 分配失败；**conv 行全缺席但有 -l ret=14 = value 路径或 cb2/H** → 下刀 blade-5/6（value 壳 0x9264b38 尾调站/K/H 面）；`-l ret=2` = M 栈守卫 trip（twin=_PAGE_SHIFT_CONST=14，16K 页邻近低概率独立致死面，若复现→修守卫全局绑定）。回退锚 = r106 8a0c089d。主树 d7056ccf（amend 含修复版补丁）、设备仓 821827a（blade-3/4 站位，7 条目）。

**★★★★r110 首刷 2s 崩定谳 + rev2 重出件（2026-11-13 s99 收尾）**：首件 kernel 31b03e47/payload f989c10e 刷后 **2s instruction fetch abort**，lr=0xfffffff00b51ccac 落在我们 probe2 内（0xB51CC58+0x54），pc=x16=0x1f0efff2934eb560=**probe2 的 `blr x16` 打到残留寄存器垃圾**——根因=probe2 的 inline asm 没把 x16 声明为操作数（裸寄存器变量只是提示，clang 直接不装载）；v3/v4 从没炸因为它们都写了 `"r"(x16)` 输入操作数。屏上 x23/x24/x25 = M 的 tag 常量原样活 ⇒ **整条 CE 链在 2s 被 early 消费者走通且 passthrough 零失败**（-l/v3 无行=ret 0）——机器早期健康，崩在我们刀上与 AMFI 无关（用户直觉命中）。LR=link 地址 ⇒ slide=0，`+vm_kernel_slide` 无害。**rev2 修复**：probe2 加 `"r"(x16)` 输入操作数，重出 **kernel `6424c2cb` / payload `ff2cf2d1`**（`out/m2381/payload-r110-ff2cf2d1.img`，103,809,024B，双端 sha 同）。静态门全绿：四站改写逐位验证（0x948afe43/0x948ae431/0x948ae005/0x948ae034）、**四个 blr x16 均有紧邻 x16 装载**（probe2=`add x16, x19, x21`）、新串 count=1、零回归（stub 22/missing 2 同 r109）。途中三坑：①构建启动与补丁推送竞态→xnu-work 暂存半截 mac_base.c（函数尾 `}` 缺失编译错）→稳定后重跑即愈；②`tree` 不重拷 staged kit（text_word_patches=5 静默出件）；③staged 树的 patches = 符号链接（推送源即生效）。**设备现态 = r110 首件已刷（2s 崩态）。刷写卡 = recovery_a 单件 ← `out/m2381/payload-r110-ff2cf2d1.img`（sha 门 ff2cf2d1），dd 读回对拍，set-active b，reboot，~3min 看屏 → 屏摄 → 硬断电 → 刷 reader 拉窗**。

**r110-rev2 判读预期（下一会话）**：若 2s 不再崩 ⇒ 探针修复生效，等同墙死点 ~95-115s msgbuf；**第一看 `grep unixnu-amfi-entconv parsed-msg.txt`**：`f2=NULL ostr=...`+`heap=...` 行 = key 符号化 NULL（预期主路径）→ heap 面定位；`f1=NULL` 行 = OSString 分配失败；`f1-handed-NULL` 行 = F1 返 NULL（证伪"F1 必成功"推理）；**conv 行全缺席但有 -l ret=14 = value 路径/cb2/H** → 下刀 blade-5/6（value 壳 0x9264b38 尾调站 / K-H 面）；`-l ret=2` = M 栈守卫 trip → 修守卫全局。**B 奖励情报：死亡 blob 主人 = /sbin/mount (com.apple.fskit.mount) @blob 0x1720d，8 对 entitlements（4 个 BOOLEAN true + mach-lookup SEQUENCE + iokit-user-client-class），与 23F84 逐字节相同**（temp-s99/fsck-ents-blob.txt + extract/os-root 37GB 可复用）。若再 2s 崩 ⇒ 新崩点读 lr 落谁家。主树 282c8873 / 设备仓 821827a。
**★★★★r110-rev2 取证回（2026-11-13 s99 尾，窗已拉取 = `reader-r110/`（log.bin sha 7d521d6b 双端对拍绿，39 槽鲜窗，uptime=69s，parsed-msg 733 行）——判读仅到分裂树定位为止，深判读交下会话**）：同墙 72s page-clock 复现（95.044s msgbuf，victim=launchd pid1，panic="failed to create OSDictionary copy: 14" @OSEntitlements.cpp:863，CRC f71048f1）。**三条硬结果**：①`-l` 行 `cont=4f92e73009263a90` = **pacia(0x9263a90, 0xe4fe)**——fall-through 传参终审坐实（s98 cont=0 伪影闭环），out=fffffff9faf8c3d0=sp 槽 ✓；②**entconv 三族行全缺席 = F1/F2 均成功** ⇒ **14 不在 key 符号化，在 VALUE 面**（cb1 的 value 递归 K 返 0→release→14，或 cb2 序列递归）——分裂树第三支命中；③探针 x16 修复生效（2s 不崩，4 站全过）。**r111 工作单（刀位已验字）**：blade-5 = donor **0x9263af8** word **0x97ffff78**（cb1 的 `bl K`，包 value 转换）+ blade-6 = donor **0x9263b6c** word **0x97ffff5b**（cb2 的 `bl K`，包序列递归）——双站单字 BL 照配方，probe 签名 (x0=value 记录, x1=runtime)，ret==0 时打印记录头（CEType@-8/tlv_len@+8/tag@+0x10/content@+0x18/clen@+0x20）+victim，冷路径零 flush。**价值面靶函数已定名**：value 符号化壳 0x9264b38 尾调 0x9271458→donor **0xb3dc3bc = OSData::withBytes(const void*,unsigned)**（map reviewed→native 0x79fb7b0）——注意 value 路径走 OSData 非 OSSymbol，与 key 路径（F1+F2 intern）不同源。死亡 blob = /sbin/mount 8 对（4 BOOLEAN true + mach-lookup SEQUENCE-of-string + iokit-user-client-class），SEQUENCE 递归/BOOLEAN 处理为头号嫌疑；对照 blob 与 23F84 逐字节同（fsck-ents-blob.txt）。判读弹性：若 blade-5 行缺席而 blade-6 命中 = SEQUENCE 值；记录 tag 可定位死在第几对。判读弹药 = temp-s99/INTEL-s99.md + ce-model.md + fsck-ents-blob.txt + extract/os-root（37GB，用后删）。

**★★★★s99 收队交接段（2026-11-13，下会话开工 = r110-rev2 判读；用户刷 rev2 后主会话/下会话提取日志）**：①**情报汇编 = `temp-s99/INTEL-s99.md`**（A/B 双代理全弹药：链全景/14 产生点/桥 27 槽/转换器家族/导入面 stub 表/blob 逐字节/exec 调用图——判读卡壳先翻它）；详档 ce-model.md、fsck-ents-blob.txt、extract/os-root（37GB，用后可删）。②**提取流程（设备 r110-rev2 崩后）**：屏摄（panic 块 CRC 照宏口径）→ 硬断电 → 刷 reader ESP（recovery_a ← payload-reader-0f1013f4.img）→ `python3 temp/acm-reader.py probe` → `read-m2381-ufs-log.py` / fetchchunk 拉窗 → `parse-m2381-window.py --msg` 存 `reader-r110/`。③**判读序**：第一 `grep unixnu-amfi-entconv reader-r110/parsed-msg.txt`（f1/f2/heap 行，分裂树见上）→ `grep unixnu-amfi-entcopy-l`（ret 值+首次可信 cont/out）→ 无 2s 崩则等同墙死点 ~95-115s。④**判读后的刀位预置**：key 符号化 NULL → heap/zone 面定位修；value 路径 → blade-5 = 0x9264b38 尾调站（B 非 BL，需尾调探针配方）或 slot repoint（0x8132e30+0x38 数据字站）；cb2/序列 → 同族。⑤挂账：主树未推 3 提交（282c8873/2b5f845f+早期）/设备仓 1（821827a）；AMFI 唯一 unresolved 导入 donor 0xad3e8d8（thunk 族绑 panic 桩）；extract 37GB 用后删；temp-s99 两代理报告原文在会话档案（要点已并入 INTEL-s99.md 与记忆档）。⑥回退锚 = r106 8a0c089d；r110-rev2 = kernel 6424c2cb / payload ff2cf2d1。

  **★★★A¹⁸ 出件完成（2026-11-13 会话九十八下半，开修轮）**：**r109 已出件待刷**——kernel **`847cc4f7`** / payload **`73212e67`**（103,809,024B；`out/m2381/payload-r109-73212e67.img`，双端 sha256 对拍同）。**施工全记录**：probe patch v4 程序化重建（S1=HEAD+iomalloc+devmode 基线——本地树陈旧（r104 代）不可作 diff 基线，r108 在 Mac 出件本地 tree 步未重跑；S3=S1+v3+probe_l，**round-trip 双文件字节级验证**）；L 桩门字段先 RE 后设计（0x9262690 `ldr x8,[x0,#0x10]`，合法=SET `0x2000000000000011`/dict `0xa000000000000010`）→ 探针**镜像门比较**：pre-call 快照 [container+0x10]，只打印"门通过仍失败"事件，常态门拒（w0=0xa 噪声）静默，行为透明；无 flush hook（writer 2s 周期+死时 v3 flush 必捕获 msgbuf 行）。kit 第 5 站位 + 6 测试组全绿。**静态门全绿**：双站已改写（blade-1 0x948afe43 与 r108 同、blade-2 0x948ae431→0xb51ca90）、探针体逐指令验证（`ldr x21,[x0,#0x10]` 快照+`x9=0xfffffff009262690`+`blr`）、`-l` 四格式串 count=1、v3 族原样。**坑**：补丁集变更触发 xnu-work 重建→Apple 头文件缓存全失→GitHub 拉取 TLS 掐×2——解法 = Linux 侧缓存整包对拷（EXTERNAL_HEADERS+iokit/DriverKit+sources JSONs）后第 3 次发车过（Restored 5）。**★★★★★r109 取证回（2026-11-13 会话九十八下半，用户刷 r109 后同墙复现——死者类大变+L 探针首战命中）**：窗 `reader-r109/`（SHA 对拍绿，39 槽鲜窗，uptime=69s）。**本轮 boot 形态大变**：死点 page-clock 72s（msgbuf 95.05s 双钟律 ✓）、**procs=2/spawn=4**（上代 84s/108/228）、屏 stage 行=fsck pid2 exec，**v3 victim 行=launchd pid1**——死者族从 xpcproxy(pid95) 换到 launchd(pid1) 执行的 fsck spawn。**blob 换族**：len=**0x1bf**（≠三代恒定的 0x332c），键串 "application-identifier"（0x16=22B UTF8），tlv=`b0 82 01 bb`+`30 2f 0c 16`，clen=1bb=len−4 ✓，isa/htag 依旧合法——**①分支（记录自洽+内容真 DER）在不同 blob 族上复证**。**L 探针首战命中（4 行，死亡事件本体，95.054500 起 70µs 后 v3 ret=14）**：`ret=14 ent=0xfffffff1788f0418 cont=0 out=18` + `pre c10=a000000000000010(dict tag✓) c18=fffffff7801827b0(=TLV+4✓) c20=1bb(clen✓) c28=fffffff39992ad78(=holder 回指✓)` + `post c8=1bf c18=…7b0 cm8=1` + `victim pid=1 launchd`。**三个定谳级事实**：⑴ **L 桩本体返回 EFAULT(14)**——不是 K 的 NULL→0xe 转换，EFAULT 在 L/M 走查层直接产生；⑵ 走查记录完全自洽（tag/content/clen/holder 全对），走查消费到 c8=0x1bf=TLV 全长（游标语义=下会话 RE 题）；⑶ **cont=0/out=0x18 ≠ K 落景色传参（x1=pacia(0x92630a90)/x2=sp）**——死路径进入 0x92639cc 的入口不是 fall-through（有其它分支直跳 bl，寄存器态不同）——0x92639d8 `cmp#0xc` 检查位区 = 分支密集区，下会话先 RE 入口分支族。**下会话=细分析+修复（用户指令）**：FAULT 面优先（EFAULT=VM 缺页语义：csb_mem 4K 映射覆盖度/blob 尾页 residency——r108 end7=页对齐指针 0xfffffff6217b8000 或为映射边界线索）；M 拒绝码面次之。

**判读预期（r110 判读轮）**：同墙死点 ~101-107s kernel clock；**第一看 `grep -E "unixnu-amfi-entcopy-l|unixnu-amfi-entcopy:" parsed-msg.txt`**（msgbuf 时序 = `-l` 行在前、v3 行在后，紧邻 v3 行的 `-l` 行 = 死路径）；`-l` 行 ret≠0 = 门通过体失败（ret 值+post cursor = 走到哪步被拒）；无 `-l` 行但有 v3 行 = 死容器门拒（x21+8 容器≠holder 形态，扩站面）。

  **A¹⁸. r109 施工+判读工作单（2026-11-13 会话九十八定稿，A¹⁷ ①命中后的进链刀）**：**病灶（现口径）** = 记录头/内容/指针全合法（A¹⁷ ①运行时三重复证）而 K 仍返回 NULL→0xe ⇒ 拒绝发生在 L/M 元素级迭代内部。**刀位（已验真）** = donor **K 的 `bl L` @ 0x92639cc**（word `0x97fffb31`→L 桩 0x9262690；K 体内唯一 bl L；单字 BL 站照 0x925d040 配方进 kit TEXT_WORD_PATCHES + probe patch v4）。**行序**（照 v3 模式：先调原体、ret≠0 才打印、逐行 flush）：row1 = L 入口三参 x0/x1/x2（x0=容器=x21+8 DER 记录、x1=continuation（PAC 形 0x92630a90 常量）、x2=sp 出槽）+ 容器前 32B hexdump（期待 dict tag 0xa000000000000010 落 +0x18）→flush→row2 = 返回 w0 + K 侧 0xc 检查位 [x21+0x18] 实值（donor 0x92639d8 `ldr [x21,#0x18]; cmp x8,#0xc`）→flush→victim。**判读分裂树**：w0=0xa → L 桩门拒（容器 tag 非法——与 holder htag 合法矛盾 ⇒ x21+8 容器≠holder 记录形态，查 G `add x0,#8` 语义）；w0∈{0xc,其他非零} → 体元素失败（M 迭代拒某元素 ⇒ r110 站进 M 迭代器/L 体 pair 处理器）；w0=0 → L 过而 K 仍败 ⇒ 死点在 K 尾部（0x9263a2c 之后路径再 RE）。**面告警**：L 桩 **11 个 BL 调用者**（0x925c380/0x9261f6c/0x9261f9c/0x92620f0/0x92622bc/0x9262c0c/0x9262d50/0x92639cc/0x9263d48/0x92649d0/0x9264e88）——站只覆盖 K 路径（死亡路径 F→G→H→K→L 已由 r106-r108 三代 msg 行闭环）；若 row 缺失但仍死同墙，再扩站。**施工建议**：双站方案 = v3 的 0x925d040 站保留（ret=14 锚）+ 新增 L 站，同行对照；probe patch 增 `unixnu_amfi_ent_copy_probe_l(container, cont, out)` + Mach.exports 导出。**出件链** = Mac（r108-chain.sh 改 r109 名）；**判读惯例 = 第一看 parsed-msg.txt**（死点预期仍 kernel clock ~101-107s；~3min 到屏；屏摄 → 硬断电 → reader 拉窗 → --msg）；回退锚 = r106 `8a0c089d`。

  **A¹⁸ 交接段（2026-11-13 会话九十八收队更新，下会话开工 = 判读 r109 + 修复；施工/出件已毕，用户将回传屏摄 + reader 通道）**：①**r109 已出件**（kernel `847cc4f7` / payload `73212e67`，双端 sha 同；`out/m2381/payload-r109-73212e67.img` 在 Linux，刷写卡已发——Mac 侧同名件 = `payload.img` 未改名，勿拿错）。②判读流程 = reader-r109 目录拉窗 → parse → **第一看 `grep -E "unixnu-amfi-entcopy-l|unixnu-amfi-entcopy:" parsed-msg.txt`**（msgbuf 时序 = `-l` 行在前 v3 行在后，**紧邻 v3 行的 `-l` 行 = 死路径**；屏摄永无探针行，只核 panic 块 CRC）。③分裂树（A¹⁸ 单）：`-l` 行 ret≠0 = 门通过体失败（ret 值 + post c8 cursor = 元素走查止步点）；无 `-l` 行有 v3 行 = 死容器门拒（x21+8 容器≠holder 形态 → 扩站面）；无 v3 行 = 站面异常回查。④**修复方向预置**：体元素失败 → post cursor 定位走查止步元素 → 对照 23F84 donor 真 blob 同键字节（out/ipsw/23F84 dmg 扫 0xfade7172/slot7 配方）分两支：我方供给截断/错位 → 修 csblob_find_blob_bytes（ubc_subr.c:3269 族）供给；元素类型合法但被拒 → RE M 迭代器拒绝分支（11 调用面已扫：0x9255390/0x92626e0/0x9262928/0x9262a50/0x92632a0/0x926342c/0x9263518/0x926360c/0x9263a24/0x9264970/0x9265108）→ r111 站进 M 或按拒绝码修。⑤工具现役 = temp-r107/dis24.py + scan_tags.py、kc.py（`XNU27_KC=…/24A446/kernelcache.decompressed`）、ipsw macho a2s、/tmp 扫描器配方（BL/B 全扫 + panic 串定位 + pacibsp funcstart 回扫）、Mac 链 r109-chain.sh。⑥主树未推累计 53（origin 跟踪基线起，本会话新增 3：f613b7f2 判读 / r109 出件 / 本交接）、设备仓 4（含 r109 站位）；推送窗口 = 用户拍板。⑦回退锚 = r106 `8a0c089d`。
## 1. 一句话现状（**2026-10-31 会话二十四收档：23F84 fileset 车道，matcher v11 判别子通道落地，战略拍板 = 关键路径优先**）：

**★★★r80 里程碑墙（2026-11-07 会话七十五收队用户报点，屏摄未转录——判读单见下）**：
  刷 w79fix-07844e4e 后屏现 **`root window read failed`**（无 CRC 转录、无诊断报文细节在案）。**源位 = `NativeUfs.cpp:1925`** = `NativeUfsBlock::doAsyncReadWrite` 失败分支（`ufsReadWindow` 返回 status ≠ success → `gRootDiagnostic.result = result` → `rootDiagnosticFailure("root window read failed")`）。**里程碑定性**：此路径要求 ①readPolicyObject ✓（墙78 修复兑现）②vr_open 验签 ✓（车道史首次）③**窗口已发布**（published ≥ 1，此前三轮恒 0）④**上层栈（IOStorageFamily/apfs/SourceStorage Data disk→root mount 链）已对窗口发起真实块读** ⑤per-4K 完整性 verify 回调路径在役（`native_ufs_verified_blocks`/monitor `verified4K` 计数）。**kfree-var 修复（#149 改绑真 impl）真机兑现——s58 期 98 槽 delete 面全程零堆错配**。
  **r81 工作单（新会话开工单）**：①**判读先要屏摄**（用户手上有）：CRC 钉消息 + 若诊断报文上屏读 `result=` 码分流 + monitor 行 `published=`/`verified4K=` 计数（非零 = I/O 真发生过）。②result 分流：`kIOReturnNotPrivileged(0xe00002bc)` = verify 完整性失败（表 vs 读回数据哈希错 → 查 hostRead4k lba 算术/DMA 一致性/表读偏移）；`kIOReturnIOError` = 传输层（SMMU/ UTRL）；`kIOReturnBadArgument` = 几何（`ufsWindowGeometry`/block×512 越界）。③静态对账第一刀 = `RootWindowRead.h` `ufsReadWindow` 的 lba 换算（`start = absolute + block*512`，窗口 absolute = `userdataFirst*4096 + window.offset`——r74 对象 offset 0x3a3b00000 期与 GPT/read lba=0x238f68 互核）+ `verify` 回调的 `windowBlock = lba + index - absolute/4096` 算术。④诊断旗标在役（native-ufs-diagnostics=1），失败时 gRootDiagnostic.result/stage 直报。⑤回退 = recovery_a ← `232a43c0`（w78fix）。

**★★★r79 判读+修复轮（2026-11-07 会话七十五续，屏摄 IMG_20261107_111130）**：
  **①墙78 修复实锤**：UFS ROOT FAILURE 不复现——`readPolicyObject` 整读真机兑现（free 降 0x21FA 页 ≈ 18.7MB 表读入），链首次推进到 kext delete 面。**②新墙 79 = 内核堆完整性 panic**：`kfree: addr 0xfffffff87ad0f980, size 192 found in heap data.* instead of * @kalloc.c:2824`（CRC `31b8ea39` 一次零纠正命中，89B；宏口径第三次实战——屏 %p = 最小十六进制渲染，转录多一 f 被口径裁决；addr 落 0xfffffff8 高段 = s71"内核堆银行"互证）。**③机制三层钉死**：`kfree_zone` 对 mangled view（bit0=1，kalloc.h `kt_mangle_var_view = view|1`）走 `kheap = kalloc_type_get_heap(ktv->kt_flags)`（view-respecting）；报文 "data.*"=zone_heap_name(z)，空串*=KHEAP_DEFAULT——**data 堆 192B（kalloc_type）对象被 plain kfree 释放**。元凶 = **s58 止损 shim**：donor `0xb441c0c`（IOLib 导出簇 kfree_type_var 尾块，12B = `bti c; orr x0,x0,#1; b kfree_ext`，98 槽 import 面）被 r58 reviewed record 绑到 `unixnu_kfreevar_shim`（丢 view 强转 KHEAP_DEFAULT，设计假设"free 的都是 iomalloc_shim default 堆对象"）；但 kalloc_type/data 堆对象（OSObject typed delete 面）同路由 → 必错配。**此路径 23F84 期从未执行**（98 槽里 16 槽 emitter 当年已绑真 impl，82 槽哨兵被 shim 盖住），24A446 车道 kext delete 面因 AMFI 解锁+root 窗口 prepared 才首跑。**④上游对账（用户问"16/17 或上游"）**：16/17 车道止步远早于此 = 面不存在；**上游 ledger 无此 shim record，emitter 自动通道绑的就是真 impl**（s58 档案原话"16 槽 emitter 已绑 native kfree_type_var 保留"）——修法 = 字面照抄上游行为。**⑤修法 = 账本 #149 改绑 `_kfree_type_var_impl`**（native ..732d39c，12B 逐构 twin；map 实证 `how=reference-symbol` 生效；iomalloc/iofree shim 不动——donor IOMalloc/IOFree KPI 本就是 default 堆配对正确）。单测绿，设备仓 defa9a4。**⑥出件**：noctf b7b22fde 不变 → prelink `4724c877` → payload **`07844e4e`**（`out/m2381/payload-w79fix-07844e4e.img`）。**刷写 = recovery_a 单件**（userdata e01c27ae 不动）；回退 = recovery_a ← `232a43c0`（w78fix）或 ← `78ca0c07`。**r80 判读预期**：kfree-var 面全走 view-respecting impl（default 配对面零回归 = iomalloc/iofree shim 照常）；下一墙 = 98 槽 callers 的更深 kext delete 面（AMFI/IOStorageFamily/apfs start 序）或 root 窗口发布（prepared=2 → SHLOG state=2 大捷旗标）。

**★★★r78 判读+修复轮（2026-11-07 会话七十五续，屏摄 IMG_20261107_105511，诊断旗标立功）**：
  **①诊断报文一次报点**：`UFS ROOT FAILURE: root window preparation failed`，**`stage=policy table object read`**（十选一直接命中）、`result=e00002bc`、`WINDOWS found=2 prepared=0 published=0 bytes=23d000000 policies=2 verify=ffffffff`、`pubPhase=1 window=0`、`IO reads=0`（失败在任何 UFS I/O 之前 = 对象查找/尺寸校验层）、`GPT magic=474e4853 544f4f52`="SHNGROOT"（布局有效 manifest=0）→ 容量匹配/分配全过，败在 `readPolicyTable`。**②墙78 根因（pair 实测铁证）**：r74 接线把**完整 manifest.bin（header 160B + signature 384B + hashes.bin）**stage 成 `<window>.vroot` 对象（OS=18,776,608B / SystemOS=11,751,968B），而 `readPolicyTable` 校验 `object->size != tableBytes`（=hashes.bin 纯表 18,776,064 / 11,751,424）——**差恰 544B 签名头，必 false**；header[32..40] count×32 = hashes.bin 两窗口逐位自洽（586,752/367,232）。此路径 r74-r75 **从未执行过**（被 ：102/SMMU/AMFI 三面墙挡住），first-panic-wins 时序翻案的实锤案例。**③修法 = `readPolicyObject` 整读完整对象**（设备仓 ab99385）：对象字节 = 签名原文，vr_open 验 staged 原件而非头部重组（编译头与对象前缀同字节同源，消除双源分歧面）；对象 offset 4k 对齐 = 块循环保持原形；单次 hostRead4k 分块循环不变。**④出件**：noctf `b7b22fde`（xnu 源不变 = 参照件缓存 b0d7eb2c 复用）→ prelink `c4f1af9b`（**text_word_patches=3 维持**）→ payload **`232a43c0`**（`out/m2381/payload-w78fix-232a43c0.img`，99MiB）。**刷写 = recovery_a 单件**（userdata e01c27ae 不动）；回退 = recovery_a ← `78ca0c07`（w77diag）或 ← `a1fc030e`（w76fix）。**r79 判读预期**：readPolicyObject 过 → vr_open 首次真机执行（trusted-key 对 pair 验签 OS/SystemOS 两窗口）→ 若 vr_open 败则 verify= 状态码 + stage="owner signature and hash table" 报点（诊断旗标在役）→ 过则窗口发布（prepared=2）→ Data disk → root policy → **root mount 首跑前沿**（SHLOG 窗口 state=2 = 大捷旗标）。

**★★★r76 首刷判读 + r77 诊断旗标轮（2026-11-07 会话七十五续，屏摄 IMG_20261107_103514）**：
  **①墙76 死实锤**：t=31s 屏上无 `"AMFI: No PMGR?"`，stage 行推进到 `bsd_init: calling IOKitInitializeTime?` 之后的 process_name/get_bsdthread_info/chgproccnt/kmem_suballoc/bsd_bufferinit（**车道史最深**）；r74 的 `:102` 政策认证门同屏证实已过。三站 shim 语义 0=benign absence 真机兑现。**②新墙 77 = `SourceStorage: UFS start failed (root step 3: root window preparation failed) @SourceStorage.cpp:124`（CRC `99186537` 一次零纠正命中，100B——宏口径第二轮实战裁决屏摄 OCR 多一位前导零）**。**③机制**：panic = SourceStorage.cpp:123 包装（start() 返 false），native_ufs_root_step=3 = findRootLayout/manifest 检查已过（否则报 "root manifest rejected"）→ `ufsPrepareRootWindows` 返回 phase=**Prepare**（`ops.prepare(i)` 败：OSTypeAlloc/init/`ufsWindowGeometry`（几何全宽必过）/IOLockAlloc/IOMalloc/`configureVerification` 十 stage 之一；Schedule/Identity 兜底串已排除）。**④时序翻案（first-panic-wins）**：r75 判读"Data disk→root policy 过"系从"死在 AMFI"倒推（循环推断）；AMFI latch 与 SourceStorage init 并发，r75 的 AMFI@31s 只是**先上屏**——step 3 失败可能是 r75 期已被 30s 等待掩盖的老病，r76 修 AMFI 后现形（代码零改动却"回归"的自洽解释）。**⑤r77 修法 = 零代码诊断旗标**：`rootDiagnosticFailure` 本有全量诊断 panic 通道（`formatUfsRootDiagnostics`：**stage=** 十站报点 + WINDOWS found/prepared/published/bytes/policies/verify= + IO reads），开关 = boot-arg `native-ufs-diagnostics=1`，现役 kernel_args 漏带 → 屏上只有兜底串。board.toml 已加旗标（设备仓）、payload 重出：**kernel 2f856b18 逐位不变** / payload `78ca0c07`（`out/m2381/payload-w77diag-78ca0c07.img`）。**刷写 = recovery_a 单件**（userdata e01c27ae 不动）；回退 = recovery_a ← `a1fc030e`（w76fix，静态门已验）或 ← `7c5c39e5`。**r77 判读预期**：panic 换成 "UFS ROOT FAILURE" 多行报文，stage= 行直指失败站（policy count/envelope/duplicate/no capacity-matching/header block count/kmem_alloc/policy table object read/mach_vm_protect/vr_open/attach 十选一），CRC 照宏口径直算；按 stage 定位后走对应修法（readPolicyTable 败 → 查 hostRead4k/SMMU 面；capacity/vr_open 败 → 查 pair/账本面）。

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
| recovery_a | reader-esp（读者件 `payload-reader-0f1013f4.img`） | `0f1013f4` | **现态 = reader**（r111 窗已拉取判读移交下会话；下会话修复出件后单件刷回 payload） |
| userdata | 24A446 tables-in-userdata 件 | `e01c27ae` | **现役**（不动） |

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
