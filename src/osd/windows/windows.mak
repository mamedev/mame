###########################################################################
#
#   windows.mak
#
#   Windows-specific makefile
#
#   Copyright Nicola Salmoria and the MAME Team.
#   Visit http://mamedev.org for licensing and usage restrictions.
#
###########################################################################


###########################################################################
#################   BEGIN USER-CONFIGURABLE OPTIONS   #####################
###########################################################################


#-------------------------------------------------
# specify build options; see each option below
# for details
#-------------------------------------------------

# uncomment next line to enable a build using Microsoft tools
# MSVC_BUILD = 1

# uncomment next line to use cygwin compiler
# CYGWIN_BUILD = 1

# uncomment next line to enable multi-monitor stubs on Windows 95/NT
# you will need to find multimon.h and put it into your include
# path in order to make this work
# WIN95_MULTIMON = 1

# uncomment next line to enable a Unicode build
# UNICODE = 1

# set this to the minimum Direct3D version to support (8 or 9)
# DIRECT3D = 9

# set this to the minimum DirectInput version to support (7 or 8)
# DIRECTINPUT = 8



###########################################################################
##################   END USER-CONFIGURABLE OPTIONS   ######################
###########################################################################


#-------------------------------------------------
# overrides
#-------------------------------------------------

# turn on unicode for all 64-bit builds regardless
ifndef UNICODE
ifdef PTR64
UNICODE = 1
endif
endif



#-------------------------------------------------
# object and source roots
#-------------------------------------------------

WINSRC = $(SRC)/osd/$(OSD)
WINOBJ = $(OBJ)/osd/$(OSD)

OBJDIRS += $(WINOBJ)



#-------------------------------------------------
# configure the resource compiler
#-------------------------------------------------

RC = @windres --use-temp-file

RCDEFS = -DNDEBUG -D_WIN32_IE=0x0501

RCFLAGS = -O coff -I $(WINSRC) -I $(WINOBJ)



#-------------------------------------------------
# overrides for the CYGWIN compiler
#-------------------------------------------------

ifdef CYGWIN_BUILD
CCOMFLAGS += -mno-cygwin
LDFLAGS	+= -mno-cygwin
endif



#-------------------------------------------------
# overrides for the MSVC compiler
#-------------------------------------------------

ifdef MSVC_BUILD

VCONV = $(WINOBJ)/vconv$(EXE)

# append a 'v' prefix if nothing specified
ifndef PREFIX
PREFIX = v
endif

# replace the various compilers with vconv.exe prefixes
CC = @$(VCONV) gcc -I.
LD = @$(VCONV) ld /profile
AR = @$(VCONV) ar
RC = @$(VCONV) windres

# make sure we use the multithreaded runtime
ifdef DEBUG
CCOMFLAGS += /MTd
else
CCOMFLAGS += /MT
endif

# turn on link-time codegen if the MAXOPT flag is also set
ifdef MAXOPT
CCOMFLAGS += /GL
LDFLAGS += /LTCG
AR += /LTCG
endif

ifdef PTR64
CCOMFLAGS += /wd4267
endif

# disable function pointer warnings in C++ which are evil to work around
CPPONLYFLAGS += /wd4191 /wd4060 /wd4065 /wd4640

# explicitly set the entry point for UNICODE builds
ifdef UNICODE
LDFLAGS += /ENTRY:wmainCRTStartup
endif

# add some VC++-specific defines
DEFS += -D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE -DXML_STATIC -D__inline__=__inline -Dsnprintf=_snprintf

# make msvcprep into a pre-build step
OSPREBUILD = $(VCONV)

ifneq ($(CROSS_BUILD),1)
# add VCONV to the build tools
BUILD += $(VCONV)

$(VCONV): $(WINOBJ)/vconv.o
	@echo Linking $@...
	@link.exe /nologo $^ version.lib /out:$@

$(WINOBJ)/vconv.o: $(WINSRC)/vconv.c
	@echo Compiling $<...
	@cl.exe /nologo /O1 -D_CRT_SECURE_NO_DEPRECATE -c $< /Fo$@

OSDCLEAN = msvcclean

msvcclean:
	@echo Deleting Visual Studio specific files...
	$(RM) *.pdb
	$(RM) *.lib
	$(RM) *.exp

endif
endif


#-------------------------------------------------
# due to quirks of using /bin/sh, we need to
# explicitly specify the current path
#-------------------------------------------------

CURPATH = ./



#-------------------------------------------------
# Windows-specific debug objects and flags
#-------------------------------------------------

# define the x64 ABI to be Windows
DEFS += -DX64_WINDOWS_ABI

# map all instances of "main" to "utf8_main"
DEFS += -Dmain=utf8_main

# debug build: enable guard pages on all memory allocations
ifdef DEBUG
DEFS += -DMALLOC_DEBUG
LDFLAGS += -Wl,--allow-multiple-definition
endif

# enable UNICODE flags for unicode builds
ifdef UNICODE
DEFS += -DUNICODE -D_UNICODE
endif



#-------------------------------------------------
# Windows-specific flags and libraries
#-------------------------------------------------

# add our prefix files to the mix
CCOMFLAGS += -include $(WINSRC)/winprefix.h

ifdef WIN95_MULTIMON
CCOMFLAGS += -DWIN95_MULTIMON
endif

# add the windows libraries
LIBS += -luser32 -lgdi32 -lddraw -ldsound -ldxguid -lwinmm -ladvapi32 -lcomctl32 -lshlwapi

ifdef CPP_COMPILE
LIBS += -lsupc++
endif

ifeq ($(DIRECTINPUT),8)
LIBS += -ldinput8
CCOMFLAGS += -DDIRECTINPUT_VERSION=0x0800
else
LIBS += -ldinput
CCOMFLAGS += -DDIRECTINPUT_VERSION=0x0700
endif

ifdef PTR64
ifdef MSVC_BUILD
LIBS += -lbufferoverflowu
else
DEFS += -D_COM_interface=struct
endif
endif



#-------------------------------------------------
# OSD core library
#-------------------------------------------------

OSDCOREOBJS = \
	$(WINOBJ)/main.o	\
	$(WINOBJ)/strconv.o	\
	$(WINOBJ)/windir.o \
	$(WINOBJ)/winfile.o \
	$(WINOBJ)/winmisc.o \
	$(WINOBJ)/winsync.o \
	$(WINOBJ)/wintime.o \
	$(WINOBJ)/winutf8.o \
	$(WINOBJ)/winutil.o \
	$(WINOBJ)/winwork.o \

# if malloc debugging is enabled, include the necessary code
ifneq ($(findstring MALLOC_DEBUG,$(DEFS)),)
OSDCOREOBJS += \
	$(WINOBJ)/winalloc.o
endif



#-------------------------------------------------
# OSD Windows library
#-------------------------------------------------

OSDOBJS = \
	$(WINOBJ)/d3d9intf.o \
	$(WINOBJ)/drawd3d.o \
	$(WINOBJ)/drawdd.o \
	$(WINOBJ)/drawgdi.o \
	$(WINOBJ)/drawnone.o \
	$(WINOBJ)/input.o \
	$(WINOBJ)/output.o \
	$(WINOBJ)/sound.o \
	$(WINOBJ)/video.o \
	$(WINOBJ)/window.o \
	$(WINOBJ)/winmain.o

ifeq ($(DIRECT3D),9)
CCOMFLAGS += -DDIRECT3D_VERSION=0x0900
else
OSDOBJS += $(WINOBJ)/d3d8intf.o
endif

# extra dependencies
$(WINOBJ)/drawdd.o : 	$(SRC)/emu/rendersw.c
$(WINOBJ)/drawgdi.o :	$(SRC)/emu/rendersw.c

# add debug-specific files
OSDOBJS += \
	$(WINOBJ)/debugwin.o

# add a stub resource file
RESFILE = $(WINOBJ)/mame.res



#-------------------------------------------------
# rules for building the libaries
#-------------------------------------------------

$(LIBOCORE): $(OSDCOREOBJS)

$(LIBOSD): $(OSDOBJS)



#-------------------------------------------------
# rule for making the ledutil sample
#-------------------------------------------------

LEDUTIL = ledutil$(EXE)
TOOLS += $(LEDUTIL)

LEDUTILOBJS = \
	$(WINOBJ)/ledutil.o

$(LEDUTIL): $(LEDUTILOBJS) $(LIBOCORE)
	@echo Linking $@...
	$(LD) $(LDFLAGS) $^ $(LIBS) -o $@



#-------------------------------------------------
# generic rule for the resource compiler
#-------------------------------------------------

$(WINOBJ)/%.res: $(WINSRC)/%.rc | $(OSPREBUILD)
	@echo Compiling resources $<...
	$(RC) $(RCDEFS) $(RCFLAGS) -o $@ -i $<



#-------------------------------------------------
# rules for resource file
#-------------------------------------------------

$(RESFILE): $(WINSRC)/mame.rc $(WINOBJ)/mamevers.rc

$(WINOBJ)/mamevers.rc: $(BUILDOUT)/verinfo$(BUILD_EXE) $(SRC)/version.c
	@echo Emitting $@...
	@"$(BUILDOUT)/verinfo$(BUILD_EXE)" -b windows $(SRC)/version.c > $@

