#if !defined(_SYSINFO_H__)
#define _SYSINFO_H__

#ifndef WIN32
	//using namespace std;
	typedef INSTR				INSTR;
	typedef unsigned INSTR		BYTE;
	typedef unsigned short      WORD;
	typedef unsigned long       DWORD;
	typedef unsigned int	 	UINT;
	typedef unsigned INSTR		UINSTR;
	typedef int		 			BOOL;
	typedef WORD				WINSTR;
	typedef INSTR				TINSTR;
	typedef WINSTR				LPWSTR;
	typedef const WINSTR		LPCWSTR;
	typedef TINSTR				LPTSTR;
	typedef const TINSTR		LPCTSTR;
	typedef BYTE *				LPBYTE;
	typedef WORD *				LPWORD;
	typedef DWORD *				LPDWORD;
	typedef int*				LPINT;
	typedef INSTR				LPSTR;

	//typedef int					size_t;
	typedef long				LONG;
	#define FALSE   0
	#define TRUE    1
	#define nullptr    0

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
	{	return x;}

	inline WORD SwapWORD(const WORD x)
	{	return x;}
#else
	#if 1  //For VC (Little Endian)
		#ifdef WIN64
			#define SwapWORD _byteswap_ushort 
			#define SwapDWORD _byteswap_ulong 
		#else
			#define SwapWORD _byteswap_ushort 
			#define SwapDWORD _byteswap_ulong 
		#endif //WIN64
		#define	SWAP
	#else
		inline DWORD SwapDWORD(const DWORD &x)
		{	return ((( x & 0xFF000000) >> 24) |(( x & 0xFF0000) >> 8) | (( x & 0xFF00) << 8) | (x << 24));}

		inline WORD SwapWORD(const WORD x)
		{	return ((( x & 0xFF) << 8) | (x >> 8));}
	#endif

	inline void SwapDWORD_Buf(LPDWORD lpBuf, int Size)
	{
		LPDWORD lpIn, lpOut;
		lpIn = lpOut = lpBuf;	
		for(int i = 0; i < Size; i++)
			*(lpOut++) = SwapDWORD(*(lpIn++));
	}

	inline void SwapWORD_Buf(LPWORD lpBuf, int Size)
	{
		LPWORD lpIn, lpOut;
		lpIn = lpOut = lpBuf;	
		for(int i = 0; i < Size; i++)
			*(lpOut++) = SwapWORD(*(lpIn++));
	}

#endif //HiByteFirst   

//For C++ 11
#define ENABLE_SHARED_POINTER

#ifdef ENABLE_SHARED_POINTER
	#define SHARED_POINTER
	#define SHARED_PTR(CLASS, ptr) shared_ptr<CLASS>(ptr)
	#define GetPtr(ptr) ptr.get()

#else
	#define SHARED_PTR(CLASS, ptr) ptr
	#define GetPtr(ptr) ptr
#endif //SHARED_POINTER

#endif //_SYSINFO_H__