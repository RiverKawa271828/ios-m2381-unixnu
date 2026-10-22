# legacy/sweet — 已废弃的 ios-sweet 路线

> 2026-10-22 用户拍板放弃 ios-sweet 方案，改用 UniXNU 启动链（B2）。
> 这里的内容**不再作为施工面**，只保留两类价值：
> 1. **硬件史料**：m2381 的 GIC/FB/UFS/触摸/启动常量、r34 取证结论；
> 2. **B1 回退机制**：`shim/build.sh` 的“payload 嵌 kernel 尾部 + image_size”
>    做法，在 ESP 路线走不通时可能复用。

## 内容

| 路径 | 说明 |
|---|---|
| `shim/` | 旧 sweet shim 源（launcher/console/mmu/ufs/fdt） |
| `shim/build.sh` | 关键：M2381 的 GKI ABL 忽略 boot.img ramdisk，所以 payload 嵌 kernel 尾部；`image_size` 在偏移 0x10；v4 + 空 ramdisk |
| `xnu/patches/` | 84 个 ios-sweet 补丁（m2381 化），只作硬件/历史参考 |
| `tools/` | 旧构建/日志/取证工具全套 |
| `images/` | `shim-boot-m2381.img`（r34 boot_b）、`Image.gz`、`payload-r34.bin`（本地大件，不入 git） |

## 不要做

- 不要把这里的 84 补丁再当主线；新主线是 UniXNU 4K 或 16K hybrid。
- 不要直接 `bash shim/build.sh` 出新 boot_b 去刷；B2 不用它。
- 不要从这里复制 ADT 当 B2 的 ADT（旧的 13GiB，sha `c85b01f0…`，只作对照）。

## 旧工作区

完整旧仓仍在 `/home/ri/work/ios-meizu-m2381`（`DEPRECATED.md`），包含
`research/`（18GB XNU/IPSW）、`rings/`、`reference/ios-sweet` 等未搬走的内容。
