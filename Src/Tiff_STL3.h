/*___________________________________  Tiff_STL3.h   ___________________________________*/

/*       1         2         3         4         5         6         7         8        */
/*34567890123456789012345678901234567890123456789012345678901234567890123456789012345678*/
/*******************************************|********************************************/
/*
*   Copyright (c) 2015  Avision Corporation All rights reserved.
*
*   Copyright protection claimed includes all forms and matters of
*   copyrightable material and information now allowed by statutory or judicial
*   law or hereinafter granted, including without limitation, material generated
*   from the software programs which are displayed on the screen such as icons,
*   screen display looks, etc.
*
*******************************************|********************************************/
/**
* @defgroup	Tiff_Module Tiff Module (STL Version 3).
* @brief		Tiff Image Module.
*
* @details
*
* For more infomation, you need to read this document.\n
* @ref TiffDocument
*
* Simple Example: This example is a template of how to using this class to do a image processing.\n
* @ref TiffExample
* @image html  ..\sheep2.jpg
*
* @page pageTiff Tiff documentation & Examples
*
* <CENTER><b>Document and Example.</b></CENTER>
*
* @section secTiffDocumentExample Tiff Document and Example.
* This page contains the subsections \ref TiffDocument.\n
* For more info see page \ref TiffExample.
*
* @subsection TiffDocument Tiff document.
* - <a href="..\Tiff_STL\Tiff6.pdf">Tiff Document(Ver 6.0)</a>
*
* Sender class. Can be used to send a command to the server.

The receiver will acknowlegde the command by calling Ack().
\msc
Sender,Receiver;
Sender->Receiver [label="Command()", URL="\ref Receiver::Command()"];
Sender<-Receiver [label="Ack()", URL="\ref Ack()", ID="1"];
\endmsc

* @subsection TiffExample Tiff Example.
* @code
#include "stdafx.h"
#include <string>
#include <iostream>
using namespace std;
#include "Tiff_STL3.h" //Has claimed the namespace...
//using namespace AV_Tiff_STL3; Don't need anymore...
main(int argc, _TCHAR* argv[]))
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
		Process(lpBuf, Width); //Add your process here.
		lpOut->PutRow(lpBuf, i);
	}
	delete []lpBuf;

	lpOut->SaveFile("Output.tif");
}
*
* @endcode
*/

#if !defined(_TIFF_STL3_)
#define _TIFF_STL3_

#if !defined(SYS_INFO)
#include "SysInfo.h"
#endif //SYS_INFO

/***************************************************************************
Virtual IO
***************************************************************************/
//Don't define Virtual_IO in Linux enviornment
//define __IO_Interface_H__ 
//#define VIRTUAL_IO_STL	

#ifdef _WINDOWS //Windows
#include <windows.h>
#pragma warning(disable : 4996)//for Net

#if defined(__IO_Interface_H__)//Slowest, it should be the same with fopen, but not the true.
#include <SysInfo\Virtual_IO.h>		
#define IO_In(FileName)					new IO_File(FileName, "rb")
#define IO_Out(FileName)				new IO_File(FileName, "wb")
#define IO_Close(IO)					delete IO
#define IO_Read(Str, Size, Count)		IO->Read(Str, Size, Count)
#define IO_Write(Str, Size, Count)		IO->Write(Str, Size, Count)
#define IO_Seek(Offset, Origin)			IO->Seek(Offset, Origin)
#define IO_Tell()						IO->Tell()
#define IO_GetHandle()					IO->GetHandle()

#elif defined(VIRTUAL_IO_STL)//Fastest
#include <fstream>
#include <SysInfo\Virtual_IO.h>

#define IO_In(FileName)					new IO_fstream(FileName, ios::in)
#define IO_Out(FileName)				new IO_fstream(FileName, ios::out)
#define IO_Close(IO)					delete IO
#define IO_Read(Str, Size, Count)		IO->Read(Str, Size, Count)
#define IO_Write(Str, Size, Count)		IO->Write(Str, Size, Count)
#define IO_Seek(Offset, Origin)			IO->Seek(Offset, Origin)
#define IO_Tell()						IO->Tell()
#define IO_GetHandle()					IO->GetHandle()

#else //Almost the same with VIRTUAL_IO_STL
#define IO_INTERFACE					FILE //Type
#define IO								File //Argument
#define IO_In(FileName)					fopen(FileName, "rb")
#define IO_Out(FileName)				fopen(FileName, "wb+")
#define IO_Close(File)					fclose(File)
#define IO_Read(Str, Size, Count)		fread(Str, Size, Count, File)
#define IO_Write(Str, Size, Count)		fwrite(Str, Size, Count, File)
#define IO_Seek(Offset, Origin)			fseek(File, Offset, Origin)
#define IO_Tell()						ftell(File)
#endif //VIRTUAL_IO

#else //Linux
#if !defined(__IO_Interface_H__)
#define IO_INTERFACE					FILE //Type
#define IO								File //Argument
#define IO_In(FileName)					fopen(FileName, "rb")
#define IO_Out(FileName)				fopen(FileName, "wb+")
#define IO_Close(File)					fclose(File)
#define IO_Read(Str, Size, Count)		fread(Str, Size, Count, File)
#define IO_Write(Str, Size, Count)		fwrite(Str, Size, Count, File)
#define IO_Seek(Offset, Origin)			fseek(File, Offset, Origin)
#define IO_Tell()						ftell(File)
#endif
#endif //WIN32

#define MAXTAG 40

#include <vector>
#include <functional>
#include <algorithm>
#include <iostream>
#include <memory>

#ifdef FIXED_VECTOR
	#include <SysInfo/FixedAllocator.h>
#endif //FIXED_VECTOR

using namespace std;

/**
* @ingroup Tiff_Module
* @namespace	AV_Tiff_STL3
* @brief		Service of CTiff image operation.
*
* @Detailed
* When you using this library, just include the header file.\n
* Header file has already declaire the namespace AV_Tiff_STL3.\n
* Interface for the CTiff_STL class.\n
*/
namespace AV_Tiff_STL3 {

	/***************************************************************************
	***************************************************************************/
	enum class TiffTagSignature {
		eNullTag = 0x0000L,
		eNewSubfileType = 0x00FEL,
		eSubfileType = 0x00FFL,
		eImageWidth = 0x0100L,
		eImageLength = 0x0101L,
		eBitsPerSample = 0x0102L,
		eCompression = 0x0103L,
		ePhotometricInterpretation = 0x0106L,
		eThreshholding = 0x0107L,
		eCellWidth = 0x0108L,
		eCellLength = 0x0109L,
		eFillOrder = 0x010AL,
		eDocumentName = 0x010DL,
		eImageDescription = 0x010EL,
		eMake = 0x010FL,
		eModel = 0x0110L,
		eStripOffsets = 0x0111L,
		eOrientation = 0x0112L,
		eSamplesPerPixel = 0x0115L,
		eRowsPerStrip = 0x0116L,
		eStripByteCounts = 0x0117L,
		eMinSampleValue = 0x0118L,
		eMaxSampleValue = 0x0119L,
		eXResolution = 0x011AL,
		eYResolution = 0x011BL,
		ePlanarConfiguration = 0x011CL,
		ePageName = 0x011DL,
		eXPosition = 0x011EL,
		eYPosition = 0x011FL,
		eFreeOffsets = 0x0120L,
		eFreeByteCounts = 0x0121L,
		eGrayResponseUnit = 0x0122L,
		eGrayResponsCurve = 0x0123L,
		eT4Options = 0x0124L,
		eT6Options = 0x0125L,
		eResolutionUnit = 0x0128L,
		ePageNumber = 0x0129L,
		eTransferFunction = 0x012DL,
		eSoftware = 0x0131L,
		eDateTime = 0x0132L,
		eArtist = 0x013BL,
		eHostComputer = 0x013CL,
		ePredicator = 0x013DL,
		eWhitePoint = 0x013EL,
		ePrimaryChromaticities = 0x013FL,
		eColorMap = 0x0140L,
		eHalftoneHints = 0x0141L,
		eTileWidth = 0x0142L,
		eTileLength = 0x0143L,
		eTileOffsets = 0x0144L,
		eTileByteCounts = 0x0145L,
		eInkSet = 0x014CL,
		eInkNames = 0x014DL,
		eNumberOfInks = 0x014EL,
		eDotRange = 0x0150L,
		eTargetPrinter = 0x0151L,
		eExtraSamples = 0x0152L, //Alpha channel : turn on (1)
		eSampleFormat = 0x0153L,
		eSMinSampleValue = 0x0154L,
		eSMaxSampleValue = 0x0155L,
		eTransforRange = 0x0156L,
		eJPEGProc = 0x0157L,
		eJPEGInterchangeFormat = 0x0201L,
		eJPEGIngerchangeFormatLength = 0x0202L,
		eJPEGRestartInterval = 0x0203L,
		eJPEGLosslessPredicators = 0x0205L,
		eJPEGPointTransforms = 0x0206L,
		eJPEGQTable = 0x0207L,
		eJPEGDCTable = 0x0208L,
		eJPEGACTable = 0x0209L,
		eYCbCrCoefficients = 0x0211L,
		eYCbCrSampling = 0x0212L,
		eYCbCrPositioning = 0x0213L,
		eReferenceBlackWhite = 0x0214L,
		eXML_Data = 0x02BCL,
		eCopyRight = 0x8298L,
		eIPTC = 0x83BBL,/*!< IPTC (International Press Telecommunications Council) metadata.*/
		ePhotoshop = 0x8649L,
		eExif_IFD = 0x8769L,
		eIccProfile = 0x8773L
	};

	/***************************************************************************
	***************************************************************************/
	enum class FieldType {
		eUnknownType = 0x0000L,
		eByte = 0x0001L,
		eASCII = 0x0002L,
		eShort = 0x0003L,
		eLong = 0x0004L,
		eRational = 0x0005L,
		eSBYTE = 0x0006L,
		eUndefineType = 0x0007L,
		eSShort = 0x0008L,
		eSLong = 0x0009L,
		eFloat = 0x000AL,
		eDouble = 0x000BL,
		eUnknown1 = 0x000CL,//Never using, Just for code analysis warning.
		eUnknown2 = 0x000DL,
		eUnknown3 = 0x000EL,
		eUnknown4 = 0x000FL,
	};

#define FieldTypeSize 16
	/***************************************************************************
	***************************************************************************/
	enum class Tiff_Err {
		eTiff_OK = 0,
		eFileOpenErr = -1,
		eVersionErr = -2,/*!< Check Win or Mac version.*/
		eTooManyTags = -3,/*!< Tags > MaxTag; */
		eTagStripErr = -4,/*!< StripByteCount.n != StripOffset.n;*/
		eMemoryAllocFail = -5,
		eDataTypeErr = -6,
		eCompressData = -7,
		eUnDefineErr = -9999,
		eTiff_NEW_TAG = 1
	};

	/***************************************************************************
	***************************************************************************/
	/*! \class TiffTag
	* This basic class is using for tiff operation.
	*
	* \par lpData:
	* The real data, if the offset is an offset.
	*/

	class TiffTag
	{
	public:
		TiffTag();
		virtual ~TiffTag();
		TiffTag(const TiffTag& Tag); //Copy Constructor
		TiffTag(TiffTag&& other) noexcept; // move constructor
		TiffTag& operator=(const TiffTag& other); // copy assignment
		TiffTag& operator=(TiffTag&& other) noexcept; // move assignment

		TiffTag(TiffTagSignature Signature);
		TiffTag(DWORD SigType, DWORD n, DWORD value, IO_INTERFACE* IO);
		TiffTag(TiffTagSignature Tag, FieldType Type, DWORD n, DWORD value, LPBYTE lpBuf = nullptr);
		bool operator()(const TiffTag& Data) const { return tag == Data.tag; };
		virtual DWORD GetValue() const;
		virtual int SaveFile(IO_INTERFACE* IO);
		virtual LPBYTE GetData() const { return lpData; }
		int ValueIsOffset() const;

		//! The Tag that identifies the field.
		TiffTagSignature	tag;

		//!	The field Type. 
		FieldType			type;

		//! The number of values, Count of the indicated Type.
		DWORD 				n;

		//! Value or Offset.
		/*
		!The Value Offset, the file offset (in bytes) of the Value for the field.
		The Value is expected to begin on a word boundary; the corresponding
		Value Offset will thus be an even number. This file offset may
		point anywhere in the file, even after the image data.
		*/
		DWORD				value;

		//! The real data, if the value is offset.
		LPBYTE				lpData;
	};

	/***************************************************************************
	***************************************************************************/
	//Special Tag
	// Fix the method signature to match the base class method it is intended to override.
	class BitsPerSampleTag : public TiffTag
	{
	public:
		BitsPerSampleTag(DWORD SigType, DWORD n, DWORD value, IO_INTERFACE* IO);
		DWORD GetValue() const override;
	};

	class ResolutionTag :public TiffTag
	{
	public:
		ResolutionTag(DWORD SigType, DWORD n, DWORD value, IO_INTERFACE* IO);
		virtual DWORD GetValue() const override;
	};

	class Exif_IFD_Tag :public TiffTag
	{
	public:
		Exif_IFD_Tag(DWORD SigType, DWORD n, DWORD value, IO_INTERFACE* IO);
		//***Don't save anything. This tag broke all rule, just skip it.
		static int ExifBufSize;//Because the bufsize is out of control, type is long, n is always 1.
	};

	class TAG//funtor, For find_if
	{
	public:
		TAG(TiffTagSignature signature) { Signature = signature; };
		bool operator()(const TiffTag* Tag) const { return Tag->tag == Signature; }
		TiffTagSignature Signature;
	};

#if SMART_POINTER
	//typedef shared_ptr<TiffTag> TiffTagPtr;
	using TiffTagPtr = shared_ptr<TiffTag>;
#else
	typedef TiffTag* TiffTagPtr;
#endif //SMART_POINTER

	//	typedef vector<TiffTagPtr> TagList;
	//	typedef TagList::iterator TiffTag_iter;
#ifdef FIXED_VECTOR
	using  TagList = FixedVector<TiffTagPtr, MAXTAG>;
#else
	using  TagList = vector<TiffTagPtr>;
#endif //FIXED_VECTOR

	using  TiffTag_iter = TagList::iterator;

#define TiffTag_Begin m_IFD.m_TagList.begin()
#define TiffTag_End m_IFD.m_TagList.end()	

	/**
	* @class Tag_equal
	* @brief funtor, For find_if
	*
	* @attention
	* The VC6 seems can't compiler STL source code.
	* If you want to using STL, just define STL in preprocessor.
	*/


	/***************************************************************************
	***************************************************************************/
	struct IFD_STRUCTURE
	{
		IFD_STRUCTURE();

		TagList		m_TagList;
		DWORD		NextIFD;
	};

	/***************************************************************************
	***************************************************************************/
	/**
	* @class Tiff
	* @brief Just Read and Write tiff file.
	*
	* @note Attention!!!
	* The VC6 seems can't compiler STL source code.
	* If you want to using STL, just define STL in preprocessor.
	*/
	class Tiff
	{
	public:
		Tiff();
		Tiff(LPCSTR FileName);
		virtual		~Tiff();
		virtual		void Reset();
		virtual		Tiff_Err CheckFile(IO_INTERFACE* IO);
		virtual		Tiff_Err ReadTiff(IO_INTERFACE* IO);
		virtual		Tiff_Err SaveTiff(IO_INTERFACE* IO);
		virtual		Tiff_Err ReadFile(LPCSTR FileName);
		virtual		Tiff_Err SaveFile(LPCSTR FileName);
		virtual		Tiff_Err SaveRaw(LPCSTR FileName);

#if defined (VIRTUAL_IO) | defined(VIRTUAL_IO_STL)
		virtual		Tiff_Err ReadMemory(LPBYTE Buffer, size_t BufSize);
		virtual		Tiff_Err SaveMemory(LPBYTE Buffer, size_t BufSize, size_t& SaveSize);
#endif //VIRTUAL_IO

		//	Tag Operation
		TiffTagPtr	GetTag(const TiffTagSignature Signature);
		DWORD		GetTagValue(const TiffTagSignature Signature);
		Tiff_Err	SetTag(TiffTagPtr NewTag);
		Tiff_Err	SetTagValue(const TiffTagSignature Signature, DWORD Value);

	protected:
		//Read Image
		virtual		TiffTagPtr	CreateTag(DWORD SignatureType, DWORD n, DWORD value, IO_INTERFACE* IO);
		void		AddTags(DWORD TypeSignature, DWORD n, DWORD value, IO_INTERFACE* IO);
		Tiff_Err	ReadImage(IO_INTERFACE* IO);
		Tiff_Err	ReadMultiStripOffset(IO_INTERFACE* IO);

		template<class T>
		void		Pack(int Width, int Length);

		//Write Image
		virtual		Tiff_Err	WriteHeader(IO_INTERFACE* IO);
		Tiff_Err	WriteIFD(IO_INTERFACE* IO);
		Tiff_Err	WriteTagData(IO_INTERFACE* IO);
		Tiff_Err	WriteImageData(IO_INTERFACE* IO);
		Tiff_Err	WriteData_Exif_IFD_Tag(IO_INTERFACE* IO);

		DWORD			m_IFD_Offset;
		IFD_STRUCTURE	m_IFD;
	};

	/***************************************************************************
	***************************************************************************/
	class CTiff :public Tiff
	{
	public:
		CTiff();
		CTiff(LPCSTR FileName);
		CTiff(string FileName);
		CTiff(int width, int length, int resolution, int samplesperpixel, int bitspersample, int AllocBuf = 1);
		virtual		~CTiff();
		virtual		Tiff_Err	ReadTiff(IO_INTERFACE* IO);
		virtual		Tiff_Err	ReadFile(LPCSTR FileName);
		virtual		Tiff_Err	ReadFile(string FileName);

#if defined(VIRTUAL_IO) | defined(VIRTUAL_IO_STL)
		virtual		Tiff_Err ReadMemory(LPBYTE Buffer, size_t BufSize);
#endif //VIRTUAL_IO

		CTiff* Clone(bool copy = false);
		Tiff_Err	CreateNew(int width, int length, int resolution, int samplesperpixel, int bitspersample, int AllocBuf = 1);
		Tiff_Err	CreateNew(int width, int length, int resolution, int samplesperpixel, int bitspersample, LPCSTR InName);
		Tiff_Err	CreateNew(int width, int length, int resolution, int samplesperpixel, int bitspersample, LPCSTR InName, LPCSTR OutName);
		Tiff_Err	SetTag(TiffTagSignature Signature, FieldType type, DWORD n, DWORD value, LPBYTE lpBuf = nullptr);
		Tiff_Err	SetTagValue(const TiffTagSignature Signature, DWORD Value);

		//operation
		template<class T>
		int	GetRow(T* lpBuf, int Line, int pixel);
		int	GetRow(LPBYTE lpBuf, int Line, int pixel = 0);
		int	GetRow(LPWORD lpBuf, int Line, int pixel = 0);
		template<class T>
		int	PutRow(T* lpBuf, int Line, int pixel);
		int	PutRow(LPBYTE lpBuf, int Line, int pixel = 0);
		int	PutRow(LPWORD lpBuf, int Line, int pixel = 0);
		template<class T>
		int	GetRowColumn(T* lpBuf, int x, int y, int RecX, int RecY);
		int	GetRowColumn(LPBYTE lpBuf, int x, int y, int RecX, int RecY);
		int	GetRowColumn(LPWORD lpBuf, int x, int y, int RecX, int RecY);
		template<class T>
		int	SetRowColumn(T* lpBuf, int x, int y, int RecX, int RecY);
		int	SetRowColumn(LPBYTE lpBuf, int x, int y, int RecX, int RecY);
		int	SetRowColumn(LPWORD lpBuf, int x, int y, int RecX, int RecY);

		LPBYTE GetRowIndex(int Line) { return GetXY(0, Line); };
		LPBYTE GetRowIndex_Q(int Line) { return GetXY_Q(0, Line); };

		//Coordinate operation. M:Math, Q:Quick(Not Check Boundary)
		LPBYTE		GetXY(int X, int Y);
		void		SetXY(int X, int Y, BYTE Value) { *(GetXY(X, Y)) = Value; };
		LPBYTE		GetXY_M(int X, int Y);
		void		SetXY_M(int X, int Y, BYTE Value) { *(GetXY_M(X, Y)) = Value; };
		LPBYTE		GetXY_Q(int X, int Y);
		void		SetXY_Q(int X, int Y, BYTE Value) { *(GetXY_Q(X, Y)) = Value; };
		LPBYTE		GetXY_MQ(int X, int Y);
		void		SetXY_MQ(int X, int Y, BYTE Value) { *(GetXY_MQ(X, Y)) = Value; };

		//Icc profile Operation.
		Tiff_Err	SetIccProfile(char* IccFile);
		void		SaveIccProfile(char* OutIccFile);
		void		RemoveIcc();

		//Image Buffer operation.
		void		SetImageBuf(LPBYTE lpBuf, bool FreeBuf = true);
		void		ForgetImageBuf();
		LPBYTE		GetImageBuf();

#ifdef TIFF_EXT
		Tiff_Err	ReadJPG(LPCSTR FileName);
		Tiff_Err	SaveJPG(LPCSTR FileName);
		Tiff_Err	ReadFile_Ext(LPCSTR FileName);
#endif //TIFF_EXT

	private:
		//useful property
		int m_Width, m_Length, m_SamplesPerPixel, m_BitsPerSample, m_BytesPerLine, m_Resolution;
		LPBYTE m_lpImageBuf;
	};
}//namespace AV_Tiff_STL3 or AV_Tiff

using namespace AV_Tiff_STL3;

//For fix C26812, for VS2019
#define NullTag								TiffTagSignature::eNullTag
#define NewSubfileType						TiffTagSignature::eNewSubfileType
#define SubfileType							TiffTagSignature::eSubfileType
#define ImageWidth							TiffTagSignature::eImageWidth
#define ImageLength							TiffTagSignature::eImageLength
#define BitsPerSample						TiffTagSignature::eBitsPerSample
#define Compression							TiffTagSignature::eCompression
#define PhotometricInterpretation			TiffTagSignature::ePhotometricInterpretation
#define Threshholding						TiffTagSignature::eThreshholding
#define CellWidth							TiffTagSignature::eCellWidth
#define CellLength							TiffTagSignature::eCellLength
#define FillOrder							TiffTagSignature::eFillOrder
#define DocumentName						TiffTagSignature::eDocumentName
#define ImageDescription					TiffTagSignature::eImageDescription
#define Make								TiffTagSignature::eMake
#define Model								TiffTagSignature::eModel
#define StripOffsets						TiffTagSignature::eStripOffsets
#define Orientation							TiffTagSignature::eOrientation
#define SamplesPerPixel						TiffTagSignature::eSamplesPerPixel
#define RowsPerStrip						TiffTagSignature::eRowsPerStrip
#define StripByteCounts						TiffTagSignature::eStripByteCounts
#define MinSampleValue						TiffTagSignature::eMinSampleValue
#define MaxSampleValue						TiffTagSignature::eMaxSampleValue
#define XResolution							TiffTagSignature::eXResolution
#define YResolution							TiffTagSignature::eYResolution
#define PlanarConfiguration					TiffTagSignature::ePlanarConfiguration
#define PageName							TiffTagSignature::ePageName
#define XPosition							TiffTagSignature::eXPosition
#define YPosition							TiffTagSignature::eYPosition
#define FreeOffsets							TiffTagSignature::eFreeOffsets
#define FreeByteCounts						TiffTagSignature::eFreeByteCounts
#define GrayResponseUnit					TiffTagSignature::eGrayResponseUnit
#define GrayResponsCurve					TiffTagSignature::eGrayResponsCurve
#define T4Options							TiffTagSignature::eT4Options
#define T6Options							TiffTagSignature::eT6Options
#define ResolutionUnit						TiffTagSignature::eResolutionUnit
#define PageNumber							TiffTagSignature::ePageNumber
#define TransferFunction					TiffTagSignature::eTransferFunction
#define Software							TiffTagSignature::eSoftware
#define DateTime							TiffTagSignature::eDateTime
#define Artist								TiffTagSignature::eArtist
#define HostComputer						TiffTagSignature::eHostComputer
#define Predicator							TiffTagSignature::ePredicator
#define TiffWhitePoint						TiffTagSignature::eWhitePoint
#define PrimaryChromaticities				TiffTagSignature::ePrimaryChromaticities
#define ColorMap							TiffTagSignature::eColorMap
#define HalftoneHints						TiffTagSignature::eHalftoneHints
#define TileWidth							TiffTagSignature::eTileWidth
#define TileLength							TiffTagSignature::eTileLength
#define TileOffsets							TiffTagSignature::eTileOffsets
#define TileByteCounts						TiffTagSignature::eTileByteCounts
#define InkSet								TiffTagSignature::eInkSet
#define InkNames							TiffTagSignature::eInkNames
#define NumberOfInks						TiffTagSignature::eNumberOfInks
#define DotRange							TiffTagSignature::eDotRange
#define TargetPrinter						TiffTagSignature::eTargetPrinter
#define ExtraSamples						TiffTagSignature::eExtraSamples //Alpha channel : turn on (1)
#define SampleFormat						TiffTagSignature::eSampleFormat
#define SMinSampleValue						TiffTagSignature::eSMinSampleValue
#define SMaxSampleValue						TiffTagSignature::eSMaxSampleValue
#define TransforRange						TiffTagSignature::eTransforRange
#define JPEGProc							TiffTagSignature::eJPEGProc
#define JPEGInterchangeFormat				TiffTagSignature::eJPEGInterchangeFormat
#define JPEGIngerchangeFormatLength			TiffTagSignature::eJPEGIngerchangeFormatLength
#define JPEGRestartInterval					TiffTagSignature::eJPEGRestartInterval
#define JPEGLosslessPredicators				TiffTagSignature::eJPEGLosslessPredicators
#define JPEGPointTransforms					TiffTagSignature::eJPEGPointTransforms
#define JPEGQTable							TiffTagSignature::eJPEGQTable
#define JPEGDCTable							TiffTagSignature::eJPEGDCTable
#define JPEGACTable							TiffTagSignature::eJPEGACTable
#define YCbCrCoefficients					TiffTagSignature::eYCbCrCoefficients
#define YCbCrSampling						TiffTagSignature::eYCbCrSampling
#define YCbCrPositioning					TiffTagSignature::eYCbCrPositioning
#define ReferenceBlackWhite					TiffTagSignature::eReferenceBlackWhite
#define XML_Data							TiffTagSignature::eXML_Data
#define CopyRight							TiffTagSignature::eCopyRight
#define IPTC								TiffTagSignature::eIPTC
#define Photoshop							TiffTagSignature::ePhotoshop
#define Exif_IFD							TiffTagSignature::eExif_IFD
#define IccProfile							TiffTagSignature::eIccProfile

#define UnknownType							FieldType::eUnknownType
#define	Byte								FieldType::eByte
#define	ASCII								FieldType::eASCII
#define	Short								FieldType::eShort
#define	Long								FieldType::eLong
#define	Rational							FieldType::eRational
#define	SBYTE								FieldType::eSBYTE
#define	UndefineType						FieldType::eUndefineType
#define	SShort								FieldType::eSShort
#define	SLong								FieldType::eSLong
#define	Float								FieldType::eFloat
#define	Double								FieldType::eDouble
#define	Unknown1							FieldType::eUnknown1
#define	Unknown2							FieldType::eUnknown2
#define	Unknown3							FieldType::eUnknown3
#define	Unknown4							FieldType::eUnknown4

#define Tiff_OK								Tiff_Err::eTiff_OK
#define FileOpenErr							Tiff_Err::eFileOpenErr
#define VersionErr							Tiff_Err::eVersionErr
#define TooManyTags							Tiff_Err::eTooManyTags
#define TagStripErr							Tiff_Err::eTagStripErr
#define MemoryAllocFail						Tiff_Err::eMemoryAllocFail
#define DataTypeErr							Tiff_Err::eDataTypeErr
#define CompressData						Tiff_Err::eCompressData
#define UnDefineErr							Tiff_Err::eUnDefineErr
//#define Tiff_NEW_TAG						Tiff_Err::eTiff_NEW_TAG

typedef CTiff* PTIFF;
typedef shared_ptr<CTiff> SPTIFF;

#endif // !DEFINED(_tIFF_STL3)