#include "stdafx.h"
#include <stdlib.h>   // For _MAX_PATH definition
#include <stdio.h>
#include <malloc.h>

struct TAG
{
	short tag;
	short type;
	unsigned int n;
	unsigned int value;
};

int DataType[12] = {
	1,//Unknown						= 0x0000L,
	1,//Byte						= 0x0001L,
	1,//ASCII						= 0x0002L,
	2,//Short						= 0x0003L,
	4,//Long						= 0x0004L,
	8,//Rational					= 0x0005L,
	1,//SBYTE						= 0x0006L,
	1,//UndefineType				= 0x0007L,
	2,//SShort						= 0x0008L,
	4,//SLong						= 0x0009L,
	4,//Float						= 0x000AL,
	8//Double						= 0x000BL
};

int Tiff_FindNextIFD(FILE *TarFile)
{
	int IFD = 8;
	int Count = 0;
	int Offset = 0;
	int ret = fseek(TarFile, IFD, SEEK_SET);
		
	do
	{
		fread(&Count, 2, 1, TarFile);
		
		Offset = ftell(TarFile);
		ret = fseek(TarFile, Count * 12, SEEK_CUR);
		Offset = ftell(TarFile);
		
		fread(&IFD, 4, 1, TarFile);
		if(IFD != 0)
			ret = fseek(TarFile, IFD, SEEK_SET);
	} while (IFD != 0);
	return Offset;
}

void Tiff_SetNextIFD(FILE *Tif_Dst, int Offset_IFD, int Offset_TifDst)
{
	int ret = fseek(Tif_Dst, Offset_IFD, SEEK_SET);
	//ret = ftell(Tif_Dst);
	int NextIFD = Offset_TifDst;
	fwrite(&NextIFD, 1, 4, Tif_Dst);
	//ret = ftell(Tif_Dst);
	ret = fseek(Tif_Dst, 0, SEEK_END);
	//ret = ftell(Tif_Dst);
}

void CheckTag(TAG* tag, int Offset, FILE *Tif_Src)
{	
	int size = DataType[tag->type] * tag->n;
		
	if ((tag->tag == 273) || (tag->tag == 279))//special case StripOffsets, StripByteCounts
		if (size > 4)
		{//Reset StripOffset data
			int i = 0, n = tag->n;
			unsigned int* lpBuf = (unsigned int*)malloc(n * 4);
			memset(lpBuf, 0, n * 4);
			unsigned int* lpIndex = lpBuf;

			int current = ftell(Tif_Src);
			fseek(Tif_Src, (tag->value), SEEK_SET);
			
			fread(lpBuf, 4, tag->n, Tif_Src);
			for (i = 0; i < n; i++)
				*(lpIndex++) += Offset;

			fseek(Tif_Src, -(n*4), SEEK_CUR);
			fwrite(lpBuf, 4, n, Tif_Src);

			fseek(Tif_Src, current, SEEK_SET);
			free(lpBuf);
		}
		else
			if(tag->tag == 273)
				tag->value += Offset;

	if (size > 4)
		tag->value += Offset;
}

void Tiff_CopyIFD(FILE* Tif_Dst, FILE* Tif_Src, int Offset_TifDst)
{
	int index = 0, Count = 0, Offset = 0, IFD = 8;
	int ret = fseek(Tif_Src, IFD, SEEK_SET);
	
	TAG *lpBuf, *lpTag;
	//TAG lpBuf[36];
	//TAG *lpTag;

	ret = fread(&Count, 2, 1, Tif_Src);
	fwrite(&Count, 1, 2, Tif_Dst);
	lpBuf = (TAG*)malloc(Count * 12);
	fread(lpBuf, 12, Count, Tif_Src);

	lpTag = lpBuf;
	for (index = 0; index < Count; index++)
		CheckTag(lpTag++, Offset_TifDst - 8, Tif_Src);//Skip Tif_Src Header(IFD Offset) 8 bytes
		
	fwrite(lpBuf, 12, Count, Tif_Dst);
	free(lpBuf);
}

void Tiff_CopyRaw(FILE* Tiff_Dst, FILE* Tif_Src)
{
	int Size = 1024, ret = 0;
	void* lpBuf = malloc(Size);
	do{
		ret = fread(lpBuf, 1, Size, Tif_Src);
		ret = fwrite(lpBuf, 1, ret, Tiff_Dst);
	} while (ret != 0);

	free(lpBuf);
}


int Tiff_Append(char* Tar, char* Src)
{
	FILE* Tif_Dst = fopen(Tar, "r+b");
	FILE* Tif_Src = fopen(Src, "r+b");
	int Offset_IFD = Tiff_FindNextIFD(Tif_Dst);

	int Offset_TifDst;
	fseek(Tif_Dst, 0, SEEK_END);
	Offset_TifDst = ftell(Tif_Dst);

	Tiff_SetNextIFD(Tif_Dst, Offset_IFD, Offset_TifDst);

	Tiff_CopyIFD(Tif_Dst, Tif_Src, Offset_TifDst);
	Tiff_CopyRaw(Tif_Dst, Tif_Src);
	
	fclose(Tif_Src);
	fclose(Tif_Dst);
	return 0;
}