###########################################################################
#
#   makefile
#
#   Core makefile for building MAME and derivatives
#
#   Copyright (c) Nicola Salmoria and the MAME Team.
#   Visit http://mamedev.org for licensing and usage restrictions.
#
###########################################################################

SRC = ..
VSBUILD = $(SRC)/buildVS
DOC = $(SRC)/documentation
TIDY_DB = ../compile_commands.json

TIDY_FLAGSX = -checks=*,-google*,-hicpp*,-readability*,-fuchsia*,cert-*,-android-*,
TIDY_FLAGSX += -llvm-header-guard,-cppcoreguidelines-pro-type-reinterpret-cast,
TIDY_FLAGSX += -cppcoreguidelines-pro-bounds-pointer-arithmetic,-cppcoreguidelines-owning-memory, 
TIDY_FLAGSX += -modernize-use-default-member-init,-cppcoreguidelines-pro-bounds-constant-array-index,
TIDY_FLAGSX += -modernize-pass-by-value,-cppcoreguidelines-pro-type-static-cast-downcast,
TIDY_FLAGSX += -cppcoreguidelines-avoid-magic-numbers,
TIDY_FLAGSX += -cppcoreguidelines-macro-usage,
TIDY_FLAGSX += -cppcoreguidelines-non-private-member-variables-in-classes,-misc-non-private-member-variables-in-classes,
TIDY_FLAGSX += -bugprone-macro-parentheses,-misc-macro-parentheses

space := 
space += 
TIDY_FLAGS = $(subst $(space),,$(TIDY_FLAGSX))

#TIDY_FLAGS = -checks=llvm-include-order,llvm-namespace-comment,modernize-use-override,modernize-use-using -fix
#TIDY_FLAGS = -checks=llvm-include-order -fix
#TIDY_FLAGS = -checks=llvm-namespace-comment -fix
#TIDY_FLAGS = -checks=modernize-use-override -fix
#TIDY_FLAGS = -checks=modernize-use-using -fix

ifeq ($(subst Windows_NT,windows,$(OS)),windows)
OBJ = obj/mingw
else
OBJ = obj/nix
endif

#-fuse-ld=gold -Wpedantic -march=native -march=native

# LTO = -flto=4  -fuse-linker-plugin -flto-partition=balanced -Wodr

CFLAGS =  $(LTO) -g -O3 -std=c++11 -I$(CURDIR)/.. -I$(CURDIR)/../.. $(CEXTRAFLAGS)
LDFLAGS = $(LTO) -g -O3 -std=c++11 $(LDEXTRAFLAGS)
LIBS = -lpthread -ldl

CC = g++
LD = @g++
MD = @mkdir
RM = @rm
DOXYGEN = @doxygen
CLANG_TIDY = clang-tidy-9

TARGETS = nltool nlwav

NLOBJ = $(OBJ)
POBJ = $(OBJ)/plib

OBJDIRS = $(OBJ) \
			$(OBJ)/analog \
			$(OBJ)/solver \
			$(OBJ)/devices \
			$(OBJ)/plib \
			$(OBJ)/devices \
			$(OBJ)/macro \
			$(OBJ)/tools \
			$(OBJ)/prg \


OBJS = $(POBJS) $(NLOBJS)

POBJS := \
	$(POBJ)/pstring.o \
	$(POBJ)/pchrono.o \
	$(POBJ)/pdynlib.o \
	$(POBJ)/pexception.o \
	$(POBJ)/pfunction.o \
	$(POBJ)/pfmtlog.o \
	$(POBJ)/poptions.o \
	$(POBJ)/pparser.o \
	$(POBJ)/pstate.o \
	$(POBJ)/pstream.o \
	$(POBJ)/putil.o \

PMAIN := $(POBJ)/pmain.o

NLOBJS := \
	$(NLOBJ)/nl_base.o \
	$(NLOBJ)/nl_parser.o \
	$(NLOBJ)/nl_setup.o \
	$(NLOBJ)/nl_factory.o \
	$(NLOBJ)/analog/nld_bjt.o \
	$(NLOBJ)/analog/nlid_fourterm.o \
	$(NLOBJ)/analog/nld_switches.o \
	$(NLOBJ)/analog/nlid_twoterm.o \
	$(NLOBJ)/analog/nld_opamps.o \
	$(NLOBJ)/devices/nld_2102A.o \
	$(NLOBJ)/devices/nld_2716.o \
	$(NLOBJ)/devices/nld_tms4800.o \
	$(NLOBJ)/devices/nld_4020.o \
	$(NLOBJ)/devices/nld_4066.o \
	$(NLOBJ)/devices/nld_4316.o \
	$(NLOBJ)/devices/nld_7448.o \
	$(NLOBJ)/devices/nld_7450.o \
	$(NLOBJ)/devices/nld_7473.o \
	$(NLOBJ)/devices/nld_7474.o \
	$(NLOBJ)/devices/nld_7475.o \
	$(NLOBJ)/devices/nld_7483.o \
	$(NLOBJ)/devices/nld_7485.o \
	$(NLOBJ)/devices/nld_7490.o \
	$(NLOBJ)/devices/nld_7493.o \
	$(NLOBJ)/devices/nld_7497.o \
	$(NLOBJ)/devices/nld_74107.o \
	$(NLOBJ)/devices/nld_74123.o \
	$(NLOBJ)/devices/nld_74153.o \
	$(NLOBJ)/devices/nld_74161.o \
	$(NLOBJ)/devices/nld_74164.o \
	$(NLOBJ)/devices/nld_74165.o \
	$(NLOBJ)/devices/nld_74166.o \
	$(NLOBJ)/devices/nld_74174.o \
	$(NLOBJ)/devices/nld_74175.o \
	$(NLOBJ)/devices/nld_74192.o \
	$(NLOBJ)/devices/nld_74193.o \
	$(NLOBJ)/devices/nld_74194.o \
	$(NLOBJ)/devices/nld_74365.o \
	$(NLOBJ)/devices/nld_74ls629.o \
	$(NLOBJ)/devices/nld_82S16.o \
	$(NLOBJ)/devices/nld_82S115.o \
	$(NLOBJ)/devices/nld_82S123.o \
	$(NLOBJ)/devices/nld_82S126.o \
	$(NLOBJ)/devices/nld_9310.o \
	$(NLOBJ)/devices/nld_9316.o \
	$(NLOBJ)/devices/nld_9322.o \
	$(NLOBJ)/devices/nld_am2847.o \
	$(NLOBJ)/devices/nld_dm9314.o \
	$(NLOBJ)/devices/nld_dm9334.o \
	$(NLOBJ)/devices/nld_mm5837.o \
	$(NLOBJ)/devices/nld_ne555.o \
	$(NLOBJ)/devices/nld_r2r_dac.o \
	$(NLOBJ)/devices/nld_tristate.o \
	$(NLOBJ)/devices/nld_schmitt.o \
	$(NLOBJ)/devices/nld_legacy.o \
	$(NLOBJ)/devices/net_lib.o \
	$(NLOBJ)/devices/nld_log.o \
	$(NLOBJ)/devices/nlid_proxy.o \
	$(NLOBJ)/devices/nld_system.o \
	$(NLOBJ)/devices/nlid_truthtable.o \
	$(NLOBJ)/macro/nlm_base.o \
	$(NLOBJ)/macro/nlm_cd4xxx.o \
	$(NLOBJ)/macro/nlm_opamp.o \
	$(NLOBJ)/macro/nlm_other.o \
	$(NLOBJ)/macro/nlm_ttl74xx.o \
	$(NLOBJ)/solver/nld_solver.o \
	$(NLOBJ)/solver/nld_matrix_solver.o \
	$(NLOBJ)/tools/nl_convert.o \
	
VSBUILDS = \
	$(VSBUILD/netlistlib.vcxproj) \
	$(VSBUILD/netlistlib.vcxproj.user \
	$(VSBUILD/nltool.vcxproj \
	$(VSBUILD/netlistlib.vcxproj.filters  \
	$(VSBUILD/nltool.vcxproj.filters \
	$(VSBUILD/netlist.sln \

DOCS = \
	doxygen.conf \
	$(DOC)/doc.css \
	$(DOC)/mainpage.dox.h \
	$(DOC)/primer_1.dox.h \
	$(DOC)/structure.dox.h \
	$(DOC)/test1-50r.svg \

ALL_OBJS = $(OBJS) $(PMAIN) $(NLOBJ)/prg/nltool.o $(NLOBJ)/prg/nlwav.o

ALL_TIDY_FILES = $(ALL_OBJS:.o=.json)
SOURCES = $(patsubst $(OBJ)%, $(SRC)%, $(ALL_OBJS:.o=.cpp))
ALLFILES = $(SOURCES) $(VSBUILDS) $(DOCS)

MAKEFILE_TARGETS_WITHOUT_INCLUDE := clean doc clang mingw nvcc


# git archive HEAD --prefix=project-name-version/ \
#     --format=zip -o project-name-version.zip

#-------------------------------------------------
# all
#-------------------------------------------------

all:    maketree depend $(TARGETS)

#-------------------------------------------------
# clean
#-------------------------------------------------

clean:
	$(RM) -rf $(OBJS) $(TARGETS) $(OBJ)/prg/nltool.o $(OBJ)/prg/nlwav.o .depend doxy/*

#-------------------------------------------------
# nltool
#-------------------------------------------------

nltool: $(OBJ)/prg/nltool.o $(PMAIN) $(OBJS)
	@echo Linking $@...
	$(LD) -o $@ $(LDFLAGS) $^ $(LIBS)

nlwav: $(OBJ)/prg/nlwav.o $(PMAIN) $(OBJS)
	@echo Linking $@...
	$(LD) -o $@ $(LDFLAGS) $^ $(LIBS)

#-------------------------------------------------
# directories
#-------------------------------------------------

$(sort $(OBJDIRS)):
	$(MD) -p $@

maketree: $(sort $(OBJDIRS))

#-------------------------------------------------
# Special targets
#-------------------------------------------------

.PHONY: clang clang-5 mingw doc native

native: 
	$(MAKE) CEXTRAFLAGS="-march=native -msse4.2 -Wall -Wpedantic -Wsign-compare -Wextra "

clang:
	$(MAKE) CC=clang++-9 LD=clang++-9 CEXTRAFLAGS="-march=native -Weverything -Werror -Wno-padded -Wno-weak-vtables -Wno-unused-template -Wno-missing-variable-declarations -Wno-float-equal -Wconversion -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-format-nonliteral  -Wno-exit-time-destructors"

clang-5:
	$(MAKE) CC=clang++-5.0 LD=clang++-5.0 CEXTRAFLAGS="-march=native -Weverything -Werror -Wno-inconsistent-missing-destructor-override -Wno-unreachable-code -Wno-padded -Wno-weak-vtables -Wno-missing-variable-declarations -Wconversion -Wno-c++98-compat -Wno-float-equal -Wno-global-constructors -Wno-c++98-compat-pedantic -Wno-format-nonliteral -Wno-weak-template-vtables -Wno-exit-time-destructors"

nvcc:
	$(MAKE) CC=/usr/local/cuda-9.0/bin/nvcc LD=/usr/local/cuda-9.2/bin/nvcc \
		CEXTRAFLAGS="-x cu  -DNVCCBUILD=1 --expt-extended-lambda --expt-relaxed-constexpr --default-stream per-thread --restrict"

tidy_db: compile_commands_prefix $(ALL_TIDY_FILES) compile_commands_postfix

#
# -Wno-c++11-narrowing : seems a bit broken
# Mostly done: -Wno-weak-vtables -Wno-cast-align
# FIXME:  -Wno-weak-vtables -Wno-missing-variable-declarations -Wno-conversion -Wno-exit-time-destructors
# FIXME: -Winconsistent-missing-destructor-override : c++ community has diverging opinions on this https://github.com/isocpp/CppCoreGuidelines/issues/721
# FIXME: -Wunreachable-code : False warnings, this a documented clang bug: https://llvm.org/bugs/show_bug.cgi?id=28994
# -Wweak-template-vtables 
#
# These have to be given:
# -Wno-missing-variable-declarations : Device factory code, on purpose 

mingw:
	$(MAKE) CEXTRAFLAGS="-DUNICODE -D_UNICODE -D_WIN32_WINNT=0x0501 -DWIN32_LEAN_AND_MEAN" \
		LDEXTRAFLAGS="-Wl,--subsystem,console -municode" LIBS= MD=@mkdir.exe SHELL=sh.exe

#
# FIXME: Unicode
#     CEXTRAFLAGS = -DUNICODE -D_UNICODE -municode
#     LDEXTRAFLAGS = -municode
#


doc: nltool
	./nltool -c docheader > ../documentation/devsyn.dox.h
	$(DOXYGEN) doxygen.conf

#-------------------------------------------------
# depends
#-------------------------------------------------

.depend: $(SOURCES)
	@echo creating .depend
	@rm -f ./.depend
	@for i in $(SOURCES); do \
		$(CC) $(CFLAGS) -MM $$i -MT `echo $$i | sed -e 's+$(SRC)+$(OBJ)+' -e 's+.cpp+.o+' ` >> ./.depend; \
	done

depend: .depend

# Include only if the goal needs it
ifeq ($(filter $(MAKECMDGOALS),$(MAKEFILE_TARGETS_WITHOUT_INCLUDE)),)
-include .depend
endif

#-------------------------------------------------
# clang tidy
#-------------------------------------------------
tidy: tidy_db
	@echo running tidy
	@for i in $(SOURCES); do \
		$(CLANG_TIDY) $$i $(TIDY_FLAGS) -header-filter=.*; \
	done

tidy_db: compile_commands_prefix $(ALL_TIDY_FILES) compile_commands_postfix


#-------------------------------------------------
# generic rules
#-------------------------------------------------

$(OBJ)/%.o: $(SRC)/%.cpp
	@echo Compiling $<...
	@$(CC) $(CDEFS) $(CFLAGS) -c $< -o $@

$(OBJ)/%.pp: $(SRC)/%.cpp
	@echo Compiling $<...
	@$(CC) $(CDEFS) $(CFLAGS) -E $< -o $@

$(OBJ)/%.s: $(SRC)/%.cpp
	@echo Compiling $<...
	@$(CC) $(CDEFS) $(CFLAGS) -S $< -o $@

$(OBJ)/%.a:
	@echo Archiving $@...
	$(RM) $@
	$(AR) $(ARFLAGS) $@ $^

$(OBJ)/%.json: $(SRC)/%.cpp
	@echo Building compile database entry for $< ...
	@echo { \"directory\": \".\", >> $(TIDY_DB)
	@echo   \"command\": \"$(CC) $(CDEFS) $(CFLAGS) -c $< -o dummy.o\", >> $(TIDY_DB)
	@echo   \"file\": \"$(CURDIR)/$<\" } >> $(TIDY_DB)
	@echo "," >> $(TIDY_DB)

compile_commands_prefix:
	@echo "[" > $(TIDY_DB)

compile_commands_postfix:
	@echo "]" >> $(TIDY_DB)
