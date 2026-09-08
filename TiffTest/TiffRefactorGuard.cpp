//////////////////////////////////////////////////////////////////////x
// TiffRefactorGuard.cpp
//
// Behavior lock for Tiff_STL3 before/after refactoring.
// Covers: 8/16-bit round-trip, LZW round-trip, RemoveTag ownership,
// multi-strip read, Planar=2 + Pack, 1-bit CMYKcm, invalid files.
//
// Exit code = number of failed cases.
//////////////////////////////////////////////////////////////////////x
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <vector>
#include <string>
using namespace std;

#include "Tiff_STL4.h"
#include "Tiff_STL3.h"//Compatibility shim must still work.
namespace AV_Tiff_STL3 {
	typedef AV_Tiff_STL3::CTiff LegacyCTiff;//Shim type check.
}
using namespace AV_Tiff_STL4;

static int g_Fails = 0;
#define CHECK(cond, name) do { \
	if (cond) { fprintf(stderr, "[PASS] %s\n", name); } \
	else { fprintf(stderr, "[FAIL] %s (line %d)\n", name, __LINE__); g_Fails++; } \
} while (0)

static const char* TempDir = "/tmp";

//**************************************************************************
// Minimal little-endian TIFF builder (uncompressed)
//**************************************************************************
struct TiffBuilder
{
	struct Entry {
		unsigned short tag;
		unsigned short type;   // 3=SHORT, 2=ASCII, 4=LONG, 5=RATIONAL
		unsigned long  count;
		unsigned long  inlineValue = 0;  // used when type*size*count <= 4
		bool           fixupImageBase = false; //LONG blob values are image-relative
		vector<unsigned char> blob;  // used otherwise
	};
	vector<Entry> entries;
	vector<unsigned char> image;

	void set(unsigned short tag, unsigned short type, unsigned long count, unsigned long v)
	{
		Entry e; e.tag = tag; e.type = type; e.count = count; e.inlineValue = v;
		entries.push_back(e);
	}
	void setBlob(unsigned short tag, unsigned short type, unsigned long count, const void* data, size_t bytes)
	{
		Entry e; e.tag = tag; e.type = type; e.count = count;
		const unsigned char* p = (const unsigned char*)data;
		e.blob.assign(p, p + bytes);
		entries.push_back(e);
	}
	void markFixup(unsigned short tag)
	{
		for (size_t i = 0; i < entries.size(); i++)
			if (entries[i].tag == tag) entries[i].fixupImageBase = true;
	}

	static int typeSize(unsigned short t) { return (t == 3) ? 2 : (t == 4) ? 4 : (t == 5) ? 8 : 1; }

	bool write(const char* path, unsigned short tagCountOverride = 0)
	{
		// sort by tag id (TIFF requires ascending)
		for (size_t i = 0; i + 1 < entries.size(); i++)
			for (size_t j = i + 1; j < entries.size(); j++)
				if (entries[j].tag < entries[i].tag)
				{ Entry t = entries[i]; entries[i] = entries[j]; entries[j] = t; }

		vector<unsigned char> f;
		auto w16 = [&f](unsigned short v) { f.push_back(v & 0xFF); f.push_back((v >> 8) & 0xFF); };
		auto w32 = [&f](unsigned long v) { for (int i = 0; i < 4; i++) f.push_back((unsigned char)((v >> (8 * i)) & 0xFF)); };

		unsigned long ifdOff = 8;
		unsigned long ifdSize = 2 + 12 * entries.size() + 4;
		unsigned long extraOff = ifdOff + ifdSize;

		// compute extra area
		unsigned long extraSize = 0;
		for (size_t i = 0; i < entries.size(); i++)
		{
			unsigned long bytes = typeSize(entries[i].type) * entries[i].count;
			if (bytes > 4)
			{
				entries[i].inlineValue = extraOff + extraSize;
				extraSize += bytes + (bytes & 1);
			}
		}
		unsigned long imageOff = extraOff + extraSize;

		//rewrite image-relative LONG values (blob form or inline form) to absolute file offsets
		for (size_t i = 0; i < entries.size(); i++)
		{
			if (!entries[i].fixupImageBase || entries[i].type != 4) continue;
			unsigned long bytes = typeSize(entries[i].type) * entries[i].count;
			if (bytes > 4)
			{
				unsigned long cnt = entries[i].count;
				for (unsigned long k = 0; k < cnt && (k + 1) * 4 <= entries[i].blob.size(); k++)
				{
					unsigned long v = 0;
					for (int b = 0; b < 4; b++) v |= (unsigned long)entries[i].blob[k * 4 + b] << (8 * b);
					v += imageOff;
					for (int b = 0; b < 4; b++) entries[i].blob[k * 4 + b] = (unsigned char)((v >> (8 * b)) & 0xFF);
				}
			}
			else
			{
				entries[i].inlineValue += imageOff;
			}
		}

		// header
		f.push_back('I'); f.push_back('I'); w16(42); w32(ifdOff);
		// IFD
		unsigned short n = (tagCountOverride != 0) ? tagCountOverride : (unsigned short)entries.size();
		w16(n);
		for (size_t i = 0; i < entries.size(); i++)
		{
			w16(entries[i].tag); w16(entries[i].type); w32(entries[i].count);
			unsigned long bytes = typeSize(entries[i].type) * entries[i].count;
			if (bytes <= 4)
				w32(entries[i].inlineValue);
			else
				w32(entries[i].inlineValue); // offset into extra area
		}
		w32(0);
		// extra area
		for (size_t i = 0; i < entries.size(); i++)
		{
			unsigned long bytes = typeSize(entries[i].type) * entries[i].count;
			if (bytes > 4)
			{
				f.insert(f.end(), entries[i].blob.begin(), entries[i].blob.begin() + bytes);
				if (bytes & 1) f.push_back(0);
			}
		}
		// image data (pad up to the announced imageOff, then append)
		while ((unsigned long)f.size() < imageOff)
			f.push_back(0);
		f.insert(f.end(), image.begin(), image.end());

		FILE* fp = fopen(path, "wb");
		if (!fp) return false;
		fwrite(&f[0], 1, f.size(), fp);
		fclose(fp);
		return true;
	}
};

static void setU16(unsigned short v, unsigned char* p) { p[0] = v & 0xFF; p[1] = (v >> 8) & 0xFF; }

//**************************************************************************
// 1) 8-bit RGB round-trip
//**************************************************************************
static void TestRoundTrip8()
{
	const int W = 17, L = 9, SPP = 3;
	string path = string(TempDir) + "/rg_t1.tif";

	CTiff out;
	out.CreateNew(W, L, 72, SPP, 8, 1);
	vector<unsigned char> ref(W * L * SPP);
	for (int i = 0; i < W * L * SPP; i++) ref[i] = (unsigned char)((i * 31 + 7) & 0xFF);
	for (int y = 0; y < L; y++)
		out.PutRow(&ref[y * W * SPP], y);
	CHECK(out.SaveFile(path.c_str()) == Tiff_OK, "t1 save");

	CTiff in;
	CHECK(in.ReadFile(path.c_str()) == Tiff_OK, "t1 read");
	CHECK(in.GetTagValue(ImageWidth) == W && in.GetTagValue(ImageLength) == L, "t1 tags");
	int bytes = W * L * SPP;
	CHECK(memcmp(in.GetImageBuf(), &ref[0], bytes) == 0, "t1 pixels");
}

//**************************************************************************
// 2) 16-bit RGB round-trip
//**************************************************************************
static void TestRoundTrip16()
{
	const int W = 11, L = 5, SPP = 3;
	string path = string(TempDir) + "/rg_t2.tif";

	CTiff out;
	out.CreateNew(W, L, 72, SPP, 16, 1);
	vector<unsigned short> ref(W * L * SPP);
	for (int i = 0; i < W * L * SPP; i++) ref[i] = (unsigned short)((i * 1237 + 99) & 0xFFFF);
	for (int y = 0; y < L; y++)
		out.PutRow((LPWORD)&ref[y * W * SPP], y);
	CHECK(out.SaveFile(path.c_str()) == Tiff_OK, "t2 save");

	CTiff in;
	CHECK(in.ReadFile(path.c_str()) == Tiff_OK, "t2 read");
	CHECK(in.GetTagValue(BitsPerSample) == 16, "t2 bps16");
	int bytes = W * L * SPP * 2;
	CHECK(memcmp(in.GetImageBuf(), &ref[0], bytes) == 0, "t2 pixels");
}

//**************************************************************************
// 3) LZW round-trip (encode path + Perplexity decode path)
//**************************************************************************
static void TestLzwOne(int W, int L, int SPP, const char* tag)
{
	string path = string(TempDir) + "/rg_t3.tif";

	CTiff out;
	out.CreateNew(W, L, 72, SPP, 8, 1);
	vector<unsigned char> ref(W * L * SPP);
	for (int i = 0; i < W * L * SPP; i++) ref[i] = (unsigned char)((i * 13 + (i >> 5)) & 0xFF);
	for (int y = 0; y < L; y++)
		out.PutRow(&ref[y * W * SPP], y);
	CHECK(out.SaveFile(path.c_str(), 1) == Tiff_OK, tag);

	CTiff in;
	CHECK(in.ReadFile(path.c_str()) == Tiff_OK, tag);
	int bytes = W * L * SPP;
	CHECK(memcmp(in.GetImageBuf(), &ref[0], bytes) == 0, tag);
}

static void TestLzw()
{
	//23x7 and 10x3 used to segfault (LZW_Compress strip loop overread).
	TestLzwOne(64, 16, 3, "t3a 64x16");
	TestLzwOne(23, 7, 3, "t3b 23x7");
	TestLzwOne(10, 3, 3, "t3c 10x3");
	TestLzwOne(33, 5, 1, "t3d 33x5 spp1");
	TestLzwOne(16, 16, 3, "t3e 16x16");
}

//**************************************************************************
// 4) RemoveTag ownership (A1 regression)
//**************************************************************************
static void TestRemoveTag()
{
	string path = string(TempDir) + "/rg_t4.tif";

	TiffBuilder b;
	b.set((unsigned short)ImageWidth, 4, 1, 4);
	b.set((unsigned short)ImageLength, 4, 1, 2);
	unsigned char bps3[6]; for (int i = 0; i < 3; i++) setU16(8, bps3 + i * 2);
	b.setBlob((unsigned short)BitsPerSample, 3, 3, bps3, 6);
	b.set((unsigned short)Compression, 3, 1, 1);
	b.set((unsigned short)PhotometricInterpretation, 3, 1, 2);
	unsigned int offsets[1] = { 0 };
	b.setBlob((unsigned short)StripOffsets, 4, 1, offsets, 4);
	b.markFixup((unsigned short)StripOffsets);
	b.set((unsigned short)SamplesPerPixel, 3, 1, 3);
	b.set((unsigned short)RowsPerStrip, 4, 1, 2);
	unsigned int counts[1] = { 24 };
	b.setBlob((unsigned short)StripByteCounts, 4, 1, counts, 4);
	b.set((unsigned short)PlanarConfiguration, 3, 1, 1);
	const char* desc = "Guardrail";
	b.setBlob(270, 2, 9, desc, 9);                 // ImageDescription, middle tag
	unsigned char rat[8] = { 200, 0, 0, 0, 10, 0, 0, 0 };
	b.setBlob(282, 5, 1, rat, 8);                  // XResolution
	const char* soft = "RefGuard";
	b.setBlob(305, 2, 9, soft, 9);                 // Software
	b.image.assign(24, 0x5A);
	CHECK(b.write(path.c_str()), "t4 craft");

	Tiff t;
	CHECK(t.ReadFile(path.c_str()) == Tiff_OK, "t4 read");

	// snapshot all tag values (the crafted file has exactly these tags)
	const unsigned short AllSigs[] = { 256, 257, 258, 259, 262, 270, 273, 277, 278, 279, 282, 284, 305 };
	const int NSIG = (int)(sizeof(AllSigs) / sizeof(AllSigs[0]));
	vector<unsigned long> vals(NSIG);
	for (int i = 0; i < NSIG; i++)
	{
		TiffTagPtr tag = t.GetTag((TiffTagSignature)AllSigs[i]);
		if (tag == nullptr) { fprintf(stderr, "[FAIL] t4 snapshot missing tag %d\n", AllSigs[i]); g_Fails++; return; }
		vals[i] = tag->GetValue();
	}

	// remove middle tag(s), then verify every remaining tag still holds its value
	CHECK(t.RemoveTag((TiffTagSignature)305) == Tiff_OK, "t4 remove 305");
	CHECK(t.RemoveTag((TiffTagSignature)270) == Tiff_OK, "t4 remove 270");
	bool ok = true;
	for (int i = 0; i < NSIG; i++)
	{
		if (AllSigs[i] == 305 || AllSigs[i] == 270) continue;
		TiffTagPtr tag = t.GetTag((TiffTagSignature)AllSigs[i]);
		if (tag == nullptr || tag->GetValue() != vals[i])
			ok = false;
	}
	CHECK(ok, "t4 remaining tags intact");

	// remove everything, then the missing one must report TagNorFound
	bool clean = true;
	for (int i = 0; i < NSIG; i++)
	{
		if (AllSigs[i] == 305 || AllSigs[i] == 270) continue;
		if (t.RemoveTag((TiffTagSignature)AllSigs[i]) != Tiff_OK) clean = false;
	}
	CHECK(clean, "t4 remove all");
	CHECK(t.RemoveTag((TiffTagSignature)305) == TagNorFound, "t4 remove missing");
	bool gone = true;
	for (int i = 0; i < NSIG; i++)
		if (t.GetTag((TiffTagSignature)AllSigs[i]) != nullptr) gone = false;
	CHECK(gone, "t4 list empty");
}

//**************************************************************************
// 5) Multi-strip read
//**************************************************************************
static void TestMultiStrip()
{
	string path = string(TempDir) + "/rg_t5.tif";
	const int W = 8, L = 6, SPP = 3, RPS = 2;
	const int STRIPS = L / RPS;
	const int STRIP_BYTES = W * SPP * RPS;

	TiffBuilder b;
	b.set((unsigned short)ImageWidth, 4, 1, W);
	b.set((unsigned short)ImageLength, 4, 1, L);
	unsigned char bps3[6]; for (int i = 0; i < 3; i++) setU16(8, bps3 + i * 2);
	b.setBlob((unsigned short)BitsPerSample, 3, 3, bps3, 6);
	b.set((unsigned short)Compression, 3, 1, 1);
	b.set((unsigned short)PhotometricInterpretation, 3, 1, 2);
	vector<unsigned char> img(STRIPS * STRIP_BYTES);
	for (int i = 0; i < (int)img.size(); i++) img[i] = (unsigned char)((i * 17 + 3) & 0xFF);
	unsigned int offs[STRIPS], cnts[STRIPS];
	for (int i = 0; i < STRIPS; i++) { offs[i] = (unsigned int)((unsigned)i * STRIP_BYTES); cnts[i] = STRIP_BYTES; }
	b.setBlob((unsigned short)StripOffsets, 4, STRIPS, offs, sizeof(offs));
	b.markFixup((unsigned short)StripOffsets);
	b.set((unsigned short)SamplesPerPixel, 3, 1, SPP);
	b.set((unsigned short)278, 4, 1, RPS);
	b.setBlob((unsigned short)StripByteCounts, 4, STRIPS, cnts, sizeof(cnts));
	b.set((unsigned short)PlanarConfiguration, 3, 1, 1);
	b.image = img;
	CHECK(b.write(path.c_str()), "t5 craft");

	CTiff in;
	CHECK(in.ReadFile(path.c_str()) == Tiff_OK, "t5 read");
	CHECK(in.GetTagValue((TiffTagSignature)278) == L, "t5 rowsperstrip reset");
	CHECK(memcmp(in.GetImageBuf(), &img[0], img.size()) == 0, "t5 pixels");
}

//**************************************************************************
// 6) Planar=2 (RRRGGGBBB) + Pack
//**************************************************************************
static void TestPlanarPack()
{
	string path = string(TempDir) + "/rg_t6.tif";
	const int W = 4, L = 2, SPP = 3;
	const int PIXELS = W * L;

	TiffBuilder b;
	b.set((unsigned short)ImageWidth, 4, 1, W);
	b.set((unsigned short)ImageLength, 4, 1, L);
	unsigned char bps3[6]; for (int i = 0; i < 3; i++) setU16(8, bps3 + i * 2);
	b.setBlob((unsigned short)BitsPerSample, 3, 3, bps3, 6);
	b.set((unsigned short)Compression, 3, 1, 1);
	b.set((unsigned short)PhotometricInterpretation, 3, 1, 2);
	//NOTE: single-strip planar with inline count=1 strip tags crashes the reader
	//(pre-existing null lpData deref), so use array-form tags with a zero-length
	//second strip to reach the Pack path.
	unsigned int offsets[2] = { 0, 0 };
	b.setBlob((unsigned short)StripOffsets, 4, 2, offsets, sizeof(offsets));
	b.markFixup((unsigned short)StripOffsets);
	b.set((unsigned short)SamplesPerPixel, 3, 1, SPP);
	b.set(278, 4, 1, L);
	unsigned int counts[2] = { (unsigned int)(PIXELS * SPP), 0 };
	b.setBlob((unsigned short)StripByteCounts, 4, 2, counts, sizeof(counts));
	b.set((unsigned short)PlanarConfiguration, 3, 1, 2);

	vector<unsigned char> img(PIXELS * SPP);
	for (int i = 0; i < PIXELS; i++)
	{
		img[i] = (unsigned char)(i + 1);                  // R plane
		img[PIXELS + i] = (unsigned char)(100 + i);       // G plane
		img[2 * PIXELS + i] = (unsigned char)(200 + i);   // B plane
	}
	b.image = img;
	CHECK(b.write(path.c_str()), "t6 craft");

	CTiff in;
	CHECK(in.ReadFile(path.c_str()) == Tiff_OK, "t6 read");
	CHECK(in.GetTagValue(PlanarConfiguration) == 1, "t6 planar reset");

	// after Pack: row-major interleaved RGB
	vector<unsigned char> expect(PIXELS * SPP);
	for (int y = 0; y < L; y++)
		for (int x = 0; x < W; x++)
		{
			int src = y * W + x, dst = (y * W + x) * 3;
			expect[dst + 0] = img[src];
			expect[dst + 1] = img[PIXELS + src];
			expect[dst + 2] = img[2 * PIXELS + src];
		}
	CHECK(memcmp(in.GetImageBuf(), &expect[0], expect.size()) == 0, "t6 packed pixels");
}

//Single-strip planar with inline (count=1) strip tags. This exact layout used
//to null-deref in ReadMultiStripOffset (A8); the direct-read path must handle it.
static void TestPlanarPackInline()
{
	string path = string(TempDir) + "/rg_t6b.tif";
	const int W = 4, L = 2, SPP = 3;
	const int PIXELS = W * L;

	TiffBuilder b;
	b.set((unsigned short)ImageWidth, 4, 1, W);
	b.set((unsigned short)ImageLength, 4, 1, L);
	unsigned char bps3[6]; for (int i = 0; i < 3; i++) setU16(8, bps3 + i * 2);
	b.setBlob((unsigned short)BitsPerSample, 3, 3, bps3, 6);
	b.set((unsigned short)Compression, 3, 1, 1);
	b.set((unsigned short)PhotometricInterpretation, 3, 1, 2);
	unsigned int offsets[1] = { 0 };
	b.setBlob((unsigned short)StripOffsets, 4, 1, offsets, 4);
	b.markFixup((unsigned short)StripOffsets);
	b.set((unsigned short)SamplesPerPixel, 3, 1, SPP);
	b.set(278, 4, 1, L);
	unsigned int counts[1] = { (unsigned int)(PIXELS * SPP) };
	b.setBlob((unsigned short)StripByteCounts, 4, 1, counts, 4);
	b.set((unsigned short)PlanarConfiguration, 3, 1, 2);

	vector<unsigned char> img(PIXELS * SPP);
	for (int i = 0; i < PIXELS; i++)
	{
		img[i] = (unsigned char)(i + 1);
		img[PIXELS + i] = (unsigned char)(100 + i);
		img[2 * PIXELS + i] = (unsigned char)(200 + i);
	}
	b.image = img;
	CHECK(b.write(path.c_str()), "t6b craft");

	CTiff in;
	CHECK(in.ReadFile(path.c_str()) == Tiff_OK, "t6b read");
	CHECK(in.GetTagValue(PlanarConfiguration) == 1, "t6b planar reset");

	vector<unsigned char> expect(PIXELS * SPP);
	for (int y = 0; y < L; y++)
		for (int x = 0; x < W; x++)
		{
			int src = y * W + x, dst = (y * W + x) * 3;
			expect[dst + 0] = img[src];
			expect[dst + 1] = img[PIXELS + src];
			expect[dst + 2] = img[2 * PIXELS + src];
		}
	CHECK(memcmp(in.GetImageBuf(), &expect[0], expect.size()) == 0, "t6b packed pixels");
}

//**************************************************************************
// 7) 1-bit CMYKcm (Avision undocumented, Planar=2 single strip)
//**************************************************************************
static void TestOneBitCmykcm()
{
	string path = string(TempDir) + "/rg_t7.tif";
	const int W = 16, L = 2, SPP = 6;
	const int STRIP_BYTES = (W / 8) * L * SPP;

	TiffBuilder b;
	b.set((unsigned short)ImageWidth, 4, 1, W);
	b.set((unsigned short)ImageLength, 4, 1, L);
	unsigned char bps6[12]; for (int i = 0; i < SPP; i++) setU16(1, bps6 + i * 2);
	b.setBlob((unsigned short)BitsPerSample, 3, SPP, bps6, 12);
	b.set((unsigned short)Compression, 3, 1, 1);
	b.set((unsigned short)PhotometricInterpretation, 3, 1, 0);
	unsigned int offsets[1] = { 0 };
	b.setBlob((unsigned short)StripOffsets, 4, 1, offsets, 4);
	b.markFixup((unsigned short)StripOffsets);
	b.set((unsigned short)SamplesPerPixel, 3, 1, SPP);
	b.set(278, 4, 1, L);
	unsigned int counts[1] = { (unsigned int)STRIP_BYTES };
	b.setBlob((unsigned short)StripByteCounts, 4, 1, counts, 4);
	b.set((unsigned short)PlanarConfiguration, 3, 1, 2);

	vector<unsigned char> img(STRIP_BYTES);
	for (int i = 0; i < STRIP_BYTES; i++) img[i] = (unsigned char)((i * 89 + 1) & 0xFF);
	b.image = img;
	CHECK(b.write(path.c_str()), "t7 craft");

	CTiff in;
	CHECK(in.ReadFile(path.c_str()) == Tiff_OK, "t7 read");
	CHECK(memcmp(in.GetImageBuf(), &img[0], img.size()) == 0, "t7 pixels");
}


//**************************************************************************
// 9) G3 / G4 (CCITT) round-trip, 1 bit per pixel
//**************************************************************************
static const char* tag(int comp) { return (comp == 4) ? "t10 G4" : "t9 G3"; }

static void TestG3G4One(int W, int L, int comp)
{
	string path = string(TempDir) + "/rg_t9.tif";

	CTiff out;
	CHECK(out.CreateNew(W, L, 72, 1, 1, 1) == Tiff_OK, tag(comp));
	int rb = (W + 7) / 8;
	vector<unsigned char> ref(rb * L);
	unsigned r = (unsigned)(W * 7 + L * 13 + comp);
	for (int i = 0; i < rb * L; i++)
	{
		r = r * 1103515245 + 12345;
		ref[i] = (unsigned char)((comp == 3 ? (r >> 20) : (unsigned)(i * 37)) & 0xFF);
	}
	//Force long runs to exercise makeup codes.
	if (L > 2)
	{
		memset(&ref[rb], 0, (size_t)rb);//white row
		memset(&ref[rb * (L - 1)], 0xFF, (size_t)rb);//black row
	}
	for (int y = 0; y < L; y++)
		out.PutRow(&ref[y * rb], y);
	CHECK(out.SaveFile(path.c_str(), comp) == Tiff_OK, tag(comp));

	CTiff in;
	CHECK(in.ReadFile(path.c_str()) == Tiff_OK, tag(comp));
	CHECK(in.GetTagValue((TiffTagSignature)258) == 1, tag(comp));
	CHECK(memcmp(in.GetImageBuf(), &ref[0], (size_t)rb * L) == 0, tag(comp));
}

static void TestG3G4()
{
	struct SZ { int w, l; };
	SZ sizes[] = { {64,9},{17,5},{3000,8},{1,3},{1728,40} };
	for (size_t i = 0; i < sizeof(sizes) / sizeof(sizes[0]); i++)
	{
		TestG3G4One(sizes[i].w, sizes[i].l, 3);
		TestG3G4One(sizes[i].w, sizes[i].l, 4);
	}
}

//**************************************************************************
// 8) Invalid files must return error codes, never throw/crash (via CTiff)
//**************************************************************************
static void TestBadFiles()
{
	// 8a: not a tiff at all
	{
		string p = string(TempDir) + "/rg_t8a.bin";
		FILE* fp = fopen(p.c_str(), "wb"); fprintf(fp, "This is not a tiff file, really."); fclose(fp);
		CTiff in;
		CHECK(in.ReadFile(p.c_str()) != Tiff_OK, "t8a not-a-tiff");
	}
	// 8b: truncated header
	{
		string p = string(TempDir) + "/rg_t8b.bin";
		FILE* fp = fopen(p.c_str(), "wb"); fprintf(fp, "II*x"); fclose(fp);
		CTiff in;
		CHECK(in.ReadFile(p.c_str()) != Tiff_OK, "t8b truncated");
	}
	// 8c: valid header, TagCount = 0
	{
		string p = string(TempDir) + "/rg_t8c.tif";
		FILE* fp = fopen(p.c_str(), "wb");
		const unsigned char hdr[10] = { 'I','I', 42, 0, 8, 0, 0, 0, 0, 0 };
		fwrite(hdr, 1, 10, fp); fclose(fp);
		CTiff in;
		CHECK(in.ReadFile(p.c_str()) != Tiff_OK, "t8c zero tags");
	}
	// 8d: missing file
	{
		CTiff in;
		CHECK(in.ReadFile("/tmp/rg_no_such_file.tif") != Tiff_OK, "t8d missing file");
	}
}

int main()
{
	TestRoundTrip8();
	TestRoundTrip16();
	TestLzw();
	TestRemoveTag();
	TestMultiStrip();
	TestPlanarPack();
	TestPlanarPackInline();
	TestG3G4();
	TestOneBitCmykcm();
	TestBadFiles();

	fprintf(stderr, "=== TiffRefactorGuard: %s (%d failed) ===\n", g_Fails ? "FAIL" : "ALL PASS", g_Fails);
	return g_Fails;
}
