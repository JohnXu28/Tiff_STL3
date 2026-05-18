#pragma once

#include "../Src/Tiff_STL3.h"
#ifdef _WINDOWS
	#include <WinGDI.h>
#else
typedef long LONG;
typedef uint16_t    WORD;
typedef uint32_t    DWORD;

typedef	struct tagBITMAPFILEHEADER {
		WORD    bfType;
		DWORD   bfSize;
		WORD    bfReserved1;
		WORD    bfReserved2;
		DWORD   bfOffBits;
	} BITMAPFILEHEADER, *PBITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER {
	DWORD      biSize;
	LONG       biWidth;
	LONG       biHeight;
	WORD       biPlanes;
	WORD       biBitCount;
	DWORD      biCompression;
	DWORD      biSizeImage;
	LONG       biXPelsPerMeter;
	LONG       biYPelsPerMeter;
	DWORD      biClrUsed;
	DWORD      biClrImportant;
} BITMAPINFOHEADER, *PBITMAPINFOHEADER;
#endif //_ WINDOWS
class BMP 
{
public:
	BMP(void);
	~BMP(void);
	int ReadTiff(char *Input);
	int SaveTiff(char *Output);
	int ReadFile(char *Inupt);
	int SaveFile(char *FileName);
	int SaveFile_ASCII(char *FileName);
	int Tiff2BMP(char *Input, char*Output);
	int Bin2BMP(char *InputFile, int Width, int Length, char *OutputFile);
	CTiff *m_lpTiff;	
private:
	void BGR2RGB(LPBYTE lpIn, LPBYTE lpOut, int Size);
	void RGB2BGR(LPBYTE lpIn, LPBYTE lpOut, int Size);
	string m_FileName;	
	BITMAPFILEHEADER m_Header;
	BITMAPINFOHEADER m_Info;
};

#define BMP_TEST 0
#if BMP_TEST

class BMP_Test:public CBase
{
public:
	BMP_Test();
	virtual ~BMP_Test();
	virtual int Initial();
	virtual int main();
	virtual char* Name(){return "BMP_Test";};
private:
};
#endif //BMP_TEST

