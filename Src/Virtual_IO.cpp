#include "stdafx.h"

#if !defined(SYS_INFO)
#include "SysInfo.h"
#endif SYS_INFO

#include "Virtual_IO.h"

/***************************************************************************************
	C Version : It will be fast, if inline is enable.
 ***************************************************************************************/
IO_File::IO_File(const char *FileName, const char *Option)
{
	m_File = fopen(FileName, Option);
}

IO_File::~IO_File()
{
	Close();
}

void IO_File::Close()
{
	if (m_File != NULL)
		fclose(m_File);
	m_File = NULL;
}

size_t IO_File::Read(LPBYTE buffer, size_t size, size_t count)
{
	return fread(buffer, size, count, m_File);
}

size_t IO_File::Write(LPBYTE buffer, size_t size, size_t count)
{
	return fwrite(buffer, size, count, m_File);
}

int IO_File::Seek(int offset, int origin)
{
	return fseek(m_File, offset, origin);
}

void* IO_File::GetHandle()
{
	return (void*)m_File;
}

/***************************************************************************************
	IO_File(STL Version)
 ***************************************************************************************/
IO_fstream::IO_fstream(const char *FileName, int option)
{
	m_File = new fstream(FileName, ios::binary | option);
}

IO_fstream::~IO_fstream()
{
	Close();
}

void IO_fstream::Close()
{
	m_File->close();
	delete m_File;
	m_File = NULL;
}

size_t IO_fstream::Read(LPBYTE buffer, size_t size, size_t count)
{
	m_File->read((char*)buffer, (int)(size * count)); return size * count;
}

size_t IO_fstream::Write(LPBYTE buffer, size_t size, size_t count)
{
	m_File->write((char*)buffer, (int)(size*count)); return (size_t)size*count;
}

//Not check yet
int IO_fstream::Seek(int offset, int origin)
{
	m_File->seekp(offset, origin); return 0;
}

void* IO_fstream::GetHandle()
{
	return (void*)m_File;
}

/***************************************************************************************
 ***************************************************************************************/
IO_Buf::IO_Buf(LPBYTE Buffer, size_t Size)
{
	m_Start = Buffer;
	m_Current = Buffer;
	m_End = Buffer + Size;
}

size_t IO_Buf::Read(LPBYTE buffer, size_t size, size_t count)
{
	size_t ReadSize = size * count;
	if ((m_Current + ReadSize) > m_End)
		ReadSize = (size_t)(m_End - m_Current);

	memcpy(buffer, m_Current, ReadSize);
	m_Current += ReadSize;

	return (size_t)m_Current;
}

size_t IO_Buf::Write(LPBYTE buffer, size_t size, size_t count)
{
	size_t WriteSize = size * count;
	if ((m_Current + WriteSize) > m_End)
		WriteSize = m_End - m_Current;

	memcpy(m_Current, buffer, WriteSize);
	m_Current += size * count;

	return (size_t)m_Current;
}

int IO_Buf::Seek(int offset, int origin)
{
	if (origin == SEEK_SET)
		m_Current = m_Start + offset;
	else if (origin == SEEK_CUR)
		m_Current = m_Current + offset;
	else if (origin == SEEK_END)
		m_Current = m_End + offset;

	if (offset > 0)
	{
		if (m_Current > m_End)
		{
			offset = (int)(m_Current - m_End);
			m_Current = m_End;
		}
	}
	else//offset <= 0
	{
		if (m_Current < m_Start)
		{
			offset = (int)(m_Current - m_Start);
			m_Current = m_Start;
		}
	}
	return offset;
}

void* IO_Buf::GetHandle()
{
	return (void*)m_Start;
}

void IO_Buf::Close()
{// Don't delete or free the memory, it's not your business.
	m_Start = NULL;
	m_Current = NULL;
	m_End = NULL;
}

