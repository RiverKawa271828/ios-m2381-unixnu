# STATE — 现势状态（m2381，2026-11-15 更新）
## 0. 现势指针（每轮首更，2026-11-15 会话一百二十一；更早指针=archive/STATE-history.md）

**★★★★★★★★★★s121 里程碑定谳：r128b 四门全过 = StDtb 墙死透 + ADSP 活着 + 充电链首次全通（用户 UI 充电图标实证）**：窗 reader-r128b/（log.bin sha 3bf85784 双端 MATCH，223 槽/445s，boot_id=0x1cc2087d，state=2 谱系**零 reset 续接**——种子红线遵守）。**B0' ✓** `[adsp] st=13 err=0`（r128 的 st=-8/0xFFCFFFBB TZ 拒死透；首快照 t≈3s 即 st=13、rdy=390ms、fw=29890K 完整读出（V8 形态满读计账 vs r128 的 29784K=计账差非病）、GLINK rx/tx 全窗爬升 69→1374、scm=0/0）；**B0 ✓** `[battery] st=4`（t=22s 起；pct 83→84 爬、mv=4428→4430、ma≈-1600 充电中、usb=1、design=3978、cyc/ans/ntf 活跃）；**B1 ✓** 用户锁屏右上充电图标 + 日志互证；**B2 ✓** AKS st=0 sg=3（opens=62 calls=640）/ DATA st=2 er=0（wr≈39k）/ SWAP retired=2204 status=0 / SMP req=1 c1-c7=step3 / 锁屏保持（7min30s+）。**结论：squash 改产原厂 V8 单文件形态（phdr 表逐字节原厂=PAS 鉴权 metadata 不破坏）上机定谳；只刷 boot_b（9c181ee4）换药成立，recovery_a 未动**。boot 时序 logo≈2:20/锁屏≈6:12(372s) 比 r128 的 168s 慢（插充 boot，非门项）。回退锚（boot_b ← boot-b-d0200a6b-r128.img）未动用。

**s121·新崩族立账（非阻塞）**：DockFolderViewServic sig11 ×53（t=340→445s，2s 重生环，**全窗唯一崩族**；PC=LR=0x2124b23dc 去滑 **0x20f9723dc**（+2b40000），待符号化）。对照面净：chronod 本窗零崩（-308 残余待续察）、dasd 零出现（r121 族）、非零 exit 仅 4 条早期行（launchd/MSUEarlyBootTask sig1、sysstatuscheck st=100、prng_seedctl c00/12，@t=72-76s）。

**s121·编排（r128b 闭环后顺位）**：**下一轮 = r129 容量轮**（ESP 内 kernel LZMA，uefi-m2381 单点改；任何内核长大的轮的前置，不变）；**音频车道自动解锁**（①已通=ADSP 起；nura CS35L45 直驱参照 + 上游 audio=12 全栈，排期用户定）→ r130 触屏（Goodix GT9916 NOT Novatek）→ addon+WiFi 同轮 → GPU 大会话，顺位不变。立账更新：chronod -308 残余（本窗零崩）、**DockFolderViewServic sig11 新族**、dasd/gamed 根修 RE（禁表生效，紧迫性低）。挂账=主树本轮推净；设备仓零改动。

（长期挂账与硬律见 auto-memory 索引；轮史与判读全文=archive/STATE-history.md）
（长期挂账与硬律见 auto-memory 索引；轮史与判读全文=archive/STATE-history.md）
