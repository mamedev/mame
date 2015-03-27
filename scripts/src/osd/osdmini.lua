project ("osd_" .. _OPTIONS["osd"])
	uuid (os.uuid("osd_" .. _OPTIONS["osd"]))
	kind "StaticLib"

	removeflags {
		"SingleOutputDir",
	}
	
	options {
		"ForceCPP",
	}

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
		MAME_DIR .. "src/osd/osdmini/minimain.c",
		MAME_DIR .. "src/osd/modules/lib/osdobj_common.*",
		MAME_DIR .. "src/osd/modules/font/font_sdl.*",
		MAME_DIR .. "src/osd/modules/font/font_windows.*",
		MAME_DIR .. "src/osd/modules/font/font_osx.*",
		MAME_DIR .. "src/osd/modules/font/font_none.*",
		MAME_DIR .. "src/osd/modules/netdev/taptun.*",
		MAME_DIR .. "src/osd/modules/netdev/pcap.*",
		MAME_DIR .. "src/osd/modules/netdev/none.*",
		MAME_DIR .. "src/osd/modules/midi/portmidi.*",
		MAME_DIR .. "src/osd/modules/midi/none.*",
		MAME_DIR .. "src/osd/modules/sound/js_sound.*",
		MAME_DIR .. "src/osd/modules/sound/direct_sound.*",
		MAME_DIR .. "src/osd/modules/sound/sdl_sound.*",
		MAME_DIR .. "src/osd/modules/sound/none.*",
	}
	
project ("ocore_" .. _OPTIONS["osd"])
	uuid (os.uuid("ocore_" .. _OPTIONS["osd"]))
	kind "StaticLib"

	options {
		"ForceCPP",
	}

	removeflags {
		"SingleOutputDir",	
	}

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
		MAME_DIR .. "src/osd/modules/osdmodule.*",
		MAME_DIR .. "src/osd/osdmini/minidir.*",
		MAME_DIR .. "src/osd/osdmini/minifile.*",
		MAME_DIR .. "src/osd/osdmini/minimisc.*",
		MAME_DIR .. "src/osd/osdmini/minisync.*",
		MAME_DIR .. "src/osd/osdmini/minitime.*",
		MAME_DIR .. "src/osd/modules/sync/work_mini.*",
	}
