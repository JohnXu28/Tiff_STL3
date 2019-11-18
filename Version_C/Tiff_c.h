#ifndef __TIFF_C_H__
#define __TIFF_C_H__ 

#include <stdio.h>
#ifdef WIN32
#include "..\Src\SysInfo.h"
#endif //WIN32


#ifndef _WINDEF_
 typedef	unsigned char 			BYTE;
 typedef	unsigned short 			WORD;
 typedef	unsigned int 			DWORD;
 typedef	WORD*					LPWORD;
 typedef	DWORD*					LPDWORD;
 typedef	const char*				LPCTSTR;
 typedef	unsigned char*			LPBYTE;
#endif //_WINDEF_

#ifndef __cplusplus
#define inline
 typedef    int						bool;

#define false	0
#define true	1
#endif //__cplusplus
  
typedef enum TiffTagSignature{
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
	//IPTC (International Press Telecommunications Council) metadata.
	IPTC						= 0x83BBL,
	Photoshop					= 0x8649L,
	Exif_IFD					= 0x8769L,
	IccProfile					= 0x8773L
	}TiffTagSignature;

typedef enum FieldType{
	UnknownType					= 0x0000L,
	Byte						= 0x0001L,
	ASCII						= 0x0002L,
	Short						= 0x0003L,
	Long						= 0x0004L,
	Rational					= 0x0005L,
	SBYTE						= 0x0006L,
	UndefineType				= 0x0007L,
	SShort						= 0x0008L,
	SLong						= 0x0009L,
	Float						= 0x000AL,
	Double						= 0x000BL
	}FieldType;

typedef enum Tiff_Err {
	Tiff_OK = 0,
	FileOpenErr = -1,
	VersionErr = -2,/*!< Check Win or Mac version.*/
	TooManyTags = -3,/*!< Tags > MaxTag; */
	TagStripErr = -4,/*!< StripByteCount.n != StripOffset.n;*/
	MemoryAllocFail = -5,
	DataTypeErr = -6,
	CompressData = -7,
	UnDefineErr = -9999,
	Tiff_NEW_TAG = 1
}Tiff_Err;

/*********************************************************
IO Interface of C Language.
*********************************************************/
#define IO_c 0
#if IO_c
#define IO_Interface					FILE //Type
#define IO_C							File //Argument
#define IO_In_C(FileName, option)		fopen(FileName, option)
#define IO_Out_C(FileName, option)		fopen(FileName, option)
#define IO_Close_C(IO_C)				fclose(IO_C)
#define IO_Read_C(Str, Size, Count)		fread(Str, Size, Count, IO_C)
#define IO_Write_C(Str, Size, Count)	fwrite(Str, Size, Count, IO_C)
#define IO_Seek_C(Offset, Origin)		fseek(IO_C, Offset, Origin)
#else//IO_c

#include "Virtual_IO_C.h"

#endif//IO_c

/*********************************************************
IO Interface of C Language.
*********************************************************/
typedef struct TiffTag *PTiffTag;
typedef DWORD (*PGetValue)(PTiffTag Tag);

DWORD GetValue_DWORD(PTiffTag This);

typedef struct TiffTag
{
	TiffTagSignature	tag;          
	FieldType			type;         
	DWORD 				n;          
	DWORD				value; 
	LPBYTE				lpData;
	PGetValue			GetValue;
}TiffTag; 

typedef struct  HEADER
{
	WORD	order;       // "II", 0x4949
	WORD	version;     // 0x2A
	DWORD	offset;      // 8, offset value of first Image File Directory,ie entry count position
}HEADER;

#define MAXTAG 30

typedef struct IFD_STRUCTURE
{
	WORD		EntryCounts;
	TiffTag*	TagList[MAXTAG];
	DWORD		NextIFD;
}IFD_STRUCTURE;

typedef struct  Tiff
{
	HEADER			Version;
	IFD_STRUCTURE	IFD;
	DWORD			IFD_Offset;

	//useful property
	int Width, Length, samplesPerPixel, bitsPerSample, BytesPerLine, Resolution;
	LPBYTE			lpImageBuf;
	LPBYTE			lpHeader;//Used for GetHeader option.
}Tiff, *LPTIFF;


//Class TiffTag
TiffTag* TiffTag_Create();
void TiffTag_Delete(TiffTag *This);
TiffTag* TiffTag_TiffTag0(TiffTagSignature Tag);
TiffTag* TiffTag_TiffTag1(TiffTagSignature Tag, FieldType Type, DWORD n, DWORD value, LPBYTE lpBuf);
TiffTag* TiffTag_TiffTag2(DWORD SigType, DWORD n, DWORD value, FILE *File);
DWORD TiffTag_GetValue_DWORD(PTiffTag This);
DWORD TiffTag_GetValue_DWORD(PTiffTag This);
DWORD TiffTag_GetValue_BitsPerSampleTag(PTiffTag This);
DWORD TiffTag_GetValue_ResolutionTag(PTiffTag This);
//DWORD TiffTag_GetValue_Exif_IFD(PTiffTag This);


//Private:
//For Saving file.
int Tiff_WriteHeader(Tiff *This, IO_Interface *File);
int Tiff_WriteIFD(Tiff *This, IO_Interface *File);
int Tiff_WriteTagData(Tiff *This, IO_Interface *File);
int Tiff_WriteImageData(Tiff *This, IO_Interface *File);
int Tiff_Write_Exif_IFD_Tag(Tiff *This, IO_Interface *File);

//Tag Operation
void Tiff_AddTags(Tiff *This, DWORD TypeSignature, DWORD n, DWORD value, IO_Interface *File);
TiffTag* Tiff_CreateTag(DWORD SigType, DWORD n, DWORD value, IO_Interface *File);
int Tiff_SetTag(Tiff *This, TiffTag *NewTag);
int Tiff_SetTag2(Tiff *This, TiffTagSignature Signature, WORD type, DWORD n, DWORD value, LPBYTE lpBuf);
DWORD Tiff_SetTagValue(Tiff *This, const TiffTagSignature Signature, DWORD Value);
TiffTag* Tiff_GetTag(Tiff *This, const TiffTagSignature Signature);
DWORD Tiff_GetTagValue(Tiff *This, const TiffTagSignature Signature);

//Read Image
int Tiff_ReadImage(Tiff *This, IO_Interface *File);
int Tiff_ReadMultiStripOffset(Tiff *This, IO_Interface *File);
void Pack_BYTE(Tiff *This, int Width, int Length);
void Pack_WORD(Tiff *This, int Width, int Length);

//Public:
Tiff* Tiff_Create();
void Tiff_Close(Tiff *This);
void Tiff_Reset(Tiff *This);
int  Tiff_ReadFile(Tiff *This, const char *FileName);
int  Tiff_Read(Tiff *This, IO_Interface *IO_C);
int  Tiff_SaveFile(Tiff *This, const char *FileName);

#ifdef __cplusplus
int Tiff_GetRow(Tiff *This, LPBYTE lpBuf, int Line, int pixel = 0);
int Tiff_GetRow(Tiff *This, LPWORD lpBuf, int Line, int pixel = 0);
int Tiff_PutRow(Tiff *This, LPBYTE lpBuf, int Line, int pixel = 0);
int Tiff_PutRow(Tiff *This, LPWORD lpBuf, int Line, int pixel = 0);
int Tiff_GetRowColumn(Tiff *This, LPBYTE lpBuf, int x, int y, int RecX, int RecY);
int Tiff_GetRowColumn(Tiff *This, LPWORD lpBuf, int x, int y, int RecX, int RecY);
#else
int Tiff_GetRow_BYTE(Tiff *This, LPBYTE lpBuf, int Line, int pixel);
int Tiff_GetRow_WORD(Tiff *This, LPWORD lpBuf, int Line, int pixel);
int Tiff_PutRow_BYTE(Tiff *This, LPBYTE lpBuf, int Line, int pixel);
int Tiff_PutRow_WORD(Tiff *This, LPWORD lpBuf, int Line, int pixel);
int Tiff_GetRowColumn_BYTE(Tiff *This, LPBYTE lpBuf, int x, int y, int RecX, int RecY);
int Tiff_GetRowColumn_WORD(Tiff *This, LPWORD lpBuf, int x, int y, int RecX, int RecY);
#endif//__cplusplus

LPBYTE Tiff_GetXY(Tiff *This, int X, int Y);
LPBYTE Tiff_GetXY_M(Tiff *This, int X, int Y);//Math Coordinate.
LPBYTE Tiff_GetImageBuf(Tiff *This);
void Tiff_SetImageBuf(Tiff *This, LPBYTE lpBuf);

Tiff* Tiff_CreateNew(Tiff *This, int width, int length, int resolution, int samplesperpixel, int bitspersample, int AllocBuf = 1);
Tiff* Tiff_CreateNew2(int width, int length, int resolution, int samplesperpixel, int bitspersample, int AllocBuf = 1);
Tiff* Tiff_CreateNewFile(int width, int length, int resolution, int samplesperpixel, int bitspersample, char* InName, char* OutName);
int Tiff_SetIccProfile(Tiff *This, char *IccFile);
void Tiff_SaveIccProfile(Tiff *This, char *OutIccFile);
void Tiff_RemoveIcc(Tiff *This);

//Macro and Define
#define m_IFD				This->IFD
#define m_IFD_Offset		This->IFD_Offset
#define m_EntryCounts		This->IFD.EntryCounts
#define m_lpImageBuf		This->lpImageBuf
#define m_lpHeader			This->lpHeader

#define ReadFile(FileName)	Tiff_ReadFile(This, FileName)
#define SaveFile(FileName)	Tiff_SaveFile(This, FileName)
#define Reset()				Tiff_Reset(This)

#define AddTags(TypeSignature, n, value, File)			Tiff_AddTags(This, TypeSignature, n, value, File)
#define CreateTag(SigType, n, value, File)				Tiff_CreateTag(SigType, n, value, File)
#define CreateTag2(SigType, n, value)					Tiff_CreateTag(SigType, n, value, NULL)
#define GetTag(Signature)								Tiff_GetTag(This, Signature)
#define GetTagValue(Signature)							Tiff_GetTagValue(This, Signature)
#define SetTag(NewTag)									Tiff_SetTag(This, NewTag)
#define SetTag2(Signature, type, n, value)				Tiff_SetTag2(This, Signature, type, n, value, NULL)
#define SetTag3(Signature, type, n, value, lpBuf)		Tiff_SetTag2(This, Signature, type, n, value, lpBuf)
#define SetTagValue(Signature, value)					Tiff_SetTagValue(This, Signature, value)
#define CreateNew(width, length, resolution, samplesperpixel, bitspersample, AllocBuf) \
		Tiff_CreateNew(width, length, resolution, samplesperpixel, bitspersample, AllocBuf)


#define m_Width				This->Width
#define m_Length			This->Length
#define m_SamplesPerPixel	This->samplesPerPixel
#define m_BitsPerSample		This->bitsPerSample
#define m_BytesPerLine		This->BytesPerLine
#define m_Resolution		This->Resolution

#endif //__TIFF_C_H__