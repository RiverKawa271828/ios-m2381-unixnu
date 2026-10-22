# archive — 旧 sweet 路线史料

这些文件是旧工作区 `/home/ri/work/ios-meizu-m2381` 的笔记/交接，只作历史与硬件
考证，不是当前施工指令。当前方案见 `../PLAN-unixnu.md`、`../BOOT-CHAIN.md`。

| 文件 | 内容 |
|---|---|
| `NOTES-m2381.md` | 旧 sweet 全程日志（阶段 0/1/2、GIC/FB/UFS、r34 取证） |
| `NOTES.md` | ios-sweet 上游全程日志 |
| `NOTES-topaz.md` | topaz（Redmi Note 12 4G）参照笔记 |
| `AGENTS-sweet.md` | 旧工作区约束 + 完整硬件表 + 旧路线图 |
| `r34-window-forensics.md` | r34 窗口读环取证（mad fatal / launchd 缓存） |
| `deepseek-handoff-r35.md` | r35 交接书（旧路线最后计划） |
| `glm-brief-r34.md`、`glm-report-r34.md` | GLM 的 r34 简报/报告 |
| `deepseek-brief-20261022.md`、`deepseek-reply-20261022.md` | 交接问答 |
| `fix-round/` | 小补丁/取证记录 |

## 使用注意

- 旧日志里的“下一步”都已被废弃；不要照着继续。
- 硬件常量（GIC/FB/UFS/触摸/内存图）仍可信，已提炼到
  `../HARDWARE-m2381.md`。
- r34 的 mad fatal/launchd 结论只解释旧路线为什么卡住；B2 换启动链后要重新取证，
  不要预设它会自动消失。
