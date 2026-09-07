# Tiff_STL3 / Tiff_STL4 — TIFF Image Module

> **Status**: This module has been fully refactored (A1–A10, B1–B5, C1–C4) and
> upgraded/renamed to **Tiff_STL4**. New code should use `Tiff_STL4.h` /
> namespace `AV_Tiff_STL4`; the legacy `Tiff_STL3.h` / `AV_Tiff_STL3` remains
> as a compatibility shim, so existing consumers (Icc4, ColorTools, Descreen,
> Dithering, …) work **without modification**. See `refactor.md` in the
> repository root for the full refactoring log.

---

## Feature Overview

| Feature | Read | Write | Notes |
|---------|------|-------|-------|
| Uncompressed | ✅ | ✅ | Single/multi strip, Planar 0/1/2 (planar reads are auto-`Pack`ed to RGBRGB) |
| LZW (Compression=5) | ✅ | ✅ | Read: `Lzw_Perplexity` (auto LSB/MSB bit-order detection); write: `Lzw` + Predictor=2 (8-bit) |
| **G3 / G4 (Compression=3/4)** | ✅ | ✅ | 1 bit/pixel. G3 1D MH + 2D (EOL/fill bits/tag bit), G4 MMR (V/H/Pass); code tables from `LibTiff/t4.h` (ITU standard) |
| 16-bit / 8-bit / 1-bit | ✅ | ✅ | 1-bit used by the G3/G4 paths |
| ICC Profile tag | ✅ | ✅ | `SetIccProfile` / `SaveIccProfile` / `RemoveIcc` |
| Predictor (horizontal differencing) | ✅ | ✅ | 8-bit, tag added automatically on save |
| Memory I/O | ✅ | ✅ | `ReadMemory` / `SaveMemory` (requires the matching IO macro branch) |
| Multi-strip read | ✅ | — | Strip tags normalized after read (RowsPerStrip=Length, Compression=1) |

---

## File Layout

```
Tiff_STL3/                        ← Directory name kept (git submodule path)
├── Include/
│   ├── Tiff_STL4.h               ← Main header: namespace AV_Tiff_STL4 (use for new code)
│   ├── Tiff_STL3.h               ← Compatibility shim: type aliases → AV_Tiff_STL4
│   └── LZW_Perplexity.h, LZW.h …
├── Src/
│   ├── Tiff_Tags.cpp             ← TiffTag family, IFD, tag operations (GetTag/SetTag/RemoveTag…)
│   ├── Tiff_RW.cpp               ← Core read/write (ReadImage/ReadTiff/SaveFile/BuildFileImage/Pack…)
│   ├── Tiff_LZW.cpp              ← LZW file paths (LZW_Compress/SaveTiff_lzw/ReadLzwStrips)
│   ├── CTiff.cpp                 ← CTiff convenience layer (CreateNew/row access/Icc Profile/JPG)
│   ├── G3G4.cpp                  ← CCITT G3/G4 codec + Tiff integration
│   ├── LZW_Integrated.cpp        ← Lzw (write) + Lzw_Perplexity (read) codecs
│   ├── Tiff_STL3_NE.cpp          ← (Legacy, not built)
│   └── Version_C/                ← C wrapper (Windows vcxproj only)
├── TiffTest/
│   └── TiffRefactorGuard.cpp     ← Behavior lock test (101 checks, target: Tiff_RefactorGuard)
├── TestImg/                      ← Sample images (incl. *_LZW.tif, 1LineArt.tif)
└── LZW/                          ← Legacy codecs (not built)
```

> **History**: the original single-file `Src/Tiff_STL3.cpp` (~2000 lines) was
> split into the four source files above and upgraded to STL4. The old content
> remains in git history (`git show HEAD:Src/Tiff_STL3.cpp`).

---

## Quick Start

### New code (Tiff_STL4)
```cpp
#include "Tiff_STL4.h"
using namespace AV_Tiff_STL4;

// Read
CTiff in;
in.ReadFile("in.tif");
int w = (int)in.GetTagValue(ImageWidth);   // tag query
LPBYTE pix = in.GetImageBuf();             // image memory

// Create + write (compression: omit = none, 1 = LZW, 3 = G3, 4 = G4)
CTiff out;
out.CreateNew(width, length, 72, samplesPerPixel, bitsPerSample, 1);
out.PutRow(rowPtr, y);            // per row
out.SaveFile("out.tif", 4);       // G4
```

### Legacy code (compatibility shim, unchanged)
```cpp
#include "Tiff_STL3.h"            // shim → Tiff_STL4.h
AV_Tiff_STL3::CTiff tiff;         // or simply CTiff tiff;
```

### Low-level API (Tiff class)
`Tiff` is the core reader/writer (throws on error paths); `CTiff` derives from
it, returns `Tiff_Err` codes instead and manages the image buffer for you.
The second parameter of `SaveFile` is the compression id: **0** none,
**1** LZW, **3** G3, **4** G4.

---

## Compression Internals

- **Write**: `BuildFileImage(img, mode)` assembles the complete file image
  (header + IFD + tag data + image + Exif) in memory and writes it in one go.
  LZW/G3/G4 compression releases the original image buffer
  (`CTiff::SaveFile` clears the cached `m_lpImageBuf` — **do not read the old
  pointer after Save**).
- **Read**: each strip reader normalizes the tags afterwards
  (StripOffsets → whole image, RowsPerStrip → Length, Compression → 1,
  Predicator removed).

---

## Build

### CMake (Linux / cross-platform)
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build          # produces Lib/libAVCMP.so and Tiff_RefactorGuard
```
Targets: `obj_Tiff_STL4` (merged into `AVCMP`), test `Tiff_RefactorGuard`.
(The directory `Tiff_STL3/` stays as-is — it is the git submodule path.)

### MSVC
`Src/Tiff_STL3.vcxproj` (Win32/x64/ARM/ARM64 configurations).

### Defines
- Required: `SYS_INFO`, `FIXED_VECTOR`, `ICC_Ver4` (engine consumers)
- `NO_SMART_POINTER` (SysInfo.h): reverts tag ownership to raw pointers with
  manual frees (default is the `unique_ptr`/`shared_ptr` mode — keep it)
- IO backend: Linux uses C stdio; Windows can select `IO_File` (Virtual_IO)
  or fstream

---

## Testing

| Test | Content | How |
|------|---------|-----|
| `Tiff_RefactorGuard` | 101 behavior checks: 8/16-bit, LZW, G3/G4 round-trips, RemoveTag ownership, multi-strip, Planar+Pack, 1-bit CMYKcm, invalid files, G3/G4 end-to-end | `cmake --build build --target Tiff_RefactorGuard && ./Tiff_RefactorGuard` (exit code = failure count) |
| `TestImg/` | Sample images for every format (incl. `*_LZW.tif`, `1LineArt.tif`) | Manual verification |

---

## Known Limitations

- G3/G4 support 1 bit/pixel, SamplesPerPixel=1 only; fax uncompressed mode
  (T4Options bit1 / T6Options bit0) is unsupported and reported as an error.
- G4 encoding does not use Pass mode (valid output, slightly below optimal
  compression).
- LZW writes Predictor=2 for 8-bit only; 16-bit is not predicted (poor gain).
- `MAXTAG = 40`: tags per page limit.
- The historical `RemoveTag` last-element bug is fixed, but note that
  `FixedVector::erase` semantics (returns bool) differ from `std::vector`
  (returns iterator) — mind this in cross-container code.

---

## History

- STL3 era: single-file `Tiff_STL3.cpp` + the LZW / VC++ AppWizard layout.
- STL4 upgrade: refactoring (file split, ownership, write path, decision
  matrix), correctness fixes (RemoveTag, GetValue truncation, LZW short-image
  overread, planar inline crash, predictor difference direction, …), G3/G4
  support, API rename. See `refactor.md` for the full log.
