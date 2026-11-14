# R-CARD 模板 — 刷写卡固定结构（2026-11-14 s108 复核轮固化）

每轮刷写卡照此结构写。变更分桶决定门集；哨兵表每轮沿用并随新墙增补。

## 0. 变更分桶（先于一切判读）

| 桶 | 判据 | 允许的门集 |
|---|---|---|
| **arg-only** | kernel 步输出 sha 逐字节等于绿件（hard assert） | 只判行为面；kernel sha 变了=管线病，停 |
| **native-hunk** | native 变更 → **validate_map 按 native sha 整串失效 → 配对全量重锚**。这是双重实验：①新配对能开机 ②修复生效 | ①②各设独立 G 门，**①在先**；卡上必须写"本轮=重锚+修复" |
| **kit/管线** | 不变输入 byte-equivalence + 复跑复现 | 确定性门 |

红线：native-hunk 轮**不许**捎带 arg 行为变更（单变量纪律，s92 先例推广）；
arg-only 能试的先试完再落 native 变更。

## 1. 刷前等价类门（audit-boot-face-delta.py，必跑）

```
python3 scripts/audit-boot-face-delta.py GREEN_SEALED CANDIDATE_SEALED [--expect-native]
```

- 三类差异：A=戳/链区 churn；B=声明了 native 变更轮的全部其余；**C=不可解释 → 不刷**
  （或显式改判重锚轮并记录）。
- **站哨兵门（s109 新增，不可被 --expect-native 覆盖）**：r120 定谳后 audit 内置 9 站
  canon（wall76×3+entcopy×2+entconv×2+entval×2），候选面任一站=donor 原字 → FAIL
  "STATIONS LOST"（r119/r120 丢 9 站=wall-76 复发，三个 16KB 区曾被 class B 吞掉）。
  有意退役某站=同 commit 更新 canon。args 门同批：`audit-boot-args.py 绿payload
  新payload`，撤/改 arg 必须显式声明（s102 sheng-data-persist 先例）。
- 两个密封面提取：`7z x payload.img EFI/UniXNU/Payloads/kernel.macho`（payload 内嵌件
  = 密封面，≠ kernel 步输出——两个 sha 都要在卡上分列）。
- 复核轮提问集三问：保真（编译/args/sha）？等价类（相对上个绿 boot 是哪类）？
  风险分区（哪些变更是硬件未验证的）？

## 2. G0 = 绿性地板（墙博馆哨兵，逐轮沿用）

已关账的墙各配一行哨兵；"很久没咬"的墙最需要哨兵（wall-42 PMGR 七轮没咬照样复活）。

| 墙 | 哨兵（窗内第一 grep） | 关账轮 |
|---|---|---|
| 2s instruction fetch abort | 屏序过 `kernel do post - done` 且 >10s 存活 | r118 |
| AMFI "No PMGR?"（wall-42/76） | `signature accepted` 时标 ≤ 绿件水平；无该 panic 行 | r76（24A446）；**r119/r120 复发=emit kit 丢 9 站（非 AMFI 域），s109 站哨兵门关账** |
| launchd abort（MKB/其他） | p1 spawn 后非零退出 = 0 | r102+ |
| SB FBSDisplayMonitor cold.6 sig5 | `CRASH.*SpringBoard` = 0 | r115 |
| kqworkloops 配额 fatal | `caught allocating too many kqworkloops` = 0 | r116 |
| watchdogd/containermanagerd spawn→brk | census 两名下 CRASH = 0 | r112 |
| entcopy ret=14 | `ret=14` = 0 | r111 |
| vnode_rele_ext usecount 下溢 | 无该 panic 行 | r118（未复发） |
| liveactivitiesd sig5（MKB events） | `CRASH.*liveactivitiesd` = 0 | r119 判读已定谳，修复随血统重锚落地 |

新墙关账后加行；哨兵行写"窗内第一 grep"级别的可执行检查。

## 3. G1+ = 本轮修复门（按轮写，判读预注册）

预期形态、判据 grep、败则分流树（照 R115-R119 卡先例）。

## 4. 资产清单（每轮登记）

- kernel 步输出 sha（命名副本 `out/m2381/kernel-<轮>-<sha8>.macho`，payload 不备份它）
- 密封面 sha（= payload 内嵌件）
- 配对 map：**血统一等资产，任何覆盖前先轮转 `.prev`**（kernel.py 已改）
- 已刷 payload 命名副本 + 双端 sha

## 5. 固定流程

fastboot 刷 recovery_a → dd 读回对拍 → --set-active=b → reboot → 3-6min 屏摄
（panic 行照宏口径转录 + verify-panic-crc.py --msg）→ 8-15min 观察 → 硬断电 →
reader ESP → 拉窗（log.bin sha 双端）→ parse --msg → 照 G 门判读。

## 6. 判读惯例（存量硬律，不全列）

slide 减法一律工具算（STATE §4）；去滑先验活；串在二进制≠注册在；屏摄为准的
早期死纪律；新墙判读第 0 步 = 上游设备记录 grep
（`git grep -i <症状> upstream/main -- docs/devices/`）。
