/////////////////////////////////////////////////////////////////////x
// CTiff.cpp: high level CTiff wrapper.
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
extern const char* strTiffErr[9];


CTiff::CTiff()
{
	m_Width = m_Length = m_SamplesPerPixel = m_BitsPerSample = m_BytesPerLine = m_Resolution = 0;
	m_lpImageBuf = nullptr;
}

CTiff::~CTiff()
{
}

CTiff::CTiff(LPCSTR FileName) :CTiff()
{
	ReadFile(FileName);
}

CTiff::CTiff(string FileName) :CTiff()
{
	ReadFile(FileName);
}

CTiff::CTiff(int width, int length, int resolution, int samplesperpixel, int bitspersample, int AllocBuf) : CTiff()
{
	CreateNew(width, length, resolution, samplesperpixel, bitspersample, AllocBuf);
}


CTiff* CTiff::Clone(bool copy)
{
	int Width = GetTagValue(ImageWidth);
	int Length = GetTagValue(ImageLength);
	int resolution = GetTagValue(XResolution);
	int samplesPerPixel = GetTagValue(SamplesPerPixel);
	int bitspersample = GetTagValue(BitsPerSample);

	CTiff* lpTiff = new CTiff(Width, Length, resolution, samplesPerPixel, bitspersample, 1);

	if (copy == true)
	{
		int Size = Width * Length * samplesPerPixel * bitspersample / 8;
		memcpy(lpTiff->GetImageBuf(), GetImageBuf(), Size);
	}
	return lpTiff;
}

Tiff_Err CTiff::SetTag(TiffTagSignature Signature, FieldType type, DWORD n, DWORD value, LPBYTE lpBuf)
{
	DWORD SigType = (int)(Signature) | ((int)type << 16);
	TiffTagPtr New = CreateTag(SigType, n, value, nullptr);
	return Tiff::SetTag(New);
}

Tiff_Err	CTiff::SetTagValue(const TiffTagSignature Signature, DWORD Value)
{
	TiffTagPtr New = Tiff::GetTag(Signature);
	if (New != nullptr)
		New->value = Value;
	return Tiff_OK;
}

Tiff_Err CTiff::CreateNew(int width, int length, int resolution, int samplesperpixel, int bitspersample, int AllocBuf)
{//Initial IFD

	Reset();

	SetTag(NewSubfileType, Long, 1, 0); //For TiffLib.

	SetTag(ImageWidth, Long, 1, width);

	SetTag(ImageLength, Long, 1, length);

	SetTag(BitsPerSample, Short, samplesperpixel, bitspersample);

	//SetTag(new BitsPerSampleTag());

	SetTag(Compression, Short, 1, 1);

	switch (samplesperpixel)
	{//0:White is zero, 1:Black is Zero, 2:RGB color, 3:Palette color, 5:CMYK, 6:YCbCr
	case 1:SetTag(PhotometricInterpretation, Short, 1, 1); break;
	case 3:SetTag(PhotometricInterpretation, Short, 1, 2); break;
	case 4:SetTag(PhotometricInterpretation, Short, 1, 5); break;
	case 6:SetTag(PhotometricInterpretation, Short, 1, 0); break;
	case 7:SetTag(PhotometricInterpretation, Short, 1, 0); break;
	default:SetTag(PhotometricInterpretation, Short, 1, 1); break;
	}

	SetTag(SamplesPerPixel, Short, 1, samplesperpixel);

	SetTag(RowsPerStrip, Long, 1, length);

	int stripByteCounts = (int)ceil((double)width * bitspersample * 0.125) * length * samplesperpixel;

	SetTag(StripByteCounts, Long, 1, stripByteCounts);

	SetTag(StripOffsets, Long, 1, 0);//For code analysis.
	TiffTagPtr StripOffsetsTag = GetTag(StripOffsets);
	if (AllocBuf == 1)
	{//Create StripOffsets) Tag directly.
		StripOffsetsTag->lpData = new BYTE[stripByteCounts];
		memset(StripOffsetsTag->lpData, 0, stripByteCounts);
		m_lpImageBuf = StripOffsetsTag->lpData;
	}
	else
		StripOffsetsTag->lpData = m_lpImageBuf = nullptr;

	Tiff::SetTag(StripOffsetsTag);

	SetTag(XResolution, Rational, 1, resolution);

	SetTag(YResolution, Rational, 1, resolution);

	SetTag(ResolutionUnit, Short, 1, 2);
	//2:inch, 3:centimeter

	SetTag(PlanarConfiguration, Short, 1, 1);
	//1: Data is stored as RGBRGB..., 2:RRRR....,GGGG...., BBBB....

	SetTag(IccProfile, (UndefineType), 0, 0);

	//Basic Property for easy using.
	m_Width = width;
	m_Length = length;
	m_SamplesPerPixel = samplesperpixel;
	m_BitsPerSample = bitspersample;
	m_Resolution = resolution;
	m_BytesPerLine = (int)ceil((double)m_Width * m_SamplesPerPixel * m_BitsPerSample * 0.125);

	return Tiff_OK;
}

Tiff_Err CTiff::CreateNew(int width, int length, int resolution, int samplesperpixel, int bitspersample, LPCSTR InName)
{
	IO_INTERFACE* IO;
	try {
		if ((IO = IO_In(InName)) == nullptr)
			throw "*** CTiff::CreateNew() --> Create File Fail *** ";

		CreateNew(width, length, resolution, samplesperpixel, bitspersample, 1);

		IO_Read(m_lpImageBuf, 1, m_BytesPerLine * m_Length);

		IO_Close(IO);
		return Tiff_OK;
	}
	catch (const char* msg)
	{
		cout << msg << endl;
		return FileOpenErr;
	}
}

Tiff_Err CTiff::CreateNew(int width, int length, int resolution, int samplesperpixel, int bitspersample, LPCSTR InName, LPCSTR OutName)
{
	CreateNew(width, length, resolution, samplesperpixel, bitspersample, InName);
	SaveFile(OutName);
	return Tiff_OK;
}

Tiff_Err CTiff::ReadTiff(IO_INTERFACE* IO)
{
	try {
		Tiff_Err ret = Tiff::ReadTiff(IO);

		if (ret != Tiff_OK)
			return ret;

		//Basic Property for easy using.
		m_Width = GetTagValue(ImageWidth);
		m_Length = GetTagValue(ImageLength);
		m_SamplesPerPixel = GetTagValue(SamplesPerPixel);
		m_BitsPerSample = GetTagValue(BitsPerSample);
		m_BytesPerLine = (int)ceil((double)m_Width * m_SamplesPerPixel * m_BitsPerSample * 0.125);
		m_Resolution = GetTagValue(XResolution);
		TiffTagPtr TempTag = Tiff::GetTag(StripOffsets);
		m_lpImageBuf = TempTag->lpData;
	}

	catch (const char* ErrMsg)
	{
		cout << "*** " << ErrMsg << " Open Error. ***" << endl;
		return FileOpenErr;
	}
	catch (const Tiff_Err& Err)
	{
		cout << "Tiff Read Filer Error : " << strTiffErr[-(int)Err] << endl;
		return Err;
	}
	catch (...)
	{
		cout << "*** CTiff::ReadFile() --> unknown Error. ***" << endl;
		return UnDefineErr;
	}

	return Tiff_OK;
}

Tiff_Err CTiff::ReadFile(LPCSTR FileName)
{
	Reset();
	try {
		if (FileName == nullptr)
		{
			cout << "FileName is NULL." << endl;
			return FileOpenErr;
		}

		IO_INTERFACE* IO = IO_In(FileName);
		if (IO == nullptr)
			throw FileName;

		Tiff_Err ret = ReadTiff(IO);

		if (ret != Tiff_OK)
			cout << FileName << ":Read Fail." << endl;

		IO_Close(IO);

		return ret;
	}
	catch (const char* ErrMsg)
	{
		cout << ErrMsg << " Open fail." << endl;
		return FileOpenErr;
	}
	//catch (const Tiff_Err &Err)
	//{
	//	cout << "Tiff Open fail, Error Code:" << Err << endl;
	//	return Err;
	//}
	catch (...)
	{
		cout << "Tiff Open fail, Unknown Error" << endl;
		return FileOpenErr;
	}
}

Tiff_Err CTiff::ReadFile(string FileName)
{
	return ReadFile(FileName.c_str());
}

//LZW_Compress frees the original image buffer while compressing, so the cached
//m_lpImageBuf must be dropped after an LZW save (A5: no dangling pointer).
Tiff_Err CTiff::SaveFile(LPCSTR FileName, int lzw)
{
	Tiff_Err ret = Tiff::SaveFile(FileName, lzw);
	if (lzw >= 1)//LZW and G3/G4 all free the original image buffer while compressing.
		m_lpImageBuf = nullptr;
	return ret;
}

#if defined(VIRTUAL_IO) | defined(VIRTUAL_IO_STL)
Tiff_Err CTiff::ReadMemory(LPBYTE Buffer, size_t BufSize)
{
	if (Tiff::ReadMemory(Buffer, BufSize) != Tiff_OK)
		return FileOpenErr;
	//Basic Property for easy using.
	m_Width = GetTagValue(ImageWidth);
	m_Length = GetTagValue(ImageLength);
	m_SamplesPerPixel = GetTagValue(SamplesPerPixel);
	m_BitsPerSample = GetTagValue(BitsPerSample);
	m_BytesPerLine = (int)ceil((double)m_Width * m_SamplesPerPixel * m_BitsPerSample * 0.125);
	m_Resolution = GetTagValue(XResolution);
	TiffTagPtr TempTag = Tiff::GetTag(StripOffsets);
	m_lpImageBuf = TempTag->lpData;
	return Tiff_OK;
}
#endif //VIRTUAL_IO or VIRTUAL_IO_STL


template<class T>   //T:type, Ts:type size
int CTiff::GetRow(T* lpBuf, int Line, int pixel)
{
	if (Line < 0)
		Line = 0;
	if (Line > (int)(m_Length - 1))
		Line = m_Length - 1;

	LPBYTE lpIndex = (LPBYTE)m_lpImageBuf + m_BytesPerLine * Line;
	int Bytes = 0;
	if (pixel != 0)
		Bytes = (int)((pixel * m_SamplesPerPixel * m_BitsPerSample) >> 3);
	else
		Bytes = m_BytesPerLine;

	memcpy(lpBuf, lpIndex, Bytes);
	return pixel;
}

int CTiff::GetRow(LPBYTE lpBuf, int Line, int pixel)
{
	return GetRow<BYTE>(lpBuf, Line, pixel);
}

int CTiff::GetRow(LPWORD lpBuf, int Line, int pixel)
{
	return GetRow<WORD>(lpBuf, Line, pixel);
}

template<class T>  //T:type, Ts:type size
int CTiff::PutRow(T* lpBuf, int Line, int pixel)
{
	if (Line < 0)
		Line = 0;
	if (Line > (int)(m_Length - 1))
		Line = m_Length - 1;

	LPBYTE lpPosition = m_lpImageBuf + m_BytesPerLine * Line;
	//Default: If pixel ==0.
	int Bytes = 0;
	if (pixel != 0)
		Bytes = (int)((pixel * m_SamplesPerPixel * m_BitsPerSample) >> 3);
	else
		Bytes = m_BytesPerLine;

	memcpy((LPBYTE)lpPosition, (LPBYTE)lpBuf, Bytes);
	return pixel;
}

int CTiff::PutRow(LPBYTE lpBuf, int Line, int pixel)
{
	return PutRow<BYTE>(lpBuf, Line, pixel);
}

int CTiff::PutRow(LPWORD lpBuf, int Line, int pixel)
{
	return PutRow<WORD>(lpBuf, Line, pixel);
}

template<class T>   //T:type, Ts:type size
int CTiff::GetRowColumn(T* lpBuf, int x, int y, int RecX, int RecY)
{//Just for Gray(8 or 16 bits) and Color(8 or 16 bits).
	if ((m_BitsPerSample != 8) && (m_BitsPerSample != 16))
		return -1;

	if (x < 0)
		x = 0;
	if (x > (m_Width - 1))
		x = m_Width - 1;
	if (x + RecX > m_Width)
		RecX = m_Width - x;

	if (y < 0)
		y = 0;
	if (y > (m_Length - 1))
		y = m_Length - 1;
	if (y + RecY > m_Length)
		RecY = m_Length - y;

	//T* lpPosition = (T*)m_lpImageBuf;
	T* lpWidthBuf = new T[m_Width * m_SamplesPerPixel];
	LPBYTE lpCurrent = (LPBYTE)lpBuf;
	int LineBufSize = (int)(RecX * m_SamplesPerPixel * m_BitsPerSample) >> 3;
	int StartY = y;
	int EndY = y + RecY;

	for (int i = StartY; i < EndY; i++)
	{
		GetRow(lpWidthBuf, i);
		T* lpStartX = lpWidthBuf + (x * m_SamplesPerPixel);
		memcpy(lpCurrent, (LPBYTE)lpStartX, LineBufSize);
		lpCurrent += LineBufSize;
	}

	delete[]lpWidthBuf;
	return 0;
}

int CTiff::GetRowColumn(LPBYTE lpBuf, int x, int y, int RecX, int RecY)
{
	return GetRowColumn<BYTE>(lpBuf, x, y, RecX, RecY);
}

int CTiff::GetRowColumn(LPWORD lpBuf, int x, int y, int RecX, int RecY)
{
	return GetRowColumn<WORD>(lpBuf, x, y, RecX, RecY);
}


template<class T>   //T:type, Ts:type size
int CTiff::SetRowColumn(T* lpBuf, int x, int y, int RecX, int RecY)
{//Just for Gray(8 or 16 bits) and Color(8 or 16 bits).
	if ((m_BitsPerSample != 8) && (m_BitsPerSample != 16))
		return -1;

	LPBYTE lpTemp = (LPBYTE)lpBuf;
	int LineBufSize = (int)(RecX * m_SamplesPerPixel * m_BitsPerSample) >> 3;
	int StartY = y;
	int EndY = y + RecY;

	for (int indexY = StartY; indexY < EndY; ++indexY)
	{
		LPBYTE lpIndex = GetXY(x, indexY);
		memcpy(lpIndex, (LPBYTE)lpTemp, LineBufSize);
		lpTemp += LineBufSize;
	}

	return 0;
}

int CTiff::SetRowColumn(LPBYTE lpBuf, int x, int y, int RecX, int RecY)
{
	return SetRowColumn<BYTE>(lpBuf, x, y, RecX, RecY);
}

int CTiff::SetRowColumn(LPWORD lpBuf, int x, int y, int RecX, int RecY)
{
	return SetRowColumn<WORD>(lpBuf, x, y, RecX, RecY);
}

LPBYTE CTiff::GetXY(int X, int Y)
{
	if (X < 0)
		X = 0;
	if (X > (m_Width - 1))
		X = m_Width - 1;

	if (Y < 0)
		Y = 0;
	if (Y > (m_Length - 1))
		Y = m_Length - 1;

	return  m_lpImageBuf + \
		(m_BytesPerLine * Y) + \
		((X * m_SamplesPerPixel * m_BitsPerSample) >> 3);
}

//Math coordination
LPBYTE CTiff::GetXY_M(int X, int Y)
{
	if (X < 0)
		X = 0;
	if (X > (m_Width - 1))
		X = m_Width - 1;

	if (Y < 0)//Math coordination, Y=0 is the bottom line of the image.
		Y = m_Length - 1;
	if (Y > (m_Length - 1))//Math coordination, Y=Length-1 is the top line of the image.
		Y = 0;

	return  m_lpImageBuf + \
		(m_BytesPerLine * (m_Length - Y - 1)) + \
		((X * m_SamplesPerPixel * m_BitsPerSample) >> 3);
}

//GetXY (quick : No Check)
LPBYTE CTiff::GetXY_Q(int X, int Y)
{
	return  m_lpImageBuf + \
		(m_BytesPerLine * Y) + \
		((X * m_SamplesPerPixel * m_BitsPerSample) >> 3);
}

//Math coordination and No Check
LPBYTE CTiff::GetXY_MQ(int X, int Y)
{
	return  m_lpImageBuf + \
		(m_BytesPerLine * (m_Length - Y - 1)) + \
		((X * m_SamplesPerPixel * m_BitsPerSample) >> 3);
}

LPBYTE CTiff::GetImageBuf()
{
	return m_lpImageBuf;
}

void CTiff::SetImageBuf(LPBYTE lpBuf, bool FreeBuf)
{
	TiffTagPtr StripOffsetTag = Tiff::GetTag(StripOffsets);
	if (FreeBuf == true)
		if (StripOffsetTag->lpData != nullptr)
			delete[]StripOffsetTag->lpData;

	StripOffsetTag->lpData = lpBuf;
	m_lpImageBuf = lpBuf;
}

//Just for Image Adapter, don't release it, because we don't want read the image again.
void CTiff::ForgetImageBuf()
{
	TiffTagPtr StripOffsetTag = Tiff::GetTag(StripOffsets);
	StripOffsetTag->lpData = 0;
	m_lpImageBuf = 0;
}

//*********************************************************************
//Icc Profile
//*********************************************************************
Tiff_Err CTiff::SetIccProfile(char* IccFile)
{
	IO_INTERFACE* IO = IO_In(IccFile);
	if (IO == nullptr)
		throw " *** CTiff::SetIccProfile() --> Icc Profile open Fail. *** ";

	int FileSize = 0;
	IO_Read((LPBYTE)&FileSize, 1, 4);
	FileSize = SwapDWORD(FileSize);

	TiffTagPtr Icc = Tiff::GetTag(IccProfile);
	if (Icc != nullptr)
	{
		if (Icc->lpData != nullptr)
			delete[]Icc->lpData;
		Icc->lpData = new BYTE[FileSize];
		Icc->n = FileSize;
		Icc->value = 0;//Offset, recaculating when saveing file.
		IO_Seek(0, SEEK_SET);
		IO_Read(Icc->lpData, 1, FileSize);
	}

	if (IO != NULL)
		IO_Close(IO);

	return Tiff_OK;
}

void CTiff::SaveIccProfile(char* OutIccFile)
{
	TiffTagPtr TempTag = Tiff::GetTag(IccProfile);
	if (TempTag != nullptr)
	{
		IO_INTERFACE* IO = IO_Out(OutIccFile);
		if (IO != nullptr)
		{
			if (TempTag->lpData != nullptr)
				IO_Write(TempTag->lpData, 1, TempTag->n);
			IO_Close(IO);
		}
	}
}

void CTiff::RemoveIcc()
{
	TiffTagSignature sig = IccProfile;

	auto pos = find_if(m_IFD.m_TagList.begin(), m_IFD.m_TagList.end(),
		[&sig](TiffTagPtr& pos) {return pos->tag == sig; });

	if (pos != m_IFD.m_TagList.end())
	{
		//The owning pointer (TagListItem) releases the tag.
		m_IFD.m_TagList.erase(pos);
	}
}


#ifdef TIFF_EXT
#include <jpeglib\jpegfile.h>
//Only for color right now.
ErrCode	CTiff::ReadJPG(LPCSTR FileName)
{
	unsigned int width, length;
	CJpeg jpeg;
	LPBYTE lpBuf = jpeg.JpegFileToRGB(FileName, &width, &length);
	if (lpBuf == NULL)
		return FileOpenErr;

	CreateNew(width, length, 72, 3, 8, 0);
	SetImageBuf(lpBuf);

	return Tiff_OK;
}

ErrCode	CTiff::SaveJPG(LPCSTR FileName)
{
	CJpeg jpeg;
	int quality = 75;
	jpeg.RGBToJpegFile(FileName, m_lpImageBuf, m_Width, m_Length, TRUE, quality);
	return Tiff_OK;
}
#endif //TIFF_EXT

//Not Finish yet.
#ifdef TIFF_EXT

Read_Lzw
void TEST_LibTiff()
{
	char Lzw_In[128];
	char Lzw_Out[128];
	char Out_Tif[128];
	ConfigGetString("WorkTemp", "Lzw_In", "lzw.tif", (char*)Lzw_In);
	ConfigGetString("WorkTemp", "Lzw_Out", "lzw_Out.tif", (char*)Lzw_Out);
	ConfigGetString("WorkTemp", "Out_Tif", "Out.tif", (char*)Out_Tif);

	int width, length;
	float resolution;
	WORD samplesperlixel, bitspersample;

	//Get Input Tiff Info
	TIFF* tif = TIFFOpen(Lzw_In, "r");
	TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
	TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &length);
	TIFFGetField(tif, TIFFTAG_XRESOLUTION, &resolution);
	TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &samplesperlixel);
	TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &bitspersample);

	CTiff tiff;
	tiff.CreateNew(width, length, (int)resolution, samplesperlixel, bitspersample);

	//Create Output Tiff Info.
	TIFF* tif2 = TIFFOpen(Lzw_Out, "w");
	//TIFFSetField(tif2, TIFFTAG_IMAGEWIDTH, width);
	TIFFSetField(tif2, TIFFTAG_IMAGEWIDTH, width);
	TIFFSetField(tif2, TIFFTAG_IMAGELENGTH, length);

	TIFFSetField(tif2, TIFFTAG_BITSPERSAMPLE, bitspersample);
	//TIFFSetField(tif2, TIFFTAG_COMPRESSION, COMPRESSION_NONE);
	TIFFSetField(tif2, TIFFTAG_COMPRESSION, COMPRESSION_LZW);
	TIFFSetField(tif2, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);

	TIFFSetField(tif2, TIFFTAG_SAMPLESPERPIXEL, samplesperlixel);
	TIFFSetField(tif2, TIFFTAG_ROWSPERSTRIP, length);

	TIFFSetField(tif2, TIFFTAG_XRESOLUTION, resolution);
	TIFFSetField(tif2, TIFFTAG_YRESOLUTION, resolution);
	TIFFSetField(tif2, TIFFTAG_RESOLUTIONUNIT, RESUNIT_INCH);

	TIFFSetField(tif2, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
	TIFFSetField(tif2, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);


	void* buf = _TIFFmalloc(TIFFScanlineSize(tif));
	LPBYTE lpBuf = new BYTE[width * 3];
	//LPBYTE lpBuf = new BYTE[Width * 3];
	for (int row = 0; row < length; row++)
	{
		TIFFReadScanline(tif, buf, row);
		tiff.GetRow(lpBuf, row, width);
		memcpy(lpBuf, buf, width * 3);
		tiff.PutRow(lpBuf, row, width);
		TIFFWriteScanline(tif2, lpBuf, row);
	}
	tiff.SaveFile(Out_Tif);
	TIFFClose(tif);
	TIFFClose(tif2);
	_TIFFfree(buf);
	delete[]lpBuf;
}
#endif //TIFF_EXT

