# extension for executables
EXE :=

ifeq ($(OS),Windows_NT)
EXE := .exe
endif
ifeq ($(OS),os2)
EXE := .exe
endif

ifndef VERBOSE
  SILENT = @
endif
CC  = gcc
CXX = g++

MAKE_CC = $(CC)
MAKE_CXX = $(CXX)
ifeq (ios-arm64,$(LIBRETRO_OS))
  # For iOS, use the host (macOS) compiler since we want to build this on the host, not the target os
  MAKE_CC = cc
  MAKE_CXX = c++
endif


.PHONY: all clean

all : m68kmake$(EXE) m68kops.cpp

clean:
	@echo Cleaning...
	-@rm -f m68kmake$(EXE)
	-@rm -f m68kmake.o
	-@rm -f m68kops.*

m68kmake.o: m68kmake.cpp
	$(SILENT) $(MAKE_CC) -x c++ -std=c++11 -o "$@" -c "$<"

m68kmake$(EXE) : m68kmake.o
	@echo Linking $@...
	$(SILENT) $(MAKE_CXX) -lstdc++ $^ -o $@

m68kops.cpp: m68kmake$(EXE)     m68k_in.cpp
	@echo Generating M68K source files...
	$(SILENT) ./m68kmake$(EXE) . m68k_in.cpp

