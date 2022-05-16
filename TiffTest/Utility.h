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

void Utility(int argc, char* argv[]);
void Test(int argc, char* argv[]);
#endif //_TIFF_UTILITY_