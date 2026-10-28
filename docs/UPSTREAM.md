# 上游与参照仓（2026-10-23 起跟随上游）

## 1. 上游是谁

| 角色 | 仓库 | 本地 | 当前 pin |
|---|---|---|---|
| **基座上游** | https://github.com/UniXNU/UniXNU | 本仓 `m2381` 分支直接挂在其上；remote `upstream`（fetch-only） | `f85b5a6` (2026-10-27) |
| **本设备设备仓（2026-10-29 拆出）** | https://github.com/RiverKawa271828/UniXNU-m2381 | `../UniXNU-m2381`（工作区根 `/home/ri/work/unixnu-meizu`） | subtree split `2f06128c`（93 提交全史）+ 拆分适配 |
| **本设备 UEFI（2026-10-29 新建）** | https://github.com/RiverKawa271828/uefi-m2381（待建仓首推） | `../uefi-m2381`（分支 main） | 基座 = 上游 uefi-sheng `sheng@0eb4514` + overlay `5297334`；实证来源 = nura 链 |
| **设备模板（UEFI）** | https://github.com/hanzi123-bilp/uefi-fuxi | `reference/uefi-fuxi` | `7b7ec15` |
| **设备模板（UniXNU 设备仓）** | https://github.com/hanzi123-bilp/unixnu-fuxi | `reference/unixnu-fuxi` | `59a91fb` |
| DarwinBoot（boot manager driver） | https://github.com/UniXNU/DarwinBoot | `reference/darwinboot` | `3325e352` |
| **SM8550 SoC 层（v2）** | https://github.com/UniXNU/UniXNU-sm8550-common | 未克隆（feed pin `23b2049f`，sheng 消费中；m2381 迁 v2 时接） | `23b2049f` |
| sheng 参照（平板，**schema v2 参考设备**） | https://github.com/UniXNU/UniXNU-sheng | `reference/unixnu-sheng` | `d76556b` |
| sheng 固件 | https://github.com/UniXNU/uefi-sheng | `reference/uefi-sheng` | `0eb4514` |
| nx729j 参照（静态绑定先例） | https://github.com/2713385115/UniXNU-nx729j | `reference/UniXNU-nx729j` | 保留 |
| 本设备 Mu fork（m2381Pkg 落点） | https://github.com/RiverKawa271828/Mu-Silicium | `reference/uefi-m2381-nura` | 保留 |

- **分支模型（2026-10-23 起）**：`m2381` = `upstream/main` + 工作区提交。
  由旧的无祖先孤儿分支 `git rebase --onto upstream/main --root` 重建（备份：
  本地分支 `m2381-pre-sync-backup`）。工作区文件（`docs/`、`devices/`、
  `tools/`、`artifacts/`、`AGENTS.md`、`README.md`）叠在上游树
  （`drivers/`、`kits/`、`package/`、`patches/`、`scripts/`、`unixnu`…）之上。
- `reference/` 其余克隆仍是**只读参照**；日常同步直接在本仓 rebase。
  2026-10-26 清理：删除死 era 克隆 `UniXNU-guacamole`/`UniXNU-qemu`/
  `UniXNU-negroni`/`UniXNU-vermeer`/`UniXNU-nuwa`/`UniXNU-pong`/`uefi-pong`/
  `UniXNU-fuxi`（他人移植快照与旧先例，内容已被上游文档/机制吸收）与
  `reference/unixnu`（旧上游对照树，本仓即上游基座后冗余）；死 donor 暂存
  `out/ipsw/20H392`、空目录 `out/ipsw/21H16` 一并删除（stage-local-candidates
  的 sorted glob 只认 `kernelcache.release.*.decompressed`，现役 21H461 不受
  影响）。保留：uefi-fuxi/unixnu-fuxi（模板）、uefi-sheng（符号链接目标，
  探针 Font.h + P2 基座）、unixnu-sheng（v2 参考设备）、UniXNU-nx729j（静态
  绑定先例）、darwinboot、uefi-m2381-nura。
- 不要向 `upstream` push（push URL = `no_push`）。上游 = 机制/驱动/补丁的
  来源；设备模板 = 板级值的来源。设备工作落在独立设备仓
  `../UniXNU-m2381`（2026-10-29 拆出），本仓不直接改上游文件
  （`README.md`/`.gitignore` 除外，见 §2 冲突策略）。

### 1.1 同步记录

- **2026-10-27：`upstream/main` `16e592a` → `f85b5a6`（+22 提交，标准
  rebase 重放 147 个工作区提交零冲突；check-m2381 4 ok、XnuPrelink kit
  测试绿）。主题多为文档/设备记录（catalog 自动化 + 大批社区记录）；
  功能面三件对我们直接有效：**① `688fa88` 修我们消费的 21H461 profile
  本身**——ADT 补 J120 product-id（0x80ac7ae0）+ root compatible 置
  J120AP + /product/audio 节点（修 "Unknown product ID (0)" 音频路由类
  问题，产品类型可查还可能解开其他按产品类型分叉的 daemon），下轮
  payload ADT 会变（与 fixup 表无关的正文变更）；**② `71ccc68` ADT 生成
  器支持 DTS CPU 节点 `unixnu,cluster-type` E/P**（SM8550 有 E/P 集群，
  可后续给 dts/m2381.dts 补标注）；**③ `bc08de6` GPU/display/SMMU 组件
  bump**（ADRENO=n 不消费）。研究结论（工作单第 2 项）：sheng/nx729j/
  fuxi 三板 text_fixups 都只有同一套 4 条基础集（AWAttentionAwareness +
  2×PosterBoard + BTAudioHALPlugin），我们 13 条是全网最大表、无可抄；
  上游无 displayless 机制解——所有推进到 SpringBoard 的板（sheng/fuxi/
  cas/umi/oneplus13t-Pagani）都走 `CONFIG_XNU_NATIVE_LEGACY_DISPLAY=y`
  legacy CPU framebuffer 车道，displayless 板（ishtar/kebab）尚在显示
  之前阶段；=y 车道件全部就位（组件源码已在上游主树 drivers/GopFramebuffer，
  sheng 侧 build-native-legacy-display.py + LegacyDisplayProbe.cpp 可
  fork，注册机制 = 晚期归档 libNativeLegacyDisplay.late.a）。
- **2026-10-26 续：设备仓 schema v2 迁移落地（消费 16e592a 机制面）**：
  board 双板迁 v2（soc_source=sm8550 pin `23b2049f`、os_profile、components
  include、config_fragments/patch_series、device_tree=dts/m2381.dts）；ADT 走
  v2 公共生成器（make-m2381-adt.py 转参考件）；P3 双遗债清偿 =
  audit-storage-interface.py→nx729j 静态绑定（build-storage-bindings.py +
  EXTRA_LD force_load libStorageBindings.a）、NativePrelink/NativeUfs 改名
  XnuPrelink/ShengUfs（设备 fork 保留并胜出：Libm literal pools + fileset
  rebinder；SMMU takeover + 单窗口发布 + 16KiB 校验块）；删设备仓 stale
  build-arm-driver.py（v1 版会盖上游 v2 版）；m2381_defconfig 对齐 sheng 绿灯
  形态（virtio-mmio-block-verified-bundle 路径 + NATIVE_LEGACY_DISPLAY=n）；
  ci-owner-policy.sh 加 XnuPrelink fork 换装、make-device-branch.sh 加
  feeds/dts/patches、check-m2381.py 合并 profile 检查（4 ok）。本机验证 =
  双板 defconfig/tree、14 组件解析、58 补丁全应用、ADT 生成 13820B、kit 测试
  绿。详见 `docs/archive/STATE-history.md` §0.14。CI 重发待用户拍板。
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

- **UEFI**：`reference/uefi-fuxi` 的 `fuxiPkg` 是 `m2381Pkg` 的模板；
  m2381Pkg 现居 UEFI 仓 `../uefi-m2381`（基座 = 上游 uefi-sheng
  `sheng@0eb4514` + overlay 提交，2026-10-29 收编，devices/m2381/uefi 与
  sync.sh 退役）；固件构建 = `./unixnu build firmware --uefi ../uefi-m2381`。
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
origin    https://github.com/RiverKawa271828/ios-m2381-unixnu.git（GitHub fork of UniXNU/UniXNU，默认分支 = m2381）
          m2381 = 本工作区 = upstream/main f85b5a6 + 工作区提交
          device-m2381 = 设备仓 orphan 镜像（已冻结回退；2026-10-29 设备仓
                         拆出独立仓 UniXNU-m2381 = ../UniXNU-m2381，新
                         dispatch 直指新仓 main，orphan 首绿后可删）
          （origin 上没有 main——2026-10-26 删除，见下）
```

- 本工作区在 `origin/m2381`。behind = 还没跟上的上游提交数，ahead = 工作区
  提交数；2026-10-27 对齐后为 ahead 147 / behind 0（2026-10-26 曾为 107）。
- `origin/main` 已删除（2026-10-26）。历史：旧 b7b6a6a 镜像在上游改写 main
  历史后与上游分叉，GitHub 的 Sync fork / PR 页报 "conflicts"；先强推对齐为
  纯镜像，随后整个删除（fork 默认分支 = m2381，删 main 零影响；docs.yml 的
  push main 触发本来就没用过，porting-record.yml 的 ref: main 指上游仓）。
  fork 页面对比 = m2381 vs 上游 main，ahead 107 / behind 0，永远干净。
  若要恢复：`git push origin <上游tip>:main`（纯镜像，勿带工作区提交）。
- 推送：rebase 后历史改写，`git push --force-with-lease origin m2381`；
  不要 `git push upstream`。推送窗口 = 用户拍板（红线 8）。

## 5. 和旧 sweet 上游的关系

旧 ios-sweet / topaz 上游已退役，只作 `docs/archive/` 的史料；不再 fetch、
不再 cherry-pick。16K 内核与旧 root 随 B2 一并废弃（2026-10-23），路线 A
的内核/OS 件全部来自上游 unixnu 构建流程。
