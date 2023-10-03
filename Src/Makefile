include ../../Makefile.inc

CPPS = Tiff_STL3.cpp 
OBJS = Tiff_STL3.o
all: $(OBJS)
		
CPPFLAGS += -fPIC

#================================================================
.PHONY:release
%.o : %.cpp
	${CC} ${CPPFLAGS} -o $@ $<
	${CP} *.o ../../Lib
	@echo "*****************************Tiff_STL3:Release************************"
	
#================================================================
clean :
	${RM} *.o

