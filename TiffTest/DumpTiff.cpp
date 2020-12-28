// DumpTiff.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <string>
#include <iostream>
#include <iomanip>
#include <map>
#include <Tiff_STL3\Src\Tiff_STL3.h>
using namespace std;

/***********************************************************************/
char* TypeName[] = {"Unknown", "Byte", "ASCII", "Short", "Long", "Rational", "SBYTE", "Undefine", "SShort", "SLong", "Float", "Double"};
typedef pair <TiffTagSignature, char*> Tag_Pair;
map<TiffTagSignature, char*> TagMap;
void InitialMap()
{
	TagMap.insert( Tag_Pair( NullTag, "NullTag" ) );
	TagMap.insert( Tag_Pair( NewSubfileType, "NewSubfileType" ) );
	TagMap.insert( Tag_Pair( ImageWidth, "ImageWidth" ) );
	TagMap.insert( Tag_Pair( ImageLength, "ImageLength" ) );
	TagMap.insert( Tag_Pair( BitsPerSample, "BitsPerSample" ) );
	TagMap.insert( Tag_Pair( Compression, "Compression" ) );
	TagMap.insert( Tag_Pair( PhotometricInterpretation, "PhotometricInterpretation" ) );
	TagMap.insert( Tag_Pair( RowsPerStrip, "RowsPerStrip" ) );
	TagMap.insert( Tag_Pair( StripByteCounts, "StripByteCounts" ) );
	TagMap.insert( Tag_Pair( PlanarConfiguration, "PlanarConfiguration" ) );
	TagMap.insert( Tag_Pair( NewSubfileType, "NewSubfileType" ) );
	TagMap.insert( Tag_Pair( ResolutionUnit, "ResolutionUnit" ) );
	TagMap.insert( Tag_Pair( Threshholding, "Threshholding" ) );
	TagMap.insert( Tag_Pair( Orientation, "Orientation" ) );
	TagMap.insert( Tag_Pair( NewSubfileType, "NewSubfileType" ) );
	TagMap.insert( Tag_Pair( SamplesPerPixel, "SamplesPerPixel" ) );
	TagMap.insert( Tag_Pair( MaxSampleValue, "MaxSampleValue" ) );
	TagMap.insert( Tag_Pair( XResolution, "XResolution" ) );
	TagMap.insert( Tag_Pair( YResolution, "YResolution" ) );
	TagMap.insert( Tag_Pair( MinSampleValue, "MinSampleValue" ) );
	TagMap.insert( Tag_Pair( CellWidth, "CellWidth" ) );
	TagMap.insert( Tag_Pair( CellLength, "CellLength" ) );
	TagMap.insert( Tag_Pair( FillOrder, "FillOrder" ) );
	TagMap.insert( Tag_Pair( DocumentName, "DocumentName" ) );
	TagMap.insert( Tag_Pair( ImageDescription, "ImageDescription" ) );
	TagMap.insert( Tag_Pair( NullTag, "NullTag" ) );
	TagMap.insert( Tag_Pair( NewSubfileType, "NewSubfileType" ) );
	TagMap.insert( Tag_Pair( Make, "Make" ) );
	TagMap.insert( Tag_Pair( Model, "Model" ) );
	TagMap.insert( Tag_Pair( StripOffsets, "StripOffsets" ) );
	TagMap.insert( Tag_Pair( NewSubfileType, "NewSubfileType" ) );
	TagMap.insert( Tag_Pair( PageName, "PageName" ) );
	TagMap.insert( Tag_Pair( XPosition, "XPosition" ) );
	TagMap.insert( Tag_Pair( YPosition, "YPosition" ) );
	TagMap.insert( Tag_Pair( FreeOffsets, "FreeOffsets" ) );
	TagMap.insert( Tag_Pair( FreeByteCounts, "FreeByteCounts" ) );
	TagMap.insert( Tag_Pair( NewSubfileType, "NewSubfileType" ) );
	TagMap.insert( Tag_Pair( GrayResponseUnit, "GrayResponseUnit" ) );
	TagMap.insert( Tag_Pair( NewSubfileType, "NewSubfileType" ) );
	TagMap.insert( Tag_Pair( GrayResponsCurve, "GrayResponsCurve" ) );
	TagMap.insert( Tag_Pair( T6Options, "T6Options" ) );
	TagMap.insert( Tag_Pair( PageNumber, "PageNumber" ) );
	TagMap.insert( Tag_Pair( TransferFunction, "TransferFunction" ) );
	TagMap.insert( Tag_Pair( Software, "Software" ) );
	TagMap.insert( Tag_Pair( DateTime, "DateTime" ) );
	TagMap.insert( Tag_Pair( Artist, "Artist" ) );
	TagMap.insert( Tag_Pair( HostComputer, "HostComputer" ) );
	TagMap.insert( Tag_Pair( Predicator, "Predicator" ) );
	TagMap.insert( Tag_Pair( TiffWhitePoint, "WhitePoint" ) );
	TagMap.insert( Tag_Pair( PrimaryChromaticities, "PrimaryChromaticities" ) );
	TagMap.insert( Tag_Pair( ColorMap, "ColorMap" ) );
	TagMap.insert( Tag_Pair( HalftoneHints, "HalftoneHints" ) );
	TagMap.insert( Tag_Pair( TileWidth, "TileWidth" ) );
	TagMap.insert( Tag_Pair( TileLength, "TileLength" ) );
	TagMap.insert( Tag_Pair( TileOffsets, "TileOffsets" ) );
	TagMap.insert( Tag_Pair( TileByteCounts, "TileByteCounts" ) );
	TagMap.insert( Tag_Pair( InkSet, "InkSet" ) );
	TagMap.insert( Tag_Pair( InkNames, "InkNames" ) );
	TagMap.insert( Tag_Pair( NumberOfInks, "NumberOfInks" ) );
	TagMap.insert( Tag_Pair( DotRange, "DotRange" ) );
	TagMap.insert( Tag_Pair( TargetPrinter, "TargetPrinter" ) );
	TagMap.insert( Tag_Pair( ExtraSamples, "ExtraSamples" ) );
	TagMap.insert( Tag_Pair( SampleFormat, "SampleFormat" ) );
	TagMap.insert( Tag_Pair( SMinSampleValue, "SMinSampleValue" ) );
	TagMap.insert( Tag_Pair( SMaxSampleValue, "SMaxSampleValue" ) );
	TagMap.insert( Tag_Pair( TransforRange, "TransforRange" ) );
	TagMap.insert( Tag_Pair( JPEGProc, "JPEGProc" ) );
	TagMap.insert( Tag_Pair( JPEGInterchangeFormat, "JPEGInterchangeFormat" ) );
	TagMap.insert( Tag_Pair( JPEGIngerchangeFormatLength, "JPEGIngerchangeFormatLength" ) );
	TagMap.insert( Tag_Pair( JPEGRestartInterval, "JPEGRestartInterval" ) );
	TagMap.insert( Tag_Pair( JPEGLosslessPredicators, "JPEGLosslessPredicators" ) );
	TagMap.insert( Tag_Pair( JPEGPointTransforms, "JPEGPointTransforms" ) );
	TagMap.insert( Tag_Pair( JPEGQTable, "JPEGQTable" ) );
	TagMap.insert( Tag_Pair( JPEGDCTable, "JPEGDCTable" ) );
	TagMap.insert( Tag_Pair( JPEGACTable, "JPEGACTable" ) );
	TagMap.insert( Tag_Pair( YCbCrCoefficients, "YCbCrCoefficients" ) );
	TagMap.insert( Tag_Pair( YCbCrSampling, "YCbCrSampling" ) );
	TagMap.insert( Tag_Pair( YCbCrPositioning, "YCbCrPositioning" ) );
	TagMap.insert( Tag_Pair( ReferenceBlackWhite, "ReferenceBlackWhite" ) );
	TagMap.insert( Tag_Pair( XML_Data, "XML_Data" ) );
	TagMap.insert( Tag_Pair( Exif_IFD, "Exif_IFD" ) );
	TagMap.insert( Tag_Pair( CopyRight, "CopyRight" ) );
	TagMap.insert( Tag_Pair( IPTC, "IPTC" ) );
	TagMap.insert( Tag_Pair( Photoshop, "Photoshop" ) );
	TagMap.insert( Tag_Pair( IccProfile, "IccProfile" ) );
}

char* GetStr(DWORD data) 
{
	map<TiffTagSignature, char*>::iterator Iter;
	TiffTagSignature sig = (TiffTagSignature)(0xFFFF & data);
	Iter = TagMap.find(sig);
	if(Iter != TagMap.end())
		return Iter->second;
	else
		return "UNKNOWN";
}
/***********************************************************************/
int DumpTiff(LPCSTR FileName)
{
	DWORD	TiffVersion;
	WORD	TagCount;
		
	if(FileName == NULL)
		return (int)FileOpenErr;
	FILE *File = fopen(FileName, "rb+");
	if(File == NULL)
	{
		cout << FileName<< "open error! \n" << endl;
		return (int)FileOpenErr;
	}
	fseek(File, 0, SEEK_SET);
	fread(&TiffVersion, 1, 4, File);

	if(TiffVersion != 0x002A4949)
	{//PC Version
		fclose(File);
		cout << "File version Err!" << endl;
		return (int)VersionErr;	
	}
	else
	{
		cout << "TIFF HEADER FOR " << FileName << endl;
		cout << "Version OK." << endl;
		cout << "****************************************" << endl;
	}

	cout.unsetf(ios_base::dec);
	cout.setf(ios_base::hex);
	cout.setf(ios_base::left);
	cout.setf(ios_base::showbase);
	int index = 4;

	int IFD_Offset;
	fread(&IFD_Offset, 1, 4, File);
	cout << setw(12) << index << "IFD Offset  : " << setw(8) << IFD_Offset << endl;
	index = IFD_Offset;
	//index += 4;

	fseek(File, IFD_Offset, SEEK_SET);
	fread(&TagCount, 1, 2, File);
	cout << setw(12) << index << "Entry Count : " << setw(8) << TagCount << endl;
	cout << "****************************************" << endl;
	index += 2;

	if(TagCount >= MAXTAG)
	{//Too many m_Tags
		fclose(File);
		return (int)TooManyTags;
	}

	int DataType[FieldTypeSize]={
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
	DWORD Temp[MAXTAG*3];
	fread(Temp, (size_t)TagCount*3, 4, File);
	
	for(int i = 0; i < TagCount*3; i+=3)
	{
		cout << setw(12) << index \
	//	<< setw(32) <<	GetStr((TiffTagSignature)Temp[i]) 
		<< setw(32) <<	GetStr(Temp[i]) \
		<< setw(8)  << (0xffff & Temp[i]) \
		<< setw(12) << TypeName[0XFFFF & (Temp[i]>>16)]
		<< setw(8) << Temp[i+1];
		int type = 0XFFFF & (Temp[i] >> 16);
		if((Temp[i+1] * DataType[type]) > 4)
			cout << "@";
		cout << setw(8) << Temp[i+2]
		<< endl << endl;
//		DumpTags(Temp[i], Temp[i+1], Temp[i+2], File);
		index += 12;
	}
	fclose(File);
	return (int)Tiff_OK;
}

/*
void CTiff::DumpTags(DWORD TypeSignature, DWORD n, DWORD value, FILE *File)
{
	TiffTag *tag = CreateTag(TypeSignature, n, value, File);
	cout << TypeSignature << n << value << endl;
}
*/

int _tmain(int argc, _TCHAR* argv[])
{
	InitialMap();
	CTiff tiff;
	DumpTiff((LPCSTR)argv[1]);
	tiff.ReadFile((LPCSTR)argv[1]);

	cout.unsetf(ios_base::dec);
	cout.setf(ios_base::dec);
	cout << "****************************************" << endl;
	cout << setw(20) << "ImageWidth " << tiff.GetTagValue(ImageWidth) << endl;
	cout << setw(20) << "ImageLength " << tiff.GetTagValue(ImageLength) << endl;
	cout << setw(20) << "Resolution " << tiff.GetTagValue(XResolution) << endl;
	cout << setw(20) << "SamplesPerPixel " << tiff.GetTagValue(SamplesPerPixel) << endl;
	cout << setw(20) << "BitsPerSample " << tiff.GetTagValue(BitsPerSample) <<  endl;
	cout << setw(20) << "StripByteCounts " << tiff.GetTagValue(StripByteCounts) <<  endl;
	
	return 0;
}

