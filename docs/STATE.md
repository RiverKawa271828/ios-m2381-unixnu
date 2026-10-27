# STATE — 现势状态（m2381，2026-10-27 重组）

> **历史全档**：2026-10-24 → 2026-10-27 全部轮次（§0.0–0.34）与当时静态区
> 在 `docs/archive/STATE-history.md`（原样快照）；每轮定谳同时完整活在
> git commit message 里。本文档 = 精简现势版：现状 / 资产 / 方法 / 开放
> 问题 / 下一步 / 轮次索引。

## 1. 一句话现状（r59 判读 = UIScreen SIGSEGV 墙破 + trait 族换墙 + r60 已出件，2026-10-27）

**r59 首轮判读（2026-10-27，acm-reader 拉窗 + 读回对拍 a9144765 ✓）**：
窗 rounds=327/509 uptime=674s 无回卷；**applied=13**（20 parsed，fixup
13a/13b 真执行）；**SIGSEGV ×588 = 0**（UIScreen 垃圾解引用墙死）；
procs=175/spawn=887。**换墙 = SIGABRT ×573 同死点**，REASON "'(null)
returned nil from -traitCollection, which is not allowed.'"：链 =
`SBCoverSheetPresentationManager sharedInstance`（once）→
`SBWallpaperController sharedInstance` → （折叠 raise）；抛点 =
`__UIGetCurrentFallbackTraitCollection+0x1f8`（line 117，
`handleFailureInFunction:`——nil screen 的 `_fallbackTraitCollection`
返 nil → displayScale 0 → 主线程 raise；该函数三个失败分支全 raise、
无构造路径）。**fixup 14** = 取 trait 机器补丁位（VA 0x18a00dc50
`mov x0,x20; bl msgSend$_fallbackTraitCollection`）→ 重定向
`bl +[UITraitCollection _backgroundThreadFallbackTraitCollection];
mov x19,x0`——UIKit 自己的 display 无关懒构造缓存 TC，整族治好。

**r60 已出件（payload-r60-edabbafd，2026-10-27）**：21 条 fixups 全表
对缓存校验过；ADT `1dd13c1b`（21 条 + PackedRAMDisk）；ESP 验四样全过。
**待刷**。

**r58 判读存档**（commit db8a5d2）：XB 断言墙破（applied=11 + sig6=0
+ SB 进应用装载）；换墙 UIScreen trait SIGSEGV ×588（x19=0xf00 垃圾）；
fixup 13a/13b = mainScreen + trait getter 双入口返 nil。r57 存档
（commit 2ec5987）：displayIdentity 墙破（applied=8 + 菊花实拍）。

**r56 = 废轮（教训已入法）**：出件漏 `--data-ramdisk`/`--trust-cache`
→ ESP 无 Data seed/initrd 行/trustcache → Data mount ENOENT panic，
applied=0。出件配方红线见 §7。r57 = 规范重建（seed `d04a0153` / trust
`7416ca4f` == r55e 件；toml initrd 行回归），已刷且**读回对拍已补**
（reader 内 dd recovery_a 前 96MiB sha == `1e7ea60b…` 镜像，2026-10-27）。

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
| recovery_a | **r59**（20 条 fixups + Data seed + trustcache + forensic reader） | `a9144765` | **现役**（已刷 2026-10-27；读回对拍 ✓ == 镜像 sha） |
| userdata | CI 重建 iOS root | `fd43ee70` | **现役**（未被动） |

- **设备现态 = iOS 链（r54），未刷回 nura-min**（用户确认 2026-10-27；
  NCM 无 172.16.42.x 接口 = 非 nura，佐证）。
- **下一发刷写 = r60 `edabbafd`（只刷 recovery_a，set_active b 重置
  预算）**；r59 轮次已闭环（判读 + 读回对拍全绿，见 §1/§7）；下会话 =
  刷 r60 → 跑 8–15 分钟 → 硬断电 → reader 读回对拍 + `fetchchunk` →
  判读 `applied≥13` + trait 族墙是否破 + SB 是否推进过 wallpaper/封面
  层：
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

- **payload 四元（r57 起，r60 沿用）**：kernel `517229e7` / 探针后
  `9252e8f5` / trust `7416ca4f` / 种子 `d04a0153`；ADT `1dd13c1b`
  （**21 条 fixups** 含 10a/10b + 13a/13b + 14，+ 上游 J120AP 身份件，
  < 32 上限）。fixup 表源 = 设备仓 OS profile `text_fixups`。
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

## 5. 开放问题

- **主墙（r59 钉死）**：SIGABRT ×573 = `__UIGetCurrentFallbackTraitCollection`
  line 117 raise（nil screen → nil fallback TC → displayScale 0）。
  fixup 14 已出件（fetch 对重定向到 UIKit 自家后台 fallback TC，懒构造
  缓存、display 无关）→ **待 r60 实机验证**。r58 的 UIScreen SIGSEGV
  墙已由 13a/13b 实锤封死（r59 sig11=0）。
- **开放缝**：mainScreen 入口缓存全局槽文件值 = rebase 指向类对象区
  （`_OBJC_CLASS_$__UISlideriOSVisualElement+0x28`），非 nil 非
  screen——0xf00 的确切来源未钉死；13a 落地后该谜降级为机制线注脚。
- **机制线（真解，排期中）**：isMainDisplay/mainIdentity/mainConfiguration
  全族 nil 同根因 = 我们的显示栈从未向 FBDisplayManager/XB 层注册过
  main display。sheng 同 SoC =y 全通对照（差异 = dpu 钩子绑定 / probe
  车队裁剪 / IOMFB 选择子实现面）；mfb C0（疑似 backboardd）fails=18
  （sel6/0x80000000 → e00002d5）仍是头号嫌疑。
- **chronod sig5 崩溃循环**（r57 实测 ×287）——次优先，主链推进后按
  sheng disable-list 处理。
- **独立主题（不阻塞）**：XS/fileset-KC 重绑定器（fileset KC 无 KASLR 表，
  prelink kit 需重绑定器）；arm64e 专项（透传已验证，FPAC/PACM/自举面/
  再签待专项）；GPU（ADRENO=n，P4 显示门过后主题化）。

## 6. 不能声称的

- 显示墙未破：SpringBoard 未到 home screen（r59 = 应用装载已启动 +
  wallpaper/封面层 trait 断言循环中）；keybag 未考（SB 未到锁屏）；
  触摸/按键/GPU 未上主题。
- =y 车道**一级已实机实证**（PE handoff + 控制台接管 + 推帧自稳 + 真
  显示配置到达 IOMFB 客户端层），且 r57 实证**用户态可画 boot progress
  UI**（菊花实拍）——但 main display 注册链仍断（isMainDisplay=0/
  mainIdentity=nil 族），"用户态显示配置供给链已通"不能声称。
- 类杀 NSAssertionHandler 未出件（兜底项；逐站点 fixup 仍优先——站点
  更准、无全系统断言静音副作用）。

## 7. 下一步（r60 刷写 + iOS 快速迭代循环——2026-10-27 r59 判读后）

**r59 首轮判读（2026-10-27，本会话 acm-reader 拉窗 + 读回对拍即时补）**：

- 窗：rounds=327/509 uptime=674s 无回卷 write_errors=0；读回对拍 ==
  `a9144765…` 即时闭环。
- **三大判读项全中**：`applied=13`（20 parsed，fixup 13a/13b 真执行）；
  **SIGSEGV ×588 = 0**（r58 的 UIScreen 墙实锤封死）；procs=175/
  spawn=887。
- **换墙 = SIGABRT ×573 同死点**（PC `1ea99af74` / slide +1f210000），
  REASON "'(null) returned nil from -traitCollection, which is not
  allowed.'"。链（验活全过）：`SBCoverSheetPresentationManager
  sharedInstance`（once 块）→ `+[SBWallpaperController sharedInstance]
  +0x70` → （raise 折叠）→ terminate。
- **抛点钉死之路（RE 增量）**：REASON 串起点 0x18b3abe33（'%@ returned
  nil...'，隔壁就是 `_UIFallbackEnvironment_NonARC.m`）→ 指针搜出
  CFString 槽 0x1f0186168 → 唯一代码 xref = `__UIGetCurrentFallback-
  TraitCollection+0x1f8`（`handleFailureInFunction:...`，line 0x75）。
  该函数取 `[mainScreen _fallbackTraitCollection]`（= `[screen
  traitCollection]`，nil screen → nil）→ displayScale≤0 且主线程 →
  raise；三个失败分支全 raise、自身无构造路径。
  **工具坑第三枚**：自制 adrp+add 全区扫描的 regex 没转义——0x24 变
  `$` 锚点静默零命中（用 re.escape 修正后拿已知引用点验证过）。
- **fixup 14 出件 → payload-r60-edabbafd**：补丁位 = 该 getter 的取值
  对 VA 0x18a00dc50（`.03,3261c50`，`mov x0,x20; bl msgSend$_fallback-
  TraitCollection`）→ 替换 `bl +[UITraitCollection _backgroundThread-
  FallbackTraitCollection]; mov x19,x0`（UIKit 自家 display 无关懒构造
  缓存 TC，scale>0）——getter 返真对象，三分支 raise 全死，整族治好。
  21 条全表校验；ADT `1dd13c1b`；ESP 四样全过。**待刷**。

1. **r60 刷写 + 测试跑 + 下会话取证**：刷卡（设备现态 = iOS 链，只刷
   recovery_a）：`fastboot getvar current-slot`（=b）→ `fastboot
   --set-active b`（预算重置）→ 刷前 sha256sum 门 → `fastboot flash
   recovery_a out/m2381/payload-r60-edabbafd.img` → 开机菜单默认 xnu
   跑 8–15 分钟（17 分钟回卷红线）→ 硬断电（长按电源）→ 菜单选 reader
   → 读回对拍（dd recovery_a 前 96MiB sha == `edabbafd…`）→
   `fetchchunk` → parser `--all`：判读 `applied≥13`（fixup 14 真执行）
   + trait 族墙是否破 + **SB 是否推进过 wallpaper/封面层**（若崩溃
   消失但卡住无新崩，看 [profile] PS/PP 采样找挂点）。每轮 = 刷
   recovery_a + 一跑一读；ADT/args 不变时 skip 重刷。
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

## 8. 轮次索引（r43 → r60；详情 = commit message + STATE-history 对应节）

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
| r57 | `1e7ea60b` | **displayIdentity 墙实锤破**：applied=8（fixup 9 真执行）+ W1–W3 swap 提前到 t≈85s + 178 procs/907 spawn + **面板画出 boot progress 菊花**；换墙 = XBApplicationController.m:120 断言（'invalid mainDisplay configuration'，SB ×560 同死点 t=66s 起，符号化定谳 isMainDisplay==0 双分支跳 handleFailureInMethod）；读回对拍补欠全绿 | 本会话判读 |
| r58 | `d27c4f36` | **XB 断言墙实锤破**（applied=11 + SB sig6=0）+ SB 进应用装载；换墙 = UIScreen trait SIGSEGV ×588（`_mainScreenThreadSafeTraitCollection+0x2c` 解引用 x19=0xf00 垃圾，12 帧验活全过）；读回对拍即时闭环 | 本会话判读 |
| r59 | `a9144765` | **UIScreen SIGSEGV 墙实锤封死**（applied=13 + sig11=0）；换墙 = traitCollection nil 断言 ×573（`__UIGetCurrentFallbackTraitCollection` line 117；REASON 串→CFString→xref 钉死抛点；SBWallpaperController/封面层链验活全过）；读回对拍即时闭环 | 本会话判读 |
| r60 | `edabbafd` | fixup 14（`__UIGetCurrentFallbackTraitCollection+0xa8` 取值对重定向 `bl _backgroundThreadFallbackTraitCollection; mov x19,x0` @ `.03,3261c50`——UIKit 自家懒构造 TC，整族治好）→ 21 条 fixups；全表校验 + ADT `1dd13c1b` + ESP 验四样全过；**待刷** | 本会话出件 |
