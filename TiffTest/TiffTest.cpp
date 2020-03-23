// TiffTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <string>
#include <iostream>
#include <memory>
using namespace std;

#define John
//#define Luke
//#define Chunyen_yang

#ifdef John
#define Tiff_C		0
#define Tiff_STL3	1

#define Tiff_Test	1
#define Single_Test	0

#define Tag_Test 0
#define RGB2CMY 0
#define DOTCOUNT 0
//#define CREATE_TIFF
//#define CREATE_TIFF_4_1
#define TEST 1

#if Tiff_STL3
#include <Tiff_STL3\Src\Tiff_STL3.h>

#define STiff CTiff
#define Tiff_Create() new CTiff
#define Tiff_CreateNew(lpTiff, width, length, resolution, samplesperpixel, bitspersample) lpTiff->CreateNew(width, length, resolution, samplesperpixel, bitspersample)
#define Tiff_ReadFile(lpTiff, FileName) lpTiff->ReadFile(FileName)
#define Tiff_SaveFile(lpTiff, FileName) lpTiff->SaveFile(FileName)
#define Tiff_Close(lpTiff) delete lpTiff
#define Tiff_GetTagValue(lpTiff, TagSig) lpTiff->GetTagValue(TagSig)
#define Tiff_SetTagValue(lpTiff, TagSig, Value) lpTiff->SetTagValue(TagSig, Value)
#define Tiff_GetRow(lpTiff, lpBuf, i) lpTiff->GetRow(lpBuf, i)
#define Tiff_PutRow(lpTiff, lpBuf, i) lpTiff->PutRow(lpBuf, i)
#define Tiff_GetImageBuf(lpTiff) lpTiff->GetImageBuf()
#define Tiff_GetXY(lpTiff, X, Y) lpTiff->GetXY(X, Y)

#endif//Tiff_STL

#if Tiff_C
#include "..\Tiff_STL3\Version_C\Tiff_c.h"
#define STiff Tiff
//	#define Tiff_ReadFile(lpTiff, fileName) Tiff_ReadFile(lpTiff, (char*)fileName)
//	#define Tiff_SaveFile(lpTiff, fileNmae) Tiff_SaveFile(lpTiff, (char*)fileNmae)
#endif //Tiff_C

#endif//John

#ifdef Luke
#include "..\..\Luke\Tiff.h"
#define STiff Tiff
#define Tiff_Create() new Tiff
#define Tiff_CreateNew(lpTiff, width, length, resolution, samplesperpixel, bitspersample) lpTiff->CreateTiffImage(width, length, resolution, samplesperpixel, bitspersample)
#define Tiff_ReadFile(lpTiff, FileName) lpTiff->ReadTiffFile(FileName)
#define Tiff_SaveFile(lpTiff, FileName) lpTiff->SaveFile(FileName)
#define Tiff_Close(lpTiff) delete lpTiff
#define Tiff_GetTagValue(lpTiff, TagSig) lpTiff->GetTagValue((tiffTagIdentity)TagSig)
#define Tiff_GetRow(lpTiff, lpBuf, i) lpTiff->GetRow(lpBuf, i)
#define Tiff_PutRow(lpTiff, lpBuf, i) lpTiff->PutRow(lpBuf, i)
#define Tiff_GetImageBuf(lpTiff) lpTiff->GetImageBuffer()
#define Tiff_GetXY(lpTiff, X, Y) lpTiff->GetXY(X, Y)
#endif //Luke


#ifdef Chunyen_yang
#include "..\..\Chunyen_yang\TiffLib.h"
#define Tiff_Create() Initialheader();
#define Tiff_CreateNew(lpTiff, width, length, resolution, samplesperpixel, bitspersample) TiffCreateNew(lpTiff,width,length,resolution,samplesperpixel,bitspersample)
#define Tiff_ReadFile(lpTiff, FileName) TiffReadFile(lpTiff,FileName)
#define Tiff_SaveFile(lpTiff, FileName) TiffSaveFile(lpTiff,FileName)
#define Tiff_Close(lpTiff) TiffColse(lpTiff)
#define Tiff_GetTagValue(lpTiff, TagSig) (int)ReadTag(lpTiff,TagSig)
#define Tiff_GetRow(lpTiff, lpBuf, i) GetRow(lpTiff,(unsigned char *)lpBuf,i)
#define Tiff_PutRow(lpTiff, lpBuf, i) PutRow(lpTiff,(unsigned char *)lpBuf,i);
#define Tiff_GetImageBuf(lpTiff) GetImageBuf(lpTiff)
#define Tiff_GetXY(lpTiff, X, Y) GetXY(lpTiff,X,Y)
#endif

#ifdef _TiffTagSignature_
enum TiffTagSignature {
	NullTag = 0x0000L,
	NewSubfileType = 0x00FEL,
	SubfileType = 0x00FFL,
	ImageWidth = 0x0100L,
	ImageLength = 0x0101L,
	BitsPerSample = 0x0102L,
	Compression = 0x0103L,
	PhotometricInterpretation = 0x0106L,
	Threshholding = 0x0107L,
	CellWidth = 0x0108L,
	CellLength = 0x0109L,
	FillOrder = 0x010AL,
	DocumentName = 0x010DL,
	ImageDescription = 0x010EL,
	Make = 0x010FL,
	Model = 0x0110L,
	StripOffsets = 0x0111L,
	Orientation = 0x0112L,
	SamplesPerPixel = 0x0115L,
	RowsPerStrip = 0x0116L,
	StripByteCounts = 0x0117L,
	MinSampleValue = 0x0118L,
	MaxSampleValue = 0x0119L,
	XResolution = 0x011AL,
	YResolution = 0x011BL,
	PlanarConfiguration = 0x011CL,
	PageName = 0x011DL,
	XPosition = 0x011EL,
	YPosition = 0x011FL,
	FreeOffsets = 0x0120L,
	FreeByteCounts = 0x0121L,
	GrayResponseUnit = 0x0122L,
	GrayResponsCurve = 0x0123L,
	T4Options = 0x0124L,
	T6Options = 0x0125L,
	ResolutionUnit = 0x0128L,
	PageNumber = 0x0129L,
	TransferFunction = 0x012DL,
	Software = 0x0131L,
	DateTime = 0x0132L,
	Artist = 0x013BL,
	HostComputer = 0x013CL,
	Predicator = 0x013DL,
	WhitePoint = 0x013EL,
	PrimaryChromaticities = 0x013FL,
	ColorMap = 0x0140L,
	HalftoneHints = 0x0141L,
	TileWidth = 0x0142L,
	TileLength = 0x0143L,
	TileOffsets = 0x0144L,
	TileByteCounts = 0x0145L,
	InkSet = 0x014CL,
	InkNames = 0x014DL,
	NumberOfInks = 0x014EL,
	DotRange = 0x0150L,
	TargetPrinter = 0x0151L,
	ExtraSamples = 0x0152L,
	SampleFormat = 0x0153L,
	SMinSampleValue = 0x0154L,
	SMaxSampleValue = 0x0155L,
	TransforRange = 0x0156L,
	JPEGProc = 0x0157L,
	JPEGInterchangeFormat = 0x0201L,
	JPEGIngerchangeFormatLength = 0x0202L,
	JPEGRestartInterval = 0x0203L,
	JPEGLosslessPredicators = 0x0205L,
	JPEGPointTransforms = 0x0206L,
	JPEGQTable = 0x0207L,
	JPEGDCTable = 0x0208L,
	JPEGACTable = 0x0209L,
	YCbCrCoefficients = 0x0211L,
	YCbCrSampling = 0x0212L,
	YCbCrPositioning = 0x0213L,
	ReferenceBlackWhite = 0x0214L,
	CopyRight = 0x8298L,
	IccProfile = 0x8773L
};
#endif //_TiffTagSignature_

#if Tiff_Test	
int ImageTest1(string FileName)
{
	cout << "Read, Write Test : " << FileName.c_str() << ".tif\n";
	STiff *lpTiff = Tiff_Create();
	string FileIn = FileName + ".tif";
	string FileOut = "Output\\" + FileName + "out.tif";

	if (Tiff_ReadFile(lpTiff, FileIn.c_str()) != Tiff_OK)
	{
		Tiff_Close(lpTiff);
		return -1;
	}

	if (Tiff_ReadFile(lpTiff, FileIn.c_str()) != Tiff_OK)//Test for Memory Leaks.
	{
		Tiff_Close(lpTiff);
		return -1;
	}

	if (Tiff_SaveFile(lpTiff, FileOut.c_str()) != Tiff_OK)
	{
		Tiff_Close(lpTiff);
		return -1;
	}

	Tiff_Close(lpTiff);
	return 0;
}

template<class T>
void TiffCopy(STiff *lpTiffDest, STiff *lpTiffSrc)
{
	int Width = Tiff_GetTagValue(lpTiffSrc, ImageWidth);
	int Length = Tiff_GetTagValue(lpTiffSrc, ImageLength);
	int samplesperpixel = Tiff_GetTagValue(lpTiffSrc, SamplesPerPixel);
	int bitspersample = Tiff_GetTagValue(lpTiffSrc, BitsPerSample);
	int Pixel = Width * samplesperpixel;

	T* lpBuf = new T[Pixel];
	for (int i = 0; i < Length; i++)
	{
		Tiff_GetRow(lpTiffSrc, lpBuf, i);
		Tiff_PutRow(lpTiffDest, lpBuf, i);
	}
	delete[]lpBuf;
}

//Raw Test
int ImageTest2(string FileName)
{
	string FileIn = "Output\\" + FileName + "out.tif";
	string FileRaw = FileName + ".raw";

	cout << "Raw Test : " << FileName.c_str() << "out.tif\n";
	STiff *lpTiff = Tiff_Create();	
	Tiff_ReadFile(lpTiff, FileIn.c_str());

	//Compare
	int ret = 0;
	int stripByteCounts = Tiff_GetTagValue(lpTiff, StripByteCounts);
	unsigned char *lpTemp1 = (unsigned char *)Tiff_GetImageBuf(lpTiff);

	//Get Raw
	FILE* file = fopen(FileRaw.c_str(), "rb");
	LPBYTE lpRaw = new BYTE[stripByteCounts];
	fread(lpRaw, 1, stripByteCounts, file);
	unsigned char* lpTemp2 = lpRaw;
	
	for (int i = 0; i < stripByteCounts; i++)
		if (*(lpTemp1++) != *(lpTemp2++))
		{
			cout << "Raw Test error! " << FileName.c_str() << "\n";
			ret = -1;
			break;
		}
	Tiff_Close(lpTiff);
	delete[]lpRaw;

	return ret;
}

int ImageTest3(string FileName)
{
	string FileIn = "Output\\" + FileName + "out.tif";
	string FileOut = "Output\\" + FileName + "out2.tif";

	cout << "Copy Test : " << FileName.c_str() << "out.tif\n";
	STiff* lpTiff = Tiff_Create();
	STiff* lpTiff2 = Tiff_Create();
	Tiff_ReadFile(lpTiff, FileIn.c_str());
	Tiff_CreateNew(lpTiff2, Tiff_GetTagValue(lpTiff, ImageWidth),
		Tiff_GetTagValue(lpTiff, ImageLength),
		Tiff_GetTagValue(lpTiff, XResolution),
		Tiff_GetTagValue(lpTiff, SamplesPerPixel),
		Tiff_GetTagValue(lpTiff, BitsPerSample)
	);

	//For memory leak test.
	Tiff_CreateNew(lpTiff2, Tiff_GetTagValue(lpTiff, ImageWidth),
		Tiff_GetTagValue(lpTiff, ImageLength),
		Tiff_GetTagValue(lpTiff, XResolution),
		Tiff_GetTagValue(lpTiff, SamplesPerPixel),
		Tiff_GetTagValue(lpTiff, BitsPerSample)
	);

	//copy test
	if (Tiff_GetTagValue(lpTiff, BitsPerSample) == 16)
		TiffCopy<unsigned short>(lpTiff2, lpTiff);
	else
		TiffCopy<unsigned char>(lpTiff2, lpTiff);

	//For Lab Color Image 
	if (Tiff_GetTagValue(lpTiff, PhotometricInterpretation) == 8)
		Tiff_SetTagValue(lpTiff2, PhotometricInterpretation, 8);

	Tiff_SaveFile(lpTiff2, FileOut.c_str());

	//Compare
	int ret = 0;
	int stripByteCounts = Tiff_GetTagValue(lpTiff, StripByteCounts);
	unsigned char* lpTemp1 = (unsigned char*)Tiff_GetImageBuf(lpTiff);
	unsigned char* lpTemp2 = (unsigned char*)Tiff_GetImageBuf(lpTiff2);
	for (int i = 0; i < stripByteCounts; i++)
		if (*(lpTemp1++) != *(lpTemp2++))
		{
			cout << "test error! " << FileOut.c_str() << "\n";
			ret = -1;
			break;
		}
	Tiff_Close(lpTiff);
	Tiff_Close(lpTiff2);

	return ret;
}
template<class T>
int GetXY_Test(STiff *lpTiff1, STiff *lpTiff2)
{
	if (Tiff_GetTagValue(lpTiff1, BitsPerSample) == 1)
		return 0;
	int Width = Tiff_GetTagValue(lpTiff1, ImageWidth);
	int Length = Tiff_GetTagValue(lpTiff1, ImageLength);
	int samplesPerPixel = Tiff_GetTagValue(lpTiff1, SamplesPerPixel);
	T *lpTemp1 = (T*)Tiff_GetImageBuf(lpTiff1);
	T *lpTemp2;

	//Get X Y pixel test, and compare to the orignal image.
	for (int i = 0; i < Length; i++)
		for (int j = 0; j < Width; j++)
		{
			lpTemp2 = (T*)Tiff_GetXY(lpTiff2, j, i);
			for (int k = 0; k < samplesPerPixel; k++)
				if ((*lpTemp1++) != *(lpTemp2++))
					return -1;
		}
	return 0;
}

int ImageTest4(string FileName)
{
	STiff *lpTiff1 = Tiff_Create();
	STiff *lpTiff2 = Tiff_Create();
	string File1 = FileName + ".tif";
	string File2 = "Output\\" + FileName + "out2.tif";
	Tiff_ReadFile(lpTiff1, File1.c_str());
	Tiff_ReadFile(lpTiff2, File2.c_str());

	int ret = 0;
	if (Tiff_GetTagValue(lpTiff1, BitsPerSample) == 16)
		ret = GetXY_Test<unsigned short>(lpTiff1, lpTiff2);
	else
		ret = GetXY_Test<unsigned char>(lpTiff1, lpTiff2);

	if (ret != 0)
		cout << "Get(X,Y) error! " << FileName.c_str() << "\n";
	else
		cout << FileName.c_str() << "Test OK.\n\n";

	Tiff_Close(lpTiff1);
	Tiff_Close(lpTiff2);

	return ret;
}

string FileName[] = { "0NULL",
"1LineArt", "2gray8", "3RGB8", "4CMYK8", "5Lab8",
"6gray16", "7RGB16", "8CMYK16", "9Lab16", "10RGB82",
"11RGB162", "12CMYK82", "13CMYK162", "14Lab82", "15Lab162",
"16MultiStrip", "17Ycc8", "18CLR68", "19CLR616" };

const int TestNum = 20;

int FullTest1()
{
	for (int i = 1; i < TestNum; i++)
		if (ImageTest1(FileName[i]) != 0)
		{
			cout << "Test 1 fail, FileName : " << FileName[i] << endl;
			return -1;
		}
	return 0;
}

int FullTest2()
{
	for (int i = 2; i < TestNum; i++)
		if (ImageTest2(FileName[i]) != 0)
		{
			cout << "Test 2 fail, FileName : " << FileName[i] << endl;
			return -1;
		}

	return 0;
}

int FullTest3()
{
	for (int i = 1; i < TestNum; i++)
		if (ImageTest3(FileName[i]) != 0)
		{
			cout << "Test 3 fail, FileName : " << FileName[i] << endl;
			return -1;
		}

	return 0;
}

int FullTest4()
{
	for (int i = 1; i < TestNum; i++)
		if (ImageTest4(FileName[i]) != 0)
		{
			cout << "Test 4 fail, FileName : " << FileName[i] << endl;
			return -1;
		}

	return 0;
}

#endif //Tiff_Test

#if Single_Test
void SingleTest()
{
	STiff* lpTiff = Tiff_Create();
	if (Tiff_ReadFile(lpTiff, "1LineArt.tif") == Tiff_OK)
	{
		lpTiff->SetTag(PhotometricInterpretation, Short, 1, 1, 0);
		Tiff_SaveFile(lpTiff, "1LineArtOut3.tif");
		Tiff_Close(lpTiff);
	}
}
#endif //Single_Test

//
//class GL
//{
//public:
//	GL() {};
//	~GL() {
//		cout << "End" << endl;
//	};
//	void display() { cout << "GL" << endl; }
//	char Data[65536] = {0};
//};

//GL *gl = new GL;
//GL g2;
//
void DumpMemory(void)
{
	//delete gl;
	DETECT_MEMORY_LEAKS;
}

#if Tag_Test

//TiffTag& NEWTAG()
//{
//	TiffTag NewTag(ImageWidth), Long, 1, 100, nullptr);
//	return NewTag;
//}

void Tag_Test_Construct()
{
	TiffTag NewTag(ImageWidth, Long, 1, 100, nullptr);
	TiffTag Temp1, Temp2;
	Temp1 = NewTag;

	//Temp2 = TiffTag(TiffTag(ImageWidth), Long, 1, 100, nullptr));

	//LPBYTE lpBuf = new BYTE[1024];

}
#endif //Tag_Test

#ifdef Tiff_STL
void CreateTiff(int argc, _TCHAR* argv[])
{
	if (argc != 8)
	{
		cout << "Creatiff Width Length Resolution Samplesperpixel Bitspersample In.raw out.tif" << endl;
	}
	else
	{
		int width = atoi(argv[1]);
		int length = atoi(argv[2]);
		int resolution = atoi(argv[3]);
		int samplesperpixel = atoi(argv[4]);
		int bitspersample = atoi(argv[5]);
		char *raw = argv[6];
		char *tif = argv[7];
		cout << "Width : " << width << endl;
		cout << "Length : " << length << endl;
		cout << "Resolution : " << resolution << endl;
		cout << "Samplesperpixel : " << samplesperpixel << endl;
		cout << "Bitspersample : " << bitspersample << endl;
		cout << "Raw : " << raw << endl;
		cout << "Tif : " << tif << endl;
		STiff *lpTiff = Tiff_Create();;
		Tiff_CreateNew(lpTiffwidth, length, resolution, samplesperpixel, bitspersample, raw, tif);
	}
}

void ProcessTemplate()
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

	for (int i = 0; i < Length; i++)
	{
		In.GetRow(lpBuf, i);
		LPBYTE lpTemp = lpBuf;
		for (int j = 0; j < Width; j++)
			//Process(lpBuf, Width); //Add your process here.
			In.PutRow(lpBuf, i);
	}
	delete[]lpBuf;

	Out.SaveFile("Output.tif");
}

#endif //Tiff_STL3


//#include <SysInfo\Virtual_IO.h>
void GetTiffHeader(char* FileName)
{
	CTiff tiff;
	tiff.ReadFile(FileName);
	int Width = tiff.GetTagValue(ImageWidth);
	int Length = tiff.GetTagValue(ImageLength);
	int Samples = tiff.GetTagValue(SamplesPerPixel);
	int Bits = tiff.GetTagValue(BitsPerSample);

	CTiff tiff2;
	tiff2.CreateNew(Width, Length, 72, Samples, Bits, 0);

	//unsigned char* lpBuf = new unsigned char[1024];
	//IO_Buf *lpBuf = new IO_Buf(lpBuf, 1024);
	//tiff2.SaveFile
	//delete lpBuf;
}

void RGB2CMYK()
{
	CTiff* lpTiff = new CTiff("sRGB.tif"); 
	int Width = lpTiff->GetTagValue(ImageWidth);
	int Length = lpTiff->GetTagValue(ImageLength);
	CTiff *lpCMYK = new CTiff(Width, Length, 72, 4, 8);

	LPBYTE lpIn = lpTiff->GetImageBuf();
	LPBYTE lpOut = lpCMYK->GetImageBuf();
	int Size = Width * Length;
	for (int i = 0; i < Size; i++)
	{
		*(lpOut++) = 255 - *(lpIn++);//C
		*(lpOut++) = 255 - *(lpIn++);//M
		*(lpOut++) = 255 - *(lpIn++);//Y
		*(lpOut++) = 0;		
	}
	lpCMYK->SaveFile("CMYK.tif");
}

#if DOTCOUNT
unsigned char DotLut[256] = {
	0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4,
	1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
	1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
	2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
	1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
	2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
	2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
	3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
	1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
	2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
	2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
	3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
	2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
	3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
	3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
	4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8
};

void DotCount(char* FileName)
{
	shared_ptr<CTiff> lpTiff = make_shared<CTiff>(FileName);
	int Width = lpTiff->GetTagValue(ImageWidth);
	int Length = lpTiff->GetTagValue(ImageLength);
	int samplesPerPixel = lpTiff->GetTagValue(SamplesPerPixel);
	int bitsPerSample = lpTiff->GetTagValue(BitsPerSample);
	int compress = lpTiff->GetTagValue(Compression);	

	if (compress != 1)
	{
		cout << "Only Support Uncompress data." << endl;
		return;
	}
	
	if (samplesPerPixel != 1)
	{
		cout << "Only Support Gray Tiff." << endl;
		return;
	}

	if (bitsPerSample != 1)
	{
		cout << "Only Support Lineart." << endl;
		return;
	}

	int BytesPerLine = (int)ceil((double)Width / 8.0);
	int Size = BytesPerLine * Length;
	LPBYTE lpIn = lpTiff->GetImageBuf();
	int dotcount = 0;
	for (int i = 0; i < Size; i++)
		dotcount += DotLut[*(lpIn++)];

	if (lpTiff->GetTagValue(PhotometricInterpretation) == 1)//Black is zero
		cout << "DotCount : " << (Width * Length - dotcount) << ", Percent : " << (double)(Width * Length - dotcount) / (Width * Length) << endl;
	else
		cout << "DotCount : " << dotcount << ", Percent : " << (double)dotcount/(Width*Length) << endl;
}
#endif //DOTCOUNT

/*********************************************************************************************************/
//		Main ( )
/*********************************************************************************************************/
int main(int argc, _TCHAR* argv[])
//int main1(int argc, char* argv[])
{
#ifdef _DEBUG
	//_CrtSetBreakAlloc(192);
	_crtBreakAlloc = 186;
	atexit(DumpMemory);
#endif //_DEBUG

	char Dir[128];
	int size = GetCurrentDirectory(128, Dir);
	cout << "Dir : " << Dir << endl;	

#if Tiff_Test
	FullTest1();
	FullTest2();
	FullTest3();
	FullTest4();
#endif//

#if	Single_Test
	SingleTest();
#endif //Single_Test
#ifdef CREATE_TIFF
	CreateTiff(argc, argv);
#endif //CREATE_TIFF

#if 0
	ProcessTemplate()
#endif //0

#if	Tag_Test
	Tag_Test_Construct();
#endif //Tag_Test

#if	RGB2CMY
	RGB2CMYK();
#endif //Tag_Test

	//	char* lpBuf = new char[128];
#if DOTCOUNT
	if (argc < 2)
		cout << "DotCount In.tif" << endl;	
	else
		DotCount(argv[1]);
#endif //DOTCOUNT

#if	TEST
	void SaveHalftone();
	SaveHalftone();
#endif //TEST
	//cout << "test end" << endl;
	return 0;
}

