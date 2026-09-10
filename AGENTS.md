# AGENTS.md

## Git

- `git push`（裸指令）已設定為同時推送 `github` 與 `Dropbox_WSL`：
  `origin` 掛了兩個 push URL（github + `/mnt/m/Dropbox/...`），直接 `git push` 即可。
- `origin` 的 fetch 走 `/mnt/m/Dropbox/Git/Tiff_STL3`；勿改回 Windows 路徑 `M:/...`（WSL 內無效）。
- 提交訊息風格比照現行歷史：`範圍:摘要`（如 `ReadMe:document TiffTest.cpp`）。
