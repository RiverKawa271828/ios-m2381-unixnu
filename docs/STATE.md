# STATE — 现势状态（m2381，2026-10-27 重组）

> **历史全档**：2026-10-24 → 2026-10-27 全部轮次（§0.0–0.34）与当时静态区
> 在 `docs/archive/STATE-history.md`（原样快照）；每轮定谳同时完整活在
> git commit message 里。本文档 = 精简现势版：现状 / 资产 / 方法 / 开放
> 问题 / 下一步 / 轮次索引。

## 1. 一句话现状（r50 后，2026-10-27）

iOS 链已全链上机并稳定复现（UEFI → XNU → launchd → SpringBoard 起跑、
watchdogd 循环带日志窗），卡在 **SpringBoard displayConfiguration 断言墙**：
displayless 用户态连环触发 `NSParameterAssert(displayConfiguration)`，
r43→r50 八轮逐点排除（五个断言站点全部被 fixup-application oracle 排除），
抛者更深。**2026-10-27 上游同步 + 调研已完成**（§7.1/7.2 结论）：上游无
displayless 机制解，全网 fixups 无可抄，**建议拍板 = =y 车道**（§7.3）。

## 2. 设备现状（2026-10-27，iOS 链现役）

| 分区 | 件 | sha 前 8 | 状态 |
|---|---|---|---|
| boot_b | 本仓 UEFI（m2381Pkg FV boot.img） | `a1d51f04` | 现役，多轮复刷不变 |
| recovery_a | ESP payload（当轮 fixup 表载体） | `4e69f450` | r50 件在机 |
| userdata | CI 重建 iOS root | `fd43ee70` | 现役，逐字节稳定 |

- 刷写/读回/回滚纪律：`docs/FLASHING.md`。回滚件**只**在 nura 仓
  `/home/ri/work/nura-meizu-m2381/artifacts/`（红线 6；本仓 artifacts/images
  已于 2026-10-26 删除）。
- **发卡规矩（r49 事故定谳）**：①发卡前先确认设备现态——走过 nura-min
  读回流程后 iOS 链已被覆盖，**必须三件全刷**；②payload 只认平铺路径
  `out/m2381/payload-rNN-<sha8>.img`（--out 双层路径歧义已废弃）；
  ③卡上必带刷前 `sha256sum` 门。

## 3. 现役件与资产

- **payload 四元（多轮不变）**：kernel `430344b3` / 探针 `bf32d8fa` /
  trust `7416ca4f` / 种子 `d04a0153`；ADT 随 fixup 表变（现 13 条，
  < CS_TEXT_FIXUP_MAX=32）。fixup 表源 = 设备仓 OS profile `text_fixups`。
- **applied 常驻集**：{BTAudio, 5, 6, 7, 8a}（每轮日志窗 parsed/applied
  即执行 oracle）。
- **device repo**：`devices/m2381/unixnu-m2381`（schema v2；os_profile =
  ipados-17.7.11-21H461；orphan 分支 `device-m2381` 供 CI）；**UEFI**：
  `devices/m2381/uefi/m2381Pkg`（shengPkg 骨架 + mars blob @036ba9f7）。
- **donor**：iPad7,1 / iPadOS 17.7.11 `21H461`（IPSW 本地已核验；20H392
  资产保留回溯）。unique-chip-id `3645ad55d47e77c1`。
- **=y 车道备件已齐（2026-10-27 复核）**：ABI 头
  `devices/…/package/OriginalLegacyFramebufferABI.h`（r47 入仓）+
  sheng 287 行流水线施工配方（STATE-history §0.30，含裁剪清单与风险）+
  组件源码已在上游主树 `drivers/GopFramebuffer/`（NativeLegacyDisplay/
  NativeGOPPresenter）+ sheng 侧 `build-native-legacy-display.py` 与
  `package/LegacyDisplayProbe.cpp` 可 fork（reference/unixnu-sheng）。
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

## 5. 开放问题

- **主墙**：displayConfiguration 抛者仍在更深处（五站点全排除后收敛）。
  三条备选路径与判据见 §7。
- **开放缝（定位收口遗留）**：①auth-rebase 编码 CFString 指针重扫（修正
  掩码 `ptr & 0x3FFFFFFFFF`，重扫被用户叫停中）；②.23 区两裸串引用点
  （0x1c943f1ec / 0x1c94419e8，垃圾符号）未反汇编定性。
- **chronod sig5 崩溃循环**（Swift 陷阱形态，r50 ×178 升温）——次优先，
  主链推进后按 sheng disable-list 处理。
- **独立主题（不阻塞）**：XS/fileset-KC 重绑定器（fileset KC 无 KASLR 表，
  prelink kit 需重绑定器）；arm64e 专项（透传已验证，FPAC/PACM/自举面/
  再签待专项）；GPU（ADRENO=n，P4 显示门过后主题化）。

## 6. 不能声称的

- 显示墙未破：SpringBoard 未到 home screen；触摸/按键/GPU 均未上主题。
- =y 车道未施工（配方与备件齐，NativeLegacyDisplay 未编未上机）。
- 类杀 NSAssertionHandler 仅评估未出件（0x1874494d4 定谳后才可用）。

## 7. 下一步（r50 后工作单；§7.1/7.2 已完成 2026-10-27）

1. **上游同步 ✅（2026-10-27）**：pin `16e592a` → `f85b5a6`（+22 提交），
   rebase 147 工作区提交零冲突，check-m2381 4 ok、kit 测试绿。要害 =
   **上游修了我们消费的 21H461 profile**（`688fa88`：ADT 补 J120
   product-id + J120AP compatible + /product/audio，下轮 payload ADT
   必变）+ ADT 生成器支持 CPU cluster-type（`71ccc68`）。详见
   `docs/UPSTREAM.md` §1.1。
2. **上游/fork 解法调研 ✅（2026-10-27）**，三问三答：
   - **fixups 有无可抄**：无。sheng/nx729j/fuxi 三板全是同一套 4 条基础集
     （AWAttentionAwareness + 2×PosterBoard + BTAudioHALPlugin，即我们的
     前 4 条）；我们 13 条全网最大，9 条全是自研。
   - **displayless 机制解**：不存在。上游全家族凡推进到 SpringBoard 的板
     （sheng/fuxi/cas/umi/oneplus13t-Pagani）defconfig 全部
     `CONFIG_XNU_NATIVE_LEGACY_DISPLAY=y` 走 legacy CPU framebuffer 车道；
     displayless 板（ishtar/kebab）还停在显示之前的阶段，无人让
     SpringBoard 无显示存活。我们的 fixups 断言歼击是在没有机制解时自创
     的迂回，r50 已证是打地鼠（抛者逐轮更深层）。
   - **其他 fork**：HackiOS 维持 10-24 不转向评估（16.5 用户态 + A11
     donor + 内存驻留模型，不同架构，无可取机制）；ios-topaz（sweet 线）
     已退役仅作硬件参照；社区 SM8550 = fuxi（kernel 阶段）/sheng（
     SpringBoard+触摸 = 同 SoC 最先证 =y 车道全通）。
3. **拍板项（待用户）——建议 b**：
   - **b. =y 车道（推荐）**：机制根因解，给用户态真 displayConfiguration，
     断言类整体停触发而非逐个灭口；与上游每块先进板同构；顺带换来真显示
     输出（BootProgress 之后能眼见 SpringBoard，诊断质变）。备件全齐：
     组件源码已在上游主树 `drivers/GopFramebuffer`，ABI 头在设备仓
     package/，sheng 侧 `build-native-legacy-display.py` +
     `LegacyDisplayProbe.cpp`（注册机制 = 晚期归档，r47 疑虑已解）可
     fork，offline-legacy-* args 全在 board.toml；施工 = §0.30 配方
     （defconfig 翻 y + 手动重跑 + 归档脚本 fork 裁剪）。
   - a. 类杀 NSAssertionHandler（0x1874494d4 + 变体入口 ret）：一次灭全
     断言传递类，但 = 全系统断言静音——后续任何真致命断言都变成带脏状态
     的未知行为且零日志，诊断变盲；且不解决无显示的下游墙（窗口/场景层
     还会以非断言形式再塌）。仅作 =y 车道被硬件事实挡住时的兜底。
   - c. 上游现成机制解：已证不存在（§7.2）。

## 8. 轮次索引（r43 → r50；详情 = commit message + STATE-history 对应节）

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
