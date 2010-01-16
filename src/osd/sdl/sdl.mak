###########################################################################
#
#   sdl.mak
#
#   SDL-specific makefile
#
#   Copyright (c) 1996-2010, Nicola Salmoria and the MAME Team.
#   Visit http://mamedev.org for licensing and usage restrictions.
#
#   SDLMAME by Olivier Galibert and R. Belmont
#
###########################################################################

###########################################################################
#################   BEGIN USER-CONFIGURABLE OPTIONS   #####################
###########################################################################


#-------------------------------------------------
# specify build options; see each option below
# for details
#-------------------------------------------------


# uncomment and edit next line to specify a distribution
# supported debian-stable, ubuntu-intrepid

# DISTRO = debian-stable
# DISTRO = ubuntu-intrepid
# DISTRO = gcc44-generic

# uncomment next line to build without OpenGL support

# NO_OPENGL = 1

# uncomment next line to build without X11 support

# NO_X11 = 1

# uncomment and adapt next line to link against specific GL-Library
# this will also add a rpath to the executable
# MESA_INSTALL_ROOT = /usr/local/dfb_GL

# uncomment the next line to build a binary using
# GL-dispatching.
# This option takes precedence over MESA_INSTALL_ROOT

USE_DISPATCH_GL = 1

# uncomment and change the next line to compile and link to specific
# SDL library. This is currently only supported for unix!
# There is no need to play with this option unless you are doing
# active development on sdlmame or SDL.

ifneq ($(TARGETOS),win32)

#SDL_INSTALL_ROOT = /usr/local/sdl13w32
#SDL_INSTALL_ROOT = /usr/local/sdl13
#SDL_INSTALL_ROOT = /usr/local/test
endif

###########################################################################
##################   END USER-CONFIGURABLE OPTIONS   ######################
###########################################################################

ifdef NOASM
DEFS += -DSDLMAME_NOASM
endif

# bring in external flags for RPM build
CCOMFLAGS += $(OPT_FLAGS)

#-------------------------------------------------
# distribution may change things
#-------------------------------------------------

ifeq ($(DISTRO),)
DISTRO = generic
else
ifeq ($(DISTRO),debian-stable)
DEFS += -DNO_AFFINITY_NP
else
ifeq ($(DISTRO),ubuntu-intrepid)
# Force gcc-4.2 on ubuntu-intrepid
CC = @gcc -V 4.2
LD = g++-4.2
#LDFLAGS += -lsupc++ -static-libgcc
else
ifeq ($(DISTRO),gcc44-generic)
CC = @gcc -V 4.4
LD = @g++ -V 4.4
else
$(error DISTRO $(DISTRO) unknown)
endif
endif
endif
endif

DEFS += -DDISTRO=$(DISTRO)

#-------------------------------------------------
# sanity check the configuration
#-------------------------------------------------

ifdef BIGENDIAN
X86_MIPS3_DRC =
X86_PPC_DRC =
FORCE_DRC_C_BACKEND = 1
endif

ifdef NOASM
X86_MIPS3_DRC =
X86_PPC_DRC =
FORCE_DRC_C_BACKEND = 1
endif

#-------------------------------------------------
# compile and linking flags
#-------------------------------------------------

ifdef SYMBOLS
ifdef BIGENDIAN
ifeq ($(TARGETOS),macosx)
CCOMFLAGS += -mlong-branch
endif	# macosx
endif	# PPC
endif	# SYMBOLS

# add an ARCH define
DEFS += "-DSDLMAME_ARCH=$(ARCHOPTS)" -DSYNC_IMPLEMENTATION=$(SYNC_IMPLEMENTATION)

# add SDLMAME TARGETOS definitions

ifeq ($(TARGETOS),unix)
SYNC_IMPLEMENTATION = tc
endif

ifeq ($(TARGETOS),freebsd)
TARGETOS = unix
SYNC_IMPLEMENTATION = ntc
endif

ifeq ($(TARGETOS),openbsd)
TARGETOS = unix
SYNC_IMPLEMENTATION = ntc
endif

ifeq ($(TARGETOS),solaris)
DEFS += -DNO_AFFINITY_NP -DNO_DEBUGGER -DSDLMAME_X11 -DSDLMAME_UNIX
SYNC_IMPLEMENTATION = tc
endif

ifeq ($(TARGETOS),unix)
DEFS += -DSDLMAME_UNIX
ifndef NO_X11
DEFS += -DSDLMAME_X11
else
DEFS += -DSDLMAME_NO_X11 -DNO_DEBUGGER
endif
endif

ifeq ($(TARGETOS),macosx)
DEFS += -DSDLMAME_UNIX -DSDLMAME_MACOSX
SYNC_IMPLEMENTATION = ntc
MAINLDFLAGS = -Xlinker -all_load
ifdef BIGENDIAN
PPC=1
endif
ifdef PPC
ifdef PTR64
CCOMFLAGS += -arch ppc64
LDFLAGS += -arch ppc64
else
CCOMFLAGS += -arch ppc
LDFLAGS += -arch ppc
endif
else
ifdef PTR64
CCOMFLAGS += -arch x86_64
LDFLAGS += -arch x86_64
else
CCOMFLAGS += -m32 -arch i386
LDFLAGS += -m32 -arch i386
endif
endif
endif

ifeq ($(TARGETOS),win32)
DEFS += -DSDLMAME_WIN32 -DNO_DEBUGGER -DX64_WINDOWS_ABI
SYNC_IMPLEMENTATION = win32
endif

ifeq ($(TARGETOS),os2)
DEFS += -DSDLMAME_OS2 -DNO_DEBUGGER
SYNC_IMPLEMENTATION = os2
# OS/2 can't have OpenGL (aww)
NO_OPENGL = 1
endif

#-------------------------------------------------
# object and source roots
#-------------------------------------------------

SDLSRC = $(SRC)/osd/$(OSD)
SDLOBJ = $(OBJ)/osd/$(OSD)

OBJDIRS += $(SDLOBJ)

SDLMAIN =

#-------------------------------------------------
# OSD core library
#-------------------------------------------------

OSDCOREOBJS = \
	$(SDLOBJ)/strconv.o	\
	$(SDLOBJ)/sdldir.o	\
	$(SDLOBJ)/sdlfile.o 	\
	$(SDLOBJ)/sdlos_$(TARGETOS).o	\
	$(SDLOBJ)/sdlsync_$(SYNC_IMPLEMENTATION).o     \
	$(SDLOBJ)/sdlwork.o

# any "main" must be in LIBOSD or else the build will fail!
# for the windows build, we just add it to libocore as well.
OSDOBJS = \
	$(SDLMAIN) \
	$(SDLOBJ)/sdlmain.o \
	$(SDLOBJ)/input.o \
	$(SDLOBJ)/sound.o  $(SDLOBJ)/video.o \
	$(SDLOBJ)/drawsdl.o $(SDLOBJ)/window.o $(SDLOBJ)/output.o \


#	$(SDLMAIN) \

ifdef SDL_INSTALL_ROOT
OSDOBJS += $(SDLOBJ)/draw13.o
endif

# add the debugger includes
CCOMFLAGS += -Isrc/debug

# add the prefix file
CCOMFLAGS += -include $(SDLSRC)/sdlprefix.h

ifdef NO_OPENGL
DEFS += -DUSE_OPENGL=0
LIBGL=
else
OSDOBJS += $(SDLOBJ)/drawogl.o $(SDLOBJ)/gl_shader_tool.o $(SDLOBJ)/gl_shader_mgr.o
DEFS += -DUSE_OPENGL=1
ifeq ($(TARGETOS),win32)
LIBGL=-lGL
else
ifdef USE_DISPATCH_GL
DEFS += -DUSE_DISPATCH_GL=1
else
LIBGL=-lGL
endif
endif
endif

#-------------------------------------------------
# specific configurations
#-------------------------------------------------

# Unix: add the necessary libraries
ifeq ($(TARGETOS),unix)

# override for preprocessor weirdness on PPC Linux
ifdef powerpc
CCOMFLAGS += -Upowerpc
endif

ifndef USE_DISPATCH_GL
ifdef MESA_INSTALL_ROOT
LIBS += -L$(MESA_INSTALL_ROOT)/lib
LDFLAGS += -Wl,-rpath=$(MESA_INSTALL_ROOT)/lib
CCOMFLAGS += -I$(MESA_INSTALL_ROOT)/include
endif
endif

ifndef SDL_INSTALL_ROOT
CCOMFLAGS += `sdl-config --cflags`
LIBS += -lm `sdl-config --libs` $(LIBGL)
else
CCOMFLAGS += -I$(SDL_INSTALL_ROOT)/include -D_GNU_SOURCE=1
#LIBS += -L/opt/intel/cce/9.1.051/lib  -limf -L$(SDL_INSTALL_ROOT)/lib -Wl,-rpath,$(SDL_INSTALL_ROOT)/lib -lSDL $(LIBGL)
LIBS += -lm -L$(SDL_INSTALL_ROOT)/lib -Wl,-rpath,$(SDL_INSTALL_ROOT)/lib -lSDL -lpthread $(LIBGL)
endif

ifndef NO_X11
LIBS += -lX11 -lXinerama
endif

# the new debugger relies on GTK+ in addition to the base SDLMAME needs
# Non-X11 builds can not use the debugger
ifndef NO_X11
OSDCOREOBJS += $(SDLOBJ)/debugwin.o $(SDLOBJ)/dview.o $(SDLOBJ)/debug-sup.o $(SDLOBJ)/debug-intf.o
CCOMFLAGS += `pkg-config --cflags gtk+-2.0` `pkg-config --cflags gconf-2.0`
LIBS += `pkg-config --libs gtk+-2.0` `pkg-config --libs gconf-2.0`
CCOMFLAGS += -DGTK_DISABLE_DEPRECATED
else
OSDCOREOBJS += $(SDLOBJ)/debugwin.o
endif # NO_X11

# make sure we can find X headers
CCOMFLAGS += -I/usr/X11/include -I/usr/X11R6/include -I/usr/openwin/include
# some systems still put important things in a different prefix
ifndef NO_X11
LIBS += -L/usr/X11/lib -L/usr/X11R6/lib -L/usr/openwin/lib
endif
endif # Unix

# Solaris: add the necessary object
ifeq ($(TARGETOS),solaris)
OSDCOREOBJS += $(SDLOBJ)/debugwin.o

# explicitly add some libs on Solaris
LIBS += -lSDL -lX11 -lXinerama -lm
endif # Solaris

# Win32: add the necessary libraries
ifeq ($(TARGETOS),win32)

ifdef SDL_INSTALL_ROOT
CCOMFLAGS += -I$(SDL_INSTALL_ROOT)/include
LIBS += -L$(SDL_INSTALL_ROOT)/lib
# -Wl,-rpath,$(SDL_INSTALL_ROOT)/lib -lSDL $(LIBGL)
endif


OSDCOREOBJS += $(SDLOBJ)/main.o
SDLMAIN = $(SDLOBJ)/main.o
DEFS += -Dmain=utf8_main

# enable UNICODE flags
DEFS += -DUNICODE -D_UNICODE
LDFLAGS += -municode

# at least compile some stubs to link it
OSDCOREOBJS += $(SDLOBJ)/debugwin.o

#LIBS += -lmingw32 -lSDL -lopengl32

# ensure we statically link the gcc runtime lib
LDFLAGS += -static-libgcc

# Static linking of SDL
LIBS += -Wl,-Bstatic -lSDL -Wl,-Bdynamic
LIBS += -lopengl32 -luser32 -lgdi32 -lddraw -ldsound -ldxguid -lwinmm -ladvapi32 -lcomctl32 -lshlwapi

endif	# Win32

# Mac OS X: add the necessary libraries
ifeq ($(TARGETOS),macosx)
OSDCOREOBJS += $(SDLOBJ)/osxutils.o
OSDOBJS += $(SDLOBJ)/SDLMain_tmpl.o

ifndef MACOSX_USE_LIBSDL
# Compile using framework (compile using libSDL is the exception)
LIBS += -framework SDL -framework Cocoa -framework OpenGL -lpthread
else
# Compile using installed libSDL (Fink or MacPorts):
#
# Remove the "/SDL" component from the include path so that we can compile
# files (header files are #include "SDL/something.h", so the extra "/SDL"
# causes a significant problem)
CCOMFLAGS += `sdl-config --cflags | sed 's:/SDL::'` -DNO_SDL_GLEXT
# Remove libSDLmain, as its symbols conflict with SDLMain_tmpl.m
LIBS += `sdl-config --libs | sed 's/-lSDLmain//'` -lpthread
endif

SDLMAIN = $(SDLOBJ)/SDLMain_tmpl.o

# the newest debugger uses Cocoa
OSDOBJS += $(SDLOBJ)/debugosx.o
endif	# Mac OS X

# OS2: add the necessary libraries
ifeq ($(TARGETOS),os2)
OSDCOREOBJS += $(SDLOBJ)/debugwin.o

CCOMFLAGS += `sdl-config --cflags`
LIBS += `sdl-config --libs`

# to avoid name clash of '_brk'
$(OBJ)/emu/cpu/h6280/6280dasm.o : CDEFS += -D__STRICT_ANSI__
endif # OS2

OSDCLEAN = sdlclean

TOOLS += \
	testkeys$(EXE)

# drawSDL depends on the core software renderer, so make sure it exists
$(SDLOBJ)/drawsdl.o : $(SRC)/emu/rendersw.c

$(SDLOBJ)/drawogl.o : $(SDLSRC)/drawogl.c $(SDLSRC)/texcopy.c

# draw13 depends
$(SDLOBJ)/draw13.o : $(SDLSRC)/blit13.h

# due to quirks of using /bin/sh, we need to explicitly specify the current path
CURPATH = ./

$(LIBOCORE): $(OSDCOREOBJS)

$(LIBOSD): $(OSDOBJS)

#-------------------------------------------------
# testkeys
#-------------------------------------------------

$(SDLOBJ)/testkeys.o: $(SDLSRC)/testkeys.c
	@echo Compiling $<...
	$(CC)  $(CFLAGS) $(DEFS) -c $< -o $@

TESTKEYSOBJS = \
	$(SDLOBJ)/testkeys.o \

testkeys$(EXE): $(TESTKEYSOBJS) $(LIBUTIL) $(LIBOCORE)
	@echo Linking $@...
	$(LD) $(LDFLAGS) $^ $(LIBS) -o $@

sdlclean:
	rm -f .depend

testlib:
	-echo LIBS: $(LIBS)
	-echo DEFS: $(DEFS)
	-echo CORE: $(OSDCOREOBJS)

ifneq ($(TARGETOS),win32)
depend:
	rm -f .depend
	@for i in `find src -name "*.c"` ; do \
		echo processing $$i; \
		mt=`echo $$i | sed -e "s/\\.c/\\.o/" -e "s!^src/!$(OBJ)/!"` ; \
		g++ -MM -MT $$mt $(CDEFS) $(CCOMFLAGS) $$i 2>/dev/null \
		| sed -e "s!$$i!!g" >> .depend ; \
	done

-include .depend

endif
