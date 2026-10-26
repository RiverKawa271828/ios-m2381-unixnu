# PLAN — 显示车道两段式（=y 先立，换代跟跑）

> 2026-10-27 定稿（用户约束：上游不久将更新 arm64e 与 iOS 27 donor）。
> 结论 = 照常按 §0.30 配方施工 =y 车道，**但把"donor 换代成本"显式管理
> 起来**：车道工作量约八成与 donor 代际无关（机制/构建/接线），两成
> donor 绑定件（ABI 头 + bindings VA 表）全部有脚本化再生成路径，换代
> = 重跑生成器，不是重新逆向。

## 1. 为什么不"等 iOS 27 再动"

1. 墙在现在：displayConfiguration 断言族是 displayless 的必然结果，等
   新 donor 解决不了（新 donor 照样没有显示栈）。
2. =y 基础设施是跨 donor 资产：构建管线 fork、defconfig、args、探针
   注册、presenter 接线——换任何 donor 都原样复用。
3. 没有显示输出的每一轮诊断都是半盲：BootProgress 页让位给真显示后，
   SpringBoard 推进肉眼可见，读窗依赖下降。
4. 上游现在没有任何 iOS 27 / arm64e 施工分支（2026-10-27 扫过 20 个
   远端分支：audio/buttons/oslog/shaders 等）；换代时点不可控，等它 =
   无限期半盲。

## 2. donor 绑定件清单（换代时的全部成本）

| 件 | 现状 | 换代动作 | 成本 |
|---|---|---|---|
| `package/OriginalLegacyFramebufferABI.h` | 21H461 生成，358 slots | audit+生成器重跑（sheng `build-gop-legacy-display.py` 流程） | 脚本化 |
| bindings VA 表（surface getters/vsync/slot-301，sheng 脚本硬编码） | 21H461 VA，同 donor 直接有效 | 新 donor 重 derive；脚本自带字节验证 = 免费保险 | 脚本化+定点 RE |
| text_fixups 表 | 13 条（4 基础 + 9 显示族自研） | **显示族 9 条直接退役**（有真显示栈后 = 睡守，无存在意义）；4 条基础集由上游 profile 维护 | 负成本 |
| kernel/OS profile 本体 | 21H461 classic arm64 KC | 跟上游新 profile；**若新代 donor 是 fileset KC，须先有 fileset rebinder**（见 §4 风险） | 视上游 |
| ADT/板级/trust/种子/UEFI/分区 | 与 donor 无关 | 不动 | 零 |

## 3. 两段执行

**段一（现在，r51–r53 量级）：21H461 上 =y 车道破墙**
1. fork sheng `scripts/build-native-legacy-display.py` → 设备仓
   （裁剪 probe 车队，保审计/驱动/bindings/晚期归档链）。
2. fork sheng `package/LegacyDisplayProbe.cpp` → 设备仓 package。
3. defconfig（base+owner）翻 `CONFIG_XNU_NATIVE_LEGACY_DISPLAY=y`，
   **手动重跑 defconfig 命令**（铁律）。
4. board.toml archives 插 `build-native-legacy-display.py`（顺序无关，
   链接前）；args 不动（offline-legacy-\* 全在）。
5. Linux 本机出 =y 内核 → payload → 刷写卡（三件全刷规矩不变）。
6. 仍 ADRENO=n：GPU 是 P4 主题，=y 的 GOPPresenter 就是它将来的底座。

**段二（上游落地新 donor / arm64e 时）：换代跟跑**
1. fetch+rebase 吃新 OS profile / 机制（upstream-first 规矩不变）。
2. 重跑 ABI 头生成 + bindings 重 derive（字节验证门）。
3. text_fixups 收缩到上游基础集。
4. 若新 donor 是 fileset KC：fileset rebinder 转正为前置（我们 fork 的
   XnuPrelink 已保留 fileset rebinder 本体，32 测试绿——这是别的板没有
   的先手）。
5. arm64e：上游背内核/用户态机制面；本侧透传验证已过（门 C），专项
   会话单列。

## 4. 风险登记

- 换代时点/形态不可控（可能 classic KC 也可能 fileset KC）——两段式把
  换代成本压到脚本化重生成，fileset 情形有 rebinder 先手。
- sheng 脚本的 xcrun/brew 依赖 → 换 clang 装配树模板法（BootLog.o 先例）。
- =y 上电后新 crasher 正常（BootProgress 页让位、显示栈初始化路径首次
  实跑）；仍按 REASON 行 + bt 验活方法读窗。
- 类杀 NSAssertionHandler 保持兜底不启用。
