# STATE — 现势状态（m2381，2026-10-28 更新）

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

**出件**：kernel `35761550` / payload `e5e9556f`
（`out/m2381/payload-dtgetprop-e5e9556f.img`）待刷（会话二十五 =
bootpath 清点 + SecureDTGetProperty fixture，§7 工作单已执行）。
刷写红线不变：只 recovery_a；boot_b `d0200a6b` /
userdata `fd43ee70` 不动；回退 = recovery_a ←
`payload-kallocfix-crc-5f59a2a0.img`。下会话工作单 = §7。

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

## 7. 工作单 = 会话二十五（已执行，判读单见文末执行档）：关键路径排空（2026-10-31 用户拍板"只想起内核→进桌面→完善驱动"）：

- **战略**：刷机从探针变回验收。离线走完 early-boot 调用图，把"到
  root mount 为止"要经过的 stubbed 目标**一次修完**，一个 payload，
  一刷验收整段。账本清零不再是目标。
- **A. 调用图清点**：入口 = early_security_init（emit 命令行
  `com.apple.driver.AppleMobileFileIntegrity=0xfffffe0008f13c80`）→
  donor AMFI init（0x8f0d230 所在函数）一带；沿 bl/blraa 静态遍历
  （条件分支双侧保守收集），收集 ledger 内目标直至关键组件收口
  （apfs / IOStorageFamily / AppleImage4 / corecrypto / ARMPlatform /
  PMGR）。walker 落 `temp/batch-fixtures/bootpath.py`。产出 = 路径
  目标清单（预计个位数到十几个）。
- **B. 逐个修**：
  1. 0xafc09d8 = SecureDTGetProperty（已定谳：donor 0x19C 独立函数、
     native 内联无符号；语义 = 遍历 DTEntry 属性（entry+8 = 属性数组、
     [entry] = nProps、prop+0x20 = size、name strcmp），写 value/size
     返 1/−1）→ ~30 行 C shim 挂 native（先对拍我们 pexpert IODT 的
     DeviceTreeNode 布局——同为 xnu-12377.122.8 应同构）+ fixture 绑定。
  2. 0xafc1b20（PE 小壳）若在路径上 → 同法。
  3. 其余路径目标：先重跑 matcher（v11 自动收新壳）；仍不能配的按
     shim/fixture 先例逐个落。
- **C. 出件一刷**：kernel → payload → 只刷 recovery_a。判读门：AMFI
  early init 整段过（`fileset: … from 0x8f…` 不再报）→ 推进到下一段；
  root mount 段新坐标 = 记账继续 B（不单独刷）。
- **D. 止损**：路径排空后 root mount 仍被同区新墙连发卡 → 停修归因
  （系统性缺口），回报拍板。
- **红线不变**：只 recovery_a；boot_b d0200a6b / userdata 不动；回退 =
  recovery_a ← payload-kallocfix-crc-5f59a2a0.img。

## 8. 车道判读档案（23F84 fileset 线，会话十三→二十四；逐墙全文亦在各 git commit message）
- **★最后一面墙（会话十三第一刀，修法已勘定）**：duplicate `boot_log_*`/`boot_diag_*`/`boot_log_export_put` ——BootLog.o 双载：libBootProgress.a（build-boot-progress.py:91 无条件编译 log_obj）× libShengUfs.a（source-storage 适配器 sources 表无条件含 BootLog.c）。**修法 = 主树 `tree/scripts/build-boot-progress.py` 在 XNU_SOURCE_STORAGE=1 时跳过 log_obj**（log_command 编译 + archive 成员表两处条件化；BootProgress.o 的 boot_log_tick 弱引用由 source-storage 档的 BootLog.o 兑现；build-report.json 的 log_sha256 条目同步条件化）。改完 `./unixnu defconfig m2381_defconfig && ./unixnu tree && ./unixnu kernel` 三连（改 defconfig 必重跑 defconfig=铁律；改设备脚本必重跑 tree 刷新副本）。
- **kernel 绿后余单（按序）**：①board.toml `[payload] trust_cache_sha256` 三值换 23F84（现挂 20H392 三值=payload fail-safe 门；kernel 步已按 profile [ipsw] fetch 清单取了 StaticTrustCache/Cryptex1,SystemTrustCache 成员，sha 之 + make-m2381-trust-bundle.py 重打包）；②args 拍板——**案甲（工作单推荐）已勘定参照 = sheng feat27 board.toml:52 实证单**，m2381 最小集 = `-v serial=3 debug=0x14e kt_fixed_zones=120 -no_slto_panic use_contiguous_hint=0 native-source-storage=1 native-ufs-root=1 noidle=1`（native-source-storage=1 是 source-storage 启动总闸=SourceStorage.cpp 门控必带；sheng-pmgr/offline-legacy-* 全不背=lane 已摘；noidle=1 防 r70 睡眠灭屏）；③`./unixnu payload`（红线：--unique-chip-id 3645ad55d47e77c1 + --trust-cache/--data-ramdisk 显式传 + -D 在子命令后）；④首刷三件套 = boot_b ec1d518b 不动 + recovery_a 新 payload + userdata 新件（已出），判读门 = native-pac-keys 每 boot 每核键编程行/fileset 绑定计数/native-source-storage=1 生效行。
- **★会话十三步0 = 上游更新（用户令：先同步再继续）**：`git fetch upstream` 看 feat/ipados27-gpu-parity 新 tip 与 **main（用户情报=上游在写文档合入 main）**；若动 = `git rebase upstream/feat/ipados27-gpu-parity m2381-ios27`（**净差验收铁律**：取上游版类冲突不能只看停点，`git diff feat..tip` 抓静默拼回）+ 子模块对齐 + 本单 12 面墙补丁逐面过 net-diff（尤其 kernel.py 双派生与设备层 ptrauth patch 是否被上游同题解法替代——若上游已修则取上游版删本地件）。
- **预期管理**：本车道 fileset 出件（emit_prelink --fileset-reference 吃 23F84 MH_FILESET donor）仍未到达——链接关过后它就是下一关（sheng 未公开通关证据，无 boot 实证参照；23F84 KC 无 LC_SYMTAB，符号化走 dyld cache/fileset 表）。回退链不动：21H461/20H392 全组 + boot_b ec1d518b + userdata 2c2aca3f。

**会话十三执行记录 + 会话十四交接单（2026-10-30 深夜，全链出件达成）**：

- **步0 上游更新**：fetch 后 feat 新 tip `f161cd2`（+3：AdrenoMetal×2 bump + 三连 bump opaque scanout + metal 前端 host check 脚本，全 GPU 侧不消费）；main +9 全 docs（device records 刷新，feat 未合 main）。rebase 238 提交**零停点**落 `3315cf8`，净差验收 57 文件全工作区资产，上游 drivers gitlink 对齐新 pin（Adreno740 90b52bf/AdrenoMetal 148698d/GopFramebuffer 7f7ae1e）。备份针 `m2381-ios27-pre-rebase-20261030`。
- **第12面墙破（BootLog.o 双载）**：`build-boot-progress.py` 在 XNU_SOURCE_STORAGE=1 时跳过 log_obj（编译+archive 成员+report 三处条件化）——kernel 步推进到 prelink。
- **fileset 出件关三墙全破（本会话主战场）**：
  ①**codeless 断言** = 设备 fork 的 emit_prelink 无 fileset 分支无条件走经典 plan()。**换装 vendored kits/XnuPrelink → 上游 feat pin 76452c6e**（fileset_match/emit/layout/shape/segmented+kc_image，8 测试全绿；21H16 arm64e 经典路 fork 退役入 git 史）+ board.toml `[kernel.prelink] extra_args=["--allow-missing"]`。**机制勘定：MH_FILESET 输入走 fileset_match 管线（--fileset-map/--fileset-reference/--allow-missing 任一切换），plan() 的 codeless 检查只属经典路**。
  ②**native __DATA 槽约束** = fileset_layout 把 native __DATA 钉在 collection `__DATA` root 起点保留（donor 内核数据位，23F84 洞=2142KiB，首个 retained kext=AppleARMPlatform@0xb4af780），我们的 native __DATA=3824KiB（BSS 大户：bp_* profiler 四件 2048KiB）。**裁 BootProgress.c bp_*：BpProfSamples 65536→4096、BpKernSamples 32768→4096**（省 1888KiB → native 1936KiB，余 206KiB≈5%）。洞后零字节为零=不可扩洞，裁码是唯一路；gScratch(ShengUfs 128KiB)功能件不动。
  ③**RELEASE 参照件产能**（--fileset-reference 的料）：`XNU_KERNEL_CONFIGS=RELEASE ./unixnu kernel --skip-prelink` 产出 `mach.release.bcm2837.unstripped.noctf`（17.6MB）→ 手动 stage 到 `tree/build/xnu/fileset-reference/` → board extra_args 换 `--fileset-reference <path> --allow-missing` → 重跑 kernel。**主树脚本三改**：build-xnu-4k-baseline.sh（config 选择+XNU_REFERENCE_BUILD=1+ubsan-minimal stub 编译钩子置顶于 crypto gate 之前；seal 路径按 config 派生）+ patches/xnu/reference-build-release-export-bypass.patch（**RELEASE-only all-kpi.exp 导出检查旁路**：IOUnifiedAddressTranslator/DART/llvm_profile 系预链 kext 符号，开源 xnu 树无源码，纯源码 RELEASE 独立内核永远缺；DEVELOPMENT 无导出检查故从未暴露）+ kernel.py（RELEASE pass 置零 m2381 取证链 owner 全家+BOOT_PROGRESS——offline-owner 补丁自带 DEVELOPMENT #error 守卫且 RELEASE 形态编不过，参照件不 boot 不发布；**compile-record 检查按 config 参数化**）。**bootstrap 链接失败=设计内可容忍**（kernel.py try/except：记录在即放行；主链接 force_load 档供体齐）。**巨型坑：host nm/llvm-nm 读 138MB Mach-O 档静默零输出**——曾据此误判"无定义体"绕远路，凡 nm 扫档必须先验输出总行数。
  - **参照件成效**：函数匹配 6807→**8293**（reference-order 5142+reference-symbol 401=名字转移主通道），data 559，未解 641 sites/378 targets（apfs 206 sites 为主）。
- **payload 步三墙**：①check_native_root 强索 sheng-pmgr=2+_sheng_pmgr_run（经典路前提）→ payload.py 加 prelinked_pmgr 参数（profile roots 含 com.apple.driver.ApplePMGR 即免检，sheng 实证 arg 行同形态）；②patch-xnu-entry-probe.py 符号解析吃 fileset "Symbols from…" 节头行→parser 跳非 hex 行；③panic screen 超洞 7 字节（0xa37>0xa30，fileset __TEXT_EXEC padding 收窄）→ **xnu-panic-screen.c 摘 frames walker 块**（-88B→2515B，保 pages/free 判读计数器；串口 backtrace 补位）；④**ESP 超 recovery_a 100MiB 分区**（stage 101MiB：fileset kernel.macho 85.2MiB）→ board.toml `forensic=false` 摘读者三件 15.3MiB。
- **出件终值**：kernel `9da51132`（带参照件）→ payload **`8a209d8b`**（96MiB，ESP 四样齐：kernel.macho 89,374,720B+patched 副本 sha 5c244659+trustcache 82,236B+scratch-data+initrd 行+ADT acba2928，卷余 9.3MB）；trust 三值 e70a385a/58960bbc/ebeaf7fb 已入 board pin；args=案甲 9 项 130B。
- **nura 读者内核需求（2026-10-30 晚拍板：nura 仓零接触，不发工单文档，需求由用户口头转告）**：需求规格留档 = 设备仓 `forensic/READER-KERNEL-REQUEST.md`——读者专用最小内核（去 GPU/DRM 全家，PE≤4MiB，保 EFI stub+UART+USB NCM+UFS+configfs，initramfs 外投契约同 build-forensic-reader.py），落成后摆 `.cache/forensic/` + board.toml `forensic` 回 true。
- **会话十四交接（首刷判读单）**：三件套 = **boot_b ec1d518b 不动 + recovery_a ← payload 8a209d8b（先 fastboot getvar partition-size:recovery_a 核 100MiB）+ userdata ← userdata-root 13.02GiB（out/m2381/ipados-26.5.2-23F84/m2381/userdata/）**；设备现态先核（FLASHING.md 单通道律+dd 读回才算证据）。**判读门（串口）**：①native-pac-keys 每 boot 每核键编程行（EnIA/EnIB/EnDA/EnDB）；②fileset 绑定计数（emit_prelink 报告的 named/mapped 数）；③native-source-storage=1 生效行 + source-storage 卷挂载；④`_unixnu_fileset_unmapped` stub 触发行（每条=一次未解调用命中，报站点不跳野指针——641 sites 的实验性余量是预期内背景噪声，非墙）；⑤panic screen（pages/free 行在，frames 行已摘）。符号化：23F84 无 LC_SYMTAB，走 dyld cache/fileset 表（r88 路线作废预注册仍有效）。回退链不动：21H461/20H392 全组 + boot_b ec1d518b + userdata 2c2aca3f + 备份针 m2381-ios27-pre-rebase-20261030。

**首刷卡（2026-10-30 交卡，会话十四按此判读）**：现态核对（current-slot=b + partition-size:recovery_a=100MiB + 自 r85/r88 轮 boot_b 未被动过——若存疑则补刷 boot_b ← 设备仓 `UniXNU-m2381/build/m2381/boot/boot.img`（ec1d518b，31,793,152 B；⚠主树 out/m2381/boot/boot.img=40f2a199 是 B2 旧件 1.66MB，勿混））。刷序 = 先 recovery_a ← payload 8a209d8b（100,663,296 B）→ 开机验 UEFI 菜单在（功能级 ESP 验证）→ 再 userdata ← 13.98 GB 件 bb5d22f4（15-30 分钟勿断）→ 开机串口判读。本轮无 dd 读回通道（读者被 ESP 空间挤出，nura 最小内核回归后恢复），证据 = 串口 + panic 屏。判读门 = STATE §8 会话十四交接五条。回退链 = recovery_a ← payload-r88-c932a128 + userdata ← 2c2aca3f 件（均已在库）。

**会话十四判读 + boot 件定谳（2026-10-30 深夜，首刷实况：UEFI 菜单活，选 first boot 立刻闪回菜单）**：

- **根因定谳 = FV 内 DarwinBoot 太旧，不是 boot_b 坏也不是内核/投递链**：ec1d518b 构建于 10-29 07:42（r81，20H392 时代），其 DarwinBoot pin=`3325e35`——`ParseKernel` 硬校验 `U32(Data+12)!=2 → EFI_UNSUPPORTED`（**只收 filetype 2 = MH_EXECUTE**）；26.5.2 fileset 内核 kernel.macho 实测 filetype=**12（MH_FILESET，cputype 0x100000c）**→ DarwinBoot 当场拒，Boot Manager 回菜单，与"选 first boot 立刻闪回"逐环节吻合。修复 pin=`3e3a8a9`（accept arm64e MH_EXECUTE 内核 + **filetype 12 在 Header==0 放行** + root policy sidecar；ESP 上 kernel.macho.crypto-hmac.json 即配它）。机制教训入档：**boot_b"冻结不动"纪律按车道绑定——换 OS 车道（内核形态/代际变）必须重估固件，DarwinBoot 的内核格式接受面跟着车道走**。
- **上游 uefi 跟进（用户拍板）**：rebase `0eb4514 → 20ac23d5`（upstream/sheng tip，5 笔 = 34c98806 ShengBootMemoryDxe 176 行换共享 UniXNU RAM disk 驱动【shengPkg 专属 fastboot-ramdisk 路径，m2381 不引用、零影响】+ docs×4）；overlay 重放 `9318b62e`（基座记载已同步改写），m2381 相关文件 rebase 零 diff。**uefi-m2381 push 需 force-with-lease，待拍板**。
- **新 boot 件出件**：FV 重编译（DarwinBoot.efi 87,040 B，10-30 03:27 新鲜于源码 mtime，增量缓存嫌疑排除）→ `package-m2381-boot.py` 打包 → 设备仓 `build/m2381/boot/boot.img` = **`115f2738`（31,793,152 B，与旧件同尺寸）**；ADSP 段 acf95fd5/adsp_dtb db6a6478 与旧件**逐字节相同**（FV 外零漂移），fd ce3ce88b→**68e5023c**（唯 FV 内容变 = DarwinBoot）。旧件备份 = 主树 `out/m2381/boot-b-ec1d518b-r81.img`（+json）。
- **补刷卡（2026-10-30 交卡）**：**只刷 boot_b ← 设备仓 `UniXNU-m2381/build/m2381/boot/boot.img`（115f2738）**，recovery_a 8a209d8b / userdata bb5d22f4 已刷在位不动；刷前现态核对照旧（current-slot=b）。判读门 = 会话十四交接五门不变（串口：native-pac-keys 行 / fileset 绑定计数 / native-source-storage 生效行 / `_unixnu_fileset_unmapped` stub 触发行=预期噪声 / panic screen）。回退 = boot_b ← `out/m2381/boot-b-ec1d518b-r81.img`。
- **读者选项丢失 = 预期内**（forensic=false 被 100MiB ESP 挤出，会话十三已档），恢复路径不变：nura 最小内核（4d67ff9 工单，PE≤4MiB）交付 → `.cache/forensic/` → board.toml `forensic=true` → payload 重建（现卷余 9.3MB，4MiB 内核+1.3MiB initramfs+0.14MiB dtb≈5.5MiB 可容）。

**会话十四二轮判读 + cpusub 门破（2026-10-30 深夜续，用户刷 boot_b 115f2738 后实测：仍秒弹回）**：

- **第二道门定谳 = com.apple.kernel 成员 cpusub 不等**：新 DarwinBoot(3e3a8a9) 对 filetype 12 放行后，`ParseKernel` 还要求 **`com.apple.kernel` 成员的 cpusubtype == 集合头 cpusubtype**（DarwinBoot.c:238，不等 → EFI_LOAD_ERROR）。实测：我们 kernel.macho 顶层 = `0xC0000002`（emit 从 donor 抄），成员 = `0x80000002`（**Linux lld 源建内核自带 CPU_SUBTYPE_ARM64E|0x80000000**）；donor 23F84 自身 top/member 一致（均 0xC0000002）。上游在 macOS 用 Apple ld64 建核天然产出 0xC0000002 故从未撞此门——**m2381 = 首个 Linux 建 fileset 上机的板**。
- **DarwinBoot 后续门全部本地预验 PASS**（免再烧刷机轮）：成员段覆盖 10/10、entry 与顶层相等、父段无重叠、trustcache（82,236B 双 cache）合法性、scratch-data.img=「DBRDISK1」packed 格式与 ADT PackedRAMDisk/KeepPacked 一致、窗口 = ADT darwinboot-memory-size 4864MiB（board owned-memory-size；若 AllocateWindow 失败屏上有大字 NO WINDOW 诊断=DiagDraw，拍照即可定位）。
- **修法**：主树 `scripts/patch-kernel-fileset-cpusub.py`（成员 cpusub 规范到顶层，原值钉 0x80000002，round-trip 自检，非 fileset 零操作）+ `payload.py` 在 kernel copyfile 后无条件调用（先于 probe/seal）；payload 重建 = **`38a2a8f9`**（96MiB；ESP 内核副本 `14bc069b`；源建 kernel 9da51132 不变；对 ESP 副本跑 ParseKernel 全模拟 = ALL PASS）。
- **补刷卡 v2（2026-10-30 交卡）**：**只刷 recovery_a ← payload `38a2a8f9`（100,663,296 B）**，boot_b `115f2738` 已在位不动、userdata 不动。开机预期：串口 `DARWINBOOT: loader v2 installed EL1` → `inputs validated; exiting boot services` → 绿条 → 内核五门照旧。回退 = recovery_a ← 8a209d8b（在库）。
- **备注**：上游两 pin（DarwinBoot 3e3a8a9 / XnuPrelink 76452c6e）与我们所用一致、无更新修复；cpusub 规范化是可上游化的通用修法（LLVM 建核 vs Apple ld64 的 cpusub 差异），后续可提上游。

**会话十四三轮判读：内核起飞 + 第一堵原生 PAC 墙定谳（2026-10-30 深夜续三，本轮=纯分析不修，修复=下会话）**：

- **三件连过**：cpusub 门(38a2a8f9)→ stage 3 复位探针点火实证(品红后 ~5s 自复位 = MMU 开启序列过，4K+arm64e 页表启用段排除)→ 诊断页轮(49f1478d,boot-progress=2+panic 屏复位探针撤收)炸出 panic 屏。**上游同款剧本验证**：stage 色条/定时复位/panic 屏全是上游 feat 自己的 patch-xnu-entry-probe.py+green-bar diagnosis 指南(unixnu.com/guide/debugging)，boot-progress=1/2 诊断页=上游盲区主力工具(120×48 阶段史，卡住冻结可拍)。
- **内核抵达深度（诊断页实录）**：pages=0x125b8e/free=0x12554e（≈4.7GiB 窗全活）→ 阶段史 clock_service_create → sched_startup → thread_daemon_init → thread_call → Thread_bind → mapping_adjust → clock_service_create(2nd)；OSLog 流验证/Firehose 16 chunks/Log queues 72 槽全配置完；末行 console=`[hmac algorithm] tried entry init`。
- **墙 = `PAC failure from kernel with IA key while branching to x8`**（ESR 0x07200000，far=0，cpsr 0x624003c4）。符号化：**pc = `_source_corecrypto14_bootstrap`+0x84 `blraaz x8`**（IA 键零上下文鉴权）；x8 ← `ldur x8,[x19,#0xb4]`，x19/x0=0xfffffff00b5f9a90（BSS 对象，BSS 初始化域）；x19 来自 +0x40/+0x4c 两次调 `_source_corecrypto14_original`（单例，cmp 相等过）→ casal 锁 `_source_corecrypto14_original_state` → 调算法入口指针；调用场景= hmac 算法 entry init。**x8 地址部分=0xfffffff00a72f938（__TEXT_EXEC 符号洞），头四字节 0xd503245f=PACIASP=真函数入口形态**（非 reporting-stub 形态；未在 emit 档案中登记）。x20=块描述符域、x24=`__startup_EARLY_BOOT_entry_source_pthread_bootstrap_23+0x18`（启动集合邻域）。
- **已排除**：①per-core 键分歧——native-pac-keys.patch 实读：首核 CNTPCT+splitmix64 派生一次、MMU off 写 `native_pac_keys` 全局，后续核一律加载存储值（含热复位），全核同键；②x8=登记在案的 fileset unmapped stub（emit 档案零命中）。
- **候选（下会话 RE 判别）**：α=签名者/鉴权者**上下文失配**（写侧带多样性、读侧 blraaz 零上下文——注意 linux-clang-ptrauth-null-fallback.patch 的降级域是否覆盖 corecrypto 注记）；β=**fixup 链重签覆盖缺口**（源建内核 __ptrauth 全局指针走 lld auth_rebase→内核早期重签；该指针若在 emit 搬运中丢了链条目，残留 lld 占位/旧签名）；γ=签名发生在不同键状态相位。判别路径：①xref 找写 [x19+0xb4] 的指令确定签名形态与预期上下文；②用 emit 的 orig↔native 映射解 0xa72f938 真身（kernel.json/kernel.map.json）；③核对 lld auth_rebase 在 fileset emit 后的存续；④若需 A/B：固定键探针（键派生改常量）可一步区分键态相位 vs 上下文失配。
- **nura 读者内核交付（用户转交交付单，文件未落本机）**：vmlinuz-unixnu-reader-7.3.0-20261030 = 3,977,728B（3.79MiB≤4MiB，sha256 1512301a4d77d090a3b245f912f8fcd901e8cc905a47bf2416a10683694d42ff）+ 专用 .config（f7142f19…a506e05）+ System.map（无 kallsyms 固定地址，离线符号化用）。预算对账 85.2+3.79+2.4≈92<100 ✓（ESP 余 9.3MB 容 3.79+1.3+0.14≈5.2 ✓）。契约：PE32+ EFI AArch64 zboot+zstd（LoadImage+LoadFile2 流程原样）、CONFIG_CMDLINE 与 v71 逐字节同、initramfs 契约不变（busybox 不动）；身份自检 `uname -r`=7.3.0-unixnu-reader。行为差异：无显示驱动（GOP 残留 fb 上 earlycon+fbcon）、USB gadget-only（configfs 绑 UDC 即通，无 host）、无模块、无 RTC（时间戳 1970 预期）、无 KASLR（配 System.map 好调）。验收=payload 菜单 reader 项→UART shell→NCM 起→dd recovery_a 前 96MiB 对拍；盯「Initramfs unpacking」不回归。翻车回带 UART 全文（dwc3/ufs/eusb2 probe 行）；源码=linux-mobile-ports meizu20-t4b@bfc79f41，配方 nura docs/UNIXNU-READER-KERNEL.md。
- **下会话开工单**：①nura 三件落本机验 sha → 摆 `.cache/forensic/`（forensic-kernel.efi 命名按 build-forensic-reader.py 契约）→ board `forensic=true`+`sheng-log=1` → payload 重建 → 读者项验收（此后每轮自带 dd 读回+SHLOG 窗）；②PAC 墙 RE 四步（上述判别路径），注意 seal-source-crypto-kernel 与该指针的相互作用；③探针轮配置（panic_screen/entry_probe_reset_delay）待 PAC 修复轮一并收队；④回退链新增：payload 49f1478d（诊断页轮）、94f67199/5209bb33（复位探针轮）、38a2a8f9（cpusub 轮）均入 out/m2381/ipados-26.5.2-23F84/m2381/payload/ 历史（payload.json 只存现役，历史 sha 在本档）。

**会话十五判读：读者通道交付轮全绿 + FV 内存图地雷拆除（2026-10-30 凌晨续，用户三拍板：分家/用 v71/留菜单）**：

- **分家定案（用户拍板）**：reader 不再 riding iOS payload——专用读者 ESP（设备仓 `forensic/build-forensic-esp.py` 出件）按需刷 recovery_a，取证轮/iOS 轮各刷各件；`fastboot boot` 不可用（nura 已实测 Booting FAILED，ABL-FASTBOOT-TABLE 卡）。board.toml 回 `forensic=false`+`size_mib=96`（iOS payload 下轮重建生效）；`sheng-log=1` 留在 kernel_args（下轮起每轮 boot 留 SHLOG 窗，读者 ESP 对刷即可采）。**reader 内核 = v71 完整版（4aad92b3，m2381 实证）**，nura 最小版（1512301a）在案未上岗；**菜单保留**（ESP toml default=forensic/timeout=3，Restart/Power off 常驻，reader 退出回菜单）。
- **最小读者内核首崩根因定谳 = FV 内存图 bug，非内核**（全屏崩溃照 + 反汇编闭环）：MemoryProtectionExceptionHandler dump（肇事镜像 [0]=UniXNU.pdb）→ ELR = reader PE+0xAFC0 `sturb w15,[x9,#-5]`，x9−5 == FAR `0x82700020` 逐字节合，x6/x7/x8/x9 四路等距 0x3AE 游标 = zstd 字面量解码写输出 → **同步外部中止（DFSC 0x10）= 写 XPU 围栏** → 围栏实名 = `tz_stat_mem@0x82700000`（stock FDT no-map，Meizu 实机反编译）→ 投递者 = **RamManagerDxe**（SiliciumPkg）：把 MemoryMapLib 洞区 [0x81F00000,0x8A800000) 整段 `MapMemoryRegion(EfiConventionalMemory)`（洞区无描述符 → ParseMemoryRange 当空闲段收编），无 KASLR zboot 的 `efi_random_alloc(seed=0)` 低址优先把 9.8MiB 解压缓冲拍进洞区首段。v71 存活 = KASLR 随机地址运气；**iOS 车道同雷**（任何 efi 分配可落洞区）。
- **修复（uefi-m2381 MemoryMapLib）**：三块 no-map 区入表（ADSP MHI 0x81F00000+0x20000 / Global Sync 0x82600000+0x100000 / TZ Stat 0x82700000+0x100000，全 [DTB] 值）→ RamManager 减法自动排除，洞区余段 0x81F20000-0x82600000(6.9MB<16MB 被弃) 与 0x82800000-0x8A800000(128MB 净土) 照常。**boot_b 新件 `d0200a6b`**（31,793,152B，ADSP 段逐字节水，现役 115f2738 备份 out/m2381/boot-b-115f2738-rollback.img）。FV 菜单/启动照常 = 修复零回归。
- **工具流澄清（勿再钻错工具）**：boot_b 打包 = **设备仓 `scripts/package-m2381-boot.py`**（v4 头 + 5 字节 ramdisk 占位 + M2381FW ADSP 段；shim 契约 `image_size==FD_SIZE` 硬校验——overlay BootShim.S 是 nura 实证契约**非遗漏**，按上游 1c036336 改 accounting 会被它正确拒绝，已回滚）；`./unixnu bootimg` = sheng 形状，m2381 勿用。
- **32MiB FAT32 陷阱（首刷实拍入册 FLASHING §7）**：数据簇 64,480 < FAT32 下限 65,525 → `mformat -F` 硬造非法卷无告警 → EDK2 Fat 驱动拒载整卷 → 菜单跌 "built-in configuration / no boot entry found"（同时解答 "Return to firmware→failed to find an operating system" 与 Tools 秒回的伴生现象）。64 MiB 修复。
- **读者 ESP 首验全绿（2026-10-30）**：`reader-esp.img` d3e6d468（64MiB）刷 recovery_a → 菜单 3 秒自动进 reader → v71 内核起（probe 报 "m2381 forensic log reader 7.3.0 / usb: acm on a600000.usb"，SHLOG 窗 magic=SHLOG001，userdata=sda22）→ **NCM 51s 立**（1d6b:0104）→ **dd 读回对拍双绿**（recovery_a 前 64MiB = d3e6d468 ✓、boot_b 前 31,793,152B = d0200a6b ✓，读者通道即刷写取证仪器）→ **fetchchunk 15/15 块 31s SHA MATCH** → read-m2381-ufs-log.py 解析 337 槽旧 boot（r55e 代遗产，header 契约守住，residue 正确截断）。
- **下会话开工单**：①iOS payload 重建（96MiB/forensic=false/sheng-log=1 已配齐）+ 刷 recovery_a 回 PAC 车道 → PAC 墙 RE 四步（会话十四三轮档判别路径原样，注意 seal-source-crypto-kernel 相互作用）；②取证使命流程 = 刷读者 ESP → 采证（probe/dd/fetchchunk）→ 刷 iOS payload 回，各轮 dd 读回照旧；③回退链新增：boot_b d0200a6b、recovery_a reader-esp d3e6d468、payload 0f1013f4（合装轮，已不在位）；④最小读者内核上岗路径 = FV 修复已就位，可单独 A/B（v71 仍为现役 reader）；⑤探针轮配置（panic_screen/entry_probe_reset_delay）仍待 PAC 修复轮收队。

**会话十五补档（2026-10-30 收队）**：iOS payload **`58676a6c` 已出件已刷**（96MiB/forensic=false/`sheng-log=1` 在 args；存档 out/m2381/payload-58676a6c.img；recovery_a 现役=此件）。首 boot **PAC 墙逐字复现**（pc 0xffffff00a98a2dc / x8 目标 0xa72f938 / pages=0x125b8e 与会话十四三轮全同；lr/x8 仅 PAC tag 域随 boot 键派生不同）——**FV 内存图修复对内核路径零扰动，墙确定性**；证据=屏摄 IMG_20261030_061956.jpg（用户拍板以屏摄为准）。SHLOG 窗采集顺延下会话（墙点即冻结，写入时长存疑，fetch 前不可再 boot xnu 以免窗口滚动）。**设备收队现态：boot_b=d0200a6b（修复版 FV）/ recovery_a=58676a6c / userdata 未动；下会话刷卡先照此核对。判读=下会话（RE 四步判别路径在会话十四三轮档）。**

**★会话十六 = PAC 墙根因定谳 + 修法落件（2026-10-30 深夜，屏摄 IMG_20261030_061956.jpg 判读 + RE 四步全闭环；主树 patches/xnu-12377/source-corecrypto14-bootstrap.patch 已重生成，kernel 重建后台进行）**：

- **RE 四步定谳（全部实证，A/B 探针免了）**：
  - **①签名/鉴权形态**：pc = `_source_corecrypto14_bootstrap+0x84` = `blraaz x8`（**IA 键、零修饰符**；capstone 全内核普查 = 3657×blraaz + 304×braaz vs 仅 7×blr——**native 编译约定 = 一切未注解函数指针 (IA,0)**，llvm 23（Homebrew 23.1.1）`-arch arm64e`/`--target=arm64e` 默认行为，独立 .c 实测复现）。x8 ← `ldur x8,[x19,#0xb4]`（kmod_info pack(4) 布局 start@0xb4 非对齐所以 ldur）= **donor `com.apple.kec.corecrypto` kext 的 kmod_info.start**；x0/x19=0xfffffff00b5f9a90 = donor kext 数据区（collection __DATA 后段 0xb47c000+0x17DA90）。机制 = 上游 `source-corecrypto14-bootstrap.patch`：扫 fileset 找 donor corecrypto 的 kmod_info → `module->start()` 起 donor 唯一 provider（防双 provider 致 PRNG 上下文未初始化）；x24=`__startup_EARLY_BOOT_entry_source_pthread_bootstrap_23+0x18` = startup 游标（pthread 姊妹补丁直调 native `pthread_start`，非同族）。
  - **②0xa72f938 真身**：kernel.json `segment_layout`（native→collection 搬运映射）定位 = **donor __TEXT_EXEC 内 donor corecrypto kext 的 start 函数**（native __TEXT_EXEC 搬至 0xa73c000+，目标在其下 0xC6C8 = donor 原代码；非 reporting-stub 非 native；入口首字 0xd503245f = **BTI c**（会话十四记 PACIASP 有误，"真函数入口"结论不变））。
  - **③auth_rebase 存续 = 存续且已被处理**：槽在 __DATA 页 216 链上（144 节点），文件槽值（我件与 donor 23F84 **逐字节相同**）= 0x8010e4fe0372b938 = auth_rebase 链条目：isAuth=1 / **key=0(IA)** / addrDiv=0 / **diversity=0xE4FE** / cacheLevel=0 / target=0x372B938 → base(0xfffffff007004000)+target = **0xA72F938 与 x8 低位逐位合**；运行值≠文件值（低位已是终值地址）→ `kernel_collection_slide`（dyld_kernel_fixups.h；HAS_APPLE_PAC=1 由 native-pac-keys.patch 定于 BCM2837.h）已按链条目 **(IA, 0xE4FE)** 重签。emit 对账：kext_chain_sites_kept=82701（含本槽）/ kext_pointers_rebound=47166（donor kext 数据区 19 个链页被重绑为终值 = 非丢失；donor 内核数据区链页随 donor 内核本体淘汰 = 设计内）。
  - **④根因 = α 上下文失配的精确形态：同键（IA）、不同 diversity（0xE4FE ≠ 0）**。写侧 boot fixup 按 donor 链条目签 (IA, 0xE4FE)；读侧 native 默认 blraaz (IA, 0) → AUTIA 恒败 → trap。**β（fixup 链丢失/未处理）证伪**（槽被正确处理）；**γ（键态相位）证伪**（键同侧、native-pac-keys 每 boot 一次编程早于 arm_init 的 fixup pass）。会话十四三轮档"x8 高位=裸链条目"解读作废（低位=已处理终值地址、高位=真签名 tag）。
- **修复（已落件）**：主树 `patches/xnu-12377/source-corecrypto14-bootstrap.patch` 重生成——`source_corecrypto14_start_once` 内 `module->start(module,data)` 改为经 `typedef kern_return_t (* __ptrauth(0, 0, 0xe4fe) …)(kmod_info_t *, void *)` **注解 lvalue** 调用（`#if __has_feature(ptrauth_calls)` 守卫，非 ptrauth 构建回退原样）。实测该写法 clang 23 产出 `mov x16,#0xe4fe; braa/blraa` 与 donor 签名约定精确对齐；**直 cast 到 qualified type 被工具链拒**（"cannot cast to __ptrauth-qualified type"）必须走 lvalue。流程 = 旧 patch 应用到 pristine feed 副本 → 改结果 → 重 diff（hunk 头 -502,3 +502,141 自动正确）→ 对 pristine dry-run PASS。0xE4FE 为 23F84 donor 链条目原值（24A446 同族预期同值）。
- **seal-source-crypto-kernel 相互作用 = 无**（seal 只 prepatch __text static_if 位点 + 重算 __TEXT_EXEC/__text HMAC，不碰 __DATA 链与 donor 区；bootstrap 改动自动进 seal 重算范围）。emit `component_hmacs` 已含 com.apple.kec.corecrypto = donor 组件 POST HMAC 重封由 emit 预接（donor 自身 POST 预期通过）。
- **重建/判读单（已全部完成，主树 commit `481d33e` 未推送）**：kernel 重建绿（首版 cast 写法被 clang 拒——**cast 到 qualified 类型本体非法，必须 cast 到"指向 qualified 类型的指针"再解引**，kernel.log `cannot cast to __ptrauth-qualified type`——修正后重跑）→ **kernel `a36d5103`**（新 unstripped 反汇编实证 bootstrap+0x84 = `mov x17,#0xe4fe; blraa x8,x17`，旧 `blraaz x8` 已消失）→ **payload `07080eb4`**（96MiB；ESP 内核副本 `bfa4f979` sha 对针 + 修复字节在位；cpusub/entry-probe(--panic-screen --no-reset)/seal(args 全队)/dtb 全链跑通；平铺件 `out/m2381/payload-pacfix-07080eb4.img`）。**刷写卡 = recovery_a ← 07080eb4（100,663,296 B，刷前 sha 门 + 读回对拍），boot_b=d0200a6b 与 userdata 不动**。判读门：①原 PAC panic 消失②阶段史推进过 clock_service_create/SYSCTL 段 startup（bootstrap 成功无打印）③**下一墙候选预注册**：donor corecrypto start 起来后 crypto KPI 表（g_crypto_funcs 一族）donor 写侧 Apple 约定 vs native 读侧 (IA,0) 的边界同族风险；emit 生成的 native→donor authenticated thunk（0xfffffff007563df0 `blraaz x0` 一族）同 conventions 面。
**★会话十七 = stub 桩首点定谳 + body-hash 配对通道 + 锁三件套 fixture（2026-10-30 深夜续，用户刷 07080eb4 后屏摄 IMG_20261030_074633.jpg；精简流程）**：

- **PAC 墙修复实机生效实锤**：panic 形态全变——旧 PAC panic 消失，donor corecrypto start 已跑起来，死在**新墙 = `_unixnu_fileset_unmapped` 桩首点**：`fileset: original component called an unmapped kernel function from 0xfffffff00a709f00 (x0 0xfffffff902f5095e x1 0x5c)`。桩语义（fileset-unmapped-call.patch）= emit 把 donor 对未配对内核函数的 bl 重写到单桩，报告返回地址+前两参。
- **首点定谳 = memset**：donor 原件 0xa709efc 的原始 bl → **0xa76ce30** = donor 内核 memset（体形=广播字节填充；x1=0x5c=填充字节）。我们 native `_memset/_secure_memset`（同地址双名）**前 0x94 字节逐字节相同**但函数尾部填充跨构建不同 → 边界哈希/shape digest 全不匹配 → matcher 十余通道全漏。全表 = **839 目标/13653 桩点**（top: 0xae40858×1679 点/25 组件、0xa7f4128×1182、0xa7f3548×1011——全是 donor 内核函数被 native 覆盖后未配对者）。
- **修法①（设备仓 kits/XnuPrelink/fileset_match.py，commit 8f85136）= `anchor_body_hash` 通道**：donor 函数体截断到第一个终止 ret 后在 native 文本做原样搜索，唯一命中且落在 native 函数起点才配（ret 后填充是构建差异；字节相同=行为相同；带镜像相对引用的函数天然不会假配）。v1 体哈希版因 end 边界含填充失效；v3 ret 截断版生效（memset+0xa76c000 自动配对）。放宽"多副本任选"会破坏 reference-order 级联（8302→8262），已回退唯一性判据。
- **修法②（fixture）= 锁三件套**：0xa7f3548=`lck_spin_lock`（CAS 0→cpu号@+8，cpu号=tpidr_el1+0x4f0）、0xa7f3fe4=`lck_spin_try_lock`、0xa7f4128=`lck_spin_unlock`——身份判据=FAST 路径 CAS 形态+与上游已钉的 `_lck_rw_unlock_shared`（0xa7f62bc）同区；kext 把 lck 当不透明对象，配到我们实现后布局自洽。fixture 落 `fixtures/fileset-23F84-symbols.json`（108→111 条）。
- **出件**：kernel **c35cf1f6** → payload **fed3dd74**（96MiB，ESP 副本 83d55ef4；PAC 修复字节在位；corecrypto 桩 12→7、全场 13653→12682 点）。存档 `out/m2381/payload-stubfix-fed3dd74.img`；设备仓 commit `8f85136`。
- **corecrypto 残余 7 个（下轮判读，靠下次 panic 的运行时参数点名）**：0xa7b50f8（全局∈{3,4}检查）/0xa7f3364（tagged 分配）/0xa938124/0xa96a0c4（20 点！全局 0xb2d8008+8 指针装入 [x0]，疑 OSObject 基类构造器存 metaClass，无写者未定名）/0xacf9870（tag 0x6827）/0xae96de0（claim-once 全局 0x7d61af0）/0xb007294。识别工具 = `temp/pac-wall/stubid.py`（site→donor 原目标+体反汇编+native 对照）。
- **上游检查（用户问）**：feat 强推重写 f161cd2→9bfc45a，净新增=纯工具脚本（build-fileset-release-reference/bundle-ipsw-trust/trust-append-module/turnip-kmd-patch——把我们手搓的 RELEASE 参照件流程正名化），**无本墙修复**；rebase 收队项未做。

- **开放项**：①0xE4FE 的字符串原型未破（BSD16 穷举常见命名串未中；硬编码即可，链条目本身是权威）②native 侧 (IA,0) 默认的编译开关源头未定位（makedefs 全树无 fptrauth 旗标 = llvm 23 arm64e target 默认；实测确认）——上游 sheng 同款构建必撞同墙，本修法可上游化。

**★会话十八 = corecrypto 残余 7 桩全数定名并出件（2026-10-30 上午，用户刷 fed3dd74 后屏摄 IMG_20261030_090251.jpg）**：

- **新墙判读**：PAC 墙与 memset 桩均未再现，首点前移到 `from 0xfffffff00a6d2848 (x0 0xfffffffae0678c20 x1 0xfffffffae0678b60)`——x0=内核栈 `sp+0x20` 上的 out 参数（调用点前一条 `add x0, sp, #0x20`），site-4 = `bl 0xa96a0c4`，正是会话十七残余表第一项。
- **7 个身份全部定名**（判据 = native 侧同形函数 + 符号表唯一，配 `build/xnu/fileset-reference/` RELEASE 参照件与 `temp/pac-wall/stubid.py`）：
  - 0xa96a0c4 → `_clock_timebase_info`（5 指令 getter，`*x0 = gTimebaseInfo`；与 native 仅 adrp/add 常量不同 → body-hash/shape 两通道天然漏配）
  - 0xa7b50f8 → `_panic_active`（`(panic_state-3)<2` 的 cset lo；corecrypto 2 点）
  - 0xae96de0 → `_register_crypto_functions`（claim-once：槽已设返 -1，否则存参 + 置 flag 字节）
  - 0xb007294 → `_register_and_init_prng`（`entropy_init(64, entropyseed)` → prng_ctx 存 → cckprng_funcs 逐槽拷贝，ptrauth 判别子 0x6961/0xf60a/0x5d01/0x4d5f/0xedeb/0x3051 与 native 逐槽相同 → 4×cc_clear；native 多 inline boot_cpu/prng_ready assert）
  - 0xa938124 → `_read_random`（refresh 0x4d5f + generate 0xedeb、0x100 分块 + cpu_number；native 走函数调用未内联 tpidr_el1）
  - 0xa7f3364 → `_lck_mtx_alloc_init`（kalloc + tag `0x22000000|flags` + 锁组引用；donor 内联 lck_grp_reference，参照件中位于 `_lck_mtx_init` 前一位且同形）
  - 0xacf9870 → `_sysctl_handle_string`（strlen+1 → oldfunc → newfunc 截断写；与参照件逐指令同，差异仅 Apple 串判别子 0x6827 与 PAC 硬化 store；donor 序夹在 handle_long/handle_opaque 之间。会话十七记其"0 点"= 非直接 bl 路径，本表按 stub 目标身份收编）
- **落件**：设备仓 `3a7c6ef`（fixtures 111→118，每条钉 donor 字节 sha + 证据行）；kernel **171bee9d** → payload **`02d7aa05`**（96MiB，ESP 内核副本 55e07c55；trust `ebeaf7fb` + data seed `d04a0153` + ADT `8bfa8cad` + panic_screen；PAC 修复字节在位实证 = `source_corecrypto14_bootstrap+0x84` `mov x17,#0xe4fe; blraa x8,x17`）。存档 `out/m2381/payload-stubfix2-02d7aa05.img`。**构建坑复现（r85 同款）**：`--trust-cache`/`--data-ramdisk` 必须显式传——首轮裸 `./unixnu payload` 出的 `50ea4195` 缺 ESP trustcache+initrd（已删，勿用）；红线件 = `out/m2381/ipados-26.5.2-23F84/m2381/original-trust-23F84.bundle` + `out/m2381/ci-artifacts/verified-root-grow/owner-data-fixture/scratch-data.img`。
- **刷前门（全绿）**：prelink reviewed 111→118、全函数 8294→8301、stubbed 1674→1659；**stubbed 表里 corecrypto 条目消失**（原 7 目标全绑定）；出件内核 7 个 call site 逐点反汇编 = bl 指向配套 native 函数（布局偏移 0x346c000 自洽，ESP 副本复核同）。
- **残余（观察项）**：corecrypto `missing` 表剩 1 条数据引用 `0xfffffff007ca3910`（非函数桩）。
- **刷写卡**：只刷 recovery_a ← `payload-stubfix2-02d7aa05.img`（100,663,296B，刷前 sha 门 + 读回对拍），boot_b=d0200a6b / userdata 不动。判读门：①本 panic 消失②corecrypto start 继续（若新墙仍在 corecrypto = 间接调用/数据链路径，stubid+body/reference 对拍续办；换组件 = 新墙照流程）③pages/free 行仍在。

**★会话十八续 = 新墙定谳（corecrypto 静态 sysctl 的 handler 鉴权）+ GPU 接装盘点（2026-10-30 上午，用户刷 02d7aa05 后屏摄 IMG_20261030_092650.jpg）**：

- **判读**：桩墙与 PAC 启动墙均未再现，corecrypto 继续起，撞出新类型墙——`PAC failure from kernel with IA key while authing x16 at pc 0xfffffff00ad17e48`（x16=0x7c9ac0700ad18a9c，x9/x17 为字段地址+哈希判别子）。**pc 归属 = native `_sysctl_register_oid_locked+0x1e8`**（emitted→build 偏移 0x346c000 反查）。对象 = `0xffffffff00b6000d0` 处的 `sysctl_oid`，在 **corecrypto 的 __DATA（0xb5f9a90..0xb6058f0）** = donor kext 自己的静态 sysctl；槽 `oid+0x30` 的 donor 链条目 = `(auth=1, key=IA, diversity=0xE2E, addrDiv=0)`，oid_arg1（+0x18）参与判别子哈希。
- **根因（α 家族新形态）**：源码 `ptrauth_auth_and_resign(handler, IA, ptrauth_function_pointer_type_discriminator(typeof), IA, blend(&field, os_hash_kernel_pointer(oid_arg1)))`——旧鉴权用**编译期类型判别子**。Apple 构建对该类型签 0xE2E，我们的 clang 产出 0（=会话十六开放项 "native (IA,0) 默认" 同源），出件按 donor 链条目原样重签 → AUTIZA 恒败。
- **修法（已落件）**：主树 `patches/xnu-12377/sysctl-handler-donor-resign.patch`（进 `profiles/arch/arm64e/patches/xnu/series`）——register 路径改 `ptrauth_strip`（xpaci，不验证）后按本内核判别子重签；unregister/dispatch 用本内核签的判别子，不动。实证 = kernel `8a493864` 反汇编 `ldr x10,[x9,#0x30]! / xpaci x10 / …hash… / pacia x10,x9`，旧 `autiza`+brk 消失；ESP 副本 `1ea8d3c0` 同。
- **出件**：kernel **8a493864** → payload **`a62f5e53`**（96MiB，trust ebeaf7fb + data seed d04a0153，panic_screen 在）。存档 `out/m2381/payload-sysctlfix-a62f5e53.img`；刷写卡 = 只刷 recovery_a ← 此件，boot_b d0200a6b / userdata 不动。
- **GPU 接装盘点（用户令"和上游一样加 GPU"，同 SoC）**：①6 子模块已在 feat pin（Adreno740/AdrenoMetal/AdrenoFirmware/AdrenoPM4/AirNir/ArmSmmu4K，无需 bump）②三件固件按 pin 拉齐（`scripts/fetch-adreno-inputs.py`；**无 token 走 api.github.com 会 403 限流，本轮用 keyring token 补拉**）：a740_sqe.fw 75,380 / gmu_gen70200.bin 66,844 / sm8550/sheng/a740_zap.mbn 12,088，sha256 全对，落 `.cache/firmware/sheng-gpu/` ③`UniXNU-sm8550-common` ≡ 设备仓 `soc/sm8550`（kits/Sm8550Platform 已有 ShengGmu/ShengGmuCommandDb/ShengGpuSmmu）④**唯一缺口 = `ShengGmuVotesSource.h`**（板级 OPP/BCM/带宽表）；消费侧 `ShengGmuVotes.c` **已在 `reference/unixnu-sheng/package/`**，只差数据表，上游公开仓均无该头（gpu.md：kept out of git）⑤上游配方 `build-adreno740-kits.py` 用 `xcrun`（macOS），Linux 需改 toolchain clang/ar ⑥DTS 有 gpu/gmu/gpu_smmu（disabled）但无 OPP 表；sheng 侧只加 `&gpu/&gpu_smmu status=okay` ⑦Metal staging（add-gpu-plugin-data.py --publish）依赖 macOS 镜像工具 → Linux 先做到内核侧 GPU attach/Turnip。
- **风险预注册**：sheng 的 `a740_zap.mbn` 是 OEM 签名件，m2381 的 PIL 鉴权可能拒；兜底 = 读者内核从本机 firmware 分区取自家 zap（dd 通道已有），勿盲刷。
- **★nura 侧供给（2026-10-30，用户问"缺的能从 nura 拿吗"；答案=全齐）**：①**本机固件**在 `~/work/meizu20/meizu20-m1/vendor-dig/vendor-root/firmware/`（另有 `meizu20_linux/vendor-fw-harvest/mainline/lib/firmware/qcom/sm8550/` 同一份）：`a740_zap.mbn` sha `3cc594a0…`（12,088B）、`a740_sqe.fw` sha `2b49a172…`（74,804B）、`gmu_gen70200.bin` sha `1a2a419c…`（66,844B，**与 sheng 件逐字节相同**）。**实锤：zap/sqe 与 sheng 件不同（sheng zap `4fd56fef…`、sqe 75,380B）→ 之前"同 SoC 直接吃 sheng 固件"会翻车**。已按 m2381 pin 落 `.cache/firmware/m2381-gpu/`（qcom/{a740_sqe.fw,gmu_gen70200.bin}, qcom/sm8550/a740_zap.mbn）。②**板级 OPP/带宽数据**在 `~/work/meizu20/meizu20-m1/dts/sm8550-meizu20-stock.dts`：`qcom,gpu-pwrlevels-0` 9 档 = 719/680/615/550/475/401/348/295 MHz（+ 同 level 的低档），每档带 `qcom,acd-level`（0x882e5ffd/0xa82f5ffd/0xe0285ffd/0xc02a5ffd/0xe02b5ffd/0xe02d5ffd/0xc02f5ffd）与 `qcom,level`（0xe0/0xc0/0x90/0x80/0x50/0x40/0x3c/0x38/0x34）+ bus 索引；`qcom,bus-table-ddr` 10 档 = 0/2,136,718/3,000,000/6,074,218/6,671,875/8,171,875/10,687,500/12,449,218/14,398,437/16,500,000 kBps；`qcom,gmu-freq-table = <220MHz 0x40 550MHz 0x80>`。③**BCM catalog** 从 mainline a740 条目（`.cache/adreno-source/.../a6xx_catalog.c` chip 0x43050a01）：SH0 buswidth 16 / MC0 buswidth 4 / ACV fixed+perfmode=BIT(3)+perfmode_bw 16,500,000，`gmu_chipid=0x7020100`。
- **GPU 剩余工序（数据已齐，纯工程）**：①写 `ShengGmuVotesSource.h`（上列三张表；消费侧 `reference/unixnu-sheng/package/ShengGmuVotes.c` 已在树）②`build-adreno740-kits.py` 去 xcrun 化（用 toolchain clang/ar，本机 Linux）③DTS 开 `&gpu/&gmu/&gpu_smmu`（sheng 只加前两个，gmu 的 reg 在 stock 是 0x3d68000 vs 我们 dtsi 0x3d6a000，**需对表核实**）④defconfig `CONFIG_XNU_NATIVE_ADRENO=y`+组件/archive 条目 ⑤payload gpu addon；Metal staging 仍限 macOS。全部在 `CONFIG_XNU_NATIVE_ADRENO=n` 后，**不扰动现役刷写链**。
- **会话十八三刷判读（2026-10-30 09:48 屏摄，刷 a62f5e53）：第 3 堵墙 = crypto KPI 表**：`PAC failure ... while branching to x2 at pc 0xfffffff00aee6958`。pc 归属 = native `SHA512_Init`（libkern/crypto/corecrypto_sha2.c），失败指令 = `braaz x2`（IA、零修饰）尾调 `g_crypto_funcs->ccdigest_init_fn`；`g_crypto_funcs`（全局 0x7cf1c70）→ corecrypto 自己的表 `0x82f3048`（corecrypto __DATA_CONST，文件态全零 = 运行期填）。**x2 = `0x34e36cb0_0a6fa120`：低 32 位恰是 corecrypto 函数 `0xa6fa120`（已验 pacibsp 函数起点、digest-init 形态），高 32 位不是合法 arm64e tag。** donor 里指向该函数的 7 个槽全是 `auth=1, key=IA, diversity=0, **addrDiv=1**` 的链条目（出件链字 0x80110000036f6120，解码 = target 0x36f6120+base、step=2 正确），即 donor 侧按**地址修饰**约定签，我们的消费者按零修饰 branch → 不匹配（=会话十六预注册的 "crypto KPI 表边界" 兑现）。**与 sysctl 墙同源（α 家族），但这次值本身不是规范签名态 → 盲改消费者有风险**；下一步 = 给 `register_crypto_functions`（已是我们的 native 绑定，corecrypto start 时必过）加一次性探针，把收到的表指针 + 前 8 个字打进 panic 行，一个刷次拿到表内全部条目的真值再定修法（probe 后立即撤）。
- **探针件（会话十八四步，2026-10-30）**：`patches/xnu-12377/crypto-funcs-probe.patch`（进 arm64e series）在 `register_crypto_functions` 注册时 panic 打表指针 + 前 9 个字（`unixnu-probe crypto_functions %p [0..8]`），HAS_APPLE_PAC 下无条件生效，**一次性**——判读后从 series 摘掉。出件 kernel `abe0d73c` → payload **`8a40d909`**（96MiB，ESP 内核副本 `0d8c9fb5`，探针串在 ESP 副本内实证；trust/seed 同前）。存档 `out/m2381/payload-cryptoprobe-8a40d909.img`。判读三选一：①4 个函数指针都是规范带 tag（`XXFFFFF00A6FA120` 形态）→ 失败值另有来源；②"高 32 签名 + 低 32 地址"两段式 → 在 `register_crypto_functions` 里按表重签（该函数已是 native 绑定）；③全零 → 表运行期后填，探针位置后移。回退 = 刷回 `a62f5e53`（sysctl 修复件）。
- **探针判读（2026-10-30 10:09 屏摄，IMG_20261030_100927）**：`unixnu-probe crypto_functions 0xfffffff0082f3048` → `[0]1de141b0_0a6fa120 [1]29f21030_0a6fa828 [2]0 [3]61c21530_0a6f8e50 [4]fffffff0082f1b00 [5]fffffff0082f2c30 [6]fffffff0082f2ee8 [7]fffffff0082f28a0 [8]fffffff0082ef338`。**结论：两种指针格式并存**——数据指针（[4]–[8]，md5/sha1/sha256/sha384/sha512 的 di）是规范地址；四个**函数指针**是 `高 32 位签名 + 低 32 位集合内偏移` 的打包形（低 32 位解出 corecrypto 函数 0xa6fa120/0xa6fa828/0xa6f8e50 等），对应 donor 侧 `autiza` 32 位鉴权（kext 集合 ABI），我们的 clang 只产 `braaz` 64 位形 → 恒败；[2] 为 0（该表字段在 donor 侧未填，属预期，不要在后续误报）。**该表在 donor/出件文件里都是全零 BSS（0x82f3048 无任何链条目），值由 corecrypto 运行时写入**——所以 emit 侧修不了，只能修读侧。
- **修法（已落件，替代探针）**：`patches/xnu-12377/crypto-funcs-unpack.patch`（探针补丁已从 series 摘除）——在 `register_crypto_functions`（已是 native 绑定）里把表拷进内核静态副本，逐槽把"打包形"还原为 `0xfffffff000000000|低32` 并按 IA/零修饰重签（`ptrauth_sign_unauthenticated`），`g_crypto_funcs` 指副本；数据指针/规范指针原样保留（判据 = 高位 `((v>>32)&0xffffff)==0xfffff0` 或解出地址不在 `__TEXT_EXEC` 区间则不处理）。此修一次覆盖表上所有消费者（SHA1/256/384/512、HMAC、AES…）。
- **出件（unpack 修法）**：kernel **`892ebfe1`** → payload **`c3b36e61`**（96MiB，ESP 内核副本 `2b22aba1`，探针串已移除实证；trust/seed 同前）。存档 `out/m2381/payload-unpack-c3b36e61.img`；回退 = `a62f5e53`。反汇编实证 = `register_crypto_functions` 内 `mov w2,#0x1c0`（448B 表拷贝）+ 逐槽高位判定/区间判定/重签循环。
- **五刷判读（2026-10-30 10:20 屏摄，刷 c3b36e61）**：**unpack 修法生效**（crypto 表墙消失），新墙 = `Invalid spinlock 0x…: <0x000000002200010f 0x0> @locks_arm.c:343`。锁字 = `lck_mtx` 的 tag `0x22000000|flags`，type 字（+8）=0 → **一个 mutex 被当 spinlock 锁**。**根因 = 会话十七 的三条 lck fixture 类型标错**：donor 0xa7f3548/0xa7f3fe4/0xa7f4128 的体全部操作 **[lock+8]**（`casa 0→cpu` 加锁 / try 加锁 / `casl` 解锁），而 `lck_spin_t` 是 **data@+0、type@+8**（`lck_spin_lock` 先验 `[+8]==0x11` 再锁 `[+0]`），mutex 才是 data@+8——当时按"CAS 0→cpu@+8"认成 spin 是误判。修法 = fixture 三条改名 `_lck_spin_{lock,try_lock,unlock}` → `_lck_mtx_{lock,try_lock,unlock}`（sha 钉不变，只换 native 符号名），设备仓提交。
- **出件（lck 改名修法）**：kernel **`e6217d36`** → payload **`30c11c32`**（96MiB，ESP 内核副本 `f1c9fb0d`）。实证 = corecrypto 里 7×lock / 9×unlock / 1×try / 1×alloc_init 的 call site 全部改指 native mutex 实现（`IOLockLock/TryLock/Unlock` 与 `lck_mtx_*` 同址别名）。存档 `out/m2381/payload-mtx-30c11c32.img`；回退 = `c3b36e61`。

**★会话十九开工单（2026-10-30 交界；用户拍板"安排下一轮，拍屏下会话发"）**：

- **用户动作**：①只刷 recovery_a ← `out/m2381/payload-mtx-30c11c32.img`（sha256 `30c11c32…250e`，100,663,296 B），boot_b `d0200a6b` / userdata 不动；②开机后把 panic 屏整段拍全（若越过了前六堵会是新类型）；③回退件 = `payload-unpack-c3b36e61.img`（已验证能走到 crypto 表墙之后）；④**不要连续多次重启**，保持单一证据点。
- **我这边顺序（含离线先行项，不等刷机）**：
  1. **离线杠杆 = donor 内核导出符号**（目标：把 1659 个未配对函数里的 kext-facing 目标批量按名字绑定，替代"一堵一刷"）。已知证据：kernel entry 的 load command 表里 cmd=`0x32`（off `0x9450`，cmdsize `32`），其后六字 = `0x2 / 0x1a0500 / 0x1a0500 / 0x1 / 0x3 / 0x4f30000`；**文件偏移 `0x1a0500` 处是符号名字符串区**（首条 `AVD_RD_DMA_LVP_COLO_DMA_CONFIG`）→ 导出/符号名区就在附近。步骤：先判 `0x32` 语义（非标准 `0x80000022`；排除 LC_DYLD_EXPORTS_TRIE `0x80000033` 的普通形态），定位 trie 头（ULEB terminal/flags/address 结构），解出 name→address；再按"native 侧同名唯一函数地址"批量落 reviewed fixture（how=`export-name`），逐条写 evidence，唯一性为唯一收编判据。
  2. 若导出表不可用：给 matcher 加**通用通道 = 去 BTI 前缀 + 忽略 adrp/add 常量后比对整函数体**（现在 body-hash 只吃逐字节相同），并把 1659 个 stub 目标按组件出清单 + 调用点数排序，先修热点。
  3. 判读屏摄 → 归类（桩 / PAC 约定 / 32 位打包指针 / 锁类型标错 / 其他）→ 修 → 重建 kernel+payload → 出件命名 `payload-<类>-<sha8>.img`。
  4. 每次修法回写 fixture evidence（判据必录）；映射只收唯一命中。
- **红线**：`CONFIG_XNU_NATIVE_ADRENO` 保持 `n`（GPU 等进桌面）；payload 命令必须带 `--unique-chip-id 3645ad55d47e77c1 --trust-cache out/m2381/ipados-26.5.2-23F84/m2381/original-trust-23F84.bundle --data-ramdisk out/m2381/ci-artifacts/verified-root-grow/owner-data-fixture/scratch-data.img`（漏了会少 trustcache/initrd，r85 坑）；刷写只碰 recovery_a；**SHLOG 在 UFS 起来之前是空的**（写手要 BootProgress 监视器 tick + native_ufs_log_open），早期 panic 别白跑读者任务。
- **判读门**：本墙消失 + corecrypto start 继续；prelink 统计（reviewed / body-hash / reference-order 配对变化，stubbed 数下降）；出件内核 call site 逐点反汇编（布局偏移 0x346c000 口径）。

**★会话十九 = 批量消账：导出符号杠杆判死 + BTI 通道落地 + kext 入口 PAC 墙（2026-10-30 上午，用户刷 30c11c32 后屏摄 IMG_20261030_103414.jpg）**：

- **新墙判读（第七堵）**：`PAC failure from kernel with IA key while authing x16 at pc 0xfffffff00aea68fc`。pc 归属 = native `OSKext::validateKextMapping(bool)+0x130`（emitted→build 减 0x346c000 = 0xfffffff007a3a8fc，源码 xnu `libkern/c++/OSKext.cpp:8165` 的 `ptrauth_auth_data(address, ptrauth_key_function_pointer, 0)`）。x16 = x21 = `0x54d301b0_0a4cdf18`，来自 `ldur x21,[x29,#-0x50]`，源头是 `ldr x21,[kmod_info,#0xb4]` = **com.apple.kec.Libm 的 `kmod_info->start`**。**值解 = 32 位打包指针**：低 32 位 `0x0a4cdf18` = emitted base(0xfffffff007004000) + donor 链条目 target `0x34c9f18` = `0xfffffff00a4cdf18`（Libm __TEXT_EXEC+0x1e18，ESP 内核对位）；高 32 位 `0x54d301b0` 是签名段。**文件态同址** = donor auth 链条目 `0x8010e4fe034c9f18`（isAuth=1 / key=IA / addrDiv=0 / diversity=**0xE4FE** / target=0x34c9f18，donor 与出件逐字节相同）→ 与 corecrypto KPI 表**同族**（donor 侧按编译器类型判别子签的 32 位打包形态），我们的 `autiza` 用零判别子 → 恒败。注：panic 行 OCR 上 `0a4cfd18` 是误读，实为 `0a4cdf18`（裁切放大核对）。
- **修法（已落件）**：`patches/xnu-12377/kext-entry-donor-pptr.patch`（进 arm64e series，位于 crypto-funcs-unpack 之后）——在 `OSKext::initialize` 从 `_PrelinkKmodInfo` 取到 kmod_info 之后归一化 `start`/`stop`：`bits[55:32] != 0xfffff0` 判为打包形态 → `0xfffffff000000000 | 低32` 还原地址并按 (IA,0) 重签；已是规范形态 → `ptrauth_strip` 后按 (IA,0) 重签。实证（native 反汇编，0x7a2d4e0 起）：`ldur x10,[x8,#0xb4] / cbz / and …,#0xffffff00000000 / cmp #0xfffff000000000 / b.ne` → `mov x9,#-0x1000000000; bfxil x9,x10,#0,#0x20; paciza x9; stur x9,[x8,#0xb4]`，`stop`（+0xbc）同形；ESP 内核副本 `c7dfa071` 内实证（`497d40b3`×2、`e923c1da`×2）。**修后 `autiza x16` 得到零判别子合法指针，且 `startfunc(kmod_info,…)` 的直接 `blraaz` 也成立**（只修 validate 不修字段会让下一步直接调用再崩）。
- **★导出符号杠杆判死（工作单第 1 项，证据确凿）**：`LC 0x32 @0x9450` = **LC_BUILD_VERSION**（cmdsize 0x20 → platform=2、minos=`0x1a0500`=26.5.0、sdk=`0x1a0500`、ntools=1、tool=3(Ld)、version=0x4f30000）——工作单把 minos 误当成"符号名区偏移"；文件偏移 `0x1a0500` 处确有串 `AVD_RD_DMA_LVP_COLO_DMA_CONFIG`，但那只是该偏移恰落在数据区，与符号表无关。**donor 全镜像没有任何 name→address 表**：①集合级 load command 无 LC_SYMTAB / LC_DYLD_INFO / LC_DYLD_EXPORTS_TRIE（只有 0x1b/0x32/0x5/0x80000034 + 段 + 298×LC_FILESET_ENTRY）；②298 个 fileset 子镜像逐个带 LC_SYMTAB，但 **nsyms=0 / nstrsize=1**（如 com.apple.kernel：symoff 73728000, nsyms 0, stroff 74190472, nstrsize 1）＝被剥；③镜像里唯一的符号名集合是 `__LINKEDIT` 首部 bplist（`SymbolsSets`，9 个 KPI bundle 共 **6124 个名字，无地址**；另有 `WeakRefFallbackSymbol=_gOSKextUnresolved`）＋ `__PRELINK_INFO` 的 `_PrelinkInfoDictionary`（38 条，只有 `_PrelinkExecutableLoadAddr`/`_PrelinkKmodInfo` 等地址，无符号）。**结论：按名字绑定在 donor 侧无源可依，此杠杆作废，不再投入。**
- **★批量通道（工作单第 2 项）落地 = BTI 前缀通道**：先量化了"为什么大量函数配对不上"——donor 每个函数入口带 `bti c`，我们的 clang 不产；已在 7920 对旧配对中实测：逐字节相等仅 42 对、去 BTI 后相等 443 对、再放宽到 relocation-masked shape 也只是 638 对（Apple 的 codegen 与我们的差异远不止 BTI），所以**不能用"整函数体"做通用通道**，但 **shape 锚点（relocation-masked，唯一性双侧）** 能经命名参照件大幅回收。
  - 实现：`kits/XnuPrelink/fileset_match.py` 新增 `without_bti()` 与 `anchor_bti_shapes()`（`Side.functions` 增记 `bti_digest`/`bti_length`；去入口 BTI + 去尾部 nop 后按双侧唯一绑定，`how='bti-shape'`，并纳入 `anchor_reference` 的可信来源集合）；`MATCHER_VERSION` 8→9（旧 map 自动失效）。设备仓 **bf1dac9**（含 3 条单测；`test_fileset.py` 的 `chain_relocations` 导入错误为上游遗留，与本改无关）。
  - 23F84 实测：**函数配对 7920 → 8450**（其中 reference-symbol 366→1353、reference-order 4796→4472、vtable 592→543、pure-leaf 98→81、bti-shape 9 直接命中——direct 少是因为多数 native 候选已被 order 通道占用，收益主要走命名参照链）；**kext-facing 目标命名 1338 → 1586 / 3094**；**stubbed 去重目标 850 → 700**（−18%；按 owner 求和 1687 → 1471）；**stubbed 去重站点数 3584 → 2642**（−26%；按 owner 求和 8557）。**成本登记**：`closure_pointers_unmapped` 641 → 767（+126，配对重排导致的可映射点变化，观察项）。
  - 三分类工具：`temp/batch-triage.py`（对 report 的 stubbed 表逐目标做 native-unique 形状试探；当前 native 侧唯一可救 128 个目标，其中 127 已被新通道收敛，剩 1 个——继续抠边际极低）。
- **出件**：kernel **`a0f98f77`**（89,374,720B）→ payload **`f7432f8e`**（96MiB；payload.json：ESP 内核副本 `c7dfa071`、trust `ebeaf7fb`、data seed `d04a0153`、ADT `8a5e00d6`、panic_screen=true、kernel_args 与上轮同）。存档 `out/m2381/payload-btibatch-f7432f8e.img`（100,663,296B）。回退件 = `payload-mtx-30c11c32.img` / `payload-unpack-c3b36e61.img`。
- **刷写卡（只刷 recovery_a）**：`payload-btibatch-f7432f8e.img`（sha256 `f7432f8e…bdbd237`，100,663,296B）；boot_b `d0200a6b` / userdata 不动；刷前 sha 门 + 刷后读回对拍。
- **判读门**：①`validateKextMapping` 墙消失（Libm 等 kext 的 start 真被调用，若后面再撞 PAC/桩再按类归）②corecrypto 之后继续起、kext 一个接一个 start（首个新墙位置即下一刀）③stubbed 表应显示 corecrypto/APFS 条目下降；`closure_pointers_unmapped` +126 是否兑现成运行时故障（若新墙落在某被重绑目标上，回退 bf1dac9 的单通道重跑对照）。
- **构建提速（用户问"是不是没多线程"）**：驱动 `unixnu kernel -j` 默认 **4**（`scripts/unixnu/kernel.py:198` 把 `ctx.jobs` 传给 `JOBS`），本机 16 核 → 以后一律 `-j16`；本轮 kernel 用默认 4 跑了两遍（第二遍增量），payload 已用 `-j16`。

**★会话二十 = 第八堵墙：donor corecrypto 的 entropy 回调类型判别子（2026-10-31 凌晨，用户刷 f7432f8e 后屏摄 IMG_20261031_003033.jpg）**：

- **判读**：kext 入口墙（validateKextMapping）与批量消账轮全部生效——corecrypto 继续起、Libm 的 start 已被调用并活着走完了 `init_with_getentropy` 的前半程。新墙 = `PAC failure from kernel with IA key while branching to x8 at pc 0xfffffff00a7098b4`（pc→build −0x346c000 = 0xfffffff00729d8b4 为内核侧相邻符号噪声；**实际 pc 属于 donor corecrypto 条目** `__TEXT_EXEC+0x391b4`，函数 = donor 的 `cckprng_init_with_getentropy`）。现场：`ldp x8,x2,[x19,#0x18]`（x19 = `prng_funcs` 在我们 __DATA 的副本 0xb6042a0，函数表槽 +0x18）→ `mov x17,#0xbcad` → `blraa x8,x17`。**x8 = 0xab883bf0_0a98a660：低 32 位 `0x0a98a660` = `&entropy_provide`（我们内核 `osfmk/prng/entropy.c` 的回调，native 0xfffffff00751e660 → emitted 同址）；高 32 位 = 我们 `paciza` 签的 tag（(IA,0) 约定）。** donor 用 `autiza x8, 0xbcad` 鉴权 → 我们签的 (IA,0) 恒败。
- **根因（α 家族第五形态，闭环）**：`prng_random.c:343` `prng_funcs.init_with_getentropy(prng_ctx, …, entropy_provide, NULL)`——我们把**内核侧回调指针**交给 donor corecrypto；donor 侧该回调类型 `cckprng_getentropy` 的鉴权判别子 = **编译器类型判别子 0xbcad**。**用我们的 clang 复现成功**：同一形状 `int32_t (*)(size_t*,void*,void*)` 在 `-fptrauth-function-pointer-type-discrimination` 下 `ptrauth_function_pointer_type_discriminator` 恰好 = **0xbcad**（`/tmp/gd*.c` 实验，同 host clang）；donor corecrypto `__TEXT_EXEC` 内 `mov x17,#0xbcad` 共 4 处（0xa7096ac/0xa7098b0/0xa71702c/0xa72d244），全部是该回调的存/调点——只签一次交接处即全覆盖。与 sysctl 0xE2E / kext 入口 0xE4FE 同族：**Apple 构建全线开类型判别，我们全线关**。
- **修法（已落件）**：`patches/xnu-12377/prng-getentropy-donor-discriminator.patch`（进 arm64e series，kext-entry-donor-pptr 之后）——`register_and_init_prng` 交接处 `unixnu_donor_getentropy(entropy_provide)` = `ptrauth_sign_unauthenticated(callback, IA, 0xbcad)`。实证（native 0x751d9b4）：`adrp/add &entropy_provide → paciza x16（参数 ABI 签）→ mov x6,x16 → mov x16,#0xbcad → pacia x6,x16 → …blraa x17(0x…3051 交 init_with_getentropy)`；ESP 内核副本 `0ac4d4a8` 内 `0602c1da`×5、`b09597d2`×1。HAS_APPLE_PAC 在该 TU 已可用（bootstrap 补丁同文件先例），零 include 增量。
- **α 家族判别子总表（本线 5 堵全收）**：corecrypto KPI 表（0x36f6120 打包+addrDiv）→ sysctl handler（0xE2E）→ kmod 入口（0xE4FE / kext-entry-donor-pptr 修）→ **entropy 回调（0xbcad，本轮）**；每一刀的通式 = "donor 按它的判别子签/验，我们按 (IA,0)"，修法均为"在交接点按对方的判别子签"。**预测：同为 `CC_SPTR`/回调类型的下一处会在别的 donor→native 函数指针交接上（如有），同一刀法。**
- **出件**：kernel **`14ce27e2`** → payload **`a268cd8d`**（96MiB；ESP 内核副本 `0ac4d4a8`；trust `ebeaf7fb` + seed `d04a0153`；panic_screen 在；kernel_args 同前）。存档 `out/m2381/payload-getentropy-a268cd8d.img`（100,663,296B）。主树 **44cb8a1**。回退件 = `payload-btibatch-f7432f8e.img`（上轮）。
- **刷写卡（只刷 recovery_a）**：`payload-getentropy-a268cd8d.img`（sha256 `a268cd8d…4db12`，100,663,296B）；boot_b `d0200a6b` / userdata 不动；刷前 sha 门 + 刷后读回对拍。
- **判读门**：①本墙消失，PRNG 初始化走完（getentropy 会在 per-CPU init/generate/refresh 反复被调，四个消费点同约定，一次到位）②corecrypto start 完成 → kext 继续逐个 start（下一堵按类归）③若新墙仍是 PAC-failure branching-to-xN 且 xN 低位落在 native `__TEXT_EXEC`，直接套 α 家族总表找交接点。
- **杂项**：本轮 kernel 全量重建用 `-j16`（stamp 变更触发干净重铺），编译+prelink 约 4 分钟（-j4 时代 13 分钟）。上游核过：feat 支线 2 新提交（AdrenoMetal pin bump + 6 个打包/GPU 脚本）与本线零交集，XnuPrelink 上游无新提交；设备仓 vendored fork 的 bti-shape 通道（bf1dac9）待回推上游。

**★会话二十一开工单（2026-10-31 凌晨收尾；设备已刷 payload-getentropy-a268cd8d 在跑，用户下会话交屏摄判读；电脑重启前资产清点=零待回收）**：

- **判读门（entropy 回调修复轮）**：①`branching to x8` 墙消失（x8 低 32 位 = &entropy_provide 的那发 blraa 不再失败）②PRNG init_with_getentropy 走完（per-CPU init/generate/refresh 四个 0xbcad 消费点同约定）③corecrypto start 完成 → kext 逐个 start，首个新墙按 α 家族总表归类。回退件 = `payload-btibatch-f7432f8e.img`。
- **用户已拍板（本会话执行）**：panic 屏加**校验行**——`scripts/xnu-panic-screen.c` 末尾渲染 `crc=xxxxxxxx len=nnn`（CRC32 over PANIC_TEXT 缓冲内容，zlib 口径），拍屏转录后对账杀 OCR 误读（cfd18/cdf18 教训）。约束：panic 屏代码住内核 padding 洞（PANIC_CAVE_END 0xfffffff00b9d0ff0，超限构建报错），CRC 增量 ~200B 预计塞得进；改动 = 主树脚本（b01035d 先例），只刷 recovery_a。
- **排队项（不挡判读）**：①上游 `scripts/build-fileset-release-reference.py` 参数化到 out/m2381 接走参照件构建（消 17.6MB 树外产物债）②设备仓 bti-shape（bf1dac9）回推上游 XnuPrelink ③**二维码整线取消（用户拍板 10-31）**：上游 AdvancedLogDxe/扫传是 UEFI 阶段工具（我们 UEFI 一直正常跑，问题全在内核侧）；panic 屏全 QR 立项一并撤销，CRC 校验行保留为准 ④UART 通道已否（无串口）。
- **基线备忘**：上游已 rebase 到 8fe2aaa（961c7da 记录）；下轮构建一律 `-j16`；刷写红线不变（只 recovery_a，boot_b d0200a6b / userdata 不动，--trust-cache/--data-ramdisk 必带）。

**★会话二十一执行档（2026-10-31 凌晨，两轮屏摄判读+两堵落地+CRC 上岗）**：

- **第九堵=entropy 补丁叠签自伤，已修已实测过墙**：屏摄 011739 → pc=donor corecrypto 回调 wrapper `blraa x8,x17(0xbcad)`，x8 目标=我们 `_entropy_provide`(+0x0)。根因=第八堵补丁 `ptrauth_sign_unauthenticated` 直接叠签（本机 clang 把函数引用物化成 `paciza`，(void*) 转换不剥离→`sign(sign(addr,IA,0),IA,0xbcad)`，autia 对干净址重算必败）；0xbcad 判别子本身没错，错在没 strip。修法=补丁加 `ptrauth_strip`（主树 13e2871；同 sysctl/kmod 已验证 idiom），出件二进制实证单签形态 `paciza→xpaci→pacia(0xbcad)`。上轮"批量轮生效实证"只证明 init 调用过墙，回调本体首次被调即崩——**教训：PAC 交接修法必须反汇编验证单签**。全链路账：corecrypto 自配置函数栈上 funcs 表（每项 pacia(IA,槽址|魔数)）→ 我们逐槽重签循环 → `cckprng_funcs` 六字段序 → funcs[5]=丢 x1 的 shim → cckprng_init stp arg5/6 → [ctx+24]/[ctx+32] + wrapper 注册 [ctx+0xad8]；崩溃 ctx=corecrypto 自有全局 0x0b6042a0。
- **第十堵=AppleImage4 `_kalloc_type_impl_external` 桩，已修**：屏摄 030636 → `unmapped kernel function from 0x08d419ec (x0 0x07f64320 x1 0x4)`。三重判据定名（源序夹逼 kalloc.c:2720 + 签名 `(kalloc_type_view_t, zflags)` + 体形 0xd001000|flags/cpu<<14/尾调 zalloc_ext）。修法=设备仓 fixture 118→119（**31fa4e5**）。方法链沉淀：桩点反汇编→原始 KC 同偏移字节特征→stubbed 账本交叉→map 邻居夹逼→源码函数序。坑：fileset 条目 idsize=0 按预链接序对齐（31 条含 xnu 本体 0x0a73c000+0x980000，与 +0x346c000 全自洽）；bl 目标勿用模式首址（差 8）。
- **CRC 校验行上岗（用户拍板项，主树 c296784）**：渲染 pages/free 后画 `crc=`=zlib crc32 over PANIC_TEXT 本轮拷贝（logn>0 才画）。**首轮实测即立功**：030636 屏 crc=0xe8dbdc06 与重建消息逐字节吻合（132B），转录被证明而非假定。**坑=本车道 panic cave 零余量**（eeedeb9a 版已吃满 0xa30，首版 +51B 构建炸）；瘦身（砍 len= 列、hex() 单调用）后 0xa0d/0xa30 余 0x23；洞后 0x13B 即真数据，不可外扩 PANIC_CAVE_END。
- **出件**：kernel **9e47d053** → payload **5f59a2a0**（96MiB，存档 `out/m2381/payload-kallocfix-crc-5f59a2a0.img`；trust `ebeaf7fb`/seed `d04a0153` 不变）。设备现役 = recovery_a ← 5f59a2a0（用户已刷，030636 屏摄即此件在跑），boot_b d0200a6b / userdata 不动。

**★会话二十二开工单（2026-10-31 凌晨收尾；用户拍板"批量处理 stubbed 墙，下会话分析和修，本轮只提取日志"）**：

- **本轮提取的日志（030636 屏摄，crc=0xe8dbdc06 已验真）**：`fileset: original component called an unmapped kernel function from 0xfffffff008d3ab04 (x0 0xfffffff58bd698a8 x1 0xfffffff00b038fb8)`，pages/free=0x125b8c/0x12544c。caller=AppleImage4 exec+0x7254（站点 0x08d3ab00）；原始 KC 同偏移本尊 bl → donor 目标 **0xfffffe000ae40858** ↔ stubbed 账本 `0x0ae40858`。体形速写：`bti c; cbz x0,slow;` 包装 +8 的 pacibsp 主函数。x0=带 0xf5 tag 的签名数据指针（站点上下文有 `autda x16,x17(0xcda1)` 多样化鉴权族），x1=0x0b038fb8（xnu exec 区间指针，疑回调/cookie）——下会话定性。
- **批量管线工作单（kalloc_type_impl_external=已验证模板，可复制）**：
  1. **输入**：kernel.json `stubbed` 按组件账本（donor 目标 VA + 站点数；全账合计 8552 站点/1471 目标）；map.json `functions`（已配对邻居，order-based）。
  2. **优先级=站点数×组件数**：头号大户 **0x0ae40858**（AppleARMPlatform 343 + WatchdogTimer 28 + AppleImage4 10 = 381 站点，且是多组件共享的内核核心函数）；次席 0x0afc0740（corecrypto 6+2/AMFI?/AppleImage4 6+2 等多组件）；逐表排序。
  3. **自动夹逼**：donor VA → map 邻居（上/下最近已配对）→ 我们构建的对应 native 区间 → xnu/kext 源码函数序候选 → 体形（masked shape）+签名+arg 用法三重验证 → **批量产 fixture**（original/original_size/original_sha256 自动算，evidence 半自动人工过目）。
  4. **无 native 对应者**（Apple-only helper）：小 shim 桥接（fileset-unmapped-call.patch 先例），逐个评估。
  5. **验收门**：重建后 stubbed 账本站点数单调下降；boot 推进过 AppleImage4（下一组件新墙按同法）。
  6. **风险**：fixup 勿盲改（#21 教训）；每条 fixture 必须证据行可回溯；跨组件共享目标修错代价=多组件同崩，先在小站点组件上验。
- **排队项不变**：①build-fileset-release-reference.py 参数化接参照件 ②bti-shape 回推上游 ③UART/QR 维持取消。基线：-j16；刷写红线只 recovery_a；--trust-cache/--data-ramdisk/--unique-chip-id 必带。

**★会话二十二执行档（2026-10-31 凌晨，批量管线轮+工具链税定谳；用户拍板 A+B=三通道落袋+CI 定量实验）**：

- **"为什么不直接用上游的"实证调查（用户问出来的关键情报）**：①上游 `kits/XnuPrelink` gitlink **冻结在 76452c6e**（会话十三 vendored 后零新提交）——bti-shape（会话十九）上游就没有，今天三刀上游更没有；②`UniXNU-sheng` main=**21H461 车道**（os_profile=ipados-17.7.11），其 `feat/ipados27-gpu-parity` 分支=**24A446 且 10-31 当天还在提 GPU trace/pid names=真机深水**，但该仓**无自有 fixture、matcher 未动**；③他们过桩墙靠的是 **macOS Apple clang codegen 与 donor 同构**（BTI 有、FPAC 软检查块无、分配器近→字节级 digest 通道直配），我们=全世界第一个 Linux(lld/clang-24) 建 fileset 内核上机（会话十四已记档），stubbed 大头=**Linux 工具链税**（BTI 无+**FPAC 软检查块 13052 处**+寄存器分配差）。sheng 的件不可刷（板级/ADT/组件集/UEFI 链全不同），工具链姿势可抄。
- **三通道落地（设备仓 b68da7b，MATCHER_VERSION 9→10，22 测试绿）**：①`anchor_bti_checked_shapes`=BTI 剥除+FPAC 检查块删除**组合归一**——旧 anchor_bti_shapes/anchor_checked_shapes 各只处理单一差异，双差函数（BTI 前缀∧检查块）全部漏网=699 目标残留的系统性根因；②`checked_shape` 删块判别子推广：aut x16,x17 两硬编码 → `autia/autda x16,<任意寄存器>`（`(w&0xfffffc1f)∈{0xdac11010,0xdac11810}`；native 全镜像扫描实证检查块仅两形、autda x16,x8/x9 变体在 IORegistryEntry/IOReporter 路径真实存在）；③`alias_thunks`=donor `bti c; b X` 别名 thunk（8B，393 站）配到 X 的 native，含链式，bind 例外表加 alias-thunk。**头号 0xae40858 定谳=`OSMetaClassBase::safeMetaCast`（1679 站/25 组件）**，三重验证=体形（0xcda1/0x38/0xfffc 逐字同）+arg 用法（x1=toMeta 对比 getMetaClass 链）+源序（OSMetaClass.cpp:201/291/385 夹逼）。
- **实测（本机 -j16 全量）**：emit 账本 **8552→4913 站点（−42%）/699→654 目标**；kernel **f763e9e6** → payload **28fe6d56**（96MiB，ESP 内核 905fb6fa，trust ebeaf7fb/seed d04a0153 不变；平铺 `out/m2381/payload-batchfix-28fe6d56.img`）。验收门"站点数单调下降"过。
- **残余分类（temp/batch-fixtures/）**：同形家族 87 目/525 站（Dispatch 家族 15+ 全同体小 thunk，1:1 门正确弃权，需 vtable 槽位/邻居消歧——0xaea0bf8=IOUserClient::Dispatch 48 站已锁）vs 真孤儿 560 目/4354 站（TOP=0xaeb8eb0 IORegistryEntry 区 190 站（registry 递归锁+两虚调+解锁，源序无对应=疑 122.8 新增）、0xaed7e2c IOService 区 168、0xa95ca30 锁函数 127（tpidr+0x1b8 计数器+casa+wfe）、0xaf6236c IOReporter 92）；源漂移类=donor xnu-12377.**122.8** vs 源 121.6（IOSimpleReporter::setValue 多一次调用，0.97 相似）。工具=temp/batch-fixtures/{batchid,fndiff}.py。
- **CI 定量实验（B 腿）**：run 41124044607 秒挂（**pre_build 期望 shell 命令串，`ci-owner-policy` 不是命令=127**；kernel-only 不需要 owner policy）→ 重发 **41124555115**（空 pre_build，kernel-only，xcode 26.6）；主树 m2381-ios27 force-with-lease→远端 m2381（63d7969），设备仓 main 已推 b68da7b。
- **判读单（下会话）**：①**CI 结果=macOS 建 stubbed 计数**→量化工具链税，拍板 B（macOS 建核转正）vs C（Linux `+fpac`，实测有 FPAC 硬陷阱=历轮 donor 侧 PAC failure 同步 panic 即证，开掉 13052 处软检查块）主攻；②若刷 28fe6d56：判读门=AppleImage4 之后下一组件新墙（stubid 同法），4913 站=背景噪声逐墙消；③排队：同形家族消歧通道（vtable 槽位法）、真孤儿 TOP 手写 fixture、pre_build=ci-owner-policy 的正确喂法考证（会话三十五跑通过，当时喂法待查）。**回推上游降级（10-31 用户拍板"真有必要吗"）**：通道集稳定前不发 PR——收益面≈零（全生态只有我们 Linux 建核，B 转正后连自己都不太需要），fork 自用零成本；触发条件=①通道集稳定后顺手发完整版或②出现第二个 Linux 构建者/上游主动来要。

**★会话二十二续 = CI 定量实验结果（2026-10-31 凌晨；**B 腿证明可行，工具链税定谳**）**：

- **喂法三坑全修**：①`pre_build` 期望 shell 命令串非脚本名（`ci-owner-policy` 127——那是 20H392 代 hdiutil 重建机制，27 车道不需要）；②政策对 CI 缺失 → 设备仓 **4b7e23f** 强制入库紧凑集（`.cache/firmware/verified-root-23F84/{OS,SystemOS}/{manifest,trusted-key,verification}` 27MB，`git add -f` 破例有档）+ 主树 `build-sheng-source-storage.py` 加 `.cache/firmware` 回退（**87acd33**）；③board.toml 无条件喂 `--fileset-reference` 而 CI 无 RELEASE 参照件 → 主树 kernel.py 文件不存在时降级去参（**9f0e4ce**；**本地 c8214f7 与远端 9f0e4ce6 同树异 sha**——git 通道 TLS 断连时经 GitHub Git Data API 推的，代理恢复后本地已 reset 对齐）。
- **决定性 run = 41132298280 全绿（11m48s，xcode 26.6）**：macOS 全链（63 补丁系+组件+XNU+emit）一次通过；此前 41124555115 的 `clang++ linker` Error = 设计内容忍的 bootstrap（会话十三既立），非 B 腿障碍。
- **工具链税同口径全表**（同 matcher v10/同 fixtures/同 donor 23F84/**双方都无参照件**）：

  | 指标 | Linux clang-24/lld | macOS xcode 26.6 |
  |---|---|---|
  | functions matched | 7163/21113 | **8324/21113 (+16%)** |
  | body-hash | 56 | **542 (×9.7)** |
  | pac-checked-shape | 350 | **1312 (×3.7)** |
  | vtable | 876 | **1285** |
  | data matched | 541 | **1200 (×2.2)** |
  | bti-checked-shape（我方新通道） | 858 | 28（macOS 无双差体=本就不欠） |
  | kext-facing named | 1517/3094 | **1783/3094** |
  | emit `stubbed functions`（按组件去重目标口径） | （带参照件）1234 | **932（输入更差仍反超 −25%）** |

  参照件在 Linux 值 ~1900 站（4913 vs 6841 site 口径）；CI 未上传 kernel.json，site 口径精确数下轮补（workflow upload 加 kernel.json 或 CI 内建参照件）。
- **判读 = B 治本、C 减税不足**：macOS 无参照已反超 Linux 带参照；pac-checked 350→1312 说明 FPAC 软检查块是 Linux 税大头（C 可把此项追平）但 body-hash ×9.7/vtable/data 差异 C 覆盖不了。**建议主攻 B**：①CI 内建 RELEASE 参照件（消最后一项输入差）②payload/userdata CI 化 ③932 残余=真 Apple-only 尾巴走 fixture/shim（上游 sheng 同款姿势）④本地 Linux 保留快迭代腿，C 作为 Linux 腿减税可选项。
- **出件态**：本地现役 kernel = 8b60dbb4（带参照件重出，ledger 4913/654 与 f763e9e6 全同、二进制 rerun 非确定）、payload **28fe6d56**（平铺 `out/m2381/payload-batchfix-28fe6d56.img`，ESP 内核 905fb6fa=f763e9e6 派生）可刷；CI 侧 kernel 未上传（只有日志）。

**★会话二十三判读 = 第十一堵定谳 = IOStorageFamily → kern_conf 设备开关族（2026-10-31 凌晨，屏摄 061501，fixture b3481fc）**：

- **转录验真**：`fileset: original component called an unmapped kernel function from 0xfffffff009f84d90 (x0 0xffffffff x1 0x0)`，pages/free=0x125b8c/0x12544c，**crc=0x20c30dfb zlib 精确吻合（030636 先例复用，转录逐字节可信）**。stage #1、verifiedK=0 failures=0——root mount 前的 donor 原生组件首发。前墙（AppleImage4 kalloc，第十堵）已过 = batchfix 件全链生效实锤。
- **钻法（新案例全程，已定型）**：拍屏 → CRC 验真 → 从 payload ESP 提 kernel.macho（mcopy，勿用本地旧 out/m2381/kernel=10-25 旧 lane）→ 站点 0x9f84d8c 反汇编 → **outer __TEXT_EXEC 定段 + donor KC 同 VA 截断空间（0xfffffe00 前缀折叠）定组件** = com.apple.iokit.IOStorageFamily → 原始 bl 目标 0xfffffe000ae3efd0 → batchid 夹逼。
- **定谳**：站点 bl 原目标 = donor `_bdevsw_isfree`（bsd_stubs.c 源码逐字吻合：1 参、-1→从 1 起扫、反索引→-index 起扫、bdevsw[] 0x38 步长、memcmp `nobdev` 56B 模板、界 0x17、miss 返 -1）。**整族 6 函数全 stubbed（18 站点）**：0xae3efd0 isfree×3、0xae3f0b4 add×4（内联 isfree+56B 拷贝）、0xae3f248 bdevsw_remove×5、0xae3f44c cdevsw_add×2（0x70 步长+112B 拷贝）、0xae3f5c4 cdevsw_remove×2、0xae3f70c cdevsw_add_with_bdev×2（3 参、内调 cdevsw_add）。donor 侧无独立 cdevsw_isfree（被内联）→ donor 地址序=我们 bsd_stubs.c 序一一对应。caller 流程 = IOMediaBSDClient 注册标准形：isfree(-1)→bdevsw_add→cdevsw_add_with_bdev→败则 bdevsw_remove 回滚+isfree(~major) 重试。
- **ABI 对拍全绿（未动源码）**：nblkdev 双方 24、nchrdev 双方 64（conf.c 已有 `[13 ... 63] = NO_CDEVICE`）、struct bdevsw=0x38/struct cdevsw=0x70 双方一致、NO_BDEVICE/NO_CDEVICE=eno_* 非零指针（零填槽会被 isfree 判占用——conf.c 无此问题）。**教训：grep `[N] = {` 会漏 range designator 条目，别急着改源码。**
- **修法 = fixture 六条**（设备仓 **b3481fc**，fileset-23F84-symbols.json，全部 sha 钉+sym 自验过；matcher 漏配根因未深究——体形差异进 body-hash 都过不了，fixture 通道即定案手段）。
- **出件态**：kernel/payload 本轮重出中（b3481fc 之后）。刷写红线不变：只 recovery_a，boot_b d0200a6b / userdata 不动；回退 = recovery_a ← payload-kallocfix-crc-5f59a2a0.img。
- **下一墙预判**：0xae3fed8（17 站点，_devsw_unlock 之后=疑似 sysctl 邻域）为本族最大残余；stubbed 总账 4913/654。

**★会话二十三续判 = 第十二堵定谳 = AMFI → libkern OSMetaClass/OSArray 块（2026-10-31 早，屏摄 065621，fixture 079aee5）**：

- **转录验真**：`unmapped kernel function from 0xfffffff008f1b150 (x0 0xfffffff00b54bfd8 x1 0xfffffff00755bbdf)`，**crc=0xbfe4144d zlib 吻合**。第十一堵（kern_conf 族）已过 = 8d3a3ca4 件全链生效。
- **定谳**：caller = AMFI 类构造器，原目标 0xfffffe000ae411c8 = **delegating OSMetaClass 构造器**（7 参 this/className/super/size/zone/zoneName/flags；体形 = 基类 ctor 调用 + gIOKitDebug 位 6 检查 + zone_create(zoneName, size, flags|0x2000000=ZC_ZFREE_CLEARMEM, 0xffff, 0) 存 *[x4]；x5 奇地址 0x755bbee = zone 名串，谜底）。返址后 `pacda x16,x17(0xcda1)` 签 vtable 存 [x0] = AMFI 鉴权族。
- **整块 8 目标 17 站点六组件**：0xae40a90=`checkMetaCastWithName(OSSymbol,Base)`（registry walk+vt0x128+super 链 0x10）、0xae411c8=OSMetaClass C2（AMFI×2+IOSurface×4）、0xae42a64=`allocClassWithName(OSString)`、0xae42ab4=`allocClassWithName(char*)`（孪生 0x50B，转换器=hA1 `OSSymbol::withString`/hA2 `withCStringNoCopy`，均已配对；**定名钥匙 = donor withString 源码语义**：OSDynamicCast vs gOSSymbolMetaClass→retain、flags@+0xc bit0=kOSStringNoCopy→withCStringNoCopy(strnlen 0x3fffe)）、0xae42dbc=`getSuperClass`（ldr [x0,#0x10]）、0xae43d58=`OSObject::operator new`（kalloc_type+实例计数 ldadd；donor 序紧邻 reviewed 的 typed_operator_new=native 同序）、0xae43fa4/0xae43fec=OSArray `getNextIndexOfObject`/`getLastObject`（count@0x14、PAC 数组@0x20 disc 0xfeb9、逐字节同 native）。
- **fixture 8 条**（079aee5，sha+sym 自验全过），ledger 4895→4872（−17 精确）；kernel **cbc4c02d**，ESP 内核 793d148d，站点 bl→0xae89b3c=0x7a1db3c+0x346c000 端到端实证；payload **07d60bc1** 存 `out/m2381/payload-libkern-07d60bc1.img`。
- **下一墙预判**：0xae4d098（**91 站**、AMFI/IOGPU/HID/IOSurface/apfs 五组件，OSDictionary 区，体形 = vt[0x100] 转发壳：`ldr x2,[x2]` 后 `braa` 槽 0x100 disc 0xc323）——**需 vtable 槽位法（工作单 B.2 的 `pair_family_by_vtable_slot`），勿手工硬啃**；native 侧 ZTV 槽项是打包指针（0x8011…形，div 0xc323 与 donor braa 逐位同），解码器未破。次候选 0xae45eb8×36（OSArray 区）/0xae46bbc×31/0xae8dbb4×45(apfs)。
- **刷写红线不变**：只 recovery_a；boot_b d0200a6b / userdata 不动；回退 = recovery_a ← payload-kallocfix-crc-5f59a2a0.img。

**★会话二十三三判 = 第十三堵 = AMFI → `_os_log_create`（屏摄 074203，fixture dcffcc1）**：crc=0x88a9cb7 吻合；站点 0x8f0dfd0 = AMFI 调 `os_log_create("com.apple.MobileFileIntegrity", "library_validation_failure")`，NULL 时 csel 回退缓存句柄。donor 0xfffffe000ae8a26c（1408B，夹在 `_os_log_equ`/`__os_log_internal` 间，读 CNTVCT 类寄存器）→ native `_os_log_create`（区间唯一候选 0x7a66f78）。**12 站点五组件**。预测核对：命中"同一未配对尾区继续发"但首发是 12 站的 0xae8a26c 而非 91 站的 0xae4d098——尾巴按调用序而非站点数排序。ledger 4872→4860（−12）；kernel **c12f1711**，payload **9f4d17f3** 存 `out/m2381/payload-oslog-9f4d17f3.img`。下一墙候选不变：0xae4d098×91（vt0x100 转发壳，需 B.2 vtable 槽位通道）、0xae8dbb4×45(apfs)、0xae45eb8×36、0xae46bbc×31。刷写红线不变。

**★会话二十三四判 = 第十四堵 = AMFI → `_IORWLockAlloc`（屏摄 075103，fixture 93b2998）**：crc=0xc4e8e4a6 吻合；站点 0x8f18b9c（caller 无参设置，报告的 x0/x1=残留寄存器——判读注意）。donor 0xfffffe000aeadd18 = 20B 尾调壳（载锁组全局、x1=0、尾调 0xa7f4a44），与 native `_IORWLockAlloc` 形状逐位同构（尾目标都在 reviewed `_lck_rw_init` 正前方）；连带 `_lck_rw_alloc_init`（2 站）一起配。ledger 4860→4848（−10）。kernel **066a8100**，payload **9aec229f** 存 `out/m2381/payload-rwlock-9aec229f.img`。**B.2 开工判定成立**：连续四墙同区（AMFI/libkern 尾区），手钻 ~30 分钟/墙+一次刷机循环 vs 尾区 636 目标打不完；且 AMFI 尾调的下一条就是 0xae4d098（91 站 vt0x100 转发壳，手啃不可行）。B.2 第一刀 = 解 native ZTV 打包指针编码（0x8011…形，div 0xc323 与 donor braa 槽逐位同，donor kmod_info 打包先例 0x8010e4fe034c9f18 可套），第二刀 = `pair_family_by_vtable_slot` 入 matcher，验收 = 0xae4d098×91 及同族 vt 壳批量转正。刷写红线不变。

**★会话二十三五判（收会话）= 第十六堵 = AMFI → `_SecureDTLookupEntry`（屏摄 075955，fixture 5d17955）**：crc=0xc8730321 吻合；站点 0x8f0d480 = AMFI 调 `SecureDTLookupEntry(0, "/chosen", &out)` 判返值==1。donor 0xfffffe000afc0740（664B，IODT 块，夹在 reference-symbol `SecureDTFindNodeWithPropertyEqualToValueHelper` / reference-order `SecureDTInitEntryIterator` 间）→ native `_SecureDTLookupEntry`（0x7babbf4 区间唯一候选）。13 站四组件。**预测 0xae4d098 未中（第三次实证：尾巴按调用序）**；donor 内联子项 helper 0xafc09d8（9 站，native 无独立符号）与 0xafc1b20（PE 小壳身份不稳）均留 B.2 批次。ledger 4848→4837（−11）。kernel **2953e2c5**，payload **9fa4809c** 存 `out/m2381/payload-dtlookup-9fa4809c.img`。**手钻腿到此收档：本会话六墙 4913→4837（−76），全部 SHA 钉 fixture+端到端 bl 实证；此后新屏一律只记坐标归 B.2 批次，不再单独 fixture**（除非 unique-candidate 级别的顺手工件）。刷写红线不变。

**★会话二十三上游侦察(B.3 完成,2026-10-31)——重大叙事修正**：

- **sheng 设备仓公开 = UniXNU/UniXNU-sheng**(devices.toml 实录,小米平板 6S Pro);`feat/ipados27-gpu-parity` 分支 board.toml = **ipados-27.0.1-24A446**。已浅克隆到 `/tmp/sheng-repo/sheng-27`。
- **叙事修正(此前记忆高估)**:porting.toml arm64e-os 区 **status=planned**,limits 明写 "no fileset prelink output exists yet, so the kernel has not booted";26.5.2 证据 = "fastboot OKAY; **kernel and userspace execution remain unobserved**";24A446 显示适配器 "still requires a hardware boot result";GPU 渲染实证**全是 iOS 17 车道的**(记录自认 "historical rendering observations above remain iOS 17 evidence")。**上游在 arm64e/fileset 车道上未上电——我们在 fileset boot 路径与上游并行甚至领先**(我们有内核活体判读循环 4913→4837)。
- **无配对账本可白拿**:设备仓 kits 空(吃 feed)、无 fixture JSON;XnuPrelink 独立仓(76452c6e=我们的 pin,main 仅多 4 docs 提交)无新配对通道。**换 24A446 的"拿他账本"论据不成立 → 留 23F84 定案**。
- **可拿的干货**:①sheng-27 的 `zzzz-sheng-ipados27-bringup.patch`(2002 行)= **Darwin26 用户态 ABI shim 全图**(posix_spawnattr 0x100 布局逐字段:psa_sec_flags@0xac 32位/0xb0 字节/后续+8)——换 27.0.1 时的现成地图,同时反证 26.5.2 正确(我们无此层);②porting.toml 23F84 KC 侦察事实(298 entries/base 0xfffffe0007004000/585131 fixups fmt8/31 roots=1864 kernel targets/42027 branch sites——与我们账本口径互证;KC=xnu-12377.122.8,公共源 121.6 最近);③`ci/macos-workflow` 分支(未并 main;pin macos-26/Xcode 输入/token 防 rate-limit/lld 安装)= B 腿参考;④SM8550 生态玩家(fuxi-on-SM8550 Xyvern08371 等)。
- **B.2 施工修正**:matcher 现版已有 vtable 机制族(`anchor_vtables/pair_vtables/pair_vtable_users/pair_metaclass_vtables/pair_metaclass_getters`)——**开工先读,扩展而非新造**;native ZTV 打包指针解码仍是第一刀。

**★会话二十四收档 = B.2 第一通道落地 = matcher v11 `pair_family_by_vtable_slot`（2026-10-31，设备仓 d0b94c0 已推）**：

- **机制定谳（比开工单预设更简单也更广）**：开工单猜的"native ZTV 打包指针解码器"其实**早已在**——`kc_image.fixups()` 的 fmt8 auth-rebase 通路原生就解（native 0x8011c323_00a23738 = site 0xfffffff007217f40 = `__ZTV12OSDictionary`+0x110，target=base+低 32 位，div=(>>32)&0xffff，与开工单猜测逐位吻合）。真正缺的是 **donor 侧壳函数的配对通道**，而其钥匙 = **槽判别子签名**：
  - donor 转发壳（`braa/blraa` 尾调虚槽 + `autda 0xcda1`）体内 movk lsl#48 判别子集合是**跨构建不变量**（div = 槽身份哈希，非方法/类哈希；OSObject::release div 0x3a87 在 3159 个 donor vtable 同值同 native）。native 对应函数用同一判别子集合 → 签名相等即同源。
  - **首案破案**：0xae4d098（91 站）= donor `OSDictionary::setObject(const char*, const OSSharedPtr<OSMetaClassBase>&)`（**非虚 inline 重载**，`ldr x2,[x2]`=解 OSSharedPtr 拿裸指针再尾调虚槽 0xc323）→ native 0xa27120 同名符号；兄弟壳 0xae4d034=OSSharedPtr<OSSymbol> 重载→0xa270b4、0xae4d068=OSString 重载→0xa270ec。donor vtable 本体 +0x110 槽 → 真身 0xae4acf0（早已配对）。donor 壳比 native 少 xpacd/cmp/brk 校验块（Apple 构建省略）+ 寄存器分配差 = 体形/字节通道必败、判别子通道必中。
- **matcher v11 实现**（`pair_family_by_vtable_slot`，入 match_images 轮环 alias_thunks 后）：签名 = 函数内 movk lsl#48 立即数集（word 掩码：braa/blraa=0xD71F0800/0xD73F0800 & 0xFFFF0C00；movk64=(w>>23)==0x1E5 且 hw==3）；守卫 = 双侧 ≤0x80B、无 calls/refs、尺寸差 ≤0x40、**一 native 被多 donor 认领则全体弃权**。单测 5 例合成（含 xpacd 校验块对拍/伪阳性/冲突弃权），全套 kit 72 测试除 test_fileset 预先存在的导入错外全绿。
- **真件结果（23F84 带件 + reference）**：最终 map **69 条 vtable-slot 配对全语义自洽**（OSSharedPtr 全家、OSOrderedSet/OSSet、IORegistryEntry::propertyExists×6、IODTPlatformExpert NVRAM 族、IOEventSource/IODMAEventSource、vfpthunk、_PE_cpu_signal、OSUserMetaClass::Dispatch）；**kext-facing named 1685→1736**；**ledger 654 目标/4848 站 → 601/4298（−53 目标 −550 站，验收门③达成）**；double-bind 79→99（弃权守卫的安全噪音）。端到端：AMFI 站点 bl → 0xfffffff00ae93120 = 翻译后 native 重载，首指令 `ldr x2,[x2]` ✓。
- **口径教训（本轮差点误判回归）**：prelink.log 打两块（内层 reference-pass + 外层主 pass），**named 基线必须取外层**；ledger 目标数必须 distinct（per-comp 求和会把 654 记成 1086）。
- **验收门②部分达成**：0xae4d098×91 ✓ 转正（经内层 reference-order 级联）；**Dispatch×48 未中**——0xaea0bf8 是 donor vtable 槽目标（255 槽，div 0x9209），但 58 个 native TV 同 div 序列分属 4 类 Dispatch 目标（IOService×46/IOUserClient×10/IOUserServer/TestDextRematch），且 IOUserClient 自有槽（D1/D0/getMetaClass/init/free）donor 侧全未配 = **鸡生蛋**：需先配 IOUserClient::init（donor 0xaf4e544，254 TV 共享，ledger x47）再以伴随槽锁类。留 B.2 续单。
- **出件**：kernel **876a14c7**，payload **2b8b8258** 平铺 `out/m2381/payload-vtslot-2b8b8258.img`（96MiB；trust bundle ebeaf7fb + seed d04a0153 + chip-id 3645ad55d47e77c1）。刷写红线不变：只 recovery_a，boot_b d0200a6b / userdata 不动；回退 = recovery_a ← payload-kallocfix-crc-5f59a2a0.img。
- **下会话工作单**：①B.2 续单 = co-slot 群一致性通道（配 init→锁 IOUserClient 类→收编 Dispatch×48+init×47×2）②大函数残墙（0xaeb8eb0×190 copyProperty 候选/0xa95ca30×127/0xaf6236c×92 需放宽通道:伴随槽+尺寸序）③门④CI 重发对照（B 腿 kernel.json 口径）④判读轮:用户刷 2b8b8258 后预期 AMFI 不再报 setObject 壳。

**★会话二十四开工单 = B.2 vtable 槽位通道转正（2026-10-31 定稿；用户拍板开工）**：

- **第一刀 = native ZTV 打包指针解码器**：已知槽 0x100 打包值 0x8011c323_00a23738（div=0xc323 与 donor braa 槽逐位同），目标 VA 猜测 = 低 32 位 | 0x07000000（0x00a23738+0x07000000=0x07a23738 恰为 VA 低 32 位，kernel base 低位贡献）。验证法：解码 `__ZTV8OSObject`/`__ZTV12OSDictionary` 全部槽项，对拍已命名函数（taggedRetain/getMetaClass 等位置应全中）；donor kmod_info 打包先例（0x8010e4fe034c9f18 = IA/div 0xE4FE/addrDiv0，会话十九档）对照位域。解码器落 `temp/batch-fixtures/`（vtslots.py）。
- **第二刀 = donor vtable 定位**：donor ZTV 无符号 → 用 donor 类构造函数（C2/C1）里的 vptr adrp 反查（先例：AMFI 类构造器 0x8f18b3c `adrp 0x7fac5d0+0x10; pacda x17(0x4e9b); str [x0]` 形）；建立 donor 类名→vtable 映射（类名从已配对函数/`__ZTV`-adjacent cstring 取）。
- **第三刀 = matcher `pair_family_by_vtable_slot`**：未配对目标 f 若出现在 donor 类 C 的 vt 槽 i，且 native 类 C 的 vt 槽 i 已解析出命名函数 → 配对（与 alias_thunks 同班进 fileset_match.py）；首件验收族 = 0xaea0bf8（IOUserClient::Dispatch 48 站）与 0xae4d098（91 站 vt0x100 转发壳）。
- **验收门**：①解码器全槽命中已知名 ②0xae4d098×91+Dispatch×48 批量转正 ③ledger 站点断崖（4848 → 预期 <4600）④CI 重发对照（B 腿口径）。
- **并行**：判读轮若用户送新屏（rwlock 件预期 0xae4d098）按例行钻法收账（CRC 验真→batchid→若 vt 壳直接归 B.2 批次不单独 fixture）。刷写红线不变（只 recovery_a，boot_b d0200a6b / userdata 不动）。

**★会话二十三开工单 = 判读轮 + 预埋轮（2026-10-31 收尾定稿；用户问"还在打地鼠吗"→ 答复=形式不变性质已变：鼠池 4913 站已全分类有底、锤子定型 30 分钟/墙、sheng 实证池打得完）**：

- **A. 判读（用户刷 28fe6d56 后）**：
  0. 刷前现态核对：`current-slot=b`、`partition-size:recovery_a`、boot_b d0200a6b 未动、userdata 未动；设备现役=recovery_a 5f59a2a0。
  1. 刷 recovery_a ← `out/m2381/payload-batchfix-28fe6d56.img`（100,663,296 B），dd 读回对拍才算证据。
  2. 拍屏 → **CRC 行先核转录**（zlib crc32 对拍，030636 先例）→ 有墙走例行钻：`temp/batch-fixtures/stubid.py <site>` → `batchid.py <donor VA>` 夹逼 → 三重验证 → fixture/通道。
  3. 判读门：①原 AppleImage4 kalloc 墙消②推进过 AppleImage4 进下一组件③`PAC failure` 新形态=还是"strip+对方判别子重签"那刀④无墙推进=记 boot 史新坐标。
- **B. 预埋（不刷机并行做，把下一批墙提前填掉）**：
  1. TOP 真孤儿预命名（按站点序，语义分析已起头）：0xaeb8eb0（IORegistryEntry 区 190 站：registry 递归锁+虚调 0x118→release 0x20+解锁，源序无对应=疑 122.8 新增——比 donor 新 tag 源码可破）、0xaed7e2c（IOService 区 168）、0xa95ca30（锁函数 127：tpidr+0x1b8 计数器+casa+wfe，会话十七 lck 先例=配我们实现自洽）、0xaf6236c（IOReporter 92）、0xae4d098（OSDictionary 尾 91）。每个 = 源序夹逼+语义定性+fixture（走 31fa4e5 模板）。
  2. 同形家族消歧通道（525 站）：vtable 槽位法——donor 类 vtable 已配对时，槽位 i 的函数=对应 native vtable 槽位 i 的函数（0xaea0bf8=IOUserClient::Dispatch 48 站已锁，native 侧 15+ 同体 Dispatch 全靠这个消歧）；matcher 加 `pair_family_by_vtable_slot`，与 alias_thunks 同班。
  3. 若 sheng 姿势可借：比对 sheng-27 仓 porting.toml 里对同类桩的处理记录。
- **C. B 腿推进（战略减税）**：CI 内建 RELEASE 参照件（参数化上游 build-fileset-release-reference.py 到 out/m2381=原排队项①，workflow kernel 步前置）→ CI kernel.json 上传（补 site 口径精确数）→ payload CI 化（trusted 材料经 secrets，勿入 runner 盘）。
- **刷写红线不变**：只 recovery_a；boot_b d0200a6b / userdata 不动；回退=recovery_a ← payload-kallocfix-crc-5f59a2a0.img。基线：-j16；--trust-cache/--data-ramdisk/--unique-chip-id 必带；CRC 行为准。

**★会话二十五执行档 = 关键路径排空第一轮：bootpath 调用图清点 + SecureDTGetProperty fixture（2026-10-31 上午，工作单 §7 全单执行）**：

- **A 调用图清点（新工具 `temp/batch-fixtures/bootpath.py`，可复用）**：从 AMFI early_security_init（0x8f13c80）+ AMFI init（0x8f0d230 所在函数）双入口对 donor KC 做静态遍历（线性扫全函数体 = 条件分支双侧保守收集；跨组件 bl 合法跟进；donor function_starts 覆盖实证充足 kernel 21113 条）。坑一处：in_code 校验最初只查 caller 自组件段把跨组件 bl 全误杀（20 函数即止），改全区段后正常。**线性闭包过宽（55 墙/3501 函数）= 内核互联度本性，d≥2 静态分不出冷热**；改 BFS 带深度排序。
- **关键判定（干净）**：**d=0 墙 = 0xafc09d8 SecureDTGetProperty = 实机末屏精确吻合（walker 有效性实证）**；且全图**活链上唯一 d=1 墙-caller = 已配对的 `_SecureDTLookupEntry` → 它的唯一墙就是 0xafc09d8**。AMFI 局部链无其他墙；其余 54 面全为内核中转（d≥2），静态不可排序——历轮教训（尾巴按调用序）下不做投机 fixture。
- **B1 = 零 shim 定案**：donor 0xafc09d8（412B，DT 区边界读全局 [0x7da4b18]/[0x7da4b28]、entry+8 属性数组、0x24 prop 头、4 字节对齐步进、命中返 1/穷尽返 −1）与 native `_SecureDTGetProperty`（0x7bac004，24B 尾调壳，加载同类全局区对）ABI 逐参同构，`_SecureDTGetPropertyInternal`（0x7bac01c，424B）循环体逐指令同 donor。entry 来自已配对 native `_SecureDTLookupEntry`（我们的 DT 树）→ 我们的全局边界即正确值。fixture 一条（设备仓 **020aa2e**，functions 137→138，sha 钉 89831e0f…）。
- **投机甄别记录（不配）**：d=2 墙 0xa8150f0（117 站点、thread_dispatch/sched_update 括号、双 mul 哈希 0x7fb5d329728ea185/0x81dadef4bc2dd44d + 0x18 桶表）与 0xa866894（93 站点、zdestroy/get_thread_ro 括号、页元数据 zone 索引形态）——哈希常量在 native 全文 **零命中**（h2.mid 唯一命中=__global_eventq 数据假阳性）→ native 无对应体（122.8 新增/Apple-only），不硬配。d≥2 其余留观，等实机按调用序点名。
- **出件**：kernel **35761550**（ledger 4298→4289 站点精确 −9，AMFI 0xafc09d8 出账，map `reviewed` 138 条；`0xfffffe000afc09d8 → _SecureDTGetProperty how=reviewed`）→ payload **e5e9556f**（96MiB，100,663,296B；trust ebeaf7fb + seed d04a0153 + chip-id 3645ad55d47e77c1 + panic_screen；kernel_args 同前队）。平铺 `out/m2381/payload-dtgetprop-e5e9556f.img`（sha256 e5e9556f…768ad）。**ESP 副本端到端实证：AMFI 站点 0x8f0d4a8 bl → 0xfffffff00b018004 = native 0x7bac004 + 0x346c000 精确命中**。
- **判读门（下会话）**：①0xafc09d8 墙消失（AMFI early init 的 DT 属性读取走通）②推进到下一墙 = 按调用序点名（bootpath 榜单 + batchid 例行钻法接应；新屏一律 CRC 验真先行）③若无墙推进 = 阶段史记新坐标（root mount 段未至）。**回退 = recovery_a ← payload-kallocfix-crc-5f59a2a0.img**。

**★会话二十五判读轮 = 第十七堵定谳 = native mac_policy_register × donor MAC policy ops 槽 + 整表重签修法（2026-10-31 中午，屏摄 IMG_20261031_102447，payload e5e9556f 已刷）**：

- **本轮先行结论：0xafc09d8 SecureDTGetProperty 墙已过**（新屏是 PAC failure 不是 stub 报告；pages 0x125b8d 比上轮 0x125b8c +1）——AMFI early init 的 DT 读取段走通，bootpath 判定兑现。
- **转录验证不用 CRC 重建**：指令级锁死——ESP 内核 pc 0xb01cfe8 处恰好是 `blraaz x8`，且其前一条 `bl 0xa7bec18`（=native `_printf`）的返回址 = lr 低 32 位（0xb01cfd8）逐位合。
- **定谳**：pc = native `_mac_policy_register+0x3f0`（0x7bb0fe8），lr 同函数 +0x3e0；x0/x19 = 0x82d1950 ∈ **sandbox `__DATA_CONST`** = 被注册的 `mac_policy_conf`；[x19+0x20]=mpc_ops → 槽 +0x398 值 = 0x72ec01f00a52e104（高位 = donor 签名 tag，低位 ∈ sandbox `__TEXT_EXEC`）。源码闭环 = `mac_base.c:745` 的 `mpo_policy_init(mpc)`（printf "calling mpo_policy_init" 在上一行；槽位精确锚定：mac_policy_ops slot 115 @0x398 = mpo_policy_init、slot 116 @0x3a0 = mpo_policy_initbsd，与 disasm 两处调用逐一对应；mac_late=0 时第二处被 `ldr w8,[x24,#0xcd0]` 门掉）。**α 家族第六形态：native 消费者 × donor 数据区函数指针槽**。AMFI policy 早前注册没炸 = 其对应槽为 NULL（cbz 跳过）。
- **修法 = registration 入口整表重签**（`patches/xnu-12377/mac-policy-ops-donor-resign.patch`，主树 **d532452**，arm64e series 尾）：`mac_policy_register` 在 mpc_ops 检查后调 `unixnu_mac_policy_ops_resign`——走全表 **335 槽全函数指针**（编译器实证 sizeof=0xa78），每非 NULL 槽 `ptrauth_strip` + `ptrauth_sign_unauthenticated(IA,0)` 回写。正当性：ops 槽的全部消费者都是本内核 native mac_* 包装器（blraaz IA,0）；donor 侧无人消费自己的 ops 表；native 指针 strip+重签幂等。**可写性先证**：同一调用早前 `mpc->mpc_runtime_flags |= REGISTERED` 写 __DATA_CONST 页成功（否则炸在更早）。
- **出件**：kernel **16cb93f8** → payload **c13089a9**（96MiB，平铺 `out/m2381/payload-macresign-c13089a9.img`，sha256 c13089a9…6c6f8；trust/seed/chip-id/args 全队不变）。刷前门全绿：native 反汇编 = `ldr x8,[x19,#0x20]; cbz; mov x9,#0; 循环 cmp #0xa70/ldr/cbz/xpaci/paciza/str`；ESP 副本 emitted 0xb01cc48 同环在位。
- **判读门（下轮）**：①mpo_policy_init 墙消失（sandbox policy 注册走完；AMFI/其他 policy 同表同修）②mac_policy_register 段过 → 下一段新墙按调用序点名（stubid/bootpath 接应）③若出现 donor __DATA_CONST 写 fault（新类型 panic）= 可写性假设翻车，转"native 副本 + entries[].mpc 换指"备选方案。**回退 = recovery_a ← payload-dtgetprop-e5e9556f.img**。
- **杂项**：串口问题已核——Meizu 20 stock DTS 有 geni-debug-uart（qup_uart@a9c000，stdout-path），Linux 车道用它做 earlycon；iOS 车道 xnu 无 GENI UART 驱动，用户已拍板"之后看吧"。主树未推送提交：184e3ad（STATE s25）+ d532452（本 patch）；设备仓 020aa2e（fixture）。

**★会话二十五二轮判读 = 第十八堵定谳 = sandbox mpo_policy_init 的 lck_grp_attr_alloc_init + 诊断页字体放大（2026-10-31 中午续，屏摄 IMG_20261031_104947，payload c13089a9 已刷）**：

- **第十七堵（MAC policy ops 整表重签）实机生效**：新屏是 stub 报告不是 PAC failure——`mac_policy_register` 已过 mpo_policy_init 的 PAC 关，重签环工作。
- **第十八堵定谳**：`unmapped kernel function from 0xfffffff00a52e140 (x0 0x82d1950=x1 0x0)`。站点 = donor sandbox `mpo_policy_init`（0xa52e104，5416B）内第一条外部调用：`bl 0xa7f2e40`。callee 48B = kalloc_type(lck_grp_attr zone) + 拷 `lck_grp_attr_default` u32 = **`lck_grp_attr_alloc_init`**（调用流：attr_alloc_init → attr_setstat → lck_grp_alloc_init(name,attr) → lck_mtx_init/lck_rw_init ×N，后四者全已配对）。native `_lck_grp_attr_alloc_init`（0x733b258，48B）与 donor **逐指令同构**（仅全局地址差）→ fixture 一条关死（设备仓 **4cea0fe**，139 条，ledger 4289→4285 精确 −4）。
- **诊断页字体放大（用户拍板项）**：小字 = `package/BootProgress.c` 画的 boot 诊断页，上游 scale 规则 `w>=2160&&h>=1400?3:1` 把 Meizu 20（1080×2400）钉在 scale=1（6×14 px 字符）。改 619 行为三段：`>=2160&&>=1400→3 / (>=1000&&>=1400)||(>=1400&&>=1000)→2 / 否则 1`——本机面板落 scale=2（字符 12×28 px）。安全性：`bp_line_bg` 像素循环按 `width/(6*scale)` 钳制（越界不可能）；右侧 ext 小面板（PC 采样/Kernel console 栏）在 120 列×12=1448>1080 时按自身宽度守卫自动跳过（内容原为噪声）。DWARF 行表实证新 623 行编入件。
- **验证工具坑（本轮三次白扫）**：BootProgress.o 的段名是**空字符串**（非 `__TEXT`），按段名过滤的扫描全扫了空气；capstone 线性 disasm 遇填充即停，须跳 4 字节健壮扫（bootpath 同款）；cmp 立即数扫描的 Rd 位写死坑第三次复发（§4.4）。
- **出件**：kernel **556a4efc** → payload **dd0da85b**（96MiB，平铺 `out/m2381/payload-grpattr-font2-dd0da85b.img`，sha256 dd0da85b…；trust/seed/chip-id/args 全队不变）。刷前门：sandbox 站点 0xa52e13c `bl 0xfffffff00a7a7258` = native 0x733b258+0x346c000 精确命中（ESP 副本）；map `reviewed` 139。
- **判读门（下轮）**：①本墙消失、sandbox lock-group 初始化段走完（同函数后续 attr_setstat/alloc_init/lck_*_init 全已配对，理论上可连走）②新墙按调用序点名 ③**屏幕上诊断页字体应变大一倍**（scale 2）——刷机后顺带确认④mac policy 注册段过 = AMFI/sandbox 后续可能撞 sandbox 主体执行路径的新墙。**回退 = recovery_a ← payload-macresign-c13089a9.img**。
