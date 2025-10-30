// TiffTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <string>
#include <iostream>
#include <memory>
#include "Utility.h"
using namespace std;

#define John
//#define Luke
//#define Chunyen_yang

#ifdef John
#define Tiff_C		0
#define Tiff_STL3	1

#define Tiff_Test	0
#define Single_Test	0
#define RGBA		0 //Alpha

#if Tiff_STL3
#include <Tiff_STL3\Src\Tiff_STL3.h>

#define STiff CTiff
#define Tiff_Create() new CTiff
#define Tiff_Clone() lpTiff->Clone()
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
	STiff* lpTiff = Tiff_Create();
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
void TiffCopy(STiff* lpTiffDest, STiff* lpTiffSrc)
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
	STiff* lpTiff = Tiff_Create();
	Tiff_ReadFile(lpTiff, FileIn.c_str());

	//Compare
	int ret = 0;
	int stripByteCounts = Tiff_GetTagValue(lpTiff, StripByteCounts);
	unsigned char* lpTemp1 = (unsigned char*)Tiff_GetImageBuf(lpTiff);

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

	Tiff_ReadFile(lpTiff, FileIn.c_str());
	STiff* lpTiff2 = Tiff_Clone();

	//For memory leak test.
	/*Tiff_CreateNew(lpTiff2, Tiff_GetTagValue(lpTiff, ImageWidth),
		Tiff_GetTagValue(lpTiff, ImageLength),
		Tiff_GetTagValue(lpTiff, XResolution),
		Tiff_GetTagValue(lpTiff, SamplesPerPixel),
		Tiff_GetTagValue(lpTiff, BitsPerSample)
	);*/

	//copy test
	if (Tiff_GetTagValue(lpTiff, BitsPerSample) == 16)
		TiffCopy<unsigned short>(lpTiff2, lpTiff);
	else
		TiffCopy<unsigned char>(lpTiff2, lpTiff);

	//For Lab Color Image 
	if (Tiff_GetTagValue(lpTiff, PhotometricInterpretation) == 8)
		Tiff_SetTagValue(lpTiff2, PhotometricInterpretation, 8);
	if (Tiff_GetTagValue(lpTiff, PhotometricInterpretation) == 9)
		Tiff_SetTagValue(lpTiff2, PhotometricInterpretation, 9);

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
int GetXY_Test(STiff* lpTiff1, STiff* lpTiff2)
{
	if (Tiff_GetTagValue(lpTiff1, BitsPerSample) == 1)
		return 0;
	int Width = Tiff_GetTagValue(lpTiff1, ImageWidth);
	int Length = Tiff_GetTagValue(lpTiff1, ImageLength);
	int samplesPerPixel = Tiff_GetTagValue(lpTiff1, SamplesPerPixel);
	T* lpTemp1 = (T*)Tiff_GetImageBuf(lpTiff1);
	T* lpTemp2;

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
	STiff* lpTiff1 = Tiff_Create();
	STiff* lpTiff2 = Tiff_Create();
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

string FileName[] = {
"0NULL", "1LineArt", "2gray8", "3RGB8", "4CMYK8",
"5Lab8", "6gray16", "7RGB16", "8CMYK16", "9Lab16",
"10RGB82", "11RGB162", "12CMYK82", "13CMYK162", "14Lab82",
"15Lab162","16MultiStrip", "17Ycc8", "18CLR68", "19CLR616",
"20IccLab8", "21IccLab16", "22Alpha8", "23Alpha16" };

const int TestNum = 24;

int FullTest1()
{
	for (int i = 1; i < TestNum; i++)
		if (ImageTest1(FileName[i]) != 0)
		{
			cout << "Test 1 fail, FileName : " << FileName[i] << endl;
			//return -1;
		}
	return 0;
}

int FullTest2()
{
	for (int i = 2; i < TestNum; i++)
		if (ImageTest2(FileName[i]) != 0)
		{
			cout << "Test 2 fail, FileName : " << FileName[i] << endl;
			//return -1;
		}

	return 0;
}

int FullTest3()
{
	for (int i = 1; i < TestNum; i++)
		if (ImageTest3(FileName[i]) != 0)
		{
			cout << "Test 3 fail, FileName : " << FileName[i] << endl;
			//return -1;
		}

	return 0;
}

int FullTest4()
{
	for (int i = 1; i < TestNum; i++)
		if (ImageTest4(FileName[i]) != 0)
		{
			cout << "Test 4 fail, FileName : " << FileName[i] << endl;
			//return -1;
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

#if RGBA //Alpha channel
void Tiff_RGBA_Test()
{
	shared_ptr<CTiff> lpTiff = make_shared<CTiff>("H:\\Git\\Tiff_STL3\\TestImg\\3RGB8.tif");
	int Width = lpTiff->GetTagValue(ImageWidth);
	int Length = lpTiff->GetTagValue(ImageLength);
	int Size = Width * Length;
	shared_ptr<CTiff> lpTiffOut = make_shared<CTiff>();
	lpTiffOut->CreateNew(Width, Length, 100, 4, 8);
	LPBYTE lpIn = (LPBYTE)lpTiff->GetImageBuf();
	LPBYTE lpOut = (LPBYTE)lpTiffOut->GetImageBuf();
	for (int i = 0; i < Size; i++)
	{
		*(lpOut++) = *(lpIn++);
		*(lpOut++) = *(lpIn++);
		*(lpOut++) = *(lpIn++);
		*(lpOut++) = 255;

	}
	lpTiffOut->SetTagValue(PhotometricInterpretation, 2);
	lpTiffOut->SetTagValue(SamplesPerPixel, 4);

	lpTiffOut->SetTag(ExtraSamples, (FieldType)1, 1, 1); //Add Alpha channel tag

	lpTiffOut->SaveFile("20Alpha8.tif");
}
#endif //RGBA

void DumpMemory(void)
{
	//delete gl;
	DETECT_MEMORY_LEAKS;
}


#if 0
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

#endif //0


/*********************************************************************************************************/
//		Main ( )
/*********************************************************************************************************/
int main(int argc, _TCHAR* argv[])
//int main1(int argc, char* argv[])
{
#ifdef _DEBUG
	//_CrtSetBreakAlloc(192);
	//_crtBreakAlloc = 205;
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

#if RGBA
	Tiff_RGBA_Test();
#endif //RGBA

#if TIFF_APPEND	
	Tiff_Append("In0.tif", "In1L.tif");
	Tiff_Append("In0.tif", "In2J.tif");
#endif //TIFF_APPEND

#if 0
	ProcessTemplate()
#endif //0

		Utility(argc, argv);
	Test(argc, argv);
	return 0;
}

