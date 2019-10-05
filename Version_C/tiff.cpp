////////////////////////////////////////////////////////////////////
// Tiff.cpp: implementation of the CTiff class.
//
/////////////////////////////////////////////////////////////////////
#include "stdafx.h"

#include "Tiff.h"
#include <math.h>
using namespace AV_Tiff;
/////////////////////////////////////////////////////////////////////
// Helpful Class and Function
/////////////////////////////////////////////////////////////////////
TIFFTAG::TIFFTAG()
{
	tag = Null;
	type = Unknown;
	n = 0;
	value = 0;
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

//////////////////////////////////////////////////////////////////////
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

TIFFTAG2::TIFFTAG2()
{
	tag = (WORD)0;
	type = (WORD)0;
	n = 0;
	value = 0;
}

TIFFTAG2::~TIFFTAG2()
{
}

const	TIFFTAG2& TIFFTAG2::operator = (const TIFFTAG& temp)
{
	tag = (WORD)temp.tag;
	type = (WORD)temp.type;
	n = temp.n;
#ifndef  Win32	
	if (n == 1)
	{
		switch (temp.type)
		{
		case Byte:value = temp.value << 24; break;
		case Short:value = temp.value << 16; break;
		default:value = temp.value; break;
		}
	}
	else
		value = temp.value;
#else
	value = temp.value;
#endif //Win32

	return *this;
}

int	TIFFTAG2::TransTag(TIFFTAG& NewTag)
{
	NewTag.tag = (TiffTagSignature)tag;
	NewTag.type = (FieldType)type;
	NewTag.n = n;
#ifndef Win32	
	if (n == 1)
	{
		switch (NewTag.type)
		{
		case Byte:NewTag.value = value >> 24; break;
		case Short:NewTag.value = value >> 16; break;
		default:NewTag.value = value; break;
		}
	}
	else
		NewTag.value = value;
#else
	NewTag.value = value;
#endif //Win32

	return 1;
}

////////////////////////////////////////////////////////////////////j
// Tiff.cpp: implementation of the CTiff class.
//
/////////////////////////////////////////////////////////////////////
CTiff::CTiff()
{
	m_lpTiffHeader = new(TIFFHEADER);
	Initialheader();
	m_lpImageBuf = NULL;
	m_IccProfile = NULL;
}

CTiff::~CTiff()
{
	delete[]m_lpTiffHeader;
	if (m_lpImageBuf != NULL)
		delete[]m_lpImageBuf;
	if (m_IccProfile != NULL)
		delete[]m_IccProfile;
}


int CTiff::Initialheader()
{
#ifdef Win32
	m_lpTiffHeader->Version.order = 0x4949;
#else
	m_lpTiffHeader->Version.order = 0x4D4D;
#endif //Win32

	m_lpTiffHeader->Version.version = 0x2A;
	m_lpTiffHeader->Version.offset = 8;

	m_lpTiffHeader->IFD.TagList[0].tag = NewSubfileType;
	m_lpTiffHeader->IFD.TagList[0].type = Byte;
	m_lpTiffHeader->IFD.TagList[0].n = 4;
	m_lpTiffHeader->IFD.TagList[0].value = 0;

	m_lpTiffHeader->IFD.TagList[1].tag = ImageWidth;
	m_lpTiffHeader->IFD.TagList[1].type = Long; //or Short
	m_lpTiffHeader->IFD.TagList[1].n = 1;
	m_lpTiffHeader->IFD.TagList[1].value = 0;

	m_lpTiffHeader->IFD.TagList[2].tag = ImageLength;
	m_lpTiffHeader->IFD.TagList[2].type = Long; //or Short
	m_lpTiffHeader->IFD.TagList[2].n = 1;
	m_lpTiffHeader->IFD.TagList[2].value = 0;

	m_lpTiffHeader->IFD.TagList[3].tag = BitsPerSample;
	m_lpTiffHeader->IFD.TagList[3].type = Short;
	m_lpTiffHeader->IFD.TagList[3].n = 1;
	m_lpTiffHeader->IFD.TagList[3].value = 0;
	//entryCount * sizeof(TIFFTAG);//8 8 8
	// You need to caculate the offset later.

	m_lpTiffHeader->IFD.TagList[4].tag = Compression;
	m_lpTiffHeader->IFD.TagList[4].type = Short;
	m_lpTiffHeader->IFD.TagList[4].n = 1;
	m_lpTiffHeader->IFD.TagList[4].value = 1;

	m_lpTiffHeader->IFD.TagList[5].tag = PhotometricInterpretation;
	m_lpTiffHeader->IFD.TagList[5].type = Short;
	m_lpTiffHeader->IFD.TagList[5].n = 1;
	m_lpTiffHeader->IFD.TagList[5].value = 2;
	//0:White is zero, 1:Black is Zero, 2:RGB color, 3:Palette color, 6:YCbCr

	m_lpTiffHeader->IFD.TagList[6].tag = StripOffsets;
	m_lpTiffHeader->IFD.TagList[6].type = Short;
	m_lpTiffHeader->IFD.TagList[6].n = 1;
	m_lpTiffHeader->IFD.TagList[6].value = 0;
	//entryCount * sizeof(TIFFTAG) + 3 * sizeof(XYRESOLUTION);

	m_lpTiffHeader->IFD.TagList[7].tag = SamplesPerPixel;
	m_lpTiffHeader->IFD.TagList[7].type = Short;
	m_lpTiffHeader->IFD.TagList[7].n = 1;
	m_lpTiffHeader->IFD.TagList[7].value = 3;

	m_lpTiffHeader->IFD.TagList[8].tag = RowsPerStrip;
	m_lpTiffHeader->IFD.TagList[8].type = Long;
	m_lpTiffHeader->IFD.TagList[8].n = 1;
	m_lpTiffHeader->IFD.TagList[8].value = 0;

	m_lpTiffHeader->IFD.TagList[9].tag = StripByteCounts;
	m_lpTiffHeader->IFD.TagList[9].type = Long;
	m_lpTiffHeader->IFD.TagList[9].n = 1;
	m_lpTiffHeader->IFD.TagList[9].value = 0;

	m_lpTiffHeader->IFD.TagList[10].tag = XResolution;
	m_lpTiffHeader->IFD.TagList[10].type = Rational;
	m_lpTiffHeader->IFD.TagList[10].n = 1;
	m_lpTiffHeader->IFD.TagList[10].value = 0;

	m_lpTiffHeader->IFD.TagList[11].tag = YResolution;
	m_lpTiffHeader->IFD.TagList[11].type = Rational;
	m_lpTiffHeader->IFD.TagList[11].n = 1;
	m_lpTiffHeader->IFD.TagList[11].value = 0;

	m_lpTiffHeader->IFD.TagList[12].tag = ResolutionUnit;
	m_lpTiffHeader->IFD.TagList[12].type = Short;
	m_lpTiffHeader->IFD.TagList[12].n = 1;
	m_lpTiffHeader->IFD.TagList[12].value = 2;
	//2:inch, 3:centimeter

	m_lpTiffHeader->IFD.TagList[13].tag = PlanarConfiguration;
	m_lpTiffHeader->IFD.TagList[13].type = Short;
	m_lpTiffHeader->IFD.TagList[13].n = 1;
	m_lpTiffHeader->IFD.TagList[13].value = 1;
	//1: Data is stored as RGBRGB..., 2:RRRR....,GGGG...., BBBB....


	m_lpTiffHeader->IFD.TagList[14].tag = IccProfile;
	m_lpTiffHeader->IFD.TagList[14].type = UndefineType;
	m_lpTiffHeader->IFD.TagList[14].n = 0;
	m_lpTiffHeader->IFD.TagList[14].value = 0;

	m_lpTiffHeader->IFD.EntryCounts = 15;

	m_lpTiffHeader->IFD.NextIFD = 0; // for only one IFD
//	m_TiffHeaderParam.nextIFD = 8; // It point to the first IFD_Position. For multipage

	return 1;
}


void CTiff::GetTag(TiffTagSignature Signature, TIFFTAG &TiffTag)
{
	//Reset TiffTag.
	TiffTag.tag = Null;
	TiffTag.type = Unknown;
	TiffTag.n = 0;
	TiffTag.value = 0;

	int TotalTag = m_lpTiffHeader->IFD.EntryCounts;
	for (int TagCount = 0; TagCount < TotalTag; TagCount++)
	{
		if (m_lpTiffHeader->IFD.TagList[TagCount].tag == Signature)
		{
			TiffTag = m_lpTiffHeader->IFD.TagList[TagCount];
			break;
		}
	}
}

int CTiff::SetTag(TIFFTAG NewTag)
{
	int TotalTag = m_lpTiffHeader->IFD.EntryCounts;
	for (int TagCount = 0; TagCount < TotalTag; TagCount++)
	{
		if (m_lpTiffHeader->IFD.TagList[TagCount].tag == NewTag.tag)
		{
			m_lpTiffHeader->IFD.TagList[TagCount] = NewTag;
			return 1;
			break;
		}
	}

	if (TagCount < MAXTAG) //The maximal size of TagList
	{//Add New Tag
		m_lpTiffHeader->IFD.TagList[TotalTag] = NewTag;
		m_lpTiffHeader->IFD.EntryCounts++;
		return 1;
	}
	else
		return -1;//The TagList is not enough,
}

int CTiff::CreateNew(int Width, int Length, int Resolution, int sampleperpixel, int bitspersample, int ColorOrder)
{//ColorOrder:Not use anymore, just for old source code which using CTiff.
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

	//Caculate StripByteCounts value.
	TIFFTAG Temp;
	Temp.tag = StripByteCounts;
	Temp.type = Long;
	Temp.n = 1;
	switch (bitspersample)
	{
	case  1:
		Temp.value = (DWORD)(ceil(Width * 0.125) * Length * sampleperpixel);
		break;
	case  8:
		Temp.value = (DWORD)(Width * Length * sampleperpixel);
		break;
	case  16:
		Temp.value = (DWORD)(Width * Length * sampleperpixel * 2);
		break;
	default:
		Temp.value = (DWORD)(Width * Length * sampleperpixel);
		break;
	}
	SetTag(Temp);

	SetTag(TIFFTAG(XResolution, Rational, 1, Resolution));
	//	xResolution.SetRationalNumber(Resolution, 1);

	SetTag(TIFFTAG(YResolution, Rational, 1, Resolution));
	//	yResolution.SetRationalNumber(Resolution, 1);

	SetTag(TIFFTAG(ResolutionUnit, Short, 1, 2));
	//2:inch, 3:centimeter

	SetTag(TIFFTAG(PlanarConfiguration, Short, 1, 1));
	//1: Data is stored as RGBRGB..., 2:RRRR....,GGGG...., BBBB....

	//IccProfile
	SetTag(TIFFTAG(IccProfile, UndefineType, 0, 0));

	if (m_lpImageBuf != NULL)
		delete[]m_lpImageBuf;

	GetTag(StripByteCounts, Temp);
	m_lpImageBuf = new BYTE[Temp.value];

	return 1;
}

int CTiff::SaveFile(LPCTSTR FileName)
{
	TIFFTAG	TagStripByteCounts;
	m_FileName = FileName;
	m_File = fopen(FileName, "wb+");
	fseek(m_File, 0, SEEK_SET);
	WriteHeader();

	GetTag(StripByteCounts, TagStripByteCounts);
	fwrite(m_lpImageBuf, 1, TagStripByteCounts.value, m_File);
	fclose(m_File);
	return 1;
}

int CTiff::WriteHeader()
{
	int	OffsetValue, ChannelNumber, BitsPerSampleValue;
	RationalNumber xResolution, yResolution;
	TIFFTAG	TagSamplePerPixel, TagBitsPerSample, TagPhotometricInterpretation, Temp;
	TIFFTAG2	TagData;
	WORD	Channel[10];

	OffsetValue = 10 + m_lpTiffHeader->IFD.EntryCounts * 12 + 4;
	//	OffsetValue = header + sizeof(taglist) + nextIFD;

	GetTag(SamplesPerPixel, TagSamplePerPixel);
	GetTag(BitsPerSample, TagBitsPerSample);
	ChannelNumber = (int)TagSamplePerPixel.value;
	if (ChannelNumber > 1)
	{
		for (int i = 0; i < ChannelNumber; i++)
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
	Temp.value = OffsetValue;
	OffsetValue += 8 * 1;
	SetTag(Temp);

	GetTag(YResolution, Temp);
	yResolution.SetRationalNumber((WORD)Temp.value, (WORD)1);
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

	fwrite(&m_lpTiffHeader->Version, 1, 8, m_File);
	fwrite(&m_lpTiffHeader->IFD.EntryCounts, 1, 2, m_File);

	int TagCount = m_lpTiffHeader->IFD.EntryCounts;
	for (int i = 0; i < TagCount; i++)
	{
		TagData = m_lpTiffHeader->IFD.TagList[i];
		fwrite(&TagData, 1, sizeof(TIFFTAG2), m_File);
	}
	fwrite(&m_lpTiffHeader->IFD.NextIFD, 1, 4, m_File);

	if (ChannelNumber > 1)
		fwrite(&Channel, 1, 2 * ChannelNumber, m_File);

	fwrite(&xResolution, 1, 8, m_File);
	fwrite(&yResolution, 1, 8, m_File);

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

	return 1;
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

int CTiff::ReadFile(LPCTSTR FileName)
{
	TIFFTAG2		TempTag2;
	TIFFTAG			TempTag;
	DWORD			TiffVersion, TiffOffset;
	WORD			TagCount, BitsPerSampleValue;
	int				i;
	RationalNumber  Resolution;

	m_FileName = FileName;
	m_File = fopen(FileName, "rb");
	fseek(m_File, 0, SEEK_SET);
	fread(&TiffVersion, 1, 4, m_File);
#ifdef Win32	
	if (TiffVersion != 0x002A4949)
	{
		return -1;
	}
#else	
	if (TiffVersion != 0x4D4D002A)
	{//Mac
		return -1;
	}
#endif //Win32

	fread(&TiffOffset, 1, 4, m_File);
	fseek(m_File, TiffOffset, SEEK_SET);
	fread(&TagCount, 1, 2, m_File);
	for (i = 0; i < TagCount; i++)
	{
		fread(&TempTag2, 1, 12, m_File);
		TempTag2.TransTag(TempTag);
		if (CheckingTag(TempTag.tag))
			SetTag(TempTag);
	}

	//Read BitsPerSample data
	fread(&m_lpTiffHeader->IFD.NextIFD, 1, 4, m_File);
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
		int ret = ferror(m_File);
	}
	else
	{
		TIFFTAG	TagStripByteCounts;
		GetTag(StripByteCounts, TagStripByteCounts);
		if (TagStripByteCounts.n != TempTag.n) return -1;

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
		//Don't worry about the StripOffset.value, It will be recount in writefile().
		SetTag(TempTag);

		delete lpStripOffsets;
		delete lpStripByteCounts;
	}
	fclose(m_File);
	return 1;
}


int CTiff::GetImage()
{
	m_File = fopen(m_FileName, "rb+");
	fseek(m_File, 0, SEEK_SET);

	TIFFTAG	TempTag;
	GetTag(StripOffsets, TempTag);
	fseek(m_File, TempTag.value, SEEK_SET);

	//Just for one Strip.
	GetTag(StripByteCounts, TempTag);
	fread(m_lpImageBuf, 1, TempTag.value, m_File);
	fclose(m_File);

	return TempTag.value;
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
	case ResolutionUnit:return	Signature; break;

	case Null:
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
	case ReferenceBlackWhite:
	case CopyRight:
	default: return Null; break;
	}
}

#ifndef Memory_Check
/*************************************************************************************************
eXtral  Utral dangerous, No Memory check.
Joint? Yes!  If You known what you doing, and you sure memory is totaly under control.
Not check yet.
*************************************************************************************************/
unsigned char* CTiff::PutData(unsigned char *lpImageBufIndex, unsigned char *lpBuf, int ByteCounts)
{
	memcpy(lpImageBufIndex, lpBuf, ByteCounts);
	return lpImageBufIndex + ByteCounts;
}

unsigned short* CTiff::PutData(unsigned short *lpImageBufIndex, unsigned short *lpBuf, int WordCounts)
{
	wmemcpy(lpImageBufIndex, lpBuf, WordCounts);
	return lpImageBufIndex + WordCounts;
}
#else
unsigned char* CTiff::PutData(unsigned char *lpImageBufIndex, unsigned char *lpBuf, int ByteCounts)
{
	unsigned char *l
		if (lpImageBufIndex == NULL)
		{

		}
	memcpy(lpImageBufIndex, lpBuf, ByteCounts);
	return lpImageBufIndex + ByteCounts;
}

unsigned short* CTiff::PutData(unsigned short *lpImageBufIndex, unsigned short *lpBuf, int WordCounts)
{
	wmemcpy(lpImageBufIndex, lpBuf, WordCounts);
	return lpImageBufIndex + WordCounts;
}
#endif //Memory_Check

//Icc Profile Option.
int CTiff::SetIccProfile(char *IccFile)
{
	FILE *file = fopen(IccFile, "rb");
	if (file == NULL)
		return -1;
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

	return 1;
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