# STATE 历史全档 — 2026-10-24 → 2026-10-27（原样快照）

> **2026-10-27 重组归档**：本文件 = docs/STATE.md 重组当时的**原样快照**
> （§0.0–0.34 全部轮次 + 当时静态区，未改写）。现势状态、方法资产与下步
> 工作单见 `docs/STATE.md`；轮次索引表也在新 STATE §1。**注意 §0.30 的
> =y 车道施工配方仍被下轮工作单（备选 b）引用**。每轮定谳同时完整活在
> git commit message 里。

---

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
  备胎零返工）**。
- **rebinder 本体已实现**（32 项 kit 测试全绿；真实 cache 链走查 419,323
  条对拍精确；真实 cache 冒烟精确停在命名门）——只欠符号旁表数据源
  （PLAN-P3-CI §O.4）。

## 0.6 2026-10-24 续六：终局拍板 = 21H461 iPad lane 正式转正（本会话执行）

- **用户拍板：路线定死 (b) iPad lane**，21H16/XS lane 关闭（KDK 21H16
  公开镜像确认不存在，Apple ID 路线不再等待；fileset-KC 重绑定器保留为
  后续主题，机制证据链在 PLAN-P3-CI §J/§M）。附带评估了 HackiOS
  （oosexgang）替代路线并否决：仅 redwood SM7475 + fastboot boot 纯内存
  模型在 m2381 不可迁移（红线 5），donor = iPhone X/A11 无 PAC 亦非门 C
  透传实证；mining 项 = GPU Mesa/freedreno 桥 + ath11k/LinuxKPI 参考。
- **IPSW 到货复验通过**：`research/ipsw/iPad_Pro_HFR_17.7.11_21H461_Restore.ipsw`
  （6.4G），两个 pin 成员 sha256 与 board.toml/fuxi 值逐字节一致
  （`044-76034-038.dmg` = b4908cde…、`044-76040-038.dmg` = 3f99b859…）。
- **check-m2381.py 全绿**（4 ok / 0 fail）：身份 iPad7,1/17.7.11/21H461/
  0x8011/board 12、userdata 几何、M2381ESP payload、root UUID 全对。
- **本地清理（用户指示）**：删除 21H16 XS IPSW（7.3G，lane 已关闭，Apple
  CDN 可随时重下）、`research/ipsw-x/`（旧 iPhone X 残留）、空 `ipsw/out/`；
  research 占用 14G→7.4G。保留：21H461 IPSW、`research/bin/ipsw` 工具、
  `Kernel_Debug_Kit_15.8.2_24H230.dmg`（macOS fileset-KC 带符号参考件，
  重绑定器后续主题唯一在手的签名参照，重下需 Apple ID）。
- **上游动态（用户问"有没有更新"）**：`upstream/main` 1a01c2a→bd127c0，
  **m2381 落后 208 commits**。重点：**PR #100 kernel 步可在 Linux 跑**
  （xcrun shim + clang，上游验证 Linux/macOS 内核符号级等价）——本机
  即可出核，摆脱 macOS runner 容量依赖；另有新社区移植文档（polaris
  SM8450、malachite MT6878、pong 等）+ driver catalog/仓库规则。**rebase
  未做**（留待下会话按 UPSTREAM.md §2 一次主题执行）。

## 0.7 2026-10-25 续五：第 22 轮首启读图 → 发布契约死结修复（详见 PLAN-P3-CI §U）

- **实机**：userdata 5.7GiB + recovery_a `c767358c` 刷后首启 = 绿条→蓝条→
  panic `Failed to mount root device`（IOKitBSDInit iOS 路径 = root 媒体
  未出现）。**根因 = `ufsBuildRootPlan` 上游契约 count≥2 与 §T 单策略
  单窗口镜像的代码级死结，与刷写质量无关**；现役 owner 配置 OWNER_* 全关，
  SystemOS/Data 挂载 panic 未编入，修后 root 挂载成功即直奔 launchd。
- **修复**：RootPublication.h 放行单 OS 窗口（多窗口契约不变，order[0]
  越界坑同修）+ 测试扩绿；三份拷贝同步。**新 owner 内核 `220721ca` + 新
  payload `e1e83910`（100MiB）已本机产出**，桌面校验（ADT/trust/ESP 内
  kernel sha）全绿，待用户单刷 recovery_a。
- **读图判别器**：launchd 级画面/exec 类新 panic = userdata 落地 + root
  挂载 + VerifiedRoot 全链实机闭环；仍同 panic = userdata 未落地（读回
  欠账坐实，probe=11 取证）。

## 0.8 2026-10-25 续六：r22 盲启更正 + 探针恢复（详见 PLAN-P3-CI §V）

- **r22 第二次读图（payload e1e83910）= 绿条+entry 诊断后静默**。官方新
  debugging 文档+探针脚本双源实锤：绿条=DarwinBoot 画、蓝条=入口探针
  stage1、panic 屏=探针补丁——**r22 payload 没打探针（device-bringup
  board.toml panic_screen=false）→ 盲启，非回退**。
- **恢复探针**：patch-xnu-entry-probe.py 的 rebase() 自适配新内核布局，
  一次通过 → 探针内核 `6666f96d` + payload **`e3c5c0fa`**（100MiB），
  桌面校验全绿，待刷 recovery_a。判定表见 §V：无蓝条=DarwinBoot 侧问题、
  蓝条=入口已执行、品红=MMU 前、panic 屏=读因、恢复界面=launchd 里程碑；
  5.7GiB 校验期间静默数分钟属预期。
- **官方格式 2 文档**（migration/steps/ufs/debugging）：SoC 共享仓
  sm8550-common + 21H461 OS profile + 组件改名已上线，sheng=格式 2 参考；
  迁移列为独立主题。

## 0.9 2026-10-25 续七：r24 读图 = 挂载贯通，墙到根认证（详见 PLAN-P3-CI §W）

- **payload `e3c5c0fa` 实机**：绿条→蓝条→`rootvp not authenticated after
  mounting @bsd_init.c:1100`。**单窗口修复实机生效、NativeUfs 发布块设备、
  APFS root 挂载全部闭环**；蓝条同时证明内核入口无恙（r22 非回退实锤）。
- **新墙 = owner 根认证**：`autherr && !owner_authorized` → panic；owner
  判定需 OWNER_ROOT_AUDIT+OWNER_ROOT_TRUST 双开（fuxi/nx729j 终态三件套
  = NATIVE_VIRTIO_BLOCK=y + 双旗 + verified-bundle 归档）。授权器
  `native_verified_block_root_authorized` 在 NativeVirtioBlock 归档（现役
  builder 只需 storage-abi/include，无需 interface-audit.json）；checkpoint
  在内核补丁内。
- **执行**：defconfig 四项翻开关（VIRTIO_BLOCK+ARCHIVE+AUDIT+TRUST）+
  board archives 插 virtio verified-bundle 构建；重出内核中。

## 0.10 2026-10-25 续八：r25 = launchd exec 里程碑，stage 7 双卷车道就绪（详见 PLAN-P3-CI §X）

- **payload `6e964598` 实机**：`initproc failed to start — Library not
  loaded: /usr/lib/libSystem.B.dylib (no dyld cache)` from /sbin/launchd —
  **根认证过、launchd 已 exec、dyld 已运行**，内核侧全链实机贯通。
- **新墙 = Cryptex SystemOS 卷缺席**（dylib+dyld 缓存都在 cryptex；单窗口
  镜像没有）。同锚死结解除 = 锚钥持久化（设备仓 keys/owner.pem）+
  `--include-systemos` 双策略 bundle（上游原生支持）。
- **全部落地**：SystemOS policy 脚本 + ci-owner-policy.sh 双卷版 + 分支/
  workflow 配套 + 两份 board windows += SystemOS + owner defconfig
  （-bundle 归档 + CACHE_MOUNT/CACHE_STARTUP/BOOT_MANIFEST/UNION_SYSTEMOS=y，
  DATA_MOUNT 仍 n）。**待用户：push 两分支 → dispatch steps="userdata"
  （pre_build 产证据+双窗镜像）→ 取件 → 本机出内核/payload → 双刷
  （userdata ≈9.4GiB + recovery_a）**。读图：恢复界面/logo = stage 7
  里程碑。

## 0.11 2026-10-25 续九：r25b 读图 = 诊断屏点名 VR_BLOCK，墙 = 树内读路径陈旧（§Z v2 未进内核）

- **诊断 payload 实机**（kernel_args += `native-ufs-diagnostics=1`，payload
  `4422a123`）：双刷后首启 = 绿条→蓝条→**UFS ROOT FAILURE 诊断大字报**：
  `root window read failed` / `stage=read block integrity` / **`verify=5`
  （= vr_status `VR_BLOCK`）** / found=2 prepared=2 published=1
  bytes=`c4e00000`（SystemOS 3,303,014,400）/ manifest=0、GPT 几何逐位对、
  SMMU 零 FAULT、READ(10) ocs=0——**userdata 落地、双策略加载、发布线程
  全部健康；死因 = 第一次校验读（reads=0 failures=1，window=1 SystemOS，
  APFS 探测刚发布媒体）`vr_verify_block` 返回 VR_BLOCK**。
- **根因 = §Z v2（`ufsReadWindow` stride 参数化）从未编进内核**：
  device-bringup 是 r22 时代手工 cp 快照，§Z 只改 fork；kits 有刷新机制
  （CI `ci-owner-policy.sh` 现拉 + feeds 规则 5）而 **drivers 无任何刷新
  纪律** → 树编进内核 `9e72b039` 的读路径 = 上游 4K 版（按 4096 切块、
  4K 索引调 verify），16K 策略要求 n=16384 → `n != expected` 必返回
  VR_BLOCK。r28 的"16K 预期命中"只验了策略字面量，读路径侧漏网。
- **修复**：fork→bringup→tree 四文件同步（`NativeUfs.cpp`/`RootWindowRead.h`
  /`tests/RootPathBridge.cpp`/`tests/RootWindowRead.cpp`）；host 测试双绿
  （RootWindowRead 全项 + RootPublication SystemOS-first/lone-OS 全契约）；
  重出内核 **`a10a1aea`** → 探针件 **`5c726a60`**（panic screen rebase 一次
  过）→ **payload `f726c7fd`**（100MiB，diagnostics 保持开）。
- **读图（下次启动）**：过墙 = launchd 级画面（stage 7 里程碑→Data 种子）；
  仍大字报 = 下一墙现名（哈希不匹配类 = 数据层取证）；panic 位 =
  NativeUfs.cpp 读失败漏斗。
- **陷阱入册**："fork→bringup→tree 三处同步"适用于 **drivers**（r28 只记录
  了 scripts）；候选硬化 = bringup drivers 改 symlink 或构建前 drift 检查
  （待拍板）。ADT `random-seed` = `os.urandom(64)`（make-m2381-adt.py），
  adt_sha 每次构建必变属设计内。

## 0.12 2026-10-25 续十：payload `f726c7fd` 实机 = stage 7 里程碑闭环（设计终点逐字复现）

- **读图**：绿条→蓝条→`tzinit[17] exited -- no exit reason available --
  (signal 0, exit status 30 )` + 寄存器页（free 掉 ~180k 页 ≈ 用户态真实
  跑过）。**16K v2 修复实机生效**：双窗发布→APFS 挂载→根认证→launchd
  exec→dyld 吃到 SystemOS cryptex 缓存（r25 "no dyld cache" 墙清除）→
  launchd 派生作业至 pid 17。
- **此画面 = 脚本钉死的 stage 终点，实机逐字复现**：
  `check-owner-boot-manifest-checkpoints.py` 要求
  `critical_process_exit == (process='tzinit[17] exited', signal=0,
  status=30, cs_status='')`（"Preserve the real later timezone failure"）；
  机理 = tzinit 做 `/var/db/timezone/zoneinfo` symlink 返回 EROFS（/var 在
  未挂的 Data 卷上）→ exit 30 → `kern_exit.c:1875`
  `proc_handle_critical_exit` 按 bring-up 纪律 panic（host 有界跑同款
  钉死）。**userdata+双窗+双策略+挂载+认证+launchd+dyld 全链实机闭环。**
- **下一站 = Data 种子**（§7 分支①，机制已由
  `check-native-ram-data-checkpoints.py` 全定义）：RAM Data 卷
  （32MiB `owner-data-32.cdr`，sha 钉 `d7cd79af…`；发布 UUID =
  defconfig 已钉的 `18AB5557-…`）→ tzinit symlink 成功（timezone 过）→
  下一墙 = `ram-data-first-abort`。三个待解工程题：①defconfig 翻
  OWNER_DATA_MOUNT + RAM block 旗标组；②种子投递通道（嵌内核 = 69.5+32MiB
  逼近 96MiB ESP 硬顶，候选 = ESP 文件/UEFI 内存通道/ABL 忽略 ramdisk 的
  反面）；③fixture 生产（prepare-owner-data-fixture 记录需 macOS hdiutil
  → CI pre_build 候选 vs 本地 Linux 化）——**待拍板后开工**。

## 0.13 2026-10-25 续十一：stage 8 开工 = Data 种子三题拍板并执行（用户授权"你来拍板"）

- **三题定案**：①投递通道 = **ESP `Payloads/scratch-data.img`（DBRDISK1
  打包）+ unixnu.toml `initrd` 行 + DarwinBoot `Request->Ramdisk` 装载 +
  ADT `/chosen/memory-map` PackedRAMDisk 占位 + NativeRamBlock 稀疏导入**
  ——全链上游现成设计（packed_ramdisk.py pack / DarwinBoot PackedRamdisk.h
  / payload.py `--data-ramdisk` 自动写 initrd+拷文件 / board 已预埋
  `packed_data_ramdisk=true`），零新代码；嵌内核方案废弃（ESP 硬顶）。
  ②fixture 生产 = **CI pre_build**（hdiutil/diskutil/OS-mount 全在
  ci-owner-policy.sh 车道；`prepare-owner-data-fixture.py` 新增
  `--volume-uuid` 用 diskutil changeVolumeUUID 钉死 18AB5557）+ 打包后与
  verified-root artifact 同车（workflow upload glob += build/owner-data-fixture/**）。
  ③defconfig 三旗标 = NATIVE_RAM_BLOCK=y + OWNER_DATA_MOUNT=y +
  DATA_VOLUME_UUID="18AB5557-…"（owner defconfig，devices/ 源同步）。
- **内核链接新墙 = baseline 的 virtio/RAM 交叉审计**（build-xnu-4k-baseline.sh
  :697：两 transport 各需 bindings.json 且 bindings 字典相等）。上游写法
  `build-native-virtio-bindings.py` 需 live-device 的 interface-audit.json
  （nx729j 注释明示静态车道不可得）→ **新设备脚本
  `build-storage-transport-record.py`**：以 build/io-storage-family/
  bindings.json（prelink-storage-bindings.json 静态 pin，18 条原始类绑定）
  + 实际 .o sha 写两份记录（board archives 各插一行）。
- **运维坑两记**：①config 变更后首次构建可能在 archive 阶段死于
  link.filelist 指向被中断构建删除的对象（本轮 = kern_exec.o）——用
  kern_exec.o.json 里的 compile DB 命令手动补编即可过检，baseline make 随后
  会按新配置真正重编；②`./unixnu defconfig NAME` 生成 out/.config（kernel
  构建读 .config 而非 board 的 defconfig 字段），改 defconfig 后必须重跑。
- **产出与在飞**：内核 **`1abf8c8a`** → 探针件 **`282d793f`**（panic screen
  rebase 一次过；NativeRamBlock/owner-data 符号在内核 ✓）；payload 待
  fixture 到货后 `--data-ramdisk build/owner-data-fixture/scratch-data.img`
  一步出件。双分支已推（m2381=`a3ec270`、device-m2381=`3b82b20`，git 端点
  恢复、普通 force push 即可）；**dispatch run `40208347346`**（steps=kernel
  最小合法值，pre_build 产 fixture，upload_verified_root=true，守望
  /tmp/r30-watch.log）。**待用户：取件 verified-root artifact（内含
  owner-data-fixture）→ 我出 payload → 只刷 recovery_a**。读图预期：
  tzinit symlink 成功（timezone 过）→ 下一墙 `ram-data-first-abort`。

### §0.13a CI 车道连环修（2026-10-25 深夜，宿主重启前交接）

- **dispatch 必带 `runner_label=macos-15`**：首dispatch（`40208347346`）漏传
  落进 arm64 池挂了一夜（r25 教训重犯）；Intel 车道秒接单。
- **fixture 步四修到绿**（run `40219843577` pre_build 全 PASS）：①
  `diskutil apfs changeVolumeUUID` 动词不存在 → 改离线字节手术；②校验和
  **不是经典 Fletcher64**——真算法 = linux-apfs-rw object.c 的
  Nakassis-complement（u64 不取模累加 + `c1/c2 = 0xFFFFFFFF-((…) mod)`，
  本地拿 21H461 OS 容器 12 个真 NXSB 逐字节验证 12/12）；③**APSB 的
  uuid 字段在块内偏移 240**（56 是 NXSB 容器布局；用 SystemOS 卷已知
  UUID 8846FE78 在 3 块真 APSB 里定位）；④bytearray 切片是拷贝——补丁后
  必须 `data[base:base+BLOCK] = block` 写回（本地模拟抓到，CI 白烧一轮的
  教训：先本地模拟再上 CI）；自检不过的伪 APSB 命中跳过不炸。最终：
  uuid 读/补/重封 3 块 ✓、**DBRDISK1 打包 32MiB→995KB**（ESP 余量无忧）。
- **kernel 步新墙**（`40219843577`）：CI `./unixnu build kernel` 的
  submodule pass 试图把 UniXNU/VerifiedRoot 克隆回被设备 fork 顶替的目录
  → pre_build 顶替后 `git rm --cached kits/VerifiedRoot` 注销 gitlink。
- **在飞：run `40220630199`**（Intel 车道，m2381=`37457b8` /
  device-m2381=`7cc3398`）：pre_build（已证绿）+ CI 内核编译（首次走我们的
  owner 车道，含 verified-root-if-evidenced/--include-systemos bundle）。
  宿主重启后守望进程已死，下会话直接查 run（token /tmp/m2381-gh-token，
  若 /tmp 被清则用 git credential store 重新提取）。
- **下会话工作单**：①查 run 40220630199（绿=给用户下载指引：artifact
  `unixnu-m2381-verified-root-1` 内含 owner-data-fixture/，放
  device-bringup/build/ 下 → 本机 `./unixnu build payload -D
  out/m2381/device-bringup --unique-chip-id 3645ad55d47e77c1 --trust-cache
  out/m2381/trust/original-trust.bundle --kernel out/m2381/kernel/kernel.probe.macho
  --data-ramdisk <device>/build/owner-data-fixture/scratch-data.img` →
  桌面校验（payload.json 含 data_ramdisk_sha256 + initrd 行在 ESP
  unixnu.toml）→ 用户只刷 recovery_a；炸=拉 job log 继续修）。②读图：
  tzinit symlink 过（timezone 关卡）→ 下一墙 ram-data-first-abort。

## 0.14 2026-10-26：设备仓 schema v2 迁移（本会话执行，上游 16e592a）

- **迁移内容**（`docs/porting/migration.md` 全步，本机验证到补丁应用）：
  `board.toml`/`board.bringup.toml` → `schema_version = 2`（`soc_source = sm8550`
  pin `23b2049f`、`os_profile = ipados-17.7.11-21H461`、`[components].include`、
  `[kernel] config_fragments/patch_series`、`[device_tree]`）；`[ipsw]`/
  `[kernel.prelink]`/text_fixups 收编进 OS profile（板只留 trust_cache_sha256
  三值覆盖）；**ADT 改走 v2 公共生成器**：新 `dts/m2381.dts`（含 sm8550.dtsi
  的 intc/ufshc/ufsphy 导出 + 16GiB 单连续窗 + ufs/ufsphy enable；GPU 节点保
  持 disabled 与 v1 一致），`make-m2381-adt.py` 留作参考件；`feeds.toml` 新增
  sm8550 soc feed；空 `patches/xnu/series`。
- **audit 清单 → 静态绑定（P3 遗债 #1）**：owner 板 archives 撤
  `audit-storage-interface.py`（fuxi 活机 gdb-stop.json 依赖，CI 必死），换
  nx729j 静态路径 `build-storage-bindings.py`（prelink-storage-bindings.json
  190 slot）+ `XNU_EXTRA_LD_ARCHIVES` force_load `libStorageBindings.a`；
  bringup 板同构（v1 已如此）。verified-root stub/if-evidenced 双件入 owner
  清单（有证据→真 bundle，无→refusing stub）。
- **改名迁移（P3 遗债 #2）**：`kits/NativePrelink`→`kits/XnuPrelink`、
  `drivers/NativeUfs`→`drivers/ShengUfs`（**均为设备 fork 胜出**：Libm literal
  pools/fileset rebinder；SMMU takeover/单窗口/16KiB 校验块——目录名对齐后
  v2 解析器自动让设备版盖过上游 submodule）；删设备仓 stale
  `scripts/build-arm-driver.py`（v1 旧脚本会盖掉上游 v2 目录驱动版）；
  `m2381_defconfig` VIRTIO bundle 路径改 `virtio-mmio-block-verified-bundle/
  libVirtioMmioBlock.a`（对齐 sheng 绿灯配置）；`NATIVE_LEGACY_DISPLAY=n`
  （display-analysis ABI 审计依赖活机证据，显示走 UEFI GOP offline-legacy-*）。
- **配套**：`ci-owner-policy.sh` 增加 XnuPrelink fork 换装（同 VerifiedRoot
  模式，防 submodule 重克隆盖 fork）+ v2 `[ipsw]` 从 profile 读；
  `make-device-branch.sh` 拷贝清单加 `feeds.toml dts patches`；
  `check-m2381.py` 升级为合并 arch/OS profile 后检查（4 ok / 0 fail 维持）；
  包补 boot-progress/adreno-kits 所需 sheng kit 源（BootProgress 系 +
  ShengGmuVotes.c）+ 拷 `build-boot-progress.py`/`build-adreno740-kits.py`。
- **本机验证**：`unixnu defconfig m2381_defconfig`/`m2381_bringup_defconfig`
  双板过（v2 输出布局 `out/m2381/ipados-17.7.11-21H461/<config>/`）；14 组件
  （Adreno740 依赖链自动入列）；58 补丁全应用（arch 53 + soc 2 + os 1 + 设备
  fork XnuPrelink 2）；DTS 编译 + `devicetree.generate` 出 ADT 13820 字节
  （seed/data/trust 映射齐）；树装配三 fork 全胜出；kit 测试（12+fileset）绿。
- **已知偏差（记录在案）**：v2 ADT cpu compatible = 各核真实 cluster
  （v1 全写 a715）；arm-io ranges size 0x8000000→(dtsi 导出算得)；ufshc/ufsphy
  带 reg 导出（v1 ufshc 仅 compatible）——均为 sheng 同 SoC 实证形态。
- **下一步 = CI 重发**（用户拍板窗口）：make-device-branch.sh 重建
  device-m2381 后 dispatch macos.yml（steps=kernel userdata payload，
  pre_build=ci-owner-policy.sh）。未刷机。

## 0.15 2026-10-26 续：本机 v2 出核彩排全绿 + CI 重发（本会话执行）

- **彩排结论（STATE §7.0 工作单）**：本机 Linux 全链 = bootstrap 编译 →
  19 archive 步 → 真实链接 → **prelink 出核**
  `kernel.macho = ee3255a8…`（69,615,616 B，v2 布局
  `out/m2381/ipados-17.7.11-21H461/m2381/kernel/`）。check-m2381 4 ok。
  六道墙全部修复（commit `cca8148`，一次主题）：
  ① **verified-bundle 旗标**：16e592a 的 build-arm-driver 强制
  `--verified-root` 且 verified 模式只认 `NativeVirtioBlock`——v2 组件目录
  里 `VirtioMmioBlock` 是正名、`NativeVirtioBlock` 是 legacy 别名
  （runtime 反查恰好过校验），archive 行补旗标即通，产出
  `virtio-mmio-block-verified-bundle/libVirtioMmioBlock.a`（defconfig 期望名）。
  ② **Font.h 兜底**：build-boot-progress.py 的 `-I uefi-sheng/…` 在装配树
  里不存在（上游 sheng 也一样）；BootProgressDisplay.h 是引号包含 →
  Font.h（AOSP 字形表，3.7K）进 `package/` 同目录即解，本机/CI 通用。
  ③ **Adreno740 链摘除（板级拍板，独立主题待归）**：libAdreno740.a 需要
  板级本地 GPU 输入（`ShengGmu*Source.h` OPP/带宽政策——上游 gpu.md 明文
  "keep with the board"，不进 git；+ `.cache/firmware/sheng-gpu` 三件钉哈希
  固件），P4 门（UEFI→内核→launchd）不需要 GPU。`CONFIG_XNU_NATIVE_ADRENO=n`
  + 板 archives/components.include 三处摘除，注释带复原路径。
  ④ **BootProgress 全量档换 stub**：122 个遥测弱外部
  （armcpu_*/boot_*/native_ufs_*/native_adreno_* 全家桶）靠 Apple ld
  "弱未定义置零"语义才能链过，本机 ld64 (cctools) 报错——上游 sheng 的
  CI（macOS）可行、本机不可行。按 bringup 板先例（同一个 ld64 弱引用墙）
  换 `build-boot-progress-stub.py`（no-op boot_progress_poll，喂
  ShengUfs/NativeUfs.cpp:44 的弱引用）；`CONFIG_XNU_BOOT_PROGRESS=n`
  （boot_progress.h 的调用点在 =n 下宏化为空操作，不产生强引用）+
  `XNU_EXTRA_LD_ARCHIVES` force_load stub archive（对齐 bringup/旧 owner
  defconfig 既有模式）。全量档随"进度 UI 阶段"主题回归。
  ⑤ **verified-root 证据链**：r30 双卷政策（verified-root-os/systemos）
  从 device-bringup 仓硬链进现设备仓 `.cache/firmware/`（连同 target-volumes
  两件 dmg + kernelcache）；fork 的 if-evidenced 脚本补 staging 步
  （docstring 约定 = 设备仓 .cache/firmware，树经符号链接可见，v2 迁移时
  只拷了脚本没同步逻辑）；ci-owner-policy.sh 收尾把政策 + dmg
  镜像进 `$DEVICE_DIR/.cache/firmware/`（CI 同构）。
  ⑥ **链接交叉审计 + bootstrap 门**：`build-storage-transport-record.py`
  补审计对象别名（审计按 archive 名推对象名 `libVirtioMmioBlock.a →
  VirtioMmioBlock.o`，构建落的是 runtime 名 `NativeVirtioBlock.o`，
  硬链接补位）；驱动 bootstrap 门加"startup.o 在场"检查（baseline 的
  trust-cache-mode 翻转会删 startup.o，中断的 pass 留下 .json 记录没有 .o
  → archives 饿死循环）；**改 defconfig 后必须手动重跑 `./unixnu
  defconfig`**（§0.13 坑二度踩实：记忆名相同则 .config 不再生）。
- **CI 重发**：`m2381 = cca8148`（fast-forward）、`device-m2381 = 27a818f`
  （orphan 重建强推）均已推；dispatch macos.yml = **run `40235412915`**
  （steps="kernel userdata payload"，pre_build=`bash
  "$DEVICE_DIR/scripts/ci-owner-policy.sh"`，xcode 26.6/macos-26，
  upload_userdata + upload_verified_root = true，upload_kernel = false）。
  取件面 = userdata 件 + verified-root 证据（内含 owner-data-fixture）+
  日志；payload 本身不留 runner（红线 7），闪光件走本机链
  （kernel ee3255a8 → 探针 → payload）。
- **注意**：本会话预演中 GPU 链与 BootProgress 全量档的摘除是**板级决策**
  （用户"你来拍板"授权延伸；依据写入板注释与 gpu.md）。上游无先例冲突：
  sheng 的等价物靠其本地件+Apple ld，我们不追。
- **未刷机**。

## 0.16 2026-10-26 续二：CI v2 车道全绿（run 40237299244）+ 取件落地 + 旧件清理

- **run `40235412915` 复盘（失败两步）**：①payload 步 = 探针补丁要 uefi 树的
  Font.h（UNIXNU_UEFI/本机 uefi-sheng 链接），runner 均无 →
  `patch-xnu-entry-probe.py` 加设备仓 `package/Font.h` 回退 +
  payload 驱动传 `UNIXNU_DEVICE`（本地/CI 统一；本地 payload 彩排 EXIT=0：
  探针+ADT+96MiB payload.img 全出）；②userdata 上传 glob 还是 v1 深度
  `out/*/userdata/**` → v2 布局改 `out/**/`（unlinked-kernel glob 同修）。
  修复 = commit `e36372f`（仅 checkout 文件，device 分支不动）。
- **run `40237299244` = success**：kernel ✓ / userdata ✓ / payload ✓ /
  userdata 上传 ✓ / verified-root 证据 ✓——**v2 CI 车道全链绿**（commit
  `e36372f` + device `27a818f`）。
- **取件落地（temp/ 用户下载三件 → 已入位）**：
  - userdata 8.74GiB → `out/m2381/ci-artifacts/userdata-root.img`，sha
    `fd43ee70…` 与 **10-25 已刷 r30 件逐字节一致**——userdata 构建确定性
    实证，**设备现役 userdata 即正确版本，首刷无需重刷 userdata**（manifest：
    OS `e5634d04…`@0x100000 / SystemOS `b4908cde…`@0x16a600000）。
  - verified-root 证据（双卷政策与 r30 在仓份逐字节一致；fixture 跨 run
    非确定——随机卷标识+时间戳，APSB 钉 18AB5557 由 pre_build post-patch
    检查把关）→ run 2 份已更新入
    `devices/…/.cache/firmware/owner-data-fixture/`；完整留档
    `out/m2381/ci-artifacts/{verified-root-1,verified-root-2?,logs-2}`。
  - 本机闪光 payload 配方：`UNIXNU_UNIQUE_CHIP_ID=3645ad55d47e77c1
    ./unixnu build payload -D devices/m2381/unixnu-m2381 --data-ramdisk
    devices/m2381/unixnu-m2381/.cache/firmware/owner-data-fixture/scratch-data.img`
    （探针自动打；无需 --kernel，吃本机内核 ee3255a8 状态）。
- **清理（用户拍板"删旧无用文件、不需要回滚件"）**：`artifacts/images/`
  全删（旧回滚件三件 + r30 已刷 userdata 参考件——新 CI 件逐字节复现、
  旧 5G artifact zip），14G 释放；`artifacts/` 只剩 README + vendor
  （OEM UFSDxe 退路，非回滚件，保留）。回滚链只在 nura 仓 artifacts
  （mu-r62…r91、esp v46+）。temp/ 三 zip 解压入位后删除。
- **未刷机**；下一步 = §7.2 本机出闪光 payload → 桌面校验 → 首刷拍板。

## 0.17 2026-10-26 续三：首刷 + UEFI 真机首通 + 冻屏诊断循环（进行中）

- **首刷（用户 fastboot 拍板）**：boot_b = boot.img `40f2a199`（P2 确定性
  复现）+ recovery_a = payload.img 96MiB（内核 ee3255a8+探针、trustcache
  `7416ca4f`、Data 种子 `67a1e4f2`、ADT 13820B）。**首轮出件漏
  --trust-cache 被桌面校验拦下**（v2 不自动带信任缓存，实证配方必须
  `--trust-cache out/m2381/trust/original-trust.bundle`）；userdata 未刷
  （fd43ee70 在设备 = CI 件逐字节一致）。fastboot 前置全过（slot=b、
  96MiB/100MiB）；刷后无 dd 通道（无 shell），本轮证据 = fastboot OKAY +
  桌面哈希。
- **读图 = 重大里程碑 + 新墙**：ABL→BootShim→**UEFI 真机首通**（m2381Pkg
  第一次上硬件）——DXE 跑起来了，kalama 平台驱动（ScmDxeLA_Kailua/Shmb/
  HALIOMMU/RPMh/Vcs/Clock/ICB…）全在加载，帧缓冲 DEBUG 控制台工作。**但
  DXE 中段后冻死**：屏面固定不动，无异常打印、无菜单（BDS→UniXNUDxe 未
  达）。未观察到 5 分钟看门狗复位。屏幕驱动顺序与 FV 清单不一致属 DXE
  依赖调度，正常；UFSDxe 是 RELEASE blob 永不打印（存在与否不代表执行）。
- **嫌疑面（驱动清单 diff）**：我们的 DXE 集与 nura 真机可用链（meizu20-
  mars-port）**只多 DarwinBoot**（DXE.inc 逐行 diff），加 FDF 尾部独有源码
  驱动（RamManagerDxe/Smbios/IortDtDxe/DtPlatformDxe/UniXNU 栈）——
  从未在此硬件跑过的就是它们；也可能是 SimpleFb 接管后显示通路断（冻屏≠
  冻系统）。
- **诊断轮（已刷，下会话读图）**：八探针版 boot_b = **`a1d51f04`**（fd
  `daad477f`；DEBUG_ERROR 直写帧缓冲，链接产物含标记已验证）：
  `M2381>> RamManager ENTRY/EXIT`、`SimpleFb ENTRY`、`IortDt ENTRY`、
  `DarwinBoot ENTRY/LOADER INSTALLED`、`UniXNUDxe ENTRY`、`UniXNUApp ENTRY`。
  探针源码：DarwinBoot.c/package/UniXNUPkg×2（工作区文件，已提交）+
  out/m2381/uefi 内 SiliciumPkg×2/QcomPkg×1（worktree 本地，gitignored，
  诊断后随 FV 恢复）。**下会话工作单**：读冻屏画面上最后一个 `M2381>>`
  标记 → 卡点 = 该标记之后的驱动段；六标记全齐仍无菜单 = BDS/菜单层问题
  （此时转 RELEASE 静默构建对比）。修复路径备选：换用 nura 链同款构建、
  逐个摘除独有驱动二分。
- 注意：boot.img 默认输出在设备仓 `build/m2381/boot/`（不是
  `out/m2381/boot/`，后者是 P2 旧件 40f2a199）。

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
| ~~`artifacts/images/{mu-r69…, esp-v71…, shim-boot…}`~~ | **已删除（2026-10-26 用户拍板）**：旧回滚件 + r30 已刷 userdata 参考件（新 CI 件 fd43ee70 逐字节复现，确定性实证）+ 旧 5G artifact zip；回滚链只在 nura 仓 artifacts |
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

## 7. 下一步（2026-10-26 彩排绿 + CI 已发后交接；10-25 刷机循环工作单见 §7.1）

0. ~~本机 Linux 出核彩排~~ **✅ 已完成（§0.15，六墙全修，kernel ee3255a8，
   commit cca8148）**。
1. ~~CI 重发~~ **✅ 已 dispatch：run `40235412915`**（steps="kernel userdata
   payload"，pre_build=ci-owner-policy，macos-26/xcode 26.6，
   upload_userdata+upload_verified_root=true）。守望 token /tmp/m2381-gh-token
   （/tmp 被清则 git credential store 重提取）。UEFI 侧未动，FV 不重建。
2. **~~CI 绿 → 取件~~ ✅（run 40237299244 全绿，取件完成见 §0.16）→
   P4 门**：本机链出闪光件：`./unixnu build payload --data-ramdisk
   …/owner-data-fixture/scratch-data.img`（吃内核 ee3255a8 状态，探针自动
   打）→ `package-m2381-boot.py` 出 boot_b 件 → 桌面校验 → **首刷
   boot_b + recovery_a 单独拍板**（FLASHING.md 纪律，刷前读回对拍；
   **userdata 不重刷**——设备现役 fd43ee70 与 CI 件逐字节一致）。
   读图预期（§7.1 分支①②③全部沿用以新内核重验）。
3. **新内核行为差留眼**：`NATIVE_LEGACY_DISPLAY=n`（本就跑 UEFI GOP
   offline-legacy-*，理论无感，实证为准）；GPU 链与 BootProgress 全量档
   本轮摘除（§0.15③④）——显示/触摸 P4 门后按主题回归；10-25 旧墙
   ram-data-first-abort 在新内核上重验。
4. **运维坑更新**：v2 树装配把 §7.2 的"三处 cp 影子覆盖"坑基本退役——
   scripts 每条命令重新拷贝、drivers/kits 是符号链接（设备 fork 直连）；
   仅 `make-device-branch` 需先删旧分支。
5. **独立主题**（不阻塞）：XS/fileset-KC rebinder（kits/XnuPrelink 内已带
   test_fileset 与 rebinder 体）；arm64e 专项；LEGACY_DISPLAY 证据阶段。

### 7.1 2026-10-25 刷机中交接（循环节奏由新内核重开后继承）

- **读图三分支（用户正在双刷：userdata=已下载 9.4GiB 双窗镜像 +
   recovery_a = `7a40b90b`，内核 `9e72b039`/探针件 `f7a2819c`，16K 验证块）**：
   ① **恢复界面 / Apple logo / 用户态画面 = stage 7 里程碑** → 下一站
     **Data 种子**：OWNER_DATA_MOUNT=y（defconfig 的 UUID 18AB5557 已钉）
     需要 RAM 种子（prepare-owner-data-fixture 需 macOS 挂载 → CI pre_build
     候选）+ BOOT_MANIFEST 已开；然后显示/触摸 P4 门；
   ② **SystemOS/union 挂载类新 panic = 取证循环**：panic 屏会说话 +
     `native_owner_cache_stage` 1-5 检查点 + owner cache checkpoint 全在；
     已实证 = cache volume UUID 8846FE78 命中、MNT_UNION 叠挂 "/"、
     native_union 三条件（vnode_mountedhere/rootvnode 互换）待机验证；
   ③ **"no dyld cache" 仍现** = union 挂上了但 dyld 路径没接通——查
     CACHE_STARTUP 的 dyld_flags bit3（免点火）与 cryptex 缓存路径；
   ⚠ **校验期静默比 r21-22 更长**（16K = 4× 读放大）属预期，等 10-20 分钟
     再判；蓝条只闪一次（stage 1）。
1. **git 收编（✅ 已完成：10-26 轮推至 63d2857，origin/main 已删；后续推送
   走新 §7.1/§7 工作单）**。原计划：`git fetch origin &&
   git push --force-with-lease origin m2381`（远端 aaaa2174 = API 重放的
   本地同内容提交，本地链多 5 个提交，强制安全）+ 重新生成 device 分支并
   强推（先 `git branch -D device-m2381`）。推完删 /tmp/m2381-gh-token。
2. **运维坑清单（已入册，复发先查）**：改 repo scripts **或 drivers** 后必须
   cp 到 tree + device-bringup + devices 三处（树是拷贝且 device 层后拷会
   影子覆盖；r29 = §Z RootWindowRead v2 漏拷导致 16K 读路径必 VR_BLOCK，
   见 §0.11）；make-device-branch 二跑前
   `git branch -D device-m2381`；pre_build 路径必须
   `bash "$DEVICE_DIR/scripts/…"`；github.com git 端点不通时用
   Git Data API 重放（/tmp/api_push.py，token /tmp/m2381-gh-token）。
3. **独立主题**（不阻塞设备循环）：格式 2 迁移（✅ 已于 2026-10-26 完成，
   见 §0.14）；上游 rebase 跟随（现 pin 16e592a）；XS/fileset-KC rebinder；
   arm64e 专项；r21 蓝条来源考古。
4. **欠账**：recovery_a dd 读回对拍（UniXNU 侧自读或豁免）；16K 读放大
   的性能优化（已验块缓存）= launchd 之后主题。

## 0.15 2026-10-26 续：watchdogd 墙破局 = XNU_BOOT_PROGRESS 转正（详见 commit）

- **实机读图（kernel `ee3255a8` + payload `8e20dc3e`）**：绿→蓝→品红后内核全链
  跑通——VM init、root 认证、launchd exec、dyld 吃 cryptex、**RAM Data 种子
  生效（tzinit 过墙，r25 的 pid17 死点清除）**、daemon 到 pid 173。墙 =
  `watchdogd[173] exited -- exit reason namespace 2 subcode 0x5`（SIGTRAP =
  watchdogd 找不到 IOWatchdog 服务的故意自杀；它是 launchd 无条件
  _PanicOnCrash 任务 → launchd panic 内核 → panic 屏打 frames/pages/free）。
  同墙 sweet 时代 10-20 轮 18 已撞（archive/NOTES-m2381.md:1326）。
- **根因 = boot-arg 在、代码不在**：payload args 含
  `sheng-disable-watchdogd=1`，但内核是 `CONFIG_XNU_BOOT_PROGRESS=n`+stub
  构建（第 35 轮 ld64 弱外部墙的折衷）→ 上游 kern_exec.c:8039 的
  `sheng_boot_watchdog_policy` 挂点编译消失 → disabled.plist 从未写出 →
  flag 死脚。官方解法（fuxi/sheng 生产同款）= flag + BootWatchdogPolicy.c。
- **修复**：`CONFIG_XNU_BOOT_PROGRESS=y`（base+owner defconfig）+ 真档回归
  （baseline force_load libBootProgress.a）+ 新
  `build-boot-progress-telemetry.py`（package/BootProgressTelemetryDefs.c =
  86 个遥测零值定义，覆盖 legacy-display/touch/Adreno/cpufreq/Wi-Fi 全部
  悬空弱引用——Apple ld 置零语义的显式化，ld64 956 因此过链）。stub 退役
  （board.toml 换条目；board.bringup.toml 仍用 stub 走 =n 车道）。
- **产出**：内核 **`e7442ae1`** → 探针+panic screen 件 `d8d18275` →
  **payload `37ff3c16`**（96MiB；args 与 8e20dc3e 逐字一致、fixture
  67a1e4f2、trust 7416ca4f 同件）。链接+prelink 一次过。
- **读图（下次启动）**：BootProgress 诊断页将首次激活（boot-progress=1）；
  Data 挂载后应见 `sheng bring-up watchdogd launch policy: 0`；watchdogd
  不再启动。下一墙候选 = 剩余 _PanicOnCrash 名单（containermanagerd/
  usermanagerd/lockdownd/remoted/trustd/remotepairingdeviced）→
  sheng-disable-jobs 扩名单挡；或 Preferences/SpringBoard 里程碑
  （boot-ui-focus3/4 已排）。

## 0.16 2026-10-26 续二：37ff3c16 实机 = BootProgress 页首启 + daemon 崩溃循环 → sheng-lite 对齐

- **实机读图（payload `37ff3c16`）**：watchdogd 墙确破——诊断页首启（蓝带
  "live XNU - native 4K - no automatic reboot"），8 核全上（c1-c7=step7）、
  UFS 363716 块零失败、RAMDISK Data 种子活（sparse=1 resident 572KiB）、
  launchd 持续派生（pid 187→252、exits 298→392/39s）。红带
  "OBSERVATION LIMIT" = 页面免责声明非故障。**墙 = 崩溃循环**：
  `assistantd` SIGABRT（ns2 sub6，两代）、`findmydeviced` SIGABRT、
  `biomed` exit=100，未到 UI 里程碑（focus3 Preferences / focus4
  SpringBoard 未现）。
- **对齐**：sheng 生产板在此阶段跑 `sheng-lite=1`（21 个已知崩溃/XPC 等待
  环作业整批禁用；本板上一轮才首次编入 BootWatchdogPolicy，此机制当时
  才真正生效）+ `boot-progress=2`（本板注释自计划 stage 7 后 bump）。
  m2381 同步：disable-jobs 四条 → sheng-lite=1。
- **产出**：内核不变（e7442ae1/d8d18275）→ **payload `61ef9181`**。
  读图预期：FAIL 行显著减少；看 focus3/focus4 里程碑；新 crasher 报名即扩。

## 0.17 2026-10-26 续三：61ef9181 实机 = lite 生效，transparencyd 风暴 → 扩禁用名单

- **实机读图（82s→404s 四张）**：lite 名单生效（findmydeviced/biomed 行消失，
  仅残留 assistantd SIGABRT）。新墙 = **transparencyd exit=100 无限重生**
  （pid 219→709，140s→404s 退出 474→1255 ≈ 3 次/秒）——透明度数据仓库在
  32MiB RAM 种子上不存在，初始化即退；launchd 引导队列被钉死，UI 里程碑
  不至。顶栏转 "LIVE: DEADLINE EXCEEDED - recording continue"（连续记录
  模式，设计内）。系统体征全健康：UFS 1526MiB 零失败、8 核 step7、
  无 panic、lsd（LaunchServices）在跑（p208/p235 采样）。
- **对齐**：sheng/fuxi 无 transparencyd 先例（上游 grep 零命中）→ 按本板
  已验证的 disabled.plist 机制扩名单（与 lite 叠加）：
  `sheng-disable-jobs=com.apple.transparencyd,com.apple.assistantd,com.apple.biomed`。
- **产出**：内核不变 → **payload `d8792315`**。读图：FAIL 行应骤减；盯
  focus3/focus4 与面板接管。旁注：MEM totalMiB=4054 = 内核现只见低段
  ~4GB（高段 13GB 映射 = 独立后续主题，非本墙）。

## 0.18 2026-10-26 续五：d0ed4e45 实机读图 + 用户关机收尾（下会话主查队列步速）

- **实机（526s 单张）**：transparencyd/assistantd/biomed 行全部消失 ✓，
  风暴确认停止：EXIT 591（上轮同期 404s 已 1255）、CPU 转入 park/idle、
  中断率 26k→8k/s。残留循环 = `callservicesd`/`nsurlsessiond`/`analyticsd`
  三只（全 status=100 exit=1 秒退 = 无基带/无网络受害者，fail-fast，
  不堵队列）。stage target = analyticsd p370，无 Preferences/SpringBoard。
- **用户观察与决定**：12+ 分钟无 UI 迹象、发热明显、主动关机。
  "能起来不会这么慢"的质疑成立待查：队列步速 ≈1 作业/秒（pid 370@526s），
  iOS 引导队列 ~500 项 → 按此步速 SpringBoard 需 15-30 分钟——需要证据
  判断这是"慢但通"还是"有半挂起作业在拖"。
- **下会话调查单（优先级序）**：
  ① 队列步速根因：每作业 ~1s 开销来自哪——AMFI 每 exec 签名校验成本 /
  UFS 页入流（verified=1514MiB 全是校验读即页入）/ xpcproxy 链 / 某类作业
  的 30s XPC 超时周期；对照上游 sheng/fuxi 到 SpringBoard 的实耗时间定
  "慢但通"还是"异常"；
  ② 挂起作业盲区：挂起不退出 → 永不出 FAIL 行；判据 = stage 行 pid/名字
  是否冻结（已教用户，未取得读数）；
  ③ 发热：cpufreq 组件未派生（sheng-cpufreq=3 无实现）疑定频最高档，
  DVFS 主题评估；
  ④ MEM totalMiB=4054（高段 13GB 未映射）与页入竞争（speculative
  ~1.2GB）。
- **设备状态**：用户已关机。boot_b 现役 UEFI 不变；recovery_a =
  `d0ed4e45`（七层参数）；userdata = v2 root 不变。commits 至
  dceeb44/r39 已提交未推送。

## 0.19 2026-10-26 续六：队列步速调查① = sheng-log UFS 日志接线 + 步速仪表（r41 件就绪）

- **本轮拍板依据（§0.18①）**：r40 诊断读图只有照片，census/阶段史没落盘。
  上游机制本就有 UFS 日志窗（`kits/sheng BootLog.c`，boot arg `sheng-log=1`：
  每 2s 把整页诊断文本 + kernel msgbuf + exec census（每进程名/起止数/首启
  秒）+ [profile] 采样写入 **userdata 尾部 32MiB**，slot 环 509 个 ≈17 分钟，
  header 最后写、关机不丢）——但 m2381 从未启用：①args 无该 arg；②BootLog.o
  根本没编（sheng 在 LEGACY_DISPLAY 档里编，本板 =n 车道）；③读回侧 sheng
  用 `fastboot boot` Linux reader，本板 ABL 裁 CmdBoot 不通 → **nura-min
  Linux 镜像 + NCM devsh dd**（用户确认不覆盖尾部）。
- **接线（一次主题三墙全修，全在本设备仓）**：
  ① `build-boot-progress.py` 增编 BootLog.o 进 libBootProgress.a（基线
    force_load 即入内核；必须用 **startup.o.json/osfmk 模板**——BootLog.c
    含 osfmk 私有头 kern/misc_protos.h，bsd 模板没有；同因移除
    TelemetryDefs.c 里的 no-op `boot_log_tick` 桩，否则重复符号）。
  ② TelemetryDefs.c 扩 ~90 个数据零定义 + 11 个 no-op 函数（BootLog.o 的
    weak 外部在 ld64 956 下必须显式定义；链接预检两轮：`native_lookup_*
    与 vfs_lookup.o 的 native-lookup-trace 补丁强定义重复 → 摘除；其余
    批量 grep 内核树零命中）。
  ③ BootLog.c fork 自带 bug：[swipe2] 行读 `native_swipe_stats[16]` 越界
    [16] 数组（-Warray-bounds 抓住）→ extern/零定义同步扩 [17]。
- **步速仪表（BootProgress.c，照片即可判读）**：第 1 行加
  `spawn=<总 spawn> <上一整秒速率>/s`（census_note 锁外原子计数）；第 5 行
  stage 加 `+<age>s`（history 每次调用都推入 → age = 距上次 spawn 尝试的
  秒数——**挂起作业盲区判据**：age 持续增长而 UFS/IRQ 健康 = 有 spawn 卡死，
  挂起作业永不出 FAIL 行，此为唯一可见处）。
- **读回流程（下轮实机用）**：nura-min 起 Linux → NCM devsh
  `dd if=/dev/block/sda22 skip=$((分区扇区-65536)) count=65536`（userdata
  尾 32MiB）→ devpull → `devices/m2381/unixnu-m2381/scripts/
  read-m2381-ufs-log.py <img>`（--all 默认按 #slot 轮次排序；--page 只出
  pacing 时间线；--spawn 只出 census；--msg 出内核缓冲）。布局解析器已用
  合成窗验证。
- **机制线（离线已定位的部分）**：UFS 读路径 = 全局 windowLock 全程串行 +
  **单槽位**（RootCommands.h "Single-slot completion policy...No
  resubmission"）+ 门铃 2µs MMIO 忙轮询 + 每传输整段 cache 维护；每 4K 页入
  = 一次 16K 验证块往返。r40 数字自洽：1514MiB/16KiB ≈ 9.7 万次往返，若
  read_ticks ≈ 全程 510s → **~5.4ms/16K 往返 ≈ 3MB/s 有效带宽**，队列
  ~1 作业/s ≈ 每作业 2-5MB 独有页 ÷ 3MB/s——"页入带宽吃满"模型成立与否
  由日志第 46 行 `UFS TIME verify=ms read=ms`（ShengUfs 分立计数器已在）
  每 2s 落盘定案。优化杠杆（后续主题，按序）：多槽位排队、相邻页批读
  （bounce 128KiB 已支持 8×16K 批，随机页入吃不到）、已验块缓存复用、
  dyld 缓存驻留后段加速验证。
- **产出**：内核 **`430344b3`**（比 e7442ae1 大 ~139KB；内核含 [spawn]/
  [profile]/[page]/xpcproxy exits/swipe2 字面量实证 BootLog 在内；诊断页第 1
  行末尾新增 `log=<state> r<rounds> e<errors>`——BootLog.o 常驻档案后
  plain extern 直连写手状态，照片即可证明日志窗在填：0=off 1=等窗
  2=writing 3=错误停 4=GPT 变更停）→ 探针+panic 屏件 **`bf32d8fa`** →
  **payload `f264c196`**（96MiB；args 与 r40 逐字一致 + sheng-log=1；trust
  7416ca4f / Data 种子 67a1e4f2 同件）。桌面校验：ESP kernel/种子 sha 逐字节
  对拍 ✓、ADT 13820B ✓、unixnu.toml 含 sheng-log=1 ✓。
  （构建中途曾出 b2681da3/b14525da 无上屏态版本，已被本组取代。）
- **待用户**：按 §0.20 工作单执行（刷 recovery_a → 跑 ≥15 分钟 → 刷
  nura 链 + nura-min → dd 读回日志窗）。

## 0.20 2026-10-26 续七：nura-min 读回工作单（r41 件 → 日志窗取证 → 恢复）

**背景**：nura-min = nura 仓的 2GiB 定长 ext4 根镜像，为"iOS 写 userdata
尾部日志 ↔ Linux 读回"设计（`nura-meizu-m2381/docs/NURA-MIN-ROOTFS.md`，
本机 serial 381QYGEP222XJ 已验收：48s 起 devsh:23、NCM 172.16.42.1、
sparse 只写头部 2GiB 物理碰不到尾部）。它替换 userdata 头部（iOS root 的
OS 窗被覆盖 = 预期代价，红线 2 豁免，读完刷回 8.74GiB 件即可）；尾部 32MiB
sheng-log 窗与 Data store（尾 1GiB reserve 内）原样保留。

**用户操作序列（本轮刷机窗口）**：
1. 刷 recovery_a ← **`f264c196`**（96MiB，本机
   `out/m2381/ipados-17.7.11-21H461/m2381/payload/payload.img`）。
2. 起 UniXNU **跑 15-18 分钟**（509 slot×2s ≈ 17 分钟全保留；更久则最早的
   几分钟被环回覆盖，无碍主判读）。期间可拍 1-2 张照片：第 1 行看
   `spawn=<n> <r>/s log=2 r<n> e0`（log=2 r 增长 = 日志在写）、第 5 行看
   `stage +<age>s`。到点长电源键关机。
3. 刷 nura 链三件（nura 仓 `/home/ri/work/nura-meizu-m2381/artifacts/`，
   刷前各自 .sha256 对拍；fastboot 前置照 FLASHING.md，slot=b）：
   `boot_b ← mu-r91-350386e0.img`、`recovery_a ← esp-recovery-v71.img`、
   `userdata ← meizu-meizu20-nura-min-r91-2G.simg`。
4. 起 nura-min（USB 出 NCM 网卡，设备侧 172.16.42.1）→ 宿主 nura 仓
   `tools/devsh.py`（busybox nc :23，自动挑 172.16.42.x 接口）：
   - 写窗到设备 /tmp：`tools/devsh.py 'dd if=/dev/sda22 bs=512
     skip=$((972326040-65536)) count=65536 of=/tmp/logwindow.img'`
     （32MiB = 尾 65536 扇区，分区相对寻址）
   - 刷写质量读回（顺带清 §7.1 对拍欠账的可行部分）：
     `tools/devsh.py 'dd if=/dev/sde43 of=/tmp/boot_b.now.img'` 与
     `dd if=/dev/sde27 of=/tmp/rec_a.now.img` → 对拍 mu-r91/esp-v71 哈希
   - 推回宿主：一个终端 `tools/devpull.py logwindow.img`（监听 5555），
     另一个 `tools/devsh.py 'busybox nc 172.16.42.2 5555 <
     /tmp/logwindow.img'`；boot_b/rec_a 同法。
5. 恢复现役（读完即可刷回）：`boot_b ← devices/m2381/unixnu-m2381/build/
   m2381/boot/boot.img`（a1d51f04 现役留底）、`recovery_a ← f264c196`、
   `userdata ← out/m2381/ci-artifacts/userdata-root.img`（fd43ee70，
   8.74GiB，本地在）。

**下会话分析单（我）**：
1. `python3 devices/m2381/unixnu-m2381/scripts/read-m2381-ufs-log.py
   logwindow.img --page > pacing.txt`；再加 `--spawn`（census：每进程名
   starts/exits/首启秒）与 `--msg`（内核缓冲）。
2. **判读决策树**：
   a. `UFS TIME read=Xms` 行序列：read_ms ÷ verified_blocks（16KiB 块数）
      = ms/往返。≈5ms → "页入带宽吃满"模型坐实（r40 自洽值 5.4ms）→
      UFS 读路径优化主题排期（多槽位排队 / 相邻页批读（128KiB bounce 已
      支持随机页入吃不到）/ 已验块缓存 / dyld 驻留后段加速）。
   b. spawn 速率序列（相邻 slot 的 spawn 总数差分）：恒 ~1/s = **慢但通**
      （再跑一轮更久即可等 SpringBoard，或直接做 a 的优化）；平台期
      （stage +age 几十秒不落）= **半挂起**，stage 行 + census 给出卡死
      作业名 → 定向 disable 或修因；对照 xpc_fails 段（spawn setup 失败
      的 label 计数）。
   c. [profile] 采样排名：read 时间不高时的 CPU 烧点（AMFI/launchd/
      xpcproxy/其他）→ 转 AMFI 每 exec 成本线；采样密度同时给发热线
      （③）备料（cpufreq 缺席定频评估）。
   d. [page] 的 `free=` 序列：4GB 窗内存压力证据（④，speculative 回收）。
   e. `log=` 行与 header：state 全程 2、e=0 → 写侧健康；state 3/4 →
      写错误/GPT 异常另查（write_errors>0 时看 read_ms 是否被日志写挤占）。
3. 收尾：STATE §0.21 读图记录 + 记忆更新；按判读结果开下一主题
   （UFS 读路径优化 / 定向 disable / 长 boot 验证 SpringBoard）。

**已知残余风险（读图前如实记录）**：写侧证据链 = 编译+链接+字符串+args+
几何全过，唯一未实证的是运行期写手（writer 线程活着且 hostWrite4k 成功）
——上屏 `log=2 r<n> e0` 就是它的运行期证据；若照片显示 log=1（等窗）
说明 hostLogOpen 被拒，读 `native-ufs-root` 的窗口拒绝原因另行排查。

## 0.21 2026-10-26 续八：r41a（b14525da）实机首启读图 = policy panic @kern_exec.c:8043（下会话先读日志再刷机）

- **照片读图（09:44，b14525da 已刷已启）**：内核走到 1s 后死——stage 史六行
  全部停在第 1 秒（init_program enter → SystemOS mount ×2 → Data mount ×2
  → `temporary watchdog launch policy enter`），**stage +355s 无更新、
  spawn=0、procs=1、EXIT 0**；panic 大字 `@kern_exec.c:8043`（消息头——
  含 errno 的 `sheng watchdog launch policy failed: %d`——被诊断页重绘
  盖住）；可见 frames 尾部 = `fffffff0088c89b0` + `fffffff0085a5838`。
- **符号化（本机 unstripped 内核表；kern_exec 段在 r41a/r41b 两构建间不变，
  符号化有效）**：8043 = `panic("sheng watchdog launch policy failed: %d")`
  即 BootWatchdogPolicy 的 disabled.plist 写链条返回非零（源码逐步：
  mkdir→open(O_CREAT|O_TRUNC|O_NOFOLLOW)→分片 write→fchown/fchmod/fsync→
  fstat 校验→O_EXCL marker）；`fffffff0088c89b0 = bsd_ast+0x328`、
  `fffffff0085a5838 = thread_bootstrap_return+4`——**正常 initproc 启动链**
  （bootstrap 线程 → bsd_ast → load_init_program → policy），无异线程失控。
  policy 在 1s 内快速失败（同秒挂）→ 立即 errno，非超时。
- **头号假设 = 撕裂的持久化 Data store**：r40 = 中途断电关机（12min 等待后
  用户关机），sheng-data-persist=2 的 store 被撕；本次 resume（同参数同位置）
  挂上损坏/只读卷 → policy 写 disabled.plist 立即 EROFS/EIO。旁证：NativeUfs
  ext 行注释的状态表正好有 "6 = resumed read-only"；照片里 DATA st= 恰被
  panic 大字盖住；r38→r40 三轮都是"拍照→断电"循环，撕裂迟早发生。**我的
  r41 delta 与 sheng-log=1 不能完全排除（写手线程与根窗口并发用单槽传输），
  以日志 errno 定谳。**
- **关键资产判断：这次 panic 的证据已在设备上**——panic 未停其他核
  （quiesce=0/halt=0，monitor 画到 359s），BootLog 写手线程大概率每 2s 一轮
  写了 ~180 轮：`[panic]` 节有完整 panic 文本（含 errno + 24 帧），
  `[data]` 行有 DATA st=（撕裂判据），msgbuf 有 policy 的 printf 与 APFS/
  persist 错误行，`[page]` 有 359s 的全页序列。**userdata 尾部在 nura-min
  读回前不可被任何 fastboot flash userdata 触碰。**
- **修订后的下会话序列**：①nura-min 读回日志窗（§0.20 步骤不变，先不刷任何
  UniXNU 件）→ parser 出 [panic]/[data]/[page]；②按 errno 定谳后刷
  **`3bafb6b9`**（= f264c196 + `sheng-data-reset=1`，清持久化 Data 回新鲜
  种子；本机 `out/m2381/payload/payload-data-reset-1.img`）→ 若假设成立，
  policy 一次过、launchd 继续（同时它带全套 r41 仪表与 sheng-log=1，日志
  连续）；干净 f264c196 留档 `payload-clean-f264c196.img`。③若 errno 指向
  别处（非 Data 卷），按 errno 修因重出件。
- **本会话产出补记**：recovery 件 `3bafb6b9`；panic 地址符号化方法入册
  （llvm-nm + unstripped noctf，跨小构建对 kern_exec 段有效）。

## 0.22 2026-10-26 续九：日志窗读回定谳 = ENOSPC（Data 卷满），撕裂假设死亡；刷 3bafb6b9 复位变体

- **读回（nura-min + NCM，全程 <1 分钟）**：32MiB 窗 dd 回 `temp/logwindow.img`
  （md5 e01d352a 设备/宿主一致，`SHLOG001` 头在）；顺带 dd boot_b/recovery_a
  对拍 = mu-r91 首 1.1MiB、esp-v71 全 100MiB **逐字节一致**（nura 链刷写质量
  过）。解析件 `temp/r41a-log.txt`（41k 行）+ `temp/r41a-msg.txt`。
- **三读定谳**：
  1. `[panic]` errno = **28 = ENOSPC**（非 EROFS/30）——撕裂假设死亡；
  2. `[data]` `D st=2` = resumed **正常**，`wr=12 cm=5 er=0`——持久化机制
     零错误，把 r40 的满卷状态完好带回；`S st=1 dirs=0 files=0 fail=1 err=28`
     （staged-apps 首个 mkdir 就 ENOSPC，什么都没写成）；
  3. 写手运行期实证：state=2、write_errors=0、长引导 447 轮覆盖 ~894s
     （≈15min）+ 最后一次引导 25 轮 48s（用户重启复测第二轮，其 25 槽覆盖
     环头——槽位 25/26 边界 IRQ 31932→31415 倒退/free +4 = 两引导接缝，
     头部 rounds=25 只是末次引导收尾，写手全程健康）。
- **死因链（msgbuf 铁证）**：`disk1s1 OwnerData` = **32MiB** APFS 卷
  （8192×4K，正对 D 行 pages=8192 的 RamBlock 窗口）；挂载即
  `smfree 209/8192`（**95.54% 满**）+ lowdisk notification；policy 的
  disabled.plist `O_TRUNC` 收缩事务被拒（`fs_tx_enter: ENOSPC num_blocks 127
  free_blocks 209 threshold_blocks 100` → `cannot enter tx size 3 to shrink
  ino 506 to 0; 28`）→ panic。r40 的 12 分钟 daemon 运行填满了 32MiB 卷，
  断电持久化把满卷带回，r41a 两次引导都在第一笔写上死。
- **修复（工作单第 3 步，假设"成立"分支）**：**`sheng-data-reset=1`** =
  RamBlock 丢弃持久化页不应用（`drivers/RamBlock/NativeRamBlock.cpp`
  `discardRequested()`，当前内核实证在）→ Data 回新鲜种子 → policy 应过。
  **为什么必须用内核内复位而不是重刷 userdata**：D-store 在分区尾
  （绝对 4K LBA `0x75dfff9` ≈ 471GiB 处），12GB userdata 件只写分区头部，
  **重刷 userdata 不清 D-store**。
- **用户刷写卡（单通道，slot=b，先 getvar 对拍）**：
  1. `fastboot getvar current-slot` 必须 = b
  2. `fastboot flash boot_b devices/m2381/unixnu-m2381/build/m2381/boot/boot.img`
     （a1d51f04；nura 链把 boot_b 换成了 mu-r91，iOS UEFI 需回装）
  3. `fastboot flash recovery_a out/m2381/payload/payload-data-reset-1.img`
     （3bafb6b9；已验证参数行含 `sheng-data-reset=1`）
  4. `fastboot flash userdata out/m2381/ci-artifacts/userdata-root.img`
     （fd43ee70；现 userdata = nura-min Linux root，必须换回 iOS root）
  起机后 ~2-3min 拍照：期望 stage 走过 `temporary watchdog launch policy
  ready`、spawn= 开始爬、页首行 `log=2 r<N> e0`。
- **遗留结构项（观测窗口受限）**：32MiB Data 卷会被再次填满（r40 用了
  ~12min），且 32MiB 本身在 APFS lowdisk 阈值之下（挂载即告警，daemon 从
  第一秒就看到 lowdisk）——队列步速观测是限时窗口；扩容 = OwnerData
  fixture 卷 + RamBlock pages=8192 双侧改，列为 r41 观测后的下一个主题。
- 回滚件不变：boot_b←a1d51f04（本卡第 2 步即回装）、userdata←fd43ee70
  （第 4 步即回装）；r41a 原始证据 = `temp/logwindow.img` +
  `temp/r41a-log.txt`（本地，不入 git）。

## 0.23 2026-10-26 续十：r42（3bafb6b9）实机 = 复位生效仍 ENOSPC；真凶 = staged-apps 灌满 32MiB 种子；修复 = 上游 grow_gib 种子扩容

- **r42 读回（logwindow-r42.img，单次引导 359 轮/716s，写手 state=2 e=0）**：
  复位确实生效——`D st=3 ld=1`（持久化页被丢弃，vs r41a ld=40），
  `wr=9069 cm=1017`（新卷被重写）；挂载时 `smfree 7827/8192`（**95.5% 空**，
  种子出厂不满）。**0.4 秒后** `gettin' low: free 835→579→323` → policy 仍
  errno 28。真凶 = **`S st=1 dirs=482 files=165 mib=31 fail=39 err=28`**：
  r39 预埋的 `sheng-staged-apps=1` 首 && 每）次引导把 OS 卷 staged 系统应用
  拷进 Data（拷了 31MiB 自己 ENOSPC，拷不完→无 marker→下轮重来），32MiB
  卷被灌满后 policy 的 disabled.plist 写死。r41a 的"满卷"也是它 r40 塞的。
- **上游复核（用户拍板"按上游修"）**：sheng board.toml:189 同样开着
  `sheng-staged-apps=1`——上游配套件 = **`scripts/seed-data-grow.py`**
  （"make a Data seed whose APFS container fills the tablet's storage"）+
  workflow `grow_gib` 输入（`grow_gib needs data_seed_url`，CI 流：
  data_seed_url→RUNNER_TEMP/data.dbr→grow→payload）。**我们的错 = 只抄了
  args 一半，种子从未扩容（r30/stage-8 的 32MiB 用到现在）**。种子几何在
  镜像里、内核自适应（"refuses a seed whose store window does not fit"），
  稀疏打包/稀疏 RAM（现 resident 仅 572KiB）→ **扩容不用改内核不用改板**。
  32MiB 在 APFS lowdisk 阈值下（挂载即告警）也是 r40 异常行为的候选因。
- **已执行**：m2381 → origin（b5ae508..d4a3d09，CI 出核必须带 r41 BootLog
  接线）；device-m2381 重建推送（27a818f→5bd5ffa，含 staged-apps/sheng-log，
  无 reset 残留）。ESP 配置核实：r42 的 reset 在主 xnu 项末尾（board.toml
  本身干净，下轮 CI 件不带 reset——种子换身份自然回新鲜卷）。
- **下一步（用户 dispatch）**：沿用上轮 payload dispatch 命令 + **`-f
  grow_gib=16`**，steps="kernel payload"（userdata 不动），data_seed_url/
  trust_cache_url 沿用私有件。取件后**只刷 recovery_a**（boot_b/userdata
  现役即 a1d51f04/fd43ee70 不动）→ 起机预期：Data 新鲜 16GiB → staged-apps
  拷完写 marker（一次性）→ policy 过 → spawn 爬升 → 日志窗采步速。
  值选取：16GiB = staged-apps 全树 + daemon 状态千倍余量；upstream --auto
  （~453GiB 填满分区）留给日常可用阶段再升。
- 残余观察项：本轮若 queue 仍 ~1 作业/s，步速真因在队列调度（§0.18① 继续），
  与卷无关；r41b 仪表（UFS TIME verify=40ms read=253ms/2s 轮）已可定量。
- **执行补记（会话内闭环）**：发现 payload 件按红线 7 永不上传 → `grow_gib`
  路线取不回种子，改为 **pre_build 内 grow**（ci-owner-policy.sh 打包后调
  上游 `seed-data-grow.py --gib 16`，grown 种子随 verified-root 件回家，
  `build/owner-data-fixture/**` 本在 upload 清单）；m2381=3fd2e56、
  device-m2381=f7a2dd2 已推。run **40316416973**（steps="",13m57s 全绿）
  产 grown 种子：logical **16.00GiB**、卷 UUID 18AB5557 保持、身份
  178b461d（换 = 首启新鲜卷）、pack 1,032,320B；取件
  `out/m2381/ci-artifacts/verified-root-grow/owner-data-fixture/`。
  本机组 payload（配方 + `--trust-cache`，内核 430344b3、无 --skip-setup
  与上轮 parity）：**`payload-seed16g-764539c9.img`**（96MiB，
  ESP 内种子 md5 与 grown 件一致、kernel bf32d8fa 与 r42 件逐字节同、
  args 尾 `sheng-log=1 sheng-staged-apps=1` 无 reset）；设备仓 .cache
  种子已换 grown 现役。**用户卡：只刷 recovery_a ← payload-seed16g-764539c9**
  （boot_b a1d51f04 / userdata fd43ee70 不动）；起机预期：Data 新鲜
  16GiB → staged-apps 首拷完成（秒级，写 RAM 卷）→ policy 过 →
  `spawn=` 爬升；后续引导 marker 跳过拷贝。

## 0.24 2026-10-26 续十一：r43（764539c9）实机 = **policy 过墙**（kern_exec.c:8043 墙清除）；日志窗读回留下会话

- **用户实机报告**：三件全换回 iOS 链（boot_b a1d51f04 / recovery_a
  764539c9 / userdata fd43ee70）后，stage 走过
  `temporary watchdog launch policy ready`——**ENOSPC 链修复实机生效**，
  launchd 继续跑（r40 以来第一次活过 Data-policy 墙）；跑了 ~11 分钟，
  用户不拍照，断电后刷回 nura-min，读回留下会话。
- **时序窗口**：日志环 509 槽 × 2s ≈ 17 分钟回卷；r43 断电在 ~13-16 分钟
  = 全程落在环内（超 17min 只丢最前几轮，中后段仍可分析）。
- **下会话读回工作单（nura-min + NCM，<1 分钟）**：
  1. `cd /home/ri/work/nura-meizu-m2381 && python3 tools/devsh.py
     'dd if=/dev/sda22 bs=512 skip=$((972326040-65536)) count=65536
     of=/tmp/logwindow.img'` → `head -c8` 应见 SHLOG001；
  2. 宿主 `python3 tools/devpull.py temp/logwindow-r43.img`（后台）+
     `devsh 'busybox nc 172.16.42.2 5555 < /tmp/logwindow.img'`；md5 对拍；
  3. `python3 devices/m2381/unixnu-m2381/scripts/read-m2381-ufs-log.py
     temp/logwindow-r43.img --all > temp/r43-log.txt`；
  4. 分析单：header（rounds/uptime/e/state=2）；`--page` 时间线 = spawn
     速率、stage +age（挂起作业唯一可见处）、EXIT 计数；`--msg` 找
     `sheng bring-up watchdogd launch policy: 0` 与 staged-apps 的
     `S st=2`（拷完+marker）或 `st=3`；UFS TIME ms/轮 = 读路径基线
     （§0.18① 队列步速定量从这里开始）。
- **判读预期**：policy: 0 + S 完成后，若 spawn 持续爬 = 队列健康，等
  SpringBoard；若 ~1 作业/s 依旧 = 队列调度专项（§0.18①）开场。
- **读回定谳（当会话补完，logwindow-r43.img md5 ddb3df16）**：r43 单引导
  324 轮/648s（环未回卷；槽 324-446 = r42/r41a 残留，parser 按轮号排序
  需注意混层）。**队列首次活**：spawn 0→**995**（11min）、常驻
  **procs=181**、速率 2-4/s 带 36/s 突发（r40 的 ~1/s × 4 以上）、断电
  瞬间队列头 = `target pid=272 backupd exec process signature accepted`
  （age 16s = 仍在推进）；EXIT 325（nonzero 56 + crashsig 49，残余崩溃
  但**不堵队列**）。staged-apps：`S dirs=735 files=348 mib=38 fail=0
  err=0` 健康拷贝中（断电时未完，marker 未写，下轮引导续拷后写）；
  `D pages=4194304`（**16GiB 店面实装**）wr=10188 cm=348 er=0；
  RAMDISK capacityMiB=**16384** residentKiB=**1008**（稀疏兑现）；
  UFS TIME 累计 verify 6.3s/read 35.4s/1908MiB 无停顿；MEM free 588MB。
  下轮**免刷**：直接起机跑 30-40min，每 ~5min 拍照（Data 持久化，
  staged-apps 下轮补完写 marker）——观测 SpringBoard 是否到场与队列
  余量；若到场则开始 UI 阶段（OBS bind/display 交接）。
- **"慢"的真身 = SpringBoard 崩溃循环（同读窗 deeper look）**：
  `[spawn]` census `SpringBoard 34/17@31`——**SpringBoard t≈31s 就首次
  exec**，然后 sig5（SIGTRAP，故意 abort）崩 267 次，同簇
  contextstored（341 次）、liveactivitiesd（289 次）；SpringBoard 与
  contextstored **同 PC `1a1b2f2ec`**（共享框架同一断言），liveactivitiesd
  同回溯帧（191d06cc0/191cd80e4）。每轮 崩→重拉→exec 过签名→
  `xpcproxy 457/231` 拖 XPC 群重启 → CPU 97% 满（idle 全程 2-3s/650s）
  = "11 分钟到 backupd"的表象，**到 UI 真实耗时 31 秒级**。
  修复方向：donor 21H461 dyld_shared_cache 符号化 `1a1b2f2ec` 定位断言
  → 按 sheng text_fixups 先例修（同病先例：PosterBoard 因 Data 卷无
  file protection 建不出 store → 补字典放行；AWAttentionAwareness init
  死锁 → 放行返回）。我们的 text_fixups 已含 PosterBoard 修复，此簇是
  另一点；contextstored/liveactivitiesd 都是受保护文件写者 + 我们
  Data 卷无 data protection = 同根头号假设。profile 顶格吃 CPU 的
  p107/p86/p76 = 内核侧线程（签名验证/VM），PF 集中 p107。


## 0.25 2026-10-26 终：下会话工作单 = SpringBoard 崩溃簇符号化（用户跑满 17min 后断电）

- **用户侧**：跑满 17 分钟后断电（环满留痕）。下会话开始时用户操作：
  `fastboot flash boot_b <nura>/mu-r91-350386e0.img` +
  `fastboot flash userdata <nura>/meizu-meizu20-nura-min-r91-2G.simg` →
  起 nura-min → NCM 通后叫我。
- **我侧读回**（<1 分钟，流程同 §0.24）：dd 尾 32MiB → devpull
  `temp/logwindow-r44.img` → parser `--all`。核对：policy:0、
  staged-apps marker（S st=2 拷完/3 已拷过）、崩溃簇轮次与周期。
- **符号化（主任务）**：崩点 PC `1a1b2f2ec`（SpringBoard+contextstored
  共用）与 `1c40464f8`（liveactivitiesd）、bt 帧 191d06cc0/191cd80e4。
  输入 = donor 21H461 dyld_shared_cache：
  `7z x research/ipsw/iPad_Pro_HFR_17.7.11_21H461_Restore.ipsw
  044-76034-038.dmg`（3.3GB，Cryptex1,SystemOS；7z 支持 APFS 只读）→
  取 `dyld_shared_cache_arm64*` → 以未幻地址定 slide（cache 系地址
  0x191d…/0x1a1b…/0x1a98…）→ 符号化断言 → 按 sheng text_fixups
  先例开方（候选：Data 卷无 file protection 的同根假设——
  contextstored/liveactivitiesd 都是受保护文件写者）。
- **修好后**：重出件只重 recovery_a（种子/板卡不动），观察
  SpringBoard 稳定住 → home screen / Settings（sheng README 同款
  里程碑）→ UI 阶段开场（OBS bind、display 交接、触摸）。

## 7.2 下会话工作单（2026-10-26 定；r41a panic 取证 → 修复 → 步速观测恢复）【已执行，判决见 §0.22/§0.23/§0.24】

**会话开始前的用户操作（fastboot，单通道，slot=b，各件先 sha256 对拍）**：
1. `fastboot flash boot_b mu-r91-350386e0.img`
2. `fastboot flash recovery_a esp-recovery-v71.img`
3. `fastboot flash userdata meizu-meizu20-nura-min-r91-2G.simg`
   （三件都在 `/home/ri/work/nura-meizu-m2381/artifacts/`；sparse 只写
   userdata 头部 2GiB，尾部 32MiB 日志窗原样保留——r41a panic 的证据在里面）
4. 起 nura-min（~48s 到 devsh；USB NCM 设备侧 172.16.42.1）。
5. 读日志窗（nura 仓 tools/）：
   `python3 tools/devsh.py 'dd if=/dev/sda22 bs=512 skip=$((972326040-65536)) count=65536 of=/tmp/logwindow.img'`
   （分区相对寻址；32MiB 整窗）→ 顺手
   `tools/devsh.py 'head -c 8 /tmp/logwindow.img'` 应见 `SHLOG001`
   （= 写手确实写过；未见 = 写侧运行期失败，转查 hostLogOpen 拒因）。
6. 推回宿主：一个终端 `python3 tools/devpull.py logwindow.img`（监听 5555），
   另一个 `python3 tools/devsh.py 'busybox nc 172.16.42.2 5555 < /tmp/logwindow.img'`；
   文件放到本仓 `temp/logwindow.img`。
7. （可选，刷写质量对拍）`dd if=/dev/sde43 of=/tmp/boot_b.now.img`、
   `dd if=/dev/sde27 of=/tmp/rec_a.now.img` 同法推回，对拍 mu-r91/esp-v71。

**下会话分析单（我，按序）**：
1. `python3 devices/m2381/unixnu-m2381/scripts/read-m2381-ufs-log.py
   temp/logwindow.img --all > temp/r41a-log.txt`（合成窗已验证；--page/
   --spawn/--msg 可分节）。
2. 定谳三读：
   a. `[panic]` 节 = 完整 panic 文本：errno 值 + 24 帧回溯 → 撕裂假设判决；
   b. `[data]` 行 `DATA st=`：6=resumed read-only / 5=degraded → 假设坐实；
      2=resumed 正常 → 另找 errno 指向（写手并发/其他）；
   c. `[page]` 序列 + header：log state=2 轮数 ~180 e=0 → r41 写侧运行期
      实证（本会话唯一未验环节）；`UFS TIME` 每轮 ms → 读路径基线数据。
3. 判决后：
   - 假设成立（预期）：用户刷 **`3bafb6b9`**（f264c196+sheng-data-reset=1，
     `out/m2381/payload/payload-data-reset-1.img`）到 recovery_a → 起机 →
     policy 应过 → launchd 继续 → 该轮日志（窗连续）直接开始回答队列步速
     （§0.18①：spawn 速率 / stage +age / UFS TIME ms/往返）；
   - errno 另有所指：按 errno 修因重出件（写明在 STATE §0.22）。
4. 恢复/收尾：boot_b ← `devices/m2381/unixnu-m2381/build/m2381/boot/boot.img`
   （a1d51f04 留底）；userdata 若下次要重刷 = `out/m2381/ci-artifacts/
   userdata-root.img`（fd43ee70）；STATE §0.22 记录 + 记忆更新。

**红线提醒**：读回完成前，任何 `fastboot flash userdata` 都会毁证据；
nura-min simg 是唯一安全件（sparse 头部）。

## 0.26 2026-10-26 续十二：r44 读回 + 崩溃簇符号化定谳（§0.25 工单执行）= 修复件 a51aed16

- **r44 读回（logwindow-r44.img，md5 64ee3a1e 宿主/设备一致）**：单引导
  540 轮/1081s（环满回卷，最前 31 轮被盖），写手 state=2 e0 全程健康；
  policy/staged-apps 行被 msgbuf 回卷吞掉（161 行窗只留中段），但系统
  体征全绿：EXIT 278/nonzero45、UFS verify 5129ms+read 25214ms 无停顿、
  spawn 721@63s、daemon 全在起（lsd protobox、sandbox 拒绝均为良性类）。
  崩溃簇仍在：SpringBoard 563 次 sig5 + liveactivitiesd 964 次 sig5；
  **contextstored 只崩 3 次后自愈**（r43 是 341 次）。
- **符号化方法（本会话新建立，可复用）**：① CRASH 行 `+N` 字段 =
  `task_shared_region_slide`（BootFaultProbe.c:61，dyld cache slide）——
  r44 slide=0xafb4000、r43=0x2750000，两轮全部 PC/bt 帧同差 0x8864000 =
  同断言跨 boot 实锤；去滑 PC 全同（SB/contextstored=0x19F3DF2EC、
  lived=0x1C18F64F8）。② 21H461 dyld 缓存提取：SystemOS dmg 现成在
  `.cache/firmware/target-volumes/`（免开 IPSW）→ 7z APFS 直读出 52 子
  缓存 + linkedit；**`.dyldlinkedit`/`.symbols` 是 ZBM-rsrc（LZBITMAP，
  decmpfs 算法 14）**——偏移表 LE u32 + `ZBM\t` 块头 + 0xFF=raw 块，用
  Corellium libzbitmap（/tmp/libzbitmap，自编）+ 自写 /tmp/zbmrsrc.c 全
  解出（dec-symbols=686MB 本地符号表 1020 万 nlist）。③ 字段语义按上游
  `scripts/symbolicate-original-user-exception.py`（localSymbolsOffset@
  0x48、uuid@0x58、imagesText@0x88、sharedRegionStart@0xE0；子缓存表 =
  stub 内 56B 条目按 `.NN` 后缀扫）+ 镜像表(3354 项)定 image，nlist 二分
  定符号。工具与解出件留 `temp/cache-extract/`（下轮直接用）。
- **定谳（file-protection 假设死亡）**：
  1. SpringBoard+contextstored = **FrontBoardServices
     `-[FBSDisplayMonitor _initWithBookendObserver:transformer:]`** 的
     "no displays" assert（`displays.firstObject==nil` 时 `cbz` 跳入第三
     断言块 brk#0；text+0x47c）。根因 = LEGACY_DISPLAY=n 内核无显示
     服务 → CA 显示列表空，与文件保护无关（§0.25 的假设被数据否定；
     sheng 不撞此墙因为它 =y 车道有 IOFramebuffer 族）。
  2. liveactivitiesd = SessionSyncEngine **IOSKeybag init Swift fatal**
     （brk#1 与 ESR f2000001 吻合）——无 SEP AppleKeyStore 键可查。
     sheng 不崩因其 `sheng-aks=1` + KeyStoreProbe.o 在 LEGACY_DISPLAY
     车道内编译生效；我们的 =n 车道从未编它 → args 里的 sheng-aks=1
     是死旗标（内核 0 符号、0 字符串实证）。
- **修复（一轮双修，均为上游先例机制）**：
  ① OS profile text_fixups 增第 5 条
     `dyld_shared_cache_arm64.09,62aee70,ffc305d1,c0035fd6` = FBS init
     入口 `ret`（sheng AWAttentionAwareness 同款手法；init 返回 self，
     监视器变 no-op，显示通知缺失无损于 GOP-only 现状）。内核侧消费者
     在内核里实证在（text-fixups/text-fixup0 字符串）；payload 校验
     original 字节后才改页，错 build 自动放空。
  ② liveactivitiesd 进 `sheng-disable-jobs`（上游 bring-up-isolation
     先例：disabled job 的 Mach lookup fail-fast 而非 964 次/轮重生）。
  **aks 组件接线 = 下轮主题**（AppleKeyStoreState 是上游 catalog 组件
  recipe 现成；KeyStoreProbe.cpp 在 sheng package；缺 = 编译接线 + 
  probe 调用点——sheng 在 legacy-display late_probe 里调，我们需要独立
  调用点，如 BootWatchdogPolicy 里）；接通后 liveactivitiesd 可解禁。
- **产出（payload-only，内核未动）**：**payload `a51aed16`**（96MiB，
  md5 22f354f8；kernel 430344b3 不变、探针件 bf32d8fa 与 r42/r43 逐字节
  同；trust 7416ca4f 同件；种子 d04a0153 = grown 16GiB 身份 178b461d
  同 r43；args 增 liveactivitiesd、ADT 增第 5 条 fixup——均在 payload
  内实证）。桌面校验全绿。
- **下轮读图（用户只刷 recovery_a ← a51aed16；跑 17min 断电 → nura-min
  读回）**：期望 SpringBoard/contextstored 的 FBS 崩溃行消失（SIGTRAP
  簇只剩 0），EXIT crashsig 大降；看 focus3=Preferences/focus4=
  SpringBoard 里程碑是否首次到场（UI 阶段开场）。新 crasher 按本法
  符号化（temp/cache-extract 全套在）。若 FBS ret 后 SpringBoard 死在
  后续 monitor 方法断言 → 照 CRASH 行符号化迭代。

## 0.27 2026-10-26 续十三：r45 读回 = FBS/Keybag 修复实机生效；SpringBoard 推进到第二层显示断言；出件 d3c11770

- **轮次波折**：r45 首次刷写卡只给了 recovery_a（当时设备还在 nura-min 配置，
  boot_b=mu-r91/userdata=nura-min）→ nura UEFI "failed to find an operating
  system"；补三件全刷（a1d51f04/a51aed16/fd43ee70，§0.22 卡）后 r45 成跑。
  **教训入册：读窗后设备必在 nura-min 配置，iOS 轮的刷写卡必须含 boot_b+
  userdata 回装三件套。**
- **r45 读回（logwindow-r45.img md5 d3983394）**：534 轮/1069s 写手 e0。
  **两项 r44 修复行为学验证 ✓**：liveactivitiesd 从崩溃名单消失（禁用生效），
  contextstored 消失（FBS fixup 生效——r43 它崩 341 次、r44 崩 3 次自愈、
  r45 零次）。EXIT 272/nonzero26/crashsig18；spawn 12/s 突发；backupd
  t+26s。**SpringBoard 1527 行 CRASH（同 PC 重复展示）但真实退出计数
  ~18 次 crashsig**（CRASH 行是"最近 3 次"滚动窗，≠ 退出数）。
- **新断言（同一堵无显示墙的更深一层）**：PC 去滑 = 0x18F68234C（slide
  1aa50000）。调用链：`start(dyld) → SpringBoard main →
  UIApplicationMainPreparations → UIApplicationInitializeForBackgroundTasks →
  -[SpringBoard init] → -[UISystemShellApplication init] →
  -[SpringBoard startupInterfaceOrientation] → BackBoardServices "failure in
  %{public}@ (%{public}@:%i) : %{public}@" 宏 → brk#0`。失败函数 =
  **BKSDisplayServicesGetRenderOverlayForDisplay 返回 NULL**（消息串里点名，
  subcache .05 实读）。
- **修复**：OS profile text_fixups 增第 6 条
  `dyld_shared_cache_arm64.25,19c1ed0,ffc301d1e923026d,20008052c0035fd6`
  = `-[SpringBoard startupInterfaceOrientation]`（unslid 0x1cfa55ed0）入口
  `mov w0,#1; ret` 返回 UIInterfaceOrientationPortrait（2732 面板原生
  竖屏；sheng AW 同款入口常量返回手法）。
- **战略评估（显示墙经济学，记录在案）**：sheng 全程仅 3 条 text_fixups
  到 home screen，因其 =y 车道有真显示栈——我们 =n 车道面对的是这一类
  assert（每个 BKS/CA 查询点都可能炸）。**LEGACY_DISPLAY 车道移植 =
  真解**，且 §0.15 的"活机证据墙"经本轮复核**已有上游离线回退**
  （audit-mobile-graphics-legacy.py 的 else 分支：接受产物 abi.json 内
  钉住的 RUNTIME_SHA；sheng CI 正在跑这条车道）。成本 = sheng 车道的
  HID/touch 板级件（Novatek/GeniSpi，我们是 GT9916 I2C）需要替换或裁剪。
  列为显示主题（P4 后），本轮继续迭代字节修复。
- **产出**：payload **d3c11770**（96MiB；kernel/probe/trust/seed 与 a51aed16
  全同，仅 ADT 增第 6 条 fixup；payload 内三条实证）。commit 随本轮。
- **下轮读图**：用户**三件全刷**（boot_b=a1d51f04 +
  recovery_a=d3c11770 + userdata=fd43ee70）→ 跑 17min 断电 → nura-min
  读窗。期望：startupInterfaceOrientation 断言消失；若 SpringBoard 死于
  下一层显示查询（UIScreen main bounds/CA display link 等）→ 新 PC 照
  本法当场符号化；若过 init → 看 focus3/focus4 与画面里程碑。

## 0.28 2026-10-27 下会话工作单：r46 读窗 → 符号化 → 第 7 条 fixup（或转显示主题评估）

- **现状（本会话收口）**：r46 已跑（payload d3c11770 = 6 条 text_fixups +
  4 项 disable 名单），GUI 未起；用户已刷回 nura-min（读窗时设备未连
  NCM，r46 证据仍在 userdata 尾部 32MiB，nura-min sparse 不触碰）。
- **会话开始（用户侧）**：起 nura-min（NCM 172.16.42.1 通）→ 叫我。
- **我侧（按序，全套方法与工具就绪）**：
  1. 读窗三步（<1 分钟，§0.24 流程）：devsh dd 尾 32MiB → devpull
     `temp/logwindow-r46.img` → md5 对拍 → parser `--all`。
  2. 定谳顺序：① CRASH 行名单——startupInterfaceOrientation 的断言
     （PC 去滑 0x18C55E24C 附近、bt 含 UISystemShellApplication init）应
     **消失**（fixup 6 生效判据）；② 新 PC 照 [[方法]] 当场符号化
     （slide = CRASH 行 `+N`；temp/cache-extract 的 stub/dec-symbols/
     libzbitmap 工具全在）；③ 若无 CRASH 行而系统仍无 UI → 查 [page]
     的 stage/spawn/EXIT 与 msgbuf（kernel panic / launchd 停摆另判）。
  3. 修复候选（r46 死点为准确认；本会话已预扫 UISystemShellApplication
     init 在 orientation 后的调用序）：`setInterfaceOrientation:(1,0,0)`
     → `defaultCenter` → **`getFBInterfaceOrientationServiceClass` +
     FBOrientationService sharedInstance**（FBS 族下一个候选断言点）→
     其后 UIKit 窗口/UIScreen 链。修法沿第 6 条同款：入口常量返回。
  4. 出件 = payload-only（kernel/probe/trust/seed 不动），桌面校验
     （新 fixup 在 payload 内、旧 6 条在）→ **三件全刷卡**（boot_b=
     a1d51f04 + recovery_a=<新件> + userdata=fd43ee70；教训 §0.27）。
- **并行评估点（拍板项，若 r46 仍是显示类断言）**：显示类断言已逐轮
  各剥一层（FBSDisplayMonitor init → startupInterfaceOrientation →
  预期 FBOrientationService → UIScreen…），继续逐条打的预期 = 还要
  2-4 轮；**LEGACY_DISPLAY 车道移植** = 类解（sheng CI 在跑同车道；
  审计离线回退已证实 = §0.27；成本 = 车道脚本移植 + HID/touch 板级件
  替换为 m2381 GT9916 或先裁剪至纯 display）——下会话若又一轮显示类
  断言，建议两条腿：先发 fixup 让实机继续往前走（信息增量），同会话
  开始车道移植评估（离线工作不占刷写轮）。
- **打卡资产**：回滚链不变（boot_b←a1d51f04、userdata←fd43ee70，
  recovery_a 各轮件可重刷）；temp/cache-extract ~4.5G 保留至显示主题
  收口（符号化唯一数据源）。

## 0.29 2026-10-27：r46 读回定谳 = mainScreen nil 异常（fixup 7 出件 c3a24827）+ legacy-display 车道评估（审计离线 dry-run PASS）

- **r46 读回（logwindow-r46.img，md5 a8623502 宿主/设备一致）**：512 轮/
  1026s（环回 ~3 轮），写手 state=2 e0 全程健康。**fixup 6 生效判据过**：
  startupInterfaceOrientation 断言从 CRASH 行消失（msgbuf 内核侧实证
  `text-fixup: dyld_shared_cache_arm64.25+0x19c1ed0 (8 bytes) applied`）。
  **唯一残余 crasher = SpringBoard**（36 起/18 退@22s，~18 次/17min；
  contextstored/liveactivitiesd/FBS 全零）；EXIT 247/nonzero26。
- **定谳 = 新异常类（SIGABRT 非 brk）**：CRASH PC 1cf936f74（slide
  +41ac000，unslid 0x1CB78AF74）+ REASON 行 probe 堆扫描全文——
  `NSInternalInconsistencyException: returning nil screen from mainScreen
  is not allowed!`。符号化链（本次新增 bl 目标验证法）：bt0-bt6 真帧 =
  abort ← abort_message ← demangling_terminate_handler ← objc_terminate ←
  std::__terminate ← libc++abi outlined（0x1dd542b60，bl
  std::__terminate 实证）；bt7-bt11 = 陈旧帧（LR 落在函数尾非调用点），
  但证明 SpringBoard 崩前在跑 **CA::OGL::SW 软件渲染器 + CoreGraphics
  重采样**（init 已过、渲染中，~1min 生命周期）。**显示墙到 UI 层**：
  LEGACY_DISPLAY=n 无显示服务 → 屏幕列表空 → UIKit 17 mainScreen 主动抛。
- **修复 = fixup 7**：`+[UIScreen mainScreen]`（unslid 0x18a001d40，
  subcache .03 文件偏移 0x3255e0c）入口后 `cbz x21` 进 raise helper
  （0x2de 行）——改 `b +8`（0x14000008）直达干净返回，mainScreen 返回
  nil；下游 ObjC nil 消息矩形归零。注意 NOP 会落进第二处 raise
  （0x2df 行），必须用分支跳过整个 raise 区。
- **产出**：payload **c3a24827**（96MiB；kernel 430344b3/探针 bf32d8fa/
  trust 7416ca4f/种子 d04a0153 全不变，仅 ADT 增第 7 条 = 13996B，
  payload 内实证 7 条全在）。commit 08d1cb8（未推送）。
- **legacy-display 车道评估（离线腿，两腿战略第一条落地）**：
  1. **审计门实证排除**：`audit-mobile-graphics-legacy.py` 离线分支在
     本机 dry-run **PASS**——candidate 对 = 当前 kernel.macho+kernel.json
     别名硬链（计划 candidate_sha256=430344b3/native=8923ccdc 双对上，
     IOMobileGraphicsFamily 原始 sha=2e044ee6 命中审计 pin）+
     abi.json 桩（钉 RUNTIME_SHA `3044c2be…`）→ 全量产物生成（358
     slots/9 abstract leaves [211,213,245,246,306,307,337,338,345]/
     vptr 表）。留档 temp/mobile-graphics-legacy-abi.dryrun.json。
     check-native-legacy-display.py 的 5 份 HVF 观测件是资格记录非
     构建门（树内零调用点）。
  2. **sheng 定谳**：`LEGACY_DISPLAY=y + ADRENO=y + MOBILE_DISPLAY=n`
     ——真显示栈 = NativeLegacyDisplay + GOP presenter，板级 provider
     没用上；GPU 拷贝失败/拒绝自动落 CPU 拷贝回退（组件内建），我们
     ADRENO=n 可直接走 CPU 路径。HID/touch（NativeHidInput/GT9916）后置。
  3. **移植清单（半个会话级）**：①设备仓 scripts/ 移植
     build-native-legacy-display.py（sheng 版生成
     OriginalLegacyFramebufferABI.h + 编 NativeLegacyDisplay；改
     kits/NativePrelink→XnuPrelink 新名）；②board.toml archives 增审计
     +build 两步（candidate 别名硬链同 storage 审计法）；
     ③defconfig 翻 `CONFIG_XNU_NATIVE_LEGACY_DISPLAY=y`（args 里
     offline-legacy-*/offline-hid-* 全组早已在）；④CI 车道同步
     （device-m2381）；⑤内核重出 → payload → 刷。
- **方法补充（r47 起生效）**：SIGABRT 类崩溃先读 REASON 行
  （boot_progress_exit_reason 堆扫描，日志已带全文）再决定要不要符号化
  ——PC 停在 abort 链上，符号化只能给机制给不了抛点；bt 上段按
  "LR-4 必须是调用指令"验活（temp/verifychain46.py / disasm46.py 可复用）。
- **下会话工作单（r47）**：①用户三件全刷（boot_b=a1d51f04 +
  recovery_a=c3a24827 + userdata=fd43ee70）→ 跑 17min 断电 → nura-min
  读窗；②判读：mainScreen raise 应消失——SpringBoard 或死在下一层
  （新 REASON 行当场定谳，§0.29 方法补充）或推进到窗口/渲染里程碑
  （census/EXIT/stage 看焦点行）；③若又一层显示类异常 → 继续剥
  （预期 UIScreen bounds/scale 或 CA 显示链）并同步启动车道移植施工
  （清单见上）；④若 SpringBoard 稳住 → UI 阶段开场（OBS bind/
  display 交接/触摸主题）。

## 0.30 2026-10-27 续：r47 读回定谳 = 背光服务链（fixup 8a/8b 出件 821ab7c0）+ 车道准备（ABI 头入仓）

- **r47 读回（logwindow-r47.img，md5 70908c35 宿主/设备一致）**：509 轮/
  1019s（环满），写手 e0 全程健康。**fixup 7 生效判据过**：mainScreen
  raise 消失，msgbuf `text-fixup: dyld_shared_cache_arm64.03+0x3255e0c
  (4 bytes) applied` 实证。唯一 crasher 仍 = SpringBoard（36/18@22s，
  EXIT 254/nonzero27）。
- **新墙 = 背光域，且 bt 全链验活（r46 的"陈旧帧"结论修正）**：REASON
  `NSInternalInconsistencyException: Invalid parameter not satisfying:
  display`。调用链每跳 bl 目标都对上下文：`-[SpringBoard
  _prepareBacklightServices]` → `SBBacklightServicesPlatformProvider
  initWithBacklightEnvironmentSessionProvider:` → （super）
  `SBBacklightPlatformProvider initWith…` → `+[SBMainDisplayRootWindow
  ScenePresentationBinder sharedInstance]`（dispatch_once）→ once 块
  `-_init` → `mainConfiguration`（无显示=空配置）→
  `initWithDisplayConfiguration:` → 断言 display。**方法定则：r46 判
  "stale" 是因为该组 LR 落在非调用指令上；r47 同样位置全是真帧——
  bl-target 校验才是判活器，位置不预示真伪。**
- **修复 = fixup 8a/8b（一轮双修，r44 先例）**：
  ① `-[SpringBoard _prepareBacklightServices]`（unslid 0x1cfa612bc，
     .25+0x19cd2bc，orig `ff4302d1…`）入口 `ret`——整个背光域（provider/
     startService/watchdog 配置/explanation 块）在 =n 上全是死路；
  ② `-[SBMainDisplayRootWindowScenePresentationBinder _init]`（unslid
     0x1d005b930，.25+0x1fc7930）入口 `ret` 返 self—— sharedInstance 的
     once 单例照常建立（nil ivar），窗口阶段消费者不能再触发同一断言。
- **产出**：payload **821ab7c0**（96MiB；kernel 430344b3/探针 bf32d8fa/
  trust/种子全不变；ADT 14136B = 9 条 fixup 全在件）。commit 2ebdf9b。
- **车道准备（离线腿推进，施工留下会话整单元）**：
  1. **ABI 头已入仓**：`devices/…/package/OriginalLegacyFramebufferABI.h`
     （20903B，sha 86b9eba40f82）= sheng 生成器逻辑 + 本机 dry-run 审计
     产物（358 slots/9 abstract leaves）生成，按 Font.h 先例提交——组件
     编译不再依赖 staged prelink candidate；donor 换代才需重生成。
  2. **施工配方（sheng build-native-legacy-display.py 287 行全读完）**：
     fork 裁剪 = 保留 ①审计+头（头已入仓可跳审计，或
     candidate 对 staged 时跑全审计）②`build-arm-driver.py
     NativeLegacyDisplay` + 358 项 vtable 重定位核对（audit methods[]
     地址逐项对）③bindings 表（vtable/ctor/dtor/gMetaClass 动态取自
     audit；surface getters/vsync/slot-301 等 VA 在 sheng 脚本硬编码——
     **同 donor 21H461 对我们直接有效**，脚本自带字节验证是免费保险）
     ④original-bindings.S→.o（绝对符号，prelink 解析机制 =
     libStorageBindings.a 同款）⑤late objcopy（去 __mod_init_func +
     globalize）⑥NativeGOPPresenter 同法 ⑦ar 成
     `libNativeLegacyDisplay.late.a` + build-report.json（baseline
     :642 force_load 并核对 archive/source sha——报告结构必须含
     source_sha256/archive_sha256/generated_abi_sha256/…）。
     **裁剪掉**：sheng 的 probe 车队（HID/KeyStore/Touch/CpuFreq/
     GpuPower/BootLog/BootReset——我们的 BootLog 已在 BootProgress 档，
     其余 =n 不需要）。
  3. **树内核实**：kits/XnuPrelink/native_prelink.py 在（import 名改
     XnuPrelink）；kits/sheng/LegacyDisplayProbe.cpp 不在树（sheng clone
     里也没有该路径，需从上游 UniXNU kits/sheng 取或改用 GOPPresenter
     注册路径——施工时第一件事核实）；drivers/NativeHidInput 不在树
     （display-first 无需）。
  4. **配置**：defconfig 翻 `CONFIG_XNU_NATIVE_LEGACY_DISPLAY=y`
     （m2381_defconfig + owner）+ **必须手动重跑 defconfig 命令**；
     board.toml archives 插 `build-native-legacy-display.py`（顺序无关，
     链接前即可）；args 不动（offline-legacy-*/sheng-fb-gpu=1 全在，
     GPU offer 拒绝自动落 CPU 拷贝）。
  5. **风险**：LegacyDisplayProbe 注册路径待核实（上述 3）；新显示栈
     上电 BootProgress 页让位/新 crasher（正常）；sheng 脚本的
     `xcrun`/`brew --prefix llvm` 调用要换本机/CI 等价物（clang 装配树
     模板法，BootLog.o 先例）。
- **下会话工作单（r48/r49）**：①r48 刷写卡 = 三件全刷（boot_b=
  a1d51f04 + recovery_a=**821ab7c0** + userdata=fd43ee70）→ 读窗 →
  REASON 行定谳（预期：背光链消失，SpringBoard 推进到窗口/场景创建或
  render server 交互层）；②同会话开车道施工（配方见上，ABI 头已备）
  → 出 =y 内核 → r49 = 车道轮。

## 0.31 2026-10-27 续二：r48 读回定谳 = workspace 转换请求层（fixup 9 出件 036e712f）

- **r48 读回（logwindow-r48.img，md5 a5027982 宿主/设备一致，SHLOG001 在）**：
  504 轮 REASON 全同串，写手健康。**fixup 8 生效判据过**：r47 背光链断言
  （`: display`）从 REASON 消失，msgbuf `text-fixup: dyld_shared_cache_arm64.
  25+0x19cd2bc (8 bytes) applied` 实证（fixup 6/7 标记同在）。`[text fixup]
  parsed=9 applied=5`：8b（1fc7930）目标/期望字节离线核对全对但标记未见于
  窗（环形缓冲可能吞掉第 5 条标记）——开放项，不阻塞。
- **新墙 = workspace 转换请求层**：REASON `NSInternalInconsistencyException:
  Invalid parameter not satisfying: displayConfiguration`。SpringBoard
  崩溃 PC 1d42d2f74（slide +8b48000 → _mach_init_doit+0x400 abort 区），
  bl-target 验活：terminate 机器全链活（exception_cleanup→failed_throw→
  __terminate→blr _objc_terminate→blr abort），外段（HomeUI/SpringBoard
  bt9-11）陈旧帧（bt11-4 直接是 brk）。chronod（18/8@36s）新崩溃循环，
  sig5 Ef2000001（Swift 陷阱形态，dispatch_client_callout 真帧）——次优先，
  等主链推进后按 sheng disable-list 处理。
- **抛出点定位（本轮方法资产：CFString 链式指针解码）**：用户态 cache 的
  __DATA_CONST 指针是 chained-fixups 编码，`存储值 = VA | 0x400_00000000`
  （r46 mainScreen description CFString 0x1f01853e8 验证）。定位路径 =
  REASON 串 → 裸串/CFString 候选 → `bl _objc_msgSend$handleFailure…` 桩
  （各二进制私有桩，UIKitCore 桩 0x18b8fe800）→ 断言站点的 x6/x7 串回读。
  **两个扫描器坑**：①ADD 立即数判定掩码必须 `0xFF800000 == 0x91000000`
  （0x7F800000 丢 bit31 永不匹配——首轮全零引用的根因）；②跨段引用距离
  超 ±24MB（UIKitCore 文本 0x18a0xxxxx → 数据 0x1f018xxxx），xref 必须
  全区扫。工具：temp/disasm48.py、temp/blstub48.py、temp/live48.py。
- **断言点实锤**：`-[SBWorkspaceTransitionRequest
  _initWithWorkspace:displayConfiguration:]`（SpringBoard 二进制，入口
  0x1cf9a5f78 = .25+0x1911f78）内两个 NSAssertionHandler 体（
  SBWorkspaceTransitionRequest.m lines 47/48：cond 串 "workspace"/
  "displayConfiguration"，格式串 "Invalid parameter not satisfying: %@"）。
  xref 站点 0x1cf9a6128 = fixup 9 目标；8a 偏移反推 = `_prepareBacklight
  Services+0x0` 双重验证偏移约定。子类 SBMainWorkspaceTransitionRequest
  走 super → 入口 ret 生效。
- **修复 = fixup 9**：`-[SBWorkspaceTransitionRequest
  _initWithWorkspace:displayConfiguration:]` 入口 `ret`（返 self，空壳
  request——同 8b 形态；workspace 参数非 nil 已过 line47，nil 的
  displayConfiguration 来自空 mainConfiguration）。表新增第 10 条：
  `dyld_shared_cache_arm64.25,1911f78,ffc301d1fa6702a9,c0035fd6fa6702a9`。
- **产出**：payload **036e712f**（96MiB，out/m2381/m2381/ipados-17.7.11-
  21H461/m2381/payload/——注意本轮 --out 多一层 m2381/，历史件在
  out/m2381/ipados…；kernel 430344b3/探针 bf32d8fa/trust 7416ca4f/种子
  d04a0153 全不变，ADT 14204B 仅变，10 条 fixup 全在件核验）。
- **r49 刷写卡（三件全刷，设备现处 nura-min）**：boot_b = a1d51f04
  （devices/…/build/m2381/boot/boot.img，不变）+ recovery_a = **036e712f**
  （上述 payload.img）+ userdata = fd43ee70（out/m2381/ci-artifacts/
  userdata-root.img，不变）。预期：displayConfiguration 断言消失，
  SpringBoard 推进到下一层（REASON 行定谳）；车道施工（§0.30 配方）在
  读窗定谳后进行。

## 0.32 2026-10-27 续三：r49 首试作废 = 旧载荷被刷（路径歧义），重发卡

- **r49 读窗（logwindow-r49.img，md5 64fb2566）判谳无效**：509 cycle 全部
  `[text fixup] parsed=9`（495×applied=5 + 会话头 9×0/2×1/3×2 的爬坡）=
  一个完整的 **821ab7c0（r48 载荷，9 条）** boot 会话（rounds=513/
  uptime=1028s）；REASON 仍 479 条全 displayConfiguration。**036e712f
  （10 条，parsed=10）零 cycle 在窗 = 该载荷从未运行**。
- **根因 = 刷写路径歧义（流程债）**：r49 出件走了 `--out out/m2381` 得到
  双层路径 `out/m2381/m2381/ipados…/payload/payload.img`，而历史命令里的
  旧路径 `out/m2381/ipados-17.7.11-21H461/m2381/payload/payload.img` 仍指
  821ab7c0——刷写用了旧路径。**修正**：新件已复制到无歧义平铺路径
  `out/m2381/payload-r49-036e712f.img`（sha256 前 8 = 036e712f）；
  旧路径文件保持 821ab7c0 原样（回滚件不覆盖）。
- **读回实证（nura-min 侧，读回发生在 nura 链覆盖后，只证明现态不证明
  刷写时态）**：boot_b=27a08ad1 / recovery_a=2c3c3e4e = nura 链现役件，
  与三件卡全不同——正常，iOS 分区在 nura-min 读回流程中被覆盖（三件卡
  规矩的由来）。
- **r49 重发卡（三件全刷，设备现处 nura-min，current-slot=b）**：
  boot_b ← `devices/m2381/unixnu-m2381/build/m2381/boot/boot.img`
  （a1d51f04）+ recovery_a ← **`out/m2381/payload-r49-036e712f.img`**
  （刷前 `sha256sum` 必须见 036e712f 开头）+ userdata ←
  `out/m2381/ci-artifacts/userdata-root.img`（fd43ee70）。刷后各件读回
  对拍（FLASHING 纪律）。预期 = displayConfiguration 断言消失、下层按
  REASON 行定谳。

## 0.33 2026-10-27 续四：r49 二试定谳 = fixup 9 目标未执行（惰性应用判活），三连修 4e69f450

- **r49b 读窗（logwindow-r49b.img，md5 baff2457）**：509 cycle **全部
  parsed=10**（036e712f 确认上机，路径修正生效）。applied=5 =
  {BTAudio, 5(62aee70), 6, 7, 8a}——**fixup 9（1911f78）与 8b 都未应用**，
  REASON 仍 500 条 displayConfiguration（SpringBoard sig6 ×1481，
  chronod sig5 ×19）。
- **内核机制定谳（读 patches/xnu/native-text-fixup.patch）**：fixup 挂在
  `vm_page_validate_cs_mapped_slow`，**惰性应用 = 仅当含目标地址的页被
  首次签名校验时**才 memcmp+patch（按 vp->v_name 匹配子缓存文件名）。
  由此：8b 不应用 = 设计内睡守（8a ret 后 binder _init 无人调 → 页不换
  入）；**fixup 9 不应用 = SBWorkspaceTransitionRequest init 根本没执行**
  = 它不是本轮抛者（fixup 应用与否本身就是判活 oracle）。
- **抛者重定位（全区域补扫，修好掩码）**：109 裸串 + 9 CFString 全区域
  xref，可执行代码引用全部收齐；assert 体指纹（currentHandler → 条件串
  入 [sp] → handleFailure）筛掉 FBS setDisplayConfiguration 日志路径与
  SBNonInteractive appendString 调试标签。真断言体三个（r48 只找到
  workspace 那个）：
  ① `-[SBSceneHostingDisplayController displayIdentityDidUpdate:
     configuration:]`（SBSceneHostingDisplayController.m，站点
     0x1cfac4bd4 = +0x2f0）
  ② `-[SBSConnectedDisplayInfoFactory _supportedScalesForDisplay
     Configuration:defaults:error:]`（0x1cff7d108 = +0x210）
  ③ `-[SBSConnectedDisplayInfoFactory _effectiveSettingsForDisplay
     Configuration:defaults:]`（0x1cff7d210 = +0x88）
- **修复 = fixup 10（三连修，r47 双修先例扩展）**：①入口 ret 返 self
  （updater，0x1cfac48e4 = .25+0x1a308e4，orig `ff8302d1 eb2b036d`）；
  ②③入口 `mov x0,#0; ret` 返 nil（getter，0x1cff7cef8 = .25+0x1ee8ef8，
  0x1cff7d188 = .25+0x1ee9188）。fixup 9/8b 保留（惰性语义下的合法睡
  守，下层若触到会自动生效）。表 13 条（< CS_TEXT_FIXUP_MAX=32）。
  三条 applied 标记兼作执行 oracle：若断言仍在而标记全无 = 抛者在
  更深处，再按 oracle 收敛。
- **产出**：payload **4e69f450**（96MiB，平铺件
  `out/m2381/payload-r50-4e69f450.img`；kernel/trust/种子不变，ADT 仅变
  13 条全在件核验）。
- **r50 刷写卡（三件全刷，current-slot=b）**：boot_b ← a1d51f04 +
  recovery_a ← **`out/m2381/payload-r50-4e69f450.img`**（刷前 sha256sum
  须 4e69f450 开头）+ userdata ← fd43ee70。预期：displayConfiguration
  断言消失；若仍在，先读三条新标记定执行面。

## 0.34 2026-10-27 续五：r50 定谳 = oracle 全排除（三新目标也未执行），抛者更深——本轮定位收口，下轮转上游同步+解法调研（用户拍板）

- **r50 读窗（logwindow-r50.img，md5 0cc85146）**：509 cycle 全部
  `parsed=13`（4e69f450 上机✓）。**applied=5 原班人马**（BTAudio/5/6/7/
  8a）——三条新 fixup（1a308e4/1ee8ef8/1ee9188）零应用 = 三个候选函数
  也都不在执行路径。**至此五个 NSParameterAssert(displayConfiguration)
  站点全部排除**（workspace 转换/scene hosting/SBSConnected×2 + 两个非
  断言假阳性），REASON 仍 499 条同串，SpringBoard sig6 ×1319，
  chronod sig5 ×178（继续升温）。
- **栈线索关闭**：r50 崩 bt 去滑后与 r48 逐帧全同（确定性栈布局）；
  bt8=0x18644158C 落 .01 数据段（无指令可反汇编）= 陈旧帧实锤；live
  链仍只有 terminate 机器。原始窗内崩溃日志缓冲只含 reason 行，无
  Last Exception Backtrace 符号。
- **定位收口时的开放缝（下轮可续）**：①auth-rebase 编码的 CFString
  指针扫描（ENC 掩码只认 `VA|0x400_00000000`，认证指针高位不同会漏；
  修正掩码 `ptr & 0x3FFFFFFFFF` 的重扫被用户叫停）；②.23 区两个裸串
  引用点（0x1c943f1ec/0x1c94419e8，垃圾符号）未反汇编定性。
- **下轮工作单（用户拍板 2026-10-27）**：
  1. **上游同步**：`git fetch upstream` + rebase m2381（照
     docs/UPSTREAM.md；现 pin 16e592a，查新增——设备仓 schema/驱动/
     patches 变化），同步记录一句话入档。
  2. **上游/fork 解法调研**：上游 UniXNU 的 sheng/nx729j/fuxi 板
     text_fixups 全表（是否比我们抄的多）、它们对 displayless 用户态
     的处理（=y 车道之外有没有机制解）、其他 fork（HackiOS/ios-topaz/
     社区 SM8550 移植）同墙解法。
  3. **备选方案（调研后拍板）**：
     a. **类杀**：`-[NSAssertionHandler handleFailureInMethod:object:
        file:lineNumber:description:]`（0x1874494d4，Foundation .03）+
        `handleFailureInFunction:…` 变体入口 ret——一次灭整个
        NSParameterAssert 传递类（displayless 必然连环触发的断言类，
        r43→r50 已花八轮）；下一墙直接变成非断言类崩溃。
     b. **=y 车道加速**（§0.30 配方已就绪，ABI 头在仓）——根因解。
     c. 上游/fork 若有现成机制解则直接取。

## 0.35 2026-10-27 续六：r52 读窗定谳 = =y 车道一级实机全过（PE handoff+真显示配置+swap），SpringBoard 崩溃循环钉死在 BannerKit displayConfiguration 断言（r50 深抛者收口）

- **读窗**：logwindow-r52.img（32MiB SHLOG 窗，sda22 尾部 skip=
  972260504，设备/宿主 md5 `b63cda4d3ca5742eab3aa9ac39a7db59` 一致）。
  单次引导 593 轮 / uptime 1189s（≈20 分钟，用户断电时刻；wrap 后窗内
  最早 t=169s），write_errors=0——payload 元组（22f8f0bd）正确上机。
- **屏幕时间线与日志对齐**：诊断页 ~2-3min 让位（BootProgress 遥测读
  真 provider 后让位）→ 苹果 logo 上屏（先模糊后清晰自稳）= OS 侧
  推帧；[swap timeline] W1 t=85.7s / W2 t=87.8s / W3 t=88.2s 三次全帧
  swap（offer/done 2.1s/213ms/1.3s）= logo 推帧时代，此后无新 swap =
  画面冻结在 logo。
- **=y 显示栈健康清单（一级全过的实证）**：
  - `DISPLAY stage=10 PM=4 calls=4 callbacks=1 guard=0 owned=1`；
  - `GOP actual mapping virtual=fffffff1a650b000 physical=b8000000
    bytes=10368000`（=1080×2400×4 全帧）；
  - IOMFB Debug Info 字典：width 0x438 / height 0x960 / format
    0x42475241（'ARGB'）/ layer0 src_stride 0x1100 / Last Swap
    Completion State=4 —— **=y 栈给用户态的是真显示配置**；
  - [scanout prepare] calls=6 failures=0；[mfb clients] C0 pid=76
    calls=30 fails=18（sel6/0x80000000 → e00002d5 ×2，杂
    e00002bc/e00002c7/e00002f0）C1 pid=86 calls=3 fails=1 —— IOMFB
    用户客户端通道已通但**选择子半数失败**（机制线头号嫌疑）；
  - r51 telemetry 换血后 `native_legacy_stage`/NativeGOPFramebuffer
    nub 记录不在 msgbuf（上述 IOMFB dict/swap 即健康实证）。
- **SpringBoard 崩溃循环定量**：census `SpringBoard 28/14`（14 次
  sig6，pid p213/215/221/223/240/248），全程 CRASH 行逐字节全同
  （PC/LR/bt 不变）→ 循环持续到 1189s 断电；`backboardd 2/0` 稳定
  （spawn 2 崩 0）→ **logo 冻结 = backboardd 持有最后合成帧**；
  chronod sig5 ×9（较 r50 ×178 降温）；nonzero=24 = SB14+chronod9+
  IOMFB_FDR_Loader 1。
- **fixup oracle**：`parsed=13 applied=5` 原班人马 {BTAudio,5,6,7,8a}
  ——显示族 8b/9/10 未醒 = 站点未达，SB 死在更浅处（与 r50 收口一致）。
- **抛点定谳（r50"抛者更深"收口）**：REASON 506 条同串
  `'Invalid parameter not satisfying: displayConfiguration'`。去滑
  N=0x7280000，bt 内→外，**bl-target 验活全过（temp/verifychain52.py，
  r52 版链）**：
  `-[SBBannerManager init]+0x68` → bl msgSend$initWithAuthorizedBundleIDs:
  → `-[BNBannerSourceListener initWithAuthorizedBundleIDs:]+0x4c` →
  bl msgSend$initWithServiceDomain:displayConfiguration:authorizedBundleIDs:
  → `-[BNBannerSourceListener initWithServiceDomain:displayConfiguration:
  authorizedBundleIDs:]+0x1e8`（BannerKit，.17 子缓存）→ bl
  msgSend$handleFailureInMethod:object:file:lineNumber:description:
  （NSAssertionHandler）→ terminate 机器 → abort。**=y 环境下 bt 用户帧
  首次全活**（=n 时代 r48-r50 的 bt 用户帧经 bl-target 复核为陈旧/错滑：
  r48 帧列表 0x1b9f54888 有 0x10000 转录滑差且正确去滑落 HomeUI 垃圾
  符号——r52 前用户态抛点链从未可靠捕获过）。
- **实参来源反汇编**：initWithAuthorizedBundleIDs 内 `adrp x8, 
  0x1e9502000; ldr x20,[x8,#0x290]`（GOT .33 0x1e9502290，类引用）→
  `msgSend$mainConfiguration` → x3 = displayConfiguration 实参。即
  **SB 侧显示配置单例 `[<类> mainConfiguration]` 返 nil**。该类引用
  存储值 0x201f4a63ee0 不合 `VA|0x400_00000000` 约定（新开放缝③，
  与①同族掩码问题）。
- **机制线线索（与 fixup 11 并行）**：mfb 选择子失败簇（e00002d5
  not-ready 类）+ mainConfiguration nil + sheng 同 SoC =y 全通（仅
  3-4 条基础 fixup 不打显示族）⇒ 我们 =y 栈与 sheng 的差异 = 供给链
  断点嫌疑：dpu 钩子绑定（weak 替身本机 ld 拒弱）/probe 车队裁剪/
  IOMFB 选择子实现面（sel6/0x80000000 等）。
- **r53 工作单**（STATE §7）：fixup 11 =
  `"dyld_shared_cache_arm64.17,670e6a0,ff8302d1fa6705a9,c0035fd6fa6705a9"`
  （深层 init 入口 ret 返 self；期望字节 = sub sp,sp,#0xa0 +
  stp x26,x25,[sp,#0x50]，已核；变体 = caller 入口 .17,670e634
  f657bda9f44f01a9）；kernel 3ab2ace4 复用只重打 payload；**发卡 =
  三件全刷**（设备已回 nura-min：boot_b e74c73da / recovery_a
  0a102220=FLASHING §6 nura ESP 值 / userdata nura root，devsh 实测）；
  读窗判读 applied=6 + 下一墙符号化。
- **用户实测补记**：设备在 ~20 分钟时被用户断电（logo 全程未变），
  随后用户自行刷回 nura-min——读窗在 nura-min 态完成（照 r41a 例行，
  SHLOG 窗内容为 iOS 引导所写，nura 引导不覆盖）。

## 0.36 2026-10-27 续七：r53 读窗定谳 = fixup 11 实机生效（applied 5→6），SpringBoard 推进到 BLSHBacklightHost brk 新墙；r54 = fixup 12（payload 1ef5eb63）

- **读窗**：logwindow-r53.img md5 `b8dd3dc9aa0a8e4f1d49196837931612` 宿主/
  设备一致。单引导 341 轮 / 684s（≈11.4 分钟 = 用户断电时刻）。**判读坑
  （环形窗残留）**：窗口 509 slots，r53 只写 slot 0-340（t=0-684s），
  slot 341-508 = **r52 残留**（t=682-1017s）——不分会段会把 r52 尾部
  数据当 r53 读（首读即中招：把 r52 尾部当"显示栈退化/Current Display
  空/swap offer=-1"，分段后翻案）。
- **fixup oracle（修正后）**：早期快照（t≈0-2s）`parsed=14 applied=0`
  = 惰性应用未触发（正常）；**尾部快照（t≈680s）`parsed=14
  applied=6`** = fixup 11（BannerKit 入口 ret）醒来且生效，原班五条
  依旧。payload 04f4d803 元组正确上机（kernel 3ab2ace4/probe
  474724fd/trust 7416ca4f/seed d04a0153 全对）。
- **SpringBoard 推进（双簇）**：①早期 sig6 = BannerKit 断言（CRASH 行
  与 r52 逐字节全同，连 slide 0x7280000 都同）= fixup 应用**前**的
  循环，FAIL 行持久；②fixup 11 应用后 SB 走过 BannerKit，**新墙 =
  sig5 brk：+[BLSHBacklightHost sharedBacklightHost]+0xbc**（bt 12 帧
  符号化全活：applicationDidFinishLaunching+0x5d4 → +[SBBacklightController
  _sharedInstanceCreateIfNeeded:] dispatch_once → init →
  -initWithContextProvider:+0x9c → BLSHBacklightHost sharedBacklightHost
  brk，PC=LR=+0xbc，BacklightServicesHost .35 子缓存）——SB 从 init
  早期推进到 applicationDidFinishLaunching 的 backlight-host 层。
- **显示栈（修正后）**：尾部快照 owned=1、GOP 全帧映射、UFS verify
  5379ms/1620MiB 正常、SWAP calls=3 accepted=1 COPY 3 次全帧——健康。
  **新观察：W1 首交换异常（offer=-1ms/done=-1ms，r52 同位置
  offer=2109ms 正常）** → 用户照片的撕裂 logo（横条缺失块）= 首帧
  内容不完整上屏后冻结——swap 管道首帧竞争，非致命（后续 swap 正常），
  记录待观察。
- **chronod sig5**：census 36/17（较 r52 ×9 加凶），照旧次优先。
- **r54 施工（用户自刷）**：fixup 12 =
  `"dyld_shared_cache_arm64.35,61c7b64,ff0301d1f65701a9,000080d2c0035fd6"`
  （+[BLSHBacklightHost sharedBacklightHost] 入口 mov x0,#0; ret 返 nil
  ——nil-messaging 下游安全，表内 1ee8ef8 同配方；页内偏移 0xb64 合规；
  变体备选 = SBBacklightController initWithContextProvider 入口）。
  text_fixups 14→15 条；kernel/探针/trust/种子不变，ADT d57a5879，
  payload **1ef5eb63** 平铺 `out/m2381/payload-r54-1ef5eb63.img`。
- **r54 读窗判读点**：`parsed=15 applied=7`（fixup 12 醒来 = host 站点
  在路径）+ REASON 换墙符号化 + W1 swap 是否复现 offer=-1（撕裂帧
  复现性）。

## 0.37 2026-10-27 续八：读窗契约根治（parser 守 header 计数）+ 官方移植文档合规对照 + r54（fixup 12）+ r55（forensic 内置取证 reader）

- **读窗误读根治（用户点名，c353cd7）**：r53 首读中招的环形窗残留 =
  parser 违反上游 BootLog.c 读者契约（"header 每轮最后写，读者永不
  信任 header 未计数的 slot"）。修复 = slots_written ≤ slot_count 时
  截断到 0..slots_written-1 并打提示，wrapped 不截；r53/r52 双窗验证。
  **不做自动清空**（上游也不清）：清窗毁早死引导证据（applied 0→6
  早期快照即证据）+ 白磨 UFS。r56 起 kernel_args 可加
  `sheng-build-tag=rNN`（BootProgress.c:526 现成）让每行带轮号。
- **官方移植文档对照（unixnu.com/zh/porting/ 全 19 页）**：报告 =
  `docs/PORTING-GAP-REVIEW.md`。骨架高度合规（device-repo v2/组件化/
  patches 正道/SoC feed）；差距集中「验证与发布」板块：官方 log-capture
  四方法对照——①AdvancedLogDxe 固件日志二维码导出（未集成，行动项），
  ②UART/拍照（屏幕照片在用），③**持久日志+Linux reader（r55 补齐）**，
  ④fastboot boot 循环（m2381 ABL 死路，等价物 = sheng-autoreset PMIC
  cell 待实测）；另有 records 适配记录页与 bring-up 检查点表两项文档债。
- **forensic reader（= 官方方法③标准形态，fork 上游 uefi-fuxi
  package/linux-log）**：
  - 机制实证：UniXNUPkg Boot.c（git 历史 e95b37d 恢复原源）`type =
    "linux"` = LoadImage(PE) + Entry->Args 置 LoadOptions + initrd 上
    LoadFile2 media + StartImage；返回后恢复 FDT/卸 initrd 回菜单。
    unixnu.toml 的 linux entry 是一等公民（二进制 strings {xnu, linux,
    action, tools} + boot: LoadImage 全链日志行）。
  - 内核 = nura ESP v71 的 BOOTAA64.EFI（pmOS r91 vmlinuz-efi，sha
    4aad92b3，**m2381 实机验证过**的同一件：UART/USB/UFS/configfs
    全通；无内嵌 initramfs，cmdline 由 entry args 覆盖）。
  - initramfs（1.3MB）= busybox 1.36.1 arm64 static（Ubuntu ports）+
    init（fork 上游：PARTNAME=userdata 查找、尾 65536×512B dd、
    configfs acm.usb0 gadget、USB serial shell）+ rebootbl（自编译
    aarch64 static，reboot(RESTART2,"bootloader")）。
  - 注入器 = devices/m2381/unixnu-m2381/scripts/add-forensic-entry.py
    （mcopy 无挂载改 FAT：两件进 Payloads/ + unixnu.toml 追 entry，
    /tmp/payload-test.img 验证通过）；构件构建 =
    forensic/build-forensic-reader.py。流程入 FLASHING §7。
- **r54 出件（fixup 12，1ef5eb63）**：r53 新墙
  `+[BLSHBacklightHost sharedBacklightHost]+0xbc` brk（sig5，bt 12 帧
  全活）→ 入口 mov x0,#0; ret 返 nil（1ee8ef8 同配方；.35,61c7b64；
  页内偏移 0xb64 合规；变体 = SBBacklightController
  initWithContextProvider 入口）。
- **r55 组件（fb2c5dd1）**：r54 裸件 + forensic entry 注入 = 现役待刷
  件；r54 裸件保留。三件卡不变（boot_b a1d51f04 / userdata fd43ee70）。
- **r55 读窗判读点**：①iOS 侧 `parsed=15 applied=7`（fixup 12 醒 =
  host 站点在路径）+ BLSHBacklightHost 换墙符号化 + W1 swap 复现性；
  ②reader 首验（USB ACM 枚举 / dd 魔数 / parser 无截断提示错误）。

### 0.37 补正（同日）：设备现态勘误 = iOS 链现役（r54 未刷回 nura）

- 交接时误记"设备在 nura-min，下轮三件全刷"——用户纠正：r54 刷完
  断电后**没有刷回 nura**，设备现态 = iOS 链（boot_b a1d51f04 /
  recovery_a 1ef5eb63 已上电 12 分钟 / userdata fd43ee70 全未被动；
  NCM 无 172.16.42.x 接口佐证非 nura）。
- **r55 刷写修正 = 只刷 recovery_a**（`fb2c5dd1`）；"三件全刷"规矩
  只在 iOS 链被 nura 覆盖后触发（r44/r49 教训的正确边界）。
- **forensic reader 的额外红利**：iOS 链现役时连 nura-min 回退都不用
  刷——reader 在 payload 里，从菜单进；reader 起不来才需要刷 nura
  ESP（只 recovery_a，boot_b 不动）。
- STATE §2/§7 + AGENTS §7 已同步修正。

## 0.38 2026-10-27 续九：reader 三连修（LFN→dtb→SecureBoot）boot_b 换件 289614ba；reader 黑屏（内核 early 挂）下会话定位

- **r55 首刷（fb2c5dd1）翻车 = LFN 破坏**：add-forensic-entry.py 用 8.3
  短名（UNIXNU~1.TOM）覆盖 toml → FAT LFN 条目丢失 → Mu FAT 驱动按
  LFN 打不开 \EFI\UniXNU\unixnu.toml → config 整体丢弃 → 菜单显示
  "built-in configuration" + no boot entry found（用户截图铁证）。
  mdel+LFN 重写的在位修复也不可靠 → **注入器退役，机制正道 =
  payload.py 插桩**（board.toml [payload] forensic=true → stage 复制
  .cache/forensic 三件 + unixnu.toml 追 type=linux entry，正规
  write_volume 通道无 LFN 风险）。教训：**mtools 在位编辑 FAT 的短名
  覆盖必毁 LFN，payload 内文件只能由构建管线生成**。
- **秒退双根因（r55b→r55c→boot_b 换件）**：
  ①entry 无 dtb key → Boot.c PrepareDeviceTree 不装树 → Linux EFI
  stub 回退固件 FDT config table（m2381 ADT，qcom 内核无效）→ 秒挂。
  修 = entry 加 `dtb = "Payloads/forensic.dtb"`（nura
  sm8550-meizu-20.dtb，Inject.c:427 InstallConfigurationTable 机制）。
  ②**ENABLE_SECUREBOOT=1**（board.toml firmware.defines，sheng 拍来）
  → Mu SB 下 LoadImage 验 PE Authenticode（Silicium 硬编码 PK/db，私钥
  不在手）→ 未签名 PE = EFI_SECURITY_VIOLATION → Boot.c 卸载回菜单。
  修 = ENABLE_SECUREBOOT 1→0（iOS 链零依赖：kernel.macho 非 PE 走
  trustcache/AMFI；FV 内模块不经 LoadImage；DarwinBoot loader=firmware
  不从卷加载）。FV 重建 → package-m2381-boot.py 重打 → **boot_b
  289614ba**（新 fd 7237ea09，替代 a1d51f04；发卡三件值更新）。
- **r55c（69538b41 = fixup 12 + reader 三件 + dtb）+ 289614ba 实机**：
  用户实测**无闪回**（LoadImage/StartImage 过了，SB 修复生效）→
  **黑屏：linux 内核滚了几行就黑，之后纯黑屏；USB 零枚举**（宿主
  lsusb 无设备侧、无 NCM、无 fastboot）。用户纠正确认 = 内核真的
  开始执行（滚字 = 内核早期输出，非 stub 阶段），死在 very early
  （USB gadget 驱动之前）。候选假设（下会话工作单
  STATE §7.1）：a. 内核 early panic（cmdline/fdt/boot services 差异
  —— nura 内核在 nura 自己的 Mu 上实机通，我们的 m2381Pkg 环境差异
  是真实变量）；b. console 失效但内核活着（USB 零枚举否掉大部分）；
  c. 兜底 = 改走 nura 原生形态（取证专用 ESP，recovery_a 与 payload
  互斥轮换）。
- **发卡现役件（2026-10-27 reader 调试后）**：boot_b = **289614ba**
  （无 SB）+ recovery_a = 当轮 payload（r55c = 69538b41）+ userdata =
  fd43ee70。回滚件只在 nura 仓。
- **下会话 = STATE §7 工作单**：黑屏定位（宿主 USB 观察 → cmdline/
  UART → 兜底 ESP 形态）+ iOS 判读顺延补做（applied=7）+ autoreset。

### 0.38 补二（同日）：黑屏现象两次纠正 + 捕获手段三选 + ramoops 闭环发现

- 用户实测纠正两次：①"内核滚几行就黑"（= 内核确实开始执行，非
  stub 阶段黑屏）；②"日志一下子就消失，拍不到"（early 输出生命周期
  极短——stub 行/early console 被 fbcon 切换清掉或死太快）。
- **捕获手段**：①慢动作录像 240fps（零成本）；②**ramoops 闭环
  （首选）**：reader DTB（nura apk 同源）自带
  `ramoops@a8000000`（reg 0x200000，console-size 0x7d800，fdtdump
  实证）——内核死前 console 在保留内存，**刷回 nura 读
  /sys/fs/pstore/console-ramoops 即得死亡现场**；③UART（ttyMSM0，
  nura cmdline 有 qcom_geni_serial，引脚待查 nura 仓）。
- ABL 重试计数自动重置需求（用户）入 STATE §7.3：机制 = GPT entry
  属性位（nura FLASHING §3 实证：set_active b 重置预算/qbootctl 起
  机 mark successful；misc 与直刷链无关）；方案 = m2381Pkg 加
  BootSlotReset DXE（起机清 boot_b tries/successful=1 + 重算
  CRC32 写 primary/backup）+ **BootLog GPT check 豁免 boot_b 属性位**
  （否则写手停 state=4 日志窗停更）；红线 = 只动属性位+CRC，不动
  分区数据区，misc 不碰。

### 0.38 补三（同日）：pstore/UART 双双判死（nura 记录），捕获收窄为录像+keep_bootcon

- 用户提示 nura 有记录——查实（BOOT-CHAIN-RE §6.3 + SUSPEND-S2IDLE
  §2.2）：**ramoops 四重死**，最致命 = 0xa8000000 被 ABL 每 boot 重写
  （nura 实测搬址也死），我们每轮重启必过 ABL → 死前 console 结构性
  留不住，ramoops 闭环划掉；**UART** = m2381 无引出（BOOT-CHAIN-RE
  墙四："交接后第一秒没有任何观测通道"），划掉。
- **可行捕获**：①240fps 慢动作录像（零成本）；②cmdline 加
  `keep_bootcon fbcon=nodefer loglevel=7`——"日志瞬间消失"的对症药
  （early console 被 fbcon takeover 清屏；panic 默认挂住不重启，early
  行会留在屏上到死）；args 改一行重打 payload。③兜底 = nura 原生
  ESP 形态不变。

### 0.38 补四（同日）：r55d = nura 保命 cmdline（第四修）已刷 087f9ca3

- 用户点破方向："直接拿 nura 现成的内核弄？nura 是正常启动的"——内核
  本来就是 nura 的（ESP v71 同件），真正的差异 = **cmdline**：EFI stub
  在 bootloader 提供 LoadOptions 时**整体替换**内核内置 cmdline，
  r55b/c 的极简 args（earlycon=efifb console=tty1 loglevel=4 quiet）
  丢光了 nura 实机 cmdline 的早期生存集合（`fw_devlink=off
  clk_ignore_unused pd_ignore_unused icc-rpmh.qos_disable=1
  icc_rpmh_qos_disable=1 qcom_geni_serial.con_enabled=1 console=ttyMSM0`
  等）——高通板上少这些 = early 阶段 clk/pd/icc 框架过早探测 = "滚几
  行死在 USB 前"的完美吻合。
- r55d = nura 完整 cmdline（**去 root=/dev/sda22 与 init=systemd**——
  本板 userdata 是 iOS root，reader 不得碰）+ `keep_bootcon loglevel=7`
  （early console 不被 fbcon takeover 清掉，日志留屏到死）。payload
  087f9ca3 已刷 recovery_a（slot=b OKAY 已 reboot）。
- 下会话 = STATE §7.1 第一测 r55d：成功则 reader 通道立通（宿主
  ttyACM 拉窗 + rebootbl 回 fastboot）；仍挂则日志留屏（拍死亡行）。

### 0.39（同日）：r55d 实测定谳 = initramfs cpio 手写 12 字段错位；r55e = b060b512（只换 initrd）

- r55d 现场三照（用户 5 次实测）：①20.6s root 挂载 panic 留屏
  （8 核企鹅 + UFS 全量枚举 sde53-78 + bdev fs 表 ext3/2/4 squashfs
  vfat msdos f2fs）；②7.3s `[7.26] Trying to unpack initramfs →
  [7.34] Initramfs unpacking failed: damaged header → Freeing initrd
  memory: 1260K`；③ABL 连败预算耗尽自动落 fastboot 一次。用户澄清：
  **内核每必起**（保命 cmdline 生效），几次中途黑屏 = 显示域抖动，
  最终全败。
- 定谳链：LoadFile2 initrd **投递链实拍证明是通的**（内核收到
  1260K 并尝试解包；DTB/cmdline 同证全通）→ 死在 **cpio 内容**：
  build-forensic-reader.py 手写 newc 头只写 12 字段（fuxi 母版
  build-linux-log-boot.py = 13 字段含尾部 check；且 namesize 少算
  NUL）→ 头 110 字节契约被破坏 → 每个文件名错位 → "damaged header"
  → 无 /init → `VFS: Unable to mount root fs on unknown-block(0,0)`
  → USB gadget 永不出现。**同根因一次性解释 panic 与无 USB 两象**。
- 修复 = cpio() 对齐 fuxi 13 字段/小写 hex/namesize 含 NUL；initrd
  重建 **402fb431**（1,293,314B），内核等价 13 字段解析（11 条目走通
  到 TRAILER）+ GNU cpio -it + 解包内容 sha 对拍源件（init/busybox/
  rebootbl 全符）三重验证全绿；手术注入（add-forensic-entry.py 重写
  文件、toml 原样 diff 相等）出 **r55e = b060b512**。设备仓
  .cache/forensic 同步换新 initrd（full rebuild 一致性）。
- 行为入档：①ABL 重试预算 = 同槽连败数次耗尽自动落 fastboot，恢复
  = `fastboot --set-active=b`（nura FLASHING §3），自动重置仍是 r56
  BootSlotReset DXE 主题；②显示域抖动（3/5 黑屏，fw_devlink=off 下
  probe 顺序非确定，DSI/面板序列偶发失手候选）不阻塞通道——reader
  通道 = 只读 userdata 尾 32MiB + USB CDC ACM，判定以 lsusb 1d6b:0104
  为准，屏幕黑照测。
- 下会话 = STATE §7.2 刷 r55e（只刷 recovery_a；先 set_active b 重置
  预算）→ reader 第一测 → 通道通即补 §7.3 iOS 判读（applied=7 +
  BLSHBacklightHost 换墙）。

### 0.40（同日）：r55e 刷写闭环 + 取证 reader 通道首验立通（通道转正）

- 刷写（本会话代刷）：`fastboot --set-active=b`（retry-count 7，预算
  重置）→ `flash recovery_a` ← payload-r55e-**b060b512** OKAY →
  reboot；现态 iOS 链只刷 recovery_a 卡合法（userdata/boot_b 未动）。
- **通道首验全绿**：菜单选 reader → 宿主 `lsusb` 出 **1d6b:0104** →
  acm-reader probe：`m2381 forensic log reader 7.3.0 / usb: acm on
  a600000.usb / userdata=sda22 sectors=972326040 log=33554432 bytes
  **magic=SHLOG001**`（32MiB SHLOG 窗在位；nura 内核分区名 = sdaNN，
  init 的 PARTNAME grep 自适配）。
- **刷写读回对拍在 reader 内闭环**（§2 纪律首次无 adb 达成）：
  `grep PARTNAME=recovery_a` → /dev/sde27 → `dd bs=1M count=96 |
  sha256sum` = **b060b512…** == 镜像 sha 逐字节相等。
- 宿主侧持久化（用户装）：udev 规则 `99-m2381-reader.rules`
  （1d6b:0104 tty MODE=0666）；工具转正 = `devices/m2381/unixnu-m2381/
  scripts/acm-reader.py`（probe / cmd / fetch，fetch = 设备端 sha256 +
  base64 流拉 + 宿主对拍）。FLASHING §7 全面改写（构建/注入手术路径/
  宿主准备/读窗循环/iOS 时长标准/rebootbl 行为）。
- **iOS 引导时长标准入档**（BootLog.c 机制）：**每 2s 写一个 8KiB 槽**，
  slot_count≈509 → **~17 分钟写满回卷**，回卷覆盖最早的槽（fixups/
  applied 证据所在）→ 标准 = 跑 **8–15 分钟**、屏幕形态稳定 + 2–3
  分钟、长按硬断电；勿超 17 分钟。历史 "10-20 分钟" 上限即回卷边界。
- rebootbl 行为实测：**只重启、不落 ABL fastboot**（我们链；nura 侧
  完整进 fastboot 方法不搬，无人值守才需要；人工 = 电源+音量减）。
- 下会话 = STATE §7.3：控制变量 iOS 跑（8–15 分钟）→ acm-reader
  fetch → parser 判读 `applied=7` + BLSHBacklightHost 换墙 + W1 swap
  → 进入 fixups 快速迭代。

### 0.41（2026-10-27 下午）：r55e 首轮 iOS 判读定谳（applied=7 + 换墙 displayIdentity）+ fixup 9 出件（payload-r56-db3a7392）+ fetchchunk 转正

- **控制变量跑**：用户 iOS（r55e payload xnu default entry）跑 ~10.5
  分钟（诊断页 1–2 分钟 → 苹果 logo 常亮未进桌面）→ 长按硬断电 →
  重上电菜单选 reader → acm-reader 拉窗。
- **通道两次单流 fetch 断流**（~13MiB 解码量处，设备端 sha 稳定）：
  分块拉（2MiB×16，每块新 ACM 会话 + dd|base64）**16/16 块 + SHA
  MATCH 31s**。顺带发现并修 acm-reader fetch 行过滤 bug（48..122 丢
  含 `+`/`/` 的 base64 行 → 整包解码必损；修 43..122）；`fetchchunk`
  子命令转正（acm-reader.py）。窗 = rounds=314 slots=314/509
  uptime=628s（未回卷，write_errors=0）。
- **三大判读项全中**：尾槽 `[text fixup] parsed=15 applied=7`（惰性
  oracle：显示族+背光族 fixup 全生效，r53 fixup 11 与 r47 8a/8b、
  fixup 12 都打上了）；**[swap timeline] W1/W2/W3 真显示 swap**
  （t=243.275/244.712/245.286s，done=1410/188/207ms；GOP 映射
  b8000000 10.4MB = 1080×2400×4）；系统全生态起跑（173 进程 / 881
  spawn；analyticsd/configd/sleepd/mDNSResponder/akd 等齐）。
- **换墙定谳 = displayIdentity 断言**：SB 自 t=68s 起 SIGABRT 循环
  （p27→195→213→…→254 不停重拉），552 次崩溃 PC/LR/bt **逐字节相同**
  （slide +a57c000）。符号化（21H461 本地 1020 万 nlist + LR-4 bl
  验活全过）：`-[UIApplication _handleDelegateCallbacksWithOptions…]`
  → `-[SpringBoard applicationDidFinishLaunching:] +0x888` →
  `+[SBProximityBacklightPolicyFactory createNew…]+0x8c` →
  `+[SBMainDisplaySystemGestureManager sharedInstance]`（once 块）→
  `-[SBMainDisplaySystemGestureManager _init]` → msgSendSuper2
  `-[SBSystemGestureManager _initWithDisplayIdentity:]`，identity =
  `+[FBDisplayManager mainIdentity]`（= sharedInstance ivar@8，无显示
  栈从不注册）= **nil** → NSParameterAssert（SBSystemGestureManager.m
  **:119**，REASON 行 `'Invalid parameter not satisfying:
  displayIdentity'` ×276 全同）。
- **RE 行政细节**：SBMainDisplaySystemGestureManager 自己还有一个
  `_initWithDisplayIdentity:` 陷阱（0x1cfe90ac0，"unallowed
  initializer"，:53）——sharedInstance 的 once 块特意走 super 绕开它；
  **勿修陷阱，修父类**。`+[FBDisplayManager mainIdentity]` = `[shared
 Instance mainIdentity]` = 返 ivar@8（retains）——根因 = legacy 显示
  车道无人给 FBDisplayManager 注册主显 identity。
- **fixup 9**：`.25,1e64f10` `-[SBSystemGestureManager
  _initWithDisplayIdentity:]` 入口 `ret` 返 self（同 8b 形；expected
  `ff8301d1f85f02a9` = sub sp,#0x60 + stp x24,x23）→ profile.toml
  16 条 → `./unixnu payload`（UNIXNU_UNIQUE_CHIP_ID=3645ad55d47e77c1）
  → add-forensic-entry（LFN 读回 OK）→ **payload-r56-db3a7392**；
  ADT 内 fixup9 字串逐字节实证在位（BLSHBacklightHost 条目之后）。
- 预期 r56 新墙：identity nil 的下游消费者（比对/解引用链）或 SB UI
  下层断言；applied 预期 8（fixup9 所在页在 sharedInstance 构造时即
  被换入）。
- 下会话 = STATE §7.1：只刷 recovery_a ← db3a7392 → xnu 8–15 分钟 →
  硬断电 → `fetchchunk` → 判读 applied=8 + displayIdentity 墙是否破
  → 下一发 fixups。


---

# 2026-10-28 → 10-31 迁入档（=y 车道 + iOS16 回退车道；自 STATE.md §1/§5/§6/§7/§8 前段原样迁入，2026-10-31 精简）

## §1 一句话现状（r77 已刷已判 + 战略转 =y；原 2026-10-28 更新版）

## 1. 一句话现状（**r77 已刷已判（2026-10-29 凌晨）+ 战略转 =y**）：

**四连击达成 = #30 实锤生效**（窗 078a8227 208/509 uptime=417s，
recovery_a 读回对拍 `93854822…2d35b` ✓）：'Failed to initialized
main screen scale!' **0 出现** + applied **25→26**（parsed=30
applied=26）；**墙链五连单调后移全景**：nil-set（r73）→ workspace
门（r74）→ position NaN（r75）→ bounds NaN（r76）→ mainScreen
scale（r77 全灭），每轮 applied +1、每轮预注册分支①精确命中。
**PB 'configuration' CRASH 133→8 暴跌**；chronod sig5 ×348（独立
主题）；回归六项全绿 + sig11 断言族 0。**新墙 = SB sig11 ×172 全
新高质量点**：`-[WGCarouselListViewController
_updateCarouselEffect]+0x9dc` → 崩在 `__os_log_impl_flatten_and_
send` 区，FAR = `0x4000000000000018`（bit62 标签态）；全链符号化验
活 = `CFXRegistrationPost → -[SBIconController
_rootFolderDidChange] → _newTodayViewController… →
SBWidgetController newAvocadoWidgetListViewController… →
updateCarouselAndRevealState → _updateCarouselEffect`——**SB 已深
达今天视图（avocado）widget 列表装载**。RE 定性 = 崩点读
`___sharedInstance.onceToken+0x8`（WidgetKit 持久状态控制器单例域
0x231ec4060，与 #30 scale 槽同页）文件态干净全 0 → **运行时
0x4000…0018 = once 块从未成功完成的毒化标记被当对象指针**——**果
非因**（r72 FatalCorruptHashTables 同款机制）：要修须找 once block
内部的失败因（疑 = dataSourceManager init 内又撞供给族），**非十
分钟定点，本轮不出 fixup 件**。**屏面对齐：进度条 1:30 出现（
r76 1:24 同量级）仍冻结**。**战略评估（用户拍板征询，本会话定
谳）**：五轮数据 → ①决策规则未触发（无复发无回退）②墙已到
"once 域毒化"层 = 供给族缺失的**下游果**——逐墙喂常量的边际收益
开始递减（每堵新墙都可能又是另一 once 域的果）③**真正的分叉点不
是 donor 而是 =y 显示供给链**：displayless 是根因，换 iPhone
donor 这些墙同形重长（换 donor 不解决 displayless），只有 =y 接
通真显示配置供给族才成片消失。**拍板 = 修复继续但换主攻：用户态
fixup 降为支路（供给族遇一堵修一堵不再整轮追），主菜转 =y main
display 注册链攻坚**；止损线 = =y 攻坚 2–3 会话，若 =y 卡死且供
给族又积 5+ 堵则评估中间路线 B（保内核换 iPhone donor 21H16）。
r77 崩点全链符号化与 once 毒化机制已存档（§8 r77 行）。**r76 已
刷已判（2026-10-29 凌晨）：fixup #30 =
`_WGMainScreenScale` block invoke 供给 2.0 替代断言
（`.37,2117fac,ffc300d1f44f01a9fd7b02a9fd830091,
0010601ee8cf17b00019803dc0035fd6` = 入口 4 指令覆盖
`fmov d0,#2.0; str d0,[once 槽 0x231ec4030]; ret`）。**r76 已刷已
判：分支①再命中 = #29 实锤生效**（窗 acd75917 271/509
uptime=544s，recovery_a 读回对拍 `2bdd272a…d287808` ✓）：bounds
NaN **0 出现**（position 也保持 0）+ applied **24→25**（parsed=29
applied=25）。**换墙 = `'Failed to initialized main screen scale!'`
×136**——**性质换挡：从"可清洗垃圾"进入"环境供给缺失"**（抛点 =
`____WGMainScreenScale_block_invoke`（WidgetKit 域 .37
0x2024c7fac）：读 `[[UIScreen mainScreen] scale]` 写 once 槽
0x231ec4030，≤0 即断言（line 18）；displayless mainScreen=nil →
scale 恒 0 → 必死。**修法换型 = 供给替代清洗**：清 NaN 有 Apple
自带路径可借，scale 没有供给源，只能我们喂（fmov #2.0，fixup 16b
先例；写槽+返回语义完备，零入边）。PB 'configuration' ×133；chronod
sig5 ×338（增量观察项）。**回归六项全绿**：applied 25 / AKS 116 开
1207 调 / vsync 120 / SWAP accepted=1 / spawn 970 / noidle 544s 无
停写无 halt；sig11 CRASH 0。**屏面对齐：进度条 1:24 出现（r75
1:54 提前 30s）但仍冻结** = 布局链推进更深后死在 scale 供给（提前
= 布局循环更早走到渲染准备 = #29 的间接证据）。**决策规则判定 =
有进展**（三连单调后移：workspace 门 → position → bounds →
mainScreen scale；墙越越接近环境本源——**用户态治愈已进入收尾
区：剩余墙候选 = scale/几何供给族与 PB displayConfiguration，都是
"缺显示配置"这一根源的下游**）。r75/r76 判读存档 = §7.3 与 §8 行）
`CA::Layer::set_bounds` 同构 NaN 清洗（`.03,26e0068,600c0034,63000014`）。
**r75 已刷已判：分支①精确成立 = #28 实锤生效**（窗 d361a0c3 250/509
uptime=501s，recovery_a 读回对拍 `23868de1…a6a461` ✓）：'CALayer
position contains NaN' **0 出现** + applied **23→24**（parsed=28
applied=24）；**换墙 = 孪生检查 `CALayer bounds contains NaN: [0 0;
nan nan]` ×111**（预注册预期候选命中：position→bounds 同函数族推进；
抛点 = `CA::Layer::set_bounds` .03/0x18948c068 链 +0xbc raise，与
set_position 完全同构：fcmp NaN 检查 + audit 豁免 cbz w0 →
raise:format）→ #29 = 同法强制清洗路径（expected `600c0034` 实读核
验，零入边）。PB 'configuration' ×98（r74 ×89 同量级）；chronod
sig5 ~223。**回归六项全绿**：applied 24 / AKS 117 开 1221 调 /
vsync 120 / SWAP accepted=1 / spawn 951 / noidle 501s 无停写无
halt；sig11 CRASH 0；X5 活值继续（I/R/S 三槽）；W1 SS 两形正常
（SB 0x205f36000 / PB 0x213fa8000 滑后）。**屏面对齐：1:03 logo +
1:54 进度条出现但冻结（与 r74 相同形态 = 布局循环仍死在 bounds）**。
**决策规则判定 = 有进展**（连续两轮墙链单调后移：workspace 门 →
position → bounds，全是首现新检查点）。r75 RE/方法详见 §7.3 存
档）
`CA::Layer::set_position` NaN 抛点强制走 Apple 自带清洗路径
（`.03,26850c4,40090034,4a000014`）。**r74 已刷已判：预注册分支①
成立 = #27 实锤生效**（窗 344e72e5 325/509 uptime=650s，recovery_a
读回对拍 `b0f2f656…a6a461` ✓）：workspace 门 'you can't call this
before [SBMainWorkspace start]' **0 出现** + applied **22→23**
（parsed=27 applied=23）。**换墙 = SB sig6 `CALayerInvalidGeometry`
'CALayer position contains NaN: [nan nan]'（delegate =
WGShortLookStyleButton，WidgetKit 控件）×~296——SB 首次死在真实视
图构造层**：过 workspace 门后在布局 widget 按钮，NaN = displayless
0 scale/bounds 几何（r60 族 0 scale 在 UI 层现形）；**用户屏面对齐
= 2:30 画出进度条（真 boot progress UI）但不动（视图布局循环
死）**。抛点 = `CA::Layer::set_position`（QuartzCore .03
0x1894310c4 链，+0x98/+0x1ac 两 raise）；**Apple 自带解药** =
audit-exempt 分支（0x1894311ec：log + NaN→0 fcsel + w2=0 递归重设
不抛）→ **#28 = 0x1894310c4 `cbz w0`→`b` 强制豁免**（expected
`40090034` 实读核验，零入边）。PB 'configuration' ×89（r73 ×164 减
半，REASON 不变）；chronod sig5 ~427（独立主题）。**回归六项全
绿**：applied 23 / AKS 116 开 1189 调 / vsync 120 start+refresh /
SWAP accepted=1 / spawn 960 / noidle 650s 无停写无 halt 无 sleep
timer expired；sig11 CRASH 0；X5 探针升级出活值（B0 M0 + I/R/S 三
槽）。**决策规则判定 = 有进展**（墙链单调后移：生命周期门 → 渲染
几何；换内核讨论不触发，见 memory kernel-donor-switch-decision-
rule）。r74 RE 修正段（"raiser 在缓存外"作废 + raiser = fw 内
SBMainWorkspace `_sharedInstanceWithNilCheckPolicy:`）见 git
a48e7ac 与 §4.9）
`+[SBMainWorkspace _sharedInstanceWithNilCheckPolicy:]` policy==1
nil-check 断言 kill（`cbz→nop`，nil 改为返回 nil）。**r74 RE 修正
（同日二判）**：r73 的"raiser 在缓存外 SpringBoard.app 盘上二进
制"结论**作废**——根因 = 扫描器 add 掩码 bug（`(w2 & 0x7F800000)
== 0x91000000` **恒假**（0x91000000 & 0x7F800000 = 0x11000000），
正确掩码 = `0xFF800000`；即 STATE §4.4 r57 同款坑第三次复发，当日
从首个脚本传染到全部 xref 扫描——只有 adrp+LDR 形可见、adrp+ADD
全盲）。**修正后 RE 定谳**：reason CFString 0x1ea27d3c8 唯一 xref
= `+[SBMainWorkspace _sharedInstanceWithNilCheckPolicy:]+0xc8`；
方法本体（fw .25 @0x1cf9a45ac）：policy==1（b.eq 路径）实例槽
`0x1f56cf6a0` nil → 0x1cf9a464c 断言路径 → NSAssertionHandler
`handleFailureInMethod:file(SBWorkspace.m CFString):line 0x6b7
(1719):description(reason CFString)` → sig6；定点 = 0x1cf9a4630
`ldr x8,[x22,#0x6a0]` + 0x1cf9a4634 `cbz x8,assert`（**零入边**，
安全 NOP）；`expected=c85243f9c80000b4` / `replacement=
c85243f91f2003d5`（expected 已 cache-extract 文件实读核验）。**方
法学新沉淀（§4.9）：零引用结论必须先过阳性对照**——BaseBoard 的
NSParameterAssert 格式串 CFString（PB 断言 ×94 每轮必经）在坏掩
码下同样"零引用"，阳性对照一击证伪工具箱。SB.app 壳二进制 120KB
不含该串的部分仍成立（非缓存框架差集也干净）。**r73 已刷已判
（2026-10-28）**：**预注册分支①成立
= fixup #26 实锤生效**（窗 451ed5c2 301/509 槽 uptime=609s 无回
卷，recovery_a 读回对拍 `a3f310d42f488a66…585e56d` ✓）：`[text
fixup] parsed=26 applied=22` = 21→22（#26 页执行 oracle）+
**NSPlaceholderSet objects[0]=nil 断言本代 0 出现**（r70–r72 三代
×33/×347 的原墙关账，r72 定谳的 nil 调用者链被 #26 拦死）。**换墙
= SB sig6 ×266 新点**：REASON = `NSInternalInconsistencyException
'you can't call this before [SBMainWorkspace start]'`（×172 行）=
预注册预期①"SBLockScreenManager=nil 的下游"形状命中：SB 反复死在
一个 workspace-start 门；死亡形状单一（PC 0x1d95e6f74 = abort 链
噪声点，bt 8 帧全终结链 = 抛点帧被 unwind 撕净，dispatch_async_
and_wait invoke 外层）。**raiser 定位 RE（本会话主破案）**：REASON
串 → 全缓存唯一实例 0x1d02917bc（SpringBoard.framework
0x1cf99c000 内，`SBWorkspace.m` 文件标记串 0x1d0291785 紧邻，同池
邻居 = sceneManager/transientOverlayPresentationManager/
currentTransaction/daemonHandlers）→ CFString 对象 0x1ea27d3c8
（isa/flags/datap/len=50 全验证）→ **引用模型扫尽 = 零引用**：
①adrp+add/ldr 配对（相邻 + 12 指令宽窗，SB fw text 全区 + SB 主
二进制窗 + 全部小段）②绝对指针槽（低 48 位 + bit42 tag 全缓存
57 映射）③i32 相对指针（SB 邻域）——全零，仅存的两个指针槽 = 两
个 CFString 对象自身的 datap 字段 ⇒ **框架内这套断言常量 = 死拷
贝；活 raiser 在缓存外 = SpringBoard.app 盘上主二进制**（镜像表实
证 /Applications/SpringBoard.app/SpringBoard 不在 21H461 缓存，
缓存只有 framework + AccessibilityBundles 拷贝）⇒ **现 cache
text_fixups 机制够不着 r74 修点**（fixup 挂 dyld 缓存文件页校验路
径；盘上 Mach-O 另一套）。**支路③核对 = PB 独立墙未动**：sig6
×164 / 'Invalid parameter not satisfying: configuration' ×94（r72
同墙）；chronod sig5 ~400 行（r57 已知独立主题）。**回归六项全
绿**：applied 22 / AKS opens=118 calls=1248 / vsync hz=120
start=1 refresh=1 / SWAP accepted=1 bytes=10.3MB / procs=196
spawn=962 / noidle 维持（609s 无 139s 停写 + PMRD 零 `sleep timer
expired` + 零 halt）；sig11 CRASH 行 0 ✓；X5 dirty 窗 R0 全窗读
通保持；W1 SS 三形（SB 滑后 0x213d5e000→去滑 0x205f02000 / PB
0x221dd0000→0x213f74000 = **跨 boot 固定装载槽 r72 实证再证**；
第三形 b1d15f9000→0x1c379d000 ×3 归属未定）。**扫描器三坑再付费
（全部入 §4 方法）**：①fsx needle 要求紧随 NUL → 短语式搜索恒 0
（找串要直搜 data.find）；②按映射起点过滤会误截跨段区域（必须重
叠式判定）；③手工解 imm12/去滑又错两次（imm12 手算 0x368 实为
0x478；b1d15f9000 手算 0x1cf3039000 实为 0x1c379d000）——**"手工
必须走 python"一天内第三次付费**。r72 判读（nil 调用者指令级钉死
全链）见 git a0447d2；r71（noidle 关账/扫描器三改）见 §7 存档；
取证双铁律（header 分代 + re-tools read_run）已工具化）
**r64 出件（2026-10-28，本机 Linux 全链）**：静态腿 = 21H461 缓存头 /
slide-v2 链格式按 dyld-1241 源码破译（链 delta 在最高 16 位、4 字节单
位；值 = 低 48 位 = 绝对未滑 VA；value_add=0），UIKitCore
**__DATA_DIRTY**（0x1f489fef0+0x39958）全链枚举 5410 槽定性（objc
类/METACLASS/RO 结构 + UIUpdateSequence 哨兵）；**四个病灶槽
（bgTC 0x1f48ce5e0 / 锁 0x1f48ce340 / __UIMainScreenTraitCollectionLock
0x1f48cf938 / mainScreen 单例槽 0x1f48d5550 = __MergedGlobals+0x20）
文件态全 0** = 懒缓存 nil 静置态，毒在运行时写路径（r63 定谳复核；
r58 崩溃点指令级复验：+0x2c = `ldr x8,[x19,#0xe0]`，x19 =
[UIScreen mainScreen] 槽值 0xf00 → FAR 0xfe0；r63 PC 0x1800F5FAC =
objc_retain 快路径 `ldr x8,[x0]`，x0 = bgTC 槽值 0xf1 → FAR 0xf1；
r62 野跳点 = CoreServicesInternal cstring 零填充 = 垃圾 IMP 落点）。
**工作假说 = ObjC 元数据/once 域损坏级联**（同时解释野 PC 与小整数
值；0xf00/0xf1 跨 slide 不变 → 偏向数据常量或确定性页内偏移，排除
"指针减错基"）。动态腿 = **UIKitCacheProbe.c**（startup.o record 编
译私有 task/vm_map ABI，BootFaultProbe 先例）编入
libNativeLegacyDisplay.late.a；2s 轮询 SpringBoard
task_shared_region_slide，vm_map_read_user 全窗扫描 __DATA_DIRTY，
发布 ext 行 4-7（X4 tick/状态/slide/非零/垃圾计数，X5
mainScreen+bgTC 槽值，X6 首垃圾事件，X7 最近变更事件）；门控
`uikit-cache-probe=1`（+`uikit-cache-proc=` 改目标进程）；
`boot-ui-focus3=Preferences` 移除（cmdline 1002B > 1000 上限）。
25 条 fixups ADT 验证（24 dyld + BTAudioHALPlugin；16a/16b 字节级
精确）；内核重链 `e86478ad`；ESP 四样全过；**payload r64 =
`54b70ad7`**（96MiB，平铺 `out/m2381/payload-r64-54b70ad7.img`）。
判读工作单见 §7。

**调研定谳（2026-10-28，上游 f85b5a6→0fc0fc1 + 四个过墙板对照）**：①
**text_fixups 全清单**：sheng = 4 条基线（AWAttention 死锁 + 2×PRSPoster
文件保护字典 + BTAudioHALPlugin）；fuxi = 0 条（连段都没有）；所有
21H461 donor 板继承 OS profile 同 4 条——**没有任何过墙板携带显示族
fixup**。我们的 23 条 = 4 基线 + 19 条自加的 displayless kill 族。②
**过墙板答案模板**（上游 docs/devices/ 新设备目录）：asteroids
（SM7635，桌面第一帧）= **AppleKeyStoreState 组件替身**（"没有替身
SpringBoard 永远跑不完 boot" T20-T29 实证）+ sheng-vsync=120 +
offline-mobile-graphics-client=**0**；oneplus13t（SM8750，**无 GPU 到
锁屏** Build 50）= NativeKeyStoreEvents 组件 + sheng-vsync=60（fixed）+
CPU framebuffer；sheng 生产 = 144Hz vsync + KeyStoreProbe 编进 display
archive；pong（SM8475）= home screen；negroni = SB+touch CPU 渲染。
**四板共性 = AKS 替身 + vsync + 注册开启，无 GPU 也能过墙**。③我们
与模板的差距表（三项全缺）：KeyStoreProbe/AppleKeyStoreState 缺
（`sheng-aks=1` 参数一直在 = 死参数）；`sheng-vsync` 缺；
`offline-mobile-graphics-client=1`（asteroids 显式关）——且 r52 的
"mfb C0 sel6 失败头号嫌疑" **就是自设探针客户端**（patch 的
native_mobile_graphics_client_probe），不是 backboardd，嫌疑权重大降。
④DPU 零替身不阻塞（asteroids 同款 LegacyDisplayStubs 跑到桌面）。

**体系方案拍板（r62 = 机制线环境闭合，非打地鼠）**：r62 四件 =
①`package/KeyStoreProbe.cpp`（sheng 原版入包）②
LegacyDisplayProbe.cpp 接回 `native_keystore_probe()` 调用（
late_probe 首行，门控 offline-key-events=1 + sheng-aks=1 + persistent
Data 全在）③build-native-legacy-display.py 编 AppleKeyStoreState
组件（v2 include+component: 条目；产物名 = legacy 运行名
NativeKeyStoreEvents.o/.late.o）入 display archive ④board.toml
+`sheng-vsync=120`（m2381 面板 120Hz；>60 同时触发
IOMFBDisplayRefresh 发布）+ client 1→0。**23 条 fixups 不动**（安全
网 + 惰性 oracle：环境愈合后显示族站点应转睡，applied 计数可跌破
14 = 体系生效的定量信号）。log-only 断言杀 / 全指定 TC / 缓存全局
审计 = **r63 备选**（若 SEGV 垃圾族仍在）。

**r62 出件（2026-10-28，本机 Linux 全链）**：kernel `542300ca`
（display archive `b6daee99` 七成员含 KeyStoreProbe.o +
NativeKeyStoreEvents.late.o，build-report probe_fleet=
m2381-trimmed+keystore）/ 探针后 `158adedb` / trust `7416ca4f` /
seed `d04a0153`；ADT `b5b4a172`（23 条不变；sha 变 = random-seed 每次
生成即换 + 无正文异常，r58 20 条件二进制对比定谳）；**payload
r62 = `0854ea5e`**（96MiB，平铺 `out/m2381/payload-r62-0854ea5e.img`）
——ESP 四样全过（scratch-data.img 在 / toml initrd 行在且 args 含
client=0+vsync=120 / trustcache 在 / ADT PackedRAMDisk 在）。
**2026-10-28 已刷**（fastboot 实录：set_active b / sha 门 /
Sending+Writing recovery_a 98304KB OKAY / reboot）；**读回对拍 ✓ +
判读已毕（2026-10-28，见下）**。

**r63 判读定谳（2026-10-28）**：读回对拍 ✓（sde27 ==
`04eb0533…d5691`）；窗 rounds=312/509 uptime=640s 无回卷；
**fixup16a/16b 真执行**（applied 67→69，=+2 新站点各一）；AKS 爆发
（opens 123 / calls 1121，r62 的 18/49 → 23 倍，t=22s 起）；vsync
120 ✓；C0=IOMFB_FDR_Loader 换形（12 calls，失败选择子族
e00002bc/c7/f0）；SWAP 泵大（calls=8222 / COPY 8221 / 10.3MB
retired）；procs=194/spawn=968；**换墙 = SB sig11 ×554 同族未愈，
死点漂回 r61 有效码点**（unslid PC `0x1800F5FAC` objc initialize 快
路径 x0=0xf1；bt1 去滑 = 0x18A00DC50 = fixup14 getter 站点）——链
路钉死 = **bgTC 缓存槽 0x1f48ce5e0 被写进 0xf1 垃圾**，病根在缓存全
局槽的写路径（构造/once 域），在 TC 内容下游之前——全指定 TC 到位
了但毒在更上游。**r64 = 缓存全局审计转正（线 2）**：①静态——枚举
UIKit 单例槽（bgTC 0x1f48ce5e0 / mainScreen 槽 / 0xf00/0xf1 同族槽）
文件值 + rebase/chained-fixup 目标定性；②动态——内核探针行
（page snapshot ext slot 加 bgTC/mainScreen 槽值 dump，t 序列看
0xf1 何时被谁写入；内核步本机 Linux 可出）；③0xf1/0xf00 特征 = 小
整数带 F 高半字节 = "指针减错基"残留嫌疑（每次 boot slide 不同
d20000/1a20c000/1c084000，跨 boot 残留一致的 0xf00/0xf1 需要解释）。

**r62 判读定谳（2026-10-28）**：读回对拍闭环——recovery_a =
**sde27**（FLASHING §6 实测表；先误探 sda10/11 为错节点，非异常），
前 96MiB sha == `0854ea5e…db1a` ✓。窗 rounds=291/509 uptime=605s
无回卷。**体系三件实锤生效**：①AKS 替身活了（opens 3→18 / calls
6→49，selector 表 0:18 / 7:15 / 14:1 / 17:15）；②vsync 生效（hz=120
start=1 refresh=1 + IOMFBDisplayRefresh dict 在）；③client=0 生效
（探针客户端消失；剩余 C0 = **IOMFB_FDR_Loader pid=75** 真实消费者，
sel6 e00002d5 失败簇仍在——r52"C0=自设探针"的修正再修正：**C0 本来
就是 IOMFB**）。**环境未愈合**：applied=67（23 parsed，与 r61 的 70
同量级，未跌破 14）；旧墙链全灭（nil traitCollection / unspecified /
XB / displayIdentity 全零 + sig6=0，r57–r60 五墙确认封死）；**换墙
仍是 SEGV 垃圾族且升级**：SpringBoard sig11 ×520（t=66s→605s 整程
respawn 循环），unslid PC `0x1822F5FAC` = **cstring 数据区**（野跳
进垃圾"代码"，字符串字节被当指令执行后 FAR=0xf1 读数中止；fp 链全
废，bt 去滑全未映射，最后有效帧不可恢复）——0xf00/0xf1 族从"有效码
+ 垃圾操作数"（r58/r61，r61 点已复验 `0x1800F5FAC` ldr x8,[x0] 有效）
升级为"垃圾控制流"。procs=200/spawn=1003；SWAP 有真活动（accepted=1
COPY 10.3MB retired=4）但终态 DISPLAY stage=0；SB 未过 wallpaper/封
面层。**r63 分支（§7 预案三分支第一支触发）**：UIKit 侧候选 = 全指
定 TC 构造 / UIKit 缓存全局审计；NSAssertionHandler log-only 类杀降
级（断言墙已全灭，无对象可杀）。

**r61 判读存档（2026-10-27）**：窗 rounds=374/509 uptime=772s 无回
卷；applied=70（23 parsed；计数首超 parsed = fixup 页被反复重校验）；
'unspecified values' 断言零出现（块 A/块 C 死透）；换墙 = SIGSEGV
×684（PC `180e15fac` / FAR=0xf1）：`PBUIPosterWallpaperViewController
init` → `__UIGetCurrentFallbackTraitCollection+0xac` →
`_backgroundThreadFallbackTraitCollection+0x10` retain 快路径 x0=0xf1
垃圾（与 r58 0xf00 同族）。结构性定谳 = UIKit 缓存单例全局在无显示
环境成批变垃圾源；用户拍板停逐发 fixup 转体系路线（本轮即落地）。

**r56 = 废轮（教训已入法）**：出件漏 `--data-ramdisk`/`--trust-cache`
→ ESP 无 Data seed/initrd 行/trustcache → Data mount ENOENT panic，
applied=0。出件配方红线见 §7。r57 = 规范重建（seed `d04a0153` / trust
`7416ca4f` == r55e 件；toml initrd 行回归），已刷且**读回对拍已补**
（reader 内 dd recovery_a 前 96MiB sha == `1e7ea60b…` 镜像，2026-10-27）。

**forensic reader 调试五连**（本会话，9c53eee→cbdd5f0→b692151→r55d→r55e）：

①fb2c5dd1 LFN 破坏（短名覆盖 toml）→ 机制正道 payload.py 插桩
（[payload] forensic=true）；②秒退根因一 = entry 无 dtb → 补
`dtb = "Payloads/forensic.dtb"`（nura sm8550-meizu-20.dtb）；③秒退根因二
= ENABLE_SECUREBOOT=1（sheng 拍来）LoadImage 拒签 → 1→0，**boot_b
换件 = 289614ba**（替代 a1d51f04）；④r55d（087f9ca3 保命 cmdline）
实测定谳 = **内核每必起**（8 核企鹅/UFS 全量枚举，keep_bootcon 留屏
成立），死因实拍 = `Initramfs unpacking failed: damaged header` →
无 /init → `VFS: Unable to mount root fs on unknown-block(0,0)` → USB
永不出现；根因 = build-forensic-reader.py 手写 cpio **12 字段错位**
（newc 契约 13 字段，fuxi 母版即 13）——LoadFile2 投递链实拍证明是通
的；⑤**r55e（b060b512，只换 initrd = 402fb431）已刷 + 读回对拍 + 通道
首验全绿**（lsusb 1d6b:0104 / magic=SHLOG001 / 32MiB 窗在位；reader 内
dd recovery_a 读回 == 镜像 sha）。下会话 = §7.3 iOS 快速迭代。

## §5+§6 开放问题/不能声称的（=y 车道期现势声明）

## 5. 开放问题

- **主墙定性（r61→r62 体系路线）**：SIGSEGV ×684 = `ldr x8,[x0]`
  x0=0xf1 垃圾（retain 快路径，bgTC 内部）——与 r58 的 0xf00 同族：
  UIKit 缓存单例全局在无显示环境成批变垃圾源。已封死墙链 =
  displayIdentity（r57）→ XB 断言（r58）→ UIScreen SIGSEGV（r59）→
  traitCollection nil 块 B（r60）→ unspecified 块 A/块 C（r61）。
  **r62 押注 = 环境闭合让真显示配置/UIScreen 供给链复活**（AKS 替身
  + vsync + client=0，四过墙板模板）；若 SEGV 垃圾族仍在，r63 备选 =
  全指定 TC 构造 / UIKit 缓存全局审计 / NSAssertionHandler log-only
  类杀（勘址已毕 0x1874494d4/0x187449618，砍 raise 留 REASON 日志）。
- **机制线（track 2 真显示注册）**：r52 的 "[mfb clients] C0 sel6
  e00002d5" 头号嫌疑 **权重大降**——C0 = 自设
  offline-mobile-graphics-client 探针客户端（r62 已关），非
  backboardd。剩余机制嫌疑 = vsync/IOMFBDisplayRefresh 缺发（r62 补）
  与 AKS 缺失（r62 补）；若 r62 判读后 mainConfiguration 仍 nil，再
  入 IOMFB 选择子实现面与 dpu 钩子绑定。
- **chronod sig5 崩溃循环**（r57 实测 ×287）——次优先，主链推进后按
  sheng disable-list 处理。
- **独立主题（不阻塞）**：XS/fileset-KC 重绑定器（fileset KC 无 KASLR 表，
  prelink kit 需重绑定器）；arm64e 专项（透传已验证，FPAC/PACM/自举面/
  再签待专项）；GPU（ADRENO=n，P4 显示门过后主题化）。

## 6. 不能声称的

- 显示墙未破：SpringBoard 未到 home screen（r61 = 已深入 wallpaper UI
  构造 + trait 机器 SIGSEGV 循环）；keybag 未考（SB 未到锁屏）；触摸/
  按键/GPU 未上主题。
- =y 车道**一级已实机实证**（PE handoff + 控制台接管 + 推帧自稳 + 真
  显示配置到达 IOMFB 客户端层），且 r57 实证**用户态可画 boot progress
  UI**（菊花实拍）——但 main display 注册链仍断（isMainDisplay=0/
  mainIdentity=nil 族），"用户态显示配置供给链已通"不能声称。
- ~~类杀 NSAssertionHandler 未出件~~ **降为 r63 备选**：r62 拍板 =
  机制线环境闭合优先（AKS 替身 + vsync + client=0，四过墙板模板），
  断言类杀只备于 SEGV 垃圾族持续的情形；双方法勘址保留
  （0x1874494d4 / 0x187449618，砍 raise 留 REASON 日志形状已定）。
- ~~r62 出件~~ **已出件已刷**（`0854ea5e`，§1/§2/§7）；AKS 替身实机
  效果 / vsync 生效 / 显示族 fixup 转睡三项都未实证（用户报未起，
  窗待读），不能声称"体系方案生效"。

## §7 下一步（r78 = =y 攻坚开修）

## 7. 下一步（**r78 = =y 攻坚开修（2026-10-29 用户拍板"直接开修"）**：
**第一刀已钉死 = IOMFB 用户客户端 sel6 失败族（e00002d5）**）

**为什么是它（案卷在案，不是侦察）**：r52 首录 `[mfb clients] C0
sel6 e00002d5`；r62 修正确认 C0 = **IOMFB_FDR_Loader pid=75 真实
消费者**（非自设探针）；探针有参数记录（`sel6 args(0,80000003)
→ 0 fail` / `args(1,1) → 0 fail` / `args(1,80000000) → 1 call 1
fail e00002d5`）= **选择子分发表里 sel6 带 0x80000000 参数的调用
被拒**。vsync（r62 已发 120Hz start+refresh）与 AKS（r62 已活）
都到位而 mainConfiguration 仍 nil —— sel6 持续失败就是 main
display 注册链上**有记录的具体断环**。修好它 = IOMFB 客户端层
FDR 路径通 → mainConfiguration/isMainDisplay 预期接通。

**开修顺序（下会话，直接动手）**：
1. **定点 sel6 语义**（半小时级）：窗内 grep `sel6` 全部行拿全参
   数×返回码矩阵；sheng 源码对照——sheng 的 IOMFB shim 用户客户
   端选择子表（`drivers/` 内 IOMFB/NativeMetal/DisplayRefresh 相
   关 + `docs/` gpu.md）逐选择子对齐，找 sel6 对应函数与
   e00002d5 的返回条件；e00002d5 语义反查（IOKit 返回码表）。
2. **补实现**：按 sheng 实现搬齐 sel6 分支（参数级直修或组件级
   补齐；改内核须重编——Linux 出核链现成，改 defconfig 必重跑
   defconfig 命令）。
3. **出件 r78 上机**：判读 = ①`[mfb clients]` sel6 失败清零/
   C0 状态改善 ②mainConfiguration 非 nil / isMainDisplay=1 ③屏
   面进度条是否走动（供给族下游预期连锁反应）。

**里程碑（可观测）**：`isMainDisplay=1` / `mainConfiguration 非
nil`（IOMFB 客户端层）→ 供给族墙（scale/geometry/displayConfig
+ once 域毒化）预期成片消失 → SB 推进 wallpaper/图标层。

**止损线**（memory kernel-donor-switch-decision-rule）：=y 攻坚
2–3 会话；卡死且供给族又积 5+ 堵 → 评估中间路线 B（保内核换
iPhone donor 21H16）。**sel6 这刀打完若注册链仍断**，下一刀按
同法从 `[mfb clients]`/IOMFB 失败族的下一记录点开（数据现成）。

**支路纪律**：fixup 降支路——判读撞供给族断言（十分钟可定点）
顺手修；once 域毒化类（果）不追。

**出件配方红线不变**：payload 三 flag + --unique-chip-id
3645ad55d47e77c1（勿带 --skip-setup）；探针正源 = devices/
.../package/*.c（tree kits/sheng = symlink 同一实体），root
package 镜像同步；判读纪律 = header 分代 + re-tools read_run +
recovery_a=sde27 + fetchchunk + 修正掩码 0xFF800000 + 零引用先过
阳性对照（§4.9）。

**r78 第一刀定谳（2026-10-29 会话，①定点 sel6 语义完成 + ②修法已进件）**：

**三层闭环**：①`e00002d5` = **kIOReturnBusy**（`iokit_common_err(0x2d5)`
Device Busy；xnu IOReturn.h:168 + 仓内 ShengUfs RootCommandsBridge
双源钉死，"NotPermitted" 假设作废；头文件副本存
`temp/IOReturn-xnu-main.h`）。②窗矩阵（子代理 A，r52/r53/r55e/r69 四
窗）：8 种去重参数组合跨窗稳定——`(N,0x80000001)→0`、
`(N,0x80000000)→e00002d5`（N=arg0 递增请求号，[5820]/[5824] 恒镜像
arg0）、`(0,0x80000003)→0`、`(1,1)/(0,2)→0`；r69 变质 = pid=86 客户端
sel6 风暴 25236 calls/6297 fails（~26 calls/s 忙等）至窗末不收敛。
regs 四窗恒 0。③**Apple 二进制 RE（donor KC
IOMobileGraphicsFamily，carve+capstone）**：prelink 段全局映射
`file = vm − 0x482c000`（kext LC fileoff 是 slice 相对，第一次 carve
全错已修正）；user client 分派表 @`0x6f354c0`（IOExternalMethodDispatch
24B/项，sel6 = 3 scalars）→ trampoline `0x6779518` → ext_wait
`0x6777390`（写 client+0xf8=arg0、清 +0x100）→ **display 虚槽 176 =
`0x678d998`**（vtable `0x6f38570`；槽 249=getDisplayDimensions 与在案
ABI 交叉验证一致；实例大小 0x18a0=6304 与探针 +5864 状态字吻合）。

**v176 wait 分派语义（0x678d998 全函数破译）**：默认返回
kIOReturnBusy；`flags.bit31==0` → 转发 display+0x1608 gate 对象
vslot29（窗里 (1,1)/(0,2) 走此路 → 0）；bit31==1 且
request(flags&0x7fffffff)∈{3,4} → inline 检查 `+5856==0 &&
+5840(powered)!=0` → 0（**m2381 已通：announcePoweredByFirmware
10-21 进件后 +5840=1 在窗可证，旧"powered 无人写"叙事证伪**）；
bit31==1 且 request∈{0,1,2} → **同走 +0x1608 gate vslot29，其 wait
形态永不完成 = Busy 全部来源**。gate 完成信号属 platform 显示管线
（本 port 未实现），GOP 路径不消费其 payload。附带破译：
default_fb_surface ext（e00002bc 族）= `display->v174` 返 0 → 打
"No Default FB Surface found" → kIOReturnError（**下一刀素材**；
e00002c7=Unsupported、e00002f0=sel28 另族）。

**修法（=sheng announce 同款 stand-in 哲学，GopFramebuffer
submodule commit 477fc19）**：`nativeLegacyInstallWaitStandin` 在
startDisplay（announce 同址同条件）copy display vtable 512 槽、slot
176 → `nativeLegacyWaitStandin`：bit31 且 request∉{3,4} → 计数
`native_legacy_wait_standin_calls` +1 并报 kIOReturnSuccess，其余
（bit31==0 / request 3/4）转发原实现不变。探针
`[mfb clients]` 块首行新增 `wait-standin N`（devices 正源 + root
package 镜像已同步）。

**r78 出件与判读预注册（§7.2）**：本机 Linux 出件中（内核重链
+ payload，改组件未动 defconfig）。判读分支：①**命中** = sel6
`e00002d5` 行清零/不再增长 + `wait-standin` 计数 >0 增长 + C0/C1
fails 停涨 → 看 mainConfiguration/isMainDisplay 侧推进与屏面进度条
（供给族下游预期连锁）；②**部分** = sel6 清零但新墙（预期 =
default_fb_surface e00002bc 族或 FDR 后续）→ 下一刀按 v174 供给面
RE（素材已在案）；③**未命中** = Busy 仍在 → standin 未触发
（primary/bootMapping 条件或 patch 时序），回读
wait-standin=0 判接线。刷写纪律照旧（设备现态三件套核对 +
recovery_a=sde27 读回对拍）。

**r78a 判读（2026-10-29 会话二）**：已刷已读（窗 267/509 t=537s 无回卷）。
**分支③未命中，根因在构建链不在设计**：`update_submodules()`
（scripts/unixnu/feeds.py:217）在 kernel 步起步跑 `git submodule
update`，把 GopFramebuffer checkout 回主仓 gitlink（33e941a），同会话
submodule commit 477fc19 被甩（reflog 实证：commit 02:51:59 →
checkout 02:52:08）→ r78 内核 = **新探针（wait-standin 行在）+ 旧组件
（无 standin）**。窗证据：wait-standin 全程 0；sel6 Busy 族原样
（新增 (4,0x80000001)→0，s6# 环 N→a）；applied 惰性涨 0→25（r77=26
同量级，引擎健康）；**C0 风暴消失（r69 25236 calls→本轮 77）**记档不
下结论；屏面 logo 1:04 / 进度条 1:25 冻结（早于 r76 1:54 与 r77
1:30）。**修正**：submodule checkout 477fc19 + 主仓 gitlink bump 同
commit（新规矩：改组件必须 submodule commit + 主仓 `git add <模块路
径>` 同轮完成，否则构建静默回退）。r78b 重建中（内核重链 + payload
三 flag）。

**r78b 判读（2026-10-29 会话二续）**：已刷已读（窗 337/509 t=678s，sha
4d9cd833 ✓）。**分支③再未命中（wait-standin 全程 0、Busy 族原样），
但本轮把静态疑点全部排除**：①构建链（90fef580=e4c01b0 组件，standin
符号与 installer 序列都在内核）；②vtable 槽位——用 LEAF 占位锚点解出
编译后 kext vtable（opaque211/245/246/306/307/338/345 + kickSwapQueue
落槽 290，槽差逐一精确），**slot176 = 0x578d998 = donor Apple wait 尾
模式吻合 = 零漂移**；③installer 全序列在 start() 内联体（IOMalloc
0x1000+memcpy+`[copy+0x580]`=orig+`adr` 单指令写 standin（**adr ±1MB
单指令，adrp 扫描假阴性——工具教训入档**）+vptr store）；④primary=
true 实证（vsync hz=120 过 startVsync 的 `!primary||!bootMapping`
门）；applied=26 同 r77 量级。**嫌疑收敛为单点：ext_wait 的 owner
（`[uc+0xd8]`）可能不是被 patch 的 display 实例**。下轮探针 =
wrappedExternal 的 sel6 分支 dump target/`[uc+0xd8]` vs
`native_legacy_object`（及该 vptr 是否=copy 表）。屏面 logo 1:04/进度
条 1:20 冻结。判读分支 ③ 的第三种形态：**修法代码全对而 wait 调用
不经过被 patch 的 vptr**。

## §8 前段（r79 = iOS 16.7.16 donor 回退工作单）

**拍板**：=y 车道停（sel6 刀 r78a/r78b 两轮未落地，触止损评估点，用户直接拍换道）；donor 回退 **iPhone10,3 / iOS 16.7.16 / 20H392**（=10-24 换配前的原配对，资产全部保留作回溯，当时"留作回溯"现在启用）。**A11/t8015 = 经典 arm64 kernelcache（非 fileset）→ 零重绑定器成本，纯配置级换道**。

**为什么成立（对 10-24 拍板的反转理由）**：r35–r77 的整条墙链（traitCollection/avocado widget/once 域/sel6 FDR gate）全部是 **21H461 用户态+kext 的 17 代墙**；换 20H392 = IOMFB kext 与 SpringBoard/UIKit/FDR_Loader 全部回到 16 代内部自洽配对，17 代墙链整体蒸发。跨代风险（xnu-10063.141.1 内核源码 vs 16 代件）回到 10-24 前的状态——**但那套墙当时已被驯服**（entropy 守卫 373f769 仍在树内、绿蓝条+panic screen 全通；20H392 prelink 三值已 derive）。

**盘点（本会话查证）**：
- **IPSW 已到位**：用户本地件 `/home/ri/Downloads/iPhone10,3,iPhone10,6_16.7.16_20H392_Restore.ipsw`（6,171,035,648 B），sha256 = `ece1194f7e233e9109c5790a8d23a4564a10a254e15ec5b09978a343265568fb` **与旧档记录的全 zip sha 一字不差**，已硬链接入 `research/ipsw/20H392/`（步骤 1 完成）。CDN URL 也验活（HTTP 200 同长度）。备用件：21H16 IPSW 用户 Downloads 在（7,805,610,705 B，sha 验算中）——fileset-KC 重绑定器路线的备件。
- 旧 pin（git `d42a993~1:.../board.toml`）：product iPhone10,3 / build 20H392 / device_class **d22ap** / chip_id 0x8015 / board_id 6；OS dmg sha：`087-86698-044.dmg = 80f426d5…c92483`。
- 20H392 kernelcache sha256 = `2ec297bac95bcba01046fd0148e97e7e5885c05a6c18827f39f534ba375c1221`（经典 arm64）。
- **prelink 三值已 derive 在案**：`docs/archive/STATE-history.md:635` = slide `-0x1000000` + AMFI `0xfffffff008f3bd18`（三重指纹法）；text_fixups 是第三个值（见 :582，需按 16 代缓存重建或从旧档取）。
- 20H392 trust cache 打包 commit 65b8eb8 在史（flat container bundle）。

**施工单（下会话，按序；设备 8% 电量 → 用户拍板"先把充电修好，上游已经有了"）**：
0. **充电先行（用户拍板）——2026-10-29 会话已完成定位与半接线，下会话继续施工**：
   - **上游查证结论（修正 r79 预期）**：上游 main（已 rebase 同步 `f85b5a6→33da813`，仅 6 个 docs 提交）**没有**充电组件；充电 = **sheng 设备仓新近 11 提交的 ADSP 电池栈**（`QcomGlink/QcomQrtr/ShengQrtr/QcomSmem/QcomSmp2p/QcomQmp/QcomRpmh/ShengBattmgr/QcomMdt` + `AdspProbe/BatteryProbe/ShengBattery`，sheng `d76556b→08cde20`；机制 = ADSP 固件跑充电与 PD/PPS 协商，BATTMGR 走 PMIC GLINK，`sheng-adsp=2` 起机、`sheng-battery=1` 发布 IOPMPowerSource）。sheng = SM8550 同 SoC，SoC 常量（SMEM 0x81d00000/AOSS 0xc300000/RSC 0x17a20000/cmd-db 0x81c60000）1:1 适用。
   - **本会话已落**：25 个充电栈源文件已拷入 `devices/m2381/unixnu-m2381/package/`（`AdspProbe.cpp`/`AdspFirmware.c` 已 fork 嵌入式源）+ `scripts/make-adsp-firmware.py`（squash：mdt+bNN→单 .mbn，含 sheng 版跳过的哈希段，压实重写 p_offset）+ squash 产物已出（adsp.mbn 30,499,631 B + adsp_dtb.mbn 12,420 B，源头 = `/home/ri/work/meizu20/meizu20_linux/vendor-fw-harvest/modem/image/`，m2381 自有固件，`.cache/firmware/adsp-src` 暂存不入 git）。**未接线**（构建零影响，check-m2381 4 ok）：build-native-legacy-display.py 编译循环/archive 成员、LegacyDisplayProbe.cpp 调用点（sheng 487–500 同款）、board args、boot-progress 构建的 AdspFirmwareBlob。
   - **固件投递定案（ESP 算术否决嵌入式路线）**：r77 ESP free 仅 12.9 MiB、kernel.macho 已 66.7 MiB（recovery_a 100 MiB 硬顶）→ **30.5 MB 编进内核装不下**。正道 = **固件放 boot_b 空闲区（~80 MB free，红线允许碰）+ vendored ShengUfs 加 `native_ufs_part_find/read` 导出**（sheng ShengUfs `0e29c46` 同款 API，内核原位读；package-m2381-boot.py 在 boot.img 后追加固件，ABL 对 image_size 之外尾字节的容忍需首刷实证，可回退）。下会话施工序：①ShengUfs 加导出（对照 sheng NativeUfs.cpp:1207/1247）②package-m2381-boot.py 追加段 ③AdspProbe fork 加 boot_b 源 ④build 脚本接线 + 调用点 + args（cmdline 1000B 预算：删死参数 `sheng-cpufreq`/`sheng-touch` 换 `sheng-adsp=2 sheng-battery=1 armcpu-spi-ipi=1` + `sheng-disable-jobs` 加 coreduetd）⑤sheng 仓 host 测试（battmgr/mdt）本地编译验证。
   - **其余可抄面盘点（用户拍板"一起拿了"）**：已拍定抄 = AMP 集群（sheng `76780d7` zzyf-sheng-amp.patch + dts cluster-type 注记，m2381 patches/xnu/series 现为空文件直接加）+ coreduetd 禁用；**缓办**（支路纪律）= CpuFreq/GpuPower probe 重挂（裁剪车队遗留，GPU/touch 车道都停）、BootLog 新行（display/governor/VM/fence，需 reader 同步）、ShengUfs stage-1 SMMU 窗口（服务 =y/WiFi 停运车道）、WiFi（sheng 刚通，WCN IC 未核对）。ShengUfs 导出移植时顺带对拍。
   - **用户另提（下会话单列）**：forensic reader 的 Linux 内核（forensic-kernel.efi 13.2 MiB）带显卡驱动可精简（forensic 只需 UFS+USB ACM+gzip+sha256）——既减 ESP 又为固件腾位；与 boot_b 路线互补非替代。
1. ~~下载~~ **已完成**（sha 对拍 ✓）；充电通了再开工：IPSW 解包（KernelCache/DeviceTree/StaticTrustCache/Cryptex1,SystemTrustCache 四件）并逐成员对拍 OS dmg sha（`087-86698-044.dmg = 80f426d5…c92483`）。
2. 新建 `profiles/os/ios-16.7.16-20H392/`（仿 21H461 profile.toml 结构：os 元数据 darwin 22.x/xnu 无关项、[ipsw] 旧 pin、[kernel.prelink] 三值、trust-cache pins、ADT 身份 d22ap/iPhone10,3/t8015）。
3. `board.toml` → `os_profile = "ios-16.7.16-20H392"`。
4. **text fixups 清零起步**：23 条自加 displayless-kill 族全部是 21H461 VA，新 donor 全部作废；16 代用户态裸起，遇墙再按 §4.9 定点法逐个补（"遇一堵修一堵"支路纪律平移）。
5. 内核重链（prelink 对 20H392 KC）→ 出件；判读门 = 20H392 时代的既有证据形态（entropy 守卫日志/绿蓝条/panic screen）。
6. userdata 重建（16.7.16 root，Linux 可构建）。
7. **设备现态**：手机已回 nura 充电——下会话首刷 = **三件全刷**（boot_b 不动件 + recovery_a 新 payload + userdata 新件）。

**红线/纪律全数平移**：刷写四样、读回对拍、判读修正掩码+阳性对照、止损线（2-3 会话，卡死则再评）。

**r83 = 20H392 供给链闭环 + 全链出件（2026-10-29 会话四，设备仓 5a6d356；首刷三件套就绪）**：

- **①verified-root 横雷定谳 + 政策对重建**：`.cache/firmware/verified-root-{os,systemos}` 是 21H461 时代政策（volume 04769D26/8846FE78，os 窗 6,077,580,800B）——`NativeUfsBlock::configureVerification()` 按**容量匹配**选政策，20H392 窗口（5,635,081,728B/2,743,074,816B）无匹配 → 设备永不发布 → root start failed。stub 车道（count=0）同样死。正道 = **重建政策对**：21H461 旧件备份 `verified-root-21h461-backup-*`，`build-owner-policies-local.py` WINDOWS 表换 20H392 四元组 → **os 343,939 块 + systemos 167,424 块全 RSA PASS**；`build-verified-root-probe.py` guard 换新值（双 sha/块数/采样源件名 087-86593-047.dmg；os 尾块 1536 与 systemos 整块结构同构故采样逻辑零改）；dylib 从 `build/build/` 补位 `build/verified-root-host/`。
- **②Data UUID derive（上游只有 macOS diskutil 人工路径，Linux 扫描器自研补缺 = `temp/scan-apfs-volume-uuids.py`）**：方法 = APSB 卷超块扫描（4K 对齐**相对容器基址**、UUID@对象+0xf0、RFC 磁盘序），**金标 = 21H461 userdata 双窗**（os 17/17 副本 = 04769D26 ✓、SystemOS 3/3 = 8846FE78 ✓）。坑三枚：①OS dmg 容器基址在流内 0x4400（GPT 前缀）非 4K 对齐裸盘——对齐过滤必须 base-relative，第一轮用裸盘对齐把真超块全滤掉收进一堆 b-tree 假 hit；②UUID 读取偏移 0xf0 曾写成 magic+0xc0（=0xe0）差 16 字节连翻两轮（needle 全序搜索才定位）；③SystemOS 窗口 offset 手算 0x150100000 实为 0x150000000（错偏 1MiB 制造"头全零/NXSB 在 2.5GB"假象）——第三次付费"一切地址算术必须 python"。**终值：root_match_volume_uuid = `DADD93F8-DBBF-4B17-A1DF-0F11E64172F7`（20H392 OS 卷唯一卷，board.toml 已换）+ SystemOS cache 卷 = `96096B28-B6DC-4BC7-9D3A-2632A0FE53F1`（m2381_defconfig 新增 `CONFIG_XNU_OWNER_CACHE_VOLUME_UUID` 覆盖主树 patch 的 8846FE78 fallback；owner_defconfig 同步；18AB5557 seed 注释纠正 = 自家卷 donor 无关）**。CI 线三件（ci-owner-policy.sh/build-owner-systemos-policy.py/主树 build-owner-os-policy.py）同步换 20H392 身份——**CI 实证项（首绿前）**，本地不验证。
- **③④内核构建（新树全量）**：defconfig 重跑 ✓ → tree ✓（GopFramebuffer HEAD=e4c01b0 与 gitlink 一致，r78a 坑未复发）→ kernel。坑：①prepare-xnu-source GitHub API 403 = **GH_TOKEN 缺失**，解法 = 21H461 旧树 `.cache/sources/*.json` 四件直接 cp（两 profile 的 driverkit/trustcache tag 完全一致 10063.121.3/8796.101.5，缓存可跨 profile 复用）；②archive-20 display audit 需要 prelink 候选（两遍构建语义）= 从 21H461 旧树 `tree/build/xnu-prelink/native-owner-mobile-graphics-candidate.{macho,json}` cp 进新树同位（audit 基准 = 21H461 ABI json 自洽；16 代 IOMFB ABI 重审 = 后续主题）；③archive-12 两政策 guard 的 21H461 硬编码三处换新（前述）。
- **跨代 prelink 断裂 = 本轮最大发现（"换 16 代内核源码"大坑解除）**：首跑 prelink 挂 `Kernel target 0xfffffff0077cf004 has 0 native address candidates: ['_IORPCMessageFromMach']`。侦察（temp/audit-prelink-binding-gap.py）= 全闭包 387,416 位点看似 139,576 missing，**按 selected 口径（roots 31 组件+依赖闭包）实际只有 2 个失配位点、同一符号 `_IORPCMessageFromMach`（IOStorageFamily/IOHIDFamily 各一），ambiguous=0**——16 代闭包引用的 base 形态 RPC 入口在 17 代内核（xnu-10063）里实现被删（只剩 Reply 形态，id 校验 0x4da2b68d-only + 描述符步进语义不同，重绑不可行）。修复 = **恢复 16 代函数**：下载 xnu-8796.101.5 源码（tree/.cache/sources/，profile trustcache_tag 同源），profile patch `restore-iorpc-from-mach.patch`（IOUserServer.cpp 逐字恢复 16 代函数体；kIORPCVersionCurrent 两代同为 190615 语义等价；`extern "C"` 显式标注防 C++ mangling——第一版补丁缺 extern "C" 被编成 `__Z20...` 符号名对不上，used 属性防 dead-strip）→ **prelink 65,346 位点全绑通过**。用户问"要不要下载 iOS16 内核源码"的答案：**不需要换源码**，只需要一个函数（已下载 8796.101.5 存 tree/.cache/sources 备用）。
- **出件（全链已验）**：kernel.macho `fe8c7897…f8908`（entropy clean-apply ✓ + prelink 绑定核对 ✓ 两道门全过；`_IORPCMessageFromMach` 落 0xfffffff008931884）→ payload **`ee012043`**（96MiB，平铺 `out/m2381/payload-r83-ee012043.img`；payload.json 四样齐：ADT 86b6ced5 = d22 身份+新 root_match、trust eb1475f6 与 board pin 逐字节吻合（IPSW 重解 trustcache 对 63e01c03/9cc478fe 重新打包验证）、data seed d04a0153、skip_setup=false 红线）。
- **首刷三件套（下会话）**：boot_b←`ec1d518b`（r81 充电段件，ABL 尾字节容忍 = 实证项）+ recovery_a←`payload-r83-ee012043` + userdata←`userdata-root.img`（8.38GB，sha 2c2aca3f）。判读门 = r11 时代证据形态（entropy 守卫日志/绿蓝条/panic screen）+ 充电栈（BATTMGR GLINK/native_adsp_stats/充电电流）+ 20H392 用户态推进。回退件 = 整组 21H461 世代（21H461 政策对备份在案）。

**r84 判读工作单（2026-10-29 会话五开工；用户报"内核跑不起来"——首刷已发生，屏幕无内核迹象。纪律重申：屏幕状态从来不是判据，lsusb `1d6b:0104`（ACM）才是通道判据；黑屏不阻塞 reader）**：

**第一刀 = 设备现态三查（定分流，不猜）**：
1. `lsusb` 看 USB 枚举：fastboot（Google/18d1 形态 vendor）？ACM（1d6b:0104）？还是全静默？
2. **分支 A：fastboot 在** → 内核没起但 ABL 活着。先 `fastboot getvar` 核对 current-slot，然后读窗（见第二刀）——boot_b 件可能根本没被 ABL 接受（**ABL 尾字节容忍 = 本轮头号嫌疑**：boot_b 件 = boot.img 1.27MB + M2381FW 段 30.5MB，若 ABL 拒 image_size 之外尾字节 → FV 没加载 → 没有 UEFI → 没有 payload 投递 → 无内核。**实证法 = 回退刷纯 boot.img（同文件前 1,279,872B？——不对，纯 boot.img = package-m2381-boot.py 不带 --adsp-* 重新出件；或 dd 取 ec1d518b 件前 1,269,760B + 头），flash boot_b 单件后 reboot 看是否进 UniXNU 菜单**。这刀同时实证"充电段是死因还是无辜"。
3. **分支 B：ACM 在（reader 活着）** → 引导链到 payload 了，reader 菜单里选 reader 入口 → `acm-reader.py probe` → 三节点对拍（boot_b/recovery_a/userdata 前 N MiB sha，卡上已备）确认刷写落地 → `fetchchunk` 拉窗 → `read-m2381-ufs-log.py --all` 判读。注意：**reader 活着 = boot_b 的 UEFI + recovery_a 投递链全通，"内核跑不起来"的问题面收窄到内核本体/参数/投递细节**。判读点按优先级：①boot-progress=2 菊花是否画出（=y UEFI 帧缓冲路径）②entropy 守卫行（r11 形态：far 0x40 / g_crypto_funcs==NULL 崩 = 守卫失效或补丁漂移）③panic screen 文本（可诊断形态）④prelink 闭包行为（IORPC 恢复符号 @0xfffffff008931884 的运行时命脉——16 代 kext 首次调用它；崩点/REASON 行符号化按 §4 流程，re-tools 现役）⑤充电栈 BATTMGR/native_adsp_stats。
4. **分支 C：全静默（无 fastboot 无 ACM）** → 最深层：xbl/abl 层或供电。power+vol-down 强制进 fastboot；进不去 = 硬砖面（xbl 层，红线内不可修，联系既有恢复路径）。
5. **分支 D：设备在 nura 侧（NCM/devsh 可达）** → 用户刷完回 nura 了 → nura 侧先把分区读回对拍（三节点 dd|sha256sum，卡上 sha 表），确认刷写落地再定分流。

**判读门预注册**：
- **命中（内核起）**：窗里 entropy 干净 + 菊花/进度条出现 + 无 panic → 看 20H392 用户态推进多深（launchd/dyld/SEGV 族/REASON 行），逐项记录。
- **未命中（kernel 早死）**：窗里 SHLOG 槽都空/只有 header → 内核没起或起在极早期。判据 = slots_written 分代纪律（memory shlog-header-discipline）+ 早期 panic 文本（panic screen 若画出来了 = ABL FB 路径通，直接读屏上文本形态）。嫌疑优先级 = boot_b 尾字节（分支 A 实证法）> entropy 守卫 > prelink 闭包 > cmdline 983B（r81 新参数集在 20H392 内核上的接受度）。
- **特殊形态：SLEEP/halt**（r70 形态）非崩溃，别误判。

**下会话开工序**：三查定分支 → 按分支走 → 全程留档（窗文件 out/m2381/reader-r84-log.bin，判读回写本节 + 记忆）。**回退链随时可用**：纯 boot.img 回退（分支 A）→ 21H461 整组回退件（政策对备份 + fd43ee70 世代 userdata + 21H461 时代 payload 件）→ nura 回退链（红线 6，只在 nura 仓）。

**r84 首刷判读（2026-10-29，用户屏摄 `IMG_20261029_102213.jpg` 定谳——panic screen 直出，本轮免取证通道）**：

- **现象**：BootProgress live HUD（"live XNU - native 4K - no automatic reboot"）+ kernel console 镜像停在 panic。白字 = `failed to get mix-n-match policy: 2` + `frames`×12 + 标准 pages/free 尾注；HUD 头部 TTBR0=`8c5F08000`/TTBR1=`8c5F09000`（内核跑在高段 DRAM 窗）；UFS 行全零（requests=0 verified4K=0 root=0）= panic 在 root mount 前，时间轴一致。注：r84 纪律"屏幕不是判据"防的是"无迹象/陈旧 UEFI 画面"；本屏有内核 panic 文本（BootProgress 是内核组件，串只能来自内核 console 环）= 内核侧直接证据，足以定谳；lsusb/reader 三查仍可照做作交叉验证，不阻塞。
- **定谳 = r12 AppleImage4 mix-n-match 墙原样复现**：串唯一实例在 20H392 donor KC `com.apple.security.AppleImage4` __TEXT（§H：file offset 0x3e20262；panic+尾注形态逐字吻合）。换回 20H392 = 跨代配对复活 → r12 墙随之复活。**判读修正：§8 r79 段"那套墙当时已被驯服"对 mix-n-match 不成立**——当年 entropy 靠守卫驯服（r11→373f769），mix-n-match 未驯服、是 r13 换配绕开的（§I）。设备现停在 16 代 lane 的历史最深处（=r12 坐标；此之外全未开荒）。
- **连带正面五项**：①boot_b `ec1d518b` 尾段 ABL 接受——r84 头号嫌疑"尾字节不容忍"**排除**（分支 A 实证法作废）；②UEFI→recovery_a payload 投递链通；③entropy 守卫生效（r11 墙未复现）；④prelink 绑定可执行（内核起、16 代 kext start 已在跑）；⑤panic screen + HUD 帧缓冲路径通。充电栈判读（BATTMGR/充电电流）被此墙挡住——它在更晚的驱动阶段。
- **12 帧地址已收**（屏摄）：`0xfffffff0046e9f60 / 46e4b30 / 46f1420` 一簇 + `0xfffffff008857598 / 88545d0 / 885486c / 88538e8 / 8853804 / 89e1094 / 8962bec / 82807c8 / 83d24a4` 一簇（0x8931884=`_IORPCMessageFromMach` 同域）——符号化待做，串身份已定谳不阻塞。
- **下一步分叉（用户问"换 ios17 版本可以吗"，待拍板）**：
  - **甲（推荐）= 修墙续攻 16 代**：§H 预注册候选仍在案。第一刀 = ①re-tools 符号化 12 帧 + 在现役 KC（2ec297ba + prelink，kernel fe8c7897）定位 getter 调用点（r12 地基可复用：串@0x3e20262、_panic 导入槽 0x5cabb18、_img4if 22 槽已 dump，差 double-deref/blr 导入形态未破）；②按数据源分叉——getter 若读 ADT `/chosen/manifest-properties`（`UniXNU-m2381/scripts/make-m2381-adt.py:204` 现 present-and-empty）→ 补 AMNM 属性即活，零内核改动；源在 CSM/chip 观测 → OS profile text_fixups 定点守卫（§4.9 entropy 先例）。出件只动 payload → **只刷 recovery_a**（boot_b/userdata 不动），读回对拍照旧。
  - **乙 = 换回 21H461**：回退链完整在案（政策对备份 + fd43ee70 世代 userdata + 21H461 payload；boot_b 不用动——UEFI/充电段 donor 无关）。成本 = 重建 payload + 一次刷写；但回到 r77 原地，=y 卡点（displayless）未变，不产生新信息。
  - **止损线建议**：甲攻 1–2 轮。若 mix-n-match 修掉后紧出第 3 面同根因墙（entropy/mix-n-match 已是"同根因的两个切面"，§I 原话），= 16 代开荒税实锤 → 直接转乙并按 §7 =y 工作单续攻（r78 探针设计在案）。

**r85 = mix-n-match 墙第一刀实施完毕（2026-10-29 会话六续，用户拍板"开修"；设备仓 b91579e）**：

- **RE 全链（r12 地基全部换算到现役 macho）**：①屏上 12 帧 = runtime VA **恒等映射**（=macho 链接 VA；"slide −0x1000000" 只是 prelink 摆放约定，不参与换算——12 帧全部落位：帧 1–3 = AppleImage4 代码，帧 4–12 = 内核 `kernel_bootstrap_thread → mac_policy_initmach → KLDBootstrap::loadSecurityExtensions → OSKext::loadKextWithIdentifier → OSKext::load → OSKext::start`）②panic 串/邻居串（0x3e20262 等）= **link VA**（r12 记的"文件偏移 0x3e20262"数字相同但性质是 VA）③AppleImage4 代码在 `__PLK_TEXT_EXEC` 0x46e4XXX–0x46fAXXX；r12 的"double-deref 导入形态" = 静态绑定 PLT 槽（0x5cab828=`SecureDTLookupEntry`、0x5cab818=`SecureDTGetProperty`、0x5cabb18=`_panic`，全部用 nm 实名）④panic 点 = AP 实例化方法（0x46e98cc）内 `cbnz w0 → panic("failed to get mix-n-match policy: %d", ret)`，ret=2 来自虚表调用 `[self+0x38](self, x21, id=12)`：kext 本地观测注册表查 id 12 不存在（Qualcomm 无芯片驱动）→ 2⑤**DT 约定无跨代断裂**：`pexpert/pexpert/device_tree.h:129` kSuccess=1，与 kext 的 `cmp w0,#1` 吻合；`/chosen/manifest-properties`+BNCH 是查到 policy 之后的下一层，非本墙数据源。
- **修法 = TXM 短路单指令补丁**：该方法自带捷径（`[self+0x1a8] bit1` 置位 → 跳过整个 policy/manifest/BNCH 块，log "system contains a TXM runtime"，经 log 尾调用返回；外层环境对象循环对返回值**零检查**）。补丁 `0xfffffff0046e9924: tbnz w8,#1 (0x37080908) → b (0x14000048)`。脚本 = 设备仓 `scripts/patch-appleimage4-txm.py`（VA→offset 每次从 `__PLK_TEXT_EXEC` 段重 derive；原字不匹配拒绝写；sidecar `.patch.json`）。kernel **55fcee13**。
- **出件**：payload **`5bc0c023`**（96 MiB，`out/m2381/payload-r85-5bc0c023.img`；trust eb1475f6 + seed d04a0153 + skip_setup=false + panic screen；ESP 四样齐，ESP 内核副本已回读验证补丁字在位）。**两个构建坑**：①`--trust-cache/--data-ramdisk` 必须显式传（首轮漏 --trust-cache → trust_cache null + ADT 缺 TrustCache memory-map 条目少 52B，r56 同族）②ADT `platform-random-seed` 每次生成新随机值 → ADT sha 永远逐字节对不上旧轮（非回归）。
- **r86 判读预注册（只刷 recovery_a）**：①补丁生效标志 = kernel console 出现 **"system contains a TXM runtime"** 行 + mix-n-match panic 消失，kext start 推进到下一个 donor 墙（按 panic 屏文本判读；候选 = 其它环境对象方法的类似芯片查询、AMFI/mac_policy 域、SecureDT 层）②同墙复现 = 对象到达分支时位形态与假设不符 → **plan B** = `0x46e99dc` cbnz→nop（跳过返回检查）或方法尾部 `mov w0,#2→#0`（0x46eac18，全部缺失观测返回"成功零值"）③横雷备注：TXM 路径跳过了 manifest-properties/BNCH 获取，后续若在别处消费这些状态出现新墙，其证据会指向下一个消费者——按 panic 文本定位即可。回退件 = payload-r83-ee012043（21H461 整组回退链不变）。

**r86 状态（2026-10-29 会话六收尾）**：**recovery_a ← payload-r85-5bc0c023 已刷**（用户报"刷完了"，设备仓 23035bb；**dd 读回对拍未确认——判读开工第一件事先补**（`dd recovery_a | sha256sum` 对 `5bc0c023…cc05`），没读回前不算落盘证据）。下会话开工序：读回对拍 → 三查（panic 屏照片 / lsusb 1d6b:0104 reader / fastboot）→ 按上面判读门分流：TXM 行 + 墙消失 = 推进（新墙按文本定位）；黑屏/无 panic = reader 拉窗（r84 分支 B 流程，`out/m2381/reader-r86-log.bin`）；同墙复现 = plan B 出件。

**r85/r86 判读 + r86 第二刀实施（2026-10-29 会话七；用户先问换线，后拍板"继续修 ios16，出件我来刷"）**：

- **r85 判读定谳（屏摄 + 本机符号化闭环）**：**TXM 捷径生效实锤**——panic 形态全变（断言 panic → data abort），现役 kernel（`55fcee13`）补丁字 `0x14000048` 在位 → mix-n-match 墙过掉。新墙 = **AppleImage4 同 kext 描述符装载函数**：esr `0x96000005`（data read TF L1）+ far `0x18` + x19=0。崩函数 0x46e467c：三次字符串调用（0x46fa0d0）→ `bl 0x46f0e20` 得 env 对象（x0=0x5caceb0，活指针）→ `ldr x19,[x0+8]` = **NULL**（芯片观测注册表 Qualcomm 无注册，r84 id=12 同源）→ 0x46e46c0 `ldur q0,[x19+0x18]` 崩。调用方 0x46f1430（全局页 0x5cb14c0）；r84 九帧内核簇原样再现；pages/free 非零 + HUD 计数全零 = 仍 root mount 前。**0x46f0e20 文件态全零 = kxld boot 时按 relocation 回填的外部引用区**（静态分析到头，运行态行为由 lr/寄存器钉死）。工具留 `temp-r85-sym/`（symmap/disasm/strings）。**同根因第三面（entropy/mix-n-match/空注册表解引用）→ 预定止损线触发**；用户知情后拍板继续修。
- **r86 修法 = 零表指针单指令**：`0xfffffff0046e46b4: ldr x19,[x0,#8] (0xf9400413) → adrp x19,0xfffffff009223000 (0xf00259f3)`。该函数 = 芯片观测描述符装载（拷 0x30 字节进全局页 0x5cac32a + 两道计数 panic 护栏），**零表让全路径结构性干净**：描述符拷全零、`[x19+0x48]`=0 过 ≥0x11 护栏（0x46e46f0）、`[x19+8]`≠哨兵走 b.ne、env 二/三段读走真对象（0x46e473c ≥4 护栏与 0x46e4758 cbz 不受影响）。**否决两个备选**：跳到 0x46e4790 = 落格式化 panic 路径（0x46fa028 带 fmt+栈参，是 panic 非 log）；整函数跳过 = 丢副作用。零页选内核 `__DATA` bss 末页（vm 0x89ec000+file 0x44000 → bss 0x8a30000..0x9224000，紧邻 `__BOOTDATA`；demand-zero + kext relocation 不碰内核 bss + 远端最少被早 boot 对象占据）。脚本 = 设备仓 `scripts/patch-appleimage4-chipobs.py`（adrp 编码脚本内算 + 解码回读自检 + 原字校验 + sidecar）。kernel **72aa6fd6**（基 = r85 件，TXM 字 `0x14000048` 共存已验）。
- **出件**：**payload-r86-2032f813.img**（96 MiB，`out/m2381/`，sha `2032f813…e52b`；trust eb1475f6 + seed d04a0153 + chip-id 3645ad55d47e77c1 + panic screen；**ESP 内核副本回读双补丁字 PASS**）。配方 = r85 同款（`--kernel` 显式喂补丁件）。
- **r87 判读门（只刷 recovery_a；boot_b ec1d518b / userdata 2c2aca3f 不动；刷后顺带补 r86 读回对拍 `2032f813…e52b`；r85 欠账 `5bc0c023…cc05` 已被覆盖、作废）**：①**推进** = 原崩点消失，出现新 panic 文本或新崩点——第一候选 = 同函数第二消费 `[env+0x10]`（0x46e4734 → `[x9+8]` 计数 vs 4，b.hs 落 0x46fa028 **格式化 panic，屏上会有文本**，按文本定位下一刀）；第二候选 = AppleImage4 start 后续步骤或 kext 链下一家。②**原墙复现** = `[x0+8]` 并非 NULL 的假设错（屏上 x19 非 0 即证伪）→ 转真表指针方向。③回退件 = payload-r83-ee012043 / r85 件 / 21H461 整组不变。**战略备注**：上游 `feat/ipados27-gpu-parity`（arm64e + 26.5.2/27.0.1 双 profile + fileset 重绑定 + 原生 PAC，sheng 同 SoC 在研）已侦察入档，为换线候选——本轮继续修 16 代为用户拍板路线。

**r86 判读定谳 + 取证闭环（2026-10-29 会话八；用户已刷 r86 并进取证通道；"下个会话修"）**：

- **r86 屏摄判读 = 精确命中预注册第一候选，零表补丁实机生效实锤**：data abort @ `pc 0xfffffff0046e4738 / lr 0x46e4734`，esr `0x96000005`，**far `0x8`**，**x19 = `0xfffffff009223000` = 零页补丁原值出现在实机寄存器**（adrp 开火、零页可读）。崩点 = `0x46e4738 ldr x8,[x9,#8]`，x9 = `[env+0x10]` = **NULL**——同一 env 对象（x0=0x5caceb0 与 r85 同值）的第二个未注册指针。执行前沿从 r85 的 0x46e46c0 前移到 0x46e4738（+0x78 字节）：描述符拷贝、≥0x11 计数护栏、哨兵比较、`bl 0x46f9d94` 全部走通。调用方 0x46f1430、九帧内核簇与 r84/r85 逐帧相同。HUD root=0/procs=0 = 秒崩（用户观察吻合）。
- **取证窗判定 = 陈旧数据（用户纠正正确，我看错的第一处）**：拉取的 log.bin（fetchchunk SHA MATCH，`2698e6f9…`）= 337 槽/678s"过 root mount + launchd + 196 进程"的 boot——**不是 r86**。判据：TXM 行 0 命中、sheng-adsp/cmdline 标记 0 命中、coreduetd 在册（229 命中）而 liveactivitiesd 缺席 = **r44+ =y 车道禁用名单指纹** → 该窗 = r77 时代 21H461 最后一次过 root mount 的 boot。**机制**：BootLog recorder 是内核侧组件，r84/r85/r86 全死在 root mount 前没活到能写窗 → 窗口自 =y 时代从未被覆盖。**纪律沉淀：秒崩 boot 的证据通道只有 panic 屏摄；reader 窗要等 boot 活过存储初始化才有货；读窗第一步的 boot 身份核对（TXM/args/禁用名单指纹）先于一切解读**。取证件 + README 归档 `out/m2381/reader-r86/`。
- **r86 读回对拍 = PASS（欠账销账）**：reader shell `dd /dev/sde27`（recovery_a）前 96MiB 设备端 sha256 = `2032f81381596cbd0d91b62a699d33c41e7230420ee1f3ac3e859c45cd85e52b` = payload-r86-2032f813.img 逐字节吻合。
- **r87 修法预注册（下会话开工单，具体病灶+修法）**：`0xfffffff0046e4734: ldr x9,[x0,#0x10] (0xF9400809) → adrp x9,0xfffffff009223000 (0xF00259E9，同零页，delta 页数 0x4B3F 相同)`。后续路径静态推演已闭环：x8=[零+8]=0 < 4 过 0x46e473c 护栏 → str 0 → 0x5cac356 → `bl 0x46f0e20` → `[x0+0x30]` cbz 分支 = 干净返回（若 [x0+0x30] 非空走 0x46f9dac 尾拷贝也安全）。脚本 = 扩展现有 `patch-appleimage4-chipobs.py` 或新增同构脚本；基 = kernel-r86-chipobs.macho（三补丁共存）。风险预注：同一 env 对象的其余 NULL 指针槽（+0x18/+0x20…）可能在 start 更深处被别的方法消费——同族单指令推进，按屏摄逐个点名。
- **UEFI 引导界面充电侦察（用户需求"安排上"）**：上游 uefi-sheng 增量仅 5 提交（docs×3 + ShengBootMemoryDxe 换共享 RAM disk 驱动），无字体/充电新货；Font.h/EFIChargerEx.h 为老提交。"引导界面充电"的实体 = **原厂高通 UEFI 二进制驱动收编**：`Binaries/<device>/QcomPkg/Drivers/QcomChargerDxe/QcomChargerDxeLA.inf` + `ChargerExDxe/ChargerExDxe.inf`，LG mh2/caymanslm、华硕 i005d、联想 9707f/j706f 各包 DXE.inc 均此挂法。m2381 = SM8550，原厂 abl 内有同族驱动，收编路 = 从原厂 abl（设备 sda 分区红线外只读 dd 或 nura 固件收割）解出两 INF+依赖 → m2381Pkg DXE.inc 挂载 → FV 出件刷 boot_b。字体 = FrameBufferSerialPortLib Font.h（已在我们基座，非新货）。**独立车道，不挡 iOS16 修墙线**；待用户排期。
- 回退链不变：payload-r83-ee012043 / r85 / r86 均在 `out/m2381/`；boot_b ec1d518b / userdata 2c2aca3f 不动。

**r87 = 芯片观测计数指针补丁实施完毕（2026-10-29 会话九，设备仓 c477f48；出件待刷）**：

- **第二刀按预注册单刀落**：`0xfffffff0046e4734: ldr x9,[x0,#0x10] (0xF9400809) → adrp x9,0xfffffff009223000 (0xF00259E9，同 r86 零页，delta 0x4B3F)`。脚本 = 新增同构 `UniXNU-m2381/scripts/patch-appleimage4-chipobs-count.py`（adrp 编码+解码回读自检+原字校验+sidecar），链 = kernel-r86-chipobs.macho → **kernel-r87-chipobs-count.macho `ba4080fc`**（三补丁共存：r85 TXM `0x14000048` @0x46e9924 + r86 adrp x19 @0x46e46b4 + r87 adrp x9，三字全验）。
- **反汇编复核（capstone 全函数，机制用眼睛看）**：补丁后路径逐指令与预注册闭环吻合——零页全零描述符拷贝 → `[x19+0x48]`=0 过 0x11 护栏 → `[x19+8]`=0≠哨兵走 b.ne → `bl 0x46f0e20` 重取 env → r87 位点 adrp x9 → `[x9+8]`=0 过 #4 护栏（0x46e473c）→ 存 0 到 0x5cac356 → 0x46e4750 再取 env → `[x0+0x30]` cbz → 0x46e478c 干净 `ret`（非空分支走 0x46f9dac 尾拷贝，源 = 真对象，同样安全）。
- **出件**：**payload-r87-b93829e9.img**（96 MiB，`out/m2381/`，sha `b93829e9…7683`；kernel `ba4080fc` + trust eb1475f6 + seed d04a0153 + chip-id 3645ad55d47e77c1 + skip_setup=false + panic screen；ESP 四样齐 + initrd 行在；**ESP 内核副本回读 sha a92c8437 = payload_kernel_sha256 且三补丁字全 PASS**）。构建坑一枚：`-D` 等公共选项必须放子命令之后（`./unixnu payload -D …`，argparse 子解析器结构；AGENTS.md 的前缀写法在本版 cli 不成立）。ADT sha 逐轮变化 = platform-random-seed 每次新生成，已知非回归。
- **r87 判读门（只刷 recovery_a；boot_b ec1d518b / userdata 2c2aca3f 不动；刷前 sha 门 `b93829e9…`，刷后读回对拍）**：①**推进** = 0x46e4738 崩点消失（预期静态闭环：函数干净 ret，kext start 越过本方法）——下一证据形态按屏摄定位：同 env 对象其余 NULL 槽（+0x18/+0x20…，风险预注在案）被更深处消费 = 同族 data abort，寄存器/far 点名下一刀；或护栏翻落 0x46fa028 格式化 panic（屏上有文本）；或 AppleImage4 start 整体越过 → 内核推进（panic 形态换内容/换 kext）。②**原墙复现** = 不预期（零页可读性已被 r86 实机证明，x19=0x92230000 在寄存器）。③**证据通道纪律**：秒崩 boot 唯一证据 = 屏摄（reader 窗仍是 r77 时代陈旧数据，读窗先验 boot 身份，在案 `out/m2381/reader-r86/`）。④回退件 = payload-r86-2032f813 / r85 / r83 / 21H461 整组不变。

**r87 判读定谳 + r88 第四刀实施完毕（2026-10-29 会话九续，设备仓见 git log；出件待刷）**：

- **r87 屏摄判读 = 预注册分支①命中，第二刀实机生效实锤**：0x46e4738 data abort **消失**，整个芯片观测描述符装载函数干净 ret，kext start 越过该函数推进到下一个消费者。新证据 = **格式化 panic** `failed to get previous stage manifest hash: 2` + 两帧新地址 `0x46ed6a0 / 0x46f1820` + r84 九帧内核簇原样；HUD root=0/procs=0 = 仍 root mount 前（kext init 期）。同根因第 5 面（entropy 守卫→mix-n-match→观测注册表→观测计数→manifest hash），但 panic 给文本 = 可定点。
- **RE 全链（机制用眼睛看，capstone 全程）**：frame2 = `0x46f181c bl 0x46ecc14` = **nonce manager init**。函数入口先把 64 字节 hash 缓冲（sp+0x460）零填充（0x46ecc50），第一查 `40A0DDD2-77F8-4392-B4A3-1E7304206516:nonce-seeds`（0x46f06c0）ret=0；第二步经 env 对象 vtable `[x19+0x50]`（key 0xb）取 previous stage manifest hash 写入该缓冲，ret=2（Qualcomm 无 iBoot 链、无前级 manifest）→ `cbnz w0` @**0x46ecd68**（0x35004940）→ 0x46ed690 panic 块（fmt 0x3e2035d，0x46fa028=panic；frame1=0x46ed6a0）。**关键语境**：①姊妹输入 seeds 的 ret==2 有 Apple 自带容忍路径（"nonce manager: error reading seeds: %d" 记日志继续，0x46ecd18）；②hash 缓冲零值即发布值（尾部 0x46ed308 把它发布到全局 0x5caea42），长度不合路径全是记日志继续非 panic；③串池与 r12 mix-n-match 串同池（0x3e20308 "…continuing because mix-n-match is permitted"——0x46e98cc 政策函数内 0x5cacef0 旗标门，r85 TXM 捷径跳过的赋值面，本轮未消费）。
- **修法 = `0xfffffff0046ecd68: cbnz w0 (0x35004940) → nop (0xD503201F)`**：manifest hash 查询失败时沿用缓冲零值继续（="无前级 manifest"语义，与姊妹输入容忍同构）。脚本 = 设备仓 `patch-appleimage4-noncemgr.py`（原字校验+sidecar）。链 = kernel-r87-chipobs-count → **kernel-r88-noncemgr `8e807ef5`**（四补丁共存四字全验）。
- **出件**：**payload-r88-c932a128.img**（96 MiB，`out/m2381/`，sha `c932a128…`；kernel 8e807ef5 + trust eb1475f6 + seed d04a0153 + chip-id 3645ad55d47e77c1 + panic screen；**ESP 内核副本回读 sha d2eea5ed = payload_kernel_sha256 且四补丁字 PASS**）。
- **r88 判读门（只刷 recovery_a；boot_b ec1d518b / userdata 2c2aca3f 不动）**：①**推进** = panic 文本换新/崩点换址——按屏摄点名下一刀；候选 = nonce manager 下游消费者（全局 0x5caea42 零 hash 的下一个读者）、AppleImage4 start 更后步骤、kext 链下一家；若内核推进过 root mount，证据通道自动升级为 reader 窗（秒崩纪律失效）。②**原墙复现** = 不预期（ret=2 语义已实证）。③回退件 = payload-r87-b93829e9 / r86 / r85 / r83 / 21H461 整组。

**r88 判读定谳（2026-10-29 会话九续二，屏摄 `IMG_20261029_204307.jpg`；**墙的性质换挡**，修墙暂停待战略拍板）**：

- **推进实锤**：r88 nop 生效——manifest hash panic 消失，kext start 又推进一段。新墙换挡：**数据 abort 崩在内核本体** `pc 0xfffffff0086af538`，esr `0x96000004`（L0 translation fault，read），far `0x455f5458455464f7` = **ASCII**（x0=`0x455f545845545f5f` = `"__TEXT_E"`，LC_SEGMENT_64 segname 首八字节，far = x0+0x598）。
- **全链符号化（LC_SYMTAB 187k 符号，内核自源码构建全量在档）**：崩点 = **`_csproc_get_blob`**（`ldr x8,[x0,#0x598]` = proc->cs_blob 槽，x0 却是载入命令指针）；调用方 = AppleImage4 回调 `0x46fc3f8`（sret 函数：`ldr x0,[x0,#8]` 取 segname → 导入桩 0x4718278（槽 0x5cb5aa0，kxld 回填）→ `csproc_get_blob(垃圾)`）；上游 = **`OSKext::start(bool)+0x370`** 的回调派发点（`blr x22` 前带 callee∈kext 文本范围校验，帧 0x88545d0=`OSKext::load`/0x88538e8/0x8853804=loadKextWithIdentifier 簇原样）。
- **定性 = 墙类换挡**：前五面（r84-r88）= "芯片观测/manifest 注册表为空"类，单指令可语义闭合；本面 = **内核/kext 边界契约级**（回调参数约定或 vtable 槽位跨代漂移——16 代 kext 期望 vs xnu-10063 内核，x0 应为对象实为 mach-o 载入命令）——正是 r83 预注"16 代 ABI 重审=后续主题"的账单开始到期的形态。此层级非十分钟单指令，且同类边界在 donor 用户态+kext 里数量庞大。**修墙动作暂停，等战略拍板**（批量静态地图 / HackiOS 开采 / QEMU 立项 / 换线，见会话记录）。
- **坑一枚（工具债）**：本 macho 的内核段与 prelink 段**文件区间重叠**（`__DATA` file 0x19e8000-0x2220000 与 `__PRELINK_TEXT` file 0x218b000-0x294f000 重叠）——串文件偏移 0x21e335d 双映射（kernel __DATA 0x91e735d 与 kext cstring 0x3e2035d 皆是），按 VA 找串必须用 `__PRELINK_TEXT` 口径（kext cstring 真身），否则 xref 扫描错段全零。

**引导界面充电 = 转上游待办（2026-10-29 用户拍板"等上游完善"，本会话侦察结论存档）**：现役 FV **已含整套充电驱动**（mars Binaries：ChargerExDxe + QcomChargerDxeLA + PmicGlinkDxe + UsbPwrCtrlDxe，depex 全 TRUE 无条件派发）——非"缺驱动"问题；嫌疑点 = GLinkDxe 在 DXE.inc 被注释（nura 遗产，APRIORI.inc 却仍在列 = 潜在不一致）与 mars 参考板配置 vs m2381 板级差异；本地收割面 `artifacts/vendor/meizu20-uefi-mods/`（166 件）已有 OEM 充电全家桶（QcomChargerDxeLA/ChargerExDxe.efi+.depex）可作替代 blob；stock 侧 off-mode charging 实体在 XBL 域（nura XBL-RE 在档），ABL 有 `oem enable/disable-charger-screen`/`off-mode-charge` 变量面。上游 uefi-sheng 有动作时按 mh2/i005d/9707f 模板对照重启此车道。

**"不打地鼠"三层方案（2026-10-29 会话九收尾，用户问"有没有系统性的修法"，待拍板）**：r88 判读定谳后墙类换挡（注册表空类 → 内核/kext 边界契约类），修墙冻结。三层 = **第 0 层（当天，零构建）**：21H461 已知良好配对回归当控制组——payload-r77-93854822.img + userdata fd43ee70（本地 sha 已验）两刷即达，r77 实证 = 内核+prelink+kext+root mount+launchd+196 进程+SB 深度推进全通，证明"内核能不能跑"与 20H392 六面墙无关；已知代价 = r77 件不含充电栈（r81 后才有），补法（半天）= board.toml 切回 21H461 profile + 现役内核源重建（r82/r83 反向全程有脚本 + verified-root-21h461-backup 在案 + 整棵 ipados-17.7.11-21H461 构建树健在）。**第 1 层（本周，天级）：QEMU 立项** = 不打地鼠的仪器（每墙小时级→分钟级 + gdb 全量观测）；首选标的 = 21H461 配对攻 displayless 单根因（IOMFB/FDR 注册链在可控硬件上全量可见）；路径 = 先开采 HackiOS QEMU 配置（同 donor iPhone10,3 同代 16.5 已跑通）→ 载体优先"精简 FV 进 QEMU"（Mu 基座 QEMU 血统 + DarwinBoot 吃 payload.img）。**第 2 层（战略终局）：上游 26.5.2/27 arm64e 线**（displayless 在该线不存在 = sheng 同 SoC 真 display+GPU；施工单在档 upstream-ios27-arm64e-lane；QEMU 复用为预检台）。20H392 线处置 = 冻结（boot_b ec1d518b 不动，回退件全在案）；QEMU 批量审计（导入桩×内核符号全量对拍）若清干净 ABI 墙可复活，否则归档"已证伪的近路"。

**iOS16 vs iOS27 两线重估（2026-10-29 会话十，用户拍板"就是想跑ios16…或者开始考虑ios27，看看上游有没有更新"）**：

- **上游实况（git fetch 实测）**：UniXNU main 仅 +1 docs；**feat/ipados27-gpu-parity +54 commits**（GPU 深水区：metal2vulkan 0dfdbcd/固定 Turnip cache identity/预先编译 pipelines/Adreno740 BO 管理与 SMMU fault termination/ArmSmmu4K bump/GopFramebuffer kept surface mappings）= sheng 同 SoC 显示与基础面已通、正打 GPU；uefi-sheng 零新增（现役基座 0eb4514 已最新）。
- **HakiOS 重大情报（10-24 评估后五天，github oosexgang，WebFetch 实测）**：xnu-8796.121.2 + stock iOS 16.5 用户态 + iPhone donor，**真机 POCO X5 Pro（redwood/SM7325）已到锁屏+主屏+控制中心+原生 app+触摸+8 核+Adreno GLES(Mesa)+扬声器/麦克风/电量 UI+Wi-Fi+振动+/private/var 持久+Sileo/Procursus/SSH**；QEMU（virt）也到锁屏（qemu/run-ios.sh，2-3min）；未通 = 蜂窝注册/蓝牙/**本内核下充电**/Metal（仅 GLES）/相机/SE·Apple 账户。移植模型 = devices/<name>/ 目录制（已双设备：SM7325 + MT6768 MediaTek）；真机装载 = fastboot RAM 加载（"nothing flashed"）；1.9k stars/632 commits/日更/APSL2.0 合规在档。
- **判读**：①"iOS16=跨代 ABI 打地鼠"的死结有了出路——**同代内核基座（xnu-8796）现成且在真机 Qualcomm 上验证到主屏**，ABI 漂移类墙由构造消除；②他们在 QEMU 跑通用户态 = Apple 基础设施缺失墙（我们 r84-r88 撞的同族）他们必有替身/注入解法可开采；③m2381 适配面 = SM8550 板级包（stock FDT/DTS 证据 + m2381Pkg UEFI 资产复用；红线5 → 须走我们的 UEFI boot 链投递，不能照搬其 fastboot RAM 模型）；④风险 = 项目 5 天新、单真机设备、SM7325→SM8550 平台差、GPU 栈不同（他们 Mesa/freedreno vs 上游 Adreno 原生）。
- **两线施工概要（待拍板主线）**：**A = iOS16@HakiOS 基座**：克隆开采（QEMU 先跑通 qemu/run-ios.sh 验工具链 → 开采 SM7325 包与 Apple 基础设施替身解法）→ m2381 devices/ 包（我们的 FDT/常量）→ boot 路径决策（m2381Pkg UEFI 载其内核+DT）。**B = iOS27@上游**：rebase 到 feat/ipados27-gpu-parity → 双 profile（先 26.5.2-23F84）+ ADT j820ap + fileset 重绑定/原生 PAC（上游已落地）+ 19 驱动仓；bump 54 commits 全是红利。两线共用：m2381Pkg/board 资产、QEMU 仪器。

**"iOS16 匹配内核拿来用"可行性侦察（2026-10-29 会话十续，用户拍板"不想弄 hakios，先把 ios16 的匹配内核拿来用"；克隆 `research/hakios-xnu-kernel` 实测）**：

- **内核仓形态 = 完美贴合我们的模型**：非整树 vendored，而是 `xnu/hakios.patch`（**3,239 行、73 文件**：bsd 29/osfmk 22/iokit 7/pexpert 4/makedefs 3/libkern 2/config 2）+ `files/` 新文件 + `apply.sh`（从 apple-oss-distributions/xnu GitHub 拉 **xnu-8796.121.2 tag**（BASE c54f35ca）打补丁成 xnu-src/ git 树）。Linux 原生 clang + theos cctools 构建，`xnu-build.sh` = Apple 原 Makefile 系统（MACHINE_CONFIGS=BCM2837 通用 arm64 机配置，DEVELOPMENT 配置，12 线程约 1 分钟出核）。
- **关键发现（必须知道的哲学差）**：HakiOS = **去 kext 化模型**——无 prelink/kernelcache 机构；驱动用 Mini* 内置替身（MiniIOMobileFramebuffer/MiniAppleKeyStore/MiniIOSurface/MiniIONetworkStack/GenericARMPE + libkern/amfi stub + kern_minimac.c = AMFI/Sandbox stand-in，"nothing is signed here"）；wifi = ath11k 对象直链内核；用户态 = IPSW 取件 + 自组 rc.jobs + 注入。**他们的 16.5 用户态在 AMFI stub + 无签名下跑到主屏**。
- **三路线定谳**：①字面整拿 = 接他们的去 kext 哲学 = 变相"弄 hakios"（用户已否决）+ 充电/显示全要按 Mini 模式重写；②纯自力（UniXNU 59 补丁重放 8796）= 账单最大且无先例；③**推荐 = 内核基座置换**：整 patch 拿来当 8796 地基（平台/调度/bsd 层已被他们在真机调通），20H392 kext prelink 与我们组件按需逐步叠上去、Mini 替身逐步换真——我们的 UEFI/boot 链/设备仓/profiles/prelink kit/root 全不动。
- **前置验证点（Phase 1 活）**：a) 121.2 vs 101.5 tag 差 × 20H392 kext 绑定（同 8796 大版本，r83 的 selected 口径审计工具可实测）；b) prelink kit × 他们的树（同 Apple tree 布局，kit 应能直接吃）；c) AMFI stub 与我们签名用户态的共存策略（内核先跑不依赖此项）；d) 组件移植账单（ADSP 充电/ShengUfs/entropy guard 在 8796 重放；entropy 墙可能随 8796 corecrypto 序自然消失）。
- **施工序**：Phase 1（1 会话）apply.sh 出树 → kernel.py/defconfig 适配 → 裸核起（串口/HUD）；Phase 2（1-2 会话）prelink 20H392 KC + 三值 derive + 组件移植；Phase 3（1 会话）payload + 上机判读。许可证 = APSL（kernel patch），研究用途与仓内现状一致。

**iOS27 线开工（2026-10-29 会话十定案：用户拍板"保守一点，用 unixnu 方案，开始搞 ios27"；HakiOS 基座否决归档；本会话只做勘测+分支准备，施工下会话开始）**：

- **重大利好（勘测实测定谳）**：上游 `feat/ipados27-gpu-parity` 的 `profiles/os/ipados-26.5.2-23F84/profile.toml` **自足且 donor = iPad17,3/j820ap/chip 0x8142/board 12 = 与我们目标一致 → m2381 大概率直接复用 profile，零 profile 编写**。profile 内含：IPSW CDN URL（23F84）、prelink 全套（emit_prelink --fileset-reference、kernel 路径、KC sha、slide -0x1000000、early_security_init=AMFI 0xfffffe0008f13c80、roots 31 组件）、ADT 身份、六个 Apple 源 pin（IOStorageFamily/IOHIDFamily/libplatform/libdispatch/AvailabilityVersions/libpthread）、userdata windows、root 卷 UUID+镜像摘要（os 546DFF4A / SystemOS 7A23BAC7）。同分支还有 27.0.1-24A446 profile 备用。
- **分支手术（已就绪）**：`m2381-ios27` 已建（= m2381 tip 5566b34），备份 = `m2381-backup-20261029`；rebase 已试跑并**中止在已知点**（过夜不留半成品）。冲突面实测 = 仅 9 文件重叠，其中实质三类，处置已定：①**gitlink 类（DarwinBoot 等）→ 一律取上游 pin**（arm64e 支持在上游；循环脚本已写好：`git ls-tree upstream/feat/ipados27-gpu-parity -- <path>` 取 160000 pin `git update-index --cacheinfo`）；②**scripts/unixnu/payload.py → 双保留合并**（HEAD 的 seal-source-crypto-kernel.py 块（source corecrypto POST 封印，arm64e 必需）+ 我们的 `UNIXNU_DEVICE=str(ctx.device)` env 透传，合并形态已在案）；③**profiles/os/ipados-17.7.11-21H461/profile.toml → 取上游版**（我们的 212 行 displayless fixup 史全死，史在 git）。kernel.py 的 bootstrap 健壮性补丁应干净落。
- **下会话开工单（按序）**：①重跑 rebase 循环（`git rebase --onto upstream/feat/ipados27-gpu-parity upstream/main m2381-ios27` + 上述三类处置，约 15 分钟机械活）→ ②check-m2381（device 仓独立，先不动 board.toml）→ ③board.toml `os_profile = "ipados-26.5.2-23F84"` + **defconfig 行生**（m2381_defconfig 加 arm64e/fileset/native-pac 项，参照 = 上游 docs/devices/sheng.md 指向的 sheng 设备仓 defconfig）→ ④IPSW 23F84 下载（URL 在 profile；`./unixnu ipsw fetch` 或直下，7-9GB 级，早 kicked off 好）→ ⑤defconfig → tree → kernel 第一遍（GH_TOKEN 记得带；两次构建语义照旧）→ ⑥verified-root 政策对 + Data UUID derive（APSB 扫描器/政策构建器全线现成，换 23F84 身份）。boot_b 预期不动（sheng 同基座已在跑 26.5.2 = m2381Pkg 的 0eb4514 基座原生支持）。
- **冻结面**：20H392/iOS16 线冻结（r88 为终点，回退件全在案）；HakiOS 内核基座评估归档（STATE 上节，若 iOS27 线遇阻可重启）。

**iOS27 线开工轮 = rebase + 换配落地（2026-10-29 会话十一，六步工作单之①②③完成；主树 tip `c2bfe13`，设备仓 `39a8ef7`；备份 `m2381-ios27-pre-rebase-20261029`=rebase 前 33b50fe）**：

- **①rebase 完成**：`git rebase --onto upstream/feat/ipados27-gpu-parity upstream/main m2381-ios27`，233 个工作区提交全落 feat tip `e5f729d`。停点四起全按既定处置：DarwinBoot gitlink（取 feat pin `3e3a8a96`）、GopFramebuffer gitlink ×2（取 `90b64efc`；其中纯 pin bump 提交 b56f1ca 用 `--allow-empty` 保史，git 另自动 drop 一条纯 bump 29998480 内容已上游）、payload.py = env 行双保留合并（我方 `UNIXNU_DEVICE` 透传 + feat 的 seal-source-crypto-kernel 块；坑一枚 = python 替换结束索引截短，冲突标记行尾巴残留成语法错误，ast.parse 门当场拦截）。**新教训（当日付费）**：③"21H461 profile 取上游版"这条 rebase **零停点静默通过**——自动合并把我们 23 条死 fixup 史拼回了 feat 版之上（+213 行），是**净差验收**（`git diff feat..tip`）才抓出来的，清理提交 `c2bfe13` 取上游版修正。**规矩入档："取上游版"类冲突处置不能只看 rebase 停点，必须看最终净差**。净差终验 = 58 文件全工作区资产（docs/package/patches/profiles+scripts 三件），gitlink 19 全对齐 feat，check 4 ok。
- **③换配完成（设备仓 39a8ef7）**：board.toml `os_profile = "ipados-26.5.2-23F84"` + root_match→`546DFF4A`（profile 自足值）+ defconfig `OWNER_CACHE_VOLUME_UUID`→`7A23BAC7`（profile systemos_volume_uuid）+ check 身份断言→iPad17,3/23F84/j820ap/0x8142/12；check-m2381 4 ok 0 fail；`./unixnu status -D ../UniXNU-m2381` 全链解析通过 = **arch 层自动选 `profiles/arch/arm64e`** + SoC feed `43da0d56`（feat 的 xnu-12377 兼容 pin）+ 63 条 xnu-12377 patch series（native-pac-keys #20）+ 11 组件齐；`./unixnu defconfig m2381_defconfig` `.config` 落位零报错。**机制勘定 = arm64e/fileset/native-PAC 不进设备 defconfig**：profile `[os] arch="arm64e"` 选架构层，`profiles/arch/arm64e` 自带 common.config + 全套 series（native-pac-keys/arm64e-threaded-starts/fileset-unmapped-call/native-security-init/trustcache-runtime-layout/offline-owner 全家）；board 的三层 series 数组随 profile 自动换装。trustcache 8796.101.5 源缓存 json 两代同 tag 已复用预置进新树 `.cache/sources/`。
- **④IPSW 供给单（用户自下载，进行中）**：iPadOS **26.5.2 / build 23F84**，`iPad17,1,iPad17,2,iPad17,3,iPad17,4_26.5.2_23F84_Restore.ipsw`，URL 在 profile；**全 zip sha256 = `f7f55fa36097a0c8f8784eefcd9eabfce3a1c55ecd470f6885f4862a9446f9c3`**（sheng.md evidence 实测值）；喂法 = `./unixnu <步> --ipsw <本地路径或URL>`（或 UNIXNU_IPSW env；成员按 HTTP range 读取，缓存 `out/ipsw/23F84/`）。
- **预期管理（sheng.md arm64e-os 节实测档，2026-10-29 读）**：该节 status = **`planned`**——sheng 自身 **fileset 出件未通**（"no fileset prelink output exists yet, so the kernel has not booted"）、全世界此线尚无 boot 实证；23F84 KC 实为 xnu-12377.**122.8**（公开源 121.6 最近）；23F84 dyld cache 含 SME 代码而 SM8550 无 SME（run-time feature 选择假说未证实）；sheng_ipados26_defconfig@4ed86f6 **已被上游回收不可达**（"not our ref"，本地 UniXNU-sheng origin/main 仍 21H461 态）——参照物只剩上游通用面（我们已按其文档自洽行生）。
- **遗留（下会话开工单，按序）**：⑤ fetch→tree→kernel 第一遍——**GH_TOKEN 先导出**（本 shell 无，r83 403 坑；driverkit tag 12377.121.6 缓存无法复用必须真拉）→ `--ipsw` 喂 IPSW → 两次构建语义照旧；⑥ board `[payload] trust_cache_sha256` 三值换 23F84（**现仍挂 20H392 三值 = payload 步 fail-safe 拒装的门，不是雷**）+ verified-root 政策对重建（build-owner-policies-local.py，23F84 四元组）+ Data UUID derive（APSB 扫描器现成）；⑦ payload 前板 args 精简拍板（现挂 21H461 时代 983B 诊断队 vs sheng.md arm64e 基线 `-v serial=3 debug=0x14e kt_fixed_zones=120 -no_slto_panic`；cmdline 1000B 上限）。回退链不动：21H461/20H392 全组 + boot_b ec1d518b。

**预期管理更正（同日深夜，用户转 QQ 群情报 + fetch 证据）**：上文"全世界此线尚无 boot 实证"**作废**——上游真机已在跑深水区：feat 分支 10-29 当天提交 = "point at the lock, leaf-walk and 8-bit framebuffer changes" / "Install-and-boot-only Data store" / turnip GMEM 渲染，10-25 起有 green bar diagnosis + automated boot testing 文档；用户 QQ 群情报 = ipados27 已跑起来、上游表示可以开刷，与提交流完全吻合。**下载决定不变 = 26.5.2/23F84**：supported 表当前配对仍是 SM8550↔26.5.2（"ipados27"是分支名口语，非 27.0.1-24A446），两 profile 同 commit 7b02190 建立。文档面滞后（arm64e-os 节仍 planned）不矛盾。对 m2381 步骤零变化：开刷靠自己的三件套，仍按 ⑤⑥。

**下会话开工单（会话十二，2026-10-29 深夜定稿；用户 IPSW 下载中；上游已确认 iOS27 全部在 feat/ipados27-gpu-parity = 我们现基座，无需动作）。前置 = ①用户给 IPSW 本地路径 ②`export GH_TOKEN=<宿主凭据>`（无 token fetch 必 403，r83 实证）**：

- **步 0 基座核对（2 分钟）**：`git fetch upstream` 看 feat tip 是否又动（本单落定于 `6ad1297`）；我们基座 = e5f729d+1（差一条 AdrenoMetal bump，ADRENO=n 不消费，可选 `git rebase upstream/feat/ipados27-gpu-parity m2381-ios27` 收齐，非必需）。`sha256sum <IPSW>` 对拍 `f7f55fa3…f9c3`。
- **步 1 fetch**：`export UNIXNU_IPSW=<路径>`；`./unixnu fetch -D ../UniXNU-m2381`。预期 = 19 gitlink 迁 feat pin + xnu-12377.121.6/driverkit 12377/六 Apple 源 shallow-fetch + unixnu.lock 写出。坑 = GH_TOKEN（403）；driverkit 12377 缓存复用不了必须真拉。
- **步 2 tree**：`./unixnu tree -D ../UniXNU-m2381`。预期 = 63 条 arch series + os zzz-boot-progress + SoC feed 43da0d56 补丁全应用零 fail（已核实：主树根 patch entropy/iorpc-export 仅 20H392 冻结 series 引用，arm64e 车道惰性）。
- **步 3 kernel 第一遍**：`./unixnu kernel -D ../UniXNU-m2381`。两次构建语义照旧（archive-20 audit 需 prelink 候选）。门 = fileset 重绑通过（emit_prelink --fileset-reference 对 23F84 MH_FILESET 输入）。**预期主战场 = fileset 出件**（sheng 自身没跑通的第一关）；坑预注册：a) 11 组件 fork 编译面 vs xnu-12377 头漂移（改组件必须 submodule commit+gitlink bump 同轮，r78a 规矩）；b) kext 直跳内核由 fileset-unmapped-call stub 兜底，关注其 report 行计数。
- **步 4 政策对 + trust 三值（kernel 绿后）**：①IPSW 解包四件逐成员对拍 profile 针（kernelcache_sha256 `0f7d50ac…`）；②board `[payload] trust_cache_sha256` 三值换装（现挂 20H392 三值 = payload 步 fail-safe 门非雷）：StaticTrustCache sha + Cryptex sha + make-m2381-trust-bundle.py 重打包 bundle sha；③verified-root 政策对重建：build-owner-policies-local.py WINDOWS 表换 23F84 四元组（os 546DFF4A/b4e40913…、SystemOS 7A23BAC7/eadb838d…，profile 自足）→ 全 RSA PASS。
- **步 5 payload + args 拍板（用户参与）**：args 两案——**甲（推荐）= sheng arm64e 基线裸起** `-v serial=3 debug=0x14e kt_fixed_zones=120 -no_slto_panic` + 最小必备（native-ufs-root=1 / sheng-pmgr=2；充电栈 sheng-adsp/battery 本轮不背，变量最少）；乙 = 21H461 诊断队砍到 ≤1000B。payload 红线：`--unique-chip-id 3645ad55d47e77c1` + `--trust-cache/--data-ramdisk` 显式传（r85 坑）+ 不带 skip-setup。**userdata 必重建**（`./unixnu userdata`，26.5.2 root，Linux 可构建）。
- **步 6 首刷（拍板后）**：三件套 = boot_b `ec1d518b` 不动 + recovery_a 新 payload + userdata 新件；刷前 sha 门 + 读回对拍。判读门 = ①内核起（entropy 行/native-pac-keys 每 boot 每核键编程行/fileset 绑定计数）②arm64e 用户态推进（launchd/dyld，SME 选择假说的实机答案）③失败按 panic screen 文本定点——**注意 23F84 KC = arm64e stripped MH_FILESET 无 LC_SYMTAB，r88 的 LC_SYMTAB 符号化路线作废，要走 dyld cache/fileset 表路线**。回退链 = recovery_a/userdata 随时回 20H392 组（r88 c932a128）或 21H461 组。

预期里程碑 = 本会话拿到 26.5.2 全链三件套可刷状态。

**会话十二执行记录 + 会话十三交接单（2026-10-30 凌晨收尾，用户令暂停待上游 iOS 27 合入 main 更新）**：

- **已完成（六步单之④①②③-大半）**：IPSW 23F84 sha256 `f7f55fa3…f9c3` 对拍供给单逐字 ✓（10.44 GB，`research/ipsw/`）；步0 基座核对 = feat tip 又 +2（`87bf1f9` AdrenoMetal bump，ADRENO=n 不消费，未收）；步1 fetch 全绿（GH_TOKEN = `GH_CONFIG_DIR=/home/ri/.config/gh gh auth token` 从宿主 keyring 取，hosts.yml 本体无 token；git 通道免 403，仅 prepare-xnu-source.py 的 GitHub API 要 token）；步2 tree 63 series 零 fail；**步4 反而超前完成**（见下）。
- **主树提交 `fe9c792`**：kernel.py 的 `UNIXNU_SDK_VERSION`/`DARWIN_TRIPLE` 从硬编码 17.0 改为随 profile 派生（arch ← profiles/arch/*/profile.toml，ios_version ← board [os]）——组件档重放编译记录的 --target 必须与 profile 的 -m*-version-min 一致，上游 clang-24 对 override 以 -Werror 拒收。
- **设备仓提交 `f5ad4ad` + `9834800`**（细节见提交语）：sm8550 feed 本地化（`local = "soc/sm8550"`，唯一改动 = smp patch 只留 start.s Affinity1 hunk）；4 参 `__ptrauth` 兼容（Apple clang only 的 "authenticates-null-values" 串；设备层 patch 令 `_AUTH_NULL` 族在 Linux LLVM 下降级为无符号=功能安全方向）；三个部署版本自检泛化（0x110000 → UNIXNU_IOS_VERSION 派生）；**archives 单对齐 sheng arm64e 已证子集**（摘 storage-bindings=190槽 21H461 pin+arm64e AUTH vtable r_type11、verified-root stub 双件、VirtioMmioBlock+transport、mockfs、legacy-display=23F84 无 ABI fixture 上游仅 24A446、sheng-platform-power=PMGR 已随 fileset roots prelink）；defconfig 重塑 = sheng_ipados26 形态（CRYPTO_BOOTSTRAP="source"+KERNEL_CHAINED_FIXUPS+SOURCE_STORAGE=y，NativeAES/VirtioBlock/Mockfs/VerifiedRootProbe/LegacyDisplay/ShengPmgr=n，**OWNER_ROOT_AUDIT/TRUST=y 复位**=BootProgress 守卫要求+sheng 对齐）。
- **步4 超前完成（构建强拉提前：build-sheng-source-storage.py 要 verified-root-23F84 政策对）**：①AEA 双窗解密——本机无 macOS /usr/bin/aea，**装了 blacktop/ipsw CLI v3.1.739（/tmp/ipsw，纯 Go AEA 实现）**，`ipsw fw aea <f>.aea --key` 取键 + `--key-val` 解密；OS dmg sha `b4e40913…` 与 SystemOS `eadb838d…` 与 profile [root] 双针逐字命中，已按 ipsw.py 缓存 sidecar 格式摆进 `out/ipsw/23F84/*.dmg(+.json)`；②**userdata-root.img 13.02 GiB 出件**（`out/m2381/ipados-26.5.2-23F84/m2381/userdata/`，OS 窗 @0x100000 = 8753512448 B，SystemOS @0x209d00000 = 5224005632 B）；③**政策对双窗全 PASS**（build-owner-policies-local.py WINDOWS 表已换 23F84 窗——**坑重踩一次：offset 手算错 1 字节级，正确值 0x100000 / 0x209d00000=8754561024，勿再手算**；os 534272 块 + systemos 318848 块逐块 RSA；双 395B trusted-key 同锚）；政策对已摆 `tree/build/verified-root-23F84/{OS,SystemOS}/`。
- **步3 墙账本（本会话 12 面，全部已破，除最后一面）**：①feed smp patch hunk1 上下文消失→本地化 ②sheng-platform-power patch 与 os 层 zzz-boot-progress hunk 耦合→patch 留 archive 摘+CONFIG_XNU_SHENG_PMGR=n 编译掉调用点 ③4参 __ptrauth→设备层 fallback ④GH 403→keyring token ⑤firehose 编译吃陈旧 fakeroot 头→rm fakeroot 重装 ⑥组件档 target 17.0 vs 26.0 -Werror→DARWIN_TRIPLE 派生 ⑦archive-07 平台电力 340 槽 21H461 pin→摘除 ⑧storage-bindings 190 槽+arm64e AUTH r_type11→摘除 ⑨ShengUfs 头漂移→vendored 换 feat fab07fcc ⑩SourceRootPolicies 的 vr_open_header 缺→VerifiedRoot fork 换 feat ed368a8 ⑪telemetry Defs 对象 arm64 被 arm64e 链接整档丢弃（"ignoring file…built for iOS-arm64"）→TRIPLE 派生 ⑫未解 90→6→duplicate 三轮（Defs 门控+EXTRA_LD 摘重+PGO 12/9 参桩+virtio 计数）。


---

# 2026-11-03 会话四十三归档移入（§9/§9G/§10/§10.X = Mac 件首刷判读 + crypto-funcs-unpack v2，已执行）

## 9. 工作单 = 会话三十九（2026-11-03 收队定稿）：Mac 件首刷判读（a715395a）

- **现态**：Mac 建核车道已转正并完成首刷。recovery_a = `a715395a`
  （payload = Mac kernel `56d24922` + trust `ebeaf7fb` + seed `d04a0153`
  + panic_screen，红线参数全带；存档 `out/m2381/payload-mackernel-a715395a.img`）。
  **首刷已发生**（用户自行刷入），首屏 = PAC 墙（见 §7k），判读留下会话。
- **A. 判读单（第一优先）**：
  1. 符号化：屏摄 `IMG_20261103_041436.jpg`（已入设备仓
     `records/mackernel-a715395a-firstboot-20261103-041436.jpg`），
     pc `0xfffffff00aeb54e4` / lr `0xfffffff00b08f4ec` / x19
     `0xfffffff00b3df3c9`，esr `0x72000000`（EC 0x1C PAC_FAIL，同墙 29 类），
     x16 = `0x59dfb7b00a6fa120`（签名态垃圾高位，鉴权失败目标），
     stages = clock_service_create→sched_startup→thread_daemon_init→
     thread_call→thread_bind→mapping_adjust（全 1s 早期），
     pages/free `0x125b07/0x1254c7`，**crc `0x71f484be`**。
     符号化用 Mac 版账本（`out/m2381/.../kernel/kernel.json` +
     `kernel.map.json`，Fedora 已回传）：先从 kernel.json 的
     collection_base/native_link_address 正推 slide（粗查 ±0x1000000
     无命中，勿再用蛮试法），再 map 定位三寄存器。
  2. 对拍墙 29（`8a935155`：同 esr 类，blraa x8，x8=`0xc2b5b6b0…`，
     pc `0xfffffff00ad39c64`）：同址 = 墙未动（PAC 纪律差不是工具链
     可修的）；新址 = 墙位推进（Mac 件症状改观，续推）。
  3. 分支处置：①站点落 donor kext 桩区 → kc.py wall 反推邻近簇
     3-5 笔 fixture（§D 按需清仓）；②站点落我方内核 → 查 Mac 工具链
     paciza 叠签/单签纪律（会话二十一铁律：PAC 交接修法必须反汇编
     验证单签；entropy 补丁 ptrauth_strip idiom 先例）。
- **B. Mac 车道资产（已固化，下次直接复用）**：朋友 MacBook Air M1
  （macOS 27.0.1，8 核 16G），ssh 免密 `naoniudeair@192.168.10.246`
  （密码 123321，XDG 无关）。工作面 `~/unixnu/`（两仓 + ipsw/，清理
  待项目全完后用户拍板）。环境 = Xcode 27.2 (27B52) + brew llvm/lld/
  mtools/dtc/unifdef/ninja/python3.14。重出件三命令 =
  `defconfig m2381_defconfig` → `tree --ipsw ~/unixnu/ipsw/…` →
  `kernel --ipsw … -j8`（GH_TOKEN 必带；defconfig 必须显式 NAME）。
- **C. 已定谳不再重查**：CI 车道死路（3 连 15min 零接机，GitHub 注释
  = macos-26 arm64 容量不足自动取消；macos.yml=冒烟车道无人当量产）；
  matcher 确定性（重跑 macho sha/账本逐字一致；首跑 vtable raise =
  半成品参照件中间态残留）；A/B 账本（missing 目标 344→190 −45%，
  总目标 1019→920 −10%，stubbed 站点=脏口径不作判读依据）；
  llvm 436MB 走 Fedora 中转法（ghcr blob+token，11.4MB/s vs brew
  直连 350KB/s）。
- **D. 按需清仓不变**：撞墙点邻近簇 3-5 笔，双车道（Linux+Fedora/
  Mac 账本）可交叉定位。回退链 = `0c6b8419` / `8a935155` 均在库。
- **红线不变**：只 recovery_a；--trust-cache/--data-ramdisk/
  --unique-chip-id 3645ad55d47e77c1 必带；--ipsw 本地件必带；-j16；
  CRC 行为准。
- **E+. Mac 全链冒烟（2026-11-04 补）**：payload 步在 Mac 跑通
  （trust/seed 已传 `~/unixnu/`），出件 sha `28164f42` ≠ Fedora
  `a715395a`——**payload 不跨宿主机逐字节复现**（宿主 dtc→DTB、
  panic-screen 宿主编译器差异），一件一线：刷写件必须与判读账本
  同源同宿主。Mac 全链可用（defconfig/tree/kernel/payload；userdata
  的 hdiutil 是 Mac 原生优势），开发可整体搬 Mac；Fedora 留刷写/
  取证 reader（USB 物理连接）与 gh 推送。
- **F. 两阶段战略（2026-11-04 用户定，修正"主力搬迁"误记）**：
  **阶段一 = Mac 开路**：用最平滑工具链把移植打穿到进桌面，排除
  工具链税干扰——若 Mac 件仍进不了桌面 = 存在非工具链的根本性
  移植 bug（本身即关键信息）；**阶段二 = 回 Linux 复现**：通了之后
  对着 Mac 件账本差（stubbed/missing 差集 + PAC 惯用法）逐项追平，
  验证 Linux 编译能否达到同样效果。判读/修墙工作（符号化/fixture/
  补丁）从会话三十九起在 Mac 侧进行；Fedora = 刷写/取证 reader/
  验证站。payload 不跨宿主复现，一件一线。

## 9G. 会话三十九执行档 = Mac 件首刷墙 30 判读定谳（2026-11-03）

- **A. 符号化全落位（slide=0，无需蛮试）**：Mac 件账本
  `kernel.json` 的 `statistics.segment_layout` 给出 collection 新布局
  （native_base `0x716c000`），panic VA 直接命中段表 ⇒ 运行时 slide=0。
  map.json 里的 `native` 字段是**旧布局**（我方原始 build 空间），查表
  前须做段映射（`new = base + (va − native_base)` 逐段）。符号源 =
  Mac 侧未剥符号内核（`mach.development.bcm2837.unstripped.noctf`，
  sha `3dcf0907` 与账本 `native_sha256` 吻合；ssh 拉回本地 llvm-nm）。
  判定链：**pc = `SHA512_Init+0x40`（braa 站点）/ lr =
  `_entropy_init+0x30` / x19 = `_entropyseed`（精确偏移 0）**；x16
  `0x59dfb7b00a6fa120` 按本项目已知格式（签名覆盖 `0xfffffff0` 前缀
  32 位）还原目标 = `0xfffffff00a6fa120`（corecrypto kext
  `__TEXT_EXEC` 内，entry 文件偏移 0xc5d040）。stages（clock→…
  →mapping_adjust，1s）与 entropy_init 早期语义吻合。crc
  `0x71f484be`。
- **B. 墙 30 机制定谳 = crypto-funcs-unpack 补丁的 discriminator
  约定债（§9.A.3 分支②，非工具链 bug）**：pc 处指令 =
  `braa x16, x17`（带鉴权间接尾调），x17 = **逐类型常量**（本站
  `0x73d8`；邻站 field1 = `0xa0d8`；第三站 = 槽址|`0x1bda<<48`——
  Apple Xcode clang 的类型多样性 discriminator）。槽链 =
  `_g_crypto_funcs`（运行时由 corecrypto kext
  `register_crypto_functions` 注册）→ field0 = `ccdigest_init_fn`。
  我方补丁 `patches/xnu-12377/crypto-funcs-unpack.patch`
  （`unixnu_unpack_function_pointer`，23182b1，10-30）把 donor
  packed 指针 unpack 后用 **`(IA, disc=0)`** 重签——与 llvm.org
  clang 的惯例互锁，与 Apple clang 的 per-type disc 不匹配 ⇒ 首个
  consumer（entropy_init→SHA512_Init，开机最早 crypto 用户）必炸。
- **C. 墙 29 对拍（§9.A.2）= 同类不同站，墙位推进**：Linux 件
  （8a935155）pc 符号化 = `_load_static_trust_cache` 邻域（**trust
  cache 域**），槽 = **`_amfi`** 表 field288，消费者 = `blraaz`
  （ctx=0）；x8 目标 `0x08f1e92c` ∈ 我方 kext 区 = donor packed
  形态**未经 unpack 漏网**（amfi 表无 unpack 补丁）。同 root 家族
  （kext 接口表 PAC 交接）、不同站点。Linux 件过 crypto 站的直接
  反汇编实证 = Linux SHA512_Init 同位 = **`braaz x2`（disc 0）**——
  与补丁约定精确互锁，"巧合过墙"。Mac 件墙位更早（entropy 先于
  trust cache）。
- **D. 两阶段战略首战报**：Mac 开路第一刷即暴露一笔 Linux 侧掩盖
  的约定债——战略价值兑现（Linux 件永远看不清这笔）。
- **D+. "Mac 更少 bug"实证（三份原生内核同口径指纹扫描）**：
  donor 23F84 金标（仅其段 12MB）/ Mac 件原生（Xcode 27.2）/
  Linux 件原生（llvm.org clang），统计工具链惯例特征指令数：
  | 特征 | donor | Mac | Linux |
  |---|---|---|---|
  | disc-0 鉴权分支（braaz+blraaz） | 8 | 10 | **3959** |
  | `bti c` | 6658 | 20374 | **1** |
  | braa 带鉴权尾调 | 772 | 957 | 672 |
  判读：**disc-0 鉴权分支是决定性指标**——donor/Mac ≈ 0，Linux 有
  3959 处（每处 = 与 donor ABI 永久分歧的消费者站点，墙 29/30 家族
  的潜在墙面）；BTI 上 Linux = 零（会话 22 matcher bti-shape 通道
  就是给这个还债的），Mac 与 donor 同族有 BTI。⇒ **Mac 件代码惯例
  = donor 惯例（修一次就收敛），Linux 件 = 永久分歧（3959 站逐站
  谈判）**。附注：FPAC 软检查数（donor native 54 / Mac 5937 /
  Linux 4894）受 build 变体影响（donor RELEASE vs 我方 DEVELOPMENT）
  不作判读依据；lane 账本面已有 missing 344→190（−45%）。
- **回退链**：Mac 件无新出件（纯判读轮，零改动）；现役 payload
  `a715395a` 不动。

## 10. 工作单 = 会话四十：crypto-funcs-unpack v2（per-field discriminator 表）

- **★10.0 查证定谳（2026-11-03 补，"查证后再修"轮）**：完整证据链
  闭合，**翻译补丁在 Mac 车道是必需的，但必须逐字段定参**：
  1. 装配层无分歧：上游 profiles 的 prelink roots **同样**含
     `com.apple.kec.corecrypto`（三份 OS profile 全含）；我们的
     集合 entry[30] = donor corecrypto（kmod_info 与 donor 字节级
     同偏移），源码重建 corecrypto 在**内核侧**（"xnu early
     random" 串在 native `__TEXT`，`CRYPTO_ARCHIVE` force_load），
     `XNU_SOURCE_CORECRYPTO14=y` 已在 defconfig。bootstrap
     `source_corecrypto14_original()` 扫到 entry[30] → **donor 件
     赢注册**——上游同样如此，sheng 也吃 donor 表。
  2. **donor 表的签名参数就编在 fixup 条目里**（我们的
     `kc_image.py fixups()` 口径）：撞墙槽位 = `0x80110000036f6120`
     = **(key=IA, disc=0, 地址连通=1)**——donor 自己的消费者用
     **槽址连通**约定；同一 donor entry 里另有 68 处 (IA,
     disc=0x73d8 等, 非地址连通) 槽——**逐槽 (key,disc,addrdiv)
     三元组，donor 内部自洽**。
  3. **三方消费者惯例实测**：我方 Linux 件 = 平翻译后 (IA, disc0,
     无地址连通)（`braaz`）——**unpack 补丁的 (IA,0) 平翻译恰好
     匹配 Linux 消费者 = Linux 过墙的真正原因**；我方 Mac 件 =
     逐字段常量（field0=0x73d8 等，源码版无 ptrauth 注解 →
     编译器 per-type 默认）≠ donor 的槽址连通 ≠ 补丁的 disc0 ⇒
     墙 30。**删补丁不可行**（donor 值对我们的消费者同样不匹配），
     硬编码翻译是正解，参数从**我方 Mac 件消费者站点**实测收集。
- **A. 收集 per-field disc 表（已跑通首轮，欠去污染）**：
  资产已固化 `temp/mackernel-sym39/`（mac_auth_sites.json=14538 鉴权
  站点切片、gcf_field_discs.json=53 站首轮结果、crypto_layout.json=
  56 字段偏移、dump-symbols.sh=符号/反汇编再生成路径）。严格
  provenance 扫描 Mac 件原生 text（adrp 页 `0x7211000`+add
  `#0x858` → gcf 解引用 → `ldr xN,[x8,#off]` → `mov x17,#const` →
  `braa/blraa`）得 53 站 25 字段，field0=0x73d8 与墙 30 实测吻合。
  **已知污染**：`+0xc0 ccchacha20poly1305_fns` 子结构站点
  （`ldr x8,[x8,#0xc0]` 二级解引用，disc 0x8550）被误归入 field0
  ——表生成器须要求 gcf 解引用后寄存器直达 src load（中间不得
  有 offset 重载）；`+0x20..0x40` di 指针字段与 `+0x870` 表外
  站点剔除。地址连通型站点（`mov x17,xK; movk #tag<<48`）单独
  归类（运行时用 unpacked 槽址重算 ctx）。
- **B. 补丁 v2**：unpack 重签改逐字段
  `ptrauth_sign_unauthenticated(addr, key, disc[field])`；host
  条件化：llvm.org 车道保 disc0 平翻译（Linux 行为不变），Apple
  clang 车道（`__apple_build_version__`）用实测表；顺带复核硬编码
  范围 `0x8330000–0xb0c0000` 对 Mac 布局的逃逸误伤。
- **C. 同族预警（不过早修）**：`_amfi`/`_img4if`/`_coretrust`/
  `__dlxk`/`_libCoreEntitlements` 接口表同吃逐字段翻译（墙 29 家族
  的 Mac 面尚未爆）；di-struct 内部 CC_SPTR 指针链 = 下一墙候选。
  修完 crypto 表先刷先判，勿一次铺开。
- **D. 出件与判读**：Mac 重出件（§9.B 三命令 + GH_TOKEN），红线
  全带；预期 = entropy 站过墙推进到下一站。判读须同宿主账本。
- **目标**：修墙 30 = `crypto-funcs-unpack.patch` v2，Mac 件重出件
  判读。**在 Mac 侧施工**（§9.F 阶段一）。
- **E. 会话四十开工顺序（2026-11-03 收队定稿）**：
  1. 表生成器（Fedora 即可，输入=temp/mackernel-sym39/ 已固化资产
     或按 dump-symbols.sh 重拉）：修 provenance 去污染规则
     （gcf 解引用寄存器必须直达 src load，中间禁止 offset 重载；
     剔除 `+0xc0` 子结构二级解引用站、`+0x20..0x40` di 指针站、
     表外 `+0x870`），产出 56 字段 × (kind, disc/addrdiv) 全表；
     无消费者站点的字段标 UNCONSTRAINED（保 disc0）。
  2. 表自检三关：field0=0x73d8 / field8=0xa0d8 必须在表；表内
     disc 值分布与 donor fixup 槽位对照（donor=槽址连通族 +
     常量族混合，我们=常量族为主）不许出现表外地址；冲突字段
     （同一 offset 多常量）逐站回看反汇编人工裁决。
  3. 补丁 v2（`patches/xnu-12377/crypto-funcs-unpack.patch` 改写，
     单提交）：`#if defined(__apple_build_version__)` 走实测表，
     `#elif` 保现状平翻译；unpack 循环按字段查表签名，UNCONSTRAINED
     走 disc0；逃逸 hatch（packed 形态判定 + 范围检查）保留但范围
     常量参数化。
  4. Fedora 侧 check-m2381 + Linux 全量构建过回归门（表改动不得
     影响 Linux 行为——A/B 账本比对 stubbed/missing 计数不变）。
  5. Mac 侧重出件（defconfig→tree→kernel，GH_TOKEN 必带）→
     A/B 快扫（Mac 账本 stubbed/missing 与 56d24922 件持平）→
     payload → 用户首刷（只 recovery_a）。
  6. 判读判据（会话四十一）：crc 换新；entropy 站过墙 = stages
     推过 mapping_adjust（pages/free 应显著上穿 0x125b07）；
     新墙记录全寄存器 + 同法符号化（账本同宿主）。
- **红线不变**：只 recovery_a；-j8（Mac）；CRC 行为准；回退链
  = `a715395a` / `8a935155` / `0c6b8419` 均在库。

## 10.X 会话四十执行档 = crypto-funcs-unpack v2 出件（2026-11-03，待刷 `24f0a941`）

- **①表生成器**（`temp/mackernel-sym39/gen-gcf-table-v2.py`，Fedora 可复跑）：
  严格 provenance（adrp+add 精确到 `_g_crypto_funcs`@0x7211858 符号 → gcf
  解引用 → **直达** field load → braa/blraa；二级解引用剔 SUBSTRUCT；表外
  offset 剔除）+ 正向符号数据流（writeback/寄存器变址建模）。修四坑：
  add 杀自身物化 / retab 不停致跨函数泄漏 / mem_operand 逗号切分 /
  **AUTH 正则漏 braa（只配了 blraa）——补上后 15 字段全出**。
  产出：15 常量字段 × 28 直达站 + 27 子结构站（0xc0 chacha 8 站 0x8550、
  di 0x20..0x40 五站、xts 0x90/0x98 四站、des 0xc8/0xe8 两站 = CC_SPTR
  槽址连通族 = C 预警家族，正确隔离）。**首轮 53 站大半是假阳性**
  （kdp `[kdp_packet+0xe98]` 等无关结构 disc 撞值）。
- **②三关自检**：field0=0x73d8✓ field8=0xa0d8✓；冲突字段=0；
  donor 对照 = **donor 23F84 KC 与 Mac 件 56d24922 的 56 槽元数据逐位全同**
  （temp/mackernel-sym39/donor-vs-mac-crypto56.json；表在 donor
  0xfffffe0007face40，Mac 件 0xfffffff007face40，同 low32）；donor 槽 disc
  = 源码注解族（逐声明），我方 = 编译器 per-type 默认——不等=预期。
  我方受约束集 {0x73d8,0x1810,0x423,0x50c7,0x2abe,0xbcad} 与 donor 集同现。
- **③补丁 v2**（主树 `a903a20`）：`__apple_build_version__` 门（Mac=Xcode
  27.2 Apple clang 实锤，kernel-bootstrap.log ExecuteExternalTool）→ 15 字段
  实测表；llvm 车道 `#else` 保平翻译。范围 hatch 常量两侧同值
  （donor corecrypto text Mac 落位 0x08f1f800-0x08f22488 在范围内，无逃逸）。
  运行时形态链定谳：KC fixup 应用后内存 = donor 注解签名指针
  （bootstrap `__ptrauth(0,0,0xe4fe)` 实证），v1/v2 解码重签。
- **④Linux 回归门**：check-m2381 4ok；Linux 全量构建过；**__TEXT_EXEC
  逐字节一致**（新 08b5c73c vs 旧 1be282e9 同 text sha e201037f，表在
  llvm 车道整体编译消失；`paciza x13` 平签名不变）。
- **⑤Mac 重出件**：kernel `14f7e7aa`（payload 内 `40b7dd55`）；
  **A/B 账本九项全零差**（kext_pointers_rebound 47473 / closure_unmapped
  334 / branches_rebound 42024 / kext_chain_sites_kept 82701 等逐项持平
  56d24922）；payload **`24f0a941939fe381…e4bc`**（96MiB，
  /tmp/m2381-sym39/payload-v2-24f0a941.img，Mac 侧
  out/m2381/.../payload/payload.img）。
- **⑥新雷 + 拆除（Mac 车道第一颗 fresh-matcher 雷）**：native 一变 →
  kernel.map.json 缓存 pairing（v1 时代，validate 比 native sha）失效 →
  Mac 车道**第一次真正跑 fresh matcher** → `anchor_vtables` reviewed
  矛盾 @0xaf5b9b4。定谳：投票错（donor IOUserClient vtable 0x7d72508 被
  错配到 native 派生类 `__ZTV22IOKitDiagnosticsClient`，同构 186/186 +
  覆写函数体形巧合），reviewed 对（slot[172]=clientClose 双侧核验）。
  修 = 设备仓 `9e5b51b` fixture reviewed data 锚
  （donor 0x7d72508 ↔ `__ZTV12IOUserClient`），bind 守卫掐断错连；
  prelink 复跑 5 个 vtable-correction 自愈、无 raise。
  坑档：①tree 的 kit = **device repo vendor 副本**（主树 kits/ 是冻结
  v8 死副本，本地重放必须 PYTHONPATH=device repo kit）；②本地 `-I` 吞
  PYTHONPATH；③非交互 ssh 无 homebrew PATH（export PATH 前置）；
  ④Mac python3.13；payload 需 `--data-ramdisk ~/unixnu/scratch-data.img
  --unique-chip-id 3645ad55d47e77c1`。
- **判读（会话四十一）**：crc 换新；entropy 站过墙 = stages 推过
  mapping_adjust（pages/free 上穿 0x125b07）；下一墙候选 = di 字段
  （v1/v2 对 in-range 数据槽仍按 fn 重签 = 潜在 PAC 位污染，_sha1_init
  的 `ldp [x20,#8]` 类）与 chacha 子结构 CC_SPTR 链。回退 = 刷回
  `a715395a`（payload）。

<!-- ===== 2026-11-04 会话四十五收队批量移档（自 STATE.md 原样搬入）===== -->

## 7. 工作单 = 会话二十七（2026-10-31 会话二十六工具轮收队定稿）：批量 fixture 首刷：

- **会话二十六已毕（本轮，工具+fixture，未推送未提交）**：墙19判读定谳
  （屏 crc 0x18815d36；0xa52e2b0 = sandbox mpo_policy_init 内 PE_parse_boot_arg_str，
  donor 0xafc17fc，36B）；**fixups 反查定名法**立杠杆（donor vtable 槽位夹逼，
  槽序对头文件声明序零歧义）；6 笔已入设备仓 fixture（共 145 条，**未提交**）：
  0xafc17fc=_PE_parse_boot_arg_str / 0xaeb8eb0=IORegistryEntry::copyProperty(const char*)
  / 0xaf4e348·0xaf4e544·0xaf4e460·0xaea0bf8=IOUserClient::init(dict)/init()/free()/Dispatch(IORPC)
  ≈ 1500/4285 账本站点。工具箱 = `/home/ri/work/tools/xnu27/`（kc.py /
  xnuindex.py / make_fixture.py / README=流程手册；REA 已迁 `/home/ri/work/tools/rea`）。
- **A. 开工序列**：
  1. 推送积压（§C 收队项：主树 4 笔 + 设备仓 2 笔）。
  2. IOUserClient 残余四目标（0xaf4babc/0xaf4bbd8/0xaf4bc40/0xaf510ac ~500 站，
     槽位邻接 copyClientEntitlements/getAsyncTargetAndMethodForIndex，同法收尾）
     + 账本 top10 其余（0xa95ca30=IOSimpleLockLock 候选已半钻，按会话二十六 B.1 单继续）
     → 全部走 `kc.py slots` + `xnuindex.py nbrs` 快线，spec 攒进
     `/home/ri/work/tools/xnu27/spec-session27.json`。
  3. `make_fixture.py add` → commit（设备仓一笔）→ defconfig+kernel+payload
     出件（-j16，--trust-cache/--data-ramdisk/--unique-chip-id 必带）。
  4. 只刷 recovery_a，等拍屏；判读门 = 0xa52e2b0 墙消失 + mpo_policy_init
     走完 + 下一段新坐标记账。回退 = recovery_a ← payload-grpattr-font2-dd0da85b.img。
- **B. 工具面**（已就位）：判读一律 `kc.py wall <屏上地址>`；坐标三事实与
  定名循环见 xnu27/README.md，扇出子代理产物格式已定（VA/候选名/三证据/置信度）。
- **止损（不变）**：路径排空后 root mount 仍同区连发卡 → 停修归因回报拍板。
- **红线不变**：只 recovery_a；boot_b d0200a6b / userdata 不动；CRC 行在位。

### （留档）会话二十六工作单（判读轮 + lck 批量预钻）——判读已在二十七完成：

- **A. 判读（用户刷 payload-grpattr-font2-dd0da85b 后）**：
  0. 刷前现态核对：current-slot=b、现役 recovery_a = dd0da85b、boot_b d0200a6b / userdata 不动；回退 = recovery_a ← payload-macresign-c13089a9.img。
  1. 拍屏 → **指令级验真**（新工艺，比 CRC 重建便宜：ESP 内核 pc 处反汇编逐指令对拍，0xb01cfe8 先例）→ 判读门：①0xa52e140 墙消失（sandbox 锁初始化段走完，后续 attr_setstat/alloc_init/lck_*_init 全已配对，理论可连走）②**诊断页小字大一倍**（scale 2 视觉确认）③新墙按调用序点名 → bootpath 榜单（temp/batch-fixtures/bootpath-rank.txt）+ batchid/stubid 例行钻法。
  2. mac policy 注册段全过后的预期下一段 = sandbox 主体执行 / AMFI 后续阶段 / apfs root-mount 逼近；新坐标先记档。
- **B. 离线预钻（不刷机，lck 家族批量 = 下一段大概率墙源）**：
  1. **第一刀 = 0xa95ca30（127 站点/8 组件，bootpath d=4，本会话已预钻一半）**：体形已定谳 = per-cpu 计数（tpidr_el1+0x1b8）+ `ldxr/casa [x0]`（0→tpidr 原值）+ wfe 自旋 + 慢路尾调 0xa7f960c；donor 括号 = `_lck_spin_assert` / `_IOSimpleLockUnlock` 之间 = **`_IOSimpleLockLock`**，native 0x745e00 同址双名候选。修法 = 对拍 native 0x745e00 体形（FAST 路径 CAS [x0] 同形即配）→ fixture（走 4cea0fe 模板）；同时核慢路 0xa7f960c 配对状态，未配则同批。自洽性先例 = 会话十七 lck_mtx 三条（kext 锁全经我们实现，字值约定自洽）。
  2. 第二刀 = bootpath 榜单 d≤7 余量逐个 bracket 甄别（0xa8150f0/0xa866894 已证无 native 对应体勿再钻）；每有 unique-candidate 级身份即顺手落 fixture。
  3. 若新屏落在别处：例行钻法优先，本单 B 项不挡判读。
- **C. 收队项**：主树 4 笔（184e3ad/d532452/ca3bc95/05e4f97）+ 设备仓 2 笔（020aa2e/4cea0fe）未推送——会话开工先拍板推送。
- **D. 止损（不变）**：路径排空后 root mount 仍同区连发卡 → 停修归因回报拍板。
- **背景排队（非关键路径，阶段②前按需）**：B.2 续单（co-slot 群一致性 = 先配 IOUserClient::init 0xaf4e544 → Dispatch×48 收编）；大函数残墙放宽通道；B 腿 CI（build-fileset-release-reference 参数化 + kernel.json site 口径）。
- **红线不变**：只 recovery_a；boot_b d0200a6b / userdata 不动；--trust-cache/--data-ramdisk/--unique-chip-id 必带；-j16；CRC 行在位、指令级验真为准。

- **战略**：刷机从探针变回验收。离线走完 early-boot 调用图，把"到
  root mount 为止"要经过的 stubbed 目标**一次修完**，一个 payload，
  一刷验收整段。账本清零不再是目标。
- **A. 调用图清点**：入口 = early_security_init（emit 命令行
  `com.apple.driver.AppleMobileFileIntegrity=0xfffffe0008f13c80`）→
  donor AMFI init（0x8f0d230 所在函数）一带；沿 bl/blraa 静态遍历
  （条件分支双侧保守收集），收集 ledger 内目标直至关键组件收口
  （apfs / IOStorageFamily / AppleImage4 / corecrypto / ARMPlatform /
  PMGR）。walker 落 `temp/batch-fixtures/bootpath.py`。产出 = 路径
  目标清单（预计个位数到十几个）。
- **B. 逐个修**：
  1. 0xafc09d8 = SecureDTGetProperty（已定谳：donor 0x19C 独立函数、
     native 内联无符号；语义 = 遍历 DTEntry 属性（entry+8 = 属性数组、
     [entry] = nProps、prop+0x20 = size、name strcmp），写 value/size
     返 1/−1）→ ~30 行 C shim 挂 native（先对拍我们 pexpert IODT 的
     DeviceTreeNode 布局——同为 xnu-12377.122.8 应同构）+ fixture 绑定。
  2. 0xafc1b20（PE 小壳）若在路径上 → 同法。
  3. 其余路径目标：先重跑 matcher（v11 自动收新壳）；仍不能配的按
     shim/fixture 先例逐个落。
- **C. 出件一刷**：kernel → payload → 只刷 recovery_a。判读门：AMFI
  early init 整段过（`fileset: … from 0x8f…` 不再报）→ 推进到下一段；
  root mount 段新坐标 = 记账继续 B（不单独刷）。
- **D. 止损**：路径排空后 root mount 仍被同区新墙连发卡 → 停修归因
  （系统性缺口），回报拍板。
- **红线不变**：只 recovery_a；boot_b d0200a6b / userdata 不动；回退 =
  recovery_a ← payload-kallocfix-crc-5f59a2a0.img。

## 8. 车道判读档案（23F84 fileset 线，会话十三→二十四；逐墙全文亦在各 git commit message）
- **★最后一面墙（会话十三第一刀，修法已勘定）**：duplicate `boot_log_*`/`boot_diag_*`/`boot_log_export_put` ——BootLog.o 双载：libBootProgress.a（build-boot-progress.py:91 无条件编译 log_obj）× libShengUfs.a（source-storage 适配器 sources 表无条件含 BootLog.c）。**修法 = 主树 `tree/scripts/build-boot-progress.py` 在 XNU_SOURCE_STORAGE=1 时跳过 log_obj**（log_command 编译 + archive 成员表两处条件化；BootProgress.o 的 boot_log_tick 弱引用由 source-storage 档的 BootLog.o 兑现；build-report.json 的 log_sha256 条目同步条件化）。改完 `./unixnu defconfig m2381_defconfig && ./unixnu tree && ./unixnu kernel` 三连（改 defconfig 必重跑 defconfig=铁律；改设备脚本必重跑 tree 刷新副本）。
- **kernel 绿后余单（按序）**：①board.toml `[payload] trust_cache_sha256` 三值换 23F84（现挂 20H392 三值=payload fail-safe 门；kernel 步已按 profile [ipsw] fetch 清单取了 StaticTrustCache/Cryptex1,SystemTrustCache 成员，sha 之 + make-m2381-trust-bundle.py 重打包）；②args 拍板——**案甲（工作单推荐）已勘定参照 = sheng feat27 board.toml:52 实证单**，m2381 最小集 = `-v serial=3 debug=0x14e kt_fixed_zones=120 -no_slto_panic use_contiguous_hint=0 native-source-storage=1 native-ufs-root=1 noidle=1`（native-source-storage=1 是 source-storage 启动总闸=SourceStorage.cpp 门控必带；sheng-pmgr/offline-legacy-* 全不背=lane 已摘；noidle=1 防 r70 睡眠灭屏）；③`./unixnu payload`（红线：--unique-chip-id 3645ad55d47e77c1 + --trust-cache/--data-ramdisk 显式传 + -D 在子命令后）；④首刷三件套 = boot_b ec1d518b 不动 + recovery_a 新 payload + userdata 新件（已出），判读门 = native-pac-keys 每 boot 每核键编程行/fileset 绑定计数/native-source-storage=1 生效行。
- **★会话十三步0 = 上游更新（用户令：先同步再继续）**：`git fetch upstream` 看 feat/ipados27-gpu-parity 新 tip 与 **main（用户情报=上游在写文档合入 main）**；若动 = `git rebase upstream/feat/ipados27-gpu-parity m2381-ios27`（**净差验收铁律**：取上游版类冲突不能只看停点，`git diff feat..tip` 抓静默拼回）+ 子模块对齐 + 本单 12 面墙补丁逐面过 net-diff（尤其 kernel.py 双派生与设备层 ptrauth patch 是否被上游同题解法替代——若上游已修则取上游版删本地件）。
- **预期管理**：本车道 fileset 出件（emit_prelink --fileset-reference 吃 23F84 MH_FILESET donor）仍未到达——链接关过后它就是下一关（sheng 未公开通关证据，无 boot 实证参照；23F84 KC 无 LC_SYMTAB，符号化走 dyld cache/fileset 表）。回退链不动：21H461/20H392 全组 + boot_b ec1d518b + userdata 2c2aca3f。

**会话十三执行记录 + 会话十四交接单（2026-10-30 深夜，全链出件达成）**：

- **步0 上游更新**：fetch 后 feat 新 tip `f161cd2`（+3：AdrenoMetal×2 bump + 三连 bump opaque scanout + metal 前端 host check 脚本，全 GPU 侧不消费）；main +9 全 docs（device records 刷新，feat 未合 main）。rebase 238 提交**零停点**落 `3315cf8`，净差验收 57 文件全工作区资产，上游 drivers gitlink 对齐新 pin（Adreno740 90b52bf/AdrenoMetal 148698d/GopFramebuffer 7f7ae1e）。备份针 `m2381-ios27-pre-rebase-20261030`。
- **第12面墙破（BootLog.o 双载）**：`build-boot-progress.py` 在 XNU_SOURCE_STORAGE=1 时跳过 log_obj（编译+archive 成员+report 三处条件化）——kernel 步推进到 prelink。
- **fileset 出件关三墙全破（本会话主战场）**：
  ①**codeless 断言** = 设备 fork 的 emit_prelink 无 fileset 分支无条件走经典 plan()。**换装 vendored kits/XnuPrelink → 上游 feat pin 76452c6e**（fileset_match/emit/layout/shape/segmented+kc_image，8 测试全绿；21H16 arm64e 经典路 fork 退役入 git 史）+ board.toml `[kernel.prelink] extra_args=["--allow-missing"]`。**机制勘定：MH_FILESET 输入走 fileset_match 管线（--fileset-map/--fileset-reference/--allow-missing 任一切换），plan() 的 codeless 检查只属经典路**。
  ②**native __DATA 槽约束** = fileset_layout 把 native __DATA 钉在 collection `__DATA` root 起点保留（donor 内核数据位，23F84 洞=2142KiB，首个 retained kext=AppleARMPlatform@0xb4af780），我们的 native __DATA=3824KiB（BSS 大户：bp_* profiler 四件 2048KiB）。**裁 BootProgress.c bp_*：BpProfSamples 65536→4096、BpKernSamples 32768→4096**（省 1888KiB → native 1936KiB，余 206KiB≈5%）。洞后零字节为零=不可扩洞，裁码是唯一路；gScratch(ShengUfs 128KiB)功能件不动。
  ③**RELEASE 参照件产能**（--fileset-reference 的料）：`XNU_KERNEL_CONFIGS=RELEASE ./unixnu kernel --skip-prelink` 产出 `mach.release.bcm2837.unstripped.noctf`（17.6MB）→ 手动 stage 到 `tree/build/xnu/fileset-reference/` → board extra_args 换 `--fileset-reference <path> --allow-missing` → 重跑 kernel。**主树脚本三改**：build-xnu-4k-baseline.sh（config 选择+XNU_REFERENCE_BUILD=1+ubsan-minimal stub 编译钩子置顶于 crypto gate 之前；seal 路径按 config 派生）+ patches/xnu/reference-build-release-export-bypass.patch（**RELEASE-only all-kpi.exp 导出检查旁路**：IOUnifiedAddressTranslator/DART/llvm_profile 系预链 kext 符号，开源 xnu 树无源码，纯源码 RELEASE 独立内核永远缺；DEVELOPMENT 无导出检查故从未暴露）+ kernel.py（RELEASE pass 置零 m2381 取证链 owner 全家+BOOT_PROGRESS——offline-owner 补丁自带 DEVELOPMENT #error 守卫且 RELEASE 形态编不过，参照件不 boot 不发布；**compile-record 检查按 config 参数化**）。**bootstrap 链接失败=设计内可容忍**（kernel.py try/except：记录在即放行；主链接 force_load 档供体齐）。**巨型坑：host nm/llvm-nm 读 138MB Mach-O 档静默零输出**——曾据此误判"无定义体"绕远路，凡 nm 扫档必须先验输出总行数。
  - **参照件成效**：函数匹配 6807→**8293**（reference-order 5142+reference-symbol 401=名字转移主通道），data 559，未解 641 sites/378 targets（apfs 206 sites 为主）。
- **payload 步三墙**：①check_native_root 强索 sheng-pmgr=2+_sheng_pmgr_run（经典路前提）→ payload.py 加 prelinked_pmgr 参数（profile roots 含 com.apple.driver.ApplePMGR 即免检，sheng 实证 arg 行同形态）；②patch-xnu-entry-probe.py 符号解析吃 fileset "Symbols from…" 节头行→parser 跳非 hex 行；③panic screen 超洞 7 字节（0xa37>0xa30，fileset __TEXT_EXEC padding 收窄）→ **xnu-panic-screen.c 摘 frames walker 块**（-88B→2515B，保 pages/free 判读计数器；串口 backtrace 补位）；④**ESP 超 recovery_a 100MiB 分区**（stage 101MiB：fileset kernel.macho 85.2MiB）→ board.toml `forensic=false` 摘读者三件 15.3MiB。
- **出件终值**：kernel `9da51132`（带参照件）→ payload **`8a209d8b`**（96MiB，ESP 四样齐：kernel.macho 89,374,720B+patched 副本 sha 5c244659+trustcache 82,236B+scratch-data+initrd 行+ADT acba2928，卷余 9.3MB）；trust 三值 e70a385a/58960bbc/ebeaf7fb 已入 board pin；args=案甲 9 项 130B。
- **nura 读者内核需求（2026-10-30 晚拍板：nura 仓零接触，不发工单文档，需求由用户口头转告）**：需求规格留档 = 设备仓 `forensic/READER-KERNEL-REQUEST.md`——读者专用最小内核（去 GPU/DRM 全家，PE≤4MiB，保 EFI stub+UART+USB NCM+UFS+configfs，initramfs 外投契约同 build-forensic-reader.py），落成后摆 `.cache/forensic/` + board.toml `forensic` 回 true。
- **会话十四交接（首刷判读单）**：三件套 = **boot_b ec1d518b 不动 + recovery_a ← payload 8a209d8b（先 fastboot getvar partition-size:recovery_a 核 100MiB）+ userdata ← userdata-root 13.02GiB（out/m2381/ipados-26.5.2-23F84/m2381/userdata/）**；设备现态先核（FLASHING.md 单通道律+dd 读回才算证据）。**判读门（串口）**：①native-pac-keys 每 boot 每核键编程行（EnIA/EnIB/EnDA/EnDB）；②fileset 绑定计数（emit_prelink 报告的 named/mapped 数）；③native-source-storage=1 生效行 + source-storage 卷挂载；④`_unixnu_fileset_unmapped` stub 触发行（每条=一次未解调用命中，报站点不跳野指针——641 sites 的实验性余量是预期内背景噪声，非墙）；⑤panic screen（pages/free 行在，frames 行已摘）。符号化：23F84 无 LC_SYMTAB，走 dyld cache/fileset 表（r88 路线作废预注册仍有效）。回退链不动：21H461/20H392 全组 + boot_b ec1d518b + userdata 2c2aca3f + 备份针 m2381-ios27-pre-rebase-20261030。

**首刷卡（2026-10-30 交卡，会话十四按此判读）**：现态核对（current-slot=b + partition-size:recovery_a=100MiB + 自 r85/r88 轮 boot_b 未被动过——若存疑则补刷 boot_b ← 设备仓 `UniXNU-m2381/build/m2381/boot/boot.img`（ec1d518b，31,793,152 B；⚠主树 out/m2381/boot/boot.img=40f2a199 是 B2 旧件 1.66MB，勿混））。刷序 = 先 recovery_a ← payload 8a209d8b（100,663,296 B）→ 开机验 UEFI 菜单在（功能级 ESP 验证）→ 再 userdata ← 13.98 GB 件 bb5d22f4（15-30 分钟勿断）→ 开机串口判读。本轮无 dd 读回通道（读者被 ESP 空间挤出，nura 最小内核回归后恢复），证据 = 串口 + panic 屏。判读门 = STATE §8 会话十四交接五条。回退链 = recovery_a ← payload-r88-c932a128 + userdata ← 2c2aca3f 件（均已在库）。

**会话十四判读 + boot 件定谳（2026-10-30 深夜，首刷实况：UEFI 菜单活，选 first boot 立刻闪回菜单）**：

- **根因定谳 = FV 内 DarwinBoot 太旧，不是 boot_b 坏也不是内核/投递链**：ec1d518b 构建于 10-29 07:42（r81，20H392 时代），其 DarwinBoot pin=`3325e35`——`ParseKernel` 硬校验 `U32(Data+12)!=2 → EFI_UNSUPPORTED`（**只收 filetype 2 = MH_EXECUTE**）；26.5.2 fileset 内核 kernel.macho 实测 filetype=**12（MH_FILESET，cputype 0x100000c）**→ DarwinBoot 当场拒，Boot Manager 回菜单，与"选 first boot 立刻闪回"逐环节吻合。修复 pin=`3e3a8a9`（accept arm64e MH_EXECUTE 内核 + **filetype 12 在 Header==0 放行** + root policy sidecar；ESP 上 kernel.macho.crypto-hmac.json 即配它）。机制教训入档：**boot_b"冻结不动"纪律按车道绑定——换 OS 车道（内核形态/代际变）必须重估固件，DarwinBoot 的内核格式接受面跟着车道走**。
- **上游 uefi 跟进（用户拍板）**：rebase `0eb4514 → 20ac23d5`（upstream/sheng tip，5 笔 = 34c98806 ShengBootMemoryDxe 176 行换共享 UniXNU RAM disk 驱动【shengPkg 专属 fastboot-ramdisk 路径，m2381 不引用、零影响】+ docs×4）；overlay 重放 `9318b62e`（基座记载已同步改写），m2381 相关文件 rebase 零 diff。**uefi-m2381 push 需 force-with-lease，待拍板**。
- **新 boot 件出件**：FV 重编译（DarwinBoot.efi 87,040 B，10-30 03:27 新鲜于源码 mtime，增量缓存嫌疑排除）→ `package-m2381-boot.py` 打包 → 设备仓 `build/m2381/boot/boot.img` = **`115f2738`（31,793,152 B，与旧件同尺寸）**；ADSP 段 acf95fd5/adsp_dtb db6a6478 与旧件**逐字节相同**（FV 外零漂移），fd ce3ce88b→**68e5023c**（唯 FV 内容变 = DarwinBoot）。旧件备份 = 主树 `out/m2381/boot-b-ec1d518b-r81.img`（+json）。
- **补刷卡（2026-10-30 交卡）**：**只刷 boot_b ← 设备仓 `UniXNU-m2381/build/m2381/boot/boot.img`（115f2738）**，recovery_a 8a209d8b / userdata bb5d22f4 已刷在位不动；刷前现态核对照旧（current-slot=b）。判读门 = 会话十四交接五门不变（串口：native-pac-keys 行 / fileset 绑定计数 / native-source-storage 生效行 / `_unixnu_fileset_unmapped` stub 触发行=预期噪声 / panic screen）。回退 = boot_b ← `out/m2381/boot-b-ec1d518b-r81.img`。
- **读者选项丢失 = 预期内**（forensic=false 被 100MiB ESP 挤出，会话十三已档），恢复路径不变：nura 最小内核（4d67ff9 工单，PE≤4MiB）交付 → `.cache/forensic/` → board.toml `forensic=true` → payload 重建（现卷余 9.3MB，4MiB 内核+1.3MiB initramfs+0.14MiB dtb≈5.5MiB 可容）。

**会话十四二轮判读 + cpusub 门破（2026-10-30 深夜续，用户刷 boot_b 115f2738 后实测：仍秒弹回）**：

- **第二道门定谳 = com.apple.kernel 成员 cpusub 不等**：新 DarwinBoot(3e3a8a9) 对 filetype 12 放行后，`ParseKernel` 还要求 **`com.apple.kernel` 成员的 cpusubtype == 集合头 cpusubtype**（DarwinBoot.c:238，不等 → EFI_LOAD_ERROR）。实测：我们 kernel.macho 顶层 = `0xC0000002`（emit 从 donor 抄），成员 = `0x80000002`（**Linux lld 源建内核自带 CPU_SUBTYPE_ARM64E|0x80000000**）；donor 23F84 自身 top/member 一致（均 0xC0000002）。上游在 macOS 用 Apple ld64 建核天然产出 0xC0000002 故从未撞此门——**m2381 = 首个 Linux 建 fileset 上机的板**。
- **DarwinBoot 后续门全部本地预验 PASS**（免再烧刷机轮）：成员段覆盖 10/10、entry 与顶层相等、父段无重叠、trustcache（82,236B 双 cache）合法性、scratch-data.img=「DBRDISK1」packed 格式与 ADT PackedRAMDisk/KeepPacked 一致、窗口 = ADT darwinboot-memory-size 4864MiB（board owned-memory-size；若 AllocateWindow 失败屏上有大字 NO WINDOW 诊断=DiagDraw，拍照即可定位）。
- **修法**：主树 `scripts/patch-kernel-fileset-cpusub.py`（成员 cpusub 规范到顶层，原值钉 0x80000002，round-trip 自检，非 fileset 零操作）+ `payload.py` 在 kernel copyfile 后无条件调用（先于 probe/seal）；payload 重建 = **`38a2a8f9`**（96MiB；ESP 内核副本 `14bc069b`；源建 kernel 9da51132 不变；对 ESP 副本跑 ParseKernel 全模拟 = ALL PASS）。
- **补刷卡 v2（2026-10-30 交卡）**：**只刷 recovery_a ← payload `38a2a8f9`（100,663,296 B）**，boot_b `115f2738` 已在位不动、userdata 不动。开机预期：串口 `DARWINBOOT: loader v2 installed EL1` → `inputs validated; exiting boot services` → 绿条 → 内核五门照旧。回退 = recovery_a ← 8a209d8b（在库）。
- **备注**：上游两 pin（DarwinBoot 3e3a8a9 / XnuPrelink 76452c6e）与我们所用一致、无更新修复；cpusub 规范化是可上游化的通用修法（LLVM 建核 vs Apple ld64 的 cpusub 差异），后续可提上游。

**会话十四三轮判读：内核起飞 + 第一堵原生 PAC 墙定谳（2026-10-30 深夜续三，本轮=纯分析不修，修复=下会话）**：

- **三件连过**：cpusub 门(38a2a8f9)→ stage 3 复位探针点火实证(品红后 ~5s 自复位 = MMU 开启序列过，4K+arm64e 页表启用段排除)→ 诊断页轮(49f1478d,boot-progress=2+panic 屏复位探针撤收)炸出 panic 屏。**上游同款剧本验证**：stage 色条/定时复位/panic 屏全是上游 feat 自己的 patch-xnu-entry-probe.py+green-bar diagnosis 指南(unixnu.com/guide/debugging)，boot-progress=1/2 诊断页=上游盲区主力工具(120×48 阶段史，卡住冻结可拍)。
- **内核抵达深度（诊断页实录）**：pages=0x125b8e/free=0x12554e（≈4.7GiB 窗全活）→ 阶段史 clock_service_create → sched_startup → thread_daemon_init → thread_call → Thread_bind → mapping_adjust → clock_service_create(2nd)；OSLog 流验证/Firehose 16 chunks/Log queues 72 槽全配置完；末行 console=`[hmac algorithm] tried entry init`。
- **墙 = `PAC failure from kernel with IA key while branching to x8`**（ESR 0x07200000，far=0，cpsr 0x624003c4）。符号化：**pc = `_source_corecrypto14_bootstrap`+0x84 `blraaz x8`**（IA 键零上下文鉴权）；x8 ← `ldur x8,[x19,#0xb4]`，x19/x0=0xfffffff00b5f9a90（BSS 对象，BSS 初始化域）；x19 来自 +0x40/+0x4c 两次调 `_source_corecrypto14_original`（单例，cmp 相等过）→ casal 锁 `_source_corecrypto14_original_state` → 调算法入口指针；调用场景= hmac 算法 entry init。**x8 地址部分=0xfffffff00a72f938（__TEXT_EXEC 符号洞），头四字节 0xd503245f=PACIASP=真函数入口形态**（非 reporting-stub 形态；未在 emit 档案中登记）。x20=块描述符域、x24=`__startup_EARLY_BOOT_entry_source_pthread_bootstrap_23+0x18`（启动集合邻域）。
- **已排除**：①per-core 键分歧——native-pac-keys.patch 实读：首核 CNTPCT+splitmix64 派生一次、MMU off 写 `native_pac_keys` 全局，后续核一律加载存储值（含热复位），全核同键；②x8=登记在案的 fileset unmapped stub（emit 档案零命中）。
- **候选（下会话 RE 判别）**：α=签名者/鉴权者**上下文失配**（写侧带多样性、读侧 blraaz 零上下文——注意 linux-clang-ptrauth-null-fallback.patch 的降级域是否覆盖 corecrypto 注记）；β=**fixup 链重签覆盖缺口**（源建内核 __ptrauth 全局指针走 lld auth_rebase→内核早期重签；该指针若在 emit 搬运中丢了链条目，残留 lld 占位/旧签名）；γ=签名发生在不同键状态相位。判别路径：①xref 找写 [x19+0xb4] 的指令确定签名形态与预期上下文；②用 emit 的 orig↔native 映射解 0xa72f938 真身（kernel.json/kernel.map.json）；③核对 lld auth_rebase 在 fileset emit 后的存续；④若需 A/B：固定键探针（键派生改常量）可一步区分键态相位 vs 上下文失配。
- **nura 读者内核交付（用户转交交付单，文件未落本机）**：vmlinuz-unixnu-reader-7.3.0-20261030 = 3,977,728B（3.79MiB≤4MiB，sha256 1512301a4d77d090a3b245f912f8fcd901e8cc905a47bf2416a10683694d42ff）+ 专用 .config（f7142f19…a506e05）+ System.map（无 kallsyms 固定地址，离线符号化用）。预算对账 85.2+3.79+2.4≈92<100 ✓（ESP 余 9.3MB 容 3.79+1.3+0.14≈5.2 ✓）。契约：PE32+ EFI AArch64 zboot+zstd（LoadImage+LoadFile2 流程原样）、CONFIG_CMDLINE 与 v71 逐字节同、initramfs 契约不变（busybox 不动）；身份自检 `uname -r`=7.3.0-unixnu-reader。行为差异：无显示驱动（GOP 残留 fb 上 earlycon+fbcon）、USB gadget-only（configfs 绑 UDC 即通，无 host）、无模块、无 RTC（时间戳 1970 预期）、无 KASLR（配 System.map 好调）。验收=payload 菜单 reader 项→UART shell→NCM 起→dd recovery_a 前 96MiB 对拍；盯「Initramfs unpacking」不回归。翻车回带 UART 全文（dwc3/ufs/eusb2 probe 行）；源码=linux-mobile-ports meizu20-t4b@bfc79f41，配方 nura docs/UNIXNU-READER-KERNEL.md。
- **下会话开工单**：①nura 三件落本机验 sha → 摆 `.cache/forensic/`（forensic-kernel.efi 命名按 build-forensic-reader.py 契约）→ board `forensic=true`+`sheng-log=1` → payload 重建 → 读者项验收（此后每轮自带 dd 读回+SHLOG 窗）；②PAC 墙 RE 四步（上述判别路径），注意 seal-source-crypto-kernel 与该指针的相互作用；③探针轮配置（panic_screen/entry_probe_reset_delay）待 PAC 修复轮一并收队；④回退链新增：payload 49f1478d（诊断页轮）、94f67199/5209bb33（复位探针轮）、38a2a8f9（cpusub 轮）均入 out/m2381/ipados-26.5.2-23F84/m2381/payload/ 历史（payload.json 只存现役，历史 sha 在本档）。

**会话十五判读：读者通道交付轮全绿 + FV 内存图地雷拆除（2026-10-30 凌晨续，用户三拍板：分家/用 v71/留菜单）**：

- **分家定案（用户拍板）**：reader 不再 riding iOS payload——专用读者 ESP（设备仓 `forensic/build-forensic-esp.py` 出件）按需刷 recovery_a，取证轮/iOS 轮各刷各件；`fastboot boot` 不可用（nura 已实测 Booting FAILED，ABL-FASTBOOT-TABLE 卡）。board.toml 回 `forensic=false`+`size_mib=96`（iOS payload 下轮重建生效）；`sheng-log=1` 留在 kernel_args（下轮起每轮 boot 留 SHLOG 窗，读者 ESP 对刷即可采）。**reader 内核 = v71 完整版（4aad92b3，m2381 实证）**，nura 最小版（1512301a）在案未上岗；**菜单保留**（ESP toml default=forensic/timeout=3，Restart/Power off 常驻，reader 退出回菜单）。
- **最小读者内核首崩根因定谳 = FV 内存图 bug，非内核**（全屏崩溃照 + 反汇编闭环）：MemoryProtectionExceptionHandler dump（肇事镜像 [0]=UniXNU.pdb）→ ELR = reader PE+0xAFC0 `sturb w15,[x9,#-5]`，x9−5 == FAR `0x82700020` 逐字节合，x6/x7/x8/x9 四路等距 0x3AE 游标 = zstd 字面量解码写输出 → **同步外部中止（DFSC 0x10）= 写 XPU 围栏** → 围栏实名 = `tz_stat_mem@0x82700000`（stock FDT no-map，Meizu 实机反编译）→ 投递者 = **RamManagerDxe**（SiliciumPkg）：把 MemoryMapLib 洞区 [0x81F00000,0x8A800000) 整段 `MapMemoryRegion(EfiConventionalMemory)`（洞区无描述符 → ParseMemoryRange 当空闲段收编），无 KASLR zboot 的 `efi_random_alloc(seed=0)` 低址优先把 9.8MiB 解压缓冲拍进洞区首段。v71 存活 = KASLR 随机地址运气；**iOS 车道同雷**（任何 efi 分配可落洞区）。
- **修复（uefi-m2381 MemoryMapLib）**：三块 no-map 区入表（ADSP MHI 0x81F00000+0x20000 / Global Sync 0x82600000+0x100000 / TZ Stat 0x82700000+0x100000，全 [DTB] 值）→ RamManager 减法自动排除，洞区余段 0x81F20000-0x82600000(6.9MB<16MB 被弃) 与 0x82800000-0x8A800000(128MB 净土) 照常。**boot_b 新件 `d0200a6b`**（31,793,152B，ADSP 段逐字节水，现役 115f2738 备份 out/m2381/boot-b-115f2738-rollback.img）。FV 菜单/启动照常 = 修复零回归。
- **工具流澄清（勿再钻错工具）**：boot_b 打包 = **设备仓 `scripts/package-m2381-boot.py`**（v4 头 + 5 字节 ramdisk 占位 + M2381FW ADSP 段；shim 契约 `image_size==FD_SIZE` 硬校验——overlay BootShim.S 是 nura 实证契约**非遗漏**，按上游 1c036336 改 accounting 会被它正确拒绝，已回滚）；`./unixnu bootimg` = sheng 形状，m2381 勿用。
- **32MiB FAT32 陷阱（首刷实拍入册 FLASHING §7）**：数据簇 64,480 < FAT32 下限 65,525 → `mformat -F` 硬造非法卷无告警 → EDK2 Fat 驱动拒载整卷 → 菜单跌 "built-in configuration / no boot entry found"（同时解答 "Return to firmware→failed to find an operating system" 与 Tools 秒回的伴生现象）。64 MiB 修复。
- **读者 ESP 首验全绿（2026-10-30）**：`reader-esp.img` d3e6d468（64MiB）刷 recovery_a → 菜单 3 秒自动进 reader → v71 内核起（probe 报 "m2381 forensic log reader 7.3.0 / usb: acm on a600000.usb"，SHLOG 窗 magic=SHLOG001，userdata=sda22）→ **NCM 51s 立**（1d6b:0104）→ **dd 读回对拍双绿**（recovery_a 前 64MiB = d3e6d468 ✓、boot_b 前 31,793,152B = d0200a6b ✓，读者通道即刷写取证仪器）→ **fetchchunk 15/15 块 31s SHA MATCH** → read-m2381-ufs-log.py 解析 337 槽旧 boot（r55e 代遗产，header 契约守住，residue 正确截断）。
- **下会话开工单**：①iOS payload 重建（96MiB/forensic=false/sheng-log=1 已配齐）+ 刷 recovery_a 回 PAC 车道 → PAC 墙 RE 四步（会话十四三轮档判别路径原样，注意 seal-source-crypto-kernel 相互作用）；②取证使命流程 = 刷读者 ESP → 采证（probe/dd/fetchchunk）→ 刷 iOS payload 回，各轮 dd 读回照旧；③回退链新增：boot_b d0200a6b、recovery_a reader-esp d3e6d468、payload 0f1013f4（合装轮，已不在位）；④最小读者内核上岗路径 = FV 修复已就位，可单独 A/B（v71 仍为现役 reader）；⑤探针轮配置（panic_screen/entry_probe_reset_delay）仍待 PAC 修复轮收队。

**会话十五补档（2026-10-30 收队）**：iOS payload **`58676a6c` 已出件已刷**（96MiB/forensic=false/`sheng-log=1` 在 args；存档 out/m2381/payload-58676a6c.img；recovery_a 现役=此件）。首 boot **PAC 墙逐字复现**（pc 0xffffff00a98a2dc / x8 目标 0xa72f938 / pages=0x125b8e 与会话十四三轮全同；lr/x8 仅 PAC tag 域随 boot 键派生不同）——**FV 内存图修复对内核路径零扰动，墙确定性**；证据=屏摄 IMG_20261030_061956.jpg（用户拍板以屏摄为准）。SHLOG 窗采集顺延下会话（墙点即冻结，写入时长存疑，fetch 前不可再 boot xnu 以免窗口滚动）。**设备收队现态：boot_b=d0200a6b（修复版 FV）/ recovery_a=58676a6c / userdata 未动；下会话刷卡先照此核对。判读=下会话（RE 四步判别路径在会话十四三轮档）。**

**★会话十六 = PAC 墙根因定谳 + 修法落件（2026-10-30 深夜，屏摄 IMG_20261030_061956.jpg 判读 + RE 四步全闭环；主树 patches/xnu-12377/source-corecrypto14-bootstrap.patch 已重生成，kernel 重建后台进行）**：

- **RE 四步定谳（全部实证，A/B 探针免了）**：
  - **①签名/鉴权形态**：pc = `_source_corecrypto14_bootstrap+0x84` = `blraaz x8`（**IA 键、零修饰符**；capstone 全内核普查 = 3657×blraaz + 304×braaz vs 仅 7×blr——**native 编译约定 = 一切未注解函数指针 (IA,0)**，llvm 23（Homebrew 23.1.1）`-arch arm64e`/`--target=arm64e` 默认行为，独立 .c 实测复现）。x8 ← `ldur x8,[x19,#0xb4]`（kmod_info pack(4) 布局 start@0xb4 非对齐所以 ldur）= **donor `com.apple.kec.corecrypto` kext 的 kmod_info.start**；x0/x19=0xfffffff00b5f9a90 = donor kext 数据区（collection __DATA 后段 0xb47c000+0x17DA90）。机制 = 上游 `source-corecrypto14-bootstrap.patch`：扫 fileset 找 donor corecrypto 的 kmod_info → `module->start()` 起 donor 唯一 provider（防双 provider 致 PRNG 上下文未初始化）；x24=`__startup_EARLY_BOOT_entry_source_pthread_bootstrap_23+0x18` = startup 游标（pthread 姊妹补丁直调 native `pthread_start`，非同族）。
  - **②0xa72f938 真身**：kernel.json `segment_layout`（native→collection 搬运映射）定位 = **donor __TEXT_EXEC 内 donor corecrypto kext 的 start 函数**（native __TEXT_EXEC 搬至 0xa73c000+，目标在其下 0xC6C8 = donor 原代码；非 reporting-stub 非 native；入口首字 0xd503245f = **BTI c**（会话十四记 PACIASP 有误，"真函数入口"结论不变））。
  - **③auth_rebase 存续 = 存续且已被处理**：槽在 __DATA 页 216 链上（144 节点），文件槽值（我件与 donor 23F84 **逐字节相同**）= 0x8010e4fe0372b938 = auth_rebase 链条目：isAuth=1 / **key=0(IA)** / addrDiv=0 / **diversity=0xE4FE** / cacheLevel=0 / target=0x372B938 → base(0xfffffff007004000)+target = **0xA72F938 与 x8 低位逐位合**；运行值≠文件值（低位已是终值地址）→ `kernel_collection_slide`（dyld_kernel_fixups.h；HAS_APPLE_PAC=1 由 native-pac-keys.patch 定于 BCM2837.h）已按链条目 **(IA, 0xE4FE)** 重签。emit 对账：kext_chain_sites_kept=82701（含本槽）/ kext_pointers_rebound=47166（donor kext 数据区 19 个链页被重绑为终值 = 非丢失；donor 内核数据区链页随 donor 内核本体淘汰 = 设计内）。
  - **④根因 = α 上下文失配的精确形态：同键（IA）、不同 diversity（0xE4FE ≠ 0）**。写侧 boot fixup 按 donor 链条目签 (IA, 0xE4FE)；读侧 native 默认 blraaz (IA, 0) → AUTIA 恒败 → trap。**β（fixup 链丢失/未处理）证伪**（槽被正确处理）；**γ（键态相位）证伪**（键同侧、native-pac-keys 每 boot 一次编程早于 arm_init 的 fixup pass）。会话十四三轮档"x8 高位=裸链条目"解读作废（低位=已处理终值地址、高位=真签名 tag）。
- **修复（已落件）**：主树 `patches/xnu-12377/source-corecrypto14-bootstrap.patch` 重生成——`source_corecrypto14_start_once` 内 `module->start(module,data)` 改为经 `typedef kern_return_t (* __ptrauth(0, 0, 0xe4fe) …)(kmod_info_t *, void *)` **注解 lvalue** 调用（`#if __has_feature(ptrauth_calls)` 守卫，非 ptrauth 构建回退原样）。实测该写法 clang 23 产出 `mov x16,#0xe4fe; braa/blraa` 与 donor 签名约定精确对齐；**直 cast 到 qualified type 被工具链拒**（"cannot cast to __ptrauth-qualified type"）必须走 lvalue。流程 = 旧 patch 应用到 pristine feed 副本 → 改结果 → 重 diff（hunk 头 -502,3 +502,141 自动正确）→ 对 pristine dry-run PASS。0xE4FE 为 23F84 donor 链条目原值（24A446 同族预期同值）。
- **seal-source-crypto-kernel 相互作用 = 无**（seal 只 prepatch __text static_if 位点 + 重算 __TEXT_EXEC/__text HMAC，不碰 __DATA 链与 donor 区；bootstrap 改动自动进 seal 重算范围）。emit `component_hmacs` 已含 com.apple.kec.corecrypto = donor 组件 POST HMAC 重封由 emit 预接（donor 自身 POST 预期通过）。
- **重建/判读单（已全部完成，主树 commit `481d33e` 未推送）**：kernel 重建绿（首版 cast 写法被 clang 拒——**cast 到 qualified 类型本体非法，必须 cast 到"指向 qualified 类型的指针"再解引**，kernel.log `cannot cast to __ptrauth-qualified type`——修正后重跑）→ **kernel `a36d5103`**（新 unstripped 反汇编实证 bootstrap+0x84 = `mov x17,#0xe4fe; blraa x8,x17`，旧 `blraaz x8` 已消失）→ **payload `07080eb4`**（96MiB；ESP 内核副本 `bfa4f979` sha 对针 + 修复字节在位；cpusub/entry-probe(--panic-screen --no-reset)/seal(args 全队)/dtb 全链跑通；平铺件 `out/m2381/payload-pacfix-07080eb4.img`）。**刷写卡 = recovery_a ← 07080eb4（100,663,296 B，刷前 sha 门 + 读回对拍），boot_b=d0200a6b 与 userdata 不动**。判读门：①原 PAC panic 消失②阶段史推进过 clock_service_create/SYSCTL 段 startup（bootstrap 成功无打印）③**下一墙候选预注册**：donor corecrypto start 起来后 crypto KPI 表（g_crypto_funcs 一族）donor 写侧 Apple 约定 vs native 读侧 (IA,0) 的边界同族风险；emit 生成的 native→donor authenticated thunk（0xfffffff007563df0 `blraaz x0` 一族）同 conventions 面。
**★会话十七 = stub 桩首点定谳 + body-hash 配对通道 + 锁三件套 fixture（2026-10-30 深夜续，用户刷 07080eb4 后屏摄 IMG_20261030_074633.jpg；精简流程）**：

- **PAC 墙修复实机生效实锤**：panic 形态全变——旧 PAC panic 消失，donor corecrypto start 已跑起来，死在**新墙 = `_unixnu_fileset_unmapped` 桩首点**：`fileset: original component called an unmapped kernel function from 0xfffffff00a709f00 (x0 0xfffffff902f5095e x1 0x5c)`。桩语义（fileset-unmapped-call.patch）= emit 把 donor 对未配对内核函数的 bl 重写到单桩，报告返回地址+前两参。
- **首点定谳 = memset**：donor 原件 0xa709efc 的原始 bl → **0xa76ce30** = donor 内核 memset（体形=广播字节填充；x1=0x5c=填充字节）。我们 native `_memset/_secure_memset`（同地址双名）**前 0x94 字节逐字节相同**但函数尾部填充跨构建不同 → 边界哈希/shape digest 全不匹配 → matcher 十余通道全漏。全表 = **839 目标/13653 桩点**（top: 0xae40858×1679 点/25 组件、0xa7f4128×1182、0xa7f3548×1011——全是 donor 内核函数被 native 覆盖后未配对者）。
- **修法①（设备仓 kits/XnuPrelink/fileset_match.py，commit 8f85136）= `anchor_body_hash` 通道**：donor 函数体截断到第一个终止 ret 后在 native 文本做原样搜索，唯一命中且落在 native 函数起点才配（ret 后填充是构建差异；字节相同=行为相同；带镜像相对引用的函数天然不会假配）。v1 体哈希版因 end 边界含填充失效；v3 ret 截断版生效（memset+0xa76c000 自动配对）。放宽"多副本任选"会破坏 reference-order 级联（8302→8262），已回退唯一性判据。
- **修法②（fixture）= 锁三件套**：0xa7f3548=`lck_spin_lock`（CAS 0→cpu号@+8，cpu号=tpidr_el1+0x4f0）、0xa7f3fe4=`lck_spin_try_lock`、0xa7f4128=`lck_spin_unlock`——身份判据=FAST 路径 CAS 形态+与上游已钉的 `_lck_rw_unlock_shared`（0xa7f62bc）同区；kext 把 lck 当不透明对象，配到我们实现后布局自洽。fixture 落 `fixtures/fileset-23F84-symbols.json`（108→111 条）。
- **出件**：kernel **c35cf1f6** → payload **fed3dd74**（96MiB，ESP 副本 83d55ef4；PAC 修复字节在位；corecrypto 桩 12→7、全场 13653→12682 点）。存档 `out/m2381/payload-stubfix-fed3dd74.img`；设备仓 commit `8f85136`。
- **corecrypto 残余 7 个（下轮判读，靠下次 panic 的运行时参数点名）**：0xa7b50f8（全局∈{3,4}检查）/0xa7f3364（tagged 分配）/0xa938124/0xa96a0c4（20 点！全局 0xb2d8008+8 指针装入 [x0]，疑 OSObject 基类构造器存 metaClass，无写者未定名）/0xacf9870（tag 0x6827）/0xae96de0（claim-once 全局 0x7d61af0）/0xb007294。识别工具 = `temp/pac-wall/stubid.py`（site→donor 原目标+体反汇编+native 对照）。
- **上游检查（用户问）**：feat 强推重写 f161cd2→9bfc45a，净新增=纯工具脚本（build-fileset-release-reference/bundle-ipsw-trust/trust-append-module/turnip-kmd-patch——把我们手搓的 RELEASE 参照件流程正名化），**无本墙修复**；rebase 收队项未做。

- **开放项**：①0xE4FE 的字符串原型未破（BSD16 穷举常见命名串未中；硬编码即可，链条目本身是权威）②native 侧 (IA,0) 默认的编译开关源头未定位（makedefs 全树无 fptrauth 旗标 = llvm 23 arm64e target 默认；实测确认）——上游 sheng 同款构建必撞同墙，本修法可上游化。

**★会话十八 = corecrypto 残余 7 桩全数定名并出件（2026-10-30 上午，用户刷 fed3dd74 后屏摄 IMG_20261030_090251.jpg）**：

- **新墙判读**：PAC 墙与 memset 桩均未再现，首点前移到 `from 0xfffffff00a6d2848 (x0 0xfffffffae0678c20 x1 0xfffffffae0678b60)`——x0=内核栈 `sp+0x20` 上的 out 参数（调用点前一条 `add x0, sp, #0x20`），site-4 = `bl 0xa96a0c4`，正是会话十七残余表第一项。
- **7 个身份全部定名**（判据 = native 侧同形函数 + 符号表唯一，配 `build/xnu/fileset-reference/` RELEASE 参照件与 `temp/pac-wall/stubid.py`）：
  - 0xa96a0c4 → `_clock_timebase_info`（5 指令 getter，`*x0 = gTimebaseInfo`；与 native 仅 adrp/add 常量不同 → body-hash/shape 两通道天然漏配）
  - 0xa7b50f8 → `_panic_active`（`(panic_state-3)<2` 的 cset lo；corecrypto 2 点）
  - 0xae96de0 → `_register_crypto_functions`（claim-once：槽已设返 -1，否则存参 + 置 flag 字节）
  - 0xb007294 → `_register_and_init_prng`（`entropy_init(64, entropyseed)` → prng_ctx 存 → cckprng_funcs 逐槽拷贝，ptrauth 判别子 0x6961/0xf60a/0x5d01/0x4d5f/0xedeb/0x3051 与 native 逐槽相同 → 4×cc_clear；native 多 inline boot_cpu/prng_ready assert）
  - 0xa938124 → `_read_random`（refresh 0x4d5f + generate 0xedeb、0x100 分块 + cpu_number；native 走函数调用未内联 tpidr_el1）
  - 0xa7f3364 → `_lck_mtx_alloc_init`（kalloc + tag `0x22000000|flags` + 锁组引用；donor 内联 lck_grp_reference，参照件中位于 `_lck_mtx_init` 前一位且同形）
  - 0xacf9870 → `_sysctl_handle_string`（strlen+1 → oldfunc → newfunc 截断写；与参照件逐指令同，差异仅 Apple 串判别子 0x6827 与 PAC 硬化 store；donor 序夹在 handle_long/handle_opaque 之间。会话十七记其"0 点"= 非直接 bl 路径，本表按 stub 目标身份收编）
- **落件**：设备仓 `3a7c6ef`（fixtures 111→118，每条钉 donor 字节 sha + 证据行）；kernel **171bee9d** → payload **`02d7aa05`**（96MiB，ESP 内核副本 55e07c55；trust `ebeaf7fb` + data seed `d04a0153` + ADT `8bfa8cad` + panic_screen；PAC 修复字节在位实证 = `source_corecrypto14_bootstrap+0x84` `mov x17,#0xe4fe; blraa x8,x17`）。存档 `out/m2381/payload-stubfix2-02d7aa05.img`。**构建坑复现（r85 同款）**：`--trust-cache`/`--data-ramdisk` 必须显式传——首轮裸 `./unixnu payload` 出的 `50ea4195` 缺 ESP trustcache+initrd（已删，勿用）；红线件 = `out/m2381/ipados-26.5.2-23F84/m2381/original-trust-23F84.bundle` + `out/m2381/ci-artifacts/verified-root-grow/owner-data-fixture/scratch-data.img`。
- **刷前门（全绿）**：prelink reviewed 111→118、全函数 8294→8301、stubbed 1674→1659；**stubbed 表里 corecrypto 条目消失**（原 7 目标全绑定）；出件内核 7 个 call site 逐点反汇编 = bl 指向配套 native 函数（布局偏移 0x346c000 自洽，ESP 副本复核同）。
- **残余（观察项）**：corecrypto `missing` 表剩 1 条数据引用 `0xfffffff007ca3910`（非函数桩）。
- **刷写卡**：只刷 recovery_a ← `payload-stubfix2-02d7aa05.img`（100,663,296B，刷前 sha 门 + 读回对拍），boot_b=d0200a6b / userdata 不动。判读门：①本 panic 消失②corecrypto start 继续（若新墙仍在 corecrypto = 间接调用/数据链路径，stubid+body/reference 对拍续办；换组件 = 新墙照流程）③pages/free 行仍在。

**★会话十八续 = 新墙定谳（corecrypto 静态 sysctl 的 handler 鉴权）+ GPU 接装盘点（2026-10-30 上午，用户刷 02d7aa05 后屏摄 IMG_20261030_092650.jpg）**：

- **判读**：桩墙与 PAC 启动墙均未再现，corecrypto 继续起，撞出新类型墙——`PAC failure from kernel with IA key while authing x16 at pc 0xfffffff00ad17e48`（x16=0x7c9ac0700ad18a9c，x9/x17 为字段地址+哈希判别子）。**pc 归属 = native `_sysctl_register_oid_locked+0x1e8`**（emitted→build 偏移 0x346c000 反查）。对象 = `0xffffffff00b6000d0` 处的 `sysctl_oid`，在 **corecrypto 的 __DATA（0xb5f9a90..0xb6058f0）** = donor kext 自己的静态 sysctl；槽 `oid+0x30` 的 donor 链条目 = `(auth=1, key=IA, diversity=0xE2E, addrDiv=0)`，oid_arg1（+0x18）参与判别子哈希。
- **根因（α 家族新形态）**：源码 `ptrauth_auth_and_resign(handler, IA, ptrauth_function_pointer_type_discriminator(typeof), IA, blend(&field, os_hash_kernel_pointer(oid_arg1)))`——旧鉴权用**编译期类型判别子**。Apple 构建对该类型签 0xE2E，我们的 clang 产出 0（=会话十六开放项 "native (IA,0) 默认" 同源），出件按 donor 链条目原样重签 → AUTIZA 恒败。
- **修法（已落件）**：主树 `patches/xnu-12377/sysctl-handler-donor-resign.patch`（进 `profiles/arch/arm64e/patches/xnu/series`）——register 路径改 `ptrauth_strip`（xpaci，不验证）后按本内核判别子重签；unregister/dispatch 用本内核签的判别子，不动。实证 = kernel `8a493864` 反汇编 `ldr x10,[x9,#0x30]! / xpaci x10 / …hash… / pacia x10,x9`，旧 `autiza`+brk 消失；ESP 副本 `1ea8d3c0` 同。
- **出件**：kernel **8a493864** → payload **`a62f5e53`**（96MiB，trust ebeaf7fb + data seed d04a0153，panic_screen 在）。存档 `out/m2381/payload-sysctlfix-a62f5e53.img`；刷写卡 = 只刷 recovery_a ← 此件，boot_b d0200a6b / userdata 不动。
- **GPU 接装盘点（用户令"和上游一样加 GPU"，同 SoC）**：①6 子模块已在 feat pin（Adreno740/AdrenoMetal/AdrenoFirmware/AdrenoPM4/AirNir/ArmSmmu4K，无需 bump）②三件固件按 pin 拉齐（`scripts/fetch-adreno-inputs.py`；**无 token 走 api.github.com 会 403 限流，本轮用 keyring token 补拉**）：a740_sqe.fw 75,380 / gmu_gen70200.bin 66,844 / sm8550/sheng/a740_zap.mbn 12,088，sha256 全对，落 `.cache/firmware/sheng-gpu/` ③`UniXNU-sm8550-common` ≡ 设备仓 `soc/sm8550`（kits/Sm8550Platform 已有 ShengGmu/ShengGmuCommandDb/ShengGpuSmmu）④**唯一缺口 = `ShengGmuVotesSource.h`**（板级 OPP/BCM/带宽表）；消费侧 `ShengGmuVotes.c` **已在 `reference/unixnu-sheng/package/`**，只差数据表，上游公开仓均无该头（gpu.md：kept out of git）⑤上游配方 `build-adreno740-kits.py` 用 `xcrun`（macOS），Linux 需改 toolchain clang/ar ⑥DTS 有 gpu/gmu/gpu_smmu（disabled）但无 OPP 表；sheng 侧只加 `&gpu/&gpu_smmu status=okay` ⑦Metal staging（add-gpu-plugin-data.py --publish）依赖 macOS 镜像工具 → Linux 先做到内核侧 GPU attach/Turnip。
- **风险预注册**：sheng 的 `a740_zap.mbn` 是 OEM 签名件，m2381 的 PIL 鉴权可能拒；兜底 = 读者内核从本机 firmware 分区取自家 zap（dd 通道已有），勿盲刷。
- **★nura 侧供给（2026-10-30，用户问"缺的能从 nura 拿吗"；答案=全齐）**：①**本机固件**在 `~/work/meizu20/meizu20-m1/vendor-dig/vendor-root/firmware/`（另有 `meizu20_linux/vendor-fw-harvest/mainline/lib/firmware/qcom/sm8550/` 同一份）：`a740_zap.mbn` sha `3cc594a0…`（12,088B）、`a740_sqe.fw` sha `2b49a172…`（74,804B）、`gmu_gen70200.bin` sha `1a2a419c…`（66,844B，**与 sheng 件逐字节相同**）。**实锤：zap/sqe 与 sheng 件不同（sheng zap `4fd56fef…`、sqe 75,380B）→ 之前"同 SoC 直接吃 sheng 固件"会翻车**。已按 m2381 pin 落 `.cache/firmware/m2381-gpu/`（qcom/{a740_sqe.fw,gmu_gen70200.bin}, qcom/sm8550/a740_zap.mbn）。②**板级 OPP/带宽数据**在 `~/work/meizu20/meizu20-m1/dts/sm8550-meizu20-stock.dts`：`qcom,gpu-pwrlevels-0` 9 档 = 719/680/615/550/475/401/348/295 MHz（+ 同 level 的低档），每档带 `qcom,acd-level`（0x882e5ffd/0xa82f5ffd/0xe0285ffd/0xc02a5ffd/0xe02b5ffd/0xe02d5ffd/0xc02f5ffd）与 `qcom,level`（0xe0/0xc0/0x90/0x80/0x50/0x40/0x3c/0x38/0x34）+ bus 索引；`qcom,bus-table-ddr` 10 档 = 0/2,136,718/3,000,000/6,074,218/6,671,875/8,171,875/10,687,500/12,449,218/14,398,437/16,500,000 kBps；`qcom,gmu-freq-table = <220MHz 0x40 550MHz 0x80>`。③**BCM catalog** 从 mainline a740 条目（`.cache/adreno-source/.../a6xx_catalog.c` chip 0x43050a01）：SH0 buswidth 16 / MC0 buswidth 4 / ACV fixed+perfmode=BIT(3)+perfmode_bw 16,500,000，`gmu_chipid=0x7020100`。
- **GPU 剩余工序（数据已齐，纯工程）**：①写 `ShengGmuVotesSource.h`（上列三张表；消费侧 `reference/unixnu-sheng/package/ShengGmuVotes.c` 已在树）②`build-adreno740-kits.py` 去 xcrun 化（用 toolchain clang/ar，本机 Linux）③DTS 开 `&gpu/&gmu/&gpu_smmu`（sheng 只加前两个，gmu 的 reg 在 stock 是 0x3d68000 vs 我们 dtsi 0x3d6a000，**需对表核实**）④defconfig `CONFIG_XNU_NATIVE_ADRENO=y`+组件/archive 条目 ⑤payload gpu addon；Metal staging 仍限 macOS。全部在 `CONFIG_XNU_NATIVE_ADRENO=n` 后，**不扰动现役刷写链**。
- **会话十八三刷判读（2026-10-30 09:48 屏摄，刷 a62f5e53）：第 3 堵墙 = crypto KPI 表**：`PAC failure ... while branching to x2 at pc 0xfffffff00aee6958`。pc 归属 = native `SHA512_Init`（libkern/crypto/corecrypto_sha2.c），失败指令 = `braaz x2`（IA、零修饰）尾调 `g_crypto_funcs->ccdigest_init_fn`；`g_crypto_funcs`（全局 0x7cf1c70）→ corecrypto 自己的表 `0x82f3048`（corecrypto __DATA_CONST，文件态全零 = 运行期填）。**x2 = `0x34e36cb0_0a6fa120`：低 32 位恰是 corecrypto 函数 `0xa6fa120`（已验 pacibsp 函数起点、digest-init 形态），高 32 位不是合法 arm64e tag。** donor 里指向该函数的 7 个槽全是 `auth=1, key=IA, diversity=0, **addrDiv=1**` 的链条目（出件链字 0x80110000036f6120，解码 = target 0x36f6120+base、step=2 正确），即 donor 侧按**地址修饰**约定签，我们的消费者按零修饰 branch → 不匹配（=会话十六预注册的 "crypto KPI 表边界" 兑现）。**与 sysctl 墙同源（α 家族），但这次值本身不是规范签名态 → 盲改消费者有风险**；下一步 = 给 `register_crypto_functions`（已是我们的 native 绑定，corecrypto start 时必过）加一次性探针，把收到的表指针 + 前 8 个字打进 panic 行，一个刷次拿到表内全部条目的真值再定修法（probe 后立即撤）。
- **探针件（会话十八四步，2026-10-30）**：`patches/xnu-12377/crypto-funcs-probe.patch`（进 arm64e series）在 `register_crypto_functions` 注册时 panic 打表指针 + 前 9 个字（`unixnu-probe crypto_functions %p [0..8]`），HAS_APPLE_PAC 下无条件生效，**一次性**——判读后从 series 摘掉。出件 kernel `abe0d73c` → payload **`8a40d909`**（96MiB，ESP 内核副本 `0d8c9fb5`，探针串在 ESP 副本内实证；trust/seed 同前）。存档 `out/m2381/payload-cryptoprobe-8a40d909.img`。判读三选一：①4 个函数指针都是规范带 tag（`XXFFFFF00A6FA120` 形态）→ 失败值另有来源；②"高 32 签名 + 低 32 地址"两段式 → 在 `register_crypto_functions` 里按表重签（该函数已是 native 绑定）；③全零 → 表运行期后填，探针位置后移。回退 = 刷回 `a62f5e53`（sysctl 修复件）。
- **探针判读（2026-10-30 10:09 屏摄，IMG_20261030_100927）**：`unixnu-probe crypto_functions 0xfffffff0082f3048` → `[0]1de141b0_0a6fa120 [1]29f21030_0a6fa828 [2]0 [3]61c21530_0a6f8e50 [4]fffffff0082f1b00 [5]fffffff0082f2c30 [6]fffffff0082f2ee8 [7]fffffff0082f28a0 [8]fffffff0082ef338`。**结论：两种指针格式并存**——数据指针（[4]–[8]，md5/sha1/sha256/sha384/sha512 的 di）是规范地址；四个**函数指针**是 `高 32 位签名 + 低 32 位集合内偏移` 的打包形（低 32 位解出 corecrypto 函数 0xa6fa120/0xa6fa828/0xa6f8e50 等），对应 donor 侧 `autiza` 32 位鉴权（kext 集合 ABI），我们的 clang 只产 `braaz` 64 位形 → 恒败；[2] 为 0（该表字段在 donor 侧未填，属预期，不要在后续误报）。**该表在 donor/出件文件里都是全零 BSS（0x82f3048 无任何链条目），值由 corecrypto 运行时写入**——所以 emit 侧修不了，只能修读侧。
- **修法（已落件，替代探针）**：`patches/xnu-12377/crypto-funcs-unpack.patch`（探针补丁已从 series 摘除）——在 `register_crypto_functions`（已是 native 绑定）里把表拷进内核静态副本，逐槽把"打包形"还原为 `0xfffffff000000000|低32` 并按 IA/零修饰重签（`ptrauth_sign_unauthenticated`），`g_crypto_funcs` 指副本；数据指针/规范指针原样保留（判据 = 高位 `((v>>32)&0xffffff)==0xfffff0` 或解出地址不在 `__TEXT_EXEC` 区间则不处理）。此修一次覆盖表上所有消费者（SHA1/256/384/512、HMAC、AES…）。
- **出件（unpack 修法）**：kernel **`892ebfe1`** → payload **`c3b36e61`**（96MiB，ESP 内核副本 `2b22aba1`，探针串已移除实证；trust/seed 同前）。存档 `out/m2381/payload-unpack-c3b36e61.img`；回退 = `a62f5e53`。反汇编实证 = `register_crypto_functions` 内 `mov w2,#0x1c0`（448B 表拷贝）+ 逐槽高位判定/区间判定/重签循环。
- **五刷判读（2026-10-30 10:20 屏摄，刷 c3b36e61）**：**unpack 修法生效**（crypto 表墙消失），新墙 = `Invalid spinlock 0x…: <0x000000002200010f 0x0> @locks_arm.c:343`。锁字 = `lck_mtx` 的 tag `0x22000000|flags`，type 字（+8）=0 → **一个 mutex 被当 spinlock 锁**。**根因 = 会话十七 的三条 lck fixture 类型标错**：donor 0xa7f3548/0xa7f3fe4/0xa7f4128 的体全部操作 **[lock+8]**（`casa 0→cpu` 加锁 / try 加锁 / `casl` 解锁），而 `lck_spin_t` 是 **data@+0、type@+8**（`lck_spin_lock` 先验 `[+8]==0x11` 再锁 `[+0]`），mutex 才是 data@+8——当时按"CAS 0→cpu@+8"认成 spin 是误判。修法 = fixture 三条改名 `_lck_spin_{lock,try_lock,unlock}` → `_lck_mtx_{lock,try_lock,unlock}`（sha 钉不变，只换 native 符号名），设备仓提交。
- **出件（lck 改名修法）**：kernel **`e6217d36`** → payload **`30c11c32`**（96MiB，ESP 内核副本 `f1c9fb0d`）。实证 = corecrypto 里 7×lock / 9×unlock / 1×try / 1×alloc_init 的 call site 全部改指 native mutex 实现（`IOLockLock/TryLock/Unlock` 与 `lck_mtx_*` 同址别名）。存档 `out/m2381/payload-mtx-30c11c32.img`；回退 = `c3b36e61`。

**★会话十九开工单（2026-10-30 交界；用户拍板"安排下一轮，拍屏下会话发"）**：

- **用户动作**：①只刷 recovery_a ← `out/m2381/payload-mtx-30c11c32.img`（sha256 `30c11c32…250e`，100,663,296 B），boot_b `d0200a6b` / userdata 不动；②开机后把 panic 屏整段拍全（若越过了前六堵会是新类型）；③回退件 = `payload-unpack-c3b36e61.img`（已验证能走到 crypto 表墙之后）；④**不要连续多次重启**，保持单一证据点。
- **我这边顺序（含离线先行项，不等刷机）**：
  1. **离线杠杆 = donor 内核导出符号**（目标：把 1659 个未配对函数里的 kext-facing 目标批量按名字绑定，替代"一堵一刷"）。已知证据：kernel entry 的 load command 表里 cmd=`0x32`（off `0x9450`，cmdsize `32`），其后六字 = `0x2 / 0x1a0500 / 0x1a0500 / 0x1 / 0x3 / 0x4f30000`；**文件偏移 `0x1a0500` 处是符号名字符串区**（首条 `AVD_RD_DMA_LVP_COLO_DMA_CONFIG`）→ 导出/符号名区就在附近。步骤：先判 `0x32` 语义（非标准 `0x80000022`；排除 LC_DYLD_EXPORTS_TRIE `0x80000033` 的普通形态），定位 trie 头（ULEB terminal/flags/address 结构），解出 name→address；再按"native 侧同名唯一函数地址"批量落 reviewed fixture（how=`export-name`），逐条写 evidence，唯一性为唯一收编判据。
  2. 若导出表不可用：给 matcher 加**通用通道 = 去 BTI 前缀 + 忽略 adrp/add 常量后比对整函数体**（现在 body-hash 只吃逐字节相同），并把 1659 个 stub 目标按组件出清单 + 调用点数排序，先修热点。
  3. 判读屏摄 → 归类（桩 / PAC 约定 / 32 位打包指针 / 锁类型标错 / 其他）→ 修 → 重建 kernel+payload → 出件命名 `payload-<类>-<sha8>.img`。
  4. 每次修法回写 fixture evidence（判据必录）；映射只收唯一命中。
- **红线**：`CONFIG_XNU_NATIVE_ADRENO` 保持 `n`（GPU 等进桌面）；payload 命令必须带 `--unique-chip-id 3645ad55d47e77c1 --trust-cache out/m2381/ipados-26.5.2-23F84/m2381/original-trust-23F84.bundle --data-ramdisk out/m2381/ci-artifacts/verified-root-grow/owner-data-fixture/scratch-data.img`（漏了会少 trustcache/initrd，r85 坑）；刷写只碰 recovery_a；**SHLOG 在 UFS 起来之前是空的**（写手要 BootProgress 监视器 tick + native_ufs_log_open），早期 panic 别白跑读者任务。
- **判读门**：本墙消失 + corecrypto start 继续；prelink 统计（reviewed / body-hash / reference-order 配对变化，stubbed 数下降）；出件内核 call site 逐点反汇编（布局偏移 0x346c000 口径）。

**★会话十九 = 批量消账：导出符号杠杆判死 + BTI 通道落地 + kext 入口 PAC 墙（2026-10-30 上午，用户刷 30c11c32 后屏摄 IMG_20261030_103414.jpg）**：

- **新墙判读（第七堵）**：`PAC failure from kernel with IA key while authing x16 at pc 0xfffffff00aea68fc`。pc 归属 = native `OSKext::validateKextMapping(bool)+0x130`（emitted→build 减 0x346c000 = 0xfffffff007a3a8fc，源码 xnu `libkern/c++/OSKext.cpp:8165` 的 `ptrauth_auth_data(address, ptrauth_key_function_pointer, 0)`）。x16 = x21 = `0x54d301b0_0a4cdf18`，来自 `ldur x21,[x29,#-0x50]`，源头是 `ldr x21,[kmod_info,#0xb4]` = **com.apple.kec.Libm 的 `kmod_info->start`**。**值解 = 32 位打包指针**：低 32 位 `0x0a4cdf18` = emitted base(0xfffffff007004000) + donor 链条目 target `0x34c9f18` = `0xfffffff00a4cdf18`（Libm __TEXT_EXEC+0x1e18，ESP 内核对位）；高 32 位 `0x54d301b0` 是签名段。**文件态同址** = donor auth 链条目 `0x8010e4fe034c9f18`（isAuth=1 / key=IA / addrDiv=0 / diversity=**0xE4FE** / target=0x34c9f18，donor 与出件逐字节相同）→ 与 corecrypto KPI 表**同族**（donor 侧按编译器类型判别子签的 32 位打包形态），我们的 `autiza` 用零判别子 → 恒败。注：panic 行 OCR 上 `0a4cfd18` 是误读，实为 `0a4cdf18`（裁切放大核对）。
- **修法（已落件）**：`patches/xnu-12377/kext-entry-donor-pptr.patch`（进 arm64e series，位于 crypto-funcs-unpack 之后）——在 `OSKext::initialize` 从 `_PrelinkKmodInfo` 取到 kmod_info 之后归一化 `start`/`stop`：`bits[55:32] != 0xfffff0` 判为打包形态 → `0xfffffff000000000 | 低32` 还原地址并按 (IA,0) 重签；已是规范形态 → `ptrauth_strip` 后按 (IA,0) 重签。实证（native 反汇编，0x7a2d4e0 起）：`ldur x10,[x8,#0xb4] / cbz / and …,#0xffffff00000000 / cmp #0xfffff000000000 / b.ne` → `mov x9,#-0x1000000000; bfxil x9,x10,#0,#0x20; paciza x9; stur x9,[x8,#0xb4]`，`stop`（+0xbc）同形；ESP 内核副本 `c7dfa071` 内实证（`497d40b3`×2、`e923c1da`×2）。**修后 `autiza x16` 得到零判别子合法指针，且 `startfunc(kmod_info,…)` 的直接 `blraaz` 也成立**（只修 validate 不修字段会让下一步直接调用再崩）。
- **★导出符号杠杆判死（工作单第 1 项，证据确凿）**：`LC 0x32 @0x9450` = **LC_BUILD_VERSION**（cmdsize 0x20 → platform=2、minos=`0x1a0500`=26.5.0、sdk=`0x1a0500`、ntools=1、tool=3(Ld)、version=0x4f30000）——工作单把 minos 误当成"符号名区偏移"；文件偏移 `0x1a0500` 处确有串 `AVD_RD_DMA_LVP_COLO_DMA_CONFIG`，但那只是该偏移恰落在数据区，与符号表无关。**donor 全镜像没有任何 name→address 表**：①集合级 load command 无 LC_SYMTAB / LC_DYLD_INFO / LC_DYLD_EXPORTS_TRIE（只有 0x1b/0x32/0x5/0x80000034 + 段 + 298×LC_FILESET_ENTRY）；②298 个 fileset 子镜像逐个带 LC_SYMTAB，但 **nsyms=0 / nstrsize=1**（如 com.apple.kernel：symoff 73728000, nsyms 0, stroff 74190472, nstrsize 1）＝被剥；③镜像里唯一的符号名集合是 `__LINKEDIT` 首部 bplist（`SymbolsSets`，9 个 KPI bundle 共 **6124 个名字，无地址**；另有 `WeakRefFallbackSymbol=_gOSKextUnresolved`）＋ `__PRELINK_INFO` 的 `_PrelinkInfoDictionary`（38 条，只有 `_PrelinkExecutableLoadAddr`/`_PrelinkKmodInfo` 等地址，无符号）。**结论：按名字绑定在 donor 侧无源可依，此杠杆作废，不再投入。**
- **★批量通道（工作单第 2 项）落地 = BTI 前缀通道**：先量化了"为什么大量函数配对不上"——donor 每个函数入口带 `bti c`，我们的 clang 不产；已在 7920 对旧配对中实测：逐字节相等仅 42 对、去 BTI 后相等 443 对、再放宽到 relocation-masked shape 也只是 638 对（Apple 的 codegen 与我们的差异远不止 BTI），所以**不能用"整函数体"做通用通道**，但 **shape 锚点（relocation-masked，唯一性双侧）** 能经命名参照件大幅回收。
  - 实现：`kits/XnuPrelink/fileset_match.py` 新增 `without_bti()` 与 `anchor_bti_shapes()`（`Side.functions` 增记 `bti_digest`/`bti_length`；去入口 BTI + 去尾部 nop 后按双侧唯一绑定，`how='bti-shape'`，并纳入 `anchor_reference` 的可信来源集合）；`MATCHER_VERSION` 8→9（旧 map 自动失效）。设备仓 **bf1dac9**（含 3 条单测；`test_fileset.py` 的 `chain_relocations` 导入错误为上游遗留，与本改无关）。
  - 23F84 实测：**函数配对 7920 → 8450**（其中 reference-symbol 366→1353、reference-order 4796→4472、vtable 592→543、pure-leaf 98→81、bti-shape 9 直接命中——direct 少是因为多数 native 候选已被 order 通道占用，收益主要走命名参照链）；**kext-facing 目标命名 1338 → 1586 / 3094**；**stubbed 去重目标 850 → 700**（−18%；按 owner 求和 1687 → 1471）；**stubbed 去重站点数 3584 → 2642**（−26%；按 owner 求和 8557）。**成本登记**：`closure_pointers_unmapped` 641 → 767（+126，配对重排导致的可映射点变化，观察项）。
  - 三分类工具：`temp/batch-triage.py`（对 report 的 stubbed 表逐目标做 native-unique 形状试探；当前 native 侧唯一可救 128 个目标，其中 127 已被新通道收敛，剩 1 个——继续抠边际极低）。
- **出件**：kernel **`a0f98f77`**（89,374,720B）→ payload **`f7432f8e`**（96MiB；payload.json：ESP 内核副本 `c7dfa071`、trust `ebeaf7fb`、data seed `d04a0153`、ADT `8a5e00d6`、panic_screen=true、kernel_args 与上轮同）。存档 `out/m2381/payload-btibatch-f7432f8e.img`（100,663,296B）。回退件 = `payload-mtx-30c11c32.img` / `payload-unpack-c3b36e61.img`。
- **刷写卡（只刷 recovery_a）**：`payload-btibatch-f7432f8e.img`（sha256 `f7432f8e…bdbd237`，100,663,296B）；boot_b `d0200a6b` / userdata 不动；刷前 sha 门 + 刷后读回对拍。
- **判读门**：①`validateKextMapping` 墙消失（Libm 等 kext 的 start 真被调用，若后面再撞 PAC/桩再按类归）②corecrypto 之后继续起、kext 一个接一个 start（首个新墙位置即下一刀）③stubbed 表应显示 corecrypto/APFS 条目下降；`closure_pointers_unmapped` +126 是否兑现成运行时故障（若新墙落在某被重绑目标上，回退 bf1dac9 的单通道重跑对照）。
- **构建提速（用户问"是不是没多线程"）**：驱动 `unixnu kernel -j` 默认 **4**（`scripts/unixnu/kernel.py:198` 把 `ctx.jobs` 传给 `JOBS`），本机 16 核 → 以后一律 `-j16`；本轮 kernel 用默认 4 跑了两遍（第二遍增量），payload 已用 `-j16`。

**★会话二十 = 第八堵墙：donor corecrypto 的 entropy 回调类型判别子（2026-10-31 凌晨，用户刷 f7432f8e 后屏摄 IMG_20261031_003033.jpg）**：

- **判读**：kext 入口墙（validateKextMapping）与批量消账轮全部生效——corecrypto 继续起、Libm 的 start 已被调用并活着走完了 `init_with_getentropy` 的前半程。新墙 = `PAC failure from kernel with IA key while branching to x8 at pc 0xfffffff00a7098b4`（pc→build −0x346c000 = 0xfffffff00729d8b4 为内核侧相邻符号噪声；**实际 pc 属于 donor corecrypto 条目** `__TEXT_EXEC+0x391b4`，函数 = donor 的 `cckprng_init_with_getentropy`）。现场：`ldp x8,x2,[x19,#0x18]`（x19 = `prng_funcs` 在我们 __DATA 的副本 0xb6042a0，函数表槽 +0x18）→ `mov x17,#0xbcad` → `blraa x8,x17`。**x8 = 0xab883bf0_0a98a660：低 32 位 `0x0a98a660` = `&entropy_provide`（我们内核 `osfmk/prng/entropy.c` 的回调，native 0xfffffff00751e660 → emitted 同址）；高 32 位 = 我们 `paciza` 签的 tag（(IA,0) 约定）。** donor 用 `autiza x8, 0xbcad` 鉴权 → 我们签的 (IA,0) 恒败。
- **根因（α 家族第五形态，闭环）**：`prng_random.c:343` `prng_funcs.init_with_getentropy(prng_ctx, …, entropy_provide, NULL)`——我们把**内核侧回调指针**交给 donor corecrypto；donor 侧该回调类型 `cckprng_getentropy` 的鉴权判别子 = **编译器类型判别子 0xbcad**。**用我们的 clang 复现成功**：同一形状 `int32_t (*)(size_t*,void*,void*)` 在 `-fptrauth-function-pointer-type-discrimination` 下 `ptrauth_function_pointer_type_discriminator` 恰好 = **0xbcad**（`/tmp/gd*.c` 实验，同 host clang）；donor corecrypto `__TEXT_EXEC` 内 `mov x17,#0xbcad` 共 4 处（0xa7096ac/0xa7098b0/0xa71702c/0xa72d244），全部是该回调的存/调点——只签一次交接处即全覆盖。与 sysctl 0xE2E / kext 入口 0xE4FE 同族：**Apple 构建全线开类型判别，我们全线关**。
- **修法（已落件）**：`patches/xnu-12377/prng-getentropy-donor-discriminator.patch`（进 arm64e series，kext-entry-donor-pptr 之后）——`register_and_init_prng` 交接处 `unixnu_donor_getentropy(entropy_provide)` = `ptrauth_sign_unauthenticated(callback, IA, 0xbcad)`。实证（native 0x751d9b4）：`adrp/add &entropy_provide → paciza x16（参数 ABI 签）→ mov x6,x16 → mov x16,#0xbcad → pacia x6,x16 → …blraa x17(0x…3051 交 init_with_getentropy)`；ESP 内核副本 `0ac4d4a8` 内 `0602c1da`×5、`b09597d2`×1。HAS_APPLE_PAC 在该 TU 已可用（bootstrap 补丁同文件先例），零 include 增量。
- **α 家族判别子总表（本线 5 堵全收）**：corecrypto KPI 表（0x36f6120 打包+addrDiv）→ sysctl handler（0xE2E）→ kmod 入口（0xE4FE / kext-entry-donor-pptr 修）→ **entropy 回调（0xbcad，本轮）**；每一刀的通式 = "donor 按它的判别子签/验，我们按 (IA,0)"，修法均为"在交接点按对方的判别子签"。**预测：同为 `CC_SPTR`/回调类型的下一处会在别的 donor→native 函数指针交接上（如有），同一刀法。**
- **出件**：kernel **`14ce27e2`** → payload **`a268cd8d`**（96MiB；ESP 内核副本 `0ac4d4a8`；trust `ebeaf7fb` + seed `d04a0153`；panic_screen 在；kernel_args 同前）。存档 `out/m2381/payload-getentropy-a268cd8d.img`（100,663,296B）。主树 **44cb8a1**。回退件 = `payload-btibatch-f7432f8e.img`（上轮）。
- **刷写卡（只刷 recovery_a）**：`payload-getentropy-a268cd8d.img`（sha256 `a268cd8d…4db12`，100,663,296B）；boot_b `d0200a6b` / userdata 不动；刷前 sha 门 + 刷后读回对拍。
- **判读门**：①本墙消失，PRNG 初始化走完（getentropy 会在 per-CPU init/generate/refresh 反复被调，四个消费点同约定，一次到位）②corecrypto start 完成 → kext 继续逐个 start（下一堵按类归）③若新墙仍是 PAC-failure branching-to-xN 且 xN 低位落在 native `__TEXT_EXEC`，直接套 α 家族总表找交接点。
- **杂项**：本轮 kernel 全量重建用 `-j16`（stamp 变更触发干净重铺），编译+prelink 约 4 分钟（-j4 时代 13 分钟）。上游核过：feat 支线 2 新提交（AdrenoMetal pin bump + 6 个打包/GPU 脚本）与本线零交集，XnuPrelink 上游无新提交；设备仓 vendored fork 的 bti-shape 通道（bf1dac9）待回推上游。

**★会话二十一开工单（2026-10-31 凌晨收尾；设备已刷 payload-getentropy-a268cd8d 在跑，用户下会话交屏摄判读；电脑重启前资产清点=零待回收）**：

- **判读门（entropy 回调修复轮）**：①`branching to x8` 墙消失（x8 低 32 位 = &entropy_provide 的那发 blraa 不再失败）②PRNG init_with_getentropy 走完（per-CPU init/generate/refresh 四个 0xbcad 消费点同约定）③corecrypto start 完成 → kext 逐个 start，首个新墙按 α 家族总表归类。回退件 = `payload-btibatch-f7432f8e.img`。
- **用户已拍板（本会话执行）**：panic 屏加**校验行**——`scripts/xnu-panic-screen.c` 末尾渲染 `crc=xxxxxxxx len=nnn`（CRC32 over PANIC_TEXT 缓冲内容，zlib 口径），拍屏转录后对账杀 OCR 误读（cfd18/cdf18 教训）。约束：panic 屏代码住内核 padding 洞（PANIC_CAVE_END 0xfffffff00b9d0ff0，超限构建报错），CRC 增量 ~200B 预计塞得进；改动 = 主树脚本（b01035d 先例），只刷 recovery_a。
- **排队项（不挡判读）**：①上游 `scripts/build-fileset-release-reference.py` 参数化到 out/m2381 接走参照件构建（消 17.6MB 树外产物债）②设备仓 bti-shape（bf1dac9）回推上游 XnuPrelink ③**二维码整线取消（用户拍板 10-31）**：上游 AdvancedLogDxe/扫传是 UEFI 阶段工具（我们 UEFI 一直正常跑，问题全在内核侧）；panic 屏全 QR 立项一并撤销，CRC 校验行保留为准 ④UART 通道已否（无串口）。
- **基线备忘**：上游已 rebase 到 8fe2aaa（961c7da 记录）；下轮构建一律 `-j16`；刷写红线不变（只 recovery_a，boot_b d0200a6b / userdata 不动，--trust-cache/--data-ramdisk 必带）。

**★会话二十一执行档（2026-10-31 凌晨，两轮屏摄判读+两堵落地+CRC 上岗）**：

- **第九堵=entropy 补丁叠签自伤，已修已实测过墙**：屏摄 011739 → pc=donor corecrypto 回调 wrapper `blraa x8,x17(0xbcad)`，x8 目标=我们 `_entropy_provide`(+0x0)。根因=第八堵补丁 `ptrauth_sign_unauthenticated` 直接叠签（本机 clang 把函数引用物化成 `paciza`，(void*) 转换不剥离→`sign(sign(addr,IA,0),IA,0xbcad)`，autia 对干净址重算必败）；0xbcad 判别子本身没错，错在没 strip。修法=补丁加 `ptrauth_strip`（主树 13e2871；同 sysctl/kmod 已验证 idiom），出件二进制实证单签形态 `paciza→xpaci→pacia(0xbcad)`。上轮"批量轮生效实证"只证明 init 调用过墙，回调本体首次被调即崩——**教训：PAC 交接修法必须反汇编验证单签**。全链路账：corecrypto 自配置函数栈上 funcs 表（每项 pacia(IA,槽址|魔数)）→ 我们逐槽重签循环 → `cckprng_funcs` 六字段序 → funcs[5]=丢 x1 的 shim → cckprng_init stp arg5/6 → [ctx+24]/[ctx+32] + wrapper 注册 [ctx+0xad8]；崩溃 ctx=corecrypto 自有全局 0x0b6042a0。
- **第十堵=AppleImage4 `_kalloc_type_impl_external` 桩，已修**：屏摄 030636 → `unmapped kernel function from 0x08d419ec (x0 0x07f64320 x1 0x4)`。三重判据定名（源序夹逼 kalloc.c:2720 + 签名 `(kalloc_type_view_t, zflags)` + 体形 0xd001000|flags/cpu<<14/尾调 zalloc_ext）。修法=设备仓 fixture 118→119（**31fa4e5**）。方法链沉淀：桩点反汇编→原始 KC 同偏移字节特征→stubbed 账本交叉→map 邻居夹逼→源码函数序。坑：fileset 条目 idsize=0 按预链接序对齐（31 条含 xnu 本体 0x0a73c000+0x980000，与 +0x346c000 全自洽）；bl 目标勿用模式首址（差 8）。
- **CRC 校验行上岗（用户拍板项，主树 c296784）**：渲染 pages/free 后画 `crc=`=zlib crc32 over PANIC_TEXT 本轮拷贝（logn>0 才画）。**首轮实测即立功**：030636 屏 crc=0xe8dbdc06 与重建消息逐字节吻合（132B），转录被证明而非假定。**坑=本车道 panic cave 零余量**（eeedeb9a 版已吃满 0xa30，首版 +51B 构建炸）；瘦身（砍 len= 列、hex() 单调用）后 0xa0d/0xa30 余 0x23；洞后 0x13B 即真数据，不可外扩 PANIC_CAVE_END。
- **出件**：kernel **9e47d053** → payload **5f59a2a0**（96MiB，存档 `out/m2381/payload-kallocfix-crc-5f59a2a0.img`；trust `ebeaf7fb`/seed `d04a0153` 不变）。设备现役 = recovery_a ← 5f59a2a0（用户已刷，030636 屏摄即此件在跑），boot_b d0200a6b / userdata 不动。

**★会话二十二开工单（2026-10-31 凌晨收尾；用户拍板"批量处理 stubbed 墙，下会话分析和修，本轮只提取日志"）**：

- **本轮提取的日志（030636 屏摄，crc=0xe8dbdc06 已验真）**：`fileset: original component called an unmapped kernel function from 0xfffffff008d3ab04 (x0 0xfffffff58bd698a8 x1 0xfffffff00b038fb8)`，pages/free=0x125b8c/0x12544c。caller=AppleImage4 exec+0x7254（站点 0x08d3ab00）；原始 KC 同偏移本尊 bl → donor 目标 **0xfffffe000ae40858** ↔ stubbed 账本 `0x0ae40858`。体形速写：`bti c; cbz x0,slow;` 包装 +8 的 pacibsp 主函数。x0=带 0xf5 tag 的签名数据指针（站点上下文有 `autda x16,x17(0xcda1)` 多样化鉴权族），x1=0x0b038fb8（xnu exec 区间指针，疑回调/cookie）——下会话定性。
- **批量管线工作单（kalloc_type_impl_external=已验证模板，可复制）**：
  1. **输入**：kernel.json `stubbed` 按组件账本（donor 目标 VA + 站点数；全账合计 8552 站点/1471 目标）；map.json `functions`（已配对邻居，order-based）。
  2. **优先级=站点数×组件数**：头号大户 **0x0ae40858**（AppleARMPlatform 343 + WatchdogTimer 28 + AppleImage4 10 = 381 站点，且是多组件共享的内核核心函数）；次席 0x0afc0740（corecrypto 6+2/AMFI?/AppleImage4 6+2 等多组件）；逐表排序。
  3. **自动夹逼**：donor VA → map 邻居（上/下最近已配对）→ 我们构建的对应 native 区间 → xnu/kext 源码函数序候选 → 体形（masked shape）+签名+arg 用法三重验证 → **批量产 fixture**（original/original_size/original_sha256 自动算，evidence 半自动人工过目）。
  4. **无 native 对应者**（Apple-only helper）：小 shim 桥接（fileset-unmapped-call.patch 先例），逐个评估。
  5. **验收门**：重建后 stubbed 账本站点数单调下降；boot 推进过 AppleImage4（下一组件新墙按同法）。
  6. **风险**：fixup 勿盲改（#21 教训）；每条 fixture 必须证据行可回溯；跨组件共享目标修错代价=多组件同崩，先在小站点组件上验。
- **排队项不变**：①build-fileset-release-reference.py 参数化接参照件 ②bti-shape 回推上游 ③UART/QR 维持取消。基线：-j16；刷写红线只 recovery_a；--trust-cache/--data-ramdisk/--unique-chip-id 必带。

**★会话二十二执行档（2026-10-31 凌晨，批量管线轮+工具链税定谳；用户拍板 A+B=三通道落袋+CI 定量实验）**：

- **"为什么不直接用上游的"实证调查（用户问出来的关键情报）**：①上游 `kits/XnuPrelink` gitlink **冻结在 76452c6e**（会话十三 vendored 后零新提交）——bti-shape（会话十九）上游就没有，今天三刀上游更没有；②`UniXNU-sheng` main=**21H461 车道**（os_profile=ipados-17.7.11），其 `feat/ipados27-gpu-parity` 分支=**24A446 且 10-31 当天还在提 GPU trace/pid names=真机深水**，但该仓**无自有 fixture、matcher 未动**；③他们过桩墙靠的是 **macOS Apple clang codegen 与 donor 同构**（BTI 有、FPAC 软检查块无、分配器近→字节级 digest 通道直配），我们=全世界第一个 Linux(lld/clang-24) 建 fileset 内核上机（会话十四已记档），stubbed 大头=**Linux 工具链税**（BTI 无+**FPAC 软检查块 13052 处**+寄存器分配差）。sheng 的件不可刷（板级/ADT/组件集/UEFI 链全不同），工具链姿势可抄。
- **三通道落地（设备仓 b68da7b，MATCHER_VERSION 9→10，22 测试绿）**：①`anchor_bti_checked_shapes`=BTI 剥除+FPAC 检查块删除**组合归一**——旧 anchor_bti_shapes/anchor_checked_shapes 各只处理单一差异，双差函数（BTI 前缀∧检查块）全部漏网=699 目标残留的系统性根因；②`checked_shape` 删块判别子推广：aut x16,x17 两硬编码 → `autia/autda x16,<任意寄存器>`（`(w&0xfffffc1f)∈{0xdac11010,0xdac11810}`；native 全镜像扫描实证检查块仅两形、autda x16,x8/x9 变体在 IORegistryEntry/IOReporter 路径真实存在）；③`alias_thunks`=donor `bti c; b X` 别名 thunk（8B，393 站）配到 X 的 native，含链式，bind 例外表加 alias-thunk。**头号 0xae40858 定谳=`OSMetaClassBase::safeMetaCast`（1679 站/25 组件）**，三重验证=体形（0xcda1/0x38/0xfffc 逐字同）+arg 用法（x1=toMeta 对比 getMetaClass 链）+源序（OSMetaClass.cpp:201/291/385 夹逼）。
- **实测（本机 -j16 全量）**：emit 账本 **8552→4913 站点（−42%）/699→654 目标**；kernel **f763e9e6** → payload **28fe6d56**（96MiB，ESP 内核 905fb6fa，trust ebeaf7fb/seed d04a0153 不变；平铺 `out/m2381/payload-batchfix-28fe6d56.img`）。验收门"站点数单调下降"过。
- **残余分类（temp/batch-fixtures/）**：同形家族 87 目/525 站（Dispatch 家族 15+ 全同体小 thunk，1:1 门正确弃权，需 vtable 槽位/邻居消歧——0xaea0bf8=IOUserClient::Dispatch 48 站已锁）vs 真孤儿 560 目/4354 站（TOP=0xaeb8eb0 IORegistryEntry 区 190 站（registry 递归锁+两虚调+解锁，源序无对应=疑 122.8 新增）、0xaed7e2c IOService 区 168、0xa95ca30 锁函数 127（tpidr+0x1b8 计数器+casa+wfe）、0xaf6236c IOReporter 92）；源漂移类=donor xnu-12377.**122.8** vs 源 121.6（IOSimpleReporter::setValue 多一次调用，0.97 相似）。工具=temp/batch-fixtures/{batchid,fndiff}.py。
- **CI 定量实验（B 腿）**：run 41124044607 秒挂（**pre_build 期望 shell 命令串，`ci-owner-policy` 不是命令=127**；kernel-only 不需要 owner policy）→ 重发 **41124555115**（空 pre_build，kernel-only，xcode 26.6）；主树 m2381-ios27 force-with-lease→远端 m2381（63d7969），设备仓 main 已推 b68da7b。
- **判读单（下会话）**：①**CI 结果=macOS 建 stubbed 计数**→量化工具链税，拍板 B（macOS 建核转正）vs C（Linux `+fpac`，实测有 FPAC 硬陷阱=历轮 donor 侧 PAC failure 同步 panic 即证，开掉 13052 处软检查块）主攻；②若刷 28fe6d56：判读门=AppleImage4 之后下一组件新墙（stubid 同法），4913 站=背景噪声逐墙消；③排队：同形家族消歧通道（vtable 槽位法）、真孤儿 TOP 手写 fixture、pre_build=ci-owner-policy 的正确喂法考证（会话三十五跑通过，当时喂法待查）。**回推上游降级（10-31 用户拍板"真有必要吗"）**：通道集稳定前不发 PR——收益面≈零（全生态只有我们 Linux 建核，B 转正后连自己都不太需要），fork 自用零成本；触发条件=①通道集稳定后顺手发完整版或②出现第二个 Linux 构建者/上游主动来要。

**★会话二十二续 = CI 定量实验结果（2026-10-31 凌晨；**B 腿证明可行，工具链税定谳**）**：

- **喂法三坑全修**：①`pre_build` 期望 shell 命令串非脚本名（`ci-owner-policy` 127——那是 20H392 代 hdiutil 重建机制，27 车道不需要）；②政策对 CI 缺失 → 设备仓 **4b7e23f** 强制入库紧凑集（`.cache/firmware/verified-root-23F84/{OS,SystemOS}/{manifest,trusted-key,verification}` 27MB，`git add -f` 破例有档）+ 主树 `build-sheng-source-storage.py` 加 `.cache/firmware` 回退（**87acd33**）；③board.toml 无条件喂 `--fileset-reference` 而 CI 无 RELEASE 参照件 → 主树 kernel.py 文件不存在时降级去参（**9f0e4ce**；**本地 c8214f7 与远端 9f0e4ce6 同树异 sha**——git 通道 TLS 断连时经 GitHub Git Data API 推的，代理恢复后本地已 reset 对齐）。
- **决定性 run = 41132298280 全绿（11m48s，xcode 26.6）**：macOS 全链（63 补丁系+组件+XNU+emit）一次通过；此前 41124555115 的 `clang++ linker` Error = 设计内容忍的 bootstrap（会话十三既立），非 B 腿障碍。
- **工具链税同口径全表**（同 matcher v10/同 fixtures/同 donor 23F84/**双方都无参照件**）：

  | 指标 | Linux clang-24/lld | macOS xcode 26.6 |
  |---|---|---|
  | functions matched | 7163/21113 | **8324/21113 (+16%)** |
  | body-hash | 56 | **542 (×9.7)** |
  | pac-checked-shape | 350 | **1312 (×3.7)** |
  | vtable | 876 | **1285** |
  | data matched | 541 | **1200 (×2.2)** |
  | bti-checked-shape（我方新通道） | 858 | 28（macOS 无双差体=本就不欠） |
  | kext-facing named | 1517/3094 | **1783/3094** |
  | emit `stubbed functions`（按组件去重目标口径） | （带参照件）1234 | **932（输入更差仍反超 −25%）** |

  参照件在 Linux 值 ~1900 站（4913 vs 6841 site 口径）；CI 未上传 kernel.json，site 口径精确数下轮补（workflow upload 加 kernel.json 或 CI 内建参照件）。
- **判读 = B 治本、C 减税不足**：macOS 无参照已反超 Linux 带参照；pac-checked 350→1312 说明 FPAC 软检查块是 Linux 税大头（C 可把此项追平）但 body-hash ×9.7/vtable/data 差异 C 覆盖不了。**建议主攻 B**：①CI 内建 RELEASE 参照件（消最后一项输入差）②payload/userdata CI 化 ③932 残余=真 Apple-only 尾巴走 fixture/shim（上游 sheng 同款姿势）④本地 Linux 保留快迭代腿，C 作为 Linux 腿减税可选项。
- **出件态**：本地现役 kernel = 8b60dbb4（带参照件重出，ledger 4913/654 与 f763e9e6 全同、二进制 rerun 非确定）、payload **28fe6d56**（平铺 `out/m2381/payload-batchfix-28fe6d56.img`，ESP 内核 905fb6fa=f763e9e6 派生）可刷；CI 侧 kernel 未上传（只有日志）。

**★会话二十三判读 = 第十一堵定谳 = IOStorageFamily → kern_conf 设备开关族（2026-10-31 凌晨，屏摄 061501，fixture b3481fc）**：

- **转录验真**：`fileset: original component called an unmapped kernel function from 0xfffffff009f84d90 (x0 0xffffffff x1 0x0)`，pages/free=0x125b8c/0x12544c，**crc=0x20c30dfb zlib 精确吻合（030636 先例复用，转录逐字节可信）**。stage #1、verifiedK=0 failures=0——root mount 前的 donor 原生组件首发。前墙（AppleImage4 kalloc，第十堵）已过 = batchfix 件全链生效实锤。
- **钻法（新案例全程，已定型）**：拍屏 → CRC 验真 → 从 payload ESP 提 kernel.macho（mcopy，勿用本地旧 out/m2381/kernel=10-25 旧 lane）→ 站点 0x9f84d8c 反汇编 → **outer __TEXT_EXEC 定段 + donor KC 同 VA 截断空间（0xfffffe00 前缀折叠）定组件** = com.apple.iokit.IOStorageFamily → 原始 bl 目标 0xfffffe000ae3efd0 → batchid 夹逼。
- **定谳**：站点 bl 原目标 = donor `_bdevsw_isfree`（bsd_stubs.c 源码逐字吻合：1 参、-1→从 1 起扫、反索引→-index 起扫、bdevsw[] 0x38 步长、memcmp `nobdev` 56B 模板、界 0x17、miss 返 -1）。**整族 6 函数全 stubbed（18 站点）**：0xae3efd0 isfree×3、0xae3f0b4 add×4（内联 isfree+56B 拷贝）、0xae3f248 bdevsw_remove×5、0xae3f44c cdevsw_add×2（0x70 步长+112B 拷贝）、0xae3f5c4 cdevsw_remove×2、0xae3f70c cdevsw_add_with_bdev×2（3 参、内调 cdevsw_add）。donor 侧无独立 cdevsw_isfree（被内联）→ donor 地址序=我们 bsd_stubs.c 序一一对应。caller 流程 = IOMediaBSDClient 注册标准形：isfree(-1)→bdevsw_add→cdevsw_add_with_bdev→败则 bdevsw_remove 回滚+isfree(~major) 重试。
- **ABI 对拍全绿（未动源码）**：nblkdev 双方 24、nchrdev 双方 64（conf.c 已有 `[13 ... 63] = NO_CDEVICE`）、struct bdevsw=0x38/struct cdevsw=0x70 双方一致、NO_BDEVICE/NO_CDEVICE=eno_* 非零指针（零填槽会被 isfree 判占用——conf.c 无此问题）。**教训：grep `[N] = {` 会漏 range designator 条目，别急着改源码。**
- **修法 = fixture 六条**（设备仓 **b3481fc**，fileset-23F84-symbols.json，全部 sha 钉+sym 自验过；matcher 漏配根因未深究——体形差异进 body-hash 都过不了，fixture 通道即定案手段）。
- **出件态**：kernel/payload 本轮重出中（b3481fc 之后）。刷写红线不变：只 recovery_a，boot_b d0200a6b / userdata 不动；回退 = recovery_a ← payload-kallocfix-crc-5f59a2a0.img。
- **下一墙预判**：0xae3fed8（17 站点，_devsw_unlock 之后=疑似 sysctl 邻域）为本族最大残余；stubbed 总账 4913/654。

**★会话二十三续判 = 第十二堵定谳 = AMFI → libkern OSMetaClass/OSArray 块（2026-10-31 早，屏摄 065621，fixture 079aee5）**：

- **转录验真**：`unmapped kernel function from 0xfffffff008f1b150 (x0 0xfffffff00b54bfd8 x1 0xfffffff00755bbdf)`，**crc=0xbfe4144d zlib 吻合**。第十一堵（kern_conf 族）已过 = 8d3a3ca4 件全链生效。
- **定谳**：caller = AMFI 类构造器，原目标 0xfffffe000ae411c8 = **delegating OSMetaClass 构造器**（7 参 this/className/super/size/zone/zoneName/flags；体形 = 基类 ctor 调用 + gIOKitDebug 位 6 检查 + zone_create(zoneName, size, flags|0x2000000=ZC_ZFREE_CLEARMEM, 0xffff, 0) 存 *[x4]；x5 奇地址 0x755bbee = zone 名串，谜底）。返址后 `pacda x16,x17(0xcda1)` 签 vtable 存 [x0] = AMFI 鉴权族。
- **整块 8 目标 17 站点六组件**：0xae40a90=`checkMetaCastWithName(OSSymbol,Base)`（registry walk+vt0x128+super 链 0x10）、0xae411c8=OSMetaClass C2（AMFI×2+IOSurface×4）、0xae42a64=`allocClassWithName(OSString)`、0xae42ab4=`allocClassWithName(char*)`（孪生 0x50B，转换器=hA1 `OSSymbol::withString`/hA2 `withCStringNoCopy`，均已配对；**定名钥匙 = donor withString 源码语义**：OSDynamicCast vs gOSSymbolMetaClass→retain、flags@+0xc bit0=kOSStringNoCopy→withCStringNoCopy(strnlen 0x3fffe)）、0xae42dbc=`getSuperClass`（ldr [x0,#0x10]）、0xae43d58=`OSObject::operator new`（kalloc_type+实例计数 ldadd；donor 序紧邻 reviewed 的 typed_operator_new=native 同序）、0xae43fa4/0xae43fec=OSArray `getNextIndexOfObject`/`getLastObject`（count@0x14、PAC 数组@0x20 disc 0xfeb9、逐字节同 native）。
- **fixture 8 条**（079aee5，sha+sym 自验全过），ledger 4895→4872（−17 精确）；kernel **cbc4c02d**，ESP 内核 793d148d，站点 bl→0xae89b3c=0x7a1db3c+0x346c000 端到端实证；payload **07d60bc1** 存 `out/m2381/payload-libkern-07d60bc1.img`。
- **下一墙预判**：0xae4d098（**91 站**、AMFI/IOGPU/HID/IOSurface/apfs 五组件，OSDictionary 区，体形 = vt[0x100] 转发壳：`ldr x2,[x2]` 后 `braa` 槽 0x100 disc 0xc323）——**需 vtable 槽位法（工作单 B.2 的 `pair_family_by_vtable_slot`），勿手工硬啃**；native 侧 ZTV 槽项是打包指针（0x8011…形，div 0xc323 与 donor braa 逐位同），解码器未破。次候选 0xae45eb8×36（OSArray 区）/0xae46bbc×31/0xae8dbb4×45(apfs)。
- **刷写红线不变**：只 recovery_a；boot_b d0200a6b / userdata 不动；回退 = recovery_a ← payload-kallocfix-crc-5f59a2a0.img。

**★会话二十三三判 = 第十三堵 = AMFI → `_os_log_create`（屏摄 074203，fixture dcffcc1）**：crc=0x88a9cb7 吻合；站点 0x8f0dfd0 = AMFI 调 `os_log_create("com.apple.MobileFileIntegrity", "library_validation_failure")`，NULL 时 csel 回退缓存句柄。donor 0xfffffe000ae8a26c（1408B，夹在 `_os_log_equ`/`__os_log_internal` 间，读 CNTVCT 类寄存器）→ native `_os_log_create`（区间唯一候选 0x7a66f78）。**12 站点五组件**。预测核对：命中"同一未配对尾区继续发"但首发是 12 站的 0xae8a26c 而非 91 站的 0xae4d098——尾巴按调用序而非站点数排序。ledger 4872→4860（−12）；kernel **c12f1711**，payload **9f4d17f3** 存 `out/m2381/payload-oslog-9f4d17f3.img`。下一墙候选不变：0xae4d098×91（vt0x100 转发壳，需 B.2 vtable 槽位通道）、0xae8dbb4×45(apfs)、0xae45eb8×36、0xae46bbc×31。刷写红线不变。

**★会话二十三四判 = 第十四堵 = AMFI → `_IORWLockAlloc`（屏摄 075103，fixture 93b2998）**：crc=0xc4e8e4a6 吻合；站点 0x8f18b9c（caller 无参设置，报告的 x0/x1=残留寄存器——判读注意）。donor 0xfffffe000aeadd18 = 20B 尾调壳（载锁组全局、x1=0、尾调 0xa7f4a44），与 native `_IORWLockAlloc` 形状逐位同构（尾目标都在 reviewed `_lck_rw_init` 正前方）；连带 `_lck_rw_alloc_init`（2 站）一起配。ledger 4860→4848（−10）。kernel **066a8100**，payload **9aec229f** 存 `out/m2381/payload-rwlock-9aec229f.img`。**B.2 开工判定成立**：连续四墙同区（AMFI/libkern 尾区），手钻 ~30 分钟/墙+一次刷机循环 vs 尾区 636 目标打不完；且 AMFI 尾调的下一条就是 0xae4d098（91 站 vt0x100 转发壳，手啃不可行）。B.2 第一刀 = 解 native ZTV 打包指针编码（0x8011…形，div 0xc323 与 donor braa 槽逐位同，donor kmod_info 打包先例 0x8010e4fe034c9f18 可套），第二刀 = `pair_family_by_vtable_slot` 入 matcher，验收 = 0xae4d098×91 及同族 vt 壳批量转正。刷写红线不变。

**★会话二十三五判（收会话）= 第十六堵 = AMFI → `_SecureDTLookupEntry`（屏摄 075955，fixture 5d17955）**：crc=0xc8730321 吻合；站点 0x8f0d480 = AMFI 调 `SecureDTLookupEntry(0, "/chosen", &out)` 判返值==1。donor 0xfffffe000afc0740（664B，IODT 块，夹在 reference-symbol `SecureDTFindNodeWithPropertyEqualToValueHelper` / reference-order `SecureDTInitEntryIterator` 间）→ native `_SecureDTLookupEntry`（0x7babbf4 区间唯一候选）。13 站四组件。**预测 0xae4d098 未中（第三次实证：尾巴按调用序）**；donor 内联子项 helper 0xafc09d8（9 站，native 无独立符号）与 0xafc1b20（PE 小壳身份不稳）均留 B.2 批次。ledger 4848→4837（−11）。kernel **2953e2c5**，payload **9fa4809c** 存 `out/m2381/payload-dtlookup-9fa4809c.img`。**手钻腿到此收档：本会话六墙 4913→4837（−76），全部 SHA 钉 fixture+端到端 bl 实证；此后新屏一律只记坐标归 B.2 批次，不再单独 fixture**（除非 unique-candidate 级别的顺手工件）。刷写红线不变。

**★会话二十三上游侦察(B.3 完成,2026-10-31)——重大叙事修正**：

- **sheng 设备仓公开 = UniXNU/UniXNU-sheng**(devices.toml 实录,小米平板 6S Pro);`feat/ipados27-gpu-parity` 分支 board.toml = **ipados-27.0.1-24A446**。已浅克隆到 `/tmp/sheng-repo/sheng-27`。
- **叙事修正(此前记忆高估)**:porting.toml arm64e-os 区 **status=planned**,limits 明写 "no fileset prelink output exists yet, so the kernel has not booted";26.5.2 证据 = "fastboot OKAY; **kernel and userspace execution remain unobserved**";24A446 显示适配器 "still requires a hardware boot result";GPU 渲染实证**全是 iOS 17 车道的**(记录自认 "historical rendering observations above remain iOS 17 evidence")。**上游在 arm64e/fileset 车道上未上电——我们在 fileset boot 路径与上游并行甚至领先**(我们有内核活体判读循环 4913→4837)。
- **无配对账本可白拿**:设备仓 kits 空(吃 feed)、无 fixture JSON;XnuPrelink 独立仓(76452c6e=我们的 pin,main 仅多 4 docs 提交)无新配对通道。**换 24A446 的"拿他账本"论据不成立 → 留 23F84 定案**。
- **可拿的干货**:①sheng-27 的 `zzzz-sheng-ipados27-bringup.patch`(2002 行)= **Darwin26 用户态 ABI shim 全图**(posix_spawnattr 0x100 布局逐字段:psa_sec_flags@0xac 32位/0xb0 字节/后续+8)——换 27.0.1 时的现成地图,同时反证 26.5.2 正确(我们无此层);②porting.toml 23F84 KC 侦察事实(298 entries/base 0xfffffe0007004000/585131 fixups fmt8/31 roots=1864 kernel targets/42027 branch sites——与我们账本口径互证;KC=xnu-12377.122.8,公共源 121.6 最近);③`ci/macos-workflow` 分支(未并 main;pin macos-26/Xcode 输入/token 防 rate-limit/lld 安装)= B 腿参考;④SM8550 生态玩家(fuxi-on-SM8550 Xyvern08371 等)。
- **B.2 施工修正**:matcher 现版已有 vtable 机制族(`anchor_vtables/pair_vtables/pair_vtable_users/pair_metaclass_vtables/pair_metaclass_getters`)——**开工先读,扩展而非新造**;native ZTV 打包指针解码仍是第一刀。

**★会话二十四收档 = B.2 第一通道落地 = matcher v11 `pair_family_by_vtable_slot`（2026-10-31，设备仓 d0b94c0 已推）**：

- **机制定谳（比开工单预设更简单也更广）**：开工单猜的"native ZTV 打包指针解码器"其实**早已在**——`kc_image.fixups()` 的 fmt8 auth-rebase 通路原生就解（native 0x8011c323_00a23738 = site 0xfffffff007217f40 = `__ZTV12OSDictionary`+0x110，target=base+低 32 位，div=(>>32)&0xffff，与开工单猜测逐位吻合）。真正缺的是 **donor 侧壳函数的配对通道**，而其钥匙 = **槽判别子签名**：
  - donor 转发壳（`braa/blraa` 尾调虚槽 + `autda 0xcda1`）体内 movk lsl#48 判别子集合是**跨构建不变量**（div = 槽身份哈希，非方法/类哈希；OSObject::release div 0x3a87 在 3159 个 donor vtable 同值同 native）。native 对应函数用同一判别子集合 → 签名相等即同源。
  - **首案破案**：0xae4d098（91 站）= donor `OSDictionary::setObject(const char*, const OSSharedPtr<OSMetaClassBase>&)`（**非虚 inline 重载**，`ldr x2,[x2]`=解 OSSharedPtr 拿裸指针再尾调虚槽 0xc323）→ native 0xa27120 同名符号；兄弟壳 0xae4d034=OSSharedPtr<OSSymbol> 重载→0xa270b4、0xae4d068=OSString 重载→0xa270ec。donor vtable 本体 +0x110 槽 → 真身 0xae4acf0（早已配对）。donor 壳比 native 少 xpacd/cmp/brk 校验块（Apple 构建省略）+ 寄存器分配差 = 体形/字节通道必败、判别子通道必中。
- **matcher v11 实现**（`pair_family_by_vtable_slot`，入 match_images 轮环 alias_thunks 后）：签名 = 函数内 movk lsl#48 立即数集（word 掩码：braa/blraa=0xD71F0800/0xD73F0800 & 0xFFFF0C00；movk64=(w>>23)==0x1E5 且 hw==3）；守卫 = 双侧 ≤0x80B、无 calls/refs、尺寸差 ≤0x40、**一 native 被多 donor 认领则全体弃权**。单测 5 例合成（含 xpacd 校验块对拍/伪阳性/冲突弃权），全套 kit 72 测试除 test_fileset 预先存在的导入错外全绿。
- **真件结果（23F84 带件 + reference）**：最终 map **69 条 vtable-slot 配对全语义自洽**（OSSharedPtr 全家、OSOrderedSet/OSSet、IORegistryEntry::propertyExists×6、IODTPlatformExpert NVRAM 族、IOEventSource/IODMAEventSource、vfpthunk、_PE_cpu_signal、OSUserMetaClass::Dispatch）；**kext-facing named 1685→1736**；**ledger 654 目标/4848 站 → 601/4298（−53 目标 −550 站，验收门③达成）**；double-bind 79→99（弃权守卫的安全噪音）。端到端：AMFI 站点 bl → 0xfffffff00ae93120 = 翻译后 native 重载，首指令 `ldr x2,[x2]` ✓。
- **口径教训（本轮差点误判回归）**：prelink.log 打两块（内层 reference-pass + 外层主 pass），**named 基线必须取外层**；ledger 目标数必须 distinct（per-comp 求和会把 654 记成 1086）。
- **验收门②部分达成**：0xae4d098×91 ✓ 转正（经内层 reference-order 级联）；**Dispatch×48 未中**——0xaea0bf8 是 donor vtable 槽目标（255 槽，div 0x9209），但 58 个 native TV 同 div 序列分属 4 类 Dispatch 目标（IOService×46/IOUserClient×10/IOUserServer/TestDextRematch），且 IOUserClient 自有槽（D1/D0/getMetaClass/init/free）donor 侧全未配 = **鸡生蛋**：需先配 IOUserClient::init（donor 0xaf4e544，254 TV 共享，ledger x47）再以伴随槽锁类。留 B.2 续单。
- **出件**：kernel **876a14c7**，payload **2b8b8258** 平铺 `out/m2381/payload-vtslot-2b8b8258.img`（96MiB；trust bundle ebeaf7fb + seed d04a0153 + chip-id 3645ad55d47e77c1）。刷写红线不变：只 recovery_a，boot_b d0200a6b / userdata 不动；回退 = recovery_a ← payload-kallocfix-crc-5f59a2a0.img。
- **下会话工作单**：①B.2 续单 = co-slot 群一致性通道（配 init→锁 IOUserClient 类→收编 Dispatch×48+init×47×2）②大函数残墙（0xaeb8eb0×190 copyProperty 候选/0xa95ca30×127/0xaf6236c×92 需放宽通道:伴随槽+尺寸序）③门④CI 重发对照（B 腿 kernel.json 口径）④判读轮:用户刷 2b8b8258 后预期 AMFI 不再报 setObject 壳。

**★会话二十四开工单 = B.2 vtable 槽位通道转正（2026-10-31 定稿；用户拍板开工）**：

- **第一刀 = native ZTV 打包指针解码器**：已知槽 0x100 打包值 0x8011c323_00a23738（div=0xc323 与 donor braa 槽逐位同），目标 VA 猜测 = 低 32 位 | 0x07000000（0x00a23738+0x07000000=0x07a23738 恰为 VA 低 32 位，kernel base 低位贡献）。验证法：解码 `__ZTV8OSObject`/`__ZTV12OSDictionary` 全部槽项，对拍已命名函数（taggedRetain/getMetaClass 等位置应全中）；donor kmod_info 打包先例（0x8010e4fe034c9f18 = IA/div 0xE4FE/addrDiv0，会话十九档）对照位域。解码器落 `temp/batch-fixtures/`（vtslots.py）。
- **第二刀 = donor vtable 定位**：donor ZTV 无符号 → 用 donor 类构造函数（C2/C1）里的 vptr adrp 反查（先例：AMFI 类构造器 0x8f18b3c `adrp 0x7fac5d0+0x10; pacda x17(0x4e9b); str [x0]` 形）；建立 donor 类名→vtable 映射（类名从已配对函数/`__ZTV`-adjacent cstring 取）。
- **第三刀 = matcher `pair_family_by_vtable_slot`**：未配对目标 f 若出现在 donor 类 C 的 vt 槽 i，且 native 类 C 的 vt 槽 i 已解析出命名函数 → 配对（与 alias_thunks 同班进 fileset_match.py）；首件验收族 = 0xaea0bf8（IOUserClient::Dispatch 48 站）与 0xae4d098（91 站 vt0x100 转发壳）。
- **验收门**：①解码器全槽命中已知名 ②0xae4d098×91+Dispatch×48 批量转正 ③ledger 站点断崖（4848 → 预期 <4600）④CI 重发对照（B 腿口径）。
- **并行**：判读轮若用户送新屏（rwlock 件预期 0xae4d098）按例行钻法收账（CRC 验真→batchid→若 vt 壳直接归 B.2 批次不单独 fixture）。刷写红线不变（只 recovery_a，boot_b d0200a6b / userdata 不动）。

**★会话二十三开工单 = 判读轮 + 预埋轮（2026-10-31 收尾定稿；用户问"还在打地鼠吗"→ 答复=形式不变性质已变：鼠池 4913 站已全分类有底、锤子定型 30 分钟/墙、sheng 实证池打得完）**：

- **A. 判读（用户刷 28fe6d56 后）**：
  0. 刷前现态核对：`current-slot=b`、`partition-size:recovery_a`、boot_b d0200a6b 未动、userdata 未动；设备现役=recovery_a 5f59a2a0。
  1. 刷 recovery_a ← `out/m2381/payload-batchfix-28fe6d56.img`（100,663,296 B），dd 读回对拍才算证据。
  2. 拍屏 → **CRC 行先核转录**（zlib crc32 对拍，030636 先例）→ 有墙走例行钻：`temp/batch-fixtures/stubid.py <site>` → `batchid.py <donor VA>` 夹逼 → 三重验证 → fixture/通道。
  3. 判读门：①原 AppleImage4 kalloc 墙消②推进过 AppleImage4 进下一组件③`PAC failure` 新形态=还是"strip+对方判别子重签"那刀④无墙推进=记 boot 史新坐标。
- **B. 预埋（不刷机并行做，把下一批墙提前填掉）**：
  1. TOP 真孤儿预命名（按站点序，语义分析已起头）：0xaeb8eb0（IORegistryEntry 区 190 站：registry 递归锁+虚调 0x118→release 0x20+解锁，源序无对应=疑 122.8 新增——比 donor 新 tag 源码可破）、0xaed7e2c（IOService 区 168）、0xa95ca30（锁函数 127：tpidr+0x1b8 计数器+casa+wfe，会话十七 lck 先例=配我们实现自洽）、0xaf6236c（IOReporter 92）、0xae4d098（OSDictionary 尾 91）。每个 = 源序夹逼+语义定性+fixture（走 31fa4e5 模板）。
  2. 同形家族消歧通道（525 站）：vtable 槽位法——donor 类 vtable 已配对时，槽位 i 的函数=对应 native vtable 槽位 i 的函数（0xaea0bf8=IOUserClient::Dispatch 48 站已锁，native 侧 15+ 同体 Dispatch 全靠这个消歧）；matcher 加 `pair_family_by_vtable_slot`，与 alias_thunks 同班。
  3. 若 sheng 姿势可借：比对 sheng-27 仓 porting.toml 里对同类桩的处理记录。
- **C. B 腿推进（战略减税）**：CI 内建 RELEASE 参照件（参数化上游 build-fileset-release-reference.py 到 out/m2381=原排队项①，workflow kernel 步前置）→ CI kernel.json 上传（补 site 口径精确数）→ payload CI 化（trusted 材料经 secrets，勿入 runner 盘）。
- **刷写红线不变**：只 recovery_a；boot_b d0200a6b / userdata 不动；回退=recovery_a ← payload-kallocfix-crc-5f59a2a0.img。基线：-j16；--trust-cache/--data-ramdisk/--unique-chip-id 必带；CRC 行为准。

**★会话二十五执行档 = 关键路径排空第一轮：bootpath 调用图清点 + SecureDTGetProperty fixture（2026-10-31 上午，工作单 §7 全单执行）**：

- **A 调用图清点（新工具 `temp/batch-fixtures/bootpath.py`，可复用）**：从 AMFI early_security_init（0x8f13c80）+ AMFI init（0x8f0d230 所在函数）双入口对 donor KC 做静态遍历（线性扫全函数体 = 条件分支双侧保守收集；跨组件 bl 合法跟进；donor function_starts 覆盖实证充足 kernel 21113 条）。坑一处：in_code 校验最初只查 caller 自组件段把跨组件 bl 全误杀（20 函数即止），改全区段后正常。**线性闭包过宽（55 墙/3501 函数）= 内核互联度本性，d≥2 静态分不出冷热**；改 BFS 带深度排序。
- **关键判定（干净）**：**d=0 墙 = 0xafc09d8 SecureDTGetProperty = 实机末屏精确吻合（walker 有效性实证）**；且全图**活链上唯一 d=1 墙-caller = 已配对的 `_SecureDTLookupEntry` → 它的唯一墙就是 0xafc09d8**。AMFI 局部链无其他墙；其余 54 面全为内核中转（d≥2），静态不可排序——历轮教训（尾巴按调用序）下不做投机 fixture。
- **B1 = 零 shim 定案**：donor 0xafc09d8（412B，DT 区边界读全局 [0x7da4b18]/[0x7da4b28]、entry+8 属性数组、0x24 prop 头、4 字节对齐步进、命中返 1/穷尽返 −1）与 native `_SecureDTGetProperty`（0x7bac004，24B 尾调壳，加载同类全局区对）ABI 逐参同构，`_SecureDTGetPropertyInternal`（0x7bac01c，424B）循环体逐指令同 donor。entry 来自已配对 native `_SecureDTLookupEntry`（我们的 DT 树）→ 我们的全局边界即正确值。fixture 一条（设备仓 **020aa2e**，functions 137→138，sha 钉 89831e0f…）。
- **投机甄别记录（不配）**：d=2 墙 0xa8150f0（117 站点、thread_dispatch/sched_update 括号、双 mul 哈希 0x7fb5d329728ea185/0x81dadef4bc2dd44d + 0x18 桶表）与 0xa866894（93 站点、zdestroy/get_thread_ro 括号、页元数据 zone 索引形态）——哈希常量在 native 全文 **零命中**（h2.mid 唯一命中=__global_eventq 数据假阳性）→ native 无对应体（122.8 新增/Apple-only），不硬配。d≥2 其余留观，等实机按调用序点名。
- **出件**：kernel **35761550**（ledger 4298→4289 站点精确 −9，AMFI 0xafc09d8 出账，map `reviewed` 138 条；`0xfffffe000afc09d8 → _SecureDTGetProperty how=reviewed`）→ payload **e5e9556f**（96MiB，100,663,296B；trust ebeaf7fb + seed d04a0153 + chip-id 3645ad55d47e77c1 + panic_screen；kernel_args 同前队）。平铺 `out/m2381/payload-dtgetprop-e5e9556f.img`（sha256 e5e9556f…768ad）。**ESP 副本端到端实证：AMFI 站点 0x8f0d4a8 bl → 0xfffffff00b018004 = native 0x7bac004 + 0x346c000 精确命中**。
- **判读门（下会话）**：①0xafc09d8 墙消失（AMFI early init 的 DT 属性读取走通）②推进到下一墙 = 按调用序点名（bootpath 榜单 + batchid 例行钻法接应；新屏一律 CRC 验真先行）③若无墙推进 = 阶段史记新坐标（root mount 段未至）。**回退 = recovery_a ← payload-kallocfix-crc-5f59a2a0.img**。

**★会话二十五判读轮 = 第十七堵定谳 = native mac_policy_register × donor MAC policy ops 槽 + 整表重签修法（2026-10-31 中午，屏摄 IMG_20261031_102447，payload e5e9556f 已刷）**：

- **本轮先行结论：0xafc09d8 SecureDTGetProperty 墙已过**（新屏是 PAC failure 不是 stub 报告；pages 0x125b8d 比上轮 0x125b8c +1）——AMFI early init 的 DT 读取段走通，bootpath 判定兑现。
- **转录验证不用 CRC 重建**：指令级锁死——ESP 内核 pc 0xb01cfe8 处恰好是 `blraaz x8`，且其前一条 `bl 0xa7bec18`（=native `_printf`）的返回址 = lr 低 32 位（0xb01cfd8）逐位合。
- **定谳**：pc = native `_mac_policy_register+0x3f0`（0x7bb0fe8），lr 同函数 +0x3e0；x0/x19 = 0x82d1950 ∈ **sandbox `__DATA_CONST`** = 被注册的 `mac_policy_conf`；[x19+0x20]=mpc_ops → 槽 +0x398 值 = 0x72ec01f00a52e104（高位 = donor 签名 tag，低位 ∈ sandbox `__TEXT_EXEC`）。源码闭环 = `mac_base.c:745` 的 `mpo_policy_init(mpc)`（printf "calling mpo_policy_init" 在上一行；槽位精确锚定：mac_policy_ops slot 115 @0x398 = mpo_policy_init、slot 116 @0x3a0 = mpo_policy_initbsd，与 disasm 两处调用逐一对应；mac_late=0 时第二处被 `ldr w8,[x24,#0xcd0]` 门掉）。**α 家族第六形态：native 消费者 × donor 数据区函数指针槽**。AMFI policy 早前注册没炸 = 其对应槽为 NULL（cbz 跳过）。
- **修法 = registration 入口整表重签**（`patches/xnu-12377/mac-policy-ops-donor-resign.patch`，主树 **d532452**，arm64e series 尾）：`mac_policy_register` 在 mpc_ops 检查后调 `unixnu_mac_policy_ops_resign`——走全表 **335 槽全函数指针**（编译器实证 sizeof=0xa78），每非 NULL 槽 `ptrauth_strip` + `ptrauth_sign_unauthenticated(IA,0)` 回写。正当性：ops 槽的全部消费者都是本内核 native mac_* 包装器（blraaz IA,0）；donor 侧无人消费自己的 ops 表；native 指针 strip+重签幂等。**可写性先证**：同一调用早前 `mpc->mpc_runtime_flags |= REGISTERED` 写 __DATA_CONST 页成功（否则炸在更早）。
- **出件**：kernel **16cb93f8** → payload **c13089a9**（96MiB，平铺 `out/m2381/payload-macresign-c13089a9.img`，sha256 c13089a9…6c6f8；trust/seed/chip-id/args 全队不变）。刷前门全绿：native 反汇编 = `ldr x8,[x19,#0x20]; cbz; mov x9,#0; 循环 cmp #0xa70/ldr/cbz/xpaci/paciza/str`；ESP 副本 emitted 0xb01cc48 同环在位。
- **判读门（下轮）**：①mpo_policy_init 墙消失（sandbox policy 注册走完；AMFI/其他 policy 同表同修）②mac_policy_register 段过 → 下一段新墙按调用序点名（stubid/bootpath 接应）③若出现 donor __DATA_CONST 写 fault（新类型 panic）= 可写性假设翻车，转"native 副本 + entries[].mpc 换指"备选方案。**回退 = recovery_a ← payload-dtgetprop-e5e9556f.img**。
- **杂项**：串口问题已核——Meizu 20 stock DTS 有 geni-debug-uart（qup_uart@a9c000，stdout-path），Linux 车道用它做 earlycon；iOS 车道 xnu 无 GENI UART 驱动，用户已拍板"之后看吧"。主树未推送提交：184e3ad（STATE s25）+ d532452（本 patch）；设备仓 020aa2e（fixture）。

**★会话二十五二轮判读 = 第十八堵定谳 = sandbox mpo_policy_init 的 lck_grp_attr_alloc_init + 诊断页字体放大（2026-10-31 中午续，屏摄 IMG_20261031_104947，payload c13089a9 已刷）**：

- **第十七堵（MAC policy ops 整表重签）实机生效**：新屏是 stub 报告不是 PAC failure——`mac_policy_register` 已过 mpo_policy_init 的 PAC 关，重签环工作。
- **第十八堵定谳**：`unmapped kernel function from 0xfffffff00a52e140 (x0 0x82d1950=x1 0x0)`。站点 = donor sandbox `mpo_policy_init`（0xa52e104，5416B）内第一条外部调用：`bl 0xa7f2e40`。callee 48B = kalloc_type(lck_grp_attr zone) + 拷 `lck_grp_attr_default` u32 = **`lck_grp_attr_alloc_init`**（调用流：attr_alloc_init → attr_setstat → lck_grp_alloc_init(name,attr) → lck_mtx_init/lck_rw_init ×N，后四者全已配对）。native `_lck_grp_attr_alloc_init`（0x733b258，48B）与 donor **逐指令同构**（仅全局地址差）→ fixture 一条关死（设备仓 **4cea0fe**，139 条，ledger 4289→4285 精确 −4）。
- **诊断页字体放大（用户拍板项）**：小字 = `package/BootProgress.c` 画的 boot 诊断页，上游 scale 规则 `w>=2160&&h>=1400?3:1` 把 Meizu 20（1080×2400）钉在 scale=1（6×14 px 字符）。改 619 行为三段：`>=2160&&>=1400→3 / (>=1000&&>=1400)||(>=1400&&>=1000)→2 / 否则 1`——本机面板落 scale=2（字符 12×28 px）。安全性：`bp_line_bg` 像素循环按 `width/(6*scale)` 钳制（越界不可能）；右侧 ext 小面板（PC 采样/Kernel console 栏）在 120 列×12=1448>1080 时按自身宽度守卫自动跳过（内容原为噪声）。DWARF 行表实证新 623 行编入件。
- **验证工具坑（本轮三次白扫）**：BootProgress.o 的段名是**空字符串**（非 `__TEXT`），按段名过滤的扫描全扫了空气；capstone 线性 disasm 遇填充即停，须跳 4 字节健壮扫（bootpath 同款）；cmp 立即数扫描的 Rd 位写死坑第三次复发（§4.4）。
- **出件**：kernel **556a4efc** → payload **dd0da85b**（96MiB，平铺 `out/m2381/payload-grpattr-font2-dd0da85b.img`，sha256 dd0da85b…；trust/seed/chip-id/args 全队不变）。刷前门：sandbox 站点 0xa52e13c `bl 0xfffffff00a7a7258` = native 0x733b258+0x346c000 精确命中（ESP 副本）；map `reviewed` 139。
- **判读门（下轮）**：①本墙消失、sandbox lock-group 初始化段走完（同函数后续 attr_setstat/alloc_init/lck_*_init 全已配对，理论上可连走）②新墙按调用序点名 ③**屏幕上诊断页字体应变大一倍**（scale 2）——刷机后顺带确认④mac policy 注册段过 = AMFI/sandbox 后续可能撞 sandbox 主体执行路径的新墙。**回退 = recovery_a ← payload-macresign-c13089a9.img**。

**★会话二十七执行档 = 批量 fixture 首刷出件（2026-10-31 下午，工作单 §7 A.1–A.3 全毕，等拍屏）**：

- **A.1 推送积压全毕**：主树 6 笔（184e3ad…da2b8db）推 m2381-ios27 并
  force-with-lease 推平远端 m2381（c4367fc 异源重复 sha 线 = 会话二十二
  TLS 断连经 GitHub API 推送的历史遗留，逐笔核有本地孪生后平掉）；
  设备仓 3 笔（020aa2e/4cea0fe/124cb0b）推 main。
- **A.2 定名 9 笔（设备仓 832db09 + 修正笔，fixture 145→154）**：
  0xaf4babc=IOUserClient::getTargetAndMethodForIndex / 0xaf4bbd8=clientDied /
  0xaf4bc40·0xaf4bd5c=initWithTask 4参/5参 / 0xaf510ac=copyClientEntitlement /
  0xaf5b9b4=clientClose / 0xa95ca30·0xa95ca6c=IOSimpleLockLock/TryLock（会话
  二十六 B.1 半钻单关账） / 0xa866894=_get_bsdtask_info（38 站红利，会话二十
  五"native 无对应体"误判翻案）。**新定名杠杆 = vtable 槽位对位**（工具
  `tools/xnu27/vtslots.py`+`vtpair.py`）：native `__ZTV12IOUserClient` 用
  map 已配对四锚点（registerNotificationPort/exportObjectToClient/
  getAsyncTargetAndMethodForIndex/getTargetAndTrapForIndex）定槽基，线性
  映射 donor kext vtable 槽 → native 槽符号直接读名；语义交叉验证 =
  getTargetAndMethodForIndex 调的槽恰是 getExternalMethodForIndex、
  clientDied 调的槽恰是 clientClose。**坑两枚**：①native 镜像基 =
  0xfffffff007004000（非 …07000000，会话二十四工作单猜测式未实测者）；
  ②donor function_starts 有合并缺项（0xa95ca30/0xa866894 是 bti c 真入口
  嵌在合并大函数内）+ make_fixture end-scan 漏 bti c（0xd503245f）致
  吞并后续函数（0xaf4bd5c 虚胖 1276B）→ 工具已修；**fixture size/sha
  权威口径 = anchor_reviewed 的 FS 表区间**（init() 300→284、
  get_bsdtask_info 120→140 两笔按此修正，全表 154 条过同口径复验）。
  0xa866894 定名链 = donor copyClientEntitlement 调用点位置 +
  native _get_bsdtask_info（task_get_proc_raw+proc_require+同款
  tst 0x800000/csel 尾，字段漂移 0x3f0↔0x5a8=122.8/121.6）。
- **A.3 出件**：kernel **`56e7a7a9`** → payload **`7c3c4b51`**（96MiB
  100,663,296B，平铺 `out/m2381/payload-ioclient-lockfix-7c3c4b51.img`，
  sha256 7c3c4b51…；trust ebeaf7fb + seed d04a0153 + chip-id
  3645ad55d47e77c1 + panic_screen + args 全队不变）。**ledger 4298→3575
  站 / 601→587 目标（−723 站 = 单会话历史最大降幅）**；map 九笔全
  how=reviewed；端到端 = ESP 副本内 native _IOSimpleLockLock 落位
  0xa9b1e00（build+0x346c000）精确命中 + 109 个闭包站点 bl 已改指 native
  （对照：已过墙的 wall-18 目标同口径 2/13 站改指 = 非闭包站点不改指是
  既定行为，非回归）。kext-facing named 1736→1751。
- **后台侦察**：账本新 top10（0xaf6236c×92/0xaf63734×65/0xaf65b14×53/
  0xaf6898c×49/0xae8dbb4×45/0xaf0a514×44/0xaf06d34×41/0xac9bfd8×39/
  0xaead9cc×38/0xad15560×38）已派扇出子代理走 vtable 对位+源序夹逼，
  产物格式 = VA/候选名/三证据/置信度，回报后按纪律定谳入 spec。
- **刷写卡**：设备现态 = iOS 链（boot_b d0200a6b / recovery_a dd0da85b /
  userdata fd43ee70）→ **只刷 recovery_a ← payload-ioclient-lockfix-
  7c3c4b51.img**（刷前 sha 门 + dd 读回对拍）。判读门：①墙 19
  （0xa52e2b0 PE_parse_boot_arg_str）消失 + sandbox mpo_policy_init 走完
  ②IOUserClient/IOSimpleLock 族不再首发 ③新墙按调用序点名（CRC 验真
  先行）④诊断页字体 scale2 维持确认。
- **回退** = recovery_a ← `payload-grpattr-font2-dd0da85b.img`。

**★会话二十七续档 = 侦察批次定谳落仓 + 二次出件（2026-10-31 傍晚，payload 换防）**：

- **后台侦察回报 10/10 全定名（全高置信度）**，主会话抽查两笔过（IOLockWakeup
  体形逐字合 + map 源序锚点 `_IOLockSleep`/`_IOLockSleepDeadline` 三连；
  kauth 六连 getter 链尾锚 `_kauth_cred_getsvgid` 吻合）后 9 笔落仓
  （设备仓 fixture 154→163）：0xaf6236c=IOReporter::addChannel /
  0xaf63734=IOSimpleReporter::setValue(y,x) / 0xaf65b14=IOStateReporter::setStateID /
  0xaf6898c=IOReportLegend::addReporterLegend(rep,grp,sub) / 0xaf0a514=IOCommandGate::runActionBlock /
  0xaf06d34=IOWorkLoop::workLoop() / 0xac9bfd8=kauth_cred_getuid /
  0xaead9cc=IOLockWakeup / 0xad15560=nanotime。侦察法 = KDK(12377.121.10.1)
  masked-word shape 唯一命中 + native vtable div 常数逐一对上 + 源序 bracket。
  **0xae8dbb4=_os_pcpu_ref_release 跳过**（native 无此导出符号，仅 slow 变体；
  12377 新增 out-of-line，留 shim 通道）。FS 口径复验修 1 笔
  （runActionBlock 真边界 80B，trampoline 0xaf0a564 = 独立 FS 条目
  = __ZL26IOCommandGateActionToBlock 白拿命名），全表 163 条 OK。
- **二次出件**：kernel **`1a896fcd`** → payload **`cda26378`**（96MiB，
  平铺 `out/m2381/payload-batch2-cda26378.img`；trust/seed/chip-id/args/
  panic_screen 全队不变）。**ledger 3575→3108 站 / 587→577 目标**
  （两批合计 −1190，账本从 4298 起步砍 27.7%）。5 笔抽核 map 全
  how=reviewed。前件 7c3c4b51 已被换防删除。
- **刷写卡更新**：**只刷 recovery_a ← `payload-batch2-cda26378.img`**
  （sha256 cda26378…，100,663,296B，刷前 sha 门 + dd 读回对拍），boot_b
  d0200a6b / userdata fd43ee70 不动。判读门不变：①墙 19（0xa52e2b0）消失 +
  sandbox mpo_policy_init 走完 ②IOUserClient/IOSimpleLock/IOReporter 族
  不再首发 ③新墙按调用序点名（CRC 验真先行）④诊断页 scale2 维持。
  **回退 = recovery_a ← payload-grpattr-font2-dd0da85b.img**。

## 7a. 工作单 = 会话二十八（2026-10-31 收队定稿）：cda26378 报错判读轮

- **★墙 20 定谳（2026-10-31 深夜，c5d05bfd 首发屏摄 221752）= Sandbox →
  `OSUnserializeXML` 三参分发壳**：CRC 0xf1702373 吻合；站点
  `0xa52e5f8 bl 0xfffffe000ae84130`（与墙 19 差 0x248 = 同一 Sandbox
  启动函数按调用序推进，墙 19 生效实证）。定名 =
  OSUnserializeXMLSharedImplementation.h:3054 KERNEL 分支
  （len<4 拒 → strcmp "\323\0\0"/0xd4 → OSUnserializeBinary / NUL 尾查
  → 两参 OSUnserializeXML）；漏配根因 = 我们 clang 合并空指针+len 检查
  成 cmp+ccmp。**22 站/6 kext 大户**（IOSurface 8/IOHIDFamily 5/IOAV
  3/AMFI 2/CoreAnalytics 2/sandbox 2）。fixture 第 165 笔（设备仓
  `47131a4`），账本 3104→**3082**（−22 精确）；端到端 = 站点 bl →
  native `0xaec8944` 体同。出件 kernel **132b605a** / payload
  **91ddb680**（`out/m2381/payload-usx-91ddb680.img`）待刷。
- **★判读完成（2026-10-31 晚）= 墙 19 定谳 = Sandbox →
  `proc_get_syscall_filter_mask_size`**：cda26378 首发墙。CRC 行验真
  （屏摄掉 1 个 f，真址 `0xfffffff00a52e3b4`，crc=0x8e706923 zlib 吻合）；
  站点 `0xa52e3b0 bl 0xfffffe000ace12a0`，账本 owner =
  com.apple.security.sandbox（4 站）。定名 = kern_proc.c:5285 flavor 函数
  （0→nsysent 558 / 1→mach_trap_count 128 / 2→mach_kobj_count）；**漏配
  根因 = Apple 折叠 const 成立即数、我们 clang 出 load，体形全散**（native
  存在且值全同 = 纯编译差）。fixture 第 164 笔（设备仓 `02d75a4`），
  账本 3108→**3104**（−4 精确）；端到端实证 = 新件站点 bl → native 体
  `0xfffffff00ad02078` 逐指令同。出件 kernel **a4f1bd89** / payload
  **c5d05bfd**（`out/m2381/payload-sbfm-c5d05bfd.img`）。
- **上游同步同轮完成**：feat/ipados27-gpu-parity 8fe2aaa→359d77c rebase
  （295 提交，仅 AdrenoMetal gitlink 冲突；老坑二现已修 924877f），
  check-m2381 4 ok + kit 7 测试绿；UPSTREAM.md §1.1 已记。**主树待推
  （force-with-lease）**；设备仓 02d75a4 待推。
- **判读工具坑**：本机 `./unixnu kernel` 漏喂 `--ipsw` 时回落 board.toml
  远程 URL 分段拉 6.4GB（3KB/s 涓流 16 分钟假死）——**Linux 本机构建必带
  `--ipsw research/ipsw/iPad17,1,iPad17,2,iPad17,3,iPad17,4_26.5.2_23F84_
  Restore.ipsw`**（本地件全在 out/ipsw/23F84/，编译对象复用）。
- **待用户**：刷 c5d05bfd（只 recovery_a，卡见下）。预期 = 墙 19 消失、
  按调用序进下一墙；cda26378 的 18 笔生效判定 = 预测墙 19
  （0xa52e2b0 PE_parse_boot_arg_str）未再首发。
- **现态**：设备 = iOS 链，recovery_a = **cda26378**（待换 c5d05bfd），
  boot_b `d0200a6b` / userdata `fd43ee70` 不动。
- **A. 判读（用户交报错屏后）**：
  0. 拍屏整段（panic 屏或其它报错形态都要）→ **CRC 行先验真**（zlib
     crc32 对拍，逐墙先例）→ 有墙走例行钻法：`kc.py wall <屏上地址>`
     → batchid/vtslots 夹逼 → 三重验证 → fixture（走会话二十七模板：
     size/sha 以 anchor_reviewed 的 FS 表区间为权威口径）。
  1. 判读分支：①墙 19（0xa52e2b0 PE_parse_boot_arg_str）消失 =
     18 笔生效，报错为新墙（按调用序点名，账本新 top10 侦察单接应）
     ②报错形态不是 stub/PAC 墙（如 NO WINDOW / DarwinBoot 拒载 / 复位
     循环）→ 按形态归因（固件面 vs 内核面），勿套墙钻法
     ③同区连发卡 → 止损线照旧（停修归因回报拍板）。
  2. 若新墙在账本 top10 内：账本余量 3108 站/577 目标，`kc.py ledger`
     取榜 → vtable 对位/KDK shape/源序夹逼三杠杆快线定名。
- **B. 回退项（如需立刻回到上一进度）**：recovery_a ←
  `out/m2381/payload-grpattr-font2-dd0da85b.img`（上轮现役，墙 18 后状态）。
- **C. 排队（不挡判读）**：①`_os_pcpu_ref_release` 无 native 导出 =
  shim 通道待做 ②账本余量侦察批（会话二十七侦察法已定型：KDK
  masked-word shape + vtable div + 源序 bracket）③kernel 1a896fcd 的
  未推送件核对（STATE e5e6ee9 已推，无积压）。
- **红线不变**：只 recovery_a；boot_b d0200a6b / userdata 不动；
  --trust-cache/--data-ramdisk/--unique-chip-id 必带；-j16；CRC 行为准。

## 7b. 会话二十九执行档 = 墙 21 定谳（2026-10-31 深夜，91ddb680 首发屏摄 222856）

- **★墙 21 定谳 = Sandbox → `_strncasecmp`**：CRC `0xfc18414e` zlib
  一次对上（本轮转录零掉字）；站点 `0xa5472e8 bl 0xfffffe000a91f920`
  （文件态验证）。callee 定名 = **strncasecmp**：三参体（`cbz x2` →
  n−1 循环 → tolower csel 对 → `ccmp x9` 尾）与二参 `_strcasecmp`
  （0xa91f8dc）是两个函数；**Apple map 在 _strcasecmp 与 _strchr
  (0xa91fa44) 之间无名** → map 名字通道必然漏配（账本 key
  `0xfffffff00a91f920`，34 站/4 kext：AppleARMPlatform 9、apfs 2、
  IOMobileGraphicsFamily 20、sandbox 3）。**native `_strncasecmp`
  存在**（全局 N_SECT `0xfffffff007506ed4`），21 条体逐条同（donor 仅
  多头部 `bti c`）→ 直配。崩溃时实参 = x0
  `ENTITLEMENT:com.apple.security.exception.iokit-user-client-class` /
  x1 `ENTITLEMENT:`（sandbox 规则编译的 entitlement 前缀判别；墙 19/20
  同一条 sandbox 启动链继续推进 = cda26378 两笔生效旁证）。
  fixture 第 166 笔（设备仓 `f355941`），账本 3082→**3048**（−34 精确，
  kernel.json 复核目标全消）。
- **工具坑（已修）**：`tools/xnu27/kc.py` `_symtab` 按 value
  `setdefault`，同址空名局部符号先到会顶掉真名 → `sym` 假阴性
  （差点误判 native 无此函数走 shim 通道）；已改为非空名优先。
- **出件**：kernel `18affaa2` / payload `936cd505`
  （`out/m2381/payload-strncasecmp-936cd505.img`）。构建三坑全避开：
  `-D` 子命令后 + `--ipsw` 本地件 + `--trust-cache/--data-ramdisk/
  --unique-chip-id 3645ad55d47e77c1` 显式传，`-j16`。
- **待用户**：刷 `936cd505`（只 recovery_a）。预期 = 墙 21 消失、
  sandbox 启动链继续推进出下一堵（或过 AMFI early_security_init 进
  下一段）；若首发不是 sandbox 链上的墙，按调用序点名。
- **回退**：recovery_a ← `out/m2381/payload-usx-91ddb680.img`（墙 20 后
  状态）；更早回退见 §7a B。
- **红线不变**：只 recovery_a；boot_b d0200a6b / userdata fd43ee70 不动；
  CRC 行为准。

## 7c. 会话三十执行档 = 墙 22 定谳（2026-10-31 深夜二轮，936cd505 首发屏摄 230709）

- **★墙 22 定谳 = Sandbox → `IOService::addMatchingNotification`**：
  CRC `0x4775235e` zlib 一次对上；站点 `0xa52ed24 bl
  0xfffffe000aedcf6c`（文件态验证；与墙 19 站 0xa52e3b0 / 墙 20 站
  0xa52e5f8 同一 sandbox 启动函数按调用序推进）。实参 x0/x1 = 两个堆
  指针（OSSymbol*/OSDictionary* = type+matching，非内核镜像区）。
  callee 定名 = **addMatchingNotification handler 形**（IOService.cpp:6616，
  OSSharedPtr 返 = sret x6）：体逐元素吻合源码 6629-6658 =
  `installNotification(...,OSIterator** existing)`（x6=&out 出参）→
  `getNextObject` 迭代（vtable 槽 0x88 disc 0x1ac8；`[next+0x48]` bit0 =
  kIOServiceInactiveState 位测）→ `invokeNotifier(next, notify)`
  0xaec397c → existing release（槽 0x28 disc 0x3a87）→
  LOCKWRITENOTIFY casa 内联 → notify release → `csel
  gIOServiceNullNotifier`（全局 [0xb4191e0]）。**Apple map 在
  waitQuiet(mach_timespec) 0xaedc95c 与 waitForMatchingService
  0xaedd148 之间无名**（最近名噪声 = waitQuiet+0x610）→ map 名字通道
  必漏；账本 key `0xfffffff00aedcf6c`，22 站/7 kext（ARMPlatform 4、
  AMFI 2、apfs 2、IOAV 3、IOHIDFamily 3、IOMobileGraphicsFamily 3、
  sandbox 5）。**native 具名全局在册**
  `__ZN9IOService23addMatchingNotificationE…` `0xfffffff007abae54`，
  70 条前奏-循环结构逐条同；系统差 = 我方 clang 每个 `autda` 后加
  `xpacd/cmp/brk` 空签名校验块（会话二十四已知 codegen 家族）→ 直配。
  尾部 4 参包装 0xaedd120（静态 handler 0xaece8b0）= 独立 donor 函数，
  end-scan 于 bti c 处正确排除。fixture 第 167 笔（设备仓 `47577c2`），
  账本 3048→**3024 站**/840 目标（比 −22 预期多降 2 站/7 目标 =
  配对联级红利，方向全为降）。
- **出件**：kernel `c709e182` / payload `7fac1e57`
  （`out/m2381/payload-addmatchnotif-7fac1e57.img`）。
- **待用户**：刷 `7fac1e57`（只 recovery_a）。预期 = 墙 22 消失、
  sandbox 启动函数继续推进出下一堵；判读 = CRC 行先验 → kc.py wall。
- **回退**：recovery_a ← `out/m2381/payload-strncasecmp-936cd505.img`。
- **红线不变**：只 recovery_a；boot_b d0200a6b / userdata fd43ee70 不动；
  CRC 行为准。

## 7d. 会话三十一执行档 = 墙 23 定谳 + 关键路径总量清点（2026-10-31 深夜三轮，7fac1e57 首发屏摄 234200）

- **★墙 23 定谳 = Sandbox → `IOService::getMatchingServices`**：CRC
  `0xcfcb40bc` zlib 一次对上；站点 `0xa52ed60 bl 0xfffffe000aedcb48`
  （与墙 22 站差 0x3c，同一 sandbox 启动函数推进）。实参 x0 = 堆
  OSDictionary*（matching），x1 = 调用方残留。定名 = 体逐元素吻合
  IOService.cpp:6398：LOCKWRITENOTIFY 内联（casa，锁 = [0xb419198]）→
  `mov w1,#4`（kIOServiceMatchedState）`mov w2,#0`（options 默认）→
  bl copyExistingServices 0xaece088 → UNLOCK → return。Apple map 在
  waitQuiet(mach_timespec) 与 addMatchingNotification 之间无名（噪声
  名 waitQuiet+0x1ec）；账本 13 站/5 kext（ARMPlatform 4、IOGPUFamily 2、
  IOHIDFamily 2、IOStorageFamily 2、sandbox 3）。native 具名全局
  `__ZN9IOService19getMatchingServicesEP12OSDictionary`
  `0xfffffff007aba788` 同语义（外部锁调用 vs donor 内联 casa）。
  fixture 第 168 笔（设备仓新提交），账本 3024→**3011 站**/835 目标。
- **★关键路径总量清点（bootpath.py BFS 复跑，现势账本）**：AMFI
  early_security_init + init 双入口可达 3519 函数；**路径上还压
  52 个 stubbed 目标 ≈ 536 站**（按 BFS 深度排序榜已出，d=2-6 浅层
  ~11 目标先把 AMFI/sandbox 段扫平，中段大量 apfs，root mount 前
  大户 = apfs 路径站 ~150）；路径外背景 520 目标/2488 站（不挡 boot，
  按拍板不追）。52 里有已甄别硬骨头（0xa8150f0 双 mul 哈希无 native
  体 = shim 类）。**注意账本站点数≠墙数**：只有执行到的才炸，52 是
  上界，按 d 从浅到深批量配平。
- **出件**：kernel `206fa427` / payload `b5f102ac`
  （`out/m2381/payload-getmatchsvc-b5f102ac.img`）。
- **下会话工作单（批量轮提案，待拍板）**：按 bootpath 榜 d 序扇出
  侦察（会话二十七模板：KDK masked-word shape + vtable div + 源序
  bracket，子代理并行定名 + 主会话抽查），分级处置 = 直配类批量
  fixture / 无 native 体类 shim 通道 / PAC 数据面类单独钻；每批
  10-20 笔一刷验收整段。
- **回退**：recovery_a ← `out/m2381/payload-addmatchnotif-7fac1e57.img`。
- **红线不变**：只 recovery_a；boot_b d0200a6b / userdata fd43ee70 不动；
  CRC 行为准。

## 7e. 会话三十二执行档 = 墙 24 定谳 + 批量轮第一发（2026-11-01 凌晨，b5f102ac 首发屏摄 235450）

- **★墙 24 定谳 = Sandbox → `IOUserClient::registerFilterCallbacks`**：
  CRC `0xbad103f6` zlib 一次对上；站点 `0xa52ee20 bl 0xfffffe000af5afc8`
  （墙 23 +0x2c0，同一 sandbox 启动函数）。体 13 条逐元素吻合
  IOUserClient.cpp:6856：`cmp x1,#0x18`（sizeof io_filter_callbacks）→
  kIOReturnBadArgument 0xe00002c2；`casal` 认领 gIOUCFilterCallbacks
  （0x7d71b88 文件态 NULL）→ 0 / kIOReturnBusy 0xe00002d5。x0 = sandbox
  的 callbacks 结构指针（**sandbox 在注册 IOUserClient 方法过滤钩子**）。
  Apple map 缺口（is_io_device_tree_entry_exists_with_name 与
  sysctl_debug_iokit 之间）。账本 2 站/1 kext。native 具名
  `0xfffffff007b39df4` 同常量（CAS 走 OSCompareAndSwapPtr 调用 vs donor
  内联 casal）。fixture 169（设备仓提交）。
- **★批量轮第一发 = 浅层带 12 目标扇出侦察全灭**：三路子代理并行
  （会话二十七模板），**12/12 定名、零 no-native-body**；主会话抽查
  2 笔（thread_tid 叶函数字段偏移 0x4e8↔0x5e8 布局差安全 / vsnprintf
  逐指令同）后入仓：`IOTaskHasEntitlement`(IOKitBSDInit.cpp:1671) /
  `smr_call`(smr.c:1449) / `thread_call_enter1`(thread_call.c:1170) /
  `socd_client_trace` / `proc_signal`(kern_proc.c:1853) / `vsnprintf` /
  `clock_get_calendar_microtime`(clock.c:431，scale_delta 内联差) /
  `kernel_thread_start` / `vfs_iterate`(vfs_subr.c:7477) /
  `vnode_open`(7807) / `VNOP_IOCTL`(kpi_vfs.c:4109) / `thread_tid`。
  fixture 169→**181**（设备仓提交），账本 3011→**2855 站**/800 目标
  （−156 站，含级联）。
- **裁决（用户问单拆还是批量）= 转批量**：单墙判读链已证明 ~10 分钟/
  墙（易类），但地鼠本质是串行暴露；批量轮 = bootpath 榜 d 序扇出
  侦察 + 每批 10-20 笔一刷验收。本件 = 第一批（12+1 笔），预期
  sandbox 启动函数段基本扫平，首发墙应换组件段（apfs 或 AMFI 更深处）。
- **出件**：kernel `98b18a2b` / payload `f0713efc`
  （`out/m2381/payload-batchrecon1-f0713efc.img`）。
- **下会话工作单 = 批量轮第二批**：bootpath 榜复跑（2855 现势）取
  d 序下一段 ~12-16 目标扇出；0xa8150f0（双 mul 哈希，已知无 native
  体）留 shim 通道专项。
- **回退**：recovery_a ← `out/m2381/payload-getmatchsvc-b5f102ac.img`。
- **红线不变**：只 recovery_a；boot_b d0200a6b / userdata fd43ee70 不动；
  CRC 行为准。

## 7f. 会话三十三执行档 = 墙 25 定谳 + shim 通道开张（2026-11-01 凌晨二轮，f0713efc 首发屏摄 003850）

- **★墙 25 定谳 = Sandbox → `read_erandom`（shim 类第一例）**：CRC
  `0x18977c8a` zlib 一次对上；站点 `0xa52ee40 bl 0xfffffe000a938070`
  （墙 24 +0x20，同一 sandbox 启动函数；x0 = sandbox buffer，x1 = 0x40
  = 取 64 字节熵）。callee = **static `read_erandom`**
  （prng_random.c:313）：map 无名（夹在 _read_erandom_generate
  0xa937fb8 与 _read_random 0xa938124 的源码序位），体头读
  `prng_ready` [0x7ca28c0] 分流 DRBG/PRNG（prng_ctx 0x7ca28b0 签名
  0xedeb = cckprng generate 判别子，会话二十一家族）。**NO-NATIVE
  类**：我方 clang 把 static 内联进所有调用者（symtab 只有
  _read_erandom_generate）；`_read_random` 多一次 `prng_funcs.refresh`
  直配有语义漂移风险 → 走 shim。
- **★shim 通道开张**：主树新补丁
  `patches/xnu-12377/fileset-read-erandom-shim.patch` =
  prng_random.c 加 `unixnu_fileset_read_erandom`（used/noinline，包
  read_erandom 同语义）+ `config/Mach.exports` 保活（内核链接
  `-exported_symbols_list all-kpi.exp`）；series = arm64e
  prng-getentropy 之后（补丁对 post-series 树 git apply --check 干净）。
  native 落位 `0xfffffff00751d7bc`，体与 donor read_erandom 同构实证。
  fixture 182（设备仓提交），账本 2855→**2852 站**/799 目标。
- **出件**：kernel `6d8e4b26` / payload `73be0a25`
  （`out/m2381/payload-erandom-shim-73be0a25.img`）。
- **下会话工作单 = 批量轮第二批**：bootpath 榜复跑（2852 现势）取
  d 序下一段 ~12-16 目标扇出侦察（会话三十二模板）；0xa8150f0 与
  本轮同类 NO-NATIVE 的挂 shim 清单。
- **回退**：recovery_a ← `out/m2381/payload-batchrecon1-f0713efc.img`。
- **红线不变**：只 recovery_a；boot_b d0200a6b / userdata fd43ee70 不动；
  CRC 行为准。

## 7g. 会话三十四执行档 = 墙 26 定谳（2026-11-01 凌晨三轮，73be0a25 首发屏摄 021900）

- **★墙 26 定谳 = Sandbox → `proc_set_syscall_filter_callbacks`**：
  CRC `0xd630bc81` zlib 一次对上；站点 `0xa52f18c bl
  0xfffffe000ace1514`（墙 25 +0x34c，同一 sandbox 启动函数；x0 = cbs
  结构，x1 = 0x80 = sizeof）。体逐元素吻合 kern_proc.c:5354：
  `cbs->version != 1`（SYSCALL_FILTER_CALLBACK_VERSION）→ 返 0x16 =
  EINVAL；`ldp x9,x8,[x0,#0x10] + orr + cbnz` =
  mach/kobj_filter_cbfunc 非空判别 → 安装路径（zalloc_ro_mut +
  panic_proc_ro_proc_backref_mismatch = mac_task_register_filter_callbacks
  RO 机制）。**语义 = sandbox 注册 syscall filter 回调进 MACF**（墙 19
  取 mask 尺寸 → 墙 26 注册回调，同一集成链）。map 无名（
  _proc_get_syscall_filter_mask 与 _proc_ro_alloc 之间，噪声名
  +0x240）。账本 2 站/1 kext。native 具名 `0xfffffff00789632c`
  （wall-19 配对簇正后方）。fixture 183（设备仓提交），账本
  2852→**2850 站**/798 目标。
- **出件**：kernel `c79aa564` / payload `71e5d28b`
  （`out/m2381/payload-sfiltercb-71e5d28b.img`）。
- **下会话工作单 = 批量轮第二批**：bootpath 榜复跑（2850 现势）取
  d 序下一段扇出；NO-NATIVE 挂 shim 清单。
- **回退**：recovery_a ← `out/m2381/payload-erandom-shim-73be0a25.img`。
- **红线不变**：只 recovery_a；boot_b d0200a6b / userdata fd43ee70 不动；
  CRC 行为准。

## 7h. 会话三十五执行档 = 批量轮第二发 + 判读/刷机解耦（2026-11-01 凌晨四轮，71e5d28b 待刷）

- **★批量轮第二发 = 12 目标再全灭**（用户尚未刷 71e5d28b，侦察完全
  离线先行）：复跑 bootpath 榜（2850 现势）= 路径剩 42 目标/417 站
  上界；取 d 序下一段 12 个扇出三路侦察，**12/12 定名、零
  NO-NATIVE**：`kauth_cred_unref`(kern_credential.c:4399) /
  `proc_uniqueid`(kern_proc.c:2142) / `vnode_update_identity`(vfs_cache.c:1139)
  / `vnode_recycle`(vfs_subr.c:3277) / `proc_ucred_unsafe`(KDK 26.5.2
  形状锚定;Apple KPI 开源无) / `os_reason_create`(KDK unique-id scheme)
  / `IORegistryIterator::iterateOver` / `mach_continuous_time`(12 字节
  mrs 叶,config 分歧路径) / `copyinstr`(copyio.c:551) /
  `vnode_getparent`(vfs_cache.c:1006,worker 内联差) / `vnode_authorize`
  (vfs_subr.c:8997,KDK 形状唯一命中) / `vnode_getfromfd`(vfs_syscalls.c:4248)。
  主会话抽查 2 笔（mrs 叶 / cred-unref zone tag #7）。fixture 183→**195**。
- **★口径坑根治（会话二十七同款再犯）**：`_vnode_authorize` end-scan
  停在 VBAD 快路早期 ret（24B），anchor_reviewed 权威 = matcher 自身
  函数区间（112B，含慢路）→ 首发 emit 报 "Reviewed function bytes
  changed"。修法 = 全量按 `Side.functions[addr]['end']` 重校 195 条
  （仅此 1 条漂移），`make_fixture.py` donor_entry_bytes 改为
  matcher-区间权威 + end-scan 兜底。**教训入册：新增 fixture 一律过
  make_fixture check（现已同口径）后再出件。**
- **出件**：kernel `07f4af0a` / payload `97f091b3`
  （`out/m2381/payload-batchrecon2-97f091b3.img`；坏件
  `1671a570` 已删——第一次 emit 失败后 payload 是旧内核打的）。
  账本 2850→**2668 站**/773 目标（−182 站/−25 目标）。
- **刷写**：**71e5d28b 作废不用，直接刷 `97f091b3`**（含 183+184-195
  共 13 笔生效）。预期 = sandbox 启动函数 + MACF 集成链 + cred/vnode
  段一并扫平，首发墙应深达 apfs 段或更后。
- **回退**：recovery_a ← `out/m2381/payload-erandom-shim-73be0a25.img`。
- **红线不变**：只 recovery_a；boot_b d0200a6b / userdata fd43ee70 不动；
  CRC 行为准。

## 7i. 会话三十六执行档 = 墙 27 定谳 + hit#N 计数器上岗（2026-11-01 凌晨五轮，97f091b3 首发屏摄 032610）

- **★批量三发生效实证 + 墙 27**：屏摄 CRC 行先验通过（zlib 对拍
  **0x74feb86**，132 字节重建消息逐字节吻合，转录零漂移）。
  `fileset: original component called an unmapped kernel function from
  0xfffffff00a52f1a8 (x0 0xfffffff14c9a0b50 x1 0x66d577f00a52f6dc)`；
  pages/free=0x125b8f/0x12534f。`kc.py wall`：站点 bl → donor
  **0xfffffe000ace1618** = `~_proc_set_syscall_filter_callbacks+0x104`；
  墙 26 目标 0xace1514 已不在账本 = 13 笔生效实证，墙 27 是同启动链
  +0x1c 的下一只。
- **定谳三连（kern_proc.c MACF 缺口清仓，全 sandbox 名下 8 站）**：
  donor 物理序 `0xace1514(墙26已修锚) → 0xace1578 → 0xace1618 →
  0xace1660` 与源序 kern_proc.c `:5355 set_callbacks → :5374 set_index
  → :5392 set_sandbox_info_callbacks → :5410 set_filter_message_flag`
  一一对应：
  | donor | native | 证据 |
  |---|---|---|
  | `0xace1618`（本轮墙） | `_proc_set_sandbox_info_callbacks` | 与 native 0x78963bc 逐指令同形；version@0==1(SANDBOX_INFO_CALLBACK_VERSION)/cbfunc@8/全局 mac_proc_get_sandbox_profile 判 NULL/str 回填；**坑=EPERM 是 1 不是 13**（BSD errno，13 是 EACCES），`mov w0,#1` 即 EPERM |
  | `0xace1578` | `_proc_set_syscall_filter_index` | 双锚夹逼唯一；`cmp w0,#2`=SYSCALL_MASK_KOBJ(proc_private.h:262)；donor 内联 ipc_kobject_set_kobjidx（umull 常量除法 0x7f218557+kobj 表基址），native 真调用=内联决策工具链税 |
  | `0xace1660` | `_proc_set_filter_message_flag` | 与 native 0x7896400 形状全同（cbz x0 早退/pacibsp/tst#2+csel/bl task_set_filter_msg_flag） |
  fixture 195→**198**（设备仓 **6250404**，size 160/72/72 matcher
  权威区间与 end-scan 双口径一致，`make_fixture.py check` 198 条全稳定）。
  账本 2668→**2660 站**/773→**770 目标**（−8 恰为三连）。
- **★分支②触发（工作单 A.1，待拍板）**：墙 24→25→26→27 连发全落
  sandbox/kern_proc 段，暴露快于配平条件成立。sandbox 组件账本残余
  **54 目标/206 站**。选项：甲=整组件清仓（批量侦察 54 目标一次性
  扇出，模板同会话三十二/三十五）；乙=继续逐墙+小批。等用户拍板。
- **hit#N 计数器上岗（工作单 C）**：`fileset-unmapped-call.patch`
  panic 消息尾加 ` hit #%u`（static counter，`++n` 先增）。
  **★新坑入册：树步 patch applier 按 hunk 头计数严格截行**——改
  patch 增删行必须同步改 `@@` 头 old/new 行数（本次 15→18），截断
  编译错在 panic 续行处（"expected expression"）而表象像格式串问题。
  另：`make_fixture.py` 需带
  `PYTHONPATH=UniXNU-m2381/kits/XnuPrelink` 才能走 matcher 权威区间
  （kc_image/fileset_match 在 kit 目录，否则静默回退 end-scan）。
- **出件**：kernel **`243c8bfb`** / payload **`0c6b8419`**
  （`out/m2381/payload-wall27trio-0c6b8419.img`，96MiB；trust
  ebeaf7fb/seed d04a0153/args 不变）。ADT sha 每轮必变=
  make-m2381-adt.py `chosen/random-seed=os.urandom(64)` 设计如此，
  非异常。构建插曲：prepare-xnu-source.py 网络读超时一次（30s
  timeout，重试即过）。
- **刷写卡**：现态 = recovery_a `97f091b3`（本轮判读件）、boot_b
  `d0200a6b`、userdata `fd43ee70` → **只刷 recovery_a ←
  `out/m2381/payload-wall27trio-0c6b8419.img`**。判读门：①墙 27 消失
  （屏上可见 hit #N 行=计数器在岗）②过 sandbox 回调注册段后下一墙
  位置（若仍 sandbox 段=分支②证据加码）③CRC 行为准。回退 =
  recovery_a ← `out/m2381/payload-batchrecon2-97f091b3.img`。
- **红线不变**：只 recovery_a；--trust-cache/--data-ramdisk/
  --unique-chip-id 3645ad55d47e77c1 必带；--ipsw 本地件必带；-j16。

## 8. 工作单 = 会话三十六（2026-11-01 收队定稿）：批量轮第三发 + 可视化改进

- **现态**：recovery_a = **`97f091b3`（待刷，含 13 笔生效：墙 26 + 批量二批 12）**；
  boot_b `d0200a6b` / userdata `fd43ee70` 不动；账本 2668 站/773 目标；
  路径上界 = 批量二发后复跑（约剩 ~30 目标/235 站）。
- **A. 判读（用户交 97f091b3 报错屏后）**：
  0. CRC 行先验（zlib 对拍，逐墙先例）→ `kc.py wall <屏上地址>` →
     账本/源码定名 → fixture（make_fixture 已改 matcher 口径权威，
     **出件前必过 `make_fixture.py check`**；emit 失败后的 payload 是
     旧内核打的，删除重出）。
  1. 分支：①首发墙换组件段（apfs/更后）= 批量模式兑现，继续按批推；
     ②同 sandbox/kern_proc 段连发 ≥2 = 暴露快于配平，按拍板换整组件
     清仓策略；③非哨兵形态 = 按形态归因（固件面 vs 内核面）。
- **B. 批量轮第三发（不挡判读，可先行）**：bootpath 榜复跑（2668 现势）
  取 d 序下段 12-16 目标 → 三路扇出侦察（会话三十二/三十五模板：map
  缺口 + 源序夹逼 + KDK 形状锚 + 体比对；子代理并行，主会话抽查 2 笔）
  → fixture → 出件。NO-NATIVE 类直接走 shim 通道（模板 =
  `fileset-read-erandom-shim.patch`：C 包装 + Mach.exports 保活 +
  series 挂 arm64e prng-getentropy 之后；**注意 make_fixture 对新 shim
  符号要先出件一次带补丁的 kernel 才能过符号校验**）。
- **C. 可视化改进（本轮拍板承诺）**：`fileset-unmapped-call.patch` 的
  panic 消息加自增计数器（`hit #N`，static counter），拍屏直接可见
  第几只地鼠；改完 `./unixnu defconfig` 不需要（非 defconfig 改动），
  重跑 tree + kernel。
- **D. 推送拍板（积压清单，等用户）**：主树 m2381-ios27（STATE 若干笔
  + shim 补丁）force-with-lease；设备仓 UniXNU-m2381（fixture 164→195
  + 批量提交）。
- **E. 止损不变**：同区连发卡 → 停修归因回报拍板；账本其余路径外
  520 目标/约 2300 站 = 背景，不挡 boot 不追。
- **红线不变**：只 recovery_a；--trust-cache/--data-ramdisk/
  --unique-chip-id 3645ad55d47e77c1 必带；--ipsw 本地件必带；-j16；
  CRC 行为准。

## 10.Y 会话四十一开工单 = 墙 31 判读与修（donor 内部消费者面，2026-11-03 首刷定谳）

- **墙 30 已过（payload `24f0a941` 首刷，屏摄
  设备仓 records/mackernel-v2-24f0a941-firstboot-20261103-075316.jpg）**：
  crc=`0x13dac5e7` 换新；stages 推过 `mapping_adjust`；pages=0x125b1c
  上穿判据线（>0x125b07）、free=0x1254dc。v2 逐字段重签真机生效。
- **新墙 31 = donor 自己的消费者 PAC 失败**：`PAC failure from kernel
  with IA key while authing x16 at pc 0xfffffff00a97af90, lr 0x…a97af60`，
  x16=x17=`0xf9aec470_0a97bba4`（donor text 指针带签名位），x8=
  `0x90ff94f0_0a70995c`（=会话二十一 cckprng funcs[5] 丢 x1 的 shim，
  高位签名垃圾），x0=`0xfffffff00b6042a0`（=会话二十一 cckprng ctx
  全局 [ctx+24]回调/[ctx+32]arg）。pc 在 donor text 区（0x8330000 内）。
  = 墙 29 家族的 Mac 面 / C 预警兑现。

## 10.Y.1 会话四十一执行档 = 10.Z 定谳 + 墙 31 根因定谳与修（2026-11-03）

- **10.Z 三假说全闭环（对 21H461 donor KC 实测）**：21H461 KC =
  `cputype 0x100000c/cpusub 0/filetype 2` = **arm64 非 arm64e 且全镜像无
  LC_DYLD_CHAINED_FIXUPS**；21H461 profile 无 `[os] arch` 键 →
  profiles.py:42 默认 arm64。**整条 21H461 车道从 donor 到 native 零 PAC
  状态 → H2 成立（税面根本不存在）**；H1 vacuous（无 auth 槽可比）；H3
  前提不成立。旁证闭环：上游 sheng 真机无税 = Apple clang 建核 per-type
  惯例与 donor 天然同族。**结论：翻译税 = 本车道独有组合（arm64e donor ×
  非 Apple 惯例 native），上游无可抄**。另证：rebinder
  （fileset_emit.py:169-335）逐位保留链词 auth/key/diversity 只重算
  target_offset——不存在"按消费者惯例重签"的通道。
- **上游情报刷新（fetch 至 fb0ec4a，11-02/03 合 main）**：a3faa4a **弃
  26 profile**，目标表 = iOS 16.7.16 + iPadOS 17.7.11（均 arm64 共享
  arm64 series）+ **iOS/iPadOS 27.0.1（arm64e）**；arm64e.md 重写
  （sheng/SM8550/24A446 = 唯一 arm64e 支持设备，"In development"）；全部
  sheng 系设备记录 arm64e 证据仍停在 2026-10-26 "fastboot download and
  boot returned OKAY; **kernel and userspace execution remain
  unobserved**"——**上游 arm64e fileset 车道零执行实绩，墙 29-31 为
  无人区**（"ipados27 跑起来"= xnu-12377（26 代内核）+ Darwin26 补丁跑
  27 用户态的**设计**叙述，我们同构）。注意：换 donor 23F84→24A446 会
  作废全部 donor 侧 fixtures（字节钉），另立决策勿顺带。
- **墙 31 归属修正 + 根因定谳（出厂件 24f0a941 内 sha 40b7dd55 逐 member
  归位）**：pc 0x0a97af90 **不在 donor text**——在 **com.apple.kernel
  成员 __TEXT_EXEC**（0x8330000 hatch 范围同时罩住 donor kext 与我方
  kernel 成员，初判误归）；shim 0x0a70995c 在 corecrypto 成员。反汇编
  出厂件 = `register_and_init_prng`（prng_random.c，entropy 链）：参数
  编排逐项对上（x0=ctx 0x…42a0/w1=8/w2=0x40/x3=seed/x4=8/x5=&nonce/
  x7=0，lr=af5c bl ml_get_timebase 返回点）。**根因 = prng-getentropy
  补丁在 Xcode 27.2 下的坏 lowering**：wrapper 参数/返回值标注为
  cckprng_getentropy（注解类型）→ Apple clang 把"未注解函数引用→注解
  类型"转换降低成 `paciza→xpaci→pacia1716(0xbcad)→cbz→**autiza(disc 0)**
  →cmp/brk→pacia(0xbcad)`——签 0xbcad 后按 disc 0 回验，**必失败**：
  FPAC（SM8550 X3/A710）在 autiza 同步异常 = 墙面（x16=x17 tag
  0xf9aec470 全合）；soft-fail 硬件也会走 brk #0xc470。Linux clang 同源
  码只出 `paciza→xpaci→pacia(0xbcad)` 无回验（对拍
  unstripped register_and_init_prng 逐指令，本会话铁证）→ Linux 车道
  entropy 过墙真因。x8=0x90ff94f0_0a70995c 定谳 = **donor 栈建 funcs 表
  槽签名 tag（尚未鉴权的原值）**，非二次签名事故，donor 侧无责。
- **修法（主树 `01d6cdd` 单提交）**：wrapper 边界去类型化
  （`static void *unixnu_donor_getentropy(void *)`）+ 结果经
  `__builtin_memcpy` 洗进注解局部变量——注解类型转换不再物化，显式
  strip/sign 内建即全部变换。**Linux 回归门过**：新件
  （kernel 77990531）getentropy 块 = `mov #0xbcad→pacia x6` 无
  autiza/cmp/brk，blraa disc 0x3051 参数编排不变；prelink 账本持平
  （closure_unmapped 603/branches_rebound 42024/chain_sites_kept
  82701）。同款构造排查 = 全树仅本处（kext-entry/sysctl/mac-policy 三
  补丁已是 uintptr_t/槽存储惯用法，bootstrap 0xe4fe 惯用法 Mac 件已
  实证）。
- **下轮 = Mac 重出件 + 反汇编门**：Mac git pull → kernel 重建 → 对新
  kernel.macho 的 register_and_init_prng 跑
  `temp/s41-w31/gate-register-and-init-prng.py`（指纹检测器已在出厂件
  验证：能精确抓 `pacia1716(0xbcad)@af84→autiza@af90`）→ autiza_after=0
  才出 payload → 刷 recovery_a 先判。判读门 = 墙 31 消（stages 推过
  entropy/prng 段，pages/free 上穿 0x125b1c）；回退 = 刷回 `24f0a941`。
  工具留 temp/s41-w31/（出厂件提取件+门+dis23.py/scan-21h461-crypto.py）。
- **Mac 车道已毕（同日）**：Mac pull c4337a9 → kernel `efbdd999` →
  **反汇编门过**（getentropy 块 = `paciza→xpaci→pacia1716(0xbcad)→
  mov x6→blraa 0x3051`，与 Linux 同构；全 TEXT_EXEC 14 处 pacia1716
  站点 0 坏指纹）→ A/B 账本四项全平（47473/334/42024/82701）→
  **payload `2273bd09`** = out/m2381/payload-w31fix-efbdd999.img
  （96MiB；ESP 副本 fe181246 = cpusub/probe/seal 后，指纹终验同样全过）。

## 10.Y.2 会话四十一首刷提取+核验档（2026-11-03，payload `2273bd09`，屏摄设备仓 records/mackernel-w31fix-2273bd09-firstboot-20261103-084514.jpg）

- **墙 31 已过（修复真机生效）**：cckprng autiza 墙消失，entropy/prng 链全通（墙 29→30→31 三连穿墙）。
- **新墙 = 换类回 unmapped-call stub 族**（墙 10 家族，非 PAC 类）：`fileset: original
  component called an unmapped kernel function from 0xfffffff008d41f6c (x0
  0xfffffff00749b2b6 x1 0xfffffffa124b8490) hit #1`，pages=0x125b1c（与墙 31 boot
  同水位）/free=0x1253dc，**crc=0x4be0eb86 = 139B 转录 zlib 一次命中（零掉字）**。
- **站点定谳**：报的是 lr（0x…f6c=0x08d41f68 bl 返回址）；owner =
  AppleImage4 __TEXT_EXEC；未映射目标 = donor **0xfffffe000aead3e0**（前一条 bl
  0x0aead2c8 = 已配 `_IOLog`，邻居锚）；账本 = AppleEffaceableStorage 组件名下
  **4 站**（跨组件调用）。邻居夹逼 = `_IOLog`(0x…2c8)..`_IOFindValueForName`
  (0x…568) 间隙（IOLib.cpp 源序：IOLogv/_IOLogv/IOPanic/IOKitKernelLogBuffer/
  IOFindNameForValue）；x0 = C 串 "no legacy blob present"（AppleImage4 域），
  x1 = tag 0xfffffffa 签名数据指针。**Linux 件已配 = `_IOLogv`
  （how=reference-order）→ Mac 件漏配 = Mac LTO 参照件序位移致 reference-order
  失效**（body/shape 通道也未见配）= **Mac 车道特有配对缺口，单 fixture 级**。
- **下轮工作单**：①按 fixture 通道钉 donor 0x0aead3e0 ↔ native `_IOLogv`
  （Mac native VA 从 Mac 件符号/map 取，fixture 格式参照 9e5b51b reviewed 锚；
  donor 字节 sha 钉从 23F84 KC 截取）→ check → Linux 件重建不受影响验证 →
  Mac 重出件（kernel→payload）→ 刷；②预期 4 站连消；③若后续站点同因
  （reference-order 在 Mac 参照件上的系统性失效面）→ 统计 Mac 账本里
  Linux how=reference-order 集的漏配数，评估批量（勿铺开，先修一刷一判）。
  回退 = 刷回 `24f0a941`。
- **战略读数**：Mac 车道已并线到 Linux 车道当年的调用序（entropy 链后的
  AppleImage4/EffaceableStorage 段 = Linux 车道会话二十一走过的地方），剩余
  墙面 = 存量 fixture 级缺口，通道现成。两阶段阶段一（Mac 开路）持续推进。
- **★刷写卡（会话四十一）**：设备现态核对（boot_b 不动 / recovery_a
  现役 = 24f0a941 / userdata 不动）→ **只刷
  `fastboot flash recovery_a out/m2381/payload-w31fix-efbdd999.img`**
  → 判读：crc 换新；墙 31（cckprng autiza）应消，stages 推过
  entropy/prng 段；新墙按调用序点名（候选 = §10.X 判读注记的 di 字段
  与 chacha 子结构 CC_SPTR 链）。回退 = recovery_a ←
  payload-v2-24f0a941.img。
- **〔原 10.Z 开工单文本存档（已执行完毕，定谳见上 10.Y.1）〕**
- **★10.Z 专项查（下会话第一刀，2026-11-03 用户拍板立项）：上游/sheng 为什么无 crypto 翻译税**
  事实基线：上游 61 条 xnu 补丁（patches/xnu/）**没有 crypto unpack/翻译
  等价物**（本会话已 grep 穷尽：ptrauth/donor/resign/cckprng 仅 3 文件、
  无表处理）；但 21H461 车道跑进过 SpringBoard（entropy/digest 全过）。
  三个假说，对 21H461 donor KC 直接检验：
  - H1（注解==默认）：21H461 donor 槽 disc 恰好等于 Apple clang per-type
    默认（17.7 代 Apple 内部注解与编译器行为一致，26 代分叉）→ 检验 =
    对 `out/ipsw/21H461/kernelcache.decompressed` 跑会话四十同款流程
    （kc_image fixups → corecrypto 56 槽 run 扫描 ad=0 key0 → 对比
    temp/mackernel-sym39/gcf_field_table_v2.json 的 15 字段值）。
  - H2（无税可交）：21H461 donor crypto 表根本非 auth 打包（平 rebase
    指针）→ 检验同上，看槽是 auth 还是普通 fixup。
  - H3（管道内交税）：上游 fileset rebinder 在 rebind 时按消费者惯例
    重签（kext_pointers_rebound 47166 那条通道）→ 检验 = 读
    fileset_emit.py/fileset_match.py 的 rebase_collection 对 auth 槽的
    处理，再对照 21H461 运行态值。
  - 判定后的行动：H1/H2 成立 → 墙 31 的修法应优先考虑"注册权换手"
    （gcf 指向源码 corecrypto 件、donor 件留场供 donor kext 符号调用）
    或沿用 v2 表扩展 donor ctx 族；H3 成立 → 把上游的重签惯例抄过来。
  - 并行：wall 31 donor 符号化（0x0a97af90/0x0a97bba4，23F84 KC 套
    fixups 运行态反汇编）+ x8 shim `0x0a70995c` 签名来源判定。
- **会话四十一工作单**：
  1. donor 侧符号化：对 23F84 kernelcache 反汇编 0x0a97af90（pc）与
     0x0a97bba4（x16 低 32 位目标），判 signer 与惯例（槽址连通 vs
     常量 vs 我们重签过被二次鉴权）。工具 = kc_image.py + donor
     反汇编（注意 donor 文件态 vs 运行态差异，套 fixups）。
  2. 判定 x8 shim 签名垃圾来源：谁签的 0x0a70995c（donor 自签?
     我们 unpack 重签后又经 donor 槽址 autia？）。
  3. 修法定性后单提交落件（候选：donor 链专用 hatch / shims 表补签 /
     funcs 表逐槽三元组翻译），先刷先判。
  4. 回退 = 刷回 `a715395a`。



## 10.Y.3 会话四十二执行档 = 墙 32 定谳与修（2026-11-03，payload `d7fd50cb` 待刷）

- **墙 32 修毕**（10.Y.2 工作单①）：设备仓 `ff4a5d4` = fixture 单条
  （donor `0xfffffe000aead3e0` ↔ native `_IOLogv`；original_size 0x10c，
  字节 sha `2d1919ea…a321d` 按 matcher fn['end'] 口径；evidence 全链）。
- **站点定谳补全**：donor 0x0aead3e0 体形 = varargs logging worker
  （pacibsp + 0x160 帧带栈金丝雀、`mov x4,x30`+xpaci 抓调用方返回址做
  前缀、256B 栈 scratch 清零、格式串拷贝 bl 0x0ae8b0bc、console 门字节
  0x07da4c5a tbnz、pacia 签名 handler 0x0a92a0c0(div 0xae56) 传
  bl 0x0a800184 emit、retab）；IOLib.cpp 源序夹逼 `_IOLog`(0x2c8) ..
  `_IOFindValueForName`(0x568)。
- **Mac 漏配机制**：Mac LTO 参照件序位移 → Mac 图**整个 0x0aead log
  邻域全空**（不止 4 站目标函数，邻域配对全灭）；native 两侧 `_IOLogv`
  均为小壳尾调静态 worker `__ZL7_IOLogvPKcPcPv`（Mac 0x…7a59b6c 0x10 /
  Linux 0x…7a92a3c 0xc；worker 0x134/0x154），Linux 车道当年配壳过墙。
- **验证链四门**：①双车道 anchor_reviewed 直跑 = Mac→0x…7a59b6c、
  Linux→0x…7a92a3c，how=reviewed；②Linux no-fix 离线重放（同 native
  b9b2aaad、临时 kit 去 fixture 条目）内容 diff **0 条**、how-only 恰
  1 条 = 纯 relabel 零内容漂移；③Mac 落件级：`4970bf21` 内 panic 站
  bl@0x…8d41f68 → **0x…0aecdb6c = emit 符号表 `_IOLogv` 真身**
  （capstone 逐指令 = native 壳 `bti c; mov x2,x30; xpaci x2;
  b worker`，stub `_unixnu_fileset_unmapped` @0x…b057e04 另址已不在
  环上）；④账本四项平（47473/334/42024/82701）vs `efbdd999`。
- **★新知 a（构建面）**：`./unixnu kernel` 每次重编 xnu（noctf 漂移：
  本地 b876efcd→b9b2aaad、Mac 8f8c0765→4672ced7）→ **fresh matcher
  收敛对数随 native 漂移摆动 ±1500 对**（8386 基线 / 8388 现建 /
  9896 no-fix-on-drifted-native 三点实测）→ 跨建整件 sha 不可比、
  fixture 影响隔离只能靠 no-fix 同 native 重放内容 diff + 头牌账本，
  不再可能做整件 byte-identical 门。
- **★新知 b（emit 面上一号）**：emit 件内 native 文本整体重定位
  +0x3474000（native `_IOLogv` 0x…07a59b6c → 件内 0x…0aecdb6c），
  **件内 nm 符号表即权威**（`_IOLogv`/`__ZL7_IOLogvPKcPcPv` 相对布局
  0x134 与 standalone native 同构）；判读落件站址先 nm 后反汇编，
  勿手拆 word 猜 stub（mov x2,x30 是壳传 LR 惯用法非 stub 特征）。
- **出件**：payload **`d7fd50cb`** = out/m2381/payload-w32fix-4970bf21.img
  （96MiB；kernel 4970bf21；trust ebeaf7fb + seed d04a0153 +
  panic_screen；Mac 建本地拉回逐字节对拍 d7fd50cb）。
- **★刷写卡（会话四十三）**：设备现态核对（boot_b 不动 / recovery_a
  现役 = 2273bd09 / userdata 不动）→ **只刷
  `fastboot flash recovery_a out/m2381/payload-w32fix-4970bf21.img`**
  → 判读：crc 换新；墙 32（unmapped-call @ lr 0x…8d41f6c
  "no legacy blob present"）应消，4 站连消，stages 推过
  AppleImage4/EffaceableStorage 段（Linux 车道会话二十一走过处）；
  新墙按调用序点名（候选 = 会话四十判读注记 di 字段与 chacha 子结构
  CC_SPTR 链，或 EffaceableStorage 后续 ADT/NVRAM 链）。回退 =
  recovery_a ← payload-w31fix-efbdd999.img（`2273bd09`）。
- **未推**：主树本节 + 设备仓 `ff4a5d4`；push 留用户拍板。
- **下阶段工作单**：①首刷判读（如上卡）②若后续站点同因（Mac 参照件
  序位移系统性漏配面）→ 统计 Mac 账本 Linux how=reference-order 集的
  漏配数，评估批量（勿铺开，先修一刷一判）③工具沉淀：temp/
  w32-fixture/{compute,verify}.py（fixture 前置计算 + 双车道验证，
  可复用为 fixture 通道标准门）。回退链 = `2273bd09`。


## 10.Y.4 会话四十二首刷提取+核验档（2026-11-03，payload `d7fd50cb`，屏摄设备仓 records/mackernel-w32fix-d7fd50cb-firstboot-20261103-110042.jpg）

- **墙 32 已过（修复真机生效）**：旧站（lr 0x…8d41f6c "no legacy blob
  present"）消失，IOLogv 链路解锁（artifact 级已验 bl→真 `_IOLogv`）。
- **新墙 = 同族 unmapped-call stub**：`fileset: original component called an
  unmapped kernel function from 0xfffffff008f13530 (x0 0x0 x1
  0xfffffff71f65e170) hit #1`，pages=0x125b06 / free=0x1253c6（vs 墙 31 boot
  0x125b1c/0x1253dc = 件间漂移量级持平），**crc=0xcae97f1 = 124B 消息行
  zlib 一次命中（零掉字，转录全验真）**。
- **★初勘判读已前置到修面**（指纹法：3-word 非重定位窗口 donor 全镜像
  唯一搜索）：
  - caller = donor `0x08f1352c` = **AMFI 成员** __TEXT_EXEC（delta =
    collection slide `0x1f000000000` 全局匀滑）；调用形态 = once-init CAS：
    x0=0、x1=x19（前一条已配 callee `0x0a7f3548` 返回的 tag 指针）、
    x2=&com.apple.kernel 全局槽 `0x0b54bf98`，返回 `tbz w0,#0` 判布尔。
  - callee = donor **`0x0ae3fe94` = `_OSCompareAndSwapPtr`**（Linux 配对图
    定名；bti c; mov x8,x0; **casal x8,x1,[x2]**; cmp; cset w0,eq; ret =
    64 位 LSE CAS，与 OSCompareAndSwap64 同体形，once-init 指针发布语义
    = Ptr 版；初勘误名 OSCompareAndSwap64 已纠）。邻居
    `_OSCompareAndSwap`(32b, 0x0ae3fe64) 已配（pac-checked-shape），
    未配对二连 = `0x0ae3fe7c`(_OSCompareAndSwap64) + `0x0ae3fe94`(Ptr)。
    native 符号 = `_OSCompareAndSwapPtr`（Mac native 0x…079eb964，
    唯一全局；勿用 0x…079eb94c=OSCompareAndSwap64）。
  - 红利面 = donor 全镜像 **38 站** bl→0x0ae3fe94（跨组件 + kernel 内）。
- **进步判读**：墙 32 四站连通过；stub 站点前进 +0x1D15C4
  （0x08d41f6c → 0x08f13530）；同族墙延续 = stubbed 长尾下一笔
  （mac stubbed 账本 3099 站/24 组件 + kernel 成员走 allow-missing 通道
  不入账本）。
- **★下轮工作单（判读已备好，直接进修）**：fixture 钉 donor
  `0x0ae3fe94` ↔ native `_OSCompareAndSwapPtr`（通道 =
  temp/w32-fixture/{compute,verify}.py；original_sha256 待算）→ 双车道
  anchor_reviewed + no-fix 重放 diff 门 → Mac 重出件（kernel→payload）→
  刷只 recovery_a → 判读。回退 = 刷回 `2273bd09`
  （payload-w31fix-efbdd999.img）。注意：`0x0ae3fe7c`
  (_OSCompareAndSwap64) 与 Ptr 同批未配，fixture 只钉本笔勿铺开，
  撞了再修。
- **★墙存量总账（10.Y.4 追加，用户问"还剩多少墙"的账本口径）**：
  - 墙宇宙三层：①kext→kernel 未名 imports = **1225 个 kext-facing 未名
    目标**（3094 named 1869；墙 32/33 类，走 allow-missing 不入账本；
    **至今撞过的全是 Linux 已配 = 系统性可回收**）②kext 自身 stubbed =
    **675 fn / 2290 站**（apfs 640 站/148 fn 为大户，ARMPlatform 301、
    IOHIDFamily 295；root mount 前必清）③data/missing = 603 指针。
  - 会话三十一 bootpath 清点（Linux 车道同 donor 同机制）= **关键路径剩
    52 目标 / 536 站**（d 序榜在，apfs 大户在后）——到 root mount 的存量
    量级。
  - 速率：Mac 车道 4 刷 4 墙（30→33），fixture 清族 4→38 站。推算：
    **一次一判 ≈ 20-40 刷到 root mount；按族批处理 ≈ 10-20 刷**。
  - **战略杠杆 = reference 换件**：Mac 独缺 1750 对里 1586 对 =
    reference 通道系统性失效（Mac 参照件 LTO 序位移）。把 Mac 车道
    `--fileset-reference` 换成 **Linux 建的参照件**（同 donor、只借
    名字+序，跨 native 构建合法）→ 大规模回填 → 未点燃的墙多数不炸，
    可能压到个位数刷。 工作单"评估批量"的量化版；上桌前先小验
    （reference 换件重放 matcher，看 named 覆盖升幅与 no-fix diff）。
  - root mount 之后 = BSD/launchd 段一批；再之后 = 用户态战役
    （dyld/SpringBoard，y-lane 死地，sheng arm64 有进桌面先例面）。
    到 root mount 和到桌面是两个量级。
- **未推**：主树 10.Y.3+10.Y.4 + 设备仓 `ff4a5d4` + records 两笔屏摄；
  push 留用户拍板。


## 10.Y.5 上游情报刷新 = sheng arm64e 执行实证实锤（2026-11-03，fetch upstream/main 3fa5ff5 + 克隆 UniXNU-sheng 深度 12）

- **★10.Z 结论部分作废**：「上游 arm64e 零执行实绩」是 **10-26 时点真、
  11-02 已过时**。sheng `porting.toml` + `reports/
  ipados27-startup-repair-2026-11-02.md` 记录：**24A446（iPadOS 27.0.1，
  arm64e）boot 到 SpringBoard PID 30 / backboardd PID 61 / wifid PID 49，
  120s autoreset 无 panic**；「当前 sheng iPadOS 27 image boots and
  accepts touch interaction，shader priming 280/280」；arm64e completion
  标准 = 「reaches launchd from the 23F84 root volume, then
  SpringBoard」。**上游不但真的在用 arm64e，还跑到我们前面**
  （我们墙 33 = pre-root-mount）。
- **开发者宣布三连核实**：①27 补丁/profiles/scripts 合入 main = 真
  （4ed86f6 profiles + 7b02190 xnu-12377 series + ab11a8c scripts——
  就是我们自会话十一起消费的基础设施）；②Metal GPU 优化合入 = 真
  （PR #160 GPU 调度 + turnip GMEM + Metal translator 修复 + 七发延迟
  捕获 + a366379 组件 pin，11-01~02 新落）；③机型仓出包脚本 =
  真（`templates/package.sh` + `unixnu release` + package.yml，
  porting.toml 明文「fork 里跑、下载后删 run」；**但上游自己
  macos.yml 运行史 = 唯一绿是 10-21 qualify 7m36s，11-01
  fix/sheng-ipados27-startup-pipeline 5m14s 取消**——fork 自跑是
  预期用法，与我们会话三十三 CI 容量结论一致）。
- **他们墙序（11-02 一天内修穿）**：①instruction fetch abort @
  native_legacy_display_allocate（Corecrypto/SourceStorage 已过）→
  修显示；②APFS 挂 root+Cryptex 后 load_init_program 死（payload 缺
  initrd/PackedRAMDisk）→ 补运行时输入（seed/trust/GPU addon/ADT/args
  870B）；③v4 修完 → SpringBoard 活。**残余 = 2 个未解析 data import
  + 24 trap references（带账上岗，不阻 boot）**——对比我们 Mac 图
  1225 kext-facing 未名 + 2290 stubbed 站：**他们的配对近乎全收敛**
  （统一 Apple clang 参照+native = 序一致 → reference 通道通吃），
  这是速度差的根源，也是①reference 换件实验的直接佐证。
- **他们的 debug 武库（回答「上游怎么 debug」）**：USB 串行日志通道
  （`read-sheng-log.py --port /dev/cu.usbmodemshenglog1 --no-reboot`，
  免刷机读日志）；`fastboot boot` RAM 驻留诊断件（不刷分区；
  **⚠ sheng 可用，m2381 靴子不支持 `fastboot boot`——用户 2026-11-03
  拍板确认，我们诊断路径仍 = 只刷 recovery_a**）；
  `sheng-autoreset=120` 泡测模式；每件静态审计（unittest kits/
  XnuPrelink + audit-startup.py 266 分支核验 + verification.json
  清单）；内核内 LAT1 观测环（32768 fault + 491519 事件、IPC 按
  cookie 配对、backboardd 帧迹）+ 宿主分析脚本
  （sheng-latency-report.py）。**先仪器后调参**。
- **harvest 清单**：①reference 换件佐证已得 ②build-gop-legacy-
  display-27.py / make-nosetup-seed27.py / cycle27.sh /
  run-sheng-unattended.sh 待 diff ③他们的 SM8550 组件栈（GPU/充电/
  音频/触控/WiFi）与 m2381 同 SoC 直接可移植（进桌面后）④arch/arm64e
  series 与我们逐条相同（同源，无差可 harvest）。
- sheng 仓克隆留 `/tmp/sheng-probe`（depth 12）。


## 10.Y.6 会话四十三执行档 = reference 换件大捷 + 墙序预测器 + 墙 34 fixture（2026-11-03，payload `abe3ec85` 待刷）

- **开工三件套**：push 两仓（主树 `c1558cb..775a89c` 21 笔 / 设备仓
  `718a533..5f7a906` 2 笔，用户拍板）+ Mac 两仓 `git pull` 对齐（Mac 侧
  scp 版 fixtures 与 `ff4a5d4` 逐字节同 → restore 后 ff-only 干净落位）。
- **★根因定谳（推翻 10.Y.3 的"Mac 参照件 LTO 序位移"机制说）= Mac 车道
  从未挂过 reference**：board.toml:168 extra_args 本就带
  `--fileset-reference`，但 kernel.py 对它有**静默丢弃逻辑**（"a runner
  without them still links, just without the naming reference"），而参照件
  是**手工侧建产物**（管线不自动建）——Linux 树在（10-30 建，sha
  `5a794da8`），Mac 树从未建 → 每次 Mac 建核无声剪掉。实测对账：Linux 图
  8742 对中 reference 通道 5909（symbol 1457+order 4452）；Mac 图
  （`mac-map-new.json`，native 4672ced7）8388 对中 reference 通道 = **0**。
- **①换件小验（零上机，全在 Linux 本地，`temp/refswap/mapB.py` = 产线
  match_images 口径：同 donor + 同 Mac native 4672ced7 + Linux 参照件）**：
  - 三计数：新增 **1828**（order 1327 + symbol 261 + vtable 101 + …）；
    已配丢失 320 **但 kext-facing = 0**（全为 kernel 成员内部对，不产生
    stub 面 → 墙安全，丢失门按安全子集判过）；共配改靶 86（抽查 =
    pure-leaf 歧义被 reference 纠正的 A 图真错，如
    `mig_get_reply_port` ↔ `lck_spin_sleep_block_invoke` 交叉错配摆正）。
  - reviewed 252 锚全不变（含墙 32 IOLogv）；kext named 1869 → 2046；
    functions_matched 8388 → 9896；data_matched 1166 → 1228
    （reference-data +97）；double-bind 174 → 318（通道竞争，预期内）。
  - **墙 33 callee `0xfffffe000ae3fe94` 自动转已配 → `0x…079eb964`
    _OSCompareAndSwapPtr [reference-order]——与 10.Y.4 预测地址逐字一致，
    墙 33 无需 fixture 即清**；CAS64 邻居 `0x…0ae3fe7c` 同批配上
    （→ `0x079eb94c` = 正确的 OSCompareAndSwap64）。
- **②预测器 v2（`temp/refswap/predict.py`：bootpath 行走器吃 map-B，
  AMFI 根 BFS，墙集 = fixup kext→kernel 未配目标，与 emit 账本口径对齐
  {kext: {内核VA: 站}}）**：到达 3434 fn / 路径 bl 墙 61 / 未配 1194。
  **三色 = 钉名即修 1 / 名缺面 0 / 结构类 1193**（两车道皆未配真残 =
  donor 有而我方 xnu 没有；emit 账本 675 fn/2290 站 = 其有 kept-site
  子集）。**top-K 施工单仅 1 笔**：`0xfffffe000a7985bc` →
  `_mach_memory_entry_port_release`（Linux call-sites 配对 + ipc_port
  邻域夹逼；d=6 / path 75 站 / ledger 5 站）。下墙候选（结构类深度序）：
  `0xa7f3f6c`（d=2，path 20/ledger 5）、`0xa841220`（d=2，path 15）、
  `0xad1d378`、`0xaf69628`（d=3）、`0xa7f8990`（d=4）。坑：行走器 v1
  BFS 快照迭代 bug（只走 2 fn）；bl 度按 fixup 站址读指令字全 0（kext bl
  过本地 stub/veneer，fixup 站 = 指针槽非 bl 指令）→ 入度用 ledger/path。
- **③墙 34 fixture**：donor `0xfffffe000a7985bc`（mach_memory_entry_
  port_release，size 0x214，sha `a7d37a03…`）↔ native 同名（Mac
  `0x0745860c` = 56B 壳 `cmp #0x24` 类字节门 / Linux `0x07469be4`）；
  双车道 anchor_reviewed 过（253rd reviewed anchor）+ donor 体 sha 现场
  重验（`temp/refswap/{fixture-w34,verify-w34}.py`）。设备仓 **`eb32f7b`**
  已推，Mac pull 落位（kit = 符号链接 tree.py:77 直指设备仓）。
- **④Mac 产线出件（reference 首次进 emit 命令行 + 墙 34 fixture）**：
  - 产线图 = **9894 对**（+1506 vs 旧 8388；native 漂移 `4672ced7` →
    `f1eefc43` 无碍），reference 通道 symbol 1250 + order 4390，named
    **2047**；**墙 33 CASPtr → reference-order ✓ / 墙 32、34 fixture 均
    reviewed ✓**（map 拉回 `temp/refswap/mac-map-refswap.json`）。
  - **账本方向门实测**：branches_rebound 42024（平）/ kept 82701（平）/
    **closure_unmapped 603 → 314（−289）/ stubbed 2290 → 1954
    （−336 站）**——stubbed/missing↓ 实证达成。
  - kernel **`0b607b2a`** → payload **`abe3ec85`**（96MiB；
    trust `ebeaf7fb` + seed `d04a0153` + chip `3645ad55d47e77c1` +
    panic_screen；已拉回存档 `out/m2381/payload-refswap-0b607b2a.img`
    逐字节对拍 `abe3ec85` ✓）。
  - **Mac 构建三坑**：①unixnu 的 `-D` 只在子命令级（`./unixnu kernel
    -D DIR` 合法，前置报 invalid choice；device 有 remembered 态可省）
    ②ssh 非交互 PATH 须 `PATH=/opt/homebrew/bin:$PATH` 前缀（homebrew
    python3.14 有 tomllib，系统 3.9 没有，登录 shell 同样不行）③payload
    三红线输入必须显式传（`--trust-cache ~/unixnu/original-trust-23F84.bundle`
    + `--data-ramdisk ~/unixnu/scratch-data.img` + `--unique-chip-id
    3645ad55d47e77c1`；裸跑报错缺件，两件已常驻 `~/unixnu/`）。
- **harvest 线收账（10.Y.5 ⑤⑥⑦）**：sheng 报告 + board.toml 全读——
  他们的 27 车道 = **donor 24A446** + archives 显式含
  `build-fileset-release-reference.py --lto`（参照件 = LTO 版 release
  构建）；我们 23F84 donor 的覆盖数字**不直接平移**（donor 不同源），
  但机制平移（参照件通道）；他们 root = 23F84 卷 + 27 内核**跨代仍到
  SpringBoard** = 工具链统一论再佐证。
- **★刷写卡（会话四十四）**：设备现态核对（boot_b 不动 / recovery_a
  现役 = d7fd50cb / userdata 不动）→ **只刷
  `fastboot flash recovery_a out/m2381/payload-refswap-0b607b2a.img`**
  → 判读：crc 换新；**墙 33（AMFI→CASPtr unmapped-call @lr 0x…8f13530）
  应消**；**墙 34 候选（mach_memory_entry_port_release，path 75 站）应消**；
  stages 应推过 AMFI 段；新墙按预测单结构类 d 序点名（首候选
  `0xa7f3f6c` / `0xa841220`，d=2）；若墙量骤减推进到 sandbox/apfs 段
  （会话三十一 d 序榜后段）= 换件红利兑现。**回退 = recovery_a ←
  payload-w32fix-4970bf21.img（d7fd50cb，现役在机），2273bd09 二级**。
- **未推**：主树本节 + §0；设备仓 `eb32f7b` 已推。

## 10.Y.7 会话四十三首刷提取+核验档（2026-11-03 晚，payload `abe3ec85`，屏摄设备仓
records/mackernel-refswap-abe3ec85-firstboot-20261103-224055.jpg）

- **换件红利立竿见影**：墙 33（AMFI→CASPtr unmapped-call）**消**；全段
  **无任何 unmapped-call 墙**——wall 32/33 时代的墙类被 reference 换件
  清场（251-fixture 时代"压类"现象的换件版）。pages/free =
  0x125b06/0x1253c6 与墙 33 boot 逐字同 = 崩点紧接 AMFI 段之后的下一步
  （sandbox 政策注册）。墙 34（mach_memory_entry_port_release）无从判定
  （未到 IPC 段），挂账。
- **新墙 = 墙 35，PAC 鉴权失败类**（esr 0x72000000 = EC 0x1C PAC_FAIL，
  同墙 29 族）：
  - pc 0xfffffff00afcde0c = 出厂件 `__TEXT_EXEC` → native eq 0x07b59e0c
    = **`_mac_policy_register + 0x520`**（Mac noctf `f1eefc43` 符号化；
    下界符号 `mac_policy_unregister` 夹逼）；lr 低位 = pc-0x14 = 前一条
    `bl 0xa7be288` 返回址（同函数，链路自洽）。
  - **反汇编直读 pc 处 = `blraa x8, x17` 且 x17=0x2abe——与屏摄 x17
    逐字吻合（结构交叉验证替代 crc，四重命中：pc 指令/lr 链/x0=x19
    `mov x0,x19`/x17 常量）。crc 全量验真不收敛**（9 行寄存器照片转录
    差一位，墙 29 同款，按用户 10-31 拍板口径挂档不追）。
  - **目标三级归属**：x8 = 0xfebd5ff00a52e104（签名指针）低 36 位
    0x0a52e104 → 出厂件 `__TEXT_EXEC` donor kext 区（emitted VA ≡ donor
    VA，老式 prelink 保址）→ **donor sandbox 成员代码**（会话三十墙 22
    站点 0xa52ed24 同邻域）。
  - **回调源三级归属**：x19/x0 = 0x082d1950 = `__DATA_CONST` → chained
    fixup 解码直读 = **"Seatbelt sandbox" 的 `mac_policy_conf`**
    （mpc_name → "Sandbox"、mpc_full_name → "Seatbelt sandbox"）；
    [conf+0x20]=mpc_ops → **ops+0x398 = `mpo_policy_init`**
    （逐行条件感知偏移行走；先前按 `;` 切分的解析把字段序打散误读成
    system_check_sysctlbyname，已纠——policy_init 取 `(mpc)` 参数与
    x0=x19 完全吻合）。
  - **槽位 auth fixup 元数据 = key=0(IA) / div=0x2ABE / adiv=0**——div
    与我方 dispatch 站 x17 逐字相同 = **元数据已收敛**（同源代码系 +
    两侧 Apple clang），**非墙 30 型约定债**。
- **判读输入（下轮）**：签名值为何验证失败——机制候选：①出厂件容器 =
  老式 prelink，其运行时 fixup 通道（kxld 系）不执行 auth 链的 pacia
  签名步（donor 系统由 collection loader 在本 boot 签名；我方通道漏签
  → 槽内高位 = 陈旧/垃圾签名）②签名目标盐值差（重定位前后值）③
  resolver 用 adiv 变体签名。倾向 ①（元数据全收敛后唯一剩余自由度 =
  签名执行步本身）。修法方向 = 出厂/运行时链路补 auth 条目 pacia 签名
  步（离线不可签，键为 per-boot 随机）。Linux 车道从未撞此墙 = arm64
  零 PAC（未签名裸指针照用）自洽。
- **记账**：未知墙存量口径换类后沿用预测单（结构类 1193 不变，但墙类
  切换点 = sandbox 注册段早于预测单 d 序的 apfs 段——实证墙序再次
  教预测器：注册回调链是 AMFI 后第一跳）。
- **未推**：主树本节 + §0；设备仓 records 单提交。

## 10.Y.8 会话四十四执行档 = 墙 35 定谳与修（2026-11-04，payload 待出）

- **判读输入复核（屏摄自读）**：x8=0xfebd5ff00a52e104 / x17=0x2abe /
  x0=x19=…082d1950 与 10.Y.7 一致；新证据 = x2/x9/x26/x27 均为
  "高位签名+低位 VA"形态寄存器残值（boot 早期确有签名类值被生产）。
- **★机制链定谳（推翻 10.Y.7 三候选：非"漏 pacia 步"、非盐值差、非
  adiv 变体——是重复签名用错约定）**，每一环实证：
  1. 文件 slot = 原始 packed：在机 kernel `0b607b2a` ops+0x398 =
     `0x80102abe0352a104`（target_rel 0x0352a104、div=0x2abe、key=IA、
     adiv=0）；容器带 LC_DYLD_CHAINED_FIXUPS（186842 站 / auth 99573）、
     filetype=12 MH_FILESET（`temp/refswap/w35-file.py`，fixups 缓存
     `w35-fixups.pkl`）。
  2. 运行时 applier = stock `kernel_collection_slide`（arm_init.c
     MH_FILESET 路径；DarwinBoot.c:189 明示 firmware 只拷贝不签）：
     `dyld_kernel_fixups.h` sign_pointer 按元数据
     sign(target, IA, div=0x2abe)（adiv=0 不混合）——与派发约定逐字
     一致 = **签的是对的**。
  3. 破坏者 = 墙 17 resign 补丁（3115d15，10-31 Linux 车道产物）：
     在机件反汇编 mac_policy_register 函数头 = `xpaci` strip →
     `mov x16,#0` → `pacia1716` **零修饰符重签全 335 槽**；运行时
     x8 高位 0x0febd5ff = sign(stripped target, IA, 0)。
  4. Apple 派发站 = `mov x17,#0x2abe; blraa x8,x17`（ABI 类型 disc，
     pc 0x0afcde0c 指令级核验）→ (IA,0) vs (IA,0x2abe) 必败 = 墙 35。
- **时序自洽**：补丁为 Linux clang（blraaz 零修饰符约定，墙 17）所写；
  Apple clang 发类型 disc；墙 30~33 Mac boot 崩点全在首个非空
  mpo_policy_init 派发之前（AMFI 段），reference 换件清场后 sandbox
  注册 = 第一次踩中。**同族清点**：kext-entry-donor-pptr 同形但实测
  自洽（AMFI kext start 已跑通）不动；sysctl-handler-donor-resign
  重签到运行时 blend 公式（其派发站同式）自洽不动。
- **★修法 = 补丁 host 条件化**（crypto-funcs-unpack v2 前例同款）：
  resign 仅在 `!defined(__apple_build_version__)` 编译——Linux 车道保
  零修饰符归一，Apple 车道保留 collection loader 元数据签名。补丁
  `git apply --check` 过（pristine feed）；主树 **`7061a05`**；scp Mac
  sha256 逐字节同（d1cdfa96…）。
- **出件（Mac 产线，抗休眠管线 `temp/refswap/w35-pipeline.sh`）**：Mac
  中途休眠断连一次（构建 ssh 死、树安全），看门狗 00:34 自动 nohup
  续跑；**kernel `9de04b76`**（sha 9de04b767fae8…，增量编译只重排
  mac_base.c）→ **payload `b63ad26e`**（96 MiB，已拉回存档
  `out/m2381/payload-w35fix-9de04b76.img` 逐字节对拍 ✓）。**★静态门
  `w35-gate.py`（指纹字级快扫，指纹自 0b607b2a 现场提取）**：resign
  循环指纹（xpaci+mov x16,#0+pacia1716）老件 1 / **新件 0**；派发对
  （movz x17,#0x2abe+blraa x8,x17）两侧同 371 = codegen 未动；
  **ESP 内嵌副本 `e6b37730` 同 PASS**。账本平：branches 42024 /
  kept 82701 / closure_unmapped 314（全持平会话四十三）= 零附带漂移。
- **判读预测（首刷）**：墙 35 消（sandbox mpo_policy_init (IA,0x2abe)
  验签过 = mac_policy_register 推进）；下一崩点预测 = sandbox
  policy_init 体内（Linux 时代墙 22 同段：IOService 调用族，reference
  换件后 kext-facing named 2047 应配）或预测单结构类 d=2 榜
  （`0xa7f3f6c` / `0xa841220`）。**新经验：工具链约定差可让"正确的
  旧修"变"新病灶"——resign-to-zero 类补丁在 Apple clang 车道全部存疑，
  已逐个清点。**
- **回退**：recovery_a ← `payload-refswap-0b607b2a.img`（abe3ec85，
  现役在机）。

## 10.Y.9 会话四十四首刷提取+核验档（2026-11-04，payload `b63ad26e`，屏摄设备仓
records/mackernel-w35fix-b63ad26e-firstboot-20261104-010612.jpg）

- **墙 35 修复真机生效**：屏上零 PAC failure，`mac_policy_register` 带着sandbox
  mpo_policy_init (IA,0x2abe) 验签过 = v2 条件化补丁按设计工作。
- **★boot 史诗级推进**：stage 轨迹首次全程点亮——`PE_lockdown_iokit →
  dtrace_early_init → trust_cache_init → OSKextRemoveKextBootstrap →
  machine_lockdown → kernel do post - done`（2s 内全过）= **整个
  kernel_bootstrap 尾段收官**；此前墙 32/33（AMFI 段）与墙 35（sandbox
  注册）全部成为历史。pages=0x125b06（恒）/ free=0x1251c6（vs 墙 35 boot
  0x1253c6 = −0x200 pages 消耗 = 更深）。
- **新墙 = 墙 36，回 unmapped-call 类**（reference 时代墙 32/33 同族，
  fixture 可修，非 PAC/工具链病）：
  `fileset: original component called an unmapped kernel function from
  0xfffffff00977dd00 (x0 0xfffffff8565e5500 x1 0xfffffff00b5a4158) hit #1`。
  - **crc=0xee584f8d 验真一次命中（139B）**——照片转录 x0/x1 各掉一个
    f，CRC 纠错定稿；x1 = 0xb5a4158 = 我方 native `__DATA`（donor kext
    拿到了指向我方内核数据的指针）；x0 = heap（0xfffffff8 前缀）。
  - **target = donor `0x0af07900` = `__ZN8OSObjectC2Ev`（OSObject 构造
    体）**：体 = `bti c; [x0+8]=1; [x0]=pacda(vtable 0x7d9e2d0, disc=
    this|0xcda1<<48); ret`（vptr+refcount 内联形态，忽略 x1）。** donor
    有两个基类构造分身：`0x0ae40a00` = OSMetaClassBase ctor（仅 vptr，
    vtable 0x7d62608）已配（pac-checked-shape）；`0x0af07900` = OSObject
    ctor（vtable 0x7d9e2d0 + refcount）漏配 = 墙 36**。
  - **native 侧两符号都独立存在**（Linux noctf nm：`__ZN8OSObjectC2Ev`
    @0x07a205a4 / `__ZN15OSMetaClassBaseC2Ev` @0x07a1d254）= **钉名即修
    类（与墙 34 同类），fixture 按名绑**。
  - **红利面 = donor 全镜像 188 直调 bl（首簇 0x841xxxx）+ 31 fixup 槽**
    （每个 kext 的 OSObject 子类构造链：caller 形态 = operator new → bl
    OSObject ctor → 覆写 vptr → instanceConstructed() → IOLock/IOMallocType
    族；caller fn bounds 0x0977dcb0..0x0977dd9c）——一针清一整类。
  - 调用时序自洽：bl 0x0af07900 设基类 vptr → 返回到 0x0977dd00 → caller
    覆写自己的 vptr（adrp 0x80d3dd0 同款 pacda 形态）= 标准 C++ 构造链。
- **判读已前置到修面（下轮 = 会话四十五施工）**：fixture
  donor `0xfffffe000af07900` ↔ native `__ZN8OSObjectC2Ev`（Mac fresh map
  拿 native 地址；标准 fixture 管线 temp/w32-fixture/verify.py 四门 +
  make_fixture），Mac 重建出件只刷 recovery_a。工具 = `temp/refswap/`
  （w35-gate.py 静态门可复用）。
- **回退**：recovery_a ← `payload-refswap-0b607b2a.img`（abe3ec85）；
  `payload-w32fix-4970bf21.img`（d7fd50cb）二级。

## 10.Y.10 会话四十五墙 36 fixture 施工档（2026-11-04，kernel `b9e733be`/
payload `ce3df35f` 待刷）

- **fixture 落位（钉名即修，与墙 34 同类）**：`0xfffffe000af07900` ↔
  `__ZN8OSObjectC2Ev`（设备仓 `6156dd4`）。native 地址 = Mac 树现役
  noctf（sha `68407e58`）符号表直读 = **`0xfffffff0079ef098`**（与
  refswap 账本已配的一参 ctor `0x…ef110` 相距 0x78 交叉吻合；Linux
  noctf = `0x07a205a4` 与 10.Y.9 案卷一致）。
- **四门验真全过**（`temp/w36-fixture/verify.py`，anchor_reviewed 原路
  径双车道）：JSON 条目在场 / donor 区间 `0x0af07900..0x0af0792c`
  size=44 sha 一致 / MAC 腿 → `0x79ef098` how=reviewed / LINUX 腿 →
  `0x07a205a4` how=reviewed；make_fixture check 254/254 全绿。
- **Mac 重建（w36-pipeline.sh，三红线输入）**：全量重编（noctf sha
  `68407e58`→`7307aecf`，6 条同名 reviewed 锚整体 −0x3c = native 重编
  漂移，语义同绑定；auto 通道随新 native 重推导净 −4，账本内自洽）。
  kernel `b9e733be`；map 9890 对，**墙 36 配对落位**
  `0x0af07900 → 0x79ef098 reviewed` ✓；`kext_targets_named`
  2047→2048（+1 = OSObject ctor 上名）。payload `ce3df35f`，存档
  `out/m2381/payload-w36fix-b9e733be.img` + kernel 同名存档。
- **静态门全过**：w35-gate 双件（kernel.macho + ESP 副本
  `8e95e957`，seal 步再加工属常态）= resign 指纹 0 / dispatch 对 371
  持平；emit 账本三分支持平（branches 42024 / kept 82701 /
  closure_unmapped 314）= 零附带漂移。
- **★站点重定向定谳（新件实证）**：墙 36 报点 `0x…977dd00` = LR（bl 在
  −4 = `0x…977dcfc`）。旧件该站 bl → 哨兵 `0x0b057dc8`（xpaci-LR 报
  caller 形态实锤）；新件同站 bl → **`0x0ae63098` = native
  `__ZN8OSObjectC2Ev` 终局地址**（noctf 原体 12 指令骨架归一化逐位
  对拍全同：pacibsp→bl 底座 ctor→vtable 0x7cef130→disc 0xcda1→
  pacda→str→refcount=1）。
- **红利面定谳 = donor 188 → 选中面 24 直调站**：donor 全镜像直调
  bl→`0x0af07900` 复核恰 188（榜：AppleH16CameraInterface 20 /
  AWCSKext 17 / IOACIPCFamily 13 / AOPTouchKext 11 / IOHIDFamily 11…）；
  最终 kernel 只含选中 entry 闭包，实换靶 24 站（哨兵站 1252→1228
  精确对账），其余 164 站在被 drop 的 267 个 m2381 无关 iPad entry 内
  = 口径自洽，非丢失。31 fixup 槽面随配对生效（map reviewed 落位）。
- **管线坑两条（已记）**：①`w35-pipeline.sh` 的 `TREE=~` 在本机展开成
  Fedora $HOME → 远端 cd 死，w36 版改 Mac 绝对路径；②查账本用
  `kernel.json` statistics（日志 grep 不出）。
- **回退**：recovery_a ← `payload-w35fix-9de04b76.img`（b63ad26e）；
  abe3ec85 二级；d7fd50cb 三级。

## 7j. 会话三十七执行档补 = 墙 29 定性（2026-11-03 凌晨，8a935155 首发屏摄 010532）

- **★墙类切换**：`8a935155`（sandbox 清仓件）不再报 unmapped-call——
  **stubbed 墙类被 251 笔 fixtures 压到换类**，新墙 = **PAC 鉴权失败**
  （`PAC failure from kernel with IA key while branching to x8`，
  `blraa x8` 打在签名指针上，esr 0x72000000 = EC 0x1C PAC_FAIL + IL，
  ESR_EC_PAC_FAIL=0x1C 常量已核）。
- 关键值（屏摄转录，**CRC 重建未完成，转录未验真**——消息 = sleh.c
  panic_with_thread_kernel_state 全寄存器转储，格式串已提取、tab/换行
  已按 emit 口径折算，仍差一位转录，用户拍板停手不追）：pc
  0xfffffff00ad39c64（我方内核 __TEXT_EXEC 内）；lr 0x5aef94700ad39c18
  （PAC 签名 LR，低位 = pc-0x4c 同函数内）；x8 = 0xc2b5b6b008f1e92c
  （鉴权失败目标，低位不落内核 text，疑 kext 区签名指针）；上下文
  x3/x9/x19/x24 = 0xc642xxx 簇（kext 数据区结构体）。
- **定性**：PAC 类 = 工具链税第二面向（我方内核无 FPAC 软检查块 13052
  处 + PAC 签名纪律与 donor 不匹配）→ **macOS 建核的对症面**（Apple
  clang 与 donor 同族 PAC 纪律）。不修，等云端构建（§9 修订版照旧）。
- 判读正面：sandbox 段启动走通到 kext 深处（unmapped-call 类在本组件
  绝迹），pages/free=0x125b8d/0x12534d 连续三轮缓步推进。回退 ←
  `0c6b8419`。下一墙处置归 §9.B（macOS 件 A/B 对照再判）。


## 7k. 会话三十八执行档 = 上游大同步 + Mac 建核转正 + 首刷（2026-11-03~04）

- **A. rebase 大同步**：315/315 笔重放到 upstream/main `bd3e947`
  （merge-base `359d77c` 核对）；冲突仅 3 处（AdrenoMetal gitlink 忠实
  落 7b885c2；payload.py 并集 check_data_input+prelinked_pmgr；
  baseline.sh ×2 hunk 取我方 XNU_KERNEL_CONFIGS 参数化；
  build-sheng-source-storage.py 双 fallback 通道并存）。XnuPrelink
  pin 已是 b1888c1（早前收编）；patch-kernel-fileset-cpusub.py 保留
  （上游 build-fileset-release-reference.py 是参照件构建器，不替代；
  payload.py 在调用）。验收 = check-m2381 4ok + Linux 全量构建出件
  `0967bf72`（上游新 check_prelink_regressions 回归门在位通过）。
  推送 = 主树 `13e9372`（forced）/设备仓 `b26168c`。
- **B. CI 车道死路定谳**：3 连 dispatch（41492845452/41495544378/
  41499247407）均排队 ~15min 零接机后 GitHub 自动取消（check-run
  注释 = "not acquired by Runner…capacity constraints"，macos-26
  arm64 池饱和；非配额非标签错——10-30 同标签 4 秒接机先例在）。
  结论：macos.yml = 冒烟车道，无人当量产（sheng/fuxi/nx729j 设备仓
  零内核 CI；上游全史 8 笔 6 取消）。**转本地 Mac 真机路线**。
- **C. M1 真机车道**（环境档案全录 §9.B）：ssh 打通（key-auth），
  两仓 + IPSW（sha `f7f55fa3` 金标对拍）+ Xcode 27.2 + 全依赖就位
  （llvm 436MB 走 Fedora ghcr 中转，brew 直连 350KB/s 不可用；
  verified-root-23F84 政策件拷组装树 build/ = fallback 第一分支）。
  坑三条：defconfig 必须显式 NAME；首跑 prelink vtable raise =
  半成品参照件中间态（完整重跑零矛盾，matcher 插桩验证后已还原）；
  matcher 确定性 = 重跑 macho sha/账本逐字一致。
- **D. 出件与 A/B**：kernel `56d24922`（89.9MB，Xcode 27.2，口径
  注记 26/27 差异用户拍板忽略）；A/B = missing 目标 344→190（−45%
  实锤），总目标 1019→920（−10%；CI 时代 −25% 预期被 Linux 侧
  fixture 251+清仓进步吃掉），stubbed 站点 2290→3116（脏口径：混入
  我方内核重编译调用点变化，弃用）。payload `a715395a`（红线全带，
  Linux 件备份 kernel.linux-0967bf72）。
- **E. 首刷**（用户自行刷入，判读留下会话 = §9）：PAC 墙再现但寄存器
  面全变（x16 非 x8，0x59df 签名态；stages 早期 1s mapping_adjust
  邻域；crc `0x71f484be`）。屏摄入设备仓 records/。pc/lr 粗查
  ±0x1000000 无命中，判读须正推 slide。
## 10.Y.12 会话四十六 = 墙 37 首刷判读 + 墙 38 施工档（2026-11-04，
payload `e6b06399` 首刷判读毕，屏摄设备仓
records/mackernel-w37fix-6d4a9f19-firstboot-20261104-023838.jpg，
records `4706e64`；全文原样移档 2026-11-04 会话四十七）

- **墙 36 + 墙 37 双墙真机通关（fixture 判读通过）**：旧报点
  `0x…977dd00`（墙 36，OSObject ctor 克隆）与 `0x…977fef8`
  （墙 37，timerEventSource）均未复现；stage 轨迹全亮收官
  （PE_lockdown_iokit → … → kernel do post - done）；pages=
  0x125b05 / free=0x1251c5（wired 2368 页 = 前沿深入新区域，非同
  点 +1 水位）；hit #1。
- **消息定稿**：crc `0x8a2bf81e` 一次精确命中（139B，**零纠正**；
  格式串 = `fileset-unmapped-call.patch` 的 panic
  `from %p (x0 0x%llx x1 0x%llx) hit #%u`，照片 x0 掉一个 f 由
  crc 命中裁定）。x0 = `0xfffffff2d947bb00`（正构造中的
  AppleARMCPU 堆对象）；x1 = `0xfffffff00b4afe88`（= AppleARMCPU
  gMetaClass，与 ctor 语义自洽）。
- **新墙 38 = 同族 unmapped-call**：报点 LR `0x…84ca3bc`（bl 站
  donor `0x84ca3b8`）；caller = **AppleARMCPU::AppleARMCPU()**
  构造器（donor `0x84ca388..`）；target = donor **`0x0af3432c` =
  `IOCPU::IOCPU()`**（更准确 = `__ZN5IOCPUC2EPK11OSMetaClass` 的
  meta 特化 ICF 克隆，meta 常量折叠后 12 条指令零调用）。定名
  三证：①调用方拼图：`mov w1,#0xf8`（instanceSize 0xf8）→
  `_OSObject_typed_operator_new` → 基类 ctor → 立即自写 vptr
  覆写 → instanceConstructed(`0xb4afe88`)；0xf8 与 AppleARMCPU
  metactr `mov w3,#0xf8` 精确对拼；②V1 = IOCPU vtable
  `0x7d6cde0`：getMetaClass 槽（vt+0x38 @`0x7d6ce18`）→
  `0xaf32df0` 返回 `0xb375cc8` = "IOCPU" metactr（`0xaf34258`）
  的 this；③克隆体形态 refcount=1 + vptr（pacda 0xcda1）+
  reserved 0x88=0，区别于墙 36 OSObject 克隆 `0x0af07900`（无
  str xzr、vtable `0x7d9e2d0`）。
- **修法 = fixture 重定向**（设备仓 `899bfae`）：↔ native
  `__ZN5IOCPUC2EPK11OSMetaClass` @ noctf `0xfffffff007b11c8c`；
  native 链 = IOCPU→IOService→IORegistryEntry→OSObject(C2 带
  meta)→OSMetaClassBase 五层，终态 {[x0]=vtable, [x0+8]=1,
  [x0+0x88]=0} 与克隆逐字段同（refcount 在 OSObject 层写 1，
  reserved 在 IOCPU 层清 0）；**x1 全链零使用**（调用方传
  AppleARMCPU meta 无副作用）；native 无 C1/C2Ev 重载（C1/C2 已
  ICF 合并，与 donor 克隆同源互证）；native vtable 值瞬态无害
  （调用方 ctor 即覆写）。fixture size=48 sha `59f22f9a…`，
  256/256 check 全稳；spec = `tools/xnu27/spec-session38.json`。
- **红利面（终数，A/B 双代理 + 账本三方一致）= 5 直调 bl + 1 fixup 槽
  = 6 站**：bl `0x84ca268/29c/3b8/424/47c` 全在 AppleARMCPU ctor 簇
  （C2/C1/alloc 路径×3）+ GOT 槽 `0x7e07bd8`；账本 key
  `0xfffffff00af3432c` sites=6 精确对拼（kext =
  com.apple.driver.AppleARMPlatform）。**克隆族全景（全 donor 结构
  扫描）**：48B 带 str-xzr 形态全镜像唯一 = 本条目，零兄弟；112 个
  44B 无-xzr C 族克隆散布全池（0x0af07900 = 已配 OSObject 成员）。
  邻近池 `0xaf32000..0xaf35000` 余 15 key/32 站未清（IOCPU canonical
  `0xaf3435c` 4 站、IOPlatformIO ctor 克隆 `0xaf329cc` 3 站、
  IODTPlatformExpert 克隆 `0xaf31f80` 5 站…）= 后续墙候选，本轮
  按需清仓纪律**不批量**。修正：IOPlatformIO（0x88）与 IOCPU（0xb8）
  同根（共同 super = OSObject 级 gMetaClass `0xb3771e8`）**非父子**；
  donor map 无 IOCPU/IOPlatformIO canonical 符号（匿名池块）；我们
  native 有 `__ZN12IOPlatformIOC2EPK11OSMetaClass`（Linux noctf
  `0x07b1135c`）= 后续池墙的现成靶。
- **Mac 重建 + 静态门**（增量 ~4min：kernel 3.5min + payload 34s；
  **native 未变零锚漂移**）：kernel `b6bfe74b`/payload `c4cb22fb`，
  存档 `out/m2381/payload-w38fix-b6bfe74b.img` + kernel 同名（本地
  已拉回同 sha）。map reviewed **255→256**（墙 38 配对落位 → noctf
  `0xfffffff007acbf10`）；**crash 站点 bl 字节实测** → `0x0af3ff14`
  = native 终局 `0xaf3ff10`（= noctf + `__TEXT_EXEC` 段移位
  0x3474000）**+4**——+4 = 跨过拷贝体 pacibsp 的 emit 既有约定，
  墙 36（bl→0x0ae6309c）与墙 37（bl→0x0af1f9e4）同款且**双前例
  真机通关实证**（档案所记 +0 系引 map 终局地址之误，本轮字节级
  复核三例约定一致）；GOT 槽 → `0x0af3ff10`（pacibsp 头）本体
  正确。w35-gate PASS（resign 指纹 0 / dispatch 对 371 持平）；
  statistics 全平（branches 42024 / kept 82701 / closure 311）=
  零附带漂移；make_fixture check 256/256。
- **回退**：recovery_a ← `payload-w37fix-6d4a9f19.img`
  （e6b06399，在机 = 已证通关墙 36+37）；ce3df35f 二级；
  b63ad26e 三级。


## ——— 2026-11-04 会话四十九 §0 现势指针（原样归档于会话五十 §0 首更） ———

## 0. 现势指针（每轮首更，2026-11-04 会话四十九）

- **现役件（待刷）**：`payload-w41fix-93eabd51.img`（payload sha8
  `b46e768a`，kernel `93eabd51`，**墙 41 双保险修复件**）。刷法 =
  recovery_a only。**回退链**：一级 = recovery_a ←
  `payload-w40fix-96422b08.img`（`a9bf924f`，已证通关墙 40、
  史最长 31s）；二级 `payload-w39fix-2f31a6bd.img`；三级
  e6b06399。
- **会话四十九已毕 = 墙 41 判读 + 双保险修复出件**：
  新墙 = **assert 硬陷阱**（esr `0xf200bffc` = brk #0xBFFC =
  `MACH_ASSERT_TRAP_CODE`；`machine_routines.c:3379 Assertion
  failed: _unsafe_kernel_text_initialized`），崩于 bsd_init
  IOKitInitializeTime 段（Time=31s = **史最长运行**，墙 40 一字
  补丁真机生效实锤）。**结构验证四重命中**（对已刷
  kernel-w40fix-96422b08.macho 实测）：①pc `0xb097854` = 断言
  thunk（bti c; adrp/add x8=__desc; brk #0xbffc）位 exact；
  ②x8 `0x7d6646c` = `ml_unsafe_kernel_text.__desc`（type=0 +
  lineno:24=0xd33=3379 + filename/expr 指向 native __TEXT 池
  `machine_routines.c`/`_unsafe_kernel_text_initialized` 逐字验
  证，chained-fixup 编码低24位=镜像偏移）；③lr `0xa9ae7f8` =
  `ml_unsafe_kernel_text`（composite `0xa9ae7c4`）自身 assert-bl
  的返回点；④标志字节 `0x7caf718/9` 文件里即 0。
  **机制 = iBoot DT intent 缺失**：`ml_unsafe_kernel_text_init`
  （STARTUP TUNABLES RANK_FIRST）只在 /chosen 有
  `kernel-ctrr-to-be-enabled` 时置位；我们 ADT（payload 侧
  devicetree.py 生成）没有该属性 → 标志恒 false → 首个调用者
  必炸。静态调用面 = 两个 `!ml_unsafe_kernel_text()` 包装
  （composite `0xa9de36c/388`）← dtrace 限制谓词族六站
  （dtrace_ioctl×2 / fbt_provide_module / sdt_init /
  systrace_init）；bsd_init 在 dtrace_postinit（:935）前跑
  provider init，与 31s 时序自洽；精确运行态入口屏上无栈不可证
  （s30 起帧链行走器已删），修法与调用方无关。
  **修 = 双保险**：①内核侧 BCM2837 门控 fallback（设备仓
  `50d67ba`，属性缺失时置 initialized+unsafe = SM8550 无 CTRR
  的真值；noctf 实测 init 两路径都存 1 到标志位）；②上游正路
  = profile.toml `["adt"."chosen"."u32"]` 加
  `kernel-ctrr-to-be-enabled = 0`（主树 `33f4580`；**ADT 实物
  字节验证** = 成品 payload 提取 kernel.macho.adt 13400B，属性
  name+len=4+value=0 与本地复现逐位一致）。门禁双件过
  （resign 0 / dispatch 371）；map 对账平（functions_matched
  9914 / reviewed 274）。
- **★判读工具账（本轮新增两条坑）**：①**屏地址归属必须先对
  `kernel-w4Xfix.macho` LC 段表判 native/donor**——「donor+slide
  低36位恒等」只对 composite 里的 donor 内容成立；composite
  native 区间（0x8334000..0xb0c0000 __TEXT_EXEC）与 donor 空间
  **不恒等**，kc.py fn 对 native 区间会误答 donor（本轮 namei
  红鲱鱼即此故）；noctf↔composite __DATA_CONST 实测平移
  +0xAE0000。②**断言类 panic 判读路径** = sleh 断言 thunk 表
  （bti/adrp/add/brk 0xbffc 模式）+ `__desc` 解码（type u8 +
  lineno:24 + filename* + expr*，指针槽 = chained-fixup 编码，
  低24位 = 镜像内偏移）+ panic-screen `put()` 语义（<32 字节全
  映射空格 → **PANIC_TEXT 无换行**，屏上折行 = 宽度折行，寄存器
  值可被硬切两行）。
- **开放问题：CRC 辅证连续第二例 MISS**：本次按 sleh.c:647 格式
  精确重建 PANIC_TEXT（单点替换/删除/插入/换位 + 双点全扫 +
  x16 专项，x16 转录 17 位超宽必错也不中）——原因未明（疑
  mini-printf 的 va_list 槽位/展开模型细节差）。结构验证不受影
  响；下轮屏摄若再现 MISS，把 verify-panic-crc.py 扩一个
  assert-格式配方并做 3 行定宽扫描。
- **时间线红利**：30s `IOService::waitForService("IORTC")` 超时
  完整呈现（stage 卡 29s + idle=29s 谜底）——**每开机 +30s 固定
  税**，IORTC provider（PM8550 RTC 驱动）= 驱动阶段任务挂账；
  修复后 boot 预期 ~31s 起推进 vfsinit/native probe 段。
- **下轮工作单 = 会话五十**：①首刷判读（recovery_a only）：
  墙 41 消 = 不再报 machine_routines.c:3379；预期推进 bsd_init
  后段（vfsinit → native apfs/graphics/iogpu/adreno/smmu/
  constants probes → dtrace/provider init 段直过）→ 新墙照旧
  分流（定名三证 → spec → make_fixture → scp Mac → 一次重建
  一刷）。②ApplePMGR 21 靶 85 站三路扇出批量（清单在档 s48
  §0④；方法卡 = fn/slots/KDK 三证不硬凑；make_fixture 带
  `PYTHONPATH=UniXNU-m2381/kits/XnuPrelink`）。③0xaedab0c
  last-CPU 屏障定名（10 站清单在档；输入 = 新墙 x0/x1 + 调用
  方上下文）。④红利对账 `blscan-donor.py` + CRC-MISS 二例取证。
  ⑤清理项（不急）：内核 fallback 与 ADT 属性双保险已过墙后，
  可评估把 fallback 补丁撤出 series（上游正路已通时的纯化）。
- **未推**：主树 `9678be3`+`6f5794c`+`783270f`+`8b09187`+
  `33f4580`（本节 docs + profile）+ 本节；设备仓
  `155d3c3`…`1b9921f` + `2d168e8` + `7742803`/`cc7c062` +
  `50d67ba`（墙 41 补丁）。Mac 树 = scp 新态（kit/补丁/profile
  三类均已同步，出件双存档 Mac `out/m2381/payload-w41fix-*`）。
- **★文件纪律（2026-11-04 用户拍板：重要文件入项目，不进 temp
  散放）**：spec 证据 = `UniXNU-m2381/kits/XnuPrelink/fixtures/
  specs/`；门禁/判读/清点工具 = `UniXNU-m2381/scripts/{emit-
  static-gate,verify-panic-crc,blscan-donor}.py`（本轮 gate 双
  件校准含 w41 = resign 0 / dispatch 371）；
  `tools/xnu27` 只留通用执行器。temp/ >10MB 隔轮清照旧。
- **车道**：Mac 主攻（ssh `naoniudeair@192.168.10.246`，工作面
  `~/unixnu/`，主树 = `~/unixnu/ios-m2381-unixnu`；reference
  已接线 sha `5a794da8`；payload 三红线输入常驻 =
  `~/unixnu/original-trust-23F84.bundle` +
  `~/unixnu/scratch-data.img` + `--unique-chip-id
  3645ad55d47e77c1`；kernel 步自动按 patch-sha 签名重暂存
  （patches.py prepare 签名对比），**soc 补丁变更 = scp 后直接
  kernel 步即可**；profile 变更 = scp 主树 profile.toml 后直接
  payload 步）；Linux = 参照件供给 + 回归门 + 离线重放 +
  ADT 本地复现验证（devicetree.generate 确定性可用）。
- **文档卫生常规（2026-11-03 用户拍板，每轮执行）**：①本指针每轮首更
  ②已执行工作单段落移 `docs/archive/STATE-history.md`（正文只留指针）
  ③未推清单与回退链核对 ④`temp/` 内 >10MB 二进制隔轮即清
  ⑤记忆索引超 24KB 即剪钩子。**红线：docs/ 的上游文档站树
  （concepts/guide/zh/reference/public）不清理不改动。**
- **参考链状态（2026-11-04 会话四十七刷新）**：donor KC sha ✓；
  fixtures 274/274 字节验真 ✓；reference 换件生效（Mac map
  reference sha `5a794da8` ✓）；24A446 policy 休眠 + KC 护栏 ✓。


## ——— 2026-11-04 会话五十（晨）§0 现势指针（判读定谳版，施工出件版归档于本日收档） ———

## 0. 现势指针（每轮首更，2026-11-04 会话五十）

- **现役件（在机）**：`payload-w41fix-93eabd51.img`（payload sha8
  `b46e768a`，kernel `93eabd51`）已刷 recovery_a，本轮首屏判读毕（见下）。
  **回退链不变**：一级 = recovery_a ← `payload-w40fix-96422b08.img`
  （`a9bf924f`，已证通关墙 40）；二级 `payload-w39fix-2f31a6bd.img`；
  三级 e6b06399。
- **★会话五十首屏判读 = 墙 42 定谳（AMFI "No PMGR?"），墙 41 截胡未受验**：
  屏面 = 无 esr / 无 brk 0xBFFC / 无 machine_routines.c:3379（墙 41 病灶
  未再现），白区 panic 文本 = `"AMFI: No PMGR?\n"
  @ConfigurationSettings.cpp:388`（donor fmt 串 `"MSG" @%s:%d` 渲染结果，
  直接 `bl _panic` = 硬 panic，非 fault 无 esr/寄存器行）。stage 冻在
  IOKitInitializeTime +29s / time=31s = panic 时刻；stage 列表语义 =
  顶条当前 + 其余已完成段按源序（bsd_init_kprintf 探针：process_name:544
  →get_bsdthread_info:571→chgproccnt:665→kmem_suballoc:672→
  bsd_bufferinit:688→IOKitInitializeTime:694→vfsinit:698）。**vfsinit
  未现 + 税未到点（+30s≈t32s）双信号 = 墙 41 的 dtrace 谓词路径本轮未
  执行**——w41 补丁在件（BCM2837 fallback + ADT 属性），下刷作数。
- **墙 42 anatomy（donor KC 全链实证）**：
  - panic 串唯一 `0xfffffe00075572ee` = `"AMFI: No PMGR?\n" @%s:%d`；
  - panic thunk `0xfffffe0008f2d318`：`cbz x0→panic`（x0 = 所查服务），
    非 NULL 路径 = x1 vtable slot+0x28（autda 0xcda1 / blraa 0x3a87
    派发）latch；
  - 热路径 = **AMFI developer-mode 配置函数 `0xfffffe0008f0d594`**（984B，
    入口 pacibsp）：①查 `/arm-io/soc-misc`（AppleSoCMisc metaclass
    `[0x7fa97c8]`，瞬时查找 0xaeb4b88 未定名）→ 无 →②fallback
    `serviceMatching("ApplePMGRNub")`（0xaedd3b4）+ **`waitForMatchingService`
    (0xaedd148, x21 = 30.0s 整 = 0x6FC23AC00)** → 超时 NULL →③
    safeMetaCast 败 →④`0xf0d960` 尾调用 thunk（`0xf0d968 bl 0xf2d318`，
    该 bl 后无续指令）→ panic 388。同函数另有 BPR query/set 失败 panic
    @367/372（`"AMFI: Unable to query/set BPR via AppleSocMisc: 0x%x"`，
    本轮未走到）；成功路径 = IOLog `"AMFI: [ApplePMGRNub] latched BPR"`
    + `_disable_developer_mode`（0xad16970）分支 = AMFI dev-mode 状态机；
  - 时序：AMFI 配置 fn 在 kext start（bsd_init 探针段之前）起表，
    t≈1s 起等 30s → t≈31s 炸；IORTC 30s 税（t≈2s 入）并行未到点。
    **墙 40/41 两轮 "史最长 31s" 解扣 = 双 ~30s 定时器到点巧合对齐**；
  - 根因 = SM8550 无 Apple PMGR/SocMisc 硬件，IOKit matching 从未跑
    （屏 probe=0 match=0 published=0）→ 服务永缺 → 30s 等待必超时必炸。
    donor ApplePMGR 驱动本体在 KC（0x898xxxx 段区，串 0x73a61d4）但永
    不到 probe；
  - 后患清单 = ConfigurationSettings.cpp assert-thunk 簇（0xf2d2xx..
    0xf2d3xx 冷区，brk#1/bti c 填充 + mini panic 函数，行 34/575/580/
    625/1150/1155/1482）——同文件未来墙照此解剖即快；
  - 工具坑：一次性 bl 扫描脚本漏 `0xf0d968` 直接 bl（该 thunk 实有直接
    调用方），**capstone 全反汇编为准**；AMFI 调用面大量 PAC 化间接派发。
- **判读辅证**：crc=0x9ad2304 = **CRC-MISS 第三例样本在档**；本屏
  PANIC_TEXT 全文已由 donor fmt 实证 → `verify-panic-crc.py` 可扩
  assert-格式配方（`"MSG" @%s:%d` 两参数）。水位 pages=0x125b08 /
  free=0x1246c8 在档（结构对比照旧）。
- **下轮工作单 = 会话五十一（待拍板，未动工）**：
  1. **墙 42 修复，推荐 B 案**：
     - **B（推荐）= fixture 整函数重定向**：donor `0xf0d594`（984B）→
       native ~30 行 benign shim（置位 checked 旗标 + log 一次 + 正常
       返回；语义 = dev-mode 缺席路径），s24 SecureDTGetProperty（9 站
       ~30 行 shim）同款先例；**一次盖掉本函数全部 4 处 panic**
       （367/372/388 + helper）。spec 入
       `UniXNU-m2381/kits/XnuPrelink/fixtures/specs/`；make_fixture 带
       `PYTHONPATH=UniXNU-m2381/kits/XnuPrelink`；静态门
       （emit-static-gate）→ Mac scp 同步 → 一重建一刷。
     - A（弃，已证不可行）= thunk 入口→ret 一词补丁：`0xf0d968` 为尾
       调用（bl 后无续指令），ret 即踩进下一函数 pacibsp，PAC 帧错位。
     - C（远期挂账）= 真供给 "ApplePMGRNub" 服务：依赖 matching 基建
       （probe=0 之根），驱动阶段一并考虑。
  2. **下一刷同时受验墙 41+42**：只刷 recovery_a（在机 = w41fix
     `b46e768a`，iOS 链未被 nura 覆盖，快捷卡合法）；预期 = AMFI 静默
     后 t≈32s IORTC 税到点推进 vfsinit → native apfs/graphics/iogpu/
     adreno/smmu/constants probes → dtrace/provider 段（= 墙 41 站点）
     → 新墙照旧分流（定名三证 → spec → make_fixture → 一重建一刷）。
  3. PMGR 21 靶 85 站三路扇出照旧（墙 42 佐证优先级）；0xaedab0c 定名
     照旧；CRC assert-配方扩充（第三例样本）。
  4. 清理项（不急）：同前（内核 fallback 撤出 series 评估等过墙后）。
- **未推**（自上轮未变，本轮仅 docs/记忆）：主树 `9678be3`+`6f5794c`+
  `783270f`+`8b09187`+`33f4580`；设备仓 `155d3c3`…`1b9921f` + `2d168e8` +
  `7742803`/`cc7c062` + `50d67ba`；本节 STATE 变更未提交。Mac 树 =
  scp 新态（kit/补丁/profile 三类均已同步，出件双存档 Mac
  `out/m2381/payload-w41fix-*`）。
- **车道**（不变）：Mac 主攻（ssh `naoniudeair@192.168.10.246`，工作面
  `~/unixnu/`，主树 `~/unixnu/ios-m2381-unixnu`，reference sha
  `5a794da8`；payload 三红线输入常驻；soc 补丁变更 = scp 后直接
  kernel 步）；Linux = 参照件 + 回归门 + ADT 本地复现。
- **文档卫生/参考链**：§0 首更 ②旧段落归档 archive ③未推核对 ④temp
  >10MB 隔轮清 ⑤记忆超 24KB 剪钩子，照旧；donor KC sha ✓ fixtures
  274/274 ✓ reference `5a794da8` ✓。

## 会话五十三 = r52 首刷判读 + 对齐收官 + r53/r54 双出件（2026-11-05，执行完移档）

- **墙 53 定谳**：r52 首刷 t=1s 死 = fileset unmapped-call 哨兵墙首站（runbook 2d 分流，非 AMFI）。donor com.apple.kec.corecrypto kext exec（0xac72428）首调 `kprintf("corecrypto_kext_start (tracing disabled)\n")` → auth-GOT 槽 0x84ac948（rebase 压缩指针无符号名）matcher 未配 → 出件槽重绑镜像内 `_unixnu_fileset_unmapped`（0xb5a9c4c）→ braa 尾跳 LR=0xac72460 = 屏值。桩语义 = 报返回地址（bl 在 −4）+ x0/x1 原参。
- **架构定谳（修正）**：donor kernel entry 在混合件里被 native 整体替换（native_base 0x7184000）；kept kext = identity。com.apple.kernel entry exec = `__TEXT_EXEC` 尾段 [0xacb4000,0xb5be908]（LC_FUNCTION_STARTS 21792 函数，`Image(data,at=0x8000)`）。auth-GOT 压缩指针 = `PAC16<<48|(VA−0x7004000)`；槽 0 = boot bind（按名绑）。
- **r53 修法两笔**：kprintf reviewed record（144 条=117+主树 26 合并+kprintf→`_kprintf`，66 槽全修，stubbed 1262→1236/missing 320→179）+ TWP NOP `0xfffffe000ac726e8 0x94003CE6→0xD503201F`（kpis-publish bl；接收器 0xb42b4a0 被 native 替换、0x7ec5178 混合件零读者、返回值忽略）。payload-r53-47859083（sha 47859083b27028d7，内嵌 kernel ab06853b）留档不刷。
- **对齐收官（用户拍板"先对齐"）**：上游 24A446 同源 xnu-12377.121.6（Darwin 26 源码未发布）+ os series 逐字同 + kit 同 commit；真缺口 = Mac 从未建 RELEASE fileset reference（session 43 同坑，23F84 参照件系 Linux 建）。修通链：`XNU_KERNEL_CONFIGS=RELEASE ./unixnu kernel --skip-prelink`（builder 携 defconfig 的 `CONFIG_XNU_KERNEL_CHAINED_FIXUPS=y`，绕过 builder 手搓 = 丢环境 → MakeInc.def else 分支要 ld-classic → Xcode 27.2 无的假象）+ Mac 补 `CROSS_CLANG`/`DARWIN_TRIPLE`（UBSan stub 块 linux_env 专属）+ stub 三缺一 negate_overflow（nm 扫组件档实测）补进 baseline.sh + reference 17.2MB 手动 stage `tree/build/xnu/fileset-reference/`。
- **r54 emit 基线（reference 元素首秀）**：reference-symbol 1489 + reference-order 4334，functions 8215→9468，stubbed 1262→1024，missing 320→286；regression policy 按新面重生成（IOAVFamily 0xb92ab20 = 旧面疑似误配改如实缺失）。payload-r54-df033828（内嵌 kernel 2221b0d2，NOP+kprintf 槽复核存活）待刷。
- **提交**：设备仓 fb8af03、kit a028840、主树 41d1d0a+93f5903+9e0225f（均未推）。判读工具 temp/s53-*.py + temp/s53-noctf-syms.json 可复用。

# STATE — 现势状态（m2381，2026-10-28 更新）
## 0. 现势指针（每轮首更，2026-11-05 会话五十五续·墙 56 定谳 + 手术 r57 出件待刷版）

- **★会话五十五续 = 墙 56 判读定谳（_IOMalloc 110 槽导入面）+ r57 手术出件待刷（2026-11-05）**：
  **r56 首刷**：fips 门过了（prng_random panic 消失，toml 手术生效），
  推进到 donor corecrypto kext start 的 PRNG init,新哨兵墙 =
  `fileset: original component called an unmapped kernel function from
  0xfffffff00ac20904 (x0 0x518 x1 0xfffffff007dcc9c8) hit #1`,crc
  `0xad79e802` 零纠正命中。**定谳 = donor `_IOMalloc` 未绑**：站
  0xac20900 → bl thunk 0xac818c0 → 槽 `0x84ac908` → donor
  `0xfffffe000b441a24`（= s54"留桩三件"之首,其"1参 kalloc_data 无安全
  孪生"判断错误）。四证定名：①callee 体 = `kalloc_ext(KHEAP_DEFAULT,
  size, Z_VM_TAG_BT(Z_WAITOK, VM_KERN_MEMORY_IOKIT), NULL)` + `ldadd`
  大小记账（flags 0x301000 逐位解出,VM_KERN_MEMORY_IOKIT=13）;②KDK
  26.5.2 `_IOMalloc`（0x7ab1e34）逐指令同形;③x0=0x518 = PRNG 上下文
  尺寸;④size 区别于 IOMallocZero(Z_ZERO +0x4)。**影响面 = 110 槽**
  （92 原样未动 + 18 已绑哨兵桩,corecrypto 站仅首发）。**无原生孪生**
  （在树调用点走 IOMalloc 宏内联;`kalloc_external` 仅 MacOSX;
  `kernel_kalloc` 已从 12377 删除）→ **s49 shim 先例**：源码
  `_unixnu_iomalloc_shim`（kalloc_ext(KHEAP_DEFAULT, size, 0x301000,
  NULL)）+ Mach.exports + series 新增
  `fileset-iomalloc-shim.patch`（三 feed 干跑全过）;账本 reviewed
  record 144→145（体 76B,sha af74cbaa…）。**r57 = payload 手术**（免
  Mac）：28B shim 写入 native 文本页 padding `0xaef0800`（7 指令:
  mov x1,x0 / adrp+add KHEAP_DEFAULT 0x7dec3e0 / w2=0x301000 / x3=0 /
  b `0xace84d4` = native kalloc_ext,从 _IOMallocTypeVarImpl 尾调实
  证）,110 槽全部重绑（pac 0x8011|0x3eec800）,native seal HMAC 重算
  （0x2856e20a…,只盖 __text;donor POST 只盖 corecrypto kext 段 =
  未触）。全图 diff = 495B/112 簇（110×4 + 28 + 32）,零杂散。**r57
  出件**：`out/m2381/payload-r57-104bff44.img`（99 MiB,内核
  `20b4729f`,unixnu.toml 与 r56 逐字同）。**预期推进面**：PRNG init
  过分配 → POST 完整跑（HMAC + KAT）→ PRNG 登记 → PE_init_iokit →
  AMFI（站表 0x924e77c/ef1c/ef5c shim 预案）。旗标不变：vfsinit →
  root/cryptex mount → SHLOG state=2。回退：recovery_a ← r56
  `94a096c3` / r55 `dd51ff60`。**工具坑三连（下次拆站先读）**：kc.py
  DON 仍钉 23F84（wall 命令读字不可信,24A446 用
  `out/ipsw/24A446/kernelcache.decompressed`）;ours↔donor slide =
  `0x1f000000000`（README 少一个零）;低 36 位换算 = `VA & 0xFFFFFFFF`
  取低 32 位（0xfffffff0 前缀下 36 位手算易错）。capstone 装在 user
  site（python3 不带 -I 跑）。
- **★下轮工作单 = 会话五十六 r57 首刷判读（runbook）**：
  0. 收屏（屏态非判决）；CRC 对拍 = plain zlib.crc32 整串（哨兵含
  `hit #N`,语义 panic 含 `@file:line`）。
  1. 判读主线：
  a. **同站 0xac20904 仍哨兵** → shim 未接管：读回槽 0x84ac908 是否
  `0x8011000003eec800`,再查 braa 鉴权失败形（PAC 类 ESR）→ 编码/区域
  复核;若 x0/x1 变了 = 已过分配、新站 → 按站分流。
  b. **数据 abort 形 panic** → 先复核 shim 两常量:kalloc_ext =
  0xace84d4（_IOMallocTypeVarImpl 尾调）、KHEAP_DEFAULT view =
  0x7dec3e0（_IOMallocAligned 包装 adrp）,与封印件再交叉。
  c. **预期推进面**：PRNG init 过分配 → **fipspost_post 完整 POST 首
  实测**（HMAC 段 diff=0 应过;KAT 面 = s54 挂账未知墙,踩到 → word 级
  thunk 扫描（勿线扫）→ 槽 → donor 坐标 → 三证定名 → 哨兵流程）→
  PRNG 登记（register_and_init_prng / cc_kext_random_init）→
  PE_init_iokit → AMFI（t≈32s IORTC 税）→ No-PMGR → PENDING 站表
  0x924e77c / 0x924ef1c / 0x924ef5c → shim 三条目 → r58。
  d. `FIPS Kernel POST Failed` = donor HMAC 不匹配（不该发生：哈希段
  diff=0;若现 = kext 段被污染,查 cpusub/probe patcher 是否越界写）。
  2. 成功旗标：vfsinit → root/cryptex mount → SHLOG state=2（reader
  cargo,read-m2381-ufs-log.py 只信 slots_written 以内）。
  3. 回退：recovery_a ← r56 `94a096c3`（一级）/ r55 `dd51ff60`（二级）。
  4. 拆站工具坑（本轮实证,先读再动）：kc.py DON 钉 23F84（wall/fn 对
  24A446 不可信;donor 用 `out/ipsw/24A446/kernelcache.decompressed`,
  实机字节用 payload 内封印件）;ours↔donor slide = `0x1f000000000`;
  槽低 32 位 = `VA & 0xFFFFFFFF`;capstone 在 user site（python3 不带
  -I）;账本权威 = 设备仓 kits/（主树 kits/ 为过期子模块勿改）。
  5. 若 r57 过 PRNG：下轮主菜 = AMFI No-PMGR 站表三条目；Mac 重建
  （fileset-iomalloc-shim.patch 已入 series + 账本 149 在案）任一轮可
  接管手术件,r57 手术即退役。


- **★会话五十五 = 墙 55 定谳（自设门，非 donor 墙）+ fips_mode 撤除 + 外科手术 r56 出件待刷（2026-11-05）**：
  **r55 首刷判读**：t=1s，stage 至 mapping_adjust 后 panic =
  `source corecrypto verification requires complete default POST
  @prng_random.c:665`，crc `0x27f71c47` 零纠正命中（plain
  crc32("message @file:line") 实证）。**病灶 = 我们自己的
  `source-corecrypto14-bootstrap.patch` 门**（STARTUP SYSCTL
  RANK_LAST，在 provider start 之前）：`PE_parse_boot_argn("fips_mode")
  && mode != 0 → panic`——s54 加的 `fips_mode=4`（FIPS_MODE_FLAG_DISABLE）
  正是该门禁止项（系列契约 = no skipped POST；PRNG 上下文登记依赖 POST
  完成态）。**r55 比 r54 死得更早**：fipspost_post 根本没跑，s54 四槽
  重绑（0x84ac980→0xaefbfdc 等）仍未上机验证。修法 = **board.toml 撤
  `fips_mode=4`**（设备仓，连同其"kpis bl retarget 在哈希段内 HMAC 永
  不匹配"的过时注释一并改写：emitter 换道后 TEXT_WORD_PATCHES 已清空、
  FIPS 哈希段 [0xac12c00,0xac818b0) diff=0，完整 HMAC 可过；DISABLE 只
  是防 KAT 未知的保险，被门判非法）。
  **r56 出件遇硬阻**：Linux out 树 kernel = r52 旧件 cfbe9b25；s54 两
  次 scp（temp/s54/kernel.macho=`3d158c4d`、kernel-r55.macho=`e64448dd`）
  走本地 payload 步骤封印后 HMAC 均为 `61f9ca2a…` ≠ r55 实际封印件
  `b2f48b39…`，且 0x3eeb64c..0x3eebfdb ~1.4KB 指令级差异（编译级）——
  **r55 的父内核 X = 未回传第三状态**（Mac payload 出件时间早于两次
  scp，X 仅存于 Mac out 树）。**外科手术定案**：对
  `payload-r55-dd51ff60.img` 全图 FAT 手术，仅重写 unixnu.toml 活动簇
  （dir entry 簇 94598 = 0x5D29800，` fips_mode=4` 移除 562→550B + 尺寸
  字段）——全图 diff 166B 全部落在 toml 簇 + size 字段内，**内核
  （封印件 787778cb）、ADT、Data seed、HMAC 报告与实机在跑 r55 逐字
  节相同**。四门验证（mtools 读回 / 7z 逐件 / 全图 diff 定位 / sha）。
  **r56 出件**：`out/m2381/payload-r56-94a096c3.img`（99 MiB，sha256
  `94a096c3f52bf242…40e1cc7`；payload.json 已写手术 provenance）。board
  已改，**后续正式出件必须重走 Mac（rsync 后本地树重出）或 Mac scp 新
  kernel**。屏幕显 args 与 loader 实际一致（内核内嵌 args 串随封印件保
  留 r55 原文 = 判读时屏显即实证）。**预期推进面**：门过 → donor
  corecrypto kext start → **fipspost_post 完整 POST 首次实测**
  （mach_absolute_time 槽 + HMAC 段 diff=0 应过；KAT 面 = s54 挂账未知
  墙，踩到按哨兵流程定名）→ PRNG 登记 → PE_init_iokit → AMFI（站表
  0x924e77c/ef1c/ef5c shim 预案在案）。成功旗标不变：vfsinit →
  root/cryptex mount → SHLOG state=2。回退：recovery_a ← r55
  `dd51ff60`（当轮在跑件）/ r54 `df033828` 二级。


- **★会话五十四 = 墙 54 判读定谳 + 四 records + r55 出件待刷（2026-11-05）**：
  **r54 首刷**（crc `0x196bb370` 零纠正一次命中 len=124B，t=1s，stage 到
  mapping adjust）= from `0xac1536c` → bl `0xac15368` → thunk `0xac819b0` →
  auth-GOT 槽 `0x84ac980`（rebase `0x8011|0x3ecaf24`）→ donor **`0xaecef24`
  = `_mach_absolute_time`**；caller `0xac15330` = corecrypto kext
  **`fipspost_post(fips_mode=0, mach_header=ki->address)`**（x1 =
  `0x7dc74e0` = corecrypto entry 基址；`static corecrypto_kext_pmach_header`
  记忆化体与 fipspost_post.c:106 逐句同源）——首调 =
  `cc_absolute_time()`（cc_absolute_time.h CC_KERNEL 分支）。
  **方法修正**：s53 "余 6 桩零调用点"系线性反汇编数据池失步漏扫（thunk 簇
  0xac818b0..0xac81a80 在线扫失步区）；word 级重扫 = 7 桩目标 bl 站
  **125/1/1/1/1/1/0**。**四 reviewed records（144→148，设备仓 62935f9）**：
  ①`0xaecef24→_mach_absolute_time`（125 站 = 最大剩余 import 面）；②
  `0xad43134→_lck_mtx_alloc_init`（槽夹逼 lck_grp_alloc_init/mtx_assert +
  KT_LCK_MTX 分配体 + caller pacda 签回锁指针）；③`0xad43db4→
  _IOLockTryLock`（槽夹逼 IOLockLock/IOLockUnlock + casa-percpu 体形）；
  ④`0xb42b4a0→_register_crypto_functions`（register_crypto.h:284，**翻案
  s53 "无孪生"**——源码级孪生在，noctf 实证）。**撤 wall-53 kpis-publish
  NOP**（TEXT_WORD_PATCHES 清空）：实测 emitter 机制 = bl 保持指 donor
  thunk、槽重绑 native → **FIPS 哈希段 [0xac12c00,0xac818b0) 文本 diff = 0
  （donor 逐位还原）**。**board.toml kernel_args + `fips_mode=4`**
  （FIPS_MODE_FLAG_DISABLE）：完整 HMAC 虽已可过，POST KAT 面仍有未知墙，
  DISABLE 让 fipspost_post 过 mach_absolute_time 后立即 CCERR_OK
  （PE_parse_boot_argn r53 已绑可解析）。**留桩 3**（故意）：
  0xb441a24（1 参 kalloc_data 形态 flags 0x31000 烘死，我们 2 参无安全
  孪生）/ 0xae9bd20（零直调）/ 0xb441b54（死 thunk）。
  **r55 出件**：`out/m2381/payload-r55-dd51ff60.img`（payload `dd51ff60` /
  kernel `e64448dd`，99 MiB，payload.json kernel_args 含 fips_mode=4 实
  证）；stubbed 1024→**1002**、corecrypto 7→**3**；四槽重绑字节级实证
  （0x84ac980→0xaefbfdc / 958→0xaceffe8 / 970→0xacf0368 / 9e8→0xb3fdb18，
  verify-r55.py 四门）；Mac 全链（defconfig→tree→kernel→payload）BUILD
  EXIT=0（~/unixnu/s54-build.log）。
- **★下轮工作单 = 会话五十五 r55 首刷判读（runbook）**：0. 收屏（屏态永
  非判决）。1. 判读主线：a. unmapped-call hit → `verify-panic-crc.py` 对
  拍 → from−4 = bl 站 → 对号 kernel.json（stubbed 1002 面，donor 坐标）
  → 三证定名（word 级 thunk 扫描法，勿线扫）→ 通道。b. **预期推进面**：
  fipspost DISABLE 过 → corecrypto_kext_start 尾段（kpis publish 已通/
  sysctl_register_oid 31+13 面/register_and_init_prng 已配/cc_kext_random
  _init）→ 新墙按站分流。c. FIPS POST Failed（不该发生：fips_mode=4 在；
  若现 = boot-arg 解析问题，查 ADT chosen 传递）。d. AMFI No-PMGR（t≈32s
  IORTC 税面）→ PENDING 站表 0x924e77c/0x924ef1c/0x924ef5c → shim 三条目
  → r56。2. 成功旗标不变：vfsinit → root/cryptex mount → SHLOG state=2。
  3. 回退：recovery_a ← r54 `df033828`；w42fix `3fe101ba` 二级。
- **★会话五十三对齐 = payload-r54 出件（2026-11-05，已刷已判 = 墙 54）**：
  `out/m2381/payload-r54-df033828.img`（99 MiB，sha256 `df033828fd9fec1f…`
  双端同；内嵌 kernel sha `2221b0d2…`）。本轮 = "跟着上游对齐"落地：
  上游 24A446 = **同源 xnu-12377.121.6**（Darwin 26 源码未发布，谁都无法
  源码换代）+ series 逐字同 + kit 同 commit b1888c1；**真缺口 = Mac 从未
  构建 RELEASE fileset reference**（26.5 车道 session 43 同坑重演，参照件
  一直是 Linux 建的）。修复链：①正确通道 = `XNU_KERNEL_CONFIGS=RELEASE
  ./unixnu kernel --skip-prelink`（builder 带 defconfig 环境，我两次绕过
  builder 手搓 = 丢 `XNU_KERNEL_CHAINED_FIXUPS`=1 → ld-classic 缺失假象）
  ②baseline UBSan stub 缺 `__ubsan_handle_negate_overflow_minimal`（nm 扫
  组件档实测三缺一，已补）③CROSS_CLANG/DARWIN_TRIPLE 两个 Linux 专属变量
  Mac 上需显式补（homebrew llvm + arm64e-apple-ios27.0）。
  **r54 emit 基线（reference 元素首秀）**：reference-symbol 1489 +
  reference-order 4334 anchors，functions 8215→**9468**，stubbed
  1262→**1024**，missing 320→**286**；regression policy 已按新观测面重生成
  （missing 149/stubbed 1024 sites，IOAVFamily 0xb92ab20 = reference 重配
  后如实缺失，旧面疑似误配）。字节复核：NOP 0xac726e8 在位 + kprintf 槽
  0x84ac948→0xb514cb8（reviewed 绑定在 reference 重配下幸存）。提交：主
  树 baseline+policy+STATE，设备仓 kit 本轮未动（fb8af03 仍为头）。
- **r53（无 reference 过渡件）**：payload-r53-47859083 已出已验证，被
  r54 取代，留档不刷。
- **会话五十三判读档（墙 53，已执行移档）**：r52 首刷 = corecrypto kprintf
  槽 0x84ac948 漏配全案 + kpis-publish NOP（本轮已翻案撤销）→ 全文在
  `docs/archive/STATE-history.md`（2026-11-05 会话五十四移档）与记忆
  `s53-r52-firstflash-unmapped-wall`。
- **换道出件完成（2026-11-05，会话五十二）**：os_profile →
  `ipados-27.0.1-24A446` 全链走通，**payload-r52 已出待刷**：
  `out/m2381/payload-r52-773d5578.img`（payload sha8 `773d5578`，kernel
  `cfbe9b25`，99 MiB ESP，Mac 出件后 Linux 拉回 sha 逐位同）；
  check-m2381 **4 ok 0 fail**；emit 报告 missing 320 / stubbed 1262 =
  政策基线吻合，adapter branches 0。首刷判读 = **会话五十三**（recovery_a
  only + `set_active b`；回退一级 = `payload-w42fix-3fe101ba.img`
  `6436442d`）。
- **★对账重大修正（板级层）**：上游实证 24A446 车道 = **arch series +
  UniXNU-sheng 板级 kernel patches（10 条）+ 板级 package capture 面**——
  侦察档"arch 68 条全有/零 we-only"口径漏了板级层。**已收编 4 条自洽件**
  （source-storage-runtime + latency-trace/exit-latency/unnest-trace 三
  hooks，定义 sheng_user_fault/exit/unnest_hook 指针）；**bringup 主件
  (2006 行) 未收编**：其 kern_exec 面在 sheng 24A446 树上重写
  （AGXMetalG17G/壁纸 27 变体），与我们 23F84 时代 zzy-* patch 同功能区
  fork 分歧，patch 级不可落（git apply/-3/patch --fuzz=3 三级 fallback 全
  试，kern_exec hunks #4-8/#11 真冲突）——**fork merge 挂账首刷证据后做**。
  prepare 新增 `git apply -3` → `patch -p1 --fuzz=3` 两级 fallback
  （3e6b3ca/378a922）。
- **步 3 终版结论（7 条 we-only）**：六条退役维持（23F84 墙修 ≠ sheng
  板级修），**prng 保留**（Xcode 27.2 转换税 + 0xbcad 24A446 三站同构实证
  donor-va ≈ 0xaf73560/0xaf74ef8/0xaf75868）。TEXT_WORD_PATCHES 清空（4 条
  23F84 钉全失效）。
- **verified-root-24A446 政策对（换 donor 隐性硬前置）**：Mac userdata 出
  件（AEA 链 = blacktop `ipsw fw aea --key` + `/usr/bin/aea decrypt`；ipsw
  CLI 3.1.741 已 brew 装机），OS 9,613,344,768B sha `8a12cdad` ✓ /
  SystemOS 6,016,729,088B sha `d5b4873d` ✓ = profile [root] pins 逐位同；
  政策对 = 通用化 build-owner-policies-local.py（窗口/pins 自
  userdata-root.json + profile 读），RSA 全块 **OS 586,752 + SystemOS
  367,232 全 PASS**。
- **emit 布局定谳**：24A446 kernel-entry __DATA 窗 0x1e8000（23F84 =
  0x22c000），native __DATA 放置其内；越界 0x1c000（撞 Libm 等四条
  retained __DATA @0xba1c000）→ **bp 采样环 4096→1024**（省 120KB）。
  ESP volume 96→99 MiB（24A446 payload 92 MiB 顶 95% 门）。
- **iOS27 源码面补齐**：BootProcessProbe/Worker/BootUiTraceProbe +
  BootLatencyTrace 家族 verbatim 收编；BootWatchdogPolicy/BootDiagExport
  双载 → build-boot-progress.py 门扩展 + TelemetryDefs 预处理门；
  ShengLatencyHooks.c 只留 native_gpu_role_thread（sheng 车道也无定义 =
  Apple ld 绑零；我们 ld64 拒 weak-undefined）。
- **参考链**：kit gitlink `b1888c12` ✓；RELEASE 参照件 Mac `5a794da8` ✓；
  trust bundle `dc655bcb` 双端 ✓；IPSW `fa93b653` 双端 ✓；kernelcache
  `38e6d9fa` ✓（profile 硬门）。
- **未推**：主树 `139c7c5` `beebdd8` `c97e6a2` `556b847` `d4766d1`
  `3e6b3ca` `378a922` `886eeee`；设备仓 `1464ec1`…`52d8e01`（12 笔）。
  Mac = rsync/scp 全量同步 ✓。
- **★首刷前提前项（2026-11-05 收队轮）**：①步 0 闭合——w42fix 屏墙
  donor 0xac8f594 定名 = **ktriage register-subsystem-strings 族**（范围门
  [0xa,0x12] = subsystem id；native `_ktriage_register_subsystem_strings`
  0x7834074 同形），24A446 AMFI 2661 bl 站扫描**零复现**（此墙新车道不
  在）。②wall-42 24A446 站表备妥（PENDING）：AMFI dev-mode 函数
  **0x924e7b0**（0x498B，/arm-io/soc-misc@0x924e800 + ApplePMGRNub
  @0x924e92c 同函数）三 caller 站 **0x924e77c/0x924ef1c/0x924ef5c**，与
  23F84 三站同构；首刷若撞 No-PMGR assert = TEXT_WORD_PATCHES 三站→
  shim 一重建一刷。③bringup fork-merge 侦察定谳——36 段中 35 段 per-file
  可落但实跑**多文件真分歧**（exception.c/iokit_rpc.c/pmap.c/proc_reg.h/
  vm_dyld_pager.c/sched_clutch/vm_param.h = 我们自己的 4k/sandbox 链改同
  行），patch 级不可落；**series 保持 r52 基线**（bringup-core 两次摘除，
  6bd09d3）；merge = 首 刷证据后按缺失面逐文件手工工程（设备仓
  e479869 普查账）。
- **★下轮工作单 = 会话五十三 r52 首刷判读（runbook，2026-11-05 定稿）**：
  0. **工具前置**（判读开始前一次）：`scp` Mac 侧 24A446 三件回 Linux
     判读工作区（`out/m2381/ipados-27.0.1-24A446/m2381/kernel/kernel.macho`
     + `tree/build/xnu/ios-obj/DEVELOPMENT_ARM64_BCM2837/mach.development.
     bcm2837.unstripped.noctf` + `kernel/kernel.json`）；`tools/xnu27/kc.py`
     路径参数化（DON/OUR/NAT 现钉 23F84，SLIDE `0x1f000000000` 不变——
     13 跑 record 验证同值），判读站坐标 = donor 空间口径。
  1. **收屏**：刷机结果屏摄（挂 = panic 屏；进 = 启动面/黑屏，黑屏不阻塞
     ——屏态永非判决）。
  2. **判读主线（分流）**：
     a. `verify-panic-crc.py` 对拍（assert 配方 `"MSG" @%s:%d` 已校准；
        24A446 PANIC_TEXT 布局/映射若变 = 会话 49 屏映射法重对 LC）；
     b. panic 站解码 `kc.py wall SITE`（donor 空间；屏地址归属先对 LC
        判 native/donor——49 课）；
     c. **AMFI "No PMGR?"（第一预期，预案已备）**：ConfigurationSettings
        :388 同族 → 启用 PENDING 站表 `0x924e77c/0x924ef1c/0x924ef5c` →
        `_bl_to_native('unixnu_amfi_devmode_config')` 三条目入
        TEXT_WORD_PATCHES → 一重建一刷（r53b）；
     d. unmapped-call 哨兵墙 → 对号 kernel.json missing 面（320 站基线
        已录）定名分流；
     e. 新墙 → 三证定名 → 通道选择（内核目标=fixture / kext 站=TWP）
        → spec → 一重建一刷。
  3. **成功分支旗标**：native pac-keys → IORTC 税（t≈32s，clock_initialize
        _calendar 面 = 23F84 墙 41 后继形态待验）→ vfsinit → root/cryptex
        mount → **SHLOG 窗 state=2 = 大捷旗标**（24A446 root layout 才
        开窗）→ launchd/SpringBoard 站。
  4. **判读后工程**：AMFI 站表启用出件；bringup fork-merge 按缺失面
     证据逐文件立项（设备仓 e479869 普查账）；PMGR 包重取证（仅当
     AMFI 等待面实锤需要 nub 路径）。


<!-- ==== 会话五十七 §0(墙57定谳+r58工作单),2026-11-06 会话五十八移档 ==== -->

## 0. 现势指针（每轮首更，2026-11-06 会话五十七·墙 57 定谳 + 同族批量手术 r58 出件待刷版）

- **★会话五十七 = 墙 57 判读定谳（_IOFree 132 槽导入面）+ 同族批量手术 r58 出件待刷（2026-11-05/06）**：
  **r57 首刷判读**（payload-r57-104bff44）：crc `0x8f64878b`（autofix 仅补前
  导 f,零数字错）,t=1s,stage 至 mapping_adjust。panic from `0xfffffff00ac209e4`
  （x0 `0xfffffff7b0ecbe00` = **堆指针**,x1 `0x518`）→ **墙 56 消实锤**：r57
  shim 分配成功,同 corecrypto PRNG-init 调用方 +0xE0 推进到清理路径。新哨兵墙
  机制 = **尾跳进哨兵**：bl `0xac209e0` → 局部直调 g(`0xac207e0`,kext 本体非
  thunk)→ pred1（槽 0x84ac9b8,已绑）非零清旗返;pred2（槽 0x84ac9d0,已绑）
  为零 → 尾跳 b `0xac818b0` → 槽 `0x84ac900` = 哨兵 `0xb5a9c4c`（r52 同址）；
  尾跳先恢复 x30 故 LR 保 `0xac209e4`。**定谳 = donor `_IOFree`**
  （`0xfffffe000b441b54`,80B,sha `1506b0c5…`：cbz x0 + KHEAP_DEFAULT view
  `0x7f08e00` + bl donor kfree_ext `0xad3e43c`;与 donor _IOMalloc 同 view
  相邻 +0x130）——**s54"死 thunk"标签翻案**。**槽枚举法定标（工具级）**：
  donor KC `__DATA_CONST` 低 48 位扫描,IOMalloc 自证 110 恰数,IOFree =
  **132 槽**（115 原样 + 17 哨兵）。**坐标定标**：kernel.macho native =
  noctf + `0x39BC000`（kernel.json statistics __TEXT_EXEC old→new 自证）；
  native kfree_ext = `0xace8d18`（kalloc_ext 0xace84d4 + 0x844,cbz x1 NULL
  自守）;native _IOFree 配 view `0x7dec4e0` ≠ shim 的 KHEAP_DEFAULT
  `0x7dec3e0` → 不能绑 native,必须镜像 shim。**r58 = 同族批量手术**
  （`UniXNU-m2381/scripts/s58-payload-surgery.py`,前像白名单三分类:donor 词/
  哨兵 → 重写,emitter 已绑 native → 保留）：
  ① shim_iofree @0xaef0820（5 指令）→ 132 槽;② shim_kfreevar @0xaef0838
  （3 指令,discard donor kt_view;= kfree_type_var `0xb441c0c` 尾块,98 槽,
  16 槽 emitter 已绑 native `0xb415f80` 保留）;③ kprintf 65 槽全量 → native
  `0xb514cb8`（s53 孪生,10 槽保留）;④ mach_absolute_time 133 槽 → native
  `0xaefbfdc`（s54 孪生,12 保留）;⑤ lck_mtx_alloc_init 8 槽 / IOLockTryLock
  10 槽 → native（fipspost 路径已实证可用,3/2 保留）;⑥ IOFreeAligned 7 槽 →
  native `0xb4151dc`（其 view = KHEAP_DEFAULT 自身,免 shim）。共 **453 槽 +
  2 shim + seal 重算**。seal 契约修正:fileset entry 的 section offset 本就
  全藏相对,不可再加 entry 基址（首版误加 0x180000 写糊 32B 字符串,r57 本体
  无损,已重做）；真 `__fips_hmacs` @payload `0x4ec5fd`,HMAC
  `6fc44336fd3f44b7…` 盖 entry `__TEXT,__text`（payload `0x3e49800`,
  0x9e0e94B）。**全图 diff 审计 = 417 簇/1506B,零不可归账字节**。
  **r58 出件**：`out/m2381/payload-r58-ca755a65.img`（99 MiB,内核封印件
  与 r57 同 = `20b4729f`,unixnu.toml 不变）。源码双轨:
  `fileset-iomalloc-shim.patch` 扩展 `unixnu_iofree_shim` +
  `unixnu_kfreevar_shim`（三 xnu feed 干跑全过,主树 5ab920a）;账本
  records 151-153 + 144-147 全量重绑注记（设备仓 d4a9ba6）。
  **记档不修**：donor `0xb441738`（114 槽大面,`b 0xad3e894` = [x0] 函数指针
  派发尾块,拷贝静态存活,语义未证）;donor `0xae9bd20`（18 槽,拷贝首调用经
  **donor 零槽 0x7dffa68** blraa,需 donor fixup 解析才能定名）。
- **★下轮工作单 = 会话五十八 r58 首刷判读（runbook）**：
  0. 收屏;CRC 对拍照旧（verify-panic-crc --autofix）。
  1. 分流：
     a. **同族推进成功**（无 fileset 哨兵墙）→ 预期面:PRNG init 过
        分配/释放 → fipspost POST 完整跑 → PRNG 登记 → PE_init_iokit →
        AMFI（t≈32s IORTC 税）→ No-PMGR → PENDING 站表 0x924e77c/ef1c/ef5c。
     b. **新哨兵墙** → 照哨兵流程:bl 定位（注意尾跳类 = LR 可能保留上游）→
        槽 → donor 坐标 → 三证定名 → 修。**候选病灶表（会话五十七预研,
        撞到即按行施工,修法已钉）**：
        | donor 桩 | 面 | 已知身份 | 钉死修法 |
        |---|---|---|---|
        | `0xb3d385c` | 292(267+25哨) | OSObject 默认构造器（体首 = pacda 写
        per-object 签名 isa,0xcda1 上下文,写 [x0]）| 直绑 native
        `__ZN8OSObjectC2Ev`（noctf 查名 +0x39BC000;s45 墙36 fixture 前例;
        注意 267 个存活槽证明 donor 拷贝部分可用——先看哨兵 25 槽的调用方
        再决定全量 or 只绑哨兵面）|
        | `0xb46c278`/`0xb46c2d0` | 186×2(161+25哨) | IOCECFamily 桩对
        （体首 = `mov w8,#1; str w8,[x0,#8]` = 置标志 + GOT 调用）| 先
        disasm 全身定名再选:native 孪生直绑 or 镜像 shim |
        | `0xb4424c4` | 134(118+16哨) | lck 默认组包装（`ldr x0,[0xb929330]`
        → tail donor lck_mtx_alloc_init）| shim:`adrp x0,<our 默认 lck_grp>
        ; ldr x0,[x0]; b native lck_mtx_alloc_init 0xaceffe8`——先从
        noctf 查我们内核的 IOLock 组全局名 |
        | `0xb441738` | 114(全 donor) | `b 0xad3e894` = [x0] 非空检查 →
        OSMetaClass 注册表派发（sub/lsr#7/cmp#0x40b≈1035 表）；我们内核
        同款模式零命中 = 寄存器分配差异 | 待屏证（x0/x1 会给对象与参数）；
        定名走 KDK 邻域 or 模式扫描（放宽寄存器）|
        | `0xae9bd20` | 18 | 拷贝首调用经 **donor 零槽 `0x7dffa68`** blraa
        （donor 文件即 0,运行态由 donor fixup 填）| 需解析 donor chained
        fixups 得运行态目标才能定名；撞墙时先看 x0/x1 |
        通用：emitter-dead 权威名单 = kernel.json `stubbed`（490 桩）;存活
        拷贝（donor 词槽）多数可用,勿全量盲修。
     c. **数据 abort / PAC 类** → 先复核 shim 常量（kfree_ext `0xace8d18`,
        KHEAP_DEFAULT `0x7dec3e0`）与 braa 鉴权形（ESR EC 0x1C）。
     d. **r58 上机自检表（刷后读回对拍,dd 读回才算证据）**：payload
        `ca755a65` 开头;shim @0xaef0820 = `1a 01 0a aa …`(mov x2,x1)@
        0xaef0838 = adrp;槽 `0x84ac900` = `0x8011000003eec820`;
        `__fips_hmacs`@payload `0x4ec5fd` 前 8B = `6fc44336fd3f44b7`。
  2. 工具坑累积（先读再动）：temp/rNN/kernel.macho = **术前件**（验手术
     必须读 payload;110 shim 词只在 payload）;rNN-shim.json 的 shim_va 可能
     是废稿（真身 = 槽值解码）;kc.py DON 钉 23F84;donor VA 低 48 位 =
     `(VA & 0xFFFFFFFFF) - 0x7004000`（直接相减会残留高位）;capstone 在
     user site;账本权威 = 设备仓 kits/。
  3. 成功旗标：vfsinit → root/cryptex mount → SHLOG state=2。
  4. 回退：recovery_a ← r57 `104bff44`（一级）/ r56 `94a096c3`（二级）。


## 归档自 STATE §0（2026-11-06 会话六十六 首更时移出）

- **★会话六十五 = r64 首刷判读（墙62/63 消实锤）+ 墙64 定谳 = OSMetaClass 六参
  zone ctor + 2 面绑 native，r65 出件待刷（2026-11-06，连轮，无 agent 直判 30 分钟）**：
  **r64 首刷判读**（payload-r64-b7535382）：crc `0x785f87e4`（屏摄），t=1s，hit #1。
  panic from `0xfffffff00925a148`（x0 `0xfffffff00bac96e8` x1 `0xfffffff007642b21`）→
  **墙 62/63 双消实锤**（IOStorageFamily 注册函数 22 调用全绑放行，推进进 AMFI）——
  静态预读战略第 3 轮兑现。**判读链（tracer 秒解）**：caller bl `0x925a144` =
  **com.apple.driver.AppleMobileFileIntegrity** __TEXT_EXEC（span 修复桶归属）→ 槽
  `0x8135d30` 哨兵 → donor `0xb3d37ec`。语义：x0 = AMFI __DATA 全零静态（metaclass
  存储），x1 = **串 "OSDetachedCertificates"**（AMFI 类名；串池邻域 = zone 名
  "iokit.OSDetachedCertificates" + ConstraintUtilities.cpp/CertManager.cpp 串）= AMFI
  静态 C++ 构造注册自类。**定名**：donor `0xb3d37ec` =
  **`__ZN11OSMetaClassC2EPKcPKS_jPP4zoneS1_19zone_create_flags_t`**（OSMetaClass
  六参 zone_create ctor，OSMetaClass.cpp logError 后首个导出函数）。三证：①donor 体
  [0xb3d37ec,0xb3d385c)——pacibsp+存 x3-x6/x0（x1/x2 活）→ bl `0xb3cfcbc` = donor 三参
  核心 ctor（委托；map 0xb3cfca8 "_unix_syscall" 系 how=calls 误名）→ tbnz 全局位门
  （=if (!(kIOTracking & gIOKitDebug))）→ zflags|ZC_ZFREE_CLEARMEM → bl `0xadb3e94`
  zone_create → `str x0,[x19]` 写 \*inZone → 返 this；②native 孪生 noctf 精确
  0x79ed4bc（km `0xB3A94BC`）：序幕与 donor 逐字节相同；native 布局镜像 donor 序
  （三参 C2 0xB3A90EC < logError 0xB3A9360（map how=body-hash 精确）< 六参
  0xB3A94BC）；③源码锚 OSMetaClass.cpp ~575 行（委托+zone_create 门逐指令吻合）。
  第 2 面 `0x83b6148` = **IOSurface**（zone-backed 类，交叉证实孪生）。**修 = 2 面绑
  native**（词 `0x80110000043A54BC`）。s65 手术（设备仓 scripts/s65-payload-surgery.py，
  新增 gate2 = noctf 表精确名+km delta 双验）：**2 面 6B/2 簇零杂散，seal 未动**。
  **出件 `out/m2381/payload-r65.img`（99MiB）sha256 `7bac97e6…` 待刷**（recovery_a ←
  r65；回退一级 = r64 `b7535382`）。账本 818→**819**。**★工具坑定谳**：native 体
  必须读 kernel.macho `__TEXT_E`（vm 0x8500000，km VA = noctf+0x39BC000 直落其中）；
  读 payload 的 donor 段布局 km VA 会拿到 donor exec band 字节（本轮误读一次浪费两
  个 dump，验算法无碍——绑定公式由 r61-r64 四轮实机背书）。本轮判读无 agent 直判：
  tracer 秒解链 → 读 x1 串秒定性（s59 同款）→ 源码序对位 → noctf 精确 → 手术，全链
  ~30 分钟 = 农场单墙标准节拍。
- **★下轮工作单 = 会话六十六（r65 判读 + AMFI/下一 kext 连清轮；激进多轮授权延续）**：
  A. **r65 刷机判读**（recovery_a ← r65 `7bac97e6`；回退一级 = r64 `b7535382`）：
     成功旗标 = 新 panic caller ≠ `0x925a148` 族（OSDetachedCertificates/IOSurface
     两类构造面放行）、crc 必为新值。若 caller 仍在 AMFI（0x92xxxx）→ tracer 秒解
     → 读 x1 串定性 → 源码序对位 → noctf 精确 → s66 手术（模板 s65，改 BINDINGS+
     面数断言+gate2）——单墙 ~30 分钟节拍，一个会话连清多墙；若换新 kext → 先查
     s64-kext-buckets.json 桶归属再同流程。
  B. **静态预读续用**：每绑完一墙，对 caller 函数前向扫下一个未绑 bl（s63 PoC 法，
     temp/s64-tracer.py 现成），预测墙先行判读——刷机只验证不探路。
  C. **IOStorageFamily 中置信 4 条人工复核**（头号 `_IOLockWakeup` 6 面：topk
     s_min=1.0 + ledger 邻居 IOLockSleepDeadline+0x14 + 语义契约唯一性）：复核过才
     绑，无证据不绑边界不变。
  D. **挂账区（非刷机轮做）**：UEFI 仓 7 条未推（含上游 sheng 5 条）找判读间隙推
     掉；temp 大件隔轮清（s64/s65 产证 wall62/63/64 报告 + farm manifest + buckets
     **保留**作 APFS 桶工作底稿）；主树 kit 账本 812 vs 设备仓 819 权威——下次 Mac
     重编/emit 复触发前先同步；主树 out/ 顶层 23F84 残留删除待用户拍板（s61 定谳勿
     引，删除不可逆）。
  E. **底稿就绪**：APFS 桶 155 面 = 下一大户（buckets/manifest/tracer 三件现成）；
     emit 维持降级挂账（复触发 = 首次 Mac 重编顺带开）。AGENTS.md 指路牌已修
     （§8/20H392 过期指针 → §0 现役车道，2026-11-06）。
- **★会话六十四 = 静态预读双墙定谳（墙62/63）+ 判读农场基建 + r64 出件待刷


## 归档自 STATE §0（2026-11-06 会话六十七 首更时移出；其"AMFI清仓/userdata已刷"两结论已被 s67 勘误，原文留档见勘误）

- **★会话六十六 = r65 首刷判读（墙64 消实锤）+ 墙65 定谳 = os_log_create + 5 面绑 native，r66 出件待刷（2026-11-06，连轮，无 agent 直判 ~40 分钟 = 农场单墙节拍；AMFI 内 sentinel 清零）**：
  **r65 首刷判读**（payload-r65-7bac97e6）：屏 crc `0x992021fb`（照片换行拼接噪声使初转录 MISS `5c859dd0`，蛮力单字符仲裁零歧义恢复 = 三处地址各一处 'f'→'8' 误读，转录已定谳），hit #1，t=1s，pages `0x12558d` free `0x124e4d`，procs=0 root=~（AMFI 静态构造带）。panic from `0xfffffff00924f21c`（x0 `0xfffffff00763e445` x1 `0xfffffff00763e463`）→ **墙 64 消实锤**（报点 `0x925a148`→`0x924f21c`，OSDetachedCertificates/IOSurface 构造面放行，推进进 AMFI 深处）。**判读链（tracer 秒解）**：caller = AMFI text span `[0x92469c0,0x92721c8]` 内 15 指令小函数 `0x924f1fc`（日志句柄 init helper）：x0 = 串 **"com.apple.MobileFileIntegrity"**（`0x763e445`，os_log subsystem = AMFI bundle id）、x1 = 串 **"library_validation_failure"**（`0x763e463`，os_log category）→ bl `0x924f218` → thunk `0x9271d28` → 槽 `0x8136200` 哨兵 → donor `0xb41d998`；随后 `cmp x0,#0`/`csel` 与 OS_LOG_DEFAULT 导入（槽 `0x81364e0` 已绑）回退并存入 AMFI 静态 `0xbac95a0` —— NULL 回退恰镜像 os_log_disabled 早退。**定名**：donor `0xb41d998` = **`_os_log_create`**（libkern/os/log.c:613，静态 os_log_create_impl 内联）。四重证据：①签名 `os_log_create(const char *subsystem, const char *category)`（log.h:161）与双串参数精确吻合；②体镜像（native km `0xB3EA19C` 读 kernel.macho `__TEXT_E`）：pacibsp+stp 墙（donor 帧 0x1a0 vs native 0x180 均匀 +0x20 平移）、全局拷贝守卫 `stur [x29,#-0x60]`、**os_log_disabled 字节旗标门→返 NULL**（donor 0x7ec4fc0 位0 vs native 0x7e4d678 位0）、key 缓冲 memset（0x100/0x120）、subsystem→category strlen+0x80 clamp len 存 [sp,#2]/[sp,#3]；③donor map 括区：_os_log_hash `0xb41d788`（map body-hash→km 0xb3ea0f0）< 靶 < __os_log_internal `0xb41df20`（call-sites→km 0xb3ea640），native 括区内唯一 noctf 函数即 create（_os_log_equ 0xb3ea13c < _oslog_try_get 0xb3ea190 < create 0xb3ea19c）；④源码锚 log.c:556 os_log_create_impl：disabled 早退 + os_log_subsystem_init 打包 + cache find/insert = 邻接 _os_log_key_hash/_os_log_hash 一族。体 `[0xb41d998,0xb41df08)` 1392B（出口块 0xb41decc→retab 0xb41df04，紧接 __os_log_internal 0xb41df20 无夹层）。**修 = 5 面绑 native**（AMFI `0x8136200` + 兄弟 kext 导入者 `0x7f85720`/`0x82da8d8`/`0x83584d0`/`0x848d450`；词 `0x80110000043E619C`）。s66 手术（设备仓 scripts/s66-payload-surgery.py，模板 s65，体终点用 map 下界+retab 实扫）：**5 面 15B/5 簇零杂散，seal 未动**。**出件 `out/m2381/payload-r66.img`（99MiB）sha256 `fa977e3a…` 待刷**（recovery_a ← r66；回退一级 = r65 `7bac97e6`）。账本 819→**820**。**★分水岭**：r66 后 AMFI 全域 `[0x92469c0,0x92721c8]` 重扫 **sentinel = 0**（temp/s66-amfi-prescan.json）——本墙是 AMFI 最后一个漏绑导入，AMFI kext 导入面清仓；下一墙将在下一个 kext 出现（当前全图余额 810 槽，桶头 apfs155/AppleARM70/sandbox63）。
- **★下轮工作单 = 会话六十七（r66 判读 + 换 kext 判读轮；激进多轮授权延续）**：
  A. **r66 刷机判读**（recovery_a ← r66 `fa977e3a`；回退一级 = r65 `7bac97e6`）：成功旗标 = caller ∉ AMFI span `[0x92469c0,0x92721c8]`（AMFI 清仓验证）、crc 必为新值。若 caller 仍在 AMFI span → 预读器漏了（先查 s66-amfi-prescan.json 对账再判读）；新 kext → 先查 s64-kext-buckets.json 桶归属 → tracer 秒解 → 读 x0/x1 串定性（本轮三串秒杀法续用）→ 源码锚/noctf 精确 → s67 手术（模板 s66）。**已知下一大户预判**：apfs（155 槽，root mount 前必经）、AppleARMPE/AppleARM70（48）、sandbox（63）。墙序预判 = APFS 系先撞（ vfsinit/roots 阶段 procs=0 → APFS driver 注册），若 caller 落 apfs → 体取证常含 mount/label 语义串，同节拍判读。
  B. **静读续用**：s66-amfi-prescan.json 已证"每墙后全域重扫"零成本（tracer ~35s），换 kext 后对新 kext span 同款重扫，预测下一墙先行判读——刷机只验证不探路。
  C. **IOStorageFamily 中置信 4 条人工复核**（头号 `_IOLockWakeup` 6 面）：复核过才绑，无证据不绑边界不变。
  D. **挂账区（非刷机轮做）**：UEFI 仓未推 **已清账（2026-11-06 会话六十六重启前间隙，force-with-lease 5297334c→e2c13592；远端旧件 = 本地 10-30 rebase 改写前版本，内容被 9318b62e 完整包含，零丢失）**；temp 大件隔轮清（s65/s66 产证 wall64/65 报告 + tracer/prescan json **保留**）；主树 out/ 顶层 23F84 残留删除待用户拍板。
  E. **emit 挂账不变**：AMFI 清仓后 kext 面只剩 810 槽，emit 复触发仍挂账（= 首次 Mac 重编顺带开）；APFS 桶 155 面工作底稿（buckets/manifest/tracer 三件）就绪。

## 归档自 STATE §0（2026-11-07 会话七十三 首更时移出；块内 s72 审查轮/s71 同步/s69-s67 各轮出件均已被 r72 新基座取代，判读结论以 r72 判读与各记忆条为准）

- **★★会话七十二 = 审查轮定谳（2026-11-07）**：用户四项拍板：**A 留 24A446**；**先刷 r71**（跳过判读等待，union 红利留 r72）；**kit 反哺先不推**；**哨兵扩容 r73 单独轮**（不与 union 首件混变量）。
  **★根因修正（2026-11-07 深夜终版，推翻本会话早前"全 referenceless"论——用户指正+Mac out 树实测）**：
  ①闪刷链基座 payload-r55(dd51ff60) = **Mac 11-05 09:04 带参照 emit**（matcher 11 / reference `713f8f91…` / kernel e64448dd，Mac out 树实测）；r56-r71 全部是对这个**带参照**基座的 payload 手术，Linux r52 无参照件（kernel.json None）是没进闪刷链的另一版。
  ②参照件 24A446 LTO 版**在 Mac 上已存在**（`tree/build/xnu/fileset-reference/lto-b19d07e101904a5f`，nm 实测 `_IOMalloc/_kfree_data/_IOFree` 三符号俱在、omitted 清单 67 条无 wall 家族）——但 **r55 map 对 wall 家族绑定为 0**（map 9471 对：reference-order 4338 + reference-symbol 1489，**reference-shape 仅 1**）→ 墙 56-71（IOMalloc/IOFree/kfree_data 族）照样逐个发生。
  ③**责任账（用户指正，认）**："没必要刷 Mac 构建的镜像"系此前会话助手判断，payload 手术一直叠在旧基座上；本会话审查在未查 Mac 侧情况下错判"全 referenceless"。教训：手术轮次锁死基座；审查必须双机对账；"没必要"三个字不能拦已构建的替代件。
  ④**r72 真杠杆 = 诊断参照通道为何漏掉 wall 家族**（reference-shape=1 是异常信号；嫌疑=通道种子传播门控 / kext 导入路径未查参照符号表 / 参照件与 donor 的 LTO 内联分歧超通道容差）→ 再 108c 源重建参照 → 重 emit → 839 账本照常兜底。上游 156 残余天花板参照不变。
  **审查账目（一手）**：上游 24A446 账本 156 fn+29 data **全部 ⊆ 我们 839+36，native 指认零分歧**；墙驱动条目 ≈163 ≈ 156（同量级互证）；check-m2381 **4 ok/0 fail**；merge-base = upstream tip 10edc1ef **零滞后**。已知 upcoming 雷：stack-shift site（PAGE_SHIFT_CONST→DER 迭代器 4K 页假错 "copyDictionary failed with 14"，sheng 系板 patch 符号，我线未绑——判读遇此直接按案卷绑 15）。
  **★r72 工作单（下会话执行；用户拍板 = Mac 侧构建 + 先查文档里的完整 Mac 构建/拉回方案照抄，别重造）**：
  **Mac 侧资产清单（本会话实探）**：连接 `naoniudeair@192.168.10.246`（免密 id_ed25519），工作区 `~/unixnu/`；非交互 ssh 必须 `PATH=/opt/homebrew/bin:$PATH` 前缀（STATE-history 4036 坑）。主树 @b130f79（落后 ~85c）+ **3389 行未提交 s44-s51 工作**（含 `prelink-regression.json` +1336 载重件、build-xnu-4k-baseline.sh、patches.py——先 diff 确认后提交/保存，**再** hard reset 到 origin/m2381-ios27）；设备仓 checkout 在 `~/unixnu/UniXNU-m2381`。信任缓存 `~/unixnu/original-trust-24A446.bundle`（sha8 dc655bcb）；**data seed = sha `d04a0153…c8486a`（=r62/r85 记录同一种子）两机均未找到实体**——按 sha 找或 `make-data-seed.py` 重造（hdiutil Mac only）。Mac 11-05 资产：payload dd51ff60（r55 基座本体）/kernel e64448dd/map 9471 对/参照件 LTO 缓存 `lto-b19d07e101904a5f`。
  **序列**：⓪**先诊断**（比重建更值钱）：拿 r55 map + 参照件 nm 表对照，查 matcher 参照通道为何 wall 家族绑 0（reference-shape=1 异常；fileset_match.py 的通道种子传播门控 / kext 导入路径是否查参照符号表）；①拉两仓 + `git submodule update --init --recursive`（ArmCpu 已拨 93a5030，QcomBaseband 新增）；②`./unixnu kernel --skip-prelink -j 8`；③`python3 scripts/build-fileset-release-reference.py --tree out/m2381/ipados-27.0.1-24A446/m2381/tree --lto`（108c 源变缓存必重编）；④`./unixnu prelink`；⑤**出件前闸门**：kernel.json `reference_sha256` ≠ null + **map 对 `_IOMalloc/_kfree_data/_IOFree` 绑定数 >0（诊断生效判据）** + 墙72 靶 `0xfffffe000b4f1304` named+bound + stubbed 对 r55 基线（28 镜像/1030 项）塌缩；⑥`./unixnu payload`（三红线旗标；seed sha 上面）→ 按**文档里的拉回方案**传回 Linux `out/m2381/` 平铺路径。
  **Linux 侧已完成前提**（均已推）：data 账本 7→36（设备仓 vendored 权威 839+36，2203d75）、ArmCpu 幽灵钉修复（64bda2d0）、判读/STATE（db815518+）。kit 代码谱系对账顺延 r73。
  **r71 首刷判读（2026-11-07 03:43 屏摄 IMG_20261107_034317）**：crc `0x71c3aef3` 蛮力单字符仲裁**唯一命中**（from/x0 各掉一 f，标准噪声）。报点 `0xfffffff00aa5ad64`（x0 `0xfffffff008488670` x1 `0x18`）hit#1 t=1s。site `0xaa5ad60`（sandbox span `[0xaa46c70,0xaa81594)` 内）≠ 墙69 `0xaa5a2e4`/墙70 `0xaa75010`/墙71 `0x92029c8` → **六槽绑定全生效、墙71 消**。**墙72（靶已解，名待参照）**：bl→GOT 槽 `0x848d0f0`→SENTINEL→donor `0xb4f1304` = `IOUserClient::dispatchExternalMethod`(0xb4f10d4) 后继函数起点——体界实锤：BTI C 入口 + 前函数 `b` 尾跳收 `0xb4f1300`；体 +0x10 `cmp x1,#0x18` 与 panic x1 自洽；0x38 字节短叶返回 `kIOReturnBadArgument`(0xe00002c2)/csel +0x13 变体 + ldaxr 全局原子。donor 布局≠noctf（体 0x38 vs noctf externalMethod 0x460），名不手定——**r72 参照 emit 自动命名+绑定，不手术**。
- **★会话七十一 = r70 首刷判读（7 槽全生效）+ 墙71 定谳（AppleMatch kfree_data 族）+ 6 槽批绑，r71 出件待刷（2026-11-07 凌晨连轮）**：
  **r70 首刷判读**（payload-r70-ecf23f32；屏摄 IMG_20261107_014846 01:48）：crc `0xb64e1097`
  单命中零纠正，hit #1，t=1s。报点 `0xfffffff0092029c8`（x0 `0xfffffff802dcc0c0` x1 `0x35`）。
  **site ≠ 墙70 的 0xaa75010 → 同 payload 必同 site 判据 → r70 七槽绑定全部生效、墙70 消**。
  **新 span = com.apple.kext.AppleMatch**（text [0x9200c20,0x92032a8) +0x1da4，s64 桶表）——
  AMFI/sandbox 的 entitlement 正则 kext，与墙 69/70 同一条执行线（entitlement 匹配流水）。
  **caller 实据**：x0 = `0xfffffff8_02dcc0c0` = **内核堆银行指针**（高 32 位 0xfffffff8 vs 文本段
  0xfffffff0——新情报：本 runtime 堆在 0xfffffff8 段），x1 = 0x35 = **size 53** → kfree_data(addr,53)。
  **span 普查**（s70-applematch-prescan.json，tracer 51 bl）：8 哨兵收敛 **2 靶**：`0xad3e8e4`×7
  站（槽 0x8126030）+ `0xad3e8fc`×1（槽 0x8126038）；**0xad3e8e4 同时 = s66 AMFI 普查二号靶**
  （12 站槽 0x8136108）+ s64 farm 118 行预判 `_kfree_data` topk 0.833（槽 0x83ae580）——三案卷
  交叉同靶。
  **墙71 定谳**：donor `0xad3e8e4` = **`_kfree_data`**、`0xad3e8fc` = **`_kfree_data_addr`**。
  四证：①donor 体与 native **逐指令同形**（0x18B/0x14B 包装器：mov x2,x1; mov x1,x0; adrp+add
  x0=kheap 全局; 尾跳 kfree_ext/kfree_addr_ext——仅各自 kheap 全局 0x7f08e00 vs 0x7dec3c0 与
  尾跳目标异）；donor kfree_data 尾跳 0xad3e43c = **map 精确 `_kfree_ext`**（native km 0xACE8D18
  同 noctf 符号=几何闭环）；②**族几何 donor/native 全同**（kfree_data +0x18 kfree_data_addr
  +0x2c kfree_shared_data；donor map 夹逼 _IOFreeTypeImpl−0x50…_kfree_shared_data+0x2c 间隙恰容
  双靶）；③noctf 精确（_kfree_data 0x732d360/km 0xACE9360、_kfree_data_addr 0x732d378/km
  0xACE9378）+ s64 farm 同 km 预判；④源锚 osfmk/kern/kalloc.h:521
  （kfree_data(elem,size)=kheap_free(KHEAP_DATA_BUFFERS,elem,size)）+ kalloc.c:2866。
  **修 = 2 靶 6 槽**（kfree_data：`0x80e1960/0x8126030/0x8136108/0x8333818/0x83ae580`→
  `0x8011000003CE5360`；kfree_data_addr：`0x8126038`→`0x8011000003CE5378`）——**一靶同时清
  AppleMatch×7 + AMFI×12 + 3 其他镜像**：设备仓 scripts/s71-payload-surgery.py，24B/6 簇零杂散，
  seal 未动，6/6 回读解码验证。**出件 out/m2381/payload-r71.img sha256 `ddbaa7b9` 待刷**。
  账本 827→**829**。
- **★下轮工作单 = 会话七十二（★审查轮，用户拍板 2026-11-07"下轮会话做审查"；激进多干授权不顺延）**：
  A. **同步完整性审查**：rebase 108c 对我们关键面的 diff 审计（profiles / payload.py / patches/xnu
     series / kit emit 代码——b6afb4b "bang root" emitter 新特性对我们出件流的影响）；kit 并集
     f9123a3 vs 设备仓账本一致性核对（应 839=839）；AdrenoMetal pin 6416a76e（GPU parity 线）对
     我们无 GPU 树的影响评估。
  B. **构建前静态门**：check-m2381 + 出件 dry-run（839 账本后 expected 哨兵变化数预估，对齐
     全图普查 778 槽/379 靶基线）；release-reference 与新补丁系列咬合验证。
  C. **r71 判读**（若已刷）：crc 必新；caller 允许 sandbox/AMFI span；**槽必非账本 839 全量**；
     同 payload 必同 site 纪律继续。
  D. **方向拍板材料**（用户三问完整账目，供 A/B 拍板）：A=留 24A446 提速（执行线 ~83 靶，家族
     自动解析 pass + 农场 3-5 靶/轮 ≈ 几十轮）；B=回 21H461 成熟线（上游 alioth 同款配对，账本
     三次清零的解药，代价=弃 iOS27）；侦察结论已入档（sheng=SM8550 同 SoC、config 假说排除、
     快因=板级成熟+全职早 3 天）。附两项待拍板：**哨兵扩容**（kext 归属名+x2/x3+槽地址，代价=
     fileset patch 改+全链重出+CRC 配方换版）、**kit 反哺推送**（f9123a3→上游 UniXNU/XnuPrelink
     main，让全生态跳过我们 700+ 条墙）。
  E. **卫生**：temp 大件隔轮清拍板（s6x-* 普查大 JSON）；备份分支 pre-sync71 保留至 r72 出件后。
- **★s71 同步+侦察（2026-11-07 用户令"先侦察，同步上游"）**：主树 rebase upstream/main **108c**
  （QcomDwc3 USB 驱动组件 + 蓝牙原生栈线 + root union fsid 修复 + iPad17,3 board_id/security_domain
  修复 + archive 可重现 + addon/UART runtime 修复；冲突三处按规矩解：AdrenoMetal pin 跟上游 GPU
  parity 线 / README 还上游 / kit gitlink 取我侧后统一 bump）。kit 仓并集 **f9123a3 = 839 条**（我们
  827 unique + 上游独有 12 条零冲突，同 VA 同 native 两家互证；上游独有 = IOService Start/Stop_Invoke
  RPC + OSSerialize setIndexed + mbuf 三兄弟 + cdevsw_setkqueueok + ttsetwater + IOPMrootDomain
  isLPWMode + panic_with_options + vfs_vnodecovered + ml_io_read8）；设备仓账本同步 829→839（c958bdd），
  主树 e86c5621 force-with-lease 已推，备份分支 m2381-ios27-pre-sync71。
  **★侦察定谳（用户"为什么别人快"案）**：**sheng = 小米 Pad 6S Pro 12.4 = SM8550 —— 与 m2381 同
  SoC**（"机型差异更大"前提不成立，是最亲兄弟）；他们 24A446 全绿（GPU Metal 149 draws/BT
  AirPods A2DP/WiFi/触控/UFS root 全 device-verified）。快的三因：①**板级线 23F84 时代已成熟**
  （共享 SM8550 支持现成，24A446 只换 OS profile）；②全职维护者早 3 天（11-02 开工 vs 我们
  11-05 换道）；③墙账两家独立收敛同批（他们 156 条里 144 与我们重叠=互证，独有 12 条=他们的
  USB 栈位置）。**config 假说排除**：我们 profile 本就带 `--fileset-reference mach.release...noctf`
  （s43 参照件），匹配参考件已对齐。我们真正的结构差 = 板级 bring-up 与换代同步做 + 换代两次
  （21H461→23F84→24A446 账本三次清零）。**下一步红利**：r72 起从同步后的树出件（新补丁+839 账本
  自动多绑）；USB 栈走他们铺好的路（QcomDwc3 + roots 加 USB 栈 + sheng-usb 模式）；可选反哺
  （把 839 推上游 UniXNU/XnuPrelink 共享，kit 本地线 96eedc2 未推）。

- **★会话六十九 = r68 首刷判读（墙出 AMFI 进 sandbox 里程碑）+ 墙69 定谳 + sandbox 普查 267哨兵/59靶 + 双靶批绑，r69 出件待刷（2026-11-07 凌晨连轮）**：
  **r68 首刷判读**（payload-r68-dd3e9636；屏摄 23:55）：crc `0x64d09506` 零纠正，hit #1，t=1s。
  报点 `0xfffffff00aa5a2e8`（转录多一 f，CRC 仲裁）。**墙离开 AMFI span** —— site `0xaa5a2e4` 落
  com.apple.security.sandbox text span `[0xaa46c70,0xaa81594)`（s64 桶表）→ **r68 LookupEntry 绑定
  生效、AMFI 清完、boot 进 sandbox init**（= s43 参照车道"崩点过AMFI进sandbox注册"里程碑）。
  **墙69 定谳**：donor `0xb26a124` = **`_proc_get_syscall_filter_mask_size`**（kern_proc.c:5289）。
  caller = sandbox init：w0=0 / w0=1 两调，结果存 sandbox 全局（UNIX/MACH mask 尺寸）。四证：①
  donor 体 52B switch（0→常量 0x22e=nsysent 558 / 1→0x80=mach_trap_count 128 / 2→全局
  mach_kobj_count / default 0，Apple release 常量化）；②noctf 精确 0x78838a4→km 0xB23F8A4；③
  native 体镜像同 switch（our build nsysent 是变量 [0x719e670]）；④源码锚。
  **sandbox 预读普查**（s64-tracer 全 span，temp/s69-sandbox-prescan.json）：**267 哨兵调用 / 59 个
  distinct donor 靶**（头号 0xaf21778×44 / 0xaf55418×28 / 0xb2aa3f8×27 / 0xad45364×25 / 0xb260110×14 /
  0xadb7cbc×10）。**头号靶定谳**：donor `0xaf21778` = **`vnode_hold`**（27.x 带锁 flavor：CAS [vp+8]
  自旋拿锁 → ldadd [vp+0xb4] holdcount++ → INT32_MAX panic；**panic `__FUNCTION__` 串 "vnode_hold"
  @0x706e6cb + "vfs_subr.c" + "vp %p holdcount overflow" 全串字节读讫 = 自报家门**；姊妹 leaf
  0xaf215f8 = matcher body-hash _vnode_hold，与 native km 0xAF5FE84 逐指令同形）。
  **修 = 2 靶 5 槽**（0x848d4f0→0x801100000423B8A4；0x80e1b10/0x8136408/0x848d828/0x84a0008→
  0x8011000003F5BE84）：scripts/s69-payload-surgery.py，19B/5 簇零杂散，seal 未动。**出件
  out/m2381/payload-r69.img sha256 `be541f1c` 待刷**。账本 823→**825**。
  **★工具坑（s69 新增两条）**：①capstone 从非 4K 对齐起点反汇编会**错位误读**（"非函数头"假象——
  必须回退到真 entry 重同步；本次两处"mid-function 假象"全是自己 offadj 双减/错位所致，幻灯
  +0x39BC000 继续全局成立）；②`offadj` 参数（KH 修正）sign 写反过一次——读 payload 统一用
  `KHEAD+(km−KBASE)` 正偏移。

- **★会话六十八 = r67 首刷判读（三笔绑定全生效）+ 墙67 定谳 SecureDTLookupEntry + 五面绑，r68 出件待刷（2026-11-06 深夜连轮）**：
  **r67 首刷判读**（payload-r67-41b52bea；屏摄 IMG_20261106_223903 22:39，用户标注"r66"系笔误——
  报点 0x924e69c ≠ 墙66 的 0x924e644/0x924e648，且 r67 22:13 出件 → 时间链定谳 = r67 首刷；判读
  纪律新增：**同 payload 必同 site，报点差可作 payload 版本判别**）。crc `0xeb9fb8ca`（verify-panic-crc
  --autofix 仅补前导 f，零纠正），hit #1，t=1s。工作单 B 三条件全过：crc 新 / caller 仍在 AMFI span /
  槽 `0x8135d08` ∉ r67 三已绑槽 → **r67 三笔（lockdown/identity×2）全部生效，墙66 消**，墙 +0x58
  推进至同一 caller `0x924e440`（LocalSigning.cpp DT 阶段）。
  **墙67 定谳**：donor `0xb5be330` = **`_SecureDTLookupEntry`**（pexpert device_tree.c）。caller 现场：
  `DTLookupEntry(0, "/chosen", &entry)`（x1="/chosen"@0x763b99e 字节读讫），返值比 #1(kDTSuccess)，
  随后 `DTGetProperty(entry, "darwinos-security-environment"@0x763e055, &ptr, &size)` —— 该槽
  `0x8135d00` r58 已绑 `_SecureDTGetProperty`（native 体逐指令同形复核讫，r58 绑定正确）。四证：
  ①donor 体 [0xb5be330,0xb5be5c8) 664B：init 旗标 cbz / NULL→csel gDTRoot[x25+0x318]+边界[x24+0x328]
  ccmp / 前导'/'跳过（空路径→*entry=root,w0=1）/ w9=0x3f 分量拷贝循环 / 走树 bl 0xb5be11c（matcher
  验讫 _SecureDTGetPropertyInternal）取字面 "name"@0x70cf529 + bl 0xb5bc51c 比名 + bl 0xb5be5c8 孩子
  迭代 / 错误块 device_tree.c 断言串（行 98/150，"Device tree pointer outside of device tree region"）；
  ②noctf 精确 0xfffffff007b5522c（km 0xB51122C = +0x39BC000）；③native 体镜像逐块同构（native
  bl 0xb511600 恰 = GetPropertyInternal 自身幻灯 km——几何闭环）；④源码锚 pexpert device_tree.c。
  **修 = 5 面绑**（`0x7f87218 0x80e17b8 0x8135d08[AMFI 双调用点 0x924e69c+0x924f00c] 0x8479a60
  0x848d0c8` → `0x801100000450D22C`）：设备仓 scripts/s68-payload-surgery.py，15B/5 簇零杂散，seal
  未动。**出件 out/m2381/payload-r68.img sha256 `dd3e9636` 待刷**。账本 822→**823**。
  **★工具坑（s68 定谳）**：payload 原生体读必须绝对偏移 `KHEAD+(km−0x7004000)`——漏 KHEAD 错位
  0x1c9800 并伪造"本地布局分歧"假象（lockdown/identity 两 runtime 锚点当场抓获翻案）；PACIBSP =
  `0xd503237f`（0xd503233f = PACIASP，勿混）。
  **★下一墙预读（s69 主菜）**：site `0x924ec80` 槽 `0x8135e08` donor `0xb472cd8` =
  **`_ZN9IOService15serviceMatchingEPKcP12OSDictionary`**（noctf 精确 0x7a7d024 → km 0xB439024；
  体镜像 withCString→withCapacity(2)→merge 同构；matcher 图未收）。caller = ConfigurationSettings.cpp
  SCRD 段：`serviceMatching("sep-endpoint,scrd", 0)` → 紧随 site 0x924ec90
  `waitForMatchingService(dict, -1=WAITFOREVER)`（槽 0x8135e20 r58 已绑）。**★绑定风险已标：本平台
  无 SEP SCRD 端点，单绑 serviceMatching = boot 挂死无 crc（比 panic 更糟的判读面）——s69 必须
  绑定 + SCRD 等待中和同轮上，单绑不刷**。
- **★会话六十七 = r66 首刷判读 + 预读 bug 正名 + 墙66 定谳 + 预读批量绑，r67 出件待刷（2026-11-06，连轮）**：
  **r66 首刷判读**（payload-r66-fa977e3a）：crc `0x7a50a1ae` 零纠正，hit #1，t=1s。panic from
  `0xfffffff00924e648`（x0 0x0 x1 0x1 = 上一调用的残渣，真签名无参）→ **caller 仍在 AMFI span 内**
  → **s66"AMFI 导入面清仓"系假零**：预读 JSON 的真结构 = `calls`（2661 条）+ 小写 verdict，s66 判读
  读了不存在的 `bls` 键 + 大写 'SENTINEL' 子串 → 空集误判为零。按 `payload_word` 低48位真值重扫：
  **AMFI 实余 114 调用 / 35 个 distinct donor 靶**（头号 0xb226cc8×24 调用点 / 0xad3e8e4×12 /
  0xaf21778×11），墙66 就在普查单里（site 0x924e644 早已记录）。
  **墙66 定谳**：donor `0xb2a405c` = **`_get_lockdown_mode_state`**（bsd/kern/lockdown_mode.c:87，
  Private.exports:407 导出）。caller = AMFI policy-config 函数 `0x924e440`（boot-arg 读 + 9×静态注册
  + PAC 字典查），调用后 `cmp w0,#0`/`cset` 存 AMFI 全局 `0x8130e58`，非零则连发
  0x10000000/0x20000000/0x40000000 三笔强制位 = lockdown-mode 执法扇出。四证：①体
  [0xb2a405c,0xb2a4134) 216B：内联 CAS 锁（0xb9cbaf8）+ init-done 旗标 + **msleep wmesg 串字面
  "get_lockdown_mode_state"（0x70a4d3d）自报名字** + 返 lockdown_mode_state 全局；②noctf 精确
  0x78bd4c4（km 0xB2794C4）；③原生体镜像（真 lck_mtx_lock/unlock 调用 + 同 msleep 形 + 同 wmesg；
  donor 内联 CAS / native 走调用 = 同操作两编译策略）；④源码锚逐指令吻合。
  **★userdata 勘误（用户记忆为准）**：24A446 userdata（Mac AEA 链件，11-05 02:32，15,631,122,432B
  sha `b280f63e9a0c8cf8…4667d`，OS 窗 `8a12cdad`@1MiB / SystemOS 窗 `d5b4873d`@9,614,393,344 与
  profile pins 逐位同）**从未刷过机**——s66 首更"r52 已带上机"系推断错误，设备现 userdata = 23F84
  旧件。本会话已从 Mac（`naoniudeair@192.168.10.246:~/unixnu/`）拉回本机
  `out/m2381/ipados-27.0.1-24A446/m2381/userdata/`（img+json 双件，sha 验讫）。
  **修 = 2 笔 3 面**（lockdown 槽 `0x8136078`×1 + identity 槽 `0x8135f80`（24 调用点共用）/`0x848d250`×2）：
  批量第二笔 = 预读普查头号靶 donor `0xb226cc8` = **`_csblob_get_identity`**（codesign 标识串 getter：
  guard [csb+0x20] 位0 → 找 CSMAGIC 0xfade0c02 CodeDirectory → BE32@[dir+0x14]=identOffset → 返
  标识串；noctf 精确 0x7843cb4/km 0xB1FFCB4 + 体镜像 + 邻体消歧[teamid=3指令leaf、cdhash=3指令缓存
  getter 与 noctf cdhash 体逐字节同=排除]）。s67 手术（设备仓 scripts/s67-payload-surgery.py）：
  **3 面 9B/3 簇零杂散，seal 未动**。**出件 `out/m2381/payload-r67.img` sha256 `41b52bea…` 待刷**。
  账本 820→**822** + os_log_create entry 勘误（假零段标注）。**判读纪律新增**：预读普查必须按
  `calls` 键 + `payload_word` 低48 == sentinel 真值过滤，任何"清零"结论前先打印计数非空验证。
- **★会话七十 = r69 首刷判读 + 墙70 定谳 + 农场 T2 定名，r70 出件待刷（2026-11-07 凌晨连轮）**：
  **r69 首刷判读**（payload-r69-be541f1c；屏摄 IMG_20261107_011047 01:10）：crc `0xf53b30e2`
  单命中零纠正（照片 f-run 17 位=多一 f 惯性，x1 反向少一 f——双侧仲裁），hit #1，t=1s。
  报点 `0xfffffff00aa75014`（x0/x1 = 两个 donor 串）。**墙仍在 sandbox span** —— site `0xaa75010`
  落 [0xaa46c70,0xaa81594) → **r69 两靶绑定生效、墙单调后移**。**caller 实据 = entitlement 前缀
  匹配**：x0 = "ENTITLEMENT:com.apple.security.exception.iokit-user-client-class"@0x7d646a2、
  x1 = "ENTITLEMENT:"@0x7d67dbe（字节读讫）——沙盒/AMFI 政策 init 的 entitlement 过滤解析。
  **墙70 定谳**：donor `0xae825b8` = **`_strncasecmp`**（map-nearest "_strcasecmp(+0x44)" 的真义 =
  靶是 map 符号身后紧邻的下一函数，donor map 缺 _strncasecmp）。四证：①donor 体 0x60B capstone
  （bti c/cbz x2 n==0→0/索引循环 ldrsb/tolower 内联 `(c−0x41)<0x1a ? c|0x20 : c`×2/ca!=cb→diff/
  ca==0→0/alen==blen==n 使 strbufcasecmp 长度尾编译消除）；②noctf 精确 0x74f0604（km 0xAEAC604），
  **noctf 家族序=源序**（strcasecmp 0x74f05c0/strncasecmp +0x44/strbufcasecmp +0x74/strlcasecmp
  +0xe4），donor 几何同（donor _strcasecmp 0xae82574=靶−0x44，matcher body-hash 在案）；③native
  镜像：_strcasecmp 17 条**逐字节 1:1**、_strncasecmp 24 条同集合**单步相邻重排**（donor ldrsb
  w13,[x1,x9] 在 cmp w11,#0x1a 前，native 互换=调度差，s67 lockdown 先例类）；④源码锚
  osfmk/device/subrs.c：strncasecmp_impl=strbufcasecmp_impl(s1,n,s2,n)+tolower 逐行对应。
  **T2 定谳（农场头名，28 面共槽 0x848d6e8）**：donor `0xaf55418` = 2 条 bti-c 别名尾跳
  （bti c; b 0xadbafd4；map-nearest _vfs_64bitready+0xac 系夹逼伪影）→ 真身 `0xadbafd4` =
  **`_current_thread_ro`**（无参 flavor：mrs tpidr_el1→thread（入参 x0 被覆写=零参）；[thr+0x420]
  =tro（donor 结构偏移）；**内联 zone-element 验证四重**（标记指针界对 [0x7df9f40]/16K 页算术
  [0x7e00140]/页元 ldrh&0x3ff==3 [0x7df9fa0]/回指针 [ro]==thread）→ 失败 bl 0xb59bd64 =
  `_zone_id_require_ro_panic`（map how=string）/邻 `___thread_ro_circularity_panic`；成功返 ro）。
  四证：①体 0x98B 全 disasm；②noctf 精确 0x73add2c（km 0xAD69D2C）+ donor map 夹逼同族
  （_get_thread_ro−0x94/_swap_task_map+0x690/_get_threadtask+0x8bc）；③native 语义镜像（bl
  _current_thread→[thr+0x4e8]→bl zone_id_require(3,0x38,ro)→回指针核对→返 ro；donor 内联 zone
  验证 vs native 走 helper = s67 lockdown"同操作两编译策略"类；偏移 0x420/0x4e8 各内核自洽，
  donor _get_thread_ro 同读 [th+0x420] 佐证）；④源码锚 osfmk/kern/bsd_kern.c:167。
  **修 = 2 靶 7 槽**（strncasecmp：`0x7f85788/0x836b580/0x848d698/0x849faa8`→`0x8011000003EA8604`；
  current_thread_ro：`0x82da968/0x848d6e8/0x849fcf0`→`0x8011000003D65D2C`；**全 __DATA_C 全局
  枚举——沙盒 span 外其他镜像的同靶哨兵槽一并绑**）：设备仓 scripts/s70-payload-surgery.py，
  28B/7 簇零杂散，seal 未动。**出件 out/m2381/payload-r70.img sha256 `ecf23f32` 待刷**。
  账本 825→**827**。**s68/s69 设备仓脚本+账本漏 commit，s70 一并收仓**。
  **★工具坑（s70 新增）**：①map-nearest(+0xNN) 判读：先查 noctf 家族序+几何（+0x44 常是"身后
  紧邻下一函数"），再下结论，勿把 addend 当函数内偏移；②noctf-syms.json 条目 = [带符号十进制
  int, 名]，`&0xFFFFFFFF` 才是 36 位坐标（0xfffffff0 前缀）；③twinhunter 归一化 hash 对**单步
  重排**盲——镜像判定落到逐条 capstone 对读。
- **★下轮工作单 = 会话七十一（r70 判读 + sandbox 农场续批）**：
  A. **刷机**：recovery_a ← r70 `ecf23f32`（payload-only；回退 = r69 `be541f1c`）。刷后 STATE §2
     已由 s70 预更（boot_b 289614ba / recovery_a r70 / userdata b280f63e）。
  B. **r70 判读**：crc 必新；caller 允许 sandbox span 内；**槽必非本轮 7 槽**（0x7f85788/0x836b580/
     0x848d698/0x849faa8/0x82da968/0x848d6e8/0x849fcf0）。current_thread_ro 28 面热路径 = 下一墙
     头号候选（若其哨兵面在 sandbox init 执行序最先撞线）。
  C. **农场续批（s71 主体，普查余 57 靶）**：T3 0xb2aa3f8×27（缓冲空间计算体待分型）；T4
     0xad45364×25（**lock_rw_old.c lck_rw 族 flavor 分型**："Taking non-sleepable RW lock with
     preemption enabled"@0x705490d、文件串 0x70547b9 已读）；0xadb7cbc×10（kfree_type_impl_internal
     邻域 = s60 家族，查 s60 已绑 native 是否同款）；0xb260110×14 / 0xb26324c×10（proc_name
     +0x7c）/0xad94128×10（thread_set_cpulimit +0x1c4）。四证 bar 不变，齐一靶绑一靶。
  D. **判读纪律**：CRC autofix 每轮必跑；kext 归属查 s64-kext-buckets.json；槽排除清单 = 账本
     functions 全量（现 827 条）；payload 体读 = KHEAD 正偏移。
- **★会话六十四 = 静态预读双墙定谳（墙62/63）+ 判读农场基建 + r64 出件待刷
  （2026-11-06，连轮，子代理并行产证）**：
  **caller 定性修正**：墙 61/62/63 的 caller 函数 [0xa41ca8c,0xa41ce70) 归属
  **IOStorageFamily**（span bug 修复后定谳；此前"memdev"系调用序与 memdev.c:665-673
  同构所致的特征误标，kext 本体 = IOStorageFamily 的设备注册函数）。
  **墙 62 定谳**：donor `0xb3d1d58` = **`cdevsw_add_with_bdev`**（r63 屏摄 crc
  `0x68352ecd` 零纠正，panic from `0xa41cb1c`，x1=`0xbb65188` cdevsw 112B 模板；
  bl `0xa41cb18` → 槽 `0x83ae510`）。三证：①体 316B——体首 bl donor `_cdevsw_add`
  `0xb3d1a98`（kernel.map how=calls）、bdev 写 chrtoblktab `0xb89bea4`[major]（cmp
  #0x17 后）、bdev 越界内联 `_cdevsw_remove` 回滚（五常量全中：cdevsw 0xb89c6f0 /
  nocdev 占位 0x7ec2ae0 / flags 0xb9284f8 / CAS 锁 0xb9ce228 / memcmp 0xae82458）、
  返 index/−1；②**s63"native 无符号"翻案**——noctf 精确 `_cdevsw_add_with_bdev`
  0x79ebd74（km `0xB3A7D74`，228B）在表（s63 只查 matcher map 未查 noctf-syms 所
  致），与 donor 逐块 1:1（native 调 `_cdevsw_add` 0xB3A7B3C + `_chrtoblk_set`
  0xB3A0FEC；几何闭环 0xB854BD8+24×0x38=`0xB855118`=native cdevsw 表基；
  chrtoblktab 0xB856D18）；③源码锚 bsd_stubs.c:266-284——with_bdev = cdevsw_add +
  chrtoblk_set + 失败回滚，`cdevsw_remove_with_bdev` 全树不存在，chrtoblktab 唯一
  读者 `chrtoblk()` 返 NODEV 属设计内（无启动早期 panic 面）。方案 B（shim 重构）
  作废。**墙 63 定谳**：donor `0xb3d1894` = **`bdevsw_remove`**（三重指纹 cmp#0x17/
  stride 0x38/表 0xb89c1b0 全中 + 与 bdevsw_add 镜像；native `_bdevsw_remove` km
  `0xB3A7968` 几何闭环同上）。**勘误**：无条件面 = `0x849f248`（apfs 反注册 fn
  0xaaedb8c），s64 工单所写 `0x849f240` 系墙 61 bdevsw_add 已绑槽之误。
  **修 = 3 面绑 native**：槽 `0x83ae510`→`_cdevsw_add_with_bdev`（词
  `0x80110000043A3D74`）+ 槽 `0x83ae490`/`0x849f248`→`_bdevsw_remove`（词
  `0x80110000043A3968`）。s64 手术（设备仓 scripts/s64-payload-surgery.py，动态枚
  举+面数断言+noctf→km delta 门）：**3 面 9B/3 簇零杂散，seal 未动**。
  **出件 `out/m2381/payload-r64.img`（99MiB）sha256 `b7535382…` 待刷**（recovery_a
  ← r64；回退一级 = r63 `0cb17660`）。账本 816→**818**（体 sha 在
  temp/s64-ledger-bodies.json）。**判读农场基建（本轮三件，报告 temp/s64-farm-report.md
  与 temp/s64-span-fix.md / s64-wall62.md / s64-wall63.md）**：A1 span bug 修复
  （旧"最小 span 者胜"退化→全部槽误归 corecrypto；新 s64-kext-buckets.py 按
  LC_FILESET_ENTRY 独立重定基 + DC 区间包含判属）→ **815 面/392 导入/27 桶清单**
  （temp/s64-kext-buckets.json；差 1 悬案溯源 = 槽 0x849fff8 高 16 位 0x8001 非
  form-A，823=form-A 口径/824=low48 口径）；farm manifest 392 导入体特征全覆盖
  （temp/s64-farm-manifest.json）+ IOStorageFamily 桶初评 **高 2/中 4/低 24**
  （temp/s64-farm-currentkext.json，中置信不绑守边界）+ tracer 复现 22/20/2
  （temp/s64-tracer.py）。**策略定谳**：纯体哈希/词级方法打不动此池（392 目标仅
  28 哈希命中、可绑级 1；已知孪生词等 topk 从未进前 3）——同 TU 锚点+体语义+
  caller 契约是主路径。中置信 4 条待人工复核（含 `_IOLockWakeup`：topk s_min=1.0
  + ledger 邻居 IOLockSleepDeadline+0x14、6 面）。
- **★会话六十三 = r62 首刷判读定谳墙61 = bdevsw 族 + 2 面绑 native `_bdevsw_add`
  r63 出件待刷（2026-11-06，本会话连轮）**：
  **r62 首刷判读**（payload-r62-2d3e8dd4）：crc `0xaf0e2d21` 零纠正（124B），t=1s，hit
  \#1。panic from `0xfffffff00a41cb04`（x0 `0x1` x1 `0xfffffff00bb651f8`）→ **墙 60 消实
  锤**（erandom 7 面生效；AppleImage4 nonce 过，推进到某驱动的设备注册）。**定谳链**：
  caller bl `0xa422054` → 标准 GOT thunk（adrp 0x83ae000+0x480）→ 槽 `0x83ae480` 哨兵 →
  donor `0xb3d1700`。语义 = **`bdevsw_add(1, 56B 模板)`**：x0=1=请求 major，x1=kext 自有
  bdevsw 模板（0xbb651f8，diversified-auth 指针数组），返回值与请求 major 比较 =
  bdevsw_add 契约。**定名**（devsw 域取证，TU=`bsd/kern/bsd_stubs.c`）：donor
  `0xb3d1700` = **`bdevsw_add`**（devsw CAS 锁 0xb9ce228 = donor `_cdevsw_add/_cdevsw_remove/
  _devsw_unlock` 同一全局；per-cpu mtx stats [tpidr+0x1c0]；bound 0x17=nblkdev−1；
  memcmp vs empty-bdevsw 占位 0x7ec2aa8 = 6×eno_\*+0）；donor `0xb3d161c` =
  **`bdevsw_isfree`**。几何铁证：donor bdevsw[24×56B]@0xb89c1b0 **紧邻** cdevsw[64×
  112B]@0xb89c6f0（donor `_cdevsw_add` stride 0x70/bound 0x3f/key 0x7ec2ae0）= conf.c
  bdevsw[]→cdevsw[] 布局。**修 = 2 面（0x83ae480/0x849f240）绑 native `_bdevsw_add`
  `0xB3A7810`**（noctf 0x79eb810 精确，体同构）；**isfree 槽 0x83ae488 已被 s61 696
  批量绑到同一 native `0xB3A7700`——独立交叉证实孪生判定**。s63 手术（设备仓
  scripts/s63-payload-surgery.py，按 donor 词动态枚举面）：词 `0x80110000043a3810`，
  diff 6B/2 簇**零杂散**，seal 未动。**出件 `out/m2381/payload-r63.img`（99MiB）
  sha256 `0cb17660…` 待刷**（recovery_a ← r63；回退一级 = r62 `2d3e8dd4`）。账本
  814→**816**（+bdevsw_add/bdevsw_isfree 双记录，体 sha 在 temp/s63-ledger-bodies.json）。
  **注**：native `_cdevsw_isfree` stride 0x70 = cdevsw(112B)、donor f 表 stride 0x38 =
  bdevsw(56B)，两表各归各勿混判；s63 手术 diff 期望须按面动态算字节数（哨兵低 48 位
  0x045A5C4C 的 byte3=0x04 与部分新词相同）。s64 可选项（待拍板）：395 集中 noctf
  精确 + 体验证的导入小批合绑（5-10 个/轮）以加速收敛，风险 = 难二分。
- **★下轮工作单 = 会话六十四（战略升级：静态引导道追踪 + 并行判读农场，终结打地鼠；用户授权激进多轮迭代）**：
  **r63 屏摄核验**（用户指示本轮不判读）：crc `0x68352ecd` 零纠正（124B）from
  `0xfffffff00a41cb1c`（x0 0x1 x1 `0xfffffff00bb65188`）→ **静态预测命中实锤**：s63 收
  队前从 bdevsw caller 前向扫描预读的下一墙 = bl 0xa41cb18 → 槽 0x83ae510 → donor
  `0xb3d1d58`（回退地址精确吻合）——**墙可静态预读，刷机只是验证不再是探路**。x1 =
  0xbb65188 = cdevsw 尺寸（112B）模板（比墙61 的 bdevsw 模板 0xbb651f8 恰低一表项），
  疑 cdevsw_add 族；donor 0xb3d1d58 落 bsd_stubs.c TU 区（源序 isfree<add<remove<cdevsw
  族），native `_bdevsw_remove`/`_cdevsw_isfree`/`_devsw_unlock` noctf 在册待对。
  A. **工具基建**（深度研究三件）：
     A1. kext 归属修复：GOT 页 → fileset entry（修 span 解析 bug：entry 内层段独立重定
         基），产出 = 815 面/392 导入按 kext 分桶清单。
     A2. **静态引导道追踪器**：从当前墙 caller 起 DFS bl→thunk→slot 链 + 本地函数跳转
         （有界深度），产出每 kext 导入触发序列 = 未来墙预告单（0xa41cb18 扫描 = PoC，
         一扫双墙：0x83ae510→0xb3d1d58、0x83ae490→0xb3d1894，均 sentinel）。
     A3. donor 导出 trie（0x4c50000 blob 0x5cc 无名串）与 kext 内层 bind（prelink 已剥）
         复攻 = 低优先级挂账。
  B. **并行判读农场**（多轮迭代引擎）：
     B1. 当前 kext（caller 所在，GOT 0x83aexxx 页）全部哨兵导入 → 扇出 agent 并行三证
         判读（管线已文档化：donor 体反汇编 → noctf/源码锚 → TU 几何 + 全局指纹 → 语义
         契约；范例 = erandom/bdevsw 双案录）；我复核抽查 → 合批绑定 → payload-r64 →
         刷机验证。
     B2. 每轮清一个 kext（或数个）；回退链逐轮滚动；账本逐轮增记（构建期价值累积）。
  B3. **s64 首批（预读墙已备）**：caller = donor memdev 注册（模板对 cdevsw 112B
         @0xbb65188 + bdevsw 56B @0xbb651f8 与 memdev.c:665-673 调用序逐行吻合；函数
         [0xa41ca8c,0xa41ce70) 22 调用中 20 已绑全绿）。墙 62 = donor `0xb3d1d58` =
         **`cdevsw_add_with_bdev`**（体首 bl donor cdevsw_add + 3 参 (1, csw, bdev)；
         **native 无此符号（单调用点被内联）→ s64 首个真判读题**：直绑 native
         `_cdevsw_add` 0xB3A7B3C（bdev 关联簿记缺失，源码在手裁决）vs 源码重构）。
         墙 63 = donor `0xb3d1894` = **`bdevsw_remove`**（体首 cmp w0,#0x17 + stride
         0x38 + 表 0xb89c1b0 三重几何指纹；条件墙——cdevsw_add_with_bdev 成功路径跳
         过；另面 0x849f240 无条件 caller）→ 直绑 native `_bdevsw_remove` 0xB3A7968
         就绪。native bsd_stubs TU 全家福（noctf）：isfree 0xB3A7700 / add 0xB3A7810 /
         remove 0xB3A7968 / cdevsw_isfree 0xB3A7A2C / cdevsw_add 0xB3A7B3C /
         cdevsw_remove 0xB3A7C9C。
  C. **emit go/no-go = 降级挂账（2026-11-06 裁决）**：emit 解决绑定机制，而现瓶颈 =
         判名（emit 同样只能绑账本内名字，392 未名面 emit 也救不了）；手术绑定已证
         廉价确定（s63 = 6B/分钟级）。**复触发条件 = 第一次因任何原因需要 Mac 重编**
         （如 shim 需求）——届时顺带开 emitter 让该次构建自绑全部账本名，一次成本双
         收益。此前不再投入。
  D. **边界**：无证据不绑——近匹配自动评分已判死（7 对已知孪生校准相似度仅
     0.08-0.24，donor 27.0.1 与 feed 编译分歧过大）；每面仍需三证，agent 产证、人复核
     后绑定；T1 精确体哈希猎手产出 ≈ 0（temp/s64-twinhunter.py）亦留档。
- **★下轮工作单 = 会话六十五（r64 判读 + IOStorageFamily 清桶轮）**：
  A. **r64 刷机判读**：预期墙 62/63 双消——caller 注册函数 22 调用全绑后该注册路径
     应放行；成功旗标 = 新 panic 来自**不同 caller**（≠0xa41cb1c 一族）或推进（crc
     必为新值）；若仍 0xa41cb1c 族 → 检查同族第 4 墙（cdevsw_remove/cdevsw_isfree
     的 caller 是否被注册失败路径触达）。回退一级 = r63 `0cb17660`。
  B. **IOStorageFamily 清桶**：桶剩 29 面/28 导入（farm currentkext.json 已备初评）；
     高置信 2 条已随 r64 绑毕；中置信 4 条人工复核后决定绑否（`_IOLockWakeup` 6 面
     = 头号候选：topk s_min=1.0 + ledger 邻居 IOLockSleepDeadline+0x14 + 语义契约
     唯一性）；低置信 24 条按"下一墙优先"逐个三证（s64-tracer.py + farm manifest
     体特征为现成产证底稿）。
  C. **全量推进序**（kext-buckets.json 桶面数）：apfs 155 / AppleARMPlatform 70 /
     sandbox 63 / IOGPUFamily 61 / IOHIDFamily 52 / AMFI 43 / IOAVFamily 40 /
     IOSurface 38 / ApplePMGR 37——每轮清一桶，静态预读下一墙后再刷（刷机只验证）。
  D. 边界不变：无证据不绑；账本逐轮增记；回退链逐轮滚动。
- **★会话六十二 = r61 首刷判读定谳墙60 = read_erandom 族 + 7 面绑 native `_read_frandom`
  r62 出件待刷（2026-11-06）**：
  **r61 首刷判读**（payload-r61-1795d6bb）：crc `0xa9474625` 零纠正（125B），t=1s，hit
  \#1。panic from `0xfffffff0090261b8`（x0 `0xfffffff00ba64b19` x1 `0x10`）→ **墙 59 消实
  锤**（696 批量 9,134 面全生效；corecrypto kext start 注册 PRNG 后推进进 AppleImage4
  nonce manager）。**定谳链（全程证据闭环）**：caller bl `0x9028f44` = 环境对象回调派发
  thunk（`ldr x16,[x0,#0x100]; mov x17,#0x12f7; braa x16,x17`；对象 = 运行时填充的
  __DATA 注册表）→ 回调 = **ctx-drop 适配器 `0x902576c`**（`mov x0,x1; mov x1,x2; b
  0x903a898`，(ctx,buf,len)→(buf,len)）→ AppleImage4 GOT thunk 0x903a898 → 槽
  `0x80e1a40` 哨兵。语义 = **16 字节随机读取**（nonce；x0 = struct+5 内嵌 16B 缓冲，
  x1 = [x10+0x48] 长度字段）。**定名**（donor 导出 trie 不可解码；source 锚 + 体取证）：
  donor `0xae9bd20` = `read_random_generate`（blraa 经 `prng_generate` 函数指针全局
  [0x7dff0a68] + 每核互斥 tpidr_el1 + 0x100 分块）；donor `0xae9bc68` = `read_erandom`
  （u32 len + `prng_ready` 门 [0x7dff0a80]）；兄弟 `0xae9bbac` = `read_erandom_generate`
  （16K PAGE_SIZE 分块；map how=string 记录独立佐证 native 0x7507818）——TU =
  `osfmk/prng/prng_random.c`（墙 55 fips 门同文件）。**修 = 7 面绑 native
  `_read_frandom` `0xAEC2BF8`**（noctf 0x7506bf8 精确；源码 read_frandom{read_erandom}
  = 门+(prng_ready?PRNG:DRBG) 语义超集；native 体近指令级同构 = 同门+同 0xedeb per-cpu
  pacia 全局+同 0x100 分块；native 无 `_read_random_generate`/`_read_erandom` 符号
  （静态内联）故 `_read_frandom` 为正解；xnu-12377 车道的 fileset-read-erandom-shim
  不在 27 车道亦不需要）。**s62 手术**（设备仓 scripts/s62-payload-surgery.py）：槽
  0x80e1a40/0x8316e80/0x833c2e8/0x8479c28/0x849fa18/0x84ac9d8/0x848d5e8 → 词
  `0x8011000003ebebf8`，pre-image 白名单+donor 词断言全中，diff 28B/7 簇**零杂散**，
  seal 未动。**出件 `out/m2381/payload-r62.img`（99MiB）sha256 `2d3e8dd4…` 待刷**
  （recovery_a ← r62；回退一级 = r61 `1795d6bb`）。账本 812→**814**（+read_erandom/
  read_random_generate 双记录，体 sha 在 temp/s62-ledger-bodies.json）。
  **对质裁决（工作单 B 项）**：s61 冲突 `0xad8f628`（map=`_task_reference` vs
  账本=`_task_reference_external`）**操作上化解**——noctf 两名同址 `0x737d884`
  （别名/ICF 合并），绑定目标唯一 = native km `0xAD39884`，s63 直绑无需裁名。
  **工具警示**：map 稀疏（仅 matcher 映过函数），近邻命名不可靠；**395 未定名集已可
  用 `temp/s62-sentinels.py` 全枚举**（824 槽/395 导入，donor 词→map 近邻名为**暂定**
  需逐个三证；824 vs r60 记账 823 差 1 未溯源）。
- **★下轮工作单 = 会话六十三**：A. 刷 r62 判读（CRC 对拍 → 分流：新哨兵墙 →
  s62-sentinels 全名单对号 + 三证顺带修；PAC/数据 abort → 疑 `_read_frandom` 绑定 →
  回退 r61 二分；无墙推进 → stage 轨迹/vfsinit/root mount/SHLOG state=2）。B. 小批队
  列：`0xb47008c` publishResource 3 面 + IOCrypto 13 三证 + `0xad8f628` 5 面（绑
  `0xAD39884`）+ 395 残余。C. emit 同输入试验 go/no-go（kit `96eedc2` synced fixture +
  RELEASE reference）不变。
- **★会话六十一 = 696 定名导入批量手术 r61 出件待刷（2026-11-06，跳过 r60 直刷 r61）**：
  **口径定谳**：工作单 696/9,139 精确复现 = **r52 期 24A446 matcher map**
  （`out/m2381/ipados-27.0.1-24A446/m2381/kernel/kernel.map.json`，v11，
  inputs `38e6d9fa`=本地 donor）∩ noctf 同名同 VA 精确 ∩ r60 payload
  form-A donor 词。**★`out/m2381/kernel.{map,json}` = 23F84 旧线残留**
  （w40 时代，键空间 0xa76c000-0xb01d12c，inputs `0f7d50ac` ≠ 本地
  donor）——已弃用勿再引。**★词形两族定谳**：kext 导入面 = form-A 压缩
  词（`0x8011<<48|(VA−0x7004000)`，diversity=0）；diversified auth 词 /
  非 auth rebase 词 / 清零位 / 内核自体 fixup 位点 = emitter 管理态非导入
  面（r60 内实测 84/129/253/1,825 个，全分类落盘 surgery.json）。
  **手术 = `UniXNU-m2381/scripts/s61-payload-surgery.py`：695 导入 /
  9,134 面全重写**（donor 词 9,134 + sentinel 0；冲突 VA `0xad8f628`
  map=`_task_reference` vs 账本=`_task_reference_external` 5 面按规矩
  排除；账本外残余 `0xb47008c` IOService::publishResource 3 面 → s62
  队列）。门全过：计数 696/9,139 精确、native km 全在
  __TEXT/__TEXT_EXEC、seal 未动、diff 27,449B/9,135 簇**零杂散**。
  孪生抽验：body-hash 通道 74 条中 52 条 donor≡native 字节同，16 差异
  全为 allocator 相关 C++ 法（r55/56 补丁重编译所致，语义等价；版面
  稳定性由 4 个过墙符号背书，名单 `temp/s61-bodyhash-diff.json`）。
  **出件 `out/m2381/payload-r61.img`（99MiB）sha256 `1795d6bb…` 待刷**。
  账本 165→**812**（净增 647，双证 = matcher how + noctf 精确，体界
  (d') 约定 = 首 4 对齐 ret/retaa/retab 或离体 b 截断，金标准 10/10、
  165/165 raw-sha 复核）；kit 仓本地 commit `96eedc2`（主树子模块指针
  未动）；设备仓 `3bc999d`。
  **C 侦察 8 条**（`temp/s61-emitter-recon.md`）：live emitter = 设备仓
  vendored fork（feeds shadow 掉主仓 submodule，v11）≠ 上游 pin
  b1888c1；adapter branches 上游有我们无；fileset-reference 管线化只有
  我们有。
- **★下轮工作单 = 会话六十二（判读 + 决断轮；用户会前刷 r61）**：
  **刷机策略**：只刷 recovery_a ← `payload-r61.img`（dd 读回对拍
  count=99；现态 recovery_a = r59 `72cc9ef4`，iOS 链未被动）。回退：
  r60 `7da7e0b3`（一级）/ r59 `72cc9ef4`（二级）/ r58 `ca755a65`（三级）。
  A. r61 首刷判读：CRC 对拍（--autofix）→ 分流：
     a) 新哨兵墙（预期落 395 未定名集或 IOCrypto 13）→ 三证定名顺带修；
     b) 数据 abort / PAC → 疑 696 批量错孪生 → 16 条 body-hash DIFF
        名单（`temp/s61-bodyhash-diff.json`）优先排查 → 刷 r60 二分定位；
     c) 无墙推进 → 看 stage 轨迹 / vfsinit / root mount / SHLOG state=2。
  B. 遗留小批：`0xad8f628` 冲突裁决（`_task_reference` vs
     `_task_reference_external` 体哈希对质）+ `0xb47008c` publishResource
     3 面 + IOCrypto 13 三证 + 395 未定名集。
  C. emit 同输入试验：synced fixture（kit `96eedc2`）+ RELEASE reference
     跑 emit，报 stubbed/trap 未决集 vs 基线（build 期 1,850 stub ref /
     payload 823 sentinel 面），终点参照 sheng 终态 2 data + 24 trap →
     emitter 路线 go/no-go（侦察 D1 live-fork / D5 adapter-branches /
     D4 reference 管线化为决策输入）。
  D. r63 排单：go → emitter 全配置出件（构建期重绑取代 payload 手术线）；
     no-go → 手术线继续（IOCrypto 13 + 395 残余 + sheng 板级件收编）。

- **★会话五十九 = r59 首刷判读 + 墙 59 定谳（kalloc_type_impl_external，AppleImage4/AMFI 双家）+ r60 批量手术出件待刷（2026-11-06）**：
  **r59 首刷判读**（payload-r59-72cc9ef4）：crc `0x719ca2b6`（autofix 补
  from/x0 各一个前导 f，零数字错），124B，t=1s，hit #1。panic from
  `0xfffffff009023d34`（x0 `0xfffffff0080e16d0` x1 `0x4`）→ **墙 58 消实锤**
  （IOLockAlloc 134 槽 + OSObject ctor 292 槽全部生效，推进过 IOCrypto
  ctor）。**定谳 = caller com.apple.security.AppleImage4**（老 21H461 车道
  墙 10 同族！）：bl `0x903a698`（auth-GOT thunk）→ 槽 `0x80e1940` =
  哨兵 → donor import `0xad3e380`；x0 = **kalloc_type_view**（name 串
  `site.nonce_magazine_t`@0x751da8e + site 位图串@0x751daa4，kt_flags
  0xf8@+0x28，kt_size 0x64@+0x2c），w1 = 4 = zflags。**注意：本墙不在
  s58 预测的 IOCrypto 13 导入之列**——另一 kext（AppleImage4，AMFI 同调）
  的 kalloc_type 先撞线；IOCrypto 13 仍是下一梯队。
  **修 = donor `0xad3e380` 直绑 native `_kalloc_type_impl_external`
  `0xace8c8c`**（noctf `0x732cc8c` 符号精确、与 `_kalloc_type_impl` LTO
  别名同址；三证 = ①noctf 符号精确且紧邻 r58 已证 `_kfree_ext
  0xace8d18`（同 kalloc.c TU）②语义体形全对（0xd01000 常量对同对、
  zflags&7 折入同构 bfxil≡orr、ktv +0/+0x28/+0x2c 读法同、尾调
  `_kalloc_type_impl_internal` donor 0xadb2914 ≡ native 0xace9214 别名证实）
  ③stubbed 账本 AppleImage4+AppleMobileFileIntegrity 双家在册）。
  **★坐标系补充定谳：kernel.map.json `native` 字段 = noctf 空间 VA**
  （0xfffffff0 前缀），km VA = noctf + `0x39BC000`（s57 kfree_ext/
  s58 IOLockAlloc 双坐标验证）。
- **r60 = `UniXNU-m2381/scripts/s60-payload-surgery.py`：10 导入 44 槽**
  （墙 59 import 8 面 + matcher 账本（body-hash，noctf 交叉复核过）已定名
  9 导入 36 面：OSUnserializeXML 18 面 / IOFreeTypeImpl 8 面 /
  kauth_cred_nfs4domain2dsnode 3 面 / OSCollection::init、buf_biowait、
  buf_invalidateblks、cluster_unlock_direct_read、vfs_iterate、
  vn_authorize_unlink 各 1 面）。pre-image 白名单全中，diff 审计
  143B/43 簇**零杂散**，seal 未动（`__fips_hmacs`@0x4ec5fd 仍
  `6fc44336fd3f44b7`）。**出件 `out/m2381/payload-r60-7da7e0b3.img`
  （99 MiB）待刷**。账本 records 156-165 入
  `kits/XnuPrelink/fixtures/fileset-24A446-symbols.json`（165 条）。
- **IOCryptoAcceleratorFamily 残余哨兵导入（坐标已采未修，排在 696 批量与
  emitter 线之后）**：
  `0xb49c874` / `0xb4e7140` / `0xb4e670c` / `0xb4e6764` / `0xb4a6a20` /
  `0xb4a6ae8` / `0xb4a0528` / `0xb4a0580` / `0xb46c278` / `0xb46c2d0`
  （IOCEC 对）/ `0xad7e344` / `0xb29df7c` / `0xaec5fe0`（13 个，matcher
  账本全未定名，需逐个三证）。DC 区 sentinel 面 r60 后 = **823**
  （395 import；其中 696 已定名 import 的 9,139 面 = donor 词隐患，r61
  批量带走；两者不相交）。
- **★会话五十八 = 墙 58 定谳（IOLockAlloc 134 槽）+ OSObject ctor 翻案（292 槽全量）+ r59 手术出件待刷（2026-11-06）**：
  **r58 首刷判读**（payload-r58-ca755a65）：crc `0xfcd9b111`（autofix 仅补 x1
  一个前导 f,零数字错）,139B,t=1s,hit #1。panic from `0xfffffff00a0aac38`
  （x0 `0xfffffff00bb3de18` = IOCrypto kext 数据段静态对象 = 调用方 ctor 的
  this 残留,x1 `0xfffffff3facb9d20`）→ **墙 57 消实锤**（r58 的 453 槽 + 2 shim
  全部生效,推进进 IOCryptoAcceleratorFamily ctor）。**定谳 = donor
  `0xb4424c4` = IOLockAlloc 变体**（bti; adrp x8,0xb929000; ldr
  x0,[x8,#0x330]; b donor lck_mtx_alloc_init `0xad43134` = **弃参默认组包装**,
  IOLockAlloc() 无参 ABI）。调用链：IOCryptoAcceleratorFamily ctor
  `bl 0xa0ab660`(auth-GOT thunk) → 槽 `0x8313b60` = sentinel。
  **修 = 直绑 native `_IOLockAlloc` `0xb416a00`**（noctf `0x7a5aa00` 符号精确：
  ldr [0xb935450]; mov x1,#0; b native lck_mtx_alloc_init `0xaceffe8`（r58 已证
  孪生）; 邻位 `_IOLockInlineInit` 同全局）——免 shim。
- **★坐标系定谳（工具级）**：24A446 车道 slide = `0x1f000000000`（kc.py 注记
  本就对;本轮一度误判 0x100000000000 系手算翻车）。payload kernel.macho =
  donor KC 全局重定基,**kext GOT 位置等同**（donor km 偏移 == payload km 偏移）,
  屏 VA = payload VA 同空间。donor kernel exec 在 payload 里被 native kernel
  **整体替换**。
- **★翻案 = s57 "0xb3d385c 267 存活槽 donor 拷贝部分可用" 死**：payload
  `0xb3d385c` = native `_OSKextFreeSite` 邻域代码（fn@`0xb3d382c`）,donor 词槽
  指向我们 native 随机代码 = 潜在乱调,不是"可用拷贝"。→ **0xb3d385c（OSObject
  默认构造器,pacda isa 0xcda1 上下文）292 面（267 donor 词 + 25 哨兵）全量重绑**
  native `__ZN8OSObjectC2Ev` `0xb3aba18`（noctf `0x79efa18` + 0x39BC000;s45 墙36
  语义前例）。本 kext ctor 在 IOLockAlloc 之后紧跟 OSObject-ctor thunk
  （0xa0ab730 → 槽 0x8313bc8）,故同轮带上。
- **r59 = `UniXNU-m2381/scripts/s59-payload-surgery.py`**：134 + 292 = **426 槽**
  （IOLockAlloc 词 `0x8011000004412a00`;OSObjectCtor 词
  `0x80110000043a7a18`）,全部 pre-image 白名单（donor 词/sentinel）命中,diff
  审计 426 簇/1278B **零杂散**,seal 未动（槽在 collection __DATA_CONST,在 fips
  HMAC 盖的 entry __text 之外;`__fips_hmacs`@0x4ec5fd 仍 `6fc44336fd3f44b7`）。
  **出件 `out/m2381/payload-r59-72cc9ef4.img`（99 MiB）待刷**。账本 records
  154/155 入 `kits/XnuPrelink/fixtures/fileset-24A446-symbols.json`。
  **工具坑（本轮新增）**：手术偏移必须 +KHEAD `0x1c9800`（ESP 头）;
  bytearray 切片是拷贝（要 memoryview 原位写）;48 位压缩词手算必错,一律程序算。
- **IOCryptoAcceleratorFamily 残余哨兵导入（同一 init 路径,r60 第一梯队,
  donor 坐标已采）**：`0xb49c874` / `0xb4e7140` / `0xb4e670c` / `0xb4e6764` /
  `0xb4a6a20` / `0xb4a6ae8` / `0xb4a0528` / `0xb4a0580` / `0xb46c278` /
  `0xb46c2d0`（IOCEC 对）/ `0xad7e344` / `0xb29df7c` / `0xaec5fe0`（13 个;
  全部经本 kext thunk 表 0xa0ab710-0xa0ab920 可达）。
- **★下轮工作单 = 会话五十九（r59 首刷判读 + IOCrypto 残余 13 导入预研，安排于 2026-11-06 收队时，只写单不执行）**：
  **A. 首刷判读主线（用户刷 r59 `72cc9ef4` 后）**：
  0. 收屏；CRC 对拍（verify-panic-crc --autofix）；t/stage/hit# 与 r58 对拍。
  1. 读回自检表（上条 d 项）+ reader dd 读回对拍（dd 读回才算证据）。
  2. 分流：
     a. **无哨兵墙**（可能性低——本 kext 残余 12 导入大概率下一站）→
        IOLockAlloc + OSObject ctor 通关实锤,直接推进:fipspost POST 完整跑
        → PRNG 登记 → PE_init_iokit → AMFI（t≈32s IORTC 税）→ No-PMGR →
        PENDING 站表 0x924e77c/ef1c/ef5c。
     b. **新哨兵墙**（第一预期,caller 大概率仍落 IOCryptoAcceleratorFamily）
        → 照哨兵流程：bl 定位（尾跳类 = LR 可能保留上游）→ thunk → 槽 →
        donor 原词 → 三证定名 → 修;若 donor 词 ∈ 下面 B 单清单 = 预研已备,
        直接按钉死修法施工。
     c. **数据 abort / PAC 类** → 复核 shim 常量（kfree_ext `0xace8d18`,
        KHEAP_DEFAULT `0x7dec3e0`）与 braa 鉴权形（ESR EC 0x1C）。
  **B. 预研单（不等刷机结果,会话开头即可跑;s57→s58 模式）**：
  对 IOCryptoAcceleratorFamily 残余 13 个 sentinel 导入逐个三证
  （donor 坐标,全部经本 kext thunk 表 0xa0ab710-0xa0ab920 可达）：
  `0xb49c874` / `0xb4e7140` / `0xb4e670c` / `0xb4e6764` / `0xb4a6a20` /
  `0xb4a6ae8` / `0xb4a0528` / `0xb4a0580` / `0xb46c278` / `0xb46c2d0`
  （IOCEC 对,候选表旧行:体首 `mov w8,#1; str w8,[x0,#8]` 置标志+GOT 调用,
  186×2 面）/ `0xad7e344` / `0xb29df7c` / `0xaec5fe0`。
  步骤（每个）：
  ① donor 体形：bounds + disasm（temp/s58-wall.py 反汇编模式,donor exec
    fo = 0x3cb0000 + (va − 0xacb4000)）；下一步定体边界（下个 bti c /
    pacibsp,冷路径块算同函数——OSObject ctor 536B 前例）。
  ② noctf 孪生：体形扫描 + 符号表（temp/s53-noctf-syms.json,noctf VA =
    placed − 0x39BC000;先试符号精确命中再退体形搜索）。
  ③ 修法三分类：native 孪生直绑（词 = 0x8011<<48|(twin−0x7004000),程序算,
    勿手算）/ 需镜像 shim（页 padding 0xaef08xx 系）/ 记档不修。
  ④ 有钉死修法者做面数枚举（donor __DATA_CONST 词扫描,s59 脚本枚举器复用）,
    预写 r60 病灶表行。
  产出 = r60 候选病灶表增补（钉死修法 + 词 + 面数）,撞墙即施工。
  `0xae9bd20` 特别注：需 donor chained fixups 解析（donor 零槽 `0x7dffa68`
  blraa,文件即 0）——侦察成本高,排在 13 导入之后;信息真空先报成本再动。
  `0xb441738`（114 面 meta 表派发尾块）同样殿后:待屏证 x0/x1 才好定名。
  **C. 施工与出件（判读或预研钉出新修法后）**：
  s60-payload-surgery.py（s59 模板;坑:+KHEAD `0x1c9800`、memoryview 原位写、
  pre-image 白名单三分类、diff 审计零杂散、seal 断言未动）→ 读回自检表 →
  账本 records 156+（设备仓 kits/fileset-24A446-symbols.json）→ STATE §0 →
  双仓 commit+push → 刷写卡（常规迭代轮只 recovery_a,sha 门 + dd 读回）。
  **D. 成功旗标**：vfsinit → root/cryptex mount → SHLOG state=2。
  **E. 回退**：recovery_a ← r59 `72cc9ef4`（一级）/ r58 `ca755a65`（二级）。

## 归档自 STATE §0（2026-11-07 会话七十三 首更时移出；块内 s72 审查轮/s71 同步/s69-s67 各轮出件均已被 r72 新基座取代，判读结论以 r72 判读与各记忆条为准）

- **★★会话七十二 = 审查轮定谳（2026-11-07）**：用户四项拍板：**A 留 24A446**；**先刷 r71**（跳过判读等待，union 红利留 r72）；**kit 反哺先不推**；**哨兵扩容 r73 单独轮**（不与 union 首件混变量）。
  **★根因修正（2026-11-07 深夜终版，推翻本会话早前"全 referenceless"论——用户指正+Mac out 树实测）**：
  ①闪刷链基座 payload-r55(dd51ff60) = **Mac 11-05 09:04 带参照 emit**（matcher 11 / reference `713f8f91…` / kernel e64448dd，Mac out 树实测）；r56-r71 全部是对这个**带参照**基座的 payload 手术，Linux r52 无参照件（kernel.json None）是没进闪刷链的另一版。
  ②参照件 24A446 LTO 版**在 Mac 上已存在**（`tree/build/xnu/fileset-reference/lto-b19d07e101904a5f`，nm 实测 `_IOMalloc/_kfree_data/_IOFree` 三符号俱在、omitted 清单 67 条无 wall 家族）——但 **r55 map 对 wall 家族绑定为 0**（map 9471 对：reference-order 4338 + reference-symbol 1489，**reference-shape 仅 1**）→ 墙 56-71（IOMalloc/IOFree/kfree_data 族）照样逐个发生。
  ③**责任账（用户指正，认）**："没必要刷 Mac 构建的镜像"系此前会话助手判断，payload 手术一直叠在旧基座上；本会话审查在未查 Mac 侧情况下错判"全 referenceless"。教训：手术轮次锁死基座；审查必须双机对账；"没必要"三个字不能拦已构建的替代件。
  ④**r72 真杠杆 = 诊断参照通道为何漏掉 wall 家族**（reference-shape=1 是异常信号；嫌疑=通道种子传播门控 / kext 导入路径未查参照符号表 / 参照件与 donor 的 LTO 内联分歧超通道容差）→ 再 108c 源重建参照 → 重 emit → 839 账本照常兜底。上游 156 残余天花板参照不变。
  **审查账目（一手）**：上游 24A446 账本 156 fn+29 data **全部 ⊆ 我们 839+36，native 指认零分歧**；墙驱动条目 ≈163 ≈ 156（同量级互证）；check-m2381 **4 ok/0 fail**；merge-base = upstream tip 10edc1ef **零滞后**。已知 upcoming 雷：stack-shift site（PAGE_SHIFT_CONST→DER 迭代器 4K 页假错 "copyDictionary failed with 14"，sheng 系板 patch 符号，我线未绑——判读遇此直接按案卷绑 15）。
  **★r72 工作单（下会话执行；用户拍板 = Mac 侧构建 + 先查文档里的完整 Mac 构建/拉回方案照抄，别重造）**：
  **Mac 侧资产清单（本会话实探）**：连接 `naoniudeair@192.168.10.246`（免密 id_ed25519），工作区 `~/unixnu/`；非交互 ssh 必须 `PATH=/opt/homebrew/bin:$PATH` 前缀（STATE-history 4036 坑）。主树 @b130f79（落后 ~85c）+ **3389 行未提交 s44-s51 工作**（含 `prelink-regression.json` +1336 载重件、build-xnu-4k-baseline.sh、patches.py——先 diff 确认后提交/保存，**再** hard reset 到 origin/m2381-ios27）；设备仓 checkout 在 `~/unixnu/UniXNU-m2381`。信任缓存 `~/unixnu/original-trust-24A446.bundle`（sha8 dc655bcb）；**data seed = sha `d04a0153…c8486a`（=r62/r85 记录同一种子）两机均未找到实体**——按 sha 找或 `make-data-seed.py` 重造（hdiutil Mac only）。Mac 11-05 资产：payload dd51ff60（r55 基座本体）/kernel e64448dd/map 9471 对/参照件 LTO 缓存 `lto-b19d07e101904a5f`。
  **序列**：⓪**先诊断**（比重建更值钱）：拿 r55 map + 参照件 nm 表对照，查 matcher 参照通道为何 wall 家族绑 0（reference-shape=1 异常；fileset_match.py 的通道种子传播门控 / kext 导入路径是否查参照符号表）；①拉两仓 + `git submodule update --init --recursive`（ArmCpu 已拨 93a5030，QcomBaseband 新增）；②`./unixnu kernel --skip-prelink -j 8`；③`python3 scripts/build-fileset-release-reference.py --tree out/m2381/ipados-27.0.1-24A446/m2381/tree --lto`（108c 源变缓存必重编）；④`./unixnu prelink`；⑤**出件前闸门**：kernel.json `reference_sha256` ≠ null + **map 对 `_IOMalloc/_kfree_data/_IOFree` 绑定数 >0（诊断生效判据）** + 墙72 靶 `0xfffffe000b4f1304` named+bound + stubbed 对 r55 基线（28 镜像/1030 项）塌缩；⑥`./unixnu payload`（三红线旗标；seed sha 上面）→ 按**文档里的拉回方案**传回 Linux `out/m2381/` 平铺路径。
  **Linux 侧已完成前提**（均已推）：data 账本 7→36（设备仓 vendored 权威 839+36，2203d75）、ArmCpu 幽灵钉修复（64bda2d0）、判读/STATE（db815518+）。kit 代码谱系对账顺延 r73。
  **r71 首刷判读（2026-11-07 03:43 屏摄 IMG_20261107_034317）**：crc `0x71c3aef3` 蛮力单字符仲裁**唯一命中**（from/x0 各掉一 f，标准噪声）。报点 `0xfffffff00aa5ad64`（x0 `0xfffffff008488670` x1 `0x18`）hit#1 t=1s。site `0xaa5ad60`（sandbox span `[0xaa46c70,0xaa81594)` 内）≠ 墙69 `0xaa5a2e4`/墙70 `0xaa75010`/墙71 `0x92029c8` → **六槽绑定全生效、墙71 消**。**墙72（靶已解，名待参照）**：bl→GOT 槽 `0x848d0f0`→SENTINEL→donor `0xb4f1304` = `IOUserClient::dispatchExternalMethod`(0xb4f10d4) 后继函数起点——体界实锤：BTI C 入口 + 前函数 `b` 尾跳收 `0xb4f1300`；体 +0x10 `cmp x1,#0x18` 与 panic x1 自洽；0x38 字节短叶返回 `kIOReturnBadArgument`(0xe00002c2)/csel +0x13 变体 + ldaxr 全局原子。donor 布局≠noctf（体 0x38 vs noctf externalMethod 0x460），名不手定——**r72 参照 emit 自动命名+绑定，不手术**。
- **★会话七十一 = r70 首刷判读（7 槽全生效）+ 墙71 定谳（AppleMatch kfree_data 族）+ 6 槽批绑，r71 出件待刷（2026-11-07 凌晨连轮）**：
  **r70 首刷判读**（payload-r70-ecf23f32；屏摄 IMG_20261107_014846 01:48）：crc `0xb64e1097`
  单命中零纠正，hit #1，t=1s。报点 `0xfffffff0092029c8`（x0 `0xfffffff802dcc0c0` x1 `0x35`）。
  **site ≠ 墙70 的 0xaa75010 → 同 payload 必同 site 判据 → r70 七槽绑定全部生效、墙70 消**。
  **新 span = com.apple.kext.AppleMatch**（text [0x9200c20,0x92032a8) +0x1da4，s64 桶表）——
  AMFI/sandbox 的 entitlement 正则 kext，与墙 69/70 同一条执行线（entitlement 匹配流水）。
  **caller 实据**：x0 = `0xfffffff8_02dcc0c0` = **内核堆银行指针**（高 32 位 0xfffffff8 vs 文本段
  0xfffffff0——新情报：本 runtime 堆在 0xfffffff8 段），x1 = 0x35 = **size 53** → kfree_data(addr,53)。
  **span 普查**（s70-applematch-prescan.json，tracer 51 bl）：8 哨兵收敛 **2 靶**：`0xad3e8e4`×7
  站（槽 0x8126030）+ `0xad3e8fc`×1（槽 0x8126038）；**0xad3e8e4 同时 = s66 AMFI 普查二号靶**
  （12 站槽 0x8136108）+ s64 farm 118 行预判 `_kfree_data` topk 0.833（槽 0x83ae580）——三案卷
  交叉同靶。
  **墙71 定谳**：donor `0xad3e8e4` = **`_kfree_data`**、`0xad3e8fc` = **`_kfree_data_addr`**。
  四证：①donor 体与 native **逐指令同形**（0x18B/0x14B 包装器：mov x2,x1; mov x1,x0; adrp+add
  x0=kheap 全局; 尾跳 kfree_ext/kfree_addr_ext——仅各自 kheap 全局 0x7f08e00 vs 0x7dec3c0 与
  尾跳目标异）；donor kfree_data 尾跳 0xad3e43c = **map 精确 `_kfree_ext`**（native km 0xACE8D18
  同 noctf 符号=几何闭环）；②**族几何 donor/native 全同**（kfree_data +0x18 kfree_data_addr
  +0x2c kfree_shared_data；donor map 夹逼 _IOFreeTypeImpl−0x50…_kfree_shared_data+0x2c 间隙恰容
  双靶）；③noctf 精确（_kfree_data 0x732d360/km 0xACE9360、_kfree_data_addr 0x732d378/km
  0xACE9378）+ s64 farm 同 km 预判；④源锚 osfmk/kern/kalloc.h:521
  （kfree_data(elem,size)=kheap_free(KHEAP_DATA_BUFFERS,elem,size)）+ kalloc.c:2866。
  **修 = 2 靶 6 槽**（kfree_data：`0x80e1960/0x8126030/0x8136108/0x8333818/0x83ae580`→
  `0x8011000003CE5360`；kfree_data_addr：`0x8126038`→`0x8011000003CE5378`）——**一靶同时清
  AppleMatch×7 + AMFI×12 + 3 其他镜像**：设备仓 scripts/s71-payload-surgery.py，24B/6 簇零杂散，
  seal 未动，6/6 回读解码验证。**出件 out/m2381/payload-r71.img sha256 `ddbaa7b9` 待刷**。
  账本 827→**829**。
- **★下轮工作单 = 会话七十二（★审查轮，用户拍板 2026-11-07"下轮会话做审查"；激进多干授权不顺延）**：
  A. **同步完整性审查**：rebase 108c 对我们关键面的 diff 审计（profiles / payload.py / patches/xnu
     series / kit emit 代码——b6afb4b "bang root" emitter 新特性对我们出件流的影响）；kit 并集
     f9123a3 vs 设备仓账本一致性核对（应 839=839）；AdrenoMetal pin 6416a76e（GPU parity 线）对
     我们无 GPU 树的影响评估。
  B. **构建前静态门**：check-m2381 + 出件 dry-run（839 账本后 expected 哨兵变化数预估，对齐
     全图普查 778 槽/379 靶基线）；release-reference 与新补丁系列咬合验证。
  C. **r71 判读**（若已刷）：crc 必新；caller 允许 sandbox/AMFI span；**槽必非账本 839 全量**；
     同 payload 必同 site 纪律继续。
  D. **方向拍板材料**（用户三问完整账目，供 A/B 拍板）：A=留 24A446 提速（执行线 ~83 靶，家族
     自动解析 pass + 农场 3-5 靶/轮 ≈ 几十轮）；B=回 21H461 成熟线（上游 alioth 同款配对，账本
     三次清零的解药，代价=弃 iOS27）；侦察结论已入档（sheng=SM8550 同 SoC、config 假说排除、
     快因=板级成熟+全职早 3 天）。附两项待拍板：**哨兵扩容**（kext 归属名+x2/x3+槽地址，代价=
     fileset patch 改+全链重出+CRC 配方换版）、**kit 反哺推送**（f9123a3→上游 UniXNU/XnuPrelink
     main，让全生态跳过我们 700+ 条墙）。
  E. **卫生**：temp 大件隔轮清拍板（s6x-* 普查大 JSON）；备份分支 pre-sync71 保留至 r72 出件后。
- **★s71 同步+侦察（2026-11-07 用户令"先侦察，同步上游"）**：主树 rebase upstream/main **108c**
  （QcomDwc3 USB 驱动组件 + 蓝牙原生栈线 + root union fsid 修复 + iPad17,3 board_id/security_domain
  修复 + archive 可重现 + addon/UART runtime 修复；冲突三处按规矩解：AdrenoMetal pin 跟上游 GPU
  parity 线 / README 还上游 / kit gitlink 取我侧后统一 bump）。kit 仓并集 **f9123a3 = 839 条**（我们
  827 unique + 上游独有 12 条零冲突，同 VA 同 native 两家互证；上游独有 = IOService Start/Stop_Invoke
  RPC + OSSerialize setIndexed + mbuf 三兄弟 + cdevsw_setkqueueok + ttsetwater + IOPMrootDomain
  isLPWMode + panic_with_options + vfs_vnodecovered + ml_io_read8）；设备仓账本同步 829→839（c958bdd），
  主树 e86c5621 force-with-lease 已推，备份分支 m2381-ios27-pre-sync71。
  **★侦察定谳（用户"为什么别人快"案）**：**sheng = 小米 Pad 6S Pro 12.4 = SM8550 —— 与 m2381 同
  SoC**（"机型差异更大"前提不成立，是最亲兄弟）；他们 24A446 全绿（GPU Metal 149 draws/BT
  AirPods A2DP/WiFi/触控/UFS root 全 device-verified）。快的三因：①**板级线 23F84 时代已成熟**
  （共享 SM8550 支持现成，24A446 只换 OS profile）；②全职维护者早 3 天（11-02 开工 vs 我们
  11-05 换道）；③墙账两家独立收敛同批（他们 156 条里 144 与我们重叠=互证，独有 12 条=他们的
  USB 栈位置）。**config 假说排除**：我们 profile 本就带 `--fileset-reference mach.release...noctf`
  （s43 参照件），匹配参考件已对齐。我们真正的结构差 = 板级 bring-up 与换代同步做 + 换代两次
  （21H461→23F84→24A446 账本三次清零）。**下一步红利**：r72 起从同步后的树出件（新补丁+839 账本
  自动多绑）；USB 栈走他们铺好的路（QcomDwc3 + roots 加 USB 栈 + sheng-usb 模式）；可选反哺
  （把 839 推上游 UniXNU/XnuPrelink 共享，kit 本地线 96eedc2 未推）。

- **★会话六十九 = r68 首刷判读（墙出 AMFI 进 sandbox 里程碑）+ 墙69 定谳 + sandbox 普查 267哨兵/59靶 + 双靶批绑，r69 出件待刷（2026-11-07 凌晨连轮）**：
  **r68 首刷判读**（payload-r68-dd3e9636；屏摄 23:55）：crc `0x64d09506` 零纠正，hit #1，t=1s。
  报点 `0xfffffff00aa5a2e8`（转录多一 f，CRC 仲裁）。**墙离开 AMFI span** —— site `0xaa5a2e4` 落
  com.apple.security.sandbox text span `[0xaa46c70,0xaa81594)`（s64 桶表）→ **r68 LookupEntry 绑定
  生效、AMFI 清完、boot 进 sandbox init**（= s43 参照车道"崩点过AMFI进sandbox注册"里程碑）。
  **墙69 定谳**：donor `0xb26a124` = **`_proc_get_syscall_filter_mask_size`**（kern_proc.c:5289）。
  caller = sandbox init：w0=0 / w0=1 两调，结果存 sandbox 全局（UNIX/MACH mask 尺寸）。四证：①
  donor 体 52B switch（0→常量 0x22e=nsysent 558 / 1→0x80=mach_trap_count 128 / 2→全局
  mach_kobj_count / default 0，Apple release 常量化）；②noctf 精确 0x78838a4→km 0xB23F8A4；③
  native 体镜像同 switch（our build nsysent 是变量 [0x719e670]）；④源码锚。
  **sandbox 预读普查**（s64-tracer 全 span，temp/s69-sandbox-prescan.json）：**267 哨兵调用 / 59 个
  distinct donor 靶**（头号 0xaf21778×44 / 0xaf55418×28 / 0xb2aa3f8×27 / 0xad45364×25 / 0xb260110×14 /
  0xadb7cbc×10）。**头号靶定谳**：donor `0xaf21778` = **`vnode_hold`**（27.x 带锁 flavor：CAS [vp+8]
  自旋拿锁 → ldadd [vp+0xb4] holdcount++ → INT32_MAX panic；**panic `__FUNCTION__` 串 "vnode_hold"
  @0x706e6cb + "vfs_subr.c" + "vp %p holdcount overflow" 全串字节读讫 = 自报家门**；姊妹 leaf
  0xaf215f8 = matcher body-hash _vnode_hold，与 native km 0xAF5FE84 逐指令同形）。
  **修 = 2 靶 5 槽**（0x848d4f0→0x801100000423B8A4；0x80e1b10/0x8136408/0x848d828/0x84a0008→
  0x8011000003F5BE84）：scripts/s69-payload-surgery.py，19B/5 簇零杂散，seal 未动。**出件
  out/m2381/payload-r69.img sha256 `be541f1c` 待刷**。账本 823→**825**。
  **★工具坑（s69 新增两条）**：①capstone 从非 4K 对齐起点反汇编会**错位误读**（"非函数头"假象——
  必须回退到真 entry 重同步；本次两处"mid-function 假象"全是自己 offadj 双减/错位所致，幻灯
  +0x39BC000 继续全局成立）；②`offadj` 参数（KH 修正）sign 写反过一次——读 payload 统一用
  `KHEAD+(km−KBASE)` 正偏移。

- **★会话六十八 = r67 首刷判读（三笔绑定全生效）+ 墙67 定谳 SecureDTLookupEntry + 五面绑，r68 出件待刷（2026-11-06 深夜连轮）**：
  **r67 首刷判读**（payload-r67-41b52bea；屏摄 IMG_20261106_223903 22:39，用户标注"r66"系笔误——
  报点 0x924e69c ≠ 墙66 的 0x924e644/0x924e648，且 r67 22:13 出件 → 时间链定谳 = r67 首刷；判读
  纪律新增：**同 payload 必同 site，报点差可作 payload 版本判别**）。crc `0xeb9fb8ca`（verify-panic-crc
  --autofix 仅补前导 f，零纠正），hit #1，t=1s。工作单 B 三条件全过：crc 新 / caller 仍在 AMFI span /
  槽 `0x8135d08` ∉ r67 三已绑槽 → **r67 三笔（lockdown/identity×2）全部生效，墙66 消**，墙 +0x58
  推进至同一 caller `0x924e440`（LocalSigning.cpp DT 阶段）。
  **墙67 定谳**：donor `0xb5be330` = **`_SecureDTLookupEntry`**（pexpert device_tree.c）。caller 现场：
  `DTLookupEntry(0, "/chosen", &entry)`（x1="/chosen"@0x763b99e 字节读讫），返值比 #1(kDTSuccess)，
  随后 `DTGetProperty(entry, "darwinos-security-environment"@0x763e055, &ptr, &size)` —— 该槽
  `0x8135d00` r58 已绑 `_SecureDTGetProperty`（native 体逐指令同形复核讫，r58 绑定正确）。四证：
  ①donor 体 [0xb5be330,0xb5be5c8) 664B：init 旗标 cbz / NULL→csel gDTRoot[x25+0x318]+边界[x24+0x328]
  ccmp / 前导'/'跳过（空路径→*entry=root,w0=1）/ w9=0x3f 分量拷贝循环 / 走树 bl 0xb5be11c（matcher
  验讫 _SecureDTGetPropertyInternal）取字面 "name"@0x70cf529 + bl 0xb5bc51c 比名 + bl 0xb5be5c8 孩子
  迭代 / 错误块 device_tree.c 断言串（行 98/150，"Device tree pointer outside of device tree region"）；
  ②noctf 精确 0xfffffff007b5522c（km 0xB51122C = +0x39BC000）；③native 体镜像逐块同构（native
  bl 0xb511600 恰 = GetPropertyInternal 自身幻灯 km——几何闭环）；④源码锚 pexpert device_tree.c。
  **修 = 5 面绑**（`0x7f87218 0x80e17b8 0x8135d08[AMFI 双调用点 0x924e69c+0x924f00c] 0x8479a60
  0x848d0c8` → `0x801100000450D22C`）：设备仓 scripts/s68-payload-surgery.py，15B/5 簇零杂散，seal
  未动。**出件 out/m2381/payload-r68.img sha256 `dd3e9636` 待刷**。账本 822→**823**。
  **★工具坑（s68 定谳）**：payload 原生体读必须绝对偏移 `KHEAD+(km−0x7004000)`——漏 KHEAD 错位
  0x1c9800 并伪造"本地布局分歧"假象（lockdown/identity 两 runtime 锚点当场抓获翻案）；PACIBSP =
  `0xd503237f`（0xd503233f = PACIASP，勿混）。
  **★下一墙预读（s69 主菜）**：site `0x924ec80` 槽 `0x8135e08` donor `0xb472cd8` =
  **`_ZN9IOService15serviceMatchingEPKcP12OSDictionary`**（noctf 精确 0x7a7d024 → km 0xB439024；
  体镜像 withCString→withCapacity(2)→merge 同构；matcher 图未收）。caller = ConfigurationSettings.cpp
  SCRD 段：`serviceMatching("sep-endpoint,scrd", 0)` → 紧随 site 0x924ec90
  `waitForMatchingService(dict, -1=WAITFOREVER)`（槽 0x8135e20 r58 已绑）。**★绑定风险已标：本平台
  无 SEP SCRD 端点，单绑 serviceMatching = boot 挂死无 crc（比 panic 更糟的判读面）——s69 必须
  绑定 + SCRD 等待中和同轮上，单绑不刷**。
- **★会话六十七 = r66 首刷判读 + 预读 bug 正名 + 墙66 定谳 + 预读批量绑，r67 出件待刷（2026-11-06，连轮）**：
  **r66 首刷判读**（payload-r66-fa977e3a）：crc `0x7a50a1ae` 零纠正，hit #1，t=1s。panic from
  `0xfffffff00924e648`（x0 0x0 x1 0x1 = 上一调用的残渣，真签名无参）→ **caller 仍在 AMFI span 内**
  → **s66"AMFI 导入面清仓"系假零**：预读 JSON 的真结构 = `calls`（2661 条）+ 小写 verdict，s66 判读
  读了不存在的 `bls` 键 + 大写 'SENTINEL' 子串 → 空集误判为零。按 `payload_word` 低48位真值重扫：
  **AMFI 实余 114 调用 / 35 个 distinct donor 靶**（头号 0xb226cc8×24 调用点 / 0xad3e8e4×12 /
  0xaf21778×11），墙66 就在普查单里（site 0x924e644 早已记录）。
  **墙66 定谳**：donor `0xb2a405c` = **`_get_lockdown_mode_state`**（bsd/kern/lockdown_mode.c:87，
  Private.exports:407 导出）。caller = AMFI policy-config 函数 `0x924e440`（boot-arg 读 + 9×静态注册
  + PAC 字典查），调用后 `cmp w0,#0`/`cset` 存 AMFI 全局 `0x8130e58`，非零则连发
  0x10000000/0x20000000/0x40000000 三笔强制位 = lockdown-mode 执法扇出。四证：①体
  [0xb2a405c,0xb2a4134) 216B：内联 CAS 锁（0xb9cbaf8）+ init-done 旗标 + **msleep wmesg 串字面
  "get_lockdown_mode_state"（0x70a4d3d）自报名字** + 返 lockdown_mode_state 全局；②noctf 精确
  0x78bd4c4（km 0xB2794C4）；③原生体镜像（真 lck_mtx_lock/unlock 调用 + 同 msleep 形 + 同 wmesg；
  donor 内联 CAS / native 走调用 = 同操作两编译策略）；④源码锚逐指令吻合。
  **★userdata 勘误（用户记忆为准）**：24A446 userdata（Mac AEA 链件，11-05 02:32，15,631,122,432B
  sha `b280f63e9a0c8cf8…4667d`，OS 窗 `8a12cdad`@1MiB / SystemOS 窗 `d5b4873d`@9,614,393,344 与
  profile pins 逐位同）**从未刷过机**——s66 首更"r52 已带上机"系推断错误，设备现 userdata = 23F84
  旧件。本会话已从 Mac（`naoniudeair@192.168.10.246:~/unixnu/`）拉回本机
  `out/m2381/ipados-27.0.1-24A446/m2381/userdata/`（img+json 双件，sha 验讫）。
  **修 = 2 笔 3 面**（lockdown 槽 `0x8136078`×1 + identity 槽 `0x8135f80`（24 调用点共用）/`0x848d250`×2）：
  批量第二笔 = 预读普查头号靶 donor `0xb226cc8` = **`_csblob_get_identity`**（codesign 标识串 getter：
  guard [csb+0x20] 位0 → 找 CSMAGIC 0xfade0c02 CodeDirectory → BE32@[dir+0x14]=identOffset → 返
  标识串；noctf 精确 0x7843cb4/km 0xB1FFCB4 + 体镜像 + 邻体消歧[teamid=3指令leaf、cdhash=3指令缓存
  getter 与 noctf cdhash 体逐字节同=排除]）。s67 手术（设备仓 scripts/s67-payload-surgery.py）：
  **3 面 9B/3 簇零杂散，seal 未动**。**出件 `out/m2381/payload-r67.img` sha256 `41b52bea…` 待刷**。
  账本 820→**822** + os_log_create entry 勘误（假零段标注）。**判读纪律新增**：预读普查必须按
  `calls` 键 + `payload_word` 低48 == sentinel 真值过滤，任何"清零"结论前先打印计数非空验证。
- **★会话七十 = r69 首刷判读 + 墙70 定谳 + 农场 T2 定名，r70 出件待刷（2026-11-07 凌晨连轮）**：
  **r69 首刷判读**（payload-r69-be541f1c；屏摄 IMG_20261107_011047 01:10）：crc `0xf53b30e2`
  单命中零纠正（照片 f-run 17 位=多一 f 惯性，x1 反向少一 f——双侧仲裁），hit #1，t=1s。
  报点 `0xfffffff00aa75014`（x0/x1 = 两个 donor 串）。**墙仍在 sandbox span** —— site `0xaa75010`
  落 [0xaa46c70,0xaa81594) → **r69 两靶绑定生效、墙单调后移**。**caller 实据 = entitlement 前缀
  匹配**：x0 = "ENTITLEMENT:com.apple.security.exception.iokit-user-client-class"@0x7d646a2、
  x1 = "ENTITLEMENT:"@0x7d67dbe（字节读讫）——沙盒/AMFI 政策 init 的 entitlement 过滤解析。
  **墙70 定谳**：donor `0xae825b8` = **`_strncasecmp`**（map-nearest "_strcasecmp(+0x44)" 的真义 =
  靶是 map 符号身后紧邻的下一函数，donor map 缺 _strncasecmp）。四证：①donor 体 0x60B capstone
  （bti c/cbz x2 n==0→0/索引循环 ldrsb/tolower 内联 `(c−0x41)<0x1a ? c|0x20 : c`×2/ca!=cb→diff/
  ca==0→0/alen==blen==n 使 strbufcasecmp 长度尾编译消除）；②noctf 精确 0x74f0604（km 0xAEAC604），
  **noctf 家族序=源序**（strcasecmp 0x74f05c0/strncasecmp +0x44/strbufcasecmp +0x74/strlcasecmp
  +0xe4），donor 几何同（donor _strcasecmp 0xae82574=靶−0x44，matcher body-hash 在案）；③native
  镜像：_strcasecmp 17 条**逐字节 1:1**、_strncasecmp 24 条同集合**单步相邻重排**（donor ldrsb
  w13,[x1,x9] 在 cmp w11,#0x1a 前，native 互换=调度差，s67 lockdown 先例类）；④源码锚
  osfmk/device/subrs.c：strncasecmp_impl=strbufcasecmp_impl(s1,n,s2,n)+tolower 逐行对应。
  **T2 定谳（农场头名，28 面共槽 0x848d6e8）**：donor `0xaf55418` = 2 条 bti-c 别名尾跳
  （bti c; b 0xadbafd4；map-nearest _vfs_64bitready+0xac 系夹逼伪影）→ 真身 `0xadbafd4` =
  **`_current_thread_ro`**（无参 flavor：mrs tpidr_el1→thread（入参 x0 被覆写=零参）；[thr+0x420]
  =tro（donor 结构偏移）；**内联 zone-element 验证四重**（标记指针界对 [0x7df9f40]/16K 页算术
  [0x7e00140]/页元 ldrh&0x3ff==3 [0x7df9fa0]/回指针 [ro]==thread）→ 失败 bl 0xb59bd64 =
  `_zone_id_require_ro_panic`（map how=string）/邻 `___thread_ro_circularity_panic`；成功返 ro）。
  四证：①体 0x98B 全 disasm；②noctf 精确 0x73add2c（km 0xAD69D2C）+ donor map 夹逼同族
  （_get_thread_ro−0x94/_swap_task_map+0x690/_get_threadtask+0x8bc）；③native 语义镜像（bl
  _current_thread→[thr+0x4e8]→bl zone_id_require(3,0x38,ro)→回指针核对→返 ro；donor 内联 zone
  验证 vs native 走 helper = s67 lockdown"同操作两编译策略"类；偏移 0x420/0x4e8 各内核自洽，
  donor _get_thread_ro 同读 [th+0x420] 佐证）；④源码锚 osfmk/kern/bsd_kern.c:167。
  **修 = 2 靶 7 槽**（strncasecmp：`0x7f85788/0x836b580/0x848d698/0x849faa8`→`0x8011000003EA8604`；
  current_thread_ro：`0x82da968/0x848d6e8/0x849fcf0`→`0x8011000003D65D2C`；**全 __DATA_C 全局
  枚举——沙盒 span 外其他镜像的同靶哨兵槽一并绑**）：设备仓 scripts/s70-payload-surgery.py，
  28B/7 簇零杂散，seal 未动。**出件 out/m2381/payload-r70.img sha256 `ecf23f32` 待刷**。
  账本 825→**827**。**s68/s69 设备仓脚本+账本漏 commit，s70 一并收仓**。
  **★工具坑（s70 新增）**：①map-nearest(+0xNN) 判读：先查 noctf 家族序+几何（+0x44 常是"身后
  紧邻下一函数"），再下结论，勿把 addend 当函数内偏移；②noctf-syms.json 条目 = [带符号十进制
  int, 名]，`&0xFFFFFFFF` 才是 36 位坐标（0xfffffff0 前缀）；③twinhunter 归一化 hash 对**单步
  重排**盲——镜像判定落到逐条 capstone 对读。
- **★下轮工作单 = 会话七十一（r70 判读 + sandbox 农场续批）**：
  A. **刷机**：recovery_a ← r70 `ecf23f32`（payload-only；回退 = r69 `be541f1c`）。刷后 STATE §2
     已由 s70 预更（boot_b 289614ba / recovery_a r70 / userdata b280f63e）。
  B. **r70 判读**：crc 必新；caller 允许 sandbox span 内；**槽必非本轮 7 槽**（0x7f85788/0x836b580/
     0x848d698/0x849faa8/0x82da968/0x848d6e8/0x849fcf0）。current_thread_ro 28 面热路径 = 下一墙
     头号候选（若其哨兵面在 sandbox init 执行序最先撞线）。
  C. **农场续批（s71 主体，普查余 57 靶）**：T3 0xb2aa3f8×27（缓冲空间计算体待分型）；T4
     0xad45364×25（**lock_rw_old.c lck_rw 族 flavor 分型**："Taking non-sleepable RW lock with
     preemption enabled"@0x705490d、文件串 0x70547b9 已读）；0xadb7cbc×10（kfree_type_impl_internal
     邻域 = s60 家族，查 s60 已绑 native 是否同款）；0xb260110×14 / 0xb26324c×10（proc_name
     +0x7c）/0xad94128×10（thread_set_cpulimit +0x1c4）。四证 bar 不变，齐一靶绑一靶。
  D. **判读纪律**：CRC autofix 每轮必跑；kext 归属查 s64-kext-buckets.json；槽排除清单 = 账本
     functions 全量（现 827 条）；payload 体读 = KHEAD 正偏移。
- **★会话六十四 = 静态预读双墙定谳（墙62/63）+ 判读农场基建 + r64 出件待刷
  （2026-11-06，连轮，子代理并行产证）**：
  **caller 定性修正**：墙 61/62/63 的 caller 函数 [0xa41ca8c,0xa41ce70) 归属
  **IOStorageFamily**（span bug 修复后定谳；此前"memdev"系调用序与 memdev.c:665-673
  同构所致的特征误标，kext 本体 = IOStorageFamily 的设备注册函数）。
  **墙 62 定谳**：donor `0xb3d1d58` = **`cdevsw_add_with_bdev`**（r63 屏摄 crc
  `0x68352ecd` 零纠正，panic from `0xa41cb1c`，x1=`0xbb65188` cdevsw 112B 模板；
  bl `0xa41cb18` → 槽 `0x83ae510`）。三证：①体 316B——体首 bl donor `_cdevsw_add`
  `0xb3d1a98`（kernel.map how=calls）、bdev 写 chrtoblktab `0xb89bea4`[major]（cmp
  #0x17 后）、bdev 越界内联 `_cdevsw_remove` 回滚（五常量全中：cdevsw 0xb89c6f0 /
  nocdev 占位 0x7ec2ae0 / flags 0xb9284f8 / CAS 锁 0xb9ce228 / memcmp 0xae82458）、
  返 index/−1；②**s63"native 无符号"翻案**——noctf 精确 `_cdevsw_add_with_bdev`
  0x79ebd74（km `0xB3A7D74`，228B）在表（s63 只查 matcher map 未查 noctf-syms 所
  致），与 donor 逐块 1:1（native 调 `_cdevsw_add` 0xB3A7B3C + `_chrtoblk_set`
  0xB3A0FEC；几何闭环 0xB854BD8+24×0x38=`0xB855118`=native cdevsw 表基；
  chrtoblktab 0xB856D18）；③源码锚 bsd_stubs.c:266-284——with_bdev = cdevsw_add +
  chrtoblk_set + 失败回滚，`cdevsw_remove_with_bdev` 全树不存在，chrtoblktab 唯一
  读者 `chrtoblk()` 返 NODEV 属设计内（无启动早期 panic 面）。方案 B（shim 重构）
  作废。**墙 63 定谳**：donor `0xb3d1894` = **`bdevsw_remove`**（三重指纹 cmp#0x17/
  stride 0x38/表 0xb89c1b0 全中 + 与 bdevsw_add 镜像；native `_bdevsw_remove` km
  `0xB3A7968` 几何闭环同上）。**勘误**：无条件面 = `0x849f248`（apfs 反注册 fn
  0xaaedb8c），s64 工单所写 `0x849f240` 系墙 61 bdevsw_add 已绑槽之误。
  **修 = 3 面绑 native**：槽 `0x83ae510`→`_cdevsw_add_with_bdev`（词
  `0x80110000043A3D74`）+ 槽 `0x83ae490`/`0x849f248`→`_bdevsw_remove`（词
  `0x80110000043A3968`）。s64 手术（设备仓 scripts/s64-payload-surgery.py，动态枚
  举+面数断言+noctf→km delta 门）：**3 面 9B/3 簇零杂散，seal 未动**。
  **出件 `out/m2381/payload-r64.img`（99MiB）sha256 `b7535382…` 待刷**（recovery_a
  ← r64；回退一级 = r63 `0cb17660`）。账本 816→**818**（体 sha 在
  temp/s64-ledger-bodies.json）。**判读农场基建（本轮三件，报告 temp/s64-farm-report.md
  与 temp/s64-span-fix.md / s64-wall62.md / s64-wall63.md）**：A1 span bug 修复
  （旧"最小 span 者胜"退化→全部槽误归 corecrypto；新 s64-kext-buckets.py 按
  LC_FILESET_ENTRY 独立重定基 + DC 区间包含判属）→ **815 面/392 导入/27 桶清单**
  （temp/s64-kext-buckets.json；差 1 悬案溯源 = 槽 0x849fff8 高 16 位 0x8001 非
  form-A，823=form-A 口径/824=low48 口径）；farm manifest 392 导入体特征全覆盖
  （temp/s64-farm-manifest.json）+ IOStorageFamily 桶初评 **高 2/中 4/低 24**
  （temp/s64-farm-currentkext.json，中置信不绑守边界）+ tracer 复现 22/20/2
  （temp/s64-tracer.py）。**策略定谳**：纯体哈希/词级方法打不动此池（392 目标仅
  28 哈希命中、可绑级 1；已知孪生词等 topk 从未进前 3）——同 TU 锚点+体语义+
  caller 契约是主路径。中置信 4 条待人工复核（含 `_IOLockWakeup`：topk s_min=1.0
  + ledger 邻居 IOLockSleepDeadline+0x14、6 面）。
- **★会话六十三 = r62 首刷判读定谳墙61 = bdevsw 族 + 2 面绑 native `_bdevsw_add`
  r63 出件待刷（2026-11-06，本会话连轮）**：
  **r62 首刷判读**（payload-r62-2d3e8dd4）：crc `0xaf0e2d21` 零纠正（124B），t=1s，hit
  \#1。panic from `0xfffffff00a41cb04`（x0 `0x1` x1 `0xfffffff00bb651f8`）→ **墙 60 消实
  锤**（erandom 7 面生效；AppleImage4 nonce 过，推进到某驱动的设备注册）。**定谳链**：
  caller bl `0xa422054` → 标准 GOT thunk（adrp 0x83ae000+0x480）→ 槽 `0x83ae480` 哨兵 →
  donor `0xb3d1700`。语义 = **`bdevsw_add(1, 56B 模板)`**：x0=1=请求 major，x1=kext 自有
  bdevsw 模板（0xbb651f8，diversified-auth 指针数组），返回值与请求 major 比较 =
  bdevsw_add 契约。**定名**（devsw 域取证，TU=`bsd/kern/bsd_stubs.c`）：donor
  `0xb3d1700` = **`bdevsw_add`**（devsw CAS 锁 0xb9ce228 = donor `_cdevsw_add/_cdevsw_remove/
  _devsw_unlock` 同一全局；per-cpu mtx stats [tpidr+0x1c0]；bound 0x17=nblkdev−1；
  memcmp vs empty-bdevsw 占位 0x7ec2aa8 = 6×eno_\*+0）；donor `0xb3d161c` =
  **`bdevsw_isfree`**。几何铁证：donor bdevsw[24×56B]@0xb89c1b0 **紧邻** cdevsw[64×
  112B]@0xb89c6f0（donor `_cdevsw_add` stride 0x70/bound 0x3f/key 0x7ec2ae0）= conf.c
  bdevsw[]→cdevsw[] 布局。**修 = 2 面（0x83ae480/0x849f240）绑 native `_bdevsw_add`
  `0xB3A7810`**（noctf 0x79eb810 精确，体同构）；**isfree 槽 0x83ae488 已被 s61 696
  批量绑到同一 native `0xB3A7700`——独立交叉证实孪生判定**。s63 手术（设备仓
  scripts/s63-payload-surgery.py，按 donor 词动态枚举面）：词 `0x80110000043a3810`，
  diff 6B/2 簇**零杂散**，seal 未动。**出件 `out/m2381/payload-r63.img`（99MiB）
  sha256 `0cb17660…` 待刷**（recovery_a ← r63；回退一级 = r62 `2d3e8dd4`）。账本
  814→**816**（+bdevsw_add/bdevsw_isfree 双记录，体 sha 在 temp/s63-ledger-bodies.json）。
  **注**：native `_cdevsw_isfree` stride 0x70 = cdevsw(112B)、donor f 表 stride 0x38 =
  bdevsw(56B)，两表各归各勿混判；s63 手术 diff 期望须按面动态算字节数（哨兵低 48 位
  0x045A5C4C 的 byte3=0x04 与部分新词相同）。s64 可选项（待拍板）：395 集中 noctf
  精确 + 体验证的导入小批合绑（5-10 个/轮）以加速收敛，风险 = 难二分。
- **★下轮工作单 = 会话六十四（战略升级：静态引导道追踪 + 并行判读农场，终结打地鼠；用户授权激进多轮迭代）**：
  **r63 屏摄核验**（用户指示本轮不判读）：crc `0x68352ecd` 零纠正（124B）from
  `0xfffffff00a41cb1c`（x0 0x1 x1 `0xfffffff00bb65188`）→ **静态预测命中实锤**：s63 收
  队前从 bdevsw caller 前向扫描预读的下一墙 = bl 0xa41cb18 → 槽 0x83ae510 → donor
  `0xb3d1d58`（回退地址精确吻合）——**墙可静态预读，刷机只是验证不再是探路**。x1 =
  0xbb65188 = cdevsw 尺寸（112B）模板（比墙61 的 bdevsw 模板 0xbb651f8 恰低一表项），
  疑 cdevsw_add 族；donor 0xb3d1d58 落 bsd_stubs.c TU 区（源序 isfree<add<remove<cdevsw
  族），native `_bdevsw_remove`/`_cdevsw_isfree`/`_devsw_unlock` noctf 在册待对。
  A. **工具基建**（深度研究三件）：
     A1. kext 归属修复：GOT 页 → fileset entry（修 span 解析 bug：entry 内层段独立重定
         基），产出 = 815 面/392 导入按 kext 分桶清单。
     A2. **静态引导道追踪器**：从当前墙 caller 起 DFS bl→thunk→slot 链 + 本地函数跳转
         （有界深度），产出每 kext 导入触发序列 = 未来墙预告单（0xa41cb18 扫描 = PoC，
         一扫双墙：0x83ae510→0xb3d1d58、0x83ae490→0xb3d1894，均 sentinel）。
     A3. donor 导出 trie（0x4c50000 blob 0x5cc 无名串）与 kext 内层 bind（prelink 已剥）
         复攻 = 低优先级挂账。
  B. **并行判读农场**（多轮迭代引擎）：
     B1. 当前 kext（caller 所在，GOT 0x83aexxx 页）全部哨兵导入 → 扇出 agent 并行三证
         判读（管线已文档化：donor 体反汇编 → noctf/源码锚 → TU 几何 + 全局指纹 → 语义
         契约；范例 = erandom/bdevsw 双案录）；我复核抽查 → 合批绑定 → payload-r64 →
         刷机验证。
     B2. 每轮清一个 kext（或数个）；回退链逐轮滚动；账本逐轮增记（构建期价值累积）。
  B3. **s64 首批（预读墙已备）**：caller = donor memdev 注册（模板对 cdevsw 112B
         @0xbb65188 + bdevsw 56B @0xbb651f8 与 memdev.c:665-673 调用序逐行吻合；函数
         [0xa41ca8c,0xa41ce70) 22 调用中 20 已绑全绿）。墙 62 = donor `0xb3d1d58` =
         **`cdevsw_add_with_bdev`**（体首 bl donor cdevsw_add + 3 参 (1, csw, bdev)；
         **native 无此符号（单调用点被内联）→ s64 首个真判读题**：直绑 native
         `_cdevsw_add` 0xB3A7B3C（bdev 关联簿记缺失，源码在手裁决）vs 源码重构）。
         墙 63 = donor `0xb3d1894` = **`bdevsw_remove`**（体首 cmp w0,#0x17 + stride
         0x38 + 表 0xb89c1b0 三重几何指纹；条件墙——cdevsw_add_with_bdev 成功路径跳
         过；另面 0x849f240 无条件 caller）→ 直绑 native `_bdevsw_remove` 0xB3A7968
         就绪。native bsd_stubs TU 全家福（noctf）：isfree 0xB3A7700 / add 0xB3A7810 /
         remove 0xB3A7968 / cdevsw_isfree 0xB3A7A2C / cdevsw_add 0xB3A7B3C /
         cdevsw_remove 0xB3A7C9C。
  C. **emit go/no-go = 降级挂账（2026-11-06 裁决）**：emit 解决绑定机制，而现瓶颈 =
         判名（emit 同样只能绑账本内名字，392 未名面 emit 也救不了）；手术绑定已证
         廉价确定（s63 = 6B/分钟级）。**复触发条件 = 第一次因任何原因需要 Mac 重编**
         （如 shim 需求）——届时顺带开 emitter 让该次构建自绑全部账本名，一次成本双
         收益。此前不再投入。
  D. **边界**：无证据不绑——近匹配自动评分已判死（7 对已知孪生校准相似度仅
     0.08-0.24，donor 27.0.1 与 feed 编译分歧过大）；每面仍需三证，agent 产证、人复核
     后绑定；T1 精确体哈希猎手产出 ≈ 0（temp/s64-twinhunter.py）亦留档。
- **★下轮工作单 = 会话六十五（r64 判读 + IOStorageFamily 清桶轮）**：
  A. **r64 刷机判读**：预期墙 62/63 双消——caller 注册函数 22 调用全绑后该注册路径
     应放行；成功旗标 = 新 panic 来自**不同 caller**（≠0xa41cb1c 一族）或推进（crc
     必为新值）；若仍 0xa41cb1c 族 → 检查同族第 4 墙（cdevsw_remove/cdevsw_isfree
     的 caller 是否被注册失败路径触达）。回退一级 = r63 `0cb17660`。
  B. **IOStorageFamily 清桶**：桶剩 29 面/28 导入（farm currentkext.json 已备初评）；
     高置信 2 条已随 r64 绑毕；中置信 4 条人工复核后决定绑否（`_IOLockWakeup` 6 面
     = 头号候选：topk s_min=1.0 + ledger 邻居 IOLockSleepDeadline+0x14 + 语义契约
     唯一性）；低置信 24 条按"下一墙优先"逐个三证（s64-tracer.py + farm manifest
     体特征为现成产证底稿）。
  C. **全量推进序**（kext-buckets.json 桶面数）：apfs 155 / AppleARMPlatform 70 /
     sandbox 63 / IOGPUFamily 61 / IOHIDFamily 52 / AMFI 43 / IOAVFamily 40 /
     IOSurface 38 / ApplePMGR 37——每轮清一桶，静态预读下一墙后再刷（刷机只验证）。
  D. 边界不变：无证据不绑；账本逐轮增记；回退链逐轮滚动。
- **★会话六十二 = r61 首刷判读定谳墙60 = read_erandom 族 + 7 面绑 native `_read_frandom`
  r62 出件待刷（2026-11-06）**：
  **r61 首刷判读**（payload-r61-1795d6bb）：crc `0xa9474625` 零纠正（125B），t=1s，hit
  \#1。panic from `0xfffffff0090261b8`（x0 `0xfffffff00ba64b19` x1 `0x10`）→ **墙 59 消实
  锤**（696 批量 9,134 面全生效；corecrypto kext start 注册 PRNG 后推进进 AppleImage4
  nonce manager）。**定谳链（全程证据闭环）**：caller bl `0x9028f44` = 环境对象回调派发
  thunk（`ldr x16,[x0,#0x100]; mov x17,#0x12f7; braa x16,x17`；对象 = 运行时填充的
  __DATA 注册表）→ 回调 = **ctx-drop 适配器 `0x902576c`**（`mov x0,x1; mov x1,x2; b
  0x903a898`，(ctx,buf,len)→(buf,len)）→ AppleImage4 GOT thunk 0x903a898 → 槽
  `0x80e1a40` 哨兵。语义 = **16 字节随机读取**（nonce；x0 = struct+5 内嵌 16B 缓冲，
  x1 = [x10+0x48] 长度字段）。**定名**（donor 导出 trie 不可解码；source 锚 + 体取证）：
  donor `0xae9bd20` = `read_random_generate`（blraa 经 `prng_generate` 函数指针全局
  [0x7dff0a68] + 每核互斥 tpidr_el1 + 0x100 分块）；donor `0xae9bc68` = `read_erandom`
  （u32 len + `prng_ready` 门 [0x7dff0a80]）；兄弟 `0xae9bbac` = `read_erandom_generate`
  （16K PAGE_SIZE 分块；map how=string 记录独立佐证 native 0x7507818）——TU =
  `osfmk/prng/prng_random.c`（墙 55 fips 门同文件）。**修 = 7 面绑 native
  `_read_frandom` `0xAEC2BF8`**（noctf 0x7506bf8 精确；源码 read_frandom{read_erandom}
  = 门+(prng_ready?PRNG:DRBG) 语义超集；native 体近指令级同构 = 同门+同 0xedeb per-cpu
  pacia 全局+同 0x100 分块；native 无 `_read_random_generate`/`_read_erandom` 符号
  （静态内联）故 `_read_frandom` 为正解；xnu-12377 车道的 fileset-read-erandom-shim
  不在 27 车道亦不需要）。**s62 手术**（设备仓 scripts/s62-payload-surgery.py）：槽
  0x80e1a40/0x8316e80/0x833c2e8/0x8479c28/0x849fa18/0x84ac9d8/0x848d5e8 → 词
  `0x8011000003ebebf8`，pre-image 白名单+donor 词断言全中，diff 28B/7 簇**零杂散**，
  seal 未动。**出件 `out/m2381/payload-r62.img`（99MiB）sha256 `2d3e8dd4…` 待刷**
  （recovery_a ← r62；回退一级 = r61 `1795d6bb`）。账本 812→**814**（+read_erandom/
  read_random_generate 双记录，体 sha 在 temp/s62-ledger-bodies.json）。
  **对质裁决（工作单 B 项）**：s61 冲突 `0xad8f628`（map=`_task_reference` vs
  账本=`_task_reference_external`）**操作上化解**——noctf 两名同址 `0x737d884`
  （别名/ICF 合并），绑定目标唯一 = native km `0xAD39884`，s63 直绑无需裁名。
  **工具警示**：map 稀疏（仅 matcher 映过函数），近邻命名不可靠；**395 未定名集已可
  用 `temp/s62-sentinels.py` 全枚举**（824 槽/395 导入，donor 词→map 近邻名为**暂定**
  需逐个三证；824 vs r60 记账 823 差 1 未溯源）。
- **★下轮工作单 = 会话六十三**：A. 刷 r62 判读（CRC 对拍 → 分流：新哨兵墙 →
  s62-sentinels 全名单对号 + 三证顺带修；PAC/数据 abort → 疑 `_read_frandom` 绑定 →
  回退 r61 二分；无墙推进 → stage 轨迹/vfsinit/root mount/SHLOG state=2）。B. 小批队
  列：`0xb47008c` publishResource 3 面 + IOCrypto 13 三证 + `0xad8f628` 5 面（绑
  `0xAD39884`）+ 395 残余。C. emit 同输入试验 go/no-go（kit `96eedc2` synced fixture +
  RELEASE reference）不变。
- **★会话六十一 = 696 定名导入批量手术 r61 出件待刷（2026-11-06，跳过 r60 直刷 r61）**：
  **口径定谳**：工作单 696/9,139 精确复现 = **r52 期 24A446 matcher map**
  （`out/m2381/ipados-27.0.1-24A446/m2381/kernel/kernel.map.json`，v11，
  inputs `38e6d9fa`=本地 donor）∩ noctf 同名同 VA 精确 ∩ r60 payload
  form-A donor 词。**★`out/m2381/kernel.{map,json}` = 23F84 旧线残留**
  （w40 时代，键空间 0xa76c000-0xb01d12c，inputs `0f7d50ac` ≠ 本地
  donor）——已弃用勿再引。**★词形两族定谳**：kext 导入面 = form-A 压缩
  词（`0x8011<<48|(VA−0x7004000)`，diversity=0）；diversified auth 词 /
  非 auth rebase 词 / 清零位 / 内核自体 fixup 位点 = emitter 管理态非导入
  面（r60 内实测 84/129/253/1,825 个，全分类落盘 surgery.json）。
  **手术 = `UniXNU-m2381/scripts/s61-payload-surgery.py`：695 导入 /
  9,134 面全重写**（donor 词 9,134 + sentinel 0；冲突 VA `0xad8f628`
  map=`_task_reference` vs 账本=`_task_reference_external` 5 面按规矩
  排除；账本外残余 `0xb47008c` IOService::publishResource 3 面 → s62
  队列）。门全过：计数 696/9,139 精确、native km 全在
  __TEXT/__TEXT_EXEC、seal 未动、diff 27,449B/9,135 簇**零杂散**。
  孪生抽验：body-hash 通道 74 条中 52 条 donor≡native 字节同，16 差异
  全为 allocator 相关 C++ 法（r55/56 补丁重编译所致，语义等价；版面
  稳定性由 4 个过墙符号背书，名单 `temp/s61-bodyhash-diff.json`）。
  **出件 `out/m2381/payload-r61.img`（99MiB）sha256 `1795d6bb…` 待刷**。
  账本 165→**812**（净增 647，双证 = matcher how + noctf 精确，体界
  (d') 约定 = 首 4 对齐 ret/retaa/retab 或离体 b 截断，金标准 10/10、
  165/165 raw-sha 复核）；kit 仓本地 commit `96eedc2`（主树子模块指针
  未动）；设备仓 `3bc999d`。
  **C 侦察 8 条**（`temp/s61-emitter-recon.md`）：live emitter = 设备仓
  vendored fork（feeds shadow 掉主仓 submodule，v11）≠ 上游 pin
  b1888c1；adapter branches 上游有我们无；fileset-reference 管线化只有
  我们有。
- **★下轮工作单 = 会话六十二（判读 + 决断轮；用户会前刷 r61）**：
  **刷机策略**：只刷 recovery_a ← `payload-r61.img`（dd 读回对拍
  count=99；现态 recovery_a = r59 `72cc9ef4`，iOS 链未被动）。回退：
  r60 `7da7e0b3`（一级）/ r59 `72cc9ef4`（二级）/ r58 `ca755a65`（三级）。
  A. r61 首刷判读：CRC 对拍（--autofix）→ 分流：
     a) 新哨兵墙（预期落 395 未定名集或 IOCrypto 13）→ 三证定名顺带修；
     b) 数据 abort / PAC → 疑 696 批量错孪生 → 16 条 body-hash DIFF
        名单（`temp/s61-bodyhash-diff.json`）优先排查 → 刷 r60 二分定位；
     c) 无墙推进 → 看 stage 轨迹 / vfsinit / root mount / SHLOG state=2。
  B. 遗留小批：`0xad8f628` 冲突裁决（`_task_reference` vs
     `_task_reference_external` 体哈希对质）+ `0xb47008c` publishResource
     3 面 + IOCrypto 13 三证 + 395 未定名集。
  C. emit 同输入试验：synced fixture（kit `96eedc2`）+ RELEASE reference
     跑 emit，报 stubbed/trap 未决集 vs 基线（build 期 1,850 stub ref /
     payload 823 sentinel 面），终点参照 sheng 终态 2 data + 24 trap →
     emitter 路线 go/no-go（侦察 D1 live-fork / D5 adapter-branches /
     D4 reference 管线化为决策输入）。
  D. r63 排单：go → emitter 全配置出件（构建期重绑取代 payload 手术线）；
     no-go → 手术线继续（IOCrypto 13 + 395 残余 + sheng 板级件收编）。

- **★会话五十九 = r59 首刷判读 + 墙 59 定谳（kalloc_type_impl_external，AppleImage4/AMFI 双家）+ r60 批量手术出件待刷（2026-11-06）**：
  **r59 首刷判读**（payload-r59-72cc9ef4）：crc `0x719ca2b6`（autofix 补
  from/x0 各一个前导 f，零数字错），124B，t=1s，hit #1。panic from
  `0xfffffff009023d34`（x0 `0xfffffff0080e16d0` x1 `0x4`）→ **墙 58 消实锤**
  （IOLockAlloc 134 槽 + OSObject ctor 292 槽全部生效，推进过 IOCrypto
  ctor）。**定谳 = caller com.apple.security.AppleImage4**（老 21H461 车道
  墙 10 同族！）：bl `0x903a698`（auth-GOT thunk）→ 槽 `0x80e1940` =
  哨兵 → donor import `0xad3e380`；x0 = **kalloc_type_view**（name 串
  `site.nonce_magazine_t`@0x751da8e + site 位图串@0x751daa4，kt_flags
  0xf8@+0x28，kt_size 0x64@+0x2c），w1 = 4 = zflags。**注意：本墙不在
  s58 预测的 IOCrypto 13 导入之列**——另一 kext（AppleImage4，AMFI 同调）
  的 kalloc_type 先撞线；IOCrypto 13 仍是下一梯队。
  **修 = donor `0xad3e380` 直绑 native `_kalloc_type_impl_external`
  `0xace8c8c`**（noctf `0x732cc8c` 符号精确、与 `_kalloc_type_impl` LTO
  别名同址；三证 = ①noctf 符号精确且紧邻 r58 已证 `_kfree_ext
  0xace8d18`（同 kalloc.c TU）②语义体形全对（0xd01000 常量对同对、
  zflags&7 折入同构 bfxil≡orr、ktv +0/+0x28/+0x2c 读法同、尾调
  `_kalloc_type_impl_internal` donor 0xadb2914 ≡ native 0xace9214 别名证实）
  ③stubbed 账本 AppleImage4+AppleMobileFileIntegrity 双家在册）。
  **★坐标系补充定谳：kernel.map.json `native` 字段 = noctf 空间 VA**
  （0xfffffff0 前缀），km VA = noctf + `0x39BC000`（s57 kfree_ext/
  s58 IOLockAlloc 双坐标验证）。
- **r60 = `UniXNU-m2381/scripts/s60-payload-surgery.py`：10 导入 44 槽**
  （墙 59 import 8 面 + matcher 账本（body-hash，noctf 交叉复核过）已定名
  9 导入 36 面：OSUnserializeXML 18 面 / IOFreeTypeImpl 8 面 /
  kauth_cred_nfs4domain2dsnode 3 面 / OSCollection::init、buf_biowait、
  buf_invalidateblks、cluster_unlock_direct_read、vfs_iterate、
  vn_authorize_unlink 各 1 面）。pre-image 白名单全中，diff 审计
  143B/43 簇**零杂散**，seal 未动（`__fips_hmacs`@0x4ec5fd 仍
  `6fc44336fd3f44b7`）。**出件 `out/m2381/payload-r60-7da7e0b3.img`
  （99 MiB）待刷**。账本 records 156-165 入
  `kits/XnuPrelink/fixtures/fileset-24A446-symbols.json`（165 条）。
- **IOCryptoAcceleratorFamily 残余哨兵导入（坐标已采未修，排在 696 批量与
  emitter 线之后）**：
  `0xb49c874` / `0xb4e7140` / `0xb4e670c` / `0xb4e6764` / `0xb4a6a20` /
  `0xb4a6ae8` / `0xb4a0528` / `0xb4a0580` / `0xb46c278` / `0xb46c2d0`
  （IOCEC 对）/ `0xad7e344` / `0xb29df7c` / `0xaec5fe0`（13 个，matcher
  账本全未定名，需逐个三证）。DC 区 sentinel 面 r60 后 = **823**
  （395 import；其中 696 已定名 import 的 9,139 面 = donor 词隐患，r61
  批量带走；两者不相交）。
- **★会话五十八 = 墙 58 定谳（IOLockAlloc 134 槽）+ OSObject ctor 翻案（292 槽全量）+ r59 手术出件待刷（2026-11-06）**：
  **r58 首刷判读**（payload-r58-ca755a65）：crc `0xfcd9b111`（autofix 仅补 x1
  一个前导 f,零数字错）,139B,t=1s,hit #1。panic from `0xfffffff00a0aac38`
  （x0 `0xfffffff00bb3de18` = IOCrypto kext 数据段静态对象 = 调用方 ctor 的
  this 残留,x1 `0xfffffff3facb9d20`）→ **墙 57 消实锤**（r58 的 453 槽 + 2 shim
  全部生效,推进进 IOCryptoAcceleratorFamily ctor）。**定谳 = donor
  `0xb4424c4` = IOLockAlloc 变体**（bti; adrp x8,0xb929000; ldr
  x0,[x8,#0x330]; b donor lck_mtx_alloc_init `0xad43134` = **弃参默认组包装**,
  IOLockAlloc() 无参 ABI）。调用链：IOCryptoAcceleratorFamily ctor
  `bl 0xa0ab660`(auth-GOT thunk) → 槽 `0x8313b60` = sentinel。
  **修 = 直绑 native `_IOLockAlloc` `0xb416a00`**（noctf `0x7a5aa00` 符号精确：
  ldr [0xb935450]; mov x1,#0; b native lck_mtx_alloc_init `0xaceffe8`（r58 已证
  孪生）; 邻位 `_IOLockInlineInit` 同全局）——免 shim。
- **★坐标系定谳（工具级）**：24A446 车道 slide = `0x1f000000000`（kc.py 注记
  本就对;本轮一度误判 0x100000000000 系手算翻车）。payload kernel.macho =
  donor KC 全局重定基,**kext GOT 位置等同**（donor km 偏移 == payload km 偏移）,
  屏 VA = payload VA 同空间。donor kernel exec 在 payload 里被 native kernel
  **整体替换**。
- **★翻案 = s57 "0xb3d385c 267 存活槽 donor 拷贝部分可用" 死**：payload
  `0xb3d385c` = native `_OSKextFreeSite` 邻域代码（fn@`0xb3d382c`）,donor 词槽
  指向我们 native 随机代码 = 潜在乱调,不是"可用拷贝"。→ **0xb3d385c（OSObject
  默认构造器,pacda isa 0xcda1 上下文）292 面（267 donor 词 + 25 哨兵）全量重绑**
  native `__ZN8OSObjectC2Ev` `0xb3aba18`（noctf `0x79efa18` + 0x39BC000;s45 墙36
  语义前例）。本 kext ctor 在 IOLockAlloc 之后紧跟 OSObject-ctor thunk
  （0xa0ab730 → 槽 0x8313bc8）,故同轮带上。
- **r59 = `UniXNU-m2381/scripts/s59-payload-surgery.py`**：134 + 292 = **426 槽**
  （IOLockAlloc 词 `0x8011000004412a00`;OSObjectCtor 词
  `0x80110000043a7a18`）,全部 pre-image 白名单（donor 词/sentinel）命中,diff
  审计 426 簇/1278B **零杂散**,seal 未动（槽在 collection __DATA_CONST,在 fips
  HMAC 盖的 entry __text 之外;`__fips_hmacs`@0x4ec5fd 仍 `6fc44336fd3f44b7`）。
  **出件 `out/m2381/payload-r59-72cc9ef4.img`（99 MiB）待刷**。账本 records
  154/155 入 `kits/XnuPrelink/fixtures/fileset-24A446-symbols.json`。
  **工具坑（本轮新增）**：手术偏移必须 +KHEAD `0x1c9800`（ESP 头）;
  bytearray 切片是拷贝（要 memoryview 原位写）;48 位压缩词手算必错,一律程序算。
- **IOCryptoAcceleratorFamily 残余哨兵导入（同一 init 路径,r60 第一梯队,
  donor 坐标已采）**：`0xb49c874` / `0xb4e7140` / `0xb4e670c` / `0xb4e6764` /
  `0xb4a6a20` / `0xb4a6ae8` / `0xb4a0528` / `0xb4a0580` / `0xb46c278` /
  `0xb46c2d0`（IOCEC 对）/ `0xad7e344` / `0xb29df7c` / `0xaec5fe0`（13 个;
  全部经本 kext thunk 表 0xa0ab710-0xa0ab920 可达）。
- **★下轮工作单 = 会话五十九（r59 首刷判读 + IOCrypto 残余 13 导入预研，安排于 2026-11-06 收队时，只写单不执行）**：
  **A. 首刷判读主线（用户刷 r59 `72cc9ef4` 后）**：
  0. 收屏；CRC 对拍（verify-panic-crc --autofix）；t/stage/hit# 与 r58 对拍。
  1. 读回自检表（上条 d 项）+ reader dd 读回对拍（dd 读回才算证据）。
  2. 分流：
     a. **无哨兵墙**（可能性低——本 kext 残余 12 导入大概率下一站）→
        IOLockAlloc + OSObject ctor 通关实锤,直接推进:fipspost POST 完整跑
        → PRNG 登记 → PE_init_iokit → AMFI（t≈32s IORTC 税）→ No-PMGR →
        PENDING 站表 0x924e77c/ef1c/ef5c。
     b. **新哨兵墙**（第一预期,caller 大概率仍落 IOCryptoAcceleratorFamily）
        → 照哨兵流程：bl 定位（尾跳类 = LR 可能保留上游）→ thunk → 槽 →
        donor 原词 → 三证定名 → 修;若 donor 词 ∈ 下面 B 单清单 = 预研已备,
        直接按钉死修法施工。
     c. **数据 abort / PAC 类** → 复核 shim 常量（kfree_ext `0xace8d18`,
        KHEAP_DEFAULT `0x7dec3e0`）与 braa 鉴权形（ESR EC 0x1C）。
  **B. 预研单（不等刷机结果,会话开头即可跑;s57→s58 模式）**：
  对 IOCryptoAcceleratorFamily 残余 13 个 sentinel 导入逐个三证
  （donor 坐标,全部经本 kext thunk 表 0xa0ab710-0xa0ab920 可达）：
  `0xb49c874` / `0xb4e7140` / `0xb4e670c` / `0xb4e6764` / `0xb4a6a20` /
  `0xb4a6ae8` / `0xb4a0528` / `0xb4a0580` / `0xb46c278` / `0xb46c2d0`
  （IOCEC 对,候选表旧行:体首 `mov w8,#1; str w8,[x0,#8]` 置标志+GOT 调用,
  186×2 面）/ `0xad7e344` / `0xb29df7c` / `0xaec5fe0`。
  步骤（每个）：
  ① donor 体形：bounds + disasm（temp/s58-wall.py 反汇编模式,donor exec
    fo = 0x3cb0000 + (va − 0xacb4000)）；下一步定体边界（下个 bti c /
    pacibsp,冷路径块算同函数——OSObject ctor 536B 前例）。
  ② noctf 孪生：体形扫描 + 符号表（temp/s53-noctf-syms.json,noctf VA =
    placed − 0x39BC000;先试符号精确命中再退体形搜索）。
  ③ 修法三分类：native 孪生直绑（词 = 0x8011<<48|(twin−0x7004000),程序算,
    勿手算）/ 需镜像 shim（页 padding 0xaef08xx 系）/ 记档不修。
  ④ 有钉死修法者做面数枚举（donor __DATA_CONST 词扫描,s59 脚本枚举器复用）,
    预写 r60 病灶表行。
  产出 = r60 候选病灶表增补（钉死修法 + 词 + 面数）,撞墙即施工。
  `0xae9bd20` 特别注：需 donor chained fixups 解析（donor 零槽 `0x7dffa68`
  blraa,文件即 0）——侦察成本高,排在 13 导入之后;信息真空先报成本再动。
  `0xb441738`（114 面 meta 表派发尾块）同样殿后:待屏证 x0/x1 才好定名。
  **C. 施工与出件（判读或预研钉出新修法后）**：
  s60-payload-surgery.py（s59 模板;坑:+KHEAD `0x1c9800`、memoryview 原位写、
  pre-image 白名单三分类、diff 审计零杂散、seal 断言未动）→ 读回自检表 →
  账本 records 156+（设备仓 kits/fileset-24A446-symbols.json）→ STATE §0 →
  双仓 commit+push → 刷写卡（常规迭代轮只 recovery_a,sha 门 + dd 读回）。
  **D. 成功旗标**：vfsinit → root/cryptex mount → SHLOG state=2。
  **E. 回退**：recovery_a ← r59 `72cc9ef4`（一级）/ r58 `ca755a65`（二级）。



## 附：会话七十三（s73/r72 全链重出轮）§0 原文（2026-11-07 s74 收档移入）

- **★★★会话七十三 = r72 全链重出（2026-11-07 深夜，用户令"开始r72"）**：诊断→修复→重建→出件一轮闭环，**出件链基座首次换成 108c 源**（r56-r71 全是 r55 基座手术，本节正式退役）。
  **⓪诊断定谳**（temp/r72-diag/，插桩复现 r55 matcher 判定）：wall 家族绑 0 是**三层门结构性漏配**——G1 shape 唯一性门：kfree_data 族=6 指令微包装器（donor 6-23 同形/native 172-174 同形，shape 永不唯一；_IOMalloc/_IOFree donor 体在 ref/native **0 同形**=LTO 内联分歧）；G2 内层配对（donor↔reference）同样配不上四靶 → reference-symbol 无名可传；G3 reference-order 缝隙填充：_IOMalloc 缝隙 length_near=0、_IOFree 4 候选被 LCS 丢弃、kfree 族连单调缝隙都不在。账本核对：s56-s71 命名靶全在（839 含 IOMalloc/IOFree/kfreevar shim+kfree_data 族），**唯一缺口=墙72**（预期，参照 emit 自动命名兑现）。
  **matcher v12 三修**（设备仓 e4bea75+4f8ba3c，vendored kit 才是运行时——主树 kit 是台账副本，v8 陈旧谱系= r73 对账项）：①`pair_call_shapes` = s71"尾跳=配对邻居"判据机制化（r55 输入 +33 绑；r72 真实输入 +8~141 随通道时序浮动）；②`anchor_reviewed` 按 `original_size` 验证（手术时代账本按 case-file 体尺寸记 sha≠fn_end 范围，839 账本上桌即炸"Reviewed function bytes changed"，修复后 839/839 全过）；③`anchor_reference_order` 抢救：LCS 首选等长候选被校验拒时，在等长候选集中找"调用签名一致且唯一"者（_ml_io_read 41 个等长候选、真身 agree=1/miss=0 但从未被评估；r55 输入 +48）。
  **Mac 链三坑（照 runbook 新增）**：④`WifiFirmware.c/BluetoothFirmware.c` 设备仓收编（108c d104c823 起强制要求板级 package 提供，sheng 上游 ce0c600 四件+`BootLatencyFiles.h` 共 129 行，fd7856a/09249ce）；⑤参照件构建须 `XNU_KERNEL_CHAINED_FIXUPS=1` 环境变量（builder 透传默认 0→ld-classic 分支→新 ld 拒收 -threaded_starts_section，defconfig m2381_defconfig:70 有 CONFIG_ 而 standalone 跑不带）。
  **★vnode_put/vnode_hold 双主定谳（s69 钉名翻案，设备仓 0e38039）**：第一遍 prelink 死在唯一 policy 违规 `stubbed apfs 0xaf215f8,1 sites(allowed 0)`。四重证据：donor 0xaf21778（216B，紧贴 _vnode_drop 尾调 0xaf2176c 之后）= **`_vnode_put`**（vnode_lock_spin casa + 内联 vnode_hold 溢出块 + put_internal_locked(vp,false) + drop_internal(vp,true)，233 热调 bsd/vfs）；donor 0xaf215f8（116B 纯递增+INT32_MAX panic，零 internal 调用）= **真 `_vnode_hold`**（r55 body-hash 字节级吻合）。s69 的 0xaf21778→vnode_hold 系 panic-PC 间接归因错（r69-r71 的 assert = mis-bind 进 native vnode_hold 的人为溢出）。账本两笔修正后 prelink 过闸。
  **闸门⑤ 全过 + 出件**：reference_sha256=448c84b7（108c 源 LTO 参照件）✓；三符号绑定>0 ✓（_IOMalloc 0xb440a08[reference-symbol]+0xb441a24→shim[reviewed]、_kfree_data[reviewed]、_IOFree 同族、shim 双条目）；**墙72 = `__ZN12IOUserClient23registerFilterCallbacksEPK19io_filter_callbacksm` 命名+绑定[reference-symbol]** ✓；stubbed 1030 项(r55)→**37 站/19 靶 + missing 2**（塌缩 96%）✓；functions_matched 9471→**17000**。kernel 743d87f4（96MiB）/payload **5a92ea2b**（99MiB）双端 sha 逐位同。
  **剩余 23 靶 = r73 判读单预注册**（r54 时代就 stubbed 的历史尾巴，policy 允许，非回归）：热点 0xb3d6498（12 kext 导入，44B "PAC 签名自指针+flag=1"初始化件，同形 0=开源树疑似永缺，参照 s45 导出旁路家族）+0xad94128（5 kext，16B getter）+Image4×4+ARMPlatform×7——首个命中者即 r73 墙73 候选，定名后走账本。**data seed 已找到**：Mac `~/unixnu/scratch-data.img` sha d04a0153d7e200bd = 工作单同一种子（s72"两机均无实体"记载有误）。Mac 未提交 3389 行经 md5 三连核对=Linux 已收编（备份=Mac stash+backup-b130f79-pre-r72 分支）。
  **★r72 首刷判读（2026-11-07 06:23 屏摄 IMG_20261107_062307）**：crc `0xd3c23f47` 一次命中（x1 补一位 f，标准噪声）。报点 `0xfffffff009bf0454`（x0 `0xfffffff388ac3340` x1 `0xfffffff00bb296a0`）hit#1 **t=2s**（r71=1s，+1s）；**stage 六行全程点亮**（PE_lockdown_iokit→dtrace_early_init→trust_cache_init→DSKextRemoveKextBootstrap→machine_lockdown→kernel do post - done）→ **墙72 消实锤**（r71 的 0xaa5ad64 sandbox/registerFilterCallbacks 未复现，死点换位+后移）。新墙 site `0x9bf0450` ∈ **com.apple.iokit.CoreAnalyticsFamily**（墙37 同 kext）。
  **★墙73 机制全解**：site 的 bl 是**直呼** kext 内 auth-thunk 池 `0x9bf8e1c`（`ldr x16,[GOT 0x827c610]; braa`，payload 字原样保留）→ GOT 槽 donor 靶 = **`0xb3d6498` = s58 案卷"未修大头 OSObject ctor 签名 isa（292 面）"家族头号**（预注册 23 靶之首，12 kext 导入）→ emit 未绑 → 槽进 stub → braa 尾跳 LR=0x9bf0454 全链自洽。语义 = arm64e kext 对象构造的**签名 isa 安装器**（`[obj]=pacda(&vtable_global,obj); [obj+8]=1`，movk diversity 常量 `#0xcda1`）。
  **关键勘误**：native 构建**有**同款惯用语且同 `#0xcda1` 常量（noctf OSMetaClass ctor 尾部内联，0x79ef030 段实拍），但**无独立 44B out-of-line 辅助件**（初搜"开源树永缺"系我 MOVK48 编码字段写错=搜了 LSL#16；修正编码后仍 0，因 native 全内联）→ **修法 = s56 iomalloc 同款 shim 家族**，非账本绑定。
  **★r73 工作单 = 0xb3d6498 shim 家族（s58 线重启）**：①ABI 深读（x1 参数角色——CoreAnalytics 调用传 x1=&0xbb296a0 自家 __DATA 但 44B 体不用 x1；caller 返回后覆写 [obj]=pacda(&0x827a6a8)——helper 的 [obj] store 对此 caller 冗余，真效果疑似仅 [obj+8]=1；11 个其他导入方的调用面普查）；②donor `0x7f077d8` global 内容与 native 对应物定位；③shim 补丁+Mach.exports+账本（同 iomalloc 三件套）+同族 0xb3d385c(292面) 一并清算；④重出件。回退=recovery_a ← payload-r71-ddbaa7b9。vnode 修正**尚未被验证**（本 boot 死在 VFS 之前，apfs/vnode 路径未跑到——r73 判读时留意）。
