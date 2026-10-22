# DeepSeek harness 交接书 — r35 窗（mobileactivationd 原地换即退体）

> 写给下一个接手的 agent：拿这份直接开工，不需要再考古。所有命令都实测过（2026-10-22 r34 窗跑通同一套管线）。
> 目标定位（用户 10-22 定调）：**尝鲜 = 跑起 iOS、出 GUI、能简单操作系统**。不是继续开发。
> 本轮 = 测距仪：让 lsd 不再等 mobileactivationd，看 SpringBoard 能走多远。

---

## 0. 任务一句话

把 userdata 镜像里 `/usr/libexec/mobileactivationd` 的**文件内容**换成镜像内 `/usr/bin/vm_stat`
的拷贝（Apple 原签名，零签名问题）→ 刷双件 → 跑 17min → 读环 → 判读 lsd 是否解锁。

**为什么这能解锁**：launchd 签名缓存（`/System/Library/xpc/launchd.plist`，r34 窗终审实锤它
遮蔽一切磁盘 plist）按**路径**拉起 mad；vm_stat 起来后不注册任何 MachService、立即退出 →
lsd 的 `xpc_connection_send_message_with_reply_sync`（MadGate copyActivationRecord）收到
**对端死亡 error** 而非永等 → lsd t2 解锁 → seeding 可望收尾 → SpringBoard 排队中。

**成败判据（先记住，读完环对表）**：
- 胜：环里 `CRASH mobileactivation` = **0**（少量 `exec mobileactivationd` 正常，起→退）；
  lsd t2 栈**不再是** r34 那条 16 帧链（判等法见 §6）；SpringBoard/backboardd 出现新栈。
- 负：lsd t2 仍 = r34 的 16 帧（逐帧平移判等）→ 「退出=error」假设死，升级路径见 §8。

## 1. 现状 60 秒

- r34 窗（boot `339b8480` + userdata `7aaab96f`）已判读完毕：**假说 B 终审实锤**（launchd
  签名缓存遮蔽磁盘 LaunchDaemons，r34mark 零出现）；**连崩群 548→0**（容器骨架生效，
  containermanagerd 活了）；Gate2 = 28 entitlements PASS（AMFI 无罪）；唯一堵点收敛 =
  mad 每把 exec 必 fatal（stock Swift stub line321，brk@mh+0x1BF3BC）→ MadGate sync 永等。
- 详细判读 = `handoff/r34-window-forensics.md`（必读 §0/§4）；深挖 = `handoff/glm-report-r34.md`。
- 全程日志 = `NOTES-m2381.md` 顶部 STATE + Session log 10-22 八条目。
- 设备现态 = **nura-min 在跑**（USB 网 172.16.42.1；boot_b 未对拍、按流程应为 mu-r69）。
  环已读走（`rings/m2381-ring-r34.*`），userdata 随便动。

## 2. 环境事实与陷阱清单（先读，省你一小时）

| # | 事实/陷阱 | 处置 |
|---|---|---|
| 1 | **`grep` 被 zsh 包成 ugrep 且 `--ignore-files` 跳过 .gitignore 的 `rings/`** → 环分析会假零匹配 | 一律 `/usr/bin/grep -a`（txt 里有杂字节，`-a` 必带） |
| 2 | **磁盘配额满（EDQUOT，node 报 -122）症状**：一切写 stdout 的命令 exit 1 零输出、`true` 能过、Write 工具报 -122（上上窗"shell 故障"同根） | 见到此形状 = 让用户清盘，别修 shell |
| 3 | zsh 把未加引号的 `===xxx===` 当 =展开 | echo 的分隔串一律加引号 |
| 4 | devsh = 纯文本 telnet 通道，二进制会坏 | 走 `gzip \| base64 -w 100`（配方 §5） |
| 5 | `rings/` 在 .gitignore 里但历届环都入库 | `git add -f rings/...` |
| 6 | 容器 NoNewPrivs：**mount/loop 一律宿主 sudo**（容器无 CAP_SYS_ADMIN）；宿主 sudo 会剥 LD_LIBRARY_PATH | 手术挂载让用户在宿主执行一条 mount，或按 nura 宿主 sudo 脚本模式 |
| 7 | **img2simg 只用默认 4K 块**（512M 块 = 宿主 OOM 实炸过一次） | `img2simg in out` 不带块参 |
| 8 | fsck 报 `Volume header needs minor repair (1,0)` = mkfs+journal 固有噪音 | `-y` 修后 `-n` 仍报 = 健康，非失败 |
| 9 | **12G simg 刷完后 `fastboot reboot` 会卡死**（三次实测）；nura-min 刷后 reboot 不卡 | iOS 件刷完让人手断电重启 |
| 10 | 红线：只碰 **boot_b + userdata**；xbl/abl/tz/modem/dtbo/super 永不碰；fastboot 单通道律 | 见 AGENTS.md §红线 |
| 11 | commit 不 push；推送窗口 = 用户拍板 | 收尾照 §7 |
| 12 | 工具位置：`python3 tools/sweet-log.py`（环解析）、`/usr/bin/img2simg`、`aarch64-linux-gnu-objdump`、`research/bin/ipsw`（DSC/符号）、devsh = `/home/ri/work/nura-meizu-m2381/tools/devsh.py` | — |
| 13 | 关键镜像/件路径 | 见下表 |

**路径表**：
```
userdata 手术基料   /home/ri/mkiosroot-work/iOSRoot-raw-r34.img     (r34 术后 raw, 12G 级)
boot (r35 沿用)     /home/ri/work/ios-meizu-m2381/shim/out/shim-boot-m2381.img
                    sha256 339b8480b71c40ec940772623e16b15875256624073e9533a45280ea8bc3b32e
mu-r69 (读环收兵用) /home/ri/work/nura-meizu-m2381/artifacts/mu-r69-97cdea20.img  (前缀 sha 97cdea20)
nura-min (读环收兵用) /home/ri/work/nura-meizu-m2381/artifacts/meizu-meizu20-nura-min-r91-2G.simg
                    sha256 a84d921233041a9cd8c6cf95572fd92edcf9d7862689cb0f764367ae0793cd3f
devsh               python3 /home/ri/work/nura-meizu-m2381/tools/devsh.py '<cmd>' --wait N
手术脚本先例         /home/ri/work/ios-meizu-m2381/.re/r34-surgery.py   (照它的骨架改)
环解析               /home/ri/work/ios-meizu-m2381/tools/sweet-log.py
```

## 3. Phase 1 — r35 手术（宿主侧，~20min）

```bash
cd /home/ri/mkiosroot-work
cp --reflink=auto iOSRoot-raw-r34.img iOSRoot-raw-r35.img     # 12G，reflink 秒级
mkdir -p r35mnt
```
挂载（**容器内不能 mount——请用户在宿主执行**）：
```bash
# 宿主:
sudo mount -t hfsplus -o loop,rw,force /home/ri/mkiosroot-work/iOSRoot-raw-r35.img /home/ri/mkiosroot-work/r35mnt
```
手术（**只做这一件事，别加戏**——r34 的骨架/Global.plist/envvar 都留着不动，r34mark 也留着无害）：
```bash
python3 - <<'EOF'
MNT='/home/ri/mkiosroot-work/r35mnt'
ma, donor = MNT+'/usr/libexec/mobileactivationd', MNT+'/usr/bin/vm_stat'
import os, shutil, hashlib
st = os.stat(ma)
open(ma,'rb').read(4)  # 确认可读
os.remove(ma)
shutil.copyfile(donor, ma)
os.chmod(ma, 0o755); os.chown(ma, st.st_uid, st.st_gid)
a=open(ma,'rb').read(); b=open(donor,'rb').read()
assert a==b and len(a)>0, 'content mismatch'
print('OK: mobileactivationd -> vm_stat,', len(a), 'bytes, sha', hashlib.sha256(a).hexdigest()[:16])
EOF
```
验证 + 出件：
```bash
fsck.hfsplus -f -y /home/ri/mkiosroot-work/iOSRoot-raw-r35.img    # 期望 The volume iOSRoot appears to be OK
fsck.hfsplus -n    /home/ri/mkiosroot-work/iOSRoot-raw-r35.img    # "minor repair (1,0)" = 固有噪音，忽略
# 卸载（宿主 sudo umount /home/ri/mkiosroot-work/r35mnt）
img2simg iOSRoot-raw-r35.img iOSRoot-s-r35.simg                   # 默认 4K 块，勿加参！
sha256sum iOSRoot-s-r35.simg                                      # 记下前缀 = 本轮身份
```

## 4. Phase 2 — 刷写（设备 fastboot，命令级）

```bash
# 0) 设备在 nura-min → 进 fastboot：
python3 /home/ri/work/nura-meizu-m2381/tools/devsh.py 'systemctl reboot --reboot-argument=bootloader' --wait 10
# 等 ~10s 后:
fastboot devices                                   # 有设备
fastboot getvar current-slot                       # 必须 = b，否则停手问用户
# 1) boot（复合命令带 sha 对拍纪律）：
sha256sum /home/ri/work/ios-meizu-m2381/shim/out/shim-boot-m2381.img   # 必须 339b8480 开头
fastboot flash boot_b /home/ri/work/ios-meizu-m2381/shim/out/shim-boot-m2381.img
# 2) userdata：
cd /home/ri/mkiosroot-work && sha256sum iOSRoot-s-r35.simg             # 对拍 Phase 1 记录的前缀
fastboot flash userdata iOSRoot-s-r35.simg                             # 必须看完整 OKAY（~5min）
# 3) 收尾：
fastboot set_active b                                              # retry 复满
fastboot reboot                                                    # ⚠大概率卡死 → 人手断电重启（陷阱#9）
```

## 5. Phase 3+4 — 运行窗与读环

1. **运行窗 = 17min**（≥9.5min 勉强可用；17min 才与 r32/r33/r33b/r34 census 可比）。
   设备黑屏 = 正常（GUI 链没起）。期间什么都不用做。
2. 窗毕 → **请用户按 pwr+vol↓**（全程唯一需要用户的手）→ 回 fastboot。
3. 收兵刷回（读环通道）：
```bash
sha256sum /home/ri/work/nura-meizu-m2381/artifacts/mu-r69-97cdea20.img
fastboot flash boot_b /home/ri/work/nura-meizu-m2381/artifacts/mu-r69-97cdea20.img
fastboot flash userdata /home/ri/work/nura-meizu-m2381/artifacts/meizu-meizu20-nura-min-r91-2G.simg
fastboot set_active b && fastboot reboot        # nura-min 刷后 reboot 不卡（陷阱#9）
# 等 ~90s（ping 172.16.42.1 通 = 起来了）
```
   （nura-min 刷入**不覆盖** userdata 尾部 1025 块的日志环——r33/r33b/r34 三窗实证。）
4. 读环（原样照抄，`--wait 90` 别省）：
```bash
python3 /home/ri/work/nura-meizu-m2381/tools/devsh.py 'D=/dev/disk/by-partlabel/userdata; S=$(blockdev --getsize64 $D); K=$((S/4096-1025)); echo K=$K; dd if=$D bs=4096 skip=$K count=1025 2>/dev/null | gzip -c | base64 -w 100' --wait 90 > /tmp/ring-r35-raw.txt 2>&1
cd /home/ri/work/ios-meizu-m2381 && python3 - <<'EOF'
import base64, gzip, re
lines = open('/tmp/ring-r35-raw.txt').read().splitlines()
b64 = [l for l in lines if len(l) >= 90 and re.fullmatch(r'[A-Za-z0-9+/=]+', l)]
print('b64 lines:', len(b64))
raw = gzip.decompress(base64.b64decode(''.join(b64)))
open('rings/m2381-ring-r35.bin','wb').write(raw)
print('bytes:', len(raw))
EOF
python3 tools/sweet-log.py rings/m2381-ring-r35.bin rings/m2381-ring-r35.txt
# 期望头行: # version 1, ring 4194304, total <T> bytes, flush #<N>
```

## 6. Phase 5 — 判读（`/usr/bin/grep -a`，命令照抄）

```bash
G=/usr/bin/grep
$G -a -m2 "boot-args" rings/m2381-ring-r35.txt            # 必须带 sweet_amfi_debug=1（boot 身份）
$G -ac "exec mobileactivationd" rings/m2381-ring-r35.txt  # 期望：少量（每 demand 一次）
$G -ac "CRASH mobileactivation" rings/m2381-ring-r35.txt  # 胜负手：期望 0（r34 = 208）
$G -ac "cs_invalid_page" rings/m2381-ring-r35.txt         # 期望 0
$G -a "SWEET AMFI: com.apple.mobileactivationd" rings/m2381-ring-r35.txt
        # r34 = 28 entitlements（mad 本尊）；r35 若 N 变小（个位数）= vm_stat 上机铁证
$G -a "SMP lsd" rings/m2381-ring-r35.txt                  # 核心：t2 栈
$G -a "SMP SpringBoard\|SMP backboardd" rings/m2381-ring-r35.txt | head
$G -a "^CRASH" rings/m2381-ring-r35.txt | awk '{print $2}' | sed 's/\[.*//' | sort | uniq -c | sort -rn
$G -a "^exec " rings/m2381-ring-r35.txt | awk '{print $2}' | sort | uniq -c | sort -rn | head -15
```
**lsd t2 判等法（防 slide 干扰，已程序化验证）**：r34 环 lsd slide = 0x1a958000，其 t2 帧链
去 slide 后前六帧 = `0x18db684a0, 0x181cc7990, 0x181cc78c4, 0x1811e6940, 0x1816c0f4c,
0x1816c2584`（16 帧，与 r33b 环逐帧全等、delta 恒 −0x1c42c000）。r35 环取 lsd 自己的
slide（`SMP lsd[..] slide 0x…` 行），t2 各帧减去它，前几帧仍等于上面序列 = **还在永等**；
变了 = 解锁。

**三种结局**：
- ① t2 变了 + 无新 fatal → 看采样里 lsd/SB 走到哪，直接记里程碑（离 GUI 的真实距离数据到手）。
- ② t2 变了但出新 crash 群 → 正常的 bring-up 打地鼠，按栈定位下一个 daemon，照 r34 报告的
  方法（环 + ipsw 符号化）走，**仍属尝鲜推进，不是新开发**。
- ③ t2 没变（16 帧还在）→ 「退出=error」假设证伪，别在原地重试；升级路径见 §8，下窗再说。

## 7. 收尾纪律

1. 环件入库：`git add -f rings/m2381-ring-r35.{bin,txt}`
2. `NOTES-m2381.md`：Session log 加「10-22 九（r35 窗）」条目 + 顶部 STATE 换新（格式照八条目）。
3. `AGENTS.md` 状态块更新（格式照现块）。
4. commit（中文长信息，风格照 `git log -3`）；**不 push**。
5. 设备交还时保持 **mu-r69 + nura-min**（与接收时同态）。
6. 需要用户动手的三处：宿主 mount/umount（§3）、pwr+vol↓（§5）、卡死断电（§4）——提前一句话说清，别让用户猜。

## 8. 升级路径（仅当 §6 结局③）

下一刀 = 补丁 **DSC** 里的 `-[MadGate copyActivationRecord:]` 入口直接返回 nil
（同步等待就地消失，不依赖「退出=error」语义）。
- 反汇编现成：`.re/d_madgate.txt`（符号 `-[MadGate copyActivationRecord:]` @DSC 未滑
  ~0x1c90cf000 区；`.re/d_lshrn.txt` = HRN 调用方）；DSC 切片 =
  `research/dsc/20H392__iPhone10,3_6/dyld_shared_cache_arm64.*`（43 片 + .a2s 符号库）。
- ⚠动手前先实测镜像内 `/System/Library/Caches/.../dyld_shared_cache_arm64` 非零且与
  research 抽取件同源（AGENTS 有「mkiosroot 把 root 内 cache 清 0」的旧注——与用户态实跑
  矛盾，先验证再动刀）。
- DSC 补丁面大（缓存页影响全进程），AMFI 放行补丁页已被 r32-33b 实证（补丁过的 mad 代码页
  跑到了 Swift fatal 才死 = 签名主校验被我们 hook 放行）。

## 9. 不许做的事（尝鲜纪律）

- ❌ 修 launchd 缓存 / AMFI hook 扩面 / 激活协议 stub（A6）/ HRN 语义深挖 —— 全是开发向，用户已明确砍掉。
- ❌ 给 mad fatal（line321）做"正解"——我们不换它修它，我们绕它。
- ❌ 重建 r34mark 类 plist 体内标记——`CRASH mad = 0` 本身就是 r35 的体内身份。
- ❌ 触碰 userdata 之外的分区、重建 ADT/内核（boot `339b8480` 原样沿用）。
