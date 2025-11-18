include ../../Makefile.inc

Module = Tiff_STL3
CPPS = \
Tiff_STL3.cpp \

#OBJS : replace *.cpp to *.o
#OBJS = $(patsubst %.cpp,%.o,$(SRCS))
OBJS=$(CPPS:.cpp=.o)

#---------------------------------------------------------
# Default target
all: test

#---------------------------------------------------------
# Test build
test: CXXFLAGS += 
test: $(OBJS)
	@echo "======= $(Module): TEST mode ======="
	$(CP) *.o ../../Lib
	
#---------------------------------------------------------
# Release build
release: CXXFLAGS += 
release: clean $(OBJS)
	@echo "======= $(Module): RELEASE mode ======="
	$(CP) *.o ../../Lib	

#---------------------------------------------------------
# Common compile rule
%.o : %.cpp	
	$(CXX) $(CXXFLAGS) -c -o $@ $<
	
#---------------------------------------------------------
clean:
	@echo "Cleaning object files..." $(Module)
	$(RM) *.o
