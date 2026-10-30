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
> 改指新仓（见 `docs/CI.md`）。**同日续**：m2381Pkg 收编新 UEFI 仓
> `../uefi-m2381`（= 上游 uefi-sheng `sheng@0eb4514` 完整克隆 + overlay
> 提交 `5297334`；实证来源 = nura 链；Binaries pin `036ba9f7` mars；旧
> Mu-Silicium fork 封存；sync.sh 与 devices/m2381/uefi 退役）；三仓收拢于
> 工作区根 `/home/ri/work/unixnu-meizu`（老主树路径留 shim 符号链接）；固
> 件构建 = `--uefi ../uefi-m2381`。

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

**r83 = 20H392 供给链闭环 + 全链出件（2026-10-29 会话四，设备仓 5a6d356；首刷三件套就绪）**：

- **①verified-root 横雷定谳 + 政策对重建**：`.cache/firmware/verified-root-{os,systemos}` 是 21H461 时代政策（volume 04769D26/8846FE78，os 窗 6,077,580,800B）——`NativeUfsBlock::configureVerification()` 按**容量匹配**选政策，20H392 窗口（5,635,081,728B/2,743,074,816B）无匹配 → 设备永不发布 → root start failed。stub 车道（count=0）同样死。正道 = **重建政策对**：21H461 旧件备份 `verified-root-21h461-backup-*`，`build-owner-policies-local.py` WINDOWS 表换 20H392 四元组 → **os 343,939 块 + systemos 167,424 块全 RSA PASS**；`build-verified-root-probe.py` guard 换新值（双 sha/块数/采样源件名 087-86593-047.dmg；os 尾块 1536 与 systemos 整块结构同构故采样逻辑零改）；dylib 从 `build/build/` 补位 `build/verified-root-host/`。
- **②Data UUID derive（上游只有 macOS diskutil 人工路径，Linux 扫描器自研补缺 = `temp/scan-apfs-volume-uuids.py`）**：方法 = APSB 卷超块扫描（4K 对齐**相对容器基址**、UUID@对象+0xf0、RFC 磁盘序），**金标 = 21H461 userdata 双窗**（os 17/17 副本 = 04769D26 ✓、SystemOS 3/3 = 8846FE78 ✓）。坑三枚：①OS dmg 容器基址在流内 0x4400（GPT 前缀）非 4K 对齐裸盘——对齐过滤必须 base-relative，第一轮用裸盘对齐把真超块全滤掉收进一堆 b-tree 假 hit；②UUID 读取偏移 0xf0 曾写成 magic+0xc0（=0xe0）差 16 字节连翻两轮（needle 全序搜索才定位）；③SystemOS 窗口 offset 手算 0x150100000 实为 0x150000000（错偏 1MiB 制造"头全零/NXSB 在 2.5GB"假象）——第三次付费"一切地址算术必须 python"。**终值：root_match_volume_uuid = `DADD93F8-DBBF-4B17-A1DF-0F11E64172F7`（20H392 OS 卷唯一卷，board.toml 已换）+ SystemOS cache 卷 = `96096B28-B6DC-4BC7-9D3A-2632A0FE53F1`（m2381_defconfig 新增 `CONFIG_XNU_OWNER_CACHE_VOLUME_UUID` 覆盖主树 patch 的 8846FE78 fallback；owner_defconfig 同步；18AB5557 seed 注释纠正 = 自家卷 donor 无关）**。CI 线三件（ci-owner-policy.sh/build-owner-systemos-policy.py/主树 build-owner-os-policy.py）同步换 20H392 身份——**CI 实证项（首绿前）**，本地不验证。
- **③④内核构建（新树全量）**：defconfig 重跑 ✓ → tree ✓（GopFramebuffer HEAD=e4c01b0 与 gitlink 一致，r78a 坑未复发）→ kernel。坑：①prepare-xnu-source GitHub API 403 = **GH_TOKEN 缺失**，解法 = 21H461 旧树 `.cache/sources/*.json` 四件直接 cp（两 profile 的 driverkit/trustcache tag 完全一致 10063.121.3/8796.101.5，缓存可跨 profile 复用）；②archive-20 display audit 需要 prelink 候选（两遍构建语义）= 从 21H461 旧树 `tree/build/xnu-prelink/native-owner-mobile-graphics-candidate.{macho,json}` cp 进新树同位（audit 基准 = 21H461 ABI json 自洽；16 代 IOMFB ABI 重审 = 后续主题）；③archive-12 两政策 guard 的 21H461 硬编码三处换新（前述）。
- **跨代 prelink 断裂 = 本轮最大发现（"换 16 代内核源码"大坑解除）**：首跑 prelink 挂 `Kernel target 0xfffffff0077cf004 has 0 native address candidates: ['_IORPCMessageFromMach']`。侦察（temp/audit-prelink-binding-gap.py）= 全闭包 387,416 位点看似 139,576 missing，**按 selected 口径（roots 31 组件+依赖闭包）实际只有 2 个失配位点、同一符号 `_IORPCMessageFromMach`（IOStorageFamily/IOHIDFamily 各一），ambiguous=0**——16 代闭包引用的 base 形态 RPC 入口在 17 代内核（xnu-10063）里实现被删（只剩 Reply 形态，id 校验 0x4da2b68d-only + 描述符步进语义不同，重绑不可行）。修复 = **恢复 16 代函数**：下载 xnu-8796.101.5 源码（tree/.cache/sources/，profile trustcache_tag 同源），profile patch `restore-iorpc-from-mach.patch`（IOUserServer.cpp 逐字恢复 16 代函数体；kIORPCVersionCurrent 两代同为 190615 语义等价；`extern "C"` 显式标注防 C++ mangling——第一版补丁缺 extern "C" 被编成 `__Z20...` 符号名对不上，used 属性防 dead-strip）→ **prelink 65,346 位点全绑通过**。用户问"要不要下载 iOS16 内核源码"的答案：**不需要换源码**，只需要一个函数（已下载 8796.101.5 存 tree/.cache/sources 备用）。
- **出件（全链已验）**：kernel.macho `fe8c7897…f8908`（entropy clean-apply ✓ + prelink 绑定核对 ✓ 两道门全过；`_IORPCMessageFromMach` 落 0xfffffff008931884）→ payload **`ee012043`**（96MiB，平铺 `out/m2381/payload-r83-ee012043.img`；payload.json 四样齐：ADT 86b6ced5 = d22 身份+新 root_match、trust eb1475f6 与 board pin 逐字节吻合（IPSW 重解 trustcache 对 63e01c03/9cc478fe 重新打包验证）、data seed d04a0153、skip_setup=false 红线）。
- **首刷三件套（下会话）**：boot_b←`ec1d518b`（r81 充电段件，ABL 尾字节容忍 = 实证项）+ recovery_a←`payload-r83-ee012043` + userdata←`userdata-root.img`（8.38GB，sha 2c2aca3f）。判读门 = r11 时代证据形态（entropy 守卫日志/绿蓝条/panic screen）+ 充电栈（BATTMGR GLINK/native_adsp_stats/充电电流）+ 20H392 用户态推进。回退件 = 整组 21H461 世代（21H461 政策对备份在案）。

**r84 判读工作单（2026-10-29 会话五开工；用户报"内核跑不起来"——首刷已发生，屏幕无内核迹象。纪律重申：屏幕状态从来不是判据，lsusb `1d6b:0104`（ACM）才是通道判据；黑屏不阻塞 reader）**：

**第一刀 = 设备现态三查（定分流，不猜）**：
1. `lsusb` 看 USB 枚举：fastboot（Google/18d1 形态 vendor）？ACM（1d6b:0104）？还是全静默？
2. **分支 A：fastboot 在** → 内核没起但 ABL 活着。先 `fastboot getvar` 核对 current-slot，然后读窗（见第二刀）——boot_b 件可能根本没被 ABL 接受（**ABL 尾字节容忍 = 本轮头号嫌疑**：boot_b 件 = boot.img 1.27MB + M2381FW 段 30.5MB，若 ABL 拒 image_size 之外尾字节 → FV 没加载 → 没有 UEFI → 没有 payload 投递 → 无内核。**实证法 = 回退刷纯 boot.img（同文件前 1,279,872B？——不对，纯 boot.img = package-m2381-boot.py 不带 --adsp-* 重新出件；或 dd 取 ec1d518b 件前 1,269,760B + 头），flash boot_b 单件后 reboot 看是否进 UniXNU 菜单**。这刀同时实证"充电段是死因还是无辜"。
3. **分支 B：ACM 在（reader 活着）** → 引导链到 payload 了，reader 菜单里选 reader 入口 → `acm-reader.py probe` → 三节点对拍（boot_b/recovery_a/userdata 前 N MiB sha，卡上已备）确认刷写落地 → `fetchchunk` 拉窗 → `read-m2381-ufs-log.py --all` 判读。注意：**reader 活着 = boot_b 的 UEFI + recovery_a 投递链全通，"内核跑不起来"的问题面收窄到内核本体/参数/投递细节**。判读点按优先级：①boot-progress=2 菊花是否画出（=y UEFI 帧缓冲路径）②entropy 守卫行（r11 形态：far 0x40 / g_crypto_funcs==NULL 崩 = 守卫失效或补丁漂移）③panic screen 文本（可诊断形态）④prelink 闭包行为（IORPC 恢复符号 @0xfffffff008931884 的运行时命脉——16 代 kext 首次调用它；崩点/REASON 行符号化按 §4 流程，re-tools 现役）⑤充电栈 BATTMGR/native_adsp_stats。
4. **分支 C：全静默（无 fastboot 无 ACM）** → 最深层：xbl/abl 层或供电。power+vol-down 强制进 fastboot；进不去 = 硬砖面（xbl 层，红线内不可修，联系既有恢复路径）。
5. **分支 D：设备在 nura 侧（NCM/devsh 可达）** → 用户刷完回 nura 了 → nura 侧先把分区读回对拍（三节点 dd|sha256sum，卡上 sha 表），确认刷写落地再定分流。

**判读门预注册**：
- **命中（内核起）**：窗里 entropy 干净 + 菊花/进度条出现 + 无 panic → 看 20H392 用户态推进多深（launchd/dyld/SEGV 族/REASON 行），逐项记录。
- **未命中（kernel 早死）**：窗里 SHLOG 槽都空/只有 header → 内核没起或起在极早期。判据 = slots_written 分代纪律（memory shlog-header-discipline）+ 早期 panic 文本（panic screen 若画出来了 = ABL FB 路径通，直接读屏上文本形态）。嫌疑优先级 = boot_b 尾字节（分支 A 实证法）> entropy 守卫 > prelink 闭包 > cmdline 983B（r81 新参数集在 20H392 内核上的接受度）。
- **特殊形态：SLEEP/halt**（r70 形态）非崩溃，别误判。

**下会话开工序**：三查定分支 → 按分支走 → 全程留档（窗文件 out/m2381/reader-r84-log.bin，判读回写本节 + 记忆）。**回退链随时可用**：纯 boot.img 回退（分支 A）→ 21H461 整组回退件（政策对备份 + fd43ee70 世代 userdata + 21H461 时代 payload 件）→ nura 回退链（红线 6，只在 nura 仓）。

**r84 首刷判读（2026-10-29，用户屏摄 `IMG_20261029_102213.jpg` 定谳——panic screen 直出，本轮免取证通道）**：

- **现象**：BootProgress live HUD（"live XNU - native 4K - no automatic reboot"）+ kernel console 镜像停在 panic。白字 = `failed to get mix-n-match policy: 2` + `frames`×12 + 标准 pages/free 尾注；HUD 头部 TTBR0=`8c5F08000`/TTBR1=`8c5F09000`（内核跑在高段 DRAM 窗）；UFS 行全零（requests=0 verified4K=0 root=0）= panic 在 root mount 前，时间轴一致。注：r84 纪律"屏幕不是判据"防的是"无迹象/陈旧 UEFI 画面"；本屏有内核 panic 文本（BootProgress 是内核组件，串只能来自内核 console 环）= 内核侧直接证据，足以定谳；lsusb/reader 三查仍可照做作交叉验证，不阻塞。
- **定谳 = r12 AppleImage4 mix-n-match 墙原样复现**：串唯一实例在 20H392 donor KC `com.apple.security.AppleImage4` __TEXT（§H：file offset 0x3e20262；panic+尾注形态逐字吻合）。换回 20H392 = 跨代配对复活 → r12 墙随之复活。**判读修正：§8 r79 段"那套墙当时已被驯服"对 mix-n-match 不成立**——当年 entropy 靠守卫驯服（r11→373f769），mix-n-match 未驯服、是 r13 换配绕开的（§I）。设备现停在 16 代 lane 的历史最深处（=r12 坐标；此之外全未开荒）。
- **连带正面五项**：①boot_b `ec1d518b` 尾段 ABL 接受——r84 头号嫌疑"尾字节不容忍"**排除**（分支 A 实证法作废）；②UEFI→recovery_a payload 投递链通；③entropy 守卫生效（r11 墙未复现）；④prelink 绑定可执行（内核起、16 代 kext start 已在跑）；⑤panic screen + HUD 帧缓冲路径通。充电栈判读（BATTMGR/充电电流）被此墙挡住——它在更晚的驱动阶段。
- **12 帧地址已收**（屏摄）：`0xfffffff0046e9f60 / 46e4b30 / 46f1420` 一簇 + `0xfffffff008857598 / 88545d0 / 885486c / 88538e8 / 8853804 / 89e1094 / 8962bec / 82807c8 / 83d24a4` 一簇（0x8931884=`_IORPCMessageFromMach` 同域）——符号化待做，串身份已定谳不阻塞。
- **下一步分叉（用户问"换 ios17 版本可以吗"，待拍板）**：
  - **甲（推荐）= 修墙续攻 16 代**：§H 预注册候选仍在案。第一刀 = ①re-tools 符号化 12 帧 + 在现役 KC（2ec297ba + prelink，kernel fe8c7897）定位 getter 调用点（r12 地基可复用：串@0x3e20262、_panic 导入槽 0x5cabb18、_img4if 22 槽已 dump，差 double-deref/blr 导入形态未破）；②按数据源分叉——getter 若读 ADT `/chosen/manifest-properties`（`UniXNU-m2381/scripts/make-m2381-adt.py:204` 现 present-and-empty）→ 补 AMNM 属性即活，零内核改动；源在 CSM/chip 观测 → OS profile text_fixups 定点守卫（§4.9 entropy 先例）。出件只动 payload → **只刷 recovery_a**（boot_b/userdata 不动），读回对拍照旧。
  - **乙 = 换回 21H461**：回退链完整在案（政策对备份 + fd43ee70 世代 userdata + 21H461 payload；boot_b 不用动——UEFI/充电段 donor 无关）。成本 = 重建 payload + 一次刷写；但回到 r77 原地，=y 卡点（displayless）未变，不产生新信息。
  - **止损线建议**：甲攻 1–2 轮。若 mix-n-match 修掉后紧出第 3 面同根因墙（entropy/mix-n-match 已是"同根因的两个切面"，§I 原话），= 16 代开荒税实锤 → 直接转乙并按 §7 =y 工作单续攻（r78 探针设计在案）。

**r85 = mix-n-match 墙第一刀实施完毕（2026-10-29 会话六续，用户拍板"开修"；设备仓 b91579e）**：

- **RE 全链（r12 地基全部换算到现役 macho）**：①屏上 12 帧 = runtime VA **恒等映射**（=macho 链接 VA；"slide −0x1000000" 只是 prelink 摆放约定，不参与换算——12 帧全部落位：帧 1–3 = AppleImage4 代码，帧 4–12 = 内核 `kernel_bootstrap_thread → mac_policy_initmach → KLDBootstrap::loadSecurityExtensions → OSKext::loadKextWithIdentifier → OSKext::load → OSKext::start`）②panic 串/邻居串（0x3e20262 等）= **link VA**（r12 记的"文件偏移 0x3e20262"数字相同但性质是 VA）③AppleImage4 代码在 `__PLK_TEXT_EXEC` 0x46e4XXX–0x46fAXXX；r12 的"double-deref 导入形态" = 静态绑定 PLT 槽（0x5cab828=`SecureDTLookupEntry`、0x5cab818=`SecureDTGetProperty`、0x5cabb18=`_panic`，全部用 nm 实名）④panic 点 = AP 实例化方法（0x46e98cc）内 `cbnz w0 → panic("failed to get mix-n-match policy: %d", ret)`，ret=2 来自虚表调用 `[self+0x38](self, x21, id=12)`：kext 本地观测注册表查 id 12 不存在（Qualcomm 无芯片驱动）→ 2⑤**DT 约定无跨代断裂**：`pexpert/pexpert/device_tree.h:129` kSuccess=1，与 kext 的 `cmp w0,#1` 吻合；`/chosen/manifest-properties`+BNCH 是查到 policy 之后的下一层，非本墙数据源。
- **修法 = TXM 短路单指令补丁**：该方法自带捷径（`[self+0x1a8] bit1` 置位 → 跳过整个 policy/manifest/BNCH 块，log "system contains a TXM runtime"，经 log 尾调用返回；外层环境对象循环对返回值**零检查**）。补丁 `0xfffffff0046e9924: tbnz w8,#1 (0x37080908) → b (0x14000048)`。脚本 = 设备仓 `scripts/patch-appleimage4-txm.py`（VA→offset 每次从 `__PLK_TEXT_EXEC` 段重 derive；原字不匹配拒绝写；sidecar `.patch.json`）。kernel **55fcee13**。
- **出件**：payload **`5bc0c023`**（96 MiB，`out/m2381/payload-r85-5bc0c023.img`；trust eb1475f6 + seed d04a0153 + skip_setup=false + panic screen；ESP 四样齐，ESP 内核副本已回读验证补丁字在位）。**两个构建坑**：①`--trust-cache/--data-ramdisk` 必须显式传（首轮漏 --trust-cache → trust_cache null + ADT 缺 TrustCache memory-map 条目少 52B，r56 同族）②ADT `platform-random-seed` 每次生成新随机值 → ADT sha 永远逐字节对不上旧轮（非回归）。
- **r86 判读预注册（只刷 recovery_a）**：①补丁生效标志 = kernel console 出现 **"system contains a TXM runtime"** 行 + mix-n-match panic 消失，kext start 推进到下一个 donor 墙（按 panic 屏文本判读；候选 = 其它环境对象方法的类似芯片查询、AMFI/mac_policy 域、SecureDT 层）②同墙复现 = 对象到达分支时位形态与假设不符 → **plan B** = `0x46e99dc` cbnz→nop（跳过返回检查）或方法尾部 `mov w0,#2→#0`（0x46eac18，全部缺失观测返回"成功零值"）③横雷备注：TXM 路径跳过了 manifest-properties/BNCH 获取，后续若在别处消费这些状态出现新墙，其证据会指向下一个消费者——按 panic 文本定位即可。回退件 = payload-r83-ee012043（21H461 整组回退链不变）。

**r86 状态（2026-10-29 会话六收尾）**：**recovery_a ← payload-r85-5bc0c023 已刷**（用户报"刷完了"，设备仓 23035bb；**dd 读回对拍未确认——判读开工第一件事先补**（`dd recovery_a | sha256sum` 对 `5bc0c023…cc05`），没读回前不算落盘证据）。下会话开工序：读回对拍 → 三查（panic 屏照片 / lsusb 1d6b:0104 reader / fastboot）→ 按上面判读门分流：TXM 行 + 墙消失 = 推进（新墙按文本定位）；黑屏/无 panic = reader 拉窗（r84 分支 B 流程，`out/m2381/reader-r86-log.bin`）；同墙复现 = plan B 出件。

**r85/r86 判读 + r86 第二刀实施（2026-10-29 会话七；用户先问换线，后拍板"继续修 ios16，出件我来刷"）**：

- **r85 判读定谳（屏摄 + 本机符号化闭环）**：**TXM 捷径生效实锤**——panic 形态全变（断言 panic → data abort），现役 kernel（`55fcee13`）补丁字 `0x14000048` 在位 → mix-n-match 墙过掉。新墙 = **AppleImage4 同 kext 描述符装载函数**：esr `0x96000005`（data read TF L1）+ far `0x18` + x19=0。崩函数 0x46e467c：三次字符串调用（0x46fa0d0）→ `bl 0x46f0e20` 得 env 对象（x0=0x5caceb0，活指针）→ `ldr x19,[x0+8]` = **NULL**（芯片观测注册表 Qualcomm 无注册，r84 id=12 同源）→ 0x46e46c0 `ldur q0,[x19+0x18]` 崩。调用方 0x46f1430（全局页 0x5cb14c0）；r84 九帧内核簇原样再现；pages/free 非零 + HUD 计数全零 = 仍 root mount 前。**0x46f0e20 文件态全零 = kxld boot 时按 relocation 回填的外部引用区**（静态分析到头，运行态行为由 lr/寄存器钉死）。工具留 `temp-r85-sym/`（symmap/disasm/strings）。**同根因第三面（entropy/mix-n-match/空注册表解引用）→ 预定止损线触发**；用户知情后拍板继续修。
- **r86 修法 = 零表指针单指令**：`0xfffffff0046e46b4: ldr x19,[x0,#8] (0xf9400413) → adrp x19,0xfffffff009223000 (0xf00259f3)`。该函数 = 芯片观测描述符装载（拷 0x30 字节进全局页 0x5cac32a + 两道计数 panic 护栏），**零表让全路径结构性干净**：描述符拷全零、`[x19+0x48]`=0 过 ≥0x11 护栏（0x46e46f0）、`[x19+8]`≠哨兵走 b.ne、env 二/三段读走真对象（0x46e473c ≥4 护栏与 0x46e4758 cbz 不受影响）。**否决两个备选**：跳到 0x46e4790 = 落格式化 panic 路径（0x46fa028 带 fmt+栈参，是 panic 非 log）；整函数跳过 = 丢副作用。零页选内核 `__DATA` bss 末页（vm 0x89ec000+file 0x44000 → bss 0x8a30000..0x9224000，紧邻 `__BOOTDATA`；demand-zero + kext relocation 不碰内核 bss + 远端最少被早 boot 对象占据）。脚本 = 设备仓 `scripts/patch-appleimage4-chipobs.py`（adrp 编码脚本内算 + 解码回读自检 + 原字校验 + sidecar）。kernel **72aa6fd6**（基 = r85 件，TXM 字 `0x14000048` 共存已验）。
- **出件**：**payload-r86-2032f813.img**（96 MiB，`out/m2381/`，sha `2032f813…e52b`；trust eb1475f6 + seed d04a0153 + chip-id 3645ad55d47e77c1 + panic screen；**ESP 内核副本回读双补丁字 PASS**）。配方 = r85 同款（`--kernel` 显式喂补丁件）。
- **r87 判读门（只刷 recovery_a；boot_b ec1d518b / userdata 2c2aca3f 不动；刷后顺带补 r86 读回对拍 `2032f813…e52b`；r85 欠账 `5bc0c023…cc05` 已被覆盖、作废）**：①**推进** = 原崩点消失，出现新 panic 文本或新崩点——第一候选 = 同函数第二消费 `[env+0x10]`（0x46e4734 → `[x9+8]` 计数 vs 4，b.hs 落 0x46fa028 **格式化 panic，屏上会有文本**，按文本定位下一刀）；第二候选 = AppleImage4 start 后续步骤或 kext 链下一家。②**原墙复现** = `[x0+8]` 并非 NULL 的假设错（屏上 x19 非 0 即证伪）→ 转真表指针方向。③回退件 = payload-r83-ee012043 / r85 件 / 21H461 整组不变。**战略备注**：上游 `feat/ipados27-gpu-parity`（arm64e + 26.5.2/27.0.1 双 profile + fileset 重绑定 + 原生 PAC，sheng 同 SoC 在研）已侦察入档，为换线候选——本轮继续修 16 代为用户拍板路线。

**r86 判读定谳 + 取证闭环（2026-10-29 会话八；用户已刷 r86 并进取证通道；"下个会话修"）**：

- **r86 屏摄判读 = 精确命中预注册第一候选，零表补丁实机生效实锤**：data abort @ `pc 0xfffffff0046e4738 / lr 0x46e4734`，esr `0x96000005`，**far `0x8`**，**x19 = `0xfffffff009223000` = 零页补丁原值出现在实机寄存器**（adrp 开火、零页可读）。崩点 = `0x46e4738 ldr x8,[x9,#8]`，x9 = `[env+0x10]` = **NULL**——同一 env 对象（x0=0x5caceb0 与 r85 同值）的第二个未注册指针。执行前沿从 r85 的 0x46e46c0 前移到 0x46e4738（+0x78 字节）：描述符拷贝、≥0x11 计数护栏、哨兵比较、`bl 0x46f9d94` 全部走通。调用方 0x46f1430、九帧内核簇与 r84/r85 逐帧相同。HUD root=0/procs=0 = 秒崩（用户观察吻合）。
- **取证窗判定 = 陈旧数据（用户纠正正确，我看错的第一处）**：拉取的 log.bin（fetchchunk SHA MATCH，`2698e6f9…`）= 337 槽/678s"过 root mount + launchd + 196 进程"的 boot——**不是 r86**。判据：TXM 行 0 命中、sheng-adsp/cmdline 标记 0 命中、coreduetd 在册（229 命中）而 liveactivitiesd 缺席 = **r44+ =y 车道禁用名单指纹** → 该窗 = r77 时代 21H461 最后一次过 root mount 的 boot。**机制**：BootLog recorder 是内核侧组件，r84/r85/r86 全死在 root mount 前没活到能写窗 → 窗口自 =y 时代从未被覆盖。**纪律沉淀：秒崩 boot 的证据通道只有 panic 屏摄；reader 窗要等 boot 活过存储初始化才有货；读窗第一步的 boot 身份核对（TXM/args/禁用名单指纹）先于一切解读**。取证件 + README 归档 `out/m2381/reader-r86/`。
- **r86 读回对拍 = PASS（欠账销账）**：reader shell `dd /dev/sde27`（recovery_a）前 96MiB 设备端 sha256 = `2032f81381596cbd0d91b62a699d33c41e7230420ee1f3ac3e859c45cd85e52b` = payload-r86-2032f813.img 逐字节吻合。
- **r87 修法预注册（下会话开工单，具体病灶+修法）**：`0xfffffff0046e4734: ldr x9,[x0,#0x10] (0xF9400809) → adrp x9,0xfffffff009223000 (0xF00259E9，同零页，delta 页数 0x4B3F 相同)`。后续路径静态推演已闭环：x8=[零+8]=0 < 4 过 0x46e473c 护栏 → str 0 → 0x5cac356 → `bl 0x46f0e20` → `[x0+0x30]` cbz 分支 = 干净返回（若 [x0+0x30] 非空走 0x46f9dac 尾拷贝也安全）。脚本 = 扩展现有 `patch-appleimage4-chipobs.py` 或新增同构脚本；基 = kernel-r86-chipobs.macho（三补丁共存）。风险预注：同一 env 对象的其余 NULL 指针槽（+0x18/+0x20…）可能在 start 更深处被别的方法消费——同族单指令推进，按屏摄逐个点名。
- **UEFI 引导界面充电侦察（用户需求"安排上"）**：上游 uefi-sheng 增量仅 5 提交（docs×3 + ShengBootMemoryDxe 换共享 RAM disk 驱动），无字体/充电新货；Font.h/EFIChargerEx.h 为老提交。"引导界面充电"的实体 = **原厂高通 UEFI 二进制驱动收编**：`Binaries/<device>/QcomPkg/Drivers/QcomChargerDxe/QcomChargerDxeLA.inf` + `ChargerExDxe/ChargerExDxe.inf`，LG mh2/caymanslm、华硕 i005d、联想 9707f/j706f 各包 DXE.inc 均此挂法。m2381 = SM8550，原厂 abl 内有同族驱动，收编路 = 从原厂 abl（设备 sda 分区红线外只读 dd 或 nura 固件收割）解出两 INF+依赖 → m2381Pkg DXE.inc 挂载 → FV 出件刷 boot_b。字体 = FrameBufferSerialPortLib Font.h（已在我们基座，非新货）。**独立车道，不挡 iOS16 修墙线**；待用户排期。
- 回退链不变：payload-r83-ee012043 / r85 / r86 均在 `out/m2381/`；boot_b ec1d518b / userdata 2c2aca3f 不动。

**r87 = 芯片观测计数指针补丁实施完毕（2026-10-29 会话九，设备仓 c477f48；出件待刷）**：

- **第二刀按预注册单刀落**：`0xfffffff0046e4734: ldr x9,[x0,#0x10] (0xF9400809) → adrp x9,0xfffffff009223000 (0xF00259E9，同 r86 零页，delta 0x4B3F)`。脚本 = 新增同构 `UniXNU-m2381/scripts/patch-appleimage4-chipobs-count.py`（adrp 编码+解码回读自检+原字校验+sidecar），链 = kernel-r86-chipobs.macho → **kernel-r87-chipobs-count.macho `ba4080fc`**（三补丁共存：r85 TXM `0x14000048` @0x46e9924 + r86 adrp x19 @0x46e46b4 + r87 adrp x9，三字全验）。
- **反汇编复核（capstone 全函数，机制用眼睛看）**：补丁后路径逐指令与预注册闭环吻合——零页全零描述符拷贝 → `[x19+0x48]`=0 过 0x11 护栏 → `[x19+8]`=0≠哨兵走 b.ne → `bl 0x46f0e20` 重取 env → r87 位点 adrp x9 → `[x9+8]`=0 过 #4 护栏（0x46e473c）→ 存 0 到 0x5cac356 → 0x46e4750 再取 env → `[x0+0x30]` cbz → 0x46e478c 干净 `ret`（非空分支走 0x46f9dac 尾拷贝，源 = 真对象，同样安全）。
- **出件**：**payload-r87-b93829e9.img**（96 MiB，`out/m2381/`，sha `b93829e9…7683`；kernel `ba4080fc` + trust eb1475f6 + seed d04a0153 + chip-id 3645ad55d47e77c1 + skip_setup=false + panic screen；ESP 四样齐 + initrd 行在；**ESP 内核副本回读 sha a92c8437 = payload_kernel_sha256 且三补丁字全 PASS**）。构建坑一枚：`-D` 等公共选项必须放子命令之后（`./unixnu payload -D …`，argparse 子解析器结构；AGENTS.md 的前缀写法在本版 cli 不成立）。ADT sha 逐轮变化 = platform-random-seed 每次新生成，已知非回归。
- **r87 判读门（只刷 recovery_a；boot_b ec1d518b / userdata 2c2aca3f 不动；刷前 sha 门 `b93829e9…`，刷后读回对拍）**：①**推进** = 0x46e4738 崩点消失（预期静态闭环：函数干净 ret，kext start 越过本方法）——下一证据形态按屏摄定位：同 env 对象其余 NULL 槽（+0x18/+0x20…，风险预注在案）被更深处消费 = 同族 data abort，寄存器/far 点名下一刀；或护栏翻落 0x46fa028 格式化 panic（屏上有文本）；或 AppleImage4 start 整体越过 → 内核推进（panic 形态换内容/换 kext）。②**原墙复现** = 不预期（零页可读性已被 r86 实机证明，x19=0x92230000 在寄存器）。③**证据通道纪律**：秒崩 boot 唯一证据 = 屏摄（reader 窗仍是 r77 时代陈旧数据，读窗先验 boot 身份，在案 `out/m2381/reader-r86/`）。④回退件 = payload-r86-2032f813 / r85 / r83 / 21H461 整组不变。

**r87 判读定谳 + r88 第四刀实施完毕（2026-10-29 会话九续，设备仓见 git log；出件待刷）**：

- **r87 屏摄判读 = 预注册分支①命中，第二刀实机生效实锤**：0x46e4738 data abort **消失**，整个芯片观测描述符装载函数干净 ret，kext start 越过该函数推进到下一个消费者。新证据 = **格式化 panic** `failed to get previous stage manifest hash: 2` + 两帧新地址 `0x46ed6a0 / 0x46f1820` + r84 九帧内核簇原样；HUD root=0/procs=0 = 仍 root mount 前（kext init 期）。同根因第 5 面（entropy 守卫→mix-n-match→观测注册表→观测计数→manifest hash），但 panic 给文本 = 可定点。
- **RE 全链（机制用眼睛看，capstone 全程）**：frame2 = `0x46f181c bl 0x46ecc14` = **nonce manager init**。函数入口先把 64 字节 hash 缓冲（sp+0x460）零填充（0x46ecc50），第一查 `40A0DDD2-77F8-4392-B4A3-1E7304206516:nonce-seeds`（0x46f06c0）ret=0；第二步经 env 对象 vtable `[x19+0x50]`（key 0xb）取 previous stage manifest hash 写入该缓冲，ret=2（Qualcomm 无 iBoot 链、无前级 manifest）→ `cbnz w0` @**0x46ecd68**（0x35004940）→ 0x46ed690 panic 块（fmt 0x3e2035d，0x46fa028=panic；frame1=0x46ed6a0）。**关键语境**：①姊妹输入 seeds 的 ret==2 有 Apple 自带容忍路径（"nonce manager: error reading seeds: %d" 记日志继续，0x46ecd18）；②hash 缓冲零值即发布值（尾部 0x46ed308 把它发布到全局 0x5caea42），长度不合路径全是记日志继续非 panic；③串池与 r12 mix-n-match 串同池（0x3e20308 "…continuing because mix-n-match is permitted"——0x46e98cc 政策函数内 0x5cacef0 旗标门，r85 TXM 捷径跳过的赋值面，本轮未消费）。
- **修法 = `0xfffffff0046ecd68: cbnz w0 (0x35004940) → nop (0xD503201F)`**：manifest hash 查询失败时沿用缓冲零值继续（="无前级 manifest"语义，与姊妹输入容忍同构）。脚本 = 设备仓 `patch-appleimage4-noncemgr.py`（原字校验+sidecar）。链 = kernel-r87-chipobs-count → **kernel-r88-noncemgr `8e807ef5`**（四补丁共存四字全验）。
- **出件**：**payload-r88-c932a128.img**（96 MiB，`out/m2381/`，sha `c932a128…`；kernel 8e807ef5 + trust eb1475f6 + seed d04a0153 + chip-id 3645ad55d47e77c1 + panic screen；**ESP 内核副本回读 sha d2eea5ed = payload_kernel_sha256 且四补丁字 PASS**）。
- **r88 判读门（只刷 recovery_a；boot_b ec1d518b / userdata 2c2aca3f 不动）**：①**推进** = panic 文本换新/崩点换址——按屏摄点名下一刀；候选 = nonce manager 下游消费者（全局 0x5caea42 零 hash 的下一个读者）、AppleImage4 start 更后步骤、kext 链下一家；若内核推进过 root mount，证据通道自动升级为 reader 窗（秒崩纪律失效）。②**原墙复现** = 不预期（ret=2 语义已实证）。③回退件 = payload-r87-b93829e9 / r86 / r85 / r83 / 21H461 整组。

**r88 判读定谳（2026-10-29 会话九续二，屏摄 `IMG_20261029_204307.jpg`；**墙的性质换挡**，修墙暂停待战略拍板）**：

- **推进实锤**：r88 nop 生效——manifest hash panic 消失，kext start 又推进一段。新墙换挡：**数据 abort 崩在内核本体** `pc 0xfffffff0086af538`，esr `0x96000004`（L0 translation fault，read），far `0x455f5458455464f7` = **ASCII**（x0=`0x455f545845545f5f` = `"__TEXT_E"`，LC_SEGMENT_64 segname 首八字节，far = x0+0x598）。
- **全链符号化（LC_SYMTAB 187k 符号，内核自源码构建全量在档）**：崩点 = **`_csproc_get_blob`**（`ldr x8,[x0,#0x598]` = proc->cs_blob 槽，x0 却是载入命令指针）；调用方 = AppleImage4 回调 `0x46fc3f8`（sret 函数：`ldr x0,[x0,#8]` 取 segname → 导入桩 0x4718278（槽 0x5cb5aa0，kxld 回填）→ `csproc_get_blob(垃圾)`）；上游 = **`OSKext::start(bool)+0x370`** 的回调派发点（`blr x22` 前带 callee∈kext 文本范围校验，帧 0x88545d0=`OSKext::load`/0x88538e8/0x8853804=loadKextWithIdentifier 簇原样）。
- **定性 = 墙类换挡**：前五面（r84-r88）= "芯片观测/manifest 注册表为空"类，单指令可语义闭合；本面 = **内核/kext 边界契约级**（回调参数约定或 vtable 槽位跨代漂移——16 代 kext 期望 vs xnu-10063 内核，x0 应为对象实为 mach-o 载入命令）——正是 r83 预注"16 代 ABI 重审=后续主题"的账单开始到期的形态。此层级非十分钟单指令，且同类边界在 donor 用户态+kext 里数量庞大。**修墙动作暂停，等战略拍板**（批量静态地图 / HackiOS 开采 / QEMU 立项 / 换线，见会话记录）。
- **坑一枚（工具债）**：本 macho 的内核段与 prelink 段**文件区间重叠**（`__DATA` file 0x19e8000-0x2220000 与 `__PRELINK_TEXT` file 0x218b000-0x294f000 重叠）——串文件偏移 0x21e335d 双映射（kernel __DATA 0x91e735d 与 kext cstring 0x3e2035d 皆是），按 VA 找串必须用 `__PRELINK_TEXT` 口径（kext cstring 真身），否则 xref 扫描错段全零。

**引导界面充电 = 转上游待办（2026-10-29 用户拍板"等上游完善"，本会话侦察结论存档）**：现役 FV **已含整套充电驱动**（mars Binaries：ChargerExDxe + QcomChargerDxeLA + PmicGlinkDxe + UsbPwrCtrlDxe，depex 全 TRUE 无条件派发）——非"缺驱动"问题；嫌疑点 = GLinkDxe 在 DXE.inc 被注释（nura 遗产，APRIORI.inc 却仍在列 = 潜在不一致）与 mars 参考板配置 vs m2381 板级差异；本地收割面 `artifacts/vendor/meizu20-uefi-mods/`（166 件）已有 OEM 充电全家桶（QcomChargerDxeLA/ChargerExDxe.efi+.depex）可作替代 blob；stock 侧 off-mode charging 实体在 XBL 域（nura XBL-RE 在档），ABL 有 `oem enable/disable-charger-screen`/`off-mode-charge` 变量面。上游 uefi-sheng 有动作时按 mh2/i005d/9707f 模板对照重启此车道。

**"不打地鼠"三层方案（2026-10-29 会话九收尾，用户问"有没有系统性的修法"，待拍板）**：r88 判读定谳后墙类换挡（注册表空类 → 内核/kext 边界契约类），修墙冻结。三层 = **第 0 层（当天，零构建）**：21H461 已知良好配对回归当控制组——payload-r77-93854822.img + userdata fd43ee70（本地 sha 已验）两刷即达，r77 实证 = 内核+prelink+kext+root mount+launchd+196 进程+SB 深度推进全通，证明"内核能不能跑"与 20H392 六面墙无关；已知代价 = r77 件不含充电栈（r81 后才有），补法（半天）= board.toml 切回 21H461 profile + 现役内核源重建（r82/r83 反向全程有脚本 + verified-root-21h461-backup 在案 + 整棵 ipados-17.7.11-21H461 构建树健在）。**第 1 层（本周，天级）：QEMU 立项** = 不打地鼠的仪器（每墙小时级→分钟级 + gdb 全量观测）；首选标的 = 21H461 配对攻 displayless 单根因（IOMFB/FDR 注册链在可控硬件上全量可见）；路径 = 先开采 HackiOS QEMU 配置（同 donor iPhone10,3 同代 16.5 已跑通）→ 载体优先"精简 FV 进 QEMU"（Mu 基座 QEMU 血统 + DarwinBoot 吃 payload.img）。**第 2 层（战略终局）：上游 26.5.2/27 arm64e 线**（displayless 在该线不存在 = sheng 同 SoC 真 display+GPU；施工单在档 upstream-ios27-arm64e-lane；QEMU 复用为预检台）。20H392 线处置 = 冻结（boot_b ec1d518b 不动，回退件全在案）；QEMU 批量审计（导入桩×内核符号全量对拍）若清干净 ABI 墙可复活，否则归档"已证伪的近路"。

**iOS16 vs iOS27 两线重估（2026-10-29 会话十，用户拍板"就是想跑ios16…或者开始考虑ios27，看看上游有没有更新"）**：

- **上游实况（git fetch 实测）**：UniXNU main 仅 +1 docs；**feat/ipados27-gpu-parity +54 commits**（GPU 深水区：metal2vulkan 0dfdbcd/固定 Turnip cache identity/预先编译 pipelines/Adreno740 BO 管理与 SMMU fault termination/ArmSmmu4K bump/GopFramebuffer kept surface mappings）= sheng 同 SoC 显示与基础面已通、正打 GPU；uefi-sheng 零新增（现役基座 0eb4514 已最新）。
- **HakiOS 重大情报（10-24 评估后五天，github oosexgang，WebFetch 实测）**：xnu-8796.121.2 + stock iOS 16.5 用户态 + iPhone donor，**真机 POCO X5 Pro（redwood/SM7325）已到锁屏+主屏+控制中心+原生 app+触摸+8 核+Adreno GLES(Mesa)+扬声器/麦克风/电量 UI+Wi-Fi+振动+/private/var 持久+Sileo/Procursus/SSH**；QEMU（virt）也到锁屏（qemu/run-ios.sh，2-3min）；未通 = 蜂窝注册/蓝牙/**本内核下充电**/Metal（仅 GLES）/相机/SE·Apple 账户。移植模型 = devices/<name>/ 目录制（已双设备：SM7325 + MT6768 MediaTek）；真机装载 = fastboot RAM 加载（"nothing flashed"）；1.9k stars/632 commits/日更/APSL2.0 合规在档。
- **判读**：①"iOS16=跨代 ABI 打地鼠"的死结有了出路——**同代内核基座（xnu-8796）现成且在真机 Qualcomm 上验证到主屏**，ABI 漂移类墙由构造消除；②他们在 QEMU 跑通用户态 = Apple 基础设施缺失墙（我们 r84-r88 撞的同族）他们必有替身/注入解法可开采；③m2381 适配面 = SM8550 板级包（stock FDT/DTS 证据 + m2381Pkg UEFI 资产复用；红线5 → 须走我们的 UEFI boot 链投递，不能照搬其 fastboot RAM 模型）；④风险 = 项目 5 天新、单真机设备、SM7325→SM8550 平台差、GPU 栈不同（他们 Mesa/freedreno vs 上游 Adreno 原生）。
- **两线施工概要（待拍板主线）**：**A = iOS16@HakiOS 基座**：克隆开采（QEMU 先跑通 qemu/run-ios.sh 验工具链 → 开采 SM7325 包与 Apple 基础设施替身解法）→ m2381 devices/ 包（我们的 FDT/常量）→ boot 路径决策（m2381Pkg UEFI 载其内核+DT）。**B = iOS27@上游**：rebase 到 feat/ipados27-gpu-parity → 双 profile（先 26.5.2-23F84）+ ADT j820ap + fileset 重绑定/原生 PAC（上游已落地）+ 19 驱动仓；bump 54 commits 全是红利。两线共用：m2381Pkg/board 资产、QEMU 仪器。

**"iOS16 匹配内核拿来用"可行性侦察（2026-10-29 会话十续，用户拍板"不想弄 hakios，先把 ios16 的匹配内核拿来用"；克隆 `research/hakios-xnu-kernel` 实测）**：

- **内核仓形态 = 完美贴合我们的模型**：非整树 vendored，而是 `xnu/hakios.patch`（**3,239 行、73 文件**：bsd 29/osfmk 22/iokit 7/pexpert 4/makedefs 3/libkern 2/config 2）+ `files/` 新文件 + `apply.sh`（从 apple-oss-distributions/xnu GitHub 拉 **xnu-8796.121.2 tag**（BASE c54f35ca）打补丁成 xnu-src/ git 树）。Linux 原生 clang + theos cctools 构建，`xnu-build.sh` = Apple 原 Makefile 系统（MACHINE_CONFIGS=BCM2837 通用 arm64 机配置，DEVELOPMENT 配置，12 线程约 1 分钟出核）。
- **关键发现（必须知道的哲学差）**：HakiOS = **去 kext 化模型**——无 prelink/kernelcache 机构；驱动用 Mini* 内置替身（MiniIOMobileFramebuffer/MiniAppleKeyStore/MiniIOSurface/MiniIONetworkStack/GenericARMPE + libkern/amfi stub + kern_minimac.c = AMFI/Sandbox stand-in，"nothing is signed here"）；wifi = ath11k 对象直链内核；用户态 = IPSW 取件 + 自组 rc.jobs + 注入。**他们的 16.5 用户态在 AMFI stub + 无签名下跑到主屏**。
- **三路线定谳**：①字面整拿 = 接他们的去 kext 哲学 = 变相"弄 hakios"（用户已否决）+ 充电/显示全要按 Mini 模式重写；②纯自力（UniXNU 59 补丁重放 8796）= 账单最大且无先例；③**推荐 = 内核基座置换**：整 patch 拿来当 8796 地基（平台/调度/bsd 层已被他们在真机调通），20H392 kext prelink 与我们组件按需逐步叠上去、Mini 替身逐步换真——我们的 UEFI/boot 链/设备仓/profiles/prelink kit/root 全不动。
- **前置验证点（Phase 1 活）**：a) 121.2 vs 101.5 tag 差 × 20H392 kext 绑定（同 8796 大版本，r83 的 selected 口径审计工具可实测）；b) prelink kit × 他们的树（同 Apple tree 布局，kit 应能直接吃）；c) AMFI stub 与我们签名用户态的共存策略（内核先跑不依赖此项）；d) 组件移植账单（ADSP 充电/ShengUfs/entropy guard 在 8796 重放；entropy 墙可能随 8796 corecrypto 序自然消失）。
- **施工序**：Phase 1（1 会话）apply.sh 出树 → kernel.py/defconfig 适配 → 裸核起（串口/HUD）；Phase 2（1-2 会话）prelink 20H392 KC + 三值 derive + 组件移植；Phase 3（1 会话）payload + 上机判读。许可证 = APSL（kernel patch），研究用途与仓内现状一致。

**iOS27 线开工（2026-10-29 会话十定案：用户拍板"保守一点，用 unixnu 方案，开始搞 ios27"；HakiOS 基座否决归档；本会话只做勘测+分支准备，施工下会话开始）**：

- **重大利好（勘测实测定谳）**：上游 `feat/ipados27-gpu-parity` 的 `profiles/os/ipados-26.5.2-23F84/profile.toml` **自足且 donor = iPad17,3/j820ap/chip 0x8142/board 12 = 与我们目标一致 → m2381 大概率直接复用 profile，零 profile 编写**。profile 内含：IPSW CDN URL（23F84）、prelink 全套（emit_prelink --fileset-reference、kernel 路径、KC sha、slide -0x1000000、early_security_init=AMFI 0xfffffe0008f13c80、roots 31 组件）、ADT 身份、六个 Apple 源 pin（IOStorageFamily/IOHIDFamily/libplatform/libdispatch/AvailabilityVersions/libpthread）、userdata windows、root 卷 UUID+镜像摘要（os 546DFF4A / SystemOS 7A23BAC7）。同分支还有 27.0.1-24A446 profile 备用。
- **分支手术（已就绪）**：`m2381-ios27` 已建（= m2381 tip 5566b34），备份 = `m2381-backup-20261029`；rebase 已试跑并**中止在已知点**（过夜不留半成品）。冲突面实测 = 仅 9 文件重叠，其中实质三类，处置已定：①**gitlink 类（DarwinBoot 等）→ 一律取上游 pin**（arm64e 支持在上游；循环脚本已写好：`git ls-tree upstream/feat/ipados27-gpu-parity -- <path>` 取 160000 pin `git update-index --cacheinfo`）；②**scripts/unixnu/payload.py → 双保留合并**（HEAD 的 seal-source-crypto-kernel.py 块（source corecrypto POST 封印，arm64e 必需）+ 我们的 `UNIXNU_DEVICE=str(ctx.device)` env 透传，合并形态已在案）；③**profiles/os/ipados-17.7.11-21H461/profile.toml → 取上游版**（我们的 212 行 displayless fixup 史全死，史在 git）。kernel.py 的 bootstrap 健壮性补丁应干净落。
- **下会话开工单（按序）**：①重跑 rebase 循环（`git rebase --onto upstream/feat/ipados27-gpu-parity upstream/main m2381-ios27` + 上述三类处置，约 15 分钟机械活）→ ②check-m2381（device 仓独立，先不动 board.toml）→ ③board.toml `os_profile = "ipados-26.5.2-23F84"` + **defconfig 行生**（m2381_defconfig 加 arm64e/fileset/native-pac 项，参照 = 上游 docs/devices/sheng.md 指向的 sheng 设备仓 defconfig）→ ④IPSW 23F84 下载（URL 在 profile；`./unixnu ipsw fetch` 或直下，7-9GB 级，早 kicked off 好）→ ⑤defconfig → tree → kernel 第一遍（GH_TOKEN 记得带；两次构建语义照旧）→ ⑥verified-root 政策对 + Data UUID derive（APSB 扫描器/政策构建器全线现成，换 23F84 身份）。boot_b 预期不动（sheng 同基座已在跑 26.5.2 = m2381Pkg 的 0eb4514 基座原生支持）。
- **冻结面**：20H392/iOS16 线冻结（r88 为终点，回退件全在案）；HakiOS 内核基座评估归档（STATE 上节，若 iOS27 线遇阻可重启）。

**iOS27 线开工轮 = rebase + 换配落地（2026-10-29 会话十一，六步工作单之①②③完成；主树 tip `c2bfe13`，设备仓 `39a8ef7`；备份 `m2381-ios27-pre-rebase-20261029`=rebase 前 33b50fe）**：

- **①rebase 完成**：`git rebase --onto upstream/feat/ipados27-gpu-parity upstream/main m2381-ios27`，233 个工作区提交全落 feat tip `e5f729d`。停点四起全按既定处置：DarwinBoot gitlink（取 feat pin `3e3a8a96`）、GopFramebuffer gitlink ×2（取 `90b64efc`；其中纯 pin bump 提交 b56f1ca 用 `--allow-empty` 保史，git 另自动 drop 一条纯 bump 29998480 内容已上游）、payload.py = env 行双保留合并（我方 `UNIXNU_DEVICE` 透传 + feat 的 seal-source-crypto-kernel 块；坑一枚 = python 替换结束索引截短，冲突标记行尾巴残留成语法错误，ast.parse 门当场拦截）。**新教训（当日付费）**：③"21H461 profile 取上游版"这条 rebase **零停点静默通过**——自动合并把我们 23 条死 fixup 史拼回了 feat 版之上（+213 行），是**净差验收**（`git diff feat..tip`）才抓出来的，清理提交 `c2bfe13` 取上游版修正。**规矩入档："取上游版"类冲突处置不能只看 rebase 停点，必须看最终净差**。净差终验 = 58 文件全工作区资产（docs/package/patches/profiles+scripts 三件），gitlink 19 全对齐 feat，check 4 ok。
- **③换配完成（设备仓 39a8ef7）**：board.toml `os_profile = "ipados-26.5.2-23F84"` + root_match→`546DFF4A`（profile 自足值）+ defconfig `OWNER_CACHE_VOLUME_UUID`→`7A23BAC7`（profile systemos_volume_uuid）+ check 身份断言→iPad17,3/23F84/j820ap/0x8142/12；check-m2381 4 ok 0 fail；`./unixnu status -D ../UniXNU-m2381` 全链解析通过 = **arch 层自动选 `profiles/arch/arm64e`** + SoC feed `43da0d56`（feat 的 xnu-12377 兼容 pin）+ 63 条 xnu-12377 patch series（native-pac-keys #20）+ 11 组件齐；`./unixnu defconfig m2381_defconfig` `.config` 落位零报错。**机制勘定 = arm64e/fileset/native-PAC 不进设备 defconfig**：profile `[os] arch="arm64e"` 选架构层，`profiles/arch/arm64e` 自带 common.config + 全套 series（native-pac-keys/arm64e-threaded-starts/fileset-unmapped-call/native-security-init/trustcache-runtime-layout/offline-owner 全家）；board 的三层 series 数组随 profile 自动换装。trustcache 8796.101.5 源缓存 json 两代同 tag 已复用预置进新树 `.cache/sources/`。
- **④IPSW 供给单（用户自下载，进行中）**：iPadOS **26.5.2 / build 23F84**，`iPad17,1,iPad17,2,iPad17,3,iPad17,4_26.5.2_23F84_Restore.ipsw`，URL 在 profile；**全 zip sha256 = `f7f55fa36097a0c8f8784eefcd9eabfce3a1c55ecd470f6885f4862a9446f9c3`**（sheng.md evidence 实测值）；喂法 = `./unixnu <步> --ipsw <本地路径或URL>`（或 UNIXNU_IPSW env；成员按 HTTP range 读取，缓存 `out/ipsw/23F84/`）。
- **预期管理（sheng.md arm64e-os 节实测档，2026-10-29 读）**：该节 status = **`planned`**——sheng 自身 **fileset 出件未通**（"no fileset prelink output exists yet, so the kernel has not booted"）、全世界此线尚无 boot 实证；23F84 KC 实为 xnu-12377.**122.8**（公开源 121.6 最近）；23F84 dyld cache 含 SME 代码而 SM8550 无 SME（run-time feature 选择假说未证实）；sheng_ipados26_defconfig@4ed86f6 **已被上游回收不可达**（"not our ref"，本地 UniXNU-sheng origin/main 仍 21H461 态）——参照物只剩上游通用面（我们已按其文档自洽行生）。
- **遗留（下会话开工单，按序）**：⑤ fetch→tree→kernel 第一遍——**GH_TOKEN 先导出**（本 shell 无，r83 403 坑；driverkit tag 12377.121.6 缓存无法复用必须真拉）→ `--ipsw` 喂 IPSW → 两次构建语义照旧；⑥ board `[payload] trust_cache_sha256` 三值换 23F84（**现仍挂 20H392 三值 = payload 步 fail-safe 拒装的门，不是雷**）+ verified-root 政策对重建（build-owner-policies-local.py，23F84 四元组）+ Data UUID derive（APSB 扫描器现成）；⑦ payload 前板 args 精简拍板（现挂 21H461 时代 983B 诊断队 vs sheng.md arm64e 基线 `-v serial=3 debug=0x14e kt_fixed_zones=120 -no_slto_panic`；cmdline 1000B 上限）。回退链不动：21H461/20H392 全组 + boot_b ec1d518b。

**预期管理更正（同日深夜，用户转 QQ 群情报 + fetch 证据）**：上文"全世界此线尚无 boot 实证"**作废**——上游真机已在跑深水区：feat 分支 10-29 当天提交 = "point at the lock, leaf-walk and 8-bit framebuffer changes" / "Install-and-boot-only Data store" / turnip GMEM 渲染，10-25 起有 green bar diagnosis + automated boot testing 文档；用户 QQ 群情报 = ipados27 已跑起来、上游表示可以开刷，与提交流完全吻合。**下载决定不变 = 26.5.2/23F84**：supported 表当前配对仍是 SM8550↔26.5.2（"ipados27"是分支名口语，非 27.0.1-24A446），两 profile 同 commit 7b02190 建立。文档面滞后（arm64e-os 节仍 planned）不矛盾。对 m2381 步骤零变化：开刷靠自己的三件套，仍按 ⑤⑥。

**下会话开工单（会话十二，2026-10-29 深夜定稿；用户 IPSW 下载中；上游已确认 iOS27 全部在 feat/ipados27-gpu-parity = 我们现基座，无需动作）。前置 = ①用户给 IPSW 本地路径 ②`export GH_TOKEN=<宿主凭据>`（无 token fetch 必 403，r83 实证）**：

- **步 0 基座核对（2 分钟）**：`git fetch upstream` 看 feat tip 是否又动（本单落定于 `6ad1297`）；我们基座 = e5f729d+1（差一条 AdrenoMetal bump，ADRENO=n 不消费，可选 `git rebase upstream/feat/ipados27-gpu-parity m2381-ios27` 收齐，非必需）。`sha256sum <IPSW>` 对拍 `f7f55fa3…f9c3`。
- **步 1 fetch**：`export UNIXNU_IPSW=<路径>`；`./unixnu fetch -D ../UniXNU-m2381`。预期 = 19 gitlink 迁 feat pin + xnu-12377.121.6/driverkit 12377/六 Apple 源 shallow-fetch + unixnu.lock 写出。坑 = GH_TOKEN（403）；driverkit 12377 缓存复用不了必须真拉。
- **步 2 tree**：`./unixnu tree -D ../UniXNU-m2381`。预期 = 63 条 arch series + os zzz-boot-progress + SoC feed 43da0d56 补丁全应用零 fail（已核实：主树根 patch entropy/iorpc-export 仅 20H392 冻结 series 引用，arm64e 车道惰性）。
- **步 3 kernel 第一遍**：`./unixnu kernel -D ../UniXNU-m2381`。两次构建语义照旧（archive-20 audit 需 prelink 候选）。门 = fileset 重绑通过（emit_prelink --fileset-reference 对 23F84 MH_FILESET 输入）。**预期主战场 = fileset 出件**（sheng 自身没跑通的第一关）；坑预注册：a) 11 组件 fork 编译面 vs xnu-12377 头漂移（改组件必须 submodule commit+gitlink bump 同轮，r78a 规矩）；b) kext 直跳内核由 fileset-unmapped-call stub 兜底，关注其 report 行计数。
- **步 4 政策对 + trust 三值（kernel 绿后）**：①IPSW 解包四件逐成员对拍 profile 针（kernelcache_sha256 `0f7d50ac…`）；②board `[payload] trust_cache_sha256` 三值换装（现挂 20H392 三值 = payload 步 fail-safe 门非雷）：StaticTrustCache sha + Cryptex sha + make-m2381-trust-bundle.py 重打包 bundle sha；③verified-root 政策对重建：build-owner-policies-local.py WINDOWS 表换 23F84 四元组（os 546DFF4A/b4e40913…、SystemOS 7A23BAC7/eadb838d…，profile 自足）→ 全 RSA PASS。
- **步 5 payload + args 拍板（用户参与）**：args 两案——**甲（推荐）= sheng arm64e 基线裸起** `-v serial=3 debug=0x14e kt_fixed_zones=120 -no_slto_panic` + 最小必备（native-ufs-root=1 / sheng-pmgr=2；充电栈 sheng-adsp/battery 本轮不背，变量最少）；乙 = 21H461 诊断队砍到 ≤1000B。payload 红线：`--unique-chip-id 3645ad55d47e77c1` + `--trust-cache/--data-ramdisk` 显式传（r85 坑）+ 不带 skip-setup。**userdata 必重建**（`./unixnu userdata`，26.5.2 root，Linux 可构建）。
- **步 6 首刷（拍板后）**：三件套 = boot_b `ec1d518b` 不动 + recovery_a 新 payload + userdata 新件；刷前 sha 门 + 读回对拍。判读门 = ①内核起（entropy 行/native-pac-keys 每 boot 每核键编程行/fileset 绑定计数）②arm64e 用户态推进（launchd/dyld，SME 选择假说的实机答案）③失败按 panic screen 文本定点——**注意 23F84 KC = arm64e stripped MH_FILESET 无 LC_SYMTAB，r88 的 LC_SYMTAB 符号化路线作废，要走 dyld cache/fileset 表路线**。回退链 = recovery_a/userdata 随时回 20H392 组（r88 c932a128）或 21H461 组。

预期里程碑 = 本会话拿到 26.5.2 全链三件套可刷状态。

**会话十二执行记录 + 会话十三交接单（2026-10-30 凌晨收尾，用户令暂停待上游 iOS 27 合入 main 更新）**：

- **已完成（六步单之④①②③-大半）**：IPSW 23F84 sha256 `f7f55fa3…f9c3` 对拍供给单逐字 ✓（10.44 GB，`research/ipsw/`）；步0 基座核对 = feat tip 又 +2（`87bf1f9` AdrenoMetal bump，ADRENO=n 不消费，未收）；步1 fetch 全绿（GH_TOKEN = `GH_CONFIG_DIR=/home/ri/.config/gh gh auth token` 从宿主 keyring 取，hosts.yml 本体无 token；git 通道免 403，仅 prepare-xnu-source.py 的 GitHub API 要 token）；步2 tree 63 series 零 fail；**步4 反而超前完成**（见下）。
- **主树提交 `fe9c792`**：kernel.py 的 `UNIXNU_SDK_VERSION`/`DARWIN_TRIPLE` 从硬编码 17.0 改为随 profile 派生（arch ← profiles/arch/*/profile.toml，ios_version ← board [os]）——组件档重放编译记录的 --target 必须与 profile 的 -m*-version-min 一致，上游 clang-24 对 override 以 -Werror 拒收。
- **设备仓提交 `f5ad4ad` + `9834800`**（细节见提交语）：sm8550 feed 本地化（`local = "soc/sm8550"`，唯一改动 = smp patch 只留 start.s Affinity1 hunk）；4 参 `__ptrauth` 兼容（Apple clang only 的 "authenticates-null-values" 串；设备层 patch 令 `_AUTH_NULL` 族在 Linux LLVM 下降级为无符号=功能安全方向）；三个部署版本自检泛化（0x110000 → UNIXNU_IOS_VERSION 派生）；**archives 单对齐 sheng arm64e 已证子集**（摘 storage-bindings=190槽 21H461 pin+arm64e AUTH vtable r_type11、verified-root stub 双件、VirtioMmioBlock+transport、mockfs、legacy-display=23F84 无 ABI fixture 上游仅 24A446、sheng-platform-power=PMGR 已随 fileset roots prelink）；defconfig 重塑 = sheng_ipados26 形态（CRYPTO_BOOTSTRAP="source"+KERNEL_CHAINED_FIXUPS+SOURCE_STORAGE=y，NativeAES/VirtioBlock/Mockfs/VerifiedRootProbe/LegacyDisplay/ShengPmgr=n，**OWNER_ROOT_AUDIT/TRUST=y 复位**=BootProgress 守卫要求+sheng 对齐）。
- **步4 超前完成（构建强拉提前：build-sheng-source-storage.py 要 verified-root-23F84 政策对）**：①AEA 双窗解密——本机无 macOS /usr/bin/aea，**装了 blacktop/ipsw CLI v3.1.739（/tmp/ipsw，纯 Go AEA 实现）**，`ipsw fw aea <f>.aea --key` 取键 + `--key-val` 解密；OS dmg sha `b4e40913…` 与 SystemOS `eadb838d…` 与 profile [root] 双针逐字命中，已按 ipsw.py 缓存 sidecar 格式摆进 `out/ipsw/23F84/*.dmg(+.json)`；②**userdata-root.img 13.02 GiB 出件**（`out/m2381/ipados-26.5.2-23F84/m2381/userdata/`，OS 窗 @0x100000 = 8753512448 B，SystemOS @0x209d00000 = 5224005632 B）；③**政策对双窗全 PASS**（build-owner-policies-local.py WINDOWS 表已换 23F84 窗——**坑重踩一次：offset 手算错 1 字节级，正确值 0x100000 / 0x209d00000=8754561024，勿再手算**；os 534272 块 + systemos 318848 块逐块 RSA；双 395B trusted-key 同锚）；政策对已摆 `tree/build/verified-root-23F84/{OS,SystemOS}/`。
- **步3 墙账本（本会话 12 面，全部已破，除最后一面）**：①feed smp patch hunk1 上下文消失→本地化 ②sheng-platform-power patch 与 os 层 zzz-boot-progress hunk 耦合→patch 留 archive 摘+CONFIG_XNU_SHENG_PMGR=n 编译掉调用点 ③4参 __ptrauth→设备层 fallback ④GH 403→keyring token ⑤firehose 编译吃陈旧 fakeroot 头→rm fakeroot 重装 ⑥组件档 target 17.0 vs 26.0 -Werror→DARWIN_TRIPLE 派生 ⑦archive-07 平台电力 340 槽 21H461 pin→摘除 ⑧storage-bindings 190 槽+arm64e AUTH r_type11→摘除 ⑨ShengUfs 头漂移→vendored 换 feat fab07fcc ⑩SourceRootPolicies 的 vr_open_header 缺→VerifiedRoot fork 换 feat ed368a8 ⑪telemetry Defs 对象 arm64 被 arm64e 链接整档丢弃（"ignoring file…built for iOS-arm64"）→TRIPLE 派生 ⑫未解 90→6→duplicate 三轮（Defs 门控+EXTRA_LD 摘重+PGO 12/9 参桩+virtio 计数）。
- **★最后一面墙（会话十三第一刀，修法已勘定）**：duplicate `boot_log_*`/`boot_diag_*`/`boot_log_export_put` ——BootLog.o 双载：libBootProgress.a（build-boot-progress.py:91 无条件编译 log_obj）× libShengUfs.a（source-storage 适配器 sources 表无条件含 BootLog.c）。**修法 = 主树 `tree/scripts/build-boot-progress.py` 在 XNU_SOURCE_STORAGE=1 时跳过 log_obj**（log_command 编译 + archive 成员表两处条件化；BootProgress.o 的 boot_log_tick 弱引用由 source-storage 档的 BootLog.o 兑现；build-report.json 的 log_sha256 条目同步条件化）。改完 `./unixnu defconfig m2381_defconfig && ./unixnu tree && ./unixnu kernel` 三连（改 defconfig 必重跑 defconfig=铁律；改设备脚本必重跑 tree 刷新副本）。
- **kernel 绿后余单（按序）**：①board.toml `[payload] trust_cache_sha256` 三值换 23F84（现挂 20H392 三值=payload fail-safe 门；kernel 步已按 profile [ipsw] fetch 清单取了 StaticTrustCache/Cryptex1,SystemTrustCache 成员，sha 之 + make-m2381-trust-bundle.py 重打包）；②args 拍板——**案甲（工作单推荐）已勘定参照 = sheng feat27 board.toml:52 实证单**，m2381 最小集 = `-v serial=3 debug=0x14e kt_fixed_zones=120 -no_slto_panic use_contiguous_hint=0 native-source-storage=1 native-ufs-root=1 noidle=1`（native-source-storage=1 是 source-storage 启动总闸=SourceStorage.cpp 门控必带；sheng-pmgr/offline-legacy-* 全不背=lane 已摘；noidle=1 防 r70 睡眠灭屏）；③`./unixnu payload`（红线：--unique-chip-id 3645ad55d47e77c1 + --trust-cache/--data-ramdisk 显式传 + -D 在子命令后）；④首刷三件套 = boot_b ec1d518b 不动 + recovery_a 新 payload + userdata 新件（已出），判读门 = native-pac-keys 每 boot 每核键编程行/fileset 绑定计数/native-source-storage=1 生效行。
- **★会话十三步0 = 上游更新（用户令：先同步再继续）**：`git fetch upstream` 看 feat/ipados27-gpu-parity 新 tip 与 **main（用户情报=上游在写文档合入 main）**；若动 = `git rebase upstream/feat/ipados27-gpu-parity m2381-ios27`（**净差验收铁律**：取上游版类冲突不能只看停点，`git diff feat..tip` 抓静默拼回）+ 子模块对齐 + 本单 12 面墙补丁逐面过 net-diff（尤其 kernel.py 双派生与设备层 ptrauth patch 是否被上游同题解法替代——若上游已修则取上游版删本地件）。
- **预期管理**：本车道 fileset 出件（emit_prelink --fileset-reference 吃 23F84 MH_FILESET donor）仍未到达——链接关过后它就是下一关（sheng 未公开通关证据，无 boot 实证参照；23F84 KC 无 LC_SYMTAB，符号化走 dyld cache/fileset 表）。回退链不动：21H461/20H392 全组 + boot_b ec1d518b + userdata 2c2aca3f。

**会话十三执行记录 + 会话十四交接单（2026-10-30 深夜，全链出件达成）**：

- **步0 上游更新**：fetch 后 feat 新 tip `f161cd2`（+3：AdrenoMetal×2 bump + 三连 bump opaque scanout + metal 前端 host check 脚本，全 GPU 侧不消费）；main +9 全 docs（device records 刷新，feat 未合 main）。rebase 238 提交**零停点**落 `3315cf8`，净差验收 57 文件全工作区资产，上游 drivers gitlink 对齐新 pin（Adreno740 90b52bf/AdrenoMetal 148698d/GopFramebuffer 7f7ae1e）。备份针 `m2381-ios27-pre-rebase-20261030`。
- **第12面墙破（BootLog.o 双载）**：`build-boot-progress.py` 在 XNU_SOURCE_STORAGE=1 时跳过 log_obj（编译+archive 成员+report 三处条件化）——kernel 步推进到 prelink。
- **fileset 出件关三墙全破（本会话主战场）**：
  ①**codeless 断言** = 设备 fork 的 emit_prelink 无 fileset 分支无条件走经典 plan()。**换装 vendored kits/XnuPrelink → 上游 feat pin 76452c6e**（fileset_match/emit/layout/shape/segmented+kc_image，8 测试全绿；21H16 arm64e 经典路 fork 退役入 git 史）+ board.toml `[kernel.prelink] extra_args=["--allow-missing"]`。**机制勘定：MH_FILESET 输入走 fileset_match 管线（--fileset-map/--fileset-reference/--allow-missing 任一切换），plan() 的 codeless 检查只属经典路**。
  ②**native __DATA 槽约束** = fileset_layout 把 native __DATA 钉在 collection `__DATA` root 起点保留（donor 内核数据位，23F84 洞=2142KiB，首个 retained kext=AppleARMPlatform@0xb4af780），我们的 native __DATA=3824KiB（BSS 大户：bp_* profiler 四件 2048KiB）。**裁 BootProgress.c bp_*：BpProfSamples 65536→4096、BpKernSamples 32768→4096**（省 1888KiB → native 1936KiB，余 206KiB≈5%）。洞后零字节为零=不可扩洞，裁码是唯一路；gScratch(ShengUfs 128KiB)功能件不动。
  ③**RELEASE 参照件产能**（--fileset-reference 的料）：`XNU_KERNEL_CONFIGS=RELEASE ./unixnu kernel --skip-prelink` 产出 `mach.release.bcm2837.unstripped.noctf`（17.6MB）→ 手动 stage 到 `tree/build/xnu/fileset-reference/` → board extra_args 换 `--fileset-reference <path> --allow-missing` → 重跑 kernel。**主树脚本三改**：build-xnu-4k-baseline.sh（config 选择+XNU_REFERENCE_BUILD=1+ubsan-minimal stub 编译钩子置顶于 crypto gate 之前；seal 路径按 config 派生）+ patches/xnu/reference-build-release-export-bypass.patch（**RELEASE-only all-kpi.exp 导出检查旁路**：IOUnifiedAddressTranslator/DART/llvm_profile 系预链 kext 符号，开源 xnu 树无源码，纯源码 RELEASE 独立内核永远缺；DEVELOPMENT 无导出检查故从未暴露）+ kernel.py（RELEASE pass 置零 m2381 取证链 owner 全家+BOOT_PROGRESS——offline-owner 补丁自带 DEVELOPMENT #error 守卫且 RELEASE 形态编不过，参照件不 boot 不发布；**compile-record 检查按 config 参数化**）。**bootstrap 链接失败=设计内可容忍**（kernel.py try/except：记录在即放行；主链接 force_load 档供体齐）。**巨型坑：host nm/llvm-nm 读 138MB Mach-O 档静默零输出**——曾据此误判"无定义体"绕远路，凡 nm 扫档必须先验输出总行数。
  - **参照件成效**：函数匹配 6807→**8293**（reference-order 5142+reference-symbol 401=名字转移主通道），data 559，未解 641 sites/378 targets（apfs 206 sites 为主）。
- **payload 步三墙**：①check_native_root 强索 sheng-pmgr=2+_sheng_pmgr_run（经典路前提）→ payload.py 加 prelinked_pmgr 参数（profile roots 含 com.apple.driver.ApplePMGR 即免检，sheng 实证 arg 行同形态）；②patch-xnu-entry-probe.py 符号解析吃 fileset "Symbols from…" 节头行→parser 跳非 hex 行；③panic screen 超洞 7 字节（0xa37>0xa30，fileset __TEXT_EXEC padding 收窄）→ **xnu-panic-screen.c 摘 frames walker 块**（-88B→2515B，保 pages/free 判读计数器；串口 backtrace 补位）；④**ESP 超 recovery_a 100MiB 分区**（stage 101MiB：fileset kernel.macho 85.2MiB）→ board.toml `forensic=false` 摘读者三件 15.3MiB。
- **出件终值**：kernel `9da51132`（带参照件）→ payload **`8a209d8b`**（96MiB，ESP 四样齐：kernel.macho 89,374,720B+patched 副本 sha 5c244659+trustcache 82,236B+scratch-data+initrd 行+ADT acba2928，卷余 9.3MB）；trust 三值 e70a385a/58960bbc/ebeaf7fb 已入 board pin；args=案甲 9 项 130B。
- **nura 读者内核需求（2026-10-30 晚拍板：nura 仓零接触，不发工单文档，需求由用户口头转告）**：需求规格留档 = 设备仓 `forensic/READER-KERNEL-REQUEST.md`——读者专用最小内核（去 GPU/DRM 全家，PE≤4MiB，保 EFI stub+UART+USB NCM+UFS+configfs，initramfs 外投契约同 build-forensic-reader.py），落成后摆 `.cache/forensic/` + board.toml `forensic` 回 true。
- **会话十四交接（首刷判读单）**：三件套 = **boot_b ec1d518b 不动 + recovery_a ← payload 8a209d8b（先 fastboot getvar partition-size:recovery_a 核 100MiB）+ userdata ← userdata-root 13.02GiB（out/m2381/ipados-26.5.2-23F84/m2381/userdata/）**；设备现态先核（FLASHING.md 单通道律+dd 读回才算证据）。**判读门（串口）**：①native-pac-keys 每 boot 每核键编程行（EnIA/EnIB/EnDA/EnDB）；②fileset 绑定计数（emit_prelink 报告的 named/mapped 数）；③native-source-storage=1 生效行 + source-storage 卷挂载；④`_unixnu_fileset_unmapped` stub 触发行（每条=一次未解调用命中，报站点不跳野指针——641 sites 的实验性余量是预期内背景噪声，非墙）；⑤panic screen（pages/free 行在，frames 行已摘）。符号化：23F84 无 LC_SYMTAB，走 dyld cache/fileset 表（r88 路线作废预注册仍有效）。回退链不动：21H461/20H392 全组 + boot_b ec1d518b + userdata 2c2aca3f + 备份针 m2381-ios27-pre-rebase-20261030。

**首刷卡（2026-10-30 交卡，会话十四按此判读）**：现态核对（current-slot=b + partition-size:recovery_a=100MiB + 自 r85/r88 轮 boot_b 未被动过——若存疑则补刷 boot_b ← 设备仓 `UniXNU-m2381/build/m2381/boot/boot.img`（ec1d518b，31,793,152 B；⚠主树 out/m2381/boot/boot.img=40f2a199 是 B2 旧件 1.66MB，勿混））。刷序 = 先 recovery_a ← payload 8a209d8b（100,663,296 B）→ 开机验 UEFI 菜单在（功能级 ESP 验证）→ 再 userdata ← 13.98 GB 件 bb5d22f4（15-30 分钟勿断）→ 开机串口判读。本轮无 dd 读回通道（读者被 ESP 空间挤出，nura 最小内核回归后恢复），证据 = 串口 + panic 屏。判读门 = STATE §8 会话十四交接五条。回退链 = recovery_a ← payload-r88-c932a128 + userdata ← 2c2aca3f 件（均已在库）。

**会话十四判读 + boot 件定谳（2026-10-30 深夜，首刷实况：UEFI 菜单活，选 first boot 立刻闪回菜单）**：

- **根因定谳 = FV 内 DarwinBoot 太旧，不是 boot_b 坏也不是内核/投递链**：ec1d518b 构建于 10-29 07:42（r81，20H392 时代），其 DarwinBoot pin=`3325e35`——`ParseKernel` 硬校验 `U32(Data+12)!=2 → EFI_UNSUPPORTED`（**只收 filetype 2 = MH_EXECUTE**）；26.5.2 fileset 内核 kernel.macho 实测 filetype=**12（MH_FILESET，cputype 0x100000c）**→ DarwinBoot 当场拒，Boot Manager 回菜单，与"选 first boot 立刻闪回"逐环节吻合。修复 pin=`3e3a8a9`（accept arm64e MH_EXECUTE 内核 + **filetype 12 在 Header==0 放行** + root policy sidecar；ESP 上 kernel.macho.crypto-hmac.json 即配它）。机制教训入档：**boot_b"冻结不动"纪律按车道绑定——换 OS 车道（内核形态/代际变）必须重估固件，DarwinBoot 的内核格式接受面跟着车道走**。
- **上游 uefi 跟进（用户拍板）**：rebase `0eb4514 → 20ac23d5`（upstream/sheng tip，5 笔 = 34c98806 ShengBootMemoryDxe 176 行换共享 UniXNU RAM disk 驱动【shengPkg 专属 fastboot-ramdisk 路径，m2381 不引用、零影响】+ docs×4）；overlay 重放 `9318b62e`（基座记载已同步改写），m2381 相关文件 rebase 零 diff。**uefi-m2381 push 需 force-with-lease，待拍板**。
- **新 boot 件出件**：FV 重编译（DarwinBoot.efi 87,040 B，10-30 03:27 新鲜于源码 mtime，增量缓存嫌疑排除）→ `package-m2381-boot.py` 打包 → 设备仓 `build/m2381/boot/boot.img` = **`115f2738`（31,793,152 B，与旧件同尺寸）**；ADSP 段 acf95fd5/adsp_dtb db6a6478 与旧件**逐字节相同**（FV 外零漂移），fd ce3ce88b→**68e5023c**（唯 FV 内容变 = DarwinBoot）。旧件备份 = 主树 `out/m2381/boot-b-ec1d518b-r81.img`（+json）。
- **补刷卡（2026-10-30 交卡）**：**只刷 boot_b ← 设备仓 `UniXNU-m2381/build/m2381/boot/boot.img`（115f2738）**，recovery_a 8a209d8b / userdata bb5d22f4 已刷在位不动；刷前现态核对照旧（current-slot=b）。判读门 = 会话十四交接五门不变（串口：native-pac-keys 行 / fileset 绑定计数 / native-source-storage 生效行 / `_unixnu_fileset_unmapped` stub 触发行=预期噪声 / panic screen）。回退 = boot_b ← `out/m2381/boot-b-ec1d518b-r81.img`。
- **读者选项丢失 = 预期内**（forensic=false 被 100MiB ESP 挤出，会话十三已档），恢复路径不变：nura 最小内核（4d67ff9 工单，PE≤4MiB）交付 → `.cache/forensic/` → board.toml `forensic=true` → payload 重建（现卷余 9.3MB，4MiB 内核+1.3MiB initramfs+0.14MiB dtb≈5.5MiB 可容）。

**会话十四二轮判读 + cpusub 门破（2026-10-30 深夜续，用户刷 boot_b 115f2738 后实测：仍秒弹回）**：

- **第二道门定谳 = com.apple.kernel 成员 cpusub 不等**：新 DarwinBoot(3e3a8a9) 对 filetype 12 放行后，`ParseKernel` 还要求 **`com.apple.kernel` 成员的 cpusubtype == 集合头 cpusubtype**（DarwinBoot.c:238，不等 → EFI_LOAD_ERROR）。实测：我们 kernel.macho 顶层 = `0xC0000002`（emit 从 donor 抄），成员 = `0x80000002`（**Linux lld 源建内核自带 CPU_SUBTYPE_ARM64E|0x80000000**）；donor 23F84 自身 top/member 一致（均 0xC0000002）。上游在 macOS 用 Apple ld64 建核天然产出 0xC0000002 故从未撞此门——**m2381 = 首个 Linux 建 fileset 上机的板**。
- **DarwinBoot 后续门全部本地预验 PASS**（免再烧刷机轮）：成员段覆盖 10/10、entry 与顶层相等、父段无重叠、trustcache（82,236B 双 cache）合法性、scratch-data.img=「DBRDISK1」packed 格式与 ADT PackedRAMDisk/KeepPacked 一致、窗口 = ADT darwinboot-memory-size 4864MiB（board owned-memory-size；若 AllocateWindow 失败屏上有大字 NO WINDOW 诊断=DiagDraw，拍照即可定位）。
- **修法**：主树 `scripts/patch-kernel-fileset-cpusub.py`（成员 cpusub 规范到顶层，原值钉 0x80000002，round-trip 自检，非 fileset 零操作）+ `payload.py` 在 kernel copyfile 后无条件调用（先于 probe/seal）；payload 重建 = **`38a2a8f9`**（96MiB；ESP 内核副本 `14bc069b`；源建 kernel 9da51132 不变；对 ESP 副本跑 ParseKernel 全模拟 = ALL PASS）。
- **补刷卡 v2（2026-10-30 交卡）**：**只刷 recovery_a ← payload `38a2a8f9`（100,663,296 B）**，boot_b `115f2738` 已在位不动、userdata 不动。开机预期：串口 `DARWINBOOT: loader v2 installed EL1` → `inputs validated; exiting boot services` → 绿条 → 内核五门照旧。回退 = recovery_a ← 8a209d8b（在库）。
- **备注**：上游两 pin（DarwinBoot 3e3a8a9 / XnuPrelink 76452c6e）与我们所用一致、无更新修复；cpusub 规范化是可上游化的通用修法（LLVM 建核 vs Apple ld64 的 cpusub 差异），后续可提上游。

**会话十四三轮判读：内核起飞 + 第一堵原生 PAC 墙定谳（2026-10-30 深夜续三，本轮=纯分析不修，修复=下会话）**：

- **三件连过**：cpusub 门(38a2a8f9)→ stage 3 复位探针点火实证(品红后 ~5s 自复位 = MMU 开启序列过，4K+arm64e 页表启用段排除)→ 诊断页轮(49f1478d,boot-progress=2+panic 屏复位探针撤收)炸出 panic 屏。**上游同款剧本验证**：stage 色条/定时复位/panic 屏全是上游 feat 自己的 patch-xnu-entry-probe.py+green-bar diagnosis 指南(unixnu.com/guide/debugging)，boot-progress=1/2 诊断页=上游盲区主力工具(120×48 阶段史，卡住冻结可拍)。
- **内核抵达深度（诊断页实录）**：pages=0x125b8e/free=0x12554e（≈4.7GiB 窗全活）→ 阶段史 clock_service_create → sched_startup → thread_daemon_init → thread_call → Thread_bind → mapping_adjust → clock_service_create(2nd)；OSLog 流验证/Firehose 16 chunks/Log queues 72 槽全配置完；末行 console=`[hmac algorithm] tried entry init`。
- **墙 = `PAC failure from kernel with IA key while branching to x8`**（ESR 0x07200000，far=0，cpsr 0x624003c4）。符号化：**pc = `_source_corecrypto14_bootstrap`+0x84 `blraaz x8`**（IA 键零上下文鉴权）；x8 ← `ldur x8,[x19,#0xb4]`，x19/x0=0xfffffff00b5f9a90（BSS 对象，BSS 初始化域）；x19 来自 +0x40/+0x4c 两次调 `_source_corecrypto14_original`（单例，cmp 相等过）→ casal 锁 `_source_corecrypto14_original_state` → 调算法入口指针；调用场景= hmac 算法 entry init。**x8 地址部分=0xfffffff00a72f938（__TEXT_EXEC 符号洞），头四字节 0xd503245f=PACIASP=真函数入口形态**（非 reporting-stub 形态；未在 emit 档案中登记）。x20=块描述符域、x24=`__startup_EARLY_BOOT_entry_source_pthread_bootstrap_23+0x18`（启动集合邻域）。
- **已排除**：①per-core 键分歧——native-pac-keys.patch 实读：首核 CNTPCT+splitmix64 派生一次、MMU off 写 `native_pac_keys` 全局，后续核一律加载存储值（含热复位），全核同键；②x8=登记在案的 fileset unmapped stub（emit 档案零命中）。
- **候选（下会话 RE 判别）**：α=签名者/鉴权者**上下文失配**（写侧带多样性、读侧 blraaz 零上下文——注意 linux-clang-ptrauth-null-fallback.patch 的降级域是否覆盖 corecrypto 注记）；β=**fixup 链重签覆盖缺口**（源建内核 __ptrauth 全局指针走 lld auth_rebase→内核早期重签；该指针若在 emit 搬运中丢了链条目，残留 lld 占位/旧签名）；γ=签名发生在不同键状态相位。判别路径：①xref 找写 [x19+0xb4] 的指令确定签名形态与预期上下文；②用 emit 的 orig↔native 映射解 0xa72f938 真身（kernel.json/kernel.map.json）；③核对 lld auth_rebase 在 fileset emit 后的存续；④若需 A/B：固定键探针（键派生改常量）可一步区分键态相位 vs 上下文失配。
- **nura 读者内核交付（用户转交交付单，文件未落本机）**：vmlinuz-unixnu-reader-7.3.0-20261030 = 3,977,728B（3.79MiB≤4MiB，sha256 1512301a4d77d090a3b245f912f8fcd901e8cc905a47bf2416a10683694d42ff）+ 专用 .config（f7142f19…a506e05）+ System.map（无 kallsyms 固定地址，离线符号化用）。预算对账 85.2+3.79+2.4≈92<100 ✓（ESP 余 9.3MB 容 3.79+1.3+0.14≈5.2 ✓）。契约：PE32+ EFI AArch64 zboot+zstd（LoadImage+LoadFile2 流程原样）、CONFIG_CMDLINE 与 v71 逐字节同、initramfs 契约不变（busybox 不动）；身份自检 `uname -r`=7.3.0-unixnu-reader。行为差异：无显示驱动（GOP 残留 fb 上 earlycon+fbcon）、USB gadget-only（configfs 绑 UDC 即通，无 host）、无模块、无 RTC（时间戳 1970 预期）、无 KASLR（配 System.map 好调）。验收=payload 菜单 reader 项→UART shell→NCM 起→dd recovery_a 前 96MiB 对拍；盯「Initramfs unpacking」不回归。翻车回带 UART 全文（dwc3/ufs/eusb2 probe 行）；源码=linux-mobile-ports meizu20-t4b@bfc79f41，配方 nura docs/UNIXNU-READER-KERNEL.md。
- **下会话开工单**：①nura 三件落本机验 sha → 摆 `.cache/forensic/`（forensic-kernel.efi 命名按 build-forensic-reader.py 契约）→ board `forensic=true`+`sheng-log=1` → payload 重建 → 读者项验收（此后每轮自带 dd 读回+SHLOG 窗）；②PAC 墙 RE 四步（上述判别路径），注意 seal-source-crypto-kernel 与该指针的相互作用；③探针轮配置（panic_screen/entry_probe_reset_delay）待 PAC 修复轮一并收队；④回退链新增：payload 49f1478d（诊断页轮）、94f67199/5209bb33（复位探针轮）、38a2a8f9（cpusub 轮）均入 out/m2381/ipados-26.5.2-23F84/m2381/payload/ 历史（payload.json 只存现役，历史 sha 在本档）。

**会话十五判读：读者通道交付轮全绿 + FV 内存图地雷拆除（2026-10-30 凌晨续，用户三拍板：分家/用 v71/留菜单）**：

- **分家定案（用户拍板）**：reader 不再 riding iOS payload——专用读者 ESP（设备仓 `forensic/build-forensic-esp.py` 出件）按需刷 recovery_a，取证轮/iOS 轮各刷各件；`fastboot boot` 不可用（nura 已实测 Booting FAILED，ABL-FASTBOOT-TABLE 卡）。board.toml 回 `forensic=false`+`size_mib=96`（iOS payload 下轮重建生效）；`sheng-log=1` 留在 kernel_args（下轮起每轮 boot 留 SHLOG 窗，读者 ESP 对刷即可采）。**reader 内核 = v71 完整版（4aad92b3，m2381 实证）**，nura 最小版（1512301a）在案未上岗；**菜单保留**（ESP toml default=forensic/timeout=3，Restart/Power off 常驻，reader 退出回菜单）。
- **最小读者内核首崩根因定谳 = FV 内存图 bug，非内核**（全屏崩溃照 + 反汇编闭环）：MemoryProtectionExceptionHandler dump（肇事镜像 [0]=UniXNU.pdb）→ ELR = reader PE+0xAFC0 `sturb w15,[x9,#-5]`，x9−5 == FAR `0x82700020` 逐字节合，x6/x7/x8/x9 四路等距 0x3AE 游标 = zstd 字面量解码写输出 → **同步外部中止（DFSC 0x10）= 写 XPU 围栏** → 围栏实名 = `tz_stat_mem@0x82700000`（stock FDT no-map，Meizu 实机反编译）→ 投递者 = **RamManagerDxe**（SiliciumPkg）：把 MemoryMapLib 洞区 [0x81F00000,0x8A800000) 整段 `MapMemoryRegion(EfiConventionalMemory)`（洞区无描述符 → ParseMemoryRange 当空闲段收编），无 KASLR zboot 的 `efi_random_alloc(seed=0)` 低址优先把 9.8MiB 解压缓冲拍进洞区首段。v71 存活 = KASLR 随机地址运气；**iOS 车道同雷**（任何 efi 分配可落洞区）。
- **修复（uefi-m2381 MemoryMapLib）**：三块 no-map 区入表（ADSP MHI 0x81F00000+0x20000 / Global Sync 0x82600000+0x100000 / TZ Stat 0x82700000+0x100000，全 [DTB] 值）→ RamManager 减法自动排除，洞区余段 0x81F20000-0x82600000(6.9MB<16MB 被弃) 与 0x82800000-0x8A800000(128MB 净土) 照常。**boot_b 新件 `d0200a6b`**（31,793,152B，ADSP 段逐字节水，现役 115f2738 备份 out/m2381/boot-b-115f2738-rollback.img）。FV 菜单/启动照常 = 修复零回归。
- **工具流澄清（勿再钻错工具）**：boot_b 打包 = **设备仓 `scripts/package-m2381-boot.py`**（v4 头 + 5 字节 ramdisk 占位 + M2381FW ADSP 段；shim 契约 `image_size==FD_SIZE` 硬校验——overlay BootShim.S 是 nura 实证契约**非遗漏**，按上游 1c036336 改 accounting 会被它正确拒绝，已回滚）；`./unixnu bootimg` = sheng 形状，m2381 勿用。
- **32MiB FAT32 陷阱（首刷实拍入册 FLASHING §7）**：数据簇 64,480 < FAT32 下限 65,525 → `mformat -F` 硬造非法卷无告警 → EDK2 Fat 驱动拒载整卷 → 菜单跌 "built-in configuration / no boot entry found"（同时解答 "Return to firmware→failed to find an operating system" 与 Tools 秒回的伴生现象）。64 MiB 修复。
- **读者 ESP 首验全绿（2026-10-30）**：`reader-esp.img` d3e6d468（64MiB）刷 recovery_a → 菜单 3 秒自动进 reader → v71 内核起（probe 报 "m2381 forensic log reader 7.3.0 / usb: acm on a600000.usb"，SHLOG 窗 magic=SHLOG001，userdata=sda22）→ **NCM 51s 立**（1d6b:0104）→ **dd 读回对拍双绿**（recovery_a 前 64MiB = d3e6d468 ✓、boot_b 前 31,793,152B = d0200a6b ✓，读者通道即刷写取证仪器）→ **fetchchunk 15/15 块 31s SHA MATCH** → read-m2381-ufs-log.py 解析 337 槽旧 boot（r55e 代遗产，header 契约守住，residue 正确截断）。
- **下会话开工单**：①iOS payload 重建（96MiB/forensic=false/sheng-log=1 已配齐）+ 刷 recovery_a 回 PAC 车道 → PAC 墙 RE 四步（会话十四三轮档判别路径原样，注意 seal-source-crypto-kernel 相互作用）；②取证使命流程 = 刷读者 ESP → 采证（probe/dd/fetchchunk）→ 刷 iOS payload 回，各轮 dd 读回照旧；③回退链新增：boot_b d0200a6b、recovery_a reader-esp d3e6d468、payload 0f1013f4（合装轮，已不在位）；④最小读者内核上岗路径 = FV 修复已就位，可单独 A/B（v71 仍为现役 reader）；⑤探针轮配置（panic_screen/entry_probe_reset_delay）仍待 PAC 修复轮收队。

**会话十五补档（2026-10-30 收队）**：iOS payload **`58676a6c` 已出件已刷**（96MiB/forensic=false/`sheng-log=1` 在 args；存档 out/m2381/payload-58676a6c.img；recovery_a 现役=此件）。首 boot **PAC 墙逐字复现**（pc 0xffffff00a98a2dc / x8 目标 0xa72f938 / pages=0x125b8e 与会话十四三轮全同；lr/x8 仅 PAC tag 域随 boot 键派生不同）——**FV 内存图修复对内核路径零扰动，墙确定性**；证据=屏摄 IMG_20261030_061956.jpg（用户拍板以屏摄为准）。SHLOG 窗采集顺延下会话（墙点即冻结，写入时长存疑，fetch 前不可再 boot xnu 以免窗口滚动）。**设备收队现态：boot_b=d0200a6b（修复版 FV）/ recovery_a=58676a6c / userdata 未动；下会话刷卡先照此核对。判读=下会话（RE 四步判别路径在会话十四三轮档）。**

**★会话十六 = PAC 墙根因定谳 + 修法落件（2026-10-30 深夜，屏摄 IMG_20261030_061956.jpg 判读 + RE 四步全闭环；主树 patches/xnu-12377/source-corecrypto14-bootstrap.patch 已重生成，kernel 重建后台进行）**：

- **RE 四步定谳（全部实证，A/B 探针免了）**：
  - **①签名/鉴权形态**：pc = `_source_corecrypto14_bootstrap+0x84` = `blraaz x8`（**IA 键、零修饰符**；capstone 全内核普查 = 3657×blraaz + 304×braaz vs 仅 7×blr——**native 编译约定 = 一切未注解函数指针 (IA,0)**，llvm 23（Homebrew 23.1.1）`-arch arm64e`/`--target=arm64e` 默认行为，独立 .c 实测复现）。x8 ← `ldur x8,[x19,#0xb4]`（kmod_info pack(4) 布局 start@0xb4 非对齐所以 ldur）= **donor `com.apple.kec.corecrypto` kext 的 kmod_info.start**；x0/x19=0xfffffff00b5f9a90 = donor kext 数据区（collection __DATA 后段 0xb47c000+0x17DA90）。机制 = 上游 `source-corecrypto14-bootstrap.patch`：扫 fileset 找 donor corecrypto 的 kmod_info → `module->start()` 起 donor 唯一 provider（防双 provider 致 PRNG 上下文未初始化）；x24=`__startup_EARLY_BOOT_entry_source_pthread_bootstrap_23+0x18` = startup 游标（pthread 姊妹补丁直调 native `pthread_start`，非同族）。
  - **②0xa72f938 真身**：kernel.json `segment_layout`（native→collection 搬运映射）定位 = **donor __TEXT_EXEC 内 donor corecrypto kext 的 start 函数**（native __TEXT_EXEC 搬至 0xa73c000+，目标在其下 0xC6C8 = donor 原代码；非 reporting-stub 非 native；入口首字 0xd503245f = **BTI c**（会话十四记 PACIASP 有误，"真函数入口"结论不变））。
  - **③auth_rebase 存续 = 存续且已被处理**：槽在 __DATA 页 216 链上（144 节点），文件槽值（我件与 donor 23F84 **逐字节相同**）= 0x8010e4fe0372b938 = auth_rebase 链条目：isAuth=1 / **key=0(IA)** / addrDiv=0 / **diversity=0xE4FE** / cacheLevel=0 / target=0x372B938 → base(0xfffffff007004000)+target = **0xA72F938 与 x8 低位逐位合**；运行值≠文件值（低位已是终值地址）→ `kernel_collection_slide`（dyld_kernel_fixups.h；HAS_APPLE_PAC=1 由 native-pac-keys.patch 定于 BCM2837.h）已按链条目 **(IA, 0xE4FE)** 重签。emit 对账：kext_chain_sites_kept=82701（含本槽）/ kext_pointers_rebound=47166（donor kext 数据区 19 个链页被重绑为终值 = 非丢失；donor 内核数据区链页随 donor 内核本体淘汰 = 设计内）。
  - **④根因 = α 上下文失配的精确形态：同键（IA）、不同 diversity（0xE4FE ≠ 0）**。写侧 boot fixup 按 donor 链条目签 (IA, 0xE4FE)；读侧 native 默认 blraaz (IA, 0) → AUTIA 恒败 → trap。**β（fixup 链丢失/未处理）证伪**（槽被正确处理）；**γ（键态相位）证伪**（键同侧、native-pac-keys 每 boot 一次编程早于 arm_init 的 fixup pass）。会话十四三轮档"x8 高位=裸链条目"解读作废（低位=已处理终值地址、高位=真签名 tag）。
- **修复（已落件）**：主树 `patches/xnu-12377/source-corecrypto14-bootstrap.patch` 重生成——`source_corecrypto14_start_once` 内 `module->start(module,data)` 改为经 `typedef kern_return_t (* __ptrauth(0, 0, 0xe4fe) …)(kmod_info_t *, void *)` **注解 lvalue** 调用（`#if __has_feature(ptrauth_calls)` 守卫，非 ptrauth 构建回退原样）。实测该写法 clang 23 产出 `mov x16,#0xe4fe; braa/blraa` 与 donor 签名约定精确对齐；**直 cast 到 qualified type 被工具链拒**（"cannot cast to __ptrauth-qualified type"）必须走 lvalue。流程 = 旧 patch 应用到 pristine feed 副本 → 改结果 → 重 diff（hunk 头 -502,3 +502,141 自动正确）→ 对 pristine dry-run PASS。0xE4FE 为 23F84 donor 链条目原值（24A446 同族预期同值）。
- **seal-source-crypto-kernel 相互作用 = 无**（seal 只 prepatch __text static_if 位点 + 重算 __TEXT_EXEC/__text HMAC，不碰 __DATA 链与 donor 区；bootstrap 改动自动进 seal 重算范围）。emit `component_hmacs` 已含 com.apple.kec.corecrypto = donor 组件 POST HMAC 重封由 emit 预接（donor 自身 POST 预期通过）。
- **重建/判读单（已全部完成，主树 commit `481d33e` 未推送）**：kernel 重建绿（首版 cast 写法被 clang 拒——**cast 到 qualified 类型本体非法，必须 cast 到"指向 qualified 类型的指针"再解引**，kernel.log `cannot cast to __ptrauth-qualified type`——修正后重跑）→ **kernel `a36d5103`**（新 unstripped 反汇编实证 bootstrap+0x84 = `mov x17,#0xe4fe; blraa x8,x17`，旧 `blraaz x8` 已消失）→ **payload `07080eb4`**（96MiB；ESP 内核副本 `bfa4f979` sha 对针 + 修复字节在位；cpusub/entry-probe(--panic-screen --no-reset)/seal(args 全队)/dtb 全链跑通；平铺件 `out/m2381/payload-pacfix-07080eb4.img`）。**刷写卡 = recovery_a ← 07080eb4（100,663,296 B，刷前 sha 门 + 读回对拍），boot_b=d0200a6b 与 userdata 不动**。判读门：①原 PAC panic 消失②阶段史推进过 clock_service_create/SYSCTL 段 startup（bootstrap 成功无打印）③**下一墙候选预注册**：donor corecrypto start 起来后 crypto KPI 表（g_crypto_funcs 一族）donor 写侧 Apple 约定 vs native 读侧 (IA,0) 的边界同族风险；emit 生成的 native→donor authenticated thunk（0xfffffff007563df0 `blraaz x0` 一族）同 conventions 面。
**★会话十七 = stub 桩首点定谳 + body-hash 配对通道 + 锁三件套 fixture（2026-10-30 深夜续，用户刷 07080eb4 后屏摄 IMG_20261030_074633.jpg；精简流程）**：

- **PAC 墙修复实机生效实锤**：panic 形态全变——旧 PAC panic 消失，donor corecrypto start 已跑起来，死在**新墙 = `_unixnu_fileset_unmapped` 桩首点**：`fileset: original component called an unmapped kernel function from 0xfffffff00a709f00 (x0 0xfffffff902f5095e x1 0x5c)`。桩语义（fileset-unmapped-call.patch）= emit 把 donor 对未配对内核函数的 bl 重写到单桩，报告返回地址+前两参。
- **首点定谳 = memset**：donor 原件 0xa709efc 的原始 bl → **0xa76ce30** = donor 内核 memset（体形=广播字节填充；x1=0x5c=填充字节）。我们 native `_memset/_secure_memset`（同地址双名）**前 0x94 字节逐字节相同**但函数尾部填充跨构建不同 → 边界哈希/shape digest 全不匹配 → matcher 十余通道全漏。全表 = **839 目标/13653 桩点**（top: 0xae40858×1679 点/25 组件、0xa7f4128×1182、0xa7f3548×1011——全是 donor 内核函数被 native 覆盖后未配对者）。
- **修法①（设备仓 kits/XnuPrelink/fileset_match.py，commit 8f85136）= `anchor_body_hash` 通道**：donor 函数体截断到第一个终止 ret 后在 native 文本做原样搜索，唯一命中且落在 native 函数起点才配（ret 后填充是构建差异；字节相同=行为相同；带镜像相对引用的函数天然不会假配）。v1 体哈希版因 end 边界含填充失效；v3 ret 截断版生效（memset+0xa76c000 自动配对）。放宽"多副本任选"会破坏 reference-order 级联（8302→8262），已回退唯一性判据。
- **修法②（fixture）= 锁三件套**：0xa7f3548=`lck_spin_lock`（CAS 0→cpu号@+8，cpu号=tpidr_el1+0x4f0）、0xa7f3fe4=`lck_spin_try_lock`、0xa7f4128=`lck_spin_unlock`——身份判据=FAST 路径 CAS 形态+与上游已钉的 `_lck_rw_unlock_shared`（0xa7f62bc）同区；kext 把 lck 当不透明对象，配到我们实现后布局自洽。fixture 落 `fixtures/fileset-23F84-symbols.json`（108→111 条）。
- **出件**：kernel **c35cf1f6** → payload **fed3dd74**（96MiB，ESP 副本 83d55ef4；PAC 修复字节在位；corecrypto 桩 12→7、全场 13653→12682 点）。存档 `out/m2381/payload-stubfix-fed3dd74.img`；设备仓 commit `8f85136`。
- **corecrypto 残余 7 个（下轮判读，靠下次 panic 的运行时参数点名）**：0xa7b50f8（全局∈{3,4}检查）/0xa7f3364（tagged 分配）/0xa938124/0xa96a0c4（20 点！全局 0xb2d8008+8 指针装入 [x0]，疑 OSObject 基类构造器存 metaClass，无写者未定名）/0xacf9870（tag 0x6827）/0xae96de0（claim-once 全局 0x7d61af0）/0xb007294。识别工具 = `temp/pac-wall/stubid.py`（site→donor 原目标+体反汇编+native 对照）。
- **上游检查（用户问）**：feat 强推重写 f161cd2→9bfc45a，净新增=纯工具脚本（build-fileset-release-reference/bundle-ipsw-trust/trust-append-module/turnip-kmd-patch——把我们手搓的 RELEASE 参照件流程正名化），**无本墙修复**；rebase 收队项未做。

- **开放项**：①0xE4FE 的字符串原型未破（BSD16 穷举常见命名串未中；硬编码即可，链条目本身是权威）②native 侧 (IA,0) 默认的编译开关源头未定位（makedefs 全树无 fptrauth 旗标 = llvm 23 arm64e target 默认；实测确认）——上游 sheng 同款构建必撞同墙，本修法可上游化。

**★会话十八 = corecrypto 残余 7 桩全数定名并出件（2026-10-30 上午，用户刷 fed3dd74 后屏摄 IMG_20261030_090251.jpg）**：

- **新墙判读**：PAC 墙与 memset 桩均未再现，首点前移到 `from 0xfffffff00a6d2848 (x0 0xfffffffae0678c20 x1 0xfffffffae0678b60)`——x0=内核栈 `sp+0x20` 上的 out 参数（调用点前一条 `add x0, sp, #0x20`），site-4 = `bl 0xa96a0c4`，正是会话十七残余表第一项。
- **7 个身份全部定名**（判据 = native 侧同形函数 + 符号表唯一，配 `build/xnu/fileset-reference/` RELEASE 参照件与 `temp/pac-wall/stubid.py`）：
  - 0xa96a0c4 → `_clock_timebase_info`（5 指令 getter，`*x0 = gTimebaseInfo`；与 native 仅 adrp/add 常量不同 → body-hash/shape 两通道天然漏配）
  - 0xa7b50f8 → `_panic_active`（`(panic_state-3)<2` 的 cset lo；corecrypto 2 点）
  - 0xae96de0 → `_register_crypto_functions`（claim-once：槽已设返 -1，否则存参 + 置 flag 字节）
  - 0xb007294 → `_register_and_init_prng`（`entropy_init(64, entropyseed)` → prng_ctx 存 → cckprng_funcs 逐槽拷贝，ptrauth 判别子 0x6961/0xf60a/0x5d01/0x4d5f/0xedeb/0x3051 与 native 逐槽相同 → 4×cc_clear；native 多 inline boot_cpu/prng_ready assert）
  - 0xa938124 → `_read_random`（refresh 0x4d5f + generate 0xedeb、0x100 分块 + cpu_number；native 走函数调用未内联 tpidr_el1）
  - 0xa7f3364 → `_lck_mtx_alloc_init`（kalloc + tag `0x22000000|flags` + 锁组引用；donor 内联 lck_grp_reference，参照件中位于 `_lck_mtx_init` 前一位且同形）
  - 0xacf9870 → `_sysctl_handle_string`（strlen+1 → oldfunc → newfunc 截断写；与参照件逐指令同，差异仅 Apple 串判别子 0x6827 与 PAC 硬化 store；donor 序夹在 handle_long/handle_opaque 之间。会话十七记其"0 点"= 非直接 bl 路径，本表按 stub 目标身份收编）
- **落件**：设备仓 `3a7c6ef`（fixtures 111→118，每条钉 donor 字节 sha + 证据行）；kernel **171bee9d** → payload **`02d7aa05`**（96MiB，ESP 内核副本 55e07c55；trust `ebeaf7fb` + data seed `d04a0153` + ADT `8bfa8cad` + panic_screen；PAC 修复字节在位实证 = `source_corecrypto14_bootstrap+0x84` `mov x17,#0xe4fe; blraa x8,x17`）。存档 `out/m2381/payload-stubfix2-02d7aa05.img`。**构建坑复现（r85 同款）**：`--trust-cache`/`--data-ramdisk` 必须显式传——首轮裸 `./unixnu payload` 出的 `50ea4195` 缺 ESP trustcache+initrd（已删，勿用）；红线件 = `out/m2381/ipados-26.5.2-23F84/m2381/original-trust-23F84.bundle` + `out/m2381/ci-artifacts/verified-root-grow/owner-data-fixture/scratch-data.img`。
- **刷前门（全绿）**：prelink reviewed 111→118、全函数 8294→8301、stubbed 1674→1659；**stubbed 表里 corecrypto 条目消失**（原 7 目标全绑定）；出件内核 7 个 call site 逐点反汇编 = bl 指向配套 native 函数（布局偏移 0x346c000 自洽，ESP 副本复核同）。
- **残余（观察项）**：corecrypto `missing` 表剩 1 条数据引用 `0xfffffff007ca3910`（非函数桩）。
- **刷写卡**：只刷 recovery_a ← `payload-stubfix2-02d7aa05.img`（100,663,296B，刷前 sha 门 + 读回对拍），boot_b=d0200a6b / userdata 不动。判读门：①本 panic 消失②corecrypto start 继续（若新墙仍在 corecrypto = 间接调用/数据链路径，stubid+body/reference 对拍续办；换组件 = 新墙照流程）③pages/free 行仍在。

**★会话十八续 = 新墙定谳（corecrypto 静态 sysctl 的 handler 鉴权）+ GPU 接装盘点（2026-10-30 上午，用户刷 02d7aa05 后屏摄 IMG_20261030_092650.jpg）**：

- **判读**：桩墙与 PAC 启动墙均未再现，corecrypto 继续起，撞出新类型墙——`PAC failure from kernel with IA key while authing x16 at pc 0xfffffff00ad17e48`（x16=0x7c9ac0700ad18a9c，x9/x17 为字段地址+哈希判别子）。**pc 归属 = native `_sysctl_register_oid_locked+0x1e8`**（emitted→build 偏移 0x346c000 反查）。对象 = `0xffffffff00b6000d0` 处的 `sysctl_oid`，在 **corecrypto 的 __DATA（0xb5f9a90..0xb6058f0）** = donor kext 自己的静态 sysctl；槽 `oid+0x30` 的 donor 链条目 = `(auth=1, key=IA, diversity=0xE2E, addrDiv=0)`，oid_arg1（+0x18）参与判别子哈希。
- **根因（α 家族新形态）**：源码 `ptrauth_auth_and_resign(handler, IA, ptrauth_function_pointer_type_discriminator(typeof), IA, blend(&field, os_hash_kernel_pointer(oid_arg1)))`——旧鉴权用**编译期类型判别子**。Apple 构建对该类型签 0xE2E，我们的 clang 产出 0（=会话十六开放项 "native (IA,0) 默认" 同源），出件按 donor 链条目原样重签 → AUTIZA 恒败。
- **修法（已落件）**：主树 `patches/xnu-12377/sysctl-handler-donor-resign.patch`（进 `profiles/arch/arm64e/patches/xnu/series`）——register 路径改 `ptrauth_strip`（xpaci，不验证）后按本内核判别子重签；unregister/dispatch 用本内核签的判别子，不动。实证 = kernel `8a493864` 反汇编 `ldr x10,[x9,#0x30]! / xpaci x10 / …hash… / pacia x10,x9`，旧 `autiza`+brk 消失；ESP 副本 `1ea8d3c0` 同。
- **出件**：kernel **8a493864** → payload **`a62f5e53`**（96MiB，trust ebeaf7fb + data seed d04a0153，panic_screen 在）。存档 `out/m2381/payload-sysctlfix-a62f5e53.img`；刷写卡 = 只刷 recovery_a ← 此件，boot_b d0200a6b / userdata 不动。
- **GPU 接装盘点（用户令"和上游一样加 GPU"，同 SoC）**：①6 子模块已在 feat pin（Adreno740/AdrenoMetal/AdrenoFirmware/AdrenoPM4/AirNir/ArmSmmu4K，无需 bump）②三件固件按 pin 拉齐（`scripts/fetch-adreno-inputs.py`；**无 token 走 api.github.com 会 403 限流，本轮用 keyring token 补拉**）：a740_sqe.fw 75,380 / gmu_gen70200.bin 66,844 / sm8550/sheng/a740_zap.mbn 12,088，sha256 全对，落 `.cache/firmware/sheng-gpu/` ③`UniXNU-sm8550-common` ≡ 设备仓 `soc/sm8550`（kits/Sm8550Platform 已有 ShengGmu/ShengGmuCommandDb/ShengGpuSmmu）④**唯一缺口 = `ShengGmuVotesSource.h`**（板级 OPP/BCM/带宽表）；消费侧 `ShengGmuVotes.c` **已在 `reference/unixnu-sheng/package/`**，只差数据表，上游公开仓均无该头（gpu.md：kept out of git）⑤上游配方 `build-adreno740-kits.py` 用 `xcrun`（macOS），Linux 需改 toolchain clang/ar ⑥DTS 有 gpu/gmu/gpu_smmu（disabled）但无 OPP 表；sheng 侧只加 `&gpu/&gpu_smmu status=okay` ⑦Metal staging（add-gpu-plugin-data.py --publish）依赖 macOS 镜像工具 → Linux 先做到内核侧 GPU attach/Turnip。
- **风险预注册**：sheng 的 `a740_zap.mbn` 是 OEM 签名件，m2381 的 PIL 鉴权可能拒；兜底 = 读者内核从本机 firmware 分区取自家 zap（dd 通道已有），勿盲刷。
- **★nura 侧供给（2026-10-30，用户问"缺的能从 nura 拿吗"；答案=全齐）**：①**本机固件**在 `~/work/meizu20/meizu20-m1/vendor-dig/vendor-root/firmware/`（另有 `meizu20_linux/vendor-fw-harvest/mainline/lib/firmware/qcom/sm8550/` 同一份）：`a740_zap.mbn` sha `3cc594a0…`（12,088B）、`a740_sqe.fw` sha `2b49a172…`（74,804B）、`gmu_gen70200.bin` sha `1a2a419c…`（66,844B，**与 sheng 件逐字节相同**）。**实锤：zap/sqe 与 sheng 件不同（sheng zap `4fd56fef…`、sqe 75,380B）→ 之前"同 SoC 直接吃 sheng 固件"会翻车**。已按 m2381 pin 落 `.cache/firmware/m2381-gpu/`（qcom/{a740_sqe.fw,gmu_gen70200.bin}, qcom/sm8550/a740_zap.mbn）。②**板级 OPP/带宽数据**在 `~/work/meizu20/meizu20-m1/dts/sm8550-meizu20-stock.dts`：`qcom,gpu-pwrlevels-0` 9 档 = 719/680/615/550/475/401/348/295 MHz（+ 同 level 的低档），每档带 `qcom,acd-level`（0x882e5ffd/0xa82f5ffd/0xe0285ffd/0xc02a5ffd/0xe02b5ffd/0xe02d5ffd/0xc02f5ffd）与 `qcom,level`（0xe0/0xc0/0x90/0x80/0x50/0x40/0x3c/0x38/0x34）+ bus 索引；`qcom,bus-table-ddr` 10 档 = 0/2,136,718/3,000,000/6,074,218/6,671,875/8,171,875/10,687,500/12,449,218/14,398,437/16,500,000 kBps；`qcom,gmu-freq-table = <220MHz 0x40 550MHz 0x80>`。③**BCM catalog** 从 mainline a740 条目（`.cache/adreno-source/.../a6xx_catalog.c` chip 0x43050a01）：SH0 buswidth 16 / MC0 buswidth 4 / ACV fixed+perfmode=BIT(3)+perfmode_bw 16,500,000，`gmu_chipid=0x7020100`。
- **GPU 剩余工序（数据已齐，纯工程）**：①写 `ShengGmuVotesSource.h`（上列三张表；消费侧 `reference/unixnu-sheng/package/ShengGmuVotes.c` 已在树）②`build-adreno740-kits.py` 去 xcrun 化（用 toolchain clang/ar，本机 Linux）③DTS 开 `&gpu/&gmu/&gpu_smmu`（sheng 只加前两个，gmu 的 reg 在 stock 是 0x3d68000 vs 我们 dtsi 0x3d6a000，**需对表核实**）④defconfig `CONFIG_XNU_NATIVE_ADRENO=y`+组件/archive 条目 ⑤payload gpu addon；Metal staging 仍限 macOS。全部在 `CONFIG_XNU_NATIVE_ADRENO=n` 后，**不扰动现役刷写链**。
- **会话十八三刷判读（2026-10-30 09:48 屏摄，刷 a62f5e53）：第 3 堵墙 = crypto KPI 表**：`PAC failure ... while branching to x2 at pc 0xfffffff00aee6958`。pc 归属 = native `SHA512_Init`（libkern/crypto/corecrypto_sha2.c），失败指令 = `braaz x2`（IA、零修饰）尾调 `g_crypto_funcs->ccdigest_init_fn`；`g_crypto_funcs`（全局 0x7cf1c70）→ corecrypto 自己的表 `0x82f3048`（corecrypto __DATA_CONST，文件态全零 = 运行期填）。**x2 = `0x34e36cb0_0a6fa120`：低 32 位恰是 corecrypto 函数 `0xa6fa120`（已验 pacibsp 函数起点、digest-init 形态），高 32 位不是合法 arm64e tag。** donor 里指向该函数的 7 个槽全是 `auth=1, key=IA, diversity=0, **addrDiv=1**` 的链条目（出件链字 0x80110000036f6120，解码 = target 0x36f6120+base、step=2 正确），即 donor 侧按**地址修饰**约定签，我们的消费者按零修饰 branch → 不匹配（=会话十六预注册的 "crypto KPI 表边界" 兑现）。**与 sysctl 墙同源（α 家族），但这次值本身不是规范签名态 → 盲改消费者有风险**；下一步 = 给 `register_crypto_functions`（已是我们的 native 绑定，corecrypto start 时必过）加一次性探针，把收到的表指针 + 前 8 个字打进 panic 行，一个刷次拿到表内全部条目的真值再定修法（probe 后立即撤）。
- **探针件（会话十八四步，2026-10-30）**：`patches/xnu-12377/crypto-funcs-probe.patch`（进 arm64e series）在 `register_crypto_functions` 注册时 panic 打表指针 + 前 9 个字（`unixnu-probe crypto_functions %p [0..8]`），HAS_APPLE_PAC 下无条件生效，**一次性**——判读后从 series 摘掉。出件 kernel `abe0d73c` → payload **`8a40d909`**（96MiB，ESP 内核副本 `0d8c9fb5`，探针串在 ESP 副本内实证；trust/seed 同前）。存档 `out/m2381/payload-cryptoprobe-8a40d909.img`。判读三选一：①4 个函数指针都是规范带 tag（`XXFFFFF00A6FA120` 形态）→ 失败值另有来源；②"高 32 签名 + 低 32 地址"两段式 → 在 `register_crypto_functions` 里按表重签（该函数已是 native 绑定）；③全零 → 表运行期后填，探针位置后移。回退 = 刷回 `a62f5e53`（sysctl 修复件）。
- **探针判读（2026-10-30 10:09 屏摄，IMG_20261030_100927）**：`unixnu-probe crypto_functions 0xfffffff0082f3048` → `[0]1de141b0_0a6fa120 [1]29f21030_0a6fa828 [2]0 [3]61c21530_0a6f8e50 [4]fffffff0082f1b00 [5]fffffff0082f2c30 [6]fffffff0082f2ee8 [7]fffffff0082f28a0 [8]fffffff0082ef338`。**结论：两种指针格式并存**——数据指针（[4]–[8]，md5/sha1/sha256/sha384/sha512 的 di）是规范地址；四个**函数指针**是 `高 32 位签名 + 低 32 位集合内偏移` 的打包形（低 32 位解出 corecrypto 函数 0xa6fa120/0xa6fa828/0xa6f8e50 等），对应 donor 侧 `autiza` 32 位鉴权（kext 集合 ABI），我们的 clang 只产 `braaz` 64 位形 → 恒败；[2] 为 0（该表字段在 donor 侧未填，属预期，不要在后续误报）。**该表在 donor/出件文件里都是全零 BSS（0x82f3048 无任何链条目），值由 corecrypto 运行时写入**——所以 emit 侧修不了，只能修读侧。
- **修法（已落件，替代探针）**：`patches/xnu-12377/crypto-funcs-unpack.patch`（探针补丁已从 series 摘除）——在 `register_crypto_functions`（已是 native 绑定）里把表拷进内核静态副本，逐槽把"打包形"还原为 `0xfffffff000000000|低32` 并按 IA/零修饰重签（`ptrauth_sign_unauthenticated`），`g_crypto_funcs` 指副本；数据指针/规范指针原样保留（判据 = 高位 `((v>>32)&0xffffff)==0xfffff0` 或解出地址不在 `__TEXT_EXEC` 区间则不处理）。此修一次覆盖表上所有消费者（SHA1/256/384/512、HMAC、AES…）。
- **出件（unpack 修法）**：kernel **`892ebfe1`** → payload **`c3b36e61`**（96MiB，ESP 内核副本 `2b22aba1`，探针串已移除实证；trust/seed 同前）。存档 `out/m2381/payload-unpack-c3b36e61.img`；回退 = `a62f5e53`。反汇编实证 = `register_crypto_functions` 内 `mov w2,#0x1c0`（448B 表拷贝）+ 逐槽高位判定/区间判定/重签循环。
- **五刷判读（2026-10-30 10:20 屏摄，刷 c3b36e61）**：**unpack 修法生效**（crypto 表墙消失），新墙 = `Invalid spinlock 0x…: <0x000000002200010f 0x0> @locks_arm.c:343`。锁字 = `lck_mtx` 的 tag `0x22000000|flags`，type 字（+8）=0 → **一个 mutex 被当 spinlock 锁**。**根因 = 会话十七 的三条 lck fixture 类型标错**：donor 0xa7f3548/0xa7f3fe4/0xa7f4128 的体全部操作 **[lock+8]**（`casa 0→cpu` 加锁 / try 加锁 / `casl` 解锁），而 `lck_spin_t` 是 **data@+0、type@+8**（`lck_spin_lock` 先验 `[+8]==0x11` 再锁 `[+0]`），mutex 才是 data@+8——当时按"CAS 0→cpu@+8"认成 spin 是误判。修法 = fixture 三条改名 `_lck_spin_{lock,try_lock,unlock}` → `_lck_mtx_{lock,try_lock,unlock}`（sha 钉不变，只换 native 符号名），设备仓提交。
- **出件（lck 改名修法）**：kernel **`e6217d36`** → payload **`30c11c32`**（96MiB，ESP 内核副本 `f1c9fb0d`）。实证 = corecrypto 里 7×lock / 9×unlock / 1×try / 1×alloc_init 的 call site 全部改指 native mutex 实现（`IOLockLock/TryLock/Unlock` 与 `lck_mtx_*` 同址别名）。存档 `out/m2381/payload-mtx-30c11c32.img`；回退 = `c3b36e61`。

**★会话十九开工单（2026-10-30 交界；用户拍板"安排下一轮，拍屏下会话发"）**：

- **用户动作**：①只刷 recovery_a ← `out/m2381/payload-mtx-30c11c32.img`（sha256 `30c11c32…250e`，100,663,296 B），boot_b `d0200a6b` / userdata 不动；②开机后把 panic 屏整段拍全（若越过了前六堵会是新类型）；③回退件 = `payload-unpack-c3b36e61.img`（已验证能走到 crypto 表墙之后）；④**不要连续多次重启**，保持单一证据点。
- **我这边顺序（含离线先行项，不等刷机）**：
  1. **离线杠杆 = donor 内核导出符号**（目标：把 1659 个未配对函数里的 kext-facing 目标批量按名字绑定，替代"一堵一刷"）。已知证据：kernel entry 的 load command 表里 cmd=`0x32`（off `0x9450`，cmdsize `32`），其后六字 = `0x2 / 0x1a0500 / 0x1a0500 / 0x1 / 0x3 / 0x4f30000`；**文件偏移 `0x1a0500` 处是符号名字符串区**（首条 `AVD_RD_DMA_LVP_COLO_DMA_CONFIG`）→ 导出/符号名区就在附近。步骤：先判 `0x32` 语义（非标准 `0x80000022`；排除 LC_DYLD_EXPORTS_TRIE `0x80000033` 的普通形态），定位 trie 头（ULEB terminal/flags/address 结构），解出 name→address；再按"native 侧同名唯一函数地址"批量落 reviewed fixture（how=`export-name`），逐条写 evidence，唯一性为唯一收编判据。
  2. 若导出表不可用：给 matcher 加**通用通道 = 去 BTI 前缀 + 忽略 adrp/add 常量后比对整函数体**（现在 body-hash 只吃逐字节相同），并把 1659 个 stub 目标按组件出清单 + 调用点数排序，先修热点。
  3. 判读屏摄 → 归类（桩 / PAC 约定 / 32 位打包指针 / 锁类型标错 / 其他）→ 修 → 重建 kernel+payload → 出件命名 `payload-<类>-<sha8>.img`。
  4. 每次修法回写 fixture evidence（判据必录）；映射只收唯一命中。
- **红线**：`CONFIG_XNU_NATIVE_ADRENO` 保持 `n`（GPU 等进桌面）；payload 命令必须带 `--unique-chip-id 3645ad55d47e77c1 --trust-cache out/m2381/ipados-26.5.2-23F84/m2381/original-trust-23F84.bundle --data-ramdisk out/m2381/ci-artifacts/verified-root-grow/owner-data-fixture/scratch-data.img`（漏了会少 trustcache/initrd，r85 坑）；刷写只碰 recovery_a；**SHLOG 在 UFS 起来之前是空的**（写手要 BootProgress 监视器 tick + native_ufs_log_open），早期 panic 别白跑读者任务。
- **判读门**：本墙消失 + corecrypto start 继续；prelink 统计（reviewed / body-hash / reference-order 配对变化，stubbed 数下降）；出件内核 call site 逐点反汇编（布局偏移 0x346c000 口径）。

**★会话十九 = 批量消账：导出符号杠杆判死 + BTI 通道落地 + kext 入口 PAC 墙（2026-10-30 上午，用户刷 30c11c32 后屏摄 IMG_20261030_103414.jpg）**：

- **新墙判读（第七堵）**：`PAC failure from kernel with IA key while authing x16 at pc 0xfffffff00aea68fc`。pc 归属 = native `OSKext::validateKextMapping(bool)+0x130`（emitted→build 减 0x346c000 = 0xfffffff007a3a8fc，源码 xnu `libkern/c++/OSKext.cpp:8165` 的 `ptrauth_auth_data(address, ptrauth_key_function_pointer, 0)`）。x16 = x21 = `0x54d301b0_0a4cdf18`，来自 `ldur x21,[x29,#-0x50]`，源头是 `ldr x21,[kmod_info,#0xb4]` = **com.apple.kec.Libm 的 `kmod_info->start`**。**值解 = 32 位打包指针**：低 32 位 `0x0a4cdf18` = emitted base(0xfffffff007004000) + donor 链条目 target `0x34c9f18` = `0xfffffff00a4cdf18`（Libm __TEXT_EXEC+0x1e18，ESP 内核对位）；高 32 位 `0x54d301b0` 是签名段。**文件态同址** = donor auth 链条目 `0x8010e4fe034c9f18`（isAuth=1 / key=IA / addrDiv=0 / diversity=**0xE4FE** / target=0x34c9f18，donor 与出件逐字节相同）→ 与 corecrypto KPI 表**同族**（donor 侧按编译器类型判别子签的 32 位打包形态），我们的 `autiza` 用零判别子 → 恒败。注：panic 行 OCR 上 `0a4cfd18` 是误读，实为 `0a4cdf18`（裁切放大核对）。
- **修法（已落件）**：`patches/xnu-12377/kext-entry-donor-pptr.patch`（进 arm64e series，位于 crypto-funcs-unpack 之后）——在 `OSKext::initialize` 从 `_PrelinkKmodInfo` 取到 kmod_info 之后归一化 `start`/`stop`：`bits[55:32] != 0xfffff0` 判为打包形态 → `0xfffffff000000000 | 低32` 还原地址并按 (IA,0) 重签；已是规范形态 → `ptrauth_strip` 后按 (IA,0) 重签。实证（native 反汇编，0x7a2d4e0 起）：`ldur x10,[x8,#0xb4] / cbz / and …,#0xffffff00000000 / cmp #0xfffff000000000 / b.ne` → `mov x9,#-0x1000000000; bfxil x9,x10,#0,#0x20; paciza x9; stur x9,[x8,#0xb4]`，`stop`（+0xbc）同形；ESP 内核副本 `c7dfa071` 内实证（`497d40b3`×2、`e923c1da`×2）。**修后 `autiza x16` 得到零判别子合法指针，且 `startfunc(kmod_info,…)` 的直接 `blraaz` 也成立**（只修 validate 不修字段会让下一步直接调用再崩）。
- **★导出符号杠杆判死（工作单第 1 项，证据确凿）**：`LC 0x32 @0x9450` = **LC_BUILD_VERSION**（cmdsize 0x20 → platform=2、minos=`0x1a0500`=26.5.0、sdk=`0x1a0500`、ntools=1、tool=3(Ld)、version=0x4f30000）——工作单把 minos 误当成"符号名区偏移"；文件偏移 `0x1a0500` 处确有串 `AVD_RD_DMA_LVP_COLO_DMA_CONFIG`，但那只是该偏移恰落在数据区，与符号表无关。**donor 全镜像没有任何 name→address 表**：①集合级 load command 无 LC_SYMTAB / LC_DYLD_INFO / LC_DYLD_EXPORTS_TRIE（只有 0x1b/0x32/0x5/0x80000034 + 段 + 298×LC_FILESET_ENTRY）；②298 个 fileset 子镜像逐个带 LC_SYMTAB，但 **nsyms=0 / nstrsize=1**（如 com.apple.kernel：symoff 73728000, nsyms 0, stroff 74190472, nstrsize 1）＝被剥；③镜像里唯一的符号名集合是 `__LINKEDIT` 首部 bplist（`SymbolsSets`，9 个 KPI bundle 共 **6124 个名字，无地址**；另有 `WeakRefFallbackSymbol=_gOSKextUnresolved`）＋ `__PRELINK_INFO` 的 `_PrelinkInfoDictionary`（38 条，只有 `_PrelinkExecutableLoadAddr`/`_PrelinkKmodInfo` 等地址，无符号）。**结论：按名字绑定在 donor 侧无源可依，此杠杆作废，不再投入。**
- **★批量通道（工作单第 2 项）落地 = BTI 前缀通道**：先量化了"为什么大量函数配对不上"——donor 每个函数入口带 `bti c`，我们的 clang 不产；已在 7920 对旧配对中实测：逐字节相等仅 42 对、去 BTI 后相等 443 对、再放宽到 relocation-masked shape 也只是 638 对（Apple 的 codegen 与我们的差异远不止 BTI），所以**不能用"整函数体"做通用通道**，但 **shape 锚点（relocation-masked，唯一性双侧）** 能经命名参照件大幅回收。
  - 实现：`kits/XnuPrelink/fileset_match.py` 新增 `without_bti()` 与 `anchor_bti_shapes()`（`Side.functions` 增记 `bti_digest`/`bti_length`；去入口 BTI + 去尾部 nop 后按双侧唯一绑定，`how='bti-shape'`，并纳入 `anchor_reference` 的可信来源集合）；`MATCHER_VERSION` 8→9（旧 map 自动失效）。设备仓 **bf1dac9**（含 3 条单测；`test_fileset.py` 的 `chain_relocations` 导入错误为上游遗留，与本改无关）。
  - 23F84 实测：**函数配对 7920 → 8450**（其中 reference-symbol 366→1353、reference-order 4796→4472、vtable 592→543、pure-leaf 98→81、bti-shape 9 直接命中——direct 少是因为多数 native 候选已被 order 通道占用，收益主要走命名参照链）；**kext-facing 目标命名 1338 → 1586 / 3094**；**stubbed 去重目标 850 → 700**（−18%；按 owner 求和 1687 → 1471）；**stubbed 去重站点数 3584 → 2642**（−26%；按 owner 求和 8557）。**成本登记**：`closure_pointers_unmapped` 641 → 767（+126，配对重排导致的可映射点变化，观察项）。
  - 三分类工具：`temp/batch-triage.py`（对 report 的 stubbed 表逐目标做 native-unique 形状试探；当前 native 侧唯一可救 128 个目标，其中 127 已被新通道收敛，剩 1 个——继续抠边际极低）。
- **出件**：kernel **`a0f98f77`**（89,374,720B）→ payload **`f7432f8e`**（96MiB；payload.json：ESP 内核副本 `c7dfa071`、trust `ebeaf7fb`、data seed `d04a0153`、ADT `8a5e00d6`、panic_screen=true、kernel_args 与上轮同）。存档 `out/m2381/payload-btibatch-f7432f8e.img`（100,663,296B）。回退件 = `payload-mtx-30c11c32.img` / `payload-unpack-c3b36e61.img`。
- **刷写卡（只刷 recovery_a）**：`payload-btibatch-f7432f8e.img`（sha256 `f7432f8e…bdbd237`，100,663,296B）；boot_b `d0200a6b` / userdata 不动；刷前 sha 门 + 刷后读回对拍。
- **判读门**：①`validateKextMapping` 墙消失（Libm 等 kext 的 start 真被调用，若后面再撞 PAC/桩再按类归）②corecrypto 之后继续起、kext 一个接一个 start（首个新墙位置即下一刀）③stubbed 表应显示 corecrypto/APFS 条目下降；`closure_pointers_unmapped` +126 是否兑现成运行时故障（若新墙落在某被重绑目标上，回退 bf1dac9 的单通道重跑对照）。
- **构建提速（用户问"是不是没多线程"）**：驱动 `unixnu kernel -j` 默认 **4**（`scripts/unixnu/kernel.py:198` 把 `ctx.jobs` 传给 `JOBS`），本机 16 核 → 以后一律 `-j16`；本轮 kernel 用默认 4 跑了两遍（第二遍增量），payload 已用 `-j16`。

**★会话二十 = 第八堵墙：donor corecrypto 的 entropy 回调类型判别子（2026-10-31 凌晨，用户刷 f7432f8e 后屏摄 IMG_20261031_003033.jpg）**：

- **判读**：kext 入口墙（validateKextMapping）与批量消账轮全部生效——corecrypto 继续起、Libm 的 start 已被调用并活着走完了 `init_with_getentropy` 的前半程。新墙 = `PAC failure from kernel with IA key while branching to x8 at pc 0xfffffff00a7098b4`（pc→build −0x346c000 = 0xfffffff00729d8b4 为内核侧相邻符号噪声；**实际 pc 属于 donor corecrypto 条目** `__TEXT_EXEC+0x391b4`，函数 = donor 的 `cckprng_init_with_getentropy`）。现场：`ldp x8,x2,[x19,#0x18]`（x19 = `prng_funcs` 在我们 __DATA 的副本 0xb6042a0，函数表槽 +0x18）→ `mov x17,#0xbcad` → `blraa x8,x17`。**x8 = 0xab883bf0_0a98a660：低 32 位 `0x0a98a660` = `&entropy_provide`（我们内核 `osfmk/prng/entropy.c` 的回调，native 0xfffffff00751e660 → emitted 同址）；高 32 位 = 我们 `paciza` 签的 tag（(IA,0) 约定）。** donor 用 `autiza x8, 0xbcad` 鉴权 → 我们签的 (IA,0) 恒败。
- **根因（α 家族第五形态，闭环）**：`prng_random.c:343` `prng_funcs.init_with_getentropy(prng_ctx, …, entropy_provide, NULL)`——我们把**内核侧回调指针**交给 donor corecrypto；donor 侧该回调类型 `cckprng_getentropy` 的鉴权判别子 = **编译器类型判别子 0xbcad**。**用我们的 clang 复现成功**：同一形状 `int32_t (*)(size_t*,void*,void*)` 在 `-fptrauth-function-pointer-type-discrimination` 下 `ptrauth_function_pointer_type_discriminator` 恰好 = **0xbcad**（`/tmp/gd*.c` 实验，同 host clang）；donor corecrypto `__TEXT_EXEC` 内 `mov x17,#0xbcad` 共 4 处（0xa7096ac/0xa7098b0/0xa71702c/0xa72d244），全部是该回调的存/调点——只签一次交接处即全覆盖。与 sysctl 0xE2E / kext 入口 0xE4FE 同族：**Apple 构建全线开类型判别，我们全线关**。
- **修法（已落件）**：`patches/xnu-12377/prng-getentropy-donor-discriminator.patch`（进 arm64e series，kext-entry-donor-pptr 之后）——`register_and_init_prng` 交接处 `unixnu_donor_getentropy(entropy_provide)` = `ptrauth_sign_unauthenticated(callback, IA, 0xbcad)`。实证（native 0x751d9b4）：`adrp/add &entropy_provide → paciza x16（参数 ABI 签）→ mov x6,x16 → mov x16,#0xbcad → pacia x6,x16 → …blraa x17(0x…3051 交 init_with_getentropy)`；ESP 内核副本 `0ac4d4a8` 内 `0602c1da`×5、`b09597d2`×1。HAS_APPLE_PAC 在该 TU 已可用（bootstrap 补丁同文件先例），零 include 增量。
- **α 家族判别子总表（本线 5 堵全收）**：corecrypto KPI 表（0x36f6120 打包+addrDiv）→ sysctl handler（0xE2E）→ kmod 入口（0xE4FE / kext-entry-donor-pptr 修）→ **entropy 回调（0xbcad，本轮）**；每一刀的通式 = "donor 按它的判别子签/验，我们按 (IA,0)"，修法均为"在交接点按对方的判别子签"。**预测：同为 `CC_SPTR`/回调类型的下一处会在别的 donor→native 函数指针交接上（如有），同一刀法。**
- **出件**：kernel **`14ce27e2`** → payload **`a268cd8d`**（96MiB；ESP 内核副本 `0ac4d4a8`；trust `ebeaf7fb` + seed `d04a0153`；panic_screen 在；kernel_args 同前）。存档 `out/m2381/payload-getentropy-a268cd8d.img`（100,663,296B）。主树 **44cb8a1**。回退件 = `payload-btibatch-f7432f8e.img`（上轮）。
- **刷写卡（只刷 recovery_a）**：`payload-getentropy-a268cd8d.img`（sha256 `a268cd8d…4db12`，100,663,296B）；boot_b `d0200a6b` / userdata 不动；刷前 sha 门 + 刷后读回对拍。
- **判读门**：①本墙消失，PRNG 初始化走完（getentropy 会在 per-CPU init/generate/refresh 反复被调，四个消费点同约定，一次到位）②corecrypto start 完成 → kext 继续逐个 start（下一堵按类归）③若新墙仍是 PAC-failure branching-to-xN 且 xN 低位落在 native `__TEXT_EXEC`，直接套 α 家族总表找交接点。
- **杂项**：本轮 kernel 全量重建用 `-j16`（stamp 变更触发干净重铺），编译+prelink 约 4 分钟（-j4 时代 13 分钟）。上游核过：feat 支线 2 新提交（AdrenoMetal pin bump + 6 个打包/GPU 脚本）与本线零交集，XnuPrelink 上游无新提交；设备仓 vendored fork 的 bti-shape 通道（bf1dac9）待回推上游。

**★会话二十一开工单（2026-10-31 凌晨收尾；设备已刷 payload-getentropy-a268cd8d 在跑，用户下会话交屏摄判读；电脑重启前资产清点=零待回收）**：

- **判读门（entropy 回调修复轮）**：①`branching to x8` 墙消失（x8 低 32 位 = &entropy_provide 的那发 blraa 不再失败）②PRNG init_with_getentropy 走完（per-CPU init/generate/refresh 四个 0xbcad 消费点同约定）③corecrypto start 完成 → kext 逐个 start，首个新墙按 α 家族总表归类。回退件 = `payload-btibatch-f7432f8e.img`。
- **用户已拍板（本会话执行）**：panic 屏加**校验行**——`scripts/xnu-panic-screen.c` 末尾渲染 `crc=xxxxxxxx len=nnn`（CRC32 over PANIC_TEXT 缓冲内容，zlib 口径），拍屏转录后对账杀 OCR 误读（cfd18/cdf18 教训）。约束：panic 屏代码住内核 padding 洞（PANIC_CAVE_END 0xfffffff00b9d0ff0，超限构建报错），CRC 增量 ~200B 预计塞得进；改动 = 主树脚本（b01035d 先例），只刷 recovery_a。
- **排队项（不挡判读）**：①上游 `scripts/build-fileset-release-reference.py` 参数化到 out/m2381 接走参照件构建（消 17.6MB 树外产物债）②设备仓 bti-shape（bf1dac9）回推上游 XnuPrelink ③**二维码整线取消（用户拍板 10-31）**：上游 AdvancedLogDxe/扫传是 UEFI 阶段工具（我们 UEFI 一直正常跑，问题全在内核侧）；panic 屏全 QR 立项一并撤销，CRC 校验行保留为准 ④UART 通道已否（无串口）。
- **基线备忘**：上游已 rebase 到 8fe2aaa（961c7da 记录）；下轮构建一律 `-j16`；刷写红线不变（只 recovery_a，boot_b d0200a6b / userdata 不动，--trust-cache/--data-ramdisk 必带）。

**★会话二十一执行档（2026-10-31 凌晨，两轮屏摄判读+两堵落地+CRC 上岗）**：

- **第九堵=entropy 补丁叠签自伤，已修已实测过墙**：屏摄 011739 → pc=donor corecrypto 回调 wrapper `blraa x8,x17(0xbcad)`，x8 目标=我们 `_entropy_provide`(+0x0)。根因=第八堵补丁 `ptrauth_sign_unauthenticated` 直接叠签（本机 clang 把函数引用物化成 `paciza`，(void*) 转换不剥离→`sign(sign(addr,IA,0),IA,0xbcad)`，autia 对干净址重算必败）；0xbcad 判别子本身没错，错在没 strip。修法=补丁加 `ptrauth_strip`（主树 13e2871；同 sysctl/kmod 已验证 idiom），出件二进制实证单签形态 `paciza→xpaci→pacia(0xbcad)`。上轮"批量轮生效实证"只证明 init 调用过墙，回调本体首次被调即崩——**教训：PAC 交接修法必须反汇编验证单签**。全链路账：corecrypto 自配置函数栈上 funcs 表（每项 pacia(IA,槽址|魔数)）→ 我们逐槽重签循环 → `cckprng_funcs` 六字段序 → funcs[5]=丢 x1 的 shim → cckprng_init stp arg5/6 → [ctx+24]/[ctx+32] + wrapper 注册 [ctx+0xad8]；崩溃 ctx=corecrypto 自有全局 0x0b6042a0。
- **第十堵=AppleImage4 `_kalloc_type_impl_external` 桩，已修**：屏摄 030636 → `unmapped kernel function from 0x08d419ec (x0 0x07f64320 x1 0x4)`。三重判据定名（源序夹逼 kalloc.c:2720 + 签名 `(kalloc_type_view_t, zflags)` + 体形 0xd001000|flags/cpu<<14/尾调 zalloc_ext）。修法=设备仓 fixture 118→119（**31fa4e5**）。方法链沉淀：桩点反汇编→原始 KC 同偏移字节特征→stubbed 账本交叉→map 邻居夹逼→源码函数序。坑：fileset 条目 idsize=0 按预链接序对齐（31 条含 xnu 本体 0x0a73c000+0x980000，与 +0x346c000 全自洽）；bl 目标勿用模式首址（差 8）。
- **CRC 校验行上岗（用户拍板项，主树 c296784）**：渲染 pages/free 后画 `crc=`=zlib crc32 over PANIC_TEXT 本轮拷贝（logn>0 才画）。**首轮实测即立功**：030636 屏 crc=0xe8dbdc06 与重建消息逐字节吻合（132B），转录被证明而非假定。**坑=本车道 panic cave 零余量**（eeedeb9a 版已吃满 0xa30，首版 +51B 构建炸）；瘦身（砍 len= 列、hex() 单调用）后 0xa0d/0xa30 余 0x23；洞后 0x13B 即真数据，不可外扩 PANIC_CAVE_END。
- **出件**：kernel **9e47d053** → payload **5f59a2a0**（96MiB，存档 `out/m2381/payload-kallocfix-crc-5f59a2a0.img`；trust `ebeaf7fb`/seed `d04a0153` 不变）。设备现役 = recovery_a ← 5f59a2a0（用户已刷，030636 屏摄即此件在跑），boot_b d0200a6b / userdata 不动。

**★会话二十二开工单（2026-10-31 凌晨收尾；用户拍板"批量处理 stubbed 墙，下会话分析和修，本轮只提取日志"）**：

- **本轮提取的日志（030636 屏摄，crc=0xe8dbdc06 已验真）**：`fileset: original component called an unmapped kernel function from 0xfffffff008d3ab04 (x0 0xfffffff58bd698a8 x1 0xfffffff00b038fb8)`，pages/free=0x125b8c/0x12544c。caller=AppleImage4 exec+0x7254（站点 0x08d3ab00）；原始 KC 同偏移本尊 bl → donor 目标 **0xfffffe000ae40858** ↔ stubbed 账本 `0x0ae40858`。体形速写：`bti c; cbz x0,slow;` 包装 +8 的 pacibsp 主函数。x0=带 0xf5 tag 的签名数据指针（站点上下文有 `autda x16,x17(0xcda1)` 多样化鉴权族），x1=0x0b038fb8（xnu exec 区间指针，疑回调/cookie）——下会话定性。
- **批量管线工作单（kalloc_type_impl_external=已验证模板，可复制）**：
  1. **输入**：kernel.json `stubbed` 按组件账本（donor 目标 VA + 站点数；全账合计 8552 站点/1471 目标）；map.json `functions`（已配对邻居，order-based）。
  2. **优先级=站点数×组件数**：头号大户 **0x0ae40858**（AppleARMPlatform 343 + WatchdogTimer 28 + AppleImage4 10 = 381 站点，且是多组件共享的内核核心函数）；次席 0x0afc0740（corecrypto 6+2/AMFI?/AppleImage4 6+2 等多组件）；逐表排序。
  3. **自动夹逼**：donor VA → map 邻居（上/下最近已配对）→ 我们构建的对应 native 区间 → xnu/kext 源码函数序候选 → 体形（masked shape）+签名+arg 用法三重验证 → **批量产 fixture**（original/original_size/original_sha256 自动算，evidence 半自动人工过目）。
  4. **无 native 对应者**（Apple-only helper）：小 shim 桥接（fileset-unmapped-call.patch 先例），逐个评估。
  5. **验收门**：重建后 stubbed 账本站点数单调下降；boot 推进过 AppleImage4（下一组件新墙按同法）。
  6. **风险**：fixup 勿盲改（#21 教训）；每条 fixture 必须证据行可回溯；跨组件共享目标修错代价=多组件同崩，先在小站点组件上验。
- **排队项不变**：①build-fileset-release-reference.py 参数化接参照件 ②bti-shape 回推上游 ③UART/QR 维持取消。基线：-j16；刷写红线只 recovery_a；--trust-cache/--data-ramdisk/--unique-chip-id 必带。

**★会话二十二执行档（2026-10-31 凌晨，批量管线轮+工具链税定谳；用户拍板 A+B=三通道落袋+CI 定量实验）**：

- **"为什么不直接用上游的"实证调查（用户问出来的关键情报）**：①上游 `kits/XnuPrelink` gitlink **冻结在 76452c6e**（会话十三 vendored 后零新提交）——bti-shape（会话十九）上游就没有，今天三刀上游更没有；②`UniXNU-sheng` main=**21H461 车道**（os_profile=ipados-17.7.11），其 `feat/ipados27-gpu-parity` 分支=**24A446 且 10-31 当天还在提 GPU trace/pid names=真机深水**，但该仓**无自有 fixture、matcher 未动**；③他们过桩墙靠的是 **macOS Apple clang codegen 与 donor 同构**（BTI 有、FPAC 软检查块无、分配器近→字节级 digest 通道直配），我们=全世界第一个 Linux(lld/clang-24) 建 fileset 内核上机（会话十四已记档），stubbed 大头=**Linux 工具链税**（BTI 无+**FPAC 软检查块 13052 处**+寄存器分配差）。sheng 的件不可刷（板级/ADT/组件集/UEFI 链全不同），工具链姿势可抄。
- **三通道落地（设备仓 b68da7b，MATCHER_VERSION 9→10，22 测试绿）**：①`anchor_bti_checked_shapes`=BTI 剥除+FPAC 检查块删除**组合归一**——旧 anchor_bti_shapes/anchor_checked_shapes 各只处理单一差异，双差函数（BTI 前缀∧检查块）全部漏网=699 目标残留的系统性根因；②`checked_shape` 删块判别子推广：aut x16,x17 两硬编码 → `autia/autda x16,<任意寄存器>`（`(w&0xfffffc1f)∈{0xdac11010,0xdac11810}`；native 全镜像扫描实证检查块仅两形、autda x16,x8/x9 变体在 IORegistryEntry/IOReporter 路径真实存在）；③`alias_thunks`=donor `bti c; b X` 别名 thunk（8B，393 站）配到 X 的 native，含链式，bind 例外表加 alias-thunk。**头号 0xae40858 定谳=`OSMetaClassBase::safeMetaCast`（1679 站/25 组件）**，三重验证=体形（0xcda1/0x38/0xfffc 逐字同）+arg 用法（x1=toMeta 对比 getMetaClass 链）+源序（OSMetaClass.cpp:201/291/385 夹逼）。
- **实测（本机 -j16 全量）**：emit 账本 **8552→4913 站点（−42%）/699→654 目标**；kernel **f763e9e6** → payload **28fe6d56**（96MiB，ESP 内核 905fb6fa，trust ebeaf7fb/seed d04a0153 不变；平铺 `out/m2381/payload-batchfix-28fe6d56.img`）。验收门"站点数单调下降"过。
- **残余分类（temp/batch-fixtures/）**：同形家族 87 目/525 站（Dispatch 家族 15+ 全同体小 thunk，1:1 门正确弃权，需 vtable 槽位/邻居消歧——0xaea0bf8=IOUserClient::Dispatch 48 站已锁）vs 真孤儿 560 目/4354 站（TOP=0xaeb8eb0 IORegistryEntry 区 190 站（registry 递归锁+两虚调+解锁，源序无对应=疑 122.8 新增）、0xaed7e2c IOService 区 168、0xa95ca30 锁函数 127（tpidr+0x1b8 计数器+casa+wfe）、0xaf6236c IOReporter 92）；源漂移类=donor xnu-12377.**122.8** vs 源 121.6（IOSimpleReporter::setValue 多一次调用，0.97 相似）。工具=temp/batch-fixtures/{batchid,fndiff}.py。
- **CI 定量实验（B 腿）**：run 41124044607 秒挂（**pre_build 期望 shell 命令串，`ci-owner-policy` 不是命令=127**；kernel-only 不需要 owner policy）→ 重发 **41124555115**（空 pre_build，kernel-only，xcode 26.6）；主树 m2381-ios27 force-with-lease→远端 m2381（63d7969），设备仓 main 已推 b68da7b。
- **判读单（下会话）**：①**CI 结果=macOS 建 stubbed 计数**→量化工具链税，拍板 B（macOS 建核转正）vs C（Linux `+fpac`，实测有 FPAC 硬陷阱=历轮 donor 侧 PAC failure 同步 panic 即证，开掉 13052 处软检查块）主攻；②若刷 28fe6d56：判读门=AppleImage4 之后下一组件新墙（stubid 同法），4913 站=背景噪声逐墙消；③排队：同形家族消歧通道（vtable 槽位法）、真孤儿 TOP 手写 fixture、bti-shape+本轮三通道**回推上游**（改基线=上游欠我们的）、pre_build=ci-owner-policy 的正确喂法考证（会话三十五跑通过，当时喂法待查）。

**★会话二十二续 = CI 定量实验结果（2026-10-31 凌晨；**B 腿证明可行，工具链税定谳**）**：

- **喂法三坑全修**：①`pre_build` 期望 shell 命令串非脚本名（`ci-owner-policy` 127——那是 20H392 代 hdiutil 重建机制，27 车道不需要）；②政策对 CI 缺失 → 设备仓 **4b7e23f** 强制入库紧凑集（`.cache/firmware/verified-root-23F84/{OS,SystemOS}/{manifest,trusted-key,verification}` 27MB，`git add -f` 破例有档）+ 主树 `build-sheng-source-storage.py` 加 `.cache/firmware` 回退（**87acd33**）；③board.toml 无条件喂 `--fileset-reference` 而 CI 无 RELEASE 参照件 → 主树 kernel.py 文件不存在时降级去参（**9f0e4ce**；**本地 c8214f7 与远端 9f0e4ce6 同树异 sha**——git 通道 TLS 断连时经 GitHub Git Data API 推的，代理恢复后本地已 reset 对齐）。
- **决定性 run = 41132298280 全绿（11m48s，xcode 26.6）**：macOS 全链（63 补丁系+组件+XNU+emit）一次通过；此前 41124555115 的 `clang++ linker` Error = 设计内容忍的 bootstrap（会话十三既立），非 B 腿障碍。
- **工具链税同口径全表**（同 matcher v10/同 fixtures/同 donor 23F84/**双方都无参照件**）：

  | 指标 | Linux clang-24/lld | macOS xcode 26.6 |
  |---|---|---|
  | functions matched | 7163/21113 | **8324/21113 (+16%)** |
  | body-hash | 56 | **542 (×9.7)** |
  | pac-checked-shape | 350 | **1312 (×3.7)** |
  | vtable | 876 | **1285** |
  | data matched | 541 | **1200 (×2.2)** |
  | bti-checked-shape（我方新通道） | 858 | 28（macOS 无双差体=本就不欠） |
  | kext-facing named | 1517/3094 | **1783/3094** |
  | emit `stubbed functions`（按组件去重目标口径） | （带参照件）1234 | **932（输入更差仍反超 −25%）** |

  参照件在 Linux 值 ~1900 站（4913 vs 6841 site 口径）；CI 未上传 kernel.json，site 口径精确数下轮补（workflow upload 加 kernel.json 或 CI 内建参照件）。
- **判读 = B 治本、C 减税不足**：macOS 无参照已反超 Linux 带参照；pac-checked 350→1312 说明 FPAC 软检查块是 Linux 税大头（C 可把此项追平）但 body-hash ×9.7/vtable/data 差异 C 覆盖不了。**建议主攻 B**：①CI 内建 RELEASE 参照件（消最后一项输入差）②payload/userdata CI 化 ③932 残余=真 Apple-only 尾巴走 fixture/shim（上游 sheng 同款姿势）④本地 Linux 保留快迭代腿，C 作为 Linux 腿减税可选项。
- **出件态**：本地现役 kernel = 8b60dbb4（带参照件重出，ledger 4913/654 与 f763e9e6 全同、二进制 rerun 非确定）、payload **28fe6d56**（平铺 `out/m2381/payload-batchfix-28fe6d56.img`，ESP 内核 905fb6fa=f763e9e6 派生）可刷；CI 侧 kernel 未上传（只有日志）。
