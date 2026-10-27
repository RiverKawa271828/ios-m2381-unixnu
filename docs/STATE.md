# STATE — 现势状态（m2381，2026-10-27 重组）

> **历史全档**：2026-10-24 → 2026-10-27 全部轮次（§0.0–0.34）与当时静态区
> 在 `docs/archive/STATE-history.md`（原样快照）；每轮定谳同时完整活在
> git commit message 里。本文档 = 精简现势版：现状 / 资产 / 方法 / 开放
> 问题 / 下一步 / 轮次索引。

## 1. 一句话现状（r55e 现役 + 取证读窗通道立通，2026-10-27）

**=y 显示车道**：r55e 首轮 iOS 判读完成（2026-10-27，acm-reader 拉窗）：
**applied=7**（15 条 parsed、r53 fixup 11 + r47 8a/8b + BLSHBacklightHost
fixup 12 等全生效）、**真显示 swap 三连**（W1/W2/W3 t=243–245s，GOP 映射
b8000000，10.4MB 帧缓冲）、系统 173 进程 / 881 spawn 全生态起跑；**换墙
= SBMainDisplaySystemGestureManager sharedInstance 内 displayIdentity
断言**（SB 自 t=68s 起 SIGABRT 循环 552 次同死点）。

**r56 = 废轮（2026-10-27 判读）**：出件漏 `--data-ramdisk`/`--trust-cache`
两 flag（commit 只记了 `./unixnu payload`）→ ESP 无 scratch-data.img +
toml 无 initrd 行 + 无 trustcache sidecar → 无 PackedRAMDisk →
NativeRamBlock 起不来 → APFS 发布不了 Data 卷 → `volume_seed` 查
UUID 18AB5557 返 0 → ENOENT panic `offline scratch Data mount failed: 2`
（kern_exec.c:8196，t≈1s）；554s 全程 panic-hold 诊断页（procs=1/
spawn=0/**applied=0**，fixup 9 根本没测到）。**ADT 的 PackedRAMDisk
映射只有带 --data-ramdisk 才生成**（payload.py devicetree）→ 手术注入
救不了，必须规范重建。

**r57 已出件并已刷（payload-r57-1e7ea60b，2026-10-27）**：规范命令
重建（kernel 增量 `517229e7`→探针后 `9252e8f5` + 完整 flags）；seed
`d04a0153` / trust `7416ca4f` == r55e 已验证件逐字节相同；unixnu.toml
== r55e 逐字节（initrd 行回归）；ADT `f5eaeb4b` 16 条 fixups（含
fixup 9 `.25,1e64f10`）+ PackedRAMDisk 映射在。forensic reader 已由
payload.py 集成（`[payload] forensic=true` 自动装），add-forensic-entry.py
仅存手术用。

**forensic reader 调试五连**（本会话，9c53eee→cbdd5f0→b692151→r55d→r55e）：

①fb2c5dd1 LFN 破坏（短名覆盖 toml）→ 机制正道 payload.py 插桩
（[payload] forensic=true）；②秒退根因一 = entry 无 dtb → 补
`dtb = "Payloads/forensic.dtb"`（nura sm8550-meizu-20.dtb）；③秒退根因二
= ENABLE_SECUREBOOT=1（sheng 拍来）LoadImage 拒签 → 1→0，**boot_b
换件 = 289614ba**（替代 a1d51f04）；④r55d（087f9ca3 保命 cmdline）
实测定谳 = **内核每必起**（8 核企鹅/UFS 全量枚举，keep_bootcon 留屏
成立），死因实拍 = `Initramfs unpacking failed: damaged header` →
无 /init → `VFS: Unable to mount root fs on unknown-block(0,0)` → USB
永不出现；根因 = build-forensic-reader.py 手写 cpio **12 字段错位**
（newc 契约 13 字段，fuxi 母版即 13）——LoadFile2 投递链实拍证明是通
的；⑤**r55e（b060b512，只换 initrd = 402fb431）已刷 + 读回对拍 + 通道
首验全绿**（lsusb 1d6b:0104 / magic=SHLOG001 / 32MiB 窗在位；reader 内
dd recovery_a 读回 == 镜像 sha）。下会话 = §7.3 iOS 快速迭代。

## 2. 设备现状（2026-10-27 r54 后 = **iOS 链现役**，未刷回 nura）

| 分区 | 件 | sha 前 8 | 状态 |
|---|---|---|---|
| boot_b | 本仓 UEFI（**无 SB 版**） | `289614ba` | **现役**（r55c 配套，ENABLE_SECUREBOOT=0） |
| recovery_a | **r57**（16 条 fixups + Data seed + trustcache + forensic reader） | `1e7ea60b` | **现役**（已刷 2026-10-27；读回对拍顺延下会话 reader 内首步做） |
| userdata | CI 重建 iOS root | `fd43ee70` | **现役**（未被动） |

- **设备现态 = iOS 链（r54），未刷回 nura-min**（用户确认 2026-10-27；
  NCM 无 172.16.42.x 接口 = 非 nura，佐证）。
- **下一发刷写 = 无（r57 已刷 2026-10-27，测试跑进行中）**；下会话取证
  = 菜单选 reader → **先读回对拍**（`dd if=<recovery_a 设备> bs=1M
  count=96 | sha256sum` == `1e7ea60b…`，补本轮欠的刷后对拍）→
  `fetchchunk` → parser `--all` 判读 `applied=8` + displayIdentity 墙
  是否破：
- **ABL 重试预算行为（r55d 实测）**：同槽连败数次后预算耗尽自动落
  fastboot（用户实测一次）。恢复 = `fastboot --set-active=b`（留在 b、
  重置预算，nura FLASHING §3）再刷；自动重置 = r56 BootSlotReset DXE
  主题。
- 读窗从此走 **forensic reader**（菜单选 "m2381 forensic log reader"，
  FLASHING §7）——**iOS 链现役时连 nura-min 回退都不用刷**，reader 在
  payload 里；nura-min 只在 reader 本身起不来时才需要（届时三件全刷
  规矩恢复生效）。
- r54 裸件（1ef5eb63）已被 r55 覆盖用途；r53 件（04f4d803）已定谳
  退役。回滚件只在 nura 仓 `/home/ri/work/nura-meizu-m2381/artifacts/`
  （红线 6）。
- **发卡规矩（r49 事故定谳）**：①发卡前先确认设备现态（NCM 接口/
  devsh 可达 = nura → 三件全刷；iOS 屏幕形态 = 只刷 recovery_a）；
  ②payload 只认平铺路径 `out/m2381/payload-rNN-<sha8>.img`；③卡上
  必带刷前 `sha256sum` 门。

## 3. 现役件与资产

- **payload 四元（r57 起）**：kernel `517229e7` / 探针后 `9252e8f5` /
  trust `7416ca4f` / 种子 `d04a0153`；ADT `f5eaeb4b`（16 条 fixups 含
  fixup 9，+ 上游 J120AP 身份件，< 32 上限）。
  fixup 表源 = 设备仓 OS profile `text_fixups`。
- **applied 常驻集（=n 时代）**：{BTAudio, 5, 6, 7, 8a}。=y 上电后该
  oracle 继续有效（惰性判活器不变）；显示族 fixup（5–10）在真显示栈下
  变睡守，换代时退役。
- **device repo**：`devices/m2381/unixnu-m2381`（schema v2；os_profile =
  ipados-17.7.11-21H461；orphan 分支 `device-m2381` 供 CI）；**UEFI**：
  `devices/m2381/uefi/m2381Pkg`（shengPkg 骨架 + mars blob @036ba9f7）。
- **donor**：iPad7,1 / iPadOS 17.7.11 `21H461`（IPSW 本地已核验；20H392
  资产保留回溯）。unique-chip-id `3645ad55d47e77c1`。
- **=y 车道（r51 施工完成）**：组件源码在主树 `drivers/GopFramebuffer`
  （组件 include `GopFramebuffer`，pin cf3b429）；设备仓
  `scripts/build-native-legacy-display.py`（audit → ABI 头漂移门 →
  358 vtable → 字节门 → bindings → probe/GOPPresenter → late.a，probe
  车队已裁剪）+ `package/LegacyDisplayProbe.cpp`（fork，五外探针拔除）+
  `package/LegacyDisplayDPUSubstitute.cpp`（weak dpu 钩子零替身）+
  `scripts/mobile-graphics-legacy-abi.json`（runtime 钉的 audit 产物，
  storage 静态 pin 先例）；telemetry 按
  `XNU_NATIVE_LEGACY_DISPLAY_QUALIFIED` 条件让位；defconfig 双文件 =y。
  换代成本 = `docs/PLAN-display-lane.md` §2（全脚本化重生成）。
- **ADT 变更预告（上游 `688fa88`）**：下轮 payload 的 ADT 会因 21H461
  profile 新增 product-id/J120AP compatible//product/audio 而变化——
  属预期正文变更，payload sha 变化不是事故。
- **诊断工具链**：符号化工具集已收编入设备仓
  `devices/m2381/unixnu-m2381/scripts/re-tools/`（见 §4.2）；
  `temp/` 只留 cache-extract（4.4G 解码缓存）+ 近两轮日志窗
  （r52/r53）+ 当轮解析产物。2026-10-27 清理 = 旧轮 payload
  r49–r55d、老 logwindow r42–r50、r49/r50-verify、r51 构建日志、
  B2 期 tools/（adt/mkadt/remotezip）全数删除。

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
   （0x7F800000 丢 bit31）；xref 必须**全区扫**（跨段引用 >±24MB）。
5. **CFString 定位路径**：REASON 串 → 裸串/CFString 候选 →
   `bl _objc_msgSend$handleFailure…` 桩（各二进制私有）→ 断言站点
   x6/x7 串回读。
6. **defconfig 铁律**：改 defconfig 后必须手动重跑 defconfig 命令。
7. **bt 用户帧可信度分代（r52 定谳）**：=n 时代 bt 用户帧多为陈旧帧
   （r50 实锤，live 链只有 terminate 机器）；=y 环境 r52 的 bt 用户帧
   首次全活（bl-target 三帧全过）。**无论哪代，bl-target 验活必做**；
   去滑先核 N 值（r48 帧列表曾有 0x10000 转录滑差 → 符号落 HomeUI
   垃圾 = 红旗）。

## 5. 开放问题

- **主墙（r52 钉死）**：SpringBoard 崩溃循环 = BannerKit
  `-[BNBannerSourceListener initWithServiceDomain:displayConfiguration:
  authorizedBundleIDs:]+0x1e8` 断言 displayConfiguration 空；实参 =
  `[<类 @GOT .33 0x1e9502290> mainConfiguration]` 返 nil。两条路：
  ①**fixup 11**（该 init 入口 ret 返 self，8b 同配方，三元组已备 =
  `dyld_shared_cache_arm64.17,670e6a0,ff8302d1fa6705a9,c0035fd6fa6705a9`，
  备选 caller 入口 `.17,670e634`）——继续 oracle 推进；②**机制线**（真解）：
  mainConfiguration 供给者为何 nil——嫌疑 = IOMFB 用户客户端选择子
  半数失败（mfb C0 pid=76（疑似 backboardd）calls=30 fails=18，
  sel6/0x80000000 → e00002d5 ×2，杂 e00002bc/c7）+ sheng 同 SoC =y
  全通对照（我们 =y 栈与其差异 = dpu 钩子绑定 / probe 车队裁剪 /
  IOMFB 选择子实现面）。
- **开放缝（定位收口遗留）**：①auth-rebase 编码 CFString 指针重扫（修正
  掩码 `ptr & 0x3FFFFFFFFF`，重扫被用户叫停中）；②.23 区两裸串引用点
  （0x1c943f1ec / 0x1c94419e8，垃圾符号）未反汇编定性；③（r52 新）GOT
  类引用 0x1e9502290 存储值 `0x201f4a63ee0` 不合 `VA|0x400_00000000`
  约定，auth-rebase 掩码待定（与①同族）。
- **chronod sig5 崩溃循环**（r52 实测 ×9，较 r50 ×178 降温）——次优先，
  主链推进后按 sheng disable-list 处理。
- **独立主题（不阻塞）**：XS/fileset-KC 重绑定器（fileset KC 无 KASLR 表，
  prelink kit 需重绑定器）；arm64e 专项（透传已验证，FPAC/PACM/自举面/
  再签待专项）；GPU（ADRENO=n，P4 显示门过后主题化）。

## 6. 不能声称的

- 显示墙未破：SpringBoard 未到 home screen（BannerKit 崩溃循环中）；
  keybag 未考（SB 未到锁屏）；触摸/按键/GPU 未上主题。
- =y 车道**一级已实机实证**（PE handoff + 控制台接管 + 推帧自稳 + 真
  显示配置到达 IOMFB 客户端层），但**用户态显示配置供给链仍断**
  （BannerKit mainConfiguration nil）——"=y 修复 displayless 用户态墙"
  不能声称，r52 REASON 同串即证。
- 类杀 NSAssertionHandler 未出件（兜底项；r52 后仍是兜底，fixup 11
  优先——站点更准、无全系统断言静音副作用）。

## 7. 下一步（r56 刷写 + iOS 快速迭代循环——2026-10-27 r55e 判读后）

**r55e 首轮判读（2026-10-27，本会话 acm-reader 拉窗 32MiB SHA 对拍）**：

- 窗：rounds=314 slots=314/509 uptime=628s（用户跑 ~10.5 分钟后硬断电，
  未回卷，最早证据完好）；write_errors=0。
- **三大判读项全中**：`applied=7`（尾槽）；**W1/W2/W3 真显示 swap**（
  t=243.275/244.712/245.286s，done=1410/188/207ms；GOP=b8000000
  10.4MB）；诊断页 deadline 退出后苹果 logo 常亮（swap timeline 与
  屏幕形态吻合）。
- **换墙 = displayIdentity 断言**：SB 自 t=68s 起 SIGABRT 循环，552 次
  崩溃 PC/LR/bt 逐字节相同（slide +a57c000）。符号化（1020 万 nlist +
  验活 LR-4 全 bl）：applicationDidFinishLaunching → SBProximity-
  BacklightPolicyFactory createNew… +0x8c → SBMainDisplaySystemGesture-
  Manager sharedInstance（once 块）→ super2 `SBSystemGestureManager
  _initWithDisplayIdentity:`，identity = `+[FBDisplayManager
  mainIdentity]`（= sharedInstance ivar@8，无显示栈从不注册）= nil →
  NSParameterAssert（SBSystemGestureManager.m:119，'Invalid parameter
  not satisfying: displayIdentity'）。注：SBMainDisplay 自己的
  _initWithDisplayIdentity: 是 'unallowed initializer'（line 53）陷阱，
  sharedInstance 特意走 super 绕开——**勿修陷阱，修父类**。
- **fixup 9 出件**：`.25,1e64f10` `-[SBSystemGestureManager
  _initWithDisplayIdentity:]` 入口 `ret` 返 self（同 8b 形）→
  **payload-r56-db3a7392**（16 条 fixups，ADT 内字串实证在位 + forensic
  entry LFN 读回 OK）。
- 工具坑两枚已修/转正：acm-reader fetch 行过滤丢 `+`/`/`（修正
  43..122）；单流 ~13MiB 必断（ACM gadget 长流卡死）→ `fetchchunk`
  分块拉（2MiB×16，每块新会话，16/16 + SHA MATCH 31s）。

1. **r57 测试跑 + 下会话取证**：r57 已刷（2026-10-27，set-active=b +
   sha 门 + 只刷 recovery_a）→ xnu 跑 8–15 分钟（17 分钟回卷红线）→
   硬断电。下会话 = 菜单选 reader → **先读回对拍 recovery_a ==
   1e7ea60b**（补欠）→ `fetchchunk` → parser `--all`：判读 `applied=8`
   （fixup 9 生效）+ displayIdentity 墙是否破（预期新墙：identity 消费
   者 nil 比对链或 SB UI 下层）→ 下一发 fixups。每轮 = 刷 recovery_a +
   一跑一读；ADT/args 不变时 skip 重刷。
   **出件配方红线（r56 废轮教训入法）**：payload 命令必须带全三件——
   `UNIXNU_UNIQUE_CHIP_ID=3645ad55d47e77c1 ./unixnu build payload -D
   devices/m2381/unixnu-m2381 --trust-cache
   out/m2381/trust/original-trust.bundle --data-ramdisk
   devices/m2381/unixnu-m2381/.cache/firmware/owner-data-fixture/scratch-data.img`；
   缺 --data-ramdisk = ADT 无 PackedRAMDisk 映射（手术救不了）→
   Data mount ENOENT panic；缺 --trust-cache = ESP 无信任缓存 sidecar。
   刷卡前对 ESP 验四样：scratch-data.img 在、toml 有 initrd 行、
   trustcache 在、ADT 有 PackedRAMDisk。
2. **拉窗纪律**：判定只看宿主（屏幕黑不阻塞）；判读前先看 header
   rounds/slots_written（跨引导不清零，parser 已守计数契约）；若
   `fetch` 断流直接换 `fetchchunk`，勿反复重试单流。
3. **显示域抖动（次优先，不阻塞）**：fw_devlink=off 下 probe 顺序非
   确定，DSI/面板序列偶发失手 → fbcon 丢显示（3/5 黑屏的候选解释）。
   需要屏显判读再议（panel/DSI probe 依赖整序）。
4. **ABL 重试计数自动重置（BootSlotReset DXE，用户需求 2026-10-27）**：
   机制实证（nura FLASHING §3）= 计数在 **boot_b 的 GPT entry 属性位**
   （Android A/B：bits 52-55 tries_remaining / bit 56 successful），
   `fastboot set_active b` 重置预算，nura 用 qbootctl 起机 mark
   successful；misc 分区与直刷链无关（不碰）。**方案 = UEFI 侧**：
   m2381Pkg 加 BootSlotReset DXE（起机读 GPT LBA2-33 → boot_b entry
   属性清 tries/successful=1 → 重算 entry array + header CRC32 →
   写 primary/backup），**配套改 XNU 侧 BootLog 的 GPT check 豁免
   boot_b 属性位**（否则 gpt_open != gpt_last 写手停 = 日志窗停更，
   r49/r50 的 gpt 0->0 机制）；红线注记 = 只动属性位 + CRC（nura
   qbootctl 先例，不动分区数据区）；手动恢复 = `fastboot set_active b
   && fastboot reboot`（用户验证过）。
5. **独立主题（不阻塞）**：chronod sig5；XS/fileset-KC 重绑定器；
   arm64e 专项；GPU/触摸主题化；AdvancedLogDxe；records 页。

## 8. 轮次索引（r43 → r57；详情 = commit message + STATE-history 对应节）

| 轮 | payload | 定谳 | 详情 |
|---|---|---|---|
| r43 | `764539c9` | policy panic 过墙（kern_exec.c:8043 墙清除） | history §0.24 |
| r44 | `a51aed16` | 崩溃簇符号化建成；FBS init ret + liveactivitiesd 禁用 | history §0.26 |
| r45 | `d3c11770` | 两修复实机生效；推进到第二层显示断言（BKSDisplayServices） | history §0.27 |
| r46 | `c3a24827` | mainScreen nil（fixup 7）；legacy-display 车道评估 | history §0.29 |
| r47 | `821ab7c0` | 背光服务链（fixup 8a/8b）；ABI 头入仓、=y 配方映射 | history §0.30 |
| r48 | `036e712f` | workspace 转换请求层（fixup 9）；CFString 解码方法 | history §0.31 |
| r49 | — | 首试作废（旧载荷被刷，路径歧义）；二试：fixup 9 未执行，惰性 oracle 确立，fixup 10 三连修 | history §0.32/0.33 |
| r50 | `4e69f450` | oracle 全排除（五站点全灭，抛者更深）；定位收口，转上游同步+调研 | history §0.34 |
| r51 | `22f8f0bd` | 上游同步 f85b5a6 + 调研收口（=y 是唯一机制解）+ **=y 车道施工出件**（内核 3ab2ace4，七轮修四墙） | PLAN-display-lane.md |
| r52 | `22f8f0bd` | **=y 一级实机全过**（PE handoff+真显示配置+swap+屏幕自稳）；SB 崩溃循环定谳 = BannerKit displayConfiguration 断言（深抛者钉死+验活）；设备回 nura-min | history §0.35 |
| r53 | `04f4d803` | **fixup 11 生效**（applied 5→6，BannerKit ret）；SB 推进到 BLSHBacklightHost brk 新墙（sig5）；W1 swap 异常→撕裂 logo 新观察 | history §0.36 |
| r54 | `1ef5eb63`（裸件） | fixup 12（BLSHBacklightHost 入口返 nil）；判读被 reader 调试顺延 | history §0.37 |
| r55a–c | `fb2c5dd1`→`69538b41` | reader 三连翻车定谳：LFN 毁（短名覆盖 toml）→ entry 缺 dtb 秒退 → ENABLE_SECUREBOOT=1 拒签（boot_b 换 `289614ba`）；payload.py 插桩转正 | history §0.37 |
| r55d | `087f9ca3` | 保命 cmdline 定谳：内核每必起（企鹅/UFS 全枚举），死因 = **initramfs cpio 手写 12 字段错位**（damaged header 实拍）→ root panic → 无 USB 同根因；LoadFile2 投递链实证是通的 | history §0.38/0.39 |
| r55e | `b060b512` | **取证 reader 通道立通**（1d6b:0104 + magic=SHLOG001 + 32MiB 窗在位）；reader 内读回对拍闭环；**首轮 iOS 判读**：applied=7 + W1–W3 真 swap（t=243–245s）+ 173 进程全生态，换墙 = displayIdentity 断言（SB SIGABRT ×552 同死点，符号化定谳 FBDisplayManager.mainIdentity=nil） | history §0.39/0.40/0.41 |
| r56 | `db3a7392` | **废轮**：出件漏 `--data-ramdisk`/`--trust-cache` → ESP 无 Data seed/initrd 行/trustcache → Data mount ENOENT panic（kern_exec.c:8196，t≈1s），554s panic-hold 诊断页，applied=0，fixup 9 未测到；ADT PackedRAMDisk 映射依赖 seed flag → 必须规范重建 | 本会话判读 |
| r57 | `1e7ea60b` | 规范重建出件（seed d04a0153 / trust 7416ca4f == r55e 件，toml==r55e，ADT f5eaeb4b 16 fixups 含 fixup 9 + PackedRAMDisk）+ 已刷（读回对拍顺延）；**测试跑进行中，下会话取证判读 applied=8 + displayIdentity 墙** | 本会话 |
