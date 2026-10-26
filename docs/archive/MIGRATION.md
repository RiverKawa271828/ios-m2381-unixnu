# 迁移记录（2026-10-22）

> **✅ 已完成归档（2026-10-27 移入 docs/archive/）**：B2→原生路线的工作区
> 迁移早已完结——本表的价值只剩"旧资产去了哪"的查证（§1/§2 映射表）。
> 注意其后又发生：`legacy/` 已从 git 移除（2026-10-23）、
> `artifacts/images/` 已删除（2026-10-26，回滚件只在 nura 仓）、
> 20H392/XS 资产已清理（2026-10-24 换配 21H461）。原样未改写。

> **2026-10-23 后记**：路线重置为原生 UniXNU 移植（B2 废弃）后，本文件所述
> "搬到 `legacy/sweet/`" 的内容已从 git 移除（只在 git 历史）；r34 回滚 shim
> 移到 `artifacts/images/shim-boot-m2381.img`。16K 内核/ADT/旧 IPSW 成为
> 死库存（见 `docs/STATE.md` §3）。

旧工作区 `/home/ri/work/ios-meizu-m2381`（ios-sweet 路线）已废弃。本仓
`/home/ri/work/ios-m2381-unixnu` 是新工作区。下面是搬了什么、为什么、没搬什么。

## 1. 搬过来的（move）

这些在新仓里是唯一副本；旧仓不再有：

| 资产 | 新位置 | 说明 |
|---|---|---|
| `reference/unixnu` | `reference/unixnu` | UniXNU 主仓；注意 DarwinBoot 子模块 pin `d099354…` 上游不存在 |
| `reference/unixnu-sheng` | `reference/unixnu-sheng` | sheng 设备仓（SM8550 平板） |
| `reference/unixnu-fuxi` | `reference/unixnu-fuxi` | Xiaomi 13 设备仓模板 |
| `reference/UniXNU-fuxi` | `reference/UniXNU-fuxi` | 另一份 fuxi 设备仓 |
| `reference/UniXNU-guacamole` | `reference/UniXNU-guacamole` | 最接近的 phone 实证（iOS16.7.16 + 16K kernel） |
| `reference/UniXNU-qemu` | `reference/UniXNU-qemu` | Data seed 生成器（未来 A 用） |
| `reference/UniXNU-vermeer` | `reference/UniXNU-vermeer` | Redmi K70 参照 |
| `reference/darwinboot` | `reference/darwinboot` | DarwinBoot 完整克隆（HEAD 85332b3） |
| `reference/uefi-fuxi` | `reference/uefi-fuxi` | Mu-Silicium + fuxiPkg（B2 UEFI 的母本，288MB） |
| `reference/uefi-sheng` | `reference/uefi-sheng` | Mu-Silicium + shengPkg（参照，287MB） |
| `xnu-kernel.zip` | `artifacts/kernel/xnu-kernel.zip` | r34 16K 内核产物（58MB） |
| `handoff/unixnu-upstream-evaluation.md` | `docs/PLAN-unixnu.md` | 评估 + B2/B1/A 方案 |

## 2. 复制过来的（copy；旧仓仍留）

| 资产 | 新位置 | 说明 |
|---|---|---|
| `NOTES-m2381.md` | `docs/archive/NOTES-m2381.md` | 旧 sweet 全程日志，硬件史料 |
| `NOTES.md` | `docs/archive/NOTES.md` | ios-sweet 上游日志 |
| `NOTES-topaz.md` | `docs/archive/NOTES-topaz.md` | topaz 参照 |
| `AGENTS.md` | `docs/archive/AGENTS-sweet.md` | 旧工作区约束（含完整硬件表/历史） |
| `handoff/*.md` | `docs/archive/` | r34 取证、r35 交接、GLM 报告等 |
| `fix-round/` | `docs/archive/fix-round/` | 小补丁记录 |
| `tools/mkadt.py`、`adt.py`、`remotezip.py` | `tools/` | ADT 生成 + IPSW 远程读取 |
| `research/ipsw-x/d22ap.adt` | `research/ipsw-x/d22ap.adt` | `mkadt.py` 的输入（保持相对路径） |
| `research/ipsw/` | `research/ipsw/` | iPhone X 16.7.16 IPSW + 5 个 DMG + BuildManifest（13GB，Apple 本地） |
| `research/xnu-8796`、`buildtree-m2381`、`patchgen-orig`、`apply-*.sh`、`walk/gate/regen` | `research/legacy-sweet/` | 16K 内核重建套件（gitignore） |
| `research/bin/ipsw` | `research/bin/ipsw` | ipsw CLI（104MB，gitignore） |
| `research/ipsw-x/Firmware/all_flash` | `research/ipsw-x/Firmware/all_flash` | d22ap DeviceTree im4p |
| `shim/out/m2381.adt` | `artifacts/adt/m2381-sweet-13g.adt` | 旧 13GiB ADT，只作对照 |
| `research/m2381.adt` | `artifacts/adt/m2381-old.adt` | 旧 ADT 样本 |
| `shim/*.c/*.h/*.S/*.ld/build.sh` | `legacy/sweet/shim/` | 旧 shim 源（B1 机制参考） |
| `xnu/patches/` | `legacy/sweet/xnu/patches/` | 84 个 ios-sweet 补丁，只作史料 |
| `tools/*.py/*.sh/*.S` | `legacy/sweet/tools/` | 旧工具全套，只作史料 |
| `shim/out/Image.gz` | `legacy/sweet/images/Image.gz` | B1 可复用 |
| `shim/out/payload-r34.bin` | `legacy/sweet/images/payload-r34.bin` | 旧 payload 样本 |
| `shim/out/shim-boot-m2381.img` | `legacy/sweet/images/shim-boot-m2381.img` | r34 boot_b 回滚件 |
| `meizu20-uefi-mods/`、`inner-fv-A.bin` | `artifacts/vendor/` | OEM UEFI DXE（UFSDxe/DiskIo/Partition/Fat），本地 blob，不入 git |
| `r23-rounds/UfsDeviceInitDxe-r23bg.c`、`ChainLoadDxe-r23ag.c` | `docs/archive/nura-uefi/` | nura UEFI RE 源（UFS fDeviceInit / chainload 参考） |

硬件资料（从外部复制）：

| 文件 | 来源 |
|---|---|
| `docs/hardware/stock-fdt.dts` | `/home/ri/work/meizu20/meizu20-m1/dts/sm8550-meizu20-stock.dts` |
| `docs/hardware/stock-live-fdt.dtb` | `.../artifacts-uefi/stock-day-20261001/stock/stock-live-fdt.dtb` |
| `docs/hardware/fdt-runtime-20261006.dtb` | `.../dts/factory-first/corpus/fdt-runtime-20261006.dtb` |
| `docs/hardware/sm8550-meizu-20.dts` | `/home/ri/work/linux-mobile-ports/arch/arm64/boot/dts/qcom/sm8550-meizu-20.dts` |
| `artifacts/images/mu-r69-97cdea20.img` | `/home/ri/work/nura-meizu-m2381/artifacts/` |
| `artifacts/images/esp-recovery-v71.img.gz` | 同上 |

## 3. 没搬（留在旧仓）

- `research/dsc` 3.9GB：dyld shared cache 抽取，符号化用，B2 不需要；留在旧仓。
- `research/ipsw-x/iOSRoot.img.HOLLOW-DO-NOT-USE` 16GB（sparse 占位，不是真镜像）：不搬。
- `rings/` 107MB：r34 窗口读环取证件，只跟旧案有关。
- `reference/ios-sweet`、`reference/ios-topaz`：旧路线参照，不再用。
- `shim/out/` 180MB：除上表三个镜像外不搬。
- `.re/` 3MB：旧 RE 工装；需要时回旧仓。
- `.github/`、`fix-round` 之外的旧 CI/脚本：不搬。
- `README.md`、`.gitignore`、`NOTES.md` 原件：旧仓保留；新仓重写。

旧仓仍可查；UniXNU 克隆、`xnu-kernel.zip`、IPSW/DMG、16K 重建套件已 move，
其余留在原处。旧仓根有 `DEPRECATED.md` 指向本仓。

## 4. 为什么这么切

- B2 不需要 `dsc`/`rings`/16GB hollow；但保留 **IPSW + DMG + 16K 重建套件**，
  以防后面要重建内核或根盘。真正需要的是 **UniXNU 启动链 + 现有 kernel/ADT +
  ESP + 回滚件**。
- `reference/` 是只读克隆，move 避免 600MB 重复；`uefi-fuxi` 288MB 是 B2 的母本。
- `legacy/sweet/` 只保留可复用的 B1 机制（shim 的 kernel 尾嵌 payload）和硬件史料，
  避免新仓看起来还像 sweet 施工面。
- 所有大件在 `.gitignore` 里，git 只跟文档/工具/小源码。
