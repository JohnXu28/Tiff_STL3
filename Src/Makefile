include ../../Makefile.inc
LIB_NAME = Tiff_STL3

CPPS = 	Tiff_STL3.cpp 
		
CPPFLAGS += -fPIC

#================================================================
allgen : ${LIB_NAME}

${LIB_NAME} : Tiff_STL3	
	${CC} ${CPPFLAGS} -O2 ${CPPS}
	${CP} *.o ../../Lib
	
#================================================================
release : ${LIB_NAME}

${LIB_NAME} : Tiff_STL3	
	${CC} ${CPPFLAGS} -O2 ${CPPS}
	${CP} *.o ../../Lib

#================================================================	
debug : ${LIB_NAME}

${LIB_NAME} : Tiff_STL3	
	${CC} ${CPPFLAGS} -O0 ${CPPS}
	${CP} *.o ../../Lib

#================================================================		
clean :
	${RM} *.o

