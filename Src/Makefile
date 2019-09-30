include ../../Makefile.inc
LIB_NAME = Tiff_STL3

CPPS = 	Tiff_STL3.cpp 
		
CPPFLAGS += -fPIC

#================================================================
.PHONY:release
release :
	${CC} ${CPPFLAGS} -O2 ${CPPS}
	${CP} *.o ../../Lib

#================================================================	
.PHONY:debug
debug :
	${CC} ${CPPFLAGS} -O0 ${CPPS}
	${CP} *.o ../../Lib

#================================================================		
clean :
	${RM} *.o

