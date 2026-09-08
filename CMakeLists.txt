cmake_minimum_required(VERSION 3.14)

#Declare a project only when built standalone (not via add_subdirectory).
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(Tiff_STL4 LANGUAGES CXX)
endif()

add_library(obj_Tiff_STL4 OBJECT
    Src/Tiff_Tags.cpp
    Src/Tiff_RW.cpp
    Src/Tiff_LZW.cpp
    Src/CTiff.cpp
    Src/G3G4.cpp
    Src/LZW_Integrated.cpp
)

target_include_directories(obj_Tiff_STL4 PRIVATE
    Include
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

#Tests need the superproject layout (obj_SysInfo target, SysInfo headers).
#Standalone builds of this module skip them unless explicitly enabled.
option(TIFF_BUILD_TESTS "Build TiffTest programs" OFF)
if(TIFF_BUILD_TESTS)
    add_subdirectory(TiffTest)
endif()
