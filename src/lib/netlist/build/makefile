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

.DEFAULT_GOAL := all

SRC = ..
#SRC = $(abspath ..)

PSRC = $(SRC)/plib
VSBUILD = $(SRC)/buildVS
DOC = $(SRC)/documentation

TIDY_FLAGSX = -checks=*,-google*,-hicpp*,readability*,-fuchsia*,cert-*,-android-*,-altera*,
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
TIDY_FLAGSX += -readability-magic-numbers,-readability-braces-around-statements,
TIDY_FLAGSX += -readability-implicit-bool-conversion,
TIDY_FLAGSX += -readability-named-parameter,-readability-function-size,
TIDY_FLAGSX += -llvmlibc-restrict-system-libc-headers,-llvmlibc-implementation-in-namespace,-llvmlibc-callee-namespace,
TIDY_FLAGSX += -modernize-use-nodiscard,
TIDY_FLAGSX += -modernize-use-auto,
#TIDY_FLAGSX += -cppcoreguidelines-pro-bounds-array-to-pointer-decay,
#TIDY_FLAGSX += -cppcoreguidelines-prefer-member-initializer,
#TIDY_FLAGSX += -modernize-use-transparent-functors,
TIDY_FLAGSX += -readability-function-cognitive-complexity,
TIDY_FLAGSX += -readability-uppercase-literal-suffix
TIDY_FLAGSX += -cert-err58-cpp


#TIDY_FLAGSX += -cppcoreguidelines-avoid-non-const-global-variables

space :=
space +=
TIDY_FLAGS = -p $(OBJ) $(subst $(space),,$(TIDY_FLAGSX))
TIDY_SOURCES = $(ALLSOURCES)

#TIDY_SOURCES = $(SRC)/devices/nld_7442.cpp $(SRC)/devices/nld_7492.cpp

#TIDY_FLAGS = -p $(OBJ) -checks=llvm-include-order,llvm-namespace-comment,modernize-use-override,modernize-use-using -fix
#TIDY_FLAGS = -checks=llvm-include-order -fix
#TIDY_FLAGS = -checks=llvm-namespace-comment -fix
#TIDY_FLAGS = -checks=modernize-use-override -fix
#TIDY_FLAGS = -checks=modernize-use-using -fix

ifeq ($(subst Windows_NT,Windows,$(OS)),Windows)
    HOSTARCH := Windows
else
    HOSTARCH := $(shell uname -s)
endif

ifeq ($(HOSTARCH),Windows)
OBJ = obj/mingw
EXESUFFIX = .exe
DOXYGEN = doxygen.exe
CEXTRAFLAGS = -DUNICODE -D_UNICODE -D_WIN32_WINNT=0x0501 -DWIN32_LEAN_AND_MEAN
LDEXTRAFLAGS = -Wl,--subsystem,console -municode
LIBS = $(EXTRALIBS)
MD = @mkdir.exe
RM = @rm.exe
SHELL = sh.exe
else ifeq ($(HOSTARCH),Darwin)
OBJ = obj/darwin
EXESUFFIX :=
DOXYGEN = @doxygen
LIBS = -lpthread -ldl $(EXTRALIBS)
MD = @mkdir
RM = @rm
else
OBJ = obj/nix
EXESUFFIX :=
DOXYGEN = @./doxygen
LIBS = -lpthread -ldl $(EXTRALIBS)
MD = @mkdir
RM = @rm
endif

CSPELL=cspell
PYTHON=@python

TIDY_DB = $(OBJ)/compile_commands.json


#LTO decreases performance :-(
#LTO = -flto=4  -fuse-linker-plugin -Wodr

CCOREFLAGS = -g -O3 -std=c++17 -I$(SRC)

CFLAGS =  $(LTO) $(CCOREFLAGS) $(CEXTRAFLAGS)
LDFLAGS = $(LTO) -g -O3 -std=c++17 $(LDEXTRAFLAGS)

CC = g++
LD = g++
CLANG_TIDY = clang-tidy-13
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
TESTOBJ = $(OBJ)/tests

DEPEND := $(OBJ)/.depend

OBJDIRS = $(OBJ) \
			$(OBJ)/analog \
			$(OBJ)/solver \
			$(OBJ)/devices \
			$(OBJ)/plib \
			$(OBJ)/macro \
			$(OBJ)/macro/modules \
			$(OBJ)/tests \
			$(OBJ)/tools \
			$(OBJ)/prg \
			$(OBJ)/generated \

MODULESOURCES += $(wildcard $(SRC)/macro/modules/*.cpp)
DEVSOURCES =  $(SRC)/solver/nld_solver.cpp
DEVSOURCES += $(wildcard $(SRC)/devices/*.cpp)
DEVSOURCES += $(wildcard $(SRC)/analog/*.cpp)
DEVSOURCES += $(wildcard $(SRC)/macro/*.cpp)
DEVSOURCES += $(MODULESOURCES)
DEVSOURCES += $(SRC)/generated/nlm_modules_lib.cpp

TESTSOURCES = $(wildcard $(SRC)/tests/*.cpp)

CORESOURCES := \
	$(SRC)/solver/nld_matrix_solver.cpp \
	$(SRC)/nl_base.cpp \
	$(SRC)/nl_parser.cpp \
	$(SRC)/nl_setup.cpp \
	$(SRC)/nl_factory.cpp \
	$(SRC)/tools/nl_convert.cpp \
	$(SRC)/generated/static_solvers.cpp \

MAINSOURCES = $(SRC)/prg/nltool.cpp $(SRC)/prg/nlwav.cpp

HEADERS = $(wildcard $(SRC)/analog/*.h)
HEADERS += $(wildcard $(SRC)/core/*.h)
HEADERS += $(wildcard $(SRC)/devices/*.h)
HEADERS += $(wildcard $(SRC)/solver/*.h)
HEADERS += $(wildcard $(SRC)/tools/*.h)
HEADERS += $(wildcard $(SRC)/*.h)
HEADERS += $(wildcard $(SRC)/generated/*.h)

PHEADERS = $(wildcard $(SRC)/plib/*.h)

PSOURCES := \
	$(PSRC)/pstring.cpp \
	$(PSRC)/pdynlib.cpp \
	$(PSRC)/pexception.cpp \
	$(PSRC)/pfunction.cpp \
	$(PSRC)/pfmtlog.cpp \
	$(PSRC)/pmain.cpp \
	$(PSRC)/poptions.cpp \
	$(PSRC)/ppmf.cpp \
	$(PSRC)/ppreprocessor.cpp \
	$(PSRC)/ptokenizer.cpp \
	$(PSRC)/putil.cpp \

NLOBJS = 	$(patsubst $(SRC)%, $(OBJ)%, $(CORESOURCES:.cpp=.o))
NLDEVOBJS = $(patsubst $(SRC)%, $(OBJ)%, $(DEVSOURCES:.cpp=.o))
TESTOBJS = 	$(patsubst $(SRC)%, $(OBJ)%, $(TESTSOURCES:.cpp=.o))
MAINOBJS = 	$(patsubst $(SRC)%, $(OBJ)%, $(MAINSOURCES:.cpp=.o))
POBJS = 	$(patsubst $(SRC)%, $(OBJ)%, $(PSOURCES:.cpp=.o))

OBJS = $(TESTOBJS) $(POBJS) $(NLOBJS) $(NLDEVOBJS) $(TESTOBJS)

VSFILES = \
	$(VSBUILD)/netlistlib.vcxproj \
	$(VSBUILD)/netlistlib.vcxproj.filters \
	$(VSBUILD)/nltool.vcxproj \
	$(VSBUILD)/nltool.vcxproj.filters \
	$(VSBUILD)/nlwav.vcxproj \
	$(VSBUILD)/nlwav.vcxproj.filters \
	$(VSBUILD)/netlist.sln \

OTHERFILES = makefile

DOCS = \
	doxygen.conf \
	$(DOC)/doc.css \
	$(DOC)/mainpage.dox.h \
	$(DOC)/primer_1.dox.h \
	$(DOC)/structure.dox.h \
	$(DOC)/test1-50r.svg \

ALL_OBJS = $(OBJS) $(MAINOBJS)

ALL_TIDY_FILES = $(ALL_OBJS:.o=.json)
ALLSOURCES = $(DEVSOURCES) $(CORESOURCES) $(TESTSOURCES) $(MAINSOURCES) $(PSOURCES)
ALLHEADERS = $(HEADERS) $(PHEADERS)
ALLFILES = $(ALLSOURCES) $(VSFILES) $(DOCS) $(OTHERFILES)

MAKEFILE_TARGETS_WITHOUT_INCLUDE := \
	clang clang-5 clang-libc gcc9 mingw native nvcc \
	clean depend doc generated manpages maketree \
	runtests tidy fix_permissions precommit srcclean

BUILD_DIRS = $(OBJDIRS) man html

# git archive HEAD --prefix=project-name-version/ \
#     --format=zip -o project-name-version.zip

#-------------------------------------------------
# PHONY
#-------------------------------------------------

.PHONY: $(MAKEFILE_TARGETS_WITHOUT_INCLUDE)

#-------------------------------------------------
# all
#-------------------------------------------------

all: maketree $(DEPEND) $(TARGETS)

#-------------------------------------------------
# clean
#-------------------------------------------------

clean:
	$(RM) -rf $(OBJS) $(TARGETS) $(MAINOBJS) $(DEPEND) doxy/*

#-------------------------------------------------
# nltool
#-------------------------------------------------

nltool$(EXESUFFIX): $(OBJ)/prg/nltool.o $(OBJS)
	@echo Linking $@...
	@$(LD) -o $@ $(LDFLAGS) $^ $(LIBS)

nlwav$(EXESUFFIX): $(OBJ)/prg/nlwav.o $(OBJS)
	@echo Linking $@...
	@$(LD) -o $@ $(LDFLAGS) $^ $(LIBS)

#-------------------------------------------------
# directories
#-------------------------------------------------

$(sort $(BUILD_DIRS)):
	$(MD) -p $@

maketree: $(sort $(BUILD_DIRS))

#-------------------------------------------------
# Special targets
#-------------------------------------------------

native:
	$(MAKE) CEXTRAFLAGS="-march=native -msse4.2 -Wall -Wpedantic -Wsign-compare -Wextra"

gcc9:
	$(MAKE) CC=g++-9 LD=g++-9 CEXTRAFLAGS="-march=native -Wall -Wextra -pedantic -Wpedantic -pedantic-errors -fext-numeric-literals -Wsign-compare" EXTRALIBS="-lquadmath" OBJ=obj/gcc9

clang:
	#$(MAKE) CC=clang++-11 LD=clang++-11 OBJ=obj/clang CEXTRAFLAGS="-march=native -msse4.2 -Weverything -Wall -pedantic -Wpedantic -Wunused-private-field -Wno-padded -Wno-unused-template -Wno-missing-variable-declarations -Wno-float-equal -Wconversion -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-format-nonliteral  -Wno-exit-time-destructors"
	$(MAKE) CC=clang++-13 LD=clang++-13 OBJ=obj/clang CEXTRAFLAGS="-march=native \
		-mllvm  -inline-threshold=2000 \
		-Wunknown-warning-option \
		-Weverything -Wall -pedantic -Wpedantic \
		-Werror -Wno-padded -Wno-weak-vtables -Wno-weak-template-vtables -Wunused-template \
		-Wno-float-equal \
		-Wno-c++98-compat -Wno-c++98-compat-pedantic \
		-Wno-exit-time-destructors \
		-Wno-undefined-reinterpret-cast -Wno-error=unused-macros \
		-Wno-error=deprecated"

clang-libc:
	#clang-11 currently broken
	#$(MAKE) CC=clang++-11 LD=clang++-11 OBJ=obj/clang CEXTRAFLAGS="-march=native
	$(MAKE) CC=clang++-13 LD=clang++-13 OBJ=obj/clang-libc CEXTRAFLAGS="-march=native \
		-stdlib=libc++ -mllvm -inline-threshold=2000 \
		-Wunknown-warning-option \
		-Weverything -Wall -pedantic -Wpedantic -Wunused-private-field  \
		-Werror -Wno-padded -Wno-weak-vtables -Wno-weak-template-vtables -Wunused-template \
		-Wmissing-variable-declarations -Wno-float-equal -Wconversion \
		-Wno-c++98-compat -Wno-c++98-compat-pedantic -Wformat-nonliteral \
		-Wno-exit-time-destructors -Winconsistent-missing-destructor-override \
		-Wno-undefined-reinterpret-cast \
		-Wunreachable-code \
		-Wmissing-prototypes" \
		LDEXTRAFLAGS=-stdlib=libc++

nvcc:
	$(MAKE) CC=/usr/local/cuda-11.3/bin/nvcc LD=/usr/local/cuda-11.3/bin/nvcc \
		OBJ=obj/nvcc CEXTRAFLAGS="--std c++17 -x cu --expt-extended-lambda \
		--expt-relaxed-constexpr --default-stream per-thread --restrict \
		--ftemplate-depth 1024  \
		-Xcompiler -O6 -Xcompiler -march=native -ccbin g++-9 " \
		DEPENDCC=g++

mingw:
	$(MAKE) CEXTRAFLAGS="-DUNICODE -D_UNICODE -D_WIN32_WINNT=0x0501 \
		-DWIN32_LEAN_AND_MEAN" LDEXTRAFLAGS="-Wl,--subsystem,console \
		-municode" LIBS= MD=@mkdir.exe SHELL=sh.exe DOXYGEN=doxygen.exe $(PARAMS)

mingw-cross:
	${MAKE}	CC="x86_64-w64-mingw32-g++-posix" LD="x86_64-w64-mingw32-g++-posix" \
		CEXTRAFLAGS="-DUNICODE -D_UNICODE -D_WIN32_WINNT=0x0501 -DWIN32_LEAN_AND_MEAN" \
		LDEXTRAFLAGS="-Wl,--subsystem,console -municode" OBJ=obj/mingw-cross \
		EXESUFFIX=.exe LIBS=

#
# run with emrun  --browser chrome nltool.html -c run  -t 10 -n kidniki nl_kidniki.cpp
#
emsdk:
	emmake $(MAKE) CC=emcc LD=emcc CEXTRAFLAGS="-fexceptions -msimd128" LDEXTRAFLAGS="-s ALLOW_MEMORY_GROWTH=0 -s TOTAL_MEMORY=268435456 -fexceptions -msimd128 --emrun --embed-file ../../../mame/machine/nl_pongf.cpp@nl_pongf.cpp --embed-file ../../../mame/audio/nl_kidniki.cpp@nl_kidniki.cpp" OBJ=obj/emsdk EXESUFFIX=.html
	# LDFLAGS  -s ASSERTIONS=1


#-------------------------------------------------
# regression tests
#-------------------------------------------------

runtests: $(TARGETS)
	./nltool$(EXESUFFIX) -c tests

#-------------------------------------------------
# man pages
#-------------------------------------------------

manpages: maketree $(TARGETS) man/nltool.1 man/nlwav.1 html/nltool.html html/nlwav.html

#-------------------------------------------------
# documentation
#-------------------------------------------------

doc:
	./nltool$(EXESUFFIX) -c docheader > ../documentation/devsyn.dox.h
	$(DOXYGEN) doxygen.conf

cspell:
	#cd .. && cspell -c build/cspell.json --no-color --locale en-GB --relative $(patsubst $(SRC)/%, %, $(ALLHEADERS) ) | sed -e "s/ - Unknown/: error: Unknown/g"
	cd .. && $(CSPELL) -c build/cspell.json --no-color --locale en-GB --relative $(patsubst $(SRC)/%, %, $(ALLHEADERS) $(ALLSOURCES)) | sed -e "s/ - Unknown/: error: Unknown/g"

#-------------------------------------------------
# depends
#-------------------------------------------------

.PHONY: depend
depend: $(DEPEND)

$(DEPEND): $(ALLSOURCES) | $(OBJ)
	@echo creating $(DEPEND)
	@$(RM) -f $(DEPEND)
	@for i in $(ALLSOURCES); do \
		$(DEPENDCC) $(CCOREFLAGS) -MM $$i -MT `echo $$i \
		| sed -e 's+$(SRC)+$(OBJ)+' -e 's+.cpp+.o+' ` \
		| sed -e 's_\.\./[a-z]*/\.\./_../_g' >> $(DEPEND); \
	done

# Include only if the goal needs it
ifeq ($(filter $(MAKECMDGOALS),$(MAKEFILE_TARGETS_WITHOUT_INCLUDE)),)
-include $(DEPEND)
endif

#-------------------------------------------------
# generated
#-------------------------------------------------

$(SRC)/generated/lib_entries.hxx: create_lib_entries.py $(DEVSOURCES)
	@echo creating $@
	$(PYTHON) create_lib_entries.py $^ > $@

$(SRC)/generated/nld_devinc.h: create_devinc.py $(DEVSOURCES)
	@echo creating $@
	$(PYTHON) create_devinc.py $^ > $@

$(SRC)/generated/nlm_modules_lib.cpp: create_modules.py $(MODULESOURCES)
	@echo creating $@
	$(PYTHON) create_modules.py $^ > $@

.PHONY: generated
generated: $(SRC)/generated/lib_entries.hxx $(SRC)/generated/nld_devinc.h $(SRC)/generated/nlm_modules_lib.cpp

#-------------------------------------------------
# fix permissions, source management
#-------------------------------------------------

fix_permissions:
	@find .. -executable -and -not -type d -print | grep -v /build | xargs -r chmod a-x
	@chmod a-x $(ALLFILES)
	@find .. -print | xargs setfacl -b

srcclean: $(ALLSOURCES) $(ALLHEADERS)
	@../../../../srcclean -u $(ALLSOURCES) $(ALLHEADERS)

precommit: fix_permissions srcclean all runtests

#-------------------------------------------------
# clang tidy
#-------------------------------------------------

tidy_db: compile_commands_prefix $(ALL_TIDY_FILES) compile_commands_postfix

tidy: tidy_db
	@echo running tidy
	@for i in $(TIDY_SOURCES); do \
		$(CLANG_TIDY) $$i $(TIDY_FLAGS) -header-filter=.*; \
	done

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

man/%.1: %$(EXESUFFIX)
	@echo Building $@...
	@help2man --include=$*.help2man --no-info ./$< > $@

html/%.html: man/%.1
	@echo Building $@...
	@groff -mandoc -Thtml $< > $@

compile_commands_prefix:
	@echo "[" > $(TIDY_DB)

compile_commands_postfix:
	@echo "]" >> $(TIDY_DB)
