# Makefile.inc - common definitions used by all makefiles
ROOT_DIR := $(CURDIR)
LIB_DIR  := $(ROOT_DIR)/../TiffTest
#$(shell mkdir -p $(LIB_DIR))

#COMPILER 1:ARM 2:ARDROID, 3:OSX(APPLE), 4:APPLE....DEFAULT:native
COMPILER = 0
TURN_ON_OPENMP = 0
RELEASE = 1
VPATH = $(shell pwd)

# default C++ flag
CXXFLAGS = -fPIC -Wall -finline-functions -Wno-reorder -Wno-unused-result -Wno-comment -Wno-unused-variable -Wno-unused-but-set-variable
#CXXFLAGS += -DSYS_INFO 
#No STL Vector
CXXFLAGS += -DFIXED_VECTOR
#For Arm project
#CXXFLAGS += -DNO_SMART_POINTER
######################################################################################
ifeq ($(COMPILER), 1)
#LINUX
#CROSS_COMPILE = arm-none-eabi-g
#CROSS_COMPILE = mingw32-
#CROSS_COMPILE = mips64el-linux-gnuabi64-
#CROSS_COMPILE = loongarch64-linux-gnuabi64-
#TOOL_CHAIN = /opt/gcc-arm-none-eabi-6_2-2016q4/bin/
#CROSS_COMPILE = arm-linux-gnueabihf-

TOOL_CHAIN = /opt/gcc-linaro-4.9.4-2017.01-x86_64_arm-linux-gnueabihf/bin/
CROSS_COMPILE = arm-linux-gnueabihf-

CXX		= $(TOOL_CHAIN)$(CROSS_COMPILE)g++
CXXFLAGS += -static-libgcc -static-libstdc++ 
CXXFLAGS += -std=c++14
#IMPL_CPPFLAGS+=-v -g


AR      = $(TOOL_CHAIN)$(CROSS_COMPILE)ar
STRIP   = $(TOOL_CHAIN)$(CROSS_COMPILE)strip
######################################################################################

else ifeq ($(COMPILER), 2)
TOOL_CHAIN = /opt/android-ndk-r26d/toolchains/llvm/prebuilt/linux-x86_64/bin/
#CROSS_COMPILE = armv7a-linux-androideabi21-clan
#CROSS_COMPILE = aarch64-linux-android21-clan
#CROSS_COMPILE = i686-linux-android21-clan
CROSS_COMPILE = x86_64-linux-android21-clan
CXX	= $(TOOL_CHAIN)$(CROSS_COMPILE)g++

#IMPL_CPPFLAGS +=-static-libgcc -static-libstdc++
CXXFLAGS += -Wno-missing-braces -Wno-overloaded-virtual
#IMPL_CPPFLAGS+=-v -g

CXXFLAGS += -std=c++14
CXXFLAGS += -DANDROID
AR      = $(TOOL_CHAIN)$(CROSS_COMPILE)ar
STRIP   = $(TOOL_CHAIN)$(CROSS_COMPILE)strip

######################################################################################
else ifeq ($(COMPILER), 3)
#OSX (APPLE)
TOOL_CHAIN = /opt/osxcross/target/bin/
CROSS_COMPILE = x86_64-apple-darwin23-
#CROSS_COMPILE = o64-

CXX      = $(TOOL_CHAIN)$(CROSS_COMPILE)clang++
CXXFLAGS += -Wno-missing-braces -Wno-overloaded-virtual
#CXXFLAGS += -std=c++23
CXXFLAGS += -DAPPLE -mmacosx-version-min=10.10
CXXFLAGS += -arch x86_64 -arch arm64 -dynamiclib

#IMPL_CPPFLAGS += -arch x86_64
AR      = $(TOOL_CHAIN)llvm-ar
STRIP   = $(TOOL_CHAIN)llvm-strip

######################################################################################
else ifeq ($(COMPILER), 4)
#APPLE
CXX      = $(TOOL_CHAIN)$(CROSS_COMPILE)clang++
CXXFLAGS += -Wno-missing-braces -Wno-overloaded-virtual
#CXXFLAGS += -std=c++23
CXXFLAGS += -DAPPLE
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S), Darwin)
UNAME_M := $(shell uname -m)
CXXFLAGS += -arch $(UNAME_M) -dynamiclib
endif


else
#CROSS_COMPILE = x86_64-linux-gnu-
#CROSS_COMPILE = i686-linux-gnu-
#TOOL_CHAIN = c:\Apps\MinGW\bin\

CXX		= $(CROSS_COMPILE)g++
CXXFLAGS +=-static-libgcc -static-libstdc++ -std=c++11


AR      = $(TOOL_CHAIN)$(CROSS_COMPILE)ar
STRIP   = $(TOOL_CHAIN)$(CROSS_COMPILE)strip
endif

#########################################################################
RM 		= rm -f
AR		= ar crsv
MV		= mv
CP		= cp

CPPFLAGS = ${IMPL_CPPFLAGS}

#########################################################################
ifeq ($(TURN_ON_OPENMP), 1)
	#LDLIBS = -fopenmp -Wl,-rpath=/usr/share/avision/lib,--enable-new-dtags
	#LDLIBS = -fopenmp -Wl,-rpath=/usr/share/AVCMS,--enable-new-dtags
	CXXFLAGS += -DTURN_ON_OPENMP 
	CXXFLAGS += -fopenmp
	#LDLIBS = -fopenmp -lomp
	LDLIBS = -fopenmp
else
	#LDLIBS = -Wl,-rpath=/usr/share/avision/lib,--enable-new-dtags		
	#LDLIBS = -Wl,-rpath='$$ORIGIN:$$ORIGIN/lib' -L'$$ORIGIN:$$ORIGIN/lib'
	#LDLIBS = -Wl,-rpath=/usr/share/AVCMS,--enable-new-dtags
	LDLIBS =  
endif

ifeq ($(COMPILER), 3)
LDLIBS += -arch x86_64 -arch arm64 -dynamiclib
#LDLIBS += -arch x86_64  -dynamiclib

else ifeq ($(COMPILER), 4)
ifeq ($(UNAME_S), Darwin)
LDLIBS += -arch $(UNAME_M) -dynamiclib
endif
#LDLIBS += -arch x86_64  -dynamiclib
#LDLIBS += -shared

else
LDLIBS +=

endif

ifeq ($(RELEASE), 1)
	CXXFLAGS += -O2
else
	CXXFLAGS += -O0 -D_DEBUG
endif