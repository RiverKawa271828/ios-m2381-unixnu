# r90 窗捕获 MWL chain/sign 离线对拍 —— donor 真值解码

生成: 2026-11-09 / r91-recon。全部结论由 python -I 结构化解析 donor 头得出，
脚本在 `/tmp/r91-recon/work/`（`parse_cache.py` / `coverage.py` /
`images_and_decode.py` / `final_analysis.py`，映射总表落盘
`/tmp/r91-recon/work/cache_map.json`），未在数据目录内执行任何解释器。

内核侧权威源（r90 基线，均在本仓 out 树内）：

- `ios-m2381-unixnu/out/m2381/ipados-27.0.1-24A446/m2381/tree/.cache/xnu-work/osfmk/mach/dyld_pager.h`
  —— `struct mwl_info_hdr`（MWL_INFO_VERS=7）：`mwli_version/page_size/pointer_format/
  binds_offset/binds_count/chains_offset/chains_size/slide/image_address`，后随 binds 表与
  `dyld_chained_starts_in_image`。**注意：结构体本身没有 "dy4l magic" 字段**（任务描述里
  的 magic 记法不成立于该结构；`dyld_v1  arm64e` magic 属于 dyld_shared_cache 文件头）。
- `.../osfmk/vm/vm_dyld_pager.c` —— `fixup_page()`（:854，segStart 查找与 sheng-mwl 打印
  :904）、`fixupPageAuth64()`（:618，sheng-chain 打印 :679）、`signPointer()`（:566，
  sheng-sign 打印 :597）、fmt 分发 switch（:963）。
- `.../EXTERNAL_HEADERS/mach-o/fixup-chains.h:103` —— **fmt=0xc 即
  `DYLD_CHAINED_PTR_ARM64E_USERLAND24`**：stride 8、unauth target 为 image offset、
  bind ordinal 24 位。不是 DYLD_CHAINED_PTR_64_OFFSET（=6），也不是 64 位无 auth 体系。

---

## 0. 结论速览

| # | 问题 | 判定 |
|---|---|---|
| 1 | userVA 0x100c26000/segStart 0x100c22000 属于谁 | **不属于 donor 共享缓存任何映射**（79 件 .hdr 全部映射 ∈ [0x180000000,0x2FD43C000)）。0x100c22000 在 4 GiB 段 0x1_0xxxxxxx，是**缓存外 PIE 独立二进制的 __DATA**；结合窗内死点在 launchd 进程，主嫌 = /sbin/launchd 本体（已核实 donor 缓存 4654 个镜像清单中**无** launchd） |
| 2 | chain fmt 0xc 解码 + sign 行验算 | 12 条全部解出且自洽；auth-bind 0xc0156ae100000000 字段 key=2/addrDiv=1/div=0x6ae1/ord=0 与 sheng-sign 行**逐位吻合**；disc blend、result 低 36 位 = unsignedAddr **两处精确闭合** |
| 3 | unsignedAddr 0x2cef08258 落点 | 运行时地址；减 r90 slide 0x714000 → **0x2ce7f4258 = region3 .75.dylddata 第二个 RW 映射（__AUTH 语义）首页 +0x258**，文件内偏移 **0x4794258**（dyld_shared_cache_arm64e.75.dylddata）。runtime 映射基反算 0x2ce7f4000+0x714000=0x2cef08000，unsignedAddr 恰为其 +0x258 —— **slide 用法对拍无误** |
| 4 | far 0x281E75CD070 与 result/chain 位形关系 | **无任何位形关系**（移位/掩码/截断/半字节/字节序全扫，无一命中；far 低 36 位 ≠ 0x2cef08258，故连"同一目标签错码"都不成立）。far 不是本页 fixup 的产物 |

**总判定（墙86 毒页来源分流）**：窗内可核对的每一步 fixup 写入（rebase 目标、auth-bind
签名、slide 加算）均与 donor 真值一致 —— **该页内核拼装本身无可指摘的错写证据**；
launchd 死点读到的 far 垃圾与本页 fixup 值无位形因果，毒源应另找（见 §5 尾注与 §6 未决点）。

---

## 1. Q1：0x100c22000 属于哪个子缓存/镜像？

### 1.1 donor 头解析

79 件 `dyld_shared_cache_arm64e.*.hdr` = 各 subcache 文件首页（`dyld_v1  arm64e`，
头长 0x238=568、版本 1，UUID@0x58）；`main` = 主缓存头（768 KiB，基址字段
`0x180000000` @0xe0，与任务给的 cache 基一致）。映射三元组（u64 addr/size/fileoff
+ u32 maxprot/initprot，stride 0x20）自 0x238 起。全量扫描得 **102 条映射**，
无一例外落在 [0x180000000, 0x2FD43C000)，布局成三个 region：

```
R1: main + .01-.24 (__TEXT r-x) + .25.dylddata (data 组) + .26.dyldreadonly
R2: .27-.53        + .54.dylddata              + .55.dyldreadonly
R3: .56-.74        + .75.dylddata              + .76.dyldreadonly + .77.dyldlinkedit
     (尾部另有 symbols/atlas 单页文件)
```

`.dylddata` 文件承载该 region 镜像的 data 段组（顺序：r-- __DATA_CONST → rw __DATA
→ rw __AUTH → r-- __AUTH_CONST …），`.dyldreadonly` 承载只读 const 组。

### 1.2 覆盖判定

```
probe segStart      0x100c22000 -> NO MAPPING
probe userVA        0x100c26000 -> NO MAPPING
probe auth 槽       0x100c26090 -> NO MAPPING
```

任务提示里的"main cache __DATA 或 4K lane 映射位"假设**被头解析否定**：缓存里根本没有
覆盖 0x1_0xxxxxxx 段的映射。0x100c22000 位于经典 arm64 PIE 装载窗（0x1_00000000 起），
fmt12 是现代独立二进制的标准格式，seg_count=2（__DATA + __DATA_CONST 带 fixup）也符合
小体积独立可执行体的形态。配合死点在 launchd 进程，且 donor 缓存镜像清单
（`main` @0x2a8 起，4654 条 `dyld_cache_image_info`，路径内联）中**没有 launchd**：

```
images: 4654  first: 0x180404000 /usr/lib/libobjc.A.dylib
              last:  0x2c8000000 /usr/lib/objc/libobjcMsgSend33.dylib
min image addr = 0x180404000 ; "launchd" 字符串 0 命中
```

→ 判定：被 fixup 页属于**某个缓存外独立二进制的 __DATA 第 2 页**（segStart=镜像加载基
+segment_offset，pageIndex=1 × 16K）。窗内上下文（launchd 进程 + fmt12 现代格式）下
主嫌是 /sbin/launchd 本体；msgbuf 不带 pid，若窗里混了其他进程的 fault，也可能是别的
out-of-cache 二进制 —— 不影响本对拍的算术结论。

顺带锚定死点坐标：libobjc.A.dylib unslid 0x180404000 → runtime 0x180B18000
(+0x714000)；`libobjc+0x17140` = **0x180B2F140**。

---

## 2. Q2：chain 逐条解码（fmt 12 USERLAND24）

位域（与内核 fixupPageAuth64 一致，auth bind 用 auth_bind24 布局）：

```
auth  = bit63 ; bind = bit62
非auth rebase: target = v & 0x7FFFFFFFFFF (43b), high8 = (v>>43)&0xFF,
               next = (v>>51)&0x7FF（步长 = next*8）
auth 条目:     target32 = v & 0xFFFFFFFF ; diversity = (v>>32)&0xFFFF ;
               addrDiv = bit48 ; key = (v>>49)&3
bind 条目:     ordinal = v & 0xFFFFFF (24b, USERLAND24)
```

解码表（off 为页内偏移；`写入预测`列给出内核 fixup 后该槽应变成的值，
image_base 即 mwli_image_address —— 对独立二进制为含 slide 的运行时装载基）：

| off | raw | auth/bind/next | 解码 | 内核写入预测 |
|-----|-----|-----------------|------|--------------|
| 0x000 | 0x020000000063260 | 0/0/4 | REBASE tgt=0x63260 high8=0 | image_base+0x63260 |
| 0x020 | 同上 | 0/0/4 | 同上（3 槽指同一目标） | 同上 |
| 0x040 | 同上 | 0/0/4 | 同上 | 同上 |
| 0x060 | 0x020000000063c00 | 0/0/4 | REBASE tgt=0x63c00 | image_base+0x63c00 |
| 0x080 | 0x010000000062a60 | 0/0/2 | REBASE tgt=0x62a60 | image_base+0x62a60 |
| 0x090 | 0xc0156ae100000000 | 1/1/2 | **AUTH-BIND ord=0 key=2 addrDiv=1 div=0x6ae1** | PAC(0x2cef08258) |
| 0x0a0 | 0x080090000000af14 | 0/0/256 | REBASE tgt=0xaf14 **high8=0x12** | image_base+0x120000000000af14 |
| 0x0a8 | 0x018000000080070 | 0/0/3 | REBASE tgt=0x80070 | image_base+0x80070 |
| 0x0c0 | 0x020000000065152 | 0/0/4 | REBASE tgt=0x65152 | image_base+0x65152 |
| 0x0e0 | 0x020000000065831 | 0/0/4 | REBASE tgt=0x65831 | image_base+0x65831 |
| 0x100 | 0x020000000065981 | 0/0/4 | REBASE tgt=0x65981 | image_base+0x65981 |
| 0x120 | 0x010000000065831 | 0/0/2 | REBASE tgt=0x65831 | image_base+0x65831 |

**walk 自洽性**：off[k+1] = off[k] + next[k]×8 对 0x0→…→0x90→0xa0 全部成立
（4→32B 四连、2→16B 两连）。**0xa0 的 next=256 ⇒ 下一跳 0xa0+2048=0xc28，而窗内第 8
行是 0xa8** —— 0xa8→0xc0(next=3, +24)→0xe0→0x100→0x120 自成一条 walk。说明这 12 行
不是单一链：sheng-chain 计数器是**全 boot 上限 12 条**（`static sheng_fixup64_logged`），
行 1-7 = walk1（0xc28 起的后续被计数器吃掉），行 8-12 = walk2（另一页/另一 fault，
链起点 0xa8）。这反而是**解码正确性的旁证**：两条 walk 内步长全自洽。

要点：
- 0xa0 的 high8=0x12 非 0 —— arm64e 指针高位携带 auth-variant 位，合法形态，写入预测
  顶字节 0x12，不是错值。
- rebase 目标 0x62a60–0x65981（≈400–416 KB）+ 0x80070，全部落在镜像自身范围内 → 与
  独立二进制的自引用（函数/数据指针）吻合；0x65152 为奇数目标，指向字节对象/串数据，
  亦非异常。

---

## 3. sign 行验算（auth-bind @0x90）

```
slot uVA    = userVA + off = 0x100c26000 + 0x90 = 0x100c26090
bind ordinal= 0x000000 → bindsArray[0]
bindsArray[0] (dyld 预算好的运行时目标) = 0x2cef08258

字段位核（raw 0xc0156ae100000000）:
  bit63=1(bit c0) auth  ; bit62=1 bind
  key      = (v>>49)&3 = 0b10 = 2   (=ASDA, 数据键 —— log: key=2 ✓)
  addrDiv  = bit48           = 1    (log: addrDiv=1 ✓)
  diversity= (v>>32)&0xFFFF  = 0x6ae1 (log ✓)
  next     = (v>>51)&0x7FF   = 2 → 下一跳 +16 (→0xa0 ✓)

disc blend: __builtin_ptrauth_blend_discriminator(0x100c26090, 0x6ae1)
          = 0x100c26090 ^ (0x6ae1 << 48) = 0x6ae1000100c26090   (log 逐字 ✓)

result 0x46cf82cef08258 结构核:
  result & 0xFFFFFFFFF = 0x2cef08258 = unsignedAddr        （低 36 位保真 ✓）
  result >> 36         = 0x46cf8                            （PAC 码落在 VA 窗之上，
                                                              VA 宽 36 位的典型形态）
  a_key = 0（pager->dyld_a_key，r90 实配值）
```

三条全中（字段位、blend、结果结构），且 PAC 具体码值依赖硬件 ptrauth 算法，
离线不重算、也无须重算 —— 结构闭合即"内核按 donor 指定的 key/div/addrDiv
对 dyld 给的 unsignedAddr 做了正确的签名"。

---

## 4. Q3：unsignedAddr 0x2cef08258 的落点与合理性

### 4.1 算式（每步可复核）

```
unslid = 0x2cef08258 - slide(0x714000) = 0x2ce7f4258
       ∈ .75.dylddata 映射 @0x2b8: unslid 0x2ce7f4000-0x2cf818000 (rw, ip=3), foff 0x4794000
页内偏移 = 0x2ce7f4258 - 0x2ce7f4000 = 0x258
文件内偏移 = 0x4794000 + 0x258 = 0x4794258   (dyld_shared_cache_arm64e.75.dylddata)

反算核: runtime 映射基 = 0x2ce7f4000 + 0x714000 = 0x2cef08000
        unsignedAddr = 0x2cef08000 + 0x258 = 0x2cef08258   ← 分毫不差
```

slide 反算严丝合缝，等于同时验证了 r90 官方 slide 字段 0x714000 与 donor 头映射
二者自洽 —— **内核 targetAdjust/slide 用法没有错**。

### 4.2 属主与语义

`.75.dylddata` = **region3**（text = .56-.74，首镜像 GameStoreKit@0x280404000，
共 1820 个镜像）的 data 段组文件。region3 data 组映射序列：

```
@0x258 r-- 29.3MB  (__DATA_CONST, fixup 后转 r--)
@0x278 rw  44.5MB  (__DATA)
@0x298 r-- 1 页    (组间对齐/隔离页)
@0x2b8 rw  16.8MB  (__AUTH)  ← 命中这里
@0x2d8 r-- 57.5MB  (__AUTH_CONST)
@0x2f8/@0x318 r--  (尾组)
```

落点 0x2ce7f4258 = **__AUTH 组首页 +0x258**，即 region3 数据布局中排最前的那个镜像的
__AUTH 段开头附近（按构建序即 GameStoreKit 一带）。ASDA（key=2）auth-bind 指向
__AUTH 区的数据符号（objc 类引用/CF 风格 auth 指针这类）—— **类型学上完全合理**；
0x258 偏移落在该组第一个含 __AUTH 段的镜像内。

诚实的边界：`.75` 正文不在本地（本地只有 sc01/sc25/sc54 三个正文），无法读出 0x258
处具体符号；sc54 同结构区抽验显示正文未加密（sc01 查得 arm64 prologue，熵 4.0–6.8，
非 8.0）但 2ndRW 页内容与 fmt12 链形不符（非本对拍依赖项）。若要钉死符号名，后续可用
linkedit-dec.bin 解 GameStoreKit 导出 trie，或提取 /sbin/launchd 对其 LC_DYLD_CHAINED_
FIXUPS 首绑做终对拍（IPSW 为 .dmg.aea，需 AEA 工具，超出本次头解析范围）。

### 4.3 与死点的坐标复核

```
死点 PC = libobjc unslid 0x180404000 + slide 0x714000 + 0x17140 = 0x180B2F140
far（垃圾被解引用值）= 0x281E75CD070 —— 与 0x180B2F140、与本页任一写入值均无关联（§5）
```

---

## 5. Q4：far 0x281E75CD070 位形分析

```
far   = 0x00000281e75cd070 (42 bit)
result= 0x0046cf82cef08258 (55 bit)

移位/掩码全扫（k=0..39，>>k / <<k 截断 50 位）:            无命中
far & 0xFFFFFFFFF = 0x1e75cd070  !=  0x2cef08258(=result 低36)   ← 关键:
    连"同一目标被签错码"都不成立（PAC 失败/错签必保低位目标）
far ^ result、result - far:                       无模式
far 的半字节窗/字节视图 vs result 及全部 12 条 chain raw:          无重合
far & 0x7FFFFFFFFFF 作 chain tgt 解释:             不匹配任何 tgt
far 是否缓存内地址:      否（unslid 缓存范围 0x180000000..0x2FD43C000 之外）
弱信号（仅供存档，不作结论）: far & 0x7FFFFFFFFF = 0x1e75cd070 恰落
    .25.dylddata rw 映射内（+0xA59070）；按 36 位分窗 far>>36 = 0x28。
    样本量 1，判为巧合级。
```

**判定：far 与本页 fixup 的任何值（chain raw、rebase 目标、bind 目标、签名结果）
无位形因果** —— 既不是漏 fixup 的原始链值，也不是错签的 PAC 码（低位不保真），
更不是 result 的移位/截断产物。毒源另有其处。

---

## 6. 毒页分流结论与未决点

**结论**：r90 内核 dyld_pager 对该页的 16K fixup 拼装，在窗内可验证的每一步
（fmt12 路由、walk 步进、rebase 写入预测、auth-bind 字段、blend discriminator、
签名结果结构、slide 反算闭合）均与 donor 真值一致。**该页不是墙86 毒页的来源**；
launchd 死点的 far 垃圾与本页写入值无任何位形关联，分流应指向其它可疑面
（未被窗覆盖的其余 fixup 页、fixup 之外的写入者、或 PAC 校验期硬件层差异）。

**未决点 / 边界**：
1. 窗只捕到全 boot 前 12 条 sheng-chain（计数器上限），该 16K 页 0xc28 之后的链项、
   以及 seg_count=2 的另一段（__DATA_CONST）未验证 —— 上位机能复现时建议同时收
   `sheng-wrote` 行（代码已埋，:742）做写入后对拍。
2. fixup 页属主镜像按上下文推为 /sbin/launchd 本体（缓存外、fmt12、死点进程吻合），
   未做二进制级确认；可用 AEA 工具从 donor IPSW 提取 /sbin/launchd 后对
   LC_DYLD_CHAINED_FIXUPS 做终极对拍（首绑 ordinal=0 → 0x2cef08258 应与其一致）。
3. far 的精确产源未定位（本报告仅证明"非本页 fixup 产物"）。

复现材料：
- 映射总表 JSON：`/tmp/r91-recon/work/cache_map.json`
- 解析/验算脚本：`/tmp/r91-recon/work/{parse_cache,coverage,images_and_decode,final_analysis}.py`
  （全部 `python3 -I` 直跑，数据目录只作参数传入）
- donor 头：`/tmp/r89/cache/`（79 × *.hdr + main + sc01/sc25/sc54 正文）
