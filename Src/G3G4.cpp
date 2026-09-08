/////////////////////////////////////////////////////////////////////x
// G3G4.cpp: CCITT Group 3 (T.4) and Group 4 (T.6) 1-bit codec for Tiff_STL3.
//
// Decode: G3 1D/2D (EOL + optional fill bits + 1D/2D tag bit), G4 MMR.
// Encode: G3 1D MH per row with EOL, G4 MMR with trailing EOFB.
// Run length code tables come from LibTiff/t4.h (ITU T.4 tables).
//////////////////////////////////////////////////////////////////////x
#include <iostream>
#include "stdafx.h"
#include "../Include/Tiff_STL4.h"

using namespace AV_Tiff_STL4;

#include <vector>
#include <cstring>
using namespace std;

#define G3CODES
#include "../../LibTiff/t4.h"

namespace AV_Tiff_STL4 {

//****************************************************************************
// Containers.
// FIXED_VECTOR : bounded run arrays on FixedVector, byte buffers on a
//                minimal growable heap buffer (new[]/delete[], no STL).
// otherwise    : std::vector everywhere.
//****************************************************************************
#ifdef FIXED_VECTOR
//Max runs per row is (W+1)/2 + guards. kMaxFaxRuns supports widths up to
//~8700 pixels (3 arrays x 4352 x 4B ~= 52KB stack at peak).
static const size_t kMaxFaxRuns = 4352;
template<typename T>
using FaxRunArray = FixedVector<T, kMaxFaxRuns>;

class FaxBuffer
{
public:
	FaxBuffer() :_p(nullptr), _n(0), _cap(0) {}
	~FaxBuffer() { delete[]_p; }

	bool reserve(size_t need)
	{
		if (need <= _cap)
			return true;
		size_t nc = _cap ? _cap : need;
		while (nc < need)
			nc *= 2;
		BYTE* q = new BYTE[nc];
		if (!q)
			return false;
		for (size_t i = 0; i < _n; i++)
			q[i] = _p[i];
		delete[]_p;
		_p = q;
		_cap = nc;
		return true;
	}

	bool push_back(BYTE b)
	{
		if (_n >= _cap && !reserve(_n + 1))
			return false;
		_p[_n++] = b;
		return true;
	}

	BYTE& operator[](size_t i) { return _p[i]; }
	const BYTE& operator[](size_t i) const { return _p[i]; }
	BYTE& back() { return _p[_n - 1]; }
	BYTE* data() { return _p; }
	const BYTE* data() const { return _p; }
	size_t size() const { return _n; }
	bool empty() const { return _n == 0; }

private:
	BYTE* _p;
	size_t _n;
	size_t _cap;
};
#else
template<typename T>
using FaxRunArray = std::vector<T>;
using FaxBuffer = std::vector<BYTE>;
#endif

const int FaxEOL = 0x001;//12 bits 000000000001

//****************************************************************************
// Bit I/O (MSB first)
//****************************************************************************
class FaxBits
{
public:
	const unsigned char* buf;
	int size;
	int bytePos;
	int bitPos;

	FaxBits(const unsigned char* b, int s) :buf(b), size(s), bytePos(0), bitPos(0) {}

	int readBit()
	{
		if (bytePos >= size)
			return -1;
		int bit = (buf[bytePos] >> (7 - bitPos)) & 1;
		if (++bitPos == 8) { bitPos = 0; bytePos++; }
		return bit;
	}
};

class FaxWriter
{
public:
	FaxBuffer& out;
	int bitPos;

	FaxWriter(FaxBuffer& o) :out(o), bitPos(0)
	{
		out.push_back(0);
	}

	void writeBits(int code, int n)
	{
		for (int i = n - 1; i >= 0; i--)
		{
			int bit = (code >> i) & 1;
			out.back() |= (unsigned char)(bit << (7 - bitPos));
			if (++bitPos == 8) { bitPos = 0; out.push_back(0); }
		}
	}

	int size() { return (int)out.size(); }
};

//****************************************************************************
// Run length tables (built once from t4.h). color 0 = white, 1 = black.
//****************************************************************************
struct FaxRunTables
{
	int termBits[2][64];
	int termCode[2][64];
	int mkBits[2][27];      //makeups 64,128,...,1728 : index run/64 - 1
	int mkCode[2][27];
	int cmBits[13];         //common makeups 1792..2560
	int cmCode[13];
	int cmRun[13];
	const tableentry* tab[2];
	int tabLen[2];
};

static FaxRunTables g_fax;
static bool g_faxInit = false;

static void FaxInit()
{
	if (g_faxInit) return;
	for (int c = 0; c < 64; c++)
	{
		g_fax.termBits[0][c] = TIFFFaxWhiteCodes[c].length;
		g_fax.termCode[0][c] = TIFFFaxWhiteCodes[c].code;
		g_fax.termBits[1][c] = TIFFFaxBlackCodes[c].length;
		g_fax.termCode[1][c] = TIFFFaxBlackCodes[c].code;
	}
	for (int m = 0; m < 27; m++)
	{
		g_fax.mkBits[0][m] = TIFFFaxWhiteCodes[64 + m].length;
		g_fax.mkCode[0][m] = TIFFFaxWhiteCodes[64 + m].code;
		g_fax.mkBits[1][m] = TIFFFaxBlackCodes[64 + m].length;
		g_fax.mkCode[1][m] = TIFFFaxBlackCodes[64 + m].code;
	}
	for (int m = 0; m < 13; m++)
	{
		g_fax.cmBits[m] = TIFFFaxWhiteCodes[91 + m].length;
		g_fax.cmCode[m] = TIFFFaxWhiteCodes[91 + m].code;
		g_fax.cmRun[m] = TIFFFaxWhiteCodes[91 + m].runlen;//1792 + 64*m
	}
	g_fax.tab[0] = TIFFFaxWhiteCodes;
	g_fax.tab[1] = TIFFFaxBlackCodes;
	g_fax.tabLen[0] = g_fax.tabLen[1] = 104;
	g_faxInit = true;
}

//Decode one white/black run. Returns run length, -1 EOF, -2 invalid.
static int FaxReadRun(FaxBits& br, int color)
{
	int code = 0, len = 0;
	int run = 0;
	for (;;)
	{
		int b = br.readBit();
		if (b < 0) return -1;
		code = (code << 1) | b;
		len++;
		if (len > 13) return -2;
		int hit = -1;
		for (int i = 0; i < g_fax.tabLen[color]; i++)
		{
			if (g_fax.tab[color][i].length == len && g_fax.tab[color][i].code == code)
			{
				hit = i;
				break;
			}
		}
		if (hit < 0) continue;
		int runlen = g_fax.tab[color][hit].runlen;
		if (runlen < 0) return -2;//EOL or invalid code inside a run.
		run += runlen;
		if (runlen < 64)
			return run;//Terminating code ends the run.
		code = 0;
		len = 0;//Makeup code: keep reading.
	}
}

//****************************************************************************
// 2D mode codes
//****************************************************************************
enum FaxMode {
	FaxMode_V0 = 0, FaxMode_VR1, FaxMode_VR2, FaxMode_VR3,
	FaxMode_VL1, FaxMode_VL2, FaxMode_VL3,
	FaxMode_Horizontal, FaxMode_Pass, FaxMode_EOF, FaxMode_Invalid
};

static FaxMode FaxReadMode(FaxBits& br)
{
	int b1 = br.readBit();
	if (b1 < 0) return FaxMode_EOF;
	if (b1 == 1) return FaxMode_V0;//1
	int b2 = br.readBit();
	if (b2 < 0) return FaxMode_EOF;
	if (b2 == 1)
	{//01x : VR1 (011) or VL1 (010)
		int b3 = br.readBit();
		if (b3 < 0) return FaxMode_EOF;
		return (b3 == 1) ? FaxMode_VR1 : FaxMode_VL1;
	}
	int b3 = br.readBit();
	if (b3 < 0) return FaxMode_EOF;
	if (b3 == 1) return FaxMode_Horizontal;//001
	int b4 = br.readBit();
	if (b4 < 0) return FaxMode_EOF;
	if (b4 == 1) return FaxMode_Pass;//0001
	int b5 = br.readBit();
	if (b5 < 0) return FaxMode_EOF;
	if (b5 == 1)
	{//00001x : VR2 (000011) or VL2 (000010)
		int b6 = br.readBit();
		if (b6 < 0) return FaxMode_EOF;
		return (b6 == 1) ? FaxMode_VR2 : FaxMode_VL2;
	}
	//000001x : VR3 (0000011) or VL3 (0000010), 7 bits.
	int b6 = br.readBit();
	if (b6 < 0) return FaxMode_EOF;
	int b7 = br.readBit();
	if (b7 < 0) return FaxMode_EOF;
	return (b7 == 1) ? FaxMode_VR3 : FaxMode_VL3;
}

//****************************************************************************
// Line helpers. Runs are alternating white/black starting with white.
//****************************************************************************
//Changing element boundaries of a run list (they alternate color, the last
//bound is >= W).
static void FaxRowBounds(const FaxRunArray<int>& runs, int W, FaxRunArray<int>& bounds)
{
	bounds.clear();
	int pos = 0;
	for (size_t i = 0; i < runs.size(); i++)
	{
		int len = runs[i];
		pos += len;
		if (len > 0)
			bounds.push_back(pos);
		else if (i == 0)
			bounds.push_back(0);//Line starts with black.
	}
	if (bounds.empty() || bounds[bounds.size() - 1] < W)
		bounds.push_back(W);
}

//First changing element boundary > a0. Bounds alternate color, so the first
//one always starts the opposite color of the run containing a0. Falls back
//to bounds back (>= W) past the end of the line.
static int FaxFindBound(const FaxRunArray<int>& bounds, int a0)
{
	for (size_t i = 0; i < bounds.size(); i++)
	{
		if (bounds[i] > a0)
			return bounds[i];
	}
	return bounds[bounds.size() - 1];
}

//Paint black pixels (from, to] (from may be -1), clamped to [0, W).
static void FaxPaintBlack(unsigned char* row, int from, int to, int W)
{
	int start = from + 1;
	if (start < 0) start = 0;
	for (int p = start; p <= to && p < W; p++)
		row[p >> 3] |= (unsigned char)(0x80 >> (p & 7));
}

//****************************************************************************
// 2D line decode (used by G4 and G3-2D). Pixels are written into `row`
// directly, so Pass mode needs no bookkeeping.
//****************************************************************************
static bool FaxDecode2DLine(FaxBits& br, const FaxRunArray<int>& refRuns, int W, unsigned char* row)
{
	FaxRunArray<int> bounds;
	FaxRowBounds(refRuns, W, bounds);

	int a0 = -1;
	int color = 0;//white
	int guard = 0;
	for (;;)
	{
		if (++guard > W + 64)
			return false;//Runaway protection.

		int b1 = FaxFindBound(bounds, a0);
		int b2 = FaxFindBound(bounds, b1);

		FaxMode m = FaxReadMode(br);
		if (m == FaxMode_EOF || m == FaxMode_Invalid) return false;

		if (m == FaxMode_Pass)
		{
			a0 = b2;//No changing element on the coding line before b2.
		}
		else if (m == FaxMode_Horizontal)
		{
			int r1 = FaxReadRun(br, color);
			if (r1 < 0) return false;
			int r2 = FaxReadRun(br, color ^ 1);
			if (r2 < 0) return false;
			if (color == 1)
				FaxPaintBlack(row, a0, a0 + r1, W);
			a0 += r1;
			if ((color ^ 1) == 1)
				FaxPaintBlack(row, a0, a0 + r2, W);
			a0 += r2;
		}
		else
		{
			int delta = 0;
			switch (m)
			{
			case FaxMode_V0: delta = 0; break;
			case FaxMode_VR1: delta = 1; break;
			case FaxMode_VR2: delta = 2; break;
			case FaxMode_VR3: delta = 3; break;
			case FaxMode_VL1: delta = -1; break;
			case FaxMode_VL2: delta = -2; break;
			case FaxMode_VL3: delta = -3; break;
			default: return false;
			}
			int a1 = b1 + delta;
			if (a1 < a0) a1 = a0;
			if (color == 1)
				FaxPaintBlack(row, a0, a1, W);
			a0 = a1;
			color ^= 1;
		}

		if (a0 >= W) break;
	}
	return true;
}

//1D MH line.
static bool FaxDecode1DLine(FaxBits& br, int W, FaxRunArray<int>& runs)
{
	runs.clear();
	int pos = 0;
	int color = 0;
	while (pos < W)
	{
		int r = FaxReadRun(br, color);
		if (r < 0) return false;
		runs.push_back(r);
		pos += r;
		if (pos > W) return false;
		color ^= 1;
	}
	return true;
}

static void FaxRunsToBits(const FaxRunArray<int>& runs, int W, unsigned char* row)
{
	int pos = -1;
	int color = 0;//white
	for (size_t i = 0; i < runs.size() && pos < W; i++)
	{
		int len = runs[i];
		if (color == 1)
			FaxPaintBlack(row, pos, pos + len, W);
		pos += len;
		color ^= 1;
	}
}

static void FaxRowRuns(const unsigned char* row, int W, FaxRunArray<int>& runs)
{
	runs.clear();
	int color = 0;
	int run = 0;
	for (int x = 0; x < W; x++)
	{
		int bit = (row[x >> 3] >> (7 - (x & 7))) & 1;
		if (bit == color) run++;
		else { runs.push_back(run); run = 1; color ^= 1; }
	}
	runs.push_back(run);
}

//****************************************************************************
// Public decode. out holds `rows` rows of `rowBytes` bytes (1 bit per pixel).
//****************************************************************************
bool G3G4_Decode(const unsigned char* in, int inBytes,
	unsigned char* out, int rowBytes, int rows,
	bool g4, unsigned options, int* rowsDone)
{
	FaxInit();
	memset(out, 0, (size_t)rowBytes * rows);
	FaxBits br(in, inBytes);

	int W = rowBytes * 8;

	//Uncompressed mode is not supported.
	if (g4 && (options & 1)) return false;
	if (!g4 && (options & 2)) return false;

	bool twoDim = g4 ? true : ((options & 1) != 0);

	FaxRunArray<int> refRuns;
	FaxRunArray<int> runs;
	refRuns.push_back(W);//Imaginary all white reference line.

	int r = 0;
	for (r = 0; r < rows; r++)
	{
		unsigned char* row = out + (size_t)r * rowBytes;
		if (g4)
		{
			if (!FaxDecode2DLine(br, refRuns, W, row))
				break;
		}
		else
		{
			//Find EOL: skip bits until 000000000001 (handles fill bits).
			int zeros = 0;
			bool found = false;
			for (;;)
			{
				int b = br.readBit();
				if (b < 0) break;
				if (b == 0) { zeros++; continue; }
				if (zeros >= 10) { found = true; break; }
				zeros = 0;
			}
			if (!found) break;

			if (twoDim)
			{
				int tag = br.readBit();
				if (tag < 0) break;
				if (tag == 1)
				{
					if (!FaxDecode1DLine(br, W, runs)) break;
				}
				else
				{
					if (!FaxDecode2DLine(br, refRuns, W, row)) break;
				}
			}
			else
			{
				if (!FaxDecode1DLine(br, W, runs)) break;
			}

			FaxRunsToBits(runs, W, row);
		}

		FaxRowRuns(row, W, refRuns);
	}
	if (rowsDone) *rowsDone = r;
	return r == rows;
}

//****************************************************************************
// Encode
//****************************************************************************
static void FaxEmitRun(FaxWriter& w, int run, int color)
{
	while (run > 2560)
	{
		w.writeBits(g_fax.cmCode[12], g_fax.cmBits[12]);//2560 makeup
		run -= 2560;
	}
	if (run >= 1792)
	{
		int idx = (run - 1792) / 64;
		w.writeBits(g_fax.cmCode[idx], g_fax.cmBits[idx]);
		run -= g_fax.cmRun[idx];
	}
	if (run >= 64)
	{
		int idx = run / 64 - 1;
		w.writeBits(g_fax.mkCode[color][idx], g_fax.mkBits[color][idx]);
		run -= (idx + 1) * 64;
	}
	w.writeBits(g_fax.termCode[color][run], g_fax.termBits[color][run]);
}

static void FaxEmit1DRow(FaxWriter& w, const unsigned char* row, int W)
{
	int color = 0;
	int run = 0;
	for (int x = 0; x < W; x++)
	{
		int bit = (row[x >> 3] >> (7 - (x & 7))) & 1;
		if (bit == color)
			run++;
		else
		{
			FaxEmitRun(w, run, color);
			color ^= 1;
			run = 1;
		}
	}
	FaxEmitRun(w, run, color);
}


static void FaxAssignRuns(FaxRunArray<int>& dst, const FaxRunArray<int>& src)
{
#ifdef FIXED_VECTOR
	dst.clear();
	for (size_t i = 0; i < src.size(); i++)
		dst.push_back(src[i]);
#else
	dst = src;
#endif
}

//Encode one G4 (MMR) row. Vertical mode when possible, horizontal otherwise.
static void FaxEncode2DRow(FaxWriter& w, const unsigned char* row, int W,
	const FaxRunArray<int>& refRuns, FaxRunArray<int>& runs)
{
	FaxRowRuns(row, W, runs);

	FaxRunArray<int> bounds;
	FaxRowBounds(refRuns, W, bounds);

	int a0 = -1;
	int color = 0;
	size_t ri = 0;
	while (a0 < W)
	{
		if (ri >= runs.size())
		{
			w.writeBits(0x1, 1);//V0 to the end of the reference line.
			a0 = W;
			break;
		}
		int a1 = a0 + runs[ri];

		int b1 = FaxFindBound(bounds, a0);
		int delta = a1 - b1;
		if (delta >= -3 && delta <= 3)
		{
			switch (delta)
			{
			case 0: w.writeBits(0x1, 1); break;
			case 1: w.writeBits(0x3, 3); break;
			case 2: w.writeBits(0x3, 6); break;
			case 3: w.writeBits(0x3, 7); break;
			case -1: w.writeBits(0x2, 3); break;
			case -2: w.writeBits(0x2, 6); break;
			case -3: w.writeBits(0x2, 7); break;
			}
			a0 = a1;
			color ^= 1;
			ri++;
		}
		else
		{
			//Horizontal mode: two runs.
			w.writeBits(0x1, 3);
			FaxEmitRun(w, runs[ri], color);
			int r2 = (ri + 1 < runs.size()) ? runs[ri + 1] : W - a1;
			FaxEmitRun(w, r2, color ^ 1);
			a0 = a1 + r2;
			ri += 2;
		}
	}
}

//****************************************************************************
// Public encode. in holds `rows` rows of `rowBytes` bytes (1 bit per pixel).
//****************************************************************************
int G3G4_Encode(const unsigned char* in, int rowBytes, int rows,
	FaxBuffer& out, bool g4)
{
	FaxInit();
	FaxWriter w(out);
	int W = rowBytes * 8;

	FaxRunArray<int> refRuns;
	FaxRunArray<int> runs;
	refRuns.push_back(W);

	for (int r = 0; r < rows; r++)
	{
		const unsigned char* row = in + (size_t)r * rowBytes;
		if (!g4)
		{
			w.writeBits(FaxEOL, 12);
			FaxEmit1DRow(w, row, W);
			continue;
		}

		FaxEncode2DRow(w, row, W, refRuns, runs);
		FaxAssignRuns(refRuns, runs);
	}

	if (g4)
	{
		w.writeBits(FaxEOL, 12);//EOFB
		w.writeBits(FaxEOL, 12);
	}

	return w.size();
}


//****************************************************************************
// Tiff integration: Compression tag 3 = G3 (T.4), 4 = G4 (T.6). 1 bit/pixel.
//****************************************************************************
Tiff_Err Tiff::G3G4_Compress(int comp)
{
	int Width = GetTagValue(ImageWidth);
	int Length = GetTagValue(ImageLength);
	int Samples = GetTagValue(SamplesPerPixel);
	int bitsPerSample = GetTagValue(BitsPerSample);
	int BytesPerLine = (Width * Samples * bitsPerSample + 7) / 8;

	//Fax coding is defined for 1 bit per pixel, single sample.
	if (bitsPerSample != 1 || Samples != 1)
		return UnSupportCompressData;

	TiffTagPtr TagStripOffsets = GetTag(StripOffsets);
	LPBYTE lpImageBuf = TagStripOffsets->lpData;

	FaxBuffer dst;
	dst.reserve((size_t)BytesPerLine * Length / 4 + 1024);
	G3G4_Encode(lpImageBuf, BytesPerLine, Length, dst, comp == 4);

	LPBYTE lpFaxBuf = new BYTE[dst.size()];
	memcpy(lpFaxBuf, dst.data(), dst.size());

	StripOffsetsTag* pTagStripOffsets = dynamic_cast<StripOffsetsTag*>(GetPtr(TagStripOffsets));
	pTagStripOffsets->SetLzwData(lpFaxBuf);

	//Single strip layout.
	TagStripOffsets->n = 1;
	TagStripOffsets->lpData = new BYTE[4];

	TiffTagPtr TagStripByteCounts = GetTag(StripByteCounts);
	TagStripByteCounts->n = 1;
	TagStripByteCounts->type = Long;
	TagStripByteCounts->lpData = new BYTE[4];
	TagStripByteCounts->value = (DWORD)dst.size();

	SetTagValue(RowsPerStrip, Length);

	//T4Options (292) / T6Options (293): uncompressed mode off, no fill bits.
	Tiff::SetTag(TagListItem(new TiffTag((comp == 3) ? T4Options : T6Options, Short, 1, 0)));
	RemoveTag(Predicator);//Predictor is meaningless for fax coding.

	//The image buffer is replaced by the fax data, release it here.
	//CTiff::SaveFile() clears its cached m_lpImageBuf pointer afterwards.
	delete[]lpImageBuf;
	SetTagValue(Compression, comp);
	return Tiff_OK;
}

Tiff_Err Tiff::SaveTiff_G3G4(IO_INTERFACE* IO, int comp)
{
	Tiff_Err ret = G3G4_Compress(comp);
	if (ret != Tiff_OK)
		return ret;
	std::vector<BYTE> img;//Module-wide file image type (BuildFileImage API).
	ret = BuildFileImage(img, comp);
	if (ret != Tiff_OK)
		return ret;
	IO_Seek(0, SEEK_SET);
	IO_Write(img.data(), 1, img.size());
	return ret;
}


Tiff_Err Tiff::ReadG3G4Strips(IO_INTERFACE* IO, bool SingleStrip, int comp)
{
	TiffTagPtr Tag = GetTag(RowsPerStrip);
	if (Tag->n != 1) //Only support single value
		return UnSupportCompressData;

	int Width = GetTagValue(ImageWidth);
	int Length = GetTagValue(ImageLength);
	int bitsPerSample = GetTagValue(BitsPerSample);
	int samplesPerPixel = GetTagValue(SamplesPerPixel);
	int BytesPerLine = (Width * samplesPerPixel * bitsPerSample + 7) / 8;
	int rowsPerStrip = GetTagValue(RowsPerStrip);

	TiffTagPtr TagStripOffsets = GetTag(StripOffsets);
	TiffTagPtr TagStripByteCounts = GetTag(StripByteCounts);

	LPDWORD lpStripOffset = nullptr;
	LPDWORD lpStripByteCounts = nullptr;
	DWORD strip = 1;
	DWORD inlineOffset = 0, inlineCount = 0;
	if (SingleStrip)
	{
		inlineOffset = GetTagValue(StripOffsets);
		inlineCount = GetTagValue(StripByteCounts);
	}
	else
	{
		if (TagStripOffsets->lpData == nullptr || TagStripByteCounts->lpData == nullptr)
			return UnSupportCompressData;//Array form required.
		strip = TagStripOffsets->n;
		lpStripOffset = (LPDWORD)TagStripOffsets->lpData;
		lpStripByteCounts = (LPDWORD)TagStripByteCounts->lpData;
	}

	LPBYTE lpStripeBuf = new BYTE[BytesPerLine * rowsPerStrip];
	int MaxStripBufSize = BytesPerLine * rowsPerStrip;
	LPBYTE lpImageBuf = new BYTE[BytesPerLine * Length];
	memset(lpImageBuf, 0, BytesPerLine * Length);
	LPBYTE lpImage = lpImageBuf;

	unsigned options = (unsigned)GetTagValue((comp == 4) ? T6Options : T4Options);

	int LinesRemain = Length;
	for (DWORD i = 0; i < strip; i++)
	{
		int offset = SingleStrip ? (int)inlineOffset : (int)*(lpStripOffset++);
		int Bufsize = SingleStrip ? (int)inlineCount : (int)*(lpStripByteCounts++);
		IO_Seek(offset, SEEK_SET);
		if (Bufsize > MaxStripBufSize)
		{
			//Fax data can be larger than the raw rows (noise expands).
			MaxStripBufSize = (int)(Bufsize * 1.5);
			delete[]lpStripeBuf;
			lpStripeBuf = new BYTE[MaxStripBufSize];
		}
		IO_Read(lpStripeBuf, 1, Bufsize);

		int Rows = (LinesRemain < rowsPerStrip) ? LinesRemain : rowsPerStrip;
		if (Rows <= 0) break;

		int done = 0;
		if (!G3G4_Decode(lpStripeBuf, Bufsize, lpImage, BytesPerLine, Rows, comp == 4, options, &done))
			cout << " *** Warning: G3/G4 Decode Error. *** " << endl;

		lpImage += BytesPerLine * Rows;
		LinesRemain -= Rows;
	}

	delete[]lpStripeBuf;

	//Reset StripOffsets) and StripByteCounts)
	ResetStripTags(lpImageBuf, BytesPerLine * Length, true);
	return Tiff_OK;
}

};//namespace AV_Tiff_STL4

