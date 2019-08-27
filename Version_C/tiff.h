//////////////////////////////////////////////////////////////////////
// Tiff.h: interface for the CTiff class.
//
//////////////////////////////////////////////////////////////////////
#ifndef _TIFF_H_
#define _TIFF_H_

#include "..\SysInfo\SysInfo.h"

namespace AV_Tiff{
typedef enum TiffTagSignatre{
	Null						= 0x0000L,
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
	CopyRight					= 0x8298L,
	IccProfile					= 0x8773L
	} TiffTagSignature;

typedef enum {
	Unknown						= 0x0000L,
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

class TIFFTAG
{
public:
	TiffTagSignature	tag;          
	FieldType			type;         
	DWORD 				n;          
	DWORD				value;
	TIFFTAG();
	TIFFTAG(TiffTagSignature Signature);
	TIFFTAG(DWORD TypeSignature, DWORD n, DWORD value);
	TIFFTAG(TiffTagSignature Tag, FieldType Type, DWORD n, DWORD value);
	bool operator()(const TIFFTAG& Data) const{return this->tag == Data.tag;};
};


class TIFFTAG2
{
public:
	WORD				tag;          
	WORD				type;         
	DWORD				n;          
	DWORD				value;       

public:
	TIFFTAG2();
	~TIFFTAG2();
	const	TIFFTAG2&	operator=(const TIFFTAG& temp);
	int		TransTag(TIFFTAG& NewTag);
};

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
	TIFFTAG		TagList[MAXTAG];
	DWORD		NextIFD;
}IFD_STRUCTURE;

typedef struct  TIFFHEADER
{
	HEADER			Version;
	IFD_STRUCTURE	IFD;
}TIFFHEADER, *LPTIFFHEADER;

//////////////////////////////////////////////////////////////////////
// Class and Function
//////////////////////////////////////////////////////////////////////
class CTiff  
{
public:
	CTiff();
	virtual ~CTiff();

	int		Initialheader();
	void	GetTag(TiffTagSignatre Signature, TIFFTAG &TiffTag);
	int		SetTag(TIFFTAG data);
	int		CreateNew(int Width, int Length, int Resolution, int sampleperpixel, int bitspersample, int ColorOrder = 0);
	int		WriteHeader();
	int		SaveFile(LPCTSTR FileName);
	int		ReadFile(LPCTSTR FileName);
//	int		DumpFile();
	int		PutRow(LPBYTE lpBuf, int Line, int Bytes);
	int		PutRow(LPWORD lpBuf, int Line, int WORDs);
	int		GetRow(LPBYTE lpBuf, int Line, int Bytes);
	int		GetRow(LPWORD lpBuf, int Line, int Words);
	int		GetRowColumn(unsigned char *lpBuf, int x, int y, int RecX, int RecY);
	int		GetRowColumn(unsigned short *lpBuf, int x, int y, int RecX, int RecY);
	unsigned char* PutData(unsigned char *lpImageBufIndex, unsigned char *lpBuf, int ByteCounts);
	unsigned short* PutData(unsigned short *lpImageBufIndex, unsigned short *lpBuf, int WordCounts);
	int		GetImage();
	int		SetIccProfile(char *IccFile);
	void	SaveIccProfile(char *OutIccFile);
	void	RemoveIcc();
// Attributes
public:

	LPCTSTR			m_FileName;
	FILE			*m_File;

	LPBYTE			m_IccProfile;
	LPTIFFHEADER	m_lpTiffHeader;
	LPBYTE			m_lpImageBuf;

private:
	TiffTagSignature CheckingTag(const TiffTagSignature Signature);

};

class RationalNumber
{
public:
	DWORD Fraction;
	DWORD Denominator;
public:
	int SetRationalNumber(WORD Fraction, WORD Denominator);
	RationalNumber();
	~RationalNumber();
};
}
#endif //_TIFF_H_

