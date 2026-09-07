/////////////////////////////////////////////////////////////////////x
// Tiff_Tags.cpp: TiffTag family, IFD, and Tiff tag operations.
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

const char* strTiffErr[9] = { "Tiff_OK", "FileOpenErr", "VersionErr", "TooManyTags", "TagStripErr", "MemoryAllocFail", "DataTypeErr", "CompressData", "UnDefineErr" };

namespace AV_Tiff_STL4 {
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

	//************************************************************
	// Just for writing code easily, for fun......(^_^) John.....
	}; //namespace AV_Tiff_STL4


TiffTag::TiffTag() :lpData(nullptr), n(0), value(0)
{
	tag = NullTag;
	type = UnknownType;
}

TiffTag::~TiffTag()
{
	if (lpData != nullptr)
		delete[]lpData;
}

//Copy Construct
TiffTag::TiffTag(const TiffTag& Tag) :tag(Tag.tag), type(Tag.type), n(Tag.n), value(Tag.value)
{
	lpData = nullptr;
	int DataSize = DataType[(int)type] * this->n;
	if (DataSize > 4)
	{
		value = 0;//Just for safety, Photoshop will issue a warning if the value is not 0, even if the value is not used.
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
		int DataSize = DataType[(int)type] * this->n;
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
	type = UnknownType;
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

TiffTag::TiffTag(DWORD SigType, DWORD n, DWORD value, IO_INTERFACE* IO)
{
	tag = (TiffTagSignature)(0xFFFF & SigType); //We not sure the DWORD is 32 bits.
	type = (FieldType)(0XF & (SigType >> 16));
	this->n = n;
	this->value = value;

	int DataSize = DataType[(int)type] * this->n;
	if (DataSize > 4)
	{//memory address
		if (IO != nullptr)
		{
			if (DataSize % 2 != 0)
			{
				DataSize += 1; //The Value is expected to begin on a word boundary; the corresponding Value Offset will thus be an even number.
				this->n += 1;
			}
			lpData = new BYTE[DataSize];
			memset(lpData, 0, DataSize);//Just for safety, Photoshop will issue a warning.
			IO_Seek(value, SEEK_SET);
			IO_Read(lpData, DataType[(int)type], n);
		}
		else
			//Not from file, Setting directly, 
			//for example setting BitsPerSample), It will set up the data in the constructor.
			lpData = nullptr;
	}
	else
		lpData = nullptr;
}

DWORD TiffTag::GetValue() const
{
	DWORD DataSize = DataType[(int)type] * this->n;
	if (DataSize <= 4)
		return value;
	else if (type == Long)
		return (*(LPDWORD)lpData);//Full 32 bits, WORD read truncated strip offsets > 64KB.
	else
		return (*(LPWORD)lpData);
}

int TiffTag::SaveFile(IO_INTERFACE* IO)
{
	int DataSize = DataType[(int)type] * n;
	int ret = 0;
	if ((DataSize > 4) && (lpData != nullptr))
		ret = (int)IO_Write(lpData, DataType[(int)type], n);
	return ret;
}

bool TiffTag::ValueIsOffset() const
{
	int DataSize = DataType[(int)type] * n;
	
	if (DataSize > 4)
		return true;
	else
		return false;	
}

/*****************************************************************************************
//Special Tags
*****************************************************************************************/
//StripOffsetTag)
StripOffsetsTag::StripOffsetsTag(DWORD SigType, DWORD n, DWORD value, IO_INTERFACE* IO)
	:TiffTag(SigType, n, value, IO)
{
	m_ImgLzw = nullptr;
}

StripOffsetsTag::~StripOffsetsTag()
{
	if (m_ImgLzw != nullptr)
	{
		delete[]m_ImgLzw;
		m_ImgLzw = nullptr;
	}
}

 void StripOffsetsTag::SetLzwData(LPBYTE lpBuf)
{
	m_ImgLzw = lpBuf;
}

LPBYTE StripOffsetsTag::GetLzwData()
{
	return m_ImgLzw;
}

//BitsPerSample)
BitsPerSampleTag::BitsPerSampleTag(DWORD SigType, DWORD n, DWORD value, IO_INTERFACE* IO)
	:TiffTag(SigType, n, value, IO)
{
	if (IO == nullptr)
	{//For CreateNew File, SetTag(BitsPerSampleTag)
		if (lpData != nullptr)
			delete []lpData;

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
ResolutionTag::ResolutionTag(DWORD SigType, DWORD n, DWORD value, IO_INTERFACE* IO)
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
//Exif_IFD_Tag::Exif_IFD_Tag(DWORD SigType, DWORD n, DWORD value, IO_INTERFACE *IO)
int Exif_IFD_Tag::ExifBufSize = 0;
Exif_IFD_Tag::Exif_IFD_Tag(DWORD SigType, DWORD n, DWORD value, IO_INTERFACE* IO)
	:TiffTag(SigType, n, value, IO)
{//Actually, we know nothing about this tag.
	IO_Seek(value, SEEK_SET);

	//Find exif tag size
	auto current = IO_Tell();
	IO_Seek(0, SEEK_END);
	auto end = IO_Tell();
	int size = end - current;

	lpData = new BYTE[size];
	memset(lpData, 0, size);

	IO_Seek(current, SEEK_SET);
	IO_Read(lpData, 1, size);

	//Special condition, We need to change the value of the this->n.
	Exif_IFD_Tag::ExifBufSize = size;
}


IFD_STRUCTURE::IFD_STRUCTURE() :NextIFD(0), m_TagList()
{
	m_TagList.clear();
}

//////////////////////////////////////////////////////////////////////
// Tiff
//////////////////////////////////////////////////////////////////////
TiffTagPtr Tiff::GetTag(const TiffTagSignature Signature)
{
	auto pos = find_if(m_IFD.m_TagList.begin(), m_IFD.m_TagList.end(),
		[&Signature](const TiffTagPtr& pos) {return pos->tag == Signature; });

	if (pos != m_IFD.m_TagList.end())
#if SMART_POINTER
		return *pos;
#else
		return pos->get();
#endif
	else
		return nullptr;
}

DWORD Tiff::GetTagValue(const TiffTagSignature Signature)
{
	TiffTagPtr Tag = GetTag(Signature);
	if (Tag != nullptr)
		return Tag->GetValue();
	else
		return 0;
}

Tiff_Err Tiff::SetTag(TiffTagPtr NewTag)
{
	if (NewTag == nullptr)
		return Tiff_OK;//If the tag is unknown, just forget it!

	TiffTagSignature sig = NewTag->tag;

	auto pos = find_if(m_IFD.m_TagList.begin(), m_IFD.m_TagList.end(),
		[&sig](const TiffTagPtr& pos) {return pos->tag == sig; });

	if (pos == m_IFD.m_TagList.end())
	{
		m_IFD.m_TagList.push_back(TagListItem(NewTag));
	}
	else
	{//Replace
		if (pos->get() != NewTag.get())
			* pos = TagListItem(NewTag);//Old tag released by the owning pointer.
	}
	return Tiff_OK;
}

Tiff_Err Tiff::SetTagValue(const TiffTagSignature Signature, DWORD Value)
{
	TiffTagPtr lpTag = GetTag(Signature);
	if (lpTag != nullptr)
	{
		//For offset tags (DataSize > 4) the value field is a file offset, not the
		//data. Writing it would corrupt the tag, so refuse instead of ignoring it.
		if (DataType[(int)lpTag->type] * lpTag->n > 4)
			return DataTypeErr;
		lpTag->value = Value;
	}
	//Missing tags are silently ignored, callers may rely on that.
	return Tiff_OK;
}

Tiff_Err Tiff::RemoveTag(const TiffTagSignature Signature)
{
	auto pos = find_if(m_IFD.m_TagList.begin(), m_IFD.m_TagList.end(),
		[&Signature](const TiffTagPtr& pos) {return pos->tag == Signature; });

	if (pos != m_IFD.m_TagList.end())
	{
#if FIXED_VECTOR
		//FixedVector::erase returns bool, std::vector::erase returns iterator.
		if (!m_IFD.m_TagList.erase(pos))
			return TagNorFound;
#else
		m_IFD.m_TagList.erase(pos);
#endif
		//The owning pointer (TagListItem) releases the tag.
		return Tiff_OK;
	}
	else
		return TagNorFound;
}
//////////////////////////////////////////////////////////////////////
// Tiff Read File Operaton
//////////////////////////////////////////////////////////////////////
TiffTagPtr Tiff::CreateTag(DWORD SigType, DWORD n, DWORD value, IO_INTERFACE* IO)
{
	TiffTag* NewTag = nullptr;

	try {
		switch ((TiffTagSignature)(0xFFFF & SigType))
		{
			//	Special Tag
		case StripOffsets:
			NewTag = new StripOffsetsTag(SigType, n, value, IO);
			break;

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
			* Sometimes I wonder if it's worth it.
			* And this tag must been read at the end of tiff.
			*/
			NewTag = new Exif_IFD_Tag(SigType, n, value, IO);
			break;

		//Skip Tag for the time being, Just for test.
		//case PageName:
		//	break;
		//case XML_Data:
		//case IPTC:
		//case Photoshop:
		//case IccProfile:
		//	NewTag = new TiffTag(SigType, n, value, IO);
		//	NewTag = nullptr;//
		//	break;

		default:
			NewTag = new TiffTag(SigType, n, value, IO);
			break;
		}
	}
	catch (bad_alloc& ba)
	{
		//if (nullptr != NewTag)
		//	delete NewTag;
		NewTag = nullptr;
		cerr << "bad_alloc caught: " << ba.what() << endl;
		return nullptr;
	}

	return (TiffTagPtr)NewTag;
}

void Tiff::AddTags(DWORD TypeSignature, DWORD n, DWORD value, IO_INTERFACE* IO)
{
	TiffTagPtr tag = CreateTag(TypeSignature, n, value, IO);

	if (tag != nullptr)
		m_IFD.m_TagList.push_back(tag);
}

