# STATE — 2026-10-23（路线重置 + 设备实测）

## 1. 结论/决策

- **B2/B1 废弃（用户 2026-10-23 拍板）**：旧 ios-sweet 16K 内核 + iPhone X
  16.7.16 root 复用路线实机跑不通；旧阶段只带经验与方法。B2 评估 →
  `docs/archive/PLAN-unixnu-b2-era.md`；B2 启动链 →
  `docs/archive/BOOT-CHAIN-b2.md`；`legacy/` 已出 git，死库存（16K 内核/ADT/
  旧 IPSW/16K 重建套件）已从本地删除。
- **现方向 = 原生 UniXNU 设备移植**（对齐上游 fuxi/sheng 模式）：UniXNU 4K
  内核 + **iPhone10,3 / iOS 16.7.16 `20H392`** IPSW root（用户拍板：手机
  镜像、不用 iPadOS；与 negroni userland 同版本、与 nuwa 同身份）+
  `unixnu-m2381` 设备仓 + `m2381Pkg`（Mu-Silicium UEFI）。母版/指南见
  `docs/porting-notes.md`。
- **设备数据豁免（用户 2026-10-23 拍板）**：无须保留设备数据，随时可刷。
  userdata 现为 nura root；抹除/重建不再需要逐次拍板（红线 2 相应更新），
  但每次实际写入仍按 `docs/FLASHING.md` 流程走。
- **设备连接（实测）**：USB 网络（NCM），设备 `172.16.42.1:23`，宿主侧接口
  `enp104s0f3u1`；工具 = `/home/ri/work/nura-meizu-m2381/tools/{devsh,
  devpull,devpush}.py`（SO_BINDTODEVICE 绕 TUN）。设备在线、root 可用。
- 分支已对齐上游：`m2381` = `upstream/main`（`b7b6a6a`）+ 工作区提交，同步 =
  `git rebase upstream/main`（见 `docs/UPSTREAM.md` §2）。

## 2. 设备现状（2026-10-23 devsh 只读实测；nura OS 在跑）

| 分区 | 设备 | sha256（整分区读回） | 备注 |
|---|---|---|---|
| boot_a | sde13 | `5f9f1503e871250494293b2e6f991db1bc6d9fb9941db58c4b0c72eac8b11e30` | nura |
| **boot_b** | sde43 | `af574775b92ce59a650bd3e85b2435133d34e43b4128a82904e0a6e3126a300f` | **现役启动槽，nura**；旧记录"r34 shim `339b8480…`"**作废** |
| **recovery_a** | sde27 | `0a102220c5adaeb9bd5e7584d62bc2c9d909ad00c9024286f1ae0d813a38653f` | 现役 nura ESP；v71 只是模板 |
| recovery_b | sde56 | `20bcd203e64210044a5148d9adcb6bdddad9be2429f90289adcd0736ee316f43` | nura |
| **userdata** | sda22 | —（不整读） | **nura root**（"Nura edge"，ext4，`root=/dev/sda22`）；≈464GiB |

几何（sysfs 512B 扇区；换算与 GPT 一致性已核）：

| 分区 | start(512B) | sectors(512B) | 字节 |
|---|---|---|---|
| boot_b | 3,665,816 | 196,608 | 100,663,296 = 96MiB |
| recovery_a | 2,528,048 | 204,800 | 104,857,600 = 100MiB |
| userdata | 18,643,776 | 972,326,040 | 497,830,932,480 ≈ 463.7GiB |

- UFS LUN0（sda）`logical_block_size = 4096` ⇒ board.toml 的 LBA 用 4096B：
  **userdata first_lba = 2,330,472，last_lba = 123,871,226**（= first +
  121,540,755 − 1）。
- 设备身份：`meizu-meizu20`，nura 内核 `7.3.0 #92-meizu-meizu20`。
- 刷机窗口还需复核：`fastboot getvar current-slot`（应为 b）与
  `partition-size:userdata`（三源对拍最后一源）。

## 3. 资产（本地，不入 git）

| 路径 | 状态 |
|---|---|
| `artifacts/images/{mu-r69-97cdea20.img, esp-recovery-v71.img.gz, shim-boot-m2381.img}` | 回滚件，保留。**注意**：三者都不是设备现役镜像（现役 = 上表 nura 件）；完全恢复 nura 需 nura 侧重刷 userdata |
| `artifacts/vendor/meizu20-uefi-mods/`、`inner-fv-A.bin` | 路线 A 要用（m2381Pkg FV） |
| `research/bin/ipsw` | ipsw CLI，保留（DSC/IM4P 抽取） |
| `research/ipsw-x/`（220K，d22ap.adt + DeviceTree im4p） | ADT 解析样本，保留 |
| `research/ipsw/`（**待用户下载**：iPhone10,3,iPhone10,6_16.7.16_20H392_Restore.ipsw，6,171,035,648 B） | 链接与 sha 见 `docs/porting-notes.md` §5；sha 必须 = `ece1194f…`（与旧路线用过的镜像同文件，Apple 仍在签名） |
| `reference/`（10 个 UniXNU/UEFI 克隆 + 5 个参照设备仓） | 见 `docs/porting-notes.md` |

## 4. 当前没做/不能声称的

- 没有设备仓、没有 m2381Pkg、没有构建任何镜像、没有刷机。
- 没有通读上游 `docs/porting/` 全文；AGENTS §2 布局仍是草案。
- ESP 载荷在 Mu 下的可见性（OEM UFSDxe 集成进 m2381Pkg FV）未验证。
- `fastboot getvar` 侧的槽位/分区复核未做（需要重启进 bootloader）。
- 上游 3 个 workflow（docs/macos/terminal）是否已在 GitHub UI 禁用未确认。

## 5. 下一步（P0 已完成 → P1）

P0 产出 = `docs/m2381-bringup.md`（施工骨架）。P1 开工顺序：

1. **备份 nura**：devpull boot_b/recovery_a 整分区 → `backups/`（首刷前置）。
2. 起 `devices/m2381/unixnu-m2381` 骨架（fork fuxi + delta 文件）。
3. `make-m2381-adt.py` 初版；`unique-chip-id` 读取路径；stock fastboot 包。
4. Linux 实测 `./unixnu build kernel` 跑通（P1 门槛）。
5. `m2381Pkg` 派生 + UFS 源码移植（P2）。

## 6. 开放问题

- **payload 载体**：ESP（recovery_a）vs 压进 ABL 83.5MB 限内；读完
  `docs/porting/uefi-firmware.md` 定案。
- **UFSDxe**：OEM blob 集成方式与 fDeviceInit 行为。
- **构建主机**：unixnu 构建树在 Linux x86_64 上是否可直接出 arm64 内核
  （上游 CI 用 macOS runner）；不行则配 CI 或容器。
- **m2381 stock fastboot 包**（`gpt_main0.bin`/`rawprogram0.xml`，userdata
  几何的 `[FW]` 侧佐证）：待收集（`/home/ri/work/meizu20/` 一带可能有）。
- **kernel defconfig 板级差异**：与 fuxi/sheng 的 PMIC/按键/显示/触摸差异。
- **显示**：cmd-mode DSC 在 UEFI 的最小点灯（SimpleFbDxe 用 ABL 帧缓冲）。
- **按键**：PMK8550 SPMI SID/键码与 fuxi/sheng 是否一致。
