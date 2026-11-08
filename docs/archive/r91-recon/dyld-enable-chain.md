# dyld map_with_linking_np 启用链定谳（工作单 B 刀 / r91 静态反汇编）

材料：`/tmp/r89/cache/dyld` = iPadOS 27.0.1-24A446 的 dyld 本体。arm64e Mach-O，
`__TEXT` vmaddr=0（文件偏移=虚拟地址，全文地址即 vmaddr），symtab 完好（4691 条
nlist），全部结论来自 capstone 5.0.7 对符号地址的直读反汇编 + 常量/xref 交叉验证。
分析只读，未改任何代码。

关键符号锚点（nlist 实测）：

| 地址 | 符号 |
|---|---|
| 0x3be0 | `dyld4::start(KernelArgs*, …)` |
| 0x1fd80 | `ProcessConfig::Process::Process(KernelArgs*, SyscallDelegate&, lsl::Allocator&)` |
| 0x202d8 | `ProcessConfig::ProcessConfig(…)`（Process 在 config+0x10，Security 在 +0x150，DyldCache 在 +0x1b0，PathOverrides 在 +0x290） |
| 0x21b6c | `ProcessConfig::DyldCache::DyldCache(…)` |
| 0xfe30 | `dyld4::prepare(APIs&, UnsafeHeader const*)` |
| 0x121f4 / 0x12240 / 0x12948 / 0x362c4 | `Loader::applyFixups` / `applyFixupsCheckCachePatching` / `PrebuiltLoader::applyFixups` / `JustInTimeLoader::applyFixups` |
| 0x14900 | `Loader::applyFixupsGeneric(…)` |
| 0x14d38 | `Loader::setUpPageInLinking(Diagnostics&, RuntimeState&, ulong, ulonglong, Array<void const*> const&)` |
| 0x155ac | `dyld4::(anon)::setUpPageInLinkingRegions(RuntimeState&, Loader const*, ulong slide, ushort, ushort, bool, Array<PageInLinkingRange> const&, Array<void const*> const&)` |
| 0x15984 | `__map_with_linking_np`（syscall 550 trampoline） |
| 0x159b0 | `dyld_map_with_linking_np(mwl_region const*, uint, mwl_info_hdr const*, uint)`（in-process fallback） |
| 0x15ba8 / 0x7a118 / 0x7a208 | `fixupPageAuth64` / `fixupPage64` / `fixupPage32` |
| 0x2f940 | `APIs::runAllInitializersForMain()` |
| 0x30bfc | `SyscallDelegate::disablePageInLinking()` |
| 0xb7c8 | `SyscallDelegate::sandboxBlockedPageInLinking() const` |
| 0x8583c | `dyld3::mapSplitCachePrivate(SharedCacheOptions const*, SharedCacheLoadInfo*)`（第二个 MWL 消费点） |
| 0xc4340 | `SyscallDelegate` 单例（`dyld4::start` 的 lambda @0x1fc68 以 `adrp x2,0xc4000+0x340` 传入 ProcessConfig/Process 构造） |

重要更正（相对工作单原文）：`tbz w8,#0 → 0x150e0` 不在 `setUpPageInLinkingRegions`
本体，而在其唯一调用者 `Loader::setUpPageInLinking`（0x14d38）内；0x150e0 是
`JustInTimeLoader::path` 尾块。两处测的是**同一个字段**（Loader+4 的 bit0），语义见 §1。

---

## 问题 1：Loader+4 bit0 门——从哪加载、谁置位

### 1.1 两处 bit0 判别的指令证据

调用者 `Loader::setUpPageInLinking`（工作单所指的"开头 tbz → 0x150e0"）：

```
0x14d6c: ldr   w8, [x0]              ; x0 = this (Loader*)
0x14d70: mov   w9, #0x7964
0x14d74: movk  w9, #0x6c34, lsl #16  ; w9 = 0x6c347964 = ASCII "dy4l"（LE 字节 64 79 34 6C）
0x14d78: cmp   w8, w9                ; Loader+0 必须是 "dy4l" magic，否则 abort（cold @0x53168）
0x14d7c: b.ne  0x15120
0x14d9c: ldrh  w8, [x0, #4]          ; ← w8 = Loader+4 的 u16
0x14da0: tbz   w8, #0, 0x150e0       ; bit0=0 → JustInTimeLoader::path@0x3f868
0x14da4: …     bl  PrebuiltLoader::path@0x12068   ; bit0=1
```

`setUpPageInLinkingRegions` 内同字段同语义的第二处（选 `mf()` 实现，工作单地址
有出入的即此）：

```
0x156c8: ldrh  w8, [x23, #4]         ; x23 = x1 = Loader const*
0x156d8: tbz   w8, #0, 0x15970       ; bit0=0 → JustInTimeLoader::mf@0x13e10
0x156dc: bl    PrebuiltLoader::mf@0x78d0          ; bit0=1
```

结论：**w8 来自 Loader 对象 +0x04 的 u16 标志半字，bit0 = isPrebuilt**。
（`Loader+0` 是 magic `"dy4l"`=0x6c347964，有独立校验函数 `Loader::hasMagic@0x341f4`；
两函数入口都先验 magic 再验 bit0，坏 magic 直接走 abort。）

### 1.2 bit0 由谁置位

`Loader::Loader(InitialOptions const&, bool, bool, bool, bool)` @0x38ecc：

```
0x38ed0: movk  w8, #0x6c34, lsl #16
0x38ed4: str   w8, [x0]              ; +0 = "dy4l"
0x38ed8: ldrh  w8, [x0, #4]
0x38edc: and   w8, w8, #0xfffffffe
0x38ee0: orr   w8, w8, w2            ; ← bit0 = 构造参数 w2
0x38ee4: strh  w8, [x0, #4]
0x38ee8…: bfi  w8, w9, #1..#12     ; bit1..bit12 依次来自 InitialOptions 字节 0..10
0x38f48: cmp   w5, #0 … orr … #0x200 ; bit9 = 构造参数 w5
```

全二进制只有两个 `bl LoaderC2@0x38ecc` 调用点：

```
0x2ec10: mov w2, #0   ; JustInTimeLoader::make@0x2e880 → JIT（磁盘）加载器，bit0=0
0x2ec20: bl  0x38ecc
0x7b1b0: mov w2, #1   ; PrebuiltLoader::PrebuiltLoader(Loader const&)@0x7b188，bit0=1
0x7b1c0: bl  0x38ecc
```

即：**bit0 在构造时定死**——JIT 加载器 0，PrebuiltLoader 1；launchd 这类常驻
shared cache 的进程，其 Loader 来自 cache 内预序列化的 PrebuiltLoaderSet，bit0
在 donor cache 生成时已写死为 1，运行期只是反序列化带出来。其它消费 bit0 的例子：
`RuntimeState::add@0xf890`（`bics wzr,#3,w8; b.ne` = 要求 bit0+bit1 全 1 才走
exclave-kit 相关分支）、`applyFixupsGeneric@0x14a30`（选 `PrebuiltLoader::
overridesDylibInCache` 与 JIT 分支）。bit14 也是同一半字里的一个 InitialOptions
位（= InitialOptions 字节 12），在 applyFixupsGeneric 里作反向门，见 §4.2。

---

## 问题 2：DYLD_PAGEIN_LINKING 解析与消费

### 2.1 解析点：ProcessConfig::Process 构造函数

串本体 `DYLD_PAGEIN_LINKING` @0xa6304（明文，__TEXT cstring）。唯一代码 xref
（adrp+add）= `ProcessC2+0x2a8`：

```
0x20008: mov  w8, #2
0x2000c: str  w8, [x19, #0x104]      ; Process+0x104 (u32) 默认 = 2
0x20010: mov  x0, x21 ; bl SyscallDelegate::internalInstall()   ; 读 commpage+0x4084 bit0 (@0x1f0bc)
0x20018: cbz  w0, 0x2008c            ; 非内部安装 → 跳过 env 解析
0x2001c: ldurb w8, [x19, #0xa0]      ; Process+0xa0 = 内核 comm page 标志（见下）
0x20020: tbnz w8, #7, 0x200a4        ; bit7 → 直接禁用
0x20024: ldr  x0, [x19, #0xc0]       ; KernelArgs 传进来的参数串矢量（env/apple）
0x20028: adrp x1, 0xa6000 ; add x1, #0x304   ; "DYLD_PAGEIN_LINKING"
0x20030: bl   __simple_getenv
0x20034: cbz  x0, 0x2008c            ; 未设置 → 走 0x2008c 收尾
0x2003c: …strlen == 1 否则无效…
0x20048: ldrb w8, [x22]              ; 取单字符
0x2004c…0x20088: '0'→0x200a4(w8=0) / '1'→w8=1 / '2'→0x201cc(w8=2) / '3'→w8=3 / 其它→无效
0x2008c: ldr  w8, [x19, #0x104]
0x20090: cmp  w8, #1
0x20094: b.le 0x200ac                ; mode<=1 不做沙盒复查
0x20098: bl   sandboxBlockedPageInLinking()      ; 见 2.3
0x200a0: tbz  w0, #0, 0x200ac
0x200a4: mov  w8, #0
0x200a8: str  w8, [x19, #0x104]      ; 沙盒挡 = 置 0
0x200ac: …
```

另有两条**按进程路径强制禁用**（0x200b8-0x2010c）：主路径前缀
`/usr/libexec/dia…`（diagnosticsextension）或 `/usr/sbin/fairplayd` → mode=0。

**缺省值 = 2**。mode 语义（由全部消费点反推，见 §4）：

| mode | 含义 |
|---|---|
| 0 | 完全禁用（不发 region、不 in-process） |
| 1 | 仅 in-process linking（跳过 syscall 550，直接走 `dyld_map_with_linking_np`） |
| 2 | **默认**：先试内核 syscall 550，失败回退 in-process；加载完成后（跑 initializers 前）向内核关闭 |
| 3 | 同 2，但**不在 initializers 前关闭**（持续保留，给内部装机用） |

### 2.2 config 键来源

`__simple_getenv` 的第一个参数是 `Process+0xc0`——由 Process 构造函数从
`KernelArgs*`（x1 入参）逐段拷出来的内核参数矢量（env 计数 `[ka+8]`、env 数组
`ka+0x10`、其后为 apple 串，0x1fdfc-0x1fe6c）。**launchd 的这些串由内核提供**——
即本 lane 里 DYLD_PAGEIN_LINKING 只能由 sheng/内核侧注入（boot 参数或内核填的
apple 矢量），用户态 shell env 对 launchd 无意义。另注意：env 解析整个被
`internalInstall()`（commpage `0xFFFFF0000+0x4084` bit0，@0x1f0bc）包住——
**非内部装机连 env 都不看，mode 恒为缺省 2（或沙盒置 0）**。

`Process+0xa0`（bit7 强制禁用源）= `dyldCommPageFlags()`（@0x2052c）读的
**comm page `0xFFFFF0000+0xC160` 的 u64，由内核填写**。bit7 是内核侧的总开关位。

### 2.3 sandboxBlockedPageInLinking 本体

```
0xb7dc: ldr  w19, [0xa0e84]         ; = 0x40000000（SANDBOX_CHECK_NO_REPORT 类常量）
0xb7e8: bl   getpid
0xb7ec: mov  w8, #0x226             ; 550
0xb7f0: str  x8, [sp]               ; vararg: 操作号 550
0xb7f4: adrp x1, 0xa5000+0xdc9      ; "syscall-unix"
0xb7fc: orr  w2, w19, #0xe
0xb800: bl   sandbox_check          ; sandbox_check(pid, "syscall-unix", 0x4000000e, 550)
0xb808: cset w0, gt                 ; 返回值>0 → 视为"被挡"
```

消费口径（Process ctor 0x2009c、DyldCache ctor 0x21c7c、setUpPageInLinking
0x14dec `eor w26,w0,#1`）一致：**返回 1 = 挡，置 mode=0 / 关沙盒放行位**。
真机上 launchd 无沙盒 → 期望返回 0。**本 lane 风险点**：若 sheng 内核的
sandbox_check 桩对任意查询返回 1（"允许"字面义），dyld 会把 mode 清 0、
MWL 静默关闭——r90 已实跑过说明现桩返回 ≤0，但这是 r91 加 [mwl-syscall]
打点后仍需盯的一环。

### 2.4 消费点汇总（谁读 Process+0x104）

Process+0x104 在 RuntimeState/Layout 里即 **ProcessConfig+0x114**
（RuntimeState+8 = ProcessConfig&，由 RuntimeStateC2@0x3c4d4 `stp x16, x1,[x0]` 实证；
applyFixupsGeneric@0x149bc、setUpPageInLinking@0x14dd8/0x15084 读的 `[x?,#0x114]`
全是它）。全二进制 `ldr w?, [x?,#0x114]` 只有 4 处：
0x149bc（applyFixupsGeneric 门，`cbz`→mode==0 跳过）、0x14dd8（mode>=2 才查沙盒）、
0x15084（`cmp #1; cset` 生成"仅 in-process"bool）、0x2f974（runAllInitializersForMain
`cmp #3`）。DyldCache ctor @0x21c6c（`cmp #2; b.lt` 决定缓存侧路径）。写只有
Process ctor 的 6 处（默认 2 + env 4 值 + 两条强制 0 + 沙盒 0）。

---

## 问题 3："dyld shared region dynamic config data was not set"

串 @0xa6688。唯一 xref = `DyldCacheC2.cold.1`（0x9c000 起），热路径在
`DyldCacheC2` 尾部：

```
0x21fd4: add  x0, x27, #0x1c         ; x27 = CacheFinder 结果对象（0x21d74）
0x21fd8: bl   FileIdTuple::operator bool()@0x18d28
0x21fdc: cbz  w0, 0x220cc
0x220cc: bl   0x9bff4   → cold.1:
0x9c000: adrp x0, 0xa6688            ; "dyld shared region dynamic config data was not set\n"
0x9c00c: bl   dyld4::halt@0x515d0    ; ← halt，进程直接终止
```

**这是致命路径，不是日志**。触发条件：CacheFinder 产出的 `FileIdTuple`
（系统级已映射 cache 的文件身份：路径+fileid）无效。该身份不是 dyld 内嵌默认
表、也不是环境变量——它来自**内核侧**：dyld 启动时 cache 已被内核映射进
shared region（正常 iOS 启动形态），dyld 通过 `SyscallDelegate::
hasExistingDyldCache@0x9ab8`（shared_region_check_np 一族）＋ KernelArgs 里的
dyld_file 串（cf. 字符串 `"dyld_file"@0xa5edc`、`"kern.dyld_flags"@0xa5dd6`）
拿到"内核到底映射了哪个 cache 文件"。所谓 "shared region dynamic config data"
就是**内核随 shared region 交给 dyld 的这组身份/配置数据**。

对本 lane 的含义：sheng 内核既然替 dyld 预映射了 16K donor cache，就必须把
cache 文件身份（dyld_file 语义）放进 KernelArgs / 内核 note，否则 dyld 在
ProcessConfig 构造期（任何 fixups 之前）就 halt。r90 实录 dyld 已走到
MWL 发 region，说明此关已过；r91 若见 launchd 死在这条串上，先查内核填的
dyld_file/apple 串。

---

## 问题 4：syscall 550 完整条件链与本 lane 落点

### 4.1 两个 MWL 消费点

`bl __map_with_linking_np@0x15984` 全二进制仅 2 处：

1. **per-loader**（r90 观测到的即此）：`setUpPageInLinkingRegions@0x158bc`，
   由 `Loader::setUpPageInLinking` 的 3 个调用点（0x15068/0x150a4/0x150d8）驱动；
2. **per-subcache**（私有映射 cache 时逐 subcache 建 region）：
   `dyld3::mapSplitCachePrivate@0x8615c`，verbose 时打
   `"Setting up kernel page-in linking for subcache %d\n"@0xac1ec` /
   `"__map_with_linking_np(subcache %d) failed, falling back to linking in-process\n"@0xac21f`。
   系统级共享 region 常态（内核已映射）不走它。

trampoline 本体（坐实 syscall 550）：

```
0x15984: mov x16, #0x226     ; 550
0x15988: svc  #0x80
0x1598c: b.lo 0x159ac (ret)  ; CF=0 成功
…失败 → cerror_nocancel
```

### 4.2 per-loader 链条（从进程启动到 svc #0x80）

```
dyld4::start(0x3be0)
 └─ lambda@0x1fc1c: ProcessConfig(0x202d4) → RuntimeState(0x3c4a8) → prepare(0xfe30)
     └─ prepare 内 fixup 循环（0x10acc bl Loader::applyFixups，含主可执行）
         └─ applyFixups → applyFixupsCheckCachePatching → applyFixupsGeneric(0x14900)
             [G1] Image::chainedFixups() 有效：0x14990 ldrb [sp+0x1e8]; cmp #1; b.ne → 整段跳过
             [G2] mode != 0：0x149b8 ldr x8,[rs+8](=config); 0x149bc ldr w8,[x8,#0x114]
                  (=Process+0x104); cbz → 跳过
             [G3] strings 数组规模：0x149c4 ldr x8,[x21,#0x10]; lsr #9; cmp #0x7c;
                  b.hi → 跳过（count > ~64.5K 不发）
             [G4] Loader+4 bit14 == 0：0x149d4 ldrh w8,[x20,#4]; tbnz #14 → 跳过
                  （bit14 = InitialOptions 字节12，Loader ctor bfi 链 @0x38f7c+；
                    缓存内序列化值，launchd donor loader 实测路径已过）
             └─ Loader::setUpPageInLinking(0x14d38)
                 [G5] magic "dy4l" 校验（0x14d6c-0x14d7c，坏则 abort）
                 [G6] open(path)：0x14dc0 bl SyscallDelegate::open（路径来自 bit0 选的
                      Prebuilt/JIT path()）；-1 → Diagnostics::error(fmt@0xa5bdf) 整段跳过
                 [G7] mode>=2 才查沙盒：0x14dd8 ldr [x8,#0x114]; cmp #2; b.lt 跳过；
                      0x14de4 ldrb [rs,#0x4f1] bit0 置位也跳过；sandboxBlocked → w26=0
                 [G8] 再次 chainedFixups==1（0x14e10-0x14e18）
                 [G9] Image::forEachSegment 回调 block_invoke@0x15138：
                      0x15138 ldrb w8,[x1,#0x2f]   ← x1 = SegmentInfo*
                      0x1513c tbnz w8,#1 → 收集
                        SegmentInfo+0x2f = LC_SEGMENT initprot 低字节
                        （parse_segment_command@0x6010-0x6014: ldr w9,[x9,#0x3c];
                          strb w9,[x19,#0x2f]）→ bit1 = VM_PROT_WRITE
                        ⇒ 只收集 RW 段；且 startsForSegment(idx)@0x15170 必须非空
                          （该段在 LC_CHAINED_FIXUPS 有 chained starts）
                      每段写 {page_size = starts+4 (u16), ptr_format = starts+6 (u16)}
                      进两个数组头（0x15188-0x151a8 strh [x8,#0x18]），并 push
                      PageInLinkingRange{vmaddr, size=pageCount*pageSize,…}
                 └─ setUpPageInLinkingRegions(0x155ac)（最多 3 次调用：call B 主路径
                      w5=(mode==1)；失败重试 call C w5=1；另一 range 组 call A w5=1）
                     │  栈上拼 mwl_info_hdr（0x15694-0x156b4）：
                     │    +0x00 u32 = 7（版本）
                     │    +0x04 u16 = page_size  （来自 starts+4；16K 段 = 0x4000）
                     │    +0x06 u16 = ptr_format （来自 starts+6；本 lane = 12 = 0xc）
                     │    +0x08 u32 = strings 区长; +0x0c u32 = strings 数
                     │    +0x10 u32 = 0x28（头长）; +0x14 u32 = 变区长
                     │    +0x18 u64 = slide;  +0x20 u64 = mf() = 加载地址
                     │  随后 ranges+strings 拷入同一 alloca（0x156e0-0x157cc）
                     ├─ w5(bool) bit0=1（mode==1 或重试）→ 直接 0x15904 →
                     │    bl dyld_map_with_linking_np（in-process，见 4.4）
                     └─ w5=0 → 0x158bc bl __map_with_linking_np(ranges, count, hdr, size)
                          = svc #0x80 / x16=0x226
                          w0==0 → 成功收工（0x158c0 cbz）
                          失败 → 若日志开（RuntimeState+0x181/0x182 位域）打
                          "__map_with_linking_np(%s) failed, falling back to linking
                           in-process\n"@0xaa48d → bl dyld_map_with_linking_np
```

### 4.3 加载完成后的关闭闸（runAllInitializersForMain）

```
0x2f964: ldr  x0, [x19, #8]          ; APIs+8 = ProcessConfig&
0x2f968: ldrb w8, [x0, #0x151]       ; config+0x151 = Security+1 = internalInstall
0x2f96c: cmp  w8, #1
0x2f974: ldr  w8, [x0, #0x114]       ; = Process+0x104 mode
0x2f978: cmp  w8, #3
0x2f97c: b.eq 0x2f984                ; internalInstall && mode==3 → 保留，不关
0x2f980: bl  disablePageInLinking    ; 否则关：
0x30bfc: mov x0, #-1 ; b 0x9b14      ; syscall 294（shared_region_check_np 桩）带 -1
```

即**默认（mode=2）下内核侧 page-in linking 只覆盖 dyld 加载期，跑 initializers
之前 dyld 主动通知内核关闭**。disable 的返回值被忽略（内核不实现 294-(-1) 也不
炸 dyld，只是内核侧保持开启——这是 sheng 侧的策略自由度）。

### 4.4 in-process fallback（dyld_map_with_linking_np@0x159b0）

同一份 region 自解析：校验 `+0==7`（0x159d4）、sizes（0x159fc/0x15a0c），按
`hdr+6`（ptr_format）分发（0x15abc-0x15b94）：

```
1        → fixupPageAuth64@0x15ba8, w4=0   （0x15b3c: mov w4,#0; b 0x15af8）
2        → fixupPage64@0x7a118,  w4=0      （0x15b20）
3        → fixupPage32@0x7a208,  w4=0      （0x15adc→0x15b30）
6        → fixupPage32@0x7a208,  w4=1      （0x15b94 cmp #6 → 0x15b9c）
9        → fixupPageAuth64@0x15ba8, w4=1   （0x15ae0→0x15af0）
0xc(12)  → fixupPageAuth64@0x15ba8, w4=1   （0x15ae8 命中落入 0x15af0）
其它(4/5/…) → 跳过该页继续（0x15afc），不是错误
```

（w4 疑为 isAuth 类旗标；region 结构坏则 cold.1 assert（0x15b68），或
fprintf 后返回 -1（0x15b6c，fmt@0xaa4d3）。）

**与 r90 内核实录对齐**：`fmt=0xc` 即 `mwl_info_hdr+6` 的 ptr_format=12
（iOS26 新 auth64 链格式），`hdr_page_size=0x4000` 即 `+4` 的 page_size——
donor cache 里 16K 页 launchd 的 loader 路径 region 头，两个数与本反汇编逐位吻合。
内核那次 "12 条 chain walk / 1 次 a_key=0 强签" 就是内核版 fixupPageAuth64 在跑。

### 4.5 本 lane（launchd，平台二进制，无 team id，4K 内核跑 16K donor cache）最可能卡点

r90 已实证：MWL region 真被内核收下并走链（fmt=12、page=0x4000）⇒ G1-G9 与
syscall 全通过，**卡点不在"是否调 550"，而在 r91 打点要区分的后续语义**。按链条
排序的核验清单（从高概率到低）：

1. **关闭闸时序（最值得关注）**：mode 缺省 2，`runAllInitializersForMain` 会向内核
   发 294(-1) 关闭。若 r91 打点显示 550 后紧跟 294，属 dyld 正常行为；若 lane 需要
   MWL 覆盖 launchd 自身初始化期，必须让 dyld 进 mode=3——唯一入口是
   internalInstall（commpage+0x4084 bit0）=1 **且**内核在参数串里注入
   `DYLD_PAGEIN_LINKING=3`（单字符；非 internal 装机 env 完全不解析）。
2. **commpage 标志位**：`0xFFFFF0000+0xC160` bit7（强制 mode=0）与 `+0x4084`
   bit0（internalInstall，连带 env 解析与 mode=3 通路）都是 sheng comm page 构建
   侧的位——内核侧一改位，dyld 行为静默翻转，且无日志。
3. **sandbox_check 桩返回值**：桩若返回 1（字面"允许"），dyld 判为 blocked →
   mode 清 0。现行为（MWL 已跑）说明桩返回 ≤0，但这是最脆的隐性开关。
4. **G3/G4 阈值类**：strings 数组 >~64.5K 条、或 rebuild/closure 出的 loader
   带了 bit14——都会静默跳过，无任何输出。launchd 走 donor 原生 loader 时
   实测没触发，但换自制主二进制/自制 closure 时要复查（G4 的 bit14 是
   InitialOptions 字节 12 的序列化值）。
5. **G6 open() 主可执行路径**：KernelArgs 给的 launchd 路径必须可 open——
   r91 打点若看到连 550 都没有且无 panic，优先怀疑 G1/G2/G6 早退。
6. 子缓存路径（mapSplitCachePrivate）在本 lane 不参与（内核已做 system-wide
   映射），其 verbose 串是区分两条路径的现成指纹：per-loader 失败串是
   `"__map_with_linking_np(%s) failed…"`，per-subcache 是
   `"__map_with_linking_np(subcache %d) failed…"`。

### 4.6 条件链总图

```
内核启动 launchd ── KernelArgs{参数串, dyld_file, commpage 标志}
   │
   ├─ [commpage 0xC160 bit7 == 0]  否→ mode=0 ────────────┐
   ├─ [路径非 diagnosticsextension/fairplayd]  否→ mode=0  │
   ├─ [internalInstall] 才解析 DYLD_PAGEIN_LINKING('0'..'3')
   ├─ [sandbox_check(op550) 返回≤0]  >0→ mode=0            │
   │                                                      ▼
   │                                            Process+0x104 = mode (默认2)
   │                                                      │
   ├─ [cache 身份 FileIdTuple 有效] 否→ halt("…dynamic config data was not set")
   │
   └─ prepare → fixups（含主可执行）
        ├ G1 chainedFixups==1 ─┐
        ├ G2 mode != 0 ────────┤
        ├ G3 strings ≤ ~64.5K ─┼─任一不过→ 不发 region（纯常规 fixups）
        ├ G4 Loader+4 bit14==0─┘
        └ setUpPageInLinking
           ├ G5 magic "dy4l" ├ G6 open(path) ├ G7 沙盒 ┘
           └ G9 逐段: initprot 含 W(bit1) 且有 chained starts
                → region{ver=7, pageSize, fmt, slide, loadaddr, ranges, strings}
                     ├ mode==1 ─────────→ dyld_map_with_linking_np（in-process）
                     └ mode≥2 → syscall 550 ─ 成功→ 内核 page-in linking 生效
                                             └ 失败→ 上述 fallback
   … 所有镜像加载完 …
   runAllInitializersForMain:
        internalInstall==1 且 mode==3 → 保留
        否则 → syscall 294(-1) 通知内核关闭 page-in linking → 跑 initializers
```

---

## 附：证据强度与未定项

已定谳（指令级，可复核）：§1 字段/写者、§2 解析与四个消费点、§3 halt 路径、
§4.2 九道门与 region 头布局、§4.3 关闭闸、§4.4 fmt 分发表、trampoline = 550。

自认的近似（不结论、仅记录）：
- `setUpPageInLinking` 的 3 个 `setUpPageInLinkingRegions` 调用点按"两块 range
  组 + 失败重试"读出；块/数组在栈上的精确布局（sp+0x60..0xa0 一带的自引用
  数组对象）未逐字段展开，不影响链条判定。
- Loader+4 bit14 与 bit1 的业务名（InitialOptions 字节 12 / 字节 0 的语义）
  无符号可依，只给了构造位来源。
- `sandbox_check(pid, "syscall-unix", 0x4000000E, 550)` 的常量 0xE 分解
  （filter 组）未再深挖；消费语义（>0 = 挡）由三处调用点交叉确证。
- `disablePageInLinking` 复用 `___shared_region_check_np` 桩（syscall 294）
  发 -1，是 xnu 非公开新语义；本仓内核侧如何回应属 UniXNU 仓课题，此处只记
  dyld 侧行为。
