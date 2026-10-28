# STATE — 现势状态（m2381，2026-10-28 更新）

> **历史全档**：2026-10-24 → 2026-10-27 全部轮次（§0.0–0.34）与当时静态区
> 在 `docs/archive/STATE-history.md`（原样快照）；每轮定谳同时完整活在
> git commit message 里。本文档 = 精简现势版：现状 / 资产 / 方法 / 开放
> 问题 / 下一步 / 轮次索引。

## 1. 一句话现状（**r75 已出件待刷（2026-10-28 深夜）**：fixup #28 =
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
| recovery_a | **r75**（fixup #28 = `CA::Layer::set_position` NaN 清洗路径强制，28 条 fixups + 扫描器三改探针 + noidle=1 + dirty 窗 R 码 + AKS 替身 + vsync + client=0 + Data seed + trustcache + forensic reader；内核仍 `ce34e9ae` 代） | `23868de1` | **出件待刷（2026-10-28 深夜）**；`out/m2381/payload-r75-23868de1.img`（96MiB，sha256 全 = `23868de161164db35c0a2b8201c3c8650155545ab19742de7f389b3943a6a461`）；ESP 四样已验（ADT PackedRAMDisk + 28 条 fixup 对拍零缺失 + scratch-data + initrd 行 + trustcache）；r74 = `b0f2f656` 已刷已判（判读 = §1） |
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

## 7. 下一步（**r75 已出件待刷（2026-10-28 深夜）**：payload
`23868de1`（内核不变 = ce34e9ae 代）；fixup #28 = `CA::Layer::
set_position` NaN 清洗路径强制；用户执行刷写）

1. **刷写卡（r75 = 只刷 recovery_a）**：iOS 链现役未被覆盖
   （boot_b=UEFI `289614ba` + recovery_a=r74 `b0f2f656` +
   userdata=`fd43ee70`）→ fastboot（电源+音量减）→
   `getvar current-slot` = b → sha 门
   `23868de161164db35c0a2b8201c3c8650155545ab19742de7f389b3943a6a461` →
   `flash recovery_a out/m2381/payload-r75-23868de1.img` → reboot。
   **跑 8–15 分硬断电**（勿超 17 分回卷线）；reader 选单照旧。
   **屏面观察点 = 进度条是否开始走动**（r74 = 2:30 出现但冻结）。
2. **r75 判读工作单（预注册）**：开场三连照旧 + header 分代。分支：
   ①**'CALayer position contains NaN' 0 出现 + applied 23→24** =
   #28 生效（sanitize 路径接管）；预期候选：布局继续 → 下一渲染
   几何墙（bounds NaN 族 / 其它 CA 检查点）、或 SB 推进到
   wallpaper/图标层（**若进度条走动 = 里程碑级信号**）；若清洗后
   渲染异常（花屏/冻结/新 REASON）细读定点。
   ②原点保留 → 查 applied（#28 页不执行 = 链路问题；执行而墙在 =
   raise 另有源（set_bounds 族？）→ REASON 细读）。
   ③PB 'configuration' 照旧记录（r74 基线 ×89）。
   ④回归六项 + sig11 CRASH 仍 0 + noidle 维持。
3. **r74 判读存档**：§1（#27 生效 + CALayer NaN 墙 + 抛点/清洗路
   径全链 + 回归六项 + 决策规则判定 = 有进展）；换内核决策规则 =
   memory `kernel-donor-switch-decision-rule`（连续两轮原地踏步才
   触发；触发后先归因内核侧/用户态侧；中间路线 = 保内核换 iPhone
   donor 21H16）。
4. **出件配方红线不变**：payload 三 flag + --unique-chip-id
   3645ad55d47e77c1（勿带 --skip-setup）；探针正源 = devices/
   .../package/*.c（tree kits/sheng = symlink 同一实体），root
   package 镜像同步；fixup 追加 = profiles/os/ipados-17.7.11-21H461/
   profile.toml text_fixups（expected 字节必须以 cache-extract 文
   件实读核验，勿手算）；判读纪律 = header 分代 + re-tools
   read_run（fixup-aware）+ recovery_a=sde27 + fetchchunk +
   **xref 扫描必须用修正掩码（add 判定 0xFF800000，§4.9）+ 零引用
   结论先过阳性对照**。
5. **独立主题（不阻塞）**：PB 'configuration' 断言（③支路；同管
   线可定点）；chronod sig5（r57 独立主题，r74 ~427 行）；
   BootSlotReset DXE；XS/fileset-KC 重绑定器（换 iPhone donor 中间
   路线的备件）；arm64e 专项；GPU/触摸/UFS 提速主题化。

## 8. 轮次索引（r43 → r61；详情 = commit message + STATE-history 对应节；r62 起 = 体系路线）

| 轮 | payload | 定谳 | 详情 |
|---|---|---|---|
| r43 | `764539c9` | policy panic 过墙（kern_exec.c:8043 墙清除） | history §0.24 |
| r44 | `a51aed16` | 崩溃簇符号化建成；FBS init ret + liveactivitiesd 禁用 | history §0.26 |
| r45 | `d3c11770` | 两修复实机生效；推进到第二层显示断言（BKSDisplayServices） | history §0.27 |
| r46 | `c3a24827` | mainScreen nil（fixup 7）；legacy-display 车道评估 | history §0.29 |
| r47 | `821ab7c0` | 背光服务链（fixup 8a/8b）；ABI 头入仓、=y 配方映射 | history §0.30 |
| r48 | `036e712f` | workspace 转换请求层（fixup 9）；CFString 解码方法 | history §0.31 |
| r49 | — | 首试作废（旧载荷被刷，路径歧义）；二试：fixup 9 未执行，惰性 oracle 确立，fixup 10 三连修 | history §0.32/0.33 |
| r50 | `4e69f450` | oracle 全排除（五站点全灭，抛者更深）；定位收口，转上游同步+调研 | history §0.34 |
| r51 | `22f8f0bd` | 上游同步 f85b5a6 + 调研收口（=y 是唯一机制解）+ **=y 车道施工出件**（内核 3ab2ace4，七轮修四墙） | PLAN-display-lane.md |
| r52 | `22f8f0bd` | **=y 一级实机全过**（PE handoff+真显示配置+swap+屏幕自稳）；SB 崩溃循环定谳 = BannerKit displayConfiguration 断言（深抛者钉死+验活）；设备回 nura-min | history §0.35 |
| r53 | `04f4d803` | **fixup 11 生效**（applied 5→6，BannerKit ret）；SB 推进到 BLSHBacklightHost brk 新墙（sig5）；W1 swap 异常→撕裂 logo 新观察 | history §0.36 |
| r54 | `1ef5eb63`（裸件） | fixup 12（BLSHBacklightHost 入口返 nil）；判读被 reader 调试顺延 | history §0.37 |
| r55a–c | `fb2c5dd1`→`69538b41` | reader 三连翻车定谳：LFN 毁（短名覆盖 toml）→ entry 缺 dtb 秒退 → ENABLE_SECUREBOOT=1 拒签（boot_b 换 `289614ba`）；payload.py 插桩转正 | history §0.37 |
| r55d | `087f9ca3` | 保命 cmdline 定谳：内核每必起（企鹅/UFS 全枚举），死因 = **initramfs cpio 手写 12 字段错位**（damaged header 实拍）→ root panic → 无 USB 同根因；LoadFile2 投递链实证是通的 | history §0.38/0.39 |
| r55e | `b060b512` | **取证 reader 通道立通**（1d6b:0104 + magic=SHLOG001 + 32MiB 窗在位）；reader 内读回对拍闭环；**首轮 iOS 判读**：applied=7 + W1–W3 真 swap（t=243–245s）+ 173 进程全生态，换墙 = displayIdentity 断言（SB SIGABRT ×552 同死点，符号化定谳 FBDisplayManager.mainIdentity=nil） | history §0.39/0.40/0.41 |
| r56 | `db3a7392` | **废轮**：出件漏 `--data-ramdisk`/`--trust-cache` → ESP 无 Data seed/initrd 行/trustcache → Data mount ENOENT panic（kern_exec.c:8196，t≈1s），554s panic-hold 诊断页，applied=0，fixup 9 未测到；ADT PackedRAMDisk 映射依赖 seed flag → 必须规范重建 | 本会话判读 |
| r57 | `1e7ea60b` | **displayIdentity 墙实锤破**：applied=8（fixup 9 真执行）+ W1–W3 swap 提前到 t≈85s + 178 procs/907 spawn + **面板画出 boot progress 菊花**；换墙 = XBApplicationController.m:120 断言（'invalid mainDisplay configuration'，SB ×560 同死点 t=66s 起，符号化定谳 isMainDisplay==0 双分支跳 handleFailureInMethod）；读回对拍补欠全绿 | 本会话判读 |
| r58 | `d27c4f36` | **XB 断言墙实锤破**（applied=11 + SB sig6=0）+ SB 进应用装载；换墙 = UIScreen trait SIGSEGV ×588（`_mainScreenThreadSafeTraitCollection+0x2c` 解引用 x19=0xf00 垃圾，12 帧验活全过）；读回对拍即时闭环 | 本会话判读 |
| r59 | `a9144765` | **UIScreen SIGSEGV 墙实锤封死**（applied=13 + sig11=0）；换墙 = traitCollection nil 断言 ×573（`__UIGetCurrentFallbackTraitCollection` line 117；REASON 串→CFString→xref 钉死抛点；SBWallpaperController/封面层链验活全过）；读回对拍即时闭环 | 本会话判读 |
| r60 | `edabbafd` | **traitCollection nil 断言墙破**（applied=14 + 'returned nil' 零出现）；换墙 = 同函数块 A scale≤0 报 "unspecified values" ×758（bg TC 故意全 unspecified，scale=0）；读回对拍即时闭环 | 本会话判读 |
| r61 | `169f160c` | fixup 15a/15b 生效（applied=70 + 'unspecified' 断言零出现，nil/scale 块全死）；**换墙 = SIGSEGV 垃圾指针族 ×684**（bgTC retain 快路径 FAR=0xf1，与 r58 0xf00 同族——UIKit 缓存全局垃圾族定性）；**用户拍板：停逐发 fixup，下会话转体系路线（上游/fork 调研优先）** | 本会话判读 |
| r62 | `0854ea5e` | **调研定谳 + 体系方案落地 + 判读定谳**：四过墙板模板（AKS 替身/vsync/client=0）三差距全补；已刷（10-28 fastboot 实录 OKAY）+ 读回对拍 ✓（sde27）+ 判读（10-28）：**体系三件实锤生效**（AKS opens 3→18 calls 6→49 / vsync hz=120 start+refresh / 探针客户端消失，C0=IOMFB_FDR_Loader 实为真实消费者——r52 嫌疑修正再修正）；applied=67 未跌破 14=环境未愈合；旧墙链全灭（r57–r60 五墙封死确认）；**换墙 = SB sig11 ×520 垃圾控制流**（unslid PC 0x1822F5FAC=cstring 区野跳，0xf00/0xf1 族升级）；r63 = UIKit 侧候选（全指定 TC / 缓存全局审计） | 出件+刷写 10-28，判读 10-28 |
| r63 | `04eb0533` | **全指定 fallback TC + 判读定谳**：RE 定谳 bgTC 病理链（懒缓存 getter → _defaultTraitCollection → mainScreen + 栈块 setScale/setGamut）；fixup16a（cbz→nop）+ fixup16b（bl $scale→fmov d0,#2.0），25 条；**已刷已验已判（10-28，sde27 对拍 ✓）**：fixup16 真执行（applied 69）+ AKS 爆发（opens 123/calls 1121）+ SWAP 泵大（8222 calls）；**垃圾族未愈——死点漂回 r61 有效码点（0x1800F5FAC，x0=0xf1），链钉死 = bgTC 缓存槽被写 0xf1，病根在槽写路径**；r64 = 缓存全局审计转正 | 出件+刷写+判读 10-28 |
| r64 | `54b70ad7` | **缓存全局审计轮**：静态定谳四病灶槽文件态全 0（懒缓存 nil 静置）= 毒在运行时写路径；UIKitCacheProbe 随件；判读 = 毒在 SB 首扫前就位 + 探针 OOB 停摆 bug（slide 双加越界，已修） | 出件+判读 10-28 |
| r65 | `d3d4e649` | 废轮：探针锁序 bug panic 实拍（递归 task_lock：capture 先 task_lock 再 get_task_map_reference，已修 = 去显式 task_lock） | 10-28 |
| r66 | `cec04b36` | 确定性早期挂死定谳 = 跨 map 失败读撞 AP 启动窗口（CPU1 最后中断入口 ArmCpu::prepareGic+0x19c）；槽 3+ "推进" = 旧会话残影；r67 A/B 件（probe 1→0 免重编） | 10-28 |
| r67 | `ce469e83` | A/B 过 = 探针罪责实锤（probe 0 后 logo+进度条回来，墙原样 sig11 ×400 @0x1800F5FAC）；r68 出件（探针 v3 非侵入化三修） | 10-28 |
| r68 | `3c177c54` | v3 存活 + 病灶槽 165 轮全程干净 vs SB 465 崩同点 → **r63 槽中毒推断被推翻（毒不在 __DATA_DIRTY）**；反汇编定位 __DATA_CONST 三槽；r69 出件（探针 v4 三窗钉定） | 10-28 |
| r69 | `2685ddf2` | 活读判读：三链槽活值 = 文件绑定值+slide 逐位吻合（预注册"全干净+SB 照崩"分支成立）→ 构造瞬态/写路径上游；sig11 符号化终审进 r70；re-tools 升级 read_run；r70 出件（#21 修正 + 寄存器捕获 + W0） | 10-28 |
| r70 | `9b3a26f0` | **#21 修正实锤生效：sig11 @0x1800F5FAC 本代 0 出现（三分支①成立，r63→r69 垃圾族关账）**；换墙 = SB sig6 `NSPlaceholderSet initWithObjects:` nil ×33 同点（调用点被 unwind 撕）；**halt = 非崩溃：空闲睡眠路径（139s timer expired→SLEEP_STATE）撞 ArmCpu::haltCPU diagnosePower 诊断 panic**；W0 dirty 窗全程未 fresh（dirty 读恒败，遗留）；r71 = nil 定位 + 睡眠处置 | 刷写+判读 10-28 |
| r71 | `89e74a92` | **noidle=1 实锤生效关账**（607s 连续写零 halt + PMRD 无 sleep timer expired，用户 10 分钟无停机实证）+ dirty 窗 R0 全窗读通 + **sig11=0 保持治愈** + 回归六项全绿（applied 21/AKS 1139/vsync/SWAP/spawn 1031）；SB sig6 ×347 原点原 REASON（dispatch_once_callout 块内）+ **PosterBoard sig6 ×167 新客**（'configuration' 断言，外层 dispatch_lane_serial_drain）；SS 扫描 266 行候选全 = 终结链帧+`<unmapped>` 数据指针 → ≤9 预算病灶 tear zone 未及 → r72 扫描器三改 | 刷写+判读 10-28 |
| r72 | `e5ae4ca9` | **扫描器三改 + 体系最大突破：nil 调用者指令级钉死**（内核 4b6bfc38）：bt 排除+真去重+bl 门把 tear zone 第一回打捞上岸 → `-[SBIconController initWithMainDisplayWindowScene:…]→widgetMetricsServer→lockScreenManager→+[SBLockScreenManager _sharedInstanceCreateIfNeeded:] dispatch_once 块`（sharedApplication 非空走正常构造，管理器 init 内 NSPlaceholderSet nil 抛出；断言孪生 SBLockScreenManager.m:362 零捕获）；展开穿 +initialize→PendingInitialize FatalCorruptHashTables 双抛；PB 同链独立墙（'configuration'）；回归六项全绿、sig11 CRASH 0（PROC 聚合行 sig11 非墙）；坑沉淀：手工去滑须 python（手算错进位差点误判）+ bl 门候选需 disasm 复核（LSDA 可撞编码） | 刷写+判读 10-28 |
| r73 | `a3f310d4` | **分支①成立：#26 实锤生效**（内核 ce34e9ae；窗 451ed5c2 301/509 uptime=609s）：applied 21→22 + NSPlaceholderSet 断言 0 出现（nil-set 墙关账）；**换墙 = SB sig6 ×266 'you can't call this before [SBMainWorkspace start]'**；回归六项全绿 + sig11=0；同日二判修正初版"raiser 在缓存外"（扫描器 add 掩码 bug，§4.9）→ raiser 实为 fw 内 SBMainWorkspace（见 r74 行） | 刷写+判读+RE 修正 10-28 |
| r74 | `b0f2f656` | **分支①成立：#27 实锤生效**（内核 ce34e9ae 代；窗 344e72e5 325/509 uptime=650s）：workspace 门 0 出现 + applied 22→23；**换墙 = SB sig6 `CALayerInvalidGeometry` 'CALayer position contains NaN'（WGShortLookStyleButton）×~296——SB 首次死在真实视图构造层**（NaN = displayless 0 scale/bounds 几何；屏面 2:30 进度条出现但冻结 = SB 画真 boot progress UI 后视图布局循环死）；PB ×89（减半）；回归六项全绿 + sig11 0 + noidle；抛点 = `CA::Layer::set_position`；**决策规则判定 = 有进展，不触发换内核**；r75 = #28 | 刷写+判读+出件 10-28 |
| r75 | `23868de1` | **fixup #28 出件**：`CA::Layer::set_position` audit-exempt `cbz w0`→`b`（.03,26850c4,40090034,4a000014，expected 实读核验，零入边）——强制 NaN position 走 Apple 自带清洗（log + NaN→0 + w2=0 递归不抛）；内核不变 ce34e9ae 代；ESP 四样过（28 条 fixup 对拍零缺失）；判读预注册 = §7.2（NaN 0 出现 + applied 23→24 = 生效；屏面观察点 = 进度条走动） | 出件待刷 10-28 |
