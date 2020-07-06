###########################################################################
#
#   makefile
#
#   Core makefile for building netlist
#
###########################################################################

#
# Notes:
#    These should go into some compile.md
#
#    Mingw notes:
#
#		graphviz is needed for documentation
#      		pacman -S mingw-w64-x86_64-graphviz
#
#       Please build documentation using
#           make mingw PARAMS=doc
#
#

SRC = ..
VSBUILD = $(SRC)/buildVS
DOC = $(SRC)/documentation

TIDY_FLAGSX = -checks=*,-google*,-hicpp*,readability*,-fuchsia*,cert-*,-android-*,
TIDY_FLAGSX += -llvm-header-guard,
# TIDY_FLAGSX += -cppcoreguidelines-pro-type-reinterpret-cast,
TIDY_FLAGSX += -cppcoreguidelines-pro-bounds-pointer-arithmetic,
#TIDY_FLAGSX += -cppcoreguidelines-owning-memory,
TIDY_FLAGSX += -modernize-use-default-member-init,-cppcoreguidelines-pro-bounds-constant-array-index,
TIDY_FLAGSX += -modernize-pass-by-value,
#TIDY_FLAGSX += -cppcoreguidelines-pro-type-static-cast-downcast,
TIDY_FLAGSX += -cppcoreguidelines-avoid-magic-numbers,
TIDY_FLAGSX += -cppcoreguidelines-macro-usage,
TIDY_FLAGSX += -cppcoreguidelines-non-private-member-variables-in-classes,-misc-non-private-member-variables-in-classes,
TIDY_FLAGSX += -bugprone-macro-parentheses,
#TIDY_FLAGSX += -misc-macro-parentheses,
TIDY_FLAGSX += -bugprone-too-small-loop-variable,
TIDY_FLAGSX += -modernize-use-trailing-return-type,
#TIDY_FLAGSX += -cppcoreguidelines-pro-bounds-array-to-pointer-decay,
TIDY_FLAGSX += -readability-magic-numbers,-readability-braces-around-statements,
TIDY_FLAGSX += -readability-implicit-bool-conversion,
TIDY_FLAGSX += -readability-named-parameter,-readability-function-size,
TIDY_FLAGSX += -llvmlibc-restrict-system-libc-headers,-llvmlibc-implementation-in-namespace,-llvmlibc-callee-namespace,
#TIDY_FLAGSX += -cppcoreguidelines-avoid-non-const-global-variables

space :=
space +=
TIDY_FLAGS = -p $(OBJ) $(subst $(space),,$(TIDY_FLAGSX))
TIDY_SOURCES = $(SOURCES)

#TIDY_SOURCES = $(SRC)/devices/nld_7442.cpp $(SRC)/devices/nld_7492.cpp

#TIDY_FLAGS = -p $(OBJ) -checks=llvm-include-order,llvm-namespace-comment,modernize-use-override,modernize-use-using -fix
#TIDY_FLAGS = -checks=llvm-include-order -fix
#TIDY_FLAGS = -checks=llvm-namespace-comment -fix
#TIDY_FLAGS = -checks=modernize-use-override -fix
#TIDY_FLAGS = -checks=modernize-use-using -fix

ifeq ($(subst Windows_NT,windows,$(OS)),windows)
OBJ = obj/mingw
EXESUFFIX = .exe
DOXYGEN = doxygen.exe
HOSTARCH = WINDOWS
else
OBJ = obj/nix
EXESUFFIX :=
DOXYGEN = @./doxygen
HOSTARCH = NIX
endif

TIDY_DB = $(OBJ)/compile_commands.json


#LTO decreases performance :-(
#LTO = -flto=4  -fuse-linker-plugin -Wodr

CCOREFLAGS = -g -O3 -std=c++14 -I$(CURDIR)/.. -I$(CURDIR)/../..

CFLAGS =  $(LTO) $(CCOREFLAGS) $(CEXTRAFLAGS)
LDFLAGS = $(LTO) -g -O3 -std=c++14 $(LDEXTRAFLAGS)
LIBS = -lpthread -ldl $(EXTRALIBS)

CC = g++
LD = @g++
MD = @mkdir
RM = @rm
CLANG_TIDY = clang-tidy-11
DEPENDCC=$(CC)


ifndef FAST
FAST=1
endif

ifeq ($(FAST),1)
CFLAGS += -DNL_USE_ACADEMIC_SOLVERS=0
endif

TARGETS = nltool$(EXESUFFIX) nlwav$(EXESUFFIX)

NLOBJ = $(OBJ)
POBJ = $(OBJ)/plib

DEPEND = $(OBJ)/.depend

OBJDIRS = $(OBJ) \
			$(OBJ)/analog \
			$(OBJ)/solver \
			$(OBJ)/devices \
			$(OBJ)/plib \
			$(OBJ)/devices \
			$(OBJ)/macro \
			$(OBJ)/tools \
			$(OBJ)/prg \
			$(OBJ)/generated \


OBJS = $(POBJS) $(NLOBJS)

POBJS := \
	$(POBJ)/pstring.o \
	$(POBJ)/pdynlib.o \
	$(POBJ)/pexception.o \
	$(POBJ)/pfunction.o \
	$(POBJ)/pfmtlog.o \
	$(POBJ)/poptions.o \
	$(POBJ)/ppreprocessor.o \
	$(POBJ)/ptokenizer.o \
	$(POBJ)/putil.o \

PMAIN := $(POBJ)/pmain.o

NLOBJS := \
	$(NLOBJ)/solver/nld_solver.o \
	$(NLOBJ)/solver/nld_matrix_solver.o \
	$(NLOBJ)/nl_base.o \
	$(NLOBJ)/nl_parser.o \
	$(NLOBJ)/nl_setup.o \
	$(NLOBJ)/nl_factory.o \
	$(NLOBJ)/analog/nld_bjt.o \
	$(NLOBJ)/analog/nld_mosfet.o \
	$(NLOBJ)/analog/nlid_fourterm.o \
	$(NLOBJ)/analog/nld_switches.o \
	$(NLOBJ)/analog/nlid_twoterm.o \
	$(NLOBJ)/analog/nld_opamps.o \
	$(NLOBJ)/tools/nl_convert.o \
	$(NLOBJ)/generated/static_solvers.o \
	$(NLOBJ)/devices/nld_2102A.o \
	$(NLOBJ)/devices/nld_tms4800.o \
	$(NLOBJ)/devices/nld_4006.o \
	$(NLOBJ)/devices/nld_4013.o \
	$(NLOBJ)/devices/nld_4020.o \
	$(NLOBJ)/devices/nld_4066.o \
	$(NLOBJ)/devices/nld_4316.o \
	$(NLOBJ)/devices/nld_7442.o \
	$(NLOBJ)/devices/nld_7448.o \
	$(NLOBJ)/devices/nld_7450.o \
	$(NLOBJ)/devices/nld_7473.o \
	$(NLOBJ)/devices/nld_7474.o \
	$(NLOBJ)/devices/nld_7475.o \
	$(NLOBJ)/devices/nld_7483.o \
	$(NLOBJ)/devices/nld_7485.o \
	$(NLOBJ)/devices/nld_7492.o \
	$(NLOBJ)/devices/nld_7490.o \
	$(NLOBJ)/devices/nld_7493.o \
	$(NLOBJ)/devices/nld_7497.o \
	$(NLOBJ)/devices/nld_74107.o \
	$(NLOBJ)/devices/nld_74123.o \
	$(NLOBJ)/devices/nld_74125.o \
	$(NLOBJ)/devices/nld_74153.o \
	$(NLOBJ)/devices/nld_74161.o \
	$(NLOBJ)/devices/nld_74163.o \
	$(NLOBJ)/devices/nld_74164.o \
	$(NLOBJ)/devices/nld_74165.o \
	$(NLOBJ)/devices/nld_74166.o \
	$(NLOBJ)/devices/nld_74174.o \
	$(NLOBJ)/devices/nld_74175.o \
	$(NLOBJ)/devices/nld_74192.o \
	$(NLOBJ)/devices/nld_74193.o \
	$(NLOBJ)/devices/nld_74194.o \
	$(NLOBJ)/devices/nld_74365.o \
	$(NLOBJ)/devices/nld_74377.o \
	$(NLOBJ)/devices/nld_74393.o \
	$(NLOBJ)/devices/nld_74ls629.o \
	$(NLOBJ)/devices/nld_82S16.o \
	$(NLOBJ)/devices/nld_82S115.o \
	$(NLOBJ)/devices/nld_9310.o \
	$(NLOBJ)/devices/nld_9316.o \
	$(NLOBJ)/devices/nld_9322.o \
	$(NLOBJ)/devices/nld_am2847.o \
	$(NLOBJ)/devices/nld_dm9314.o \
	$(NLOBJ)/devices/nld_dm9334.o \
	$(NLOBJ)/devices/nld_mm5837.o \
	$(NLOBJ)/devices/nld_ne555.o \
	$(NLOBJ)/devices/nld_r2r_dac.o \
	$(NLOBJ)/devices/nld_roms.o \
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
	$(NLOBJ)/macro/nlm_roms.o \
	$(NLOBJ)/macro/nlm_ttl74xx.o \

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

MAKEFILE_TARGETS_WITHOUT_INCLUDE := gcc9 clang clang-5 mingw doc native maketree tidy


# git archive HEAD --prefix=project-name-version/ \
#     --format=zip -o project-name-version.zip

#-------------------------------------------------
# PHONY
#-------------------------------------------------

.PHONY: all gcc9 clang clang-5 mingw doc native maketree $(DEPEND) depend 

#-------------------------------------------------
# all
#-------------------------------------------------

all: maketree depend $(TARGETS)

#-------------------------------------------------
# clean
#-------------------------------------------------

clean:
	$(RM) -rf $(OBJS) $(TARGETS) $(OBJ)/prg/nltool.o $(OBJ)/prg/nlwav.o $(DEPEND) doxy/*

#-------------------------------------------------
# nltool
#-------------------------------------------------

nltool$(EXESUFFIX): $(OBJ)/prg/nltool.o $(PMAIN) $(OBJS)
	@echo Linking $@...
	$(LD) -o $@ $(LDFLAGS) $^ $(LIBS)

nlwav$(EXESUFFIX): $(OBJ)/prg/nlwav.o $(PMAIN) $(OBJS)
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

native:
	$(MAKE) CEXTRAFLAGS="-march=native -msse4.2 -Wall -Wpedantic -Wsign-compare -Wextra "

gcc9:
	$(MAKE) CC=g++-9 LD=g++-9 CEXTRAFLAGS="-march=native -Wall -pedantic -Wpedantic -fext-numeric-literals -Wsign-compare -Wextra" EXTRALIBS="-lquadmath"

clang:
	#$(MAKE) CC=clang++-11 LD=clang++-11 OBJ=obj/clang CEXTRAFLAGS="-march=native -msse4.2 -Weverything -Wall -pedantic -Wpedantic -Wunused-private-field -Wno-padded -Wno-unused-template -Wno-missing-variable-declarations -Wno-float-equal -Wconversion -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-format-nonliteral  -Wno-exit-time-destructors"
	$(MAKE) CC=clang++-11 LD=clang++-11 OBJ=obj/clang CEXTRAFLAGS="-march=native \
		-mllvm -inline-threshold=2000 \
		-Wunknown-warning-option \
		-Weverything -Wall -pedantic -Wpedantic -Wunused-private-field  \
		-Werror -Wno-padded -Wno-weak-vtables -Wno-weak-template-vtables -Wno-unused-template \
		-Wmissing-variable-declarations -Wno-float-equal -Wconversion \
		-Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-format-nonliteral \
		-Wno-exit-time-destructors -Winconsistent-missing-destructor-override \
		-Wno-return-std-move-in-c++11 -Wno-unreachable-code"

clang-libc:
	#clang-11 currently broken
	#$(MAKE) CC=clang++-11 LD=clang++-11 OBJ=obj/clang CEXTRAFLAGS="-march=native -msse4.2 -Weverything -Wall -pedantic -Wpedantic -Wunused-private-field -Wno-padded -Wno-unused-template -Wno-missing-variable-declarations -Wno-float-equal -Wconversion -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-format-nonliteral  -Wno-exit-time-destructors"
	$(MAKE) CC=clang++-10 LD=clang++-10 OBJ=obj/clang-libc CEXTRAFLAGS="-march=native \
		-stdlib=libc++ -mllvm -inline-threshold=2000 \
		-Wunknown-warning-option \
		-Weverything -Wall -pedantic -Wpedantic -Wunused-private-field  \
		-Werror -Wno-padded -Wno-weak-vtables -Wno-weak-template-vtables -Wno-unused-template \
		-Wmissing-variable-declarations -Wno-float-equal -Wconversion \
		-Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-format-nonliteral \
		-Wno-exit-time-destructors -Winconsistent-missing-destructor-override \
		-Wno-return-std-move-in-c++11 -Wno-unreachable-code"
		LDEXTRAFLAGS=-stdlib=libc++

clang-5:
	$(MAKE) CC=clang++-5.0 LD=clang++-5.0 CEXTRAFLAGS="-march=native -Weverything -Werror -Wno-inconsistent-missing-destructor-override -Wno-unreachable-code -Wno-padded -Wno-weak-vtables -Wno-missing-variable-declarations -Wconversion -Wno-c++98-compat -Wno-float-equal -Wno-global-constructors -Wno-c++98-compat-pedantic -Wno-format-nonliteral -Wno-weak-template-vtables -Wno-exit-time-destructors"

nvcc:
	$(MAKE) CC=/usr/local/cuda-10.2/bin/nvcc LD=/usr/local/cuda-10.2/bin/nvcc \
		OBJ=obj/nvcc CEXTRAFLAGS="--std c++14 -x cu  -DNVCCBUILD=102 --expt-extended-lambda \
		--expt-relaxed-constexpr --default-stream per-thread --restrict \
		--ftemplate-depth 1024  \
		-Xcompiler -O6 -Xcompiler -march=native -ccbin g++-8 " \
		DEPENDCC=g++

tidy_db: compile_commands_prefix $(ALL_TIDY_FILES) compile_commands_postfix

#
# Mostly done: -Wno-weak-vtables -Wno-cast-align
# FIXME:  -Wno-weak-vtables -Wno-weak-template-vtables -Wno-exit-time-destructors
# FIXME: -Winconsistent-missing-destructor-override : c++ community has diverging opinions on this https://github.com/isocpp/CppCoreGuidelines/issues/721
# FIXME: -Wunreachable-code : False warnings, this a documented clang bug: https://llvm.org/bugs/show_bug.cgi?id=28994
# -Wweak-template-vtables
#
# These have to be given:
# -Wno-missing-variable-declarations : Device factory code, on purpose

mingw:
	$(MAKE) CEXTRAFLAGS="-DUNICODE -D_UNICODE -D_WIN32_WINNT=0x0501 \
		-DWIN32_LEAN_AND_MEAN" LDEXTRAFLAGS="-Wl,--subsystem,console \
		-municode" LIBS= MD=@mkdir.exe SHELL=sh.exe DOXYGEN=doxygen.exe $(PARAMS)

#
# FIXME: Unicode
#     CEXTRAFLAGS = -DUNICODE -D_UNICODE -municode
#     LDEXTRAFLAGS = -municode
#


doc: nltool$(EXESUFFIX)
	./nltool$(EXESUFFIX) -c docheader > ../documentation/devsyn.dox.h
	$(DOXYGEN) doxygen.conf

#-------------------------------------------------
# depends
#-------------------------------------------------

$(DEPEND): maketree $(SOURCES)
	@echo creating $(DEPEND)
	@rm -f $(DEPEND)
	@for i in $(SOURCES); do \
		$(DEPENDCC) $(CCOREFLAGS) -MM $$i -MT `echo $$i | sed -e 's+$(SRC)+$(OBJ)+' -e 's+.cpp+.o+' ` >> $(DEPEND); \
	done

depend: $(DEPEND)

# Include only if the goal needs it
ifeq ($(filter $(MAKECMDGOALS),$(MAKEFILE_TARGETS_WITHOUT_INCLUDE)),)
-include $(DEPEND)
endif

#-------------------------------------------------
# clang tidy
#-------------------------------------------------
tidy: tidy_db
	@echo running tidy
	@for i in $(TIDY_SOURCES); do \
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
