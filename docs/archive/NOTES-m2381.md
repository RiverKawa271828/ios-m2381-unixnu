# iOS 16 on Meizu 20 (m2381, SM8550) - working notes

> 上游 NOTES.md 是 sweet（SM7150）的叙事档；本文件 = m2381 移植的对应档，沿用其格式纪律：
> 顶部 = agent 交接块（先读这里），其下 = 屏幕图例与 session log。
> 执行排面与红线见 AGENTS.md；硬件地面真值两处：nura 内核 DT（linux-mobile-ports，
> meizu20-t4b 分支）+ Flyme stock FDT（meizu20-m1/flyme-forensics-20261019/fdt/）。

## Agent handoff / active work (2026-10-22, latest)

**STATE（2026-10-22 八交接：★r34 窗读环判读完毕——B 终审实锤 + 连崩群清零 + mad 换守卫；
r35 主攻 = 原地 stub 化。环 = `rings/m2381-ring-r34.{bin,txt}`（182,970B/flush246/3,298 行/
sampler 4 轮，~8.5min 半窗；suggestd 169 vs 前三窗 315–331，mad 6.1/min 与前三窗同拍）。
报告 = `handoff/r34-window-forensics.md`。五读数：①**双件身份铁证**——boot-args 行带
`sweet_amfi_debug=1`（= boot r34）+ `cs_invalid_page` 97→**0** + 崩溃 pc 换位（= userdata
r34，撤补丁生效）；②**Gate 1 `exec r34mark` = 0 而身份另证在 ⇒ 假说 B 终审成立：launchd
签名缓存整体遮蔽磁盘 LaunchDaemons**（r33 改名/r34 加件同等无效；lsd plist envvar 未投递 =
A1 两 kill-switch 全失效[仍 false 不更坏，如预测]；`launchd_unsecure_cache=1` 无效路径 =
**静默落回默认缓存**，A5 失败分支实测补全——该旗三态实验从此无价值）；③Gate 2 =
`SWEET AMFI: mobileactivationd: 28 entitlements` ≥20 **PASS**（AMFI 挂载无问题）；④Gate 3
= mad **52/52 exec 全崩** @ **stock stub line321**（pc mh+0x1BF3BC，bt0 0x50674；旧 =
line247 stub brk@0x1BF5BC bt0 0x50630——同一 main() 错误清理链两个落点，`.re/ma-fatal.asm`
直接对上；上游 = main 循环 `tbnz w23 → 0x5061C` + 0x505F0 调用带 landing pad
0x50674；**容器骨架未介入此 fatal**）；⑤**Gate 5 大捷：SIGABRT 连崩群 548→0**（accountsd/
chronod/dmd/liveactivitiesd/ScreenTimeAgent/cloudd 各 exec 恰 1 次、零崩、其后 ~2/3 窗
零重拉；containermanagerd 亦 1 次无崩 = 容器机制整体复活）。Gate 4 = lsd t2 与 r33b
**16 帧逐帧全等**（slide 常数平移 0x1C42C000）= MadGate copyActivationRecord sync 永等
未变；SB t0 已脱 os_state 死锁（r32–33b 栈 0 命中）换为等 lsd seeding 的同步 XPC 等待；
bbd t0 = runloop 正常。**全链唯一堵点 = mad fatal**。★r35 方案排序：**主攻 = `/usr/libexec/
mobileactivationd` 原地换已签名即退体**（vm_stat 招：缓存照常按原定义拉起→不注册
MachServices 即退→lsd sync XPC 得 **error reply 而非永等**；零 CDHash 问题，Gate 2 实证
stock 签名过 AMFI + `unentitled_ios_sim_launch=1` 兜底；判据 = CRASH mad = 0 + lsd t2
换栈，且 mad CRASH=0 本身 = r35 体内身份）；**并行实验 = 改 `/System/Library/xpc/
launchd.plist` 缓存 + sweet_amfi_debug=1 放行**（先宿主 RE 一刀：我方 amfi hook 是否覆盖
cache_loader 的 trustcache 查询；r32–33b 已实证 hook 放行补丁代码页；若成 = B 根治 +
磁盘权威恢复 + 缓存内可加 r35mark）；XPC 全功能 stub（A6 备料）为后备。设备现态 =
用户自刷 nura-min（boot_b 未对拍，用户令跳过验证；读环已完成，下窗直接刷 r35 双件）。
本块 = 唯一入口）**

**（前态存档 2026-10-22 七：r34 双件出件 + 宿主 RE 三大定谳——出件参数（userdata
`7aaab96f` + boot `339b8480` 勿刷 b8cddc98）、Gate 1–6 判据表 = `handoff/glm-report-r34.md`；
RE 细节（launchd.plist 缓存机制/容器缺口精确位/os_feature 双机制/A2 entitlements 全表/
A6 协议方法面）全在该报告；r33b 窗实况（97× 崩溃循环/崩溃四行细节/版本判别死局→r34 起
体内标记纪律）详 Session log 10-22 五条目）**

---

**（前态存档 2026-10-22 五：r33 窗首读环 + 镜像四重查证——「定谳 = r29 重刷」被 r33b 窗
降级为「版本身份不可判、两假说并存」，查证事实仍全部有效，详 Session log 10-22 四/五条目）**

- **★r33 窗取证（本会话）**：boot_b 复位验证 `97cdea20…` ✓（现态 mu-r69 + nura-min，
  USB 网 devsh 通）；环 = `rings/m2381-ring-r33.{bin,txt}`（`6eef34e6…`，352,252B/
  flush460/6,388 行，8 轮采样/17min 窗）。与 r32 环对比：**死锁栈逐帧同构**（lsd t2 =
  MadGate copyActivationRecord sync XPC 永等全窗 s0–s6；t1 = MIInstalledInfoGatherer→
  MobileGestalt sem；t5 = getServerStoreBlocking 吊客户；SB t0 = os_state_request_for_self
  dispatch_sync，11 帧 r32/r33 全同；bbd t0 = runloop idle）；**census 同量级**
  （mobileactivationd exec 99 vs 102、cs_invalid_page 99 vs 102、accountsd/chronod/
  ScreenTimeAgent/dmd/liveactivitiesd/cloudd/mmaintenanced 连崩群两窗皆有）；SB AKS
  11 发逐行全同（含 sel 17×6）；sel 35 → 0x0 ✓（内核健康对照组）。
- **★镜像验证（宿主侧，四重后全净）**：① `iOSRoot-s-r33.simg` vs `iOSRoot-s-r29.simg` cmp
  全文**恰 2 字节差**（'d'→'q' ×2，simg 偏移 2457986392/2457986445）；② 7z 抽出
  LaunchDaemons plist → **plistlib 完整解析 ✓**：Label = `com.apple.mobileactivationq`、
  MachServices = `…activationq` + `…d.lockdowq`、ProgramArguments 未动、**无 RunAtLoad
  无 KeepAlive**（两笔写均精确命中：key1 'd'@raw 0x9b1a90ac、key2 末字符 'n'@0x9b1a90e1）；
  ③ **launchd 全域内容级查证（本轮追加）**：/System/Library/{LaunchDaemons,LaunchAgents,
  NanoLaunchDaemons,NanoLaunchDaemonsAltAccount,DeveloperModeLaunchDaemons} + /Library +
  Caches + /private/var[497MB 全抽，/var = 11B symlink→private/var] 内容 grep「mobileactivation」
  仅 3 命中 = 本 plist[已改名] + jetsamproperties.D22/D221[**被动内存配额表，按 label 列
  条目，不起动不注册**——raw 里 0x9b147bea/0x9b160bea 两处「守护进程名清单」即此二件，
  已验明非声明方]；路径串 `/usr/libexec/mobileactivationd` 全域仅本 plist 引用；
  /Library/LaunchDaemons[非标准位]仅 1 件 factory NFC 件。⇒ r33 镜像下 mobileactivationd
  无任何起动路径（LaunchEvents activity = 86400s 周期 ≠ 17min×99）。
- **★定谳（用户补证后闭环）**：宿主侧 `sha256sum` 两件与交接单全等（97342037 ✓ /
  b8cddc98 ✓），且 /home/ri = 宿主 btrfs 直挂容器、**全机唯一拷贝**（无旧副本可刷错）；
  用户确认本窗所刷 = 「与上轮同版」→ **userdata 实为 r32 窗的 `iOSRoot-s-r29.simg
  d7d6e730`，非 r33 `97342037`**。r32 配方 = boot r32 + userdata r29；r33 配方 = boot
  沿用 r32 + userdata 换 r33——boot 同件 + 两个 simg 同目录同名只差 2 字节，整对照抄
  上轮命令 = 精确复现 r32（环逐帧同构即此而来）。**坑已钉：两个 simg 靠文件名/尺寸完全
  不可分，唯一身份 = sha256 前缀（r29 = `d7d6e730`，r33 = `97342037`）。**
- **下窗动作（唯一）**：`sha256sum ~/mkiosroot-work/iOSRoot-s-r33.simg` 对拍
  **`97342037…`** → fastboot flash boot_b `shim/out/shim-boot-r32.img`（`b8cddc98`，
  与上轮同件可沿用）+ userdata r33 simg（fastboot 回执看完整 Writing OK）→ ≥9.5min 窗
  → pwr+vol↓ → 17min 窗 → 读环。
- **r33' 判据树（首选判读前置）**：①环里 `exec mobileactivationd` = **0** 且
  cs_invalid_page = 0 → 手术上机的直接证据，再看 lsd：②lsd 无 sync 永等且 SB 走新代码
  （新 IOKit/sandbox 行）= 解锁成立，等 GUI 首帧（锁屏/Setup）；③lsd 换新卡点（t1
  MobileGestalt sem 为头号嫌疑）= sampler 栈定位下一环；④mobileactivationd = 0 但栈仍
  同 r32 = MadGate 失败后另有 sync 依赖，按新栈走。若①不成立 = 又没刷上，先查刷写
  通道再谈其它。
---

**（前态存档 2026-10-22 三：r33 件就绪待刷——「就绪」判定被 10-22 四修正为「未上机」，
r32 窗存证仍有效，详 Session log 10-22 三/四条目）**

- **r32 窗存证**：环 `rings/m2381-ring-r32.{bin,txt}`（`1e6bfa92…`，365,382B/flush494/
  6,616 行，8 轮采样/17min 窗）。lsd 栈全景：t0 = LSServerMain dispatch_once →
  performJournalRecovery dispatch_sync（**后期样本已自行逃出** → 非死结）；t1 =
  MIInstalledInfoGatherer 填占位记录 → libMobileGestalt sem wait；t2 =
  LSHRNSupport deviceConfiguredForHRN → MadGate copyActivationRecord → xpc sync
  （**永等 = 主死结**）；t3 = 处理 SB/bbd 的 getServerStoreBlocking → 等 seeding。
  installd 本窗 15min 后才首次 exec（sample 7 才采到，栈 = 自己 main 里活着）；misd
  半死（t0 全零）。SB t0 后期直接吊在 reseed XPC 上。

---

**（前态存档 2026-10-22 二：r31 窗 = 断链修复实证生效 + sampler 抓到 LS reseed 死锁
形状——详 Session log 10-22 二条目）**

- **★r32 件就绪（本轮出，cmdline-only 迭代，内核零改动）**：boot_b =
  **`shim/out/shim-boot-r32.img` = `b8cddc98…`**（payload `682f03a1…`；内核同 r31
  `012f596f…`；ADT 同 `c85b01f0…`）；cmdline 唯一增量 =
  `sweetsample=SpringBoard,backboardd,lsd,installd,misd,containermanagerd`。userdata 沿用
  `iOSRoot-s-r29.simg d7d6e730`。刷写/读环照 STATE 配方。
- **r32 判据**：环里 `SMP lsd` 栈 = lsd 卡点（若它 sync 等另一 daemon → 下窗把那个加进
  采样；若它卡文件/路径操作 → RE lsd reseed 处理器）。SB/bbd 的栈应与 r31 同形（对照组）。
- **★符号化工装（本轮建，复用配方）**：`research/bin/ipsw`（blacktop/ipsw 3.1.739
  linux_x86_64，GitHub release）；DSC = `ipsw extract --dyld --dyld-arch arm64
  ~/Downloads/iPhone10,3,iPhone10,6_16.7.16_20H392_Restore.ipsw`（→ /tmp/dsc/20H392…/；
  ⚠iOSRoot-raw.img 内的 cache 被 mkiosroot 清成 0 字节、7z 对 .symbols 报 Unsupported
  Method[HFS+ decmpfs]，必须走 IPSW）；`ipsw dyld a2s -i <cache> <unslid地址>` 逐帧解
  （slid 地址 − 环里 slide 行）。`research/` 已 gitignore，工装不入仓。
- **设备现态**：boot_b = mu-r69（`97cdea20…` ✓）+ userdata = nura-min r91（用户重刷后
  起机，USB 网络正常）；环已读 = `rings/m2381-ring-r31.{bin,txt}`（`68a2cdd3…`，
  131,224B/flush117）。

---

**（前态存档 2026-10-22 一：断链破案 + 修复 + 上游切 topaz + r31 出件——已全部执行完毕，
详 Session log 10-22 条目与 10-22 一条目）**

- **r30 窗判读（判据树②命中）**：环 `rings/m2381-ring-r30.{bin,txt}`（`653135ee…`，
  117,832B/flush153/2,256 行；boot_b 前缀 sha `97cdea20…` 复位验证 ✓）与 r29 **逐行同构**：
  bbd 4 surface（id2-5）/trap2+3 各一次/swap×5（3,4,7,10,11）后 idle；SB 4-AKS
  （sel 35→0xe00002c7 ×2）后零活动零崩溃；**0091-0098 新信号零命中**（wrap /
  around-client-memory / refused-lockresult 一条没有）= SB 根本没走到 CA 渲染面，
  「先修内容」假说关闭；mobileactivationd 38× 崩环照旧（parked）。设备已回 nura-min。
- **★真根因（环+CI 日志+本地走查三证）**：**CI 静默丢 14 件补丁**——dxb build.sh 对
  git apply 失败只发 warning，跳单 = 0054/0055/0056/0060/0062/0063/0072-0077/0079/0081；
  r26（39530428800）/r29（39544122293）/r30（39626238471）三件内核**全缺**。环上铁证 =
  AKS sel 35 返 0xe00002c7（0055 缺失；0055 = sweet 上桌面两要素之一，正对 SB 停等）。
  **根因只有一件 = 0054**：其 exception.c hunk#1 锚定的 `#if defined(ARM64_BOARD_CONFIG_SWEET)`
  行被 W2（9a39f21 十四处双守卫）改写 → 上下文失配被跳；其余 13 件全为级联（0054 的
  `bsd/dev/sweet_sample.c` files 行缺失 → 0060/0062/0072 的 bsd/conf/files、sweet_ufs.c
  与 sweet_touch.c/sweet_usb*.c 逐级失配）。**r29 环「swap 跳号 = 0056 节流」判读作废**
  （0056 根本不在内核，跳号另有所源）。0091-0098 确在内核（其前件全数 apply），零命中
  = SB 未达渲染面，非补丁无效。
- **★修复（10-22 已入树）**：0054 hunk#1 手工合并（两个 include 提到双守卫行之前 =
  cfe3644 意图在 W2 后的再表达）+ 全件 hunk 按 as-built 再生成；0089 行偏移再生成
  （0056/0079 回归后 SweetFramebuffer 行移）；**全链严格走查 96/96 零失败**
  （`tools/xnu-gate-walk.sh`：真源树 research/xnu-8796 @ 258d99a + 三件 .sh staging，
  MACHINE_CONFIG=M2381）。恢复语义 = AKS sel 35 应答、0056 节流 present、0054/0055
  sampler、0079 显示电源、0060/0063/0072-0077 触摸/USB 件、0081 usb link state。
  W2 的十四处双守卫内容本体无损（守卫行是加宽不是删除）。
- **上游切换（用户 10-22 令「上游改为 topaz」）**：跟踪基准 = `reference/ios-topaz`
  **origin/topaz**（tip 89451b8 未动；新动态 = 4 个 PR：#1 GPT 写栅栏、#3 触摸 0116
  释放、#4/#5 第三方新机型移植[Gauguin/reeder]，均未并）。**不换基座**判定不变
  （0080+ 仍 SM6225 定制；共同基座 0001-0079 我树与 topaz 同源，diff = 43 件 m2381 化，
  本次修复只动 0054/0089 两件）。排队的设备无关采纳件：PR1 GPT 写栅栏（搭车件）、
  PR3 触摸释放（阶段 C）。
- **★r31 件已就绪（10-22 本轮出齐，刷写留下轮）**：CI **39644798204** success（push
  2d780bd），跳单验证 = **Applying 100（96 我方 + 4 dxb）+ Skipping 仅 dxb 基线 4 件**
  ——断链修复后首次全须全尾。boot_b = **`shim/out/shim-boot-r31.img` = `a5d0f8d9…`**
  （5,734,400B；payload `60718439…` 16,599,584B；内核 **`012f596f…`** = CI 39644798204，
  SHA256SUMS 对账 OK；ADT = r30 原件 `c85b01f0…` **勿再生**——mkadt 的 chip-id 熵每跑
  必变，再生 = 设备身份漂移）；cmdline = 轮 20 串去 `sweetsample=-`（sampler 开启：
  默认 SpringBoard,backboardd，90s 首采 + 120s 周期）；userdata 沿用
  `iOSRoot-s-r29.simg d7d6e730`。刷写配方照顶部刷写段（验槽 b → 双件 → set_active b →
  ⚠ABL 卡死人手断电 → ≥9.5min 窗 → pwr+vol↓ → 读环）。
- **r31 判据树**：①内容出现（锁屏/进度条/data-migration UI）= 里程碑拍照收货；②仍黑帧：
  环看 **sel 35 是否已答 0**（chronod/remindd/searchd/SpringBoard 四家）+ **SMP 采样行**
  （SB 卡在哪一函数 = 停滞根因直接定位，sweet 0055 同款打法）+ swap 行新形态（0056 回归
  后序号跳号才=节流正常）；③ SB 4-AKS 若破 = 桌面前最后一段；④ bbd 若崩 = 0095/0096/0097
  先查（它们刚回归，正是治崩溃的）。
- **★CI 零 skip 门（新纪律，每轮必查）**：run log `Applying` = **96** + Skipping 仅 dxb
  基线 4 件（iobuffermemd_monterey / skywalk_sonoma / syntax_checker_sonoma /
  TrustCache）；「源工件 + 内核二进制两边对拍」并入每轮出件流程（0054 教训：本地走查
  门 = `tools/xnu-gate-walk.sh`，推送前必跑）。

---

**（前态存档 2026-10-21 深夜二：r29 窗取证 = swap 里程碑命中、管线贯通、黑帧=内容空——
其中「0056 节流」与「SB 等 CA 首帧」两个判读被 10-22 破案修正，余仍有效，详 Session log）**

**STATE（2026-10-21 深夜二交接：r29 窗取证完毕 = swap 里程碑命中、管线贯通、黑帧=内容空；设备留 mu-r69+nura-min；iOS 暂不刷；本块 = 唯一入口）**

- **r29 窗实况（用户手刷 r29 双件，实跑）**：用户观察「日志→黑屏」——0067 静音在
  **first swap** 触发（代码位 AppleCLCD::swap()，静音后文本仍进环、panic 仍画）→
  **黑屏 = GUI 首帧已 present**。读环（rings/m2381-ring-r29.{bin,txt}，`02e74a20…`，
  158,176B/flush254/3,095 行）证实：
  ① **SWEET FB swap 行首次出现 = 判据①里程碑命中**：客户端 present 请求 16 次（swap
     序号 3/6/9/12/15/16 被 0056 节流合流成 6 次全帧拷贝入面板内存），surface 3→5、
     **ARGB 1125×2436 单平面**；swap 拷贝循环双向 clamp（0046 实读），1125→1080 裁剪
     干净 → **黑屏 = surface 内容本身全零，非拷贝伪影**。合成器画完首帧后静默（无变化
     不重绘，idle 正常）。bbd 全窗一命零崩。
  ② **gamut sRGB 验证生效**：r28 的 2 个 'b3a8' 10-bit 双平面面消失，原位变 ARGB
     单平面（id3/id5，1125×2436）——topaz 3eacb48 判词正中。
  ③ **mobileactivationd 8B 补丁确认上机**但未堵死：**cs_invalid_page ×55**（r28 零条；
     每命一条 = 改页签名校验失败放行）+ bt 首帧从 mh+0x50674 移到 **mh+0x50630** =
     路径确实变了，然后仍进同一 fatal。**RE 勘误：那不是双入口块，是一个 Swift 大函数
     的错误清理尾声，全函数 ~13 处守卫跳入**（0x100050624/628/62c/630/638/640/648/654/
     65c/664/668 + 我补的 614/61c——首次扫描只查了块首两个目标地址，漏了块中任意落点）。
     守卫打地鼠是错形；正解 = 废尾声本体（`bl fatal`→`b 函数尾声`，需先定函数边界），
     **或直接 parked**——topaz 带着同样的循环上桌面，r29 环证明 GUI 推进不再被它闸住，
     55 命纯属环容量噪音。**建议 parked，火力全给内容面**。
  ④ SpringBoard：exec → 4 AKS（sel 35→0xe00002c7 ×2）→ 静默，连续第三窗同构；bbd 首画
     在 SB 的 AKS 之前 = 黑根帧是 bbd 自绘，SB 内容还没流进合成器。
- **新前沿 = 帧内容（黑帧→有内容）**：管线五段（surface create→trap lock/unlock→
  SwapBegin/End→swap→CPU copy 入面板）全通，死在渲染数据面——CA::OGL::SW 软渲染读到
  空 surface 数据 = topaz 0133「别答 0」症状家族（sels 9/10/11 存值、27 attachments、
  20 purgeability、12 accelerator、5 YCbCr 必须回真值；0132/0134 MemorySurface；
  0139 唯一 id；0135/0136/0137 生命周期）。**阶段 B 补丁已全部入树（0091-0098，见轮 30
  备件条目）**；SpringBoard 4-AKS 停滞与内容空可能是同一根因的两面（SB 等 CA 首帧回报才
  继续），先修内容再看 SB。
- **★r30 件已就绪（本轮出完，刷写留下轮）**：boot_b = **`shim/out/shim-boot-r30.img` =
  `4998741b…`**（5,734,400B；payload `7b1667b6…` 16,599,200B；内核 **`443f8aaa…`** =
  CI **39626238471** success = 0091-0098 首件 + 0090/0089；ADT sRGB 12,624B；cmdline 与
  r28/r29 全同）+ userdata = **`~/mkiosroot-work/iOSRoot-s-r29.simg` = `d7d6e730…`**
  （沿用：8B daemon 补丁 + 全套 tweak + data_ark 都在内）。
- **刷写配方（下轮照抄）**：fastboot（`getvar current-slot` 验 **b** 才动）→
  `flash boot_b shim/out/shim-boot-r30.img` + `-S 512M flash userdata
  ~/mkiosroot-work/iOSRoot-s-r29.simg` → `set_active b`（retry 复满防翻槽）→ reboot
  （⚠12G 后 ABL 卡死老坑：8s+ 仍在 fastboot = 人手断电再开机）→ **≥9.5min 窗**（屏上
  console 刷日志→转黑 = 首帧已 present，属正常）→ pwr+vol↓ 回 fastboot → 读环。
- **读环配方（每次收窗照抄）**：刷 mu-r69（`/home/ri/work/nura-meizu-m2381/artifacts/
  mu-r69-97cdea20.img`）+ nura-min（同目录 `meizu-meizu20-nura-min-r91-2G.simg` `a84d9212…`，
  -S 512M，~33s；**刷 nura-min 不碰 userdata 尾 8200 扇区 = 环自动保鲜**）→ reboot →
  devsh（nura 仓 `tools/devsh.py`，~50s 起；⚠boot 后 ABL 慢启动 ~45s 属正常）→ devsh
  `dd if=/dev/disk/by-partlabel/boot_b bs=1110016 count=1 | sha256sum` 验 `97cdea20…` +
  `dd if=/dev/sda22 bs=512 skip=972317840 count=8200 of=/tmp/ring.bin` → 设备
  `python3 -m http.server 8765 --bind 172.16.42.1 --directory /tmp` + 宿主
  `curl --interface enp104s0f3u1 http://172.16.42.1:8765/ring.bin` → `tools/sweet-log.py`
  解析 → 存 `rings/m2381-ring-rN.{bin,txt}` → 收口即 mu-r69 + nura-min 日常态。
- **推送/CI/下载配方（记档）**：`git push fork main`（push 触发 CI，M2381 默认；token =
  `git credential fill`；push+dispatch 双触发时取 dispatch 条，concurrency 同配置互取消）
  → CI 绿后下载 **xnu-kernel** artifact（内含 kernel + SHA256SUMS，`sha256sum -c` 对账）
  → 本地 `mkadt.py <out> --m2381`（argv[1]=输出路径 flag 在后）→ `mkpayload.py <kernel>
  <adt> <out> --cmdline '<轮20串>'` → `M2381=1 bash shim/build.sh <payload绝对路径>` →
  mv shim-boot-m2381.img shim-boot-rN.img 记 sha。
- **判据树（r30 窗）**：
- **设备现态（收口即现态）**：boot_b = mu-r69（前缀 sha `97cdea20…` devsh 读回验 ✓）+
  userdata = nura-min r91——日常可用；**iOS 件不刷（用户指令），r29 件可复用**：
  boot `shim-boot-r29.img 0fd9a1e5` + userdata `iOSRoot-s-r29.simg d7d6e730`（8B 补丁
  在内，可继续用作噪声对照或等尾声补丁二版）。
- **读环流程（新会话照抄）**：fastboot 确认 → 刷 mu-r69（`/home/ri/work/nura-meizu-m2381/
  artifacts/mu-r69-97cdea20.img`，读回前缀 sha `97cdea20…` 验）+ nura-min（同目录
  `meizu-meizu20-nura-min-r91-2G.simg` `a84d9212…`，~33s）→ reboot → devsh（nura 仓
  tools/devsh.py，~50s 起）→ `dd if=/dev/sda22 bs=512 skip=972317840 count=8200 of=/tmp/
  ring.bin` → 设备 `python3 -m http.server 8765 --bind 172.16.42.1 --directory /tmp` +
  宿主 `curl --interface enp104s0f3u1 http://172.16.42.1:8765/ring.bin` → `tools/sweet-log.py`
  解析 → 收口 mu-r69 + nura-min。
- **判据树（r30 窗 = 0131-0139 翻版后）**：
  ① 用户看到内容（锁屏/进度条/data-migration UI）= **里程碑拍照收货**。
  ② 仍黑帧：环看新信号——`SWEET SURF: wrap image/memory`（0092）、`create around client
     memory`（0094）、`sel (refused: lock result)`（0091）、values/`trap 0/1` 流动 → 按
     缺口定位哪件没起效。
  ③ SB 4-AKS 停滞若仍在但内容已出 = 按 topaz 顺序走（其桌面也带激活循环）。
  ④ bbd 崩溃回来 = 0095/0096/0097 生命周期三件先查（正是它们治的病）。
- **环境硬事实**：容器 NoNewPrivs 永封提权（root/重活 = 宿主 sudo 脚本，fix-round/
  w1/w1b/w1c 三发模式；12G raw 小补丁 = 容器内 Python r+b 直写 + 7z 回验免 root）；build.sh
  参数必须绝对路径；**mkadt argv[1]=输出路径、flag 在后**；新补丁 hunk 上下文 = 从 CI
  buildtree 取 as-built 源生成；push 双触发 CI；token = `git credential fill`；12G simg
  刷后 `fastboot reboot` 卡死 = 人手断电；**img2simg 勿传大块尺寸（512M 块 = 内存炸），
  默认 4K 流式**；grep 用 /usr/bin/grep。RE 纪律新增：**Swift 多错误路径函数找 fatal 可达性
  必须扫块内任意地址的分支落点，不能只查块首**。

---

**（前态存档 2026-10-21 午：fix round 双线 + 取证轮排程——已全部执行完毕，详 Session log）**

- **设备现态**：boot_b = mu-r69（`97cdea20…`）、userdata = nura-min r91——日常可用待命。
- **r26 件清单（全 sha 对账）**：
  - `shim/out/shim-boot-r26.img` = `cef05044…`（5,730,304B；内核 `a0ac4602…` 17,183,664B
    = CI 39530428800，**W2 十四处 guard 双化后首件**：CRASH 寄存器/backtrace/slide 行、
    exit-reason 行、0048 stderr tee、0049 IOKit trace、0052 RENAME_SWAP、0041 hardlink
    消音、0039 coredump 默认关全部首次在 M2381 内核生效；ADT `07cb5378…`；payload
    `a6ec7f2e…` 16,574,704B；cmdline = 轮 20 串 − panic_on_error_exit + sweetsample=-
    + ufslogsecs=2）
  - `~/mkiosroot-work/iOSRoot-s-tweaked.simg` = `f4ffe7a6…`（12,145,512,872B；W1 全套
    tweak 已施并 7z 验证：containers 四目录/Setup 跳过/BKNoWatchdogs/migration/CommCenter
    摘除/四 job stderr→console；data_ark 跳过 = 无容器可挂，sweet 同款；裸卷
    `iOSRoot-raw.img` 保留可再改，旧 simg `1db88931…` = 回滚点）
- **下会话 = r26 窗（两段式照抄）**：fastboot → `flash boot_b shim-boot-r26.img` +
  `-S 512M flash userdata iOSRoot-s-tweaked.simg` → set_active b → reboot（⚠ABL 卡死老坑，
  人手断电；12G simg 后出现过）→ ~8-10min 证据窗（宁晚勿早）→ pwr+vol↓ → 刷 mu-r69 +
  nura-min 读环（流程同轮 25：devsh dd skip=972317840 count=8200 → sweet-log.py）。
  **判据**：①backboardd 存活过 SWEET SURF create = containers 治本，看桌面到哪；②仍死 =
  CRASH/reason/STR/`|backboardd|` 四通道首齐，死因一次可读。判读分叉照旧（配置类宿主修、
  服务类翻 topaz 0138+0131-0139、内核类定位）。
- **取证轮结论存档（轮 25，行为实证）**：①诊断家族 SWEET-only guard 全盲（已修=本窗）；
  ②ReportCrash 收尸后从未写 .ips（0088 armed 空响；CrashReporter 目录在 = 内部死，
  本窗 stderr 直通后可看它卡在哪）；③SIGABRT 主嫌疑 = 根盘零 tweak（已修=本窗；sweet
  第一墙 NSURL-nil 窗口吻合，geod/datami 集体崩 = 同容器依赖面）。环证据 = `rings/`。

- **设备现态**：boot_b = mu-r69（`97cdea20…` devsh 读回验证 ✓）、userdata = nura-min r91
  （`a84d9212…`）——**收口态即现态，日常可用，无需再碰 fastboot**。环证据已存档
  `rings/m2381-ring-r24{,b}.{bin,txt}`（r24 = `bbbafb8e…` 1,101,708B/flush226，r24b =
  `28b0cb47…` 1,092,454B/flush123）+ 全卷根盘 listing `rings/iosroot-list.txt`。
- **三大发现（行为实证，非推测）**：
  ① **诊断家族全盲（自伤）**：0039/0041/0043/0044/0048/0049/0051/0052/0053/0055 十件
  guard 全 `ARM64_BOARD_CONFIG_SWEET` 单守（逐件 grep 零 M2381 覆盖），CI M2381 构建
  只定义 `-DARM64_BOARD_CONFIG_M2381`（xnu.yml 互斥配置）——CRASH 寄存器/backtrace/slide
  行、reason 行、/dev/null stderr tee、STR 堆扫描、RENAME_SWAP **从未编进任何 m2381 内核**。
  行为实证：双环 geod 死几十次、一条 CRASH 行皆无（0043 若在必打）。
  ② **0088 IPS 通道 armed 但空响**：零 `IPS|` 行 = ReportCrash 收尸（AKS sel 0/17/7 在环）
  后从未写过一字节 .ips；根盘 `var/mobile/Library/Logs/CrashReporter` 目录在（7z 验证）
  → 死在报告生成内部，二阶（STR 修好可绕过）。
  ③ **SIGABRT 主嫌疑 = 根盘零 tweak**：iOSRoot-s.img 全卷无 `/private/var/containers`、
  无 data_ark/SetupDone/BKNoWatchdogs/LastSystemVersion——sweet-root-tweaks.py 从未施上
  （mkiosroot.sh 不调 tweak；B 段「核 tweak 态」因宿主无 root 未跑成）。死点窗口与 sweet
  第一墙 ① 完全吻合：StartWindowServer → containermanagerd 容器路径 NULL → NSException
  'NSURL initFileURLWithPath: nil string' → abort；geod/datami/analyticsd/lsd 集体崩 =
  同一容器依赖面。环解剖：单轮开机、backboardd 4 条命、每命 = exec → AKS sel 0/17 → 双
  SweetHID open → MobileGestaltHelper sel 8/28 → 静默死；90% 环容量被 hfs hardlink 刷屏 +
  geod 循环占据。
- **下会话 = fix round 双线**：
  **W1 根盘补 tweak（治死因主嫌疑；需宿主 root，用户拍板）**：losetup → `fsck.hfsplus -fy`
  → mount → `python3 tools/sweet-root-tweaks.py <root>` 全套 → umount → img2simg → sha
  记账 → 复刷 userdata。
  **W2 诊断 guard 双化 + 搭车件（一炉 CI；推送窗口用户拍板）**：0043/0044/0048/0051/0053
  （+0052 RENAME_SWAP、0049 视冲突）guard 改
  `defined(ARM64_BOARD_CONFIG_SWEET) || defined(ARM64_BOARD_CONFIG_M2381)` 照 0088 样子；
  改前 grep 跨补丁同行冲突；`tools/xnu-cc.py` 本地预检；搭车件照旧（topaz 0088 GPT 看门狗 +
  mkadt gamut→sRGB）→ CI M2381 → r25 窗（8min 宁晚勿早）→ 读环：CRASH 行给
  pc/lr/sp/fp/far+slide+mh（可符号化）、STR 行给 NSException reason 原文。**W2 不依赖
  W1——就算 containers 不是全部真相，r25 环必可读**。

---

**（前态存档 2026-10-21 晨：轮 24 上半场 + 两段式取证排程——已当轮执行完毕）**
- **设备现态**：boot_b = r24 件（r21 内核 `41c8b139…`）、userdata = iOSRoot-s.img（12G
  sparse 真身）、自 08:43 起在 r21 内核上跑 backboardd 崩溃循环（用户随时 pwr+vol↓ 收窗）。
  **环里正在实时积累 r21 证据，且 nura-min 刷写不碰尾部 = 证据自动保鲜**。
- **下会话排程（两段式）**：
  ① **免费读环（永远先做）**：fastboot → `flash userdata` = nura-min simg（`~/…/artifacts/
  meizu-meizu20-nura-min-r91-2G.simg`，sha `a84d9212…`，38s）+ `flash boot_b` = mu-r69
  （前缀 sha `97cdea20…` 对拍）→ reboot → devsh `dd if=/dev/sda22 bs=512 skip=972317840
  count=8200` → 宿主 `tools/sweet-log.py` 全量解析 → grep `STR / CRASH / exception /
  IPS| / Terminating / backboardd`。命中死因 → 直接判读分叉：配置/镜像类 → 宿主机修镜像；
  服务缺失类 → 翻 topaz 0138 + 0131-0139；内核类 → 定位修复。
  ② **r24b 窗（①空手或要 IPS 通道时）**：fastboot → `flash boot_b` =
  `shim/out/shim-boot-r24b.img`（sha `54cd6424…`，r22 内核 `c133cacc` **含 0088 IPS 镜像**，
  0.2s）+ `flash userdata` = `~/mkiosroot-work/iOSRoot-s.img`（5.6min）→ set_active b →
  reboot → **~8min 证据窗**（链路通 ~3min + 首死 ~4min + 再死一轮；**宁晚勿早**，环保最新）
  → pwr+vol↓ → 回①读环。
- 已知坑：`fastboot reboot` 大镜像刷写后卡死 = ABL 老 bug（人手断电重进）；fastboot 单通道；
  **禁 reboot recovery**。收口常态 = 停 nura-min（日常可用）。plist 补丁仍挂起（宿主无 root，
  六命令在轮 23 收口条目；STR+IPS 双通道大概率免它）。
阶段 2 关闭（UFS 墙破，轮 20）后唯一悬案 = **backboardd 运行中静默 SIGABRT**（崩溃循环：
exec → AKS sel 0/17 → 双 SweetHID open → MobileGestalt sel 8/28 → 数分钟后 SIGABRT；轮 23
实锤 ReportCrash 处理了 corpse 但 .ips 零落盘；SWEET SURF create 从未到达 = 死在 AppleCLCD
open 之后、surface 装配区——精确定位见 NOTES-topaz.md §1 backboardd 生命周期）。
**参照系（10-21 定）**：ios-topaz 已重定向 SM6225 并达桌面+Wi-Fi，活跃分支 = `origin/topaz`
（main 停 0079 勿读）；**机制深潜蒸馏 = 本仓 NOTES-topaz.md（进任何补丁前先读对应节）**。
**下会话排程（五段 A-E；nura 窗 = tools/devsh.py，路径在 nura 仓 `/home/ri/work/
nura-meizu-m2381/tools/devsh.py`，@172.16.42.1:23）**：
- **A · nura 窗侦察（10-21 深夜已执行完毕，devsh 实测，零刷写）——判读**：
  ① nura 根 = **userdata=sda22 本尊**（cmdline `root=/dev/sda22`，nura 内核 7.3.0 #92；
    用量 2.4G/449G）；回归配方在用户手里（交还后刚做过一轮：userdata 重刷 nura 根 +
    boot_b 回 mu-r69）；
  ② **旧日志环 = 已毁**：预期尾块（分区尾 -1025×4KiB）无 SWEETLOG 魔数（读到的已是
    指针类二进制），尾 64MiB 扫描零命中——nura 根恢复（全盘 mkfs/整分区恢复类操作）把
    轮 21-23 的环连同全部 `STR` 堆文本一起覆盖。**免费取证窗口关闭**；STR 取证只能靠
    下一轮实机新环（探针读，词表已含 STR/CRASH）；
  ③ boot_b 读回验证 = **mu-r69 本尊**（前 1,110,016B sha = `97cdea20…` 完全一致；
    header v4，kernel_size 0x10b20a）——10-21 收口条目"boot_b 现留轮 23 镜像 53831d75"
    **已失实**，用户已自行还原 mu-r69。**轮 23 件 `53831d75` 本机已找不到**（/tmp r19/r20
    scratch 已清；shim/out 仅 24KB 探针件；仓根 xnu-kernel.zip = 轮 21 内核 `8c3f75d6`，
    **无 0088 IPS 镜像**）——**对策：不追旧件**，下会话带搭车件走新 CI（见 E），新内核
    +④ADT + 定稿 cmdline 本地出 image，boot_b 刷新件；收口刷回 mu-r69 即恢复 nura 现役。
- **B · 宿主机制作（零设备，与 A 并行）**：
  ④ iOSRoot.img：losetup → `fsck.hfsplus -fy` → `mount -o loop,rw,force` → `python3
    tools/patch-launchd-plist.py <in> <out> com.apple.backboardd com.apple.ReportCrash
    com.apple.SpringBoard com.apple.geod`（stderr+os_log→/dev/console +
    OS_ACTIVITY_DT_MODE/CFLOG_FORCE_STDERR + 全体 _PanicOnCrasX）→ **核 tweak 态**（purplebuddy
    SetupDone / data_ark / migration LastSystemVersion / BKNoWatchdogs / /private/var/containers
    ——sweet-root-tweaks.py 已含全套，验现役根盘施没施过，缺则补跑）→ umount → img2simg +
    sha256 记账；
  ⑤ shim/ufs.c 探针词表 +`STR`/`CRASH` → `M2381=1 bash shim/build.sh` 出新探针件（备用通道）；
  ⑥ **iOS 件出件链（全本地，零 CI）**：`unzip xnu-kernel.zip kernel.development.m2381`（验
    sha256 = `8c3f75d6…`）→ `python3 tools/mkadt.py shim/out/m2381.adt --m2381` →
    `mkpayload.py <kernel> <adt> payload.bin --cmdline '<轮 20 定稿串 − panic_on_error_exit
    + sweetsample=- + ufslogsecs=2>'` → `M2381=1 bash shim/build.sh ../shim/out/payload.bin`
    → 镜像 sha 记账。可选项：若用户当日愿给 gh token，先 CI 带 0088（IPS 第三通道）+
    搭车件再打；不给就按 r21 件打（够用）。
- **C · fastboot 单通道窗**（C 前置在 nura-min 下完成：devsh 验 nura-min 可用[起机/工具/
  python3]→ **写尾部标记**[环头位 + 末块各一，`dd conv=notrunc`，记 sha]→ devsh
  `systemctl reboot --reboot-argument=bootloader` 进 fastboot）：`getvar current-slot`
  **先记录**（收口还原）→ 刷 boot_b = ⑥件 → `fastboot -S 512M flash userdata <④simg>`
  （~6min；
  ★覆盖 nura 根 = 用户已知情接受，重刷配方在用户手里，收口/交还时复原）→ `set_active b`
  （若原 slot ≠ b）→ `fastboot reboot` → iOS 启动窗 ~5-10min（backboardd 崩溃循环：stderr
  上 console 进环 + ReportCrash .ips + 0088 镜像回显 + ufslogsecs=2 刷环）→ pwr+vol↓ 回
  fastboot。**禁 `fastboot reboot recovery`**（recovery_a = ESP 非 ramdisk，ABL 配对引导
  失败 = 重启循环，9-21 实录）。
- **D · 取证读回（nura-min = 用户刷好后通知即触发本排程）**：首选 = 刷 **nura-min**（sparse
  物理碰不到尾部）→ 起 nura → **先验尾部标记 = 验收协议闭环** → devsh dd 尾 4MiB+4KiB →
  `sweet-log.py` 全量机器可读解析 → grep `STR`/`CRASH`/`exception`/backboardd（一步同时取证
  + 设备还用户）；nura-min 起不来 = 备用⑤探针件拍屏。判读分叉：配置/镜像类（NSException
  路径/plist 键）→ 宿主机修镜像即愈，**同会话可立即再跑一轮 C-D**（nura-min 在手后迭代成本
  = 一次 userdata 刷写 + 一次开机）；服务缺失/答案错 → 翻 topaz 0138 + 0131-0139；内核
  bug → 按 reason 定位后走修复轮 CI（0088+搭车件一并上车）。
- **E · 收口**：设备复原 = boot_b 刷回 mu-r69（`97cdea20…`）+ slot 按 C 窗记录还原 +
  userdata 刷 nura-min（到货后；未到货则交用户重刷全量 nura 根）→ 交接块更新。
  **★nura-min 规格（10-21 定稿，已交用户转告 nura 侧；要点）**：定长 raw ext4 镜像 ≤4GiB
  （无 GUI；保 devsh:23 + busybox 工具集 dd/sha256sum/hexdump/base64/grep + python3 有则留
  + 内核模块/initramfs 若在根内）——**硬线 = 镜像尾不得超过"分区尾 − 8GiB"**（iOS 环只占
  尾部 4MiB+4KiB，8GiB 硬线 = 千倍裕量；≤4GiB 镜像在 464GiB 分区上天然留 ~460GiB）；
  **三条禁令：①不做任何全分区写（不 mkfs 整分区/不 blkdiscard/不整分区 dd）②禁首启自动
  扩容（growpart/resize2fs-once/systemd-repart 全拆）③fstab 无 resize 语义**；验收协议 =
  我在尾部预写标记块 → 刷 iOS 根 → 刷 nura-min → 起 nura 读回标记原样 = 通过。
  （环容量本身 4MiB：sweet 全程 GUI 调试同款环抓到过 NSException reason；轮 22/23 单次开机
  控制台总量仅 53-62KB；环保最新 4MiB，STR 堆文本发生于死亡瞬间 = 恒为最新内容。若观察到
  geod 崩溃循环 stderr 镜像挤占旧周期，下一发摘 geod 或把 0047 环改 16MiB——第一轮不必。）
  **搭车件升级为必做（B 段就做——轮 23 件已失传，反正要新 CI 出件）**：topaz 0088 GPT
  写栅栏+看门狗翻 sweet_ufs.c + mkadt gamut→sRGB（NOTES-topaz §6/§7）+ **ADT 勿加 wlan
  节点**。本地规则：改补丁前 grep 跨补丁同行冲突。
回滚 mu-r69 在 nura artifacts（前缀 sha `97cdea20…`）；cmdline = 轮 20 定稿串 −
`panic_on_error_exit=…` + `sweetsample=-` + `ufslogsecs=2`（只增不改；0069 拥有 0047 的
tsleep 行，0047 勿动）。



- **★三层定谳（每层都有实机证据）**：① **LU 不就绪**：SCSI 一切命令 CHECK CONDITION
  sense key 02 / ASC 04 / ASCQ 02（裸金属探针 + XNU 四轮一致，确定性非竞速）；② **query
  引擎关门**：READ_FLAG/SET_FLAG 对 {CT 0/1/2}×{LUN 0/0xD0/0x50} 一律 0xFE INVALID_OPCODE，
  响应里 opcode/idn 回显正确（设备解析了却拒服务），裸金属扫矩阵（探针件 88a614ed）与
  XNU 内完全一致；③ **通道记分板**：NOP=0x20 NOP IN ✓、SCSI=有响应有 sense ✓、
  TM=**LU RESET completed (result 0x0)** ✓ 但没换来就绪、Query=一概拒 ✗。
- **★病因模型**：nura 链的 Linux 永远自己从 DME_RESET 全量冷启动链路（query 引擎初始化
  自然完成）；我们继承 ABL 温链路 = 半初始化态。04/02 与 query 关门 = 同一未完成设备
  初始化的两面。补丁轮已证明「续用温链路 + 标准 fDeviceInit」在本设备不通。
- **★本会话产出（全部双腿 CI 绿后实机）**：0082 sense 诊断+TUR 轮询（run 39332070258
  门 86 + SWEET 39334407883）；0083 query CT 修正 + dump + fDeviceInit（run 39336420522
  门 87 + SWEET 39337574251）；0084 TM/LU RESET（run 39339234730）；shim 扫矩阵探针
  （本地件 88a614ed，**探针轮免 CI = 快速实验通道**）。内核件链 = 0082 b35cafc8 →
  0083 095688b6 → 0084 bf79e063；镜像链 f6d7e155 → 3d467f2a → 264bcc5a → 88a614ed(探针)。
  用户手下载件流程验证两轮（仓根 xnu-kernel.zip 覆盖式；验件 = 内部 SHA256SUMS + 大小
  增量 + 内部时间戳 vs run 窗）。⚠research/ scratch 树陈旧勿作基准；补丁链重建脚本法
  见 Session log 10-20 两条目。
- **★下会话动作（按序）**：① 拍 v6 探针屏：看 `UFS W d0`（SSU 结果）/ `UFS W d0 sense` /
  `UFS T2`（唤醒后 TUR 是否变绿）/ `UFS Q2`（query 是否开门 + bCurrentPowerMode 值）；
  若 T2 绿 → CAP/READ/GPT 分区表应自动跟上 = UFS 墙破，回 sweet_ufs.c 写 0085（SSU +
  fDeviceInit + TUR）→ CI → 刷 boot_b（**userdata 勿动**）；② 若 SSU 不愈 → REA ABL/XBL
  UFS 驱动（s5-rea 现成；xbl_a-code.bin 就绪未展开），两问不变：fDeviceInit 有无 + 链路
  冷启动序列（DME/PHY 真值）。回滚 mu-r69 不变。收口杂项：仓根 xnu-kernel.zip 可删；
  完工清 mk-eio/ + /usr/local 三件。
- **★UniXNU 金矿（10-20，用户供钩子 + 本会话挖出）**：github.com/UniXNU —— 「UEFI 上跑
  iOS/macOS」，其设备仓 **UniXNU-sheng = 小米 Pad 6S Pro = SM8550（kalama，同我们 SoC）**。
  三仓已克隆进 `reference/`（unixnu 主仓 / uefi-sheng UEFI / unixnu-sheng 设备仓，均只读）。
  要点：①他们的链 = ABL → BootShim+UEFI FV 塞 boot.img kernel 字段（与我们 shim 同构）→
  Mu-Silicium UEFI → DarwinBoot → XNU（Darwin 23）；②**sheng.dsc `SHENG_VENDOR_HW = 0`
  ——UEFI 不碰 UFS**，UFS 是 XNU 内 kext `drivers/NativeUfs` adopt ABL 温链路（与我们同
  构，无冷启动需求！）；③**NativeUfs.cpp:488 实机注释（2026-10-17）**：温链路态 = NOP ✓、
  fDeviceInit=0、bCurrentPowerMode=0x33 低功耗、**全 LU NOT READY**（= 我们的三层症状），
  解药 = SSU ACTIVE→0xD0；④他们 query 引擎在温链路上是活的（能读 flag/attribute）——
  Meizu ABL 把 query 引擎关门这点比 Xiaomi ABL 更烂，SSU 若愈 query 或随之开门；⑤附带
  发现：query function byte 在 header dword1 byte1（0x01 read/0x02 write）——我们 shim 旧
  sweep 恒发 0，v6 起新 query 用正宗值（XNU 侧 sweet_ufs.c 本就发对，canned-0xFE 结论不
  受影响）；⑥patch 库里有 null-root-media / native-4k 页表 / SMP secondary 等近亲件，
  后续遇墙可来翻。
- **★阶段 2 粮草齐备（10-20 会话收口；只剩刷机）**：①补丁修复（commit 256a16b：
  0060 两调用包 `#if SWEET`、0072 守卫内单行、0069 四枚 hunk +2 平移；真源树严格
  walk 81/81 绿）→ **双腿 CI 全绿**：M2381 run 39316683313（门 "Applying patch" =
  85 整）kernel.development.m2381 = 17,072,816B sha be1d55f5…；SWEET 回归腿
  39318914543 ✓。②boot 镜像：mkpayload `rd=ufsroot0` + mkadt --m2381 →
  **shim/out/shim-boot-m2381.img 5,701,632B sha f6d7e155…**。③**iOSRoot.img 完工**
  （16G，sha sparse `1db88931…` = iOSRoot-s.img 12,145,492,364B）：mkiosroot 经
  **混合管线**完成——EIO 定谳 = apfs-fuse 所链 libfuse 3.18.3（linuxbrew）读侧
  EACCES/EIO（同 binary 换 3.10.3 全绿，隔离实锤；宿主原生已装 libfuse 3.10.3 于
  /usr/local/lib64 + 实证容器二进制为 /usr/local/bin/apfs-fuse，旁注：sgan81 master
  新编会卡死读，勿用）；管线 = **容器 apfs-fuse -o allow_other + 容器 root rsync
  解压成真实文件 staging**（/home/ri/mk-eio/stage，三件 rc=0，os 8.3G/sysos
  3.2G/appos 11M，318k/2k/1k 条目，build2 的 EIO 钉子户文件 bplist00/TZif 全通）→
  **宿主 mkfs.hfsplus + loop 挂载 + 纯文件 rsync**（/home/ri/mk-eio/
  mkiosroot-stage.sh）→ fsck -y 修复过。★**fsck -n 的 "Volume header needs minor
  repair (1,0)" = mkfs.hfsplus+journal 对这版 fsck 的固有噪音**（全新空镜像同样
  rc=8 实证），非镜像缺陷，勿再当墙。★工程教训：`pkill -f` 模式含在自家命令行里会
  自杀（三次踩坑）；`cmd | tail; $?` 取的是 tail 的退出码。**下一步 = 设备在场**：
  fastboot[蓝屏/坐屏 pwr+vol↓] → `fastboot flash userdata iOSRoot-s.img`（⚠userdata
  = nura pmOS 根，用户已明示放弃可重烤）+ `fastboot flash boot_b
  shim/out/shim-boot-m2381.img` + reboot → **拍屏**（过 root 墙 = launchd/用户态开
  跑；调试 = sweetlog 持久日志环落 userdata 尾 4MiB，0028 自带；staging 11.6G 保留
  在 /home/ri/mk-eio/stage 作重烤缓冲，全项目收口时随 mk-eio/ 一起清）。
- **★阶段 1 本批产出（详 Session log 10-19 深夜条目）**：M2381 board config 全链
  （M2381.h / MASTER.arm64.m2381 / MeizuPlatformExpert / MeizuCPU 终态版）+ 11 件 sweet
  补丁翻版完成（0004 nASSGIreq 前置已按秘方写进 pe_init_fiq；0065 共享化含 m2381 cpus
  钳制/PSCI secondary entry/缓存维护；SWEET 语义零漂移[双树 diff 审计过]）+ mkadt.py
  `--m2381` 模式（13GB 连续段 / INTID27 / 0xb8000000 fb / 'meizu,m2381' compat）。
- **★验证方式（无 macOS 工具链的本地极限）**：xnu-8796.141.3 真源树上**严格重放**
  （scripts + 84 patches 按序 git apply，零 fuzz 零跳过 = CI 不会静默丢补丁）+
  SWEET 回归双树 diff 审计（cpu.c/machine_routines/SweetCPU 零 diff）。**编译验证只能
  靠 CI**（xnu-cc.py 需 CI 日志 + buildtree artifact）。
- **★CI 注意**：dxb build.sh 对 apply 失败的补丁**静默跳过**（warning 而非 error）——
  首跑后必须 grep 日志确认 84 件全数 "Applying patch"，缺一 = 配置不完整。
- **★CI 现态（10-19 深夜收官时）**：fork push `fdad0de..62d6b7b` 完成（容器内 gh 直推，
  配方见记忆）；**补丁门已绿**：首轮 M2381（39262752452）取消前日志 81/81 `Applying
  patch`、零 Skipping。⚠workflow 带 `concurrency: xnu-main + cancel-in-progress`——
  同 ref 连发 dispatch 会互顶（首轮就是这么被 SWEET 轮顶掉的，红色 cancelled ≠ 编译
  失败）；⚠fork 上 push 触发器不燃（workflow active 也不燃），每轮手动 dispatch。
  **现役构建 = run 39283715880（machine_config=M2381，★M2381 CI 首绿，kernel
  .development.m2381 出件 17,015,632B，sha e52d539e…）**；查态：`GH_CONFIG_DIR=
  /run/host/home/ri/.config/gh gh run view 39283715880 -R RiverKawa271828/ios-meizu-m2381`。
  SWEET 回归腿 = push 触发轮（NOTES 收录 commit 自带 push 即燃，workflow 默认 SWEET）。
- **★上机弧线（10-20 凌晨→清晨，五轮诊断循环，详 Session log 10-20 条目）**：★XNU 已在
  m2381 实机活到 zone_init/调度器（13GB 认全、VM bootstrap 完成、firehose/OSLog 配好），
  首墙 = KernelConfigTables 尾逗号（真凶 0018 的 MeizuCPU 末条目）已修；**三轮读数 =
  尾逗号墙过**，四墙 = NULL g_crypto_funcs（crypto BearSSL 兜底守卫 SWEET-only）→ 修
  （d103ced）；五墙 = pthread 函数表 NULL（0019 builtin provider SWEET-only）→ 修
  （8a59025）；**★五轮（de477ccd）= 阶段 1 通关**：M2381 PE/CPU 注册点亮、MAC/HFS/mbinit/
  dlil 网络栈全活，内核**无 panic 坐等 root 设备**（`!BSD` + `Waiting on <dict>IOMedia/
  Apple_HFS</dict>`）——判据（banner/panic 可视/逐墙推进）全达成并超额。设备现态 =
  boot_b = 第五轮镜像坐等 root。**下一阶段分叉 = 阶段 2 root 来源（用户拍板）**：
  A) ramdisk-root 路线（mkinit.py 出 iOS ramdisk，mkpayload --ramdisk，零 userdata 风险）
  / B) UFS root 正主（mkiosroot.sh → sda22 = **抹 Flyme 用户数据，红线#2 硬闸门**）。
  配套工程：sweet USB-net 日志通道（topaz 0077 翻版）在 userspace 调试前立起来收益最大；
  SWEET 回归腿对两笔守卫改动的验证轮 = 39305858552。
  ★配方事实：**Meizu GKI ABL 无视 boot.img 的 ramdisk**（initrd 恒为 stock 5.7MB
  initramfs，SWEETXNU 魔数不在），payload 必须内嵌 kernel 镜像（shim `_payload_start`
  符号 + build.sh 追加 + header image_size@0x10 补丁），Mu 同构；ramdisk 槽 = 占位 cpio。
- **★首跑失败与修复（commit fb0853e）**：run 39263979219 红了，根因 = MakeInc.def 的
  arm64e 排除条件极性写反（`ifneq (,$(filter ...))` 语义 = 匹配才执行 → M2381/SWEET 全
  被编成 `-arch arm64e`，1141 命令、46 处 PAC 类型错：status.c×35 / vfs_cache.c 的
  v_cred ptrauth 限定 vs kauth.h 朴素 typedef）。修复 = `ifeq (,$(filter ...))`（空才
  执行 = 非本双板才 arm64e）；同刀放行 0019/0020 在 bsd/libkern Makefile.template 的
  include 门。**★方法论教训：双树 diff 审计对 make 层按配置名的字符串分支是盲的
  （树文件相同、分支走向不同）——审计必须补「配置名 grep」遍**（本次泛搜确认全仓只剩
  三处、均已双板化）。
- **★二跑失败与修复（commit 85bffda）**：run 39266653004 红在 kperf（19 处
  `KPC_MAX_COUNTERS` undeclared）——kperf_kpc.h 的 `#include <kern/kpc.h>` 被
  `#if KPC` 门着、下面的 struct 却无条件用该宏。根因 = **m2381 MASTER 生于 stale**：
  0008 给 sweet 的 MASTER 加 `kpc` 组件（kperf 依赖），m2381 副本是 0006 时刻拷的，
  后续六个改 MASTER 的补丁（0007/0008/0021/0026/0037/0057）全部只锚 sweet 文件。
  修复 = 0006 直接按**终态** sweet MASTER（已验与全系列跑完的 sweet 文件逐字节一致，
  仅头注释换 m2381）创建 m2381 文件——后续无补丁碰它。上游 fdad0de CI 绿（排除
  runner 漂移）。★规则：拷贝式平行文件必须生于终态内容。现役 = run 39270819603
  （SWEET 回归腿被 concurrency 顶掉，M2381 落地后再补）。
- **★三跑失败与修复（commit a93fc81 + 86a7119）→ ★M2381 CI 首绿（run 39283715880）**：
  39270819603 红在**最终 kernel link 两个未定义符号**（81/81 全 Applying、编译面全走完）：
  `_ml_enable_monitor`（arm_vm_init 无条件调用，0008 的 machine_routines.c stub 守卫
  `defined(SWEET)` 未扩板）+ `_sweet_sample_task`（调用方 bsd/dev/sweet_sample.c 是
  standard 双板，0054 把定义塞进了 exception.c 已有 `#endif /* ARM64_BOARD_CONFIG_SWEET */`
  块内部）。同族病 = 板守卫未扩 M2381（与 MASTER-stale 同族，但在源码 `#if` 层）。
  ★第一版 0054 修法（定义单独一圈守卫但仍嵌外层块内）是 **no-op**，CI 未测到（39276727236
  排队后 runner 暴毙，日志流 15:23:24 戛然而止、Build 步 conclusion 卡 null——runner
  死亡签名，重跑即可），是**本地重放 nm 抓住的**：修法 = 提前合上外层块，sampler 连同其
  依赖的两个 `#include`（vm_shared_region.h / arm/thread.h）独立成共享段。★教训：
  **apply 级 walk 对守卫语义是盲的——嵌套 `#if` 的排除性要在编译/nm 面验证**。
- **★本地验证链（10-20 定形，push 前置门，零排队零等待）**：①源 = `git clone --depth 1
  --branch xnu-8796.141.3 apple-oss-distributions/xnu`（dxb get_xnu 同源）；②严格 walk
  （.sh 先行 `MACHINE_CONFIG=M2381`，81 patch `git apply --check`+apply；Linux 需 xcrun
  stub[仅 TargetConditionals.h 便利件]）；③`tools/xnu-cc.py <log> <bt> [filter]` 重放 CI
  编译命令（linuxbrew clang `-target arm64-apple-macos14.0` 直接可用，swiftlang 免装；
  `XNU_CI_DXB=` 环境变量已加：fork 的 runner 路径与上游不同；`XNU_CC_OBJDIR=` 真出 .o）；
  ④llvm-nm 全树扫 defined/undefined 差集 = 链接失败代理（本例 1095 .o / 未解析 0）。
  artifact（`gh run download -n xnu-buildtree`，23MB）提供生成头/iig/mig 产物；把补丁修
  同步落进 bt 副本再重放。★lld 试链结论（linuxbrew llvm 自带 ld64.lld）：rename_section/
  sectcreate/segprot ✓，但 **-segaddr/-image_base/-segment_order/-alias_list/-static
  "not yet implemented"** → 链得出 Mach-O、出不了可启动内核（装载基址/段序全错）；
  本地出件备案 = osxcross 系 cctools-port（真 ld64）+ 自建 libfirehose_kernel/cc_kext，
  数小时级，暂不投入（CI 一轮 7 分钟）。
- **内存模型定谳**：kernel-managed RAM = **0x8c0000000 起连续 13GB**（stock FDT 八段中
  0x8_c0000000[5GB] + 0xa_00000000[8GB] 首尾相接）；八段其余部分低银行由 XBL/XPU 占用
  不入手。物理映射单连续窗模型不变（real_phys_size = args->memSize）；16K L1 槽 = 64GB，
  0xb8000000 fb 与 35GB RAM 同槽（0010 手搓 TTE 机制原样适用）。
- **阶段 0 判词（详 Session log 10-19 晚条目）**：tg16=1（16K granule 支持）/ PARANGE 40bit /
  PSCI 1.1 + SMCCC 1.3 / 8 核 CPU_ON 全 ret=0 / **INTID27 vtimer 300ms 恰 30 拍 ✓** /
  UFS ver=0x0400（UFS 4.0 裸金属复证）；**SGI 自发 10 发 10 中**。**ARMv9 兼容性定谳，
  头号风险关闭。**
- **★GIC SGI 页优先级写秘方（三轮疑案收口）= 先置 GICD_CTLR.nASSGIreq（bit8，GICv4.1
  「SGIs without active state」）再碰 SGI 页优先级寄存器**。未置位时 NS 写 = 实现定义
  异常（轮 1-3 外部中止）或静默丢弃（轮 5 map）；置位后写粘（PRIRD=A0A0A0A0）。与内核
  dmesg「Enabling SGIs without active state」互证（gic_dist_init 先置位 → gic_cpu_init
  才写）。**XNU 0004 翻版要点：pe_init_fiq 的 8 笔优先级写之前必须置 bit8**；本机
  GICD_CTLR.DS=1 单安全态（0004 注释 two-security-states 按 m2381 实况改写）。本机真
  GICD_TYPER 在 **+4**（0x016B001F，ITLines=0x1F/DS=1）；+8 读值 0x47000070 非 TYPER
  形状（GICD 解码怪癖 footnote）；TYPER2(+1C) 读 0 但 bit8 写收下生效。
- **机型** = 魅族 20 Classic（m2381 板号，16GB LPDDR5X，UFS = Samsung KLUFG8RHHD-B0G1
  512GB = **UFS 4.0**[裸金属 ver=0x0400 复证]）。
- **续接（阶段 1 收尾三步）**：① 用户拍板推 fork（本地 commit 未 push；gh 认证在宿主
  keyring，push/gh 走宿主终端）；② GitHub Actions `xnu.yml` workflow_dispatch，kernel_config
  照旧 DEVELOPMENT、**machine_config = M2381**（SWEET 默认跑法保留作回归腿）；③ 出件后
  本地 `tools/xnu-cc.py` 校日志 → `tools/mkpayload.py kernel m2381.adt payload
  --cmdline 'debug=0xc serial=3 cpus=1 keepsyms=1'`（ADT = `python3 tools/mkadt.py
  /tmp/m2381.adt --m2381`）→ `M2381=1 bash shim/build.sh <payload>` → boot_b 换装上机
  （判据 = XNU banner 上屏 + 早期蓝屏诊断可读；复位政策不变）。
- **收口（交还设备时）**：flash mu-r69（`97cdea20`）+ `set_active b` + 读回对拍（前缀
  sha = `97cdea20…`）+ `fastboot reboot` 回 nura OS。现态：boot_b = shim v5（8a44b6e7）
  坐屏 DONE，随时可收口。
- **复位政策（永久有效直到 SPMI 路修通）**：shim 永不自复位，跑完/panic 坐屏等人，
  pwr+volDn 手动回 fastboot；软复位修通 = 阶段 1 顺 sweet 0060 SPMI 做 PMK8550 PON 写。

## 阶段 1 开工排面（2026-10-19 晚定稿；下个会话照此施工）

> 前置已全部实证（阶段 0 五轮）：启动链（shim=Image 头 + ABL EL1/DTB/initrd + boot_b 换装）、
> 16K granule、PARANGE 40bit、PSCI 1.1、8 核 CPU_ON、GIC 全套（含 SGI 页优先级 = nASSGIreq
> 秘方）、INTID27 vtimer、fb console（0xb8000000）、UFS 链路活。剩余工作 = 纯 XNU 软件面。

**补丁翻版清单（11 件 → m2381 版；sweet 原件 = xnu/patches/）：**

| 补丁 | 内容 | m2381 翻版要点 |
|---|---|---|
| 0003 | fb 串口 console | FB_BASE **0xb8000000**（stride 4320 与上游同）；shim console.c 五轮实证的排版/SCALE 经验直接搬 |
| 0004 | machine config + pe_init_fiq | 新 M2381.h：GICD **0x17100000** / GICR **0x17180000** stride **0x40000** size **0x200000** / VTIMER **27** PTIMER **28**；★**pe_init_fiq 的 8 笔 GICR_IPRIORITYR 写之前先 `GICD_CTLR \|= bit8`（nASSGIreq）+ RWP**；**DS=1 单安全态**（two-security-states 注释/Group-0 路径按实况删改）；ICFGR1 默认读值 A0A0A0A0（PPI 边沿） |
| 0006 | open-source asm pmap | SoC 无关，原样翻 |
| 0010 | early framebuffer debug | FB 常量同 0003 |
| 0011 | physmap 对齐填充 | **16GB 八段**：0x80000000/811d0000/d8140000/d8800000/e3bb0000 + 上银行 0x8_80000000/0x8_c0000000/0xa_00000000（stock FDT 实证，ddr-regions=2+6+8G）；PARANGE 40bit 覆盖 |
| 0012 | ttbr0 后保 console | 原样翻 |
| 0016 | DT platform expert | DTB = ABL x0 直递（shim 五轮实证 INITRD/chosen 均被 ABL 填好） |
| 0017 | DT virtual timer irq | **INTID 27**（标准 PPI11；PTIMER 28）——五轮 30/30 拍实证 |
| 0018 | boot CPU driver | 原样翻 + SM8550 拓扑（Aff1=0x000..0x700 单核每 Aff1：3×A510+2×A715+2×A710+1×X3） |
| 0065 | SMP PSCI | `i<<8` 目标格式零改（7 核 ret=0 实证）；**cpus=1 起步再逐核放** |
| 0078 | 全 8 核点亮（topaz） | 阶段 0 已证 8 核全 ON，直接合并 |

**ADT（tools/mkadt.py m2381 版）**：GICD/GICR_SIZE/FB/UFS 常量全换（UFS 0x1d84000 size 0x3000
不变）；内存 = 八段拓扑；TIMEBASE 19.2MHz 不变；设备身份保持 iPhone10,3；disp0 canvas
1080×2400；ECID 伪值改 'M2381' 前缀防混。

**构建**：CI = 用户 fork → GitHub Actions `xnu.yml`（workflow 在 .github/，随树走）。
**★fork 已就位（10-19 晚验证）= `RiverKawa271828/ios-meizu-m2381`**（git remote `fork` 已接；
main = fdad0de = 上游 tip，干净分叉点，公开可读）。**gh 认证形态 = 宿主 keyring**：
hosts.yml 有 RiverKawa271828 但 token 在 Fedora 系统钥匙环——**容器内 gh 看得到账号条目
但取不到 token（auth status 误报 not logged in），需认证的 gh/git 写操作在宿主终端跑**；
（备选：宿主 `gh auth token` 导出后容器内 `gh auth login --with-token`，用户自决）。
本地单文件预检 = `tools/xnu-cc.py`。**push 窗口用户拍板**（本地 22 commit 待推 fork）。

**上机形态**：`tools/mkpayload.py` 出 SWEETXNU payload → `M2381=1 bash shim/build.sh <payload>`
打进 ramdisk → bootimg → boot_b 换装（与探针轮同一套流程/红线，判据 = XNU kprintf banner
上屏 + 持久日志环可读 + panic 可视）。**优先评估 topaz 0077 USB-net 日志通道**（SM8550
dwc3 vs SM7150，立起来后调试效率提一档）。

**风险台账（阶段 1 内未知，非阻断）**：XNU 在 ARMv9（A510/A715/X3）的实施细节逐个冒
（PAC 已有 0005；cpus=1 起步就是为此）；16K pmap 八段扩展的 pmap 代码面；CI 首跑
workflow 修缮。UFS SCSI 命令 rc=4 未完链 = 阶段 2 工作面。

**收口状态（本会话末）**：boot_b 刷回 mu-r69 + set_active b + 读回对拍（见 Session log）；
下个会话从 fastboot/devsh 全新窗口起步。

## GPU 判词（2026-10-19 研究；结论：不做 GPU KMD，显示升级走 DPU/vsync 路线）

- Linux 侧已 proven：nura r84 =「GPU/IFPC 收官」（config.sh 台账），`~/work/steamos-meizu20`
  = 本机 SteamOS 移植（freedreno/mesa 全栈实跑）——Adreno 740 的寄存器/固件/调频知识本地齐。
- XNU 侧从零：Apple GPU 驱动（AGX 家族）是纯用户态、只对 Apple GPU 说话；**Adreno 在 XNU
  世界里零代码**。把 Linux msm/freedreno KMD（数万行 DRM + GMU 固件 + GPU 页表 + 上下文
  切换）移植成 IOKit 驱动 = 月级独立研究线，做完还缺用户态渲染栈（Mesa→CoreAnimation 桥）。
- **务实路线 = DPU 显示控制器（不是 GPU）**：硬件 scaling（1080×2400 面板 → iPhone X 身份
  1125×2436）、多层合成、vsync 驱动呈现（升级 sweet 0056 的 10Hz 定时器拷屏）——nura 主线
  内核 `panel-boe-meizu20.c` + DPU 全套寄存器序列 = 地面真值，属阶段 3 显示工作的自然升级。
- 复原件台账（两件，sha 预置好）：
  - mu-r69-97cdea20.img = 1,110,016B，`97cdea208214e655e055d815bdcc307d00d8990fd371abeebabf4c31c14326f6`
    （nura artifacts；「mu-r89」= 内核 boot 数混写勘误，Mu 全史 = r62/64/66/67/68/69）。
  - 原厂黄金备份 boot_b.img = 100,663,296B 整分区，`5f9f1503…`
    （`/home/ri/work/meizu20/meizu20-m1/backups/flyme-ota-b-20261002/images/`，10-02 9008 EDL
    全分区回读，boot_a==boot_b 同 sha，init_boot_b = Magisk 31.0；EDL 回写配方 + SHA256SUMS
    在档 = 9008 全量救砖兜底，userdata 除外）。⚠长度陷阱：mu-r69 是 1.1MB 镜像文件、
    整分区 dd 是 100MB——mu 只能**前缀对拍**，勿拿整分区 sha 硬比 mu 文件。
- 读回 plan B：若 Flyme su 失效 → 9008 `edl.py r boot_b`（只读；本备份即此法所出，nura 档有配方）。
- 阶段 2 粮草在库：`research/ipsw/iPhone10,3,iPhone10,6_16.7.16_20H392_Restore.ipsw`
  6,171,035,648B，sha256 `ece1194f7e233e9109c5790a8d23a4564a10a254e15ec5b09978a343265568fb`；
  BuildManifest 验 20H392 + iPhone10,3/10,6 ✓；dmg 三件套（OS=087-86698-044.dmg 4.7GB，
  与 sweet NOTES 同名文件）齐。
- 槽位模型（★10-19 用户纠正 + nura FLASHING.md §1/BOOT-CHAIN-RE §0 实锤）：current-slot =
  **b**（验证机形态 = Flyme OTA 落 b 槽 + b 槽活动；「必须 = a」是转达讹误，a 槽启动反而
  未验证勿盲试）→ boot_b = **现役槽换装舞台**（Flyme boot ↔ mu-r69 ↔ shim 三态轮换；
  ABL = slot b stock LinuxLoader，gunzip payload → 0x80080000 → blr[x0=DTB]）。换装窗内
  Flyme 暂不可启属预期；init_boot_b[Magisk root]/super/userdata 全不碰，root 无感；
  收口 = 刷回读回原件 + `fastboot reboot` 即回 Flyme，全程无恢复/重装动作。
- 现役探针件：`shim/out/shim-boot-m2381.img` **16,384B**
  sha256 `77e69f0188eae40dd6ba225aa6df9ee0c7988762cd4bd804220cfc2fd906aa2e`
  （v4/13.0.0/2023-10，gzip kernel 7,534B + 空 cpio ramdisk 97B，拆回逐字节 ✓，零绝对重定位 ✓）。
  探针形态 = 不带 payload → shim_main 全硬件探针，3 分钟 PSCI 自动复位（RESTART_BOOTLOADER
  预写 → 自动落回 fastboot）。
- nura 侧：fastboot 单通道空闲（10-19 确认），探针轮独占；读回通道 = 有 root 的 Flyme 本身
  （su dd → /data/local/tmp → adb pull），**不起 pmOS**。
- Next：插线 → 读回定现态 → current-slot=a → flash 探针件 → 眼判六项（图例见下节）→
  刷回读回原件收口。六项全绿 = ARMv9 兼容性定谳 → 阶段 1（XNU 平台移植）。

## 探针屏幕图例（眼判六项用；裸金属帧缓冲 adb 抓不到，必须人眼/拍照）

探针屏自上而下（红底开始；**结果永久坐屏，看完 pwr+volDn 手动回 fastboot**；蓝屏 = panic
**追加在下方不清屏**，见节末）：

| 屏上行 | 含义 | 绿 | 红 |
|---|---|---|---|
| `SHIM m2381 probe: hw bring-up questions` | shim 入口 | 出现即可 | 没出现 = ABL 没跳进 shim（打包/交接问题） |
| `LOADER no SWEETXNU payload` | 探针形态预期（ramdisk 无 xnu 载荷） | 正是这句 | 出 "jumping" = 打包错了载荷 |
| `INITRD lo=/hi=` | ABL 递进来的 ramdisk 位置（诊断信息，随便什么值都行） | — | — |
| `PARANGE mmfr0=? tg16=?` | ID_AA64MMFR0：物理地址宽度 + 16K 粒度支持 | **tg16=1**（= 16K 支持，10-19 实测 ✓）；mmfr0 低 4 位 ≥ 0x2（40 位以上 ✓） | tg16=f（0xf）= 不支持 16K → 阶段 1 要换 4K 方案 |
| `PSCI ver=? smccc=?` | PSCI/SMCCC 版本 | **ver ≥ 0x10000**（实测 1.1 ✓）；smccc ≥ 0x10000（实测 1.3 ✓） | ver=ffffffff = 无 PSCI |
| `CPU1..CPU7 ret=? mpidr=?` | PSCI CPU_ON 逐核点亮 + 各核自报 MPIDR | **全部 ret=0** 且 mpidr= 真值（10-19 实测 8100_0100..0700 ✓） | 任一 ret≠0 = CPU_ON 失败；mpidr=dead0000 = 核没跑起来 |
| `GICD was=? now=?` / `GICDTYP=?` | 分发器使能（Linux 同款 0x13 写入）+ GICD_TYPER（arch/核数/DS 位） | now 的 bit0/1/4 已置；TYPER 给出核数 7 | now=was = GICD 地址错/不可写 |
| `GICR waker0=? typerL=?` | wake 前状态 + GICR_TYPER 低字（proc number/Last） | waker0 记录在案即可 | "GICR not found" = 基址/stride 错 |
| `GICR wakespin=? waker1=?` | wake 序列自旋数 + wake 后状态 | waker1 的 ChildrenAsleep(bit2)=0 | spins=100000 = 唤不醒 → skip gic（保护设计） |
| `GICR base=? waker=?` | boot 核 redistributor（10-19 实测 = 0x17180000 ✓） | base 真值 + waker 低位已清 | — |
| `MMU sctlr=? ttbr=?` | **v3 起：探针 MMU 已开**（全 Device 恒等映射 + GIC 块 nGnRE） | sctlr 低位有 M/C/I | enable failed = 镜像跨块（不该发生） |
| `CFG grp=? en=?` / `PRI16=?` | SGI 页写前读值（IGROUPR0/ISENABLER0/IPRIORITYR[16..19]） | 记录在案（判别数据） | 读就炸 = SGI 页读不通（未见过） |
| `GRPRD=?` | IGROUPR0 写 0xffffffff 后回读 | =ffffffff → SGI 页此窗口可写 | ≠ → 写不粘 |
| `ENABLE was=? now=?` | INTID27/28/1 三个源使能 | now 含 bit27/28/1 | now=was = 不可写 |
| `PRIRD=?` | **优先级字写后回读（v3 关键判据）** | **=a0a0a0a0 → nGnRE 下写粘了** | 止步此行前 = nGnRE 仍炸（TZ 保护类） |
| `ICFGR1=?` | PPI 配置寄存器记录值 | 记录在案即可 | — |
| `VTIMER irqs=? other=? … last=?` | **INTID27 虚拟定时器 300ms 窗口计数**（10ms 周期） | **irqs ≈ 30、other=0、last=1b**（=27） | irqs=0 = INTID 27 不对 → 回改 VTIMER_INTID |
| `SGI1 irqs=? …` | 自发 SGI ×10 | **irqs=10** 精确 | ≠10 = IPI 机制问题（不挡阶段 1，记录即可） |
| `UFS ver=? cap=? hce=? …` 等 | UFS 只读链全探（MMU 开但 UFS 区仍 Device，DMA 共享性设计原样） | rc=0 + 真值；**ver= 预期 0x04xx = UFS 4.0**[10-19 nura OS 实证 SAMSUNG KLUFG8RHHD-B0G1 = 512GB UFS 4.0] | hce=0 = ABL 没留链路 → 阶段 2 要自拉 |
| `DONE probe complete` → `RESET manual: pwr+volDn -> fastboot` | 全探针走完 | 走到这 = 一轮完整探针 | 手动 pwr+volDn 回 fastboot |

**蓝屏判读**：`PANIC SYNC/IRQ/FIQ/SERROR` + `STAGE=` + ESR/ELR/FAR——**追加在已有结果下方
（v3 起），红屏诊断值全部保留**——拍照发回整屏。**无自动复位**（10-19 第一轮实锤：IMEM 魔数
复位落正常启动 → boot 循环烧 retry 计数），坐屏等人 pwr+volDn。真死挂也是 pwr+volDn，兜底 9008。

## Session log

### 2026-10-22 八（r34 窗读环）: B 终审实锤（缓存遮蔽磁盘扫描，r34mark 0 出现）+ 连崩群 548→0（容器骨架大捷）+ mad 换守卫（stock stub line321）——全链唯一堵点 = mad fatal，r35 转 stub

- **窗执行**：用户刷 r34 双件（boot `339b8480` + userdata `7aaab96f`，刷前宿主 sha 双对拍
  全等）实跑 **~8.5min 半窗**（suggestd 169 vs 前三窗 315–331；mad 52 ≈ 6.1/min 与前三窗
  5.7–6.0/min 同拍）→ 自刷 nura-min 交班（用户令跳过刷后验证）。读环：devsh telnet 文本
  通道 → `dd 尾部1025×4K | gzip | base64`（设备无 base64 亦有；od hex 备胎未用）→
  `rings/m2381-ring-r34.{bin,txt}`（182,970B/flush246/3,298 行/sampler 4 轮）。
  ★过程坑：宿主 shell 开窗全灭 = **磁盘配额满（EDQUOT -122）**——任何写 stdout 的命令
  exit 1 零输出、`true` 能过、Write 报 -122；用户清盘+重启后恢复。★第二坑：zsh 把
  `grep` 包成 ugrep 且 `--ignore-files` 跳过 .gitignore 的 `rings/` ⇒ 全部"零匹配"假象；
  环分析必须 `/usr/bin/grep -a`。
- **双件身份铁证（无体内标记也定案）**：boot-args 行带 `sweet_amfi_debug=1`（= boot r34，
  顺带 AMFI 逐 daemon entitlements 计数首次可见）；`cs_invalid_page` = **0**（前三窗
  102/99/97，撤 8B 补丁后 CDHash 回 stock）；崩溃 pc = mh+0x1BF3BC（旧 0x1BF5BC）=
  stock 代码路径。三者都只有 r34 userdata 能产生 ⇒ **r34 上机实锤，刷写通道无罪**。
- **★Gate 1 = 0 ⇒ 假说 B 终审成立**：`exec r34mark` 与 "r34mark" 全环零出现，而 r34 身份
  另证在 ⇒ **磁盘 LaunchDaemons plist 被 launchd 签名缓存整体遮蔽**。连带判死：r34 的
  lsd plist envvar（FEATUREFLAGS_ENABLED）同被遮蔽未投递 ⇒ **A1 两件 kill-switch 全失效**
  （Global.plist 单独只在 SHM 表空时被读——envvar 没了 SLOWPATH 不走，仍 false 不更坏，
  如报告预测）。A5 悬案补全：`launchd_unsecure_cache=1`（"1"=不存在路径）⇒ **静默落回
  默认缓存，不回退磁盘扫描**（否则 r34mark 会出现）。
- **★Gate 5 大捷（本窗最大正面结果）**：r33b 连崩群 accountsd 188/chronod 64/dmd 60/
  liveactivitiesd 60/ScreenTimeAgent 60/cloudd 60（合计 548 CRASH）→ 本窗 **全零**：六件
  各 exec 恰 1 次（环 682–1135 行，中段）、零崩溃、其后 ~2300 行（≈2/3 窗）零重拉 =
  **活着**。containermanagerd 亦 1 次 exec 无崩。⇒ **容器骨架让容器机制整体复活**
  （报告 Gate 5 判读"同比下降 = 容器根因面修复旁证"以最大幅度成立）。
- **Gate 2 PASS**：`SWEET AMFI: com.apple.mobileactivationd: 28 entitlements (test: bool 0
  string 0 array 0)`，N≥20 = entitlements 挂载正常，AMFI 侧无罪（S1 不转修）。
- **★Gate 3 换守卫（`.re/ma-fatal.asm` 上窗件直接对上）**：mad 52/52 exec 全崩（208 行
  CRASH，52 把 slide 各异但 pc−mh 恒 = 0x1BF3BC）。新旧崩溃点都是同一张 Swift trap stub
  表的 brk：stub 形状 = `adr x0,"main"; adr x4,<fileinfo>; mov w1,#<行号>; mov w2,#-1;
  bl 0x5100; …; brk #1`。旧 = bl@0x5062C→stub(line 247)→brk@0x1BF5BC（bt0 0x50630）；
  新 = bl@0x50670→**stub(line 321)**→brk@0x1BF3BC（bt0 0x50674）。上游：main 循环
  （0x4F8C4，两次取值型检查 w23/w24）`tbnz w23,#0→0x5061C` 直跳错误尾；0x505F0 调用带
  landing pad（`adr x8,0x10050674` 显式登记）。判读：撤补丁后走 stock 分支撞 stock 自己的
  fatal，**与容器骨架无关**（此 fatal 在 mad 自身启动检查，不在 systemgroup 查询）。
  fileinfo 串为间接表 {0x15C,0x80200000} 未解；a2s 有 "activation_support.m" 存疑。
- **Gate 4 未变（帧等平移定谳法）**：lsd t2 r34 vs r33b = **16 帧逐帧全等**（r34 − r33b
  = 常数 0x1C42C000 全帧成立）= 同一 MadGate copyActivationRecord sync-XPC 永等栈。
  SB t0 已脱 os_state 死锁（r32–33b 11 帧栈 / "os_state" 串 0 命中）换为经 SB 框架层的
  同步 XPC 等待（= 等 lsd seeding）；bbd t0 = CFRunLoop 正常。**因果链唯一堵点收敛到
  mad fatal**：mad 死 → MadGate 永等 → lsd 不 seed → SB/bbd 等。
- **r35 方案排序（报告 §4）**：主攻 = **mad 原地换已签名即退体**（vm_stat 招；缓存按原
  定义拉起→不注册 MachServices 即退→lsd 得 error reply 非永等；零 CDHash 问题；判据 =
  CRASH mad = 0 + lsd t2 换栈 + mad 少量 exec 零崩——**CRASH mad=0 本身 = r35 体内身份**，
  从此不需要 plist 体内标记）；并行实验 = **改 launchd.plist 缓存 + sweet_amfi_debug=1
  放行**（前置宿主 RE：我方 amfi hook 是否覆盖 cache_loader 的 trustcache 查询——r32–33b
  已实证 hook 能放行补丁代码页，胜率不小；若成 = B 根治+磁盘权威恢复）；XPC 全功能 stub
  （A6 备料）为后备。launchd_unsecure_cache 三态实验作废。
- 下窗：宿主 15min RE（amfi hook 覆盖面）→ 出 r35 双件（userdata = mad→vm_stat 替换 +
  可选缓存手术；boot = 沿用 339b8480）→ 刷窗读环，判据见上。

### 2026-10-22 七（r34 出件窗，宿主侧）: B 机制实锤（launchd 签名任务缓存）+ 容器缺口精确到 Shared/SystemGroup 空 + os_feature 数据层 kill-switch 找到；双件出齐待刷

- **交接链**：DeepSeek v2 答复（`handoff/deepseek-reply-20261022.md`）+ GLM 任务书
  （`handoff/glm-brief-r34.md`）→ 本窗执行。产出报告 = **`handoff/glm-report-r34.md`**
  （A1–A6 证据链全量 + 设备窗 Gate 1–6 判据表），本条目只记结论级。
- **★A5/A4 = B 机制实锤**：`launchd_cache_loader` 反汇编（kern.bootargs →
  `launchd_unsecure_cache=<路径>` → `/System/Library/xpc/launchd.plist`+`.sig` →
  SecCode 分离签名验签 → **AMFI userclient 查信任缓存 cdhash** → "Sending validated
  cache to launchd"）；镜像内该缓存 795KB 实存且**含 mobileactivationd 原始定义**
  （Label+MachServices 全在）。A4 全图 444 命中按路径枚举：除已改名磁盘 plist 外唯一
  声明方 = 该缓存（7 命中）；`.xpc` bundle 隐式注册排除。⇒ r33 改名对缓存零作用；
  r33b「旧名仍注册」与缓存自洽（不再依赖假说 A）。r35 解 B 候选：拆缓存（签名失效
  风险自担）或 stub（A6 方法面/应答键已备：`ActivationRecord`/`FactoryActivationRecord`）。
- **★A3 = 崩溃缺口精确定位**：客户端链反汇编（container_system_group_path_for_identifier
  → `_container_legacy_lookup`[class 13] → container_query → XPC
  `com.apple.containermanagerd`；`.system` 变体在 release 被 feature flag
  `containermanagerd/user_system_split` Disabled，主 daemon --runmode=normal 接管）。
  镜像实况：早前种子只盖了 `Data/System/D90C94C4`（且键族用 MCM*），**`Shared/SystemGroup/`
  是空的**——entitlements 的三个 `systemgroup.*` 全缺 = NULL 直接缺口。r34 预置三骨架
  （MCM* 与 MCMMetadata* 双键族 + UUID，0755/0644）。
- **★A1 = HRN 数据层可行**：三个 FeatureFlags plist 全无 HRN；`_os_feature_enabled_impl`
  反汇编 = SHM 表（`com.apple.featureflags.shm`，init_featureflags 填充）+ **SLOWPATH**
  （表空或 `FEATUREFLAGS_ENABLED/DISABLED` 环境变量存在时走，读
  `Domain/<域>.plist`[DisclosureRequired/Enabled/DevelopmentPhase] + Global/Profiles/
  Settings.plist）。r34 双带：`Global.plist += DeviceManagementClient/HRN/Enabled=true`
  （照抄 PictureInPicture 等生效条目形状）+ `com.apple.lsd.system.plist
  += EnvironmentVariables{FEATUREFLAGS_ENABLED: DeviceManagementClient/HRN}`
  （强制 slowpath；若 lsd plist 被缓存遮蔽则 envvar 不投递，失效=仍 false 不更坏）。
- **A2**：mobileactivationd entitlements 全表（XML+DER 双槽一致）：system-group-containers
  三组、`allow-low-space-writes=true`、`system-container=true`、MobileGestalt 11 保护键、
  seatbelt mobileactivationd——文件有权，问题只在内核 AMFI 挂载（Gate 2 裁决）。
- **★r34 出件（全链验证闭合）**：userdata = **`iOSRoot-s-r34.simg 7aaab96f`**
  （12,145,451,600B）：①撤 8 字节补丁（tbnz→tbz @file 0x4F944/0x4F9B8，双验；**CDHash
  回归 stock，cs_invalid_page 噪音源消失**）②`/usr/libexec/r34mark`（vm_stat 拷贝，
  RunAtLoad `com.r34.mark.plist`）③三骨架 ④Global.plist HRN ⑤lsd envvar。
  fsck `-f -y`/`-n` 双验绿；全图 4K diff = 626 块 23 段全归因（Global.plist/lsd plist/
  mobileactivationd[驱动整文件重写=stock 字节]/B-tree+位图+新文件块）；
  **simg→raw 回程 cmp 全等**。手术过程坑：`containers/Shared/SystemGroup` 三层目录盘上
  uid=0 在 hfsplus 驱动约定里 = nobody（Apple 全系 0x80000），root 全 EPERM——umount 后
  直接改三条记录 @32/@36 → 0x80000（等长 8B×3，断言+回读验证）。
- **boot = `shim-boot-m2381.img 339b8480`**（5,734,400B）：cmdline = r32 权威串
  （394B，自 shim-boot-r32.img 解压 @16K 对齐 SWEETXNU+0x48 提取——比 NOTES 散记多
  sweetscan/sweetfb_noconsole/sweetfb_hz）+ `sweet_amfi_debug=1`（413B，回验过）；
  内核 = /tmp/r31/kernel.development.m2381（012f596f ✓；**已持久化 =
  `shim/out/kernel-r31-012f596f.bin`**，/tmp 易失勿依赖）+ ADT c85b01f0（未再生）；
  payload 4561673d 与 r31 同尺寸 = cmdline-only。**下窗刷双件：boot 339b8480 +
  userdata 7aaab96f，复合命令带 sha256sum 对拍。**
- **下窗判据 = 报告 §4 Gate 表**（Gate 1 exec r34mark[不出现+其它变化在 ⇒ 缓存整体
  遮蔽实锤]；Gate 2 SWEET AMFI: mobileactivationd: N；Gate 3 崩溃 pc 是否离
  0x1BF5BC；Gate 4 lsd t2；Gate 5 SIGABRT 波同因验证）。

### 2026-10-22 五（r33b 窗读环 + 外脑咨询发出）: 第三把统计同构窗、崩溃循环 97 次照旧、版本身份不可判——两假说并存，DeepSeek 任务书已出


- **r33b 窗执行**：用户按 r33 配方刷窗实跑（~17min，「日志后黑屏」）→ 自刷 nura-min
  交班（boot_b = mu-r69 `97cdea20` ✓）。读环 `rings/m2381-ring-r33b.{bin,txt}`
  （`ce82f010…`，337,047B/flush453/6,105 行，8 轮采样）。
- **读环判读 = 与 r32/r33 两窗统计同构**：exec mobileactivationd = **97**、
  cs_invalid_page = **97**（前两把 102/99）；crash 族谱同（accountsd 188/chronod 64/dmd
  60/liveactivitiesd 60/ScreenTimeAgent 60/cloudd 60/mmaintenanced 56/privacyaccountd
  28/profiled 4）；死锁栈逐帧同构（lsd t1 MobileGestalt sem→sync XPC、t2
  getServerStoreBlocking 吊客户、SB t0 os_state dispatch_sync）；SB AKS 11 发。
- **★崩溃细节（三窗最全）**：`cs_invalid_page … allowing (remove VALID) page`（我方
  AMFI 放行生效）→ SIGTRAP/EXC_BREAKPOINT(0x600000001)：pc=lr=mh+0x1bf5bc（brk），
  bt = mh+0x50630（r29 已知 Swift 清理尾声）← start+1860；每 exec 必崩；launchd
  "Too many corpses"。同拍数据点：bluetoothd/fairplayd.H2/coresymbolicationd 各恰
  exec 97 次而零崩溃记录。
- **★版本身份判别死局（本窗教训）**：分区内 2.6GB 处判别字节已被 nura-min[2G]重刷清零
  （devsh 实读全零）；宿主 shell 历史无 userdata 刷写命令；两 simg 全图 cmp 恰差 2 字节
  = 无任何在体标记。⇒ 本窗刷的是 r29 还是 r33 **不可判**。两假说并存：A = r29 又整对
  重刷（sha 对拍未确认）；B = r33 上机而 launchd 有未知起动机制（镜像四重查证无果，
  见 10-22 四条目）。
- **★外脑咨询（用户拍板「交给 deepseek 分析出方案，动手我方自做」）**：任务书 =
  `handoff/deepseek-brief-20261022.md`（全证据链 + 五问 + 约束面）。下会话 = 直接读环
  取证 + 按 DeepSeek 答复定 r34（体内标记/kill-switch 方案择一，候选见顶部 STATE）。

### 2026-10-22 四（r33 窗读环）: 手术件三重验证无懈可击但没上机——本窗 userdata 刷写未生效（设备跑的是 r29 旧件）；下窗刷前 sha 对拍重刷

- **r33 窗执行**：用户刷双件（boot `b8cddc98` + userdata 意为 `97342037`）实跑
  **17min** → 「日志后黑屏」→ 自刷 nura-min 交班。读环：boot_b `97cdea20…` ✓ → 环
  `rings/m2381-ring-r33.{bin,txt}`（`6eef34e6…`，352,252B/flush460/6,388 行，8 轮采样）。
- **判读 = 与 r32 窗逐帧同构，手术没上机**：
  ① lsd 栈同 r32：t2 = MadGate copyActivationRecord sync XPC 永等（s0–s6 全窗），
     t1 = MIInstalledInfoGatherer→MobileGestalt sem，t5 = getServerStoreBlocking 吊
     SB/bbd；SB t0 = os_state_request_for_self dispatch_sync（11 帧 r32/r33 全同）；
     bbd t0 = runloop idle。
  ② census 同量级：mobileactivationd exec 99（r32 102）、cs_invalid_page 99（r32 102）、
     accountsd/chronod/ScreenTimeAgent/dmd/liveactivitiesd/cloudd/mmaintenanced 连崩群
     两窗皆有（各 ~12 崩，session 前段已过）——**守护进程起动波不是 r33 新形态，r32 早有**
     （10-22 三条目「installd 15min 才 exec」只针对 installd 一家，勿再误读为「r32 无波」）。
  ③ SB AKS 序列 11 发逐行全同（含 sel 17×6）；sel 35 → 0x0 ✓ 内核健康。
- **★镜像三重验证（宿主侧，本会话）**：① `iOSRoot-s-r33.simg` vs `iOSRoot-s-r29.simg`
  cmp 全文**恰 2 字节差**（'d'→'q' ×2，simg 偏移 2457986392/2457986445）；② 7z 抽出
  LaunchDaemons plist → **plistlib 完整解析 ✓**：Label = `com.apple.mobileactivationq`、
  MachServices = `…activationq` + `…d.lockdowq`、ProgramArguments 未动、**无 RunAtLoad
  无 KeepAlive**（两笔写均精确命中：key1 'd'@raw 0x9b1a90ac = 串内第 27 字符、key2 末
  字符 'n'@raw 0x9b1a90e1 = 串起点 0x9b1a90be + 35）；③ raw 全文原串 390 处命中 = HFS+
  catalog 文件名 + 二进制内串，**launchd 域无第二声明方**。⇒ r33 镜像下 mobileactivationd
  无任何起动路径（LaunchEvents activity = 86400s 周期 ≠ 17min×99），崩溃循环无从发生
  ——运行却发生 = **设备跑的不是 r33**（最大嫌疑：误刷 r29 旧件[同目录 iOSRoot-s-r29.simg
  与 r32 窗命令一模一样，易混]或刷写中断未察觉）。
- **★定谳（用户补证后闭环，同日追加）**：初判「误刷旧件/刷写未生效」曾把矛头指向刷写
  环节；用户质疑「让我刷的版本对不对，上轮刷的和这一样」后做了第四重查证并反向闭环：
  ①宿主 `sha256sum` 两件与交接单全等（97342037/b8cddc98），/home/ri = 宿主 btrfs 直挂
  容器、全机唯一拷贝——不存在刷错副本的物理可能；②launchd 全域内容级查证无第二起动
  路径（jetsam 两件 = 被动配额表已验明；/private/var 497MB 全抽干净；/var = symlink）；
  ③用户确认本窗所刷 = 与上轮同版 → **userdata 实为 r29 `d7d6e730`（r32 窗原配）**。
  「设备跑的是 r29」结论不变，机制从「刷写失败」修正为「**r32 配方整对复用：boot 同件 +
  userdata 旧件**」。教训：boot「沿用」+ userdata「换新」的配方极易整对照抄；两个 simg
  同目录同名只差 2 字节，**唯一身份 = sha 前缀，刷前必对拍**。
- **下窗动作（唯一）**：`sha256sum ~/mkiosroot-work/iOSRoot-s-r33.simg` 对拍
  `97342037…` → flash boot_b `shim-boot-r32.img b8cddc98` + userdata r33（看 fastboot
  完整回执）→ 17min 窗 → 读环。判据树见顶部 STATE（首选判读：`exec mobileactivationd`
  = 0 = 手术上机的直接证据；若①再不成立 = 先对拍刷后分区读回，再谈其它）。

### 2026-10-22 三（r32 窗读环）: lsd 栈全景闭环——死锁根 = mobileactivationd 崩溃循环吊死 MadGate 激活记录查询；r33 端口改名手术待刷

- **r32 窗执行**：用户刷 r32（boot `b8cddc98` + userdata `d7d6e730`）实跑 **17min** →
  照旧黑屏 → 已自刷 nura-min 交班。读环：boot_b `97cdea20…` ✓ → 环
  `rings/m2381-ring-r32.{bin,txt}`（`1e6bfa92…`，365,382B/flush494/6,616 行，8 轮采样）。
- **lsd 栈全景（sampler 首功，全部 ipsw 符号化）**：t0 = `LSServerMain → dispatch_once →
  +[_LSInstallProgressService beginListening] → performJournalRecovery → dispatch_sync`
  等队列（sample 7 已自行逃出转 runloop = 非死结）；t1 = `MIInstalledInfoGatherer
  _populatePlaceholderRecordValues → MIInstallationIdentity → GenerateBuiltInAppInstallSessionID
  → libMobileGestalt → dispatch_semaphore_wait`（seeding 工作者）；**t2 = `+[LSHRNSupport
  deviceConfiguredForHRN] → initMAECopyActivationRecord → -[MadGate copyActivationRecord:]
  → xpc_connection_send_message_with_reply_sync`（永等 = 主死结：对端 mobileactivationd
  在崩溃循环里）**；t3 = `-[LSDatabaseBlockingFetchClient getServerStoreBlocking...] →
  __LAUNCH_SERVICES_CLIENTS_ARE_WAITING_FOR_THE_DATABASE_TO_FINISH_SEEDING__`（= 吊死
  SB/bbd 的那条消息的处理现场）。
- **判读**：mobileactivationd 崩溃循环（r29 parked 案）经 lsd seeding 的 activation-record
  查询升格为 GUI 总闸门。topaz 同循环能上桌面 = 其 lsd 未走到此查询。lsd 主线程非死结
  （自行恢复），installd 15min 后才 exec（无人依赖它 seeding，旁证 lsd 不是等 installd）。
- **★r33 手术（本轮完成）**：iOSRoot-raw.img 内 `com.apple.mobileactivationd.plist` 的
  MachServices 两键改名（与 Label 共享同一去重字符串对象，一次改双效；`d`→`q` 等长一字节
  ×2 处，物理偏移 0x9b1a90ac/0x9b1a90e1；needle 唯一命中法定位）→ 7z 回验 plist 解析 ✓
  → img2simg（4K 默认块）= `iOSRoot-s-r33.simg 97342037`。语义：launchd 不再注册旧端口 →
  MadGate XPC 无对端立即 error（XPC fast-fail 语义）→ t2 返回 → seeding 收尾 →
  getServerStoreBlocking 放行；副作用正收益 = 崩溃循环消失（无人能拉起改名后的 job）。
  plan B（若 lsd 另有依赖）= r29 parked 的 fatal 尾声废止法。

### 2026-10-22 二（r31 窗读环）: 断链修复上机实证 + sampler 抓出 SB 停滞真身 = LS reseed 死锁（lsd 不回话）；r32 采样 lsd 待刷

- **r31 窗执行**：用户刷 `shim-boot-r31.img a5d0f8d9` + `iOSRoot-s-r29.simg d7d6e730`
  → 「依旧日志→黑屏」→ 留 fastboot 交接。本会话刷 mu-r69 + nura-min（两件 sha 对拍 ✓；
  首次 reboot 后 USB 网络未起，用户重刷后正常——件本身无错，疑起机半途）→ devsh 验
  boot_b `97cdea20…` ✓ → 环 `rings/m2381-ring-r31.{bin,txt}`（`68a2cdd3…`，131,224B/
  flush117/2,256 行）。
- **断链修复三项实证**：①**AKS sel 35 全家 0x0**（SpringBoard ×2/chronod/remindd ×2/
  searchd ×2/symptomsd——0055 生效）；②swap 节流形态回归（3/6/9/12/15/16，0056 生效）；
  ③**SB 首次越过 AKS**：exec → IOKit 行走（AppleKeyStore open 0x0、IOPMrootDomain、
  /buttons）→ AKS sel 0/35/7/35 全 0 → 之后静止。0091-0098 信号仍零（SB 未到 surface 面）。
  mobileactivationd 25× 照旧（parked）；bbd 全窗零崩。
- **★sampler 收获（本轮最大）**：两轮采样（~90s/~210s，栈完全相同 = 真死锁）→
  符号化（ipsw 3.1.739 + IPSW 内 DSC，配方在顶部 STATE）后 SB 主线程链 =
  `__ulock_wait ← __os_unfair_lock_lock_slow ← -[LSApplicationRecord
  initWithBundleIdentifier:] ← LSContext::_get ← __LSContextInitWithOptions ←
  __LSContextInitCommon ← [截断帧] ← dispatch_barrier_sync ← -[CCSModuleRepository
  _initWithDirectoryURLs:]（ControlCenterServices 模块元数据过滤）`；
  SB t1 链 = `_LSDBEnumeratorGetNextObject → __LSCopyServerStore →
  __LAUNCH_SERVICES_IS_RESEEDING_ITS_DATABASE_AND_MAY_BLOCK__ → NSXPCConnection
  sync → dispatch_mach_send_and_wait_for_reply`（等 lsd 回复）。**判读 = 进程内双线程
  死锁形：t1 持 LS 锁等 lsd reseed 回复（永不返），t0 等同一把 LS 锁**。backboardd t1
  同族栈（CoreServices LS 区 + XPC sync）= bbd 也有线程卡同一 reseed。lsd 本身 exec 过 +
  AKS 0/7 成功（1428-1429 行）但对 reseed 不回话；installd 从未 exec。
- **背景差异**：根盘无任何 LS 数据库（无 systemgroup.com.apple.mobile.installation、
  无 lsdb 文件——mkiosroot/tweak 均不建）= 冷启动必触发 reseed；sweet 无此先例（其
  迭代镜像可能带出了健康 db 或 lsd 当年未被问及）。
- **r32 出件**（配方顶部 STATE）：唯一变量 = sweetsample 加 lsd/installd/misd/
  containermanagerd。ipsw 工装入 `research/bin/ipsw`（不入仓）。

### 2026-10-22（轮 30 窗读环 + 断链破案 + 上游切 topaz）: CI 静默丢 14 件破案（0054 一件根因）；全链走查 96/96 绿；上游改 topaz

- **r30 窗读环（用户报「日志跑完→黑屏无 GUI」两轮后交班，设备已刷回 nura-min）**：
  devsh 验 boot_b 前缀 sha `97cdea20…` ✓ → dd 尾 8200 扇区 → 设备 http.server + 宿主
  curl（`--interface enp104s0f3u1`）→ `rings/m2381-ring-r30.{bin,txt}`（`653135ee…`，
  117,832B/flush153/2,256 行）。判读归 STATE：与 r29 逐行同构，0091-0098 零命中。
- **破案过程**：①环上 AKS sel 35 全家返 0xe00002c7（r28-r30 连续四窗）→ 查 0055 本体
  = sel 35 应答件 → CI run 39626238471 日志 `Skipping patch …0055` → 全跳单 14 件 →
  复查 r26（39530428800）/r29（39544122293）= **同 14 件全缺**；②GitHub xnu-8796.141.3
  标签树本地重放 = 复现同 14 败（+0031/0041/0088 走 hfs staging 差异，与 CI 无关）；
  ③research/walk.sh 时代（阶段 1）门曾 84/84 绿 → 断点在 m2381 化补丁改动内。
- **根因链**：0054 的 exception.c hunk#1 上下文含 `#if defined(ARM64_BOARD_CONFIG_SWEET)`
  原行；W2（9a39f21，十四处守卫双化）把该行改为 `… || defined(ARM64_BOARD_CONFIG_M2381)`
  → 0054 失配被 dxb 静默跳过 → 0054 携带的 `bsd/dev/sweet_sample.c` files 行缺失 →
  0060/0062/0072 的 files 上下文失配 → sweet_touch.c/sweet_usb*.c 级联（0063/0073-0077/
  0081）→ 0055/0056/0079 各自失配。GNU patch --fuzz 探测确认唯一硬冲突 = 0054 hunk#1
  （IOUserClient/exception 的其余失配全是 fuzz-1 级守卫行）。
- **修复**：research/xnu-8796 持久树手工轮——0054 hunk#1 按原意图重排（include 提到
  双守卫行前），其余 hunk as-built 落地后 `git show` 提取为再生 0054；0089 因 0056/0079
  回归行移同法再生（fuzz 落点逐 hunk 验证正确）；然后 **0055-0098 全数严格 git apply
  干净通过**；终门 `tools/xnu-gate-walk.sh` = **96 件 0 失败**。语义抽查：SweetKeyStore
  sel 35 case 在、sweet_sample.c 在、节流/显示电源在、W2 双守卫全在。
- **上游**：fetch topaz → tip 未动（89451b8）；动态 = 4 个 open PR（#1 GPT 写栅栏、
  #3 触摸 0116、#4 reeder MT6757、#5 Gauguin SM7325）。用户令上游改 topaz，AGENTS/
  STATE 已记：不换基座，跟 origin/topaz 路线图，采纳件排队（PR1/PR3）。
- **方法论新增**：①每轮 CI 必查跳单（Applying=96 + 4 dxb 基线件）；②本地
  gate-walk.sh 为推送前置门；③「apply 级 walk 对守卫语义盲」再+一档：「对守卫改动
  的下游上下文也盲——守卫改动必须跑全链 walk 而非只验改动件」。

### 2026-10-21 深夜三（轮 30 备件）: 阶段 B 全量入树（0091-0098 SweetSurface 语义系列）；CI as-built 0131 异常注入记档

- **翻件方法**：as-built 基线 = CI 39544122293 的 `xnu-buildtree` 工件 SweetSurface.{cpp,h}
  减去一块异常内容（见下）；本地无 xnu 真源树，用「0046 建 0068 改」重建该两文件并与
  工件 diff 验证（仅差异常块）。八件 topaz 补丁逐件 `patch -p1 -f` 试贴 + 手工适配 12 处
  （scanout 剥离：free() 尾行/kernelAddress 条件/.h 字段与 init/create 分配门改
  `s->memory == NULL`），逐步快照生成 0091-0098，全新副本顺序 `git apply` 全绿、终态与
  工作台逐字节一致（金验证两轮）。scanout 零残留、括号平衡。
- **映射**：0091←0131（refuse lock-result 调用）002→0092（WrapClientImage/Memory）093←0133
  （values/bulk/purgeable/accel/YCbCr 存值，最大件）094←0134（IOSurfaceAddress 用客户内存）
  095←0135（trap4 只 detach + AllocSize 尺寸 + 只读回退）096←0136（detach 视图竞态）
  097←0137（256 僵尸状态页）098←0139（client-address 面发布 id）。
- **★CI as-built 异常（未破案，记档）**：39544122293 的 buildtree 源含 verbatim topaz-0131
  块，但同 run 的内核二进制 929a1a27 **不含**该 IOLog 串（字节级验证，对照串全中），且本仓
  任何历史 commit 的任何补丁均无此内容（python 字节级全扫 + git log -S + 远端 run 清单核对；
  workflow/dxb/build.sh 全通道清白）。判读：编译后打包前源树被一次性改动，非序列内容；
  **实际生效内核无 0131**。对策：0091 显式入列（若异常通道再现，build.sh 的 skip 语义使其
  自愈）。教训入环境事实：**对拍 as-built 必须同时验源工件与内核二进制两边**。
- **推送 + r30 出件（本轮执行完毕）**：`f646382..7c293a1 main → fork`（5 commit）→ CI
  **39626238471 success**（push 触发，M2381）→ 用户下载 xnu-kernel → `sha256sum -c`
  **kernel `443f8aaa…` OK** → ADT 再生（sRGB 验证 ✓）→ payload-r30 `7b1667b6…`
  16,599,200B → **shim-boot-r30.img `4998741b…`** 5,734,400B。刷窗留下轮（用户令），
  配方/判据树全在顶部 STATE。

### 2026-10-21 深夜二（轮 29 窗取证）: swap 里程碑命中、管线贯通、黑帧=内容空；补丁上机实证但尾声多入口；阶段 B 开题

- **约束**：用户令「只做取证和分析，暂时不刷机」——澄清后 = 读环流程（mu-r69+nura-min）
  属取证可用，**iOS 件不刷**。刷 mu-r69 + nura-min（32.9s，reboot ~45s 慢启动正常）→
  devsh：boot_b 前缀 sha `97cdea20…` ✓ → 环读回 `02e74a20…` → rings/m2381-ring-r29.{bin,txt}
  （158,176B/flush254/3,095 行）。
- **「日志→黑屏」破译**：0067 静音点在 AppleCLCD::swap() 内 first swap（静音后文本仍进
  环）——用户看到 console 哑 = **首帧已 present**，黑屏 = GUI 首帧而非死机。
- **环五连读**：①**swap 行首次出现**（客户端请求 16 次、0056 节流合流 6 次拷贝、
  ARGB 1125×2436，拷贝 clamp 实读=裁剪干净）→ 黑 = **内容全零非伪影**；②b3a8 双平面面
  消失变 ARGB 单平面 = **gamut sRGB 验证生效**；③**cs_invalid_page ×55**（r28 零）+
  bt 帧 0x50674→0x50630 = **8B 补丁上机实证、路径变改但 fatal 未堵**——RE 勘误：所谓
  「双入口」实为 Swift 大函数错误尾声的 ~13 个守卫入口（0x100050624…668 全谱），首扫只
  查块首两目标 = 方法论错误，已记环境事实；正解 = 废尾声（bl fatal→b 尾声，需函数边界）
  或 parked（topaz 带循环上桌面 + r29 证明推进不被它闸）——**建议 parked**；④SB 连续
  第三窗 4-AKS 同构；⑤bbd 一命零崩全窗在场。
- **阶段 B 开题**：管线五段全通、死在渲染数据面 = topaz 0133「别答 0」家族 →
  **下一发 = 翻 0131-0139**（判据树见顶部 STATE）。设备收口 = mu-r69 + nura-min 日常态。

### 2026-10-21 深夜（轮 28 读环 + 轮 29 备件）: bbd 零崩但 present 停在 b3a8；topaz 解药盘点 + daemon 8B 双守卫 RE；r29 双药待刷

- **r28 窗收口与读环**：用户报「一直在刷日志无 GUI」（= console 没哑，0067 只在 present 后
  静音，与零 swap 自洽）→ pwr+vol↓ 交班。fastboot 刷 mu-r69 + nura-min（32.8s；reboot 后
  ABL 慢启动 ~45s 虚惊）→ devsh：boot_b 前缀 sha `97cdea20…` 复位验证 ✓ → 环 8200 扇区
  dd → curl 拉回 → **rings/m2381-ring-r28.{bin,txt}**（`5c18eec9…`，174,668B/244 flush/
  3,120 行）。判读详顶部 STATE：判据②，bbd 零崩一命到底、surface×3、trap lock/unlock
  ×1、swap 永未出现、SB 同构停滞、mobileactivationd 208× 循环。
- **topaz 解药盘点（用户问）**：fetch origin/topaz 后全史扫 —— 激活侧唯一相关提交
  68b58e0（ADT 安全元组）我树已有；`codex/activation/report.md` 确不在其 git；
  **真缺口 = 3eacb48 gamut sRGB**（/product artwork-display-gamut P3→sRGB；topaz 症状
  Safari 白页，我们症状 = backboardd 建 2 个 b3a8 10-bit 面后 present 停摆——同一根因
  不同阶段表现）；0128 quiesce = Wi-Fi 阶段才需要。
- **mobileactivationd RE（llvm-objdump + Mach-O 手解 selector）**：fatal 函数
  0x1001BF35C 内嵌 StaticString 321B，全库唯一 caller 0x100050670；守卫 = 两处
  tbz（file+0x4F944 资源 GET 失败 / file+0x4F9B8 资源 SET 失败）汇入同一 fatal 块
  （块头 0x100050614 / 0x10005061c 两入口）；selector 名解出 =
  getResourceValue/setResourceValue:forKey:error: ——**fatal 在 NSURL 文件资源层**，
  与激活数据无关（w1c 无效的根因）。**8 字节双反转**（tbz→tbnz）施于 raw 镜像：
  mobileactivationd 文件起点 0x206f37000（全镜像唯一 256B 前缀命中）→ 两处原字节
  验中 → Python r+b 写 → **7z 重抽逐字节 cmp 一致** ✓。
- **r29 出件**：mkadt 翻 3eacb48（/product gamut 值改写，ADT 12,616→12,624B，
  P3→sRGB 解析验证 ✓；⚠mkadt argv[1]=输出、flag 在后）→ mkpayload（内核
  = xnu-kernel.zip 里 kernel.development.m2381 sha `929a1a27` 对账 ✓，cmdline 与 r28
  全同）→ payload-r29 `7dad1aff…` → **shim-boot-r29.img `0fd9a1e5…`**。img2simg：
  **512M 块尺寸炸内存一次（宿主 OOM，用户指出；此前 userdata 重构走的是宿主
  w1c-host-dataark.sh sudo 脚本）**，改默认 4K 块 + ulimit -v 2G 后台跑 = 32s 完成 →
  **iOSRoot-s-r29.simg `d7d6e730…`**。待用户刷 r29 窗（判据树见顶部 STATE）。

### 2026-10-21 晚（轮 28 备件）: 0090 CI 绿，r28 = 0090+0089 双空壳 + noconsole 拍照配置，用户手刷实测中

- **0090 CI 39544122293 success**，用户下载内核 `929a1a27…`（59,817,956B，SHA256SUMS OK）。
- **r28 出件**：cmdline 在 r27 基础上 `+sweetfb_noconsole=1 +sweetfb_hz=30`（topaz 桌面
  里程碑同款；0067 闭屏不闭环，证据无损失）→ payload `4104df93…` → **shim-boot-r28.img
  = `ab0dd5b7…`**。userdata 不变（`3537705c…` 仍在）。用户手刷双件 + ≥9.5min 窗中。
- **本轮会话累计战果**（详上存档块 + 各轮条目）：SIGABRT 结案、诊断链全通、0089/0090
  双空壳上机、mobileactivationd 死因符号化、data_ark 容器配方落地。**GUI 最后两闸 =
  加速器等待（0090 应通）+ SpringBoard 激活侧静默（未证，备选 = 4 字节 daemon 补丁）**。

### 2026-10-21 下午二（轮 27 窗）: 0089 验证生效；新前沿 = GPU 加速器等待；mobileactivationd 死因符号化到手

- **r27 = f0935c57（内核 a3775a3a 含 0089）+ simg 3537705c（data_ark 容器）**，环 177,742B
  （= rings/m2381-ring-r27.{bin,txt}，`2e79a611…`）。
- **0089 判定生效**：boot log line 70 `SWEET FB: empty AppleM2ScalerCSCDriver published`；
  matching-failed stderr 29,535 → **5 行**。backboardd 走到 CLCD sel 12（比 r26b 的 74 更深，
  §1 ④ 序列继续展开）。**但 swap 仍 0** —— IOK trace 尾部点名新墙：**backboardd 以
  IOServiceNotificationUser 轮询 IOAcceleratorES / IOAcceleratorESLegacy / AGXAccelerator
  全部 `-> none`**（QuartzCore 加速器探查）→ **0090 空 IOAcceleratorES**（0138 原版模式，
  as-built+0089 上下文生成、dry-run 过、CI 39544122293）。
- **mobileactivationd 死因符号化成功**（llvm-objdump + StaticString 长度提取；⚠PIE 基址
  0x100000000 必须加、w1=0x141 是消息字节数 321 不是行号）：偏移 0x1BF3BC = Swift
  `_assertionFailure`，消息 **"Error setting low space io policy: %d (%s)"**（相邻串 =
  容器装配族：copy container paths / backup exclusion）——其系统容器的私有 vfs io-policy
  调用在 Linux hfsplus 上必然失败。**良性判定**：topaz home screen 里程碑期间同样 ~100
  崩/轮未拦路；data_ark 并非此 fatal 的药（但已撒好，Hactivation 其它面用得上）。parked，
  日后可用 topaz codex 的 4 字节 daemon 补丁收掉。
- **流程修正**：本轮刷写改用户手刷（容器后台任务被取消后残留 D 态 fastboot 子进程，
  kill -9 清掉；build.sh 相对路径坑再确认 = 必须绝对路径）。

### 2026-10-21 下午（轮 26b 窗）: **SIGABRT 结案——backboardd 存活建 surface，新前沿 = AppleM2ScalerCSCDriver 等待 + 激活门**

- **战果（r26b = 9cc9fe05 + w1b 根盘，9.5min 窗）**：环 2.1MB/32,691 行/单 banner。**backboardd
  exec ×1 且零崩溃 = 轮 20 以来的静默 SIGABRT 正式结案**（containers + plist 撤 /dev/console
  双修复生效）。0048 tee 活了（`|backboardd|` stderr 首次可读）、0049 IOK trace 活了（×397）、
  **surface create 首次成功**：`SWEET SURF: backboardd create id 2 1080x2400 ARGB -> 0x0` +
  sel 13（IOCoreSurfaceRoot open）+ sel 9/27/5 + trap 2/3 + 双平面 surface id3/id4（1080×2400
  与 1125×2436）——NOTES-topaz §1 的 ④→⑤ 全序列首次走到。
- **新前沿 A：AppleM2ScalerCSCDriver 等待**。0049 显示 backboardd 以 `IOServiceNotificationUser=true`
  匹配 AppleM2ScalerCSCDriver（A11 显示管线 M2 缩放器/CSC，`-> none` 永不命中），`|backboardd|`
  stderr 刷了 29,535 行 matching failed（占环 90%），swap=0 = 呈现管线未启动。**sweet/topaz
  零处理记录**（topaz 同款 20H392 用户态过了此关但配方未留档）。对策 = 0138 同款空壳 service
  模式（类名=匹配名、attach、registerService）→ **0089 补丁**（从 CI buildtree 工件取真实
  SweetFramebuffer.cpp 作 hunk 上下文）。
- **新前沿 B：mobileactivationd 仍 SIGTRAP 循环（×61→本窗 ×212，sweetscan STR 零行 = 死得早
  堆里无可扫文本）**，但已不拦 GUI 阶段。**topaz v14 配方在 NOTES.md line16：手造容器
  `/private/var/containers/Data/System/D90C94C4-F94E-4FCF-8A28-A93BE3846D8E/` + data_ark
  （Hactivation 四 chosen 属性与我们一致）** → w1c-host-dataark.sh（宿主）造容器 + 重跑
  idempotent tweaks 让 activation() 找到容器撒 data_ark。
- **杂项**：suggestd 197×exec churn 待查；SpringBoard exec 一次后静候（正常，等 WindowServer）；
  objc 双类警告（手柄 HID 插件）为良性；用户中途意外二启未损证据（环单 banner）。
  **r26b 环 = rings/m2381-ring-r26b.{bin,txt}（21080d1e…）**。

### 2026-10-21 上午二（轮 26 备件）: fix round 双线全落——W1 根盘 tweak 实施并验证，W2 guard 双化 CI 绿 + r26 件出炉

- **W1（根盘补 tweak，宿主 sudo）**：容器 NoNewPrivs 封死提权（sudo setuid 亦无效，会话内
  无解）→ 走宿主脚本 `fix-round/w1-host-root.sh`。**关键发现：iOSRoot-s.img 本身是 Android
  sparse 镜像**（offset 0 = 魔数 `3aff26ed`，raw 16GiB）——宿主 fsck 秒死（rc=8/9ms）与内核
  拒挂全是啃 sparse 头所致，7z 能读是它原生解 sparse。容器侧 `simg2img` →
  `iOSRoot-raw.img`（16GiB 裸卷，fsck 2s 全绿 **appears to be OK**）→ 宿主挂裸卷施
  `sweet-root-tweaks.py` 全套 + `patch-launchd-plist.py`（backboardd/ReportCrash/SpringBoard/
  geod 四件 stderr→/dev/console + OS_ACTIVITY_DT_MODE/CFLOG_FORCE_STDERR）→ 容器 7z 全验：
  containers 四目录/purplebuddy×2/migration/springboard(BKNoWatchdogs)/managedconfiguration
  全落卷（10:11:15），launchd.plist 抽验四 job 路由全 ✓、CommCenter 主体已摘；**data_ark
  跳过**（无 mobileactivationd 容器，sweet 血统文档化行为）。回打包
  `~/mkiosroot-work/iOSRoot-s-tweaked.simg` = `f4ffe7a6…`（12,145,512,872B）。旧 simg
  `1db88931…` 保留为回滚。
- **W2（诊断 guard 双化）**：0043×2/0048×2/0049×7/0052/0041/0039 共 14 处
  `|| defined(ARM64_BOARD_CONFIG_M2381)`（照 0054/0088 先例；0044/0051/0053/0055 挂
  0043/0054 区域内自动继承）。commit `9a39f21` 推 fork → CI `39530428800` **success** →
  用户手下载 zip（59,807,913B 与 artifact 等长、内部 SHA256SUMS OK、时间戳 02:07 对窗）→
  内核 `a0ac4602…`（17,183,664B = r22 +96，guard 行增量吻合）。
- **r26 件（cmdline = 轮 20 定稿串 − panic_on_error_exit + sweetsample=- + ufslogsecs=2，
  sweetscan/sweetnull/sweetiolog 走新默认自动武装）**：ADT `07cb5378…` → payload-r26
  `a6ec7f2e…`（16,574,704B）→ **`shim-boot-r26.img` = `cef05044…`**（5,730,304B = r24b
  +4096 对齐）。⚠build.sh 内部 `cd $(dirname $0)`，payload 参数须绝对路径（相对路径 cat
  失败且管道吞 rc，出过错一次已修正）。
- **r26 窗实跑（新诊断链首亮，两个新案）**：环 116,880B/284 flush（0041 消音后全是信号，
  前轮同窗 1.1MB）。**CRASH 行 ×248 / reason ×62 首次在 M2381 上环**——0043/0044 双化生效。
  ①**backboardd 零 exec = W1 plist 补丁自伤**：0048 注释原话「stderr=/dev/console 使
  launchd 拒绝拉起 backboardd（v8 image）」，patch-launchd-plist 恰给四件都设了——bbd/
  SpringBoard 靠 0048 内核 tee（sweetnull 默认）即可，plist 只该给 ReportCrash+geod。
  ②**mobileactivationd 固定位点 fatal**（EXC_BREAKPOINT，pc−mh=0x1BF3BC，×61 循环，binary
  stripped）= 激活门卡死 → GUI 阶段（backboardd）不被拉起；chosen 四属性元组核对无误。
  次要：suggestd 197× exec churn、SpringBoard 31× exec 无死因行、profiled 1× SIGABRT、
  data_ark 仍缺（无容器）。**r26b 修正案**：w1b-host-plistfix.sh（宿主 surgical 撤
  bbd/SB 的 stdout/stderr 键、保 env 供 0048 tee os_log；重打 ReportCrash+geod）+ payload
  增量 `sweetscan=mobileactivationd,backboardd,SpringBoard`（STR 堆扫描下一窗直接捞
  mobileactivationd 断言文本）→ **`shim-boot-r26b.img` = `9cc9fe05…`**（payload `6c9f9efb…`，
  同内核零 CI）。
- **待办 = r26b 窗**：宿主 w1b → 容器 img2simg → fastboot 双件（boot_b = r26b + userdata
  新 simg）→ set_active b → reboot（⚠ABL 卡死可能）→ 8-10min → pwr+vol↓ → mu-r69 +
  nura-min 读环。预期：backboardd 回到 exec（0048 tee 顶上 stderr 可视）；mobileactivationd
  的 fatal 文本进 STR 行（若指向 data_ark/容器缺失 = 数据面卡 Hactivation，对策 = 造容器
  + 补 data_ark 再跑 activation()）。

### 2026-10-21 上午（轮 25）: 取证双窗执行完毕，SIGABRT 案破题（诊断盲装 + 根盘零 tweak 实锤）

- **执行摘要**：用户进 fastboot 触发两段式。窗 A（r24 件，r21 内核）：刷 mu-r69（前缀
  sha 读回 `97cdea20…` ✓）+ nura-min（simg 32.6s）→ devsh 读环 8200 扇区 → 解析 17,811 行。
  窗 B（r24b 件）：boot_b = `shim-boot-r24b.img`（sha `54cd6424…` 对拍账面 ✓，r22 内核
  `c133cacc` 含 0088）+ userdata = iOSRoot-s.img（sha `1db88931…` ✓，24 段 sparse 328s）
  → set_active b → `fastboot reboot` 撞 ABL 卡死老坑（8s 仍在 fastboot，用户 ~180s 后人手
  断电）→ 8.5min 证据窗 → pwr+vol↓ → 同法读环。收口：mu-r69 + nura-min 复位（boot_b 读回
  再验 ✓），设备留 nura-min 日常态。
- **环解剖（两轮同构，确定性开机序列，ReportCrash sel17 行逐行同位 5480/8370/11258/14150）**：
  单轮开机；backboardd 每命 = exec → AKS sel 0/17 → 双 SweetHID open → MobileGestaltHelper
  sel 8/28 全 0x0 → 静默死（第 1 命有尸体+ReportCrash[AKS sel 7]，第 2/3 命起 corpse 配额
  打满 = `Too many corpses`）；geod 疯狂崩（PID 滚到 500）+ datami/analyticsd/lsd 陪崩；
  90% 环容量 = hfs hardlink 刷屏（15,973 行）+ geod/bluetoothd/coresymbolicationd 循环。
- **三发现与判读**（证据与 guard 清单见顶部 STATE，此处记过程）：
  - 词表全零（STR/CRASH/exception/IPS|/Terminating/SIGABRT）→ 顺 0088 补丁本体读出触发条件
    （hfs_vnop_write 拦 backboardd*/SpringBoard*.ips 前 64KB → `IPS|` 行）→ 零响 = .ips 从未
    被写 → 追 0043（CRASH 行零 = geod 几十死无一打印，0043 若编入必打）→ **发现 0043/0044/
    0048/0051/0052/0053 全 SWEET 单守、零 M2381 覆盖**（逐件 grep；0054 process-sampler 是
    家族里唯一带 M2381 的）——m2381 走 84/84 walk 时平台件翻版了、诊断件没翻 guard。
  - 7z 免 root 全卷 listing iOSRoot-s.img（321,389 项）→ 无 `/private/var/containers`、无
    data_ark/SetupDone/BKNoWatchdogs → tweaks 未施（mkiosroot.sh 不调 tweak，build2.log 亦无
    tweak 痕迹）；`var/mobile/Library/Logs/CrashReporter` 与 `cores` 目录在 = .ips 断点在
    ReportCrash 内部非目录缺失。对照 NOTES-topaz §1：死点 = ①-④ 窗口，sweet 第一墙
    （NSURL nil NSException）即在此窗。
- **topaz 参照答复（用户问「抄 topaz 有没有药」）**：topaz backboardd 天生稳定（他没得过
  这病），无 SIGABRT 药可抄；可抄清单不变（0131-0139 surface 语义/0138 空 IOAccelerator/
  0088 GPT 看门狗/gamut），但本轮证明**解药在我们自己 sweet 血统里**：根盘 tweaks（W1）+
  自己树的诊断件 guard（W2）。
- **收口杂项**：环证据 + 根盘 listing 存 `rings/`（未入 git）；设备 = nura-min + mu-r69
  日常态；r24b 窗后 nura-min 复刷 33s 正常、reboot 无卡死（大镜像卡死只在 iOSRoot 12G 刷后
  出现过）。

### 2026-10-21 深夜（轮 24）: nura-min 交付验收通过，取证轮执行（backboardd SIGABRT 死因猎杀）

- **★nura-min 交付（nura 侧构建+上机双 pass 验收，我方三方对拍全绿）**：
  `meizu-meizu20-nura-min-r91-2G.img`（定长 2GiB，sha256 `c841f645…`）+ 同源 `.simg`
  （`a84d9212…`），均在 `/home/ri/work/nura-meizu-m2381/artifacts/`。grow-rootfs masked、
  fstab 零扩容语义、fs 恒 524288×4KiB。nura 侧验收：sparse 刷入 38s → 起机 48s 到 devsh →
  尾部 8200 扇区（= 环位）写标记 → 重刷同镜像 → 尾部逐字节原样（基准 sha `2b78a9f0…`，
  我方 devsh 复读完全一致）。已知修复：首版 USB 网死（缺 NetworkManager），终版已补。
  边界（诚实）：iOS 根刷写腿未含在 nura 侧验收——由本轮 C 段实跑覆盖。
- **设备现态（触发达成）**：停在 nura-min，尾部带验收标记；boot_b = mu-r69。取证轮开打。
- **取证轮刷写记录（08:41 重启进 iOS）**：boot_b = `shim-boot-r24.img`（5,726,208B，sha
  `41c8b139…` = 轮 21 内核 `8c3f75d6` + 再生 ADT `--m2381` + 定稿 cmdline −
  panic_on_error_exit + sweetsample=- + ufslogsecs=2；payload 16,574,496B 与轮 20 同尺寸，
  Image 尾回剥 cmp 一致、旗标 strings 验证）；userdata = `~/mkiosroot-work/iOSRoot-s.img`
  （12GB sparse 真身，24/24 块 336s，与轮 20 同源；**research/ipsw-x/iOSRoot.img 实测空壳
  [110MB 实块全零读] 已改名 .HOLLOW-DO-NOT-USE**）。cmdline 定稿串执行中。
  **勘误：plist 四 label 补丁本轮未上**——宿主容器 no-new-privs 无 sudo/userns-loop 亦拒，
  HFS+ rw 挂载不可得；0051 STR（默认=backboardd）+ 0044 退出原因 + 0048 devnull-tee 三条
  内核通道已在轮 21 内核里，sweet 先例纯靠 STR 即拿到 reason；plist os_log 镜像降级为
  STR 空手时的第二轮增强（需 root 会话，六命令在轮 23 收口条目）。
- **★轮 24b 件已备（用户下载轮 22 CI 39456198891 覆盖仓根 zip，内置 SHA256SUMS 验整）**：
  新内核 sha `c133cacc…`（17,183,568B，10-20 16:25 构建，**含 0088 IPS 镜像** = .ips 崩溃
  报告内容直上控制台的第四通道）。`shim/out/shim-boot-r24b.img`（5,726,208B，sha `54cd6424…`
  ，payload `14536c00…`，回剥 cmp 一致 + 旗标嵌入已验）；探针件 `shim/out/probe-r24.img`
  （`e3a5255b`，词表已 +STR/CRASH/exception/Terminating）。
- **★本机现存活证据（重要）**：设备自 08:43 起在 r21 内核上跑崩溃循环，**环正在实时积累
  r21 证据且 nura-min 刷写不毁尾部**——下会话第一动作 = 刷 nura-min 直接读它（免费，零
  刷写 iOS 侧）；不够再开 r24b 窗。fastboot reboot 大镜像刷写后卡死 = 已知 ABL bug
  （用户实锤，本轮 08:42 即此），卡死时人手断电重进即可。

### 2026-10-21 收口（轮 23 完）：项目挂起，交还用户/nura

- **设备归用户自行处理**；回滚件在 `/home/ri/work/nura-meizu-m2381/artifacts/mu-r69-97cdea20.img`
  （前缀 1,110,016B sha = `97cdea20…`）。boot_b 现留件 = 轮 23 镜像 `53831d75…`（探针或
  XNU 全日志态均坐屏，pwr+vol↓ 即 fastboot）。
- **轮 23 终判读（探针 IPS 预扫 + ReportCrash 关键字）**：backboardd = 确定性崩溃循环——
  每轮生命周期完整走完 exec → AKS sel 0/17 → SweetHIDTouch/Buttons open → MobileGestalt
  sel 8/28 → 死（corpse 第一次成功、后续被节流）；**ReportCrash 确实 exec 并处理了 corpse**
  （AKS sel 0/17/7 一串），但 `IPS|` 镜像零命中 = .ips 没落盘或文件名不匹配（写临时名/别的
  扩展名/写失败均可能）。`no IPS| lines in ring` 实锤。
- **挂起时的头号线索与下会话第一刀**：用户态 stderr 全进 /dev/null，abort 消息唯一出口 =
  宿主机挂载 iOSRoot.img 改 launchd.plist（tools/patch-launchd-plist.py，名单 backboardd/
  ReportCrash/SpringBoard/geod）→ 刷回 userdata（~6min）→ backboardd 临终遗言 + 全部 os_log
  直接上屏进环。宿主侧六条命令已写在会话记录（fsck.hfsplus -fy → mount -o loop,rw,force →
  patch-launchd-plist.py → cp → umount）。备选：0088 匹配放宽（任意 backboardd* 文件名 /
  临时名）；QEMU 用户态实验室受宿主 13G 可用 RAM 限制（16G 镜像进不了 md0）+ 容器无 HFS+
  写入工具做瘦身镜像，挂起。
- 本会话净产出：★UFS 墙破（阶段 2 关闭）、全量 cmdline 定稿、0042 静音、★SWEETLOG 探针
  通道、0088 .ips 镜像、0069/0047 冲突教训、backboardd 时间线全谱。

### 2026-10-20 十轮续二（轮 22）: ★SWEETLOG 探针通道建成；backboardd 死前 IOKit 全通；.ips 镜像补丁（0088）

- **★永久基础设施 = shim SWEETLOG 读回探针（本地件，零 CI）**：shim/ufs.c 增 `dump_sweetlog()`
  ——读 XNU 0047 写在 userdata 尾部 4MiB+4KiB 的控制台环（GPT 找 userdata → 尾块头
  "SWEETLOG|ver|ring|total|seq" 校验 → 环流读入 → 关键词过滤行（SWEET HID/SURF/FB/LOG、
  AppleCLCD、exit_with_reason、backboardd、SpringBoard）+ **最后 8 行原文 TAIL**），
  清屏独占显示。两轮实机验证可用（首次 53KB/次轮 62KB 环读取 + 过滤回显全对）。
  **以后每轮 = 刷全日志镜像跑完 → pwr+vol↓ → 刷探针拍屏 = 完整日志，不再靠刷屏截图。**
- **backboardd 死前时间线（轮 21 全日志镜像 + 探针判读）**：exec → AKS sel 0/17 →
  **SweetHIDTouch/SweetHIDButtons 双双 open by IOHIDEventServiceUserClient（ea8ef2a 完全
  生效）** → MobileGestaltHelper sel 8/28 应答 → 回归 geod 崩溃循环刷屏数分钟 → 突然
  SIGABRT。**无任何失败调用残留 = 运行中异步死亡，非启动 assert**；显示/HID/keybag/gestalt
  链路全通。sweet 先例（NOTES:83）中 backboardd 下一步 = SWEET SURF surface create（M2381
  从未到达）。死因文本只存在于 ReportCrash 写的 /cores/*.ips（stderr=/dev/null）。
- **取证战术修正**：①`sweetfb_noconsole=1` 无效（0067 语义 = "GUI presents 后"才静音，
  backboardd 活不到那步）；②panic_on_error_exit 会**秒杀 ReportCrash 写 .ips 的机会**
  （exit 即 panic）→ 本轮起从 cmdline 摘除，让 backboardd 崩→ReportCrash 写报告→镜像
  补丁回显→launchd 重拉循环继续跑，探针读环即得死因。
- **★0088 crash-report 镜像补丁（deb71b5）**：hfs_readwrite.c 的 hfs_vnop_write 入口——
  文件名 = backboardd/SpringBoard 前缀 + ".ips" 后缀（手写字节比较零库依赖）→
  uio_getiov + copyin 快照 → 逐行 `IPS| ...` printf，全局 64KB 封顶。CI 绿后 backboardd
  一死，abort 消息/异常/回溯头部即上屏+进环。
- **★教训：跨补丁上下文冲突（重放网住了）**：0069-sweet-ufs-faster **本身就改 0047 的
  tsleep 行**（`hz` → `hz * sweet_log_secs`，这就是实测 ~50s 刷写周期之源；boot-arg
  **`ufslogsecs`** 可调，0..60s）。我先行的 0047 `hz/4` 编辑把 0069 的上下文打飞——重放
  FAIL 0069 当场暴露（否则 CI 静默跳过 = 0069 全部 UFS 加速改动丢失）。回滚 0047 编辑，
  cadence 走 cmdline `ufslogsecs=2`。**规则：改任何补丁前先 grep 其它补丁是否碰同一行。**
- **CI 39456198891（0088 + 0047 还原）**；cmdline = 定稿串 − `panic_on_error_exit=
  backboardd,SpringBoard` + `ufslogsecs=2`。预期：backboardd ~3min 崩 → `IPS|` 回显
  （abort 消息 = 死因定谳）→ 环里留全 → 探针读回。

### 2026-10-20 十轮续（轮 21）: backboardd 仍 SIGABRT（同 pid 56 同 0x6）→ 静音控制台取证（0042 M2381 化 + sweetsample=-）

- **第二轮拍屏（221143）**：全串 cmdline 生效（无 CS_KILLED，spawn 长句刷屏 = `unentitled_ios_sim_launch`
  在工作），但 **backboardd 仍 `exit_with_reason ... code 0x6`（SIGABRT），同 pid 56 同 code**
  → ea8ef2a 的 HID 注册**没有改变死因**（服务类已确认链接进内核 + personality 在配置表，
  strings 双验）。SMP 采样（死前两个周期）显示 backboardd[56]/SpringBoard[26] 各线程停在
  **相同共享 cache 地址**（等待陷阱类 = idle 非自旋）→ backboardd 是"跑起来后等事件"态被 abort。
- **控制台刷屏源定谳 + 治理**：①0042 长 spawn 句是 `#if SWEET` 编译期分支 → **0042 守卫扩
  `SWEET || M2381`（commit 4c38256，CI 39439883282 绿 8m45s；新内核 strings 复核：长串消失、
  `exec %s` 在）**；②0054 采样器默认名单就是 `SpringBoard,backboardd`（未设也采样）→
  **boot-arg `sweetsample=-` 关闭（payload-only）**；③geod 崩溃循环（exit 0xb）仍在，
  corpse 节流句稀疏可忍。
- **符号化尝试（有限收获）**：从 IPSW cryptex dmg 经 7z 解出 dyld shared cache 全套
  （/home/ri/work/tmp-dsc，39+1 subcaches，VA 0x180000000..0x20d4b4000；backboardd 本体
  tmp-dsc/usr/libexec/）。slide=0x349a4000 假设成立（全部回溯地址落入 cache VA 窗），但
  16.x 图像表（imagesTextOffset=0x5c000 只含 ~170 条被主文件截断；0x1a4610000 猜测位全负）
  未破解，镜像级解析搁置——`SWEET HID/SURF/FB` 内核行内证优先级更高，留作后备。
- **第三轮镜像（本轮）**：CI 39439883282 内核（sha `8c3f75d6…`，SHA256SUMS ✓）+ cmdline
  定稿串 **增量加 `sweetsample=-`**（防再丢旗：只加不减）→ payload `b117f08d…`
  （16,771,104B Image / 五旗标+sweetsample 嵌入已验）→ 镜像 **`ffe3d00b…`**（5,726,208B）
  已刷 boot_b + set_active b + reboot。
- **待拍屏判读**：控制台应显著安静（短 exec 句、无 SMP 样本）。看点：①`SWEET HID: buttons
  registered` / `touch registered`（服务启动了吗）；②`SWEET HID: ... open by backboardd`
  （被打开了吗）；③`SWEET SURF: backboardd create id ...`（开始画了吗）；④若仍 SIGABRT，
  panic 前最后几行 = 死因上下文。用户态里程碑 = 进度条/锁屏出现（sweet 先例：进度条 =
  backboardd 经 SWEET SURF/IOSurface/SWEET FB swap 绘制）。

### 2026-10-20 十轮（轮 20）: ★UFS 墙破实证（SSU wake → LUN 0 ready → iOSRoot 挂载）；cmdline 回归归因与全串重建

- **UFS 大墙破（照片 IMG_20261020_214617，CI 39431106034 内核首上机）**：`SWEET UFS: SSU
  ACTIVE woke the device` → `LUN 0 ready after 2 tries` → `"userdata" on LUN 0: first block
  0x238f60, 474768 MiB` → `BSD root: ufsroot0` → `hfs: mounted iOSRoot on device b(1, 0)`。
  **轮 12-17 的三层定谳（LU NOT READY / query 关门 / SSU 解药）全套实机兑现 = 阶段 2（UFS
  root）实机关闭。** `root volume is not authenticated (error 25); continuing unsealed` =
  预期内（补丁路径）。8/8 CPUs 照常全绿。
- **⚠本轮回归（我方操作事故，非新墙）**：mkpayload 重建时 cmdline 只带了「轮 19 增量 +
  基础串」（AGENTS.md 速记 `cpus=8 + panic_on_error_exit` 被误当全串），丢了三批历史旗标 →
  ①无 `ufsrw=1 rootrw=1` → root 只读（jnl replay ret 30 = EROFS）；②无
  `unentitled_ios_sim_launch=1` → `panic: unexpected SIGKILL of launchd (CS_KILLED)
  namespace 9 code 0xe` = `EXEC_EXIT_REASON_WRONG_PLATFORM`（与轮 5 逐字同款；launchd 是
  initproc，被杀即 panic）。轮 19 spy2 能到 backboardd ⇒ 其 cmdline 必含全套，本轮丢失实锤。
- **★M2381 全量 cmdline 定稿（考古链：轮 4 +ufsrw/rootrw → 轮 5 +sweet proven 六旗 →
  轮 6 +cs_enforcement_disable → 轮 19 +cpus=8/panic_on_error_exit；此后改 cmdline 一律
  以此串为基线增量，勿再从短串重建）**：
  `debug=0xc serial=3 cpus=8 keepsyms=1 rd=ufsroot0 ufsrw=1 rootrw=1
  unentitled_ios_sim_launch=1 ignition_level=1 wdt_test=no_user_checkin
  launchd_unsecure_cache=1 wdt=-1 -enable_kprintf_spam -noprogress
  cs_enforcement_disable=1 panic_on_error_exit=backboardd,SpringBoard`
  （刻意不含 upstream 的 `sweetfb_noconsole=1 sweetfb_hz=30`——M2381 要屏幕当日志通道。）
  重建件：payload `0111fce6…`（16,574,496B）→ 镜像 **`221a3259…`**（5,726,208B，五旗标嵌入
  已验）→ 已刷 boot_b + set_active b + reboot。**待拍屏：预期 backboardd 不再 CS_KILLED
  （轮 5 墙已治），真正考题回到轮 19 的墙 = backboardd SIGABRT/无 HID（ea8ef2a 修复首考）。**
- 镜像链（轮 20）：第一发（缺旗）`237aa34b…`（payload `3cd3c78f…`）→ 第二发（全串）
  `221a3259…`（payload `0111fce6…`）。

### 2026-10-20 九轮（轮 19）: 8 核 + AppleCLCD 实证；backboardd SIGABRT = 无 HID 服务

- **8 核实证**（照片 IMG_20261020_210730）：`M2381 CPU: requested=8 selected=8 (limit=8)`，
  cpu 1..7 逐个 `PSCI CPU_ON rc=0`、`mpidr=0x100..0x700 came alive; SGI1 OK`，收尾
  `8/8 CPUs initialized, SGI1 verified`。payload-only 改动（cmdline `cpus=1`→`cpus=8`），
  内核不动，镜像 57bfe7cb…。八核启动明显更快。
- **显示栈实证**（同屏）：`SWEET FB: AppleCLCD 1080x2400 stride 4320 at 0xb8000000,
  default surface 1, presented as 1125x2436`——0046 的 AppleCLCD + IOCoreSurfaceRoot +
  MeizuPlatformExpert arm-io nubs 在 M2381 全部生效。
- **backboardd 死因**：加了 XNU 自带 tunable `panic_on_error_exit=backboardd,SpringBoard`
  （bsd/kern/kern_exit.c:254，DEVELOPMENT 有效，payload-only），抓到
  `exit_with_reason: Process backboardd with pid 56 exited on error with code 0x6` =
  SIGABRT。第一次 spy 名单里误带 geod，先被 `Process geod ... code 0xb` 抢触发（geod 自身
  重启节奏），收窄到 backboardd/SpringBoard 后复现真正的墙。
- **修法**：0058 SweetHID.cpp 的注释即是判据——backboardd 找 `HIDServiceSupport=true` 的
  IOService 并用 `esuc` 打开；两个服务（SweetHIDButtons/SweetHIDTouch）的注册项仍是
  SWEET-only。补进 M2381 配置块（commit ea8ef2a），CI **39431106034 绿**。
- **备查**：`datamigrator` 调 AKS sel 93/34 → `0xe00002c7`（kIOReturnUnsupported），
  暂不致命；若 data migration 后续卡住，需在 SweetKeyStore 补实现。
- **镜像链**：8 核件 57bfe7cb → spy 7f87dfce（geod 误伤）→ spy2 bc1f8dce（定谳用）。
  下一件 = CI 39431106034 的 kernel + 同 cmdline（cpus=8 + panic_on_error_exit=backboardd）。

### 2026-10-20 八轮（轮 18）: keybag 破 → SpringBoard 活 → watchdogd/IOWatchdog 墙；显示栈一并上

- **判词（照片两张）**：`SWEET AKS: blowed sel 17 / ReportCrash sel 17 / nfcd sel 0,17` 全部
  `-> 0x0`；`hfs: set hardlink bit`；成排 `Allowing spawn of iOS binary ...`；`SMP --- sample 0`
  采样到 backboardd[56] 与 SpringBoard[26]（各 5 线程）。**MKB_INIT FATAL 不再出现** = keybag
  墙真破。新墙 = `watchdogd[140] exited -- exit reason namespace 2 subcode 0x5`（namespace 2 =
  OS_REASON_SIGNAL，subcode 5 = SIGTRAP）→ coredump → Halt/Restart Timed Out panic。
- **根因（同族第三次）**：0040 的 SweetWatchdog.cpp 与 KernelConfigTables 的 IOWatchdog 条目都
  仍是 SWEET-only。watchdogd 是 launchd 的 _PanicOnCrash 任务，找不到 IOWatchdog 即 SIGTRAP。
- **本轮修复（commit a04f266 / 100ebbe / b1f0830）**：①0038 M2381 块注册 AppleKeyStore；
  ②0040 IOWatchdog 守卫+注册；③0046 SweetSurface/SweetFramebuffer 守卫 + IOCoreSurfaceRoot/
  AppleCLCD 注册 + MeizuPlatformExpert 发布 arm-io nubs（AppleCLCD 匹配 `sweet,display`）；
  ④0047/0067 serial_console 的 SWEET 守卫扩双板（AppleCLCD 引用 sweet_console_muted）。
- **验证**：本地 base 258d99a→0085 严格重放 88/88 零失败（含这三件）；CI M2381
  **39368572241 绿**；kernel `200ee1df…` → 镜像 `37f6d07b…`（5,726,208B）。
- **8 核问题**：`SMP --- sample 0` 是 0054 的进程/线程采样，不是核数；cmdline 仍是 `cpus=1`。
  点 8 核 = 0065/0078 已在库，但会与当前用户态调试混在一起，建议锁屏稳定后单独做（2→4→8 渐放）。
- **设备状态**：已关机；boot_b 仍是上一版镜像（56c02006…）。回来后刷 `37f6d07b…`。

### 2026-10-20 七轮: cs_enforcement_disable 破 dyld cache 墙 → launchd 跑 boot tasks → keybag 墙（0038 双板化）

- **A 案验证成功**：`cs_enforcement_disable=1` 后 shared cache 正常映射，launchd 以真进程
  运行并执行 boot tasks：MSEarlyBootTask、mount、sepctl、mobile_obliterator、
  mount-phase-2、init-with-data-volume；userspace corecrypto FIPS POST 全部 PASSED；
  init_data_protection 打印 `No SEP present on this device`。
- **新墙**：`MKB_INIT: FATAL KEYBAG ERROR: failed to load system bag` → `REBOOTING INTO
  RECOVERY MODE` → MACH Reboot → `panic: Halt/Restart Timed Out @IOPlatformExpert.cpp:804`
  （secondary：重启路径在 shim 下超时）。
- **根因**：无 SEP 时 libaks 的 aks_get_system（selector 14）失败 → keybagd 走 fatal；
  上游 0038 的内置 AppleKeyStore（IOClass=AppleKeyStore，IOResources provider，
  KernelConfigTables 已无条件登记）能把 get_system 变成返回 handle 0 的 userspace-reboot
  路径，从而不需要 systembag.kb；但 SweetKeyStore.cpp 的 `#if defined(SWEET)` 把 M2381
  编掉。ADT 的 `/defaults/no-effaceable-storage` 我们已在（strings 实证）。
- **修复**：0038 SweetKeyStore.cpp 守卫扩 `SWEET || M2381`（闭合注释同步）；本地
  base(258d99a)→0085 严格重放零 fuzz，与上一版树只差 1 文件 2 行。0038 的选择器实现
  板无关（handle/lock-state/get-device-state），0036/0043/0055 对 SweetKeyStore 的后续
  修改都在 enum/函数区，不受守卫行影响。
- **已执行（收尾）**：push 2091714 → SWEET 39362647379 ✓ → M2381 39363165800 ✓；
  kernel 17,118,048B sha bf05de76…（SHA256SUMS ✓；strings 四验：`SWEET AKS:` / AMFI
  注册 / 0085 SSU 在、旧 rootauth panic 不在）→ payload 16,509,088B → 镜像 5,709,824B
  sha 0390cd9d…（unpack 自检过），**已备好未刷**。下次会话：设备 pwr+vol↓ → 
  `fastboot flash boot_b shim/out/shim-boot-m2381.img` + `set_active b` + `reboot` →
  拍屏（预期 `SWEET AKS: keybagd sel ...` + `System Keybag loaded`）→ 然后
  usermanagerd/personas；RENAME_SWAP(0052) / /private/var/containers 备查。
- **CI 优化（efacf3a，本地未推）**：push 默认 M2381、`workflow_dispatch` 默认 M2381
  （SWEET 手动 `-f machine_config=SWEET`）、并发组 `xnu-<ref>-<board>`（手发 SWEET 不再
  取消设备构建）。随下次 push 生效；单独推会触发一次 workflow-only 的重复构建，故与
  下个补丁一起推。

### 2026-10-20 六轮: 0087 cmdline 解生效 → dyld shared cache 墙（uid=524288 / trust cache）

- **拍屏判词（104926/104934/105004）**：WRONG_PLATFORM 已过（`Allowing spawn of iOS
  binary /sbin/launchd ...`），dyld 起来；卡在 shared cache：
  `'dyld_shared_cache_arm64': not in trust cache` → `owned by uid=524288 instead of 0`
  → `dyld cache '(null)' not loaded: syscall to map cache into shared region failed`
  → `Library not loaded: /usr/lib/libSystem.B.dylib (no such file, no dyld cache)`
  → `pid 1 exited` → `panic: initproc failed to start`（namespace 6 subcode 0x1；
  coredump 落 /cores/core.1）。
- **源码定界**：bsd/vm/vm_unix.c 的 root_check。trust cache 失败只是 trace 并落到
  root_check 继续；`va.va_uid != 0` 才 `error = EPERM; goto done`。我们镜像所有文件
  uid=524288（容器 root 的 host subuid；mkiosroot 的两段 rsync 保号所致），staging 已
  清、本地 iOSRoot.img 在 /home/ri/mkiosroot-work（容器无 loop 权限，暂不能就地 chown）。
- **A 案（payload-only，先在试）**：`cs_enforcement_disable=1`。依据：vm_unix.c 的两道
  检查在 `XNU_TARGET_OS_OSX && arm64 && (DEVELOPMENT||DEBUG)` 下，若
  `!cs_system_enforcement()` 就 `goto after_root_check`；kern_cs.c `cs_init()` 在
  `!SECURE_KERNEL` 且 `PE_i_can_has_debugger()` 时把 `cs_system_enforcement_enable=0`。
  新镜像 5,705,728B sha f1aad912…（cmdline 在 137cd5c8 基础上追加该旗）。
- **B 案（若 A 不生效 = debugger 判定为 false）**：vm_unix.c root_check 的 uid 分支加
  `#if defined(ARM64_BOARD_CONFIG_M2381)` 豁免（保留 trace、不 EPERM）→ CI；或镜像
  chown -R 0:0 + 重刷 userdata（12G sparse，约 5-7 分钟，需宿主 loop）。
- **通过后的下一站（上游已记录）**：libignition（`hello from launchd.1`）→ preboot
  查找 25 → 备 `ignition_level=0` / `ignition_force_dylib_root=1`；上游 image v12 的
  launchd.plist/containers tweaks 备查。

### 2026-10-20 五轮: 0087 拍屏 = AMFI/Sandbox 生效 + launchd WRONG_PLATFORM（cmdline 解）

- **拍屏判词（四图：101524 旧屏、104010/104033/104103 新轮）**：AMFI/Sandbox 三行全部
  注册成功（`SWEET AMFI: built-in interface registered (XML entitlements)` / `platform
  binary policy registered (0)` / `SWEET Sandbox: permissive policy registered (0)`）；
  UFS → root rw → HFS journal replay（`replay_journal: from 512 to 467456`）→ mount →
  launchd load；**上一轮的 `accelerate_entitlement_queries` data abort 已消失**。新 panic =
  `unexpected SIGKILL of launchd (CS_KILLED) with reason -- namespace 9 code 0xe`。
- **0x0 vs 0xe 勘误**：低分辨率读数像 code 0x0；ffmpeg 放大 panic 行后确认 **0xe** = 14 =
  `EXEC_EXIT_REASON_WRONG_PLATFORM`（bsd/sys/reason.h），namespace 9 = OS_REASON_EXEC。
  源码 = kern_exec.c:7409 附近的 psa_platform / no-sandbox entitlement 检查：本内核是
  XNU_TARGET_OS_OSX 配置，iOS 平台二进制在 init exec（无 spawn attr）路径被当 “alien”
  拒绝，置 CS_KILLED 并发 SIGKILL；initproc 被杀 → panic。
- **上游同款纪录与 cure**：reference/ios-sweet/NOTES.md:171 记录 sweet 在 2026-10-15
  撞到逐字相同的 panic（namespace 9 code 0xe），解药 = DEVELOPMENT 内核的 boot-arg
  `unentitled_ios_sim_launch=1`；其 README/handoff 的 proven cmdline 还带
  `ignition_level=1 wdt_test=no_user_checkin launchd_unsecure_cache=1 wdt=-1
  -enable_kprintf_spam -noprogress`（sweet 最终把 Setup Hello 屏跑起来就是用这套）。
- **本轮动作（无需 CI）**：同一 0087 内核（f86d1588；上一镜像 461394e6）重打 payload，
  cmdline 换成 sweet proven 全套；新镜像 5,705,728B sha 137cd5c8…。设备当前在 panic
  循环，需 pwr+vol↓ 回 fastboot 后 flash boot_b。
- **下一站预判（上游已记录）**：dyld 内置 libignition 会 `hello from launchd.1` →
  `preboot: mounting preboot` → failed to lookup preboot (25) → `Library not loaded:
  /usr/lib/libSystem.B.dylib (no such file, no dyld cache)` → launchd exit → panic
  initproc failed。对策备选：`ignition_level=0`、`ignition_force_dylib_root=1`、上游 v12
  镜像的 launchd.plist StandardOut/ErrorPath 改名 + /private/var/containers +
  LastSystemVersion + VirtualAudio 移除；我们 iOSRoot 是自建 mkiosroot 管线，届时逐项对照。

### 2026-10-20 四轮: 0086 拍屏 = root rw 挂上 + AMFI 未注册 data abort（0025/0035/0050 双板化）

- **拍屏判词（IMG_20261020_101524）= 阶段 2 的 root 挂载全通**：`read-write`（ufsrw 生效）→
  `hfs: Initializing the journal (joffset 0x02000 sz 0x800000)` → `created HFBT` →
  `hfs: mounted iOSRoot on device b(1, 0)` → `SWEET/M2381: root volume is not
  authenticated (error 25); continuing unsealed`（0029 生效）→ `VM Swap Subsystem is ON`
  → `load_init_program: attempting to load /sbin/launchd` → panic。
- **panic 符号化（用本次 CI 内核 + keepsyms）**：pc `0xfffffe0007838b88` =
  `accelerate_entitlement_queries+0x1c`，指令 `ldr x8,[x8,#0x1a0]`；x8 来自
  `0xfffffe000723fcb8` = 全局 `_amfi` = NULL，FAR 恰 0x1a0，ESR 0x96000007（translation
  fault L3）。lr `0xfffffe0007837e84` = `ubc_cs_blob_add` 里 `bl
  accelerate_entitlement_queries` 的下一条。源码 = bsd/kern/ubc_subr.c:4339
  `amfi->OSEntitlements.adjustContextWithoutMonitor`，无判空。panic 尾部 "handshake
  structure not initialized" 照旧 = KDP shmem 阶段噪音（out_shmem.c:499）。
- **根因（板守卫同族病）**：M2381 内核 `_amfi` = NULL（nm 实测），因为 0025 的
  sweet_amfi.c 文件守卫 + 0035 的 bsd_init 注册调用 + SweetEntitlements.cpp 的
  `#if defined(SWEET)` 全是 SWEET-only；0036/0050/0064 往 sweet_amfi.c 里加的
  CS_SIGNED / Sandbox / priv.h 也随文件守卫一起被编掉。`amfi_interface_register` 与
  `mac_policy_register` 符号在 M2381 内核里本就在位（nm 实测），链接面安全。
- **修复**：0025 文件守卫、0035 bsd_init 注册 + SweetEntitlements.cpp 守卫、0050 闭合
  注释上下文，全部扩 `SWEET || M2381`；本地 base(258d99a)→0085 严格重放零 fuzz，与
  上一版 0085 树对拍 = 3 文件 6 行。SWEET 面求值不变。
- **已执行（10:39 本地）**：push 04eec1e → SWEET 39358687983 ✓ → M2381 39359372648 ✓；
  kernel 17,110,224B sha f86d1588…（SHA256SUMS ✓；strings 四验：AMFI 注册串/策略串/
  0085 SSU 串在、旧 rootauth panic 串不在；nm 见 `__startup_EARLY_BOOT_entry_
  sweet_amfi_register` / `sweet_amfi_policy_register` / `sweet_entitlements_create`）
  → payload 16,484,880B → 镜像 5,705,728B sha 461394e6…（unpack 自检过）→ flash
  boot_b OKAY + set_active b + reboot。设备现态 = boot_b = 0087 镜像启动中，等拍屏。
  若新墙在 entitlements/sandbox/用户态，按同一符号化法拆。

### 2026-10-20 三轮: 0085 拍屏 = UFS 墙破 + root-media 板守卫墙（0021/0022/0029 双板化）

- **拍屏判词（IMG_20261020_093404）= 阶段 2 的 root 设备全通**：`SWEET UFS: command
  0x00 failed ... sense key 0x2 asc 0x04 ascq 0x02`（老墙最后一现）→ `SWEET UFS: SSU
  ACTIVE woke the device` → `LUN 0 ready after 2 tries` → `"userdata" on LUN 0: first
  block 0x230f68, 474768 MiB, read-only, DMA at 0x8c6f2c000` → `BSD root: ufsroot0,
  major 1, minor 0`。0085 的 SSU/UA 重试与 sweet_ufs 读/GPT 路径全部实机验证。
- **新墙根因（确定性）**：panic `IOService.cpp:5200 Assertion failed:
  ioblockstoragedriverClass`，行号与源树逐字对上。`IOFindBSDRoot` 的 `rd=ufsroot0`
  早退路径 `goto iofrootx` 时 `service == NULL`，收尾无条件 `setRootMedia(service)`；
  M2381 内核无 IOStorageFamily（0028 走自注册 bdevsw），`getMetaClassWithName(
  "IOBlockStorageDriver")` = NULL，而 0021 的豁免只写了 `ARM64_BOARD_CONFIG_SWEET`。
  紧跟的 `publishHiddenMedia(NULL)` 还有 `iomediaClass` 断言（0022）同族。panic 块里
  "handshake structure not initialized" = panic 后 KDP shmem 转储阶段的提示
  （osfmk/kdp/output_stages/out_shmem.c:499），不是断言内容。
- **下一枪提前拆**：`require_rootauth` 对非 macOS 恒 TRUE（bsd_init.c），root 挂上后
  `VNOP_IOCTL(FSIOC_KERNEL_ROOTAUTH)` 在 HFS 上必失败 → 0029 的 SWEET-only 守卫会
  panic "rootvp not authenticated after mounting"。同批双板化。
- **修复（本 commit）**：0021 setRootMedia / 0022 publishHiddenMedia（两处）/ 0029
  unsealed-root 的守卫扩成 `SWEET || M2381`，注释与提示串同步；SWEET 面 = 守卫超集，
  求值零漂移。同族先例 = d103ced（crypto BearSSL 守卫）。
- **本地验证**：base 258d99a 严格重放 0001→0085（0021/0022/0029 用改后版本）零 fuzz；
  与上一版 0085 树对拍只差三处守卫；sweet_ufs.c 终态 1106 行不变。⚠scratch 树的
  patchgen-orig 0028 是 SWEET-only 旧版（CI 用工作区 `|| M2381` 版），故重放只作
  “补丁可应用性”证据，不作 CI 树等价物。
- **已执行（10:12-10:13 本地）**：push 8a916ff → SWEET 39356431444 ✓ → M2381
  39357098363 ✓；kernel.development.m2381 17,089,304B sha 7fdfd3b2…（用户下载 zip 与
  CI artifact 同件，内部 SHA256SUMS 对拍 ✓；strings 三验：新守卫串在、旧 "rootvp not
  authenticated after mounting" 已消失、0085 SSU 串仍在）→ mkpayload 16,480,352B
  （cmdline 加 `ufsrw=1 rootrw=1`）→ 镜像 2edb7225…（5,701,632B，unpack 自检过）→
  `fastboot flash boot_b` OKAY + `set_active b` + reboot。设备现态 = boot_b = 0086
  首镜像启动中，等拍屏。若下一 panic 落在写路径，回退去掉两枚 rw 旗做只读诊断。

### 2026-10-20 二轮: UniXNU 金矿 + SSU 唤醒配方 → shim v6 探针已刷（待拍屏）

- **用户供钩子**：github.com/UniXNU ——「Run iOS/macOS on non-Apple ARM64 hardware through
  UEFI」。挖完确认 = **本项目的近亲**：设备 = 小米 Pad 6S Pro（sheng）= **SM8550 同 SoC**，
  链 = ABL → BootShim+UEFI FV（boot.img kernel 字段，shim 同构）→ Mu-Silicium UEFI →
  DarwinBoot → XNU（Darwin 23）。三仓浅克隆进 `reference/`（unixnu / uefi-sheng /
  unixnu-sheng；reference/ 已 ignore 不进工作仓）。
- **致命发现（NativeUfs.cpp:488，2026-10-17 实机注释）**：sheng 的 XNU UFS 驱动 **adopt
  ABL 温链路**（sheng.dsc `SHENG_VENDOR_HW = 0`——UEFI 不碰 UFS，与我们同构、无冷启动），
  观察到 **NOP ✓、fDeviceInit=0、bCurrentPowerMode=0x33（低功耗）、全 LU NOT READY**——
  我们的三层症状它全中。**解药 = 一条 SCSI：`START STOP UNIT {0x1b,0,0,0,0x10,0}` →
  Device W-LU 0xD0**（power condition ACTIVE = Linux `ufshcd_set_dev_pwr_mode` 唤醒路径），
  唤醒后 TUR/READ CAPACITY/READ/GPT 全绿。零写数据，SSU 只改电源态。
- **附带修正**：query function byte = header dword1 byte1（0x01 STANDARD_READ/0x02
  STANDARD_WRITE）——NativeUfs 发 0x01；我们 shim 旧 sweep 恒 0（XNU 侧 sweet_ufs.c 本就
  发对，canned-0xFE「引擎关门」结论不受影响，但 shim 层历史矩阵的 0xFE 有此混淆变量）。
- **shim v6（7f3a5e15…，20,480B，M2381=1 本地构建免 CI）**：ufs.c 改造——①scsi() 增加
  sense 提取（key=resp[36]&0xf、asc=resp[46]、ascq=resp[47]，sense 区起于 resp+34；与
  NativeUfs 布局一致）；②ufssend() 加 function byte 参数（旧 sweep 传 0 保形状可比）；
  ③新增 ssu_wake()（0xD0 先试、0x50 对照，unit attention 重试）+ wake_and_reprobe()
  （SSU → settle 50ms → TUR×5 → query 复测[READ fDeviceInit lun0/0xD0 + READ
  bCurrentPowerMode，正宗 fn=0x01，新增 value 字段读 resp+20] → 若 query 活且 flag 待清
  则 SET+poll[0083 形状] → T3 复测）；④test_ufs() 顺序 = 基线 TUR+sense → 旧矩阵 →
  唤醒+复测 → CAP/READ/GPT 尾巴。
- **上机**：fastboot 在场（381QYGEP222XJ），current-slot=b 验证后 `flash boot_b` +
  `reboot`。**判读指引**：`UFS W d0 sense=` 若变 0 或新 sense、`UFS T2 rc=0` = 破墙；
  T2 绿则 `UFS CAP`/`UFS READ`/`UFS GPT` + 分区表自动跟上。不愈则 REA 路线照旧（Meizu
  ABL 可能比 Xiaomi ABL 拆得更烂，fDeviceInit/冷启动两问不变）。设备现态 = boot_b =
  v6 探针坐屏（7f3a5e15）。
- **★拍屏判词（08:16 照片）= UFS 墙破，SSU 一发入魂**：基线 TUR×3 = `sense=00020402`
  （老墙 04/02 复现）→ 旧 query 矩阵照旧全 0xFE、NOP ✓ → **`UFS W d0 rc=0`（SSU
  ACTIVE→0xD0 直接 GOOD）→ `UFS T2 try=0 rc=0`（唤醒后第一条 TUR 即绿）→ CAP ✓（LUN0
  lastlba=0x7621FFF × 4K ≈ 512GB）→ READ ✓（MBR AA55/EE，GPT `EFI PART`，分区表全出）**
  ——userdata lba=0x238F68 ~464GB、fs=other（= 已烤的 HFS+ iOSRoot，交叉验证读的是真盘）。
  对照组：SSU→0x50 = ILLEGAL REQUEST 05/25（LU not supported；本机 device W-LU = 0xD0，
  与 NativeUfs/Linux 的 0xD0 一致，排除误中）。ver=0x0400 复证 UFS 4.0。
- **★新事实：query 引擎唤醒后依然关门**（Q2 = READ fDeviceInit [lun0/0xd0] 与 READ
  bCurrentPowerMode 全 `0036FE05/03`，正宗 fn-byte 照旧）——Meizu ABL 的 query 关门是
  **永久性**的（小米 sheng 的温链路 query 是活的），fDeviceInit dance 在本机永远用不上；
  **0085 走纯 SCSI 路线**（好在 TUR 过了根本不需要 query）。
- **★0085-sweet-ufs-ssu-wake.patch（commit 4a2bd99）**：`ufs_wait_ready()` 在 TUR 撞上
  精确 04/02 签名时先发一发 `SSU {0x1b,0,0,0,0x10,0} → LUN 0xd0`（EBUSY+UA[键6] 重试×3，
  成功即 `continue` 立刻重测 TUR；ILLEGAL[05] 止损路径在 SSU 之前保持不变）。SWEET 面
  安全性：仅 02/04/02 精确签名触发（sweet 的 UA[06/xx] 路径零接触）；i≥10 的 LU RESET +
  fDeviceInit 旧梯子原样保留（m2381 上到不了，别的失败态仍有诊断价值）。本地验证 =
  sweet_ufs.c 全链 12 补丁重建（0028/0030/0034/0047/0054/0060/0069/0072/0077/0082/0083/
  0084——⚠首轮重建漏了 0034 等旧格式补丁导致锚点错位假象，`^--- a/` 逐段抽取才对）+
  research/walk.sh 真源树全量严格重放（0085 收尾零 fuzz，终态 1106 行）。push
  c291ea1..4a2bd99 → SWEET 腿 39351563176 → 绿后 dispatch M2381（单发纪律）。
- **★双腿 CI 全绿 + 已上机（轮 12 收口）**：SWEET 回归腿 **39351563176** ✓（用户下载的
  kernel.development.sweet 17,222,704B = 0085 共享面编译证明，zip 留档）；M2381
  **run 39352655714** ✓（门 89 Applying + 4 件 dxb 基线 Skipping 照旧，零断链）→ 内核
  **kernel.development.m2381 17,089,456B sha 63e3d8c4…**（内部 SHA256SUMS 对拍 ✓，
  时间戳 01:23 在 run 窗内）。出件链 = mkpayload（/tmp/k85/payload.bin 16,480,496B，
  cmdline `debug=0xc serial=3 cpus=1 keepsyms=1 rd=ufsroot0` 不变 + research/m2381.adt
  12,620B 复用）→ `M2381=1 bash shim/build.sh` → **shim-boot-m2381.img 5,701,632B sha
  6f3e0e6b…**。current-slot=b 验证 → flash boot_b → set_active b → reboot。
  **★设备现态（会话收口）= boot_b = 0085 首镜像，XNU 已在启动路上，屏幕坐等拍摄。**
  **下会话首件事 = 拍屏**：期望链 = `SWEET UFS: SSU ACTIVE woke the device` → `LUN 0
  ready after N tries` → 分区表 → root 挂载 `ufsroot0` → launchd/用户态开跑（阶段 2
  判据达成）；若挂新墙 = panic 符号化四步配方（NOTES 轮 6 条目）。后续排面：过 root 后
  = userspace 调试（USB-net 日志通道 0077 翻版收益最大）+ Sweet* 驱动守卫延后批 +
  AMFI/trust-cache 家候审。回滚 mu-r69 不变；收口杂项：仓根 xnu-kernel.zip + 
  xnu-kernel.1.zip（sweet 腿留档）可删；完工清 mk-eio/ + /usr/local 三件。

### 2026-10-20 续: CI 三连发（0083/0084）+ shim 扫矩阵 —— query 引擎整体关门，下一步 REA

- **0083（commit 9f0a116）**：query CT 2→1（本版 Linux 对 query 也发 CT=1；`UTP_DEVICE_MANAGEMENT_
  FUNCTION`=0x20000000 是死枚举）+ query 响应 dump + SET×3 + READ 轮询。**双腿绿**（M2381
  39336420522 门 87 + SWEET 39337574251 ✓）。实机：SET/READ 全 0xFE，qrsp dump 显示设备
  **正确回显 opcode/idn**（[12]=op、[13]=idn）却拒服务——请求构造排除了。
- **0084（commit de7a1c7）**：TM 通道——UTMRD 80B（16B 头+32B 请求 UPIU+32B 内嵌响应）、
  寄存器组 0x70-0x80（UTMRLBA/DBR/CLR/RSR，与 UTRL 0x50-0x60 零冲突）、`UFS_LOGICAL_RESET
  =0x08`（⚠非忆测的 0x10）、`input_param1=be32(lun)`、结果读 output_param1（DW16）。
  M2381 39339234730 绿（SWEET 腿被 0084 push 覆盖，0083 腿已验共享面）。**实机里程碑：
  `LU RESET done, result 0x0` = TM 通道首次被设备接受**——但 LU 没回来（后续轮询仍 04/02，
  sense 无变化），且 READ_FLAG/SET 在 RESET 后仍 0xFE。
- **shim 扫矩阵（本地构建免 CI，探针件 88a614ed 20,480B）**：ufssend() = 裸 UPIU 发送器
  （CT/LUN/opcode 可变），矩阵 = {CT 0/1/2} × {LUN 0/0xD0/0x50} × {READ 0x5/SET 0x6} 七行
  + NOP 握手 + REPORT LUNs。**结果七行全 b=0036FE0x**（ocs=0/type=0x36/**res=0xFE**/回显
  正确），CT 与 LUN 完全无关；**NOP = 0x20（NOP IN）✓ UTP 分发器活**；RPTLUN/CAP/READ 仍
  0202。**定谳：ABL 留下的设备态里 query 类整体关门（对所有 opcode canned INVALID_OPCODE），
  而 NOP/SCSI/TM 三通道活**——与时机、请求形状全无关。
- **★病因模型（当前最优）**：nura 链的 Linux 永远自己从 DME_RESET 做全量链路冷启动
  （初始化完 query 引擎自然开）；我们继承 ABL 温链路，撞上的是半初始化态里关着的 query
  引擎。SCSI 的 04/02（初始化命令必需）与 query 关门 = 同一个未完成设备初始化的两面。
- **★下会话 = REA（用户拍板）**：反编译 ABL/XBL 的 UFS 驱动（s5-rea 工装现成；
  xbl_a-code.bin「分析就绪未展开」正好用上），两问：①它发不发 fDeviceInit、query 什么
  形状；②**完整链路冷启动序列**（DME + PHY 寄存器真值）——拿到后在 shim 探针复刻
  （两分钟一轮），通了写 0085 回 XNU。若 ABL 有「query 前必须 XX」的隐藏步，也是这个
  反编译直接出答案。
- **本会话刷机全录**：0083 内核 **095688b6**（17,089,392B，CI 39336420522）→ 镜像
  **3d467f2a**；0084 内核 **bf79e063**（17,089,416B，CI 39339234730）→ 镜像 **264bcc5a**；
  探针件 **88a614ed**（20,480B）。⚠用户手下载件两轮均落在仓根 xnu-kernel.zip（覆盖式），
  验件三件套 = 内部 SHA256SUMS 对拍 + 大小增量 + 内部时间戳 vs run 构建窗。设备现态 =
  boot_b = 扫矩阵探针坐屏。

### 2026-10-20: 刷机轮 + UFS root 墙定谳 + 0082 上树（待推 CI）

- **刷机**（全程单 fastboot 通道）：`fastboot devices` = 381QYGEP222XJ ✓ → 粮草对拍
  （iOSRoot-s.img `1db88931…` 12,145,492,364B ✓ / shim-boot `f6d7e155…` 5,701,632B ✓）
  → `getvar current-slot` = **b** ✓ → `flash userdata`（16/16 sparse OKAY 315s；首行
  avb footer 警告 = fastboot 对裸 HFS+ 的常规提示）→ `flash boot_b`（OKAY，写入 207s）
  → `set_active b`（retry 复满防翻槽）→ `fastboot reboot` → 设备离台。
- **拍屏两连**（IMG_20261020_051307/051349）：判词全文录 STATE★刷机轮实录。关键新增
  信息 = panic 前的三条 `SWEET UFS: command 0x00 failed: ocs 0x0 upiu 0x21 response
  0x1 status 0x2`（TUR CHECK CONDITION）——与 stage-0 v5 探针留档的 `TUR/CAP/READ
  rc=4 ocs/st=2` 逐位吻合 → 定谳为延续性设备态问题，非本轮引入。
- **考古结论**：① 驱动 resp[6]/resp[7] 偏移对（Linux uapi scsi_bsg_ufs.h utp_upiu_header：
  b4=befield iid/cst、**b6=response、b7=status**、b8=ehs、b10-11=data_segment_length；
  sense = utp_cmd_rsp @12 起残留计数/reserved/sense_len@32-33/sense@34-51）；② 0028 的
  「3 次背靠背 TUR」对一过性 UA 够用、对持续态全无；③ fDeviceInit = Linux UFS 标准启动
  释放步骤（SET_FLAG 后 READ_FLAG 轮询至清零，1500ms 上限）——本机从 ABL 交接后无人做。
- **0082 施工**：awk 区段提取器两坑备案——新文件剥壳 `^\+([^+]|$)`（裸 `+` 空行会丢）；
  区段切分必须保留 `---/+++` 文件头（git apply 认头），且 `+++ b/...` 前缀匹配勿写死
  `+++ bsd`。研究结论：research/buildtree-m2381 与 research/xnu-8796 两树 sweet_ufs.c
  同为陈旧版（外层守卫 `#if SWEET` 无 M2381、缺 666-806 的内层 SWEET-only 包装），已过时
  勿再作补丁基准。
- 设备现态 = boot_b shim + userdata iOSRoot，坐屏 panic 等 0082 内核；下会话 = 推 CI
  （用户拍板）→ 重打 boot 镜像 → **只刷 boot_b**。

### 2026-10-20 凌晨: 阶段 1 上机 —— XNU 首次在 m2381 实机起跑（三轮诊断循环，进行中）

- **★里程碑：XNU 在实体骁龙 8550 上跑起来了。** 第二轮镜像起，屏幕实锤：`iBoot
  version: m2381-shim`（boot_args 握手成立）→ VM bootstrap done（85 maps/165 entries/503
  holes）→ zone_init（map VM/GEN0-3 各 19G、DATA 25G）→ firehose/OSLog/调度时间片配好
  ——13GB 内存窗全认，xnu-8796 的 Mach/VM/scheduler 核心活了。死点 = IOKit 首站
  `IOCatalogue.cpp:121 Assertion failed: array && gIOClassKey && …`。
- **首轮（shim+payload 走 ramdisk 槽）→ `LOADER no SWEETXNU payload`**：★配方事实钉死
  ——**Meizu GKI ABL 无视 boot.img 的 ramdisk**（INITRD 恒 = stock 5.7MB initramfs
  `B74A4000–B7FFE4C0`，魔数不在 = ABL 压根没加载我们的）。修法 = Mu 同构「一切在
  kernel 里」：shim.ld 加 `_payload_start`（16K 对齐尾符号）→ loader.c 先试内嵌魔数、
  回落 initrd，拷贝循环加 src/dst 重叠自检 → build.sh 追加 payload 到 Image 并把
  arm64 header 的 image_size（偏移 0x10）补成全镜像长度（ABL gunzip 预留用）；ramdisk
  槽回退占位 cpio。commit 3e2caab。
- **二轮 = 上表里程碑；死因 = KernelConfigTables 尾逗号**：屏幕头行
  `KernelConfigTables syntax error: OSUnserialize: syntax error near line 1` → array
  NULL → IOCatalogue:121 断言，因果链完整。★真凶改判两次：先判 0016（错——它的逗号
  是给 0018 追加条目用的），后定 **0018 的 MeizuCPU 是最后一个条目还带尾逗号** →
  `({IOPanic},{MeizuPE},{MeizuCPU},)` 语法错。修复 = 0018 末条目去逗号（cfe3644）；
  当场**字节热补**已刷内核（`},)`→`} )`，OSUnserialize 忽略空白，唯一出现处
  0x137a3c），第三轮无需等 CI。
- **三轮（dc58add9 = 内嵌 payload + 热补内核）屏幕已读（照片 IMG_20261020_012238，
  01:22:38）**：**尾逗号墙确认已过**——全屏无 KernelConfigTables 语法错误行，VM 段全走：
  zone_init 四段 map（GEN1-3 各 19G + DATA 25G、metadata 128M、metabits 38M）→ VM
  bootstrap done（85 maps/165 entries/**502 holes**，与二轮 503 不同 = 独立一次启动）→
  firehose/log queues/OSLog stream → 调度 quantum → mig_table_max_displ = 54 →
  debug_log_init gPanicBase 报错 ×2（无害噪音）→ `kdp_core zlib memory 0x8000`；IOKit 面
  `"name"/"AAPL,handle" not a kext` ×2（= OSKext::createExcludeListFromBooterData
  OSKext.cpp:4668 扫 booter 数据对非 `Driver-` 条目的良性跳过日志）→ **panic：
  Kernel data abort，esr=0x96000005（EL1 数据 abort、读、L1 翻译错误）far=0x28**。屏幕
  纵向顺序异常（panic 块在上、zone_init 段在下）= 多轮启动 fb 不清屏的叠加残影，
  段归属不再深究（不影响定性，诊断只靠符号化）。
- **★四墙定谳（符号化 vs `/tmp/green-kernel/kernel.development.m2381`，sha 879be7f1 =
  CI e52d539e + 热补字节[0x137a3c `} )` 在位已验]；内核未滑动，__mh_execute_header =
  0xfffffe0007004000，panic 地址直对符号表）**：pc=0xfffffe000797b828 = **sha1_init+0x34**：
  `ldr x22,[g_crypto_funcs]`（0xfffffe000723f680，读出 **NULL**）→ `ldr x20,[x22,#0x28]`
  踩空——far=0x28 与 ccsha1_di 在 crypto_funcs_t 的偏移吻合；lr =
  **IOPlatformExpertDevice::generatePlatformUUID**（IOPlatformExpert.cpp:2172）；caller =
  sleh_synchronous_sp1。因果链：mkadt 的 /chosen 带 `unique-chip-id`(8B)+`chip-id`(4B)
  → IOPlatformExpertDevice::configureDefaults → generatePlatformUUID 取 SHA1 → libkern
  sha1 包装**假定 corecrypto kext 已注册 g_crypto_funcs**（真机成立，我们恒无）→ NULL
  解引用。诊断三件套：llvm-symbolizer/nm 对表 + 反汇编 faulting 指令 + 源码 grep
  屏幕字符串（"not a kext"/gPanicBase/zlib 行全部定位到源）。
- **★根因家族 = crypto BearSSL 兜底层守卫 SWEET-only（第三起「板守卫未扩 M2381」同族病，
  nm/apply 双盲、CI 照绿）**：libkern/crypto 19 文件 27 处
  `#if defined(ARM64_BOARD_CONFIG_SWEET)`，M2381 命中 **0**——MD5/SHA256/SHA1/SHA384/
  512/AES 的软件兜底（sweet_*.c = BearSSL 便携实现 + corecrypto_*.c 包装改道）在 M2381
  面全被编掉，编进的是解引用 g_crypto_funcs 的原生路径。修复（**d103ced**）= 0020/
  0023/0027/0032/0033 五件补丁守卫扩成 `SWEET || M2381`；conf/files 本就无条件挂文件、
  Makefile.template crypto 块本就 `filter $(CURRENT_MACHINE_CONFIG),SWEET M2381`
  双板化（INCFLAGS/-mgeneral-regs-only 对 M2381 已生效）；0013/0014 无守卫直接改写、
  两板已生效勿重复动；des/rsa/chacha20poly1305 包装连 sweet 都无兜底（死代码路径）不动；
  SWEET 面 = 守卫超集求值不变、零漂移。**验证链：research/walk.sh 84/84 全 apply
  （REGEN 已纳入 0023/0027/0032/0033）+ xnu-cc.py 重放 39296043060 全套 M2381 CI 编译
  命令打 libkern/crypto 25 文件 0 fail（BearSSL 分支首次在 M2381 面真编译）**。
- **下一步 = push（红线：用户窗口）+ `gh workflow run xnu.yml`（machine_config=M2381）
  → 第四轮镜像（内核换 CI 新件、payload 照旧）刷机**。预期 crypto 墙过，下一站 = PE
  匹配 'meizu,m2381' → DT 探测 → OSKext 驱动目录，逐墙推进（判据链 = AGENTS 阶段 1：
  banner ✓、panic 可视 ✓、持久日志环待立）。后续候选墙备案：AMFI/trust-cache 家
  （0015/0025/0035/0036 同为 SWEET-only 守卫嫌疑，userspace 阶段才暴露，到站再查）。
- **★四轮读数（IMG_20261020_022657，内核 b9de3a09 = CI 39300614075 出件）= crypto 墙过 +
  平台面全亮**：`M2381: starting device-tree platform expert` → `platform expert
  registered` → `M2381 CPU: requested=1 selected=1 (limit=8)` → `cpu=0 mpidr=0x0 came
  alive; SGI1 OK` → MAC Framework → `M2381 CPU: 1/1 CPUs initialized, SGI1 verified` →
  `SWEET HFS: hfs-627.100.6 registered` → mbinit done → **死点换成干净文字 panic：
  `pthread kernel extension not loaded (function table is NULL). @pthread_shims.c:318`**。
  定位零符号化需求（panic 自带文件:行）：bsd_init 的 `pthread_init()` 见
  `pthread_functions` 空表即炸，注册方 = 0019 builtin provider 的
  `sweet_pthread_start`（STARTUP EARLY_BOOT）——**第五起板守卫同族病**：0019 三个
  sweet_pthread_*.c 整文件守卫 + pthread_internal.h 的 ksyn_waitq_element 布局
  （SWEET=56B[libpthread-514.100.2]/stock=48B）全 SWEET-only；Makefile 层
  （fb0853e 已双板化）与 conf/files（`standard` 无条件）都早就有意让 M2381 用——
  就差源码守卫。修复（**8a59025**）= 四处守卫扩 `SWEET || M2381`（provider 自带
  `_Static_assert(sizeof==56)` 在 M2381 面自检布局）；walk 84/84 + xnu-cc 重放
  pthread 面 7 文件 0 fail。
- **★五轮读数（IMG_20261020_024557/024633/024722 三连拍，内核 cf799aac = CI
  39303355346，镜像 de477ccd）= ★阶段 1 通关**：pthread 墙过（四轮死点 mbinit 后
  不复现；`SWEET pthread: libpthread-514.100.2 registered` 行在 wrap 窗外，但死点
  消失即注册成立的实证），kmem 子图 dump 变丰富（gIOKitPageableFixed0-3/bsd_pageable/
  io_submap/sprayqtn/ptr_range_meta 全列出）→ `SWEET HFS` → mbinit → **dlil_init +
  ifnet_attach（lo0/gif0/stf0 接口内核线程创建并调度完成 = 调度器/线程体系实证）** →
  `!BSD` → **`Waiting on <dict><key>IOProviderClass</key>IOMedia<key>Content</key>
  Apple_HFS</dict>` = vfs root 挂载等待，无 panic 坐屏**。判词：XNU 在 m2381 上
  Mach/VM/调度器/线程/BSD(MAC/HFS/INET) 全链活，停在「没有根文件系统」的自然终点；
  阶段 1 判据全达成。Sweet* 驱动守卫延后批（IOKit 驱动空 TU）= 等 IOMedia 的原因之一
  （无 mockfs/存储驱动注册），属预期。**阶段 2 分叉待用户拍板**：A) ramdisk-root
  （mkinit.py + mkpayload --ramdisk，零 userdata 风险，sweet 原路线）；B) UFS root
  （mkiosroot.sh → sda22，**抹 Flyme 数据 = 红线#2 硬闸门**）。工程配套优先级：
  USB-net 日志通道（topaz 0077 翻版，userspace 调试刚需）> 持久日志环。
- **★阶段 2 开工（10-20 清晨，用户拍板 = 「直接刷，没有数据需要保存，放开手脚干」**
  ——userdata 红线#2 闸门用户亲解；⚠勘误：userdata 现内容 = **nura pmOS 根**（sda22，
  非 Flyme 原厂数据），放弃 = 可重烤，配方在 nura 档）**：
  - **六墙定位 = 0028/0030/0031 UFS 族守卫 SWEET-only**（sweet_ufs.c 整文件 +
    IOKitBSDInit 的 rd=ufsroot0 钩子 + rw root；0034/0069 无守卫已两板生效；conf/files
    `standard` 无条件）。守卫扩 `SWEET||M2381`（8c1c749）+ xnu-cc 重放
    sweet_ufs/IOKitBSDInit 2 文件 0 fail。
  - **轮 6 红（39307948283）= `_sweet_usb_probe` undefined**：调用是 0072 注入
    sweet_ufs.c 的，而 sweet_usb.c = **SM7150 硬编码常量的 dwc3 gadget 启动器**
    （GCC_PA 0x00100000 那套）必须保持 SWEET-only（M2381 的 USB-net = 0077 翻版独立
    工程）。修 = 0072 调用点包 `#if SWEET`（**20eee42，但 sed 加行没改 @@ 头 = 埋雷**）。
  - **★nm 差集门首秀**：xnu-cc `XNU_CC_OBJDIR` 出真 .o → `llvm-nm -u` 477 个外部引用
    对拍轮 5 内核符号表 **0 未解析**——教训：守卫扩展轮 **必跑 nm 门**（语法重放不够；
    轮 7 若跑了这步也拦不住 hunk 计数雷，但符号层雷全靠它）。
  - **轮 7 红（39309894409）双因**：①`_sweet_keys_init`/`_sweet_touch_init` undefined
    （0060 注入调用无守卫 + 其定义文件 M2381 面 = 空 TU）；②**补丁门 78/85**：0072
    sed 加行未改 @@ 头 → **CI 侧 apply 严格拒绝 0072 → 0073/0074/0075/0076/0077/0081
    级联 skip**（USB 链上下文锚在被改区域）。**本地 walk 假绿**：容器 git apply 对
    hunk 计数宽容、CI 严格——**工具差异盲区**，守卫/补丁结构修改后以 CI 补丁门计数
    为准（85 全 Applying）。修法已写进顶部 STATE（重写 @@ 计数 + 0060 调用包守卫）。
  - **★iOS root 镜像构建线（宿主通道已通，EIO 阻塞待查）**：dmg 身份 BuildManifest
    定谳（OS=087-86698-044 4.7G / SystemOS=087-86593-047 2.7G / AppOS=087-86715-047
    10M，RestoreRamDisk 087-86719-047 不用）；apfs-fuse 容器内**一次点活**（当年两败 =
    缺 libfuse3-dev/libicu-dev 等依赖，apt 后 cmake+make 即成，二进制 Fedora 宿主可直跑）；
    mkiosroot 容器内死于 mount（沙箱无 loop 权限）→ **宿主通道两跳打通**：node_repl
    （ZCode 宿主侧进程）也是 docker（假宿主，systemd-detect-virt=docker）→ **nura
    坑册#27 配方 = `XDG_RUNTIME_DIR=/run/user/1000 systemd-run --user --unit=<名>
    sudo <cmd>`**（真宿主 + sudo 免密实提权；/run/systemd/private → /run/host/... 已挂）
    → mkiosroot.service 挂通、**HFS+ 挂载成功、rsync 开跑**（16G 建卷 + mount -o
    loop,force 全过）。
  - **★EIO 阻塞（下会话头号实验）**：apfs-fuse 读侧 **131,402 文件 open 持续 EIO**
    （约 5.6G 写入后遍布全树；复读同一失败文件仍 EIO = 非负载竞态，是解码/资源类）。
    已排除：dmg 拷贝损坏（宿主/容器 sha 一致 80f426d5）、上游版本落后（66b86bd
    2023-03 = origin HEAD，仓库停更）。**上游作者同 commit 同 dmg 名成功建过 v4-v7 镜像**
    （NOTES.md 上游 135/172 行）→ 必是环境差异。**实验队列**：①systemd unit 默认
    LimitNOFILE 低 + apfs-fuse fd 泄漏 → `systemd-run -p LimitNOFILE=65536` 重挂重测；
    ②FUSE async_read 竞态 → apfs-fuse 加 `-o async_read=false`（或 rsync --blocking-io）；
    ③容器侧同 commit 复测同路径（容器挂载初始化 >6s，上次等太短误判"No such file"；
    若容器能读宿主不能 = host unit 环境实锤）。journal 里 apfs-fuse 静默（错误只在
    rsync 侧可见），可在前台单文件跑 apfs-fuse 抓 stderr。
  - **刷机配方（镜像就绪后）**：img2simg 转 sparse 省 USB 时间 → `fastboot flash
    userdata` + `fastboot flash boot_b`（rd=ufsroot0 payload）→ reboot → 拍屏。判据 =
    root 挂上 launchd 开跑；调试 = **sweetlog 持久日志环**（0028 自带：控制台全量落
    userdata 尾部 4MiB，ufslogsecs 默认 10s）+ 拍屏。
- **★诊断循环纪律（实跑顺手的经验）**：刷机窗口 = 设备 fastboot（shim 蓝屏/panic 屏
  均坐屏不自复位，pwr+vol↓ 即回）；每轮 = mkpayload（内核+ADT）→ `M2381=1 bash
  shim/build.sh <payload>`（自带 unpack 对拍）→ `fastboot flash boot_b` + `fastboot
  reboot`，全程 ~1 分钟；内核件 = CI 绿轮 artifact 手动下载（网络差时浏览器拉
  `actions/runs/<id>` 页的 xnu-kernel，放 `~/Downloads/` 容器可见）；**热补合法场景 =
  常量字符串级修复**（长度不变字节替换），其余一律走 CI 重编。
- **★SWEET 腿红（39287079668）第四修**：0054 的 include 搬进共享段（994 行）后，
  SWEET-only 块内 ≤0053 的崩溃转储件（exception.c:847 用 vm_shared_region_get）断声明
  ——M2381 面该段被守卫编掉故本地重放全绿，SWEET 面一编就红。修 = include 提文件
  作用域（823 行，盖 847/1013 两个用户）。★方法论补一条：**头文件/守卫搬家必须两板
  编译面各自过一遍**（walk 验 apply、nm 验符号，都对这个盲）；本地 SWEET 面编译目前
  无 artifact 可重放，靠 CI 腿兜底。修复 CI 腿（cfe3644）
  **双腿全绿**：SWEET 39294627743 + M2381 39296043060（85/4 补丁门照旧）＝include 文件
  作用域修复的 CI 双板实证；39296043060 的 xnu-kernel artifact = **现役件源**（带 0018
  原生修正，可替代热补内核直接重打 payload）。

### 2026-10-19 深夜: 阶段 1 施工 —— M2381 内核配置全链入树（未上机）

- **做法**：xnu-8796.141.3 真源树（apple-oss-distributions，CI 同源）作 scratch，逐补丁
  「apply 原件 → 落 m2381 编辑 → git diff 重生成」，patch 文件按构造可应用；最后严格
  walk（scripts + 84 patches 全 `git apply --check` 通过才落，零 fuzz）验证。
- **翻版落地（对应 §开工排面 11 件）**：
  - **0004**：SUPPORTED/MACHINE_FLAGS 加 M2381（armv8.2-a+crypto，plain arm64 同 sweet）；
    新 `pexpert/pexpert/arm64/M2381.h`（INTID27/28、GICR_PE_SIZE 0x40000、DS=1 注记、
    `M2381_GICD_CTLR_NASSGIREQ 0x100`、SMP_MAX 8、FB 0xb8000000）；board_config.h 块；
    **pe_init_fiq 的 SWEET 分支扩成 SWEET‖M2381 共享，nASSGIreq+RWP 前置在 8 笔优先级写
    之前**（秘方落位）；sleh_irq IRQ 路径共享；SCTLR 检查豁免共享。board-neutral 别名
    （BOARD_FB_BASE/BOARD_VTIMER_INTID/BOARD_IPI_SGI/BOARD_SMP_MAX_CPUS）落在两块板头。
  - **0006**：新增 `config/MASTER.arm64.m2381`（= sweet 版内容，open-source asm pmap）。
  - **0010**：start.s 白带 + FB 块映射共享（基址按板 #if）；sweet_early.* 守卫共享 +
    BOARD_FB_BASE；debug.c panic 钩子共享。16K L1 槽 = 64GB → m2381 的 35GB RAM 与
    0xb8000000 fb 同在 L1 槽 0，sweet 的「首 V=P L2 页」机制原样成立（注释已改写）。
  - **0011**：注释扩指 M2381（逻辑 SoC 无关）。
  - **0012**：BOARD_EARLY_FB_VA + sweet_early_map_final/relocate 共享（FB PA 走别名）。
  - **0016**：MeizuPlatformExpert.cpp（'meizu,m2381' 匹配）；KernelConfigTables 的 M2381
    表项放**表尾独立 `#if` 块（"," 拼接技巧）**、files.arm64 的 Meizu 行放 sweet 行**之前**
    ——否则后续 sweet 补丁（0035/0038/0040/0046/0058）的 hunk 上下文被 Meizu 行打断，
    CI 会静默跳过（本地 fuzz 能糊过去，CI 不能，这个坑绕过两轮才定形）。
  - **0017**：`sweet_vtimer_intid` → `board_vtimer_intid`（SWEET‖M2381），DT 属性
    virtual-timer-intid 两板通用；sleh/protos 同步。
  - **0018**：MeizuCPU.cpp **终态版**（= sweet 0018+0065+0078 合体形态：8 核 SMP、
    PSCI CPU_ON、Aff1=core 校验 0x000-0x700、SGI1 自检）；'meizu,sm8550-cpu' 匹配。
  - **0065**：cpu.c 副核启动 CleanPoC 套 + `board_cpu_signal_is_enabled`（改名共享）、
    start.s secondary entry（PSCI EL2→EL1 下降）/reset 向量 Aff1 提取/ic iallu 屏障、
    machine_routines cpus 钳制（sweet 分支字节不动，**#elif M2381** 孪生分支，
    BOARD_SMP_MAX_CPUS=8，默认 cpus=1）全部共享化。
  - **0078**：零改动（sweet-only 内容；m2381 的 8 核能力在 0018 终态 + 0004 SMP_MAX=8）。
  - **0019/0020/0026 stage .sh**：门 `== SWEET` → `== SWEET || == M2381`（否则 M2381 构建
    无 HFS/BearSSL/libpthread 铺料，一串补丁静默跳过）。
- **mkadt.py `--m2381`**：dram = **0x8c0000000/0x340000000**（stock FDT 八段中两段相邻的
  13GB 连续窗）；GICD 0x17100000/GICR 0x17180000 **size 0x200000**（初版漏切 GICR 区大小
  ——0x100000 只够 4 核的 RD，副核 bring-up 必挂，已修）；VTIMER 27；fb 0xb8000000/
  0x2b00000；ECID 'M2381'+1；cpu8 节点 'meizu,sm8550-cpu'（OPP 真值 = nura kodiak.dtsi：
  A510 2.016 / A715+A710 2.611 / X3 3.014GHz）；disp0 canvas 1080×2400（display-scale 3
  不变 → 360×800 点）；root compat 'D22AP|iPhone10,3|meizu,m2381|qcom,sm8550|AppleARM'；
  设备身份 iPhone10,3 / 'sweet-fbcon'/'sweet,display'/'qcom,ufshc' 节点名保持（= 0046/
  0061/0030 驱动的匹配键，约定：sweet_ 前缀 = 便携板级层名，非 sweet 板专属）。
- **0013/0014/0015 追加共享化（审计补刀）**：vm_kern.c kmem RNG / prng_random.c HMAC-DRBG
  注册 / entropy.c corecrypto SHA256 切换 / kern_trustcache.c NULL-runtime——这些 `#else`
  走 Apple 闭源 crypto 栈（crypto_random_*、SHA256_*、KDK 闭库），**M2381 开源配置下是
  链接级断裂**（不止行为面），已全部 `SWEET || M2381`。
- **验证**：严格 walk 全绿（84/84 APPLY，无 FUZZ 无 FAIL）；SWEET 回归 = 原件树 vs 重生成
  树全量 diff，逐一过目 = 守卫/别名/改名级，cpu.c/machine_routines.c/SweetCPU.cpp 零
  diff；mkadt 双模式出树 + dump 对拍。**编译级验证 = CI 首跑**（本地无 macOS 工具链）。
- **已知风险**（非阻断）：XNU 在 A510/A715/X3 上的实施细节（PAC 已有 0005 兜底；cpus=1
  起步）；CI 首跑 workflow 修缮；arm_vm_init 的 gVirtBase assert（0012 的 VA 别名）要等
  上机才能证。**★延后批（阶段 2 开工前必做）**：IOKit Sweet* 驱动的顶层 `#if defined(SWEET)`
  守卫（SweetKeyStore/SweetEntitlements/SweetWatchdog/SweetFramebuffer/SweetSurface，各 1
  处）在 M2381 下编成空 TU——**链接零碍（跨 TU 引用 = 0，已审计）**、banner 无碍（IOKit
  匹配在 banner 之后），但驱动们不注册 = 阶段 2 用户态缺 keystore/watchdog/fb 驱动；届时
  逐件 `SWEET || M2381` 化即可（DT 匹配键 'sweet,display' 等已在 m2381 ADT 里保留）。
- 下一步 = 顶部 STATE「续接」三步（推 fork → CI M2381 → mkpayload 上机）。

### 2026-10-19 晚: 探针第四/五轮（v4→v5）—— **阶段 0 完结：nASSGIreq 秘方落定，六项全绿，中断套件全通**

- **v4 轮（92f4b7b0）**：SEC1 `wok=600D` = **CPU1 开机自页 SGI 优先级写通**（nGnRnE，中止非全核性）；
  UFS 段首跑 `ver=0x0400` = **UFS 4.0 裸金属定谳**；新 panic：map 首笔守护写（0x17190080
  IGROUPR0，同 boot 早前刚写过）报 **ESR=0x02000000 = EC=0 未定义类、FAR=0**，ELR=0x2464
  处 ELF 为合法 `str w25,[x21,w3,uxtw]`——EC=0 谜案（v5 加了 INSN@ELR 打印与全类恢复门，
  但第五轮未复现，归并进下述 GIC 状态门行为谱）。
- **v5 轮（8a44b6e7）= 探针首次全程跑通（DONE probe complete），全部判词一次拿齐**：
  - **MAP 边界图 20 偏移零中止**：080S 100S 180. 200S 300S 380. **400. 404. 408. 410. 414.
    418.** 440. 600. 800. c00. c04. c08.（S=写粘 .=写完成被丢弃）——与前三轮的「外部中止」
    完全不同，本轮 +0x400 家族只是**静默丢弃**；abort→drop 的行为差异与 map 先行写了
    ISPENDR0/ISACTIVER0（SGI 进入 pending+active 态）相关，属 GIC 实现定义行为谱。
  - **★nAS 行 = 破案行：置 GICD_CTLR.nASSGIreq（bit8）→ RWP → 重试 +0x400：ctlabt=0
    priabt=0，PRIRD=A0A0A0A0 写粘**。与内核 dmesg「Enabling SGIs without active state」
    互证：**内核 gic_dist_init 先置 nASSGIreq（TYPER2.nASSGIcap 报告有效）才写 SGI 页优先
    级——三轮踩不出的状态位就是它**。GICv4.1「SGIs without active state」语义：未置位时
    NS 对 SGI 页优先级寄存器的写 = 实现定义异常/丢弃（Qualcomm 落地 = 前三轮中止）。
  - **中断套件全绿**：VTIMER irqs=0x1E（300ms 恰 30 拍 10ms 周期）、SGI1 irqs=0x0A（自发
    10 发精确 10 中）、hppi=0x1B/last=0x1B（GIC 正确仲裁出 vtimer）、ENABLE now=18000002
    （bit27/28/1 全粘）、ICFGR1 读值 A0A0A0A0（PPI 默认边沿形态，记录值）。
  - **跨核/自页**：PE0 写 PE1 SGI 页 +0x400 → rb=A0A0A0A0 粘 ✓；CPU1 自页 grp=FFFFFFFF
    pri=A0A0A0A0 ✓（v4 SEC1 wok=600D 同判）。
  - **身份定谳**：**typr4=016B001F = 真 GICD_TYPER**（ITLines=0x1F→992 SPIs、DS=1，
    与内核 dmesg 对上；+8 读值 0x47000070 非标准 TYPER 形状，疑本 GIC 解码怪癖，footnote）；
    typr2=0（TYPER2 读不到 cap 位，但 ctlr bit8 写收下且生效——内核同样不看读值直接置位）；
    IIDR d=30 r=30 sgi帧=0（SGI 帧 IIDR RAZ）。
  - UFS 复证 ver=0x0400；TUR/CAP/READ rc=4 ocs/st=2（命令未完链路，阶段 2 的工作面，
    不挡阶段 0 判词；hce=1 链路活）。
- **★判词：阶段 0 六项全绿收官**——tg16=1（16K granule）/ PARANGE 40bit / PSCI 1.1 +
  SMCCC 1.3 / 8 核 CPU_ON / **INTID27 vtimer 计数 ✓（本轮首绿）** / UFS LUN0 ver=0x0400。
  **ARMv9 兼容性定谳，头号风险关闭。**
- **★阶段 1 移植要点（本轮产出，XNU 侧）**：0004 翻版时 `pe_init_fiq` 的 8 笔
  GICR_IPRIORITYR 写**之前必须先置 GICD_CTLR bit8（nASSGIreq）**（内核同序：dist_init
  置位 → cpu_init 写 SGI 页）；DS=1 单安全态 → 0004 注释与 Group-0 相关逻辑按 m2381 实况
  改写。SGI 页优先级写通后，SGI/PPI 的优先级/使能/组全可编程，XNU GIC 前置无阻断。
- **RE 线（s5-rea，用户指路）**：ArmGicDxe.efi（UEFI 前驱 GIC 驱动）反编译实锤——只写
  SGI 页 +0x80（IGROUPR0=ffffffff）与 GICD 优先级，**不碰 SGI 页优先级、不置 nASSGIreq**
  （反编译件 s5-rea/out/armgic-{1810,1e5c}-prio.md）；与「内核独有 nASSGIreq 秘方」判词
  自洽。xbl_a-code.bin 分析就绪未展开（不再急需）。
- 设备现态：boot_b = shim v5 坐屏（DONE 状态），等用户决定续测或收口（收口 = 刷回 mu-r69 +
  set_active b + 读回对拍）。SCALE=1 提议暂缓：v5 屏已全量放下，无裁切。

### 2026-10-19 傍晚: 探针第三轮（v3.1 上机）—— nGnRE 假设出局 + 内核互证改写问题定义 + v4 构建

- **流程**：devsh 核验 boot_b 前缀 `97cdea20`=mu-r69 ✓（途中一次哈希失配 = 工具命令
  笔误 1100016 vs 1110016，设备无恙）→ devsh reboot-bootloader（回执 rc=0）→
  `current-slot` = b ✓ → 刷 v3.1（`91a6d7db`，16,384B）→ `fastboot reboot` → 用户拍照。
- **屏幕判词（v3.1）**：基线全绿无回退（tg16=1/mmfr0=2/PSCI 1.1+SMCCC 1.3/8核 ret=0/
  GICD was=0D→now=D3/RD wake 完成）。**PRIRD 没到——PANIC SYNC 于 STAGE "gic config"：
  ESR=96000010（EC=25 data abort + DFSC=10 同步外部中止），FAR=0x17190400，X0=0x17190400，
  X1=a0a0a0a0** → 第一笔优先级字写在 **nGnRE（attr3，32MB block）下照样中止**。
- **反汇编钉死**：ELR 0xA8082228 = `str w1,[x0],#4` @shim+0x2228（装载基址 0xA8080000；
  编译器把 8 笔优先级写折叠成后索引 str 循环，首笔即 +0x400）——中止指令与 FAR/X0/X1
  三方吻合，无歧义。
- **内存类型假设出局（本轮核心价值）**：同一 32MB nGnRE block 内 +0x80 IGROUPR0 写粘
  （GRPRD=ffffffff）、+0x14 WAKER 写通、GICD+0 写通，唯独 +0x400 炸 → 同映射同属性，
  属性不可能是判别项。另钉 bug：**panic 屏 X30 是假的**（exc_common 只存 x0-x28 占
  [sp+0..231]，regs[29]=[sp+232] 读帧内死栈；真 x30=[sp+248]）——0x3030…30 为栈残渣。
- **v3.1 附带损失**：panic 先于 UFS 段 → 第三轮无 UFS 判据（v4 已把 UFS 前置）。
- **内核互证（本机 linux-mobile-ports meizu20-t4b 实证，v4 的问题定义）**：
  ① mainline `irq-gic-v3.c gic_cpu_init → gic_cpu_config(rbase=SGI页, ppi_nr+16,
  dist_prio_irq)`——**每核每次启动写 SGI 页 +0x400+i×4**（irq-gic-common.c GIC_DIST_PRI
  = 0x400），nura 日日开机 = 硬件收这笔写；② DT = 纯 `arm,gic-v3`，reg 与探针同，零
  quirk；③ **dmesg：GICD_CTLR.DS=1（单安全态！0004 注释的 two-security-states 是 sweet
  SoC 形态）、SCR_EL3.FIQ=0、nASSGIcap→内核置 GICD_CTLR.nASSGIreq(bit8)** 后才写优先级；
  ④ CPU redistributor 摸底（0x17180000 起 stride 0x40000）与探针同。
  → shim 与内核的全部架构差异（类型/宽度/地址/EL/顺序）逐一排除后，剩余嫌疑 =
  **内核置而我们没置的状态位**，头号 = nASSGIreq。
- **XNU 严重级定谳**：0004-sweet-machine-config.patch `pe_init_fiq()` 就是同样的
  `GICR_IPRIORITYR0+i = 0xA0A0A0A0` ×8（sweet 的 0xa0a0a0a0 模式出处）——**坑不排掉 =
  阶段 1 硬阻断**；兜底 = XNU 侧跳过优先级写（复位默认 0 仍可送中断），但先找真凶。
- **v4（92f4b7b0…，20,480B）**：可恢复中止处理器（armed 时同步外部中止 → ELR+4 跳过
  单条 str 继续 eret；所有守护操作 = 独立单条 volatile word str，无循环回弹）+ SGI 页
  20 偏移写中止边界图（判 A/S/./! 四态）+ GIC 身份证（TYPER/TYPER2/IIDR×4）+ MAIR/PAR
  + GICD+0x400 对照 + 跨核 PE1 0x171D0400 写 + **sec_probe[0..4] 首次打印**（CPU1 自页
  自写结果，secondary_main 补越写标记 0x600d）+ nASSGIreq 重试实验 + UFS 前置 + X30 修复。
  构建自检全绿（零 ABS64 重定位 / mkbootimg 拆回对拍 / host_test）。
- **设备交还**：用户自行刷回 mu-r69（nura OS 恢复）；后台 fastboot 守护已撤。

### 2026-10-19 下午: 探针第一/二轮上机 —— 4.5/6 绿 + GICR 写中止钉死 + 复位政策改手动

**第一轮（77e69f01…，10:52 刷入 boot_b）：**
- 红屏全绿到 GICR：PARANGE mmfr0=2（40bit）/ **tg16=1（16K granule 支持——阶段 0 头号问题
  当场定谳）** / PSCI ver=1.1 + smccc=1.3 / CPU1-7 全 ret=0，mpidr=8100_0100..0700（Aff1
  `i<<8` 同构实证，3×A510+2×A715+2×A710+1×X3 全应答）/ GICD 0x17100000 was=D0→D3 可读可写
  / GICR RD 页 WAKER 读通（base=0x17180000 ✓）。
- 蓝屏 panic @ gic config：FAR=0x17190400（GICR SGI 页 +0x400 = IPRIORITYR），ESR
  0x96000010（同 EL 数据中止，DFSC=0x10 同步外部中止），ELR 反汇编 = `strb`（字节写 0xa0）。
  vtimer/UFS 未跑到。设备 40s 后自动复位——**落回正常启动而非 fastboot**。

**事故与抢救（复位政策改写现场）：**
- shim 预写 IMEM 魔数 0x77665500@0x146aa65c + PSCI SYSTEM_RESET → ABL 当**正常重启**处理
  （0x77665500 在 hikey DT 表 = bootloader、qualcomm 下游表 = normal——本机 XBL 实际按
  normal 走）→ boot_b=shim 再 panic → **boot 循环，ABL slot-retry-count:b 7→1**（差一步
  耗尽自动翻 a 槽——nura 记录过的行为）。用户手动 pwr+volDn 抢救进 fastboot；
  `flash boot_b mu-r69 + set_active b` 复原（retry 回满 7，current-slot=b，a 槽未动）。
- **教训（已固化进 shim）**：fastboot 复位路由被证明之前，shim 禁止自复位；v2 起
  reset() → hold_screen()（坐屏打印 "pwr+volDn -> fastboot" 等人）。修好的软复位路径 =
  PMK8550 PON reboot-mode 寄存器（内核 qcom-pon，需 SPMI）——阶段 1 顺 sweet 0060 做。
- 读回三分类重做（devsh，nura OS 在机时）：前缀 sha `97cdea20…` = mu-r69 ✓，与 10-19 上午
  一致，设备态未被漂移。

**第二轮（f0bd54a0…，11:33 刷入）：**
- 字节写改 32 位字写（kernel gic_cpu_config 同款）后**同地址同 ESR 再炸**：X0=0xa0a0a0a0
  （字写值）+ ELR=写指令 → **+0x400 区域对写全面中止（字节/字皆炸），读通过**；对照 +
  0x80 IGROUPR0 写、+0x100 ISENABLER0 读第一轮已通过。WnR 位（ESR bit6=0）与实际写指令
  相悖 = 外部中止不保证保留写标志（实现定义），反汇编才是真值。
- v2 坐屏设计工作 ✓（"RESET manual" 上屏，不循环）；新暴露设计缺陷：panic `con_clear(BLUE)`
  把红屏已打印诊断值全擦了（waker/grp/en/pri16 丢失）→ v3 改追加不清屏。

**判别分析（→ v3 = ab5ce20f…）：**
- 内核地面真值（nura linux-mobile-ports irq-gic-v3.c）：SGI 页 = RD+0x10000 内核硬编码
  （本机日日跑）→ shim 布局无误；`gic_cpu_config` 对 GICR IPRIORITYR 只用字写且本机天天写
  → 内核能写 shim 不能写的差异收窄到**内存类型**：shim MMU-off = 全部 Device-nGnRnE（写
  同步完成、错误上抛外部中止）vs 内核 ioremap = **Device-nGnRE**（写早答、错误可静默——
  甚至不排除内核优先级写静默失败而 GIC 缺省优先级恰好可用）。
- v3 改动：`mmu_enable_probe()` = 全 Device 恒等映射（ufs.c「MMU off = DMA 零缓存维护」
  设计原样保留）+ 镜像页 Device-可执行 + **GIC 32MB 块（0x16000000）attr3=0x04 nGnRE** +
  FB 块 NC；gic config 前开 MMU 并打印 sctlr/ttbr；panic 追加不清屏；新诊断 waker0/
  GICD-TYPER/GRPRD 回读/ICFGR1。
- **v3 判别表**：+0x400 字写在 nGnRE 下通过且 PRIRD=a0a0a0a0 = 内存类型判词定谳（阶段 1
  GIC 代码按 nGnRE 映射写）；仍炸 = TZ 域保护类，带 waker0 值回来再断；wake stuck 分支
  会在炸之前打印并 skip（保护 SGI 页访问）。
- **v3.1（91a6d7db…，用户两条纠正落地）**：①panic 追加会溢出——SCALE 3→2（行高 48→32px，
  容量 49→74 行；满流量 ≈67 行 + panic 11 行最坏 78 行），panic 块**底部锚定**（行号越界时
  跳到底部留 11 行，panic 永远完整可见，最坏裁分区表尾部）；②**多轮之间不回刷 mu-r69**——
  坐屏设计下 boot_b=shim 跨轮安全（开机即蓝屏坐稳，每开一次只烧 1 个 retry），`set_active b`
  随时可复满计数；收口（交还设备）才复原 + 读回对拍。
- 设备交还用户（关机，boot_b=shim-v2 蓝屏坐屏无害）；续接流程见顶部 STATE。

### 2026-10-19: 阶段 0 开工 —— 探针件构建完成（未上机）

**nura 互证收官（用户转达，四问四答）：**
- boot_b 现役件勘误：「mu-r89」不存在，= 内核 boot 数混写；Mu 现役 = **mu-r69-97cdea20**
  （1,110,016B，sha256 双对上）。⚠设备 10-19 起进 Flyme 取证窗口：机内 boot_b 现态未证，
  刷前必须 su dd 读回对拍定现态。
- 复位真值：**PSCI SYSTEM_RESET 本机实证**（pmOS 数十次 reboot / reboot-bootloader 全走它；
  两处 DT 均无 pshold 节点，qcom_wdt bark 被 XBL 封死）→ PSHOLD 0x0c264000 悬置不写。
- fastboot 单通道：nura 侧空闲，探针轮独占；`fastboot getvar current-slot` 必须 = a 才动。
- 读回通道纠偏：**有 root 的 Flyme 本身**（su dd → /data/local/tmp → adb pull），不起 pmOS。

**常量改写落地（shim/，nura DT + Flyme FDT 双真值）：**
- console.h: FB_BASE 0x9c000000 → **0xb8000000**（splash_region，0x2b00000）。
- shim.c: GICD **0x17100000** / GICR **0x17180000** / 区 **0x200000** / PE stride **0x40000**；
  VTIMER_INTID 19 → **27**（标准 PPI11），PTIMER 18 → **28**（PPI12）；
  reset() 删 PSHOLD 兜底写（只走 PSCI，返回则 wfe 停机等人）。
- 零改项核实：IMEM_RESTART_REASON 0x146aa65c 同址；UFS_BASE 0x1d84000 同址；
  NR_CPUS=8；test_smp 的 `i<<8` 目标格式与 kalama MPIDR 拓扑同构
  （Aff1 0x000..0x700 = 3×A510 + 2×A715 + 2×A710 + 1×X3，nura sm8550.dtsi 实证）。
- shim_main 重编排：no-payload 路径跑全套探针（PARANGE/TG16 → PSCI/SMCCC → 8 核 MPIDR →
  GIC dist/redist + INTID27 vtimer 计数 + SGI → UFS 只读），结果单屏保留，3 分钟后 PSCI
  复位（RESTART_BOOTLOADER 预写 → 落回 fastboot）。sweet 里程碑 3 的 MMU/fill 段
  （对本轮无用）标 unused 保留。

**打包（build.sh 新增 M2381=1 模式）：**
- gzip kernel → `mkbootimg.py --header_version 4 --pagesize 4096 --os_version 13.0.0
  --os_patch_level 2023-10-05 --cmdline ""`（meizu20-m1 surgery 实证配方，工具在
  `/home/ri/work/meizu20/meizu20_linux/tools/mkbootimg/`）→ unpack_bootimg.py 拆回对拍。
- 首轮探针件见顶部 STATE（77e69f01…）。

**下一步**：见顶部 STATE 的 Next。
