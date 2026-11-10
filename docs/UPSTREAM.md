# 上游与参照仓（2026-10-23 起跟随上游）

## 1. 上游是谁

| 角色 | 仓库 | 本地 | 当前 pin |
|---|---|---|---|
| **基座上游** | https://github.com/UniXNU/UniXNU | 本仓 `m2381-ios27` 分支挂在其 **`main`** 上（2026-11-08 谱系修正：feat tip `d2666583` 已被 main 吸收，"lane 跟 feat"规则过期）；remote `upstream`（fetch-only） | lane pin `3375ac9d`（2026-11-10，main） |
| **本设备设备仓（2026-10-29 拆出）** | https://github.com/RiverKawa271828/UniXNU-m2381 | `../UniXNU-m2381`（工作区根 `/home/ri/work/unixnu-meizu`） | subtree split `2f06128c`（93 提交全史）+ 拆分适配 |
| **本设备 UEFI（2026-10-29 新建）** | https://github.com/RiverKawa271828/uefi-m2381（待建仓首推） | `../uefi-m2381`（分支 main） | 基座 = 上游 uefi-sheng `sheng@0eb4514` + overlay `5297334`；实证来源 = nura 链 |
| **设备模板（UEFI）** | https://github.com/hanzi123-bilp/uefi-fuxi | `reference/uefi-fuxi` | `7b7ec15` |
| **设备模板（UniXNU 设备仓）** | https://github.com/hanzi123-bilp/unixnu-fuxi | `reference/unixnu-fuxi` | `59a91fb` |
| DarwinBoot（boot manager driver） | https://github.com/UniXNU/DarwinBoot | `reference/darwinboot` | `3325e352` |
| **SM8550 SoC 层（v2）** | https://github.com/UniXNU/UniXNU-sm8550-common | 未克隆（feed pin `23b2049f`，sheng 消费中；m2381 迁 v2 时接） | `23b2049f` |
| sheng 参照（平板，**schema v2 参考设备**） | https://github.com/UniXNU/UniXNU-sheng | `reference/unixnu-sheng` | `d76556b` |
| sheng 固件 | https://github.com/UniXNU/uefi-sheng | `reference/uefi-sheng` | `0eb4514` |
| nx729j 参照（静态绑定先例） | https://github.com/2713385115/UniXNU-nx729j | `reference/UniXNU-nx729j` | 保留 |
| 本设备 Mu fork（已封存） | https://github.com/RiverKawa271828/Mu-Silicium | 本地克隆已删（2026-10-29，继任 = `../uefi-m2381` 新仓） | GitHub 留 meizu20-mars-port 史料 |

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

- **2026-11-10：`upstream/main` `5dc00d20` → `3375ac9d`（gap 61 条，463 个
  工作区提交标准 rebase 重放→460 落位；3 条重放中变空自动掉）。
  **网络面（本轮最大的非 git 工程）**：FlClash 国际节点全死（直连/代理/
  socks5/六镜像/ssh-over-443 全灭，根因 = TUN auto-route 对一切境外流量
  fake-accept，`8.8.8.8:1` 0.1ms "连上" 即铁证）→ **旁路实例配方**：
  `/tmp/bypass-proxy/bypass-core`（用户 FlClash_pmos 树 vendored mihomo
  库自建 main，CGO_ENABLED=0 -tags with_gvisor，goproxy.cn 供模块）+
  config 取自现役 profile（43 节点）+ `interface-name: wlp4s0`
  （SO_BINDTODEVICE 逃 TUN 循环劫持 = 成败唯一开关）+ DoH DNS（:53 全被
  TUN dns-hijack 污染成 fake-ip，DoH 443 直连 223.5.5.5 免疫）+ 全套
  updateListeners 手动补（上游 fork 注释掉了 executor 里的
  updateListeners + lan-allowed-ips 缺省空 = 入站全拒的暗坑两连）。
  20 节点活（新加坡 118ms）→ fetch/submodule fetch/ls-remote 全程走
  socks5h://127.0.0.1:17890。**上游 61 条主题：① macOS 27.3 车道整建制
  上线**（macosx 内核目标/Data seed + firmlinks/Darwin26 MAC hooks
  end_rename_swap+service-port derive/47 位用户地址空间/8GiB arm64 共享
  区/vc-text 启动日志/exec-log/rosp+apfs_boot_util/KPI closure+AMFI
  early initializer/DarwinBoot 载 macOS KC/GPU plugin for macOS 27/
  AdrenoMetal base limits+half-float 采样）+ **② sheng- 前缀从 boot
  args 全量退役**（47ec6a52：sheng-touch=4→touch=4，scripts/patches/
  drivers 三层全改；影响判读 = 仅驱动侧诊断旗标惰性化，我们 probe
  命名空间自洽不消费，board.toml 唯一 sheng-log=1 由自家 patch 消费
  照旧）+ **③ arm64e 车道直接相关修复**（f65660d0 PAR_EL1
  mmu_kvtop_wpreflight 同步 + a11553a7 user PAC keys off + f105b318
  4K pager source page/union reads/modload order）+ AppSync installd
  内签 + 并发 UFS Data 直取（56afc53f）+ XnuPrelink 26C5035r AMFI
  stack shift 绑定 + RamBlock 单写位图复位 + 打包/文档多件。
  **冲突账**：AdrenoMetal gitlink ×1（取上游 `c68f9a1d`）+ README.md
  ×20（取工作区版，自动化环收）+ `profiles/arch/arm64e/patches/xnu/
  series` ×9（union 逐条并入：上游 native-modload-lock-order/
  native-union-readdir-eof/native-at-par-sync/native-pac-arm64-user-jop
  四新条 + 我们 sheng 族全保留）+ kits/XnuPrelink gitlink ×1（中途取
  上游、终局收口 vendored）。**老坑第五现**：rebase 完成后重放旧 bump
  把 8 条 drivers gitlink 拖回 stale → 对齐提交 `2a376abe`（18/20
  =上游 tip）；两条有意例外 = **drivers/AppleKeyStoreState 保
  `58e5461`**（上游钉 `6ff4f11d` 未发布到其子仓 = 幽灵钉，ls-remote
  实证仅 main=58e5461）+ **kits/XnuPrelink 保 vendored**。kit 并集：
  上游 kit main（macOS 构建/集合名 fileset_names.py/26C5035r AMFI
  stack shift + fixture）merge 进 vendored `m2381-sync-71` →
  `f129ca7f`（零冲突；上游同期独立删了 test_fileset.py = 与我们本轮
  判断殊途同归），主树收口 `01ab48cf`。设备仓侧：test_fileset.py
  化石删除 `bc64864`（测已退役的 21H16 双模 API，上游从未有过此
  文件）。收尾门：check-m2381 **4 ok 0 fail** + kit 测试 **7/7 绿** +
  CLI 冒烟（defconfig 列单 + tree 落位 = ipados-27.0.1-24A446 正确）。
  备份 tag `m2381-ios27-pre-sync-20261110`（= 90543bfd）。origin 远端
  push = 用户窗口（不主动）。旁路实例（mihomo :17890/:19090）暂留
  /tmp 供下轮 r92 编译门复用；重启即失，重建配方见本条。

- **2026-11-08：集成线转 `upstream/main`，`main` → `5dc00d20`（gap 13 条，
  437 个工作区提交标准 rebase 重放；2 冲突全部按 §0⑦ 续作配方解毕）。
  **谱系修正**：`feat/ipados27-gpu-parity` tip `d2666583` 已被 main 吸收
  （feat tip = main 祖先）→ "lane 跟 feat / main docs 不并入"规则过期，
  现役集成线 = main。上游 13 条主题：**AppSync 适配器**（7fdf1e61，iPadOS
  27 用户应用适配，新功能面）+ **背光 fix**（54796a72，backboardd 选物理
  亮度控制——与我们显示线直接相关，r88 判读后可消费）+ **USB/BT docs**
  （b56f17c0/68b52589/1950226f）+ gitlink bump 4 条（AdrenoMetal
  8d96e320/b6de3eb1 → `6d7f7343`、metal2vulkan 642c4fef → `963f0e64`、
  ArmCpu+AppleKeyStoreState 5dc00d20 → `696e0921`/`58e5461e`）+ 打包管线
  社区 PR。冲突① 99e5a148（test metal extended-range）：gitlink 取上游
  `6d7f7343` + 保留本 lane 脚本（老坑第三现）；冲突② 64bda2d0（s72
  ArmCpu 守卫 93a5030）：上游 pin ≠ 守卫值 → 取上游（守卫使命由上游
  接棒；ls-remote 验证 696e0921/58e5461e 均为各 driver main tip，无幽灵钉）。
  **老坑第四现**：rebase 完成后重放序列后段旧 bump 把三处 drivers
  gitlink 拖回 stale，单独提交 `c13a05af` 对齐（10-31 先例 924877f 同款；
  今后同步轮 rebase 完成后必须 `git ls-tree -r` 对拍全部 gitlink 再收尾）。
  **kits/XnuPrelink 保留 `68ebb26d` 不动**（s74 vendored v12 matcher +
  账本 812 = 出件链运行时权威；与上游 main pin `c645d76b` 互不为祖先，
  上游独有 `ef4fe4f` "bind three more 24A446 kernel imports" 待下轮
  kit 并集吸收）。上游 13 条对 `patches/` 零触碰 → r87 三件套
  （sheng-shared-region-sign-gate / pac-fail-capture /
  kfree-ext-panic-caller）文件零变化免重放。收尾门：check-m2381
  4 ok 0 fail + kit 测试 7/8 绿（`test_fileset.py` 系 s74 pin 68ebb26d
  自带 pre-existing 坏——import 的 `chain_relocations` 在我们线
  native_prelink.py 从 0dbb4fa 起就不存在，与本轮无关，挂账）+ CLI
  冒烟（defconfig 列单/落位，profile = ipados-27.0.1-24A446 正确）。
  origin/m2381-ios27 远端 pin `f2b04369` 落后（push = 用户窗口）。
  备份 `m2381-ios27-pre-sync-20261108`（= fd90ca3a）。

- **2026-10-31（晚）：`upstream/feat/ipados27-gpu-parity` `8fe2aaa` →
  `359d77c`（本 lane 窗口 +10 提交，标准 rebase 重放 295 个工作区提交；
  唯一冲突 = AdrenoMetal gitlink，取上游 `77cb207`，且重放旧 bump 再次
  把 gitlink 拖回 stale `7b885c2`——老坑第二现，已修 `924877f` 对齐；
  check-m2381 4 ok + XnuPrelink kit 7 测试全绿 + CLI 冒烟通过）。
  主题仍纯 GPU 线（ADRENO=n 零消费）：turnip gmem mutable swap patch
  再扩（+185）、frame path trace（runtime/driver/kernel 贯通）、
  render pass timestamp、锁外 BoFree/异步余量/连续分配上限等 perf；
  子模块 bump Adreno740 `7fa13d4` / AdrenoMetal `77cb207` /
  GopFramebuffer `74ff4c4`。**kit 上游（XnuPrelink）零新提交**：main
  仍只有 import+docs（f876e3e），feat/ipados27-gpu-parity 冻结在我们
  matcher pin `76452c6e`；sheng 设备仓仅 +1（08cde20 WiFi 记录）。
  `upstream/main` +12 提交全是社区设备记录 + log-capture.md 改版
  （新增 **recovery reader 通道**：TWRP 类 rooted recovery + ADB dd 读
  log 分区——我们 ACM reader 的备选通道，暂不消费），不在 feat 线上，
  按既定规矩 lane 跟 feat、main docs 不并入。备份
  `m2381-pre-sync-20261031b`。
- **2026-10-31：`upstream/feat/ipados27-gpu-parity` `310fb75` → `8fe2aaa`
  （本 lane 窗口 +9 提交，m2381-ios27 标准 rebase 重放 269 个工作区提交；
  3 个旧时代子模块 pin bump 因被上游超越而 skip，1 个 test 提交
  `fc6ab72` 的脚本部分保留、gitlink 部分取上游 pin；rebase 后
  AdrenoMetal gitlink 被旧 bump 拖回 pre-force-push 的 148698d，已修复
  对齐上游 tip 7b885c2——46d4563。check-m2381 4 ok + XnuPrelink kit
  测试绿 + CLI 冒烟通过）。主题 = GPU/显示诊断线：turnip GMEM mutable
  swap patch（488 行）+ direct scanout fence wait/render mode +
  `unixnu --os` CLI（cli/config/profiles，默认仍走 board os_profile，
  我们零适配）+ `9bfc45a` 的 6 个打包/参照件脚本；子模块 bump
  AdrenoMetal×3 / Adreno740 / GopFramebuffer（**上游 force-push 换血**，
  旧 pin 7f7ae1e 已不在其新历史线上）。feat 仍未并入 main（main 只有
  docs）。对 m2381 直接可用的两件：`scripts/build-fileset-release-
  reference.py`（我们 `--fileset-reference` 用的 RELEASE 参照件构建
  脚本，硬编码 sheng lane 路径，需参数化到 out/m2381 后接走，消除
  17.6MB 树外产物的复现性债）与 `--os` 快捷切 profile。ADRENO=n 不变，
  GPU 线内容暂不消费。kit 上游（XnuPrelink 76452c6）与 uefi-sheng
  上游（sheng 支线）本轮零新提交；设备仓 bti-shape（bf1dac9）回推
  上游仍欠。

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
