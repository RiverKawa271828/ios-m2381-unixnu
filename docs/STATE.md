# STATE — 现势状态（m2381，2026-10-28 更新）

## 0. 现势指针（每轮首更，2026-11-04 会话四十五续）

- **现役件（已刷待判读）**：`payload-w38fix-b6bfe74b.img`（payload
  `c4cb22fb`，kernel `b6bfe74b`，墙 38 fixture 件，10.Y.12 全门验
  讫）**用户已刷 recovery_a，待首刷屏摄判读**。在机 = w38fix。
- **下轮工作单 = 会话四十七：①首刷判读 + ②恢复批量修复（用户
  2026-11-04 拍板）**：
  ①判读 = 墙 38 消（不再报 `from 0xfffffff0084ca3bc`）+ 同簇 4 站
  （0x84ca268/29c/424/47c 同靶直过）+ stages 推进 + 新墙按类分流。
  ②**批量 = AppleARMPlatform 池 15 key/32 站**（B 普查地图在
  10.Y.12），两种已验证机械定名法：
  - **vtable 槽位对位法（本轮已验证 4/4 EXACT）**：donor
    `__ZTV<Class>` 槽 N ↔ native 同名 vtable 槽 N → native 符号；
    override 簇四站已得靶：`0xaf32ddc`↔`IOCPU::setProperty(OSSymbol*,OSObject*)`、
    `0xaf32dc8`↔`IOCPU::getProperty(OSSymbol*)const`、
    `0xaf32d0c`↔`IOCPU::serializeProperties(Serialize*)const`、
    `0xaf32bdc`↔`IOCPU::setProperties(OSObject*)`（native 槽序 =
    noctf `__ZTV5IOCPU`+0x10，chained-fixup 低 32 位 = 镜像内偏移，
    donor 基 `0x7004000`/native 基 `0x7004000` 同式解码）。
  - **ctor 克隆法（墙 36~38 三连验证）**：`0xaf329cc`（IOPlatformIO
    实例 ctor 克隆，3 站）↔ native `__ZN12IOPlatformIOC2EPK11OSMetaClass`
    （Linux noctf `0x07b1135c`，Mac 待查）；`0xaf3435c`（4 站）=
    donor IOCPU canonical，**C1 vs D2 判别待做**（看调用方构造/析构
    上下文；native C1/C2 已合并则同靶 C2）；`0xaf329f8`（IOPlatformIO
    canonical 16B，4 站）；`0xaf31f80`（IODTPlatformExpert ctor 克隆，
    5 站，池边）；`0xaf32aec/af8/b28` + `0xaf34670/6e0/770/7f8`
    （IOCPUInterruptController 簇大概率，同法）。
  - 批量纪律：每 entry 走 spec→make_fixture add→check；evidence
    三证（槽位/体形/调用上下文）；双车道 noctf 对照；**一次重建
    一刷**（w38 已证同类条目真机行为）。
  ③延伸簇：ApplePMGR 21 targets（关键路径）= 池清后下一个侦察
  对象；IOHIDFamily 46 / apfs 137 targets 在后（apfs = root 挂载
  大户）。
  回退 = recovery_a ← `payload-w37fix-6d4a9f19.img`（e6b06399）；
  ce3df35f 二级；b63ad26e 三级。
- **车道**：Mac 主攻（ssh `naoniudeair@192.168.10.246`，工作面
  `~/unixnu/`；reference 已接线 sha `5a794da8`；payload 三红线输入
  常驻；防休眠管线 = `temp/refswap/w36-pipeline.sh`——**变量必须用
  Mac 绝对路径**，`~/` 会被本机展开成 Fedora $HOME）；Linux = 参照件
  供给 + 回归门 + 离线重放。
- **未推**：主树 `7061a05`+`830893e`+`2c3371c`+`8ba8336`+`58dec46`+
  本节 docs（§0+§10.Y.12）；设备仓 `155d3c3`+`6156dd4`+`beef9fe`+
  `bdd72fd`+`4706e64`（墙 38 records）+`899bfae`（墙 38 fixture）。
  Mac 树 = scp 新态（fixture `de0ea7af` 已同步 + 出件双存档在
  Mac `out/m2381/`）。
- **文档卫生常规（2026-11-03 用户拍板，每轮执行）**：①本指针每轮首更
  ②已执行工作单段落移 `docs/archive/STATE-history.md`（正文只留指针）
  ③未推清单与回退链核对 ④`temp/` 内 >10MB 二进制隔轮即清（本轮清
  `w36-fixture/mac-native-noctf.macho` 24MB 与 /tmp 出件中间件；
  脚本/JSON/map 留）⑤记忆索引超 24KB 即剪钩子。**红线：docs/ 的上游
  文档站树（concepts/guide/zh/reference/public）不清理不改动。**
- **参考链状态（2026-11-04 会话四十五续刷新）**：donor KC sha ✓；
  fixtures 256/256 字节验真 ✓（255 + 墙 38）；reference 换件生效
  （Mac map reference sha `5a794da8` ✓）；24A446 policy 休眠 + KC
  护栏 ✓。

> **历史全档**：2026-10-24 → 2026-10-27 全部轮次（§0.0–0.34）与当时静态区
> 在 `docs/archive/STATE-history.md`（原样快照）；每轮定谳同时完整活在
> git commit message 里。本文档 = 精简现势版：现状 / 资产 / 方法 / 开放
> 问题 / 下一步 / 轮次索引。

> **2026-10-29 结构变更：设备仓拆出独立仓**
> [UniXNU-m2381](https://github.com/RiverKawa271828/UniXNU-m2381)（本地
> `../UniXNU-m2381`；`git subtree split` 保 93 提交全史，独立实跑
> check-m2381 = 4 ok 0 fail）。本文与历史档中所有
> `devices/m2381/unixnu-m2381/…` 路径现指该仓对应路径；构建/取证命令
> 改为 `-D ../UniXNU-m2381` 与 `../UniXNU-m2381/scripts/…`；CI dispatch
> 改指新仓（见 `docs/CI.md`）。**同日续**：m2381Pkg 收编新 UEFI 仓
> `../uefi-m2381`（= 上游 uefi-sheng `sheng@0eb4514` 完整克隆 + overlay
> 提交 `5297334`；实证来源 = nura 链；Binaries pin `036ba9f7` mars；旧
> Mu-Silicium fork 封存；sync.sh 与 devices/m2381/uefi 退役）；三仓收拢于
> 工作区根 `/home/ri/work/unixnu-meizu`（老主树路径留 shim 符号链接）；固
> 件构建 = `--uefi ../uefi-m2381`。

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

## 10.Y.12 会话四十六 = 墙 37 首刷判读 + 墙 38 施工档（2026-11-04，
payload `e6b06399` 首刷判读毕，屏摄设备仓
records/mackernel-w37fix-6d4a9f19-firstboot-20261104-023838.jpg，
records `4706e64`）

- **墙 36 + 墙 37 双墙真机通关（fixture 判读通过）**：旧报点
  `0x…977dd00`（墙 36，OSObject ctor 克隆）与 `0x…977fef8`
  （墙 37，timerEventSource）均未复现；stage 轨迹全亮收官
  （PE_lockdown_iokit → … → kernel do post - done）；pages=
  0x125b05 / free=0x1251c5（wired 2368 页 = 前沿深入新区域，非同
  点 +1 水位）；hit #1。
- **消息定稿**：crc `0x8a2bf81e` 一次精确命中（139B，**零纠正**；
  格式串 = `fileset-unmapped-call.patch` 的 panic
  `from %p (x0 0x%llx x1 0x%llx) hit #%u`，照片 x0 掉一个 f 由
  crc 命中裁定）。x0 = `0xfffffff2d947bb00`（正构造中的
  AppleARMCPU 堆对象）；x1 = `0xfffffff00b4afe88`（= AppleARMCPU
  gMetaClass，与 ctor 语义自洽）。
- **新墙 38 = 同族 unmapped-call**：报点 LR `0x…84ca3bc`（bl 站
  donor `0x84ca3b8`）；caller = **AppleARMCPU::AppleARMCPU()**
  构造器（donor `0x84ca388..`）；target = donor **`0x0af3432c` =
  `IOCPU::IOCPU()`**（更准确 = `__ZN5IOCPUC2EPK11OSMetaClass` 的
  meta 特化 ICF 克隆，meta 常量折叠后 12 条指令零调用）。定名
  三证：①调用方拼图：`mov w1,#0xf8`（instanceSize 0xf8）→
  `_OSObject_typed_operator_new` → 基类 ctor → 立即自写 vptr
  覆写 → instanceConstructed(`0xb4afe88`)；0xf8 与 AppleARMCPU
  metactr `mov w3,#0xf8` 精确对拼；②V1 = IOCPU vtable
  `0x7d6cde0`：getMetaClass 槽（vt+0x38 @`0x7d6ce18`）→
  `0xaf32df0` 返回 `0xb375cc8` = "IOCPU" metactr（`0xaf34258`）
  的 this；③克隆体形态 refcount=1 + vptr（pacda 0xcda1）+
  reserved 0x88=0，区别于墙 36 OSObject 克隆 `0x0af07900`（无
  str xzr、vtable `0x7d9e2d0`）。
- **修法 = fixture 重定向**（设备仓 `899bfae`）：↔ native
  `__ZN5IOCPUC2EPK11OSMetaClass` @ noctf `0xfffffff007b11c8c`；
  native 链 = IOCPU→IOService→IORegistryEntry→OSObject(C2 带
  meta)→OSMetaClassBase 五层，终态 {[x0]=vtable, [x0+8]=1,
  [x0+0x88]=0} 与克隆逐字段同（refcount 在 OSObject 层写 1，
  reserved 在 IOCPU 层清 0）；**x1 全链零使用**（调用方传
  AppleARMCPU meta 无副作用）；native 无 C1/C2Ev 重载（C1/C2 已
  ICF 合并，与 donor 克隆同源互证）；native vtable 值瞬态无害
  （调用方 ctor 即覆写）。fixture size=48 sha `59f22f9a…`，
  256/256 check 全稳；spec = `tools/xnu27/spec-session38.json`。
- **红利面（终数，A/B 双代理 + 账本三方一致）= 5 直调 bl + 1 fixup 槽
  = 6 站**：bl `0x84ca268/29c/3b8/424/47c` 全在 AppleARMCPU ctor 簇
  （C2/C1/alloc 路径×3）+ GOT 槽 `0x7e07bd8`；账本 key
  `0xfffffff00af3432c` sites=6 精确对拼（kext =
  com.apple.driver.AppleARMPlatform）。**克隆族全景（全 donor 结构
  扫描）**：48B 带 str-xzr 形态全镜像唯一 = 本条目，零兄弟；112 个
  44B 无-xzr C 族克隆散布全池（0x0af07900 = 已配 OSObject 成员）。
  邻近池 `0xaf32000..0xaf35000` 余 15 key/32 站未清（IOCPU canonical
  `0xaf3435c` 4 站、IOPlatformIO ctor 克隆 `0xaf329cc` 3 站、
  IODTPlatformExpert 克隆 `0xaf31f80` 5 站…）= 后续墙候选，本轮
  按需清仓纪律**不批量**。修正：IOPlatformIO（0x88）与 IOCPU（0xb8）
  同根（共同 super = OSObject 级 gMetaClass `0xb3771e8`）**非父子**；
  donor map 无 IOCPU/IOPlatformIO canonical 符号（匿名池块）；我们
  native 有 `__ZN12IOPlatformIOC2EPK11OSMetaClass`（Linux noctf
  `0x07b1135c`）= 后续池墙的现成靶。
- **Mac 重建 + 静态门**（增量 ~4min：kernel 3.5min + payload 34s；
  **native 未变零锚漂移**）：kernel `b6bfe74b`/payload `c4cb22fb`，
  存档 `out/m2381/payload-w38fix-b6bfe74b.img` + kernel 同名（本地
  已拉回同 sha）。map reviewed **255→256**（墙 38 配对落位 → noctf
  `0xfffffff007acbf10`）；**crash 站点 bl 字节实测** → `0x0af3ff14`
  = native 终局 `0xaf3ff10`（= noctf + `__TEXT_EXEC` 段移位
  0x3474000）**+4**——+4 = 跨过拷贝体 pacibsp 的 emit 既有约定，
  墙 36（bl→0x0ae6309c）与墙 37（bl→0x0af1f9e4）同款且**双前例
  真机通关实证**（档案所记 +0 系引 map 终局地址之误，本轮字节级
  复核三例约定一致）；GOT 槽 → `0x0af3ff10`（pacibsp 头）本体
  正确。w35-gate PASS（resign 指纹 0 / dispatch 对 371 持平）；
  statistics 全平（branches 42024 / kept 82701 / closure 311）=
  零附带漂移；make_fixture check 256/256。
- **回退**：recovery_a ← `payload-w37fix-6d4a9f19.img`
  （e6b06399，在机 = 已证通关墙 36+37）；ce3df35f 二级；
  b63ad26e 三级。

