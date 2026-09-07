# Tiff_STL3 / Tiff_STL4 — TIFF 影像模組

> **現況**：本模組已完成全面重構（A1–A10、B1–B5、C1–C4）並升級改名為 **Tiff_STL4**。
> 新代碼請使用 `Tiff_STL4.h` / namespace `AV_Tiff_STL4`；舊 `Tiff_STL3.h` / `AV_Tiff_STL3`
> 以相容 shim 保留，既有消費端（Icc4、ColorTools、Descreen、Dithering…）**不需修改**。
> 完整重構記錄見仓库根目錄 `refactor.md`。

---

## 功能總覽

| 功能 | 讀 | 寫 | 說明 |
|------|----|----|------|
| Uncompressed | ✅ | ✅ | 單/多 strip、Planar 0/1/2（planar 讀取自動 `Pack` 成 RGBRGB）|
| LZW（Compression=5） | ✅ | ✅ | 讀：`Lzw_Perplexity`（LSB/MSB 位元序自動判別）；寫：`Lzw` + Predictor=2（8-bit）|
| **G3 / G4（Compression=3/4）** | ✅ | ✅ | 1 bit/pixel。G3 1D MH + 2D（EOL/fill/tag bit）、G4 MMR（V/H/Pass）；碼表取自 `LibTiff/t4.h`（ITU 標準）|
| 16-bit / 8-bit / 1-bit | ✅ | ✅ | 1-bit 僅 G3/G4 用途 |
| ICC Profile tag | ✅ | ✅ | `SetIccProfile` / `SaveIccProfile` / `RemoveIcc` |
| Predictor（水平差分） | ✅ | ✅ | 8-bit，寫入端自動加 tag |
| 記憶體 I/O | ✅ | ✅ | `ReadMemory` / `SaveMemory`（需對應 IO 巨集分支）|
| Multi-strip 讀取 | ✅ | — | 含 strip tag 正規化（讀後 RowsPerStrip=Length、Compression=1）|

---

## 檔案配置

```
Tiff_STL3/                        ← 目錄名保留（git submodule 路徑）
├── Include/
│   ├── Tiff_STL4.h               ← 主標頭：namespace AV_Tiff_STL4（新代碼用）
│   ├── Tiff_STL3.h               ← 相容 shim：型別別名 → AV_Tiff_STL4（舊代碼免改）
│   └── LZW_Perplexity.h、LZW.h …
├── Src/
│   ├── Tiff_Tags.cpp             ← TiffTag 家族、IFD、tag 操作（GetTag/SetTag/RemoveTag…）
│   ├── Tiff_RW.cpp               ← 核心讀寫（ReadImage/ReadTiff/SaveFile/BuildFileImage/Pack…）
│   ├── Tiff_LZW.cpp              ← LZW 存讀路徑（LZW_Compress/SaveTiff_lzw/ReadLzwStrips）
│   ├── CTiff.cpp                 ← CTiff 高階包裝層（CreateNew/Row access/Icc Profile/JPG）
│   ├── G3G4.cpp                  ← CCITT G3/G4 codec + Tiff 接線
│   ├── LZW_Integrated.cpp        ← Lzw（寫）+ Lzw_Perplexity（讀）codec
│   ├── Tiff_STL3_NE.cpp          ← （舊版，未納入建置）
│   └── Version_C/                ← C wrapper（僅 Windows vcxproj）
├── TiffTest/
│   └── TiffRefactorGuard.cpp     ← 行為鎖定測試（101 項，target: Tiff_RefactorGuard）
├── TestImg/                      ← 測試影像（含 LZW 範例）
└── LZW/                          ← 舊版 codec（未納入建置）
```

> **沿革**：原單檔 `Src/Tiff_STL3.cpp`（~2000 行）已於 B1 拆分為上述四個原始檔並
> 升級改名 STL4；舊內容保存於 git 歷史（`git show HEAD:Src/Tiff_STL3.cpp`）。

---

## 快速上手

### 新代碼（Tiff_STL4）
```cpp
#include "Tiff_STL4.h"
using namespace AV_Tiff_STL4;

// 讀取
CTiff in;
in.ReadFile("in.tif");
int w = (int)in.GetTagValue(ImageWidth);   //tag 值查詢
LPBYTE pix = in.GetImageBuf();             //影像記憶體

// 建立 + 寫入（compression：省略=無、1=LZW、3=G3、4=G4）
CTiff out;
out.CreateNew(width, length, 72, samplesPerPixel, bitsPerSample, 1);
out.PutRow(rowPtr, y);            // 每列
out.SaveFile("out.tif", 4);       // G4
```

### 舊代碼（相容 shim，免改）
```cpp
#include "Tiff_STL3.h"            // shim → Tiff_STL4.h
AV_Tiff_STL3::CTiff tiff;         // 或直接 CTiff tiff;
```

### 低階 API（Tiff 類）
`Tiff` 為核心讀寫類（錯誤路徑丟例外）；`CTiff` 繼承之，改為回傳 `Tiff_Err`
並代管影像 buffer。`SaveFile` 第二參數即壓縮碼：**0** 無、**1** LZW、**3** G3、**4** G4。

---

## 壓縮流程內部

- **寫入**：`BuildFileImage(img, mode)` 於記憶體組出完整檔案（header + IFD +
  tag data + image + Exif）後單次寫出；LZW/G3/G4 壓縮後原影像 buffer 會被釋放
  （`CTiff::SaveFile` 會同步清掉 `m_lpImageBuf` 快取 — **Save 後請勿再讀原指標**）。
- **讀取**：各 strip reader 讀後會正規化 tag（StripOffsets→整塊影像、
  RowsPerStrip→Length、Compression→1、移除 Predicator）。

---

## 建置

### CMake（Linux / 跨平台）
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build          # 產出 Lib/libAVCMP.so 與 Tiff_RefactorGuard
```
Target：`obj_Tiff_STL4`（併入 `AVCMP`）、測試 `Tiff_RefactorGuard`。
（目錄名 `Tiff_STL3/` 為 git submodule 路徑，故維持不變。）

### MSVC
`Src/Tiff_STL3.vcxproj`（含 Win32/x64/ARM/ARM64 設定）。

### 編譯旗標
- 必要 define：`SYS_INFO`、`FIXED_VECTOR`、`ICC_Ver4`（引擎消費端）
- `NO_SMART_POINTER`（SysInfo.h）：tag 所有權退回 raw pointer + 手動釋放
  （預設為 `unique_ptr`/`shared_ptr` 模式，建議維持）
- IO 後端：Linux 使用 C stdio；Windows 可選 `IO_File`（Virtual_IO）或 fstream

---

## 測試

| 測試 | 內容 | 執行 |
|------|------|------|
| `Tiff_RefactorGuard` | 101 項行為鎖定：8/16-bit、LZW、G3/G4 round-trip、RemoveTag 所有權、multi-strip、Planar+Pack、1-bit CMYKcm、壞檔處理、G3/G4 端對端 | `cmake --build build --target Tiff_RefactorGuard && ./Tiff_RefactorGuard`（exit code = 失敗數）|
| `TestImg/` | 各格式樣本影像（含 `*_LZW.tif`、`1LineArt.tif`）| 手動驗證用 |

---

## 已知限制

- G3/G4 僅支援 1 bit/pixel、SamplesPerPixel=1；fax uncompressed mode
  （T4Options bit1 / T6Options bit0）不支援，偵測到即回錯誤。
- G4 編碼未使用 Pass mode（合法輸出，壓縮率略低於最佳）。
- LZW 寫入使用 Predictor=2 僅限 8-bit；16-bit 不做 predictor（效果不佳）。
- `MAXTAG = 40`：單頁 tag 數上限。
- `RemoveTag` 對最後一個 tag 的歷史 bug 已修復，惟 `FixedVector::erase`
  語義（回傳 bool）與 `std::vector`（回傳 iterator）不同，跨容器代碼需留意。

---

## 沿革

- STL3 時期：單檔 `Tiff_STL3.cpp` + LZW/VC++ AppWizard 結構。
- STL4 升級：重構（拆檔/所有權/寫檔路徑/決策矩陣）、正確性修復
  （RemoveTag、GetValue 截斷、LZW 小圖越界、planar inline crash、
  predictor 差分方向等）、G3/G4 新增、API 升級改名。詳見 `refactor.md`。
