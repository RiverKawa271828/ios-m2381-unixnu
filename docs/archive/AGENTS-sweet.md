# ios-meizu-m2381 — 魅族 20 Classic（m2381 / SM8550）跑 iOS 16 移植工作区

> **会话根 = 本目录**（2026-10-19 自 nura 拆出独立推进；记忆已预置本 key，无需搬家）。
> 环境：工作环境 = **Ubuntu 容器**，宿主 = Fedora——装包用 apt，构建方法优先查 nura 文档。
> 新会话首跑：确认系统提示中 memories 路径的 key 目录非空（应有 ≥2 条记忆）；若为空
> （key 与预置不同），从 nura 会话根的记忆库拷 `ios-sweet-meizu20-assessment.md` 与
> `meizu20-knowledge-in-nura.md` 两件（路径 = 系统提示中的 memories 路径，勿写进公开文档）。
> **状态（2026-10-22 八收口：★r34 窗读环判读完毕——B 终审实锤 + 连崩群清零，r35 转
> stub。报告 = `handoff/r34-window-forensics.md`，环 = rings/m2381-ring-r34。五读数：
> ①双件身份铁证（boot-args `sweet_amfi_debug=1` + cs_invalid_page 97→0 + 崩溃 pc 换位
> = r34 上机，刷写通道无罪）；②Gate1 r34mark=0 + 身份另证在 ⇒ **launchd 签名缓存整体
> 遮蔽磁盘 LaunchDaemons**（r33 改名/r34 加件同等无效；lsd envvar 未投递 = A1 两
> kill-switch 失效；launchd_unsecure_cache=1 无效路径 = 静默默认缓存，该旗实验作废）；
> ③Gate2 = 28 entitlements PASS（AMFI 无罪）；④Gate3 = mad 52/52 全崩 @ **stock stub
> line321**（brk@mh+0x1BF3BC，旧 0x1BF5BC = line247；同一条 main() 错误清理链，容器骨架
> 未介入此 fatal）；⑤**Gate5 大捷：SIGABRT 连崩群 548→0**（六 daemon 各 exec 1 次零崩
> 零重拉 = 容器机制复活，containermanagerd 同活）。Gate4 = lsd t2 与 r33b 16 帧全等
> （MadGate 永等未变）；SB 脱 os_state 死锁改等 lsd seeding；bbd 正常。**全链唯一堵点 =
> mad fatal**。★r35 排序：主攻 = `/usr/libexec/mobileactivationd` 原地换已签名即退体
> （vm_stat 招：缓存照常拉起→即退→lsd 得 error 非永等；零 CDHash 问题；判据 = CRASH
> mad=0 + t2 换栈，CRASH mad=0 本身 = 体内身份）；并行实验 = 改 launchd.plist 缓存 +
> sweet_amfi_debug=1 放行（前置宿主 RE：amfi hook 是否覆盖 cache_loader trustcache 查询）；
> XPC 全功能 stub（A6 备料）后备]。r34 出件细节存档：userdata `iOSRoot-s-r34.simg 7aaab96f`
> + boot `shim-boot-m2381.img 339b8480`（内核 012f596f/ADT c85b01f0；**勿刷 b8cddc98**），
> 手术/验证全链 = `handoff/glm-report-r34.md`。此前同日：r33b 窗 97× 崩溃循环 + 版本身份
> 不可判；CI 断链破案修复 [dxb 静默丢 14 件补丁，0054 根因+13 级联，全链走查 96/96 绿；
> r31 窗实证全部生效]。
> **★上游切换（10-22 用户令）：跟踪基准 = reference/ios-topaz
> origin/topaz**（MakrSas fork；其 topaz 分支达桌面+触摸+按键+Wi-Fi；机制深潜 =
> NOTES-topaz.md 先读；新动态走 PR：#1 GPT 写栅栏/#3 触摸释放排队采纳）；**不换基座**
> ——共同基座 0001-0079 与 topaz 同源（43 件 m2381 化 =有意差异），0080+ 其 SM6225
> 定制勿整段照搬。★新纪律：每轮 CI 必查跳单（Applying=96+4 dxb 基线件），推送前必跑
> `tools/xnu-gate-walk.sh`（全链严格走查，守卫类改动尤其要跑全链）；★符号化工装 =
> research/bin/ipsw + `ipsw extract --dyld` 从 IPSW 抽 DSC（root 内 cache 被 mkiosroot
> 清成 0 字节，7z 解不了 decmpfs，必须走 IPSW）。
> 回滚 mu-r69 在 `/home/ri/work/nura-meizu-m2381/artifacts/`。
> 基础设施：★shim SWEETLOG 读回探针（`M2381=1 bash shim/build.sh` 无参构建，读 userdata
> 尾部日志环，IPS 预扫 + 过滤行 + TAIL）；cmdline 定稿串见 NOTES 轮 20 条目（增改只做增量；
> 注意 0069 拥有 0047 的 tsleep 行，0047 勿再动，刷写间隔走 boot-arg `ufslogsecs`）。
> 续接 = ★下一窗交接对象 = DeepSeek harness，**唯一入口 = `handoff/deepseek-handoff-r35.md`**
> （自足操作书：r35 = mad 原地换 vm_stat 即退体；手术/刷写/读环/判读全配方命令级 + 环境
> 陷阱清单 + 升级路径；背景深读 = NOTES 10-22 八条目 + r34-window-forensics.md）。

## 这是什么

把 ios-sweet 证明的「iOS 16 xnu-8796 + 真 iPhone X 用户态跑在 Qualcomm 安卓机上」路线移植到
**魅族 20 Classic**（用户 10-19 确认机型：20 Classic = m2381 板号、16GB LPDDR5X，与魅族 20
仅 RAM/存储配置差异，nura 全部实证出自这台实体机）。可行性判词（10-19，全 FDT 实证）：方向成立，启动链交接协议吻合
（shim = Linux Image 头，ABL 以 EL1+DTB+initrd 进入）；真成本在 XNU SoC 平台移植（~2800 行核心
补丁）+ 触摸/按键驱动，不在启动链。全部判据与 SM8550 常量见本仓记忆 `ios-sweet-meizu20-assessment`。

## 布局（2026-10-19 重建：工作仓在顶层，参照克隆收进 reference/）

```
（顶层）        工作仓 = ios-sweet 结构骨架（git 本尊，origin = 上游 dipa-studio/ios-sweet
                只读，tip fdad0de）：shim/ tools/ xnu/patches/ NOTES.md README.md .github/。
                移植改动直接在顶层树上做，与上游零发散起步（有意发散 = AGENTS.md，
                本地 commit 52eadd9 + 325d5ec）。
AGENTS.md       本文件（上游原版随时 `git show fdad0de:AGENTS.md` 找回）
reference/      参照克隆（.gitignore 忽略，不进工作仓）：
  ios-sweet/    dipa-studio 上游原样克隆（对照基准，tip 与顶层一致）
  ios-topaz/    MakrSas fork——★10-21 实况：已整体重定向到新机型 Redmi Note 12 4G
                （SM6225「khaje」，代号 topaz），不再是同设备姊妹 fork；**活跃分支 =
                topaz**（本地 checkout 的 main 停在 0079 已过时；fetch 后 origin/topaz
                = 0139+，README 状态 2026-10-20 = home screen+触摸+按键+Wi-Fi 全通；
                GPU 用 HackiOS 私有补丁在机工作、源未公开故不入仓）。读法：
                `git -C reference/ios-topaz show origin/topaz:<path>`。对 m2381 的
                价值：①其 backboardd 稳定跑到桌面 → 我机运行中 SIGABRT 属环境/配置
                差异非移植根本缺陷；②0092-0107 显示链（MDP 直扫 iOS surfaces/双缓冲
                vsync 翻转/真实 dcache 维护——Apple SoC 配置把 clean_dcache 变 no-op）+
                0114 对齐裁边画布 = 阶段 3 显示路线图；③0131-0139 SweetSurface 系列
                （0137 backboardd seed-read-after-release 崩溃修复等）= 我机 backboardd
                SIGABRT 案发区（SWEET SURF create 之前/之中）的现成药箱；④0138 空
                IOAcceleratorES：QuartzCore 每次 backboardd 启动对 IOAcceleratorES
                空轮询 300×100ms=30s，补一个空 service 免等；⑤0088 GPT 看门狗 +
                RECOVERY.md（XNU 写 userdata 时防 GPT 毁，与我们的日志环/IPS 镜像同
                分区共存相关）；⑥0113-0130 TopazModem = Wi-Fi 全内核栈模板（modem PAS
                boot+GLINK/QRTR+WLFW+WMI/HTT+WPA2，自 topaz-woa 移植；m2381 若做需换
                WCN6750+nura DT 真值）。原 sweet 期三件（0078 八核含 2×A76 / 0079 显示
                电源 / 0077+tools/sweet-usbnet.sh USB-net 日志 UDP→192.168.7.1:5140）
                仍在 main 未删。**跟随策略（10-21 用户拍板「跟着他的进度来」）= 不换
                基座**（0080+ 全是 SM6225 定制，我树共同基座 0001-0079 已全有；m2381
                平台面 8 核 ARMv9/UFS4.0/用户态全链领先其平台深度），按其路线图定向
                移植设备无关件：SIGABRT 破案后 SweetSurface 0131-0139 + 0138 直接
                翻版；触摸轮用其 0083-0090 diag 打法打 GT9916；Wi-Fi/GPU 待 B/C 阶段
                后评估（GPU 跟踪 HackiOS 开源）。桌面里程碑不依赖 0080+——sweet
                0069s = 0046 时代内核 + 根盘 v12-v14 tweaks，两样我树皆备。
                **★10-22 用户令「上游改为 topaz」**：跟踪基准正式切换到本仓（fetch 后
                topaz 分支 tip 未动 89451b8；新动态 = PR #1 GPT 写栅栏/#3 触摸 0116 释放
                排队采纳，#4/#5 = 第三方新机型移植仅供参照）；不换基座判定不变——我树
                0001-0079 与其同源（43 件 m2381 化 = 有意差异，10-22 断链修复只动了
                0054/0089），其 0080+ SM6225 定制勿整段照搬。
```

NOTES.md = 上游 sweet 全程日志（翻版某 00XX 补丁前先查对应条目；84 个补丁本体在 xnu/patches/）。

同步纪律：上游跟进 = `git fetch` + cherry-pick（勿直接 pull/rebase，会撞 AGENTS.md）；
推送 = 用户自己的 fork remote（窗口用户拍板）。reference/ 两仓只读不改动。

## 启动向量（2026-10-19 定稿；fastboot boot 判死源自 9-21 卷一实测，勿再提）

1. **`fastboot boot` = ⛔死（历史禁令，档案实锤）**：魅族 ABL 裁剪 CmdBoot 本体——
   9-21 M1 首试 `unknown command`（pmos_linux experiment-log 卷一 #1；（四十一·乙）再钉
   「历史禁令进协议设计前必须过一遍」）。RAM 启动路线在本机不存在。
2. **boot_b 换装 = 唯一主路径（确定性成立）**：ABL 从 boot_b 引导 boot.img 是现役事实
   ——Mu 链正是为此而生（Mu 伪装内核经 ABL 跳转的打包先例，shim 同构；**Mu 镜像 =
   header_version 4 + kernel + 占位 ramdisk，日日实机背书**）。shim ↔ Mu 换装各一条
   fastboot flash（秒级），Mu 回滚件全在 nura 档（**mu-r69-97cdea20**，sha256
   `97cdea208214e655e055d815bdcc307d00d8990fd371abeebabf4c31c14326f6`，实测自
   `nura artifacts/mu-r69-97cdea20.img`）。★勘误（10-19）：旧排面写「mu-r89」= 版本制混写——
   #89 是内核 boot 数（内核 = r88），Mu 与内核版本制完全独立，Mu 现役 = r69（10-06 起 DTS
   零改动沿用），**r70-r89 不存在**（artifacts 实测 Mu 全史 = r62/r64/r66/r67/r68/r69）。
   shim 死挂恢复 = power+vol down 重进 fastboot 刷回**读回原件**（mu-r69 或 Flyme stock，
   见 §阶段0 流程①，勿盲刷 Mu），兜底 9008。
3. **recovery 槽 = 三重死，勿再考虑**：①本链 recovery_a = Mu 的 ESP（Linux 引导载体），
   刷 shim 即毁 pmOS；②GKI 2.0 恢复槽无内核（9008 备份：boot_b kernel 46.9MB/ramdisk 0，
   recovery_a kernel 0/ramdisk 16.7MB），ABL recovery 引导 = boot 槽内核 + recovery 槽
   ramdisk 配对；③**AIOS ABL 把 recovery 分区纳入正常启动校验链**（卷一 #3 实录：外来
   boot.img 格式 recovery → LoadImageAndAuth 失败 → 重启循环，清 misc 无效）。

⚠另注：主线 Linux 经 ABL 直启 <1s 必死是 R1-R19 旧墙（Linux 特有；Mu 类自足固件实机跑通）。
shim = Mu 同类裸金属自足载荷且异常全画屏可视（蓝屏 panic 带槽位号），该风险类被 shim 自身
诊断设计覆盖。

## 开工排面（2026-10-19 钉；每阶段独立可停，commit 不 push，推送窗口用户）

### 阶段 0 = shim 探针轮（头号目标：ARMv9 兼容性从猜变实测；零 userdata 损失）

**四个常量改写**（全部 Flyme stock FDT 实证，素材 meizu20-m1/flyme-forensics-20261019/fdt/）：

| 常量 | sweet 值（在库） | 魅族 20 值 | 位置 |
|---|---|---|---|
| FB_BASE | 0x9c000000 | **0xb8000000**（size 0x2b00000；1080×2400 a8r8g8b8 stride 4320 与上游同） | shim/console.h |
| GICD_BASE | 0x17a00000 | **0x17100000** | shim/shim.c |
| GICR_BASE / PE stride / 区大小 | 0x17a60000 / 0x20000 / 0x100000 | **0x17180000 / 0x40000 / 0x200000** | shim/shim.c |
| VTIMER_INTID | 19（SM7150 惯例） | **27**（标准 PPI11；PTIMER=28） | shim/shim.c |
| IMEM_RESTART_REASON | 0x146aa65c | **同址 0x146aa65c；★10-19 探针实锤：预写魔数 0x77665500 + PSCI 复位落回正常启动（非 fastboot）→ 魔数表不合本机 XBL，此路禁走**；修好的软复位 = PMK8550 PON reboot-mode（内核 qcom-pon 路径，需 SPMI 驱动——阶段 1 顺 sweet 0060 框架做） | shim/shim.c（魔数写已删） |
| PSHOLD | 0x0c264000 | **悬置不写**（nura 判定 10-19：PSCI SYSTEM_RESET 本机实证可用——pmOS 无数次 `systemctl reboot` + `reboot --reboot-argument=bootloader` 落 fastboot 均工作[软复位 = 内核 qcom-pon 写 PON 寄存器，非 IMEM]；mainline sm8550 dtsi 与 stock FDT 均**无 pshold 节点**，arm64 重启链唯一活口就是 PSCI[WDT bark 被 XBL 门闩封死=硬定律#10]。0x0C264000 是 Qualcomm 跨代惯例地址但本机零验证，勿盲写） | shim/shim.c |

**编译+打包** = `M2381=1 bash shim/build.sh`（aarch64-linux-gnu-gcc 交叉 + 零绝对重定位自检；
gzip kernel → mkbootimg.py 打 v4 → unpack_bootimg.py 自动拆回对拍 kernel/ramdisk；sweet 旧形态
= 不带 M2381 默认）。mkbootimg.py/unpack_bootimg.py 在
`/home/ri/work/meizu20/meizu20_linux/tools/mkbootimg/`（MKBOOTIMG= 可覆盖）；配方 = Meizu ABL
已验形态（meizu20-m1 surgery 先例 + mu-r69 日日背书）：
```
mkbootimg.py --header_version 4 --pagesize 4096 --os_version 13.0.0 \
  --os_patch_level 2023-10-05 --cmdline "" \
  --kernel shim/out/Image.gz --ramdisk shim/out/ramdisk.bin --output shim-boot-m2381.img
```
探针形态 = shim 不带 payload（自动进全硬件探针：PARANGE/MMFR[TG16]、PSCI/SMCCC 版本、
8 核 MPIDR[Aff1 = 0x000..0x700，nura DT 与 sweet `i<<8` 目标格式同构、test_smp 零改]、
INTID27 vtimer 计数 + SGI、UFS 只读 probe——**v3 起 gic config 段在 MMU-on nGnRE 映射下
跑**[mmu_enable_probe：全 Device 恒等映射保 ufs DMA 共享性 + GIC 32MB 块 attr3=0x04]，
库内零 SCSI WRITE，安全）。**★复位政策（10-19 两轮实锤后钉死）：shim 永不自复位，跑完/panic
一律坐屏等人，pwr+volDn 手动回 fastboot**——第一轮的 40s PSCI 自动复位落回正常启动引发
boot 循环，把 ABL slot-retry-count:b 烧到 1（差一步自动翻 a 槽，nura 有记录），手动抢救 +
`fastboot set_active b` 复满。**★探针轮收官（10-19 晚，五轮上机，详 NOTES-m2381.md
Session log）：阶段 0 完结，六项全绿**——tg16=1（16K granule 定谳）/PARANGE 40bit/PSCI
1.1+SMCCC 1.3/8 核 ret=0（mpidr 81000100..0700）/**INTID27 vtimer 300ms 恰 30 拍**/UFS
ver=0x0400（UFS 4.0 裸金属复证）；**SGI 自发 10 发 10 中**。**ARMv9 兼容性定谳，头号风险
关闭。★GIC SGI 页优先级写秘方 = 先置 GICD_CTLR.nASSGIreq（bit8，GICv4.1「SGIs without
active state」）再碰 SGI 页优先级**——未置位时 NS 写 = 实现定义中止（R1-3）或静默丢弃
（R5 map），置位后写粘（PRIRD=A0A0A0A0）；与内核 dmesg「Enabling SGIs without active
state」互证（gic_dist_init 置位 → gic_cpu_init 才写）。本机 GICD_CTLR.DS=1 单安全态
（0004 注释 two-security-states 按 m2381 实况改写）；真 GICD_TYPER 在 **+4**（016B001F），
+8 读值非 TYPER 形状（footnote）。**XNU 0004 翻版硬要点：pe_init_fiq 的 8 笔优先级写
之前必须置 bit8**。RE 线（s5-rea Ghidra 工装）副产品：UEFI 前驱 ArmGicDxe 只写 SGI 页
+0x80 不碰优先级不置 bit8（反编译件在 s5-rea/out/）。现役探针件 **v5**：
`shim/out/shim-boot-m2381.img` 20,480B，sha256
`8a44b6e701a7b0e2a4c75f4fbd240edbdf7d00fe364bcdc39c1928e4b49cd9a7`（可恢复中止全类门 +
SGI 页 20 偏移写边界图 + GIC 身份证 + nASSGIreq 实验 + UFS 前置 + panic INSN/寄存器扩展 +
X30 修复）。设备现态 = boot_b = shim v5 坐屏（DONE），**收口（交还设备）= flash mu-r69 +
set_active b + 读回对拍**；续接 = **阶段 1 XNU 平台移植**（NOTES-m2381.md 顶部 STATE）。

**流程**（全程只碰 boot_b；**读回通道 = 有 root 的 Flyme 本身**[su dd → /data/local/tmp →
adb pull]，不起 pmOS——起 pmOS = 重刷三件，破「只碰 boot_b」）：设备形态 = **Flyme OTA 落
b 槽 + b 槽活动**（FLASHING.md §1 验证机形态，`fastboot getvar current-slot` → `b`；★10-19
勘误：早前「必须 = a」说法错误，a 槽启动反而未验证勿盲试）→ boot_b = **现役槽换装舞台**
（Flyme boot ↔ mu-r69 ↔ shim 三态轮换，换装窗内 Flyme 暂不可启属预期；init_boot_b[Magisk
root]/super/userdata 全不碰，root 无感）：①读回 boot_b（通道随现役系统：**nura OS 运行时 =
devsh**[tools/devsh.py @172.16.42.1:23，节点 /dev/disk/by-partlabel/boot_b = sde43，10-19 实测]；
**Flyme 窗 = su dd**[/dev/block/bootdevice/by-name/boot_b → adb pull]），**对拍两个预置 sha 三分类**：
整分区 sha =`5f9f1503…`（= 原厂 Flyme boot，meizu20-m1 10-02 黄金备份）→ 复原件 = 该备份；
**前 1,110,016B** 前缀 sha =`97cdea20…`（mu-r69 是 1.1MB 镜像文件，整分区 dd 是
100,663,296B——mu 只能前缀对拍，勿拿整分区 sha 硬比）→ mu 在机，复原件 = mu-r69；
都不是 → 第三态，存底档先问用户再动；② `adb reboot bootloader`[nura OS 在机时 = devsh
`systemctl reboot --reboot-argument=bootloader`，10-19 实证] → `fastboot getvar
current-slot` **实证 = b** 才继续 → `fastboot flash boot_b shim/out/shim-boot-m2381.img` →
屏幕眼判六项（TG16=16K 页支持 / PARANGE / PSCI ≥1.0 / INTID27 timer 计数 / 8 核 MPIDR 全报 /
UFS LUN0 读通）；③ **探针坐屏不自动复位（v2 起）→ 人手 power+vol down 回 fastboot** →
**多轮循环：直接刷下一个探针件再 reboot，回刷 mu-r69 不是必须**（10-19 用户纠正；坐屏设计
下 boot_b=shim 跨轮无风险[开机即蓝屏坐稳，每开一次只烧 1 个 retry]，计数低了随时
`set_active b` 复满——不碰分区，任意时刻可做）；**收口（会话结束/交还设备）才**：
flash 回 ① 定义的复原件 → `set_active b` → 收口读回对拍（同①分类法）→ `fastboot reboot`
回现役系统（全程无恢复/重装动作）。蓝屏 = panic 追加打印不清屏（v3 起），逐项修常量
重试。会话结束必须复位原态或在交接块写明 boot_b 现留件。

**判据**：六项全绿 → ARMv9 兼容性定谳，进阶段 1；timer 不计数/UFS 死 → 常量级问题，回改重跑。
顺带评估：topaz 0077 USB-net 日志通道移植面（SM8550 dwc3 vs SM7150）——立起来后阶段 1-3
调试效率提一档，越早越好。

### 阶段 1 = XNU 平台移植（kalama/m2381）

**★施工面完成（2026-10-19 深夜；详 NOTES-m2381.md Session log 同日条目）**：11 件翻版 +
M2381.h/MASTER.arm64.m2381/MeizuPlatformExpert/MeizuCPU 终态 + mkadt `--m2381` 全部入树，
严格 walk 84/84 零跳过、SWEET 回归审计 PASS。剩余 = 推 fork → CI（machine_config=M2381）
→ mkpayload 上机。**本地验证纪律**：xnu-8796 真源树 scratch 逐补丁 apply（dxb build.sh
对失败补丁静默跳过，任何 fuzzy/失败都 = CI 丢补丁）。

- sweet 补丁翻版清单：0003/0004/0006/0010/0011/0012/0016/0017/0018/0065/0078 → meizu20 版
  （machine config、fb console、pmap/physmap、early-fb、DT timer irq、boot CPU driver、SMP PSCI）。
- **pmap 注意**：魅族 **16GB** 多段内存（stock FDT memory 节八段可见 15.69GiB）——
  **★定谳（10-19 深夜）**：kernel-managed RAM = **0x8c0000000 起连续 13GB**（八段中
  0x8_c0000000[5GB] + 0xa_00000000[8GB] 首尾相接），单连续窗模型零内核改动；低银行不入手
  （XBL/XPU carve-out 带）；16K L1 槽 = 64GB → fb 与 RAM 同槽，0010 机制原样适用。
- SMP：A510/A715 = ARMv9；topaz 0078（2×A76 大核点亮）是最近亲先例，`cpus=1` 起步再逐核放。
- 构建走 CI（macOS runner）：用户 fork 本仓到 GitHub → `gh workflow run xnu.yml`（workflow
  随树在 `.github/`）；本地单文件预检 = `tools/xnu-cc.py` 省枪。
- ADT = `tools/mkadt.py` 出 m2381 版（设备身份保持 iPhone10,3；disp0 canvas 1080×2400）。
- **判据**：XNU kprintf banner 上屏 + 持久日志环可读 + panic 可视。

### 阶段 2 = UFS root

- UFS 补丁 0028/0030/0031/0034/0069 翻版；**UFS = 4.0 512GB，已定谳**（★10-19 实证链：nura
  OS SCSI INQUIRY 报出 `SAMSUNG KLUFG8RHHD-B0G1` → Samsung 目录 = UFS 4.0 512GB[153-ball]；
  20 Classic 按生产批次 2024-04-29 前后分 4.0/3.1，本机 = 前批次。早前笔记「3.1」作废。
  阶段 0 探针 ver= 寄存器预期 0x04xx，裸金属侧再对一次）。sweet 是 2.x，「VER 寄存器/版本
  处理要过一遍」工作面不变[UFSHCI 4.0 寄存器图向后兼容，Gear5 2-lane 由 ABL 拉起续用]；
  **控制器同址 0x1d84000**，ICE 0x1d88000 无视[stock FDT 带 ufs_ice+ufs_ice_hwkm 节]，
  PHY 0x1d80000；「续用 ABL 已拉起链路」策略成立——魅族 ABL 同样从 UFS 启动。
- `tools/mkiosroot.sh` 出根（iPhone X 16.7.16 20H392 IPSW；root 入 userdata=sda22——
  **抹 Flyme 用户数据，动手前用户拍板**）。
- **判据**：`rd=ufsroot0` rw 挂上 → launchd 起来 → 守护链到 backboardd。

### 阶段 3 = GUI 与输入

- 显示：AppleCLCD CPU copy（fb 0xb8000000；cmd-mode BOE AMOLED DSC 面板 splash 连续扫描出
  有 pmOS simpledrm 先例背书）；0046 IOSurface/IOMFB 原样继承；0061 显示身份；0079 显示电源
  照 topaz。
- **触摸 = Goodix GT9916（berlin 族）走 GENI I²C**（★10-19 勘误：stock FDT 表面是 SPI 节，
  但 **nura 内核日日实跑 I²C 形态**——i2c4@a90000 400kHz、0x5d、irq tlmm25 edge-falling、
  reset tlmm24 active-low、avdd l14b_3p2 / vddio l12b_1p8、1080×2400）。sweet 0060/0063 本就
  是 GENI I²C 框架 → 翻版成本 = 换 GT9896→GT9916 协议细节（mainline goodix_berlin 驱动 +
  nura 在机行为 = 双参考）；avdd rail 勘异项：stock humu L9B ~2.96V vs nura l14b 3.2V
  （nura 台账在案）。
- 按键 = PMK8550 PON（`pon_pwrkey` + `pon_resin`=vol-down，nura DT 主线惯例形态在库；sweet
  0060 的 SPMI 框架翻版换 SID/中断号）。
- **GPU 不做**（判词见 NOTES-m2381 §GPU：Linux 侧 proven 但 XNU 侧 Adreno 零代码，KMD 移植 =
  独立研究线）；显示流畅度升级走 **DPU/vsync 路线**（nura 主线 panel+DPU 驱动 = 寄存器真值）。
- **判据**：锁屏 → 上滑解锁 → home screen（对齐 sweet 10-18 状态）。

## nura 借鉴地图（用户拍板「遇困难借鉴 nura」——先查这里再动手）

| 卡在哪 | 去 nura 查什么 |
|---|---|
| 任何 SM8550 MMIO/中断/regulator/时钟真值 | nura 内核 fork `/home/ri/work/linux-mobile-ports`（分支 meizu20-t4b，`sm8550-meizu-20.dts`）+ Flyme stock FDT 反编译——**Linux 在此机完整跑起 = 这两处就是硬件地面真值** |
| 面板/splash/DSC 行为 | nura `docs/PANEL-CORRUPTION.md`（cmd-mode BOE AMOLED DSC、splash 连续扫描出、§5④ bloff 判词） |
| 刷写/验证纪律 | nura `docs/FLASHING.md` §8 + dd 读回才算证据 + fastboot 单通道律 + bringup-traps 记忆（33 坑） |
| 启动链原理（Mu/MsBootPolicy/ESP） | nura `docs/BOOT-CHAIN-RE.md` + `docs/DESIGN.md` |
| GENI I2C/SPI/SPMI 探针实战与坑 | nura 私有 `docs/meizu20/experiment-log.md`（触摸/录音/NFC 轮；i2c bind 顺序、geni autoprobe 等坑全在册） |
| 睡眠/唤醒（后期才碰） | nura `docs/SUSPEND-S2IDLE.md` |
| 战役级求助 | nura 会话根 = `/home/ri/work/nura-meizu-m2381`（记忆 40+ 条）；叙事档 = `/home/ri/work/pmos_linux` |

## 红线（从 nura 继承，一条不少）

1. 刷写只碰 **boot_b**（shim 载体）+ **userdata**（iOS 根）；xbl/abl/tz/modem/dtbo/super/persist
   永不碰；`--i-am-present` 纪律 + dd 读回才算证据（刷写时长不算）。
2. **抹 userdata = 抹 Flyme 用户数据**——动 userdata 前必须用户拍板（阶段 2 的硬闸门）。
3. **fastboot 单通道**：本仓与 nura 侧任何刷写互斥。
4. 回滚件全在 nura 档（boot_b = **mu-r69-97cdea20** 可随时复原[另注意设备在 Flyme 窗口时
   boot_b 现内容需读回判定]；pmOS 三件重刷配方齐）。
5. iOS IPSW 是 Apple 版权件：只本地使用，不入仓不外发（沿 ios-sweet 同一政策）。
6. 每阶段独立可停；commit 不 push，推送/发布窗口 = 用户拍板。
