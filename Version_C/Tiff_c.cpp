#include "stdafx.h"
#include "tiff_c.h"
#include <stdio.h>
#include <malloc.h>
#include <memory.h >
#include <math.h>
#include <string.h >

#ifndef SWAP 

inline DWORD SwapDWORD(const DWORD x)
{
	return (((x & 0xFF000000) >> 24) | ((x & 0xFF0000) >> 8) | ((x & 0xFF00) << 8) | (x << 24));
}

inline WORD SwapWORD(const WORD x)
{
	return (((x & 0xFF) << 8) | (x >> 8));
}
#endif //SWAP

#define FieldTypeSize  12				   
#define Exif_IFD_Size  44

//return the size of the data type.
const int DataType[FieldTypeSize] = {
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
	8//Double						= 0x000BL
};

//////////////////////////////////////////////////////////////////////
// Class TiffTag
//////////////////////////////////////////////////////////////////////

TiffTag* TiffTag_Create()
{
	TiffTag *This = (TiffTag*)malloc(sizeof(TiffTag));
	This->tag = NullTag;
	This->lpData = NULL;
	This->type = UnknownType;
	This->n = 0;
	This->value = 0;
	return This;
}

//Destructor of TiffTag
void TiffTag_Delete(TiffTag *This)
{
	if (This->lpData != NULL)
		free(This->lpData);
	free(This);
}

TiffTag* TiffTag_TiffTag0(TiffTagSignature Tag)
{
	TiffTag *This = (TiffTag*)malloc(sizeof(TiffTag));
	This->tag = Tag;
	This->type = UnknownType;
	This->n = 0;
	This->value = 0;
	This->lpData = NULL;
	return This;
}

TiffTag* TiffTag_TiffTag1(TiffTagSignature Tag, FieldType Type, DWORD n, DWORD value, LPBYTE lpBuf)
{
	TiffTag *This = (TiffTag*)malloc(sizeof(TiffTag));
	This->tag = Tag;
	This->type = Type;
	This->n = n;
	This->value = value;
	This->lpData = lpBuf;
	return This;
}

TiffTag* TiffTag_TiffTag2(DWORD SigType, DWORD n, DWORD value, IO_Interface *IO_C)
{
	int DataSize;
	TiffTag *NewTag;
	NewTag = (TiffTag*)malloc(sizeof(TiffTag));
	NewTag->tag = (TiffTagSignature)(0xFFFF & SigType);
	NewTag->type = (FieldType)(0XFFFF & (SigType >> 16));
	NewTag->n = n;
	NewTag->value = value;

	DataSize = DataType[NewTag->type] * NewTag->n;
	if (DataSize > 4)
	{//memory address
		if (IO_C != NULL)
		{
			NewTag->lpData = (LPBYTE)malloc(DataSize);
			IO_Seek_C(value, SEEK_SET);
			IO_Read_C(NewTag->lpData, DataType[NewTag->type], n);
		}
		else
			//Not from file, Setting directly, 
			//for example setting BitsPerSample, It will set up the data in the constructor.
			NewTag->lpData = NULL;
	}
	else
		NewTag->lpData = NULL;

	return NewTag;
}


//Virtual Function
DWORD TiffTag_GetValue_DWORD(PTiffTag This)
{
	DWORD DataSize = DataType[This->type] * This->n;
	if (DataSize <= 4)
		return This->value;
	else
		return (*(LPWORD)This->lpData);
}

DWORD TiffTag_GetValue_BitsPerSampleTag(PTiffTag This)
{
	if (This->n == 1)
		return This->value;
	else
		if (This->lpData != NULL)
			return (DWORD)(*(LPWORD)This->lpData);
		else
			return 0;
}

DWORD TiffTag_GetValue_ResolutionTag(PTiffTag This)
{
	return (DWORD)(*(LPDWORD)(This->lpData) / *(LPDWORD)(This->lpData + 4));
}


DWORD TiffTag_GetValue_Exif_IFD(PTiffTag This)
{
	return (DWORD)(This->lpData);
}

//////////////////////////////////////////////////////////////////////
// CTiffFile
//////////////////////////////////////////////////////////////////////
Tiff* Tiff_Create()
{
	Tiff *This = (Tiff*)malloc(sizeof(Tiff));
	m_IFD.EntryCounts = 0;
	m_IFD.NextIFD = 0;
	memset(m_IFD.TagList, 0, sizeof(TiffTag*) * MAXTAG);
	return This;
}

void Tiff_Close(Tiff *This)
{
	Tiff_Reset(This);
	free(This);
}

void Tiff_Reset(Tiff *This)
{
	int i;
	int TagCount = This->IFD.EntryCounts;
	TiffTag **TagList = This->IFD.TagList;
	for (i = 0; i < TagCount; i++)
		TiffTag_Delete(TagList[i]);
	This->IFD.EntryCounts = 0;
}

//////////////////////////////////////////////////////////////////////
// CTiffFile Read File Operaton
//////////////////////////////////////////////////////////////////////
TiffTag* Tiff_GetTag(Tiff *This, const TiffTagSignature Signature)
{
	TiffTag **TagList = m_IFD.TagList;
	TiffTag *Tag = NULL;
	int TagCount = m_IFD.EntryCounts;
	int index = 0;
	for (index = 0; index < TagCount; index++)
		if (TagList[index]->tag == Signature)
		{
			Tag = TagList[index];
			break;
		}
	return Tag;
}

DWORD Tiff_GetTagValue(Tiff *This, const TiffTagSignature Signature)
{
	TiffTag *Tag = GetTag(Signature);
	if (Tag != NULL)
		return Tag->GetValue(Tag);
	else
		return 0;
}

int Tiff_SetTag(Tiff *This, TiffTag *NewTag)
{
	TiffTag **TagList = m_IFD.TagList;
	TiffTagSignature sig = NewTag->tag;	
	int TagCount = m_IFD.EntryCounts;
	int index = 0;
	bool TagFound = false;

	if (NewTag == NULL)
		return 0;

	for (index = 0; index < TagCount; index++)
		if (TagList[index]->tag == sig)
		{
			if (TagList[index] != NewTag)
			{//Replace 
				TiffTag_Delete(TagList[index]);
				TagList[index] = NewTag;
			}
			TagFound = true;
			break;
		}


	if (TagFound == false)
	{//Can't find the tag, Add it.
		TagList[TagCount] = NewTag;
		m_IFD.EntryCounts++;
	}

	return 0;
}

int Tiff_SetTag2(Tiff *This, TiffTagSignature Signature, WORD type, DWORD n, DWORD value, LPBYTE lpBuf)
{
	DWORD SigType = (Signature) | (type << 16);
	TiffTag *New = CreateTag(SigType, n, value, NULL);
	return SetTag(New);
}

DWORD Tiff_SetTagValue(Tiff *This, const TiffTagSignature Signature, DWORD Value)
{
	TiffTag* lpTag = GetTag(Signature);
	if (lpTag != NULL)
		lpTag->value = Value;
	return SetTag(lpTag);
}

//////////////////////////////////////////////////////////////////////
// CTiffFile Read File Operaton
//////////////////////////////////////////////////////////////////////
int Tiff_ReadFile(Tiff *This, const char *FileName)
{
	IO_Interface *IO;

	Reset();
	if (FileName == NULL)
		return -1;//FileOpenErr;

	IO = IO_In_C(FileName, "rb");

	if (Tiff_Read(This, IO) != 0)
		return -1;

	IO_Close_C(IO);
	return 0;
}

int Tiff_Read(Tiff *This, IO_Interface *IO_C)
{
	TiffTag *TempTag;
	DWORD	TiffVersion;
	WORD	TagCount;
	DWORD	Temp[MAXTAG * 3];
	int i;

	if (IO_C == NULL)
		return -1;//FileOpenErr;

	IO_Seek_C(0, SEEK_SET);
	IO_Read_C(&TiffVersion, 1, 4);

	if (TiffVersion != 0x002A4949)
	{//PC Version
		IO_Close_C(IO_C);
		IO_C = NULL;
		return -1;//VersionErr;	
	}

	IO_Read_C(&m_IFD_Offset, 1, 4);
	IO_Seek_C(m_IFD_Offset, SEEK_SET);
	IO_Read_C(&TagCount, 1, 2);
	if (TagCount >= MAXTAG)
	{//Too many m_Tags
		IO_Close_C(IO_C);
		return -1;//TooManyTags;
	};

	IO_Read_C(Temp, TagCount * 3, 4);

	//Read IFD Data.
	for (i = 0; i < TagCount * 3; i += 3)
		AddTags(Temp[i], Temp[i + 1], Temp[i + 2], IO_C);

	Tiff_ReadImage(This, IO_C);

	//Setup Useful info
	//CTiff::CTiff
	m_Width = GetTagValue(ImageWidth);
	m_Length = GetTagValue(ImageLength);
	m_SamplesPerPixel = GetTagValue(SamplesPerPixel);
	m_BitsPerSample = GetTagValue(BitsPerSample);
	m_BytesPerLine = (int)ceil(m_Width * m_SamplesPerPixel * m_BitsPerSample * 0.125);
	m_Resolution = GetTagValue(XResolution);
	TempTag = GetTag(StripOffsets);
	m_lpImageBuf = TempTag->lpData;

	return 0;
}


int Tiff_SaveFile(Tiff *This, const char *FileName)
{
	int ret;
	IO_Interface *IO_C = IO_Out_C(FileName, "wb+");
	if (m_IFD.EntryCounts == 0)
		return -1;
	ret = Tiff_WriteHeader(This, IO_C);
	ret = Tiff_WriteIFD(This, IO_C);
	ret = Tiff_WriteTagData(This, IO_C);
	ret = Tiff_WriteImageData(This, IO_C);
	ret = Tiff_Write_Exif_IFD_Tag(This, IO_C);
	IO_Close_C(IO_C);
	return ret;
}

TiffTag* Tiff_CreateTag(DWORD SigType, DWORD n, DWORD value, IO_Interface *IO_C)
{	
	TiffTag *NewTag = NULL;

	//Some special tag need to take care.
	switch ((0xFFFF & SigType))
	{
	//	Special Tag **********************
	case BitsPerSample:
	{//NewTag = new BitsPerSampleTag(SigType, n, value, File);
		NewTag = TiffTag_TiffTag2(SigType, n, value, IO_C);
		NewTag->GetValue = (PGetValue)&TiffTag_GetValue_BitsPerSampleTag;
		if (IO_C == NULL)
		{//For CreateNew File, SetTag(BitsPerSampleTag)
			if (NewTag->lpData != NULL)
				free(NewTag->lpData);

			if (n != 1)
			{
				DWORD i = 0;
				LPWORD lpTemp = (LPWORD)malloc(n * 2);
				NewTag->lpData = (LPBYTE)lpTemp;
				for (i = 0; i < n; ++i)
					*(lpTemp++) = (WORD)value;
			}
			else
			{
				NewTag->value = value;
				NewTag->lpData = NULL;
			}
		}
	}
	break;

	case XResolution:
	case YResolution:
	{//            NewTag = new ResolutionTag(SigType, n, value, File);
		NewTag = TiffTag_TiffTag2(SigType, n, value, IO_C);
		NewTag->GetValue = (PGetValue)&TiffTag_GetValue_ResolutionTag;
		if (IO_C == NULL)
		{
			LPDWORD lpTemp = (LPDWORD)malloc(4 * 2);// DWORD[2];
			lpTemp[0] = (DWORD)(value * 100);
			lpTemp[1] = 100;
			NewTag->lpData = (LPBYTE)lpTemp;
		}
	}
	break;

	//Just Skip This tag. Unknown info of Photoshop, Out of spec of Tiff6
	case Exif_IFD:
	{
		/*
		 * Photoshop Tag, Still Unknown...
		 * Don't save anything. This tag broke all rule, just skip it.
		 * If you want to save this tag.
		 * You may need to pay more expense.
		 * Sometimes I wonder that it is worth to do such thing.
		 */
		NewTag = TiffTag_TiffTag2(SigType, n, value, IO_C);
		NewTag->lpData = (LPBYTE)malloc(Exif_IFD_Size);
		IO_Seek_C(NewTag->value, SEEK_SET);
		IO_Read_C(NewTag->lpData, 1, Exif_IFD_Size);
		NewTag->GetValue = (PGetValue)&TiffTag_GetValue_Exif_IFD;
		break;
	}
	break;

	default:
		NewTag = TiffTag_TiffTag2(SigType, n, value, IO_C);
		NewTag->GetValue = (PGetValue)&TiffTag_GetValue_DWORD;
		break;

	}
	return NewTag;
}

void Tiff_AddTags(Tiff *This, DWORD TypeSignature, DWORD n, DWORD value, IO_Interface *File)
{
	TiffTag *tag = CreateTag(TypeSignature, n, value, File);
	TiffTag **TagList = m_IFD.TagList;
	int index = m_IFD.EntryCounts;
	if (tag != NULL)
	{
		TagList[index] = tag;
		m_IFD.EntryCounts++;
	}
}

int Tiff_ReadImage(Tiff *This, IO_Interface *IO_C)
{
	DWORD Width = GetTagValue(ImageWidth);
	DWORD Length = GetTagValue(ImageLength);
	DWORD rowsPerStrip = GetTagValue(RowsPerStrip);
	DWORD stripOffsets = GetTagValue(StripOffsets);
	DWORD planarConfiguration = GetTagValue(PlanarConfiguration);
	DWORD bitsPerSample = GetTagValue(BitsPerSample);
	DWORD samplesPerPixel = GetTagValue(SamplesPerPixel);
	LPBYTE lpImageBuf = NULL;
	if (Length == rowsPerStrip)
	{//Single Strip
		if ((planarConfiguration == 0) || (planarConfiguration == 1))
		{//RGBRGB
			TiffTag *TempTag = NULL;
			DWORD stripByteCounts = GetTagValue(StripByteCounts);
			lpImageBuf = (LPBYTE)malloc(stripByteCounts);
			IO_Seek_C(stripOffsets, SEEK_SET);
			IO_Read_C(lpImageBuf, 1, stripByteCounts);
			//Set ImageBuf address to StripOffset Tag
			TempTag = GetTag(StripOffsets);
			TempTag->lpData = lpImageBuf;
		}
		else
		{//PlanarConfiguration == 2, 
			if (bitsPerSample == 1)
			{//Undocument, just for avision, CMYKcm(4 or 6) 1bit.
				TiffTag *TempTag = NULL;
				DWORD stripByteCounts = (Width >> 3) * Length * samplesPerPixel;
				lpImageBuf = (LPBYTE)malloc(stripByteCounts);
				IO_Seek_C(stripOffsets, SEEK_SET);
				IO_Read_C(lpImageBuf, 1, stripByteCounts);
				//Set ImageBuf address to StripOffset Tag
				TempTag = GetTag(StripOffsets);
				TempTag->lpData = lpImageBuf;
			}
			else
			{//RRRGGGBBB, For PhotoShop CS2
				Tiff_ReadMultiStripOffset(This, IO_C);
				if (bitsPerSample == 8)
					Pack_BYTE(This, Width, Length);
				else
					Pack_WORD(This, Width, Length);

				//Reset PlanarConfiguration -> 1.
				Tiff_SetTagValue(This, PlanarConfiguration, 1);
			}
		}
	}
	else
		Tiff_ReadMultiStripOffset(This, IO_C);

	return 0;
}

int Tiff_ReadMultiStripOffset(Tiff *This, IO_Interface *IO_C)
{
	DWORD i;
	LPBYTE lpImageBuf, lpImageBufTemp;
	LPDWORD lpStripOffset, lpStripByteCounts;
	int TotalSize = 0;

	TiffTag *TagStripOffsets = GetTag(StripOffsets);
	TiffTag *TagStripByteCounts = GetTag(StripByteCounts);
	DWORD stripByteCounts = 0;
	DWORD strip = TagStripOffsets->n;
	LPDWORD lpTemp = (LPDWORD)TagStripByteCounts->lpData;
	for (i = 0; i < strip; ++i)
		stripByteCounts += *(lpTemp++);

	lpImageBuf = (LPBYTE)malloc(stripByteCounts);
	lpImageBufTemp = lpImageBuf;
	lpStripOffset = (LPDWORD)TagStripOffsets->lpData;
	lpStripByteCounts = (LPDWORD)TagStripByteCounts->lpData;

	for (i = 0; i < strip; i++)
	{
		int Bufsize;
		int offset = *(lpStripOffset++);
		IO_Seek_C(offset, SEEK_SET);
		Bufsize = *(lpStripByteCounts++);
		TotalSize += Bufsize;
		IO_Read_C(lpImageBufTemp, 1, Bufsize);
		lpImageBufTemp += Bufsize;
	}

	//Reset StripOffsets and StripByteCounts
	free(TagStripOffsets->lpData);
	TagStripOffsets->lpData = lpImageBuf;
	TagStripOffsets->n = 1;
	TagStripOffsets->type = Long;
	//		TagStripOffsets->value = lpImageBuf;//Don't care, It mean's nothing.

	free(TagStripByteCounts->lpData);
	TagStripByteCounts->lpData = NULL;
	TagStripByteCounts->type = Short;
	TagStripByteCounts->n = 1;
	TagStripByteCounts->value = stripByteCounts;

	//Reset RowsPerStrip, it should be the same with Length;
	SetTagValue(RowsPerStrip, GetTagValue(ImageLength));


	return 0;
}

void Pack_BYTE(Tiff *This, int Width, int Length)
{
	int i, j, PixelsPerChannel;
	TiffTag *TagStripOffsets = GetTag(StripOffsets);
	PixelsPerChannel = Width * Length;
	if (GetTagValue(SamplesPerPixel) == 3)
	{
		BYTE* lpR = (BYTE*)TagStripOffsets->lpData;
		BYTE* lpG = lpR + PixelsPerChannel;
		BYTE* lpB = lpG + PixelsPerChannel;
		BYTE* lpNewImage = (LPBYTE)malloc(PixelsPerChannel * 3);
		BYTE* lpTemp = lpNewImage;
		for (i = 0; i < Length; i++)
			for (j = 0; j < Width; j++)
			{
				*(lpTemp++) = *(lpR++);
				*(lpTemp++) = *(lpG++);
				*(lpTemp++) = *(lpB++);
			}

		free(TagStripOffsets->lpData);//Old Image
		TagStripOffsets->lpData = (BYTE*)lpNewImage;
	}
	else
	{
		BYTE* lpC = (BYTE*)TagStripOffsets->lpData;
		BYTE* lpM = lpC + PixelsPerChannel;
		BYTE* lpY = lpM + PixelsPerChannel;
		BYTE* lpK = lpY + PixelsPerChannel;
		BYTE* lpNewImage = (LPBYTE)malloc(PixelsPerChannel * 4);
		BYTE* lpTemp = lpNewImage;
		for (i = 0; i < Length; i++)
			for (j = 0; j < Width; j++)
			{
				*(lpTemp++) = *(lpC++);
				*(lpTemp++) = *(lpM++);
				*(lpTemp++) = *(lpY++);
				*(lpTemp++) = *(lpK++);
			}
		free(TagStripOffsets->lpData);//Old Image
		TagStripOffsets->lpData = (BYTE*)lpNewImage;
	}
}

void Pack_WORD(Tiff *This, int Width, int Length)
{
	int i, j, PixelsPerChannel;
	TiffTag *TagStripOffsets = GetTag(StripOffsets);
	PixelsPerChannel = Width * Length;
	if (GetTagValue(SamplesPerPixel) == 3)
	{
		WORD* lpR = (WORD*)TagStripOffsets->lpData;
		WORD* lpG = lpR + PixelsPerChannel;
		WORD* lpB = lpG + PixelsPerChannel;
		WORD* lpNewImage = (LPWORD)malloc(PixelsPerChannel * 3 * 2);
		WORD* lpTemp = lpNewImage;
		for (i = 0; i < Length; i++)
			for (j = 0; j < Width; j++)
			{
				*(lpTemp++) = *(lpR++);
				*(lpTemp++) = *(lpG++);
				*(lpTemp++) = *(lpB++);
			}

		free(TagStripOffsets->lpData);//Old Image
		TagStripOffsets->lpData = (BYTE*)lpNewImage;
	}
	else
	{
		WORD* lpC = (WORD*)TagStripOffsets->lpData;
		WORD* lpM = lpC + PixelsPerChannel;
		WORD* lpY = lpM + PixelsPerChannel;
		WORD* lpK = lpY + PixelsPerChannel;
		WORD* lpNewImage = (LPWORD)malloc(PixelsPerChannel * 4 * 2);
		WORD* lpTemp = lpNewImage;
		for (i = 0; i < Length; i++)
			for (j = 0; j < Width; j++)
			{
				*(lpTemp++) = *(lpC++);
				*(lpTemp++) = *(lpM++);
				*(lpTemp++) = *(lpY++);
				*(lpTemp++) = *(lpK++);
			}
		free(TagStripOffsets->lpData);//Old Image
		TagStripOffsets->lpData = (BYTE*)lpNewImage;
	}
}

//////////////////////////////////////////////////////////////////////
// CTiffFile Write File Operaton
//////////////////////////////////////////////////////////////////////

int Tiff_WriteHeader(Tiff *This, IO_Interface *IO_C)
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

	IO_Write_C(&TiffHeader, sizeof(WORD), 4);
	return 0;
}


int Tiff_WriteIFD(Tiff *This, IO_Interface *IO_C)
{
	int index;
	LPDWORD lpOutData, lpTemp;
	TiffTag	*TempTag;
	int TagCount = m_IFD.EntryCounts;
	TiffTag **TagList = m_IFD.TagList;

	const int IFD_Offset = 8;//Header Size;
	WORD EntryCounts = m_IFD.EntryCounts;
	int OffsetValue = IFD_Offset + 2 + m_IFD.EntryCounts * 12 + 4;//	OffsetValue = header + sizeof(taglist) + nextIFD;

	IO_Seek_C(IFD_Offset, SEEK_SET);
	IO_Write_C(&EntryCounts, sizeof(WORD), 1);

	//Recaculate Tag Offset.
	for (index = 0; index < TagCount; index++)
	{
		int DataSize;
		bool Offset = false;
		switch (TagList[index]->type)
		{
		case SBYTE:
		case UndefineType:
		case SShort:
		case SLong:Offset = true;
		default:break;
		}

		if (Offset == true)
		{//Now Just for Icc profile
			DataSize = DataType[TagList[index]->type] * TagList[index]->n;
			TagList[index]->value = OffsetValue;
			OffsetValue += DataSize;
		}
		else
		{
			DataSize = DataType[TagList[index]->type] * TagList[index]->n;
			if (DataSize > 4)
			{
				TagList[index]->value = OffsetValue;
				OffsetValue += DataSize;
			}
		}
	}

	//Some Special Tag need to be reset...
	//StripOffset
	TempTag = GetTag(StripOffsets);
	TempTag->value = OffsetValue;

	OffsetValue += GetTagValue(StripByteCounts);

	TempTag = GetTag(Exif_IFD);
	if (TempTag != NULL)
		TempTag->value = OffsetValue;


	//Write IFD
	lpOutData = (LPDWORD)malloc(EntryCounts * 12 + 12);
	lpTemp = lpOutData;

	for (index = 0; index < TagCount; index++)
	{
		*lpTemp++ = TagList[index]->tag | (TagList[index]->type << 16);
		*lpTemp++ = TagList[index]->n;
		*lpTemp++ = TagList[index]->value;
	}

	IO_Write_C(lpOutData, sizeof(DWORD), EntryCounts * 3);
	free(lpOutData);

	//Write Next IFD	
	IO_Write_C(&m_IFD.NextIFD, sizeof(DWORD), 1);

	return 0;
}


int Tiff_WriteTagData(Tiff *This, IO_Interface *IO_C)
{
	int index;
	int TagCount = m_IFD.EntryCounts;
	TiffTag **TagList = m_IFD.TagList;
	for (index = 0; index < TagCount; index++)
	{
		int DataSize = DataType[TagList[index]->type] * TagList[index]->n;
		if (DataSize > 4)
			IO_Write_C(TagList[index]->lpData, DataType[TagList[index]->type], TagList[index]->n);
	}
	return 0;
}

int Tiff_WriteImageData(Tiff *This, IO_Interface *IO_C)
{
	DWORD stripByteCounts = GetTagValue(StripByteCounts);
	TiffTag *TempTag = GetTag(StripOffsets);
	IO_Write_C(TempTag->lpData, 1, stripByteCounts);
	return 0;
}

int Tiff_Write_Exif_IFD_Tag(Tiff *This, IO_Interface *IO_C)
{

	TiffTag *TempTag = GetTag(Exif_IFD);
	if (TempTag != NULL)
		IO_Write_C(TempTag->lpData, 1, Exif_IFD_Size);
	return 0;
}

//////////////////////////////////////////////////////////////////////
// CTiff
//////////////////////////////////////////////////////////////////////
Tiff* Tiff_CreateNew2(int width, int length, int resolution, int samplesperpixel, int bitspersample, int AllocBuf)
{
	Tiff *This = (Tiff*)malloc(sizeof(Tiff));
	Tiff_CreateNew(This, width, length, resolution, samplesperpixel, bitspersample, AllocBuf);
	return This;
}

Tiff* Tiff_CreateNew(Tiff *This, int width, int length, int resolution, int samplesperpixel, int bitspersample, int AllocBuf)
{//Initial IFD
	int stripByteCounts;

	//Tiff *This = (Tiff*)malloc(sizeof(Tiff));

	Reset();

	SetTag2(NewSubfileType, Byte, 4, 0);

	SetTag2(ImageWidth, Long, 1, width);

	SetTag2(ImageLength, Long, 1, length);

	SetTag2(BitsPerSample, Short, samplesperpixel, bitspersample);

	//SetTag(new BitsPerSampleTag());

	SetTag2(Compression, Short, 1, 1);

	if (samplesperpixel == 4)
		SetTag2(PhotometricInterpretation, Short, 1, 5);
	else if (samplesperpixel == 3)
		SetTag2(PhotometricInterpretation, Short, 1, 2);
	else //Gray or Lineart   PhotometricInterpretation, 0:WhiteIsZero, 1:BlackIsZero
		SetTag2(PhotometricInterpretation, Short, 1, 0);

	//0:White is zero, 1:Black is Zero, 2:RGB color, 3:Palette color, 5:CMYK, 6:YCbCr

	SetTag2(SamplesPerPixel, Short, 1, samplesperpixel);

	SetTag2(RowsPerStrip, Long, 1, length);

	stripByteCounts = (DWORD)(ceil(width * bitspersample * 0.125) * length * samplesperpixel);
	SetTag2(StripByteCounts, Long, 1, stripByteCounts);

	if (AllocBuf == 1)
	{//Create StripOffsets Tag directly.
		TiffTag *StripOffsetsTag = TiffTag_TiffTag1(StripOffsets, Long, 1, 0, NULL);
		StripOffsetsTag->lpData = (LPBYTE)malloc(stripByteCounts);
		memset(StripOffsetsTag->lpData, 0, stripByteCounts);
		SetTag(StripOffsetsTag);
		m_lpImageBuf = StripOffsetsTag->lpData;
	}
	else
		m_lpImageBuf = NULL;

	SetTag2(XResolution, Rational, 1, resolution);

	SetTag2(YResolution, Rational, 1, resolution);

	SetTag2(ResolutionUnit, Short, 1, 2);
	//2:inch, 3:centimeter

	SetTag2(PlanarConfiguration, Short, 1, 1);
	//1: Data is stored as RGBRGB..., 2:RRRR....,GGGG...., BBBB....

	SetTag2(IccProfile, UndefineType, 0, 0);

	//m_IFD.EntryCounts = (WORD)m_IFD.TagList.size();

	//Basic Property for easy using.
	m_Width = width;
	m_Length = length;
	m_SamplesPerPixel = samplesperpixel;
	m_BitsPerSample = bitspersample;
	m_Resolution = resolution;
	m_BytesPerLine = (int)ceil(width * samplesperpixel * bitspersample * 0.125);

	return This;
}

Tiff* Tiff_CreateNewFile(int width, int length, int resolution, int samplesperpixel, int bitspersample, char *InName, char *OutName)
{
	Tiff *This;
	FILE *stream;
	int TotalBytes;
	LPBYTE lpBuf;

	if ((stream = fopen(InName, "r")) == NULL)
		return NULL;

	This = Tiff_CreateNew2(width, length, resolution, samplesperpixel, bitspersample, 1);
	TotalBytes = m_BytesPerLine * m_Length;
	lpBuf = (LPBYTE)malloc(TotalBytes);
	fread(lpBuf, 1, TotalBytes, stream);
	memcpy(m_lpImageBuf, lpBuf, TotalBytes);
	SaveFile(OutName);
	fclose(stream);
	free(lpBuf);
	return 0;
}

#ifdef __cplusplus
int Tiff_GetRow(Tiff *This, LPBYTE lpBuf, int Line, int pixel)
#else
int Tiff_GetRow_BYTE(Tiff *This, LPBYTE lpBuf, int Line, int pixel)
#endif
{
	LPBYTE lpIndex;
	int Bytes = 0;

	if (Line < 0)
		Line = 0;
	if (Line > (int)(m_Length - 1))
		Line = m_Length - 1;

	lpIndex = (LPBYTE)m_lpImageBuf + m_BytesPerLine * Line;

	if (pixel != 0)
		Bytes = (int)((pixel * m_SamplesPerPixel * m_BitsPerSample) >> 3);
	else
		Bytes = m_BytesPerLine;

	memcpy(lpBuf, lpIndex, Bytes);
	return pixel;
}

#ifdef __cplusplus
int Tiff_GetRow(Tiff *This, LPWORD lpBuf, int Line, int pixel)
{
	return Tiff_GetRow(This, (LPBYTE)lpBuf, Line, pixel);
}
#else
int Tiff_GetRow_WORD(Tiff *This, LPWORD lpBuf, int Line, int pixel)
{
	return Tiff_GetRow_BYTE(This, (LPBYTE)lpBuf, Line, pixel);
}
#endif


#ifdef __cplusplus
int Tiff_PutRow(Tiff *This, LPBYTE lpBuf, int Line, int pixel)
#else
int Tiff_PutRow_BYTE(Tiff *This, LPBYTE lpBuf, int Line, int pixel)
#endif
{
	LPBYTE lpPosition;
	int Bytes = 0;

	if (Line < 0)
		Line = 0;
	if (Line > (int)(m_Length - 1))
		Line = m_Length - 1;

	lpPosition = m_lpImageBuf + m_BytesPerLine * Line;
	//Default: If pixel ==0.

	if (pixel != 0)
		Bytes = (int)((pixel * m_SamplesPerPixel * m_BitsPerSample) >> 3);
	else
		Bytes = m_BytesPerLine;

	memcpy((LPBYTE)lpPosition, (LPBYTE)lpBuf, Bytes);
	return pixel;
}

#ifdef __cplusplus
int Tiff_PutRow(Tiff *This, LPWORD lpBuf, int Line, int pixel)
{
	return Tiff_PutRow(This, (LPBYTE)lpBuf, Line, pixel);
}
#else
int Tiff_PutRow_WORD(Tiff *This, LPWORD lpBuf, int Line, int pixel)
{
	return Tiff_PutRow_BYTE(This, (LPBYTE)lpBuf, Line, pixel);
}
#endif


#ifdef __cplusplus
int Tiff_GetRowColumn(Tiff *This, LPBYTE lpBuf, int x, int y, int RecX, int RecY)
#else
int Tiff_GetRowColumn_BYTE(Tiff *This, LPBYTE lpBuf, int x, int y, int RecX, int RecY)
#endif
{//Just for Gray(8 or 16 bits) and Color(8 or 16 bits).
	LPBYTE lpWidthBuf, lpCurrent;
	int i, LineBufSize, StartY, EndY;

	if ((m_BitsPerSample != 8) && (m_BitsPerSample != 16))
		return -1;
	
	lpWidthBuf = (LPBYTE)malloc(m_Width * m_SamplesPerPixel * (m_BitsPerSample >> 3));
	lpCurrent = lpBuf;
	LineBufSize = (int)(RecX * m_SamplesPerPixel * m_BitsPerSample * 0.125);
	StartY = y;
	EndY = y + RecY;

	for (i = StartY; i < EndY; i++)
	{
		LPBYTE lpStartX = NULL;

#ifdef __cplusplus
		Tiff_GetRow(This, lpWidthBuf, i, 0);
#else
		Tiff_GetRow_BYTE(This, lpWidthBuf, i, 0);
#endif
		lpStartX = lpWidthBuf + (x * m_SamplesPerPixel * (m_BitsPerSample >> 3));
		memcpy(lpCurrent, lpStartX, LineBufSize);
		lpCurrent += LineBufSize;
	}

	free(lpWidthBuf);
	return 0;
}

#ifdef __cplusplus
int Tiff_GetRowColumn(Tiff *This, LPWORD lpBuf, int x, int y, int RecX, int RecY)
{
	return Tiff_GetRowColumn(This, (LPBYTE)lpBuf, x, y, RecX, RecY);
}
#else
int Tiff_GetRowColumn_WORD(Tiff *This, LPWORD lpBuf, int x, int y, int RecX, int RecY)
{
	return Tiff_GetRowColumn_BYTE(This, (LPBYTE)lpBuf, x, y, RecX, RecY);
}
#endif


LPBYTE Tiff_GetXY(Tiff *This, int X, int Y)
{
	if (X < 0)	X = 0;
	if (X > (m_Width - 1))	X = m_Width - 1;

	if (Y < 0)	Y = 0;
	if (Y > (m_Length - 1))Y = m_Length - 1;

	return  m_lpImageBuf + \
		(m_BytesPerLine * Y) + \
		((X * m_SamplesPerPixel * m_BitsPerSample) >> 3);
}


//Math coordination
LPBYTE Tiff_GetXY_M(Tiff *This, int X, int Y)
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


LPBYTE Tiff_GetImageBuf(Tiff *This)
{
	return m_lpImageBuf;
}

void Tiff_SetImageBuf(Tiff *This, LPBYTE lpBuf)
{
	TiffTag *StripOffsetTag = GetTag(StripOffsets);
	if (StripOffsetTag->lpData != NULL)
		free(StripOffsetTag->lpData);
	StripOffsetTag->lpData = lpBuf;
	m_lpImageBuf = lpBuf;
}

//*********************************************************************
//Icc Profile
//*********************************************************************
int Tiff_SetIccProfile(Tiff *This, char *IccFile)
{
	int FileSize = 0;
	TiffTag *Icc = NULL;
	IO_Interface *IO_C = IO_In_C(IccFile);

	if (IO_C == NULL)
		return -1;

	IO_Read_C(&FileSize, 1, 4);
	FileSize = SwapDWORD(FileSize);

	Icc = GetTag(IccProfile);
	if (Icc != NULL)
	{
		if (Icc->lpData != NULL)
			free(Icc->lpData);
		Icc->lpData = (LPBYTE)malloc(FileSize);
		Icc->n = FileSize;
		Icc->value = 0;//Offset, recaculating when saveing file.
		IO_Seek_C(0, SEEK_SET);
		IO_Read_C(Icc->lpData, 1, FileSize);
		IO_Close_C(IO_C);
	}
	return 0;
}

void Tiff_SaveIccProfile(Tiff *This, char *OutIccFile)
{
	TiffTag *TempTag = GetTag(IccProfile);
	if (TempTag != NULL)
	{
		IO_Interface *IO_C = IO_Out_C(OutIccFile);
		if (IO_C != NULL)
		{
			if (TempTag->lpData != NULL)
				IO_Write_C(TempTag->lpData, 1, TempTag->n);
			IO_Close_C(IO_C);
		}
	}
}

void Tiff_RemoveIcc(Tiff *This)
{
	TiffTag *TempTag = GetTag(IccProfile);
	if (TempTag->n != 0)
	{
		if (TempTag->lpData != NULL)
			free(TempTag->lpData);

		TempTag->lpData = NULL;
		TempTag->n = 0;
		TempTag->value = 0;
	}
}

//*********************************************************************
//Test Example
//*********************************************************************
#define TEST 0
#if TEST
#include "Tiff.h"

int main()
{
	Tiff* lpTiff = Tiff_Create();
	if (Tiff_ReadFile(lpTiff, "3RGB8.tif") == Tiff_OK)
		Tiff_SaveFile(lpTiff, "Out.tif");

	return 0;
}
#endif //TEST