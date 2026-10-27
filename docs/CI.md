# 云端构建（GitHub Actions）

## 现役（2026-10-27 更新）

- **`macos.yml` = 内核/userdata/payload 正式车道**（上游带来，
  workflow_dispatch）：`steps="kernel userdata payload"`，`device_ref` =
  设备仓 orphan 分支 `device-m2381`（`pre_build` = ci-owner-policy，换装
  fork 驱动/kit、补 staging）；全绿先例 run `40237299244`（2026-10-26），
  取件流程与产物契约见 `docs/archive/PLAN-P3-CI.md`。
- kernel 步只能 macOS（xcodebuild + iphoneos SDK）；Linux 侧全链路本机
  已跑通（上游 PR#100 工具链，见 docs/UPSTREAM.md §1.1 同步记录）。
- **`porting-record.yml`（移植记录检查）**：设备分支自检车道。上游版只
  有 `workflow_call`；本仓在设备仓 `ci/porting-record.yml` 存了加
  `push(workflows: device-*)`/`workflow_dispatch` 触发的副本，
  `scripts/make-device-branch.sh` 把它装进 `device-m2381` 分支的
  `.github/workflows/`——镜像更新即自检 `porting.toml`（对照
  [records 规范](https://unixnu.com/zh/porting/records)；本地预检 =
  `python3 scripts/porting-record.py check --device
  devices/m2381/unixnu-m2381`，2026-10-27 全绿 13 areas）。
- `docs.yml` / `terminal.yml` / `sync-devices.yml`：上游文档/工具
  workflow，未用；在 Actions 页**禁用**（只禁用不改文件，避免每次
  rebase 多一处冲突）。

## 本地构建优先

日常出件在本机 Linux：defconfig / fetch / tree / payload / bootimg 全链路
（`out/m2381/`，出件纪律见 docs/FLASHING.md §3）；CI 出正式件与回归。
