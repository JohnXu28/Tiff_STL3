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
*
* @endcode
*/

#if !defined(_TIFF_STL3_)
#define _TIFF_STL3_

#include <SysInfo\SysInfo.h>

#ifdef WIN32
#include <windows.h>
#pragma warning(disable : 4996)//for Net

/***************************************************************************
Virtual IO
***************************************************************************/
//#define VIRTUAL_IO	
//#define VIRTUAL_IO_STL	

#if defined(VIRTUAL_IO)//Slowest, it should be the same with fopen, but not the true.
#include <SysInfo\Virtual_IO.h>
#define IO_In(FileName)					new IO_File(FileName, "rb")
#define IO_Out(FileName)				new IO_File(FileName, "wb")
#define IO_Close(IO)					delete IO
#define IO_Read(Str, Size, Count)		IO->Read(Str, Size, Count)
#define IO_Write(Str, Size, Count)		IO->Write(Str, Size, Count)
#define IO_Seek(Offset, Origin)			IO->Seek(Offset, Origin)
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
#define IO_GetHandle()					IO->GetHandle()

#else //Almost the same with VIRTUAL_IO_STL
#define IO_Interface					FILE //Type
#define IO								File //Argument
#define IO_In(FileName)					fopen(FileName, "rb")
#define IO_Out(FileName)				fopen(FileName, "wb+")
#define IO_Close(File)					fclose(File)
#define IO_Read(Str, Size, Count)		fread(Str, Size, Count, File)
#define IO_Write(Str, Size, Count)		fwrite(Str, Size, Count, File)
#define IO_Seek(Offset, Origin)			fseek(File, Offset, Origin)
#endif //VIRTUAL_IO

#else //WIN64
#define IO_Interface					FILE //Type
#define IO								File //Argument
#define IO_In(FileName)					fopen(FileName, "rb")
#define IO_Out(FileName)				fopen(FileName, "wb+")
#define IO_Close(File)					fclose(File)
#define IO_Read(Str, Size, Count)		fread(Str, Size, Count, File)
#define IO_Write(Str, Size, Count)		fwrite(Str, Size, Count, File)
#define IO_Seek(Offset, Origin)			fseek(File, Offset, Origin)
#endif //WIN32

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

#define MAXTAG 40

#include <vector>
#include <functional>
#include <algorithm>
#include <iostream>
#include <memory>
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
namespace AV_Tiff_STL3{

	/***************************************************************************
	***************************************************************************/
	enum TiffTagSignature{
		NullTag						= 0x0000L,
		NewSubfileType				= 0x00FEL,
		SubfileType					= 0x00FFL,
		ImageWidth					= 0x0100L,
		ImageLength					= 0x0101L,
		BitsPerSample				= 0x0102L,
		Compression					= 0x0103L,
		PhotometricInterpretation	= 0x0106L,
		Threshholding				= 0x0107L,
		CellWidth					= 0x0108L,
		CellLength					= 0x0109L,
		FillOrder					= 0x010AL,
		DocumentName				= 0x010DL,
		ImageDescription			= 0x010EL,
		Make						= 0x010FL,
		Model						= 0x0110L,
		StripOffsets				= 0x0111L,
		Orientation					= 0x0112L,
		SamplesPerPixel				= 0x0115L,
		RowsPerStrip				= 0x0116L,
		StripByteCounts				= 0x0117L,
		MinSampleValue				= 0x0118L,
		MaxSampleValue				= 0x0119L,
		XResolution					= 0x011AL,
		YResolution					= 0x011BL,
		PlanarConfiguration			= 0x011CL,
		PageName					= 0x011DL,
		XPosition					= 0x011EL,
		YPosition					= 0x011FL,
		FreeOffsets					= 0x0120L,
		FreeByteCounts				= 0x0121L,
		GrayResponseUnit			= 0x0122L,
		GrayResponsCurve			= 0x0123L,
		T4Options					= 0x0124L,
		T6Options					= 0x0125L,
		ResolutionUnit				= 0x0128L,
		PageNumber					= 0x0129L,
		TransferFunction			= 0x012DL,
		Software					= 0x0131L,
		DateTime					= 0x0132L,
		Artist						= 0x013BL,
		HostComputer				= 0x013CL,
		Predicator					= 0x013DL,
		WhitePoint					= 0x013EL,
		PrimaryChromaticities		= 0x013FL,
		ColorMap					= 0x0140L,
		HalftoneHints				= 0x0141L,
		TileWidth					= 0x0142L,
		TileLength					= 0x0143L,
		TileOffsets					= 0x0144L,
		TileByteCounts				= 0x0145L,
		InkSet						= 0x014CL,
		InkNames					= 0x014DL,
		NumberOfInks				= 0x014EL,
		DotRange					= 0x0150L,
		TargetPrinter				= 0x0151L,
		ExtraSamples				= 0x0152L,
		SampleFormat				= 0x0153L,
		SMinSampleValue				= 0x0154L,
		SMaxSampleValue				= 0x0155L,
		TransforRange				= 0x0156L,
		JPEGProc					= 0x0157L,
		JPEGInterchangeFormat		= 0x0201L,
		JPEGIngerchangeFormatLength = 0x0202L,
		JPEGRestartInterval			= 0x0203L,
		JPEGLosslessPredicators		= 0x0205L,
		JPEGPointTransforms			= 0x0206L,
		JPEGQTable					= 0x0207L,
		JPEGDCTable					= 0x0208L,
		JPEGACTable					= 0x0209L,
		YCbCrCoefficients			= 0x0211L,
		YCbCrSampling				= 0x0212L,
		YCbCrPositioning			= 0x0213L,
		ReferenceBlackWhite			= 0x0214L,
		XML_Data					= 0x02BCL,
		CopyRight					= 0x8298L,
		IPTC						= 0x83BBL,/*!< IPTC (International Press Telecommunications Council) metadata.*/
		Photoshop					= 0x8649L,
		Exif_IFD					= 0x8769L,
		IccProfile					= 0x8773L
	};

	/***************************************************************************
	***************************************************************************/
	enum FieldType{
		Unknown			= 0x0000L,
		Byte			= 0x0001L,
		ASCII			= 0x0002L,
		Short			= 0x0003L,
		Long			= 0x0004L,
		Rational		= 0x0005L,
		SBYTE			= 0x0006L,
		UndefineType	= 0x0007L,
		SShort			= 0x0008L,
		SLong			= 0x0009L,
		Float			= 0x000AL,
		Double			= 0x000BL,
		Unknown1		= 0x000CL,//Never using, Just for code analysis warning.
		Unknown2		= 0x000DL,
		Unknown3		= 0x000EL,
		Unknown4		= 0x000FL,
	};

#define FieldTypeSize 16
	/***************************************************************************
	***************************************************************************/
	enum ErrCode{
		Tiff_OK			= 0,
		FileOpenErr		= -1,
		VersionErr		= -2,/*!< Check Win or Mac version.*/
		TooManyTags		= -3,/*!< Tags > MaxTag; */
		TagStripErr		= -4,/*!< StripByteCount.n != StripOffset.n;*/
		MemoryAllocFail	= -5,
		DataTypeErr		= -6,
		CompressData	= -7,
		UnDefineErr		= -9999,
		Tiff_NEW_TAG	= 1
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
		TiffTag(DWORD SigType, DWORD n, DWORD value, IO_Interface *IO);
		TiffTag(TiffTagSignature Tag, FieldType Type, DWORD n, DWORD value, LPBYTE lpBuf = nullptr);
		bool operator()(const TiffTag& Data) const{ return tag == Data.tag; };
		virtual DWORD GetValue() const;
		virtual int SaveFile(IO_Interface *IO);
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
	class BitsPerSampleTag :public TiffTag
	{
	public:
		BitsPerSampleTag(DWORD SigType, DWORD n, DWORD value, IO_Interface *IO);
		DWORD GetValue() const;
	};

	class ResolutionTag :public TiffTag
	{
	public:
		ResolutionTag(DWORD SigType, DWORD n, DWORD value, IO_Interface *IO);
		virtual DWORD GetValue() const;
	};

	class Exif_IFD_Tag :public TiffTag
	{
	public:
		Exif_IFD_Tag(DWORD SigType, DWORD n, DWORD value, IO_Interface *IO);
		//***Don't save anything. This tag broke all rule, just skip it.		
	};

	class TAG//funtor, For find_if
	{
	public:
		TAG(TiffTagSignature signature){ Signature = signature; };
		bool operator()(const TiffTag* Tag) const{ return Tag->tag == Signature; }
		TiffTagSignature Signature;
	};

#ifdef SHARED_POINTER
	typedef shared_ptr<TiffTag> TiffTagPtr;
#else
	typedef TiffTag *TiffTagPtr;
#endif //SHARED_POINTER

	typedef vector<TiffTagPtr> TagList;
	typedef TagList::iterator TiffTag_iter;
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
		IFD_STRUCTURE(){ NextIFD = 0; }
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
		virtual		ErrCode CheckFile(IO_Interface *IO);
		virtual		ErrCode	ReadTiff(IO_Interface *IO);
		virtual		ErrCode SaveTiff(IO_Interface *IO);
		virtual		ErrCode	ReadFile(LPCSTR FileName);
		virtual		ErrCode	SaveFile(LPCSTR FileName);

#if defined (VIRTUAL_IO) | defined(VIRTUAL_IO_STL)
		virtual		ErrCode ReadMemory(LPBYTE Buffer, size_t BufSize);
		virtual		ErrCode SaveMemory(LPBYTE Buffer, size_t BufSize, size_t &SaveSize);
#endif //VIRTUAL_IO

		//	Tag Operation
		TiffTagPtr	GetTag(const TiffTagSignature Signature);
		DWORD		GetTagValue(const TiffTagSignature Signature);
		ErrCode		SetTag(TiffTagPtr NewTag);
		ErrCode		SetTagValue(const TiffTagSignature Signature, DWORD Value);

	protected:
		//Read Image
		virtual		TiffTagPtr	CreateTag(DWORD SignatureType, DWORD n, DWORD value, IO_Interface *IO);
		void		AddTags(DWORD TypeSignature, DWORD n, DWORD value, IO_Interface *IO);
		ErrCode		ReadImage(IO_Interface *IO);
		ErrCode		ReadMultiStripOffset(IO_Interface *IO);

		template<class T>
		void		Pack(int Width, int Length);

		//Write Image
		virtual		ErrCode	WriteHeader(IO_Interface *IO);
		ErrCode		WriteIFD(IO_Interface *IO);
		ErrCode		WriteTagData(IO_Interface *IO);
		ErrCode		WriteImageData(IO_Interface *IO);
		ErrCode		WriteData_Exif_IFD_Tag(IO_Interface *IO);

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
		virtual		~CTiff();
		virtual		ErrCode	ReadTiff(IO_Interface *IO);
		virtual		ErrCode	ReadFile(LPCSTR FileName);		

#if defined(VIRTUAL_IO) | defined(VIRTUAL_IO_STL)
		virtual		ErrCode ReadMemory(LPBYTE Buffer, size_t BufSize);
#endif //VIRTUAL_IO

		ErrCode		CreateNew(int width, int length, int resolution, int samplesperpixel, int bitspersample, int AllocBuf = 1);
		ErrCode		CreateNew(int width, int length, int resolution, int samplesperpixel, int bitspersample, LPCSTR InName);
		ErrCode		CreateNew(int width, int length, int resolution, int samplesperpixel, int bitspersample, LPCSTR InName, LPCSTR OutName);
		ErrCode		SetTag(TiffTagSignature Signature, WORD type, DWORD n, DWORD value, LPBYTE lpBuf = nullptr);
		ErrCode		SetTagValue(const TiffTagSignature Signature, DWORD Value);

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

		LPBYTE GetRowIndex(int Line){ return GetXY(0, Line); };
		LPBYTE GetRowIndex_Q(int Line){ return GetXY_Q(0, Line); };

		//Coordinate operation. M:Math, Q:Quick(Not Check Boundary)
		LPBYTE		GetXY(int X, int Y);
		void		SetXY(int X, int Y, BYTE Value){ *(GetXY(X, Y)) = Value; };
		LPBYTE		GetXY_M(int X, int Y);
		void		SetXY_M(int X, int Y, BYTE Value){ *(GetXY_M(X, Y)) = Value; };
		LPBYTE		GetXY_Q(int X, int Y);
		void		SetXY_Q(int X, int Y, BYTE Value){ *(GetXY_Q(X, Y)) = Value; };
		LPBYTE		GetXY_MQ(int X, int Y);
		void		SetXY_MQ(int X, int Y, BYTE Value){ *(GetXY_MQ(X, Y)) = Value; };

		//Icc profile Operation.
		ErrCode		SetIccProfile(char *IccFile);
		void		SaveIccProfile(char *OutIccFile);
		void		RemoveIcc();

		//Image Buffer operation.
		void		SetImageBuf(LPBYTE lpBuf);
		void		ForgetImageBuf();
		LPBYTE		GetImageBuf();

#ifdef TIFF_EXT
		ErrCode	ReadJPG(LPCSTR FileName);
		ErrCode	SaveJPG(LPCSTR FileName);
		ErrCode	ReadFile_Ext(LPCSTR FileName);
#endif //TIFF_EXT

#ifdef DUMP_TIFF
		int DumpTiff(LPCSTR FileName);
		void DumpTags(DWORD TypeSignature, DWORD n, DWORD value, FILE *File);
#endif //DUMP_TIFF

	private:
		//useful property
		int m_Width, m_Length, m_SamplesPerPixel, m_BitsPerSample, m_BytesPerLine, m_Resolution;
		LPBYTE m_lpImageBuf;
	};

	//************************************************************
	// Just for writing code easily, for fun......(^_^) John.....
	//*************************************************************j

	inline WORD Tiff_encode_L(double data)
	{//range 0 ~ 100
		int intPart, rationPart;

		if (data < 0)
			return 0x0;
		else
		{
			intPart = (int)data;
			rationPart = (int)((data - intPart) * 256 + 0.5);
			return (0xFF00 & (int)(intPart * 255 / 100 + 0.5) << 8) | (0xFF & rationPart);
		}
	}

	inline WORD Tiff_encode_ab(double data)
	{
		return (short)(data * 256);
	}

	inline double Tiff_decode_L(WORD data)
	{
		return (double)data * 100 / 65535.0;
	}

	//bug???
	inline double Tiff_decode_ab(WORD data)
	{
		if (data < 0x8000)
			return (short)data / 256.0;
		else
			return (data - 0xFFFF) / 256.0;
	}

	inline double Tiff_decode_L_8(BYTE data)
	{
		return (double)data * 100 / 255.0;
	}

	inline double Tiff_decode_ab_8(BYTE data)
	{
		if (data < 128)
			return (double)data;
		else
			return data - 255;
	}

}//namespace AV_Tiff_STL3 or AV_Tiff

using namespace AV_Tiff_STL3;

#endif // !defined(_TIFF_STL3_)
