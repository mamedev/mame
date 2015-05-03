function maintargetosdoptions(_target)
end


project ("osd_" .. _OPTIONS["osd"])
	uuid (os.uuid("osd_" .. _OPTIONS["osd"]))
	kind "StaticLib"

	removeflags {
		"SingleOutputDir",
	}
	
	options {
		"ForceCPP",
	}

	dofile("retro_cfg.lua")
	
	includedirs {
		MAME_DIR .. "3rdparty",
		MAME_DIR .. "3rdparty/bx/include",
		MAME_DIR .. "3rdparty/winpcap/Include",
		MAME_DIR .. "src/emu",
		MAME_DIR .. "src/lib",
		MAME_DIR .. "src/lib/util",
		MAME_DIR .. "src/osd",
		MAME_DIR .. "src/osd/modules/render",
		MAME_DIR .. "src/osd/retro",
		MAME_DIR .. "src/osd/retro/libretro-common/include",
	}

	files {
 		MAME_DIR .. "src/osd/osdnet.c",
 		MAME_DIR .. "src/osd/modules/netdev/taptun.c",
		MAME_DIR .. "src/osd/modules/netdev/pcap.c",
 		MAME_DIR .. "src/osd/modules/netdev/none.c",
		MAME_DIR .. "src/osd/modules/debugger/debugint.c",
		MAME_DIR .. "src/osd/modules/debugger/none.c",
		MAME_DIR .. "src/osd/modules/lib/osdobj_common.c",
		MAME_DIR .. "src/osd/modules/sound/none.c",
		MAME_DIR .. "src/osd/modules/sound/retro_sound.c",
		MAME_DIR .. "src/osd/retro/libretro.c",
		MAME_DIR .. "src/osd/retro/retromain.c",
	}

	if _OPTIONS["NO_USE_MIDI"]=="1" then
		files {
			MAME_DIR .. "src/osd/modules/midi/none.c",
		}
	else
		files {
			MAME_DIR .. "src/osd/modules/midi/portmidi.c",
		}
	end

	if _OPTIONS["NO_OPENGL"]=="1" then
		files {
			MAME_DIR .. "src/osd/retro/libretro-common/glsym/rglgen.c",
			MAME_DIR .. "src/osd/retro/libretro-common/glsym/glsym_gl.c",
		}
	end
	
project ("ocore_" .. _OPTIONS["osd"])
	uuid (os.uuid("ocore_" .. _OPTIONS["osd"]))
	kind "StaticLib"

	options {
		"ForceCPP",
	}

	removeflags {
		"SingleOutputDir",	
	}

   dofile("retro_cfg.lua")
	
	includedirs {
		MAME_DIR .. "src/emu",
		MAME_DIR .. "src/osd",
		MAME_DIR .. "src/lib",
		MAME_DIR .. "src/lib/util",
		MAME_DIR .. "src/osd/retro",
		MAME_DIR .. "src/osd/retro/libretro-common/include",
	}

	if _OPTIONS["targetos"]=="linux" then
		BASE_TARGETOS = "unix"
		SDLOS_TARGETOS = "unix"
		SYNC_IMPLEMENTATION = "tc"
	end

	if _OPTIONS["targetos"]=="windows" then
		BASE_TARGETOS = "win32"
		SDLOS_TARGETOS = "win32"
		SYNC_IMPLEMENTATION = "windows"
	end

	if _OPTIONS["targetos"]=="macosx" then
		BASE_TARGETOS = "unix"
		SDLOS_TARGETOS = "macosx"
		SYNC_IMPLEMENTATION = "ntc"
	end

	files {
 		MAME_DIR .. "src/osd/osdcore.c",
 		MAME_DIR .. "src/osd/modules/osdmodule.c",
		MAME_DIR .. "src/osd/modules/font/font_none.c",
		MAME_DIR .. "src/osd/modules/lib/osdlib_retro.c",
		MAME_DIR .. "src/osd/modules/midi/none.c",
		MAME_DIR .. "src/osd/modules/osdmodule.c",
		MAME_DIR .. "src/osd/modules/sync/sync_retro.c",
		MAME_DIR .. "src/osd/retro/retrodir.c",
		MAME_DIR .. "src/osd/retro/retrofile.c",
		MAME_DIR .. "src/osd/retro/retroos.c",
	}

	if _OPTIONS["NOASM"]=="1" then
		files {
			MAME_DIR .. "src/osd/modules/sync/work_mini.c",
		}
	else
		files {
			MAME_DIR .. "src/osd/modules/sync/work_osd.c",
		}
	end

project ("libco")
	uuid (os.uuid("libco"))
	kind "StaticLib"

	includedirs {
		MAME_DIR .. "src/osd/retro/libretro-common/include",
	}

	files {
		MAME_DIR .. "src/osd/retro/libretro-common/libco/libco.c",
	}

-- ###########################################################################
-- #
-- #   retro.mak : based on osdmini.mak
-- #
-- ###########################################################################
--X DEFS += -DOSD_RETRO
-- 
-- CCOMFLAGS += -include $(SRC)/osd/retro/retroprefix.h
-- 
-- #-------------------------------------------------
-- # object and source roots
-- #-------------------------------------------------
-- 
-- MINISRC = $(SRC)/osd/$(OSD)
-- MINIOBJ = $(OBJ)/osd/$(OSD)
-- 
-- LIBCOOBJ = $(MINIOBJ)/libretro-common/libco
-- 
-- OSDSRC = $(SRC)/osd
-- OSDOBJ = $(OBJ)/osd
-- 
-- OBJDIRS += $(MINIOBJ) \
-- 	$(OSDOBJ)/modules/sync \
-- 	$(OSDOBJ)/modules/lib \
-- 	$(OSDOBJ)/modules/midi \
-- 	$(OSDOBJ)/modules/font \
-- 	$(LIBCOOBJ)
-- 
-- 
-- ifeq ($(VRENDER),opengl)
-- GLOBJ = $(OBJ)/osd/$(OSD)/libretro-common/glsym
-- OBJDIRS += $(GLOBJ)
-- endif
-- 
-- #-------------------------------------------------
-- # OSD core library
-- #-------------------------------------------------
-- 
--X OSDCOREOBJS := \
--X 	$(MINIOBJ)/retrodir.o \
--X 	$(MINIOBJ)/retrofile.o \
--X 	$(MINIOBJ)/retroos.o \
--X 	$(OSDOBJ)/modules/font/font_none.o \
--X 	$(OSDOBJ)/modules/lib/osdlib_retro.o \
--X 	$(OSDOBJ)/modules/sync/sync_retro.o \
--X 	$(OSDOBJ)/modules/midi/none.o \
--X 	$(OSDOBJ)/modules/osdmodule.o \
-- 
-- 
-- INCPATH += -I$(SRC)/osd/retro/libretro-common/include
-- 
--X ifdef NOASM
--X OSDCOREOBJS += $(OSDOBJ)/modules/sync/work_mini.o
--X else
--X OSDCOREOBJS += $(OSDOBJ)/modules/sync/work_osd.o
--X endif
-- 
-- #-------------------------------------------------
-- # OSD mini library
-- #-------------------------------------------------
-- 
--X OSDOBJS = \
--X 	$(MINIOBJ)/libretro.o \
--X 	$(MINIOBJ)/retromain.o \
--X 	$(OSDOBJ)/modules/lib/osdobj_common.o  \
--X 	$(OSDOBJ)/modules/sound/retro_sound.o \
--X 	$(OSDOBJ)/modules/sound/none.o \
--X 	$(OSDOBJ)/modules/debugger/debugint.o \
--X 	$(OSDOBJ)/modules/debugger/none.o \
-- 
-- 
--X ifdef NO_USE_MIDI
--X 	DEFS += -DNO_USE_MIDI
--X 	OSDOBJS += $(OSDOBJ)/modules/midi/none.o
--X else
--X 	OSDOBJS += $(OSDOBJ)/modules/midi/portmidi.o
--X endif
-- 
--X OSDOBJS += $(LIBCOOBJ)/libco.o
-- 
--X ifeq ($(VRENDER),opengl)
--X OSDOBJS += $(GLOBJ)/rglgen.o
-- ifeq ($(GLES), 1)
-- OSDOBJS += $(GLOBJ)/glsym_es2.o
--X else
--X OSDOBJS += $(GLOBJ)/glsym_gl.o
--X endif
--X endif
-- 
-- ifneq ($(platform),android)
-- LIBS += -lpthread
-- BASELIBS += -lpthread
-- endif
-- 
-- #-------------------------------------------------
-- # rules for building the libaries
-- #-------------------------------------------------
-- 
--X $(LIBOCORE): $(OSDCOREOBJS)
-- 
--X $(LIBOSD): $(OSDOBJS)
-- 
--X # Override to force libco to build as C
--X $(LIBCOOBJ)/%.o: $(SRC)/%.c | $(OSPREBUILD)
--X 	@echo Compiling $< FOR C ONLY...
--X 	$(REALCC) $(CDEFS) $(CCOMFLAGS) $(CONLYFLAGS) $(INCPATH) -c $< -o $@
--X ifdef CPPCHECK
--X 	@$(CPPCHECK) $(CPPCHECKFLAGS) $<
--X endif
-- 
--X ifeq ($(armplatform), 1)
--X $(LIBCOOBJ)/armeabi_asm.o:
--X 	$(REALCC) -I$(SRC)/osd/$(OSD)/libretro-common/include -c $(SRC)/osd/$(OSD)/libretro-common/libco/armeabi_asm.S -o $(LIBCOOBJ)/armeabi_asm.o
--X endif
