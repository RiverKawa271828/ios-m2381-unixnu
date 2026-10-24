# STATE — 2026-10-24（donor 换配 21H16 落地；P1–P3 执行记录见 §0 与 PLAN-P3-CI）

## 0. 2026-10-24 donor 换配（本会话执行）

- **换配定案（用户拍板）**：IPSW root = **iPhone11,2（iPhone XS）/ iOS 17.7
  `21H16` / d321ap**，替代 20H392。根因 = 内核源码 xnu-10063.141.1（iOS
  17.6 代）与 20H392（16 代）全程跨代配对，实机的 entropy 墙（r11）与
  AppleImage4 mix-n-match 墙（r12）皆源于此；21H16 与 fuxi/nx729j 的
  21H461 同 train 原生配对，仍为手机身份。Apple 已不签 21H16，与本流程
  无关（不走 Apple 签名 restore）。
- **IPSW 核对**：sha256 `6fd757be…`（与 ipsw.me 记录一致）；BuildManifest
  d321ap = ApChipID 0x8020（t8020）/ ApBoardID 0x0e；OS 件
  `044-17947-018.dmg`（sha `9b69f645…`）、Cryptex 件 `044-17442-018.dmg`
  （sha `3b21f08a…`）；kernelcache 解压后 sha `0fb5cf59…`。本地件与 URL：
  `research/ipsw/iPhone11,2,iPhone11,4,iPhone11,6_17.7_21H16_Restore.ipsw`。
- **prelink 三值**：slide = `-0x1000000`（上游约定不变，绑定核对裁决）；
  **early_security_init = AMFI `0xfffffff008f3bd18`**，三重指纹法 derive
  （函数序言 6 字在 21H461/20H392/21H16 同型，且各自 AMFI __TEXT_EXEC 内
  唯一命中：21H461 命中 = fuxi 已知值 +0xaae0 ✓，20H392 命中 = r11 实机
  已验证值 +0x13f8 ✓，21H16 = TEXT_EXEC 0xfffffff008f2ea10 + 0xd308）。
  text_fixups 留空（P4 用户态再补）。
- **trust caches**：21H16 两件 sha `090c6a97…` / `8e43fc06…`；flat bundle
  重生 = `e9f6c5b9…`（2868+80 entries，make-m2381-trust-bundle.py PIN 同步）。
- **ADT**：make-m2381-adt.py 身份换 d321ap 真值（iPhone11,2 / D321 /
  "iPhone XS" / fallback iPhone10,3（真 d321ap 值）/ subtype 2436 沿用 m2381
  面板 2400），生成验证通过。
- **NativePrelink kit 三处适配（21H16 = arm64e + t8020 段布局首次出现）**：
  ① `macho()` 门放宽到 arm64e（subtype 0/2 + PAC 版本位；原有显式拒绝
  测试改为新契约）；② `unserialize_xml` 剥 `<plist>` 包裹；③ emit 源段
  由固定段名改为按组件 VA 包含关系解析（t8020 cache 把 kext 段装进内核级
  `__TEXT_EXEC`/`__DATA_CONST`/`__DATA`）。26 项 kit 测试全绿 + 真实 21H16
  cache 干跑（31 roots 中 30 模块全段落位；IOHIDEventDriver 两代同为无
  loadaddr 条目，plan() codeless 路径处理）。
- **Libm literal pools 不用重 derive**：三张池表逐字节同在三代 cache
  （20H392/21H461/21H16），前缀+sha 校验天然跨代。
- **entropy 守卫**：patch 保留（对同代配对无影响、防御性），但 trampoline
  VA `0x46D8B3C` 是 20H392 prelinked 镜像专属，21H16 出核后须重 derive
  （脚本内已加 [TODO kernel step] 标注 + 空填充探针 failsafe）。
- **预检**：check-m2381.py 身份断言换 iPhone11,2/21H16/d321ap + 0x8020/14，
  全绿（仅 root_match_volume_uuid 照旧 WARN 待 P3）。

## 0.1 2026-10-24 续：21H16 指针机制硬 blocker（第 14 轮准备）

- 续会话下钻发现第 13 轮适配不完整：21H16 cache 是 **fileset kernel
  collection**（MH_FILESET + LC_FILESET_ENTRY×239 + LC_DYLD_CHAINED_FIXUPS），
  **无 `_PrelinkLinkKASLROffsets` 表**（20H392/21H461 均有），指针为 dyld
  chained-fixups 编码（16K 页、pointer_format 8、函数指针 auth-rebase
  形态）。kit `plan()` 依赖该表 → CI kernel 步必 KeyError。**21H16 需先
  开发 fileset-KC/chained-fixups 重绑定器**（可离线做，非配置活）。
- 由此产生 donor 决策点：(a) 保留 XS/21H16，先开发重绑定器；(b) 退回
  fuxi 同款 21H461 iPad donor（机制全实证，但平板身份需用户重新确认）。
  详见 `docs/PLAN-P3-CI.md` §J。
- 已完成中性预做：21H461 kernelcache 拉取解压（sha 对上 fuxi pin，无论
  (a)(b) 都要用）；设备 orphan 分支自 70d68bc 重生成（device-m2381=6256b00
  / device-m2381-bringup=d8ba932）；entropy 守卫判定保留（21H16 corecrypto
  14.0 同 21H461 版本，同代配对下无害）。

## 0.2 2026-10-24 续二：donor 定案 21H461（(b) 路先行，预换完成）

- **用户拍板：先试平板。** donor = iPad7,1 / 17.7.11 / 21H461 / j120ap /
  t8011（fuxi/nx729j 同款 IPSW）；XS/21H16 + fileset-KC 重绑定器降级为
  后续主题（§0.1 的 blocker 不阻塞本路）。
- **不依赖本地 IPSW 的预换全部完成**：21H461 件全取（kernelcache 对上
  fuxi pin；BuildManifest j120ap=0x8011/0xc；DeviceTree.j120ap；两 trust
  cache `4563bdbf…`/`9742f3d5…`）；board 两文件全量换 pin（AMFI =
  fuxi 值 `0xfffffff005ab4ae0`，三重指纹在本 cache 独立验证）；**trust
  bundle 重生 = `7416ca4f…`，逐字节复现 fuxi pin**（provenance 交叉验证）；
  ADT 换 j120ap 身份（对真 DeviceTree 逐值验证，scale=2/subtype=2732）；
  text_fixups 换 nx729j 四条 21H461 值；root UUID 与 OWNER UUID 转正
  （同镜像同 UUID，占位清除）；check-m2381 **4 ok / 0 fail / 0 warn**；
  kit 对 21H461 cache 干跑全对（KASLR 表在场、30/31 落位、包含解析回落
  经典段、AMFI 校验过）——本轮 arm64e 适配对经典布局零回归。
- **entropy**：21H461 路不应用二进制守卫（跳板是 20H392 专属；同 train
  先例无需）；源码补丁保留。
- **待本地 IPSW 到货**：全件 sha256 + 两个 dmg 成员哈希 + 两 trust cache
  对拍 board pin（fuxi 值）→ push 窗口 → dispatch（runbook = PLAN-P3-CI §K）。

## 0.3 2026-10-24 续三：最终定序 = 先 iOS，不行转 iPadOS

- **用户拍板**：(a) 路（XS/21H16 + fileset-KC 重绑定器）先行；(b) 路
  （21H461 iPad，`224686f`）为已提交备胎，转换零返工。
- **三门判定**（详见 PLAN-P3-CI §L）：A = 离线 rebinder 自洽（利好：
  21H16 KC 全链接无 bind，只 rebase 偏移）；B = CI prelink 通过；C = 上机
  PAC 语义（对策：运行时再签名或 QARMA 预签名）。任一门确认死路 → 转 (b)。

## 0.4 2026-10-24 续四：门 A 格式层风险清零

- 21H16 链格式（dyld chained fixups，pointer_format 8）完全破译并经全量
  验证：419,323 条目零未解释、目标 100% 落已映射段；条目 = 标准
  auth_rebase 位域 + 镜像基相对 target32，唯一非标 = next 以 4 字节为
  单位；存储值无签名（Gate C 对策不变）。详见 PLAN-P3-CI §M。
- 门 A 剩余 = kit 内 rebinder 实现 + 测试扩绿 + 原生内核自洽干跑。

## 0.5 2026-10-24 续五：门 A 新墙 = release cache 全面符号剥离（用户拍板点）

- 链走查语义层收口：419,323 条 plain meta 位零污染；roots 闭包 31+7，
  选定组件 69,386 站点、组件目标 67 对全在闭包内（0 违规）；kernel 目标
  去重 = **1,795 VA**（详见 PLAN-P3-CI §O.1）。
- **新墙**：21H16 release fileset KC 全 cache 零符号名（kernel nsyms=0、
  组件 symtab 全指向桩、dyld_info 垃圾、trie 空）——经典"按名映射
  kernel 目标"无 donor 侧数据源。跨代字节匹配死于 arm64/arm64e 结构差；
  panic 字符串交叉引用天花板实测 **7.6%**（60% 目标函数零字符串引用），
  判死（§O.2/§O.3）。
- 元数据事实：`_Prelink*` 地址 = 真实 VA 的有符号十进制（u64 补码即真
  值，与 LC_FILESET_ENTRY extoff 精确相等）；19 条 sentinel（INT64_MAX
  无 size）天然跳过。
- **门 A 唯一干净解 = KDK 21H16**（Apple Developer 下载，需 Apple ID
  一次认证）→ 符号旁表喂 rebinder；**无 KDK → 按 §L 转 (b)（21H461
  备胎零返工）**。rebinder 本体与命名源正交、可先行。

## 1. 结论/决策

- **现方向 = 原生 UniXNU 设备移植**（对齐上游 fuxi/sheng 模式）：UniXNU 4K
  内核 + IPSW root（**2026-10-24 起为 iPhone11,2 / 21H16**，原 20H392 决策
  已被换配取代）+ `unixnu-m2381` 设备仓 + `m2381Pkg`（Mu-Silicium
  UEFI）。母版/指南见 `docs/porting-notes.md`。
- **设备数据豁免（用户 2026-10-23 拍板）**：无须保留设备数据，随时可刷；
  每次实际写入仍按 `docs/FLASHING.md` 流程走。
- **P2 UEFI 基座（本会话定案，A3 评估产物）**：`m2381Pkg` 从 **shengPkg
  骨架**派生（`reference/uefi-sheng`，唯一同 SoC 且 UniXNU 全链——菜单/
  UniXNUDxe/DarwinBoot——编进 FV 的模板），平台 blob 用 **Device-Binaries
  `@036ba9f7` 的 mars 集**（nura 现役链每天实证的那套 UFSDxe/时钟/RPMh
  胶水，公开仓可 pin；注意 mars ≠ infiniti，UFSDxe.efi 哈希不同），板级
  常量从 nura `m2381Pkg` 搬（MemoryMapLib/m2381.toml/FdtBlob）。依据：
  ① uefi-fuxi 的 fuxiPkg 是半程 bring-up 态（DarwinBoot/FuxiBootMemoryDxe
  全注释、vendor 栈关闭、Binaries 空），不能当母版；② nura 的包是 Linux
  EFI-stub 链，把 UniXNU 链移植进去成本高；③ shengPkg 的 UFS 也是
  device-extracted blob（非源码），binaries/sheng 公开仓没有——mars 有。
- **payload 载体定案（量化）**：m2381 ABL 的 gunzip 窗 [0x80080000,
  0x85580000) 共 ~85MB（kernel+ramdisk+DT 间隙合计）。fuxi/sheng 式
  ramdisk 载荷（192MiB FAT 打进 boot.img）在这个窗里算出负 RdLoad，直接
  触发"Not Enough space"拒载 ⇒ **payload 必须走 recovery_a ESP**
  （M2381ESP ≤96MiB，negroni 先例；nura 的 Mu 链每次开机都在实 proof
  mars UFSDxe 读 recovery_a 这条路）。
- **构建主机定案（P1 门槛实测）**：Linux x86_64 跑通 defconfig/fetch/tree/
  status/ipsw fetch（kernelcache 用 `research/bin/ipsw` 预置绕开
  compression_tool）；**kernel 步 = xcodebuild + iphoneos SDK，只能
  macOS**（本机实测失败点即此；nx729j GUIDE `[已核对]` 同）。内核 =
  GitHub Actions `macos.yml`（workflow_dispatch：device_repository/
  device_ref/defconfig/steps/ipsw_url…，macos-26 + Xcode 26.6）。
  **前提 = 设备仓独立成库推 GitHub**（workflow 契约要求 device/board.toml
  在库根）+ `UNIXNU_UNIQUE_CHIP_ID` secret——推送窗口 = 用户拍板（红线 8）。
- **unique-chip-id（软件种子，2026-10-23 定）**：`3645ad55d47e77c1`。
  设备侧无可暴露的整机硬件序列号（UFS wwid 只有厂商 EUI），按 nx729j
  惯例生成一次、永久使用；喂 ADT `/chosen/unique-chip-id` + `chip-id=0x207`。
- 设备连接：USB 网络（NCM），`172.16.42.1:23`，宿主接口 `enp104s0f3u1`；
  工具 = `/home/ri/work/nura-meizu-m2381/tools/{devsh,devpull,devpush}.py`。
  设备在线、root 可用。注意本机 /dev/block 是 major:minor 命名：
  sda=8:0，sda22=259:6，sde43(boot_b)=259:34，sde27(recovery_a)=259:18。

## 2. 设备现状（2026-10-23 实测；nura OS 在跑）

| 分区 | 设备 | sha256（整分区读回） | 备注 |
|---|---|---|---|
| boot_a | sde13 | `5f9f1503…` | nura |
| **boot_b** | sde43 | `af574775…`（整分区） | **现役启动槽**；内容 = **mu-r91-350386e0**（nura 件，1.1MiB，前缀哈希命中；整分区 sha 含尾部旧镜像残留属预期） |
| **recovery_a** | sde27 | `0a102220…` | 现役 nura ESP = **esp-recovery-v71**（逐字节对拍命中） |
| recovery_b | sde56 | `20bcd203…` | nura |
| **userdata** | sda22 | —（不整读） | **nura root**（"Nura edge"，ext4）；≈463.7GiB |

- **现役镜像对拍（A2，本会话）**：recovery_a = `esp-recovery-v71.img.gz`
  逐字节吻合；boot_b 前 1,110,016B = `mu-r91-350386e0.img`（设备侧读回
  哈希 `350386e0…` 与文件名内嵌 sha 一致）。**回滚链完整**：全部现役件
  都在 `/home/ri/work/nura-meizu-m2381/artifacts/`（mu-r62…r91、esp v46+）。
  本仓 `artifacts/images/` 的三件旧回滚件都不是现役版本。
- **userdata 几何三源对拍（本会话，全部吻合）**：
  ① sysfs 换算（10-23）；② 实机 GPT 表项 sda22 first=2,330,472
  last=123,871,226；③ GPT header last_usable=`0x7621FFA`（=123,871,226）。
  board.toml 的值为权威值。GPT 全表已存档见会话记录（32 项，private=sda6
  1,792 扇区等与 BOOT-CHAIN-RE §6.2 一致）。
- stock fastboot 包（gpt_main0/rawprogram）本地未找到；实机 GPT 直读是
  更强证据，`[FW]` 侧佐证缺口已闭环（stock 包以后找到可作第四源）。

## 3. 资产（本地，不入 git）

| 路径 | 状态 |
|---|---|
| `research/ipsw/iPhone10,3,iPhone10,6_16.7.16_20H392_Restore.ipsw` | **已下载并核对**：sha256 = `ece1194f…` ✓（6,171,035,648 B）。dmg 成员哈希已进 board.toml |
| `out/ipsw/20H392/` | kernelcache.decompressed（`2ec297ba…`，已回填 board.toml pin）+ DeviceTree.d22ap.im4p + 两个 trust cache（`63e01c03…` / `9cc478fe…`） |
| `reference/uefi-m2381-nura/` | nura Mu-Silicium fork（meizu20-mars-port）sparse 克隆：m2381Pkg 源码 + m2381.toml，P2 常量来源 |
| `/tmp/dev-binaries/` | Device-Binaries 克隆（mars @036ba9f7 验证用；临存） |
| `artifacts/images/{mu-r69…, esp-v71…, shim-boot…}` | 旧回滚件（**非现役**；现役恢复件在 nura artifacts） |
| `artifacts/vendor/meizu20-uefi-mods/` | 本机 OEM UEFI 模块（P2 备选，mars blob 卡死时用） |
| `~/.local/bin/compression_tool` | Linux shim（lzfse 封装；本机环境件，不进 git） |

## 4. 当前没做/不能声称的

- **内核没编**：kernel 步只能 macOS；CI 需先推设备仓（用户拍板）。
  Linux 侧到 xcodebuild 缺口为止全部跑通。
- **P2 已达成（2026-10-23 深夜，commits `8e78e03`+`c9dc5d4`）**：m2381Pkg
  overlay（`devices/m2381/uefi/`）+ uefi-sheng 工作树（`out/m2381/uefi`，
  Binaries@036ba9f7 mars）→ **Linux 本机出 FV**（3MiB，DEBUG_CLANGPDB），
  FV 体检：UniXNU/UniXNUDxe/DarwinBoot/UFSDxe/BdsDxe/SimpleFbDxe/ButtonsDxe
  全在卷；KEP 库虽在 Silicon 树编译但 FV map 零命中（未链接，无雷）。
  `package-m2381-boot.py` 产出确定性 boot.img（1,662,976B，v4 头 + 无
  appended DTB + Gunyah WDT off shim，两次构建同 sha `40f2a199…`）。
  **未上机，不能声称 ABL 收这枚镜像**。
- **prelink 三值待重derive**：slide / early_security_init / text_fixups
  还是 21H461 的值，20H392 的要在 CI/kernel 步重derive（board.toml 已标注
  TODO；prelink 指针绑定核对会大声失败，failsafe）。
- **root_match_volume_uuid / CONFIG_XNU_OWNER_DATA_VOLUME_UUID**：P3 建
  20H392 userdata 时派生（fuxi 的 21H461 值不可抄；check-m2381.py 会拦截）。
- 没有 m2381Pkg 文件、没有 FV、没有镜像、没有刷机。
- m2381Pkg 的 UEFI 侧未开工（P2）；sheng blob 在 m2381 上未实测（mars 有
  nura 实证，sheng 无）。
- `fastboot getvar current-slot` / `partition-size:userdata` 复核未做
  （刷机窗口做；GPT 直读已闭环几何，此项是形式复核）。

## 5. P1 执行记录（2026-10-23 本会话，对照原计划 §A/B/C）

**A. 消化 nura 资产 — 完成**
1. BOOT-CHAIN-RE 全文读完；`m2381-bringup.md` 的 ABL 约束逐条对回源文档，
   并新增：KEP 是 Mu 链雷（sheng/uefi-sheng 树与 nura 现版源码均无，出 FV
   后 grep 复核一次）、FD>15MB 死因修正（非 ABL 尺寸门；BootShim 拷贝越
   0xA8000000 栅栏为候选——FD 保持 3MB 不扩）、`oem kernel-log-enable`
   写路径会卡死 ABL（禁用）。
2. 现役镜像对拍：recovery_a=v71 ✓、boot_b=mu-r91（前缀法）✓，回滚链完整。
3. P2 基座评估 → shengPkg 骨架 + mars blob + nura 常量（§1 决策）。
4. ramoops `0xa5100000`：ABL 零感知/活动范围够不到 ⇒ DT panic region
   候选成立（porting-notes §5 取证栈配套）。

**B. 设备仓骨架 — 完成（`devices/m2381/unixnu-m2381/`）**
5. board.toml（全部值带 [DTB]/[DEVICE]/[FW] 标注）：ipsw（chip 0x8015/
   board 6 从 BuildManifest 读）、userdata（三源对拍）、payload（M2381ESP
   96MiB/ESP 路线）、prelink pin（20H392 kernelcache sha 实测回填）、
   trust cache sha（IPSW 内件直算）。
6. `make-m2381-adt.py`：iPhone 身份（d22ap 真值：model=iPhone10,3、
   target=D22、product-name="iPhone X"、idiom=phone、subtype=2400、
   scale=3），全字段解析验证通过。
7. `check-m2381.py`：preflight 骨架（TODO 值=硬失败，实测拦截 P3 占位）；
   `package/unixnu.toml` 菜单；`configs/m2381_defconfig`（=SoC 级 fuxi 版
   + UUID TODO 标注）。
8. unique-chip-id 种子：`3645ad55d47e77c1`（§1）。

**C. 构建打通 — Linux 侧全部跑通，kernel 步定案 macOS/CI**
9. DarwinBoot 坏 pin 修复（`85332b317ee8…`）；defconfig ✓ / fetch ✓
   （submodules+feeds，`unixnu.lock` 生成）/ status ✓（全部模块解析
   base）/ ipsw fetch ✓（kernelcache 预置 + DeviceTree + 2×trust cache）；
   `build kernel` 失败点 = xcodebuild（定论）。

## 6. 开放问题（更新）

- ~~payload 载体~~ **已定案**：recovery_a ESP（§1，量化依据）。
- ~~UFSDxe~~ **方案已定**：mars blob @036ba9f7（nura 实证）；OEM blob 备选。
  sheng blob 是否必要 = P2 实验题。
- ~~构建主机~~ **已定案**：kernel = GitHub Actions macOS（macos.yml 原生
  支持外部设备仓 + 本地 IPSW URL 覆盖）；其余全部本机 Linux。
- **设备仓推送窗口**：CI 前把 `devices/m2381/unixnu-m2381` 推成独立库
  （或用户指定其他形态）——等拍板。
- ~~20H392 prelink 值~~ **已随 21H16 换配 derive（2026-10-24）**：slide
  `-0x1000000`、AMFI `0xfffffff008f3bd18`（三重指纹法，见 §0）；text_fixups
  留待 P4 用户态。21H16 出核后仍欠：entropy 守卫 trampoline 重 derive。
- **kernel defconfig 板级差异**：目前 = fuxi 全同（SoC 级）；PMIC/按键/
  显示/触摸差异在 P4+ 主题化。
- **显示/按键**：cmd-mode DSC 最小点灯（SimpleFbDxe 用 ABL 帧缓冲）、
  PMK8550 按键 SID——P2 m2381Pkg 内解决。

## 7. 下一步（按顺序）

0. **P3 = `docs/PLAN-P3-CI.md`**（2026-10-23 定，推送授权已给）。
1. ~~**P2 开工**~~ **完成（2026-10-23）**：FV + boot.img 均在本机产出；
   overlay 工作流 = `devices/m2381/uefi/sync.sh` → `unixnu build firmware
   --uefi out/m2381/uefi` → `package-m2381-boot.py`。P2 遗留 = 上机验证
   （P4 才做）。
2. **CI 内核**（等推送拍板）：设备仓独立成库 → dispatch macos.yml
   （steps=kernel，defconfig=m2381_defconfig，ipsw_url=本地件需公网可达或
   用 board.toml url）→ 顺路重derive prelink 三值。
3. P3：userdata 重建件（macOS/CI 步）+ 两个 UUID 派生 + check-m2381.py
   放行。
4. P4 上机门：UEFI 菜单 → UniXNU → XNU banner → launchd（四门 + 刷写纪律
   照 `docs/FLASHING.md`；首刷前 `fastboot getvar` 形式复核）。
