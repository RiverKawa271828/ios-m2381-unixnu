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
