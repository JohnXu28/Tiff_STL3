include ../../Makefile.inc

CPPS = Tiff_STL3.cpp 
OBJS = Tiff_STL3.o
all: $(OBJS)
		
CPPFLAGS += -fPIC

CPPFLAGS += $(SYS_INCLUDE)

#================================================================
.PHONY:test
%.o : %.cpp
	${CXX} ${CPPFLAGS} -o $@ $<
	${CP} *.o ../../Lib
	@echo "*****************************Tiff_STL3:test************************"

#================================================================
.PHONY:release
release:
	${CXX} ${CPPFLAGS} ${CPPS}
	${CP} *.o ../../Lib
	@echo "*****************************Tiff_STL3:Release************************"
	
#================================================================
clean :
	${RM} *.o
	@echo "*****************************Tiff_STL3:Clean************************"