#ifndef __IO_Interface_C_H__
#define __IO_Interface_C_H__
#include <fstream>

struct IO_S
{
	void* (*Close)(IO_S* This);
	size_t (*Read)(IO_S* This, LPBYTE , size_t, size_t);
	size_t (*Write)(IO_S* This, LPBYTE , size_t, size_t);
	int (*Seek)(IO_S* This, int, int);
	void* (*GetHandle)(IO_S* This);
	void* pImpl;
	unsigned char *pStart, *pCurrent, *pEnd;
};

//C language. fopen version...
//#define io_file 1
#define io_buf 1

#define IO_Interface					IO_S //Type
#define IO_C							IO_S //Argument
//#define IO_In_C(FileName)				IO_Buf_In(FileName)
//#define IO_Out_C(FileName)			IO_Buf_Out(FileName)
//#define IO_Close_C(IO_C)				IO_S->Close(IO_C)
#define IO_Read_C(Str, Size, Count)		IO_S->Read(IO_C, (LPBYTE)Str, Size, Count)
#define IO_Write_C(Str, Size, Count)	IO_S->Write(IO_C, (LPBYTE)Str, Size, Count)
#define IO_Seek_C(Offset, Origin)		IO_S->Seek(IO_C, Offset, Origin)

IO_S* IO_File(const char* FileName, const char* Option);
void  IO_File_Close(IO_S* This);
IO_S* IO_Buf(LPBYTE Buffer, size_t Size);
void  IO_Buf_Close(IO_S* This);

#endif //__IO_Interface_C_H__