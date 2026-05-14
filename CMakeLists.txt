cmake_minimum_required(VERSION 3.20)

project(Tiff_STL3)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

message("")
message("=====================================COMPILE SCANNER=========================================")
message("       CMAKE_SYSTEM_PROCESSOR:   ${CMAKE_SYSTEM_PROCESSOR}")
message("       CMAKE_CROSSCOMPILING:   ${CMAKE_CROSSCOMPILING}")
message("       CMAKE_C_COMPILER:   ${CMAKE_C_COMPILER}")
message("       CMAKE_CXX_COMPILER:   ${CMAKE_CXX_COMPILER}")
message("       TOOLCHAIN_PATH:     ${TOOLCHAIN_PATH}")
message("       CROSS_PREFIX:   ${CROSS_PREFIX}")
message("       CMAKE_AR:   ${CMAKE_AR}")
message("============================================================================================")
message("")

# ===== Output =====
set(OUTPUT_DIR ${CMAKE_SOURCE_DIR}/Build)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR})

# ===== Modules =====
add_subdirectory(src)


# ===== Apps =====
add_subdirectory(TiffTest)