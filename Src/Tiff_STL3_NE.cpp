/////////////////////////////////////////////////////////////////////x
// Tiff_STL.cpp: implementation of the CTiff class.
// 
//////////////////////////////////////////////////////////////////////
//********************************************************************
//	You can get the sample code in "Tiff_STL3.h"
//********************************************************************
#include "stdafx.h"
#include <iostream>
#include "Tiff_STL3.h"
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
#include <iostream>
#include <string>

using namespace std;

//////////////////////////////////////////////////////////////////////
// Helpful Class and Function
//////////////////////////////////////////////////////////////////////
//#define _WINDOWS

namespace AV_Tiff_STL3 {
	int DataType[FieldTypeSize] = {
		1,//Unknown						= 0x0000L,
		1,//Byte						= 0x0001L,
		1,//ASCII						= 0x0002L,
		2,//Short						= 0x0003L,
		4,//Long						= 0x0004L,
		8,//Rational					= 0x0005L,
		1,//SBYTE						= 0x0006L,
		1,//UndefineType				= 0x0007L,
		2,//SShort						= 0x0008L,
		4,//SLong						= 0x0009L,
		4,//Float						= 0x000AL,
		8,//Double						= 0x000BL,
		1,//Unknown						= 0x000CL,//Never using, Just for code analysis warning.
		1,//Unknown						= 0x000DL,
		1,//Unknown						= 0x000EL,
		1,//Unknown						= 0x000FL,
	};
};//AV_Tiff_STL3 or AV_Tiff.

//////////////////////////////////////////////////////////////////////
// Class TiffTag
//////////////////////////////////////////////////////////////////////
TiffTag::TiffTag() :tag(NullTag), lpData(nullptr), type(Unknown), n(0), value(0)
{}

TiffTag::~TiffTag()
{
	tag = NullTag;
	type = Unknown;
	n = 0;
	value = 0;

	if (lpData != nullptr)
		delete[]lpData;
	lpData = nullptr;

}

//Copy Construct
TiffTag::TiffTag(const TiffTag & Tag) :tag(Tag.tag), type(Tag.type), n(Tag.n)
{
	lpData = nullptr;
	int DataSize = DataType[type] * this->n;
	if (DataSize > 4)
	{
		lpData = new BYTE[DataSize];
		memcpy(lpData, Tag.lpData, DataSize);
	}
}

//move constructor
TiffTag::TiffTag(TiffTag&& Tag) noexcept :tag(Tag.tag), type(Tag.type), n(Tag.n), value(Tag.value), lpData(Tag.lpData)
{
	Tag.lpData = nullptr;
}

// copy assignment
TiffTag& TiffTag::operator=(const TiffTag& Tag)
{
	if (this != &Tag)
	{
		tag = Tag.tag;
		type = Tag.type;
		n = Tag.n;

		if (lpData != nullptr)
			delete[]lpData;

		lpData = nullptr;
		int DataSize = DataType[type] * this->n;
		if (DataSize > 4)
		{
			lpData = new BYTE[DataSize];
			memcpy(lpData, Tag.lpData, DataSize);
		}
	}
	return *this;
}

//Move assignment
TiffTag& TiffTag::operator=(TiffTag&& Tag) noexcept // move assignment
{
	if (this != &Tag)
	{
		tag = Tag.tag;
		type = Tag.type;
		n = Tag.n;

		if (lpData != nullptr)
			delete[]lpData;

		lpData = Tag.lpData;
		Tag.lpData = nullptr;
	}
	return *this;
}

TiffTag::TiffTag(TiffTagSignature Signature)
{
	tag = Signature;
	lpData = nullptr;
	type = Unknown;
	n = 0;
	value = 0;
}

TiffTag::TiffTag(TiffTagSignature Tag, FieldType Type, DWORD n, DWORD value, LPBYTE lpBuf)
{
	tag = Tag;
	type = Type;
	this->n = n;
	this->value = value;
	lpData = lpBuf;
}

TiffTag::TiffTag(DWORD SigType, DWORD n, DWORD value, IO_INTERFACE *IO)
{
	tag = (TiffTagSignature)(0xFFFF & SigType); //We not sure the DWORD is 32 bits.
	type = (FieldType)(0XF & (SigType >> 16));
	this->n = n;
	this->value = value;

	int DataSize = DataType[type] * this->n;
	if (DataSize > 4)
	{//memory address
		if (IO != nullptr)
		{
			lpData = new BYTE[DataSize];
			IO_Seek(value, SEEK_SET);
			IO_Read(lpData, DataType[type], n);
		}
		else
			//Not from file, Setting directly, 
			//for example setting BitsPerSample, It will set up the data in the constructor.
			lpData = nullptr;
	}
	else
		lpData = nullptr;
}

DWORD TiffTag::GetValue() const
{
	DWORD DataSize = DataType[this->type] * this->n;
	if (DataSize <= 4)
		return value;
	else
		return (*(LPWORD)lpData);
}

int TiffTag::SaveFile(IO_INTERFACE *IO)
{
	int DataSize = DataType[type] * n;
	int ret = 0;
	if (DataSize > 4)
		ret = (int)IO_Write(lpData, DataType[type], n);
	return ret;
}

int TiffTag::ValueIsOffset() const
{
	int DataSize = DataType[type] * n;
	int ret = 0;
	if (DataSize > 4)
		ret = 1;
	return ret;
}

/*****************************************************************************************
//Special Tags
*****************************************************************************************/
//BitsPerSample
BitsPerSampleTag::BitsPerSampleTag(DWORD SigType, DWORD n, DWORD value, IO_INTERFACE *IO)
	:TiffTag(SigType, n, value, IO)
{
	if (IO == nullptr)
	{//For CreateNew File, SetTag(BitsPerSampleTag)
		if (lpData != nullptr)
			delete lpData;

		if (n != 1)
		{
			LPWORD lpTemp = new WORD[n];
			lpData = (LPBYTE)lpTemp;
			for (DWORD i = 0; i < n; ++i)
				*(lpTemp++) = (WORD)value;
		}
		else
		{
			this->value = value;
			this->lpData = nullptr;
		}
	}
}

DWORD BitsPerSampleTag::GetValue() const
{
	if (this->n == 1)
		return this->value;
	else
		if (lpData != nullptr)
			return (DWORD)(*(LPWORD)lpData);
		else
			return 0;
}

//Resolution
ResolutionTag::ResolutionTag(DWORD SigType, DWORD n, DWORD value, IO_INTERFACE *IO)
	:TiffTag(SigType, n, value, IO)
{
	if (IO == nullptr)
	{
		LPDWORD lpTemp = new DWORD[2];
		lpTemp[0] = (DWORD)(value * 100);
		lpTemp[1] = 100;
		lpData = (LPBYTE)lpTemp;
	}
}

DWORD ResolutionTag::GetValue() const
{
	return (DWORD)(*(LPDWORD)lpData / *(LPDWORD)(lpData + 4));
}

/*
* Photoshop Tag, Still Unknown...
* Don't save anything. This tag broke all rule, just skip it.
* If you want to save this tag.
* You may need to pay more expense.
* Sometimes I wonder that it is worth to do such thing.
*/

//const int Exif_IFD_Size = 44; //photoshop 7
//const int Exif_IFD_Size = 80; //photoshop cs3 
const int Exif_IFD_Size = 256;  //xujy : write 0 is ok.
//Exif_IFD_Tag::Exif_IFD_Tag(DWORD SigType, DWORD n, DWORD value, IO_INTERFACE *IO)
Exif_IFD_Tag::Exif_IFD_Tag(DWORD SigType, DWORD n, DWORD value, IO_INTERFACE *IO)
	:TiffTag(SigType, n, value, IO)
{//Actually, we know nothing about this tag.

	lpData = new BYTE[Exif_IFD_Size];
	memset(lpData, 0, Exif_IFD_Size);
	IO_Seek(value, SEEK_SET);
	IO_Read(lpData, 1, Exif_IFD_Size);

	//Special condition, We need to change the value of the this->n.
	//this->n = Exif_IFD_Size / DataType[type];	
}

//////////////////////////////////////////////////////////////////////
// Tiff
//////////////////////////////////////////////////////////////////////
Tiff::Tiff()
{
	m_IFD.NextIFD = 8;
	//IO = nullptr;
}

Tiff::Tiff(LPCSTR FileName)
{
	ReadFile(FileName);
}

Tiff::~Tiff()
{
	Reset();
}

void Tiff::Reset()
{
#if (!SMART_POINTER)
	//for_each(TiffTag_Begin, TiffTag_End, [](TiffTag* pos) {delete pos; });
	for(const auto& pos: m_IFD.m_TagList)
		delete pos; 
#endif //SMART_POINTER

	m_IFD.m_TagList.clear();
}

//////////////////////////////////////////////////////////////////////
// Tiff Read File Operaton
//////////////////////////////////////////////////////////////////////
TiffTagPtr Tiff::GetTag(const TiffTagSignature Signature)
{
	auto pos = find_if(TiffTag_Begin, TiffTag_End,
		[&Signature](TiffTagPtr pos) {return pos->tag == Signature; });

	if (pos != TiffTag_End)
		return *pos;
	else
		return 0;
}

DWORD Tiff::GetTagValue(const TiffTagSignature Signature)
{
	TiffTagPtr Tag = GetTag(Signature);
	if (Tag != nullptr)
		return Tag->GetValue();
	else
		return 0;
}

ErrCode Tiff::SetTag(TiffTagPtr NewTag)
{
	if (NewTag == nullptr)
		return Tiff_OK;//If the tag is unknown, just forget it!

	TiffTagSignature sig = NewTag->tag;

	auto pos = find_if(TiffTag_Begin, TiffTag_End,
		[&sig](TiffTagPtr pos) {return pos->tag == sig; });

	if (pos == TiffTag_End)
	{
		m_IFD.m_TagList.push_back(NewTag);
		m_IFD.m_TagList.size();//for what??? I forget it.
	}
	else
	{//Replace
		if (*pos != NewTag)
		{
#if (!SMART_POINTER)
			delete *pos;
#endif //SMART_POINTER

			*pos = NewTag;
		}
	}
	return Tiff_OK;
}

ErrCode Tiff::SetTagValue(const TiffTagSignature Signature, DWORD Value)
{
	TiffTagPtr lpTag = GetTag(Signature);
	if (lpTag != nullptr)
		lpTag->value = Value;
	return SetTag(lpTag);
}

//////////////////////////////////////////////////////////////////////
// Tiff Read File Operaton
//////////////////////////////////////////////////////////////////////
ErrCode Tiff::CheckFile(IO_INTERFACE *IO)
{
#if defined(VIRTUAL_IO_STL)
	fstream *file = reinterpret_cast<fstream*>(dynamic_cast<IO_fstream*>(IO)->GetHandle());
	if (file->is_open() == false)
		return FileOpenErr;

#elif defined(VIRTUAL_IO)
	if (IO->GetHandle() == nullptr)
		return FileOpenErr;

#else //C version.
	if (IO == nullptr)
		return FileOpenErr;

#endif //VIRTUAL_IO_STL
	return Tiff_OK;
}

ErrCode Tiff::ReadFile(LPCSTR FileName)
{
	Reset();

	//for linux old compiler -fno-exceptions, 
	// __cxa_throw_bad_array_new_length@@CXXABI_1.3.8
	if (FileName == nullptr)
	{
		cout << " *** Tiff::ReadFile() --> FileName is nullptr. *** " << endl;
		return FileOpenErr;
	}

	IO_INTERFACE *IO = IO_In(FileName);

	if (CheckFile(IO) != Tiff_OK)
		return FileOpenErr;

	ErrCode ret = ReadTiff(IO);
	if (IO != NULL)
		IO_Close(IO);

	return ret;
}

ErrCode Tiff::ReadTiff(IO_INTERFACE *IO)
{
	DWORD	TiffVersion;
	WORD	TagCount;
	ErrCode ret = Tiff_OK;

	IO_Seek(0, SEEK_SET);
	IO_Read((LPBYTE)&TiffVersion, 1, 4);

	if (TiffVersion != 0x002A4949)
	{//PC Version
		ret = VersionErr;
		IO_Close(IO);
		cout << " *** Tiff is not PC version. *** " << endl;
		return VersionErr;
	}

	IO_Read((LPBYTE)&m_IFD_Offset, 1, 4);
	IO_Seek(m_IFD_Offset, SEEK_SET);
	IO_Read((LPBYTE)&TagCount, 1, 2);
	if (TagCount >= MAXTAG)
	{//Too many m_Tags
		ret = TooManyTags;
		IO_Close(IO);
		return ret;
	}

	DWORD Temp[MAXTAG * 3];
	IO_Read((LPBYTE)Temp, TagCount * 3, 4);

	//Read IFD Data.
	for (int i = 0; i < TagCount * 3; i += 3)
		AddTags(Temp[i], Temp[i + 1], Temp[i + 2], IO);

	ret = ReadImage(IO);

	return ret;
}

ErrCode Tiff::SaveFile(LPCSTR FileName)
{
	ErrCode ret = Tiff_OK;

	IO_INTERFACE *IO = IO_Out(FileName);
	if (IO == nullptr)
	{
		cout << "Save Fiel, open file error." << endl;
		return FileOpenErr;
	}

	if (m_IFD.m_TagList.size() == 0)
	{
		IO_Close(IO);
		cout << "*** Tiff::SaveFile() --> TiffTag EntryCounts is 0. ***" << endl;
		return FileOpenErr;
	}

	ret = SaveTiff(IO);
	IO_Close(IO);
	return ret;
}

ErrCode Tiff::SaveTiff(IO_INTERFACE *IO)
{
	ErrCode ret = WriteHeader(IO);
	ret = WriteIFD(IO);
	ret = WriteTagData(IO);
	ret = WriteImageData(IO);
	ret = WriteData_Exif_IFD_Tag(IO);
	return ret;
}

ErrCode Tiff::SaveRaw(LPCSTR FileName)
{
	ErrCode ret = Tiff_OK;

	IO_INTERFACE *IO = IO_Out(FileName);
	if (IO == nullptr)
	{
		return FileOpenErr;
		//throw "*** File(Save) Open Error. ***";
	}
	if (m_IFD.m_TagList.size() == 0)
	{
		IO_Close(IO);
		//throw "*** Tiff::SaveFile() --> TiffTag EntryCounts is 0. ***";
	}

	int Width = GetTagValue(ImageWidth);
	int Length = GetTagValue(ImageLength);
	int Samples = GetTagValue(SamplesPerPixel);
	int Bits = GetTagValue(BitsPerSample);
	cout << endl << "Width:" << Width << " Length:" << Length << " Samples:" << Samples << " Bits:" << Bits << endl;
	ret = WriteImageData(IO);
	IO_Close(IO);
	return ret;
}

TiffTagPtr Tiff::CreateTag(DWORD SigType, DWORD n, DWORD value, IO_INTERFACE *IO)
{
	TiffTag *NewTag = nullptr;

	switch ((TiffTagSignature)(0xFFFF & SigType))
	{//Default Tag
	case NewSubfileType:
	case SubfileType:
	case ImageWidth:
	case ImageLength:
	case Compression:
	case PhotometricInterpretation:
	case RowsPerStrip:
	case StripByteCounts:
	case PlanarConfiguration:
	case ResolutionUnit:
	case Threshholding:
	case Orientation:
	case SamplesPerPixel:
	case MinSampleValue:
	case MaxSampleValue:
	case CellWidth:
	case CellLength:
	case FillOrder:
	case DocumentName:
	case ImageDescription:
	case Make:
	case Model:
	case StripOffsets:
	case PageName:
	case XPosition:
	case YPosition:
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
	case TiffWhitePoint:
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
	case XML_Data:
	case CopyRight:
	case IPTC:
	case Photoshop:
	case IccProfile:
		NewTag = new TiffTag(SigType, n, value, IO);
		break;

		//	Special Tag
	case BitsPerSample:
		NewTag = new BitsPerSampleTag(SigType, n, value, IO);
		break;

	case XResolution:
	case YResolution:
		NewTag = new ResolutionTag(SigType, n, value, IO);
		break;

		//Just Skip this tag.
	case Exif_IFD:
		/*
		* Photoshop Tag, Still Unknown...
		* Don't save anything. This tag broke all rule, just skip it.
		* If you want to save this tag.
		* You may need to pay more expense.
		* Sometimes I wonder that it is worth to do such thing.
		*/
		NewTag = new Exif_IFD_Tag(SigType, n, value, IO);
		break;

	default:
		NewTag = new TiffTag(SigType, n, value, IO);
		break;
	}

	return (TiffTagPtr)NewTag;
}

void Tiff::AddTags(DWORD TypeSignature, DWORD n, DWORD value, IO_INTERFACE *IO)
{
	TiffTagPtr tag = CreateTag(TypeSignature, n, value, IO);
	if (tag != nullptr)
		m_IFD.m_TagList.push_back(tag);
}

ErrCode Tiff::ReadImage(IO_INTERFACE *IO)
{
	if (GetTagValue(Compression) != 1)//No compress
		return CompressData;

	DWORD Width = GetTagValue(ImageWidth);
	DWORD Length = GetTagValue(ImageLength);
	DWORD rowsPerStrip = GetTagValue(RowsPerStrip);
	DWORD stripOffsets = GetTagValue(StripOffsets);
	DWORD planarConfiguration = GetTagValue(PlanarConfiguration);
	DWORD bitsPerSample = GetTagValue(BitsPerSample);
	DWORD samplesPerPixel = GetTagValue(SamplesPerPixel);
	LPBYTE lpImageBuf = nullptr;

	if (rowsPerStrip == 0)
	{
		rowsPerStrip = Length;
		TiffTagPtr NewTag = SMART_PTR(TiffTag, new TiffTag(RowsPerStrip, Short, 1, Length));
		m_IFD.m_TagList.push_back(NewTag);
	}

	if (Length == rowsPerStrip)
	{//Single Strip
		if ((planarConfiguration == 0) || (planarConfiguration == 1))
		{//RGBRGB
			DWORD stripByteCounts = GetTagValue(StripByteCounts);
#if 1//Liao's Bug, data Error.
			int ImgSize = (Width * Length * samplesPerPixel * bitsPerSample) >> 3;
			if (ImgSize != (int)stripByteCounts)
			{
				stripByteCounts = ImgSize;
				SetTagValue(StripByteCounts, stripByteCounts);
			}
#endif //Check StripByteCounts. 

			lpImageBuf = new BYTE[stripByteCounts];
			IO_Seek(stripOffsets, SEEK_SET);
			IO_Read(lpImageBuf, 1, stripByteCounts);
			//Set ImageBuf address to StripOffset Tag
			TiffTagPtr TempTag = GetTag(StripOffsets);
			TempTag->lpData = lpImageBuf;
		}
		else
		{//PlanarConfiguration == 2, 
			if (bitsPerSample == 1)
			{//Undocument, just for avision, CMYKcm(4 or 6) 1bit.
				DWORD stripByteCounts = (Width >> 3) * Length * samplesPerPixel;
				lpImageBuf = new BYTE[stripByteCounts];
				IO_Seek(stripOffsets, SEEK_SET);
				IO_Read(lpImageBuf, 1, stripByteCounts);
				//Set ImageBuf address to StripOffset Tag
				TiffTagPtr TempTag = GetTag(StripOffsets);
				TempTag->lpData = lpImageBuf;
			}
			else
			{//RRRGGGBBB, For PhotoShop CS2
				ReadMultiStripOffset(IO);

				if (bitsPerSample == 8)
					Pack<BYTE>(Width, Length);
				else
					Pack<WORD>(Width, Length);

				//Reset PlanarConfiguration -> 1.
				SetTagValue(PlanarConfiguration, 1);
			}
		}
	}
	else
		ReadMultiStripOffset(IO);

	return Tiff_OK;
}

ErrCode Tiff::ReadMultiStripOffset(IO_INTERFACE *IO)
{
	TiffTagPtr TagStripOffsets = GetTag(StripOffsets);
	TiffTagPtr TagStripByteCounts = GetTag(StripByteCounts);
	DWORD stripByteCounts = 0;
	DWORD strip = TagStripOffsets->n;
	LPDWORD lpTemp = (LPDWORD)TagStripByteCounts->lpData;
	for (DWORD i = 0; i < strip; ++i)
		stripByteCounts += *(lpTemp++);

	LPBYTE lpImageBuf = new BYTE[stripByteCounts];
	LPBYTE lpImageBufTemp = lpImageBuf;
	LPDWORD lpStripOffset = (LPDWORD)TagStripOffsets->lpData;
	LPDWORD lpStripByteCounts = (LPDWORD)TagStripByteCounts->lpData;
	int TotalSize = 0;
	for (DWORD i = 0; i < strip; i++)
	{
		int offset = *(lpStripOffset++);
		IO_Seek(offset, SEEK_SET);
		int Bufsize = *(lpStripByteCounts++);
		TotalSize += Bufsize;
		IO_Read(lpImageBufTemp, 1, Bufsize);
		lpImageBufTemp += Bufsize;
	}

	//Reset StripOffsets and StripByteCounts
	delete[]TagStripOffsets->lpData;
	TagStripOffsets->lpData = lpImageBuf;
	TagStripOffsets->n = 1;
	TagStripOffsets->type = Long;
	//TagStripOffsets->value = lpImageBuf;//Don't care, It mean's nothing.

	delete[]TagStripByteCounts->lpData;
	TagStripByteCounts->lpData = nullptr;
	TagStripByteCounts->type = Short;
	TagStripByteCounts->n = 1;
	TagStripByteCounts->value = stripByteCounts;

	//Reset RowsPerStrip, it should be the same with Length;
	SetTagValue(RowsPerStrip, GetTagValue(ImageLength));

	return Tiff_OK;
}

template<class T>   //T:type, Ts:type size
void Tiff::Pack(int Width, int Length)
{
	TiffTagPtr TagStripOffsets = GetTag(StripOffsets);
	int PixelsPerChannel = Width * Length;
	if (GetTagValue(SamplesPerPixel) == 3)
	{
		T* lpR = (T*)TagStripOffsets->lpData;
		T* lpG = lpR + PixelsPerChannel;
		T* lpB = lpG + PixelsPerChannel;
		T* lpNewImage = new T[PixelsPerChannel * 3];
		T* lpTemp = lpNewImage;
		for (int i = 0; i < Length; i++)
			for (int j = 0; j < Width; j++)
			{
				*(lpTemp++) = *(lpR++);
				*(lpTemp++) = *(lpG++);
				*(lpTemp++) = *(lpB++);
			}

		delete[]TagStripOffsets->lpData;//Old Image
		TagStripOffsets->lpData = (BYTE*)lpNewImage;
	}
	else
	{
		T* lpC = (T*)TagStripOffsets->lpData;
		T* lpM = lpC + PixelsPerChannel;
		T* lpY = lpM + PixelsPerChannel;
		T* lpK = lpY + PixelsPerChannel;
		T* lpNewImage = new T[PixelsPerChannel * 4];
		T* lpTemp = lpNewImage;
		for (int i = 0; i < Length; i++)
			for (int j = 0; j < Width; j++)
			{
				*(lpTemp++) = *(lpC++);
				*(lpTemp++) = *(lpM++);
				*(lpTemp++) = *(lpY++);
				*(lpTemp++) = *(lpK++);
			}
		delete[]TagStripOffsets->lpData;//Old Image
		TagStripOffsets->lpData = (BYTE*)lpNewImage;
	}
}

//////////////////////////////////////////////////////////////////////
// Tiff Write File Operaton
//////////////////////////////////////////////////////////////////////
ErrCode Tiff::WriteHeader(IO_INTERFACE *IO)
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

	IO_Write((LPBYTE)TiffHeader, sizeof(WORD), 4);
	return Tiff_OK;
}

ErrCode Tiff::WriteIFD(IO_INTERFACE *IO)
{
	const int IFD_Offset = 8;//Header Size;
	int OffsetValue = IFD_Offset + 2 + (int)m_IFD.m_TagList.size() * 12 + 4;//	OffsetValue = header + sizeof(taglist) + nextIFD;
	WORD EntryCounts = (WORD)m_IFD.m_TagList.size();//EntryCounts;
	IO_Seek(IFD_Offset, SEEK_SET);
	IO_Write((LPBYTE)&EntryCounts, sizeof(WORD), 1);

	//Recaculate Tag Offset.	
	//for (auto pos = TiffTag_Begin; pos != TiffTag_End; ++pos)
	for (auto& pos : m_IFD.m_TagList)
	{
		bool Offset = false;
		switch (pos->type)
		{
		case SBYTE:
		case UndefineType:
		case SShort:
		case SLong:Offset = true;
		default:break;
		}

		int DataSize;
		if (Offset == true)
		{//Now Just for Icc profile
			DataSize = DataType[(int)pos->type] * pos->n;
			pos->value = OffsetValue;
			OffsetValue += DataSize;
		}
		else
		{
			DataSize = DataType[(int)pos->type] * pos->n;
			if (DataSize > 4)
			{
				pos->value = OffsetValue;
				OffsetValue += DataSize;
			}
		}
	}

	//Some Special Tag need to be reset...
	//StripOffset
	TiffTagPtr	TempTag = GetTag(StripOffsets);
	TempTag->value = OffsetValue;

	OffsetValue += GetTagValue(StripByteCounts);

	TempTag = GetTag(Exif_IFD);
	if (TempTag != nullptr)
		TempTag->value = OffsetValue;

	//Write IFD
	DWORD *lpOutData = new DWORD[EntryCounts * 3 + 12];
	memset(lpOutData, 0, EntryCounts * 3 + 12);
	DWORD *lpTemp = lpOutData;

	//for_each(TiffTag_Begin, TiffTag_End, [&lpTemp](const TiffTagPtr& pos)
	for (const auto& pos : m_IFD.m_TagList)
	{
		*lpTemp++ = pos->tag | (pos->type << 16);
		*lpTemp++ = pos->n;
		*lpTemp++ = pos->value;
	};

	IO_Write((LPBYTE)lpOutData, sizeof(DWORD), EntryCounts * 3);
	delete[]lpOutData;

	//Write Next IFD
	DWORD NextIFD = 0;
	IO_Write((LPBYTE)&NextIFD, sizeof(DWORD), 1);

	return Tiff_OK;
}

ErrCode Tiff::WriteTagData(IO_INTERFACE *IO)
{
//	for_each(TiffTag_Begin, TiffTag_End,[&IO](const TiffTagPtr& pos) {pos->SaveFile(IO); });
	for (const auto& pos : m_IFD.m_TagList)
		pos->SaveFile(IO);

	return Tiff_OK;
}

ErrCode Tiff::WriteImageData(IO_INTERFACE *IO)
{
	DWORD stripByteCounts = GetTagValue(StripByteCounts);
	TiffTagPtr TempTag = GetTag(StripOffsets);
	IO_Write(TempTag->lpData, 1, stripByteCounts);
	return Tiff_OK;
}

ErrCode	Tiff::WriteData_Exif_IFD_Tag(IO_INTERFACE *IO)
{
	TiffTagPtr TempTag = GetTag(Exif_IFD);
	if (TempTag != nullptr)
		IO_Write(TempTag->lpData, 1, Exif_IFD_Size);
	return Tiff_OK;
}

//Not Fully Test yet...
#if defined(VIRTUAL_IO) | defined(VIRTUAL_IO_STL)
ErrCode Tiff::SaveMemory(LPBYTE Buffer, size_t BufSize, size_t &SaveSize)
{
	IO_INTERFACE *IO = new IO_Buf(Buffer, BufSize);
	ErrCode ret = SaveTiff(IO);
	SaveSize = dynamic_cast<IO_Buf*>(IO)->Tell();
	IO_Close(IO);
	return ret;
}

//Not Fully Test yet...
ErrCode Tiff::ReadMemory(LPBYTE Buffer, size_t BufSize)
{
	IO_INTERFACE *IO = new IO_Buf(Buffer, BufSize);
	ErrCode ret = ReadTiff(IO);
	IO_Close(IO);
	return ret;
}
#endif//(VIRTUAL_IO) | (VIRTUAL_IO_STL)

//////////////////////////////////////////////////////////////////////
// CTiff
//////////////////////////////////////////////////////////////////////
CTiff::CTiff()
{
	m_Width = m_Length = m_SamplesPerPixel = m_BitsPerSample = m_BytesPerLine = m_Resolution = 0;
	m_lpImageBuf = nullptr;
}

CTiff::~CTiff()
{}

CTiff::CTiff(LPCSTR FileName)
{
	ReadFile(FileName);
}

CTiff::CTiff(int width, int length, int resolution, int samplesperpixel, int bitspersample, int AllocBuf)
{
	CreateNew(width, length, resolution, samplesperpixel, bitspersample, AllocBuf);
}

ErrCode CTiff::SetTag(TiffTagSignature Signature, WORD type, DWORD n, DWORD value, LPBYTE lpBuf)
{
	DWORD SigType = (Signature) | (type << 16);
	TiffTagPtr New = CreateTag(SigType, n, value, nullptr);
	return Tiff::SetTag(New);
}

ErrCode	CTiff::SetTagValue(const TiffTagSignature Signature, DWORD Value)
{
	TiffTagPtr New = Tiff::GetTag(Signature);
	if (New != nullptr)
		New->value = Value;
	return Tiff_OK;
}

ErrCode CTiff::CreateNew(int width, int length, int resolution, int samplesperpixel, int bitspersample, int AllocBuf)
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

	int stripByteCounts = (DWORD)(ceil(width * bitspersample * 0.125) * length * samplesperpixel);
	SetTag(StripByteCounts, Long, 1, stripByteCounts);

	SetTag(StripOffsets, Long, 1, 0);//For code analysis.
	TiffTagPtr StripOffsetsTag = GetTag(StripOffsets);
	if (AllocBuf == 1)
	{//Create StripOffsets Tag directly.
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

	SetTag(IccProfile, UndefineType, 0, 0);

	//Basic Property for easy using.
	m_Width = width;
	m_Length = length;
	m_SamplesPerPixel = samplesperpixel;
	m_BitsPerSample = bitspersample;
	m_Resolution = resolution;
	m_BytesPerLine = (int)ceil(m_Width * m_SamplesPerPixel * m_BitsPerSample * 0.125);

	return Tiff_OK;
}


ErrCode CTiff::CreateNew(int width, int length, int resolution, int samplesperpixel, int bitspersample, LPCSTR InName)
{
	IO_INTERFACE *IO;

	if ((IO = IO_In(InName)) == nullptr)
	{
		cout << "*** CTiff::CreateNew() --> Create File Fail *** ";
		return FileOpenErr;
	}

	CreateNew(width, length, resolution, samplesperpixel, bitspersample, 1);

	IO_Read(m_lpImageBuf, 1, m_BytesPerLine * m_Length);

	IO_Close(IO);

	return Tiff_OK;
}

ErrCode CTiff::CreateNew(int width, int length, int resolution, int samplesperpixel, int bitspersample, LPCSTR InName, LPCSTR OutName)
{
	CreateNew(width, length, resolution, samplesperpixel, bitspersample, InName);
	SaveFile(OutName);
	return Tiff_OK;
}


ErrCode CTiff::ReadTiff(IO_INTERFACE *IO)
{
	ErrCode ret = Tiff::ReadTiff(IO);

	if (ret != Tiff_OK)
		return ret;

	//Basic Property for easy using.
	m_Width = GetTagValue(ImageWidth);
	m_Length = GetTagValue(ImageLength);
	m_SamplesPerPixel = GetTagValue(SamplesPerPixel);
	m_BitsPerSample = GetTagValue(BitsPerSample);
	m_BytesPerLine = (int)ceil(m_Width * m_SamplesPerPixel * m_BitsPerSample * 0.125);
	m_Resolution = GetTagValue(XResolution);
	TiffTagPtr TempTag = Tiff::GetTag(StripOffsets);
	m_lpImageBuf = TempTag->lpData;
	return Tiff_OK;
}


ErrCode CTiff::ReadFile(LPCSTR FileName)
{
	Reset();
	IO_INTERFACE *IO = IO_In(FileName);
	if (IO == nullptr)
	{
		cout << FileName << " Open Fail" << endl;
		return FileOpenErr;
	}

	ErrCode ret = ReadTiff(IO);

	IO_Close(IO);
	return ret;
}


#if defined(VIRTUAL_IO) | defined(VIRTUAL_IO_STL)
ErrCode CTiff::ReadMemory(LPBYTE Buffer, size_t BufSize)
{
	if (Tiff::ReadMemory(Buffer, BufSize) != Tiff_OK)
		return FileOpenErr;
	//Basic Property for easy using.
	m_Width = GetTagValue(ImageWidth);
	m_Length = GetTagValue(ImageLength);
	m_SamplesPerPixel = GetTagValue(SamplesPerPixel);
	m_BitsPerSample = GetTagValue(BitsPerSample);
	m_BytesPerLine = (int)ceil(m_Width * m_SamplesPerPixel * m_BitsPerSample * 0.125);
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

	//T* lpPosition = (T*)m_lpImageBuf;
	T* lpWidthBuf = new T[m_Width*m_SamplesPerPixel];
	LPBYTE lpCurrent = (LPBYTE)lpBuf;
	int LineBufSize = (int)(RecX * m_SamplesPerPixel * m_BitsPerSample) >> 3;
	int StartY = y;
	int EndY = y + RecY;

	for (int i = StartY; i < EndY; i++)
	{
		GetRow(lpWidthBuf, i);
		T *lpStartX = lpWidthBuf + (x * m_SamplesPerPixel);
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

	if (Y < 0)
		Y = m_Length - 1;
	if (Y > (m_Length - 1))
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

ErrCode CTiff::SetIccProfile(char *IccFile)
{
	IO_INTERFACE *IO = IO_In(IccFile);
	if (IO == nullptr)
	{
		cout << " *** CTiff::SetIccProfile() --> Icc Profile open Fail. *** " << endl;
		return FileOpenErr;
	}

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

void CTiff::SaveIccProfile(char *OutIccFile)
{
	TiffTagPtr TempTag = Tiff::GetTag(IccProfile);
	if (TempTag != nullptr)
	{
		IO_INTERFACE *IO = IO_Out(OutIccFile);
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

	auto pos = find_if(TiffTag_Begin, TiffTag_End,
		[&sig](TiffTagPtr pos) {return pos->tag == sig; });

	if (pos != m_IFD.m_TagList.end())
	{
		//delete *pos;
		m_IFD.m_TagList.erase(pos);
	}
}