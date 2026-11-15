# STATE — 现势状态（m2381，2026-11-16 更新）
## 0. 现势指针（每轮首更，2026-11-16 会话一百二十六收队 → 下一会话 s127 接续；更早指针=archive/STATE-history.md）

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
源码：设备仓 `ad274ad`；ArmCpu/GopFramebuffer 为**工作树改动**（pin 未动，diff 存 `out/m2381/r134-diffs/`，
子模块仓 origin=UniXNU 无推送权）。**r135 工作单=GPU 执行面**（卡尾）；关键更正：r132 的
"BV IB2 base 0xffff0000" 是 `peekFetch` 的"索引未知"编码 + 16×deadbeef=peek 读失败，不是坏基址。

**设备/件现态（交接时）**：用户已刷 **r133**（手机 UI 确认），`recovery_a`=r133 `07935d96`；
`current-slot`=b，`boot_b`=r129 `fc2e8f17`（未动），种子 `9c28654c` 零改动。r132 窗
（`out/m2381/reader-r132/`，sha `5dcdd522…`，header rounds=622 slots_written=622/503 **已回卷**，
uptime=1243s）仍是最近一份完整取证。**Setup 已走完、进入桌面**（About：iPadOS 27.0.1 / 可用 15.53 GB）。

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

**s126 档（历史，r132 已出并已判）**：

**★★★★★★★★★★s126（r131 接续，2026-11-16）——r131c/r131d 判读完成 + r132 已出件待刷**：

**判读（窗 sha `cee4af59…`，header rounds=460 slots_written=460/503 uptime=919s，屏 t=922s → 当轮非残影）**：
- **GPU 探针已活**（r131c 的 display-builder 链修复生效）：`GPU3 H278 R1 I0 F0 rd0/d0 f9 g1 s400 G0 q6 u0`、
  `[gpu ibs] submissions=1`（内核自测；ring 空 r0/d0）、`GPU5 M a0.ffffffff/0.ffffffff k0 s5/0 r1`、
  `GPU6 K0`、`[metal askers] backboardd:5 SpringBoard:6 Setup:5`。
- **addon 三挂载从未执行**（GPU 未加速的根因）：`A files=-1 wr=0 same=0 fail=0 ... mount=0/-1 mount2=0/-1
  mount3=0/-1`，`[gpu windows] mount fail stage 0`。根因=**arg 名退役失配**：内核构建树 `kern_exec.c:8576`
  gate 读 `sheng-gpu-addon`，r131c payload 的 args 行实测是 `gpu-addon=1`（件内 strings 实录）→
  `native_gpu_addon_prepare()` 直接 return。Metal 插件 `NativeAdrenoMetal.bundle` 不存在 → CA 无 GPU
  后端（FB total=153/922s，swap 每 ~5s 一帧，offer→done 7-37ms=驱动侧不慢，慢在客户端）。
- **CPU/UFS 清白**：SMP `req=1 run=7` 七核 step7；SCHED 30ms sleeps n=64 mean_late=880µs max=1101µs
  >50ms=0；UFS failures=0 integrity=0、1.34M req（~0.13ms/req）、verified 6.7GiB。不是 CPU/UFS 瓶颈。
- **Setup「看不到下一步」= 自家 live banner 盖住底部**：`BootProgress.c crash_banner()` 在 OS 接管后把
  不透明黑底诊断行画进 scanout fb（最多 48 行；1080x2400→scale 2 ≈ ≥1000px）；touch 正常（77 触点/
  944 HID），t≈790-801s 的右下角点击就是 Continue 位。开关=`sheng-banner=0`（只停 OS 接管后的屏上
  绘制，UFS 窗照记）。
- **崩族无新墙**：crashes=23/exits=824（922s）；DockFolder sig11 `0x211d6e3dc` − slide `0x23fc000` =
  `0x20f9723dc` = 在册 `DesktopServicesPriv TNode::GetNodeFromPath+0x2b4`；duetexpertd sig6 NSException 在册。

**r131d 判读（窗 `out/m2381/reader-r131d/`，sha `e357e6f4…`，header rounds=362/503 uptime=723s）——addon 门开了，GPU 仍卡 render-server**：
- addon 挂载已执行：`A files=6 wr=6 mount=3/2 mount2=1/2 mount3=1/2`（**stage 3=挂载成功**；status 2 只是后校验
  找密封面 `AGXMetalA10.bundle` 不存在，本 root 无该 bundle，属预期）；`M a3.2/1.2 k0 s5/0 r1 o22/0/19`、
  `K24 …`（插件已打开）。
- **客户端侧 GPU 已工作**：`[gpu clients]` cameracaptured/SpringBoard/Setup 均有 slot 与 4.1GB 映射；
  `[gpu ibs] submissions=37 ibs=112 max=18 over16=1`；`[gpu methods] 1:16 2:445 3:463 5:36 15:16`。
- **render server 仍无 Metal 设备**：backboardd 6 次 open 全关、peak 0M、0 映射；method 1/15 恰为 16 次 =
  16 次客户端 open（2+6+8），即 backboardd 的 Turnip 连接在 **selector probe** 阶段就失败；m2v 报
  `Vulkan(VkResultCode(-3))`。候选根因=**xnu grant 补丁只做字面名字比对**，驱动属性 `"*"` 从未生效 →
  沙箱过滤面拒掉 backboardd 的 selectors。
- **“卡”的根因（与 GPU 无关）**：r130 stage3 起 IRQ 率 2.5k/s→14.5k/s，swap 间隔 48ms→5005ms（显示族
  `swap_wait_gated` 的 5 s 看门狗）。engineInit 打开了 GENI SE 中断线（0xe1c），本板 SE4 无 FIFO 面、切
  GPIO 位bang 后无人清 → 12–15k/s 电平风暴；s1/s2（engineInit 未真跑）median 48–54ms，s3 起 5005ms。
- Setup 按钮：banner 已关（sheng-banner=0 生效），仍看不到 = 0.2 fps 画面冻结的推断，待 r132 后验证。
- 崩族：crashes=13；W0 SEGFAIL p450 = 在册 DockFolder（`0x211bd23dc`−slide=`0x20f9723dc`）；
  W1 `PUB no device: Vulkan(VkResultCode(-3))`。

**r132 已出件（内核两改 + payload，Mac 双端 sha 对拍）**：`out/m2381/payload-r132-irqfix-200f5279.img`
（99MiB，sha `200f5279…`；kernel 输入 `544f19a7`；payload kernel `325a222c`/LZMA `0f926828`；addon
`13398d63`/trust `ccff5d20`/seed `9c28654c` 全同）。改动：①`GoodixTouchProbe.cpp` 切 GPIO 后
`wr32(se,0xe1c,0)` + 清 `GENI_M_IRQ_CLEAR`；②`native_sandbox_grant` 支持 `"*"`
（`patches/xnu-12377/zzw-native-adreno-sandbox-grant.patch` + `patches/xnu/` 镜像）。
卡=`out/m2381/R132-CARD.md`，**待用户刷 recovery_a**；门=G1 IRQ ~2.5k/s + swap ~50ms / G2 触屏不回退 +
按钮可见 / G3 Query≥22、K 出 6802、backboardd 有映射、submissions 增长 / G4 T0+崩族不回归。
**运维债**：addon gate 名待下次 kernel 构建改读 `gpu-addon`（保留 sheng- fallback）；若 G3 只到 Query 增长
而设备仍 -3，下轮在 Turnip device-init（VMA/内存/feature）打点。

---
**s125 交接档（历史，r131c 判读已于 s126 完成）**：

**★★★★★★★★★★s125 收队交接（r131 GPU 轮，2026-11-16 会话一百二十五）——下一会话第一刀=判 r131c 的 GPU 结果**：

**设备现态（交接时）**：用户正在测 **r131c A**（`payload-r131c-gpu9addon-bd65b52a.img`，
`gpu-power=9 gpu-metal=1 gpu-addon=1`，保留 boot-progress=2）。`current-slot`=b，`boot_b`=r129
`fc2e8f17`（未动）；`recovery_a` 按用户动作=r131c A 或已换 reader `d01bed78`。**已判的上一次 =
r131b A**：窗 `out/m2381/reader-r131b/`（sha `ecfb05ab…`，743s）——**540s 进语言选择界面（解锁
成功）**，CPU 呈现 SWAP retired=103，崩族 DockFolder sig11 627 / backboardd sig6 620 /
duetexpertd 166 / chronod 116；GPU 未起（探针未被调用，见下）。

**下一会话开工序（照做）**：
1. **先看用户手里的 r131c 结果**：若已拉窗→直接判；若只有屏摄→按 stage 判；若还没拉→
   刷 reader `out/m2381/reader-esp-d01bed78.img` 后 `acm-reader.py probe|fetchchunk` +
   `read-m2381-ufs-log.py --all` 拉窗（窗入
   `out/m2381/reader-r131c/`，先核 header rounds/slots_written/uptime 防环残影）。
2. **GPU 判据**：`X4-X7`（探针 setRow 4-7）与页 `GPU3-6` 行应出现 `GPU m9 mapped=.. running`
   和 stage 1→9；`[gpu windows] mount fail stage`、`M a5.0 k0 s5/0 r1`、`[gpu clients]`/
   `[gpu ibs]` 非零、SWAP accepted 增长、崩族是否收敛；T0（run=7/adsp=13/battery=4/AKS/DATA=2/
   DISPLAY=10/touch HID）。
3. **args 陷阱（必读）**：设备仓 `board.toml` 仍是 r130 stage1 的默认值
   （`touch=1 offline-hid-input=0 touch-hid=0 gpu-power=1`），r131b/r131c 的 payload 全部靠
   `--kernel-arg` 覆盖。**出新 payload 必须带**：`touch=4 offline-hid-input=1
   offline-hid-home=1 touch-hid=1 gpu-power=9 gpu-metal=1 gpu-addon=1`（U 件再加
   `boot-progress=0 sheng-exc-name=none`），行长 A=755/U=713（<1000 DarwinBoot 上限）。
4. **若探针行仍空（GP0）**：先核构建副本——display builder 编 `ROOT/package/LegacyDisplayProbe.cpp`
   （**主树副本**，815 行），不是设备仓 package（716 行）。r131c 已在主树副本加 weak 调用，
   内核 `17d6c074` 反汇编实证 `ldr [got]; cbz; bl sheng_gpu_power_probe`（在 touch 之后）；
   再核 payload 的 args 尾（`gpu-power=9 …`，行长 755 < 1000）。
5. **若 stage 卡住**：按卡内表分级——`gpu-power=5`（只读 GMU 前置）/`6`（时钟树）/`7`（GMU 冷启）/
   `gpu-zap=0`（跳 ZAP）；`GPB` 行给失败步。若 A 件 2s/早期死，回滚 r130 `39533707`。
6. **若 GPU 起但底部仍刷屏挡 UI**：换 U 件 `payload-r131c-ui-2a007acb.img`（`boot-progress=0
   sheng-exc-name=none`），同内核同 addon。
7. GPU 判完再开 **iPhone 身份题**（下方 s122 立账⑥）：先做①ADT 串实验（cheap、单变量、可回退），
   不够再评估②整条 iPhone lane（Data seed 重置=谱系清零）。

**出件命令模板（Mac，改 args 即出新 stage；kernel 不变时只跑 payload）**：
```sh
cd ~/unixnu/ios-m2381-unixnu
./unixnu payload -D ../UniXNU-m2381 \
  --kernel out/m2381/ipados-27.0.1-24A446/m2381/kernel/kernel.macho \
  --data-ramdisk ~/unixnu/scratch-data.img \
  --owner-trust-cache ~/unixnu/owner-trust-24A446-gpu.bundle \
  --unique-chip-id 3645AD55D47E77C1 \
  --gpu-addon build/gpuaddon-r131.bin \
  --kernel-arg touch=4 --kernel-arg offline-hid-input=1 --kernel-arg offline-hid-home=1 \
  --kernel-arg touch-hid=1 --kernel-arg gpu-power=9 --kernel-arg gpu-metal=1 \
  --kernel-arg gpu-addon=1
```
改等级就换 `gpu-power=5|6|7|9`，加 `--kernel-arg gpu-zap=0` 跳 ZAP；出件后
`cp payload/payload.img out/m2381/payload-r131d-<名>-<sha8>.img` 并回传 Linux（rsync 单文件）。
**注意 kernel.macho 必须与 payload.json 的 kernel_sha256 一致**（17d6c074 是 r131c 内核；
重编内核后内核 sha 会变，先跑 kernel 再 payload）。

**r131c 件**（`out/m2381/`，双端 sha 对拍）：
- **A GPU** `payload-r131c-gpu9addon-bd65b52a.img`（sha `bd65b52a…`，99MiB；native kernel
  `17d6c074c4a1546bfec1ccb3330164f7926dcbfebb6baebbfd71a00025e2bb92`；addon `13398d63`；
  owner trust `ccff5d20`；seed `9c28654c`；ESP 内 `kernel.macho.gpuaddon` 12,843,120B 已核）
- **U UI** `payload-r131c-ui-2a007acb.img`（sha `2a007acb…`；同内核/addon，args 追加
  `boot-progress=0 sheng-exc-name=none`）
- 作废件：r131 `8bdb2265`/`19675c26`（2s 死：gScratch 越界写穿 gHost）、r131b
  `6a5ba18d`/`c9ff50c0`（探针未被调用：主树副本遮蔽）。回滚 r130 `39533707`。
- 卡=`out/m2381/R131-CARD.md`（含 r131b 判读、r131c 件、stage→修法表）。

**Mac 构建环境（下一会话直接可用）**：主树 `~/unixnu/ios-m2381-unixnu`（HEAD `4ff74a4`+工作树
同步；**Adreno740 gitlink 653e11c / AdrenoMetal gitlink c68f9a1 已在 Mac index `git add`，勿被
reset 打回**）；设备仓 `~/unixnu/UniXNU-m2381`。addon 工具链：brew meson/glslang/shaderc/
spirv-tools/bison3/rustup；Mesa pin `85fda3b3`（17 patch 已 apply，`.cache/mesa-work`）+
metal2vulkan `963f0e64` + `.cache/mesa-venv`(meson 1.12.1) + rustup `1.98.0`
（`/opt/homebrew/Cellar/rustup/1.29.3/bin` 进 PATH，crates.io 直连）。出件链
`~/unixnu/r131-kernel.sh`（XNU 五 env）；五件 payload 输入：`~/unixnu/scratch-data.img`
（9c28654c）、`~/unixnu/original-trust-24A446.bundle`（dc655bcb）、`~/unixnu/owner-trust-24A446-gpu.bundle`
（ccff5d20，含 addon 三 CDHash）、`build/gpuaddon-r131.bin`（13398d63）、`--unique-chip-id
3645AD55D47E77C1`。GitHub/googlesource
直连不通时用 Linux 代理 `192.168.10.185:7890`（时断，重试）；内核/payload 出件不需要代理。
**未推提交**：主树 `b2132c08..HEAD`（s124+s125 共 7 笔，HEAD 含本交接审计）；设备仓
`678a8f9`/`50eb0fc`/`2c4cd4d`。用户点头再推。

## 0. 现势指针（每轮首更，2026-11-16 会话一百二十四；更早指针=archive/STATE-history.md）

**★★★★★★★★★★s124（11-16）r130 触屏轮收工 = 触屏打通（HID 1794 reports / 手势被识别）；解锁卡在共享 abort 墙；下一步 r131 GPU+addon**：

**触屏闭环（四个 stage 逐级、每级一件）**：
- **stage 2（`0860f56e`）**：时钟/引脚全绿（`s4` CLK_OFF 消、`rcg=0`、`sda/scl=0x7`=func1+pull-up），引擎步止于
  `GENI_IF_DISABLE` —— **根因=该位被当致命门**（Linux i2c-qcom-geni 只拿它选 DMA，我们的 prepare 却 `return -1`）。
- **stage 3（`4e564c8f`）**：去假门后 `engineInit` 全过（`fw=00000303 ifd=1 cfg=03000021 clk=21 scl=00502c16`），
  但探针 `p=to`、`confirm=-2` → **FIFO 传输不执行**。
- **stage 3b（`fce06286`）**：换 **SDA/SCL 开漏 GPIO 位bang（≤100 kHz）**（照 `RigoLigoRLC/unixnu-nativegenii2c`
  的 sm-t733 同条件回退）→ **stage 3 主门过**：`pid=9916R addr=10308 axes=1080x2400 rail=ldob14/3200mV(1)`；
  同时 `TCHE st=41 cmd=0800ba04 tx=0 irq=0` **钉死 FIFO 禁位**（命令在位、FIFO 不吃字节）。
- **stage 4（`21482cd1`，args-only）**：帧线程跑起来（`[touch] strokes` 涨到 157）但 HID 未发布。
- **stage 4b（`39533707`）**：补 `offline-hid-home=1`（`HidInputProbe.cpp:220` 的门，我漏了）→
  `TOUCH HID started=1 touch-reports=1794 result=0 | h st=3 ld=0 rs=1 sz=208`、`[touch edge] installed=1
  contacts=17 from_edge=9`。**用户的慢速上滑被识别（出现旋转进度条）＝触屏轮 T1/T2 手势面前通过。**

**新墙（下一轮触屏侧的对手，非触屏 bug）**：解锁时 **backboardd sig6** 与 chronod/duetexpertd **同一
PC/LR**（s4b `PC2382591fc LR18087145c`；s4 `PC237fe91fc LR18060145c` = 同一站点、跨 boot 只差 slide
0x270000）→ 一个**共享框架 abort 点**被多个守护进程踩到；backboardd 崩→事件/渲染断→进度条回锁屏。
账：symbolize 该站点（s117 管线：去滑 + 24A446 dyld cache），另立"解锁墙"工单。

**其余**：T0 全绿（`run=7` / adsp st=13 / battery st=4 100% / AKS st=0 sg=3 / DATA st=2 / DISPLAY stage=10）；
`duetexpertd sig6` 与 `backboardd sig6` 入册（新增两族，同 abort 站点）。190s 进锁屏 = 七核正常速度
（早前 372s 那几轮应是 `run=0` 单核）。**排程不变**：r131 = GPU+addon（用户已拍板提前），按键顺延 r132。

**交接（会话一百二十四）**：件全在 `out/m2381/`（stage2/3/3b/4/4b 五件 + 各自卡）；设备现态 = 已刷
`39533707`（stage 4b），`boot_b` 仍是 r129 `fc2e8f17`（未动）。**未推提交**：设备仓 `GeniI2c.c/h`、
`GoodixTouchProbe.cpp`、`BootProgress.c`（Linux/Mac 双端同 sha），主树无改动。

## 0. 现势指针（每轮首更，2026-11-16 会话一百二十三；更早指针=archive/STATE-history.md）

**★★★★★★★★★★s123（11-16）r130 触屏轮 stage 1 已上机 + r131 排程改 GPU+addon（用户拍板）**：
- **r129 窗面哨兵（r130 第一刀）全绿**：reader-r129（sha 564ae4c4…，1681s）ADSP st=13 / battery st=4 /
  AKS st=0 sg=3 / DATA st=2 / crashes=2 全在册；唯一账 = **SMP run=0**（同内核同 args 在 r127/r128 窗
  是 run=7 → 头号假设=**LZMA 启动路径改变解压后内核落位/物理地址面**，钉法=r128b plain 双件 A/B）。
- **r130 stage 1（payload fd95aed5 / kernel 6c7bac76）已刷已判**：锁屏 6:12，T0 不回退；页行
  `TCH1 gcc=40000200 c2x=00004000 core=00004000 mahb=00000000 sahb=80000002 s4=80004000 rcg=00100000`
  = QUP wrap1 未投票 + S4 CLK_OFF（stage 2 靶面）。**首个 2s 死已定谳并修**：探针把 TLMM 映射建在
  gpio44 页、访问又加一次 `gx_tlmm_ctl(44)`（4 处同类），改相对偏移后过。
- **同款驱动侦察（用户要求，不闭门造车）**：pong（UniXNU-pong，SM8475/iPadOS18）`patches/xnu/
  …-pong-touch-console.patch` = 完整 899 行 XNU Goodix Berlin-D 驱动（GENI SE4+TLMM+**RPMh 供电**+
  GCC/RCG+复位+帧解析），另有 goodix-resume 与 physical-keys 补丁；ruby（GT9916R，iOS16）实机已上滑解锁；
  nura 主线 goodix_berlin I2C 为总线形态参照。协议面逐项一致 → 已按 pong 补两缺口：**>2 指点区单次重读
  （含尾部校验）** 与 **IC info misc+48/+50/+52/+54（ABI 8/8、真实坐标轴）**，另加 **RPMh 供电兜底**
  （复用 QcomRpmh.c 的 cmd-db/RSC 实现，ldob14/ldob14/ldob9 候选，TCS 2）。
- **r130 待办**：stage 2（touch=2，clocks/pins/engine）→ stage 3（touch=3，复位+识别+rails 兜底）→
  stage 4（touch=4 + offline-hid-input=1 + touch-hid=1）到 T2 上滑解锁。件由主树出、**刷机一律用户执行**。

**交接 s123 → s124（2026-11-16，用户开新会话）**：
- **设备现态**：slot=b；boot_b = r129 `fc2e8f17`（LZMA）在位；**recovery_a = reader ESP `d01bed78`**；
  最近跑过的 = stage 1 `fd95aed5`（已判）。
- **开工序（照做）**：①读本 §0 + `out/m2381/R130-STAGE2-CARD.md`；②用户刷 stage 2
  `payload-r130-touch2-0860f56e.img`（同一内核 7df9eb07，只差 `touch=2`）→ 锁屏后 ≥8 min → 硬断电 →
  刷 reader ESP → 新会话拉窗判 `[touch 1p]/[touch 2]/[touch 3]`；③通过则 stage 3
  `payload-r130-touch3-04b7dde7.img`（`[touch 4]` 身份/坐标轴/rail）；④通过则出 stage 4 件
  （`touch=4` + `offline-hid-input=1` + `touch-hid=1`，同内核）冲 T2 上滑解锁；⑤T2 后转 **r131 GPU+addon**
  （工作单已在下节，r132 才是按键）。
- **判读工具**：`UniXNU-m2381/scripts/acm-reader.py probe|fetchchunk` + `read-m2381-ufs-log.py --all`；
  窗入 `out/m2381/reader-r130-sN/`。2s 级死只认屏摄。
- **未推提交（两仓各 3 笔，新会话说一声再推）**：主树 `70051daf`/`5abd9712`/`6a5cad7e`；设备仓
  `b59de0d`/`0add857`/`eca0533`。两仓工作树均干净。
- **已知账**：SMP `run=0`（r129/r130 LZMA 启动；头号假设=解压后内核落位影响 PSCI CPU_ON，A/B 钉法=
  r128b plain 双件）；chronod sig6 / DockFolderViewServic sig11 两族在册；BootLog 主树陈旧副本坑已清
  （`package/BootLog.c` 不要再出现在主树 package/）。

## 0. 现势指针（每轮首更，2026-11-15 会话一百二十二；更早指针=archive/STATE-history.md）

**★★★★★★★★★★s122 终档：r129 容量轮已刷已判（用户面三门过，2026-11-15）——G0 ✓ 进锁屏（LZMA 解压路径机上工作=本轮主命题结）、G1 ✓ 充电正常（r128b 里程碑保持）、G3 ✓ boot 时长与 r128 无可感差异（解码 ≤秒级）。**判读债（下会话开工单第一刀）**：窗面哨兵（ADSP st=13/battery st=4/AKS st=0 sg=3/SMP req=1/DATA st=2）不拉窗，r130 触屏轮开工前先核 reader-r129 脸（FD 换代后的单变量卫生；不核就叠新变量=回退难归因）。原出件账：侦察三件全清——①上游 UniXNU/DarwinBoot/UniXNUPkg 全链**无现成压缩形态**（tip 5243c68 零 lzma；s118 挂账预案 A 落地，落点=主树 drivers/DarwinBoot（loader 源头）+ uefi 树仅复用其 Mu_Basecore SDK，uefi 仓零文件改动），复用公共域 LzmaDec SDK（vendored 六文件零改字）；②recovery_a 分区实测定谳 **=100 MiB**（FLASHING §3），size_mib=99 贴墙（卷内余 5.4 MiB / kernel 91.46 MiB）；③修法拍板=**成员名不变、内容 LZMA-alone**（FORMAT_ALONE preset 9e dict 16MiB 补写已知 size；91→17.5 MiB=5.22x，解码 host 0.7s/机上估 3-10s），DarwinBoot `ExpandKernel` 检测 props 0x5D+已知 size 后 `LzmaDecode`（dest 当字典，64KB scratch bump 分配器）再 ParseKernel；plain 内核照旧接受=回退只换 payload。**内核零改动**（member 解压=在机 r128 内核 6b4642eb 逐字节同）+ 种子/trust/vroot 原样回收（9c28654c/dc655bcb/5c976276）+ **args/TOML 与 r128 逐字同**；`--kernel-probed` 新开关复用已补内核（probe/seal 拒收二次执行）。host 门全绿：test-lzma-kernel 6 checks（生产代码抽取编译验证：round-trip/corrupt/oversize/unknown-size/passthrough）+ 上游 parser/ramdisk 测试 PASS + 固件 -Werror 零告警 + check-m2381 ✓。**件**：boot_b ← `out/m2381/boot-b-fc2e8f17-r129.img`（31,903,744B，新 FD 1da8600f + **固件段逐字节同 r128b**：adsp a9b3fd8a/dtb ee426dfc，V8 PAS metadata 未动）；recovery_a ← `out/m2381/payload-r129-lzma-676e2e5d.img`（99MiB，**卷余 79.3 MiB = GPU/audio/addon 预算解锁**）。卡 = `out/m2381/R129-CARD.md`（G0 到锁屏=主门 + UEFI 屏打 `kernel lzma 18482928 -> 95928320 bytes`；G1 ADSP st=13 + battery st=4 不回退；G2 回归哨；G3 容量+解码耗时；stall 脸先裸重启=s121 协议；红线=绝不 data-reset）。已知良性差异：卷上无 crypto-hmac.json（host 侧 seal 产物零设备消费）、ADT sha 代代异（随机种子）。**刷后编排**：r130 触屏（Goodix GT9916；开工第一刀=核 r129 窗面哨兵债）→ addon+WiFi 同轮 → GPU 大会话 → 音频（已解锁，ADSP 起）。立账（本轮顺手账已收两件）：①**DockFolderViewServic sig11 定名 = DesktopServicesPriv `TNode::GetNodeFromPath`+0x2b4**（s117 sym.py 管线，去滑 0x20f9723dc；Dock 文件夹枚举路径解析崩，2s 重生环，fresh-Data 谱系用户态症状，修法 RE 开放）；②**NativeUfs stall 侦察毕（只读）**：run4/5 脸机制钉死 = `verifiedVolumeQuery`（NativeVirtioBlock.cpp:250，expectedUUID 对拍）取到 IOMedia UUID **F914C62A…**（native_verified_block_root_uuid@:281=发布面 UUID）≠ 信封签名 volume_uuid（`NativeUfsBlock::authenticatesVolume` NativeUfs.cpp:1791、strcmp:1807 → 0）→ root 不发布 / UFS requests 冻 5 / manifest digest=0；SHNGROOT（RootLayout.h:159）=版面 magic 非尾窗内容；修面 = UFS 窗 IOMedia UUID 派生源 vs verified-root-24A446 信封一致性（RE 轮再开）。③长会话中断预算看门狗（658/625us@machine_routines_common）备忘保持；**⑤设备身份/UI 形态题（用户提出，2026-11-15 侦察定谳）**：现役 ADT 身份实测 = iPad17,3 / J820 / j820ap / chip 33090（profile `[ipsw]product`+`[adt.root.string]`），24A446 iPad IPSW 仅含 iPad17,1–4；锁屏示"按 home 键进入"而本机无 home。上游两派实录：①**iPad 身份派**——`asteroids`=Nothing Phone (3a) 用**同一份 24A446 iPad17,3 镜像**实录"to the home screen, with touch, buttons"（同 donor 手机先例=平板 idiom 非宿命）；`alioth` 17.7.11 亦接受 ~ipad idiom（代价：壁纸只认 ~ipad 集合/device-class 改造）。②**iPhone 身份派**——`cupid`=小米12 自建 `ios-27.0.1-24A446`（iPhone17,2/D94AP，不同 IPSW/kext/ADT identity + XnuPrelink fixture 重定向），理由原文"An iPhone profile is required for a phone: iPadOS presents iPad product identity, idiom and display assumptions that SpringBoard and UIKit read from /product"；`apollo`=iPhone10,3+iOS16.5 同款老线。**定谳**：身份=OS profile 级四件套（IPSW+kext+fixture+ADT），非 ADT 字串旋钮（上游零"只改串"案例）；本机 ADT **无 buttons 节点**（实测无 buttons/home/faceid 串）；上游按键走驱动面（sheng ButtonProbe + sheng_buttons_press 经 HID sink，非 ADT 节点）→ 解锁缺的是**输入通路**（触摸 r130 / 按键驱动），非身份；"按 home 键"提示的 idiom 判据待 RE（capability 位 vs 节点缺失未分）。**r130 预置**：A 补 buttons/按键映射（最轻，与触屏同轮）；B ADT 身份探针（单变量可回退，仅诊断）；C 另立 iPhone 24A446 车道（=换道量级，需 iPhone IPSW+fixture 重定向）。

**⑥目标设备对齐台账（用户拍板 2026-11-15：功能集对齐 asteroids，iPhone 身份单列评估）**。

| # | asteroids(Nothing Phone 3a) 已具备 | 我方现状 | 归属轮 / 缺口动作 |
|---|---|---|---|
| 1 | 进 home screen（真实桌面） | 锁屏点亮，未解锁 | **r130 触屏（上滑）** = 主门 T2 |
| 2 | 触屏（含多点/释放） | 无 | r130 |
| 3 | 按键（power/vol → HID，ButtonProbe+sheng_buttons_press） | 无（ADT 无 buttons 节点；驱动面缺） | **r132 按键轮**（GPU 提前，顺延） |
| 4 | GPU addon（Turnip + Metal；本机 Adreno 740/SM8550 同 sheng） | 仅 GOP 软件呈现（r129 前身） | **r131 GPU+addon 轮**（用户拍板提前；上游整拿） |
| 5 | 系统 App / 图标 | donor 卷内已有，未达桌面 | 随 1 解锁自然暴露 |
| 6 | 壁纸（device-class 改造 `@3x~ipad`） | PosterBoard 在册未见绘出 | 随 4 之后（GPU）或独立小轮 |
| 7 | Wi-Fi（WCN6750 ath11k，含 DHCP/上网） | 无 | addon+WiFi 同轮 |
| 8 | 蓝牙（2026-11-13 起） | 无 | Wi-Fi 后轮 |
| — | 音频（我方已有 ADSP st=13 底子；asteroids 未标 Audio） | ADSP 活，未接音频栈 | 音频轮（ADSP 已解锁） |

**判据**：每轮完成即以该行为准，逐行打勾；不合并轮次（单变量纪律）。

**iPhone 身份评估门（决策项，非施工项）**：
- **触发**：r130 触屏解锁成功之后（输入通路解决后，残留的"平板 idiom"才算真问题）。
- **评估输入**（四类可量判据）：①锁屏/桌面最终布局与 20:9 面板的匹配（缩放、Safe Area、圆角/缺口假设）②系统手势（上滑返回/多任务/分屏）是否符合 20:9 手机形态 ③壁纸 device-class 与 PosterBoard 集合（现仅 ~ipad）④App 呈现（iPad 专属 UI 在手机屏上的排布）。全部四项可接受 → **no-go（留在 iPad 车道）**；任一为实质阻碍 → **go**。
- **go 的代价四件（已侦察）**：iPhone 24A446 profile（24A446 iPhone 支**仅 iPhone17,1**；iPhone 11=iPhone12,1 不在集）、kernelcache prelink fixture 重定向（13 vnodeop_desc 按 vnop_* + 8 metaclass 槽 + 5 符号）、trust cache+userdata 重建、**Data seed 重建=谱系清零（唯一贵项）**。
- **产出**：`docs/STATE` 一条 go/no-go 决定记录 + 若 go 则立独立换道轮工作单（等价 21H461→24A446 量级）。
**参照与理由**：同 SoC 集成参照 = **sheng**（SM8550，触屏/GPU/WiFi/BT/音频/电池/键盘全 device-verified）；手机形态目标 = **asteroids**（唯一与我们同 donor 的手机实证）；iPhone 线唯一先例 = **cupid**（仅 BSD init，卡在 root device=我们已通段）。已清墙（AMFI/sandbox/dyld/launchd/root volume）均 donor 无关、换道可复用。几何理由（非审美）：面板 2400×1080≈20:9 vs iPad17,3 假设 4:3。排期：r130 触屏 → r131 按键 → addon+WiFi → GPU → 音频 → 其后 iPhone 身份评估门。

**④ADSP 固件迁移预案（addon 轮工作单素材，s122 拍板）**：正家=Data 活卷 /private/var/firmware（上游 AdspFirmware.c 本为 VFS loader，我 fork 源序 boot_b→/embedded→modem→VFS 的 VFS 源已在链上；交付走 addon 机制=Wi-Fi/蓝牙/VPU 同形态，种子零碰）；顺序=双源过渡（boot_b 先命中零变）→验证 [adsp] 从活卷吃→撤 package-m2381-boot.py 固件段（boot_b 缩回纯 boot image，固件升级变 recovery_a 单件轮）；**权衡=VFS 源等 Data 挂载，battery st=4 从 t≈22s 后移**（上游同款 "VFS loading waits for addon installation"）——时差不可接受则 boot_b 降级长期早源、活卷当更新源共存（四源序天然支持），退役是选项非必须；ESP 路（r129 后 79.3MiB 装得下 30.6MB）=备选需 DarwinBoot 加 member 管线，非首选。chronod -308 残余、dasd/gamed 根修 RE（禁表生效，紧迫性低）。

### 下一轮工作单 r130（触屏轮，2026-11-15 s122 交接；开工即执行，不含未决策项）

**目标**：Goodix GT9916 触屏 → 锁屏上滑解锁进桌面（= 身份/UI 题的 A 路线，asteroids 先例：同 donor 手机已带触屏进桌面）。

**第一刀 = 判读债（先于一切构建）**：拉窗 reader-r129 核窗面哨兵 —— `[adsp] st=13 err=0`、`[battery] st=4`、AKS st=0 sg=3、SMP req=1、DATA st=2、无新崩族；全绿才在 r129 FD 上叠触屏变量（FD 换代后的单变量卫生）。窗口产出 `reader-r129/`（log.bin sha 双端 MATCH）。

**第二刀 = 硬件侦察（只读，四个具体产出）**：
1. dtbo 提取：`/dev/block/by-name/dtbo` 或与 nura 对照（nura `BOOT-CHAIN-RE.md`:86 实录 `ro.boot.dtbo_idx=9` = **goodix brl-d 触控 + BOE AMOLED cmd-mode DSC 面板**）；解 overlay 用 dtc 反编译。
2. 触控节点五元数：**总线/地址**（记忆提示 i2c4@0x5d，须验证）、**IRQ GPIO**（提示 TLMM25）、**reset GPIO**（提示 TLMM24）、**供电轨**（提示 l14b+l12b）、**面板几何**（提示 1080×2400，横竖/原点方向要读 dts）。
3. 固件来源：GT9916 固件在 dtbo/分区/内置？主线 `goodix_berlin_core.c`(nura 实证) 的 fw 路径与校验。产出=设备仓 `records/` 一条 + probes 脚本。
4. 上游面读齐：`docs/porting/input.md` 的接口表（QcomGeniSpi / ShengNovatekTouch NvtProtocol / TouchBoard.h / TouchProbe.cpp / NativeHidInput.cpp / ButtonProbe.cpp）。**关键判定**：本机总线是 I2C 还是 SPI —— 决定是「复用 Novatek 协议核心、只写 Goodix 识别/固件/报帧模块」还是「照 TouchBoard.h 写板级路径 + 自写 Goodix 协议」。

**第三刀 = 组件接线（依侦察结论单变量）**：`[components]` 加入触控传输（GENI I2C 或 SPI 组件，上游有则用）、ShengHidInput（OS 报告面）；板级文件按 TouchBoard.h 布局（电源/reset/传输回调/中断/坐标），arm64e 侧按 sheng 记录处理 IOHID 事件驱动 vtable 重签（220 判别子/64KB 表法，见 sheng 记录）。

**args**：`touch=1→4` 分阶段（资源/时钟引脚/芯片识别/固件+扫描），逐阶段上，不一次全开。

**门（预注册）**：
- **T0** 到锁屏（回归不退化：ADSP st=13 / battery st=4 / AKS / SMP / DATA）。
- **T1** `touch=3` 读到芯片 ID 与几何（矩阵行列数、坐标范围）。
- **T2（主门）** 固件下载成功 + 首帧有效 → `touch=4` 扫描出点 → **锁屏上滑解锁进桌面**（屏摄为准）。
- **T3** 回归哨 + 崩谱对照（新崩族记档）。

**红线**：种子零改动、绝不 data-reset；只碰 boot_b + recovery_a；stall 脸先裸重启再判；每轮单变量（触屏轮不夹 GPU/WiFi）。

**败则**：总线/引脚不可得 → 回 dtbo 与 nura DTS 对拍；芯片无响应 → 电源时序（l14b/l12b 电压与 enable 序）+ reset 时序；固件下载失败 → 保留 stage/CRC/SRAM 读回诊断（上游 advise）再重试；解锁仍提示 home → 转身份/按键题（B/C 路线，见立账⑤）。

### 下一轮工作单 r131（GPU+addon 轮，2026-11-16 用户拍板提前；原 r131 按键顺延 r132）

**排程决定（用户 2026-11-16）**：GPU 与 addon **提前**到触屏之后、按键之前——理由=我们与上游 sheng
**同款 SM8550**，sheng 的 GPU 栈已设备验证（Adreno 740 + Metal + addon），复用面最大。r130 触屏
stage 2/3/4 仍在本轮收尾；按键轮顺延为 r132（ButtonProbe + `native_buttons_set_sink` 真实现替换现空桩）。

**目标**：Metal/Turnip GPU addon + 原生 Adreno740 内核驱动，在 24A446 车道上出首帧 GPU 合成
（壁纸/桌面由 GPU 画，代 GOP 软件呈现）。

**已备资产（侦察结论，无需发明）**：
- 组件：`Adreno740`（pin 6fa7bae2，hardware=Adreno 740/SM8550，**verified_devices=["sheng"]**；依赖
  Sm8550Platform/AdrenoPM4/AdrenoFirmware/ArmSmmu4K）；`drivers/AdrenoMetal` 已在仓。
- kits：`kits/AdrenoPM4`、`kits/AdrenoFirmware`、`kits/ArmSmmu4K` **已在本仓**（GPU 侧前置大半就位）。
- 上游参数面：sheng 记录 = `gpu-power=9` / `gpu-metal=1` / `gpu-addon=1`；s123 立账原文「上游整拿」。
- 容量：r129 后 recovery_a 卷余 **79.3 MiB**，gpuaddon（NGPUADD2）装得下。
- 显示面：DISPLAY stage=10 / SWAP accepted=1（direct scanout 路径已在），GPU 首帧判据可用 SWAP/FB 计数对照。

**第一刀 = host 侦察（只读，产出缺口清单）**：读 sheng 的 GPU area 记录 + `docs/porting/gpu.md`，逐项对
M2381 的 DT/kit 输入：
1. GPUCC/GMU/ZAP 固件（vendor 收割件是否齐；`proprietary-files.txt` 清单）；
2. `ShengGmu*Source.h` 里的 **OPP/带宽表**（上游不放进 git 的板级输入，需从本机 DT/主频表推导）；
3. apps SMMU 的 GPU stream id 与 stage-1 窗口（ArmSmmu4K 现成，ShengUfs 有 display bank 先例）；
4. reserved memory / GMU pool（Adreno740 与 AdrenoMetal 的 ABI 记录是否同 kernelcache 38e6d9fa）。

**第二刀 = 内核侧（单变量）**：Adreno740 编译进核（`CONFIG_XNU_NATIVE_ADRENO=y`）+ AdrenoFirmware/PM4
kit，`gpu-power=1..9` 分阶段（资源/ZAP/GMU/CP ring/power perf），**先不带 addon**；门=内核 probe 行
（CP ring/NMEM/GMU HFI）全绿且 T0 回归不退化。

**第三刀 = addon**：`scripts/make-gpu-addon.py`（UniXNU）打 AdrenoMetal/Turnip 用户态 bundle →
`./unixnu payload --gpu-addon kernel.macho.gpuaddon`；args `gpu-metal=1 gpu-addon=1`；门=addon 挂载 +
Metal device 创建（用户态日志）+ 首帧 GPU 合成（SWAP accepted 增长、FB copy 退出）。

**门（预注册）**：G0 内核 GPU probe（gmu/cp/smmu 全绿，T0 不回退）；G1 addon 挂载 + Metal 设备创建；
G2 首帧 GPU 合成（屏摄 + 窗口计数）；G3 ≥8 min 稳定 + 崩谱对照。

**红线**：payload 新增成员只有 `kernel.macho.gpuaddon`；seed/trust/vroot 零改动；只碰 boot_b+recovery_a；
每轮单变量（GPU 轮不夹 WiFi/音频）；stall 脸先裸重启。

**参考**：sheng（同 SoC、24A446、GPU 全栈设备验证）为主；asteroids（手机形态 + GPU addon）为形态参照；
cupid 仅作 iPhone 身份线参考（r132+ 身份评估门用）。

### r131 一并办的三件事（用户 2026-11-16 拍板"当然可以顺手做，一次解决三个"）

三件事共用一次出件/一次刷机窗口，但**各自独立判据**（互不掩盖，任一件失败不阻塞另两件）：

| # | 事 | 判据 | 说明 |
|---|---|---|---|
| ① | **共享 abort 站点 symbolize**（解锁墙） | 拿到该站点的模块+符号名 | 一个站点同时打崩 **chronod / duetexpertd / backboardd** 三个守护进程（s4b 三行同 `PC2382591fc LR18087145c`；s4 同站点 `PC237fe91fc LR18060145c`，跨 boot 只差 slide 0x270000）→ 改一处可望"一次解决三个" |
| ② | **r131 GPU+addon**（工作单见上节） | G0/G1/G2/G3 | 与①并行：G0 是内核 probe，不吃 UI；G1/G2 若被①挡住则如实记"判据待①" |
| ③ | 位bang 提速评估（顺手） | 每帧成本 | 仅评估不改码；触屏已通，提速是后话 |

**①的具体做法（下一会话第一刀，只读；工具与料已在本机，无需重建）**：

**已核实的字段语义（读生产者，非推测）**：`UniXNU-m2381/package/BootFaultProbe.c:90` 的格式串
`… F%llx +%llx R%u …` 里 `+` 那一项印的是 `f.slide = current_task()->task_shared_region_slide`
——**所以 `+330000` 就是共享区 slide**，不是别的位移。于是本窗：
- PC → **VA = 0x2382591fc − 0x330000 = 0x2382261fc**
- LR → **VA = 0x18087145c − 0x330000 = 0x18054145c**

**已跑出的两条结果（s124 收队前，只读）**：
- `0x18054145c` → `/usr/lib/system/libsystem_pthread.dylib +0x745c = _pthread_mach_thread_np.cold.2 + 0x1f0`
- `0x2382261fc` → **“no image”**（`scratch-r112/scripts/symbolicate.py` 只用主缓存 `imagesText`（MAIN+0x88）
  建镜像表，覆盖不到 0x238… 这一段）→ 下一刀必须换取镜像表的路子（`parse_cache.py` / 子缓存
  `.dylddata` 的映射表），或先确认该 VA 落在哪个子缓存文件里再定镜像。

**⚠ 本轮踩过的坑（硬律，已写进记忆）**：不要 `Path.read_bytes()` 整份 1.2GB `.symbols`，也不要
对百万级候选 slide 做 Python 循环；`S.find(...)` 回 -1 时 `S[p:e]` 会退化成 `S[p:-1]` 的整段拷贝。
本条已把 30GB 机器打到 zram swap 7.3G/8G 满、ZCode 被杀。正确姿势=**mmap 只读 + 全程有界切片 +
find 结果先判 -1**，且**先读生产者代码确认字段语义再算 VA**。

**②反查步骤**：`/home/ri/work/scratch-r112/`（24A446 缓存抽取件，7.3GB，已在位：子缓存 85 个在
`extract/System/Library/Caches/com.apple.dyld/`，符号 1.2GB 在根）+ `scripts/re-tools/*`。
命中镜像后看符号与反汇编前若干条；`sig6`=SIGABRT，优先找 `abort/__assert_rtn/raise` 类站点。

**③判定**：三条 daemon 的 `bt` 字段差在哪一层——若都停在同一框架的同一断言，修面 = 该断言的前置条件
（一处修三族）。

**交接（会话一百二十四 → 一百二十五）**：
- **设备现态**：已刷 stage 4b `39533707`；`boot_b` = r129 `fc2e8f17`（未动）；`recovery_a` = reader ESP
  `d01bed78`（拉完 s4b 窗）。**触屏可用**（慢速上滑会出解锁进度条，然后回锁屏）。
- **件**（`ios-m2381-unixnu/out/m2381/`）：stage2 `0860f56e` / stage3 `4e564c8f` / stage3b `fce06286` /
  stage4 `21482cd1` / stage4b `39533707`；卡 = `R130-STAGE{2,3,3B,4,4B}-CARD.md`。
  窗 = `reader-r130-{s1..s4b}/`（每份 `log.bin`+`parsed-all.txt`）。
- **未推提交**：设备仓 `GeniI2c.c`、`GeniI2c.h`、`GoodixTouchProbe.cpp`、`BootProgress.c`（Linux/Mac 双端
  sha 一致，Mac 上已参与构建）；主树仅文档改动（`docs/STATE.md`）。**推之前说一声**。
- **本会话已修的路牌**：`AGENTS.md` 的构建命令序（旗标在子命令后）+ 明确"出件在 Mac、Linux 只改码/判读"。
- **开局序**：①读本 §0 与本节；②`git status` 三仓，确认未推清单；③r131 第一刀 = host 侦察（工作单）
  + ①的 symbolize 只读侦察（可同轮并行）。

（长期挂账与硬律见 auto-memory 索引；轮史与判读全文=archive/STATE-history.md）
