#include <windows.h>

#define LZW_MAX_BIT		12
#define LZW_MIN_BIT		9
#define CLEAR_CODE		256
#define END_CODE		257
#define MAX_DIC_SIZE	4094

#define NULL_CODE -1

struct Code
{
	int pre ;		// PreCode;
	int head;		// HeadCodeOfPreCodeIsItself;
	int next;		// NextCodeOfHaveSamePreCode;
	int str;
	int str_size;
};

class Lzw
{
private:
	Code code_list[1<<LZW_MAX_BIT];
	int code_size;
	UINT8 encode_bit;
	UINT16 pre_index;
	UINT32 pre_size;

	void init();
	void table_reset();
	void deinit();
	int AddCode( int pre_code, int code );
	int FindStr(int pre_code, int code);
	int ReadCode(UINT8 **inbuf);
	int WriteValue(UINT8 **outbuf, int code);
	int WriteCode(UINT8 **outbuf, UINT16 index);
	int WriteCodeFlush(UINT8 **outbuf);
	int GetFirstCode(int code);
	int CheckEncodeListFull();
	int CheckDecodeListFull();
public :
	Lzw();
	~Lzw();
	int EncodeSize(UINT32 inbuf_size);
	int Encode(UINT8 *inbuf, UINT8 *outbuf, UINT32 inbuf_size);
	int Decode(UINT8 *inbuf, UINT8 *outbuf, UINT32 outbuf_size);
	void PredicatorDecode(UINT8* inbuf, UINT32 width, UINT32 length, UINT32 channel);
	void PredicatorEncode(UINT8* inbuf, UINT32 width, UINT32 length, UINT32 channel);

	int m_EncodeSize;
	LPBYTE m_lpEncodeBuf;
};