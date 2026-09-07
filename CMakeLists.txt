add_library(obj_Tiff_STL4 OBJECT
    src/Tiff_Tags.cpp
    src/Tiff_RW.cpp
    src/Tiff_LZW.cpp
    src/CTiff.cpp
    src/G3G4.cpp
    src/LZW_Integrated.cpp
)

target_include_directories(obj_Tiff_STL4 PRIVATE
    Include
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

add_subdirectory(TiffTest)
