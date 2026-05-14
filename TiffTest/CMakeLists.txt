add_executable(Main
#	DumpTiff.cpp
#	Halftone_gemini.cpp
	stdafx.cpp
	test.cpp
#	Tiff_Append.cpp
    TiffTest.cpp
#    To_Halftone_1bit_acc.cpp
    Utility.cpp
    
)

target_link_libraries(Main
    PRIVATE
        Lib_Tiff_STL3::Lib_Tiff_STL3
)