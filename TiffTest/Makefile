# TiffTest - Linux build for the Tiff module test application.
#
# Links against libTiff_STL3.a produced by Src/Makefile_Test
# (the top Makefile builds Src first, then this directory).
#
# Usage:
#   make                # build $(BUILD)/TiffTest
#   make test           # build + run (reads ../TestImg)
#   make clean
#
# Optional input directory override: ./$(BUILD)/TiffTest <imgdir>

include ../Makefile.inc

BUILD = build_test

BIN = $(BUILD)/TiffTest
CPPS = TiffTest.cpp \
       BMP.cpp \
       test.cpp \
       Tiff_Append.cpp \
       Utility.cpp

OBJS = $(addprefix $(BUILD)/,$(notdir $(CPPS:.cpp=.o)))
DEPS = $(OBJS:.o=.d)

SYS_INCLUDE = -I../Src -I../Include -I../..

CXXFLAGS += $(SYS_INCLUDE)

#LDLIBS += -lAVCMS -L.
LDLIBS += -ldl -L. -lTiff_STL3

.PHONY: all release test clean

all: $(BIN)

test: $(BIN)
	./$(BIN) ../TestImg

#================================================================
$(BIN): $(OBJS) libTiff_STL3.a | $(BUILD)
	@echo "*****************************TiffTest:link************************"
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDLIBS)

$(BUILD)/%.o: %.cpp | $(BUILD)
	$(CXX) $(CXXFLAGS) -MMD -MP -c -o $@ $<

$(BUILD):
	mkdir -p $@

#================================================================
release: $(BIN)
	@echo "*****************************TiffTest:Release************************"

#================================================================
clean:
	@echo "*****************************TiffTest:clean************************"
	$(RM) -r $(BUILD)
