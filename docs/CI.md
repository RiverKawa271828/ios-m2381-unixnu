# 云端构建（GitHub Actions）

## 现役（2026-10-29 更新：设备仓拆出独立仓）

- **设备仓 = 独立仓 [UniXNU-m2381](https://github.com/RiverKawa271828/UniXNU-m2381)
  （2026-10-29 自本仓 subtree split 拆出，本地 `../UniXNU-m2381`）**：
  dispatch 改 `device_repository=RiverKawa271828/UniXNU-m2381` +
  `device_ref=main`（`pre_build` = ci-owner-policy 不变，脚本随设备仓走）。
  旧 orphan 分支 `device-m2381` 冻结在 origin 作回退，新仓 CI 首绿后可删；
  `make-device-branch.sh` 已退役（孤儿镜像机器随拆分无用）。
- **`macos.yml` = 内核/userdata/payload 正式车道**（上游带来，
  workflow_dispatch）：`steps="kernel userdata payload"`；全绿先例 run
  `40237299244`（2026-10-26，orphan 分支时代），取件流程与产物契约见
  `docs/archive/PLAN-P3-CI.md`。
- kernel 步只能 macOS（xcodebuild + iphoneos SDK）；Linux 侧全链路本机
  已跑通（上游 PR#100 工具链，见 docs/UPSTREAM.md §1.1 同步记录）。
- **`porting-record.yml`（移植记录检查）**：上游版只有 `workflow_call`；
  原经 `make-device-branch.sh` 装进 orphan 分支自检 `porting.toml`——
  该机器随拆分退役；独立仓本地自检照跑（对照
  [records 规范](https://unixnu.com/zh/porting/records)；本地预检 =
  `python3 scripts/porting-record.py check --device ../UniXNU-m2381`，
  2026-10-27 全绿 13 areas）。
- `docs.yml` / `terminal.yml` / `sync-devices.yml`：上游文档/工具
  workflow，未用；在 Actions 页**禁用**（只禁用不改文件，避免每次
  rebase 多一处冲突）。

## 本地构建优先

日常出件在本机 Linux：defconfig / fetch / tree / payload / bootimg 全链路
（`out/m2381/`，出件纪律见 docs/FLASHING.md §3）；CI 出正式件与回归。
