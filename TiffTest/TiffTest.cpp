// TiffTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <string>
#include <iostream>
using namespace std;

#define John
//#define Luke
//#define Chunyen_yang

#ifdef John
//#define Tiff_C

#define Tiff_Test 1
#define Tiff_STL
//#define Tag_Test 1
//#define CREATE_TIFF
//#define CREATE_TIFF_4_1
#define FREQUENCE 1

#ifdef Tiff_STL
#include <Src\tiff_stl3.h>

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

#ifdef Tiff_C
#include "tiff_c.h"
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
enum TiffTagSignature{
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

	if (Tiff_ReadFile(lpTiff, FileIn.c_str()) != 0)
	{
		Tiff_Close(lpTiff);
		return -1;
	}

	if (Tiff_ReadFile(lpTiff, FileIn.c_str()) != 0)//Test for Memory Leaks.
	{
		Tiff_Close(lpTiff);
		return -1;
	}

	if (Tiff_SaveFile(lpTiff, FileOut.c_str()) != 0)
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

int ImageTest2(string FileName)
{
	string FileIn = "Output\\" + FileName + "out.tif";
	string FileOut = "Output\\" + FileName + "out2.tif";

	cout << "Copy Test : " << FileName.c_str() << "out.tif\n";
	STiff *lpTiff = Tiff_Create();
	STiff *lpTiff2 = Tiff_Create();
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
	unsigned char *lpTemp1 = (unsigned char *)Tiff_GetImageBuf(lpTiff);
	unsigned char * lpTemp2 = (unsigned char *)Tiff_GetImageBuf(lpTiff2);
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

int ImageTest3(string FileName)
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
"16MultiStrip", "17Ycc8", "18"};

const int TestNum = 19;

int FullTest1()
{
	for (int i = 1; i < TestNum; i++)
		if (ImageTest1(FileName[i]) != 0)
			return -1;
	return 0;
}

int FullTest2()
{
	for (int i = 1; i < TestNum; i++)
		if (ImageTest2(FileName[i]) != 0)
			return -1;
	return 0;
}

int FullTest3()
{
	for (int i = 1; i < TestNum; i++)
		if (ImageTest3(FileName[i]) != 0)
			return -1;
	return 0;
}

void SingleTest()
{
	STiff *lpTiff = Tiff_Create();
	Tiff_ReadFile(lpTiff, "10RGB82.tif");
	Tiff_SaveFile(lpTiff, "10RGB82Out.tif");
	Tiff_Close(lpTiff);
}

#endif //Tiff_Test
class GL
{
public:
	GL(){};
	~GL(){ 
		cout << "End" << endl; 
	};
	void display(){ cout << "GL" << endl; }
	char Data[65536];
};

GL *gl = new GL;
GL g2;

void DumpMemory(void)
{
	delete gl;
	DETECT_MEMORY_LEAKS;
}

#if Tag_Test

//TiffTag& NEWTAG()
//{
//	TiffTag NewTag(ImageWidth, Long, 1, 100, nullptr);
//	return NewTag;
//}

void Tag_Test_Construct()
{
	TiffTag NewTag(ImageWidth, Long, 1, 100, nullptr);
	TiffTag Temp1, Temp2;
	Temp1 = NewTag;

	Temp2 = TiffTag(TiffTag(ImageWidth, Long, 1, 100, nullptr));

	//LPBYTE lpBuf = new BYTE[1024];

}
#endif //Tag_Test

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
		CTiff tiff;
		tiff.CreateNew(width, length, resolution, samplesperpixel, bitspersample, raw, tif);
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

void CREATE_TIFF41()
{
	FILE *C, *M, *Y, *K;
	C = fopen("4800x6784_errdif_c.bin", "rb");
	M = fopen("4800x6784_errdif_m.bin", "rb");
	Y = fopen("4800x6784_errdif_y.bin", "rb");
	K = fopen("4800x6784_errdif_k.bin", "rb");
	int Width = 4800;
	int Length = 6784;
	int BytesPerLine = Width / 8;
	CTiff Out;
	Out.CreateNew(Width, Length, 600, 4, 1);

	LPBYTE lpC = new BYTE[BytesPerLine];
	LPBYTE lpM = new BYTE[BytesPerLine];
	LPBYTE lpY = new BYTE[BytesPerLine];
	LPBYTE lpK = new BYTE[BytesPerLine];
	LPBYTE lpOut = Out.GetImageBuf();

	for (int i = 0; i < Length; i++)
	{
		fread(lpC, 1, BytesPerLine, C);
		memcpy(lpOut, lpC, BytesPerLine);
		lpOut += BytesPerLine;

		fread(lpM, 1, BytesPerLine, M);
		memcpy(lpOut, lpM, BytesPerLine);
		lpOut += BytesPerLine;

		fread(lpY, 1, BytesPerLine, Y);
		memcpy(lpOut, lpY, BytesPerLine);
		lpOut += BytesPerLine;

		fread(lpK, 1, BytesPerLine, K);
		memcpy(lpOut, lpK, BytesPerLine);
		lpOut += BytesPerLine;
	}

	Out.SaveFile("CMYK_err.tif");

	delete[]lpC;
	delete[]lpM;
	delete[]lpY;
	delete[]lpK;
	fclose(C);
	fclose(M);
	fclose(Y);
	fclose(K);
}


/*********************************************************************************************************/
//		Main ( )
/*********************************************************************************************************/
int main(int argc, _TCHAR* argv[])
//int main1(int argc, char* argv[])
{	
#ifdef _DEBUG
	//_CrtSetBreakAlloc(192);
	_crtBreakAlloc = 198;
	atexit(DumpMemory);
#endif //_DEBUG

#if Tiff_Test
	FullTest1();
	FullTest2();
	FullTest3();
	//	SingleTest();
#endif//

#ifdef CREATE_TIFF
	CreateTiff(argc, argv);
#endif //CREATE_TIFF

#if 0
	ProcessTemplate()
#endif //0

#ifdef CREATE_TIFF_4_1
		//Read 4 files (CMYK 1bit raw) combine to 1 tif.
		CREATE_TIFF41();
#endif //CREATE_TIFF_4_1

#ifdef	Tag_Test
	Tag_Test_Construct();
#endif //Tag_Test

	return 0;
}

