/////////////////////////////////////////////////////////////////////x
// Tiff_STL.cpp: implementation of the CTiff class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "..\SysInfo\SysInfo.h"
#include "Tiff_STL.h"
#include <algorithm>
#include <functional>
#include <math.h>
//////////////////////////////////////////////////////////////////////
// Helpful Class and Function
//////////////////////////////////////////////////////////////////////
//#define _WINDOWS
using namespace AV_Tiff_STL;

TIFFTAG::TIFFTAG()
{
	Reset();
}

TIFFTAG::TIFFTAG(TiffTagSignature Signature)
{
	tag = Signature;
	type = Unknown;
	n = 0;
	value = 0;
}

TIFFTAG::TIFFTAG(DWORD TypeSignature, DWORD n, DWORD value)
{
	tag = (TiffTagSignature)(0xFFFF & TypeSignature);
	type = (FieldType)(0XFFFF & (TypeSignature >> 16));
	this->n = n;
	this->value = value;
}

TIFFTAG::TIFFTAG(TiffTagSignature Tag, FieldType Type, DWORD n, DWORD value)
{
	tag = Tag;
	type = Type;
	this->n = n;
	this->value = value;
}

void TIFFTAG::Reset()
{
	tag = NullTag;
	type = Unknown;
	n = 0;
	value = 0;
}

//////////////////////////////////////////////////////////////////////u
// Helpful Class and Function
//////////////////////////////////////////////////////////////////////

RationalNumber::RationalNumber()
{
	Fraction = 0;
	Denominator = 0;
}

RationalNumber::~RationalNumber()
{
}

int RationalNumber::SetRationalNumber(WORD fraction, WORD denominator)
{
	Fraction = fraction * 100;
	Denominator = denominator * 100;
	return 1;
}



//////////////////////////////////////////////////////////////////////
// CTiff
//////////////////////////////////////////////////////////////////////

CTiff::CTiff()
{
	IFD_Offset = IFD.NextIFD = 8;
	IFD.m_Tags.reserve(MAXTAG);
	m_lpImageBuf = NULL;
	m_FileName = NULL;
	m_File = NULL;
	m_IccProfile = NULL;
}

CTiff::~CTiff()
{
	IFD.m_Tags.clear();
	if (m_lpImageBuf != NULL)
		delete[]m_lpImageBuf;
	if (m_IccProfile != NULL)
		delete[]m_IccProfile;
}

TiffTagSignature CTiff::CheckingTag(const TiffTagSignature Signature)
{
	switch (Signature)
	{
	case NewSubfileType:
	case SubfileType:
	case ImageWidth:
	case ImageLength:
	case BitsPerSample:
	case Compression:
	case PhotometricInterpretation:
	case StripOffsets:
	case SamplesPerPixel:
	case RowsPerStrip:
	case StripByteCounts:
	case XResolution:
	case YResolution:
	case PlanarConfiguration:
	case XPosition:
	case YPosition:
	case IccProfile:
	case ReferenceBlackWhite:
	case ResolutionUnit:return	Signature; break;

	case NullTag:
	case Threshholding:
	case Orientation:
	case MinSampleValue:
	case MaxSampleValue:
	case CellWidth:
	case CellLength:
	case FillOrder:
	case DocumentName:
	case ImageDescription:
	case Make:
	case Model:
	case PageName:
	case FreeOffsets:
	case FreeByteCounts:
	case GrayResponseUnit:
	case GrayResponsCurve:
	case T4Options:
	case T6Options:
	case PageNumber:
	case TransferFunction:
	case Software:
	case DateTime:
	case Artist:
	case HostComputer:
	case Predicator:
	case WhitePoint:
	case PrimaryChromaticities:
	case ColorMap:
	case HalftoneHints:
	case TileWidth:
	case TileLength:
	case TileOffsets:
	case TileByteCounts:
	case InkSet:
	case InkNames:
	case NumberOfInks:
	case DotRange:
	case TargetPrinter:
	case ExtraSamples:
	case SampleFormat:
	case SMinSampleValue:
	case SMaxSampleValue:
	case TransforRange:
	case JPEGProc:
	case JPEGInterchangeFormat:
	case JPEGIngerchangeFormatLength:
	case JPEGRestartInterval:
	case JPEGLosslessPredicators:
	case JPEGPointTransforms:
	case JPEGQTable:
	case JPEGDCTable:
	case JPEGACTable:
	case YCbCrCoefficients:
	case YCbCrSampling:
	case YCbCrPositioning:
		//	case ReferenceBlackWhite:
	case CopyRight:
	default: return NullTag; break;
	}
}

ErrCode CTiff::GetTag(const TiffTagSignature Signature, TIFFTAG &TiffTag)
{
	TAG_LIST::iterator Index = find_if(IFD.m_Tags.begin(), IFD.m_Tags.end(), TIFFTAG(Signature));
	if (Index != IFD.m_Tags.end())
		TiffTag = *Index;
	else
		TiffTag = TIFFTAG(NullTag, Unknown, 0, 0);//Return Null Tag.

	return Tiff_OK;
}

DWORD CTiff::GetTagValue(const TiffTagSignature Signature)
{
	TIFFTAG TempTag;
	GetTag(Signature, TempTag);
	return TempTag.value;
}

ErrCode CTiff::SetTag(const TIFFTAG NewTag)
{
	if (CheckingTag(NewTag.tag) == NullTag) return Tiff_OK;//If the tag is unknown, just forget it!
	TAG_LIST::iterator Index = find_if(IFD.m_Tags.begin(), IFD.m_Tags.end(), TIFFTAG(NewTag.tag));
	if (Index != IFD.m_Tags.end())
		*Index = NewTag;
	else
	{
		IFD.m_Tags.push_back(NewTag);
		IFD.EntryCounts = (WORD)IFD.m_Tags.size();
	}
	return Tiff_OK;
}

ErrCode CTiff::SetTagValue(const TiffTagSignature Signature, int Value)
{
	TIFFTAG TempTag;
	GetTag(Signature, TempTag);
	if (TempTag.tag == NullTag)
	{
		TempTag.tag = Signature;
		TempTag.type = Short;
		TempTag.n = 1;
	}
	TempTag.value = Value;
	return SetTag(TempTag);
}

ErrCode CTiff::ReadFile(LPCTSTR FileName)
{
	DWORD	TiffVersion;
	WORD	TagCount, BitsPerSampleValue;
	RationalNumber  Resolution;
	TIFFTAG TempTag;
	int		i;

	if (FileName == NULL)
		return FileOpenErr;
	m_FileName = FileName;
	m_File = fopen(FileName, "rb+");
	if (m_File == NULL)
		return FileOpenErr;
	fseek(m_File, 0, SEEK_SET);
	fread(&TiffVersion, 1, 4, m_File);

	if (TiffVersion != 0x002A4949)
	{
		fclose(m_File);
		m_File = NULL;
		return VersionErr;
	}

	fread(&IFD_Offset, 1, 4, m_File);
	fseek(m_File, IFD_Offset, SEEK_SET);
	fread(&TagCount, 1, 2, m_File);
	if (TagCount >= MAXTAG)
	{//Too many m_Tags
		fclose(m_File);
		m_File = NULL;
		return TooManyTags;
	}

	DWORD Temp[MAXTAG * 3];
	fread(Temp, TagCount * 3, 4, m_File);
	for (i = 0; i < TagCount * 3; i += 3)
	{
		TIFFTAG TempTag(Temp[i], Temp[i + 1], Temp[i + 2]);
		SetTag(TempTag);
	}

	//Read BitsPerSample data
	GetTag(BitsPerSample, TempTag);
	if (TempTag.n != 1)
	{
		fseek(m_File, TempTag.value, SEEK_SET);
		fread(&BitsPerSampleValue, 1, 2, m_File);
		TempTag.value = BitsPerSampleValue;
		SetTag(TempTag);
	}

	//Read Resolution Data
	GetTag(XResolution, TempTag);
	fseek(m_File, TempTag.value, SEEK_SET);
	fread(&Resolution, 1, sizeof(RationalNumber), m_File);
	TempTag.value = Resolution.Fraction / Resolution.Denominator;
	SetTag(TempTag);

	GetTag(YResolution, TempTag);
	fseek(m_File, TempTag.value, SEEK_SET);
	fread(&Resolution, 1, sizeof(RationalNumber), m_File);
	TempTag.value = Resolution.Fraction / Resolution.Denominator;
	SetTag(TempTag);

	//Read Data
	GetTag(StripOffsets, TempTag);
	if (TempTag.n == 1)
	{
		fseek(m_File, TempTag.value, SEEK_SET);
		GetTag(StripByteCounts, TempTag);
		if (m_lpImageBuf != NULL)
			delete[]m_lpImageBuf;
		m_lpImageBuf = new BYTE[TempTag.value];
		fread(m_lpImageBuf, 1, TempTag.value, m_File);
	}
	else
	{
		TIFFTAG	TagStripByteCounts;
		GetTag(StripByteCounts, TagStripByteCounts);
		if (TagStripByteCounts.n != TempTag.n) return TagStripErr;

		int Strip = TempTag.n;
		int stripByteCounts = 0;

		LPDWORD lpStripOffsets = new DWORD[Strip];
		LPDWORD lpStripByteCounts = new DWORD[Strip];

		fseek(m_File, TempTag.value, SEEK_SET);//StripOffsets
		fread(lpStripOffsets, 4, TempTag.n, m_File);
		fseek(m_File, TagStripByteCounts.value, SEEK_SET);
		fread(lpStripByteCounts, 4, TagStripByteCounts.n, m_File);

		for (i = 0; i < Strip; i++)
			stripByteCounts += lpStripByteCounts[i];

		if (m_lpImageBuf != NULL)
			delete[]m_lpImageBuf;
		m_lpImageBuf = new BYTE[stripByteCounts];

		LPBYTE lpTemp = m_lpImageBuf;
		for (i = 0; i < Strip; i++)
		{
			fseek(m_File, lpStripOffsets[i], SEEK_SET);
			fread(lpTemp, 1, lpStripByteCounts[i], m_File);
			lpTemp += lpStripByteCounts[i];
		}

		//Reset Tag Data "StripOffset, StripByteCounts, RowPerStrip"
		TIFFTAG	TagImageLength, TagRowsPerStrip;

		TagStripByteCounts.n = 1;
		TagStripByteCounts.value = stripByteCounts;
		SetTag(TagStripByteCounts);

		GetTag(ImageLength, TagImageLength);
		GetTag(RowsPerStrip, TagRowsPerStrip);
		TagRowsPerStrip.value = TagImageLength.value;
		SetTag(TagRowsPerStrip);
		TempTag.n = 1;//StripOffset
		//Don't worry about the StripOffset.value, It will be recount in writefile.
		SetTag(TempTag);

		delete lpStripOffsets;
		delete lpStripByteCounts;
	}

	//IccProfile
	GetTag(IccProfile, TempTag);
	if (TempTag.n != 0)
	{
		m_IccProfile = new BYTE[TempTag.n];
		fseek(m_File, TempTag.value, SEEK_SET);
		fread(m_IccProfile, 1, TempTag.n, m_File);
	}
	else
		SetTag(TIFFTAG(IccProfile, UndefineType, 0, 0));

	fclose(m_File);
	m_File = NULL;

	return Tiff_OK;
}

ErrCode CTiff::CreateNew(LPCTSTR FileName, int Width, int Length, int Resolution, int sampleperpixel, int bitspersample)
{//Used for scanning! Doesn't allocate Buf.
	ErrCode ret = CreateNew(Width, Length, Resolution, sampleperpixel, bitspersample, 0);
	if (ret != Tiff_OK)
		return ret;
	m_FileName = FileName;
	return SaveFile(FileName);
}

ErrCode CTiff::CreateNew(int Width, int Length, int Resolution, int sampleperpixel, int bitspersample, int AllocBuf)
{//Initial IFD
	TIFFTAG Temp;

	SetTag(TIFFTAG(NewSubfileType, Byte, 4, 0));

	SetTag(TIFFTAG(ImageWidth, Long, 1, Width));

	SetTag(TIFFTAG(ImageLength, Long, 1, Length));

	SetTag(TIFFTAG(BitsPerSample, Short, sampleperpixel, bitspersample));
	//entryCount * sizeof(TIFFTAG);//8 8 8
	// You need to caculate the offset later.
	SetTag(TIFFTAG(Compression, Short, 1, 1));
	if (sampleperpixel == 4)
		SetTag(TIFFTAG(PhotometricInterpretation, Short, 1, 5));
	else
		SetTag(TIFFTAG(PhotometricInterpretation, Short, 1, 2));
	//0:White is zero, 1:Black is Zero, 2:RGB color, 3:Palette color, 5:CMYK, 6:YCbCr

	SetTag(TIFFTAG(StripOffsets, Long, 1, 0));
	//It will be recaculated when writeing file.
	//entryCount * sizeof(TIFFTAG) + 3 * sizeof(XYRESOLUTION);

	SetTag(TIFFTAG(SamplesPerPixel, Short, 1, sampleperpixel));

	SetTag(TIFFTAG(RowsPerStrip, Long, 1, Length));

	Temp.tag = StripByteCounts;
	Temp.type = Long;
	Temp.n = 1;
	Temp.value = (DWORD)(ceil(Width * bitspersample * 0.125) * Length * sampleperpixel);
	SetTag(Temp);

	SetTag(TIFFTAG(XResolution, Rational, 1, Resolution));
	//	xResolution.SetRationalNumber(Resolution, 1);

	SetTag(TIFFTAG(YResolution, Rational, 1, Resolution));
	//	yResolution.SetRationalNumber(Resolution, 1);

	SetTag(TIFFTAG(ResolutionUnit, Short, 1, 2));
	//2:inch, 3:centimeter

	SetTag(TIFFTAG(PlanarConfiguration, Short, 1, 1));
	//1: Data is stored as RGBRGB..., 2:RRRR....,GGGG...., BBBB....

	SetTag(TIFFTAG(IccProfile, UndefineType, 0, 0));

	if (m_lpImageBuf != NULL)
		delete[]m_lpImageBuf;

	if (AllocBuf == 1)
	{
		GetTag(StripByteCounts, Temp);
		m_lpImageBuf = new BYTE[Temp.value];
	}
	return Tiff_OK;
}

ErrCode CTiff::CreateBuf()
{
	if (m_lpImageBuf != NULL)
		delete[]m_lpImageBuf;

	int Width = GetTagValue(ImageWidth);
	int Length = GetTagValue(ImageLength);
	int samplesPerPixel = GetTagValue(SamplesPerPixel);
	int bitsPerSample = GetTagValue(BitsPerSample);
	int BufSize = (int)(ceil(Width * bitsPerSample * 0.125) * Length * samplesPerPixel);
	SetTagValue(StripByteCounts, BufSize);
	m_lpImageBuf = new BYTE[BufSize];

	return Tiff_OK;
}

int CTiff::GetRow(LPBYTE lpBuf, int Line, int Bytes)
{// 8 or 16 Bits, Who cares.
	TIFFTAG TagImageWidth, TagImageLength, TagBitsPerSample, TagSamplesPerPixel;
	LPBYTE	lpIndex;
	int		BytesPerLine;

	GetTag(ImageWidth, TagImageWidth);
	GetTag(ImageLength, TagImageLength);
	GetTag(BitsPerSample, TagBitsPerSample);
	GetTag(SamplesPerPixel, TagSamplesPerPixel);
	BytesPerLine = (int)ceil(TagImageWidth.value * TagSamplesPerPixel.value * TagBitsPerSample.value * 0.125);

	if (Line < 0)
		Line = 0;
	if (Line > (int)(TagImageLength.value - 1))
		Line = TagImageLength.value - 1;

	lpIndex = (LPBYTE)m_lpImageBuf + BytesPerLine * Line;
	memcpy(lpBuf, lpIndex, Bytes);

	return Bytes;
}

int CTiff::GetRow(LPWORD lpBuf, int Line, int Words)
{
	int ByteCounts = Words * 2;
	GetRow((LPBYTE)lpBuf, Line, ByteCounts);
	return Words;
}

int CTiff::PutRow(LPBYTE lpBuf, int Line, int Bytes)
{
	LPBYTE lpPosition;
	lpPosition = (LPBYTE)m_lpImageBuf;

	TIFFTAG	TempTag;
	GetTag(ImageWidth, TempTag);
	int Width = TempTag.value;

	GetTag(SamplesPerPixel, TempTag);
	int samplesPerPixel = TempTag.value;

	GetTag(BitsPerSample, TempTag);
	int bitsPerSample = TempTag.value;

	int BytesPerLine = (int)(Width * samplesPerPixel * bitsPerSample * 0.125);

	GetTag(ImageLength, TempTag);
	int Length = TempTag.value;

	if (Line < Length)
	{
		lpPosition += BytesPerLine * Line;
		memcpy(lpPosition, lpBuf, Bytes);
		return Bytes;
	}
	else
		return 0;
}

int CTiff::PutRow(LPWORD lpBuf, int Line, int WORDs)
{
	int ByteCounts = WORDs * 2;
	PutRow((LPBYTE)lpBuf, Line, ByteCounts);
	return WORDs;
}

ErrCode CTiff::SaveFile(LPCTSTR FileName)
{
	TIFFTAG	TagStripByteCounts;
	m_File = fopen(FileName, "wb+");
	if (m_File == NULL) return FileOpenErr;
	fseek(m_File, 0, SEEK_SET);
	ErrCode ret = WriteHeader();

	if (m_lpImageBuf != NULL)
	{//Single Page! MultiPage is almost the same process below.
		//Start Page! When scan New Page you need to call again.
		StartPage();
		//Write Image Buffer! MultiPage just call Scan. When jam just call ScanFail.
		GetTag(StripByteCounts, TagStripByteCounts);
		Scan(m_lpImageBuf, TagStripByteCounts.value);
		//End File
		EndFile();
	}


	return Tiff_OK;
}

ErrCode CTiff::StartPage()
{
	ErrCode ret = WriteIFD();
	if (ret != Tiff_OK)
	{
		fclose(m_File);
		m_File = NULL;
		return ret;
	}
	return Tiff_OK;
}

ErrCode CTiff::Scan(LPBYTE lpBuf, int Count)
{
	if (m_File == NULL) return FileOpenErr;
	fwrite(lpBuf, sizeof(char), Count, m_File);
	return Tiff_OK;
}

ErrCode CTiff::ScanFail()
{//Not ready yet. 
	return Tiff_OK;
}

ErrCode CTiff::EndFile()
{//	Write NextIFD = 0;
	if (m_File == NULL) return FileOpenErr;
	fseek(m_File, IFD_Offset + 2 + IFD.EntryCounts * 12, SEEK_SET);
	DWORD NextIFD = 0;
	fwrite(&NextIFD, sizeof(NextIFD), 1, m_File);
	fclose(m_File);
	m_File = NULL;
	return Tiff_OK;
}


ErrCode CTiff::WriteHeader()
{
	WORD	TiffHeader[4];
#ifdef HiByteFirst 
	TiffHeader[0] = 0x4D4D;     // "MM", 0x4D4D
	TiffHeader[1] = 0x002A;     // 0x2A
	TiffHeader[2] = 0x0000;     // 0x2A
	TiffHeader[3] = 0x0008;     // 0x2A
#else
	TiffHeader[0] = 0x4949;     // "II", 0x4949
	TiffHeader[1] = 0x002A;     // 0x2A
	TiffHeader[2] = 0x0008;     // 0x2A
	TiffHeader[3] = 0x0000;     // 0x2A
#endif //HiByteFirst

	fwrite(TiffHeader, sizeof(WORD), 4, m_File);
	return Tiff_OK;
}

ErrCode CTiff::WriteIFD()
{
	int		i, OffsetValue, ChannelNumber, BitsPerSampleValue;
	RationalNumber xResolution, yResolution;
	TIFFTAG	TagSamplePerPixel, TagBitsPerSample, TagPhotometricInterpretation, Temp;
	WORD	Channel[10];

	IFD_Offset = IFD.NextIFD = IFD_Offset;
	OffsetValue = IFD_Offset + 2 + IFD.EntryCounts * 12 + 4;//	OffsetValue = header + sizeof(taglist) + nextIFD;

	GetTag(SamplesPerPixel, TagSamplePerPixel);
	GetTag(BitsPerSample, TagBitsPerSample);
	ChannelNumber = (int)TagSamplePerPixel.value;
	if (ChannelNumber > 1)
	{
		for (i = 0; i < ChannelNumber; i++)
			Channel[i] = (WORD)TagBitsPerSample.value;
		BitsPerSampleValue = TagBitsPerSample.value;
		TagBitsPerSample.value = OffsetValue;
		OffsetValue += 2 * (int)ChannelNumber;
		SetTag(TagBitsPerSample);
	}
	else
	{
		GetTag(PhotometricInterpretation, TagPhotometricInterpretation);
		TagPhotometricInterpretation.value = 1;
		SetTag(TagPhotometricInterpretation);
		BitsPerSampleValue = TagBitsPerSample.value;
	}

	GetTag(XResolution, Temp);
	xResolution.SetRationalNumber((WORD)Temp.value, (WORD)1);
	int ResolutionX = Temp.value;
	Temp.value = OffsetValue;
	OffsetValue += 8 * 1;
	SetTag(Temp);

	GetTag(YResolution, Temp);
	yResolution.SetRationalNumber((WORD)Temp.value, (WORD)1);
	int ResolutionY = Temp.value;
	Temp.value = OffsetValue;
	OffsetValue += 8 * 1;
	SetTag(Temp);

	//IccProfile
	GetTag(IccProfile, Temp);
	int IccFileSize = Temp.n;
	Temp.value = OffsetValue;
	OffsetValue += IccFileSize;
	SetTag(Temp);


	GetTag(StripOffsets, Temp);
	Temp.value = OffsetValue;
	SetTag(Temp);

	fseek(m_File, IFD_Offset, SEEK_SET);
	if (IFD.EntryCounts >= MAXTAG)
		return TooManyTags;
	else
		fwrite(&IFD.EntryCounts, sizeof(IFD.EntryCounts), 1, m_File);

	TAG_LIST::iterator index;
	DWORD *lpOutData = new DWORD[MAXTAG * 3 + 10];
	DWORD *lpTemp;

	if (lpOutData == NULL)
		return MemoryAllocFail;
	else
		lpTemp = lpOutData;

	for (index = IFD.m_Tags.begin(); index != IFD.m_Tags.end(); index++)
	{
		*lpTemp++ = (index->tag) | (index->type << 16);
		*lpTemp++ = index->n;
		*lpTemp++ = index->value;
	}
	fwrite(lpOutData, sizeof(DWORD), IFD.EntryCounts * 3, m_File);
	delete[]lpOutData;

	TIFFTAG TagStripByteCounts;
	GetTag(StripByteCounts, TagStripByteCounts);
	IFD.NextIFD = OffsetValue + TagStripByteCounts.value;
	fwrite(&IFD.NextIFD, sizeof(DWORD), 1, m_File);

	if (ChannelNumber > 1)
		fwrite(&Channel, sizeof(char), 2 * ChannelNumber, m_File);

	fwrite(&xResolution, sizeof(char), 8, m_File);
	fwrite(&yResolution, sizeof(char), 8, m_File);

	//Reset XResolution and YResolution
	GetTag(XResolution, Temp);
	Temp.value = ResolutionX;
	SetTag(Temp);
	GetTag(YResolution, Temp);
	Temp.value = ResolutionY;
	SetTag(Temp);

	//Set BitsPerSample to real value;
	if (ChannelNumber > 1)
	{
		TagBitsPerSample.value = BitsPerSampleValue;
		SetTag(TagBitsPerSample);
	}

	//Write Icc Profile
	TIFFTAG Icc;
	GetTag(IccProfile, Icc);
	if (Icc.n != 0)
		fwrite(m_IccProfile, 1, Icc.n, m_File);

	return Tiff_OK;
}


int CTiff::GetRowColumn(unsigned char *lpBuf, int x, int y, int RecX, int RecY)
{//Just for Gray(8 or 16 bits) and Color(8 or 16 bits).
	TIFFTAG	TagBitsPerSample;
	GetTag(BitsPerSample, TagBitsPerSample);
	if ((TagBitsPerSample.value != 8) || (TagBitsPerSample.value != 16))
		return -1;

	int i, BytesPerLine;
	LPBYTE  lpPosition;
	lpPosition = (LPBYTE)m_lpImageBuf;

	TIFFTAG	TagImageWidth, TagSamplesPerPixel;
	GetTag(ImageWidth, TagImageWidth);
	GetTag(BitsPerSample, TagBitsPerSample);
	GetTag(SamplesPerPixel, TagSamplesPerPixel);
	BytesPerLine = (int)ceil(TagImageWidth.value * TagSamplesPerPixel.value * TagBitsPerSample.value * 0.125);

	LPBYTE lpWidthBuf = new BYTE[BytesPerLine];
	LPBYTE lpCurrent = lpBuf;

	int StartX, EndX, StartY, EndY;
	if (TagBitsPerSample.value == 8)
	{
		StartX = x; EndX = x + RecX;
		StartY = y; EndY = y + RecY;
	}
	else
	{//For 16 bits
		StartX = x * 2; EndX = (x + RecX) * 2;
		StartY = y * 2; EndY = (y + RecY) * 2;
	}

	for (i = StartY; i < EndY; i++)
	{
		GetRow(lpWidthBuf, i, BytesPerLine);
		memcpy(lpCurrent, lpWidthBuf + StartX * TagSamplesPerPixel.value, (int)(RecX * TagSamplesPerPixel.value * TagBitsPerSample.value * 0.125));
		lpCurrent += (int)(RecX * TagSamplesPerPixel.value * TagBitsPerSample.value * 0.125);
	}

	delete[]lpWidthBuf;
	return 1;
}

int CTiff::GetRowColumn(unsigned short *lpBuf, int x, int y, int RecX, int RecY)
{//Just for Gray and Color.
	return GetRowColumn((unsigned char*)lpBuf, x, y, RecX, RecY);
}

unsigned char* CTiff::PutData(unsigned char *lpImageBufIndex, unsigned char *lpBuf, int ByteCounts)
{
	memcpy(lpImageBufIndex, lpBuf, ByteCounts);
	return lpImageBufIndex + ByteCounts;
}

unsigned short* CTiff::PutData(unsigned short *lpImageBufIndex, unsigned short *lpBuf, int WordCounts)
{
	memcpy(lpImageBufIndex, lpBuf, WordCounts);
	return lpImageBufIndex + WordCounts;
}

ErrCode CTiff::SetIccProfile(char *IccFile)
{
	FILE *file = fopen(IccFile, "rb");
	if (file == NULL)
		return FileOpenErr;
	int FileSize = 0;
	fread(&FileSize, 1, 4, file);
	FileSize = SwapDWORD(FileSize);
	if (m_IccProfile != NULL)
		delete[]m_IccProfile;
	m_IccProfile = new BYTE[FileSize];
	fseek(file, 0, SEEK_SET);
	fread(m_IccProfile, 1, FileSize, file);
	fclose(file);

	TIFFTAG Icc;
	GetTag(IccProfile, Icc);
	Icc.n = FileSize;
	SetTag(Icc);

	return Tiff_OK;
}

void CTiff::SaveIccProfile(char *OutIccFile)
{
	TIFFTAG TempTag;
	GetTag(IccProfile, TempTag);
	if (TempTag.n != 0)
	{
		FILE *file = fopen(OutIccFile, "wb+");
		if (file != NULL)
		{
			if (m_IccProfile != NULL)
				fwrite(m_IccProfile, 1, TempTag.n, file);
			fclose(file);
		}
	}
}

void CTiff::RemoveIcc()
{
	TIFFTAG TempTag;
	GetTag(IccProfile, TempTag);
	if (TempTag.n != 0)
	{
		if (m_IccProfile != NULL)
			delete[]m_IccProfile;

		m_IccProfile = NULL;
		TempTag.n = 0;
		TempTag.value = 0;
		SetTag(TempTag);
	}
}
