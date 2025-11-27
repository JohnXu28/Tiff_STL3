include ../../Makefile.inc

Module = Tiff_STL3
CPPS = \
Tiff_STL3.cpp \

#OBJS : replace *.cpp to *.o
#OBJS = $(patsubst %.cpp,%.o,$(SRCS))
OBJS = $(CPPS:.cpp=.o)
DEPS = $(CPPS:.cpp=.d)

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
	$(CXX) $(CXXFLAGS) -MMD -MP -c -o $@ $< 

#---------------------------------------------------------
-include $(DEPS)

#---------------------------------------------------------
clean:
	@echo "Cleaning object files..." $(Module)
	$(RM) *.o *.d