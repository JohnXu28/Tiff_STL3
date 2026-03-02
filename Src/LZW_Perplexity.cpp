// LZWCodec.cpp
#include "stdafx.h"
#include "Lzw_Perplexity.h"
#include <string.h>

#define TIFF_LZW_MAX_BITS  12
#define TIFF_LZW_MAX_DICT  4096
#define TIFF_CLEAR_CODE    256
#define TIFF_EOI_CODE      257
#define TIFF_FIRST_CODE    258

#define LZW_MAX_BITS  12
#define LZW_MAX_DICT  4096
#define LZW_CLEAR     256
#define LZW_EOI       257
#define LZW_FIRST     258

struct LZWEntry {
    uint16_t prefix;
    uint8_t  suffix;
};

struct LZWDict {
    LZWEntry entry[TIFF_LZW_MAX_DICT];
    uint16_t size;
    uint8_t  codeBits;
    uint16_t nextLimit;
};

struct BitReader {
    const uint8_t* buf;
    int size;
    int bytePos;
    int bitPos;
};

static inline void BR_Reset(BitReader* br)
{
    br->bytePos = 0;
    br->bitPos = 0;
}

// LSB-first bit reader (TIFF)
static bool ReadBitsLSB(BitReader* br, int bits, uint16_t* code)
{
    uint32_t v = 0;
    int read = 0;

    while (read < bits) {
        if (br->bytePos >= br->size) return false;

        uint8_t cur = br->buf[br->bytePos] >> br->bitPos;
        int take = 8 - br->bitPos;
        if (take > bits - read) take = bits - read;

        v |= (uint32_t)(cur & ((1u << take) - 1u)) << read;

        br->bitPos += take;
        read += take;

        if (br->bitPos == 8) {
            br->bitPos = 0;
            br->bytePos++;
        }
    }

    *code = (uint16_t)v;
    return true;
}

// MSB-first bit reader (Photoshop/GIF style)
static bool ReadBitsMSB(BitReader* br, int bits, uint16_t* code)
{
    uint32_t v = 0;

    for (int i = 0; i < bits; ++i) {
        if (br->bytePos >= br->size) return false;

        uint8_t cur = br->buf[br->bytePos];
        int bit = (cur >> (7 - br->bitPos)) & 1;

        v = (v << 1) | bit;

        br->bitPos++;
        if (br->bitPos == 8) {
            br->bitPos = 0;
            br->bytePos++;
        }
    }

    *code = (uint16_t)v;
    return true;
}

enum LZWBitOrder {
    LZW_LSB_FIRST = 0,
    LZW_MSB_FIRST = 1
};

static bool DetectBitOrder(const uint8_t* in, int inSize, LZWBitOrder* order)
{
    BitReader br = { in, inSize, 0, 0 };
    uint16_t code = 0;

    // Try LSB-first
    if (ReadBitsLSB(&br, 9, &code) && code == TIFF_CLEAR_CODE) {
        *order = LZW_LSB_FIRST;
        return true;
    }

    // Try MSB-first
    BR_Reset(&br);
    if (ReadBitsMSB(&br, 9, &code) && code == TIFF_CLEAR_CODE) {
        *order = LZW_MSB_FIRST;
        return true;
    }

    // No CLEAR found; treat as invalid
    return false;
}

static void LZW_Reset(LZWDict* d)
{
    d->size = TIFF_FIRST_CODE;
    d->codeBits = 9;
    d->nextLimit = 1 << d->codeBits;

    for (int i = 0; i < 256; ++i) {
        d->entry[i].prefix = 0xFFFF;
        d->entry[i].suffix = (uint8_t)i;
    }
}

uint8_t Lzw_Perplexity::FirstChar(struct Entry* dict, int code) 
{
    while (dict[code].prefix != 0xFFFF) code = dict[code].prefix;
    return dict[code].suffix;
}
// Core TIFF LZW decode, automatic bit-order detection
bool Lzw_Perplexity::Decode(
    const uint8_t* in, int inSize,
    uint8_t* out, int outCapacity,
    int* outSize)
{
    if (!in || !out || inSize <= 0) return false;

    Entry dict[LZW_MAX_DICT];

    auto ResetDict = [&](int& nextCode, int& codeBits) {
        for (int i = 0; i < 256; i++) {
            dict[i] = { 0xFFFF, (uint8_t)i };
        }
        nextCode = LZW_FIRST;
        codeBits = 9;
        };

    int nextCode, codeBits;
    ResetDict(nextCode, codeBits);

    uint32_t bitBuf = 0;
    int bitCount = 0;
    int inPos = 0;
    int outPos = 0;

    // 輔助 Lambda: 讀取指定位元 (TIFF 預設是 MSB-first bit filling but LSB-first bits)
    // 註：多數 TIFF 實作為 LSB-first
    auto GetCode = [&]() -> int {
        while (bitCount < codeBits) {
            if (inPos >= inSize) return -1;
            bitBuf |= (uint32_t)in[inPos++] << (32 - 8 - bitCount);
            bitCount += 8;
        }
        uint32_t code = bitBuf >> (32 - codeBits);
        bitBuf <<= codeBits;
        bitCount -= codeBits;
        return (int)code;
        };

    int code, oldCode = -1;
    uint8_t stack[LZW_MAX_DICT];

    while ((code = GetCode()) != -1 && code != LZW_EOI) {
        if (code == LZW_CLEAR) {
            ResetDict(nextCode, codeBits);
            oldCode = -1;
            continue;
        }

        int cur = code;
        int sp = 0;

        // 處理字典中不存在的 Code (K-W-K 情況)
        if (cur >= nextCode) {
            if (oldCode == -1 || cur > nextCode) return false; // 損壞
            stack[sp++] = (uint8_t)FirstChar((Entry*)dict, oldCode);
            cur = oldCode;
        }

        // 展開 Code
        while (cur != 0xFFFF && cur < LZW_MAX_DICT) {
            stack[sp++] = dict[cur].suffix;
            cur = dict[cur].prefix;
            if (sp >= LZW_MAX_DICT) return false;
        }

        // 寫入輸出
        if (outPos + sp > outCapacity) return false;
        for (int i = sp - 1; i >= 0; i--) {
            out[outPos++] = stack[i];
        }

        // 更新字典
        if (oldCode != -1 && nextCode < LZW_MAX_DICT) {
            dict[nextCode].prefix = (uint16_t)oldCode;
            dict[nextCode].suffix = (uint8_t)FirstChar(dict, oldCode);
            nextCode++;

            // TIFF 規格：當 nextCode 為 511, 1023, 2047 時增加位元
            // 注意：這是 TIFF 的 "Early Change" 邏輯
            if (nextCode == 511) codeBits = 10;
            else if (nextCode == 1023) codeBits = 11;
            else if (nextCode == 2047) codeBits = 12;
        }
        oldCode = code;
    }

    *outSize = outPos;
    return true;
}

//Read Tiff with Predictor=2, call this function to restore original pixel data.
void Lzw_Perplexity::PredicatorDecode(
    uint8_t* buf,
    int width,
    int rows,
    int samplesPerPixel,
    int bitsPerSample)
{
    // Process only 8 or 16 bits per sample
    if (bitsPerSample != 8 && bitsPerSample != 16)
        return;

    int bytesPerSample = bitsPerSample / 8;
    int bytesPerPixel = samplesPerPixel * bytesPerSample;
    int rowBytes = width * bytesPerPixel;

    for (int y = 0; y < rows; ++y) {
        uint8_t* row = buf + y * rowBytes;

        if (bitsPerSample == 8) {
            // Your original implementation, but with the step size adjusted: measured in “one pixel” units
            for (int x = bytesPerPixel; x < rowBytes; ++x) {
                row[x] = (uint8_t)(row[x] + row[x - bytesPerPixel]);
            }
        }
        else { // 16-bit, used for predictor, Avoid carry misalignment            
            uint16_t* row16 = (uint16_t*)row;
            int pixelsInRow = width * samplesPerPixel;

            for (int i = 1; i < pixelsInRow; ++i) {
                row16[i] = (uint16_t)(row16[i] + row16[i - 1]);
            }
        }
    }
}

//Encod.
// --- Bit writer (LSB-first, TIFF spec) ------------------------------

struct BitWriter {
    uint8_t* buf;
    int size;
    int bytePos;
    int bitPos;
    bool overflow;
};

static inline void BW_Init(BitWriter* bw, uint8_t* outBuf, int outSize)
{
    bw->buf = outBuf;
    bw->size = outSize;
    bw->bytePos = 0;
    bw->bitPos = 0;
    bw->overflow = false;
}

static inline void BW_WriteBits(BitWriter* bw, uint16_t code, int bits)
{
    if (bw->overflow) return;

    uint32_t v = code;
    int written = 0;

    while (written < bits) {
        if (bw->bytePos >= bw->size) {
            bw->overflow = true;
            return;
        }

        int take = 8 - bw->bitPos;
        if (take > bits - written) take = bits - written;

        uint8_t mask = (uint8_t)((1u << take) - 1u);
        uint8_t bitsv = (uint8_t)((v >> written) & mask);

        bw->buf[bw->bytePos] &= (uint8_t)~(mask << bw->bitPos);
        bw->buf[bw->bytePos] |= (uint8_t)(bitsv << bw->bitPos);

        bw->bitPos += take;
        written += take;

        if (bw->bitPos == 8) {
            bw->bitPos = 0;
            bw->bytePos++;
        }
    }
}

static inline int BW_Finish(BitWriter* bw)
{    
	// if the last byte is partially filled, it still counts as one byte
    if (bw->overflow) return -1;
    return (bw->bitPos > 0) ? (bw->bytePos + 1) : bw->bytePos;
}

// --- Encoder dictionary ------------------------------------------------

struct LZWEncEntry {
    int prefix;   // pre code
    uint8_t ch;   // new char
};

// Simple hash: (prefix<<8 + ch) * 257, then mod
struct LZWHashEntry {
    int prefix;
    uint8_t ch;
    int code;
};

#define LZW_HASH_SIZE 8192 //Simply 8K, slightly larger than 4096 to reduce collision

static void LZWEnc_ResetDict(
    LZWEncEntry* dict,
    LZWHashEntry* hash,
    int* nextCode,
    int* codeBits,
    int* nextLimit)
{    
	// initialize base dictionary 0..255
    for (int i = 0; i < 256; ++i) {
        dict[i].prefix = -1;
        dict[i].ch = (uint8_t)i;
    }

    //clear hash
    for (int i = 0; i < LZW_HASH_SIZE; ++i) {
        hash[i].prefix = -1;
        hash[i].ch = 0;
        hash[i].code = -1;
    }

    *nextCode = TIFF_FIRST_CODE;
    *codeBits = 9;
    *nextLimit = 1 << *codeBits;
}

// Simple hash: (prefix<<8 + ch) * 257, then mod
static inline int LZWEnc_HashKey(int prefix, uint8_t ch)
{
    uint32_t k = ((uint32_t)(prefix & 0xFFFF) << 8) | ch;
    k *= 257u;
    return (int)(k % LZW_HASH_SIZE);
}

//Find code for (prefix,ch), return -1 if not found
static int LZWEnc_Find(
    LZWHashEntry* hash,
    int prefix,
    uint8_t ch)
{
    if (prefix < 0) return -1;

    int h = LZWEnc_HashKey(prefix, ch);
    int start = h;

    while (true) {
        if (hash[h].code == -1) {
			// empty slot, not found
            return -1;
        }
        if (hash[h].prefix == prefix && hash[h].ch == ch)
            return hash[h].code;

        h++;
        if (h == LZW_HASH_SIZE) h = 0;
        if (h == start) return -1; // full
    }
}

// insert (prefix,ch)->code
static void LZWEnc_Insert(
    LZWHashEntry* hash,
    int prefix,
    uint8_t ch,
    int code)
{
    int h = LZWEnc_HashKey(prefix, ch);
    int start = h;

    while (true) {
        if (hash[h].code == -1) {
            hash[h].prefix = prefix;
            hash[h].ch = ch;
            hash[h].code = code;
            return;
        }
        h++;
        if (h == LZW_HASH_SIZE) h = 0;
        if (h == start) {
			// hash full，just clear it and start over (should not happen if hash size is reasonably larger than dict size)
            return;
        }
    }
}

// --- TIFF LZW Encode string ---------------------------------------------

bool Lzw_Perplexity::Encode(
    const uint8_t* in, int inSize,
    uint8_t* out, int outCap,
    int* outSize)
{
    if (!in || !out || !outSize || inSize <= 0 || outCap <= 0)
        return false;

    BitWriter bw;
    BW_Init(&bw, out, outCap);
	//First clear the output buffer to avoid garbage data when bit OR    
    memset(out, 0, outCap);

    LZWEncEntry dict[TIFF_LZW_MAX_DICT];
    LZWHashEntry hash[LZW_HASH_SIZE];
    int nextCode, codeBits, nextLimit;

    LZWEnc_ResetDict(dict, hash, &nextCode, &codeBits, &nextLimit);

	// TIFF : first code must be CLEAR
    BW_WriteBits(&bw, TIFF_CLEAR_CODE, codeBits);

	int prefix = -1; // corrent prefix code, -1 : none, 0..255: single byte, >=258: dict code

    for (int i = 0; i < inSize; ++i) {
        uint8_t c = in[i];

        if (prefix < 0) {
			// first byte, just set prefix
            prefix = c;
            continue;
        }

        int code = LZWEnc_Find(hash, prefix, c);
        if (code != -1) {
			// prefix + c add to dict already, just update
            prefix = code;
        }
        else {
            // output prefix
            BW_WriteBits(&bw, (uint16_t)prefix, codeBits);

            // Add new string into (prefix,c) dir
            if (nextCode < TIFF_LZW_MAX_DICT) {
                dict[nextCode].prefix = prefix;
                dict[nextCode].ch = c;
                LZWEnc_Insert(hash, prefix, c, nextCode);
                nextCode++;

                if (nextCode == nextLimit && codeBits < TIFF_LZW_MAX_BITS) {
                    codeBits++;
                    nextLimit <<= 1;
                }
            }
            else {
                //Dictionary is full. CLEAR and reset.
                BW_WriteBits(&bw, TIFF_CLEAR_CODE, codeBits);
                LZWEnc_ResetDict(dict, hash, &nextCode, &codeBits, &nextLimit);
            }

            prefix = c;
        }

        if (bw.overflow) {
            *outSize = outCap;
            return false;
        }
    }

    // Last prefix
    if (prefix >= 0) {
        BW_WriteBits(&bw, (uint16_t)prefix, codeBits);
    }

    // End Of Information
    BW_WriteBits(&bw, TIFF_EOI_CODE, codeBits);

    int bytesWritten = BW_Finish(&bw);
    if (bytesWritten < 0) {
        *outSize = outCap;
        return false;
    }

    *outSize = bytesWritten;
    return true;
}

void Lzw_Perplexity::PredicatorEncode(
    uint8_t* buf,
    int width,
    int rows,
    int samplesPerPixel,
    int bitsPerSample)
{
    if (bitsPerSample != 8 && bitsPerSample != 16)
        return;

    int bytesPerSample = bitsPerSample / 8;
    int bytesPerPixel = samplesPerPixel * bytesPerSample;
    int rowBytes = width * bytesPerPixel;

    for (int y = 0; y < rows; ++y) {
        uint8_t* row = buf + y * rowBytes;

        if (bitsPerSample == 8) {
            // Horizontal difference in bytes
            for (int x = bytesPerPixel; x < rowBytes; ++x) {
                row[x] = (uint8_t)(row[x] - row[x - bytesPerPixel]);
            }
        }
        else {//16bits, perform subtraction on uint16_t            
            uint16_t* row16 = (uint16_t*)row;
            int pixelsInRow = width * samplesPerPixel;

            for (int i = pixelsInRow - 1; i > 0; --i) {
                row16[i] = (uint16_t)(row16[i] - row16[i - 1]);
            }
        }
    }
}