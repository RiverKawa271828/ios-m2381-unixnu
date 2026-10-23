# 云端构建（GitHub Actions）

## 现状（2026-10-23）

- 本仓自己的 workflow：**无**。旧 16K 内核构建 `xnu-16k.yml` 随 B2 废弃删除
  （文件内容可在 git 历史查：删除前的 `.github/workflows/xnu-16k.yml`）。
- 上游的 `docs.yml` / `macos.yml` / `terminal.yml` 随 rebase 进入本仓树，push
  会触发它们——在 GitHub Actions 页面逐个 **Disable**（只禁用、不改文件，
  避免以后每次 rebase 都多一处冲突）。

## 未来 CI（待设备落地后加）

- `uefi.yml`：Linux 构建 m2381Pkg FV（EDK2 CLANGPDB），等
  `devices/m2381/uefi/m2381Pkg` 落地后加。
- 设备仓构建：`unixnu` 构建树吃 `unixnu-m2381` 的流水，先本机跑通再考虑。

## 本地构建优先

路线 A 的构建主体（unixnu 构建树 + Mu-Silicium）都在本机/参照树里跑；CI 只
做回归，不做首发。
