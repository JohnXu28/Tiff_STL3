/////////////////////////////////////////////////////////////////////x
// Tiff_LZW.cpp: LZW compression/decompression file paths.
// Split from Tiff_STL.cpp (B1).
//********************************************************************
//	You can get the sample code in "Tiff_STL4.h"
//********************************************************************
#include <iostream>
#include "stdafx.h"
#include "../Include/Tiff_STL4.h"

using namespace AV_Tiff_STL4;

#include <algorithm>
//#include <functional> //replaced by those files below by klocwork
#include <type_traits>
#include <cstddef>
#include <cstdlib>
#include <exception>
#include <new>
#include <iosfwd>
#include <cstdio>
#include <cstring>
#include <cmath>

using namespace std;

#include "../Include/LZW.h"
#include "../Include/LZW_Perplexity.h"

#if LZW
Tiff_Err Tiff::LZW_Compress()
{
	//Add Predicator Tag, for caculate offset correctly.
	const int StripSize = 16; //Max strips
	int Width = GetTagValue(ImageWidth);
	int Length = GetTagValue(ImageLength);
	int Samples = GetTagValue(SamplesPerPixel);
	int bitsPerSample = GetTagValue(BitsPerSample);
	int BytesPerLine = (Width * Samples * bitsPerSample + 7) / 8;
	//Rows per strip, at least 1, so short images (Length < StripSize) are safe.
	int rows = (Length + StripSize - 1) / StripSize;
	if (rows < 1)
		rows = 1;
	//Real strip count, never read past the image buffer.
	const int Strips = (Length + rows - 1) / rows;

	SetTagValue(RowsPerStrip, rows);

	//StripOffsets
	TiffTagPtr TagStripOffsets = GetTag(StripOffsets);
	//Backup Original Image Data Pointer
	LPBYTE lpImageBuf = TagStripOffsets->lpData;
	LPBYTE lpLzwBuf = new BYTE[Length * BytesPerLine * 2]; //Assume the compress ratio is 50%
	LPBYTE lpLzwBufTemp = lpLzwBuf;
	//Set LZW Data Pointer
	StripOffsetsTag* pTagStripOffsets = dynamic_cast<StripOffsetsTag*>(GetPtr(TagStripOffsets));
	pTagStripOffsets->SetLzwData(lpLzwBuf);
	//Set new StripOffsets Data
	TagStripOffsets->n = Strips;
	TagStripOffsets->lpData = new BYTE[Strips * 4];

	//StripByteCounts
	TiffTagPtr TagStripByteCounts = GetTag(StripByteCounts);
	TagStripByteCounts->n = Strips;
	TagStripByteCounts->type = Long;
	TagStripByteCounts->lpData = new BYTE[Strips * 4];

	//Start point of Image Data(LZW)
	//int Offset = CaculateOffset();
	int Offset = 0;
	Lzw* encoder = new Lzw;

	LPDWORD pOffset = (LPDWORD)(TagStripOffsets->lpData);
	LPDWORD pByteCounts = (LPDWORD)(TagStripByteCounts->lpData);
	int BytesPerStrip = 0;
	LPBYTE lpImage = lpImageBuf;
		
	if (bitsPerSample == 8)//Don't do predicator for 16 bits, It is not effective.	
	{
		encoder->PredicatorEncode(lpImage, Width, Length, Samples);
		m_IFD.m_TagList.push_back(TagListItem(new TiffTag(Predicator, Short, 1, 2, nullptr)));
	}
	else
		SetTagValue(Predicator, 1); //May not exist, Just for safety.

	for (int i = 0; i < Strips; ++i)
	{
		int ThisRows = rows;
		if (i == (Strips - 1))
			ThisRows = Length - rows * i;//Last strip

		BytesPerStrip = ThisRows * BytesPerLine;

		//Set Offset
		pOffset[i] = Offset;

		//Compress Strip Data
		DWORD CompressedSize = encoder->Encode(lpImage, lpLzwBufTemp + Offset, BytesPerStrip);

		//Set ByteCounts
		pByteCounts[i] = CompressedSize;
		Offset += CompressedSize;
		lpImage += BytesPerStrip;
	}

	//The image buffer is replaced by the LZW strip data, release it here.
	//CTiff::SaveFile() clears its cached m_lpImageBuf pointer afterwards.
	delete[]lpImageBuf;
	delete encoder;
	SetTagValue(Compression, 5); //LZW

	return Tiff_OK;
}

Tiff_Err Tiff::SaveTiff_lzw(IO_INTERFACE* IO)
{
	Tiff_Err ret = LZW_Compress();
	if (ret != Tiff_OK)
		return ret;
	vector<BYTE> img;
	ret = BuildFileImage(img, 1);
	if (ret != Tiff_OK)
		return ret;
	IO_Seek(0, SEEK_SET);
	IO_Write(&img[0], 1, img.size());
	return ret;
}

//Shared LZW strip reader. SingleStrip = single strip file with inline
//(count=1) StripOffsets/StripByteCounts values.
Tiff_Err Tiff::ReadLzwStrips(IO_INTERFACE* IO, bool SingleStrip)
{
	TiffTagPtr Tag = GetTag(RowsPerStrip);
	if (Tag->n != 1) //Only support single value
		return UnSupportCompressData;

	int Width = GetTagValue(ImageWidth);
	int Length = GetTagValue(ImageLength);
	int bitsPerSample = GetTagValue(BitsPerSample);
	int samplesPerPixel = GetTagValue(SamplesPerPixel);
	int BytesPerLine = (bitsPerSample * samplesPerPixel + 7) / 8 * Width;
	int rowsPerStrip = GetTagValue(RowsPerStrip);
	int predicator = GetTagValue(Predicator);

	TiffTagPtr TagStripOffsets = GetTag(StripOffsets);
	TiffTagPtr TagStripByteCounts = GetTag(StripByteCounts);

	LPDWORD lpStripOffset = nullptr;
	LPDWORD lpStripByteCounts = nullptr;
	DWORD strip = 1;
	DWORD inlineOffset = 0, inlineCount = 0;
	if (SingleStrip)
	{
		inlineOffset = GetTagValue(StripOffsets);
		inlineCount = GetTagValue(StripByteCounts);
	}
	else
	{
		if (TagStripOffsets->lpData == nullptr || TagStripByteCounts->lpData == nullptr)
			return UnSupportCompressData;//Array form required.
		strip = TagStripOffsets->n;
		lpStripOffset = (LPDWORD)TagStripOffsets->lpData;
		lpStripByteCounts = (LPDWORD)TagStripByteCounts->lpData;
	}

	//The Lzw data may larger than the original data, so we need to prepare a buffer for LZW decode.
	int MaxStripBufSize = BytesPerLine * rowsPerStrip;
	LPBYTE lpStripeBuf = new BYTE[MaxStripBufSize];
	LPBYTE lpStripeBuf_Out = new BYTE[MaxStripBufSize];

	LPBYTE lpImageBuf = new BYTE[BytesPerLine * Length];
	memset(lpImageBuf, 0, BytesPerLine * Length);
	LPBYTE lpImage = lpImageBuf;

	Lzw_Perplexity* Lzw_Decode = new Lzw_Perplexity;

	int LinesRemain = Length;
	int OutSize = 0;
	for (DWORD i = 0; i < strip; i++)
	{
		int offset = SingleStrip ? (int)inlineOffset : (int)*(lpStripOffset++);
		IO_Seek(offset, SEEK_SET);
		int Bufsize = SingleStrip ? (int)inlineCount : (int)*(lpStripByteCounts++);
		if (Bufsize > MaxStripBufSize)
		{
			MaxStripBufSize = (int)(Bufsize * 1.5);
			delete[]lpStripeBuf;
			lpStripeBuf = new BYTE[MaxStripBufSize];
		}

		IO_Read(lpStripeBuf, 1, Bufsize);

		int ThisBytes = 0;
		if (LinesRemain > rowsPerStrip)
		{
			ThisBytes = BytesPerLine * rowsPerStrip;
			LinesRemain -= rowsPerStrip;
		}
		else
			ThisBytes = BytesPerLine * LinesRemain;

		if(Lzw_Decode->Decode(lpStripeBuf, Bufsize, lpStripeBuf_Out, ThisBytes, &OutSize) == false)
		{
			cout << " *** Warning: LZW Decode Error. *** " << endl;
			break;
		}
		memcpy(lpImage, lpStripeBuf_Out, OutSize);

		if (OutSize != ThisBytes)
		{
			cout << " *** Warning: " << ThisBytes - OutSize << endl;
			memset(lpImage + OutSize, 255, ThisBytes - OutSize);
		}

		lpImage += ThisBytes;
	}

	delete[]lpStripeBuf;
	delete[]lpStripeBuf_Out;

	if (predicator == 2)
	{
		Lzw_Decode->PredicatorDecode(lpImageBuf, Width, Length, samplesPerPixel);
		RemoveTag(Predicator); //Photoshop hang if tag is not removed.
	}

	//Reset StripOffsets) and StripByteCounts)
	ResetStripTags(lpImageBuf, BytesPerLine * Length, true);
	delete Lzw_Decode;
	return Tiff_OK;
}

Tiff_Err Tiff::ReadSingleStripOffset_LZW(IO_INTERFACE* IO)
{
	return ReadLzwStrips(IO, true);
}

Tiff_Err Tiff::ReadMultiStripOffset_LZW(IO_INTERFACE* IO)
{
	return ReadLzwStrips(IO, false);
}

#endif //LZW

#if LZW
#endif //LZW

