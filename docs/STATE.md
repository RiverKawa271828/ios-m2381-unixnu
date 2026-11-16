# STATE — 现势状态（m2381，2026-11-17 更新）
## 0. 现势指针（2026-11-17 s129 r138 已判 → r139 手机身份出件中；更早指针=archive/STATE-history.md）

**★★★★★★★★★★r138 窗判读定谳（11-17，窗 reader-r138 sha f092b310/rounds=398/503/uptime=795s 无回卷）**：
**G1 FAIL（dpu 链死在最后一环）+ 画像面全未变 + 仪器面立起 + M1 几何全在案**：
- **M1（r137 顺带门）定谳一半**：`[boot mem] phys=8c0004000 dram=80000000 raw=8/10262MiB adopted=0
  refused=0`——8 段窗外内存几何全在案（**窗落 bank2 中段**；窗下 0x880000000+0x38700000=903MiB、
  窗上 0x9f0010000+0x20fff0000=8.25GiB 到 0xC00000000）；**adoption 循环一个没吃且 refused=0**（连
  过滤都没记）——r140 刀口=adoption 过滤的 gPhysBase 对拍（0x880000000 run end=0x8b8700000 <
  0x8c0004000 理应过 below-span 滤网却静默全拒）。
- **G1（dpu 直扫主门）FAIL=仪器立功**：survey 健康（w0=80460669=105 组/70 流/4 银行、**found=30003
  i=3 bank=3=i562 假设命中**、s2cr/sctlr 条件全过、TTBR/TCR 写入回读 cc9ff000/800020 生效）→
  **st=dead0005**：50ms watch **us=0 第一拍**撞 fsr=0x402 撤退（0x400=旧账 bit10 不在 kDpuFaults
  0xc00001fe 掩码里；**bit1=AF 对齐错误=新账**=显示流第一个取数就崩）。**刀口（r140）**：dpuEnable
  入口清 FSR 再开 watch + FAR（undo 已读 c[4..6] 但行没印）面化——AF 根因等 FAR 定谳。
- **G2 未变**：SWAPQ 尾行 val=57 kick=2797 xfer=0 nohead=57 **copy=56/57 仍 100% CPU copy**（r136
  基线 39/40；直扫没接上=回退设计按预期走，fb=0 因 directOn 未武装）。bytes=10368000=单 surface。
- **G3 未变**：节拍器 5.0s 贯穿 + 尾部精确 60s 心跳（W53–57）；触摸 20 笔划只 W33/34@388s 一簇
  响应（+36ms 第二帧后即停）——触摸→出帧链照旧大半断。
- **G4 全绿**：SMP run=7、ADSP st=13、battery st=4 pct=100 mv=4408 usb=1、AKS st=0 sg=3、UFS
  126 万请求零 fail、无 recover/fault/stall 行；fence 执行 884/882 健康、wptr 零掉写；
  **timeouts=1199/1369=s128 等待检测滞后残留未修（非本轮靶）**；EXIT 2643/nonzero 1173 同量级。
- **[gpu ring] 仍 0 行——谜底=Mac Adreno740 HEAD=54b41b8，a7d2ded 修从未进 Mac**（r137 只改了
  Linux 子模块；bundle 流只传了 GopFramebuffer/ArmCpu）。下一轮 kernel 重编时把 a7d2ded 带 Mac。
- **仪器新知**：SWAPQ 行数据健康（此前"全零"判读=抓了窗头槽位；**计数器是累计值，判读必须取窗尾**）；
  fwait=56 新脸（copy 路也有 fence wait）。
- **r138 出件三坑补账**：树步按父仓 gitlink checkout 子模块（指针不记=链接丢符号，实炸两次）+
  子模块对位必须 `--hard`；Mac"GopFramebuffer r134 脏树"=内容逐位等于 Linux 已提交态（sha 对拍后
  checkout 净化零风险）。

**★★★★r139=手机身份轮（payload-only，kernel 5081facb 不动，与 r138 判读同会话出件）**：
用户拍板修平板身份脸（r138 首看：iPad17,3 身份 + unit-scale=3 → UI 偏大裁切）。**身份真值全部取自
Apple 真 DeviceTree.d48ap.im4p**（iPhone 16 Plus @ 24A446 同 build，从 Apple CDN 定点 range 抽取
解析 2026-11-17）：root=model iPhone17,4/target D48/compatible D48AP；product=phone idiom/subtype
2796/scale 3/P3/"iPhone 16 Plus"/fallback iPhone16,2/thin-bezel 1。落板级覆盖（board.toml [adt.*]，
deep merge 板级后赢）设备仓 `c62a959`。**先例账**：uke（SM7675 手机）已在手机上跑同 24A446 profile
（booted，dock UI 症状已解——仍是 pad 身份）；pong 备了 iPhone17,1 身份 on iPadOS 18.7 donor（设备
结果 pending）；asteroids 把本条 iPhone17,4/24A446 车道备到 userdata 未刷。**做成=首例 iPhone 身份
的 iOS 27 build 上机**。chip-id 0x207/board-id 12/donor [ipsw] product=iPad17,3 全不动（单变量）。
回滚锚=r138 payload（39de885c）。判据：UI 换 phone 脸（dock/图标/状态栏）；dpu/GPU/T0 行为不回归
（kernel 同 binary）；若仍 pad 脸→查 MobileGestalt Data 侧缓存（外科 prefs 注入，不用 data-reset）。



**s128 续（2026-11-17）——r137 取证轮双件出件完毕，待用户刷**：r136 窗判读（详块见下）后，M1 缺口
取证刀落地：**boot_b `out/m2381/boot-b-e9085e5a-r137.img`**（FD `b7bd804e`=DarwinBoot 4583617 不变+
RamManagerDxe `RAMRANGE` 印条（uefi-m2381 `dad0e255`，XBL RamPartition 上报的可用区间直读））+
**recovery_a `out/m2381/payload-r137-bootmem-30e950c5.img`**（kernel 输入 `0483cdb4`；series **101**
=+`outside-memory-diag.patch`（below-span 过滤**前**捕获 loader 原始 pairs+gPhysBase/gDramBase 进
`native_boot_memory[]`）+BootLog `[boot mem]` 行+Adreno740 `a7d2ded`（face submitted 计数挪到
submitFramed=修 r136 `[gpu ring]` 缺席——根因=Ring::submit 生产从不调用）。主树 `ff92b33f`/`638be888`
（patch 重生成：首版单行上下文 hunk git-apply/patch(1) 双败）+设备仓 `62a1f55`/`28d743a`+uefi
`dad0e255`。**判据=R137-CARD.md**：`[boot mem]` 一行定谳（raw>0/adopted=0=窗位错；raw=0=map 缺 bank2）。
args/addon/trust/seed 全同；回滚锚同 r136。

**s128 判读（2026-11-17）——r136 窗已拉已判：GPU 三门过（冻结未复发+取证面立起），M1 未过（16 GiB 没接管）；存储三连答已给**：

**刷写实证**：boot_b=sde43 dd 读回 sha 逐字节=`b4849654`（新 FD 确实启动）；recovery_a=sda10 首块
`M2381`=r136 payload。窗 `out/m2381/reader-r136/`（sha `39f5b838`，rounds=253/503 uptime=504s 无回卷）。
**G2/G3 过**：GPU 本轮健康——fence submitted=1590 completed=1588、`[gpu wptr] writes=1590 dropped=0
repeated=0 stuck=0`（掉 WPTR 写=零）、无 `[gpu recover]`（=无 hang，门逻辑正确缺席）、无 stall/unstick；
`[gpu ring]` 行缺席=待查（gate=ring_face[2]>0，fence_wait 850+ 次应已填 face——疑 020f460 未进内核或
weak 未链，核 kernel.macho 符号）；`[gpu ring]`/`[gpu fault]`/`[gpu recover+]` 同查。T0 全绿（SMP run=7、
ADSP st=13、battery st=4 ma=-462、AKS sg=3、UFS failures=0 922k reqs）；崩族=duetexpertd sig6 旧脸
（slide +b24000），EXIT 1783/nonzero 754；PosterBoard 首现客户端（s3 4187M）。
**M1 未过**：`MEM totalMiB=4864`——adoption 没发生。排查链：DarwinOutsideRanges 两侧都收（窗上/窗下，
MemoryWindow.h 实证）、adoption 依赖链全接（QcomPkg.dsc.inc→RamManagerLib=XBL RamPartition 协议版；
m2381.fdf:87 有 RamManagerDxe）、ADT 三属性在、内核 patch 在（series 100 干净应用）→ 缺口收窄到
**UEFI 内存图里没有窗外的 conventional 段**（MemoryMapLib 静态表止于 0x100000000；RamManager gap-fill
范围=XBL RamPartition 条目，bank2 是否在其中待 boot 屏实据）。**决定性实据=boot 屏 DARWINBOOT 行**
（`ARM64 va=… pa=… owned=…` + `free memory outside the window: N ranges of 8, X bytes`）——请用户下轮
boot 拍屏；或 r137 在内核侧把 outside 表打进窗。**注**：stock cmdline `memsize=3072000` 推 12G 一说不采信
（用户定谳 16G，nura 已更正该误记）。
**存储三连答（用户问"很卡是否空间不够/6G vs 15.79G/能否扩 128G"）**：①卡≠存储——Data store 16 GiB
（`D st=2 … pages=4194304`）、present ~1.95 GiB、可用 15.79G 属实，无 thrashing 脸（UFS 0 fail；"数据不断
转移"不存在）；卡=5s 交帧节奏（client 帧循环，s127 定谳未变，offer/done 毫秒级每帧秒完）。②About
"总容量 6G"=卷上报 bug（s126 立案的 6GB/15.53GB 不一致，本轮 15.79G 与 16 GiB store 几何一致——错的是
"总容量"栏）。③扩 128G 技术可行（userdata LUN 463.6 GiB，Data store 只占 16 GiB；扩=seed 重建+
data-reset=清装机数据，红线决定）但**不消卡**，后置。
**回滚锚**：boot_b `fc2e8f17`/payload `a29c87f9`（r135），或单回（新旧交叉自动 no-op）。
**下一轮工作单（r137）**：①拍 boot 屏 DARWINBOOT 行定谳窗位/bank2；②按实据修 map（RamManager
Print 增强 or MemoryMapLib 静态补 bank 2）+内核侧把 outside 表打进窗；③`[gpu ring]` 行缺席查因；
④5s 交帧节奏（用户态）不变优先级。



**★★★★★★★★★★s128 开窗判读（2026-11-16）——r135 窗已拉已判：GPU 执行面通了（fence 楔子破、stall 归零），三处残留=冻结相位 118 s/等待检测滞后/客户端 5 s 节奏；电源/触摸验证全过**：

**窗核验**：`out/m2381/reader-r135/`（sha `72448ef7`，header rounds=276/503 slots_written=276/503 uptime=550s
write_errors=0，无回卷残影）。`recovery_a`=reader（r135 窗已拉完， reader 可继续用或按刷写纪律覆盖）。

**G1（GPU 执行面）主门过**：`[fence wait]` **submitted=1322 completed=1267**（r134: 204/9；r132: 500/9）——
**楔子破了**；`[gpu stall]`/`[gpu unstick]` 全窗 **0 行**（r132 有 3018 行）。X4 `r1e8f/1e8f`=ring head==tail
（CP 吃空）；`[ring]/[ib]` 快照是真命令流（`[ib] iova=10f311000` >32-bit peek 已修好，r132 的 deadbeef 消失）；
`[fence page] dram==cached` 全程一致。method6=WaitFence 1826 calls/354 超时。54b41b8 系列（BV 进度/快照/fault
隔离）+36/48-bit peek 修法**实机生效**。

**新发现①双 ring + 冻结相位 118 s**：preemption 两 ring（high/low；`ringForFence`: fence≤ring0.lastSubmitted→
ring0，否则→ring1）。fence 页 word1 **t=14→132 s 恒=0x9**（low ring 冻结，只执行过 9 个 fence），**t≈134 s
瞬间解冻**（word1 9→0x4e9=1257，backlog 秒级执行完）；解冻点与 **[gpu clients] 从 2 客户端（cameracaptured+
backboardd）变 4（+SpringBoard+iconservicesagent）** 同拍——Metal 客户端首次 open 与解冻相关。稳态后 fence 页
每 5 s +2（=客户端交帧节奏，GPU 从不落后）。
**新发现②等待检测滞后**：稳态（t>150 s）completed_=1267（落后 fence 页 55）且又超时 **328 次**——fence 页
GPU 已写、wait() 仍超时；`completed_` 只在 wait() 成功时推进（NativeAdrenoRing.cpp:657），stall 机全程 0 行
（r132 的 noteStall 盲区仍在，有积压的 low ring 从未被记录）。候选机制=fence 页映射属性/acquireDeviceWrites
在本板不生效（cache_.acquire 已装、效果存疑），或 stall 机只看"最新提交的 ring"。
**5 s 节奏=客户端侧未变（s127 预言坐实）**：W 间隔 t=165–268 s 恒 ~5.0 s（期间零触摸），尾部精确 60 s 间隔
（419.995/480.003/540.004=定时器心跳）；7 次触摸各自立即出 1–2 帧（offer 0–2 ms/done 2–128 ms）。r134 的
41–972 ms 窗是连续滑动时段不可比；但 r134 W82–W94 有 13 帧 0.65 s 减速流（flick 流）而 r135 触摸后 2 帧即停
——手势流式是否还在待 r136 主动滑动验证。**GPU 每帧秒级完成，5 s 节奏=客户端交帧节奏**（内核 pin 上抬不消此墙）。
**G4 addon 过**：`A files=6 wr=4 same=2 mount=3/2`（新 addon 件真落盘），M a3.2/1.2、K86、plugin opens=10/
closes=7（无拒绝）、[gpu clients] 4 客户端持设备（backboardd s0 4709M）、owner trust 无签名失败。
**G3 T0 全绿**：**SMP run=7**（c1–c7=step7，r134 run=0 观察点解除=当时瞬时态）、ADSP st=13 rx/tx 流动、
battery st=4 pct=99 ma=-519 usb=1（充电链无回归）、DATA st=2 er=0、AKS st=0 sg=3、DISPLAY stage=10 owned=1、
UFS failures=0（965k req）、IRQ top=27（timer；29k→10.5k/s 随 churn 回落，spawn 4376）。
**崩族**：EXIT 1980/nonzero 850/crashsig 9（r132: 28548/14138——churn 塌一个量级）；DockFolder sig11 unslid
**0x20f9723dc=r131c 同址**（TNode::GetNodeFromPath+0x2b4 旧脸）；duetexpertd sig6 同 abort 族；新脸=
cameracaptured `pipelineLibrary:error: (answered nil)` + iconservicesagent VkResult(-5)（持续）。
**DeepSeek 三域验证**：电源 ✓（r128b 里程碑保持）；触摸 ✓（TCH2–5 健康、TLMM 拆除在位、7 笔划到用户态且
UI 立即出帧）；GPU ✓ 内核执行面通（残留=上述三处）。

**s128 工作单（r136，按序）**：
1. **等待检测滞后**（内核一刀）：静态核 fence 页映射属性与 acquireDeviceWrites 在本板路径（确认 cache_.acquire
   是否真失效）；completed_ 改为 observe/完成路径也推进（不只 wait 成功）；BootLog 加 per-ring fence wait 行
   （ring0/ring1 各自 lastSubmitted/completed/seqno + fence page PA）终结归属歧义。
2. **冻结相位取证**：ring1 首个提交时刻 + WPTR doorbell/GMU AHB fence status 接口核对；确认"SpringBoard open
   唤醒"是 ring-switch 还是 power 路径；若 doorbell 未发→补发路径。
3. **客户端 5 s 节奏**（用户态，s127 遗留）：m2v runtime wait_until_completed 计时打点 → addon 重编；
   CA/backboardd 帧循环 5 s 上限定位。
4. **16 GiB 内存窗平台轮（s128 侦察已立，不消三堵墙但消内存压力类）**：16 GiB 物理（dts `memory@80000000
   reg=<0 0x80000000 0x4 0x0>` 已声明），现行只给 XNU 4.75 GiB 单连续窗口（`board.toml [payload.adt]
   owned_memory_size=0x130000000`；SM8550 物理映射连续段 ~5.5 GiB 扣 PIL 等 carve 的净值，现行生成器
   只支持单窗口 256 MiB–8 GiB）。**上游全套可抄**：DarwinBoot tip `5243c68`（我们 pin `b33d75a`）带
   `owned_memory_outside_ranges`/`owned_memory_outside_reserved`/"BASE:SIZE" 表 + `memory-outside` 开关
   （窗外空闲 DRAM 内核逐页试读后接管为 VM 内存）；内核补丁 `window-outside-memory-adoption.patch`
   （621 行，pmap/pmap_data/arm_vm_init/machine_routines）现只挂 macOS 27.3 profile，24A446 未带——抄入
   我们 series（97→98）；carve 清单已盘点（PIL 0x8A800000–0xA2A80000、AOP 0x81C00000–0x81CA0000、
   gpu_micro_code 0x9b09a000、zap、framebuffer、loader inputs=dts reserved-memory+UEFI MemoryMapLib）。
   同批上游还有 Data seed 脚本修复（seed-output/kernel-only 绑定/APFS 发现）。**判据**：MEM totalMiB
   4864→~16000、free 稳态显著抬升、r132 式 churn 风暴不再复发、T0/GPU 门不回归。**顺序**=GPU 三堵墙
   收口后（r137 平台轮），或与 r136 内核轮合并由用户拍板。iconservicesagent VkResult(-5)
   （MEMORY_MAP_FAILED）在内存轮后复看——可能是池/映射上限的脸。
5. **r136 判据**：稳态 wait 超时≈0；completedFence 贴住 fence 页（差≤在飞数）；主动滑动出现 ≥10 帧连续流
   （W 间隔 <120 ms）；冻结相位不复发（t<60 s 内 fence 页推进）；T0 不回归。
**回滚锚**：r134 `payload-r134-irqring-b09ec0af.img` / r133 `payload-r133-uiscale3-07935d96.img`。

---

**★★★★★★★★★★s127 收队（2026-11-16）——r135 已刷已测：硬件加速仍未起来，s128 第一刀=拉 r135 窗归因（已执行，见上）**：

**设备现态**：用户已刷 **r135**（`payload-r135-gpuseries-a29c87f9.img`），实测**硬件加速仍未起**；`current-slot`=b，
`boot_b`=r129 `fc2e8f17`（未动），种子 `9c28654c` 零改动。**设备重启**：reader 里 `/bin/rebootbl` 回 fastboot 后
`fastboot reboot`；若已在系统内长按电源重启（无需再刷任何件）。**资产回收已完成**：构建日志/`gpuaddon-r135.bin`/
`trust.json`/owner trust 在 `out/m2381/r135-assets/`（addon `c9dcedcc`、trust `3a7da3e9` 已双端对拍）。
**判据缺口**：没有 r135 窗（用户直接测完）；r135 boot 的 UFS 环仍在，s128 先刷 reader `d01bed78` 拉窗。

**s128 工作单（按序，先归因再改代码）**：
1. **拉 r135 窗并判读**（核 header rounds/slots_written/uptime）：
   - `[fence wait]` 的 `completed/wrote/submitted` 是否推进（上游 BV 进度修法是否生效）；`[gpu stall]` 行数与快照；
     `[gpu unstick]` 动作数；`[swap timeline]` 帧间隔（不得回归 >1s）；`SWAPQ` 的 `fwait/to`。
   - **addon/插件**：`[gpu addon]` `A files=6 mount=3/2`、`M a…`/`K` marks、`[gpu clients]`/`[gpu ibs]`/`[metal askers]`、
     `native_adreno_metal_diag`（m2v 设备创建/selector/attach）；**owner trust 新 CDHash**（`17067100`/`af0d0f86`/`f3449fcf`）
     是否被接受。
   - **T0**：SMP `run=?`（r134 是 run=0 回归）、ADSP/battery/AKS/DATA/DISPLAY/UFS、崩族。
2. **按窗分叉**：
   - fence 推进但 Metal 无 → 卡点移到用户态：m2v/Turnip 设备创建、`[metal table]`、QuartzCore 全局写（per-cache 表）、
     插件是否被 AMFI/owner trust 接受；控制变量=AdrenoMetal 回 `c68f9a1` 单变量对照，或按 diag 打印逐段定位。
   - fence 仍冻结 → 驱动修法不充分：查新 AdrenoMetal↔Adreno740 客户端 ABI（提交/命令流版本）、`[gpu stall]` 快照
     fault 候选、CP protect/UCHE/TSB 状态位；必要时在 18 个上游提交里二分。
   - 帧拍回归（>1s）→ 先回 r134 `payload-r134-irqring-b09ec0af.img` 保桌面，再分析。
3. **回滚锚**：r134 `payload-r134-irqring-b09ec0af.img`（帧拍 41–972ms 可用）/ r133 `payload-r133-uiscale3-07935d96.img`。
4. 后置（均不消现墙）：16 GiB owned_memory（DarwinBoot `b33d75a→5243c68` 的 memory-window 属性已在上游）、Data 容量/
   About 上报、缩放画布（430 pt）、iPhone lane（触发条件见本文件早前段落）。

**★★★★★★★★★★s127 交接（r132 判读，2026-11-16 会话一百二十六收队）——桌面达成，GPU 门开但执行面 stall，5 s 帧拍/IRQ 风暴仍在**：

**s127 已做（r133 payload-only 出件，2026-11-16）**：工作单第 1 项完成。`out/m2381/payload-r133-uiscale3-07935d96.img`
（99 MiB，sha `07935d96…`，Mac/Linux 双端对拍 ✓）——相对 r132 **唯一 delta = args `offline-legacy-unit-scale=2→3`**
（1080x2400 → 360x800 pt）；kernel 输入 `544f19a7` **逐字节同 r132**（payload 步未重编内核，已核），payload kernel
`325a222c`/LZMA `0f926828`、addon `13398d63`/trust `ccff5d20`/seed `9c28654c` 全同；ADT `fe4a0f3c` 只是
`/chosen` random-seed 每轮新生成，非 delta。卡=`out/m2381/R133-CARD.md`。

**r133 判读（用户面，2026-11-16）**：**U1 通过——手机 UI 达成**（360x800 pt）。瑕疵=控件偏大
（dock 栏太大，iPadOS 布局在手机画布上的固有脸，iPhone lane 才是解）+ Settings 滑动不可用（=0.2 fps
帧拍，与 GPU/帧拍墙同源，用户拍板"等 GPU 修复"）。**scale=3 保留，不回退。**

**s127 已做（r134 内核轮出件，2026-11-16）**：`out/m2381/payload-r134-irqring-b09ec0af.img`（99 MiB，
sha `b09ec0af…`，双端对拍 ✓）。kernel 输入 **`161432cf`**（新）：①`ArmCpu::interrupt` 记 **INTID 环**
（32 槽 `(core<<24)|INTID`，页 `IRQ RING` 打最后 8 个 + 窗口最高频 `top=<id> x<k>` + `long=`/`unh=`）；
②Goodix identify 后**拆 gpio24/25 TLMM 引脚中断**（INTR_CFG/STATUS 清零，`TCH5 pinint` 留拆前现场；
源=stock 节点 `goodix,irq-gpio` flags `0x2008` 武装 gpio25 下降沿，pinctrl-msm sm8550 +0x8/+0xc）；
③页新增 **`SWAPQ` 行**（swapstat[16]+`native_legacy_swap_timeouts`+实时队列深度 `native_legacy_swap_pending`）。
payload kernel `16c0fdb0`/LZMA `199db47f`；addon/trust/seed 全同；args 与 r133 逐字同（scale=3 保留）。
卡=`out/m2381/R134-CARD.md`。**预期帧拍仍 ~5 s（不修 GPU）**；判据 G1 IRQ 命名（top=240→本修生效；
top=0/27→改查 IPC/timer）、G2 SWAPQ 失配脸、G3 TCH5 拆前现场、G4 T0/崩族。
**r134 判读（已刷已拉窗，窗 `out/m2381/reader-r134/`，sha `aed4934d…`，header rounds=353/503 uptime=711s 无回卷）**：
- **5 s 帧拍消失**：`[swap timeline]` W 间隔 **+41–972 ms**（多数 41–67 ms，偶发 380/972），offer=0、done=6–18ms。
- **IRQ 风暴定性=INTID 27（arch timer PPI）**：`IRQ RING last: 27… top=27 x32`，速率 3.3–4.7k/s（r132 19k/s），
  `long=0/3`、`unh=0/0`；**TLMM 不是风暴源**（`TCH5 irq cfg=0x000000e2 status=0`=靴子武装过但从未挂起）；
  r132 的 GENI/引脚两个假说都死。速率随 churn 崩塌而回落（spawn 1188 vs 56473）。
- **SWAPQ**：`val=105 ret=105 to=0 fwait=0 copy=104 depth=0/2`——无 family 5 s cancel，显示走 CPU copy。
- **GPU fence 楔子仍在**：`[fence wait] calls=202 timeouts=201 last=204 wrote=203 submitted=204 completed=9`；
  `[gpu stall]` 0 行；`[scanout wait]` 970/865 超时（500ms 级）；`[gpu ibs] submissions=196 ibs=2511`；
  `[gpu clients]` backboardd s0 4860M。
- **T0 回归观察点：SMP `run=0`，c1..c7=step3**（r132 run=7）——LZMA/SMP 旧悬案或 r134 ArmCpu 环改动嫌疑，r135 观察。
- T0 其余：ADSP st=13、battery st=4 pct=100、AKS st=0 sg=3、UFS failures=0、DISPLAY stage=10 owned=1。
源码：设备仓 `ad274ad`；ArmCpu/GopFramebuffer 为**工作树改动**（pin 未动，diff 存 `out/m2381/r134-diffs/`，
子模块仓 origin=UniXNU 无推送权）。**r135 工作单=GPU 执行面**（卡尾）；关键更正：r132 的
"BV IB2 base 0xffff0000" 是 `peekFetch` 的"索引未知"编码 + 16×deadbeef=peek 读失败，不是坏基址。

**s127 GPU 侦察（r132 窗重读，r135 的靶）**：①**帧不是 5 s 才被 GPU 画完**——`[swap timeline]` 每帧
`offer=0ms done=12–18ms`（早退），慢的是**客户端每 ~5005 ms 才交下一帧**；②`[fence wait]` 全程冻结在
`last=500 wrote=499 submitted=500 completed=9`：**fence 500 永不完成**，之后所有等它的 wait 全超时
（2416 calls/2415 timeouts），9 = 冻结前完成的等待数；③stall 状态机以 `next=observed+1`（CP 正在做的
fence）为键，`if(next!=stall_fence)` 每次新提交就把 kick/drain 计时**清零**（NativeAdrenoSubmit.cpp:590），
fence 500 这一幕 stage 永远到不了 kick（窗里 kick=2 只是 fence 88/382 两次短 stall 的）——r135 候选修法=
**跟踪最老未完成 fence**（不是最新），并让 kick 在等待线程无限阻塞时也能跑；④peek 失败只在 **>32-bit
IOVA**：CP 的 IB1_BASE 实测 `0x1_c5e9000`（hi=1），`peekWordsUncached` 走不通 → 全部 deadbeef（先修
peek 的 36/48-bit 走表再读 IB 字）；⑤SMMU `FSR=400 FAR=0x1b72e_fd2f4ffd` 是一条野地址翻译错，与 CP 的
IB 地址不同域，需在 r135 用 linux arm-smmu 定义解码；⑥`nmb_command_buffer_wait(handle,0)` =
`wait_until_completed()`（无期限，metal2vulkan-runtime command_buffer/mod.rs:3985），完成门由 Vulkan fence
打开——门不开则客户端线程停在那；前端的 5 s 脸要查 CA/backboardd 帧循环的 5 s 上限（尚未定位，r135 第一刀）。

**上游更新侦察（s127，upstream/main `71400b56`，落后 143 commits，merge-base `3375ac9d`）**：**我们的内核补丁面零变化**
（`patches/xnu-12377` 0 文件、`ipados-27.0.1-24A446` profile 0 文件、只有 `macos-27.3-26C5035r` 在长）——所以
可以只挑驱动 pin 动，不必整分支 rebase。**关键更新**：
1. **Adreno740 `653e11c→54b41b8`（+18）＝正是我们墙的修法系列**：`2a91aa1 把 BV 线程进度算作 GPU 进度`、
   `9578923 stall handler 不打扰正在干活的 GPU`、`172f53f ring switch 未完成不归罪任何 job`、
   `031f35c/a8fa18f 一个进程地址空间出错不再拖垮其它 AS`、`54b41b8 保留卡死 CP 快照并定位 fault 地址候选`。
   fence 500 卡死 + BV 不推进 + 旁观者受害，逐条对脸。
2. **AdrenoMetal `6d7f734→9f6a887`（+51，qualified Metal main）**：texture policy、Turnip concurrent binning 回
   Mesa 默认、present/diag 修复——**必须内核+插件/addon 同轮重编**（客户端 ABI 对）。
3. **DarwinBoot `b33d75a→5243c68`**：新增 memory-window 属性 `darwinboot-memory-excluded/-loader-inputs/
   -memory-outside`（`scripts/unixnu/devicetree.py` +31 同批）——第 4 项 16 GiB 的正路（窗口跨多段 DRAM/收养窗外内存）。
4. 其它：Data seed 脚本修复（seed-output 集成/kernel-only 绑定/APFS 发现）、payload/bootimg FAT 发布与 ramdisk
   编码修复、ShengUfs/QcaBluetooth pin（macOS 修复，与我们无关）。
**r135 建议**：只做**双 pin 上抬**（Adreno740+AdrenoMetal，components.toml 与 gitlink 双落，s125 教训），先读
`2a91aa1/9578923/172f53f/031f35c` 四个 diff 确认，再出 kernel+plugin(addon+owner trust)；整分支 rebase 留到
GPU/IRQ 收口后（Mac 镜像要重 rsync，且 seed/payload 行为会变）。

**s127 已做（r135 出件，2026-11-16）**：`out/m2381/payload-r135-gpuseries-a29c87f9.img`（99 MiB，sha `a29c87f9…`，
双端对拍 ✓）。**双 pin 上抬已落**：Adreno740 `653e11c→54b41b8`、AdrenoMetal `c68f9a1→9f6a887`（components.toml
与 gitlink 双落；catalog 旧值 6d7f734 同改）；build-adreno-metal-bridge/plugin/turnip 与
`scripts/data/gpu-metal-vulkan-inputs.json` 取上游版（translator `963f0e64→555b13d5`）；新增上游
`macho-self-bind-weak-imports.py`；**内核新增补丁 `gpupool-device-pager-close`（96→97）**——新驱动 pool view
（`NativeAdrenoPublish.cpp`）弱引 `iokit_physical_descriptor_close_notify`，上游只在 macOS 27.3 profile 带它；补丁是
通用 IOKit 代码。kernel 输入 **`02a92612`**（`bvProgressState` 与 hook 已在符号表核实），payload kernel
`d09b786f`/LZMA `eedaa24e`；**addon `c9dcedcc`**（plugin CDHash `17067100`/daemon `af0d0f86`/Turnip `f3449fcf`；
plugin 用 `--no-replay`：新驱动 user-client TU 引 `sys/resource_private.h`，host 模拟缺该头；replay 只是 host 诊断）；
**owner trust `3a7da3e9`**（`make-owner-trust-bundle.py` 已泛化 `--plugins/--originals/--output`，默认行为不变，24A446
原始两模块字节未改）；seed 零改动、args 与 r134 逐字同（scale=3）。卡=`out/m2381/R135-CARD.md`，**待用户刷**。
判据：G1 fence `completed` 上升/不冻结、帧间隔不回归；G2 unstick 动作下降；G3 **SMP run=7（重点）**+T0；G4 addon 挂载/插件。

**设备/件现态（交接时）**：用户已刷 **r134**（`payload-r134-irqring-b09ec0af.img`），窗已拉
（`out/m2381/reader-r134/`，sha `aed4934d…`，header rounds=353/503 uptime=711s 无回卷），recovery_a 现=reader（拉窗后）；
`current-slot`=b，`boot_b`=r129 `fc2e8f17`（未动），种子 `9c28654c` 零改动。**Setup 已走完、进入桌面**
（About：iPadOS 27.0.1 / 可用 15.53 GB）。

**r132 判读（三句话）**：
1. **沙箱通配生效，GPU 门开了**：`[gpu clients] backboardd 2/0/1 s0 4888M`（render server 首次拿到设备）、
   `[gpu ibs] submissions=492 ibs=7376 max=128 over16=137`、method 1/15=22（16 客户端 + 6 backboardd）、
   `X7 K430`。GPU 合成路径已接上。
2. **但 GPU 执行面 stall、帧仍 5 s 一拍**：`[gpu stall] fence=382`（3018 行，BV IB2 base `0xffff0000` deadbeef，
   job>=1662 br=1658 bv=503）、`[gpu unstick] kick=2 last=382 … unit=0us`、`[fence wait] calls=2416 timeouts=2415
   last=500 wrote=499 submitted=500 completed=9`（等待把 0 当超时）、`[scanout wait] calls=2369 timeouts=2162
   asked=500ms mean=500.8ms`（scanout client 从未拿到 offer）。`[swap timeline]` p50=**5004ms**、p90=5123、max=49568；
   `SWAP calls=208 accepted=1 copy=208 retired=208`。显示族 5 s 看门狗仍是帧率上限。
3. **IRQ 风暴没被 r132 的 GENI 修复压住**：IRQ=23.87M/1246s（19k/s；尾部 34–36k/s），r131d 15.9k/s，
   r130-s2 基线 2.5k/s。GENI `SE_IRQ_EN` 不是（唯一）风暴源；新候选=**TLMM gpio25 引脚中断**（bootloader
   可能留着 intr_cfg 使能，帧线程只轮询、不清 intr_cfg/status）或 spawn churn/IPI。需 INTID 取证。

**同时记录（判读债/新账）**：
- **churn/内存压力**：spawn=56,473（尾部 435/s）、EXIT 28,548/nonzero 14,138、xpcproxy 28,419/14,325；
  free 在 5k–58k 页间摆动（20–230 MiB）。`iconservicesagent` m2v `no-device VkResult(-5)`（内存分配失败）
  且 10 open/10 close 循环。5 s 帧拍 + 4.75 GiB owned + churn 叠加=用户体感“惊天动地”。
  窗里**没有 EXIT 记录**（sheng-banner=0 把 CRASH/EXIT 画布也关了）——下次取证窗要么开 banner，
  要么把 exit 记录写进 [page]。
- T0 全绿：SMP run=7 / ADSP st=13 / battery st=4 pct=100 / AKS st=0 sg=3 / DATA st=2 / DISPLAY stage=10 PM=4 /
  touch HID started=1（142 strokes/1886 reports）/ UFS failures=0。

**下一会话工作单（按序）**：
1. **r133 payload-only（用户已点名）**：args 把 `offline-legacy-unit-scale` 改成 **3**（1080x2400→360x800 pt
   像素精确；asteroids/Nothing Phone 3a 的 bring-up 同值），看 Setup/Settings 点尺寸与布局；不好就回退 2。
   内核/种子不动。
2. **5 s 帧拍/IRQ 取证（主墙）**：给异常入口加 **GIC INTID 环形记录**（page 打最后 N 个 IRQ 号）；单变量试验=
   identify 后清 **gpio25 intr_cfg(+0x1008)/status(+0x100C)**（也查 gpio24）；同时把
   `native_legacy_swap_timeouts` 与 validated/pending 队列状态打进 [page]，确认 family 的 5 s cancel 与
   completeCurrentSwap 失配。
3. **GPU 执行面**：BV ring 的 `0xffff0000` IB2 基址（插件命令编码/提交路径）；`nmb_command_buffer_wait(handle,0)`
   的 0 超时语义（frontend）；scanout client 收不到 offer（fb-gpu=1 的 handoff）——先让一帧真正 GPU 完成/呈现，
   再谈流畅度。
4. **内存 16 GiB 题**：现 `owned_memory_size=0x130000000`=4.75 GiB（MEM totalMiB=4864）；board
   `dram_size=0x400000000`、DTS memory@80000000 size 0x400000000 都是 16 GiB → 限制在 payload ADT 的
   owned 窗口。要用满 16 GiB=平台轮（改 owned_memory_size + UEFI 内存图/ADT 预留 + 核 stock FDT
   reserved-memory/TZ 区），先评估再动。
5. **容量题**：Data 卷=16 GiB（RAMDISK capacityMiB=16384/DATA pages=4194304）；About 的“总容量 6 GB/
   可用 15.53 GB”不一致，先查卷上报（系统卷 vs Data 卷）。要真加大 Data 卷=新 seed/overlay →
   **data-reset 红线**（谱系清零），不当场做。
6. **iPhone IPSW lane（用户已拍板 2026-11-16：跟上游同款）**：采用上游 `ios-27.0.1-24A446`
   profile 的 iPhone 产品（s122 账=**iPhone17,1**；开工前先核上游当前树 + 24A446 restore 的
   BuildManifest 产品集，17,1 与 17,4 是否同 restore/同 kernelcache 待确认）。列为**后期待办**，
   当前 GPU/帧拍/IRQ 收口后再开，不切换设备。
   - **注意（避免错误归因）**：现役墙（5 s swap 看门狗、GPU BV stall/0 超时 fence/scanout 无 offer、
     IRQ 风暴、4.75 GiB 内存 churn）**与 donor 身份无关**，换 iPhone lane 不会自动消墙；
     iPad 身份也不是“选错机型”——sheng/asteroids 都在 iPad17,3 上跑到桌面，asteroids 甚至是手机。
   - 参考 recipe：asteroids iPhone17,4/d48ap（唯一完整 pin/remap/policy/seed 记录，仅供 recipe 参照）；
     cupid iPhone17,2/D94AP（只到 BSD init、卡 root device=我们已通段）；peridot iPhone14,5（未完成）。
   - 代价：**Data seed 重建=谱系清零**（红线，需单独拍板）+ display/HID fixture 按 17,1 重做
     pin/remap（379/207 槽语义）+ 新 OS/SystemOS 窗口重签 root policy + trust cache/userdata 重建
     + **Metal plugin 的 QuartzCore 地址按 17,1 dyld cache UUID 查表**（硬编码 iPad 地址会写错全局）。

更早的 §0 指针（s126/s125 及 s122 以前）已迁入 `docs/archive/STATE-history.md`。
