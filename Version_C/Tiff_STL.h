//////////////////////////////////////////////////////////////////////j
// Tiff_STL.h: interface for the CTiff_STL class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_TIFF_STL_)
#define _TIFF_STL_

#ifndef WIN32
#include "..\SysInfo\SysInfo.h"
#endif //WIN32
#include <windows.h>
#include <vector>
using namespace std;

#define MAXTAG 30

namespace AV_Tiff_STL{
typedef enum TiffTagSignatre{
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
	CopyRight					= 0x8298L,
	IccProfile					= 0x8773L
	} TiffTagSignature;

enum FieldType{
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
	};

enum ErrCode{
	Tiff_OK						=  0,
	FileOpenErr					= -1,
	VersionErr					= -2,//Check Win or Mac version.
	TooManyTags					= -3,//Tags > MaxTag;
	TagStripErr					= -4,//StripByteCount.n != StripOffset.n;
	MemoryAllocFail				= -5,
	DataTypeErr					= -6,
	UnDefineErr					= -9999
};

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
	void Reset();
	bool operator()(const TIFFTAG& Data) const{return this->tag == Data.tag;};
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

struct IFD_STRUCTURE
{
	WORD		EntryCounts;
	vector<TIFFTAG>		m_Tags;
	DWORD		NextIFD;
};

/*
struct TIFFHEADER
{
	DWORD			IFD_Offset;
	IFD_STRUCTURE	IFD;
};
*/

//************************************************************

//*************************************************************y

typedef vector<TIFFTAG> TAG_LIST;
class CTiff  
{
public:
	CTiff();
	virtual		~CTiff();
	CTiff(int Width, int Length, int Resolution, int sampleperpixel, int bitspersample, int AllocBuf = 1);
	CTiff(LPCTSTR FileName, int Width, int Length, int Resolution, int sampleperpixel, int bitspersample);

	virtual ErrCode		ReadFile(LPCTSTR FileName);
	virtual ErrCode		SaveFile(LPCTSTR FileName);

	TiffTagSignature CheckingTag(const TiffTagSignature Signature);
	LPBYTE		GetImageBuf(){return m_lpImageBuf;}
	//Be carefully, When you using this function, you need to take care of the memory you set up in this function.
	void		SetImageBuf(unsigned char *lpBuf){m_lpImageBuf = lpBuf;}
	ErrCode		GetTag(const TiffTagSignatre Signature, TIFFTAG &TiffTag);
	DWORD		GetTagValue(const TiffTagSignature Signature);
	ErrCode		SetTag(const TIFFTAG data);
	ErrCode		SetTagValue(const TiffTagSignature Signature, int Value);
	ErrCode		CreateNew(int Width, int Length, int Resolution, int sampleperpixel, int bitspersample, int AllocBuf = 1);
	ErrCode		CreateNew(LPCTSTR FileName, int Width, int Length, int Resolution, int sampleperpixel, int bitspersample);
	ErrCode		StartPage();
	ErrCode		Scan(LPBYTE lpBuf, int Count);
	ErrCode		ScanFail();
	ErrCode		EndPage();
	ErrCode		EndFile();
	ErrCode		SetIccProfile(char *IccFile);
	ErrCode		CreateBuf();
	void		SaveIccProfile(char *OutIccFile);
	void		RemoveIcc();
	
	int			PutRow(LPBYTE lpBuf, int Line, int Bytes);
	int			PutRow(LPWORD lpBuf, int Line, int WORDs);
	int			GetRow(LPBYTE lpBuf, int Line, int Bytes);
	int			GetRow(LPWORD lpBuf, int Line, int Words);
	int			GetRowColumn(unsigned char *lpBuf, int x, int y, int RecX, int RecY);
	int			GetRowColumn(unsigned short *lpBuf, int x, int y, int RecX, int RecY);
	unsigned char* PutData(unsigned char *lpImageBufIndex, unsigned char *lpBuf, int ByteCounts);
	unsigned short* PutData(unsigned short *lpImageBufIndex, unsigned short *lpBuf, int WordCounts);

private:
	ErrCode		WriteHeader();
	ErrCode		WriteIFD();

	DWORD			IFD_Offset;
	IFD_STRUCTURE	IFD;

	LPCTSTR		m_FileName;
	LPBYTE		m_lpImageBuf;
	LPBYTE		m_IccProfile;
	FILE		*m_File;
};


inline WORD Tiff_encode_L(double data)
{//range 0 ~ 100
	int intPart, rationPart;

	if (data < 0)
		return 0x0;
	else
	{
		intPart = (int) data;
		rationPart = (int)((data - intPart) * 256 + 0.5);
		return (0xFF00 & (int)(intPart * 255 / 100 + 0.5) << 8) | (0xFF & rationPart);
	}
}

inline WORD Tiff_encode_ab(double data)
{	return (short)(data * 256);}

inline double Tiff_decode_L(WORD data)
{  	return (double)data * 100 / 65535.0;}

inline double Tiff_decode_ab(WORD data)
{	return (short)data / 256;}

}//namespace AV_Tiff_STL
#endif // !defined(_TIFF_STL_)
