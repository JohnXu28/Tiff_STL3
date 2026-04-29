#ifndef LZW_CODEC_H
#define LZW_CODEC_H

#include <stdint.h>
struct Entry {
    uint16_t prefix;
    uint8_t  suffix;
};

class Lzw_Perplexity
{
private:
    uint8_t FirstChar(struct Entry* dict, int code);
	void ResetDict(Entry* dict, int& nextCode, int& codeBits);

public:
    Lzw_Perplexity() {};
    ~Lzw_Perplexity() {}
    bool Decode(
        const uint8_t* in, int inSize,
        uint8_t* out, int outCapacity,
        int* outSize);

    bool Encode(
        const uint8_t* in, int inSize,
        uint8_t* out, int outCapacity,
        int* outSize);

    //Your original Predictor can use our previous Generic version.
    void PredicatorDecode(
        uint8_t* buf,
        int width,
        int rows,
        int samplesPerPixel,
        int bitsPerSample = 8);

    
    void PredicatorEncode(
        uint8_t* buf,
        int width,
        int rows,
        int samplesPerPixel,
        int bitsPerSample = 8);

    
    //Entry dict[LZW_MAX_DICT];
    Entry dict[4096];
};
#endif
