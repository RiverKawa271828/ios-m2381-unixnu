# 上游与参照仓（2026-10-22 重定向）

## 1. 上游是谁

| 角色 | 仓库 | 本地 | 当前 pin |
|---|---|---|---|
| **基座上游（只读）** | https://github.com/UniXNU/UniXNU | `reference/unixnu` + git remote `upstream` | `e95b37d` (2026-10-22) |
| **设备模板（UEFI）** | https://github.com/hanzi123-bilp/uefi-fuxi | `reference/uefi-fuxi` | `7b7ec15` |
| **设备模板（UniXNU 设备仓）** | https://github.com/hanzi123-bilp/unixnu-fuxi | `reference/unixnu-fuxi` | `59a91fb` |
| DarwinBoot（boot manager driver） | https://github.com/UniXNU/DarwinBoot | `reference/darwinboot` | `85332b3` |
| guacamole 先例 | https://github.com/wochgn/UniXNU-guacamole | `reference/UniXNU-guacamole` | `6269d31` |
| qemu 先例（Data seed 脚本） | https://github.com/xVanTuring/UniXNU-qemu | `reference/UniXNU-qemu` | `97b75b1` |
| sheng 参照（平板） | https://github.com/UniXNU/UniXNU-sheng | `reference/unixnu-sheng` | `77490e5` |
| sheng 固件 | https://github.com/UniXNU/uefi-sheng | `reference/uefi-sheng` | `0eb4514` |
| fuxi 另一 fork | https://github.com/matejkoo217/UniXNU-fuxi | `reference/UniXNU-fuxi` | `41c2aac` |
| vermeer 参照 | https://github.com/yyx2025/UniXNU-vermeer | `reference/UniXNU-vermeer` | `9137aba` |

- `reference/` 全部是**只读参照**，不与工作区共用 history；不要在里面改文件。
- 本工作区是 **m2381 下游移植**，不是 UniXNU 的 fork：不要向 `upstream` push。
  `upstream` 的 push URL 已设为 `no_push`；`origin` 留空，等用户 fork 后再设。
- 上游 = 机制/驱动/补丁的来源；设备模板 = 板级值的来源。两者都不整段 merge。

## 2. 更新流程

```bash
# 1) 看上游有没有新东西（只 fetch，不 merge）
git -C reference/unixnu fetch origin
git -C reference/unixnu log --oneline e95b37d..origin/main

# 2) 按需看 diff
git -C reference/unixnu diff --stat e95b37d..origin/main
git -C reference/unixnu diff e95b37d..origin/main -- drivers/UniXNUPkg patches/xnu

# 3) 只把相关改动移植到 devices/m2381/（一次上游主题一个 commit），
#    更新本文件的 pin 表；不要 git pull/rebase 工作区。
```

- **UEFI**：`reference/uefi-fuxi` 的 `fuxiPkg` 是 `m2381Pkg` 的模板；上游
  `UniXNUPkg`/`DarwinBoot` 的改动按主题 cherry-pick/手抄。
- **boot manager**：`reference/unixnu` 的 `UniXNUPkg`、`drivers/`、`patches/xnu`；
  B2 只用 `UniXNU.efi` + `DarwinBoot`，不跑 `unixnu kernel/userdata`。
- **设备值**：`reference/unixnu-fuxi` 只借 `board.toml`/`scripts/` 结构；
  iPad7,1/21H461、192MiB payload、sheng-pmgr、Data seed 都不要带进 B2。

## 3. 已知上游粗糙点（2026-10-22）

- `reference/unixnu` 把 `drivers/DarwinBoot` 的 gitlink 固定在
  `d099354d…`，该 commit 在 `UniXNU/DarwinBoot` 不存在（只有 `85332b3` /
  `12bc9e2` / `09c60e6`）。`git submodule update --init --recursive` 会失败；
  用 `85332b3` 或本地 `reference/darwinboot` 代替。
- `unixnu-fuxi` 是从 sheng 设备仓改的：README/触摸支持表仍是 sheng 的
  （Novatek NT36532E），不能当 fuxi 已验证的证据；触摸/PMGR 仍要自己接。
- `unixnu-fuxi` 的 `[payload]` 192MiB、`[ipsw]` iPad7,1、`sheng-*` 内核参数
  是**全量 4K 路线**的模板，B2 不采用。
- fuxi 的 UEFI FV 没有 UFS DXE（只有 MassStorage app）；B2 的 ESP 路线必须
  另加 OEM `UFSDxe` + `DiskIo`/`Partition`/`Fat`（见 `BOOT-CHAIN.md` §7.1.1）。

## 4. 本工作区的远端

```text
upstream  https://github.com/UniXNU/UniXNU.git   (fetch only; push = no_push)
origin    （未设；用户 fork 后自行添加）
```

- 需要提交到用户 fork 时：`git remote add origin <fork-url>`，再 `git push -u origin master`。
- 不要 `git push upstream`；`no_push` 会直接拒绝。

## 5. 和旧 sweet 上游的关系

旧 ios-sweet / topaz 上游已退役，只作 `docs/archive/` 与 `legacy/sweet/` 的史料；
不再 fetch、不再 cherry-pick。B2 的内核是 `legacy/sweet` 里的 16K 产物的**预编译
Mach-O**，不需要回到旧上游重建。
