# STATE — 现势状态（m2381，2026-11-17 s130 更新）
## 0. 现势指针（2026-11-17 s130 收队 → 下会话第一刀 = r143 prelink 布局墙专轮；r141b 判读/rebase 已闭环；更早指针=archive/STATE-history.md）

**★★★★★★★★★★r143 出件卡墙收队（2026-11-17 s130 深夜）：native __DATA 超 donor 洞 32KB**。rebase 后
kernel 编译已过（patch 全落+链接过），prelink 布局校验拒：`Native __DATA overlaps a retained segment`
（fileset_layout.py validate）。**账目**：native __DATA placement `0xfffffff00b834000-0xfffffff00ba24000`
= 0x1f0000（2MB）；洞 = donor __DATA 头洞到 `com.apple.kec.Libm` 220B data 段（0xfffffff00ba1c000 钉尾）
= **0x1e8000 硬顶**（s105 账）；超 **0x8000 = 32KB**。撞的 retained = 0xba27700 起的 5KB 块（首跑，
删环前 0x1f4000）→ 本轮已删 wifi 死环 85KB（native_wifi_text[128][112]+note[64][112]+BootLog 读侧，
写侧自 r88 冻结就不存在；设备仓 5268fd4 后续提交）→ 只缩到 0x1f0000（**-16KB**，环大半在 bss/zerofill
路径、placement 只吃到 16KB）→ 仍超 32KB。

**矛盾未解（下会话刀口，按序）**：**刀口 0 = r141 成功态铁证对账**——`out/m2381/payload-r141-padrestore-f5aefc7f.img`
（Linux 侧在案）里的 kernel.macho fileset 可直接解析 native __DATA 的真实尺寸/位置（r141 出件成功的
合法布局）——拿它与 r143 的 0x1f0000 对差，64KB 增量一测便知；r143 的组件 tree 实为 r141 组合（tree 步对 .git 悬空的组件 skip——Adreno740
a7d2ded/ArmCpu 64042533/GopFramebuffer 7bcb6d9/RamBlock 4be2d7e/ShengUfs 31c3d395，与 r137 kernel 编译
态一致）、patch 集 = r137 同款（series md5 7dd5e6fba5 双端对拍 ✓，上游 210 条只动 patches/mesa 用户态），
且删了 85KB——native __DATA 理应 ≤ r141 成功态，实测反超顶 32KB。**嫌疑排序**：①tree 步 skip 的组件
残留态与 r141 出件态有隐藏差异（4be2d7e/31c3d395 是 r104"rsync freeze"提交——**先验证 r141 出件时 tree
组件 HEAD 是否同款**，对 R140/R141-CARD 或 payload.json 谱系）②prelink pairing 判定变化（map drift
813→642 两跑不同——lineage map 已被 os.replace 覆盖，**r141 lineage map 已丢，下次跑前先备份
kernel.map.json**）③diagram：fileset_layout 的 describe()/validate diag 打印已在（设备仓 24cd397），
跑一轮即得全段账。**快速候选刀**：再缩死重凑 32KB——wifi 车道残余（native_wifi_pmic[472]/dump[108]/
stats[29]/nw_wifi_stats/nw_wifi_where + BootLog [wifi-*] 行，~3KB，写侧同样不存在）+ 各仪器环瘦身
（boot_ui_trace_rows[14][120]=6.7KB 等）——**缩前先定位 64KB 增量，否则仪器面白伤**。

**除障已闭环（本轮付费清单）**：①prepare-xnu-source.py 网络挂死（上游新脚本拉 Apple 头，Mac api 慢
45s×3 重试）→ Linux 侧预热三份 json 缓存+头文件体推 tree/.cache/sources（TAG=xnu-10063.121.3）②diag
patch 重构两轮：插桩最终落 adoption patch 自有 + 区（weak_import extern 双 profile 安全），diag 恢复
原版 hunk+[32] 扩容+hunk2 重锚 ③ml_adopt 在 machine_routines.c 不在 arm_vm_init.c（extern 补）④
RamBlock/ShengUfs prepare 链接墙：上游 737f6da 的 ml_static_mfree_prepare 定义只挂在 macOS 27.3 memwired
patch（未实机验证）→ **两子模块 hold 回验证 pin**（RamBlock 60a54ce8/ShengUfs 99d8bcd3，catalog 双落）
⑤tree 残留清扫（tree 步对 .git 悬空组件 skip——rsync 双仓后 tree 内残留旧态，需手工 rsync 修正）。
主树推至 5bf7d2a9、设备仓 24cd397。


**★★★★★★★★★★r141b 工作脸窗判读定谳（2026-11-17 s130，窗 out/m2381/reader-r141b/ sha 4ebe17f4 双端对拍 ✓，
rounds=220/503 slots_written=220/503 uptime=438s write_errors=0 无回卷）**：
- **主门=非确定性挂起坐实**：同镜像 r141（f5aefc7f）黑屏窗死脸（fence 118 out=1 挂+合成器 t=172 后沉默）
  vs 本工作窗 **[gpu ring] seq=667 done=667 out=0 rewind=0 全 drain + 合成器 W1@119s→W31@449s 持续出帧**——
  挂起类病时好时坏，代码无罪（判读定律"同镜像重跑翻脸=先怀疑非确定性"命中）。[gpu ring] **首个工作基线入档**
  （subs=69、fence seq=667）。[gpu wptr] writes=667 dropped=0 stuck=0、[gpu ibs] 659/1771、4 客户端持设备。
- **残留未变**：[fence wait] timeouts=378/450（等待检测滞后脸——completed=666 贴住 submitted=667=020f460 的
  monotone 起效，超时次数仍高待新窗定位分布）；**5.0s 节拍器仍在**（W 间隔 4960-5106ms、offer/done 毫秒级）；
  SWAPQ 尾 copy=30/31 仍 100% CPU copy 但 **xfer=1 首现**；dpu 脸同 r140（dead0005、rel=400,400,400 活崩环境脸
  在案、go=0 fb=30）。
- **T0 全绿**：SMP run=7、UFS 84.8 万请求零 fail、REJ 黑屏脸 0 行（pad 身份 ✓）、EXIT 1363/nonzero 563。
- **结论**：主线回归卡顿收口。**r143=rebase 后首件**（上游 210 条渲染 checkpoint 波+fence face 仪器进 Mac），
  判据=GPU 面不回归+[gpu ring] 新基线+5s 节拍是否被上游 m2v/CA 改动影响。

**★★★★★★★★★★上游 210 条 rebase 全部落位（2026-11-17 s130）**：upstream/main `3375ac9d→387267a5`，
650 条工作区提交重放→638 落位（12 条 gitlink-only 空掉），**STATE/patches/AGENTS/FLASHING 与备份分支
`m2381-pre-sync-20261117` 零 diff=内容零损**。子模块重落（catalog 双落）：Adreno740 `1c9029bc`+诊断重放
（39b98f5 fence face+b084e0a ring face——**等待检测滞后的 observeBookkeeping 修法随 pin 进树**）、ArmCpu
stats 波 `acba4ee`+IRQ ring 重放（5327d7d）、GopFramebuffer `b0d812d`+swap depth 重放（be658c5）、
DarwinBoot 保留 4583617、RamBlock/SoftwareAES/AdrenoMetal（`79a2d579` checkpoint 波）跟上游。M1 diag 扩面
（outside-memory-diag.patch 五 hunk：init 入口 0xAA55/kept-killed 账本/table 计数/cap 0x400/adopt 入口
0xABAD+runs；BootLog [boot mem+] init= 行新增，设备仓 5268fd4）。**流程大坑入档：11-10 起的提交全在
detached HEAD 上（m2381 分支指针停在 11-10 前、origin 停在 s56），首次 rebase 错重放老链 232 条；
checkout -B 归位后二次 rebase 才对——rebase 前必核 HEAD 是真分支且 tip=最近提交。** UPSTREAM §1.1 全记。

**r143 出件工序**：Mac 侧 rsync 双仓（带完整排除清单 dry-run 核对）→ `./unixnu kernel`（重编：上游
runtime changes+诊断重放+M1 diag 扩面+BootLog 扩行）→ `./unixnu payload`（args 不动 47=47）→ dd 读回
对拍（**纪律恢复：r138 起历次刷写缺 dd 读回**）→ 用户刷 recovery_a。回滚锚=r141（f5aefc7f）。

**归档指针（s130）：s129 收队新况/上游侦察/r142 二分/r139/r138/r140 判读与 r136 及更早的 §0 段落已移
`archive/STATE-history.md`（s130 归档段）。r141b 判读与 rebase 全账在 s130 深夜块与 UPSTREAM §1.1。

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
