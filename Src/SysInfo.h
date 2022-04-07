#if !defined(_SYSINFO_H__)
#define _SYSINFO_H__

#if !defined(WIN32) && !defined(X64)
	#include <stdint.h>

	typedef int8_t		INT8;
	typedef uint8_t		UINT8;
	typedef uint8_t		BYTE;

	typedef int16_t		INT16;
	typedef uint16_t    UINT16;
	typedef uint16_t    WORD;

	#define ICUINT32TYPE //For Icc4.h icUInt32Number --> 64bits for 64bit compiler
	typedef int32_t		INT32;
	typedef uint32_t    UINT32;
	typedef uint32_t    DWORD;

	#define ICUINT64TYPE
	typedef int64_t		INT64;
	typedef int64_t		LONGLONG;
	typedef uint64_t	UINT64;
	typedef uint64_t	ULONGLONG;

	typedef unsigned int	 	UINT;
	typedef char				INSTR;
	typedef unsigned char		UINSTR;
	typedef bool	 			BOOL;
	typedef WORD				WINSTR;
	typedef const WINSTR		LPCWSTR;
	typedef BYTE *				LPBYTE;
	typedef WORD *				LPWORD;
	typedef DWORD *				LPDWORD;
	typedef int*				LPINT;
	typedef const char*			LPCSTR;
	//typedef int				size_t;

	#define FALSE				0
	#define TRUE				1
	#define nullptr				0

	#ifdef Mac
		#define HiByteFirst
	#endif 
#else
	#include <windows.h>
	#include <stdlib.h>
	#ifdef _DEBUG
		#define DBG_Printf(fmt, ...)	printf("XUJY:"##fmt, ## __VA_ARGS__)

		//For Memory leak detection.
		#define _CRTDBG_MAP_ALLOC 
		#ifndef DBG_NEW     
		#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )     
		#endif//DBG_NEW

		#define new DBG_NEW 
		#define DETECT_MEMORY_LEAKS _CrtDumpMemoryLeaks()
		//You can using these command to find who alloc the momory.
		//VC will pause when alloc memory 100 times.
		//_CrtSetBreakAlloc(100) or _crtBreakAlloc = 100; 
	#else		
		#define DETECT_MEMORY_LEAKS 
	#endif //_DEBUG
#endif//WIN32

// *************************************************
// Common Define
// *************************************************
#define NoErr				0x00000000
#define UnKnown				0xFFFFFFFF
#define FileNotFound		0xFFFFFFFE

#ifdef _DEBUG
	#define DBG_Printf(fmt, ...)	printf("XUJY:"##fmt, ## __VA_ARGS__)
#else
	#define DBG_Printf(fmt, ...)	
#endif //_DEBUG

//DWORD SwapDWORD(const DWORD x);
//WORD SwapWORD(const WORD x);

#ifdef HiByteFirst 
	inline DWORD SwapDWORD(const DWORD x)
	{
		return x;
	}

	inline WORD SwapWORD(const WORD x)
	{
		return x;
	}
#else		
	#ifdef WIN32  //For VC (Little Endian)		
			#define	SWAP
			#define SwapWORD _byteswap_ushort 
			#define SwapDWORD _byteswap_ulong 
	#endif //WIN32

	#ifdef LINUX
		#define	SWAP
		#define SwapWORD __builtin_bswap16  
		#define SwapDWORD __builtin_bswap32 
	#endif//WIN32

#ifndef SWAP
inline DWORD SwapDWORD(const DWORD x)
{	return (((x & 0xFF000000) >> 24) | ((x & 0xFF0000) >> 8) | ((x & 0xFF00) << 8) | (x << 24));}

inline WORD SwapWORD(const WORD x)
{	return (((x & 0xFF) << 8) | (x >> 8));}
#endif //SWAP

#endif //HiByteFirst   

inline void SwapDWORD_Buf(LPDWORD lpBuf, int Size)
{
	LPDWORD lpIn, lpOut;
	lpIn = lpOut = lpBuf;
	for (int i = 0; i < Size; i++)
		*(lpOut++) = SwapDWORD(*(lpIn++));
}

inline void SwapWORD_Buf(LPWORD lpBuf, int Size)
{
	LPWORD lpIn, lpOut;
	lpIn = lpOut = lpBuf;
	for (int i = 0; i < Size; i++)
		*(lpOut++) = SwapWORD(*(lpIn++));
}

//For C++ 11
#define ENABLE_SHARED_POINTER

#ifdef ENABLE_SHARED_POINTER
	#define SMART_POINTER 1
	//#define SMART_PTR(CLASS, ptr) shared_ptr<CLASS>(ptr)
	#define SMART_PTR(CLASS, ptr) unique_ptr<CLASS>(ptr)
	#define GetPtr(ptr) ptr.get()
#else
	#define SMART_POINTER 0
	#define SMART_PTR(CLASS, ptr) ptr
	#define GetPtr(ptr) ptr
#endif //SHARED_POINTER

#endif //_SYSINFO_H__