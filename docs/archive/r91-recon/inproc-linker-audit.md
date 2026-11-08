# dyld in-process linker 静态审计（工作单 C 刀，r91）

对象：`/tmp/r89/cache/dyld`（24A446 donor，Mach-O arm64e，symtab 完好；`__TEXT` vmaddr=0 →
文件偏移 == vmaddr，下文地址两者通用）。
样本：`/tmp/r89/cache/dyld_shared_cache_arm64e.*.hdr`（79 个，每个子缓存文件的首 4K）。
内核侧参照：`/home/ri/work/unixnu-meizu/ios-m2381-unixnu/patches/xnu-12377/sheng-mwl-page-in-linking.patch`、
`/home/ri/work/unixnu-meizu/temp/r76/xnu-apply-test/osfmk/mach/dyld_pager.h`（Apple 原版
`struct mwl_info_hdr`/`struct mwl_region` 定义）。
官方格式参照：apple-oss-distributions/dyld `include/mach-o/fixup-chains.h`、
`include/mach-o/dyld_cache_format.h`（main 分支；`dyld_chained_starts_in_segment` 自
dyld-940 起未变：`page_count` u16@0x14、`page_start[]`@0x16）。两份头已存
`/tmp/r91-recon/fixup-chains.h`、`/tmp/r91-recon/dyld_cache_format.h`。

---

## 0. 总路线图（先给结论用的图）

24A446 dyld 有**两条** MWL（map_with_linking）路径，各有各的内核 syscall + 各有各的
用户态 fallback，审计时必须分开：

```
[A] per-image 路径（JIT 加载的磁盘镜像）
    Loader::setUpPageInLinking@0x14d38
      → 组 mwl blob（fmt 取镜像自身 chained fixups；page_size 取 starts）
      → ___map_with_linking_np@0x15984 (svc #0x226 = 550)
      → 失败 → log "__map_with_linking_np(%s) failed, falling back to linking in-process"
      → dyld_map_with_linking_np@0x159b0（用户态逐页走链）
            → fixupPageAuth64@0x15ba8 / fixupPage64@0x7a118 / fixupPage32@0x7a208

[B] 子缓存路径（现役，本刀主战场）
    dyld3::mapSplitCachePrivate@0x8583c
      → 逐 mapping mmap 子缓存文件（MAP_PRIVATE|MAP_FIXED，prot 来自 mws 表）
      → 组 mwl blob（硬编码 mwli_page_size=0x4000、mwli_pointer_format=13）
      → ___map_with_linking_np
      → 内核 gate 拒绝（mwli_page_size != PAGE_SIZE=4K）← wall-86 现状
      → log "__map_with_linking_np(subcache %d) failed, falling back to linking in-process"
      → DyldSharedCache::fixupDataPages@0x5ad6c（slide_info5 走链重定向）
```

关键事实：**[B] 路径的 fallback 不是 dyld_map_with_linking_np，而是 fixupDataPages**。
fmt 13（`DYLD_CHAINED_PTR_ARM64E_SHARED_CACHE`）在 dyld_map_with_linking_np 的分派表里
**没有分支 → 静默跳页返回"成功"**——所以 [A] 的 fallback 即使被喂到 13 也只会空转，
真正的用户态修复在 [B] 的 fixupDataPages。

---

## 1. Q1：dyld_map_with_linking_np@0x159b0 的 blob 解析与 pageIndex

### 1.1 实参

符号名是 `dyld_map_with_linking_np(mwl_region const*, unsigned, mwl_info_hdr const*, unsigned)`，
但 IPA 对 static 单调用点做了实参特化：唯一调用点 0x15908（在 setUpPageInLinkingRegions 里）
传 `x0 = blob 指针, x1 = blobSize`，函数体也只读 x0/x1。前两个形参（regions/count）在函数体
中完全未使用。

### 1.2 头部校验（0x159d0–0x15a10）

```asm
159d0: ldr w8, [x0]            ; mwli_version
159d4: cmp w8, #0x7            ; 必须 == 7 (MWL_INFO_VERS)
159d8: b.ne 0x15b8c            ; -> return -1
159e0: ldrh w10, [x0, #0x6]    ; mwli_pointer_format
159e4: ldp w8, w9, [x0, #0x8]  ; mwli_binds_offset, mwli_binds_count
159e8: cmp w10, #0x3
159ec: mov w10, #0x2
159f0: cinc w10, w10, ne       ; 条目宽度 = (fmt==3)?4:8 字节
159f4: lsl w10, w9, w10
159f8: add w10, w10, w8
159fc: cmp w10, w1             ; binds 表越界 blobSize?
15a00: b.hi 0x15b6c            ; -> fprintf "bind table extends past blob, blobSize=%d,..."
15a04: ldp w8, w9, [x19, #0x10] ; mwli_chains_offset, mwli_chains_size
15a08: add w9, w9, w8
15a0c: cmp w9, w1              ; chains 表越界?
15a10: b.hi 0x15b8c            ; -> return -1
```

### 1.3 mwl_info_hdr 布局（反汇编重建，与内核 `dyld_pager.h` 逐字段吻合）

| 偏移 | 宽 | dyld 写入方 | 内核字段名 |
|---|---|---|---|
| +0x00 | u32 | 7（两处构造：0x15698 `mov w8,#7; str`；0x85fc0 `movk` 组合写入） | `mwli_version` |
| +0x04 | u16 | arg5 = 各段统一 page_size（0x1569c `strh w28,[x22,#4]`） | `mwli_page_size` |
| +0x06 | u16 | arg4 = pointer_format（0x156a0 `strh w26,[x22,#6]`） | `mwli_pointer_format` |
| +0x08 | u32 | binds 表 blob 内偏移（0x156a4 `stp w20,w24,[x22,#8]`） | `mwli_binds_offset` |
| +0x0c | u32 | binds 条数 | `mwli_binds_count` |
| +0x10 | u32 | chains 容器偏移（两处均写 0x28：0x156a8/0x85fc0 组合） | `mwli_chains_offset` |
| +0x14 | u32 | chains 容器大小 | `mwli_chains_size` |
| +0x18 | u64 | arg m = slide（0x156b0 `str x27,[x22,#0x18]`；0x85fe8 `stp x14,x15` 低半） | `mwli_slide` |
| +0x20 | u64 | `loader->mf(state)` / 子缓存映射基址（0x156e0 `str x0,[x22,#0x20]`） | `mwli_image_address` |

chains 容器（@mwli_chains_offset）= `{u32 seg_count; u32 seg_info_offset[seg_count];
dyld_chained_starts_in_segment 池}`，即官方 `dyld_chained_starts_in_image` 布局；
每个 `dyld_chained_starts_in_segment` 为官方布局（见 §2.1），由调用方用 memmove
从镜像 LC_DYLD_CHAINED_FIXUPS payload（0x15748 `ldr x1,[x28,#0x30]` = startsForSegment
结果，0x15758 memmove `size = *(u32*)starts`）或从 slide_info5（0x8606c 逐 u16 拷
page_start，目标 +0x16）填入。

### 1.4 逐页循环与 pageIndex 算法（0x15a40–0x15b08）

```asm
15a40: ldr x8, [x19, #0x20]     ; base = mwli_image_address
15a44: ldr x9, [x23, #0x8]      ; starts->segment_offset (u64 @+8)
15a48: add x25, x9, x8          ; segStartVA = base + segment_offset
; ... 每页:
15a60: ldrh w11, [x19, #0x4]    ; w11 = mwli_page_size (u16)
15a64: mul w11, w24, w11        ; w24 = 页循环计数
15a68: add x0, x25, x11         ; pagePtr = segStartVA + pageIdx*page_size
; 段搜索（找到包含 pagePtr 的段）:
15a8c: ldrh w14, [x2, #0x4]     ; w14 = starts_in_segment->page_size (u16 @+4)
15a90: ldrh w15, [x2, #0x14]    ; w15 = page_count (u16 @+0x14)
15a94: umaddl x15, w15, w14, x13 ; segEnd = segStart + page_count*page_size
15a98: cmp x15, x0
15a9c: b.hi 0x15ab0             ; pagePtr < segEnd -> 命中
; 命中后:
15ab0: sub x9, x0, x13          ; x9 = pagePtr - segStart
15ab4: ldrh w8, [x19, #0x6]     ; w8 = mwli_pointer_format
15ab8: udiv x3, x9, x14         ; pageIndex = (pagePtr - segStart) / page_size
```

**答 Q1**：pageIndex 除数 = 命中段的 `dyld_chained_starts_in_segment.page_size`
（u16 @+4），**不是硬编码 4K，也不是 PAGE_SIZE**。pagePtr 的合成同样用该段
page_size 乘循环计数，所以"页→槽"全程自洽。硬编码只出现在**blob 构造方**：
[A] 路径 page_size 取镜像 starts；[B] 路径 mapSplitCachePrivate 硬编码
`mwli_page_size = 0x4000`、`fmt = 13`、每段 page_size = 0x4000（0x85fc4/0x85fc8
`movk x8,#0x4000,lsl#32; movk x8,#0xd,lsl#48`；0x8601c `mov w4,#0x4000; movk w4,#0xd,lsl#16`）。

fmt 分派（0x15abc–0x15ba4）：1→fixupPageAuth64(base=mwli_slide)；2→fixupPage64(base=slide)；
3→fixupPage32；9/0xc→fixupPageAuth64(base=mwli_image_address)；6→fixupPage64(base=address)；
**其余（含 13）→ `b 0x15afc` 静默跳下一页**，整页循环走完 `return 0`（假成功）。
seg_count==0 时直接 return 0（0x15a1c/0x15b48）。页找不到所属段时走
`.cold.1`@0x9d528 = `__assert_rtn("fixupPage","Loader.cpp",2008,"segInfo != nullptr")` → abort。

---

## 2. Q2：fixupPageAuth64@0x15ba8 的走链

### 2.1 输入

```asm
15bc8: add x8, x2, #0x16
15bcc: ldrh w8, [x8, w3, uxtw #1]  ; pageStart = starts->page_start[pageIndex] (u16 @starts+0x16)
15bd4: cmp x8, x9(0xffff)
15bd8: b.eq 0x15d10                ; 0xFFFF = 页无 fixups，直接返回
15be4: ldr w9, [x1, #0x8]          ; mwli_binds_offset
15be8: add x22, x1, x9             ; x22 = binds 目标数组（u64）
15bec: add x21, x0, x8             ; x21 = walk 指针 = page + pageStart
15bf0: ldrh w8, [x1, #0x4]         ; w8 = mwli_page_size (u16)
15bf4: add x23, x0, x8             ; x23 = 页尾 = page + mwli_page_size
15bf8: cmp w4, #0x0                ; w4 = bool（fmt1=0 用 +0x18 slide；fmt9/0xc=1 用 +0x20 address）
15c04: csel x8, x9(0x20), x8(0x18), ne
15c08: ldr x24, [x1, x8]           ; x24 = rebase base
```

注意两点：
1. `page_start[]` 读偏移 **+0x16**（`page_count` u16@0x14 之后）——与官方现行头一致
   （dyld-940 至今未变；老印象中"page_count u32@0x14 / page_start@0x18"是错的，三个
   版本 tag 的 fixup-chains.h 已逐一核对）。
2. 走链页界用的是 **hdr 的 mwli_page_size（info+4）**，不是段的 page_size。dyld 构造
   blob 时会校验各段 page_size 一致（0x1525c→Diagnostics `"page_size is different in
   different segments"`；fmt 同理 0x15238→`"pointer_format is different in different
   segments"`），所以实际二者相等；但字段来源不同这一点在内核侧对应实现里是
   `segInfo->page_size`（见 §4 内核 hunk），属于**语义差一位的接口**，两边构造保证一致
   才不出事。

### 2.2 条目解码（0x15c14–0x15c44）

```asm
15c14: ldr x25, [x21]              ; entry
15c18: tbnz x25, #0x3f, 0x15c48    ; bit63 = auth
15c1c: tbnz x25, #0x3e, 0x15c90    ; bit62 = bind
; --- rebase (63=0, 62=0): dyld_chained_ptr_arm64e_rebase (USERLAND 布局)
15c20: and x8, x25, #0x7ffffffffff ; target 43 bits
15c24: lsr x9, x25, #43
15c28: add x8, x8, x24             ; target + base
15c2c: add x0, x8, x9, lsl #56     ; | (high8 = entry[43:50]) << 56
15c30: ubfx x8, x25, #51, #11      ; next 11 bits
15c34: str x0, [x21]               ; 写回
15c38: add x9, x21, x8, lsl #3     ; 8 字节 stride
15c40: cbnz x8, 0x15c0c            ; next==0 结束
```

与官方 `struct dyld_chained_ptr_arm64e_rebase { target:43; high8:8; next:11; bind:1; auth:1 }`
逐位一致；fmt 1（target=vmaddr，base=slide）与 fmt 9/0xc（target=vm offset，base=装载
地址）共用此代码，仅 base 选择不同——官方注释 "stride 8, unauth target is vmaddr/vm
offset" 与此吻合。

bind 分支（0x15c48–0x15cc0）：

```asm
; auth bind (63=1,62=1): dyld_chained_ptr_arm64e_auth_bind24
15c48: ubfx x4, x25, #49, #2       ; key = bits[49:50]
15c4c: lsr x8, x25, #32
15c54: and w9, w25, #0xffffff      ; ordinal 24 bits（fmt 9 时 bits[16:24) 恒 0，兼容 16-bit ord）
15c58: ldr w10, [x20, #0xc]        ; mwli_binds_count
15c60: b.hs 0x15cf4                ; 越界 -> "out of range bind ordinal %u (max %u)"
15c64: ubfx x2, x25, #48, #1       ; addrDiv = bit48
15c6c: ldr x0, [x22, x9, lsl #3]   ; value = binds[ord]
15c70: and w3, w8, #0xffff         ; diversity = bits[32:48)
15c74: mov x1, x21                 ; diversifier 基 = 槽地址
15c78: bl 0x15d2c <signPointer>
; unauth bind (63=0,62=1): dyld_chained_ptr_arm64e_bind24
15ca4: ubfx x9, x25, #32, #19      ; addend 19 bits（无符号视图）
15ca8: orr x8, x8, #0xfffffffffffc0000  ; bit50=1 时按 18-bit 符号扩展
15cb0: csel x8, x9, x8, eq
15cbc: add x0, x9(binds[ord]), x8  ; value + addend，不签名
; auth rebase (63=1,62=0): dyld_chained_ptr_arm64e_auth_rebase
15c84: ldr x9, [x20, #0x20]        ; mwli_image_address
15c88: add x0, x9, w25, uxtw       ; value = base + entry[0:32)
```

全部四种条目的位布局与 `/tmp/r91-recon/fixup-chains.h` 的 USERLAND24 结构体逐位一致
（含 next 恒 @bits[51:62)、auth=bit63、bind=bit62）。

### 2.3 signPointer@0x15d2c

```asm
15d34: bfi x1, x8, #48, #16        ; modifier = 槽地址高 16 位替换为 disc
15d3c: csel x16, x8(disc), x1(addr|disc<<48), eq   ; useAddrDiv=false 时只用 16 位 disc
15d64/15d78/15d54/15d90: pacia1716 / pacib1716 / pacda / pacdb   ; key 0/1/2/3
```

即**硬件 PAC、无额外 salt**：modifier = 槽 VA（高 16 位放 disc）或纯 16 位 disc。
value==0 → 存 0（15d2c）。key 取自条目 bits[49:50]，0=A、1=B、2=DA、3=DB。

### 2.4 跨页判断

chain 推进是 `next*8` 的纯偏移推进，唯一约束 `walk < page + mwli_page_size`
（循环头 0x15c0c `cmp x21, x23; b.hs 0x15cd8` → fprintf
`"fixup chain entry (%p) off end of page starting at %p"`（stderr，0xaa530）后返回——
auth 版也打印，非静默）。
即：链若指向页外，本页终止、不跨页续走——续走责任在下一页自己的 page_start 槽。
设计上每 16K 页独立成链，与官方 page-in linking 语义一致。

fixupPage64@0x7a118（fmt 2/6）同构：stride 4（`lsl #2`）、next 12 bits@[51:62)、
bind ord 24 bits + addend 8 bits@[24:32)、rebase target 36 bits + high8@[36:44)
`new = (target+base) | high8<<56` —— 与官方 `dyld_chained_ptr_64{,_rebase,_bind}` 一致。
fixupPage32@0x7a208 / fixupChain32@0x7a2d0 为 32 位格式，本 lane 不涉及，未深挖。

---

## 3. Q3：写回机制

**全路径无 mprotect / vm_remap / mach_vm 调用。** 整个 dyld 的 `vm_protect` 只有 3 处
（0x7d584、0x85038、0x8511c），均不在 MWL/fixup 路径上。写回一律是**普通 str 到进程
已映射页**：

- [A] per-image fallback：fixupPage* 里 `str x0, [x21]`（0x15c34 等）。页面属于
  `loader->mf(state)` 的既有映射；若该段是 r-x/r--（例如 __AUTH_CONST 未解锁），
  str 会直接 fault——dyld 在此路径上**没有任何解锁动作**，依赖"待修复段本就可写"
  这一前提（PageInLinkingRange 的 prot 位即 r/w/x 打印，logB
  `'%14.*s (%c%c%c) 0x%012llX->0x%012llX (fileOffset=0x%0llX, size=%lluKB)'`）。
- [B] 子缓存 fallback（现役）：mapSplitCachePrivate 先把各 mapping 以
  MAP_PRIVATE|MAP_FIXED mmap 到共享区（0x85b1c `bl _mmap`，prot 取自 mws 的
  maxProt|WRITE 组合，0x85ad8 `orr w12, w10, #0x2`），MWL 被拒后由
  **fixupDataPages@0x5ad6c** 写：walker 回调块 0x5b224 里

```asm
5b24c: ldrb w9, [x1, #0x13]        ; Fixup.auth 标志
5b250: tbz w9, #0, 0x5b268
5b254: add x0, x1, #0x10
5b25c: bl 0x6d30 <mach_o::PointerMetaData::signPointer>   ; auth 条目签名
5b268: str x8, [x19]              ; *Fixup.location = value_add + runtimeOffset + slide
```

  也是裸 store。skip 条件有三：`slideInfoFileSize==0`（0x5adf4 `cbz → 下一 mapping`）、
  bool 实参为 0 时 `flags bit7`（0x5ae00 `ldr w8,[x26,#0x28]; tbnz w8,#7 → 跳过`，
  flags@0x28 的 bit7=0x80）——**注意 .25 的 mws[6] flags=0x85 正含 bit7**，若调用点
  bool 实参（`options+0x19` 字节）为 0，该 mapping 的链根本不修，盘上指针原样留在
  内存 = 另一个现成的毒源，判别时可查该 mapping（0x1efc24000..0x1efcdc000）的槽是否
  全等于 donor 值；以及版本非 2/3/5 → Error。只读 mapping（prot 1/5，如 mws[0] 头页、
  mws[7] slide 池页）slideSize=0，walker 不进入，"只读映射怎么处理"的答案是：
  **不处理，也不会碰到**。
- MAP_PRIVATE ⇒ 写的是 COW 私有页，**毒不落盘、进程私有**——与 wall-86
  "launchd 消费毒槽"的症状拓扑一致（毒在它自己的页里）。

---

## 4. Q4：对拍（真实 .hdr 解析 + 槽位算术模拟）

### 4.1 头部实据（.25/.54 dylddata、.26 dyldreadonly）

全部 79 个 .hdr 为 `dyld_v1  arm64e` / cacheType=2(multi-cache) / platform=2 /
flags=0x1000（bit12；按官方位域落在 newFormatTLVs，实际是我们 builder 的自定义含义，
待与 builder 源对表）。每子缓存自带 mapping 两张表：

- `mappingWithSlide` @hdr+0x138：**56 字节记录**，与官方
  `dyld_cache_mapping_and_slide_info`（address/size/fileOffset/slideInfoFileOffset/
  slideInfoFileSize/flags/maxProt/initProt）**完全一致**（fixupDataPages 按
  56 步进读 +0x18/+0x20/+0x28/+0x30 与之吻合，0x5adb0 `lsl x9,x8,#6; sub x9,x9,x8,lsl#3`
  = count*56 ✓）。
- `mapping` @hdr+0x238：**32 字节记录** {addr u64, size u64, foff u64, {maxProt,initProt}
  u64}——非官方 40B 布局，是 builder 自有格式（内容与 mws 一一对应）。

.25.dylddata mws 表（实值）：

| i | addr | size | foff | slideOff | slideSize | flags | prot |
|---|---|---|---|---|---|---|---|
| 0 | 0x1e00dc000 | 0x4000 | 0x0 | 0 | 0 | 0x0 | 5/5 |
| 1 | 0x1e00e0000 | 0x1e2c000 | 0x4000 | 0xdc0163a | 0xf2e | 0x4 | 3/1 |
| 2 | 0x1e3f0c000 | 0x2c44000 | 0x1e30000 | 0xdc00000 | 0x163a | 0x0 | 3/3 |
| 3 | 0x1e6b50000 | 0x24000 | 0x4a74000 | 0xdc02568 | 0x2a | 0x44 | 3/1 |
| 4 | 0x1e6b74000 | 0x2198000 | 0x4a98000 | 0xdc02592 | 0x10e4 | 0x1 | 3/3 |
| 5 | 0x1e8d0c000 | 0x6f18000 | 0x6c30000 | 0xdc03676 | 0x37a4 | 0x5 | 3/1 |
| 6 | 0x1efc24000 | 0xb8000 | 0xdb48000 | 0xdc06e1a | 0x74 | 0x85 | 3/1 |
| 7 | 0x1f1cdc000 | 0x8000 | 0xdc00000 | 0 | 0 | 0x0 | 1/1 |

地址链连续性已程序化核验：mws[0..6] 逐条 contig，mws[2]/mws[7] 前的 0x2000000
（32MB）为子缓存区间隙，无重叠。mws[7] = slide-info 池自身（r--、无 slide）。
ext@0x228 = `0x800000020100000c` 常量（官方头止于 0x224，此为我们 builder 的标记，
非官方字段）。

**局限（如实声明）**：slide_info5 结构本体在 foff 0xdc0163a 等 >0x1000 处，79 个 4K
样本不含任何 `page_start[]` 数组，故"拿真实条目解码第一步"无法在样本内完成；
下面对拍用真实页几何做槽位算术模拟，条目级解码留待整文件（建议下一轮直接取
`dyld_shared_cache_arm64e.25.dylddata` 全文件，slideSize 总计 <0x4000，量很小）。

### 4.2 槽位算术模拟（16K 页几何，以 .25 mws[1] 为例）

映射 size=0x1e2c000，slide_info5 声明 page_size=0x4000 ⇒ page_starts[] 有 0x78B 项，
第 i 项 = 16K 页 i 的链头偏移。取 fault 在映射基址 +0x14000（16K 页 #5）：

- **dyld fixupDataPages**：`pageVA = base + 5*0x4000`；`head = page_starts[5]`；
  PointerFormat::make(13) walker 按 fmt 13 布局推进（next=entry[52:62)，stride 8）。
  页号、槽数组、页大小三者同源（同一 slide_info5）⇒ **无错位**。
- **旧内核 fixup_page（PAGE_SIZE=4K，sheng patch 之前）**：
  `pageIndex = (userVA - segStart) >> 12 = 20` → 读 `page_start[20]` = **16K 页 #20 的
  链头**，套在 16K 页 #5 的 4K 物理页上；且 `end_contents = contents + 4K` 截断。
  更糟：该段 page_count=0x78B，pageIndex 合法域本应 [0,0x78B)，4K 算法可算出
  最大 0x1E2B ⇒ **page_start[] 越界读**（读进 value_add/池内相邻数据当链头）。
  这是教科书式毒写生成器——但见 §5，wall-86 的内核 gate 在此之前就拒了，该代码路径
  **不可达**。
- **dyld_map_with_linking_np 若被喂 fmt13**：分派落空 → 整段静默跳页 → 假成功、
  零修复（所有指针保持盘上值）。当前不发生（[B] 不走这条 fallback；[A] 的镜像
  不是 fmt13），但这是**将来排查"修复没发生却无报错"时的头号陷阱**。

---

## 5. Q5：结论与判别

### 5.1 判定

**"dyld 用户态 in-process linker 算术毒写"假设：不成立（就页索引/走链算术而言）。**

依据：
1. 两条 fallback 的页→槽算术全部同源取参（[A] 用 starts 自身 page_size；
   [B] 用 slide_info5 自身 page_size/page_starts/value_add），指令级无 4K 硬编码，
   无槽位错位（§2、§4.2）。
2. 条目位域解码与 Apple 官方 fixup-chains.h 逐位一致（arm64e USERLAND24、
   PTR_64/OFFSET、SHARED_CACHE fmt13 四种全核对）。
3. 写回是 MAP_PRIVATE 私有页裸 store，无 RO 页误写通道（§3）。
4. wall-86 现状（内核 gate `mwli_page_size(0x4000) != PAGE_SIZE(0x1000)` 先拒，
   `kr = KERN_INVALID_ARGUMENT; goto done;` 发生在任何内核写之前）⇒ **内核 MWL
   拼装路径在 wall-86 上零写入**，毒必产自用户态或数据本身。

### 5.2 毒写的剩余主嫌（按吻合度排序）

1. **value 语义组装错**：fixupDataPages 每槽写
   `final = value_add(slide_info5+0x10) + runtimeOffset + slide(实参)`。若 slide
   实参（调用点取 `options+8`，0x86310/0x86450 附近）或某 mapping 的 value_add
   与实际映射基址不一致，该 mapping 内**所有 plain-rebase 槽统一偏移一个常数**
   ——与 r89 实测"毒槽字节不可由 donor 推导"高度相容（donor 值 + 常数 delta 正是
   "不可推导"的常见形态）。**下一步对拍建议**：取毒页 ≥2 个 plain-rebase 槽，
   求 X−D（毒值−donor 值）：多个槽 delta 相等 ⇒ 组装错实锤；逐槽不同 ⇒ 走链/数据错。
2. **cache 数据本身**（builder 写的 page_starts/链条目）：4K 样本无法验证，
   需整文件（slide 池 <0x4000 字节，成本极低）。
3. **内核 MWL 拼装**：wall-86 排除（§5.1.4）；**sheng lane 生效后才有嫌疑**。
   sheng 的 16K 拼装（dyld_pager_peek 4 页进 kalloc 缓冲 → fixup_page 一次 →
   copy-out 4K 片）逻辑已核读：pageIndex/end_contents/page_shift 全部改为
   `segInfo->page_size` 派生，与 dyld 读侧一致；但注意 signPointer 语义差：
   dyld 用硬件 PAC 无 salt（§2.3），sheng 内核用
   `pmap_sign_user_ptr(..., pager->dyld_a_key)` 且**删掉了 `dyld_a_key==0` 时不
   签名的旧分支**——若 a_key 参与方式与 dyld 侧硬件签名不等价，同一槽在两条路径
   下签名不同 ⇒ auth 条目交叉验证会炸。这是 sheng lane 独有的判别点。

### 5.3 与"内核 MWL 拼装写错"的判别点（可执行）

1. **日志互斥**：console 出现 `__map_with_linking_np(subcache %d) failed, falling
   back to linking in-process`（0xac21f）= 用户态路径在跑；dmesg 出现
   `sheng-mwl:` / `sheng-peek[%d]:` / `sheng-chain[%d]:` / `sheng-sign:` =
   内核路径在跑。两者互斥，一票判定哪条腿站着。
2. **delta 形态**：毒槽 X−D 为常数（按 mapping 分组）⇒ value_add/slide 组装错
   （用户态）；呈"每 16K 页头错到 +3 槽"模式 ⇒ 4K 页索引 bug（现知仅旧内核有）；
   无规律 ⇒ builder 写的链数据错（数据侧）。
3. **lane 切换对照**：sheng lane（内核收 16K）上毒消失 ⇒ 旧状态下的毒与内核无关，
   用户态/数据侧继续查；sheng lane 上毒仍在但形态改变 ⇒ 内核拼装或 a_key 签名差。
4. 复用 `/home/ri/work/unixnu-meizu/temp/r76/xnu-apply-test/tests/unit/vm/vm_map_with_linking_tests.c`
   内核单测，把 .25 mws[1] 的真实几何（16K、0x78B 页）灌进去做回归。

### 5.4 本刀新发现（工作单外，需记录）

- [B] 路径 fallback = `DyldSharedCache::fixupDataPages`（slide_info5 走链），而
  **不是** r89 工作单认定的 `dyld_map_with_linking_np`；后者对 fmt 13 是静默假成功。
- mapSplitCachePrivate 构造 blob 时 mwli_page_size/fmt/page_size 是**硬编码
  0x4000/13/0x4000**（不读 cache 的实际值）——4K 内核上等于把"拒绝"写死，
  这正是 wall-86 每个子缓存都走 fallback 的机制原因。
- 我们 builder 的子缓存头 `mapping` 表为 32B 记录（非官方 40B）；官方消费方主要用
  mws 表（56B、官方格式一致），暂无消费方踩坑证据，但属格式分叉，记录备查。

---

## 附 A：关键地址速查（/tmp/r89/cache/dyld）

| 地址 | 符号/作用 |
|---|---|
| 0x15984 | `___map_with_linking_np`（svc #0x226=550 trampoline） |
| 0x159b0–0x15ba7 | `dyld4::dyld_map_with_linking_np(info, size)` 用户态逐页走链（[A] fallback） |
| 0x15ba8–0x15d2b | `dyld4::fixupPageAuth64`（fmt 1/9/0xc） |
| 0x15d2c–0x15dab | `signPointer(value, div, useAddrDiv, disc, key)`（硬件 PAC） |
| 0x7a118 | `dyld4::fixupPage64`（fmt 2/6） |
| 0x7a208 / 0x7a2d0 | `fixupPage32` / `fixupChain32` |
| 0x14d38–0x152bb | `Loader::setUpPageInLinking`（[A] 入口，forEachSegment 建区） |
| 0x155ac–0x15983 | `setUpPageInLinkingRegions`（建 blob、调 syscall、fallback） |
| 0x8583c | `dyld3::mapSplitCachePrivate`（[B] 入口；blob 硬编码 0x4000/13 @0x85fc0–0x85fe8） |
| 0x5ad6c | `DyldSharedCache::fixupDataPages(slide, bool)`（[B] fallback，v2/v3/v5 分派） |
| 0x90148 | `PointerFormat::forEachFixupLocationInChain`（通用 walker，fmt 虚分派） |
| 0x5b224 | [B] 回调块（final = value_add + runtimeOffset + slide；裸 store） |
| 0x9d528 | `.cold.1` = `__assert_rtn("fixupPage","Loader.cpp",2008,"segInfo != nullptr")` |

## 附 B：错误/日志串（佐证）

- `bind table extends past blob, blobSize=%d, offset=%d, count=%d`（0xaa4d3）
- `fixup chain entry (%p) off end of page starting at %p`（0xaa530）
- `out of range bind ordinal %u (max %u)`（0xaa567）
- `Setting up kernel page-in linking for %s`（0xaa41b）/ `... for subcache %d`（0xac1ec）
- `__map_with_linking_np(%s) failed, falling back to linking in-process`（0xaa48d）
- `__map_with_linking_np(subcache %d) failed, falling back to linking in-process`（0xac21f）
- `pointer_format is different in different segments` / `page_size is different in
  different segments`（0xa999c / 0xa99ce）
