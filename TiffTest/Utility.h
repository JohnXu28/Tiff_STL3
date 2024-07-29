//Header file for utility and test
#if !defined(_TIFF_UTILITY_)
#define _TIFF_UTILITY_

#define DOTCOUNT 0
#define RAW2TIFF 0
#define TIFF2RAW 0
#define Tiff_SWAPDWORD 0
#define RGB2CMY 0
#define GRAY2K	0
#define TIFF2DAT 0
#define TEST 0
#define Tag_Test 0
#define LAB_TEST 0
#define CAM02_TEST 0
#define SWAP_TEST 0
#define TIFF_APPEND 0
#define GRAY2CMYK 0
#define CMYK1_CMYK8 1
#define KYM_Tiff 0

void Utility(int argc, char* argv[]);
void Test(int argc, char* argv[]);
int Tiff_Append(char* Tar, char* Src);
#endif //_TIFF_UTILITY_