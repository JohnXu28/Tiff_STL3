========================================================================
    STATIC LIBRARY : Tiff_STL3 Project Overview
========================================================================

AppWizard has created this Tiff_STL3 library project for you.

This file contains a summary of what you will find in each of the files that
make up your Tiff_STL3 application.


***Tiff_STL3.vcxproj***

  * This is the main project file for VC++ projects generated using an Application Wizard.
    It contains information about the version of Visual C++ that generated the file, and
    information about the platforms, configurations, and project features selected with the
    Application Wizard.

***Tiff_STL3.vcxproj.filters***

  * This is the filters file for VC++ projects generated using an Application Wizard. 
    It contains information about the association between the files in your project 
    and the filters. This association is used in the IDE to show grouping of files with
    similar extensions under a specific node (for e.g. ".cpp" files are associated with the
    "Source Files" filter).


/////////////////////////////////////////////////////////////////////////////

***StdAfx.h, StdAfx.cpp***
  
  * These files are used to build a precompiled header (PCH) file
    named Tiff_STL3.pch and a precompiled types file named StdAfx.obj.


***Tiff_STL3.cpp, Tiff_STL3.h***

  * These files are core of tiff class, basically you just need these two files.
    *** if compiler support C++ 11 , turn on define ENABLE_SHARED_POINTER ***
	
***Virtual_IO.cpp, Virtual_h***

  * If we didn't define VIRTUAL_IO, or VIRTUAL_IO_STL, we don't need these files.
    used for testing io stream.
	
***Tiff_c.cpp,....Files in Version_C***

  * These files was writtern long times ago, Easy to understand.
	
/////////////////////////////////////////////////////////////////////////////
========================================================================
    TiffTest Project Overview
========================================================================
***TiffTest.cpp***

  * Turn on define Tiff_Test, it will test all tiff files in folder "TestImg".
  
    1. Read Tiff files and clone files into Output folder, example : 3RGB8.tif --> 3RGBOut.tif.
    2. Copy test, example : 3RGB8.tif --> 3RGBOut2.tif.
    3. Read 3RGB8Out.tif, 3RGB8Out2.tif, and check these two files are equivalent.
	

/////////////////////////////////////////////////////////////////////////////
Other notes:

This class didn't suppot compressd Tiff.
This Tiff calss is designed for image processing and Icc color management testing.
We use it for descreen, color conversion(RGB, Lab, Ycc [8 or 16bis]--> RGB, Lab, CMYK[8 or 16bits]), 
dithering,...conver to ps, pdf,...
Most of time, the final output will send to print, so we don't want to compress.

We can find the examples in TiffTest.cpp or Tiff_STL3.h

Test Images:

- 0  NULL : pure empty files
- 1  LineArt : 1 channel, 1 bit
- 2  gray8 : Gray, 8 bits 
- 3  RGB8 : R,G,B color image, 8 bits, interlace
- 4  CMYK8 : C,M,Y,K color image, 8 bits, interlace
- 5  Lab8 : Lab color image, 8 bits, interlace
- 6  gray16 : Gray, 16 bits, interlace
- 7  RGB16 : R,G,B color image, 16 bits, interlace
- 8  CMYK16 : C,M,Y,K color image, 16 bits, interlace
- 9  Lab16 : Lab color image, 8 bits, interlace
- 10 RGB82 : R,G,B color image, 8 bits, noninterlace
- 11 RGB162 : R,G,B color image, 16 bits, noninterlace
- 12 CMYK82 : C,M,Y,K color image, 8 bits, noninterlace
- 13 CMYK162 : C,M,Y,K color image, 16 bits, noninterlace
- 14 Lab82", Lab color image, 8 bits, noninterlace
- 15 Lab162",Lab color image, 16 bits, noninterlace
- 16 MultiStrip : Multiple Strip Image. 
- 17 Ycc8 : Special file format for image processing simulation 
- 18 Special : Special Tiff format, Old version can't parser this tiff.

/////////////////////////////////////////////////////////////////////////////
Example:
		#include "stdafx.h"
		#include <string>
		#include <iostream>
		using namespace std;
		#include "Tiff_STL3.h" //Has claimed the namespace...
		//using namespace AV_Tiff_STL3; Don't need anymore...
		main(int argc, _TCHAR* argv[]))
		{
			CTiff In, Out;
			In.ReadFile("Input.tif");
		
			int Width = In.GetTagValue(ImageWidth);
			int Length = In.GetTagValue(ImageLength);
			int resolution = In.GetTagValue(XResolution);
			int samplesPerPixel = In.GetTagValue(SamplesPerPixel);
			int bitspersample = In.GetTagValue(BitsPerSample);
		
			Out.CreateNew(Width, Length, resolution, samplesPerPixel, bitspersample);
		
			int BytesPerLine = Width * samplesPerPixel * bitspersample / 8;
		
			LPBYTE lpBuf = new BYTE[BytesPerLine];
		
			for(int i = 0; i < Length; i++)
			{
				In.GetRow(lpBuf, i);
				LPBYTE lpTemp = lpBuf;
				for(int j = 0; j < Width; j++)
				Process(lpBuf, Width); //Add your process here.
				In.PutRow(lpBuf, i);
			}
			delete []lpBuf;
		
			Out.SaveFile("Output.tif");
		}