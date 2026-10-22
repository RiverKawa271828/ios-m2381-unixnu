# NOTES-topaz.md —— ios-topaz 机制深潜（MakrSas 怎么把它跑起来的；m2381 视角蒸馏）

> 对象 = `reference/ios-topaz` @ **`origin/topaz`**（89451b8，2026-10-21 fetch；**活跃分支 =
> topaz，main 停 0079 勿读**）。tip 状态 = Redmi Note 12 4G（SM6225「khaje」/ Snapdragon 685）
> **home screen + 触摸 + 按键 + Wi-Fi 全通**（WPA2-PSK、Safari 过 HTTPS）；GPU（Adreno 610）
> 用 HackiOS 私有补丁在机工作、源未公开故不入仓。Boot 向量 = `fastboot boot`（Xiaomi ABL 支持，
> **Meizu 禁区勿比**）。本文件 = 实现机制蒸馏 + 对 m2381 的行动映射；细节用时回原仓读补丁本体：
> `git -C reference/ios-topaz show origin/topaz:xnu/patches/<NNNN>-*.patch`。
> 10-21 拍板：**不换基座、跟路线图**（0080+ 全是 SM6225 定制；共同基座 0001-0079 我树全有）。

## 0. 路线总览（他们在哪、我们在哪）

sweet 0001-0079（我树已有）→ 他们 0080 起重定向 SM6225：CPU/ARMv8.0 修正（0080-0082）→
触摸/按键/电池（0083-0091）→ **显示管线换代**（0092-0120：MDP 直扫/双缓冲/真实 dcache/亮度）→
**Wi-Fi 全内核栈**（0113-0130，自 topaz-woa 移植）→ **SweetSurface GPU 语义加固**（0131-0139）。
对照：我们平台深度领先（8 核 ARMv9 vs 4 核 A73 银核无 16K；UFS 4.0 vs 3.0；用户态全链常驻），
GUI 结果落后（他们桌面已亮，我们卡 backboardd 静默 SIGABRT）。**桌面里程碑不依赖 0080+**——
sweet 0069s = 0046 时代内核 + 根盘 v12-v14 tweaks，两样我树皆备（sweet-root-tweaks.py 与
他们逐字节同源，v12-v14 全套已含）。

## 1. backboardd 生命周期（GUI 链路核心；我机死点所在区）

sweet/topaz 实测的完整序列（哪步断 = 哪类墙）：

```
exec → AKS sel 0/17 → HID 通知/服务匹配（0058 IOHIDFamily user client）→ SweetHID open
→ MobileGestalt（sel 8/28…）→ main 内 dispatch_once = StartWindowServer：
  ① MTLSetShaderCachePath ← BaseBoard libraryPath ← containermanagerd XPC
     （sweet 第一墙：容器路径 NULL → NSException 'NSURL initFileURLWithPath: nil string'
      → 修法 = 镜像造 /private/var/containers + 0052 RENAME_SWAP 仿制）
  ② CAWindowServer serverWithOptions → Metal 先行：CAMetalContextCreate 对
     IOAcceleratorES 空轮询 300×100ms = **30s/backboardd 启动**（0138：发布无
     MetalPluginName 的空 service，Metal 越过它，renderer 不变，30s 免除）
  ③ 无 GPU → renderer = CA::OGL::SW 软件光栅（CA::WindowServer 后端族：
     AppleCLCDDisplay / VirtualDisplay / MemorySurface；249 个 CA_* env 旋钮）
  ④ display 枚举：IOMobileFramebuffer 匹配 → open AppleCLCD → sel 57/15/22/29/33/79/74/
     12/6/27/17/84/50 → IOCoreSurfaceRoot open（sel 13）
  ⑤ 有东西要画时（sweet = data-migration UI 首画）才见 surface create → trap 0-5
     （lock/unlock）→ IOMFB SwapBegin/SwapEnd → SWEET FB swap = CPU copy 入面板内存
```

**我机（m2381）死点 = ④ 之后、⑤ 之前**：AppleCLCD 已 open（SWEET FB sel 行在环）、
SWEET SURF create 从未出现 → 死在 ①-③ 的窗口服务装配区（或 idle 后被异步 abort）。
轮 23 实锤 ReportCrash 处理了 corpse 但 .ips 零落盘 → 三通道取证（见 NOTES-m2381 STATE）。

**SweetSurface 语义加固系列（0131-0139，2026-10-20 一天内打磨，多条由 backboardd 崩溃驱动
——等我们 surface 流动起来后这就是必修清单）**：

| 补丁 | 语义 | 症状 |
|---|---|---|
| 0133 | sels 9/10/11 存值、27 批量 attachments、20 purgeability、12 accelerator、5 YCbCr —— 别答 0 | CA GL renderer 读到全 0 |
| 0132/0134 | IOSurfaceWrapClientImage/Memory；IOSurfaceAddress = creator 自有内存的 surface | 图标/层内容空白 |
| 0139 | 每个 surface 独立 id（全 0 id 时释放一个 = backboardd 里其余全被 unmap） | **backboardd 崩溃** |
| 0135 | trap 4 只 detach 不 drop 引用（一次 release 曾双倍解引用 + unmap 使用中 surface）；client 内存按 IOSurfaceAllocSize 取，写不了退只读 | **使用中 surface 被 unmap** |
| 0136 | trap 4 detach 该 task 的 view；trap 5 前的 lookup 给新 view（竞态 lookup 丢 state page） | 竞态崩溃 |
| 0137 | 最近 256 个已释放 view 的 state page 保持映射（backboardd 读另一 client 刚释放 surface 的 seed） | **backboardd 崩溃** |
| 0131 | 拒绝以 lock result 应答的调用（Wrap 族） | 全 0 IOSurface client |

另 0068（我树已有）= 已映射 surface 的 lookup 返 base -1（backboardd snapshot crash）。

## 2. 显示管线演进（0092-0120；我阶段 B 用 0046 CPU copy，阶段 D 再吃技法）

- **0046 时代（我树现役）**：swap 时 CPU copy 全帧（~10MB@1080×2400）入 ABL 面板内存；
  sweet 0056 起 vsync 定时器按 `sweetfb_hz`（10/30Hz）解耦复制与 swap 频率（0055 教训：
  backboardd 58Hz present × 全帧 memcpy = 600MB/s，单核饿死全系统）。
- **0092/0093 MDP 直扫**：AppleCLCD 把 iOS surface 直接指给 MDP INTF1 扫描；硬条件 =
  surface 落**连续 RAM 且 <4GiB**（0093 scanout from contiguous RAM + mkadt `--midmem`）；
  0097 控制台画成 iOS 帧样式（cached shadow + 走 MDP，panic 切换）。
- **0100-0103 双缓冲律**：vsync 翻转、绝不画进正在扫描的缓冲；console copy 持 putc 锁；
  compose（dim/应用层）在 back buffer 合成后整帧翻。
- **★dcache 雷区（0101/0107/0111 三轮踩平）**：Apple SoC 配置里 `flush_dcache/clean_dcache`
  是 coherent-IO **no-op**；真机 DMA 给显示引擎前必须真实 clean。0101 全局真 flush → 系统
  爬行 → 0107/0111 = 局部 clean（`sweet_dcache_clean`，只对显示/DSI buffer）。**m2381 阶段 D
  必踩同类坑，先记**。DSI 长写等待也随 0107 调优（100×200µs 替 50×1ms）。
- **亮度/电源（0104-0106/0110/0079）**：DSI DCS 0x51 亮度——**异步 thread_call 送**
  （CoreBrightness 在 backboardd 里同步等 DSI 完成会 stall 整个窗口服务）；AppleARMBacklight
  service（`sweetbl=1` 选装——它使能后 CoreBrightness 数字调光曾杀 CPU 渲染性能）；DCS
  0x28/0x29 才是真显示电源（IOMFB RequestPowerChange(0) 在 ss-ea AMOLED 上只是调暗）。
- **0114 画布对齐**：MDP 直扫时 canvas 必须 = 面板原生（`--native`，否则 backboardd 要求
  不存在的硬件缩放器）；iPhone X 1125×2436 画布 + SRC_XY 裁剪窗（0094 裁剪曾黑屏被撤，
  0114 用 16B 对齐 crop 窗成功）。CPU copy 时代 iPhone X 画布无碍（我树 0061 现役）。

## 3. 输入（触摸/按键打法；我阶段 C 打 GT9916 用同款循环）

0083-0090 的调试循环值得整套照搬：probe → **diag panic**（`sweettouchdiag=1`：失败时把
探针原始值 panic 上屏，免日志通道）→ stats（`=2` 逐帧统计）→ 分阶段修。坑谱：0087 = QUP
SE2 时钟 RCG 偏移写错（0x1f3a8）**直接挂全机**（400kHz + IRQ-gated polling 才稳）；0086/
0089/0090 = FT 解析器逐步修正（event 3 = 空 slot；TD_STATUS 0xff = 无触点帧 = 全释放）。
0062 = 触摸供电 RPMh rail vote + **IOPMPowerSource 假电池**（UI 要电池存在才安分）。
m2381 对应：GT9916 @ i2c4 a90000 0x5d、irq tlmm25、reset tlmm24、avdd l14b_3p2（nura DT
真值 = 地面答案，MakrSas 只能从 topaz-woa 逆向）；GENI I²C 框架 = sweet 0060/0063 翻版换协议。

## 4. Wi-Fi / 调制解调器（0113-0130；远期，架构先记账）

栈序：TopazModem = modem DSP **PAS boot 经 TZ**（固件读 stock `modem_a` 分区，hash segment
拆自 modem.b01）→ GLINK/QRTR（0118）→ host services pd-mapper/tftp/rmtfs（0121）→ WLFW
mission mode（0122，含 restart/CAP retry 0124）→ HTC/WMI/HTT（0125）→ radio API/passive
scan（0123/0126）→ station/assoc/**WPA2-PSK 4-way + PMK join 在内核**（0129/0130）。iOS 侧
经 wifid 看到以太网形态数据面。全在 XNU 里，无 Linux 借道。
**launchd 门槛**：`com.apple.wifid` 带 `_LimitLoadToDeviceTree = wlan`——**ADT 无 `wlan` 节点
launchd 根本不拉它**。m2381 现在切勿加 wlan 节点（无硬件支撑 = wifid 崩溃循环）；做 Wi-Fi 时
再加 + 配 TopazModem 式驱动（SM8550 = WCN6750，nura DT/mainline ath11k 有真值可对照）。
**0127/0128 quiesce 依赖**：StartIOKit 对 registry root 的 busy hold 在 SWEET 配置下永不释放
（无 IOKit daemon 来报 launched）→ IOKit 永不 quiesce → configd 的 InterfaceNamer 不报
"quiet" → **wifid（及其后的 SpringBoard）开机干等**。0127 `sweetbusy=N` 是诊断（谁持 busy），
0128 = 跳过该 hold。我机到网络阶段必踩，先记账。

## 5. GPU 现状（跟踪件，不挡路）

- HackiOS = iOS 16 on POCO X5 Pro 的移植，其 Adreno 内核驱动私有；topaz 承诺其公开后并入
  （0131-0139 的 SweetSurface 语义加固就是为 CA 的 GL renderer + GPU 准备的地面工程）。
- **`--midmem` 的教训**：Adreno 610 DMA 够不着 4GiB 以上 → mkadt 把内核 RAM 挪到
  0x80000000+7G（含 <4G 窗口）。**m2381 含义**：我们内核 RAM = 0x8c0000000 起 13GB **全在
  4G 以上**（低银行 = XBL/XPU carve-out 带被排除）。A740 = 7xx 族 48-bit VA，可能无此限制，
  但做 GPU 时第一条就是验证 DMA 上限；必要时学 --midmem 从低银行抠一窗给 GPU buffer。
- 不做 GPU 不挡桌面：CA::OGL::SW 软渲染（慢但通），sweet/topaz 都先这么过的。

## 6. ADT / 镜像细节（可搭车的小件）

- **`artwork-display-gamut` P3 → sRGB**：iPhone X product 属性报 P3 → MobileGestalt
  HasExtendedColorDisplay → WebKit 把页面 tile 画进 10-bit surface（'w30r'/'b3a8'）→ 内核
  提供不了 → **Safari 全白**。m2381 mkadt 应搭车（我树 mkadt 无此处理）。
- `--native`（直扫画布 = 面板）/ `--midmem`（见 §5）/ wlan 节点（见 §4，现在别加）。
- home-button-type 2 / display-scale 3 / unique-chip-id+chip-id（IOPlatformUUID）——我树已有同款。
- 根盘 tweaks：sweet-root-tweaks.py 与他们同源未改（v12-v14 全套含）——m2381 待验证的是
  现役 iOSRoot 是否施用过（宿主机挂载时顺手核）。

## 7. 安全与恢复（他们真·砖过一次）

- **2026-10-19 事故**：LUN0 GPT 被毁 → 全机 `Load Error`（fastboot 里 `partition-size:*` 全
  Not found）→ 用 2026-10-01 的 `gpt_both0.bin` `fastboot flash partition:0` 修复。教训 =
  XNU 写 UFS 期间 GPT 是单点生死。
- **topaz 0088 GPT 看门狗机制**（`sweet_ufs.c`，小补丁）：①写栅栏——`ufs_write_blocks` 拒
  LBA<6 及备份 GPT 头前 4 块（EPERM）；②`ufs_find_partition` 时记主/备 GPT 的 CRC；
  ③看门狗线程定期复读，**变更即 panic**（带现场死比静默毁盘好）。
- m2381 评估：我们写面已开（0047 日志环写 userdata 尾、0088 IPS 镜像、rw 根盘），**强烈建议
  搭下一发 CI 翻入**。注意 m2381 日志环写在 userdata 分区内尾部，与栅栏不冲突（栅栏护的是
  分区表头/尾，不是分区内容）；若 userdata 是 LUN0 末分区，备份 GPT 紧贴其后，栅栏正好护住。

## 8. 对 m2381 的行动映射（速查）

| topaz 资产 | m2381 动作 | 阶段 |
|---|---|---|
| backboardd 死因方法论（§1 ①-③） | 三通道取证（STATE 排程①-③） | A 破案（现在） |
| 0138 空 IOAcceleratorES | 直接翻版（30s 白等 + idle 样本成分） | A/B |
| 0131-0139 SweetSurface 语义 | surface 开始流动后按症状对号翻版 | B |
| 0092-0120 显示技法（双缓冲/局部 dcache/异步亮度/对齐裁边） | 阶段 D 按 m2381 DPU 常量重做（nura 真值在手，可超 MDP 路线） | D |
| 0083-0090 触摸调试循环 | 照搬打法打 GT9916（diag panic 件值得先做） | C |
| 0062 假电池 + rail vote | 同款（nura regulator 真值） | C |
| 0113-0130 Wi-Fi 栈 + 0128 quiesce + wlan DT 门槛 | 架构记账；WPA 4-way 内核做法可抄 | 远期 |
| HackiOS GPU + --midmem 4G 教训 | 跟踪开源；验证 A740 DMA 上限 | 远期 |
| 0088 GPT 看门狗 | **搭下一发 CI 翻入**（见 §7） | A（搭车） |
| gamut sRGB | **mkadt 搭车**（见 §6） | A（搭车） |
