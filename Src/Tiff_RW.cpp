/////////////////////////////////////////////////////////////////////x
// Tiff_RW.cpp: Tiff core file read/write.
// Split from Tiff_STL.cpp (B1).
//********************************************************************
//	You can get the sample code in "Tiff_STL4.h"
//********************************************************************
#include <iostream>
#include "stdafx.h"
#include "../Include/Tiff_STL4.h"

using namespace AV_Tiff_STL4;

//TIFF little endian magic: "II" + 42.
static const DWORD TiffMagicLE = 0x002A4949;

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
namespace AV_Tiff_STL4 {
	extern int DataType[FieldTypeSize];
}; //namespace AV_Tiff_STL4


Tiff::Tiff() :m_IFD_Offset(0)
{
	m_IFD.NextIFD = 8;
	//IO = nullptr;
}

Tiff::Tiff(LPCSTR FileName) :Tiff()
{
	ReadFile(FileName);
}

Tiff::~Tiff()
{
	Reset();
}

void Tiff::Reset()
{
	for (auto& pos : m_IFD.m_TagList)
		pos.reset();//TagListItem (unique_ptr/shared_ptr) releases the tag.

	m_IFD.m_TagList.clear();
}

//////////////////////////////////////////////////////////////////////
// Tiff Read File Operaton
//////////////////////////////////////////////////////////////////////
Tiff_Err Tiff::CheckFile(IO_INTERFACE* IO)
{
#if defined(VIRTUAL_IO_STL)
	fstream* file = reinterpret_cast<fstream*>(dynamic_cast<IO_fstream*>(IO)->GetHandle());
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

Tiff_Err Tiff::ReadFile(LPCSTR FileName)
{
	Reset();

	if (FileName == nullptr)
		throw " *** Tiff::ReadFile() --> FileName is nullptr. *** ";

	IO_INTERFACE* IO = IO_In(FileName);
	if (CheckFile(IO) != Tiff_OK)
	{
		if (IO != nullptr) // Tiff is not PC version, but the file is opened successfully.
			IO_Close(IO);
		throw FileName;
	}

	Tiff_Err ret = ReadTiff(IO);
	if (IO != NULL)
		IO_Close(IO);

	return ret;
}

Tiff_Err Tiff::ReadTiff(IO_INTERFACE* IO)
{
	DWORD	TiffVersion;
	WORD	TagCount;
	Tiff_Err ret = Tiff_OK;

	IO_Seek(0, SEEK_SET);
	IO_Read((LPBYTE)&TiffVersion, 1, 4);

	if (TiffVersion != TiffMagicLE)
	{//PC Version
		ret = VersionErr;
		//IO_Close(IO); //Don't close, it will been closed later.
		throw " *** Tiff is not PC version. *** ";
	}

	IO_Read((LPBYTE)&m_IFD_Offset, 1, 4);
	IO_Seek(m_IFD_Offset, SEEK_SET);
	IO_Read((LPBYTE)&TagCount, 1, 2);
	if (TagCount >= MAXTAG)
	{//Too many m_Tags
		ret = TooManyTags;
		return ret;
	}
	else if (TagCount == 0)
	{
		ret = TagNorFound;
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

Tiff_Err Tiff::SaveFile(LPCSTR FileName, int lzw)
{
	Tiff_Err ret = Tiff_OK;

	try {
		IO_INTERFACE* IO = IO_Out(FileName);
		if (IO == nullptr)
			throw "*** File(Save) Open Error. ***";
		if (m_IFD.m_TagList.size() == 0)
		{
			IO_Close(IO);
			throw "*** Tiff::SaveFile() --> TiffTag EntryCounts is 0. ***";
		}

		if(lzw == 1)
			ret = SaveTiff_lzw(IO);
		else if (lzw == 3 || lzw == 4)
			ret = SaveTiff_G3G4(IO, lzw);
		else
			ret = SaveTiff(IO);

		IO_Close(IO);
		return ret;
	}
	catch (const char* ErrMsg)
	{
		cout << ErrMsg << endl;
		return FileOpenErr;
	}
}

//Build the complete TIFF file image (header + IFD + tag data + image data +
//Exif data) in memory. ImageMode selects the image writer: 0 raw, 1 LZW blob,
//3/4 fax blob. Byte layout is identical to the previous sequential writers.
Tiff_Err Tiff::BuildFileImage(vector<BYTE>& img, int ImageMode)
{
	img.clear();
	auto put = [&img](const void* p, size_t n)
	{
		const BYTE* b = (const BYTE*)p;
		img.insert(img.end(), b, b + n);
	};

	//Header ("II", 42, IFD offset 8).
	BYTE header[8] = { 'I','I', 0x2A, 0x00, 8, 0, 0, 0 };
	put(header, 8);

	//Strip offset bookkeeping (was WriteIFD's mutation).
	int OffsetValue = 8 + 2 + (int)m_IFD.m_TagList.size() * 12 + 4;
	for (const auto& pos : m_IFD.m_TagList)
	{
		int DataSize = DataType[(int)pos->type] * pos->n;
		if (DataSize > 4)
		{
			pos->value = OffsetValue;
			OffsetValue += DataSize;
		}
	}

	TiffTagPtr TempTag = GetTag(StripOffsets);
	if (TempTag->n != 1)
	{
		LPDWORD pOffset = (LPDWORD)(TempTag->lpData);
		for (DWORD i = 0; i < TempTag->n; ++i)
			pOffset[i] += OffsetValue;

		TiffTagPtr TempTag2 = GetTag(StripByteCounts);
		LPDWORD lpStripByteCounts = (LPDWORD)TempTag2->lpData;
		OffsetValue = pOffset[TempTag->n - 1] + lpStripByteCounts[TempTag2->n - 1];
	}
	else
	{
		TempTag->value = OffsetValue;
		OffsetValue += GetTagValue(StripByteCounts);
	}

	TiffTagPtr ExifTag = GetTag(Exif_IFD);
	if (ExifTag != nullptr)
		ExifTag->value = OffsetValue;

	//IFD.
	int EntryCounts = (int)m_IFD.m_TagList.size();
	WORD EntryCountsW = (WORD)EntryCounts;
	put((LPBYTE)&EntryCountsW, sizeof(WORD));
	for (const auto& pos : m_IFD.m_TagList)
	{
		DWORD packed = (DWORD)((int)(pos->tag) | ((int)(pos->type) << 16));
		put((LPBYTE)&packed, sizeof(DWORD));
		put((LPBYTE)&pos->n, sizeof(DWORD));
		put((LPBYTE)&pos->value, sizeof(DWORD));
	}
	DWORD NextIFD = 0;
	put((LPBYTE)&NextIFD, sizeof(DWORD));

	//Tag data blobs (tag list order).
	for (const auto& pos : m_IFD.m_TagList)
	{
		int DataSize = DataType[(int)pos->type] * pos->n;
		if (DataSize > 4 && pos->lpData != nullptr)
			put(pos->lpData, DataSize);
	}

	//Image data (modes 1/3/4 keep the compressed blob in the tag slot).
	{
		TiffTagPtr tagStripOffsets = GetTag(StripOffsets);
		if (ImageMode >= 1)
		{
			StripOffsetsTag* offset = (StripOffsetsTag*)GetPtr(tagStripOffsets);
			//Multi strip LZW: the blob size is the SUM of the per strip counts.
			DWORD total = GetTagValue(StripByteCounts);
			TiffTagPtr cntTag = GetTag(StripByteCounts);
			if (cntTag->n > 1 && cntTag->lpData != nullptr)
			{
				total = 0;
				LPDWORD c = (LPDWORD)cntTag->lpData;
				for (DWORD i = 0; i < cntTag->n; i++)
					total += c[i];
			}
			put(offset->GetLzwData(), total);
		}
		else
			put(tagStripOffsets->lpData, GetTagValue(StripByteCounts));
	}

	//Exif data.
	if (ExifTag != nullptr)
		put(ExifTag->lpData, Exif_IFD_Tag::ExifBufSize);

	return Tiff_OK;
}

Tiff_Err Tiff::SaveTiff(IO_INTERFACE* IO)
{
	vector<BYTE> img;
	Tiff_Err ret = BuildFileImage(img, 0);
	if (ret != Tiff_OK) return ret;
	IO_Seek(0, SEEK_SET);
	IO_Write(&img[0], 1, img.size());
	return ret;
}

Tiff_Err Tiff::SaveRaw(LPCSTR FileName)
{
	Tiff_Err ret = Tiff_OK;

	try {
		IO_INTERFACE* IO = IO_Out(FileName);
		if (IO == nullptr)
			throw "*** File(Save) Open Error. ***";
		if (m_IFD.m_TagList.size() == 0)
		{
			IO_Close(IO);
			throw "*** Tiff::SaveFile() --> TiffTag EntryCounts is 0. ***";
		}
		int Width = GetTagValue(ImageWidth);
		int Length = GetTagValue(ImageLength);
		int Samples = GetTagValue(SamplesPerPixel);
		int Bits = GetTagValue(BitsPerSample);
		cout << endl << "Width:" << Width << " Length:" << Length << " Samples:" << Samples << " Bits:" << Bits << endl;
		TiffTagPtr Tag = GetTag(StripOffsets);
		IO_Write(Tag->lpData, 1, GetTagValue(StripByteCounts));
		IO_Close(IO);
		return ret;
	}
	catch (const char* ErrMsg)
	{
		cout << ErrMsg << endl;
		return FileOpenErr;
	}
}

//Replace the data pointer of a tag and release the previous buffer (A4).
static void SetTagLpData(TiffTagPtr Tag, LPBYTE lpBuf)
{
	if (Tag->lpData != nullptr)
		delete[]Tag->lpData;
	Tag->lpData = lpBuf;
}

Tiff_Err Tiff::ReadImage(IO_INTERFACE* IO)
{
	Tiff_Err ret = Tiff_OK;
	//if (GetTagValue(Compression) != 1)//No compress
	//	throw CompressData;

	DWORD Compress = GetTagValue(Compression);
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
		m_IFD.m_TagList.push_back(TagListItem(new TiffTag(RowsPerStrip, Short, 1, Length)));
	}

	//Supported combinations:
	//  Single strip (Length == rowsPerStrip)
	//    Planar 0/1 : Compress 1 direct read, 5 LZW, 3/4 G3/G4.
	//    Planar 2   : bits==1 Avision CMYKcm 1bit direct read (undocumented),
	//                 otherwise PhotoShop CS2 planar, read + Pack + Planar reset.
	//  Multi strip  : Compress 1 ReadMultiStripOffset, 5 LZW, 3/4 G3/G4.
	if (Length == rowsPerStrip)
	{//Single Strip
		if ((planarConfiguration == 0) || (planarConfiguration == 1))
		{//RGBRGB
			DWORD stripByteCounts = GetTagValue(StripByteCounts);
			if (Compress == 1)
			{
				UINT64 size = ((UINT64)Width * samplesPerPixel * bitsPerSample + 7) / 8 * Length;
				if (size > 0xFFFFFFFF)
					throw " *** Image Size is too big, size > 0xFFFFFFFF *** ";
				stripByteCounts = (DWORD)size;
				lpImageBuf = new BYTE[stripByteCounts];
				IO_Seek(stripOffsets, SEEK_SET);
				IO_Read(lpImageBuf, 1, stripByteCounts);
				//Set ImageBuf address to StripOffset Tag
				SetTagLpData(GetTag(StripOffsets), lpImageBuf);
			}
			else if (Compress == 5)
			{
				ReadSingleStripOffset_LZW(IO);
			}
			else if (Compress == 3 || Compress == 4)
			{
				ReadG3G4Strips(IO, true, (int)Compress);
			}
		}
		else
		{//PlanarConfiguration) == 2, 
			if (bitsPerSample == 1)
			{//Undocument, just for avision, CMYKcm(4 or 6) 1bit.
				DWORD stripByteCounts = (Width >> 3) * Length * samplesPerPixel;
				lpImageBuf = new BYTE[stripByteCounts];
				IO_Seek(stripOffsets, SEEK_SET);
				IO_Read(lpImageBuf, 1, stripByteCounts);
				//Set ImageBuf address to StripOffset Tag
				SetTagLpData(GetTag(StripOffsets), lpImageBuf);
			}
		else
		{//RRRGGGBBB, For PhotoShop CS2
			TiffTagPtr TagOffsets = GetTag(StripOffsets);
			TiffTagPtr TagCounts = GetTag(StripByteCounts);
			if (TagOffsets->lpData == nullptr || TagCounts->lpData == nullptr)
			{//Single strip with inline (count=1) tags, ReadMultiStripOffset can not be used.
				UINT64 size = (UINT64)Width * Length * samplesPerPixel * bitsPerSample / 8;
				if (size > 0xFFFFFFFF)
					throw " *** Image Size is too big, size > 0xFFFFFFFF *** ";
				lpImageBuf = new BYTE[(DWORD)size];
				IO_Seek(stripOffsets, SEEK_SET);
				IO_Read(lpImageBuf, 1, (DWORD)size);
				//Set ImageBuf address to StripOffset Tag
				SetTagLpData(TagOffsets, lpImageBuf);
			}
			else
				ReadMultiStripOffset(IO);

			if (bitsPerSample == 8)
				Pack<BYTE>(Width, Length);
			else
				Pack<WORD>(Width, Length);

			//Reset PlanarConfiguration) -> 1.
			SetTagValue(PlanarConfiguration, 1);
		}
		}
	}
	else if (Compress == 1)//No compress		
			ReadMultiStripOffset(IO);
#if LZW
	else if (Compress == 5)
		ret = ReadMultiStripOffset_LZW(IO);
#endif //LZW
	else if (Compress == 3 || Compress == 4)
		ret = ReadG3G4Strips(IO, false, (int)Compress);

	return ret;
}

Tiff_Err Tiff::ReadMultiStripOffset(IO_INTERFACE* IO)
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

	//Reset StripOffsets) and StripByteCounts)
	ResetStripTags(lpImageBuf, stripByteCounts);

	return Tiff_OK;
}

//Shared tail of the strip readers: swap the strip tags over to the assembled
//image buffer and normalize RowsPerStrip (and optionally Compression).
Tiff_Err Tiff::ResetStripTags(LPBYTE lpImageBuf, DWORD TotalBytes, bool SetCompression)
{
	TiffTagPtr TagStripOffsets = GetTag(StripOffsets);
	delete[]TagStripOffsets->lpData;
	TagStripOffsets->lpData = lpImageBuf;
	TagStripOffsets->n = 1;
	TagStripOffsets->type = Long;
	//TagStripOffsets->value = lpImageBuf;//Don't care, It mean's nothing.

	TiffTagPtr TagStripByteCounts = GetTag(StripByteCounts);
	delete[]TagStripByteCounts->lpData;
	TagStripByteCounts->lpData = nullptr;
	TagStripByteCounts->type = Short;
	TagStripByteCounts->n = 1;
	TagStripByteCounts->value = TotalBytes;

	//Reset RowsPerStrip), it should be the same with Length;
	SetTagValue(RowsPerStrip, GetTagValue(ImageLength));
	if (SetCompression)
		SetTagValue(Compression, 1);

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






//Not Fully Test yet...
#if defined(VIRTUAL_IO) | defined(VIRTUAL_IO_STL)
Tiff_Err Tiff::SaveMemory(LPBYTE Buffer, size_t BufSize, size_t& SaveSize)
{
	vector<BYTE> img;
	Tiff_Err ret = BuildFileImage(img, 0);
	SaveSize = (img.size() > BufSize) ? BufSize : img.size();
	if (ret == Tiff_OK)
		memcpy(Buffer, &img[0], SaveSize);
	return ret;
}

//Not Fully Test yet...
Tiff_Err Tiff::ReadMemory(LPBYTE Buffer, size_t BufSize)
{
	IO_INTERFACE* IO = new IO_Buf(Buffer, BufSize);
	Tiff_Err ret = ReadTiff(IO);
	IO_Close(IO);
	return ret;
}
#endif//(VIRTUAL_IO) | (VIRTUAL_IO_STL)

//////////////////////////////////////////////////////////////////////
// CTiff
//////////////////////////////////////////////////////////////////////
