# devices/m2381 — m2381 原生 UniXNU 移植施工面（骨架）

现方向 = **原生 UniXNU 移植**（见 `docs/PLAN-unixnu.md`）。计划落地结构：

```
devices/m2381/
  unixnu-m2381/          # 设备仓，从 reference/unixnu-fuxi 派生
    board.toml           # SoC / [ipsw] / [userdata] / [kernel] / 归档构建器
    configs/ patches/ scripts/ package/ assets/
  uefi/
    m2381Pkg/            # 从 reference/uefi-fuxi/Platforms/Xiaomi/fuxiPkg 派生
  out/                   # 构建产物（gitignore）
```

## 母版与指南

- 上游 `docs/porting/`：`index.md` / `bring-up.md` / `device-repository.md` /
  `device-tree.md` / `uefi-firmware.md`——**动工前先通读**。
- 设备仓母版 = `reference/unixnu-fuxi`；UEFI 母本 =
  `reference/uefi-fuxi` 的 `fuxiPkg`（同为 SM8550）。
- **不改 `reference/`**；复制到 `devices/m2381/` 再改。

## m2381 特有点（差异清单，详见 AGENTS.md §5 / docs/PLAN-unixnu.md §4）

1. ABL：v4、gunzip ≤ ~83.5MB、**忽略 boot ramdisk**、无 CmdBoot ⇒ payload
   载体大概率走 recovery_a ESP，需 FV 集成 OEM UFSDxe（`artifacts/vendor/`）。
2. FD `0xA7100000+0x300000`；FB `0xb8000000`；高段 DRAM `0x8c0000000` 起加
   进 GCD（fuxi 版只加 4GB 以下，必须改）。
3. `[userdata]` first/last_lba **必须从 m2381 实机 GPT 读**，不许抄 fuxi。
4. 按键 PMK8550（SID/键码待与 fuxi 对比）；显示 cmd-mode DSC（先用 ABL 帧
   缓冲 `0xb8000000`）。
