# r34 窗取证报告（设备窗读环 + Gate 1–6 判读）

> 环 = `rings/m2381-ring-r34.{bin,txt}`（`simg 7aaab96f` + boot `339b8480` 刷入后实跑窗；
> 182,970B / flush #246 / 3,298 行 / sampler 4 轮）。读环通道：nura-min devsh → dd userdata
> 尾部 1025×4K 块 → gzip+base64 → sweet-log.py。
> 前置 = `handoff/glm-report-r34.md`（出件 + Gate 表定义）。
> 判据树执行 = 报告 §4；判读日期 2026-10-22。

## 0. 一页判读

1. **双件身份铁证**：boot = r34 ✓（boot-args 行带 `sweet_amfi_debug=1`）；userdata = r34 ✓
   （`cs_invalid_page` 97→**0** = 撤补丁生效 + 崩溃点换位到 stock 路径 stub，见 §3）。
2. **假说 B 终审定谳：launchd 签名任务缓存整体遮蔽磁盘 LaunchDaemons 扫描**——Gate 1
   `exec r34mark` = **0**（全环连 "r34mark" 字符串都没有），而 r34 身份独立证明在
   （cs_invalid_page=0 与 pc 换位都是只有 r34 userdata 才有的行为）。r33「改 plist」与
   r34「加 plist」在缓存面前同样无效；**lsd plist 的 `FEATUREFLAGS_ENABLED` envvar 同被
   遮蔽未投递 ⇒ A1 两件 kill-switch 全部失效**（符合报告预测的失效模式：仍 false 不更坏）。
   A5 悬而未决的失败分支实测补全：`launchd_unsecure_cache=1`（无效路径）⇒ **静默落回默认
   缓存**，没有回退磁盘扫描。
3. **Gate 5 意外大捷：SIGABRT 连崩群清零**。accountsd/chronod/dmd/liveactivitiesd/
   ScreenTimeAgent/cloudd（r33b 合计 548 崩）本窗**各 exec 恰 1 次、零 CRASH、其后约
   2/3 窗再无重拉**（exec 位置在环 682–1135 行，窗尾 3298 行处仍活着）——容器骨架让
   容器机制整体工作起来（containermanagerd 亦 1 次 exec 无崩）。
4. **Gate 3 换守卫**：mad 仍 52/52 exec 全崩，但 pc = mh+0x1BF3BC = **stock 二进制
   main() 错误清理链上的另一个 fatal stub**（stub 传源码 line 321；旧补丁路径是 line 247
   stub @0x1BF5BC）。崩溃根因与容器骨架无关——骨架没救回 mad。
5. **Gate 4 未变**：lsd t2 与 r33b **16 帧逐帧全等**（slide 常数平移 0x1C42C000）=
   仍是 `LSHRNSupport deviceConfiguredForHRN → MadGate copyActivationRecord → xpc sync`
   永等。SB t0 已脱离 os_state 死锁（r32–33b 11 帧栈 0 命中）换为等 lsd seeding 的同步
   XPC 等待；bbd t0 = runloop 正常。**全链唯一堵点 = mad fatal。**
6. **窗长 ≈ 半窗**（~8.5–9min：suggestd 169 vs 前三窗 315–331；mad 节奏 6.1/min 与前三窗
   5.7–6.0/min 同拍）。census 已按每分钟归一，结论不受影响。

## 1. 身份判据（环境补充判据，报告 §4 尾）

| 判据 | r34 窗读数 | 判定 |
|---|---|---|
| boot-args `sweet_amfi_debug=1` | 在（boot-args 行实测） | boot = `339b8480` ✓ |
| `cs_invalid_page` 计数 | **0**（r32/33/33b = 102/99/97） | userdata = r34 ✓（stock CDHash） |
| 崩溃 pc 偏移 | mh+0x1BF3BC（旧 = 0x1BF5BC） | stock 代码路径 ✓ |
| slide 指纹 | mad/lsd/SB 全 0x1a958000 | 本窗一致 ✓ |

## 2. Gate 表判读（报告 §4 逐条）

| Gate | 读数 | 判定 |
|---|---|---|
| 1 `exec r34mark` | **0**（无任何 r34mark 痕迹） |userdata 身份另证在 ⇒ **缓存整体遮蔽实锤**（§0.2），非刷写作废 |
| 2 `SWEET AMFI: mobileactivationd: N` | **28 entitlements**（N≥20） | **PASS**——entitlements 挂载正常，AMFI 侧无问题（S1 不用转修） |
| 3 CRASH mad 计数与 pc | 208 行 = 52 exec × 4 行全崩；pc = 0x1BF3BC+slide（全部一致，52 把 slide 各异） | **换守卫**——骨架未救回；按新 fatal 走（§3） |
| 4 lsd t2 | 与 r33b 16 帧全等（MadGate 永等） | 主墙未开：daemon 仍死循环，t2 原地 |
| 5 SIGABRT 连崩群 | 548 → **0**（各 1 exec 零崩零重拉） | **容器根因面修复旁证成立**（最大幅度下降） |
| 6 AMFI r34mark 行 | 无（Gate 1 未发生） | 不适用 |

## 3. 新守卫定位（Gate 3 的"按新 fatal 走"）

- 两个崩溃点都是 `.re/ma-fatal.asm`（上窗反汇编）里的 **Swift trap stub 表**成员：
  每个 stub = `adr x0,"main"; adr x4,<fileinfo>; mov w1,#<源码行>; mov w2,#-1; bl 0x5100
  （fatal 报告函数）; …; brk #1`。
- 旧（补丁二进制，r29–r33b）：进入错误尾 @0x5061C，`bl@0x5062C → stub(line 247)`
  → brk@**0x1BF5BC**，bt0 = 0x50630。
- 新（stock 二进制，r34）：进入点更深，`bl@0x50670 → stub(line 321)` → brk@**0x1BF3BC**，
  bt0 = 0x50674。
- 上游：main() 里一个循环（0x4F8C4，迭代 x26/x20 计数）做两次取值型检查
  （w23/w24 布尔），`tbnz w23,#0 → 0x5061C` 直接跳错误尾；循环后还有
  `bl 0x101bff34 → cbnz w0 → 0x50630` 错误路径。0x505F0 处调用带 landing pad
  （`adr x8, 0x10050674` 作为错误返回地址被显式登记）。
- 判读：撤补丁后控制流走 stock 分支（tbz），撞的是 stock 路径自己的 fatal
  （line 321）。与 r33b 案同族（main 启动期错误 → fatal），**容器骨架未介入此路径**——
  fatal 的触发点在 mad 自己的启动记录/资源检查里，不是 systemgroup 目录查询。
  line 247/321 对应源文件未知（fileinfo 串是间接表 {0x15C,0x80200000}，未解出；
  a2s 内有 "activation_support.m" 字符串，存疑待查）。
- **结论：mad 的 fatal 不随容器骨架消失 ⇒ r35 别再从容器侧救 mad，直接上 stub。**

## 4. r35 建议（按 DeepSeek 判决树 + 本窗新事实排序）

1. **主攻 = 原地二进制 stub 化**（`/usr/libexec/mobileactivationd` → vm_stat 拷贝类
   已签名字节码，路径/服务名不动）：
   - 缓存照常按原定义拉起（路径不变）；stub 是 Apple 原签名（零 CDHash 问题——
     Gate 2 实证 stock 签名过 AMFI；`unentitled_ios_sim_launch=1` 在 boot-args 兜底）；
   - stub 起来即退、不注册 MachServices ⇒ lsd 的 sync XPC 拿到 **error reply 而非永等**
     ⇒ MadGate/lsd t2 走失败分支（快败 vs HRN 门，二一）；
   - 判据（r35 环）：`CRASH mobileactivation` = **0** + lsd t2 换栈 + mad exec 少量零崩。
     mad CRASH=0 本身就是 r35 镜像的体内身份。
2. **并行低成本实验 = 改签名缓存 + `sweet_amfi_debug=1` 放行**：r34 boot 第一次带上了
   `sweet_amfi_debug=1`。若我方 AMFI hook 覆盖 launchd_cache_loader 的
   "cdhash in trustcache" 查询（r32–33b 已实证 hook 放行补丁代码页 = 覆盖代码签名主校验），
   则改 `/System/Library/xpc/launchd.plist`（删/改 mad 条目 + 加 r35mark）+ 去签上机一把
   即知。**若成 = B 机制根治 + 磁盘权威恢复**（此后所有 plist 手术全部复活）。
   前置 RE（下窗宿主侧 15 分钟）：我方内核 amfi hook 是否在 `SecStaticCodeCheckValidity`
   userclient 路径上。
3. XPC 全功能 stub（A6 方法面/应答键已备）留作 1 的失败后备；MadGate 解码侧一刀
   反汇编（MobileActivation.framework 不在 DSC，从根盘刻）到 1 失败再补。
4. `launchd_unsecure_cache` 三态（删/无效路径/显式路径）已无实验价值——本窗实证无效
   路径 = 静默默认缓存，不会回退磁盘扫描。

## 5. 环境与过程备忘

- 读环：devsh telnet 是文本通道，4.2MB 尾部块走 `dd | gzip | base64 -w 100`（设备无
  base64 之外的编码器实测有；fallback = od -tx1）。gzip 后 ~490KB base64，`--wait 90` 富余。
- 宿主 `grep` 被 zsh 包成 ugrep 且 `--ignore-files` 会跳过 .gitignore 的 `rings/`——
  环分析一律用 `/usr/bin/grep -a`（txt 里有少量二进制字节需 -a）。
- 窗长归一：mad 6.1 exec/min（前三窗 5.7–6.0）；suggestd 169（前三窗 315–331）。
- 冲击波时序：SB exec @环 682 行、containermanagerd @778、backboardd @779、lsd @911、
  chronod @904、accountsd @991、cloudd @1135；mad 崩溃循环贯穿全窗至 3280/3298 行。
- census 全表：xpcproxy 636 / suggestd 169 / fairplayd.H2 53 / bluetoothd 53 /
  mobileactivationd 52 / coresymbolicationd 52（后四家与 mad 同拍 exec、零崩）/
  wave 六家各 1 / SB·bbd·lsd·containermanagerd 各 1；CRASH：mad 208 + profiled 4。
- AKS 582 行（sweetscan 活跃）；`sel 35` 多家 → 0x0（内核健康对照 ✓）。
