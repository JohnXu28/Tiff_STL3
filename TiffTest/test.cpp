#include "stdafx.h"
#include <string>
#include <iostream>
#include <memory>
#include <Windows.h>
#include <Tiff_STL3\Src\Tiff_STL3.h>
#include <Lch/cam02.h>
#include "Utility.h"
using namespace std;

void RGB2CMYK()
{
    CTiff* lpTiff = new CTiff("sRGB.tif");
    int Width = lpTiff->GetTagValue(ImageWidth);
    int Length = lpTiff->GetTagValue(ImageLength);
    CTiff* lpCMYK = new CTiff(Width, Length, 72, 4, 8);

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

#if LAB_TEST
void LAB_Test(_TCHAR* argv[])
{    
    //shared_ptr<CTiff> lpTiff = make_shared<CTiff>(argv[1]);
    shared_ptr<CTiff> lpTiff = make_shared<CTiff>("H:\\Git\\Tiff_STL3\\TestImg\\9Lab16.tif");
    int Width = lpTiff->GetTagValue(ImageWidth);
    int Length = lpTiff->GetTagValue(ImageLength);
    int Size = Width * Length;
    LPWORD lpIn = (LPWORD)lpTiff->GetImageBuf();
    LPWORD lpOut = lpIn;

    for (int i = 0; i < Size; i++)
    {
        WORD L = *(lpIn++);
        WORD a = *(lpIn++);
        WORD b = *(lpIn++);
        (*lpOut++) = L;
        (*lpOut++) = a ^ 0x8000;
        (*lpOut++) = b ^ 0x8000;
    }
    lpTiff->SetTagValue(PhotometricInterpretation, 9); /* ICC L*a*b* [Adobe TIFF Technote 4] */
    lpTiff->SaveFile("23IccLab16.tif");
}

#endif //LAB_TEST

#if CAM02_TEST
void CAM02_Test(_TCHAR* argv[])
{
    cout << "Cam02 Test" << endl;
    CAM02 cam02; //Default is D65
    
    double D65_XYZ[3] = { 95.05, 100, 108.89 };
    double JCH[3] = { 0 };
    cam02.Forward(D65_XYZ, JCH);
    cout << "D65 X, Y, Z: " << D65_XYZ[0] << " " << D65_XYZ[1] << " " << D65_XYZ[2] << endl;
    cout << "Cam02 Jch  : " << JCH[0] << " " << JCH[1] << " " << JCH[2] << endl;
}
#endif //CMA02

#if SWAP_TEST
void SWAP_Test(_TCHAR* argv[])
{
    //shared_ptr<CTiff> lpTiff = make_shared<CTiff>(argv[1]);
    shared_ptr<CTiff> lpTiff = make_shared<CTiff>("E:\\Temp\\Swap4.tif");
    int Width = lpTiff->GetTagValue(ImageWidth);
    int Length = lpTiff->GetTagValue(ImageLength);
    int Size = Width * Length;
    LPWORD lpIn = (LPWORD)lpTiff->GetImageBuf();
    
    SwapWORD_Buf(lpIn, Size/2);
    
    lpTiff->SaveFile("E:\\Temp\\SwapOut4.tif");
}

#endif //SWAP_TEST

#if GRAY2CMYK
void Gray2CMYK(_TCHAR* argv[])
{
    shared_ptr<CTiff> lpTiff = make_shared<CTiff>("H:\\Phoenix\\TestImg\\Gray255.tif");
    int Width = lpTiff->GetTagValue(ImageWidth);
    int Length = lpTiff->GetTagValue(ImageLength);
    shared_ptr<CTiff> lpCMYK = make_shared<CTiff>(Width, Length, 600, 4, 8);
    LPBYTE lpIn = lpTiff->GetImageBuf();
    LPBYTE lpOut = lpCMYK->GetImageBuf();

    int Size = Width * Length;
    //lpIn += 3;
    //lpOut += 3;
    for (int i = 0; i < Size; i++)
    {   
        lpOut++;
        lpOut++;        
        lpOut++;
        *(lpOut++) = 255 - *(lpIn++);
    }

    lpCMYK->SaveFile("H:\\Phoenix\\TestImg\\K255.tif");
}
#endif //GRAY2CMYK

void Test(int argc, _TCHAR* argv[])
{
#if	Tag_Test
    Tag_Test_Construct();
#endif //Tag_Test

#if	RGB2CMY
    RGB2CMYK();
#endif //Tag_Test

#if	TEST
    void SaveHalftone();
    //SaveHalftone();

    void Gray2K(int argc, _TCHAR * argv[]);
    Gray2K(argc, argv);
#endif //TEST

#if	LAB_TEST
    LAB_Test(argv);
#endif //Tag_Test

#if	CAM02_TEST
    CAM02_Test(argv);
#endif //Tag_Test


#if	SWAP_TEST
    SWAP_Test(argv);
#endif //SWAP_TEST

#if GRAY2CMYK
    Gray2CMYK(argv);
#endif //GRAY2CMYK
    //cout << "test end" << endl;
}