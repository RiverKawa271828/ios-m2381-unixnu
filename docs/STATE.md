# STATE — 现势状态（m2381，2026-11-15 更新）
## 0. 现势指针（每轮首更，2026-11-15 会话一百二十一；更早指针=archive/STATE-history.md）

## 0. 现势指针（每轮首更，2026-11-15 会话一百二十二；更早指针=archive/STATE-history.md）

**★★★★★★★★★★s122 终档：r129 容量轮已刷已判（用户面三门过，2026-11-15）——G0 ✓ 进锁屏（LZMA 解压路径机上工作=本轮主命题结）、G1 ✓ 充电正常（r128b 里程碑保持）、G3 ✓ boot 时长与 r128 无可感差异（解码 ≤秒级）。**判读债（下会话开工单第一刀）**：窗面哨兵（ADSP st=13/battery st=4/AKS st=0 sg=3/SMP req=1/DATA st=2）不拉窗，r130 触屏轮开工前先核 reader-r129 脸（FD 换代后的单变量卫生；不核就叠新变量=回退难归因）。原出件账：侦察三件全清——①上游 UniXNU/DarwinBoot/UniXNUPkg 全链**无现成压缩形态**（tip 5243c68 零 lzma；s118 挂账预案 A 落地，落点=主树 drivers/DarwinBoot（loader 源头）+ uefi 树仅复用其 Mu_Basecore SDK，uefi 仓零文件改动），复用公共域 LzmaDec SDK（vendored 六文件零改字）；②recovery_a 分区实测定谳 **=100 MiB**（FLASHING §3），size_mib=99 贴墙（卷内余 5.4 MiB / kernel 91.46 MiB）；③修法拍板=**成员名不变、内容 LZMA-alone**（FORMAT_ALONE preset 9e dict 16MiB 补写已知 size；91→17.5 MiB=5.22x，解码 host 0.7s/机上估 3-10s），DarwinBoot `ExpandKernel` 检测 props 0x5D+已知 size 后 `LzmaDecode`（dest 当字典，64KB scratch bump 分配器）再 ParseKernel；plain 内核照旧接受=回退只换 payload。**内核零改动**（member 解压=在机 r128 内核 6b4642eb 逐字节同）+ 种子/trust/vroot 原样回收（9c28654c/dc655bcb/5c976276）+ **args/TOML 与 r128 逐字同**；`--kernel-probed` 新开关复用已补内核（probe/seal 拒收二次执行）。host 门全绿：test-lzma-kernel 6 checks（生产代码抽取编译验证：round-trip/corrupt/oversize/unknown-size/passthrough）+ 上游 parser/ramdisk 测试 PASS + 固件 -Werror 零告警 + check-m2381 ✓。**件**：boot_b ← `out/m2381/boot-b-fc2e8f17-r129.img`（31,903,744B，新 FD 1da8600f + **固件段逐字节同 r128b**：adsp a9b3fd8a/dtb ee426dfc，V8 PAS metadata 未动）；recovery_a ← `out/m2381/payload-r129-lzma-676e2e5d.img`（99MiB，**卷余 79.3 MiB = GPU/audio/addon 预算解锁**）。卡 = `out/m2381/R129-CARD.md`（G0 到锁屏=主门 + UEFI 屏打 `kernel lzma 18482928 -> 95928320 bytes`；G1 ADSP st=13 + battery st=4 不回退；G2 回归哨；G3 容量+解码耗时；stall 脸先裸重启=s121 协议；红线=绝不 data-reset）。已知良性差异：卷上无 crypto-hmac.json（host 侧 seal 产物零设备消费）、ADT sha 代代异（随机种子）。**刷后编排**：r130 触屏（Goodix GT9916；开工第一刀=核 r129 窗面哨兵债）→ addon+WiFi 同轮 → GPU 大会话 → 音频（已解锁，ADSP 起）。立账（本轮顺手账已收两件）：①**DockFolderViewServic sig11 定名 = DesktopServicesPriv `TNode::GetNodeFromPath`+0x2b4**（s117 sym.py 管线，去滑 0x20f9723dc；Dock 文件夹枚举路径解析崩，2s 重生环，fresh-Data 谱系用户态症状，修法 RE 开放）；②**NativeUfs stall 侦察毕（只读）**：run4/5 脸机制钉死 = `verifiedVolumeQuery`（NativeVirtioBlock.cpp:250，expectedUUID 对拍）取到 IOMedia UUID **F914C62A…**（native_verified_block_root_uuid@:281=发布面 UUID）≠ 信封签名 volume_uuid（`NativeUfsBlock::authenticatesVolume` NativeUfs.cpp:1791、strcmp:1807 → 0）→ root 不发布 / UFS requests 冻 5 / manifest digest=0；SHNGROOT（RootLayout.h:159）=版面 magic 非尾窗内容；修面 = UFS 窗 IOMedia UUID 派生源 vs verified-root-24A446 信封一致性（RE 轮再开）。③长会话中断预算看门狗（658/625us@machine_routines_common）备忘保持；**⑤设备身份/UI 形态题（用户提出，2026-11-15 侦察定谳）**：现役 ADT 身份实测 = iPad17,3 / J820 / j820ap / chip 33090（profile `[ipsw]product`+`[adt.root.string]`），24A446 iPad IPSW 仅含 iPad17,1–4；锁屏示"按 home 键进入"而本机无 home。上游两派实录：①**iPad 身份派**——`asteroids`=Nothing Phone (3a) 用**同一份 24A446 iPad17,3 镜像**实录"to the home screen, with touch, buttons"（同 donor 手机先例=平板 idiom 非宿命）；`alioth` 17.7.11 亦接受 ~ipad idiom（代价：壁纸只认 ~ipad 集合/device-class 改造）。②**iPhone 身份派**——`cupid`=小米12 自建 `ios-27.0.1-24A446`（iPhone17,2/D94AP，不同 IPSW/kext/ADT identity + XnuPrelink fixture 重定向），理由原文"An iPhone profile is required for a phone: iPadOS presents iPad product identity, idiom and display assumptions that SpringBoard and UIKit read from /product"；`apollo`=iPhone10,3+iOS16.5 同款老线。**定谳**：身份=OS profile 级四件套（IPSW+kext+fixture+ADT），非 ADT 字串旋钮（上游零"只改串"案例）；本机 ADT **无 buttons 节点**（实测无 buttons/home/faceid 串）；上游按键走驱动面（sheng ButtonProbe + sheng_buttons_press 经 HID sink，非 ADT 节点）→ 解锁缺的是**输入通路**（触摸 r130 / 按键驱动），非身份；"按 home 键"提示的 idiom 判据待 RE（capability 位 vs 节点缺失未分）。**r130 预置**：A 补 buttons/按键映射（最轻，与触屏同轮）；B ADT 身份探针（单变量可回退，仅诊断）；C 另立 iPhone 24A446 车道（=换道量级，需 iPhone IPSW+fixture 重定向）。

**⑥目标设备对齐台账（用户拍板 2026-11-15：功能集对齐 asteroids，iPhone 身份单列评估）**。

| # | asteroids(Nothing Phone 3a) 已具备 | 我方现状 | 归属轮 / 缺口动作 |
|---|---|---|---|
| 1 | 进 home screen（真实桌面） | 锁屏点亮，未解锁 | **r130 触屏（上滑）** = 主门 T2 |
| 2 | 触屏（含多点/释放） | 无 | r130 |
| 3 | 按键（power/vol → HID，ButtonProbe+sheng_buttons_press） | 无（ADT 无 buttons 节点；驱动面缺） | r131 按键轮（或 r130 若 touch 即通则顺手） |
| 4 | GPU addon（Turnip + Metal，Adreno 810） | 仅 GOP 软件呈现（r129 前身） | GPU 大会话（上游 gpu-power=9/gpu-metal=1/gpu-addon=1 整拿） |
| 5 | 系统 App / 图标 | donor 卷内已有，未达桌面 | 随 1 解锁自然暴露 |
| 6 | 壁纸（device-class 改造 `@3x~ipad`） | PosterBoard 在册未见绘出 | 随 4 之后（GPU）或独立小轮 |
| 7 | Wi-Fi（WCN6750 ath11k，含 DHCP/上网） | 无 | addon+WiFi 同轮 |
| 8 | 蓝牙（2026-11-13 起） | 无 | Wi-Fi 后轮 |
| — | 音频（我方已有 ADSP st=13 底子；asteroids 未标 Audio） | ADSP 活，未接音频栈 | 音频轮（ADSP 已解锁） |

**判据**：每轮完成即以该行为准，逐行打勾；不合并轮次（单变量纪律）。

**iPhone 身份评估门（决策项，非施工项）**：
- **触发**：r130 触屏解锁成功之后（输入通路解决后，残留的"平板 idiom"才算真问题）。
- **评估输入**（四类可量判据）：①锁屏/桌面最终布局与 20:9 面板的匹配（缩放、Safe Area、圆角/缺口假设）②系统手势（上滑返回/多任务/分屏）是否符合 20:9 手机形态 ③壁纸 device-class 与 PosterBoard 集合（现仅 ~ipad）④App 呈现（iPad 专属 UI 在手机屏上的排布）。全部四项可接受 → **no-go（留在 iPad 车道）**；任一为实质阻碍 → **go**。
- **go 的代价四件（已侦察）**：iPhone 24A446 profile（24A446 iPhone 支**仅 iPhone17,1**；iPhone 11=iPhone12,1 不在集）、kernelcache prelink fixture 重定向（13 vnodeop_desc 按 vnop_* + 8 metaclass 槽 + 5 符号）、trust cache+userdata 重建、**Data seed 重建=谱系清零（唯一贵项）**。
- **产出**：`docs/STATE` 一条 go/no-go 决定记录 + 若 go 则立独立换道轮工作单（等价 21H461→24A446 量级）。 **目标设备对齐（用户问，2026-11-15 定谳）**：24A446 支持集实录 = iPad 支 `iPad17,1–4` + **iPhone 支仅 `iPhone17,1`（iPhone 16 Pro）**，另被引用 iPhone17,2（cupid）/iPhone17,4（asteroids）——**iPhone 11（iPhone12,1）不在该支持集**（记忆纠偏）。适配最好 = ①**sheng**（同 SoC SM8550，触屏/GPU/WiFi/BT/音频/电池/键盘全 device-verified）=同 SoC 集成参照；②**asteroids=Nothing Phone (3a)**（手机形态最高：已进桌面/触屏/按键/GPU addon/图标/壁纸/WiFi/BT，且 donor 与我们同=24A446 iPad17,3 镜像）=手机目标对齐；③cupid（iPhone 身份线最成熟但仅到 BSD init，卡在 root device=我们已通段）。**iPhone 换道代价四件**：①iPhone 24A446 profile（量小）②kernelcache prelink fixture 重定向（cupid 配方=13 vnodeop_desc 按 vnop_* + 8 metaclass 槽 + 5 符号；asteroids 记 32 项差异）③trust cache + userdata 重建（管道已有）④**Data seed 重建=谱系清零（唯一贵项）**。墙（AMFI/sandbox/dyld/root volume）均 donor 无关可复用。**正经理由（非审美）**：面板 2400×1080≈20:9 vs iPad17,3 假设 4:3——UI 缩放/分屏/锁屏 idiom/壁纸 device-class 全按平板走；iPhone 身份几何更对口。排期=独立换道轮，排在 r130 触屏→addon+WiFi→GPU→音频之后评估。

**④ADSP 固件迁移预案（addon 轮工作单素材，s122 拍板）**：正家=Data 活卷 /private/var/firmware（上游 AdspFirmware.c 本为 VFS loader，我 fork 源序 boot_b→/embedded→modem→VFS 的 VFS 源已在链上；交付走 addon 机制=Wi-Fi/蓝牙/VPU 同形态，种子零碰）；顺序=双源过渡（boot_b 先命中零变）→验证 [adsp] 从活卷吃→撤 package-m2381-boot.py 固件段（boot_b 缩回纯 boot image，固件升级变 recovery_a 单件轮）；**权衡=VFS 源等 Data 挂载，battery st=4 从 t≈22s 后移**（上游同款 "VFS loading waits for addon installation"）——时差不可接受则 boot_b 降级长期早源、活卷当更新源共存（四源序天然支持），退役是选项非必须；ESP 路（r129 后 79.3MiB 装得下 30.6MB）=备选需 DarwinBoot 加 member 管线，非首选。chronod -308 残余、dasd/gamed 根修 RE（禁表生效，紧迫性低）。

### 下一轮工作单 r130（触屏轮，2026-11-15 s122 交接；开工即执行，不含未决策项）

**目标**：Goodix GT9916 触屏 → 锁屏上滑解锁进桌面（= 身份/UI 题的 A 路线，asteroids 先例：同 donor 手机已带触屏进桌面）。

**第一刀 = 判读债（先于一切构建）**：拉窗 reader-r129 核窗面哨兵 —— `[adsp] st=13 err=0`、`[battery] st=4`、AKS st=0 sg=3、SMP req=1、DATA st=2、无新崩族；全绿才在 r129 FD 上叠触屏变量（FD 换代后的单变量卫生）。窗口产出 `reader-r129/`（log.bin sha 双端 MATCH）。

**第二刀 = 硬件侦察（只读，四个具体产出）**：
1. dtbo 提取：`/dev/block/by-name/dtbo` 或与 nura 对照（nura `BOOT-CHAIN-RE.md`:86 实录 `ro.boot.dtbo_idx=9` = **goodix brl-d 触控 + BOE AMOLED cmd-mode DSC 面板**）；解 overlay 用 dtc 反编译。
2. 触控节点五元数：**总线/地址**（记忆提示 i2c4@0x5d，须验证）、**IRQ GPIO**（提示 TLMM25）、**reset GPIO**（提示 TLMM24）、**供电轨**（提示 l14b+l12b）、**面板几何**（提示 1080×2400，横竖/原点方向要读 dts）。
3. 固件来源：GT9916 固件在 dtbo/分区/内置？主线 `goodix_berlin_core.c`(nura 实证) 的 fw 路径与校验。产出=设备仓 `records/` 一条 + probes 脚本。
4. 上游面读齐：`docs/porting/input.md` 的接口表（QcomGeniSpi / ShengNovatekTouch NvtProtocol / TouchBoard.h / TouchProbe.cpp / NativeHidInput.cpp / ButtonProbe.cpp）。**关键判定**：本机总线是 I2C 还是 SPI —— 决定是「复用 Novatek 协议核心、只写 Goodix 识别/固件/报帧模块」还是「照 TouchBoard.h 写板级路径 + 自写 Goodix 协议」。

**第三刀 = 组件接线（依侦察结论单变量）**：`[components]` 加入触控传输（GENI I2C 或 SPI 组件，上游有则用）、ShengHidInput（OS 报告面）；板级文件按 TouchBoard.h 布局（电源/reset/传输回调/中断/坐标），arm64e 侧按 sheng 记录处理 IOHID 事件驱动 vtable 重签（220 判别子/64KB 表法，见 sheng 记录）。

**args**：`touch=1→4` 分阶段（资源/时钟引脚/芯片识别/固件+扫描），逐阶段上，不一次全开。

**门（预注册）**：
- **T0** 到锁屏（回归不退化：ADSP st=13 / battery st=4 / AKS / SMP / DATA）。
- **T1** `touch=3` 读到芯片 ID 与几何（矩阵行列数、坐标范围）。
- **T2（主门）** 固件下载成功 + 首帧有效 → `touch=4` 扫描出点 → **锁屏上滑解锁进桌面**（屏摄为准）。
- **T3** 回归哨 + 崩谱对照（新崩族记档）。

**红线**：种子零改动、绝不 data-reset；只碰 boot_b + recovery_a；stall 脸先裸重启再判；每轮单变量（触屏轮不夹 GPU/WiFi）。

**败则**：总线/引脚不可得 → 回 dtbo 与 nura DTS 对拍；芯片无响应 → 电源时序（l14b/l12b 电压与 enable 序）+ reset 时序；固件下载失败 → 保留 stage/CRC/SRAM 读回诊断（上游 advise）再重试；解锁仍提示 home → 转身份/按键题（B/C 路线，见立账⑤）。

（长期挂账与硬律见 auto-memory 索引；轮史与判读全文=archive/STATE-history.md）
