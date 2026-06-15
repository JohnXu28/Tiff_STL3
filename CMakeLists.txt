add_library(obj_Tiff_STL3 OBJECT
    src/Tiff_STL3.cpp
    src/LZW.cpp
    src/LZW_Perplexity.cpp
)

target_include_directories(obj_Tiff_STL3 PRIVATE
    Include
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)
