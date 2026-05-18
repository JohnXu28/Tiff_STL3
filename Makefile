# Makefile to build all programs in all subdirectories
#
# DIRS is a list of all subdirectories containing makefiles
# (The library directory is first so that the library gets built first)
#

DIRS = 	Src \
		TiffTest \
		

BUILD_DIRS = ${DIRS} ${CDIRS}

# Per-directory extra make flags (e.g. -f <alt-makefile>)
Src_mflags      = -f Makefile_Test
TiffTest_mflags = -f Makefile_Test

# Dummy targets for building and clobbering everything in all subdirectories

.PHONY:all
all:
	@echo "*****************************Tiff_STL3************************"
	@ for dir in ${BUILD_DIRS}; do \
		extra=""; \
		[ "$$dir" = "Src" -o "$$dir" = "TiffTest" ] && extra="$(Src_mflags)"; \
		(cd "$$dir"; ${MAKE} $$extra); \
	done

.PHONY:allgen
allgen:
	@echo "*****************************Tiff_STL3 release************************"
	@ for dir in ${BUILD_DIRS}; do \
		extra=""; \
		[ "$$dir" = "Src" -o "$$dir" = "TiffTest" ] && extra="$(Src_mflags)"; \
		(cd "$$dir"; ${MAKE} $$extra allgen); \
	done

.PHONY:debug
debug:
	@echo "*****************************Tiff_STL3 Debug************************"
	@ for dir in ${BUILD_DIRS}; do \
		extra=""; \
		[ "$$dir" = "Src" -o "$$dir" = "TiffTest" ] && extra="$(Src_mflags)"; \
		(cd "$$dir"; ${MAKE} $$extra debug); \
	done

.PHONY:release
release:
	@echo "*****************************AVCMP Release************************"
	@ for dir in ${BUILD_DIRS}; do \
		extra=""; \
		[ "$$dir" = "Src" -o "$$dir" = "TiffTest" ] && extra="$(Src_mflags)"; \
		(cd "$$dir"; ${MAKE} $$extra release); \
	done

clean:
	@echo "*****************************AVCMP Clean************************"
	@ for dir in ${BUILD_DIRS}; do \
		extra=""; \
		[ "$$dir" = "Src" -o "$$dir" = "TiffTest" ] && extra="$(Src_mflags)"; \
		(cd "$$dir"; ${MAKE} $$extra clean); \
	done
