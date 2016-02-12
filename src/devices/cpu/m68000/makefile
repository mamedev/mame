# extension for executables
EXE :=

ifeq ($(OS),Windows_NT)
EXE := .exe
endif
ifeq ($(OS),os2)
EXE := .exe
endif

ifndef verbose
  SILENT = @
endif
CC  = gcc
CXX = g++

.PHONY: all clean

all : m68kmake$(EXE) m68kops.cpp

clean:
	@echo Cleaning...
	-@rm -f m68kmake$(EXE)
	-@rm -f m68kmake.o
	-@rm -f m68kops.*
	
m68kmake.o: m68kmake.cpp
	$(SILENT) $(CC) -x c++ -std=c++11 -o "$@" -c "$<"

m68kmake$(EXE) : m68kmake.o
	@echo Linking $@...
	$(SILENT) $(CXX) -lstdc++ $^ -o $@

m68kops.cpp: m68kmake$(EXE) 	m68k_in.cpp
	@echo Generating M68K source files...
	$(SILENT) ./m68kmake$(EXE) . m68k_in.cpp

