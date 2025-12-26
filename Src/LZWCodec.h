#ifndef LZW_CODEC_H
#define LZW_CODEC_H

#include <cstdint>

//
//#if 0
//void TIFF_UndoPredictor_Generic(     
//    uint8_t* buf,
//    int width,
//    int rows,
//    int samplesPerPixel,
//    int bitsPerSample);
//
//bool TIFF_LZW_DecompressStrip_AutoDetect(
//    const uint8_t* in, int inSize,
//    uint8_t* out, int outCapacity,
//    int* outSize);
//#endif //0

int LZW_Compress(uint8_t* in, int InSize, uint8_t* out, int* OutSize);
int LZW_Decompress(uint8_t* in, int InSize, uint8_t* out, int* OutSize);

#endif // LZW_CODEC_H