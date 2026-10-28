# STATE — 现势状态（m2381，2026-10-28 更新）

> **历史全档**：2026-10-24 → 2026-10-27 全部轮次（§0.0–0.34）与当时静态区
> 在 `docs/archive/STATE-history.md`（原样快照）；每轮定谳同时完整活在
> git commit message 里。本文档 = 精简现势版：现状 / 资产 / 方法 / 开放
> 问题 / 下一步 / 轮次索引。

> **2026-10-29 结构变更：设备仓拆出独立仓**
> [UniXNU-m2381](https://github.com/RiverKawa271828/UniXNU-m2381)（本地
> `../UniXNU-m2381`；`git subtree split` 保 93 提交全史，独立实跑
> check-m2381 = 4 ok 0 fail）。本文与历史档中所有
> `devices/m2381/unixnu-m2381/…` 路径现指该仓对应路径；构建/取证命令
> 改为 `-D ../UniXNU-m2381` 与 `../UniXNU-m2381/scripts/…`；CI dispatch
> 改指新仓（见 `docs/CI.md`）。

## 1. 一句话现状（**r77 已刷已判（2026-10-29 凌晨）+ 战略转 =y**）：

**四连击达成 = #30 实锤生效**（窗 078a8227 208/509 uptime=417s，
recovery_a 读回对拍 `93854822…2d35b` ✓）：'Failed to initialized
main screen scale!' **0 出现** + applied **25→26**（parsed=30
applied=26）；**墙链五连单调后移全景**：nil-set（r73）→ workspace
门（r74）→ position NaN（r75）→ bounds NaN（r76）→ mainScreen
scale（r77 全灭），每轮 applied +1、每轮预注册分支①精确命中。
**PB 'configuration' CRASH 133→8 暴跌**；chronod sig5 ×348（独立
主题）；回归六项全绿 + sig11 断言族 0。**新墙 = SB sig11 ×172 全
新高质量点**：`-[WGCarouselListViewController
_updateCarouselEffect]+0x9dc` → 崩在 `__os_log_impl_flatten_and_
send` 区，FAR = `0x4000000000000018`（bit62 标签态）；全链符号化验
活 = `CFXRegistrationPost → -[SBIconController
_rootFolderDidChange] → _newTodayViewController… →
SBWidgetController newAvocadoWidgetListViewController… →
updateCarouselAndRevealState → _updateCarouselEffect`——**SB 已深
达今天视图（avocado）widget 列表装载**。RE 定性 = 崩点读
`___sharedInstance.onceToken+0x8`（WidgetKit 持久状态控制器单例域
0x231ec4060，与 #30 scale 槽同页）文件态干净全 0 → **运行时
0x4000…0018 = once 块从未成功完成的毒化标记被当对象指针**——**果
非因**（r72 FatalCorruptHashTables 同款机制）：要修须找 once block
内部的失败因（疑 = dataSourceManager init 内又撞供给族），**非十
分钟定点，本轮不出 fixup 件**。**屏面对齐：进度条 1:30 出现（
r76 1:24 同量级）仍冻结**。**战略评估（用户拍板征询，本会话定
谳）**：五轮数据 → ①决策规则未触发（无复发无回退）②墙已到
"once 域毒化"层 = 供给族缺失的**下游果**——逐墙喂常量的边际收益
开始递减（每堵新墙都可能又是另一 once 域的果）③**真正的分叉点不
是 donor 而是 =y 显示供给链**：displayless 是根因，换 iPhone
donor 这些墙同形重长（换 donor 不解决 displayless），只有 =y 接
通真显示配置供给族才成片消失。**拍板 = 修复继续但换主攻：用户态
fixup 降为支路（供给族遇一堵修一堵不再整轮追），主菜转 =y main
display 注册链攻坚**；止损线 = =y 攻坚 2–3 会话，若 =y 卡死且供
给族又积 5+ 堵则评估中间路线 B（保内核换 iPhone donor 21H16）。
r77 崩点全链符号化与 once 毒化机制已存档（§8 r77 行）。**r76 已
刷已判（2026-10-29 凌晨）：fixup #30 =
`_WGMainScreenScale` block invoke 供给 2.0 替代断言
（`.37,2117fac,ffc300d1f44f01a9fd7b02a9fd830091,
0010601ee8cf17b00019803dc0035fd6` = 入口 4 指令覆盖
`fmov d0,#2.0; str d0,[once 槽 0x231ec4030]; ret`）。**r76 已刷已
判：分支①再命中 = #29 实锤生效**（窗 acd75917 271/509
uptime=544s，recovery_a 读回对拍 `2bdd272a…d287808` ✓）：bounds
NaN **0 出现**（position 也保持 0）+ applied **24→25**（parsed=29
applied=25）。**换墙 = `'Failed to initialized main screen scale!'`
×136**——**性质换挡：从"可清洗垃圾"进入"环境供给缺失"**（抛点 =
`____WGMainScreenScale_block_invoke`（WidgetKit 域 .37
0x2024c7fac）：读 `[[UIScreen mainScreen] scale]` 写 once 槽
0x231ec4030，≤0 即断言（line 18）；displayless mainScreen=nil →
scale 恒 0 → 必死。**修法换型 = 供给替代清洗**：清 NaN 有 Apple
自带路径可借，scale 没有供给源，只能我们喂（fmov #2.0，fixup 16b
先例；写槽+返回语义完备，零入边）。PB 'configuration' ×133；chronod
sig5 ×338（增量观察项）。**回归六项全绿**：applied 25 / AKS 116 开
1207 调 / vsync 120 / SWAP accepted=1 / spawn 970 / noidle 544s 无
停写无 halt；sig11 CRASH 0。**屏面对齐：进度条 1:24 出现（r75
1:54 提前 30s）但仍冻结** = 布局链推进更深后死在 scale 供给（提前
= 布局循环更早走到渲染准备 = #29 的间接证据）。**决策规则判定 =
有进展**（三连单调后移：workspace 门 → position → bounds →
mainScreen scale；墙越越接近环境本源——**用户态治愈已进入收尾
区：剩余墙候选 = scale/几何供给族与 PB displayConfiguration，都是
"缺显示配置"这一根源的下游**）。r75/r76 判读存档 = §7.3 与 §8 行）
`CA::Layer::set_bounds` 同构 NaN 清洗（`.03,26e0068,600c0034,63000014`）。
**r75 已刷已判：分支①精确成立 = #28 实锤生效**（窗 d361a0c3 250/509
uptime=501s，recovery_a 读回对拍 `23868de1…a6a461` ✓）：'CALayer
position contains NaN' **0 出现** + applied **23→24**（parsed=28
applied=24）；**换墙 = 孪生检查 `CALayer bounds contains NaN: [0 0;
nan nan]` ×111**（预注册预期候选命中：position→bounds 同函数族推进；
抛点 = `CA::Layer::set_bounds` .03/0x18948c068 链 +0xbc raise，与
set_position 完全同构：fcmp NaN 检查 + audit 豁免 cbz w0 →
raise:format）→ #29 = 同法强制清洗路径（expected `600c0034` 实读核
验，零入边）。PB 'configuration' ×98（r74 ×89 同量级）；chronod
sig5 ~223。**回归六项全绿**：applied 24 / AKS 117 开 1221 调 /
vsync 120 / SWAP accepted=1 / spawn 951 / noidle 501s 无停写无
halt；sig11 CRASH 0；X5 活值继续（I/R/S 三槽）；W1 SS 两形正常
（SB 0x205f36000 / PB 0x213fa8000 滑后）。**屏面对齐：1:03 logo +
1:54 进度条出现但冻结（与 r74 相同形态 = 布局循环仍死在 bounds）**。
**决策规则判定 = 有进展**（连续两轮墙链单调后移：workspace 门 →
position → bounds，全是首现新检查点）。r75 RE/方法详见 §7.3 存
档）
`CA::Layer::set_position` NaN 抛点强制走 Apple 自带清洗路径
（`.03,26850c4,40090034,4a000014`）。**r74 已刷已判：预注册分支①
成立 = #27 实锤生效**（窗 344e72e5 325/509 uptime=650s，recovery_a
读回对拍 `b0f2f656…a6a461` ✓）：workspace 门 'you can't call this
before [SBMainWorkspace start]' **0 出现** + applied **22→23**
（parsed=27 applied=23）。**换墙 = SB sig6 `CALayerInvalidGeometry`
'CALayer position contains NaN: [nan nan]'（delegate =
WGShortLookStyleButton，WidgetKit 控件）×~296——SB 首次死在真实视
图构造层**：过 workspace 门后在布局 widget 按钮，NaN = displayless
0 scale/bounds 几何（r60 族 0 scale 在 UI 层现形）；**用户屏面对齐
= 2:30 画出进度条（真 boot progress UI）但不动（视图布局循环
死）**。抛点 = `CA::Layer::set_position`（QuartzCore .03
0x1894310c4 链，+0x98/+0x1ac 两 raise）；**Apple 自带解药** =
audit-exempt 分支（0x1894311ec：log + NaN→0 fcsel + w2=0 递归重设
不抛）→ **#28 = 0x1894310c4 `cbz w0`→`b` 强制豁免**（expected
`40090034` 实读核验，零入边）。PB 'configuration' ×89（r73 ×164 减
半，REASON 不变）；chronod sig5 ~427（独立主题）。**回归六项全
绿**：applied 23 / AKS 116 开 1189 调 / vsync 120 start+refresh /
SWAP accepted=1 / spawn 960 / noidle 650s 无停写无 halt 无 sleep
timer expired；sig11 CRASH 0；X5 探针升级出活值（B0 M0 + I/R/S 三
槽）。**决策规则判定 = 有进展**（墙链单调后移：生命周期门 → 渲染
几何；换内核讨论不触发，见 memory kernel-donor-switch-decision-
rule）。r74 RE 修正段（"raiser 在缓存外"作废 + raiser = fw 内
SBMainWorkspace `_sharedInstanceWithNilCheckPolicy:`）见 git
a48e7ac 与 §4.9）
`+[SBMainWorkspace _sharedInstanceWithNilCheckPolicy:]` policy==1
nil-check 断言 kill（`cbz→nop`，nil 改为返回 nil）。**r74 RE 修正
（同日二判）**：r73 的"raiser 在缓存外 SpringBoard.app 盘上二进
制"结论**作废**——根因 = 扫描器 add 掩码 bug（`(w2 & 0x7F800000)
== 0x91000000` **恒假**（0x91000000 & 0x7F800000 = 0x11000000），
正确掩码 = `0xFF800000`；即 STATE §4.4 r57 同款坑第三次复发，当日
从首个脚本传染到全部 xref 扫描——只有 adrp+LDR 形可见、adrp+ADD
全盲）。**修正后 RE 定谳**：reason CFString 0x1ea27d3c8 唯一 xref
= `+[SBMainWorkspace _sharedInstanceWithNilCheckPolicy:]+0xc8`；
方法本体（fw .25 @0x1cf9a45ac）：policy==1（b.eq 路径）实例槽
`0x1f56cf6a0` nil → 0x1cf9a464c 断言路径 → NSAssertionHandler
`handleFailureInMethod:file(SBWorkspace.m CFString):line 0x6b7
(1719):description(reason CFString)` → sig6；定点 = 0x1cf9a4630
`ldr x8,[x22,#0x6a0]` + 0x1cf9a4634 `cbz x8,assert`（**零入边**，
安全 NOP）；`expected=c85243f9c80000b4` / `replacement=
c85243f91f2003d5`（expected 已 cache-extract 文件实读核验）。**方
法学新沉淀（§4.9）：零引用结论必须先过阳性对照**——BaseBoard 的
NSParameterAssert 格式串 CFString（PB 断言 ×94 每轮必经）在坏掩
码下同样"零引用"，阳性对照一击证伪工具箱。SB.app 壳二进制 120KB
不含该串的部分仍成立（非缓存框架差集也干净）。**r73 已刷已判
（2026-10-28）**：**预注册分支①成立
= fixup #26 实锤生效**（窗 451ed5c2 301/509 槽 uptime=609s 无回
卷，recovery_a 读回对拍 `a3f310d42f488a66…585e56d` ✓）：`[text
fixup] parsed=26 applied=22` = 21→22（#26 页执行 oracle）+
**NSPlaceholderSet objects[0]=nil 断言本代 0 出现**（r70–r72 三代
×33/×347 的原墙关账，r72 定谳的 nil 调用者链被 #26 拦死）。**换墙
= SB sig6 ×266 新点**：REASON = `NSInternalInconsistencyException
'you can't call this before [SBMainWorkspace start]'`（×172 行）=
预注册预期①"SBLockScreenManager=nil 的下游"形状命中：SB 反复死在
一个 workspace-start 门；死亡形状单一（PC 0x1d95e6f74 = abort 链
噪声点，bt 8 帧全终结链 = 抛点帧被 unwind 撕净，dispatch_async_
and_wait invoke 外层）。**raiser 定位 RE（本会话主破案）**：REASON
串 → 全缓存唯一实例 0x1d02917bc（SpringBoard.framework
0x1cf99c000 内，`SBWorkspace.m` 文件标记串 0x1d0291785 紧邻，同池
邻居 = sceneManager/transientOverlayPresentationManager/
currentTransaction/daemonHandlers）→ CFString 对象 0x1ea27d3c8
（isa/flags/datap/len=50 全验证）→ **引用模型扫尽 = 零引用**：
①adrp+add/ldr 配对（相邻 + 12 指令宽窗，SB fw text 全区 + SB 主
二进制窗 + 全部小段）②绝对指针槽（低 48 位 + bit42 tag 全缓存
57 映射）③i32 相对指针（SB 邻域）——全零，仅存的两个指针槽 = 两
个 CFString 对象自身的 datap 字段 ⇒ **框架内这套断言常量 = 死拷
贝；活 raiser 在缓存外 = SpringBoard.app 盘上主二进制**（镜像表实
证 /Applications/SpringBoard.app/SpringBoard 不在 21H461 缓存，
缓存只有 framework + AccessibilityBundles 拷贝）⇒ **现 cache
text_fixups 机制够不着 r74 修点**（fixup 挂 dyld 缓存文件页校验路
径；盘上 Mach-O 另一套）。**支路③核对 = PB 独立墙未动**：sig6
×164 / 'Invalid parameter not satisfying: configuration' ×94（r72
同墙）；chronod sig5 ~400 行（r57 已知独立主题）。**回归六项全
绿**：applied 22 / AKS opens=118 calls=1248 / vsync hz=120
start=1 refresh=1 / SWAP accepted=1 bytes=10.3MB / procs=196
spawn=962 / noidle 维持（609s 无 139s 停写 + PMRD 零 `sleep timer
expired` + 零 halt）；sig11 CRASH 行 0 ✓；X5 dirty 窗 R0 全窗读
通保持；W1 SS 三形（SB 滑后 0x213d5e000→去滑 0x205f02000 / PB
0x221dd0000→0x213f74000 = **跨 boot 固定装载槽 r72 实证再证**；
第三形 b1d15f9000→0x1c379d000 ×3 归属未定）。**扫描器三坑再付费
（全部入 §4 方法）**：①fsx needle 要求紧随 NUL → 短语式搜索恒 0
（找串要直搜 data.find）；②按映射起点过滤会误截跨段区域（必须重
叠式判定）；③手工解 imm12/去滑又错两次（imm12 手算 0x368 实为
0x478；b1d15f9000 手算 0x1cf3039000 实为 0x1c379d000）——**"手工
必须走 python"一天内第三次付费**。r72 判读（nil 调用者指令级钉死
全链）见 git a0447d2；r71（noidle 关账/扫描器三改）见 §7 存档；
取证双铁律（header 分代 + re-tools read_run）已工具化）
**r64 出件（2026-10-28，本机 Linux 全链）**：静态腿 = 21H461 缓存头 /
slide-v2 链格式按 dyld-1241 源码破译（链 delta 在最高 16 位、4 字节单
位；值 = 低 48 位 = 绝对未滑 VA；value_add=0），UIKitCore
**__DATA_DIRTY**（0x1f489fef0+0x39958）全链枚举 5410 槽定性（objc
类/METACLASS/RO 结构 + UIUpdateSequence 哨兵）；**四个病灶槽
（bgTC 0x1f48ce5e0 / 锁 0x1f48ce340 / __UIMainScreenTraitCollectionLock
0x1f48cf938 / mainScreen 单例槽 0x1f48d5550 = __MergedGlobals+0x20）
文件态全 0** = 懒缓存 nil 静置态，毒在运行时写路径（r63 定谳复核；
r58 崩溃点指令级复验：+0x2c = `ldr x8,[x19,#0xe0]`，x19 =
[UIScreen mainScreen] 槽值 0xf00 → FAR 0xfe0；r63 PC 0x1800F5FAC =
objc_retain 快路径 `ldr x8,[x0]`，x0 = bgTC 槽值 0xf1 → FAR 0xf1；
r62 野跳点 = CoreServicesInternal cstring 零填充 = 垃圾 IMP 落点）。
**工作假说 = ObjC 元数据/once 域损坏级联**（同时解释野 PC 与小整数
值；0xf00/0xf1 跨 slide 不变 → 偏向数据常量或确定性页内偏移，排除
"指针减错基"）。动态腿 = **UIKitCacheProbe.c**（startup.o record 编
译私有 task/vm_map ABI，BootFaultProbe 先例）编入
libNativeLegacyDisplay.late.a；2s 轮询 SpringBoard
task_shared_region_slide，vm_map_read_user 全窗扫描 __DATA_DIRTY，
发布 ext 行 4-7（X4 tick/状态/slide/非零/垃圾计数，X5
mainScreen+bgTC 槽值，X6 首垃圾事件，X7 最近变更事件）；门控
`uikit-cache-probe=1`（+`uikit-cache-proc=` 改目标进程）；
`boot-ui-focus3=Preferences` 移除（cmdline 1002B > 1000 上限）。
25 条 fixups ADT 验证（24 dyld + BTAudioHALPlugin；16a/16b 字节级
精确）；内核重链 `e86478ad`；ESP 四样全过；**payload r64 =
`54b70ad7`**（96MiB，平铺 `out/m2381/payload-r64-54b70ad7.img`）。
判读工作单见 §7。

**调研定谳（2026-10-28，上游 f85b5a6→0fc0fc1 + 四个过墙板对照）**：①
**text_fixups 全清单**：sheng = 4 条基线（AWAttention 死锁 + 2×PRSPoster
文件保护字典 + BTAudioHALPlugin）；fuxi = 0 条（连段都没有）；所有
21H461 donor 板继承 OS profile 同 4 条——**没有任何过墙板携带显示族
fixup**。我们的 23 条 = 4 基线 + 19 条自加的 displayless kill 族。②
**过墙板答案模板**（上游 docs/devices/ 新设备目录）：asteroids
（SM7635，桌面第一帧）= **AppleKeyStoreState 组件替身**（"没有替身
SpringBoard 永远跑不完 boot" T20-T29 实证）+ sheng-vsync=120 +
offline-mobile-graphics-client=**0**；oneplus13t（SM8750，**无 GPU 到
锁屏** Build 50）= NativeKeyStoreEvents 组件 + sheng-vsync=60（fixed）+
CPU framebuffer；sheng 生产 = 144Hz vsync + KeyStoreProbe 编进 display
archive；pong（SM8475）= home screen；negroni = SB+touch CPU 渲染。
**四板共性 = AKS 替身 + vsync + 注册开启，无 GPU 也能过墙**。③我们
与模板的差距表（三项全缺）：KeyStoreProbe/AppleKeyStoreState 缺
（`sheng-aks=1` 参数一直在 = 死参数）；`sheng-vsync` 缺；
`offline-mobile-graphics-client=1`（asteroids 显式关）——且 r52 的
"mfb C0 sel6 失败头号嫌疑" **就是自设探针客户端**（patch 的
native_mobile_graphics_client_probe），不是 backboardd，嫌疑权重大降。
④DPU 零替身不阻塞（asteroids 同款 LegacyDisplayStubs 跑到桌面）。

**体系方案拍板（r62 = 机制线环境闭合，非打地鼠）**：r62 四件 =
①`package/KeyStoreProbe.cpp`（sheng 原版入包）②
LegacyDisplayProbe.cpp 接回 `native_keystore_probe()` 调用（
late_probe 首行，门控 offline-key-events=1 + sheng-aks=1 + persistent
Data 全在）③build-native-legacy-display.py 编 AppleKeyStoreState
组件（v2 include+component: 条目；产物名 = legacy 运行名
NativeKeyStoreEvents.o/.late.o）入 display archive ④board.toml
+`sheng-vsync=120`（m2381 面板 120Hz；>60 同时触发
IOMFBDisplayRefresh 发布）+ client 1→0。**23 条 fixups 不动**（安全
网 + 惰性 oracle：环境愈合后显示族站点应转睡，applied 计数可跌破
14 = 体系生效的定量信号）。log-only 断言杀 / 全指定 TC / 缓存全局
审计 = **r63 备选**（若 SEGV 垃圾族仍在）。

**r62 出件（2026-10-28，本机 Linux 全链）**：kernel `542300ca`
（display archive `b6daee99` 七成员含 KeyStoreProbe.o +
NativeKeyStoreEvents.late.o，build-report probe_fleet=
m2381-trimmed+keystore）/ 探针后 `158adedb` / trust `7416ca4f` /
seed `d04a0153`；ADT `b5b4a172`（23 条不变；sha 变 = random-seed 每次
生成即换 + 无正文异常，r58 20 条件二进制对比定谳）；**payload
r62 = `0854ea5e`**（96MiB，平铺 `out/m2381/payload-r62-0854ea5e.img`）
——ESP 四样全过（scratch-data.img 在 / toml initrd 行在且 args 含
client=0+vsync=120 / trustcache 在 / ADT PackedRAMDisk 在）。
**2026-10-28 已刷**（fastboot 实录：set_active b / sha 门 /
Sending+Writing recovery_a 98304KB OKAY / reboot）；**读回对拍 ✓ +
判读已毕（2026-10-28，见下）**。

**r63 判读定谳（2026-10-28）**：读回对拍 ✓（sde27 ==
`04eb0533…d5691`）；窗 rounds=312/509 uptime=640s 无回卷；
**fixup16a/16b 真执行**（applied 67→69，=+2 新站点各一）；AKS 爆发
（opens 123 / calls 1121，r62 的 18/49 → 23 倍，t=22s 起）；vsync
120 ✓；C0=IOMFB_FDR_Loader 换形（12 calls，失败选择子族
e00002bc/c7/f0）；SWAP 泵大（calls=8222 / COPY 8221 / 10.3MB
retired）；procs=194/spawn=968；**换墙 = SB sig11 ×554 同族未愈，
死点漂回 r61 有效码点**（unslid PC `0x1800F5FAC` objc initialize 快
路径 x0=0xf1；bt1 去滑 = 0x18A00DC50 = fixup14 getter 站点）——链
路钉死 = **bgTC 缓存槽 0x1f48ce5e0 被写进 0xf1 垃圾**，病根在缓存全
局槽的写路径（构造/once 域），在 TC 内容下游之前——全指定 TC 到位
了但毒在更上游。**r64 = 缓存全局审计转正（线 2）**：①静态——枚举
UIKit 单例槽（bgTC 0x1f48ce5e0 / mainScreen 槽 / 0xf00/0xf1 同族槽）
文件值 + rebase/chained-fixup 目标定性；②动态——内核探针行
（page snapshot ext slot 加 bgTC/mainScreen 槽值 dump，t 序列看
0xf1 何时被谁写入；内核步本机 Linux 可出）；③0xf1/0xf00 特征 = 小
整数带 F 高半字节 = "指针减错基"残留嫌疑（每次 boot slide 不同
d20000/1a20c000/1c084000，跨 boot 残留一致的 0xf00/0xf1 需要解释）。

**r62 判读定谳（2026-10-28）**：读回对拍闭环——recovery_a =
**sde27**（FLASHING §6 实测表；先误探 sda10/11 为错节点，非异常），
前 96MiB sha == `0854ea5e…db1a` ✓。窗 rounds=291/509 uptime=605s
无回卷。**体系三件实锤生效**：①AKS 替身活了（opens 3→18 / calls
6→49，selector 表 0:18 / 7:15 / 14:1 / 17:15）；②vsync 生效（hz=120
start=1 refresh=1 + IOMFBDisplayRefresh dict 在）；③client=0 生效
（探针客户端消失；剩余 C0 = **IOMFB_FDR_Loader pid=75** 真实消费者，
sel6 e00002d5 失败簇仍在——r52"C0=自设探针"的修正再修正：**C0 本来
就是 IOMFB**）。**环境未愈合**：applied=67（23 parsed，与 r61 的 70
同量级，未跌破 14）；旧墙链全灭（nil traitCollection / unspecified /
XB / displayIdentity 全零 + sig6=0，r57–r60 五墙确认封死）；**换墙
仍是 SEGV 垃圾族且升级**：SpringBoard sig11 ×520（t=66s→605s 整程
respawn 循环），unslid PC `0x1822F5FAC` = **cstring 数据区**（野跳
进垃圾"代码"，字符串字节被当指令执行后 FAR=0xf1 读数中止；fp 链全
废，bt 去滑全未映射，最后有效帧不可恢复）——0xf00/0xf1 族从"有效码
+ 垃圾操作数"（r58/r61，r61 点已复验 `0x1800F5FAC` ldr x8,[x0] 有效）
升级为"垃圾控制流"。procs=200/spawn=1003；SWAP 有真活动（accepted=1
COPY 10.3MB retired=4）但终态 DISPLAY stage=0；SB 未过 wallpaper/封
面层。**r63 分支（§7 预案三分支第一支触发）**：UIKit 侧候选 = 全指
定 TC 构造 / UIKit 缓存全局审计；NSAssertionHandler log-only 类杀降
级（断言墙已全灭，无对象可杀）。

**r61 判读存档（2026-10-27）**：窗 rounds=374/509 uptime=772s 无回
卷；applied=70（23 parsed；计数首超 parsed = fixup 页被反复重校验）；
'unspecified values' 断言零出现（块 A/块 C 死透）；换墙 = SIGSEGV
×684（PC `180e15fac` / FAR=0xf1）：`PBUIPosterWallpaperViewController
init` → `__UIGetCurrentFallbackTraitCollection+0xac` →
`_backgroundThreadFallbackTraitCollection+0x10` retain 快路径 x0=0xf1
垃圾（与 r58 0xf00 同族）。结构性定谳 = UIKit 缓存单例全局在无显示
环境成批变垃圾源；用户拍板停逐发 fixup 转体系路线（本轮即落地）。

**r56 = 废轮（教训已入法）**：出件漏 `--data-ramdisk`/`--trust-cache`
→ ESP 无 Data seed/initrd 行/trustcache → Data mount ENOENT panic，
applied=0。出件配方红线见 §7。r57 = 规范重建（seed `d04a0153` / trust
`7416ca4f` == r55e 件；toml initrd 行回归），已刷且**读回对拍已补**
（reader 内 dd recovery_a 前 96MiB sha == `1e7ea60b…` 镜像，2026-10-27）。

**forensic reader 调试五连**（本会话，9c53eee→cbdd5f0→b692151→r55d→r55e）：

①fb2c5dd1 LFN 破坏（短名覆盖 toml）→ 机制正道 payload.py 插桩
（[payload] forensic=true）；②秒退根因一 = entry 无 dtb → 补
`dtb = "Payloads/forensic.dtb"`（nura sm8550-meizu-20.dtb）；③秒退根因二
= ENABLE_SECUREBOOT=1（sheng 拍来）LoadImage 拒签 → 1→0，**boot_b
换件 = 289614ba**（替代 a1d51f04）；④r55d（087f9ca3 保命 cmdline）
实测定谳 = **内核每必起**（8 核企鹅/UFS 全量枚举，keep_bootcon 留屏
成立），死因实拍 = `Initramfs unpacking failed: damaged header` →
无 /init → `VFS: Unable to mount root fs on unknown-block(0,0)` → USB
永不出现；根因 = build-forensic-reader.py 手写 cpio **12 字段错位**
（newc 契约 13 字段，fuxi 母版即 13）——LoadFile2 投递链实拍证明是通
的；⑤**r55e（b060b512，只换 initrd = 402fb431）已刷 + 读回对拍 + 通道
首验全绿**（lsusb 1d6b:0104 / magic=SHLOG001 / 32MiB 窗在位；reader 内
dd recovery_a 读回 == 镜像 sha）。下会话 = §7.3 iOS 快速迭代。

## 2. 设备现状（2026-10-27 r54 后 = **iOS 链现役**，未刷回 nura）

| 分区 | 件 | sha 前 8 | 状态 |
|---|---|---|---|
| boot_b | 本仓 UEFI（**无 SB 版**） | `289614ba` | **现役**（r55c 配套，ENABLE_SECUREBOOT=0） |
| recovery_a | **r77**（fixup #30 = `_WGMainScreenScale` 供给 2.0，30 条 fixups + 扫描器三改探针 + noidle=1 + dirty 窗 R 码 + AKS 替身 + vsync + client=0 + Data seed + trustcache + forensic reader；内核仍 `ce34e9ae` 代） | `93854822` | **出件待刷（2026-10-29 凌晨）**；`out/m2381/payload-r77-93854822.img`（96MiB，sha256 全 = `93854822e007320bed07fd6530f8091bb968e244e48db2b64cbb0edb24f2d35b`）；ESP 四样已验（ADT PackedRAMDisk + 30 条 fixup 对拍零缺失 + scratch-data + initrd 行 + trustcache）；r76 = `2bdd272a` 已刷已判（判读 = §1） |
| userdata | CI 重建 iOS root | `fd43ee70` | **现役**（未被动） |

- **设备现态 = iOS 链（r54 起），未刷回 nura**；r62 已刷已验已判
  （2026-10-28：set_active b → flash recovery_a OKAY → reboot；reader
  内 sde27 读回对拍 ✓；判读见 §1）。
- **reader 三连流程实录（本轮全过，方法固化）**：进 fastboot（电源+音
  量减）→ `fastboot reboot` → 菜单选 "m2381 forensic log reader" →
  `acm-reader.py probe` → `cmd 'dd if=/dev/sde27 bs=1M count=96 |
  sha256sum'` → `fetchchunk /tmp/r62-window/log.bin`（sha 对拍）→
  `read-m2381-ufs-log.py --all` 判读。**坑两枚**：recovery_a 节点 =
  **sde27**（FLASHING §6），勿按 sda 大 LUN 猜；ACM 通道推大文本会
  丢字节（strings/tr 输出花掉），一切比对走设备端 sha。
- **ABL 重试预算行为（r55d 实测）**：同槽连败数次后预算耗尽自动落
  fastboot（用户实测一次）。恢复 = `fastboot --set-active=b`（留在 b、
  重置预算，nura FLASHING §3）再刷；自动重置 = r56 BootSlotReset DXE
  主题。
- 读窗从此走 **forensic reader**（菜单选 "m2381 forensic log reader"，
  FLASHING §7）——**iOS 链现役时连 nura-min 回退都不用刷**，reader 在
  payload 里；nura-min 只在 reader 本身起不来时才需要（届时三件全刷
  规矩恢复生效）。
- r54 裸件（1ef5eb63）已被 r55 覆盖用途；r53 件（04f4d803）已定谳
  退役。回滚件只在 nura 仓 `/home/ri/work/nura-meizu-m2381/artifacts/`
  （红线 6）。
- **发卡规矩（r49 事故定谳）**：①发卡前先确认设备现态（NCM 接口/
  devsh 可达 = nura → 三件全刷；iOS 屏幕形态 = 只刷 recovery_a）；
  ②payload 只认平铺路径 `out/m2381/payload-rNN-<sha8>.img`；③卡上
  必带刷前 `sha256sum` 门。

## 3. 现役件与资产

- **payload 四元（r64 起）**：kernel 重链 `e86478ad`（r63 系 =
  `542300ca`/探针后 `158adedb`）/ trust `7416ca4f` / 种子 `d04a0153`；
  ADT fixup 表 = **25 条**（24 dyld + BTAudioHALPlugin 基线；
  16a/16b 字节级在位）。fixup 表源 = 设备仓 OS profile
  `text_fixups`。**r64 新增 = display archive 携带
  UIKitCacheProbe.o**（八成员；startup.o record 编译私有
  task/vm_map ABI），board.toml +`uikit-cache-probe=1`
  -`boot-ui-focus3`（cmdline 1000B 上限）。
- **applied 常驻集（=n 时代）**：{BTAudio, 5, 6, 7, 8a}。=y 上电后该
  oracle 继续有效（惰性判活器不变）；显示族 fixup（5–10）在真显示栈下
  变睡守，换代时退役。
- **device repo**：`devices/m2381/unixnu-m2381`（schema v2；os_profile =
  ipados-17.7.11-21H461；orphan 分支 `device-m2381` 供 CI）；**UEFI**：
  `devices/m2381/uefi/m2381Pkg`（shengPkg 骨架 + mars blob @036ba9f7）。
- **donor**：iPad7,1 / iPadOS 17.7.11 `21H461`（IPSW 本地已核验；20H392
  资产保留回溯）。unique-chip-id `3645ad55d47e77c1`。
- **=y 车道（r51 施工完成）**：组件源码在主树 `drivers/GopFramebuffer`
  （组件 include `GopFramebuffer`，pin cf3b429）；设备仓
  `scripts/build-native-legacy-display.py`（audit → ABI 头漂移门 →
  358 vtable → 字节门 → bindings → probe/GOPPresenter → late.a，probe
  车队已裁剪）+ `package/LegacyDisplayProbe.cpp`（fork，五外探针拔除）+
  `package/LegacyDisplayDPUSubstitute.cpp`（weak dpu 钩子零替身）+
  `scripts/mobile-graphics-legacy-abi.json`（runtime 钉的 audit 产物，
  storage 静态 pin 先例）；telemetry 按
  `XNU_NATIVE_LEGACY_DISPLAY_QUALIFIED` 条件让位；defconfig 双文件 =y。
  换代成本 = `docs/PLAN-display-lane.md` §2（全脚本化重生成）。
- **ADT 变更预告（上游 `688fa88`）**：下轮 payload 的 ADT 会因 21H461
  profile 新增 product-id/J120AP compatible//product/audio 而变化——
  属预期正文变更，payload sha 变化不是事故。
- **诊断工具链**：符号化工具集已收编入设备仓
  `devices/m2381/unixnu-m2381/scripts/re-tools/`（见 §4.2）；
  `temp/` 只留 cache-extract（4.4G 解码缓存）+ 近两轮日志窗
  （r52/r53）+ 当轮解析产物。2026-10-27 清理 = 旧轮 payload
  r49–r55d、老 logwindow r42–r50、r49/r50-verify、r51 构建日志、
  B2 期 tools/（adt/mkadt/remotezip）全数删除。

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

- **主墙定性（r61→r62 体系路线）**：SIGSEGV ×684 = `ldr x8,[x0]`
  x0=0xf1 垃圾（retain 快路径，bgTC 内部）——与 r58 的 0xf00 同族：
  UIKit 缓存单例全局在无显示环境成批变垃圾源。已封死墙链 =
  displayIdentity（r57）→ XB 断言（r58）→ UIScreen SIGSEGV（r59）→
  traitCollection nil 块 B（r60）→ unspecified 块 A/块 C（r61）。
  **r62 押注 = 环境闭合让真显示配置/UIScreen 供给链复活**（AKS 替身
  + vsync + client=0，四过墙板模板）；若 SEGV 垃圾族仍在，r63 备选 =
  全指定 TC 构造 / UIKit 缓存全局审计 / NSAssertionHandler log-only
  类杀（勘址已毕 0x1874494d4/0x187449618，砍 raise 留 REASON 日志）。
- **机制线（track 2 真显示注册）**：r52 的 "[mfb clients] C0 sel6
  e00002d5" 头号嫌疑 **权重大降**——C0 = 自设
  offline-mobile-graphics-client 探针客户端（r62 已关），非
  backboardd。剩余机制嫌疑 = vsync/IOMFBDisplayRefresh 缺发（r62 补）
  与 AKS 缺失（r62 补）；若 r62 判读后 mainConfiguration 仍 nil，再
  入 IOMFB 选择子实现面与 dpu 钩子绑定。
- **chronod sig5 崩溃循环**（r57 实测 ×287）——次优先，主链推进后按
  sheng disable-list 处理。
- **独立主题（不阻塞）**：XS/fileset-KC 重绑定器（fileset KC 无 KASLR 表，
  prelink kit 需重绑定器）；arm64e 专项（透传已验证，FPAC/PACM/自举面/
  再签待专项）；GPU（ADRENO=n，P4 显示门过后主题化）。

## 6. 不能声称的

- 显示墙未破：SpringBoard 未到 home screen（r61 = 已深入 wallpaper UI
  构造 + trait 机器 SIGSEGV 循环）；keybag 未考（SB 未到锁屏）；触摸/
  按键/GPU 未上主题。
- =y 车道**一级已实机实证**（PE handoff + 控制台接管 + 推帧自稳 + 真
  显示配置到达 IOMFB 客户端层），且 r57 实证**用户态可画 boot progress
  UI**（菊花实拍）——但 main display 注册链仍断（isMainDisplay=0/
  mainIdentity=nil 族），"用户态显示配置供给链已通"不能声称。
- ~~类杀 NSAssertionHandler 未出件~~ **降为 r63 备选**：r62 拍板 =
  机制线环境闭合优先（AKS 替身 + vsync + client=0，四过墙板模板），
  断言类杀只备于 SEGV 垃圾族持续的情形；双方法勘址保留
  （0x1874494d4 / 0x187449618，砍 raise 留 REASON 日志形状已定）。
- ~~r62 出件~~ **已出件已刷**（`0854ea5e`，§1/§2/§7）；AKS 替身实机
  效果 / vsync 生效 / 显示族 fixup 转睡三项都未实证（用户报未起，
  窗待读），不能声称"体系方案生效"。

## 7. 下一步（**r78 = =y 攻坚开修（2026-10-29 用户拍板"直接开修"）**：
**第一刀已钉死 = IOMFB 用户客户端 sel6 失败族（e00002d5）**）

**为什么是它（案卷在案，不是侦察）**：r52 首录 `[mfb clients] C0
sel6 e00002d5`；r62 修正确认 C0 = **IOMFB_FDR_Loader pid=75 真实
消费者**（非自设探针）；探针有参数记录（`sel6 args(0,80000003)
→ 0 fail` / `args(1,1) → 0 fail` / `args(1,80000000) → 1 call 1
fail e00002d5`）= **选择子分发表里 sel6 带 0x80000000 参数的调用
被拒**。vsync（r62 已发 120Hz start+refresh）与 AKS（r62 已活）
都到位而 mainConfiguration 仍 nil —— sel6 持续失败就是 main
display 注册链上**有记录的具体断环**。修好它 = IOMFB 客户端层
FDR 路径通 → mainConfiguration/isMainDisplay 预期接通。

**开修顺序（下会话，直接动手）**：
1. **定点 sel6 语义**（半小时级）：窗内 grep `sel6` 全部行拿全参
   数×返回码矩阵；sheng 源码对照——sheng 的 IOMFB shim 用户客户
   端选择子表（`drivers/` 内 IOMFB/NativeMetal/DisplayRefresh 相
   关 + `docs/` gpu.md）逐选择子对齐，找 sel6 对应函数与
   e00002d5 的返回条件；e00002d5 语义反查（IOKit 返回码表）。
2. **补实现**：按 sheng 实现搬齐 sel6 分支（参数级直修或组件级
   补齐；改内核须重编——Linux 出核链现成，改 defconfig 必重跑
   defconfig 命令）。
3. **出件 r78 上机**：判读 = ①`[mfb clients]` sel6 失败清零/
   C0 状态改善 ②mainConfiguration 非 nil / isMainDisplay=1 ③屏
   面进度条是否走动（供给族下游预期连锁反应）。

**里程碑（可观测）**：`isMainDisplay=1` / `mainConfiguration 非
nil`（IOMFB 客户端层）→ 供给族墙（scale/geometry/displayConfig
+ once 域毒化）预期成片消失 → SB 推进 wallpaper/图标层。

**止损线**（memory kernel-donor-switch-decision-rule）：=y 攻坚
2–3 会话；卡死且供给族又积 5+ 堵 → 评估中间路线 B（保内核换
iPhone donor 21H16）。**sel6 这刀打完若注册链仍断**，下一刀按
同法从 `[mfb clients]`/IOMFB 失败族的下一记录点开（数据现成）。

**支路纪律**：fixup 降支路——判读撞供给族断言（十分钟可定点）
顺手修；once 域毒化类（果）不追。

**出件配方红线不变**：payload 三 flag + --unique-chip-id
3645ad55d47e77c1（勿带 --skip-setup）；探针正源 = devices/
.../package/*.c（tree kits/sheng = symlink 同一实体），root
package 镜像同步；判读纪律 = header 分代 + re-tools read_run +
recovery_a=sde27 + fetchchunk + 修正掩码 0xFF800000 + 零引用先过
阳性对照（§4.9）。

**r78 第一刀定谳（2026-10-29 会话，①定点 sel6 语义完成 + ②修法已进件）**：

**三层闭环**：①`e00002d5` = **kIOReturnBusy**（`iokit_common_err(0x2d5)`
Device Busy；xnu IOReturn.h:168 + 仓内 ShengUfs RootCommandsBridge
双源钉死，"NotPermitted" 假设作废；头文件副本存
`temp/IOReturn-xnu-main.h`）。②窗矩阵（子代理 A，r52/r53/r55e/r69 四
窗）：8 种去重参数组合跨窗稳定——`(N,0x80000001)→0`、
`(N,0x80000000)→e00002d5`（N=arg0 递增请求号，[5820]/[5824] 恒镜像
arg0）、`(0,0x80000003)→0`、`(1,1)/(0,2)→0`；r69 变质 = pid=86 客户端
sel6 风暴 25236 calls/6297 fails（~26 calls/s 忙等）至窗末不收敛。
regs 四窗恒 0。③**Apple 二进制 RE（donor KC
IOMobileGraphicsFamily，carve+capstone）**：prelink 段全局映射
`file = vm − 0x482c000`（kext LC fileoff 是 slice 相对，第一次 carve
全错已修正）；user client 分派表 @`0x6f354c0`（IOExternalMethodDispatch
24B/项，sel6 = 3 scalars）→ trampoline `0x6779518` → ext_wait
`0x6777390`（写 client+0xf8=arg0、清 +0x100）→ **display 虚槽 176 =
`0x678d998`**（vtable `0x6f38570`；槽 249=getDisplayDimensions 与在案
ABI 交叉验证一致；实例大小 0x18a0=6304 与探针 +5864 状态字吻合）。

**v176 wait 分派语义（0x678d998 全函数破译）**：默认返回
kIOReturnBusy；`flags.bit31==0` → 转发 display+0x1608 gate 对象
vslot29（窗里 (1,1)/(0,2) 走此路 → 0）；bit31==1 且
request(flags&0x7fffffff)∈{3,4} → inline 检查 `+5856==0 &&
+5840(powered)!=0` → 0（**m2381 已通：announcePoweredByFirmware
10-21 进件后 +5840=1 在窗可证，旧"powered 无人写"叙事证伪**）；
bit31==1 且 request∈{0,1,2} → **同走 +0x1608 gate vslot29，其 wait
形态永不完成 = Busy 全部来源**。gate 完成信号属 platform 显示管线
（本 port 未实现），GOP 路径不消费其 payload。附带破译：
default_fb_surface ext（e00002bc 族）= `display->v174` 返 0 → 打
"No Default FB Surface found" → kIOReturnError（**下一刀素材**；
e00002c7=Unsupported、e00002f0=sel28 另族）。

**修法（=sheng announce 同款 stand-in 哲学，GopFramebuffer
submodule commit 477fc19）**：`nativeLegacyInstallWaitStandin` 在
startDisplay（announce 同址同条件）copy display vtable 512 槽、slot
176 → `nativeLegacyWaitStandin`：bit31 且 request∉{3,4} → 计数
`native_legacy_wait_standin_calls` +1 并报 kIOReturnSuccess，其余
（bit31==0 / request 3/4）转发原实现不变。探针
`[mfb clients]` 块首行新增 `wait-standin N`（devices 正源 + root
package 镜像已同步）。

**r78 出件与判读预注册（§7.2）**：本机 Linux 出件中（内核重链
+ payload，改组件未动 defconfig）。判读分支：①**命中** = sel6
`e00002d5` 行清零/不再增长 + `wait-standin` 计数 >0 增长 + C0/C1
fails 停涨 → 看 mainConfiguration/isMainDisplay 侧推进与屏面进度条
（供给族下游预期连锁）；②**部分** = sel6 清零但新墙（预期 =
default_fb_surface e00002bc 族或 FDR 后续）→ 下一刀按 v174 供给面
RE（素材已在案）；③**未命中** = Busy 仍在 → standin 未触发
（primary/bootMapping 条件或 patch 时序），回读
wait-standin=0 判接线。刷写纪律照旧（设备现态三件套核对 +
recovery_a=sde27 读回对拍）。

**r78a 判读（2026-10-29 会话二）**：已刷已读（窗 267/509 t=537s 无回卷）。
**分支③未命中，根因在构建链不在设计**：`update_submodules()`
（scripts/unixnu/feeds.py:217）在 kernel 步起步跑 `git submodule
update`，把 GopFramebuffer checkout 回主仓 gitlink（33e941a），同会话
submodule commit 477fc19 被甩（reflog 实证：commit 02:51:59 →
checkout 02:52:08）→ r78 内核 = **新探针（wait-standin 行在）+ 旧组件
（无 standin）**。窗证据：wait-standin 全程 0；sel6 Busy 族原样
（新增 (4,0x80000001)→0，s6# 环 N→a）；applied 惰性涨 0→25（r77=26
同量级，引擎健康）；**C0 风暴消失（r69 25236 calls→本轮 77）**记档不
下结论；屏面 logo 1:04 / 进度条 1:25 冻结（早于 r76 1:54 与 r77
1:30）。**修正**：submodule checkout 477fc19 + 主仓 gitlink bump 同
commit（新规矩：改组件必须 submodule commit + 主仓 `git add <模块路
径>` 同轮完成，否则构建静默回退）。r78b 重建中（内核重链 + payload
三 flag）。

**r78b 判读（2026-10-29 会话二续）**：已刷已读（窗 337/509 t=678s，sha
4d9cd833 ✓）。**分支③再未命中（wait-standin 全程 0、Busy 族原样），
但本轮把静态疑点全部排除**：①构建链（90fef580=e4c01b0 组件，standin
符号与 installer 序列都在内核）；②vtable 槽位——用 LEAF 占位锚点解出
编译后 kext vtable（opaque211/245/246/306/307/338/345 + kickSwapQueue
落槽 290，槽差逐一精确），**slot176 = 0x578d998 = donor Apple wait 尾
模式吻合 = 零漂移**；③installer 全序列在 start() 内联体（IOMalloc
0x1000+memcpy+`[copy+0x580]`=orig+`adr` 单指令写 standin（**adr ±1MB
单指令，adrp 扫描假阴性——工具教训入档**）+vptr store）；④primary=
true 实证（vsync hz=120 过 startVsync 的 `!primary||!bootMapping`
门）；applied=26 同 r77 量级。**嫌疑收敛为单点：ext_wait 的 owner
（`[uc+0xd8]`）可能不是被 patch 的 display 实例**。下轮探针 =
wrappedExternal 的 sel6 分支 dump target/`[uc+0xd8]` vs
`native_legacy_object`（及该 vptr 是否=copy 表）。屏面 logo 1:04/进度
条 1:20 冻结。判读分支 ③ 的第三种形态：**修法代码全对而 wait 调用
不经过被 patch 的 vptr**。

## 8. 下一步（**r79 = iOS 16.7.16 donor 回退（2026-10-29 用户拍板"不搞了，换ios16的版本"）**）

**拍板**：=y 车道停（sel6 刀 r78a/r78b 两轮未落地，触止损评估点，用户直接拍换道）；donor 回退 **iPhone10,3 / iOS 16.7.16 / 20H392**（=10-24 换配前的原配对，资产全部保留作回溯，当时"留作回溯"现在启用）。**A11/t8015 = 经典 arm64 kernelcache（非 fileset）→ 零重绑定器成本，纯配置级换道**。

**为什么成立（对 10-24 拍板的反转理由）**：r35–r77 的整条墙链（traitCollection/avocado widget/once 域/sel6 FDR gate）全部是 **21H461 用户态+kext 的 17 代墙**；换 20H392 = IOMFB kext 与 SpringBoard/UIKit/FDR_Loader 全部回到 16 代内部自洽配对，17 代墙链整体蒸发。跨代风险（xnu-10063.141.1 内核源码 vs 16 代件）回到 10-24 前的状态——**但那套墙当时已被驯服**（entropy 守卫 373f769 仍在树内、绿蓝条+panic screen 全通；20H392 prelink 三值已 derive）。

**盘点（本会话查证）**：
- **IPSW 已到位**：用户本地件 `/home/ri/Downloads/iPhone10,3,iPhone10,6_16.7.16_20H392_Restore.ipsw`（6,171,035,648 B），sha256 = `ece1194f7e233e9109c5790a8d23a4564a10a254e15ec5b09978a343265568fb` **与旧档记录的全 zip sha 一字不差**，已硬链接入 `research/ipsw/20H392/`（步骤 1 完成）。CDN URL 也验活（HTTP 200 同长度）。备用件：21H16 IPSW 用户 Downloads 在（7,805,610,705 B，sha 验算中）——fileset-KC 重绑定器路线的备件。
- 旧 pin（git `d42a993~1:.../board.toml`）：product iPhone10,3 / build 20H392 / device_class **d22ap** / chip_id 0x8015 / board_id 6；OS dmg sha：`087-86698-044.dmg = 80f426d5…c92483`。
- 20H392 kernelcache sha256 = `2ec297bac95bcba01046fd0148e97e7e5885c05a6c18827f39f534ba375c1221`（经典 arm64）。
- **prelink 三值已 derive 在案**：`docs/archive/STATE-history.md:635` = slide `-0x1000000` + AMFI `0xfffffff008f3bd18`（三重指纹法）；text_fixups 是第三个值（见 :582，需按 16 代缓存重建或从旧档取）。
- 20H392 trust cache 打包 commit 65b8eb8 在史（flat container bundle）。

**施工单（下会话，按序；设备 8% 电量 → 用户拍板"先把充电修好，上游已经有了"）**：
0. **充电先行（用户拍板）——2026-10-29 会话已完成定位与半接线，下会话继续施工**：
   - **上游查证结论（修正 r79 预期）**：上游 main（已 rebase 同步 `f85b5a6→33da813`，仅 6 个 docs 提交）**没有**充电组件；充电 = **sheng 设备仓新近 11 提交的 ADSP 电池栈**（`QcomGlink/QcomQrtr/ShengQrtr/QcomSmem/QcomSmp2p/QcomQmp/QcomRpmh/ShengBattmgr/QcomMdt` + `AdspProbe/BatteryProbe/ShengBattery`，sheng `d76556b→08cde20`；机制 = ADSP 固件跑充电与 PD/PPS 协商，BATTMGR 走 PMIC GLINK，`sheng-adsp=2` 起机、`sheng-battery=1` 发布 IOPMPowerSource）。sheng = SM8550 同 SoC，SoC 常量（SMEM 0x81d00000/AOSS 0xc300000/RSC 0x17a20000/cmd-db 0x81c60000）1:1 适用。
   - **本会话已落**：25 个充电栈源文件已拷入 `devices/m2381/unixnu-m2381/package/`（`AdspProbe.cpp`/`AdspFirmware.c` 已 fork 嵌入式源）+ `scripts/make-adsp-firmware.py`（squash：mdt+bNN→单 .mbn，含 sheng 版跳过的哈希段，压实重写 p_offset）+ squash 产物已出（adsp.mbn 30,499,631 B + adsp_dtb.mbn 12,420 B，源头 = `/home/ri/work/meizu20/meizu20_linux/vendor-fw-harvest/modem/image/`，m2381 自有固件，`.cache/firmware/adsp-src` 暂存不入 git）。**未接线**（构建零影响，check-m2381 4 ok）：build-native-legacy-display.py 编译循环/archive 成员、LegacyDisplayProbe.cpp 调用点（sheng 487–500 同款）、board args、boot-progress 构建的 AdspFirmwareBlob。
   - **固件投递定案（ESP 算术否决嵌入式路线）**：r77 ESP free 仅 12.9 MiB、kernel.macho 已 66.7 MiB（recovery_a 100 MiB 硬顶）→ **30.5 MB 编进内核装不下**。正道 = **固件放 boot_b 空闲区（~80 MB free，红线允许碰）+ vendored ShengUfs 加 `native_ufs_part_find/read` 导出**（sheng ShengUfs `0e29c46` 同款 API，内核原位读；package-m2381-boot.py 在 boot.img 后追加固件，ABL 对 image_size 之外尾字节的容忍需首刷实证，可回退）。下会话施工序：①ShengUfs 加导出（对照 sheng NativeUfs.cpp:1207/1247）②package-m2381-boot.py 追加段 ③AdspProbe fork 加 boot_b 源 ④build 脚本接线 + 调用点 + args（cmdline 1000B 预算：删死参数 `sheng-cpufreq`/`sheng-touch` 换 `sheng-adsp=2 sheng-battery=1 armcpu-spi-ipi=1` + `sheng-disable-jobs` 加 coreduetd）⑤sheng 仓 host 测试（battmgr/mdt）本地编译验证。
   - **其余可抄面盘点（用户拍板"一起拿了"）**：已拍定抄 = AMP 集群（sheng `76780d7` zzyf-sheng-amp.patch + dts cluster-type 注记，m2381 patches/xnu/series 现为空文件直接加）+ coreduetd 禁用；**缓办**（支路纪律）= CpuFreq/GpuPower probe 重挂（裁剪车队遗留，GPU/touch 车道都停）、BootLog 新行（display/governor/VM/fence，需 reader 同步）、ShengUfs stage-1 SMMU 窗口（服务 =y/WiFi 停运车道）、WiFi（sheng 刚通，WCN IC 未核对）。ShengUfs 导出移植时顺带对拍。
   - **用户另提（下会话单列）**：forensic reader 的 Linux 内核（forensic-kernel.efi 13.2 MiB）带显卡驱动可精简（forensic 只需 UFS+USB ACM+gzip+sha256）——既减 ESP 又为固件腾位；与 boot_b 路线互补非替代。
1. ~~下载~~ **已完成**（sha 对拍 ✓）；充电通了再开工：IPSW 解包（KernelCache/DeviceTree/StaticTrustCache/Cryptex1,SystemTrustCache 四件）并逐成员对拍 OS dmg sha（`087-86698-044.dmg = 80f426d5…c92483`）。
2. 新建 `profiles/os/ios-16.7.16-20H392/`（仿 21H461 profile.toml 结构：os 元数据 darwin 22.x/xnu 无关项、[ipsw] 旧 pin、[kernel.prelink] 三值、trust-cache pins、ADT 身份 d22ap/iPhone10,3/t8015）。
3. `board.toml` → `os_profile = "ios-16.7.16-20H392"`。
4. **text fixups 清零起步**：23 条自加 displayless-kill 族全部是 21H461 VA，新 donor 全部作废；16 代用户态裸起，遇墙再按 §4.9 定点法逐个补（"遇一堵修一堵"支路纪律平移）。
5. 内核重链（prelink 对 20H392 KC）→ 出件；判读门 = 20H392 时代的既有证据形态（entropy 守卫日志/绿蓝条/panic screen）。
6. userdata 重建（16.7.16 root，Linux 可构建）。
7. **设备现态**：手机已回 nura 充电——下会话首刷 = **三件全刷**（boot_b 不动件 + recovery_a 新 payload + userdata 新件）。

**红线/纪律全数平移**：刷写四样、读回对拍、判读修正掩码+阳性对照、止损线（2-3 会话，卡死则再评）。
