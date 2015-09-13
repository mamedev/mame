# extension for executables
EXE :=

ifeq ($(OS),Windows_NT)
EXE := .exe
endif
ifeq ($(OS),os2)
EXE := .exe
endif

.PHONY: all clean

all : m68kmake$(EXE) m68kops.c clean

clean:
	@echo Cleaning...
	-@rm -f m68kmake$(EXE)
	-@rm -f m68kmake.o
	
m68kmake.o: m68kmake.c
	@echo $(notdir $<)
	@gcc -x c++ -std=gnu++98 -o "$@" -c "$<"

m68kmake$(EXE) : m68kmake.o
	@echo Linking $@...
	@g++ -lstdc++ $^ -o $@

m68kops.c: m68kmake$(EXE) 	m68k_in.c
	@echo Generating M68K source files...
	@m68kmake$(EXE) . m68k_in.c

