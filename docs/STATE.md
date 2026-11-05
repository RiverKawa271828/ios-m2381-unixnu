# STATE — 现势状态（m2381，2026-10-28 更新）
## 0. 现势指针（每轮首更，2026-11-06 会话六十三·墙61 bdevsw 族定谳 payload-r63 待刷版）

- **★会话六十三 = r62 首刷判读定谳墙61 = bdevsw 族 + 2 面绑 native `_bdevsw_add`
  r63 出件待刷（2026-11-06，本会话连轮）**：
  **r62 首刷判读**（payload-r62-2d3e8dd4）：crc `0xaf0e2d21` 零纠正（124B），t=1s，hit
  \#1。panic from `0xfffffff00a41cb04`（x0 `0x1` x1 `0xfffffff00bb651f8`）→ **墙 60 消实
  锤**（erandom 7 面生效；AppleImage4 nonce 过，推进到某驱动的设备注册）。**定谳链**：
  caller bl `0xa422054` → 标准 GOT thunk（adrp 0x83ae000+0x480）→ 槽 `0x83ae480` 哨兵 →
  donor `0xb3d1700`。语义 = **`bdevsw_add(1, 56B 模板)`**：x0=1=请求 major，x1=kext 自有
  bdevsw 模板（0xbb651f8，diversified-auth 指针数组），返回值与请求 major 比较 =
  bdevsw_add 契约。**定名**（devsw 域取证，TU=`bsd/kern/bsd_stubs.c`）：donor
  `0xb3d1700` = **`bdevsw_add`**（devsw CAS 锁 0xb9ce228 = donor `_cdevsw_add/_cdevsw_remove/
  _devsw_unlock` 同一全局；per-cpu mtx stats [tpidr+0x1c0]；bound 0x17=nblkdev−1；
  memcmp vs empty-bdevsw 占位 0x7ec2aa8 = 6×eno_\*+0）；donor `0xb3d161c` =
  **`bdevsw_isfree`**。几何铁证：donor bdevsw[24×56B]@0xb89c1b0 **紧邻** cdevsw[64×
  112B]@0xb89c6f0（donor `_cdevsw_add` stride 0x70/bound 0x3f/key 0x7ec2ae0）= conf.c
  bdevsw[]→cdevsw[] 布局。**修 = 2 面（0x83ae480/0x849f240）绑 native `_bdevsw_add`
  `0xB3A7810`**（noctf 0x79eb810 精确，体同构）；**isfree 槽 0x83ae488 已被 s61 696
  批量绑到同一 native `0xB3A7700`——独立交叉证实孪生判定**。s63 手术（设备仓
  scripts/s63-payload-surgery.py，按 donor 词动态枚举面）：词 `0x80110000043a3810`，
  diff 6B/2 簇**零杂散**，seal 未动。**出件 `out/m2381/payload-r63.img`（99MiB）
  sha256 `0cb17660…` 待刷**（recovery_a ← r63；回退一级 = r62 `2d3e8dd4`）。账本
  814→**816**（+bdevsw_add/bdevsw_isfree 双记录，体 sha 在 temp/s63-ledger-bodies.json）。
  **注**：native `_cdevsw_isfree` stride 0x70 = cdevsw(112B)、donor f 表 stride 0x38 =
  bdevsw(56B)，两表各归各勿混判；s63 手术 diff 期望须按面动态算字节数（哨兵低 48 位
  0x045A5C4C 的 byte3=0x04 与部分新词相同）。s64 可选项（待拍板）：395 集中 noctf
  精确 + 体验证的导入小批合绑（5-10 个/轮）以加速收敛，风险 = 难二分。
- **★下轮工作单 = 会话六十四（战略升级：静态引导道追踪 + 并行判读农场，终结打地鼠；用户授权激进多轮迭代）**：
  **r63 屏摄核验**（用户指示本轮不判读）：crc `0x68352ecd` 零纠正（124B）from
  `0xfffffff00a41cb1c`（x0 0x1 x1 `0xfffffff00bb65188`）→ **静态预测命中实锤**：s63 收
  队前从 bdevsw caller 前向扫描预读的下一墙 = bl 0xa41cb18 → 槽 0x83ae510 → donor
  `0xb3d1d58`（回退地址精确吻合）——**墙可静态预读，刷机只是验证不再是探路**。x1 =
  0xbb65188 = cdevsw 尺寸（112B）模板（比墙61 的 bdevsw 模板 0xbb651f8 恰低一表项），
  疑 cdevsw_add 族；donor 0xb3d1d58 落 bsd_stubs.c TU 区（源序 isfree<add<remove<cdevsw
  族），native `_bdevsw_remove`/`_cdevsw_isfree`/`_devsw_unlock` noctf 在册待对。
  A. **工具基建**（深度研究三件）：
     A1. kext 归属修复：GOT 页 → fileset entry（修 span 解析 bug：entry 内层段独立重定
         基），产出 = 815 面/392 导入按 kext 分桶清单。
     A2. **静态引导道追踪器**：从当前墙 caller 起 DFS bl→thunk→slot 链 + 本地函数跳转
         （有界深度），产出每 kext 导入触发序列 = 未来墙预告单（0xa41cb18 扫描 = PoC，
         一扫双墙：0x83ae510→0xb3d1d58、0x83ae490→0xb3d1894，均 sentinel）。
     A3. donor 导出 trie（0x4c50000 blob 0x5cc 无名串）与 kext 内层 bind（prelink 已剥）
         复攻 = 低优先级挂账。
  B. **并行判读农场**（多轮迭代引擎）：
     B1. 当前 kext（caller 所在，GOT 0x83aexxx 页）全部哨兵导入 → 扇出 agent 并行三证
         判读（管线已文档化：donor 体反汇编 → noctf/源码锚 → TU 几何 + 全局指纹 → 语义
         契约；范例 = erandom/bdevsw 双案录）；我复核抽查 → 合批绑定 → payload-r64 →
         刷机验证。
     B2. 每轮清一个 kext（或数个）；回退链逐轮滚动；账本逐轮增记（构建期价值累积，
         emitter go/no-go 押后到农场跑顺后再议）。
  C. **边界**：无证据不绑——近匹配自动评分已判死（7 对已知孪生校准相似度仅
     0.08-0.24，donor 27.0.1 与 feed 编译分歧过大）；每面仍需三证，agent 产证、人复核
     后绑定；T1 精确体哈希猎手产出 ≈ 0（temp/s64-twinhunter.py）亦留档。
- **★会话六十二 = r61 首刷判读定谳墙60 = read_erandom 族 + 7 面绑 native `_read_frandom`

- **★会话六十二 = r61 首刷判读定谳墙60 = read_erandom 族 + 7 面绑 native `_read_frandom`
  r62 出件待刷（2026-11-06）**：
  **r61 首刷判读**（payload-r61-1795d6bb）：crc `0xa9474625` 零纠正（125B），t=1s，hit
  \#1。panic from `0xfffffff0090261b8`（x0 `0xfffffff00ba64b19` x1 `0x10`）→ **墙 59 消实
  锤**（696 批量 9,134 面全生效；corecrypto kext start 注册 PRNG 后推进进 AppleImage4
  nonce manager）。**定谳链（全程证据闭环）**：caller bl `0x9028f44` = 环境对象回调派发
  thunk（`ldr x16,[x0,#0x100]; mov x17,#0x12f7; braa x16,x17`；对象 = 运行时填充的
  __DATA 注册表）→ 回调 = **ctx-drop 适配器 `0x902576c`**（`mov x0,x1; mov x1,x2; b
  0x903a898`，(ctx,buf,len)→(buf,len)）→ AppleImage4 GOT thunk 0x903a898 → 槽
  `0x80e1a40` 哨兵。语义 = **16 字节随机读取**（nonce；x0 = struct+5 内嵌 16B 缓冲，
  x1 = [x10+0x48] 长度字段）。**定名**（donor 导出 trie 不可解码；source 锚 + 体取证）：
  donor `0xae9bd20` = `read_random_generate`（blraa 经 `prng_generate` 函数指针全局
  [0x7dff0a68] + 每核互斥 tpidr_el1 + 0x100 分块）；donor `0xae9bc68` = `read_erandom`
  （u32 len + `prng_ready` 门 [0x7dff0a80]）；兄弟 `0xae9bbac` = `read_erandom_generate`
  （16K PAGE_SIZE 分块；map how=string 记录独立佐证 native 0x7507818）——TU =
  `osfmk/prng/prng_random.c`（墙 55 fips 门同文件）。**修 = 7 面绑 native
  `_read_frandom` `0xAEC2BF8`**（noctf 0x7506bf8 精确；源码 read_frandom{read_erandom}
  = 门+(prng_ready?PRNG:DRBG) 语义超集；native 体近指令级同构 = 同门+同 0xedeb per-cpu
  pacia 全局+同 0x100 分块；native 无 `_read_random_generate`/`_read_erandom` 符号
  （静态内联）故 `_read_frandom` 为正解；xnu-12377 车道的 fileset-read-erandom-shim
  不在 27 车道亦不需要）。**s62 手术**（设备仓 scripts/s62-payload-surgery.py）：槽
  0x80e1a40/0x8316e80/0x833c2e8/0x8479c28/0x849fa18/0x84ac9d8/0x848d5e8 → 词
  `0x8011000003ebebf8`，pre-image 白名单+donor 词断言全中，diff 28B/7 簇**零杂散**，
  seal 未动。**出件 `out/m2381/payload-r62.img`（99MiB）sha256 `2d3e8dd4…` 待刷**
  （recovery_a ← r62；回退一级 = r61 `1795d6bb`）。账本 812→**814**（+read_erandom/
  read_random_generate 双记录，体 sha 在 temp/s62-ledger-bodies.json）。
  **对质裁决（工作单 B 项）**：s61 冲突 `0xad8f628`（map=`_task_reference` vs
  账本=`_task_reference_external`）**操作上化解**——noctf 两名同址 `0x737d884`
  （别名/ICF 合并），绑定目标唯一 = native km `0xAD39884`，s63 直绑无需裁名。
  **工具警示**：map 稀疏（仅 matcher 映过函数），近邻命名不可靠；**395 未定名集已可
  用 `temp/s62-sentinels.py` 全枚举**（824 槽/395 导入，donor 词→map 近邻名为**暂定**
  需逐个三证；824 vs r60 记账 823 差 1 未溯源）。
- **★下轮工作单 = 会话六十三**：A. 刷 r62 判读（CRC 对拍 → 分流：新哨兵墙 →
  s62-sentinels 全名单对号 + 三证顺带修；PAC/数据 abort → 疑 `_read_frandom` 绑定 →
  回退 r61 二分；无墙推进 → stage 轨迹/vfsinit/root mount/SHLOG state=2）。B. 小批队
  列：`0xb47008c` publishResource 3 面 + IOCrypto 13 三证 + `0xad8f628` 5 面（绑
  `0xAD39884`）+ 395 残余。C. emit 同输入试验 go/no-go（kit `96eedc2` synced fixture +
  RELEASE reference）不变。
- **★会话六十一 = 696 定名导入批量手术 r61 出件待刷（2026-11-06，跳过 r60 直刷 r61）**：
  **口径定谳**：工作单 696/9,139 精确复现 = **r52 期 24A446 matcher map**
  （`out/m2381/ipados-27.0.1-24A446/m2381/kernel/kernel.map.json`，v11，
  inputs `38e6d9fa`=本地 donor）∩ noctf 同名同 VA 精确 ∩ r60 payload
  form-A donor 词。**★`out/m2381/kernel.{map,json}` = 23F84 旧线残留**
  （w40 时代，键空间 0xa76c000-0xb01d12c，inputs `0f7d50ac` ≠ 本地
  donor）——已弃用勿再引。**★词形两族定谳**：kext 导入面 = form-A 压缩
  词（`0x8011<<48|(VA−0x7004000)`，diversity=0）；diversified auth 词 /
  非 auth rebase 词 / 清零位 / 内核自体 fixup 位点 = emitter 管理态非导入
  面（r60 内实测 84/129/253/1,825 个，全分类落盘 surgery.json）。
  **手术 = `UniXNU-m2381/scripts/s61-payload-surgery.py`：695 导入 /
  9,134 面全重写**（donor 词 9,134 + sentinel 0；冲突 VA `0xad8f628`
  map=`_task_reference` vs 账本=`_task_reference_external` 5 面按规矩
  排除；账本外残余 `0xb47008c` IOService::publishResource 3 面 → s62
  队列）。门全过：计数 696/9,139 精确、native km 全在
  __TEXT/__TEXT_EXEC、seal 未动、diff 27,449B/9,135 簇**零杂散**。
  孪生抽验：body-hash 通道 74 条中 52 条 donor≡native 字节同，16 差异
  全为 allocator 相关 C++ 法（r55/56 补丁重编译所致，语义等价；版面
  稳定性由 4 个过墙符号背书，名单 `temp/s61-bodyhash-diff.json`）。
  **出件 `out/m2381/payload-r61.img`（99MiB）sha256 `1795d6bb…` 待刷**。
  账本 165→**812**（净增 647，双证 = matcher how + noctf 精确，体界
  (d') 约定 = 首 4 对齐 ret/retaa/retab 或离体 b 截断，金标准 10/10、
  165/165 raw-sha 复核）；kit 仓本地 commit `96eedc2`（主树子模块指针
  未动）；设备仓 `3bc999d`。
  **C 侦察 8 条**（`temp/s61-emitter-recon.md`）：live emitter = 设备仓
  vendored fork（feeds shadow 掉主仓 submodule，v11）≠ 上游 pin
  b1888c1；adapter branches 上游有我们无；fileset-reference 管线化只有
  我们有。
- **★下轮工作单 = 会话六十二（判读 + 决断轮；用户会前刷 r61）**：
  **刷机策略**：只刷 recovery_a ← `payload-r61.img`（dd 读回对拍
  count=99；现态 recovery_a = r59 `72cc9ef4`，iOS 链未被动）。回退：
  r60 `7da7e0b3`（一级）/ r59 `72cc9ef4`（二级）/ r58 `ca755a65`（三级）。
  A. r61 首刷判读：CRC 对拍（--autofix）→ 分流：
     a) 新哨兵墙（预期落 395 未定名集或 IOCrypto 13）→ 三证定名顺带修；
     b) 数据 abort / PAC → 疑 696 批量错孪生 → 16 条 body-hash DIFF
        名单（`temp/s61-bodyhash-diff.json`）优先排查 → 刷 r60 二分定位；
     c) 无墙推进 → 看 stage 轨迹 / vfsinit / root mount / SHLOG state=2。
  B. 遗留小批：`0xad8f628` 冲突裁决（`_task_reference` vs
     `_task_reference_external` 体哈希对质）+ `0xb47008c` publishResource
     3 面 + IOCrypto 13 三证 + 395 未定名集。
  C. emit 同输入试验：synced fixture（kit `96eedc2`）+ RELEASE reference
     跑 emit，报 stubbed/trap 未决集 vs 基线（build 期 1,850 stub ref /
     payload 823 sentinel 面），终点参照 sheng 终态 2 data + 24 trap →
     emitter 路线 go/no-go（侦察 D1 live-fork / D5 adapter-branches /
     D4 reference 管线化为决策输入）。
  D. r63 排单：go → emitter 全配置出件（构建期重绑取代 payload 手术线）；
     no-go → 手术线继续（IOCrypto 13 + 395 残余 + sheng 板级件收编）。

- **★会话五十九 = r59 首刷判读 + 墙 59 定谳（kalloc_type_impl_external，AppleImage4/AMFI 双家）+ r60 批量手术出件待刷（2026-11-06）**：
  **r59 首刷判读**（payload-r59-72cc9ef4）：crc `0x719ca2b6`（autofix 补
  from/x0 各一个前导 f，零数字错），124B，t=1s，hit #1。panic from
  `0xfffffff009023d34`（x0 `0xfffffff0080e16d0` x1 `0x4`）→ **墙 58 消实锤**
  （IOLockAlloc 134 槽 + OSObject ctor 292 槽全部生效，推进过 IOCrypto
  ctor）。**定谳 = caller com.apple.security.AppleImage4**（老 21H461 车道
  墙 10 同族！）：bl `0x903a698`（auth-GOT thunk）→ 槽 `0x80e1940` =
  哨兵 → donor import `0xad3e380`；x0 = **kalloc_type_view**（name 串
  `site.nonce_magazine_t`@0x751da8e + site 位图串@0x751daa4，kt_flags
  0xf8@+0x28，kt_size 0x64@+0x2c），w1 = 4 = zflags。**注意：本墙不在
  s58 预测的 IOCrypto 13 导入之列**——另一 kext（AppleImage4，AMFI 同调）
  的 kalloc_type 先撞线；IOCrypto 13 仍是下一梯队。
  **修 = donor `0xad3e380` 直绑 native `_kalloc_type_impl_external`
  `0xace8c8c`**（noctf `0x732cc8c` 符号精确、与 `_kalloc_type_impl` LTO
  别名同址；三证 = ①noctf 符号精确且紧邻 r58 已证 `_kfree_ext
  0xace8d18`（同 kalloc.c TU）②语义体形全对（0xd01000 常量对同对、
  zflags&7 折入同构 bfxil≡orr、ktv +0/+0x28/+0x2c 读法同、尾调
  `_kalloc_type_impl_internal` donor 0xadb2914 ≡ native 0xace9214 别名证实）
  ③stubbed 账本 AppleImage4+AppleMobileFileIntegrity 双家在册）。
  **★坐标系补充定谳：kernel.map.json `native` 字段 = noctf 空间 VA**
  （0xfffffff0 前缀），km VA = noctf + `0x39BC000`（s57 kfree_ext/
  s58 IOLockAlloc 双坐标验证）。
- **r60 = `UniXNU-m2381/scripts/s60-payload-surgery.py`：10 导入 44 槽**
  （墙 59 import 8 面 + matcher 账本（body-hash，noctf 交叉复核过）已定名
  9 导入 36 面：OSUnserializeXML 18 面 / IOFreeTypeImpl 8 面 /
  kauth_cred_nfs4domain2dsnode 3 面 / OSCollection::init、buf_biowait、
  buf_invalidateblks、cluster_unlock_direct_read、vfs_iterate、
  vn_authorize_unlink 各 1 面）。pre-image 白名单全中，diff 审计
  143B/43 簇**零杂散**，seal 未动（`__fips_hmacs`@0x4ec5fd 仍
  `6fc44336fd3f44b7`）。**出件 `out/m2381/payload-r60-7da7e0b3.img`
  （99 MiB）待刷**。账本 records 156-165 入
  `kits/XnuPrelink/fixtures/fileset-24A446-symbols.json`（165 条）。
- **IOCryptoAcceleratorFamily 残余哨兵导入（坐标已采未修，排在 696 批量与
  emitter 线之后）**：
  `0xb49c874` / `0xb4e7140` / `0xb4e670c` / `0xb4e6764` / `0xb4a6a20` /
  `0xb4a6ae8` / `0xb4a0528` / `0xb4a0580` / `0xb46c278` / `0xb46c2d0`
  （IOCEC 对）/ `0xad7e344` / `0xb29df7c` / `0xaec5fe0`（13 个，matcher
  账本全未定名，需逐个三证）。DC 区 sentinel 面 r60 后 = **823**
  （395 import；其中 696 已定名 import 的 9,139 面 = donor 词隐患，r61
  批量带走；两者不相交）。
- **★会话五十八 = 墙 58 定谳（IOLockAlloc 134 槽）+ OSObject ctor 翻案（292 槽全量）+ r59 手术出件待刷（2026-11-06）**：
  **r58 首刷判读**（payload-r58-ca755a65）：crc `0xfcd9b111`（autofix 仅补 x1
  一个前导 f,零数字错）,139B,t=1s,hit #1。panic from `0xfffffff00a0aac38`
  （x0 `0xfffffff00bb3de18` = IOCrypto kext 数据段静态对象 = 调用方 ctor 的
  this 残留,x1 `0xfffffff3facb9d20`）→ **墙 57 消实锤**（r58 的 453 槽 + 2 shim
  全部生效,推进进 IOCryptoAcceleratorFamily ctor）。**定谳 = donor
  `0xb4424c4` = IOLockAlloc 变体**（bti; adrp x8,0xb929000; ldr
  x0,[x8,#0x330]; b donor lck_mtx_alloc_init `0xad43134` = **弃参默认组包装**,
  IOLockAlloc() 无参 ABI）。调用链：IOCryptoAcceleratorFamily ctor
  `bl 0xa0ab660`(auth-GOT thunk) → 槽 `0x8313b60` = sentinel。
  **修 = 直绑 native `_IOLockAlloc` `0xb416a00`**（noctf `0x7a5aa00` 符号精确：
  ldr [0xb935450]; mov x1,#0; b native lck_mtx_alloc_init `0xaceffe8`（r58 已证
  孪生）; 邻位 `_IOLockInlineInit` 同全局）——免 shim。
- **★坐标系定谳（工具级）**：24A446 车道 slide = `0x1f000000000`（kc.py 注记
  本就对;本轮一度误判 0x100000000000 系手算翻车）。payload kernel.macho =
  donor KC 全局重定基,**kext GOT 位置等同**（donor km 偏移 == payload km 偏移）,
  屏 VA = payload VA 同空间。donor kernel exec 在 payload 里被 native kernel
  **整体替换**。
- **★翻案 = s57 "0xb3d385c 267 存活槽 donor 拷贝部分可用" 死**：payload
  `0xb3d385c` = native `_OSKextFreeSite` 邻域代码（fn@`0xb3d382c`）,donor 词槽
  指向我们 native 随机代码 = 潜在乱调,不是"可用拷贝"。→ **0xb3d385c（OSObject
  默认构造器,pacda isa 0xcda1 上下文）292 面（267 donor 词 + 25 哨兵）全量重绑**
  native `__ZN8OSObjectC2Ev` `0xb3aba18`（noctf `0x79efa18` + 0x39BC000;s45 墙36
  语义前例）。本 kext ctor 在 IOLockAlloc 之后紧跟 OSObject-ctor thunk
  （0xa0ab730 → 槽 0x8313bc8）,故同轮带上。
- **r59 = `UniXNU-m2381/scripts/s59-payload-surgery.py`**：134 + 292 = **426 槽**
  （IOLockAlloc 词 `0x8011000004412a00`;OSObjectCtor 词
  `0x80110000043a7a18`）,全部 pre-image 白名单（donor 词/sentinel）命中,diff
  审计 426 簇/1278B **零杂散**,seal 未动（槽在 collection __DATA_CONST,在 fips
  HMAC 盖的 entry __text 之外;`__fips_hmacs`@0x4ec5fd 仍 `6fc44336fd3f44b7`）。
  **出件 `out/m2381/payload-r59-72cc9ef4.img`（99 MiB）待刷**。账本 records
  154/155 入 `kits/XnuPrelink/fixtures/fileset-24A446-symbols.json`。
  **工具坑（本轮新增）**：手术偏移必须 +KHEAD `0x1c9800`（ESP 头）;
  bytearray 切片是拷贝（要 memoryview 原位写）;48 位压缩词手算必错,一律程序算。
- **IOCryptoAcceleratorFamily 残余哨兵导入（同一 init 路径,r60 第一梯队,
  donor 坐标已采）**：`0xb49c874` / `0xb4e7140` / `0xb4e670c` / `0xb4e6764` /
  `0xb4a6a20` / `0xb4a6ae8` / `0xb4a0528` / `0xb4a0580` / `0xb46c278` /
  `0xb46c2d0`（IOCEC 对）/ `0xad7e344` / `0xb29df7c` / `0xaec5fe0`（13 个;
  全部经本 kext thunk 表 0xa0ab710-0xa0ab920 可达）。
- **★下轮工作单 = 会话五十九（r59 首刷判读 + IOCrypto 残余 13 导入预研，安排于 2026-11-06 收队时，只写单不执行）**：
  **A. 首刷判读主线（用户刷 r59 `72cc9ef4` 后）**：
  0. 收屏；CRC 对拍（verify-panic-crc --autofix）；t/stage/hit# 与 r58 对拍。
  1. 读回自检表（上条 d 项）+ reader dd 读回对拍（dd 读回才算证据）。
  2. 分流：
     a. **无哨兵墙**（可能性低——本 kext 残余 12 导入大概率下一站）→
        IOLockAlloc + OSObject ctor 通关实锤,直接推进:fipspost POST 完整跑
        → PRNG 登记 → PE_init_iokit → AMFI（t≈32s IORTC 税）→ No-PMGR →
        PENDING 站表 0x924e77c/ef1c/ef5c。
     b. **新哨兵墙**（第一预期,caller 大概率仍落 IOCryptoAcceleratorFamily）
        → 照哨兵流程：bl 定位（尾跳类 = LR 可能保留上游）→ thunk → 槽 →
        donor 原词 → 三证定名 → 修;若 donor 词 ∈ 下面 B 单清单 = 预研已备,
        直接按钉死修法施工。
     c. **数据 abort / PAC 类** → 复核 shim 常量（kfree_ext `0xace8d18`,
        KHEAP_DEFAULT `0x7dec3e0`）与 braa 鉴权形（ESR EC 0x1C）。
  **B. 预研单（不等刷机结果,会话开头即可跑;s57→s58 模式）**：
  对 IOCryptoAcceleratorFamily 残余 13 个 sentinel 导入逐个三证
  （donor 坐标,全部经本 kext thunk 表 0xa0ab710-0xa0ab920 可达）：
  `0xb49c874` / `0xb4e7140` / `0xb4e670c` / `0xb4e6764` / `0xb4a6a20` /
  `0xb4a6ae8` / `0xb4a0528` / `0xb4a0580` / `0xb46c278` / `0xb46c2d0`
  （IOCEC 对,候选表旧行:体首 `mov w8,#1; str w8,[x0,#8]` 置标志+GOT 调用,
  186×2 面）/ `0xad7e344` / `0xb29df7c` / `0xaec5fe0`。
  步骤（每个）：
  ① donor 体形：bounds + disasm（temp/s58-wall.py 反汇编模式,donor exec
    fo = 0x3cb0000 + (va − 0xacb4000)）；下一步定体边界（下个 bti c /
    pacibsp,冷路径块算同函数——OSObject ctor 536B 前例）。
  ② noctf 孪生：体形扫描 + 符号表（temp/s53-noctf-syms.json,noctf VA =
    placed − 0x39BC000;先试符号精确命中再退体形搜索）。
  ③ 修法三分类：native 孪生直绑（词 = 0x8011<<48|(twin−0x7004000),程序算,
    勿手算）/ 需镜像 shim（页 padding 0xaef08xx 系）/ 记档不修。
  ④ 有钉死修法者做面数枚举（donor __DATA_CONST 词扫描,s59 脚本枚举器复用）,
    预写 r60 病灶表行。
  产出 = r60 候选病灶表增补（钉死修法 + 词 + 面数）,撞墙即施工。
  `0xae9bd20` 特别注：需 donor chained fixups 解析（donor 零槽 `0x7dffa68`
  blraa,文件即 0）——侦察成本高,排在 13 导入之后;信息真空先报成本再动。
  `0xb441738`（114 面 meta 表派发尾块）同样殿后:待屏证 x0/x1 才好定名。
  **C. 施工与出件（判读或预研钉出新修法后）**：
  s60-payload-surgery.py（s59 模板;坑:+KHEAD `0x1c9800`、memoryview 原位写、
  pre-image 白名单三分类、diff 审计零杂散、seal 断言未动）→ 读回自检表 →
  账本 records 156+（设备仓 kits/fileset-24A446-symbols.json）→ STATE §0 →
  双仓 commit+push → 刷写卡（常规迭代轮只 recovery_a,sha 门 + dd 读回）。
  **D. 成功旗标**：vfsinit → root/cryptex mount → SHLOG state=2。
  **E. 回退**：recovery_a ← r59 `72cc9ef4`（一级）/ r58 `ca755a65`（二级）。

## 1. 一句话现状（**2026-10-31 会话二十四收档：23F84 fileset 车道，matcher v11 判别子通道落地，战略拍板 = 关键路径优先**）：

**车道**：本机 Linux 全链出件（`./unixnu kernel|payload -D
../UniXNU-m2381`），donor = iPad17,1-4 / iPadOS 26.5.2 `23F84`
（xnu-12377.122.8 fileset KC，298 entries），native = 自建 xnu-12377
（arm64e PAC）。**内核已活**：每次刷机内核都在执行，panic 屏 =
fileset 未配调用的报告哨兵（fileset-unmapped-call.patch），现卡在
**AMFI early_security_init（root mount 之前）**。

**账本**：stubbed 654 目标/4848 站 → **601/4298**（会话二十四，
matcher v11 `pair_family_by_vtable_slot` = 槽判别子签名通道，69 配对
全语义自洽，设备仓 d0b94c0）；kext-facing named 1685→1736。
**战略拍板（2026-10-31 用户定"只想起内核→进桌面→完善驱动"）：
关键路径优先，不再追账本清零**——只修"到 root mount 为止"实际执行
路径上的墙，一刷验收整段；账本其余降级为背景（B.2 续单挂起）。

**最近墙**（按调用序出队；判读惯例 = CRC 验真→payload ESP 提
kernel→站点反汇编→batchid/vtslots 夹逼→三重验证）：第十六堵 =
SecureDTLookupEntry（已配）；会话二十四末新屏 = **0xafc09d8 =
SecureDTGetProperty**（9 站，AMFI /chosen 属性读取；donor 独立函数、
native 内联无独立符号，需 ~30 行 shim）。规矩：**关键路径上的坐标
当场修，路径外一律只记坐标**。

**出件**：kernel `18affaa2` / payload `936cd505`
（`out/m2381/payload-strncasecmp-936cd505.img`，fixture 166 笔；
ledger 3082→**3048 站**/847 目标 = 墙 21 −34 精确）**待刷**。
刷写红线不变：只 recovery_a；boot_b `d0200a6b` /
userdata `fd43ee70` 不动。下会话工作单 = §7b（墙 21 判读单已毕，
待刷后按调用序出队）。

## 2. 设备现状（2026-10-31 = **iOS 链现役**）

| 分区 | 件 | sha 前 8 | 状态 |
|---|---|---|---|
| boot_b | 本仓 UEFI（23F84 线配套） | `d0200a6b` | **现役**（不动） |
| recovery_a | payload vtslot | `2b8b8258` | **现役**（会话二十四已刷；会话二十五出件 `e5e9556f` 待刷换防） |
| userdata | CI 重建 iOS root | `fd43ee70` | **现役**（未动；源件 = `out/m2381/ci-artifacts/userdata-root.img` 8.8G，sha 同，勿删） |

- **读窗 = forensic reader**（专用件 `payload-reader-0f1013f4.img`；
  23F84 payload 本体 forensic=false 无 reader——ESP 超 recovery_a 已摘
  读者三件）。流程：fastboot → 菜单选 "m2381 forensic log reader" →
  `acm-reader.py probe/cmd/fetchchunk` → `read-m2381-ufs-log.py`。
  recovery_a 节点 = **sde27**（勿按 sda 猜）；ACM 通道推大文本会丢
  字节，一切比对走设备端 sha。
- **发卡规矩**：①发卡前确认设备现态（nura 态 = 三件全刷；iOS 态 =
  只刷 recovery_a）；②payload 只认平铺路径
  `out/m2381/payload-<名>-<sha8>.img`；③卡上必带刷前 sha256sum 门，
  dd 读回对拍才算证据。
- **ABL 重试预算**：同槽连败数次自动落 fastboot；恢复 =
  `fastboot --set-active=b`（留在 b、重置预算）。

## 3. 现役件与资产

- **构建**：`./unixnu kernel|payload -D ../UniXNU-m2381 -j16`（-D 在
  子命令后）；改 defconfig 必重跑 defconfig；改设备脚本必重跑 tree
  （staged kit = 设备仓 `kits/XnuPrelink` 的拷贝，改 matcher 后
  tree+kernel 即生效，map 按 matcher_signature 自动失效重建）。
- **payload 四元**：kernel `876a14c7` / trust =
  `out/m2381/ipados-26.5.2-23F84/m2381/original-trust-23F84.bundle`
  （ebeaf7fb）/ 种子 =
  `out/m2381/ci-artifacts/verified-root-grow/owner-data-fixture/scratch-data.img`
  （d04a0153，勿删）/ `--unique-chip-id 3645ad55d47e77c1`。kernel_args
  = `-v serial=3 debug=0x14e kt_fixed_zones=120 -no_slto_panic
  use_contiguous_hint=0 native-source-storage=1 native-ufs-root=1
  noidle=1 boot-progress=2 sheng-log=1`。panic screen + CRC 行上岗
  （zlib 0xEDB88320 over PANIC_TEXT 消息副本，判读先复算再钻）。
- **配对**：matcher v11（设备仓 `kits/XnuPrelink/fileset_match.py`，
  单测 27 例）；fixtures = 同目录 `fixtures/fileset-23F84-symbols.json`
  （137 函数 sha 钉）；批次工具 = `temp/batch-fixtures/`（batchid.py
  夹逼 / vtslots.py 判别子签名）。RELEASE 参照件已内建于 kernel 步。
- **donor**：iPad17,1-4 / iPadOS 26.5.2 `23F84`（IPSW 本地已核验
  `out/ipsw/23F84/`，全 zip sha f7f55fa3…，**留着勿删**）。
- **device repo**：`../UniXNU-m2381`（os_profile = ipados-26.5.2-23F84）；
  **UEFI**：`../uefi-m2381`。
- **诊断工具**：设备仓 `scripts/re-tools/`（cachesym/symbolicate/
  disasm…；cache 目录 `UNIXNU_CACHE_EXTRACT` 可覆盖，23F84 用户态
  符号化时需从 SystemOS DMG 重建 cache-extract，步骤见 memory
  dyld-cache-symbolication）。
- **阶段②预告（显示/驱动）**：=y LegacyDisplay 车道组件 = 主树
  `drivers/GopFramebuffer` + 设备仓 `scripts/build-native-legacy-display.py`
  （21H461 期施工，换代重生成见 docs/PLAN-display-lane.md）；GPU 固件
  已按 m2381 pin 落设备仓 `.cache/firmware/m2381-gpu/`（nura 侧供给
  实证，zap/sqe 与 sheng 件不同勿混用）。

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

- **root mount 前关键路径**（阶段①主战场）：AMFI early_security_init
  调用图上的 stubbed 目标（首个 = SecureDTGetProperty shim，§7）；其后
  apfs / IOStorageFamily root-mount 段未走到过，可能有新坐标。
- **root mount 后**：用户态（信任缓存/Data 卷/launchd）无先例区；
  donor 用户态与 donor 内核配套（23F84 全套），理论顺于老 20H392
  跨代错配，但纯探索。
- **B.2 续单（非关键路径，阶段②前按需）**：co-slot 群一致性通道
  （先配 IOUserClient::init 0xaf4e544 → 伴随槽锁类 → 收编 Dispatch×48
  + init×47×2）；大函数残墙放宽（0xaeb8eb0×190 copyProperty 候选、
  0xa95ca30×127、0xaf6236c×92）；0xafc1b20 PE 小壳身份。
- **GPU 接装**（阶段②）：固件/OPP/bus 表已备（§8 会话十八续档），
  缺 votes 头/配方去 xcrun/DTS/defconfig——显示破墙后主题化。

## 6. 不能声称的

- root mount 未通过：仍卡 AMFI early_security_init（panic 哨兵按调用
  序出队）。用户态未到达：信任缓存/Data 卷/launchd 全部未实测。
- 桌面/显示：=y LegacyDisplay 车道停留在 21H461 期（一级已实机实证但
  main display 注册链未通），未在 23F84 车道重验。
- matcher v11 通道只覆盖纯转发壳族；账本 601 目标大多数未配且**不构成
  当前目标**（关键路径优先拍板）。

## 9. 工作单 = 会话三十九（Mac 件首刷判读）+ §10 工作单 = 会话四十（crypto-funcs-unpack v2）——均已执行，全文移 `docs/archive/STATE-history.md`（2026-11-03 会话四十三按文档卫生常规移档）。

执行档副本：§9G = Mac 件首刷墙 30 判读（判读全文在 archive + git commit）；§10.X = crypto-funcs-unpack v2 出件（payload 24f0a941，已刷已判 = 墙 30 过，见 10.Y）。

## 7~10.Y.10 批量移档（2026-11-04 会话四十五收队，文档卫生常规）：§7（会话二十七
工作单）、§8 车道判读档案（会话十三→二十四）、§7a~§7i + §7j/§7k（会话二十八~
三十八工作单与执行档）、§8'（会话三十六工作单）、§10.Y~§10.Y.10（会话四十一~
四十五全部开工单与执行档）均已执行，全文移 `docs/archive/STATE-history.md`
（2026-11-04 批次，原样保留）。正文只留现势区（§0~§6）、§9 存根与本轮
10.Y.11。

## 10.Y.11 会话四十五续 = 墙 36 首刷判读 + 墙 37 施工档（2026-11-04，
kernel `6d4a9f19`/payload `e6b06399` 待刷，屏摄设备仓
records/mackernel-w36fix-b9e733be-firstboot-20261104-020733.jpg）

- **墙 36 修复真机生效（fixture 判读通过）**：旧报点
  `0x…977dd00` 不再报；stage 轨迹保持全亮（PE_lockdown_iokit → … →
  kernel do post - done）；pages=0x125b07 / free=0x1251c7（+1 同水位
  = 走过旧墙后即刻撞下一站）；hit #1。
- **消息定稿**：crc `0x1fa14f06` 一次精确命中（591,872 候选唯一命中，
  **零纠正**；配方先复现上轮 0xee584f8d 自校通过）。x0 =
  `0xfffffff7863fe090`（f7 前缀，非上轮 f8 heap 段）；x1 =
  `0xf5db14b0097807a0`（PAC tag 指针）。
- **新墙 37 = 同族 unmapped-call**：报点 LR `0x…977fefc`（bl 站
  donor `0x977fef8`）；caller = **CoreAnalyticsFamily** start 装配
  大函数（`0x977fd14..0x97802ac`，1432B；与旧墙 36 caller 同 kext
  不同函数且更晚）；target = donor **`0x0af0f77c` =
  `IOTimerEventSource::timerEventSource(OSObject*, Action)`**（二参
  工厂重载；j 重载已排除：常量 1 作 init timeout + caller 只传 2 参）；
  构造链叙事完整：基类 ctor（墙 36 已修）→ workLoop → commandGate →
  timer 工厂（墙 37）。未配成因 = donor 体内联差异致 body-hash 失败
  且无字符串/vtable 证据 = matcher 正常漏网（非"已配却报"）。
- **红利面 = 321 直调 bl + 100 fixup 槽（约 100 entry，
  **最大单笔 fixture**；榜：IOAccessoryManager 26/AppleHPM 19/
  ApplePearlSEPDriver 19/AppleSPU 10…）；CoreAnalyticsFamily 自身
  4 站，同 caller 内还有 0x977ff40/0x977ff88 两站（钉名后同函数继续
  推进，预期行为）。
- **墙 37 fixture 落位**（设备仓 `bdd72fd`）：↔ native
  `__ZN18IOTimerEventSource16timerEventSourceEP8OSObjectPFvS1_PS_E`；
  native 地址 = Mac noctf（7307aecf）`0xfffffff007aab9e0` / Linux
  `0x07aeedbc`；四门双车道全过（donor 区间 size=284 sha 一致；双
  reviewed；make_fixture check 255/255）。native 形态逐项同构（0x60
  typed alloc/vptr 0xcda1/instanceConstructed 内联/vtable[+0x140]
  PAC key 0x5f4/失败路 0x3a87）。
- **Mac 重建 + 静态门**（增量重编 ~4min，ninja 热缓存；**本轮零锚
  漂移**：native 未变，reviewed 0 drift）：kernel `6d4a9f19`/payload
  `e6b06399`，存档 `out/m2381/payload-w37fix-6d4a9f19.img` + kernel
  同名存档。w35-gate 双件 PASS（resign 0/dispatch 371）；map 9896 对
  （+6：reviewed +1 + auto 回收 5；kext_targets_named 2048→2051）。
- **站点重定向 + 红利静态对账**：新件 bl `0x…977fef8` →
  `0x0af1f9e0` = native timerEventSource 终局地址（noctf 原体骨架
  0x60B 归一化逐位同）；直调 bl 实换靶 26 站（哨兵 1228→1203）；
  closure_pointers_unmapped 314→**311**（−3）、kext_pointers_rebound
  47493→47496（+3）；branches/kept 持平 = 零附带漂移。其余 321−26
  直调站在被 drop 的 entry 内。
- **回退**：recovery_a ← `payload-w36fix-b9e733be.img`（ce3df35f）；
  b63ad26e 二级；abe3ec85 三级。

## 10.Y.12（会话四十六，已执行移档）→ 全文在 `docs/archive/STATE-history.md`

## 10.Y.14 会话四十八 = 墙 40 定谳 + 一字补丁修复出件（2026-11-04，Mac ssh 全程，未刷机）

- **判读**：屏摄（IMG_20261104_061101）= PAC failure from kernel with
  IA key while branching to x8 at pc `0xfffffff0084ca4fc`，lr
  `0x62df8a700aeea59c`（签名值），esr `0x72000000`，stage 全亮维持
  （kernel do post - done 后），pages `0x125b08`/free `0x1249c8`。
  CRC 复算 = 多行 PAC 新格式未覆盖（见 §0①），**结构验证法**定谳：
  donor `0x84ca4fc: blraa x8, x17` 与屏报逐字吻合。
- **机制（全链实证）**：`start()+0x48` = `adrp g 页; ldr [g,#0xfd8];
  add x9,x8,#0x2c0; ldr x8,[x8,#0x2c0]; mov x17,x9; movk x17,#0x3c68;
  blraa x8,x17`。donor 全镜像扫描：g 全局唯一读者 = 本站、表页
  donor 文件值全零（BSS）、零 fixup、零写入者（kernel+kext 全部
  298 entry fixups 解析零命中）——**表 = 混合内核的 native
  __DATA_CONST**（kernel entry = donor text + 我们 data）。final 页
  节点 = auth IA + addrDiv、div=0x4529、target=final `0xaee785c` =
  noctf `0x7a7385c` = `__ZN9IOService4openEPS_jPv` twin（donor
  `0xaec90cc`）。donor 读者标签 `0x3c68` vs 我方槽 div `0x4529` →
  FPAC。两标签在 donor movk 站点各 254 处 = 工具链标签漂移
  （donor 23F84 clang vs 我们 26.5.2 构建）。**移位 +0x10 假设三读者
  独立吻合但被否决**（会自断我们自己的读者）；语义重配对挂账。
- **修法**：kit `TEXT_WORD_PATCHES` 一字通道（旧字钉死 fail-fast）+
  首条目 `0x84ca4f8: 0xF2E78D11→0xF2E8A531`（movk imm16
  0x3c68→0x4529）。单测 10+11 OK。scp Mac → kernel 重建（tree 自动
  重暂存，**无需手动 tree**，本轮实证）→ 补丁字 fo `0x14c64f8`
  字节级验真。静态门 resign **0** / dispatch **371** 持平 PASS。
- **出件**：kernel `96422b08`（candidate）/payload `a9bf924f`，双存档
  `out/m2381/payload-w40fix-96422b08.img` + kernel 同名。回退链见 §0。
- **挂账**：①PAC 格式 CRC 转录验真（PANIC_TEXT 副本 + \t/\n→空格）
  ②移植表 per-slot 语义重配对 ③同族审计（final 表页 div 清单已记
  §0②）④ApplePMGR 21 targets。

## 10.Y.13 会话四十七 = 墙 38 首刷判读 + 墙 39 定名 + 池批量 18 笔清仓（2026-11-04，
payload `2f31a6bd` 全门验讫待刷；屏摄判读现场 = 本节判读部分）

- **墙 38 真机通关（fixture 判读通过）**：旧报点 `0x…84ca3bc`（ctor
  簇 IOCPU C2 克隆）未复现；本轮回扫 ctor 簇 0x84ca200..0x84ca4b4
  实测 5 直调站 `0x84ca268/29c/3b8/424/47c` 全打 `0x0af3432c`
  （= fixture 覆盖面精确兑现，含 GOT 槽共 6 站全灭）；stage 轨迹
  全亮维持；pages 0x125b07 / free 0x1249c7（比 w37 boot 再深
  ~2046 页 wired，前沿继续推进）；hit #1。
- **消息定稿 + CRC 配方再校准**：先以墙 38 已知对（139B /
  `0x8a2bf81e`）复算配方确认无尾换行；本轮报文 113B →
  **`0x50d36e66` 零纠正命中**（屏显 17 位 = 9 前导零补齐）。
  `from 0xfffffff0084ca524 (x0 0x1 x1 0x10001)` 全真——x0/x1 为
  残寄存器（无参 getter 忽略实参），与签名自洽。
- **新墙 39 = 同族 unmapped-call**：报点 LR `0x…84ca524`（bl 站
  donor `0x84ca520`）；caller = **`AppleARMCPU::start`**（donor
  `0x84ca4b4..0x84cad7c`，2248B；ctor 在 0x84ca4b4 前已完整通过）；
  target = donor **`0x0a963a34` = `ml_get_cpu_count`**（16B getter，
  读 `topology_info.num_cpus`）。定名三证：①体形家族互证：同 TU
  donor `_ml_get_boot_cpu_number 0xa963a44` = `boot_cpu->cpu_id`
  解引、`_ml_get_cpu_number 0xa963a58` = phys_id 循环返下标 i，
  与 osfmk/arm64/machine_routines.c:1391/1415 逐句同；②KDK
  26.5.2_25F84 地址序 `cpu_count→boot_cpu_number→cpu_number` 与
  donor 逐位同；③调用点 `str w0,[x19,#0xec]` + 同函数尾部
  last-CPU 屏障 `OSIncrementAtomic()+1 == [x19+0xec]` = CPU 数
  语义闭环。
- **关键路径清障（start() 全 callee 对账）**：mapped 12 / **unmapped
  6**。顺手定名 3 个（全部 machine_routines.c 同 TU，KDK 尺寸/
  地址序双夹逼）：`0xa963aa0 = ml_get_cluster_number`（0x6c 循环
  体返 `cpus[i].cluster_id`@+8；结构体 osfmk/arm/cpu_topology.h
  字段序钉死）；`0xa963bc4 = ml_get_max_cpu_number`（16B 读
  `[g+0x280]` = base+8 = max_cpu_id；base = g+0x278 由 cpus/boot_cpu
  双锚反推）。**挂账 1 个 = `0xaedab0c`**（3 站）：0xbc 包装器
  `gObj->vt+0xb8(arg0, arg1?:[g0xb376cf0])` + tpidr 跨 CPU 计数 +
  尾调 `vt+0x2a0(0)`；donor map 邻名 = IOServicePH::systemPowerChange
  但 native 全镜像 csel 指纹扫描零孪生（donor 122.8 版本增量），
  不硬凑——它是 last-CPU 屏障（start() 尾 0x84cad54/6c 双 bl 夹
  sched_cpu_init_completed），SMP 全核起来才触发，下轮墙照带
  x0/x1 定名。
- **池批量 18 笔 fixtures 全落位**（256→274，四个 spec 批次，
  子代理三路扇出 + 主线 1 路）：
  - **ml 族 ×3**（主线，见上）：`0xa963a34→_ml_get_cpu_count`、
    `0xa963aa0→_ml_get_cluster_number`、`0xa963bc4→_ml_get_max_cpu_number`。
  - **vtable 槽位法 ×4**（子代理，4/4 复核 EXACT）：`0xaf32ddc→
    __ZN5IOCPU11setPropertyEPK8OSSymbolP8OSObject`、`0xaf32dc8→
    __ZNK5IOCPU11getPropertyEPK8OSSymbol`、`0xaf32d0c→
    __ZNK5IOCPU19serializePropertiesEP11OSSerialize`、`0xaf32bdc→
    __ZN5IOCPU13setPropertiesEP8OSObject`（各 2 fixup 槽 0 bl；
    ptrauth diversifier 活代码交叉验证；IOCPU.h:77-80 声明序
    对读）。
  - **IOPlatformIO/IODT ×3**（子代理）：`0xaf329cc→
    __ZN12IOPlatformIOC2EPK11OSMetaClass`（3 站）、**`0xaf329f8→
    __ZN12IOPlatformIOD2Ev`（翻案：16B = 死 vptr 毒化
    `mov x8,#-1; str x8,[x0]`，非 C1 thunk；native D2 = 尾调
    super no-op 链，-1 只写不读，重定向安全）**、`0xaf31f80→
    __ZN18IODTPlatformExpertC2EPK11OSMetaClass`（6 站）。
  - **IOCPUInterruptController 簇 ×8**（子代理）：`0xaf34670→
    __ZN24IOCPUInterruptControllernwEm`（kalloc.type operator new，
    **zone 名串 "site.IOCPUInterruptController" + kt_size 0xb8**
    铁证；关键路径：start() 0x84ca818 `mov w0,#0xb8` 前置）、
    `0xaf346e0→__ZN24IOCPUInterruptControllerC1Ev`（0x84ca820，
    Itanium new-X 双连）、`0xaf34770→_IOCPURunPlatformQuiesceActions`
    （队列 +0x20）、`0xaf347f8→_IOCPURunPlatformActiveActions`
    （队列 +0x30；donor 无 hibernate bl = LTO 消除，配置性形态差）、
    `0xaf32aec→getMachProcessor / 0xaf32af8→getCPUGroupSize /
    0xaf32b28→getCPUGroup`（vt 槽 +0x5b0/+0x5a8/+0x5a0 声明序夹逼）、
    **`0xaf3435c→__ZN5IOCPUD2Ev`（再翻案：C1 假设推翻——唯一 bl 站
    0x84ca2e8 在 `AppleARMCPU::D0Ev` deleting-dtor 序列内（bl dtor
    → `_OSObject_typed_operator_delete(ktv,this,0xf8)` 尾调，主线
    独立复核成立），另有 0x84ca2c8/2d0 两条 8B D1/D2 veneer**）。
  - **池身份定谳（背景发现）**：`0xaf32xxx–0xaf34xxx` 整簇不是匿名
    kext 池，是 **donor kernel-proper 的 iokit 代码**（IOCPU.cpp +
    IOPlatformActions.cpp，exec 段尾部），native 同源同版全部有
    twin 符号——解释了本批为何全部一次命中。上轮"super =
    OSObject 级 gMetaClass 0xb3771e8"修正：**0xb3771e8 = IOService
    gMetaClass**，IOPlatformIO/IOCPU 共同 super = IOService
    （IOPlatformIO.h:42）。
- **★工具坑（本轮踩实）**：`make_fixture.py` 的权威区间依赖 kit
  的 `kc_image`/`fileset_match.Side`，无 `PYTHONPATH=
  UniXNU-m2381/kits/XnuPrelink` 时静默降级 end-scan，本机算出
  `0xaf32af8` size=60 ≠ matcher 函数表 48 → Mac `anchor_reviewed`
  当场拒件（`Reviewed function bytes changed`）= fail-fast 设计
  有效。修复 = 摘 18 笔带 PYTHONPATH 重入（17/18 区间本就一致，
  仅 0xaf32af8 60→48）。§0 有备忘。
- **Mac 重建 + 静态门**（kernel+payload 全链 BUILD_EXIT=0）：kernel
  `2f31a6bd`（candidate_sha256）/payload `426ae6ce`，双存档
  `out/m2381/payload-w39fix-2f31a6bd.img` + kernel 同名（本地已拉
  回同 sha）。**native 本轮有布局漂移（noctf 整体 −0x193F4）**，
  map 全部从本 build noctf 重推导内部自洽；map reviewed **274/274
  全绑**（+18 精确）；functions named 9896→**9914**；kext-facing
  named 2051→**2065**；unmapped closure 311→**308**；stubbed
  functions 521。**站点重定向字节级实测 ×5**：bl 0x84ca520→
  `0xa9accb8`（= ml_get_cpu_count 终局，`bti c; adrp; ldr
  w0,[g,#0x664]; ret` 逐句同构）+ cluster_number/max_cpu_number/
  icu new/icu C1 全验，终局镜像里 topology 双偏移 num_cpus@0x664 /
  max_cpu_id@0x668 相邻对上。静态门 `gate-s47.py`（新写，w37 件
  校准）：resign **0** / dispatch 对 **371** 持平 PASS。
- **回退**：recovery_a ← `payload-w38fix-b6bfe74b.img`（c4cb22fb，
  已证通关墙 36~38）；e6b06399 二级；ce3df35f 三级。

## 11. 换道工作单 = 会话五十一：os_profile → ipados-27.0.1-24A446
（2026-11-05 用户拍板；侦察对账档 = 记忆 `sheng-24a446-recon-ledger`；
上游实证实录 = sheng porting.toml：SpringBoard PID 30 / backboardd 61 /
wifid 49 / 触控 / WiFi device-verified，视觉呈现未验）

**会话五十二排程（2026-11-05 定）**：本单步 0~7 一次跑完（换道施工全链
出件，payload-r52 待刷）；首刷+判读 = 会话五十三。**步 0（可选，
不阻塞）**：w42fix 屏摄轻判读——decode `0xa5b8db4` 站 bl → 目标 →
23F84 map 定名被调内核函数；donor→内核按名绑定，该函数名大概率 =
24A446 首刷第一面同型墙，提前备 fixture/spec 省 one round。**Mac 侧
准备**：24A446 IPSW 需上 Mac（`scp` 过 LAN 或 Mac 直接下载同 CDN URL），
替换/并存 `UNIXNU_IPSW` 指向（23F84 件保留勿覆盖，回退车道要用）。

**IPSW 到货复验（已毕 2026-11-05）**：
`research/ipsw/iPad17,1,iPad17,2,iPad17,3,iPad17,4_27.0.1_24A446_Restore.ipsw`
—— 字节数 11,654,176,243 与 CDN Content-Length 逐位同；Restore.plist =
24A446/27.0.1 ✓；`kernelcache.release.ipad17`（24 MB）在位，raw 成员
`802b495b…`，profile pin `38e6d9fa…` = 解码后 sha（由
`scripts/unixnu/kernel.py:334` 在 fetch/kernel 步硬门验）；整包 sha256
`fa93b653cc03c80ee5009302545479cb3b0f7bbaaa43f43a25e70cc328575b7a` 存档。

**步骤**：

1. **profile 切换**：设备仓 `board.toml` `os_profile = "ipados-27.0.1-24A446"`；
   IPSW 本地喂 `--ipsw research/ipsw/…ipsw` 或 `UNIXNU_IPSW`。铁律：改
   board 必重跑 `./unixnu defconfig m2381_defconfig`。
2. **fetch + sha 门**：`./unixnu fetch`（KernelCache / DeviceTree /
   StaticTrustCache / Cryptex1,SystemTrustCache；`kernelcache_sha256`
   硬门必过，不过即停）。
3. **we-only 8 patch 逐条 24A446 重验**（对 24A446 donor kernelcache 找
   同站/同病灶，结论 = 保留/改站/退役，逐条入 porting.toml）：
   `fileset-amfi-devmode-shim`（预计退役换步 4 板级方案）、
   `prng-getentropy-donor-discriminator`、`crypto-funcs-probe`、
   `crypto-funcs-unpack`、`mac-policy-ops-donor-resign`、
   `sysctl-handler-donor-resign`、`kext-entry-donor-pptr`、
   `fileset-read-erandom-shim`。
4. **板级 PMGR 包（m2381 PlatformPower，第一版）**：抄 sheng
   `package/ShengPlatformPower.cpp` 模式——`lookup("ApplePMGR")` /
   `lookup("ApplePMGRNub")` 注册仿制 metaclass + `serviceMatching`
   personality 发布 nub 面（AMFI `waitForMatchingService` 命中即安）；
   PMIC/Rpmh 实质功率管理后续轮补。放设备仓 `package/` + 构建登记。
5. **kit**：gitlink 保持 `b1888c12`（= sheng startup-pipeline tip，含
   fileset emitter function/data maps + 24A446 startup import/abs-branch
   修复）+ 本地 matcher/TEXT_WORD_PATCHES 提交；RELEASE 参照件必须
   `--lto`（sheng 教训：packaging success ≠ boot compatibility；
   corecrypto kprintf rebind 即此翻车）。
6. **ADT/identity 平移**：iPad17,3/J820AP 不变；`/chosen`
   `kernel-ctrr-to-be-enabled=0` 双保险属性随迁（墙 41 对策）；24A446
   ADT 结构差异由 payload 步自检报。
7. **构建链**：defconfig → fetch → tree → kernel（xnu-12377.121.6，
   darwin26 系列 4 条 OS patch 自动进 series）→ firmware → payload
   （三红线显式旗标）→ 刷 recovery_a only（boot_b/userdata 不动）→
   `set_active b` → reboot。
8. **首刷判读预期**：AMFI 静默（板级 nub 或 shim 命中）→ t≈32s IORTC
   税 → vfsinit → root/cryptex mount → **SHLOG 窗若 state=2 开始写 =
   大捷旗标**（落盘通道 root mount 后才开）→ launchd/SpringBoard 站。
   判读 = 屏摄；新墙照旧分流（定名三证 → 通道选择 → spec → 一重建一刷）。
9. **回退链**：recovery_a ← `payload-w42fix-3fe101ba.img`（`6436442d`）
   一级；w41fix 二级；w40fix 三级。
10. **风险**：①23F84 地址簿 fixture（311 closure）全量作废重做，新
    emitter 吸收大头；②`--allow-missing` 语义仅限 sheng 已容忍面
    （2 data refs + 24 trap refs 级别），超出即按墙处理不放过；③24A446
    的 `early_security_init` AMFI wrapper 地址与 `roots` 清单以 profile
    pin 为准（`0xfffffe00092551ac`），若 mismatch 先查 profile 再查件。
