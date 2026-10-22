# devices/m2381 — B2 施工面（骨架）

目前只有骨架，还没有复制/修改任何上游代码。下一步从这里派生：

```
devices/m2381/
  uefi/
    m2381Pkg/            # 从 reference/uefi-fuxi/Platforms/Xiaomi/fuxiPkg 复制
      m2381.dsc
      m2381.fdf
      m2381.dec
      DeviceBuild.py
      Drivers/{SimpleFb?,M2381BootMemoryDxe,M2381KeyInputDxe}
      Library/MemoryMapLib
  unixnu-m2381/          # 从 reference/unixnu-fuxi 复制（设备仓）
    board.toml
    scripts/
    package/
  esp/                   # ESP 内容 staging（UniXNU.efi、toml、kernel、ADT）
  out/                   # esp.img、boot.img、日志（gitignore）
```

## 派生原则

- **不要改 `reference/` 里的克隆**；复制到 `devices/m2381/` 再改。
- UEFI 以 uefi-fuxi 的 `fuxiPkg` 为母本：FD `0xA7100000+0x300000`、
  SimpleFbDxe、FuxiBootMemoryDxe、FuxiKeyInputDxe、UniXNUDxe、DarwinBoot。
- 设备仓以 unixnu-fuxi 为母本；但 B2 不用 `native-ufs-root` / `sheng-pmgr` /
  Data seed / trust cache，payload 改成 ESP 内容。
- ADT 用 `tools/mkadt.py --m2381` 改，不直接复用旧 13GiB ADT。

## 第一轮要改的点

1. `FuxiBootMemoryDxe`：加高段 DRAM（`0x8c0000000` 起 3–4GiB）到 GCD。
2. `MemoryMapLib`：FD/窗口/display 地址按 m2381 核对（多数与 fuxi 同址）。
3. `BootShim`：保存 x0 DTB；`FD_BASE=0xA7100000`、`FD_SIZE=0x300000`、
   `REQUIRES_KERNEL_HEADER=1`。
4. `DarwinBoot`：32MiB 同余 patch（guacamole）。
5. ADT：`darwinboot-memory-size`、`dram-base/size`、`owned_memory_size`。
6. `unixnu.toml`：Payloads 路径、超时、无 Data seed/trust cache。

## 不在这里做的事

- 不改 `reference/` 的 git 历史；不把 Apple IPSW/Apple 件提交进 git。
- 不在 `out/` 里放不可重建的唯一样本（回滚件在 `artifacts/`）。
