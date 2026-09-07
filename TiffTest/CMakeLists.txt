add_executable(Tiff_RefactorGuard
	TiffRefactorGuard.cpp
)

target_compile_definitions(Tiff_RefactorGuard PRIVATE SYS_INFO FIXED_VECTOR)

target_include_directories(Tiff_RefactorGuard PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/../Include
	${CMAKE_CURRENT_SOURCE_DIR}/../..
	${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(Tiff_RefactorGuard
	PRIVATE
		obj_Tiff_STL4
		obj_SysInfo
)

#Legacy test main, needs Lib_Tiff_STL3 target which no longer exists.
#add_executable(Main
#	stdafx.cpp
#	test.cpp
#	TiffTest.cpp
#	Utility.cpp
#)
#target_link_libraries(Main PRIVATE Lib_Tiff_STL3::Lib_Tiff_STL3)
