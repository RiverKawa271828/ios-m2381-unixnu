# STATE — 现势状态（m2381，2026-10-27 重组）

> **历史全档**：2026-10-24 → 2026-10-27 全部轮次（§0.0–0.34）与当时静态区
> 在 `docs/archive/STATE-history.md`（原样快照）；每轮定谳同时完整活在
> git commit message 里。本文档 = 精简现势版：现状 / 资产 / 方法 / 开放
> 问题 / 下一步 / 轮次索引。

## 1. 一句话现状（r51 后，2026-10-27）

iOS 链已全链上机并稳定复现（UEFI → XNU → launchd → SpringBoard 起跑、
watchdogd 循环带日志窗），displayless 断言墙的机制解已选定并施工完成：
**=y 显示车道（NativeLegacyDisplay + GOP presenter，CPU presentation）
本机出件**——内核 `3ab2ace4`、payload **`22f8f0bd`**（平铺
`out/m2381/payload-r51-22f8f0bd.img`，96MiB，ADT 346c6cde = 13 fixups +
上游 688fa88 的 J120AP/product-id）。上游同步 f85b5a6 + 调研三答
（fixups 无可抄/displayless 无机制解/其他 fork 无新解）已入档
（`05cdd33`）；换代跟跑方案 = `docs/PLAN-display-lane.md`。**下一步 =
发卡三件全刷 r52 读窗**（§7）。

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
- =y 车道已施工**出件**（内核 3ab2ace4/payload 22f8f0bd，全验证门过）
  但**未上机**——r52 刷写读窗前不声称显示栈实机生效。
- 类杀 NSAssertionHandler 未出件（兜底项，=y 上机受阻才启用）。

## 7. 下一步（r52 工作单：刷写读窗）

1. **发卡（三件全刷，发卡规矩）**：设备现态 = iOS 链在役（r50 读回后
   未经 nura 流程）→ 三件：boot_b `a1d51f04`（不变，可复刷）+
   recovery_a ← **`out/m2381/payload-r51-22f8f0bd.img`**（平铺路径，
   刷前 sha256 门 = `22f8f0bdab319e81…`）+ userdata `fd43ee70`（不变）。
2. **读窗判据**：①payload 四元核验（parsed=13 全在）；②**真显示输出**
   （BootProgress 页让位或显示栈 banner——=y 生效的直观判据）；
   ③`displayConfiguration` 断言族应消失（REASON 行换成下一墙）；
   ④fixup oracle：显示族 fixup 若开始 applied = 对应站点真在执行路径。
   新 REASON 按 REASON 行 + bt 验活（bl-target 判活器）符号化。
3. **预期新墙域**：窗口/场景创建层、render server 交互、或 IOMFB 用户
   客户端连接（驱动 newUserClient = 原始类 slot 141 绑定）。
4. **换代跟跑（上游 arm64e / iOS 27 donor 落地时）**：照
   `docs/PLAN-display-lane.md` §3 段二执行（audit/ABI 头/bindings 重
   derive 全脚本化；fileset-KC 情形启用 fork 内 fileset rebinder）。

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
| r51 | `22f8f0bd` | 上游同步 f85b5a6 + 调研收口（=y 是唯一机制解）+ **=y 车道施工出件**（内核 3ab2ace4，七轮修四墙）；待刷 | PLAN-display-lane.md |
