# 上游与参照仓（2026-10-23 起跟随上游）

## 1. 上游是谁

| 角色 | 仓库 | 本地 | 当前 pin |
|---|---|---|---|
| **基座上游** | https://github.com/UniXNU/UniXNU | 本仓 `m2381` 分支直接挂在其上；remote `upstream`（fetch-only） | `16e592a` (2026-10-26) |
| **设备模板（UEFI）** | https://github.com/hanzi123-bilp/uefi-fuxi | `reference/uefi-fuxi` | `7b7ec15` |
| **设备模板（UniXNU 设备仓）** | https://github.com/hanzi123-bilp/unixnu-fuxi | `reference/unixnu-fuxi` | `59a91fb` |
| DarwinBoot（boot manager driver） | https://github.com/UniXNU/DarwinBoot | `reference/darwinboot` | `3325e352` |
| guacamole 先例 | https://github.com/wochgn/UniXNU-guacamole | `reference/UniXNU-guacamole` | `6269d31` |
| qemu 先例（Data seed 脚本） | https://github.com/xVanTuring/UniXNU-qemu | `reference/UniXNU-qemu` | `97b75b1` |
| sheng 参照（平板） | https://github.com/UniXNU/UniXNU-sheng | `reference/unixnu-sheng` | `d76556b` |
| sheng 固件 | https://github.com/UniXNU/uefi-sheng | `reference/uefi-sheng` | `0eb4514` |
| fuxi 另一 fork | https://github.com/matejkoo217/UniXNU-fuxi | `reference/UniXNU-fuxi` | `41c2aac` |
| vermeer 参照 | https://github.com/yyx2025/UniXNU-vermeer | `reference/UniXNU-vermeer` | `9137aba` |

- **分支模型（2026-10-23 起）**：`m2381` = `upstream/main` + 工作区提交。
  由旧的无祖先孤儿分支 `git rebase --onto upstream/main --root` 重建（备份：
  本地分支 `m2381-pre-sync-backup`）。工作区文件（`docs/`、`devices/`、
  `tools/`、`artifacts/`、`legacy/`、`AGENTS.md`、`README.md`）叠在上游树
  （`drivers/`、`kits/`、`package/`、`patches/`、`scripts/`、`unixnu`…）之上。
- `reference/` 其余克隆仍是**只读参照**；`reference/unixnu` 降级为干净构建
  树/对照，日常同步直接在本仓 rebase，不再经 reference 搬运。
- 不要向 `upstream` push（push URL = `no_push`）。上游 = 机制/驱动/补丁的
  来源；设备模板 = 板级值的来源。设备工作落在 `devices/m2381/`，不直接改
  上游文件（`README.md`/`.gitignore` 除外，见 §2 冲突策略）。

### 1.1 同步记录

- **2026-10-26（规矩定死：一切以上游为准，用户拍板）**：`upstream/main` →
  `16e592a`（`c62bf12` +42 提交，历史未改写，标准 `git rebase upstream/main`
  重放 104 个工作区提交）。主题：**设备仓 schema v2**（`[board] soc_source`/
  `os_profile` 分层 profile：arch→SoC→os→device；`component:` archive 条目 +
  组件目录/依赖解析，v1 板仍兼容）；**UniXNU-sm8550-common SoC 仓**（sheng
  已迁 v2 消费它，m2381 同 SoC 直接受益）；**驱动出主树**（drivers/Native*
  从 UniXNU 主树移入设备仓/组件仓：Adreno740+AdrenoMetal = sheng GPU 加速、
  Ath12kWifi、NativeGeniSpi 等）；`kits/NativePrelink` → **`kits/XnuPrelink`**
  改名（build/native-prelink → build/xnu-prelink）；**userdata 可在 Linux
  构建**（0f85bc2，macos.yml steps 撤独立 userdata 项）；entry-probe 工具收
  编上游（c1f6966）；VerifiedRoot 子模块 bump（def0266）。**CI 定论**：上游
  pin `runs-on: macos-26` 的注释点名 macos-15 Xcode clang 会拒绝 -Werror 的
  警告选项——run 40220630199（Xcode 26.3/Apple clang 17 对
  `-Wno-unnecessary-virtual-specifier` 报 unknown warning option）正是此墙；
  m2381 的 runner_label/macos-15 车道已撤（e04c4dd）。audit-storage-interface
  的 gdb-stop.json 活机依赖上游原样保留 = 作者本机资格化工具，CI 板（上游
  sheng v1/v2）都不进 archive 清单——m2381 owner board.toml 的 fuxi v1 清单
  需迁移（未做）。冲突处置：.gitignore 并集；README.md/AGENTS.md 取工作区
  版；entry-probe 对取上游版（NOP-padding 容忍随之丢弃，若 payload 再需再
  derive）；DarwinBoot 先取上游 pin 又被重放旧提交盖回，终以 843d46a 定为
  `3325e352`（loader handoff 诊断）。工作树清掉 12 个上游已移走的 stale
  驱动/kit 目录（备份分支可取回）。check-m2381 4 ok / 0 fail。
  备份分支 `m2381-pre-sync-20261026`。
- 2026-10-25：`upstream/main` → `c62bf12`（相对旧 pin `b7b6a6a` +209 提交，
  **且上游改写过 main 历史**——`b7b6a6a`/`e95b37d` 均不再是现 main 的祖先，
  等价补丁在新链下有新 SHA）。主题：**PR#100 = kernel 步 Linux 可跑**
  （`scripts/linux/` 整套工具链：打 `__builtin_xnu_type_*` 补丁的 clang、
  cctools-port ld64、mig/iig/lzfse；`docs/building.md` Linux host 节；
  Linux/Mac 内核不逐字节相同但符号/对象/kalloc_type 签名/prelink 绑定等价）
  ；**macos.yml 重写**（只发脱敏文本日志；新增 `pre_build`/`trust_cache_url`/
  `data_seed_url`/`skip_setup`/`runner_label`——`macos-15` = Intel lane，
  解 arm64 容量拥堵）；payload（GPU addon 写入数据卷、signed-text fixups 进
  DT、确认安装项、重启/关机保留）；storage（实验性双系统分区、备份可选）；
  大批社区端口文档（polaris/malachite/pandora/pong/myron/umi/merlinx…）；
  `c62bf12` = Tools 菜单插件系统 + Advanced Logger 插件。
  **同步方法（上游改写历史后）**：普通 `git rebase upstream/main` 会把旧链
  上游提交当独立提交重放而全面冲突；正确做法 = 从工作区首提交的父提交
  （旧 pin）`git rebase --onto upstream/main <旧pin> m2381`，只重放工作区
  提交。本次 49 个工作区提交零冲突落位（备份分支
  `m2381-pre-rewritesync`）；rebase 后 check-m2381 4 ok / 0 fail、kit 32
  测试全绿、工作区文件零丢失（改动面全部来自上游新内容）。
- 2026-10-22 11:40：`git fetch` 后所有 pin 未变；`upstream/main` = `e95b37d`，
  `uefi-fuxi` = `7b7ec15`，`unixnu-fuxi` = `59a91fb`，`darwinboot` = `85332b3`，
  guacamole = `6269d31`，qemu = `97b75b1`，sheng = `77490e5`，uefi-sheng =
  `0eb4514`；没有新主题要移植。
- 2026-10-23：`upstream/main` `e95b37d` → `b7b6a6a`（+69 提交）。主题：
  **GPU**（NativeAdreno/NativeMetal/NativeHidInput pin 多次 bump；非一致渲染
  目标改系统内存渲染；免硬件 binning；Turnip 任意偏移 bind host-import 图像；
  two-plane 表面纹理；GPU addon user-client 白名单；shader prime list；自建
  `/private/var/ngpu`）；**xnu 补丁**（union lookup 只搜下层目录；sandbox
  grant 覆盖服务属性设置；bindfs force-root/getattr 恢复；DriverKit header
  列表认证）；**payload**（重启/关机项保留；确认安装项；板级参数传给
  skip-setup、`--no-auto-dim`；entry probe 定时复位）；**storage**（备份可选；
  实验性双系统分区）；**scripts**（shader cache 按翻译器构建 key；kernel 参数
  允许路径；bridge manifest 免构建写出）；**CI**（macOS workflow 重写、日志
  脱敏）；docs 新增大量社区端口（merlinx/umi/pandora/nuwa/nx729j/cheeseburger/
  zeta/raphael/fajita/tb128fu/pong 等）。同日把 `m2381` 重建为 `b7b6a6a` +
  11 个工作区提交（冲突仅 `README.md`/`.gitignore`：前者取工作区版，后者取
  并集）。对 B2 的直接增量有限（多数是 userland/GPU 侧），但 A 路线和以后
  上机会吃到，按用户要求保持跟随。

## 2. 更新流程

```bash
# 1) 拉上游、看新增（不自动合并）
git fetch upstream
git log --oneline <上一pin>..upstream/main

# 2) 同步：把工作区提交 rebase 到新 tip（线性历史，behind 归零）
git rebase upstream/main m2381
#    ⚠ 上游改写过历史时此路会重放旧链上游提交而全面冲突——改用：
#    git rebase --onto upstream/main <工作区首提交的父=旧pin> m2381
#    （只重放工作区提交；工作区首提交 = git log --reverse
#      --author='m2381 workspace' upstream/main..m2381 | head -1 的父）
#    冲突面应只有 README.md / .gitignore（工作区文件）：
#    README.md 取工作区版；.gitignore 保持"上游块 + 工作区块"并集；
#    上游文件一律不改。

# 3) rebase 改写历史，推送用（推送窗口 = 用户拍板，红线 8）：
git push --force-with-lease origin m2381

# 4) 更新本文件 pin 表 + §1.1 同步记录（一次上游主题一句话）。
```

- **UEFI**：`reference/uefi-fuxi` 的 `fuxiPkg` 是 `m2381Pkg` 的模板；Mu 树
  不进本仓，`devices/m2381/m2381Pkg` 由那棵树构建。
- **boot manager / 内核侧**：上游树就在本仓 `m2381` 里（`drivers/`、
  `patches/xnu`、`kits/`…），跟随上游 = rebase；要单拿某个主题也可以
  `git cherry-pick`。路线 = 原生移植：`unixnu kernel/userdata` 全流程都要跑
  （board.toml 驱动），不是只取 EFI 件。
- **设备值**：`reference/unixnu-fuxi` 是 fuxi（小米13，同 SM8550）已验证形态
  的模板，m2381 设备仓以它为母版；但 `board.toml` 的 userdata LBA、payload
  载体、板级差异必须按 m2381 实机改，不要盲抄。

## 3. 已知上游粗糙点（2026-10-22）

- ~~DarwinBoot gitlink 指向不存在的 commit~~ 已解决（2026-10-26）：上游
  pin 前移到 `3325e352` 且真实存在，本仓 gitlink 跟随（843d46a）；
  `git submodule update --init` 不再需要手工 cacheinfo。
- `unixnu-fuxi` 是从 sheng 设备仓改的：README/触摸支持表仍是 sheng 的
  （Novatek NT36532E），不能当 fuxi 已验证的证据；触摸/PMGR 仍要自己接。
- `unixnu-fuxi` 的 `[payload]` 192MiB、`[ipsw]` iPad7,1、`sheng-*` 内核参数
  是**全量 4K 路线**的模板，B2 不采用。
- fuxi 的 UEFI FV 没有 UFS DXE（只有 MassStorage app）；m2381 走 ESP 载荷
  同样必须在 FV 里加 OEM `UFSDxe` + `DiskIo`/`Partition`/`Fat`（B2 时期的
  论证见 `docs/archive/BOOT-CHAIN-b2.md` §7.1.1）。

## 4. 本工作区的远端

```text
upstream  https://github.com/UniXNU/UniXNU.git                   (fetch only; push = no_push)
origin    https://github.com/RiverKawa271828/ios-m2381-unixnu.git
          main  = UniXNU e95b37d 的旧镜像（保留不动；要追新可 fast-forward）
          m2381 = 本工作区 = upstream/main b7b6a6a + 工作区提交
```

- 本工作区在 `origin/m2381`。它曾是无共同祖先的孤儿分支，GitHub 因此把上游
  全部提交都算成"落后"（出现过 11 ahead / 181 behind 的大数字）；
  2026-10-23 重建后 ahead/behind 恢复真实含义：behind = 还没跟上的上游
  提交数，ahead = 工作区提交数。
- `origin/main` 仍是 e95b37d 旧镜像；不要把工作区提交推上 `main`。
- 推送：rebase 后历史改写，`git push --force-with-lease origin m2381`；
  不要 `git push upstream`。推送窗口 = 用户拍板（红线 8）。

## 5. 和旧 sweet 上游的关系

旧 ios-sweet / topaz 上游已退役，只作 `docs/archive/` 的史料；不再 fetch、
不再 cherry-pick。16K 内核与旧 root 随 B2 一并废弃（2026-10-23），路线 A
的内核/OS 件全部来自上游 unixnu 构建流程。
