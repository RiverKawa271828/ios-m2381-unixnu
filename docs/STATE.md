# STATE — 现势状态（m2381，2026-10-27 重组）

> **历史全档**：2026-10-24 → 2026-10-27 全部轮次（§0.0–0.34）与当时静态区
> 在 `docs/archive/STATE-history.md`（原样快照）；每轮定谳同时完整活在
> git commit message 里。本文档 = 精简现势版：现状 / 资产 / 方法 / 开放
> 问题 / 下一步 / 轮次索引。

## 1. 一句话现状（r52 定谳，2026-10-27）

**=y 车道一级（驱动/控制台/OS 推帧）实机全过**：payload `22f8f0bd`
单次引导 593 轮 / 1189s（诊断页 ~2-3min 让位 → 苹果 logo 上屏自稳，
PE handoff + NativeLegacyDisplay 接管实证）；日志环：`DISPLAY stage=10`、
GOP FB 映射 `0xb8000000` 全帧 10,368,000B（=1080×2400×4）、三次全帧
swap 完成（t≈86-88s）、IOMFB Debug Info 给出**真显示配置**（ARGB
layer0、swap completion state=4）——=y 显示栈健康。**但 SpringBoard
未破墙**：sig6 崩溃循环 14 次（census 28/14），backboardd 2/0 稳定
（屏上 logo 冻结 = 最后合成帧）；REASON 506 条同串
displayConfiguration——**抛者首次钉死且 bl-target 验活**：
`-[BNBannerSourceListener initWithServiceDomain:displayConfiguration:
authorizedBundleIDs:]+0x1e8`（BannerKit），链 = SBBannerManager init →
initWithAuthorizedBundleIDs → 该 init → NSAssertionHandler，实参 =
`[<类> mainConfiguration]` 返 nil。r50"抛者更深"悬案收口。applied=5
原班人马（8b/9/10 未醒 = 站点未达，SB 死在更浅处）。设备已刷回
nura-min，下轮**三件全刷**。

## 2. 设备现状（2026-10-27 r52 读窗后 = **nura-min**）

| 分区 | 件 | sha 前 8 | 状态 |
|---|---|---|---|
| boot_b | nura UEFI | `e74c73da` | **设备现态 = nura-min**（用户 r52 读窗前刷回） |
| recovery_a | nura ESP | `0a102220` | 同上（与 FLASHING §6 nura 值对拍一致） |
| userdata | nura root | — | 同上（devsh 7.3.0 在跑，172.16.42.1 可达） |

- **下轮发卡 = 三件全刷**（iOS 链已被 nura 覆盖，r44/r49 规矩）。
- r52 iOS 链三件（a1d51f04 / **22f8f0bd** / fd43ee70）已走完读窗定谳
  使命；boot_b `a1d51f04` 与 userdata `fd43ee70` 仍是现役件可复用。
- 刷写/读回/回滚纪律：`docs/FLASHING.md`。回滚件**只**在 nura 仓
  `/home/ri/work/nura-meizu-m2381/artifacts/`（红线 6；本仓 artifacts/images
  已于 2026-10-26 删除）。
- **发卡规矩（r49 事故定谳）**：①发卡前先确认设备现态——走过 nura-min
  读回流程后 iOS 链已被覆盖，**必须三件全刷**；②payload 只认平铺路径
  `out/m2381/payload-rNN-<sha8>.img`（--out 双层路径歧义已废弃）；
  ③卡上必带刷前 `sha256sum` 门。

## 3. 现役件与资产

- **payload 四元（r51 起）**：kernel `3ab2ace4` / 探针 `474724fd` /
  trust `7416ca4f` / 种子 `d04a0153`；ADT 346c6cde（13 条 fixups +
  上游 688fa88 的 J120AP compatible/product-id//product/audio，< 32 上限）。
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
- **诊断工具链（不入 git）**：`temp/` = symbolicate46.py / disasm48.py /
  blstub48.py / live48.py / cache-extract / logwindow-rNN.img 序列。

## 4. 方法资产（反复要用的）

1. **fixup 惰性应用判活器（r49 定谳）**：text_fixup 挂在
   `vm_page_validate_cs_mapped_slow`，含目标地址的页首次签名校验时才
   patch ⇒ **applied 标记 = 函数真在执行路径；零应用 = 不在路径**。
   不应用不等于修复失败，可能只是目标函数没被执行。
2. **dyld 缓存符号化**：CRASH 行 `+N` 去滑；**先读 REASON 行**（SIGABRT
   的 PC 停在 abort 链，给不了抛点）；bt 帧序内→外，live 判据 =
   **bl-target 校验**（LR 落点必须是真调用指令——r47 修正 r46 的"陈旧帧"
   误判：位置不预示真伪，bl-target 才是判活器）。全步骤见
   memory `dyld-cache-symbolication` + `temp/symbolicate46.py`。
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

## 7. 下一步（r53 工作单——r52 定谳后 2026-10-27）

1. **r53 出件 = fixup 11**：`profiles/os/ipados-17.7.11-21H461/profile.toml`
   `text_fixups` 追加
   `"dyld_shared_cache_arm64.17,670e6a0,ff8302d1fa6705a9,c0035fd6fa6705a9"`
   （BannerKit `-[BNBannerSourceListener initWithServiceDomain:
   displayConfiguration:authorizedBundleIDs:]` 入口 ret 返 self，8b 同
   配方；期望字节 = `sub sp,sp,#0xa0; stp x26,x25,[sp,#0x50]` 已核）。
   ADT/payload sha 变 = 预期正文变更。变体（深层 ret 后下游再崩时）：
   caller `initWithAuthorizedBundleIDs:` 入口 `.17,670e634`（期望
   `f657bda9f44f01a9`）。kernel 不变（3ab2ace4 复用），只重打 payload。
2. **发卡 = 三件全刷**（设备在 nura-min 态）：boot_b←`a1d51f04` +
   recovery_a←新 payload（平铺 `out/m2381/payload-r53-<sha8>.img`）+
   userdata←`fd43ee70`；刷前 sha256 门 + 每槽读回对拍 + `getvar
   current-slot` = b。
3. **读窗定谳（照 r47-r52 例行）**：`parsed=14 applied=6`（fixup 11
   醒来 = BannerKit 站点在路径）；REASON 变化 = 下一墙符号化（照 §4
   方法 2/7）；若 SB 活 = 锁屏形态判读（keybag 首考）/respring 周期；
   swap timeline 与 IOMFB dict 继续对照（显示栈回归检查）。
4. **机制线（并行，不阻塞 r53）**：①解开放缝③（GOT 类引用解码）或
   全区扫 `msgSend$mainConfiguration` 桩反查实现类，定位
   mainConfiguration 供给者；②对照 sheng 的 IOMFB 用户客户端选择子
   实现面，查 mfb sel6/0x80000000 e00002d5 未实现项（=y 栈补全方向）。
5. **独立主题（不阻塞）**：chronod sig5（次优先）；XS/fileset-KC 重绑定
   器；arm64e 专项；GPU/触摸主题化（显示墙破后）。

## 8. 轮次索引（r43 → r52；详情 = commit message + STATE-history 对应节）

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
