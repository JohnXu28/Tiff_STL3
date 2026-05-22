include ../../Makefile.inc
LIB_DIR  := $(CURDIR)/../../Lib

Module = Tiff_STL3
CPPS = \
Tiff_STL3.cpp \
LZW_Perplexity.cpp \

#OBJS : replace *.cpp to *.o
#OBJS = $(patsubst %.cpp,%.o,$(SRCS))
OBJS = $(CPPS:.cpp=.o)
DEPS = $(CPPS:.cpp=.d)

#---------------------------------------------------------
# Default target
all: test

#---------------------------------------------------------
# Test build
test: CXXFLAGS += -I../..
test: $(OBJS)
	@echo "======= $(Module): TEST mode ======="
	$(CP) *.o $(LIB_DIR)

#---------------------------------------------------------
# Release build
release: CXXFLAGS += -I../..
release: clean $(OBJS)
	@echo "======= $(Module): RELEASE mode ======="
	$(CP) *.o $(LIB_DIR)

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