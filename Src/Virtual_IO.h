#ifndef __IO_Interface_H__
#define __IO_Interface_H__
#include <fstream>
using namespace std;

class IO_Interface
{
public:
	IO_Interface(){};
	virtual ~IO_Interface(){};
	virtual size_t Read(LPBYTE buffer, size_t size, size_t count) = 0;
	virtual size_t Write(LPBYTE buffer, size_t size, size_t count) = 0;
	virtual int Seek(int offset, int origin) = 0;
	virtual void* GetHandle() = 0;
	virtual void Close() = 0; //Same as Reset

};

class IO_File: public IO_Interface
{
public:
	IO_File(const char *FileName, const char *Option);
	~IO_File();
	size_t Read(LPBYTE buffer, size_t size, size_t count);;
	size_t Write(LPBYTE buffer, size_t size, size_t count);
	int Seek(int offset, int origin);
	void* GetHandle();
	void Close();

private:
	IO_File();
	FILE *m_File;
};

class IO_fstream: public IO_Interface
{
public:
	IO_fstream(const char *FileName, int option);
	~IO_fstream();
	size_t Read(LPBYTE buffer, size_t size, size_t count);
	size_t Write(LPBYTE buffer, size_t size, size_t count);
	int Seek(int offset, int origin);
	void* GetHandle();
	void Close(); //Same as Reset


private:
	IO_fstream();
	fstream *m_File;
};

class IO_Buf: public IO_Interface
{
public:
	IO_Buf(LPBYTE Buffer, size_t Size);
	size_t Read(LPBYTE buffer, size_t size, size_t count);
	size_t Write(LPBYTE buffer, size_t size, size_t count);
	size_t Tell(){return m_Current - m_Start;};
	int Seek(int offset, int origin);
	void* GetHandle();
	void Close(); //Same as Reset

private:
	IO_Buf(){};//Force using IO_Buf(Buffer, size);
	LPBYTE m_Start, m_End, m_Current;	
};


#endif //__IO_Interface_H__