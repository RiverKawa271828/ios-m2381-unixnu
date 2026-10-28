# devices/m2381 — m2381Pkg 落地处

设备仓本体已于 2026-10-29 拆出为独立仓（用户拍板"按上游要求来拆"）：

- **仓库**：<https://github.com/RiverKawa271828/UniXNU-m2381>
- **本地**：`/home/ri/work/UniXNU-m2381`（与本仓并排，`../UniXNU-m2381`）
- **提取方式**：`git subtree split -P devices/m2381/unixnu-m2381`——
  93 提交全史随行（tip `2f06128c`），独立实跑
  `python3 scripts/check-m2381.py` = 4 ok 0 fail
- **构建指向**：主树侧 `./unixnu -D ../UniXNU-m2381`（路径记忆）；
  CI dispatch = `device_repository=RiverKawa271828/UniXNU-m2381`、
  `device_ref=main`；旧 orphan 分支 `device-m2381` 冻结在 origin 作回退，
  新仓 CI 首绿后可删
- `keys/*.pem`（owner 签名锚）仍为本地文件不入仓（`*.pem` gitignore，
  随拆分迁至新仓同位置）

本目录现仅存 `uefi/`（m2381Pkg 源，经 sync.sh 进 Mu fork；迁独立仓列
二期）。历史路径 `devices/m2381/unixnu-m2381/…` 出现在本仓 docs 与
commit message 时，一律指上述独立仓的对应路径。
