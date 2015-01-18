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


#-------------------------------------------------
# object and source roots
#-------------------------------------------------

MINISRC = $(SRC)/osd/$(OSD)
MINIOBJ = $(OBJ)/osd/$(OSD)
LIBCOOBJ = $(OBJ)/osd/$(OSD)/libretro-sdk/libco

OSDCOMMONSRC  = $(SRC)/osd/modules/lib
OSDCOMMONOBJ = $(SRC)/osd/modules/lib

OBJDIRS += $(MINIOBJ) $(LIBCOOBJ) 

ifeq ($(VRENDER),opengl)
GLOBJ = $(OBJ)/osd/$(OSD)/libretro-sdk/glsym
OBJDIRS += $(GLOBJ)
endif

#-------------------------------------------------
# OSD core library
#-------------------------------------------------

OSDCOREOBJS := \
	$(MINIOBJ)/retrodir.o \
	$(MINIOBJ)/retrofile.o \
	$(MINIOBJ)/retromisc.o \
	$(MINIOBJ)/retrosync.o \
	$(MINIOBJ)/retrowork.o \
	$(MINIOBJ)/retroos.o \
	$(MINIOBJ)/../modules/sound/retro_sound.o \
	$(MINIOBJ)/../modules/sound/none.o \
	$(MINIOBJ)/../modules/debugger/none.o \
	$(MINIOBJ)/../modules/debugger/debugint.o \
	$(OSDCOMMONOBJ)/osdobj_common.o

#-------------------------------------------------
# OSD mini library
#-------------------------------------------------

OSDOBJS = $(LIBCOOBJ)/libco.o 

ifeq ($(VRENDER),opengl)
OSDOBJS += $(GLOBJ)/rglgen.o
ifeq ($(GLES), 1)
OSDOBJS += $(GLOBJ)/glsym_es2.o
else
OSDOBJS += $(GLOBJ)/glsym_gl.o
endif
endif

#-------------------------------------------------
# rules for building the libaries
#-------------------------------------------------

$(LIBOCORE): $(OSDCOREOBJS)

#$(LIBOSD): $(OSDOBJS)

