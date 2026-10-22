# 云端构建（GitHub Actions）

新仓保留旧 sweet 路线的 macOS 内核构建，并把它放到**归档路径**下；B2 正常
不需要它（B2 用现成的 `artifacts/kernel/kernel.development.m2381`）。

## `xnu-16k`（已移植，手动 + 路径触发）

- 文件：`.github/workflows/xnu-16k.yml`
- runner：`macos-14`（Xcode 15；iig 与 xnu-8796 的 IORPC ABI 匹配）
- 输入：`macos_version`（默认 13.5）、`kernel_config`（DEVELOPMENT）、
  `machine_config`（M2381 / SWEET）、`keep_going`
- 做法：拉 `blacktop/darwin-xnu-build`（pin `6639065`），把
  `legacy/sweet/xnu/patches/*.patch|*.sh` 拷进 `dxb/patches/`，运行
  `dxb/build.sh`，收集 `fakeroot/System/Library/Kernels/kernel.*`。
- 产物：`xnu-kernel-16k`（`out/`，含 `SHA256SUMS` 与 `buildtree.tgz`）。
- 触发：手动 `workflow_dispatch`，或 push 到 `main`/`m2381` 且改动
  `legacy/sweet/xnu/patches/**` / 本 workflow。
- 用途：只在需要重建 16K 内核时用；重建后把 `kernel.development.m2381`
  放回 `artifacts/kernel/`，并把新 sha256 记到 `docs/STATE.md`。
- 旧 workflow 原件也归档在 `legacy/sweet/.github/workflows/xnu.yml`（GitHub
  只运行根 `.github/workflows`，归档件仅作参照）。

## 本地预检

```bash
legacy/sweet/tools/xnu-gate-walk.sh   # 96 个 patch + 3 个 .sh 严格 walk（会重置 xnu-8796）
python3 legacy/sweet/tools/xnu-cc.py --help
```

- 2026-10-22 本地实测：`=== gate: 96 patches, 0 failures`（base `258d99a`）。

## UEFI 构建（B2 主线，待加 CI）

B2 的 UEFI 来自 `reference/uefi-fuxi` 派生的 `devices/m2381/m2381Pkg`。
EDK2/CLANGPDB 在 Linux 可构建，等 `m2381Pkg` 落地后再加
`.github/workflows/uefi.yml`；现在先在本机跑 `uefi-fuxi/Resources/Scripts/
build_uefi.py`（设备名 `m2381`）。CI 只构建 UEFI FV，不跑 `fastboot`。

## 首次云端运行

- 2026-10-22 推 `m2381` 后，`xnu-16k` 已自动触发（run #1，macOS-14）。
- run 页面：`https://github.com/RiverKawa271828/ios-m2381-unixnu/actions`

## 推送

- `origin` = `RiverKawa271828/ios-m2381-unixnu`；工作区分支是 `m2381`，
  `main` 保留为上游镜像。push `m2381` 才会触发云构建。
- `upstream` 是 UniXNU/UniXNU，**fetch-only**（push URL 写成 `no_push`）。
