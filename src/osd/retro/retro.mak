###########################################################################
#
#   retro.mak : based on osdmini.mak
#
#   ANDROID RETROARCH OS Dependent  makefile
#
###########################################################################
#
#   Copyright Aaron Giles
#   All rights reserved.
#
#   Redistribution and use in source and binary forms, with or without
#   modification, are permitted provided that the following conditions are
#   met:
#
#       * Redistributions of source code must retain the above copyright
#         notice, this list of conditions and the following disclaimer.
#       * Redistributions in binary form must reproduce the above copyright
#         notice, this list of conditions and the following disclaimer in
#         the documentation and/or other materials provided with the
#         distribution.
#       * Neither the name 'MAME' nor the names of its contributors may be
#         used to endorse or promote products derived from this software
#         without specific prior written permission.
#
#   THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND ANY EXPRESS OR
#   IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
#   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
#   DISCLAIMED. IN NO EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
#   INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
#   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
#   SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
#   HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
#   STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
#   IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
#   POSSIBILITY OF SUCH DAMAGE.
#
###########################################################################
DEFS += -DOSD_RETRO

CCOMFLAGS += -include $(SRC)/osd/retro/retroprefix.h

#-------------------------------------------------
# object and source roots
#-------------------------------------------------

MINISRC = $(SRC)/osd/$(OSD)
MINIOBJ = $(OBJ)/osd/$(OSD)

LIBCOOBJ = $(OBJ)/osd/$(OSD)/libretro-common/libco

OSDSRC = $(SRC)/osd
OSDOBJ = $(OBJ)/osd

OBJDIRS += $(MINIOBJ) $(LIBCOOBJ) \
	$(OSDOBJ)/modules/sync \
	$(OSDOBJ)/modules/font \
	$(OSDOBJ)/modules/lib \
	$(OSDOBJ)/modules/midi

ifeq ($(VRENDER),opengl)
GLOBJ = $(OBJ)/osd/$(OSD)/libretro-common/glsym
OBJDIRS += $(GLOBJ)
endif

#-------------------------------------------------
# OSD core library
#-------------------------------------------------

OSDCOREOBJS := \
	$(MINIOBJ)/retrodir.o \
	$(MINIOBJ)/retrofile.o \
	$(MINIOBJ)/retroos.o \
	$(MINIOBJ)/../modules/font/font_none.o \
	$(MINIOBJ)/../modules/lib/osdlib_retro.o \
	$(OSDOBJ)/modules/sync/sync_retro.o \
	$(MINIOBJ)/../modules/midi/none.o \
	$(OSDOBJ)/modules/osdmodule.o

INCPATH += -I$(SRC)/osd/retro/libretro-common/include

ifdef NOASM
OSDCOREOBJS += $(OSDOBJ)/modules/sync/work_mini.o
else
OSDCOREOBJS += $(OSDOBJ)/modules/sync/work_osd.o
endif

#-------------------------------------------------
# OSD mini library
#-------------------------------------------------

OSDOBJS = \
	$(MINIOBJ)/../modules/sound/retro_sound.o \
	$(MINIOBJ)/../modules/sound/none.o \
	$(MINIOBJ)/libretro.o \
	$(MINIOBJ)/retromain.o \
	$(MINIOBJ)/../modules/debugger/none.o \
	$(MINIOBJ)/../modules/debugger/debugint.o \
	$(MINIOBJ)/../modules/debugger/debugint.o \
	$(OSDOBJ)/modules/lib/osdobj_common.o

ifdef NO_USE_MIDI
	DEFS += -DNO_USE_MIDI
	OSDOBJS += $(OSDOBJ)/modules/midi/none.o
else
	OSDOBJS += $(OSDOBJ)/modules/midi/portmidi.o
endif

OSDOBJS += $(LIBCOOBJ)/libco.o

ifeq ($(VRENDER),opengl)
OSDOBJS += $(GLOBJ)/rglgen.o
ifeq ($(GLES), 1)
OSDOBJS += $(GLOBJ)/glsym_es2.o
else
OSDOBJS += $(GLOBJ)/glsym_gl.o
endif
endif

ifneq ($(platform),android)
LIBS += -lpthread
BASELIBS += -lpthread
endif

#-------------------------------------------------
# rules for building the libaries
#-------------------------------------------------

$(LIBOCORE): $(OSDCOREOBJS)

$(LIBOSD): $(OSDOBJS)

# Override to force libco to build as C
$(LIBCOOBJ)/%.o: $(SRC)/%.c | $(OSPREBUILD)
	@echo Compiling $< FOR C ONLY...
	$(REALCC) $(CDEFS) $(CCOMFLAGS) $(CONLYFLAGS) $(INCPATH) -c $< -o $@
ifdef CPPCHECK
	@$(CPPCHECK) $(CPPCHECKFLAGS) $<
endif

ifeq ($(armplatform), 1)
$(LIBCOOBJ)/armeabi_asm.o:
	$(REALCC) -I$(SRC)/osd/$(OSD)/libretro-common/include -c $(SRC)/osd/$(OSD)/libretro-common/libco/armeabi_asm.S -o $(LIBCOOBJ)/armeabi_asm.o
endif

