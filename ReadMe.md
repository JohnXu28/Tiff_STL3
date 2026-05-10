# Tiff_STL3 Project

## Static Library Overview

AppWizard has created this Tiff_STL3 library project for you. This file contains a summary of what you will find in each of the files that make up your Tiff_STL3 application.

---

## Project Files

### Tiff_STL3.vcxproj

This is the main project file for VC++ projects generated using an Application Wizard. It contains information about the version of Visual C++ that generated the file, and information about the platforms, configurations, and project features selected with the Application Wizard.

### Tiff_STL3.vcxproj.filters

This is the filters file for VC++ projects generated using an Application Wizard. It contains information about the association between the files in your project and the filters. This association is used in the IDE to show grouping of files with similar extensions under a specific node (for e.g. ".cpp" files are associated with the "Source Files" filter).

---

## Core Source Files

### StdAfx.h, StdAfx.cpp

These files are used to build a precompiled header (PCH) file named `Tiff_STL3.pch` and a precompiled types file named `StdAfx.obj`.

### Tiff_STL3.cpp, Tiff_STL3.h

These files are the core of the tiff class. Basically, you just need these two files.

> **Note:** If your compiler supports C++11, turn on the `ENABLE_SHARED_POINTER` define.

### Virtual_IO.cpp, Virtual_IO.h

If you don't define `VIRTUAL_IO` or `VIRTUAL_IO_STL`, you don't need these files. They are used for testing IO streams.

### Tiff_c.cpp and Version_C Files

These files were written a long time ago but are easy to understand.

---

## TiffTest Project

### TiffTest.cpp

Turn on the `Tiff_Test` define to test all TIFF files in the "TestImg" folder.

**Test Procedure:**
1. Read TIFF files and clone files into Output folder (example: `3RGB8.tif` → `3RGBOut.tif`)
2. Perform copy test (example: `3RGB8.tif` → `3RGBOut2.tif`)
3. Read `3RGB8Out.tif` and `3RGB8Out2.tif`, and verify that these two files are equivalent

---

## General Notes

### Limitations and Design

- **Compression:** This class does not support compressed TIFF files.
- **Purpose:** This TIFF class is designed for image processing and ICC color management testing.
- **Use Cases:** Descreen, color conversion (RGB, Lab, YCC [8 or 16-bit] → RGB, Lab, CMYK [8 or 16-bit]), dithering, conversion to PS/PDF, etc.
- **Output:** Most of the time, the final output is sent to print, so compression is not desired.

For examples, refer to `TiffTest.cpp` or `Tiff_STL3.h`.

---

## Test Images

| # | Type | Description |
|---|------|-------------|
| 0 | NULL | Pure empty files |
| 1 | LineArt | 1 channel, 1 bit |
| 2 | gray8 | Gray, 8 bits |
| 3 | RGB8 | R,G,B color image, 8 bits, interlaced |
| 4 | CMYK8 | C,M,Y,K color image, 8 bits, interlaced |
| 5 | Lab8 | Lab color image, 8 bits, interlaced |
| 6 | gray16 | Gray, 16 bits, interlaced |
| 7 | RGB16 | R,G,B color image, 16 bits, interlaced |
| 8 | CMYK16 | C,M,Y,K color image, 16 bits, interlaced |
| 9 | Lab16 | Lab color image, 16 bits, interlaced |
| 10 | RGB82 | R,G,B color image, 8 bits, non-interlaced |
| 11 | RGB162 | R,G,B color image, 16 bits, non-interlaced |
| 12 | CMYK82 | C,M,Y,K color image, 8 bits, non-interlaced |
| 13 | CMYK162 | C,M,Y,K color image, 16 bits, non-interlaced |
| 14 | Lab82 | Lab color image, 8 bits, non-interlaced |
| 15 | Lab162 | Lab color image, 16 bits, non-interlaced |
| 16 | MultiStrip | Multiple Strip Image |
| 17 | Ycc8 | Special file format for image processing simulation |
| 18 | CLR68 | 6 channels, 8 bits, embedded ICC profile |
| 19 | CLR616 | 6 channels, 16 bits, embedded ICC profile |
| 20 | IccLab8 | Lab, 8 bits, embedded ICC profile |
| 21 | IccLab16 | Lab, 16 bits, embedded ICC profile |
| 22 | Alpha8 | Alpha channel, 8 bits |
| 23 | Alpha16 | Alpha channel, 16 bits |
| 24 | Lzw8 | RGB, 8 bits, LZW compression |
| 25 | Lzw16 | RGB, 16 bits, LZW compression |
| 26 | Lzw_Gray8 | Gray, 8 bits, LZW compression |
| 27 | Lzw_Gray16 | Gray, 16 bits, LZW compression |
| 28 | Lzw_SWOP8 | SWOP, 8 bits, LZW compression |
| 29 | Lzw_SWOP16 | SWOP, 16 bits, LZW compression* |
| 30 | Lzw_Lab8 | Lab, 8 bits, LZW compression |
| 31 | Lzw_Lab16 | Lab, 16 bits, LZW compression |

> *Note: Compression size is larger than the original size

---

## Example Usage

```cpp
#include "stdafx.h"
#include <string>
#include <iostream>
using namespace std;
#include "Tiff_STL3.h" // Has claimed the namespace...
// using namespace AV_Tiff_STL3; // Don't need anymore...

int main(int argc, _TCHAR* argv[])
{
    SPTIFF lpIn = make_shared<CTiff>("Input.tif");

    int Width = lpIn->GetTagValue(ImageWidth);
    int Length = lpIn->GetTagValue(ImageLength);
    int resolution = lpIn->GetTagValue(XResolution);
    int samplesPerPixel = lpIn->GetTagValue(SamplesPerPixel);
    int bitspersample = lpIn->GetTagValue(BitsPerSample);

    SPTIFF lpOut = make_shared<CTiff>(Width, Length, resolution, samplesPerPixel, bitspersample);

    int BytesPerLine = Width * samplesPerPixel * bitspersample / 8;
    LPBYTE lpBuf = new BYTE[BytesPerLine];

    for(int i = 0; i < Length; i++)
    {
        lpIn->GetRow(lpBuf, i);
        LPBYTE lpTemp = lpBuf;
        for(int j = 0; j < Width; j++)
        {
            Process(lpBuf, Width); // Add your process here.
        }
        lpOut->PutRow(lpBuf, i);
    }
    delete []lpBuf;

    lpOut->SaveFile("Output.tif");
    
    return 0;
}
```

---
