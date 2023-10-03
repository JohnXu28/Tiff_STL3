#include "stdafx.h"
#include "..\Src\SysInfo.h"
#include "Virtual_IO_C.h"

/***************************************************************************************
	C Version : It will be fast, if inline is enable.
 ***************************************************************************************/
#define m_File (FILE*)This->pImpl
size_t IO_File_Read(IO_S *This, LPBYTE buffer, size_t size, size_t count)
{	
	return fread(buffer, size, count, m_File);	
}

size_t IO_File_Write(IO_S *This, LPBYTE buffer, size_t size, size_t count)
{	
	return fwrite(buffer, size, count, m_File);
}

int IO_File_Seek(IO_S *This, int offset, int origin)
{	
	return fseek(m_File, offset, origin);
}

void* IO_File_GetHandle(IO_S *This)
{	
	return (void*)m_File;
}

//Constructor.
IO_S* IO_File(const char *FileName, const char *Option)
{
	IO_S *This = (IO_S*)malloc(sizeof(IO_S));
	This->pImpl = (void*)fopen(FileName, Option);
	This->Read = IO_File_Read;
	This->Write = IO_File_Write;
	This->Seek = IO_File_Seek;	
	This->GetHandle = IO_File_GetHandle;
	This->pStart = nullptr;
	This->pEnd = nullptr;
	This->pCurrent = nullptr;
	This->Close = nullptr; //?
	return This;
}

//Destruct.
void IO_File_Close(IO_S *This)
{
	FILE *file = (FILE*)(This->GetHandle(This));
	if(file != NULL)
		fclose(file);
	free(This);
}

/***************************************************************************************
	IO Buf. You Must alloc memory first.
 ***************************************************************************************/
#define m_Start This->pStart
#define m_Current This->pCurrent
#define m_End This->pEnd

size_t IO_Buf_Read(IO_S *This, LPBYTE buffer, size_t size, size_t count)
{	
	size_t ReadSize = size * count;
	if(((LPBYTE)m_Current + ReadSize) > m_End)
		ReadSize = (size_t)((LPBYTE)m_End - m_Current);

	memcpy(buffer, m_Current, ReadSize);
	m_Current = (LPBYTE)m_Current + ReadSize;

	return (size_t)m_Current;
}

size_t IO_Buf_Write(IO_S *This, LPBYTE buffer, size_t size, size_t count)
{	
	size_t WriteSize = size * count;
	if(((LPBYTE)m_Current + WriteSize) > m_End)
		WriteSize = (LPBYTE)m_End - m_Current;

	memcpy(m_Current, buffer, WriteSize);
	m_Current = (LPBYTE)m_Current + size * count;

	return (size_t)m_Current;
}

int IO_Buf_Seek(IO_S *This, int offset, int origin)
{	
	if(origin == SEEK_SET)
		m_Current = (LPBYTE)m_Start + offset;
	else if(origin == SEEK_CUR)
		m_Current = (LPBYTE)m_Current + offset;
	else if(origin == SEEK_END)
		m_Current = (LPBYTE)m_End + offset;

	if (offset > 0)
	{
		if(m_Current > m_End)
		{
			offset = (int)((LPBYTE)m_Current - m_End);
			m_Current = m_End;			
		}
	}
	else//offset <= 0
	{
		if(m_Current < m_Start)
		{
			offset = (int)((LPBYTE)m_Current - m_Start);
			m_Current = m_Start;
		}	
	}
	return offset;
}

void* IO_Buf_GetHandle(IO_S *This)
{	return (void*)m_Start;}


IO_S* IO_Buf(LPBYTE Buffer, size_t Size)
{	
	IO_S *This = (IO_S*)malloc(sizeof(IO_S));
	This->Read = IO_Buf_Read;
	This->Write = IO_Buf_Write;
	This->Seek = IO_Buf_Seek;	
	This->GetHandle = IO_Buf_GetHandle;
	This->pImpl = Buffer;
		
	m_Start = Buffer;
	m_Current = Buffer;
	m_End = Buffer + Size;
	
	This->Close = nullptr;
	return This;
}

void IO_Buf_Close(IO_S *This)
{
	// We malloc Memory, we free.
	m_Start = NULL;
	m_Current = NULL;
	m_End = NULL;
	
	free(This->pImpl);
	free(This);
}

IO_S* IO_Buf_In(const char* FileName)
{
	FILE *file = fopen(FileName, "rb");
	size_t Size = 1024*1024*2;
	LPBYTE lpTemp = (LPBYTE)malloc(Size);
	
	size_t ret = fread(lpTemp, 1, Size, file);
	IO_S *This = IO_Buf(lpTemp, ret);
	
	fclose(file);

	return This;
}
