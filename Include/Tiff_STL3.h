/////////////////////////////////////////////////////////////////////x
// Tiff_STL3.h : Compatibility shim.
//
// The Tiff module has been upgraded and renamed to Tiff_STL4
// (namespace AV_Tiff_STL4, header Tiff_STL4.h). This shim keeps the
// legacy Tiff_STL3 include path and the AV_Tiff_STL3 namespace working
// for existing consumers. New code should include Tiff_STL4.h and use
// AV_Tiff_STL4 directly.
//////////////////////////////////////////////////////////////////////x
#pragma once
#include "Tiff_STL4.h"

namespace AV_Tiff_STL3 {
	//Import everything for unqualified lookup from legacy code.
	using namespace AV_Tiff_STL4;

	//Explicit aliases so qualified names (AV_Tiff_STL3::X) keep working
	//(using directives do not inject names for qualified lookup).
	using TiffTagSignature = AV_Tiff_STL4::TiffTagSignature;
	using FieldType        = AV_Tiff_STL4::FieldType;
	using Tiff_Err         = AV_Tiff_STL4::Tiff_Err;
	using TiffTag          = AV_Tiff_STL4::TiffTag;
	using StripOffsetsTag  = AV_Tiff_STL4::StripOffsetsTag;
	using BitsPerSampleTag = AV_Tiff_STL4::BitsPerSampleTag;
	using ResolutionTag    = AV_Tiff_STL4::ResolutionTag;
	using Exif_IFD_Tag     = AV_Tiff_STL4::Exif_IFD_Tag;
	using TiffTagPtr       = AV_Tiff_STL4::TiffTagPtr;
	using TagListItem      = AV_Tiff_STL4::TagListItem;
	using TagList          = AV_Tiff_STL4::TagList;
	using TiffTag_iter     = AV_Tiff_STL4::TiffTag_iter;
	using IFD_STRUCTURE    = AV_Tiff_STL4::IFD_STRUCTURE;
	using Tiff             = AV_Tiff_STL4::Tiff;
	using CTiff            = AV_Tiff_STL4::CTiff;
	using AV_Tiff_STL4::SwapDWORD;
	using AV_Tiff_STL4::SwapWORD;
}
