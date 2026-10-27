# STATE — 现势状态（m2381，2026-10-27 重组）

> **历史全档**：2026-10-24 → 2026-10-27 全部轮次（§0.0–0.34）与当时静态区
> 在 `docs/archive/STATE-history.md`（原样快照）；每轮定谳同时完整活在
> git commit message 里。本文档 = 精简现势版：现状 / 资产 / 方法 / 开放
> 问题 / 下一步 / 轮次索引。

## 1. 一句话现状（r55c+289614ba 上机，reader 黑屏待查，2026-10-27）

**=y 显示车道**：r53 fixup 11 生效（applied 6），SB 推进 BLSHBacklightHost
brk 新墙；r54/r55c = fixup 12 + forensic reader（待判读，iOS 判读被
reader 调试占用）。

**forensic reader 调试三连**（本会话，commits 9c53eee→cbdd5f0→b692151）：
①fb2c5dd1 LFN 破坏（短名覆盖 toml）→ 机制正道 payload.py 插桩
（[payload] forensic=true）；②秒退根因一 = entry 无 dtb → 补
`dtb = "Payloads/forensic.dtb"`（nura sm8550-meizu-20.dtb）；③秒退根因二
= ENABLE_SECUREBOOT=1（sheng 拍来）LoadImage 拒签 → 1→0，**boot_b
换件 = 289614ba**（替代 a1d51f04）。r55c（69538b41）+289614ba 实机：
**无闪回（LoadImage 过了）但黑屏无内核滚字，USB 零枚举 = 内核 early
挂死**（gadget 都没到）。下会话 = 黑屏定位（工作单 §7）。

## 2. 设备现状（2026-10-27 r54 后 = **iOS 链现役**，未刷回 nura）

| 分区 | 件 | sha 前 8 | 状态 |
|---|---|---|---|
| boot_b | 本仓 UEFI（**无 SB 版**） | `289614ba` | **现役**（r55c 配套，ENABLE_SECUREBOOT=0） |
| recovery_a | r55c（fixup 12 + forensic reader + dtb） | `69538b41` | **现役已上电**（菜单选 reader 黑屏中） |
| userdata | CI 重建 iOS root | `fd43ee70` | **现役**（未被动） |

- **设备现态 = iOS 链（r54），未刷回 nura-min**（用户确认 2026-10-27；
  NCM 无 172.16.42.x 接口 = 非 nura，佐证）。
- **r55 刷写 = 只刷 recovery_a**（boot_b/userdata 未被覆盖，r44/r49
  "三件全刷"规矩只在 iOS 链被 nura 覆盖时触发——现态合法快捷卡）：
  ```bash
  fastboot getvar current-slot      # 必须 = b
  sha256sum out/m2381/payload-r55-fb2c5dd1.img   # 前 8 = fb2c5dd1
  fastboot flash recovery_a out/m2381/payload-r55-fb2c5dd1.img
  fastboot reboot
  ```
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

## 7. 下一步（reader 黑屏定位工作单——2026-10-27 交接）

**现场态**：设备黑屏挂在 reader 内核 early（USB 零枚举、无 NCM、无
fastboot；宿主已确认）。r55c+289614ba（无 SB）上电，无闪回 = LoadImage/
StartImage 都过了，内核被跳进但 early 死。

1. **黑屏定位（优先）**——候选假设按成本排：
   a. **（用户实测纠正）内核滚了几行再黑，且一下子消失拍不到** =
     内核开始执行、early 输出可见后死在 very early（USB 零枚举）。
     "滚几行"大概率 = EFI stub 输出（Booting/DTB/Exiting boot
     services）或 early console；随后黑 = fbcon 未接管/内核死。
     **捕获手段三选（拍不到就用后两个）**：
     ①手机**慢动作录像 240fps** 拍黑屏前几帧（零成本，第一试）；
     ②**ramoops 闭环（首选）**：reader 的 DTB（nura sm8550-meizu-20）
     **自带 ramoops@a8000000**（console-size 0x7d800 保留内存）——
     内核 console/pstore 写入后即使挂死，**重启刷回 nura 读
     /sys/fs/pstore/console-ramoops** 即得死亡现场（要求 nura 的
     stock/vendor_boot DTB 同节点——同源 apk，大概率在；若 nura 侧
     pstore 空则需确认 no-map 保留区未被 ABL 覆写）；
     ③UART（ttyMSM0，cmdline nura 形态有 qcom_geni_serial.con_enabled
     ——物理接引脚查 nura 仓/HARDWARE）。
   b. 候选假设（按 ramoops 内容定）：UEFI ExitBootServices 后内存
     map 冲突（我们高段 GCD 0x8c0000000 布局 vs nura Mu 的 map，
     EfiBootServices/保留区重叠 → 内核 early panic）；DTB 与内核
     版本错位（内核=v71/r91 ESP 件，DTB=apk rc6-r91，核对同 build）；
     保留区（ramoops@0xa8000000 等）被我们固件/内核占用；EL2/内存
     属性差异（m2381Pkg vs nura Mu）。
   c. **console 补强**：cmdline 换 nura 形态 `console=ttyMSM0,
     115200n8 earlycon keep_bootcon`（m2381 UART 引脚查
     docs/HARDWARE，nura FLASHING 可能记录了 UART 接法）。
   d. **兜底形态**：不走 UniXNU 菜单 LoadImage，改 nura 原生路径
     （取证专用 ESP：nura BOOTAA64.EFI 同件由 nura Mu 引导；但
     nura Mu = boot_b 换回 nura 固件——与 iOS 链互斥，每轮换刷）；
     或第一信号改 **USB serial gadget 只回显**（init 最早 say 后
     立即起 gadget，不等 dd——定位死点前后）。
   c. **换载体形态（兜底）**：reader 不走 UniXNU 菜单 LoadImage，改
     nura 原生形态——recovery_a 只刷一个**取证专用 ESP**（nura
     BOOTAA64.EFI 同件 + 我们的 initramfs 挂 ESP 上，由 nura Mu 引导
     的原样路径）——但 recovery_a 被占用，需与 payload 互斥（每轮
     读完刷回 payload，多一次 96MiB 刷写 ≈ 秒级）。
2. **iOS 判读（被 reader 调试顺延，不丢）**：正常引导 iOS（菜单
   default=xnu 3 秒超时）→ 等 10-20 分钟断电 → 现在读窗通道 = reader
   （若通）或临时回 nura-min（recovery_a 刷 nura ESP，boot_b 不动）。
   判读 `parsed=15 applied=7` + BLSHBacklightHost 换墙 + W1 swap。
3. **ABL 重试计数自动重置（r56 主题，用户需求 2026-10-27）**：
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
   && fastboot reboot`（用户验证过）。与 sheng-autoreset 并列 r56。
5. **独立主题（不阻塞）**：chronod sig5；XS/fileset-KC 重绑定器；
   arm64e 专项；GPU/触摸主题化；AdvancedLogDxe；records 页。

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
| r53 | `04f4d803` | **fixup 11 生效**（applied 5→6，BannerKit ret）；SB 推进到 BLSHBacklightHost brk 新墙（sig5）；W1 swap 异常→撕裂 logo 新观察 | history §0.36 |
| r55 | `fb2c5dd1` | r54（fixup 12，BLSHBacklightHost 入口返 nil，裸件 1ef5eb63）+ **forensic 内置取证 reader**（type=linux entry，FLASHING §7）；读窗循环脱 nura-min；待刷 | history §0.37 |
