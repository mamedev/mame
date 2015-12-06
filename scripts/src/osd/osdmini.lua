-- license:BSD-3-Clause
-- copyright-holders:MAMEdev Team

---------------------------------------------------------------------------
--
--   osdmini.lua
--
--   Rules for the building of osdmini
--
---------------------------------------------------------------------------

function maintargetosdoptions(_target,_subtarget)
end


project ("osd_" .. _OPTIONS["osd"])
	uuid (os.uuid("osd_" .. _OPTIONS["osd"]))
	kind (LIBTYPE)

	removeflags {
		"SingleOutputDir",
	}
	
	addprojectflags()

	dofile("osdmini_cfg.lua")
	
	includedirs {
		MAME_DIR .. "src/emu",
		MAME_DIR .. "src/osd",
		MAME_DIR .. "src/lib",
		MAME_DIR .. "src/lib/util",
		MAME_DIR .. "src/osd/sdl",
		MAME_DIR .. "src/osd/modules/render",
		MAME_DIR .. "3rdparty",
		MAME_DIR .. "3rdparty/winpcap/Include",
		MAME_DIR .. "3rdparty/bgfx/include",
		MAME_DIR .. "3rdparty/bx/include",
	}

	files {
		MAME_DIR .. "src/osd/osdnet.cpp",
		MAME_DIR .. "src/osd/osdnet.h",
		MAME_DIR .. "src/osd/osdmini/minimain.cpp",
		MAME_DIR .. "src/osd/osdmini/osdmini.h",
		MAME_DIR .. "src/osd/osdepend.h",
		MAME_DIR .. "src/osd/modules/lib/osdobj_common.cpp",
		MAME_DIR .. "src/osd/modules/lib/osdobj_common.h",
		MAME_DIR .. "src/osd/modules/font/font_sdl.cpp",
		MAME_DIR .. "src/osd/modules/font/font_windows.cpp",
		MAME_DIR .. "src/osd/modules/font/font_osx.cpp",
		MAME_DIR .. "src/osd/modules/font/font_none.cpp",
		MAME_DIR .. "src/osd/modules/netdev/taptun.cpp",
		MAME_DIR .. "src/osd/modules/netdev/pcap.cpp",
		MAME_DIR .. "src/osd/modules/netdev/none.cpp",
		MAME_DIR .. "src/osd/modules/midi/portmidi.cpp",
		MAME_DIR .. "src/osd/modules/midi/none.cpp",
		MAME_DIR .. "src/osd/modules/sound/js_sound.cpp",
		MAME_DIR .. "src/osd/modules/sound/direct_sound.cpp",
		MAME_DIR .. "src/osd/modules/sound/coreaudio_sound.cpp",		
		MAME_DIR .. "src/osd/modules/sound/sdl_sound.cpp",
		MAME_DIR .. "src/osd/modules/sound/none.cpp",
	}
	
project ("ocore_" .. _OPTIONS["osd"])
	uuid (os.uuid("ocore_" .. _OPTIONS["osd"]))
	kind (LIBTYPE)

	removeflags {
		"SingleOutputDir",	
	}

	addprojectflags()

	dofile("osdmini_cfg.lua")
	
	includedirs {
		MAME_DIR .. "src/emu",
		MAME_DIR .. "src/osd",
		MAME_DIR .. "src/lib",
		MAME_DIR .. "src/lib/util",
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
		MAME_DIR .. "src/osd/osdcore.cpp",
		MAME_DIR .. "src/osd/osdcore.h",
		MAME_DIR .. "src/osd/modules/osdmodule.cpp",
		MAME_DIR .. "src/osd/modules/osdmodule.h",		
		MAME_DIR .. "src/osd/osdmini/minidir.cpp",
		MAME_DIR .. "src/osd/osdmini/minifile.cpp",
		MAME_DIR .. "src/osd/osdmini/minimisc.cpp",
		MAME_DIR .. "src/osd/osdmini/minisync.cpp",
		MAME_DIR .. "src/osd/osdmini/minitime.cpp",
		MAME_DIR .. "src/osd/modules/sync/work_mini.cpp",
	}
