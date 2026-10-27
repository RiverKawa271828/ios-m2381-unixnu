# STATE — 现势状态（m2381，2026-10-27 重组）

> **历史全档**：2026-10-24 → 2026-10-27 全部轮次（§0.0–0.34）与当时静态区
> 在 `docs/archive/STATE-history.md`（原样快照）；每轮定谳同时完整活在
> git commit message 里。本文档 = 精简现势版：现状 / 资产 / 方法 / 开放
> 问题 / 下一步 / 轮次索引。

## 1. 一句话现状（r61 判读 = 两块 raise 全死 + SIGSEGV 垃圾指针族新墙 + **用户拍板转体系路线**，2026-10-27）

**r61 首轮判读（2026-10-27，acm-reader 拉窗 + 读回对拍 169f160c ✓）**：
窗 rounds=374/509 uptime=772s 无回卷；**applied=70**（23 parsed；计数
首次超过 parsed = fixup 页被反复重校验，SB 走得深的新信号）；**r60 的
'unspecified values' 断言零出现**（块 A/块 C 死透，fixup 15 生效）；
procs=175/spawn=909。**换墙 = SIGSEGV ×684 同死点**（PC `180e15fac` /
slide +d20000 / FAR=0xf1）：链全符号化且**已深入 wallpaper UI 构造**——
`-[PBUIPosterWallpaperViewController init]` → `rotateToInterfaceOrientation:`
→ `BSUIAnimationFactory` → `UIView _setupAnimation...` → `UIViewController
view/loadView` → `-[UIView initWithFrame:]` → `_UIViewCommonInitWithFrame`
→ `__UIGetCurrentFallbackTraitCollection+0xac` →
`_backgroundThreadFallbackTraitCollection+0x10` → retain 快路径
`ldr x8,[x0]`，**x0=0xf1 垃圾**（与 r58 的 0xf00 同族病）。

**结构性定谳 + 用户拍板（2026-10-27）**：UIKit 的"缓存单例全局"
（mainScreen 槽 r58 / bgTC 缓存槽 r61）在咱们的无显示环境里**成批变成
垃圾源**，逐发 fixup = 打地鼠（r58→r61 四轮实证）。**用户拍板：下会话
先调研上游（UniXNU docs/porting + sheng/fuxi 设备仓 profile 的 fixup
全清单）和其他 fork（negroni/pong 等）的解法，再定体系方案**——候选 =
①NSAssertionHandler 双方法 log-only 类杀（勘址已毕：
`-[NSAssertionHandler handleFailureInMethod:...]` @ 0x1874494d4 /
`handleFailureInFunction:...` @ 0x187449618，头部 NSLog 路径 =
SHLOG REASON 行来源，补丁只砍 raise 保留日志信号）②全指定 TC 构造
③UIKit 缓存全局垃圾族系统审计 ④机制线 track 2（真显示注册）。r60
的判读与 fixup 15 全案 = commit 6c731dc；r58/r59/r57 = db8a5d2 /
61a41c8 / 2ec5987。

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
| recovery_a | **r61**（23 条 fixups + Data seed + trustcache + forensic reader） | `169f160c` | **现役**（已刷 2026-10-27；读回对拍 ✓ == 镜像 sha） |
| userdata | CI 重建 iOS root | `fd43ee70` | **现役**（未被动） |

- **设备现态 = iOS 链（r54），未刷回 nura-min**（用户确认 2026-10-27；
  NCM 无 172.16.42.x 接口 = 非 nura，佐证）。
- **下一发刷写 = 待体系方案定稿后出 r62**（调研优先，用户拍板——见
  §7）；r61 轮次已闭环（读回对拍 ✓ == `169f160c`），现态 = r61 在跑
  wallpaper 层 SIGSEGV 循环，不影响调研；
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

- **payload 四元（r57 起，r61 沿用）**：kernel `517229e7` / 探针后
  `9252e8f5` / trust `7416ca4f` / 种子 `d04a0153`；ADT `f6dba410`
  （**23 条 fixups** 含 10a/10b + 13a/13b + 14 + 15a/15b，+ 上游 J120AP
  身份件，< 32 上限；≥32 上限前先做体系收敛）。fixup 表源 = 设备仓
  OS profile `text_fixups`。
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

- **主墙定性（r61，用户拍板转体系路线）**：SIGSEGV ×684 =
  `ldr x8,[x0]` x0=0xf1 垃圾（retain 快路径，bgTC 内部）——**与 r58
  的 0xf00 同族**：UIKit 缓存单例全局在无显示环境成批变垃圾源。已封死
  墙链 = displayIdentity（r57）→ XB 断言（r58）→ UIScreen SIGSEGV
  （r59）→ traitCollection nil 块 B（r60）→ unspecified 块 A/块 C
  （r61）。**逐发 fixup 已到收益边界（用户判：打地鼠）**，转体系路线
  （§7）。
- **机制线（track 2，真解不变）**：给显示栈注册 main display（dpu 钩子
  / IOMFB 客户端 / FBDisplayManager 注册链），sheng 同 SoC =y 全实现是
  模板；mfb C0 fails（sel6/0x80000000 → e00002d5）仍是头号嫌疑。
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

- 显示墙未破：SpringBoard 未到 home screen（r61 = 已深入 wallpaper UI
  构造 + trait 机器 SIGSEGV 循环）；keybag 未考（SB 未到锁屏）；触摸/
  按键/GPU 未上主题。
- =y 车道**一级已实机实证**（PE handoff + 控制台接管 + 推帧自稳 + 真
  显示配置到达 IOMFB 客户端层），且 r57 实证**用户态可画 boot progress
  UI**（菊花实拍）——但 main display 注册链仍断（isMainDisplay=0/
  mainIdentity=nil 族），"用户态显示配置供给链已通"不能声称。
- ~~类杀 NSAssertionHandler 未出件~~ **已升格为体系方案候选（r62 主体）**：
  双方法勘址已毕（0x1874494d4 / 0x187449618，头部 NSLog = REASON 行
  来源），补丁形状 = 砍 raise 留日志；是否出件等上游/fork 调研对比后
  拍板（§7）。

## 7. 下一步（**体系路线**：上游/fork 调研 → 系统性方案 → r62；用户拍板停止逐发 fixup——2026-10-27 r61 判读后）

**r61 首轮判读（2026-10-27，本会话 acm-reader 拉窗 + 读回对拍即时补）**：

- 窗：rounds=374/509 uptime=772s 无回卷；读回对拍 == `169f160c…`
  即时闭环。
- **三大判读项全中**：`applied=70`（23 parsed；计数首超 parsed = fixup
  页被反复重校验的新信号）；**'unspecified values' 断言零出现**（r60
  墙死，fixup 15 生效）；procs=175/spawn=909。
- **换墙 = SIGSEGV ×684**（PC `180e15fac` / slide +d20000 / FAR=0xf1），
  全链符号化：`-[PBUIPosterWallpaperViewController init]` →
  `rotateToInterfaceOrientation:duration:` → `BSUIAnimationFactory`
  animate → `UIView _setupAnimation...` → `UIViewController view` →
  `loadView` → `-[UIView initWithFrame:]` → `_UIViewCommonInitWithFrame`
  → `__UIGetCurrentFallbackTraitCollection+0xac` →
  `_backgroundThreadFallbackTraitCollection+0x10` → retain 快路径
  `ldr x8,[x0]`，**x0=0xf1 垃圾**（与 r58 0xf00 同族）。
- **结构性定谳**：UIKit 缓存单例全局（mainScreen 槽 / bgTC 缓存槽）
  在无显示环境成批变垃圾源；assert 族可以逐块杀（已证明），但 SEGV
  垃圾族逐发 fixup = 打地鼠。**用户拍板：停逐发，转体系路线，下会话
  先调研上游与 fork。**

1. **下会话第一步 = 调研（用户拍板：先看上游和其他 fork 的解法）**：
   ①上游 UniXNU 树 `docs/porting/` 五篇的 SpringBoard/bring-up 相关段
   + sheng / fuxi 设备仓 OS profile 的 `text_fixups` 全清单（sheng 同
   donor 同 SoC 全实现——它的 fixup 表与显示栈就是答案模板；fuxi 同
   SoC）；②negroni（SM8450）/ pong（SM8475）等已过 SB 墙的 fork 的
   公开 fixup/显示方案（GitHub 搜索 UniXNU fork + profiles）；③对照
   我们 23 条，产出差距表：他们有而我们没有的（显示注册路径 /
   NSAssertionHandler 类杀 / 全指定 TC / UIKit shim）+ 他们的顺序。
2. **体系方案候选（调研后拍板出 r62）**：
   ①**NSAssertionHandler log-only 类杀**（勘址已毕：handleFailureIn-
   Method @ 0x1874494d4 / handleFailureInFunction @ 0x187449618；头部
   NSLog 路径 = SHLOG REASON 行来源——**砍 raise 留日志**，信号不丢；
   assert 族一次性终结，r43–r61 全家族覆盖）；
   ②**全指定 TC 构造**（scale/idiom/gamut/style/layoutDirection 全具体
   值，杜绝 unspecified 族渗漏——找 UIKit 内建全指定源或 traitCollection-
   WithDisplayScale 组合）；
   ③**UIKit 缓存全局垃圾族审计**（0xf00/0xf1 病根：枚举 UIKit 缓存单例
   槽的文件值 rebase 目标，批量定性哪些在本环境必垃圾）；
   ④**机制线 track 2**（真显示注册——sheng 模板，调研后排期）。
3. **判读/刷写纪律不变**：每轮 = 只刷 recovery_a + 一跑一读 + 17 分钟
   回卷红线 + reader 内读回对拍 + fetchchunk；出件配方红线（三 flag +
   ESP 四样）不变。
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

## 8. 轮次索引（r43 → r61；详情 = commit message + STATE-history 对应节；r62 起 = 体系路线）

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
| r60 | `edabbafd` | **traitCollection nil 断言墙破**（applied=14 + 'returned nil' 零出现）；换墙 = 同函数块 A scale≤0 报 "unspecified values" ×758（bg TC 故意全 unspecified，scale=0）；读回对拍即时闭环 | 本会话判读 |
| r61 | `169f160c` | fixup 15a/15b 生效（applied=70 + 'unspecified' 断言零出现，nil/scale 块全死）；**换墙 = SIGSEGV 垃圾指针族 ×684**（bgTC retain 快路径 FAR=0xf1，与 r58 0xf00 同族——UIKit 缓存全局垃圾族定性）；**用户拍板：停逐发 fixup，下会话转体系路线（上游/fork 调研优先）** | 本会话判读 |
