#include "stdafx.h"
#include <string>
#include <iostream>
#include <memory>
#include "Utility.h"
#include <Tiff_STL3\Src\Tiff_STL3.h>
using namespace std;

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
		cout << "DotCount : " << dotcount << ", Percent : " << (double)dotcount / (Width * Length) << endl;
}
#endif //DOTCOUNT

#if RAW2TIFF
void CreateTiff(int argc, _TCHAR* argv[])
{
	if (argc != 8)
	{
		cout << "Creatiff Width Length Resolution Samplesperpixel Bitspersample In.raw out.tif" << endl;
		return;
	}	
	
	int width = atoi(argv[1]);
	int length = atoi(argv[2]);
	int resolution = atoi(argv[3]);
	int samplesperpixel = atoi(argv[4]);
	int bitspersample = atoi(argv[5]);
	char* raw = argv[6];
	char* tif = argv[7];
	cout << "Width : " << width << endl;
	cout << "Length : " << length << endl;
	cout << "Resolution : " << resolution << endl;
	cout << "Samplesperpixel : " << samplesperpixel << endl;
	cout << "Bitspersample : " << bitspersample << endl;
	cout << "Raw : " << raw << endl;
	cout << "Tif : " << tif << endl;
	CTiff* lpTiff = new CTiff;
	lpTiff->CreateNew(width, length, resolution, samplesperpixel, bitspersample, raw, tif);
}
#endif //RAW2TIFF

#if TIFF2RAW
void Tiff2Raw(int argc, _TCHAR* argv[])
{
	if (argc < 2)
	{		
		cout << "Tiff2Raw in.tif" << endl;
		return;
	}

	shared_ptr<CTiff> lpTiff = make_shared<CTiff>(argv[1]);
	int Width = lpTiff->GetTagValue(ImageWidth);
	if (Width == 0)//open fail
		return;

	int Length = lpTiff->GetTagValue(ImageLength);
	int samplesPerPixel = lpTiff->GetTagValue(SamplesPerPixel);
	int bitsPerSample = lpTiff->GetTagValue(BitsPerSample);
	int compress = lpTiff->GetTagValue(Compression);
	int Size = Width * Length * samplesPerPixel * bitsPerSample / 8;

	char FileName[32] = {0};
	sprintf(FileName, "%dX%d_%d_%d.raw", Width, Length, samplesPerPixel, bitsPerSample);
	LPBYTE lpIndex = lpTiff->GetImageBuf();

	FILE* file = fopen(FileName, "wb+");
	fwrite(lpIndex, 1, Size, file);
	fclose(file);

	cout << "Output File : " << FileName << endl;
}
#endif //TIFF2RAW


#if GRAY2K
void Gray2K(int argc, _TCHAR* argv[])
{
	if (argc < 3)
	{
		cout << "Tools for IPS, Convert Gray to K." << endl;
		cout << "Gray2K in.tif out.tif" << endl;
		return;
	}

	shared_ptr<CTiff> lpTiff = make_shared<CTiff>(argv[1]);
	int Width = lpTiff->GetTagValue(ImageWidth);
	if (Width == 0)//open fail
	{
		cout << "Width is 0." << endl;
		return;
	}

	if (lpTiff->GetTagValue(PhotometricInterpretation) == 0)//open fail
	{
		cout << "PhotometricInterpretation is 0." << endl;
		return;
	}


	int Length = lpTiff->GetTagValue(ImageLength);
	int samplesPerPixel = lpTiff->GetTagValue(SamplesPerPixel);
	int bitsPerSample = lpTiff->GetTagValue(BitsPerSample);
	int compress = lpTiff->GetTagValue(Compression);

	lpTiff->SetTagValue(PhotometricInterpretation, 0);

	LPBYTE lpIn = lpTiff->GetImageBuf();
	LPBYTE lpOut = lpTiff->GetImageBuf();
	int Size = Width * Length;

	for (int i = 0; i < Size; i++)
	{
		*(lpOut++) = 255 - *(lpIn++);
	}

	lpTiff->SaveFile(argv[2]);
}
#endif //GRAY2K


#if Tiff_SWAPDWORD
void Tiff_SwapDword(char* FileName)
{
	CTiff Tiff(FileName);
	int Width = Tiff.GetTagValue(ImageWidth);
	int Length = Tiff.GetTagValue(ImageLength);

	if ((Width % 4) != 0)
	{
		cout << "Width % 4 : " << Width % 4 << endl;
		cout << "Just return." << endl;
	}

	LPBYTE lpbuf = Tiff.GetImageBuf();
	SwapWORD_Buf((LPWORD)lpbuf, (Width / 2) * Length);
	Tiff.SaveFile("out.tif");

}
#endif //Tiff_SWAPDWORD

#if TIFF2DAT
void Tiff2Dat(int argc, _TCHAR* argv[])
{
	if (argc < 3)
	{
		cout << "Tools for IPS2020" << endl;
		cout << "Tiff2Dat in.tif out.dat" << endl;
		return;
	}

	shared_ptr<CTiff> lpTiff = make_shared<CTiff>(argv[1]);
	int Width = lpTiff->GetTagValue(ImageWidth);
	if (Width == 0)//open fail
		return;

	int Length = lpTiff->GetTagValue(ImageLength);
	int samplesPerPixel = lpTiff->GetTagValue(SamplesPerPixel);
	int bitsPerSample = lpTiff->GetTagValue(BitsPerSample);
	int compress = lpTiff->GetTagValue(Compression);

	LPBYTE lpIndex = lpTiff->GetImageBuf();
	FILE* file = fopen(argv[2], "wb+");
	for (int i = 0; i < Length; i++)
	{
		for (int j = 0; j < Width; j++)
		{
			for (int k = 0; k < samplesPerPixel; k++)
			{
				fprintf(file, "%3d,", *(lpIndex++));
			}
		}
		fprintf(file, "\n");
	}

	fclose(file);
}
#endif //TIFF2DAT

#if CMYK8_CMYK1
//LUT 1bit to 8 bits
unsigned int LUT18[16] = {
	0x00000000, //0000
	0x000000FF, //0001
	0x0000FF00, //0010
	0x0000FFFF, //0011
	0x00FF0000, //0100
	0x00FF00FF, //0101
	0x00FFFF00, //0110
	0x00FFFFFF, //0111
	0xFF000000, //1000
	0xFF0000FF, //1001
	0xFF00FF00, //1010
	0xFF00FFFF, //1011
	0xFFFF0000, //1100
	0xFFFF00FF, //1101
	0xFFFFFF00, //1110
	0xFFFFFFFF, //1111
};

void Extend_1To8(LPBYTE lpIn1, LPBYTE lpOut8, int width)
{
	LPBYTE lpIn = lpIn1;
	LPINT lpOut = (LPINT)lpOut8;
	BYTE Hi, Lo;
	for (int i = 0; i < width; i++)
	{
		Hi = (*lpIn & 0xF0) >> 4;
		Lo = *(lpIn) & 0x0F;
		lpIn++;
		*(lpOut++) = LUT18[Hi];
		*(lpOut++) = LUT18[Lo];
	}
}

void CMYK8_KYMC(int argc, _TCHAR* argv[])
{
	if (argc != 8)
	{
		cout << "KYMC2Tiff Width Length Resolution Samplesperpixel Bitspersample In.raw out.tif" << endl;
		cout << "Input KYMC 1bit, Output CMYK 8 bits" << endl;
		return;
	}

	int width = atoi(argv[1]);
	int length = atoi(argv[2]);
	int resolution = atoi(argv[3]);
	int samplesperpixel = atoi(argv[4]);
	int bitspersample = atoi(argv[5]);
	char* raw = argv[6];
	char* tif = argv[7];
	cout << "Width : " << width << endl;
	cout << "Length : " << length << endl;
	cout << "Resolution : " << resolution << endl;
	cout << "Samplesperpixel : " << samplesperpixel << endl;
	cout << "Bitspersample : " << bitspersample << endl;
	cout << "Raw : " << raw << endl;
	cout << "Tif : " << tif << endl;

	shared_ptr<CTiff> lpTiff = make_shared<CTiff>(width, length, resolution, 4, 8);
	FILE* file = fopen(raw, "rb");
	int bytesPerLine1 = width / 8;
	int bytesPerLine8 = width * 4;
	LPBYTE lpIn = new BYTE[bytesPerLine1];
	LPBYTE lpC = new BYTE[width];
	LPBYTE lpM = new BYTE[width];
	LPBYTE lpY = new BYTE[width];
	LPBYTE lpK = new BYTE[width];
	LPBYTE lpOut = new BYTE[bytesPerLine8];
	memset(lpC, 0, width);
	memset(lpM, 0, width);
	memset(lpY, 0, width);
	memset(lpK, 0, width);

	for (int i = 0; i < length; i++)
	{
		fread(lpIn, 1, bytesPerLine1, file);		
		Extend_1To8(lpIn, lpK, bytesPerLine1);

		fread(lpIn, 1, bytesPerLine1, file);		
		Extend_1To8(lpIn, lpY, bytesPerLine1);

		fread(lpIn, 1, bytesPerLine1, file);		
		Extend_1To8(lpIn, lpM, bytesPerLine1);

		fread(lpIn, 1, bytesPerLine1, file);		
		Extend_1To8(lpIn, lpC, bytesPerLine1);
		
		LPBYTE lpTemp = lpOut;
		LPBYTE lpTempC = lpC;
		LPBYTE lpTempM = lpM;
		LPBYTE lpTempY = lpY;
		LPBYTE lpTempK = lpK;
		for (int j = 0; j < width; j++)
		{
			*(lpTemp++) = *(lpTempC++);
			*(lpTemp++) = *(lpTempM++);
			*(lpTemp++) = *(lpTempY++);
			*(lpTemp++) = *(lpTempK++);
		}
		lpTiff->PutRow(lpOut, i);
	}
	
	fclose(file);
	lpTiff->SaveFile(tif);
	delete[]lpIn;
	delete[]lpC;
	delete[]lpM;
	delete[]lpY;
	delete[]lpK;
	delete[]lpOut;
	
}
#endif //CMYK8_CMYK1

void Utility(int argc, _TCHAR* argv[])
{
#if RAW2TIFF
	CreateTiff(argc, argv);
#endif //CREATE_TIFF

#if TIFF2RAW
	Tiff2Raw(argc, argv);
#endif //TIFF2RAW

	//	char* lpBuf = new char[128];
#if DOTCOUNT
	if (argc < 2)
		cout << "DotCount In.tif" << endl;
	else
		DotCount(argv[1]);
#endif //DOTCOUNT

#if TIFF2DAT	
	Tiff2Dat(argc, argv);
#endif //TIFF2DAT

#if Tiff_SWAPDWORD
	Tiff_SwapDword(argv[1]);
#endif //Tiff_SWAPDWORD

#if GRAY2K
	Gray2K(argc, argv);
#endif //GRAY2K

#if CMYK8_CMYK1
	CMYK8_KYMC(argc, argv);
#endif //GRAY2K

}