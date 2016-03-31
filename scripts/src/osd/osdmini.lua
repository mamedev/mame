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

project ("qtdbg_" .. _OPTIONS["osd"])
	uuid (os.uuid("qtdbg_" .. _OPTIONS["osd"]))
	kind (LIBTYPE)

	dofile("osdmini_cfg.lua")
	includedirs {
		MAME_DIR .. "src/emu",
		MAME_DIR .. "src/devices", -- accessing imagedev from debugger
		MAME_DIR .. "src/osd",
		MAME_DIR .. "src/lib",
		MAME_DIR .. "src/lib/util",
		MAME_DIR .. "src/osd/modules/render",
		MAME_DIR .. "3rdparty",
	}
	removeflags {
		"SingleOutputDir",
	}

	files {
		MAME_DIR .. "src/osd/modules/debugger/debugqt.cpp",
	}

project ("osd_" .. _OPTIONS["osd"])
	uuid (os.uuid("osd_" .. _OPTIONS["osd"]))
	kind (LIBTYPE)

	removeflags {
		"SingleOutputDir",
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
		MAME_DIR .. "src/osd/osdmini/minimain.cpp",
		MAME_DIR .. "src/osd/osdmini/osdmini.h",
		MAME_DIR .. "src/osd/osdepend.h",
		MAME_DIR .. "src/osd/modules/lib/osdobj_common.cpp",
		MAME_DIR .. "src/osd/modules/lib/osdobj_common.h",
		MAME_DIR .. "src/osd/modules/font/font_sdl.cpp",
		MAME_DIR .. "src/osd/modules/font/font_windows.cpp",
		MAME_DIR .. "src/osd/modules/font/font_dwrite.cpp",
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
		MAME_DIR .. "src/osd/modules/sound/xaudio2_sound.cpp",
		MAME_DIR .. "src/osd/modules/input/input_module.h",
		MAME_DIR .. "src/osd/modules/input/input_common.cpp",
		MAME_DIR .. "src/osd/modules/input/input_common.h",
		MAME_DIR .. "src/osd/modules/input/input_dinput.cpp",
		MAME_DIR .. "src/osd/modules/input/input_none.cpp",
		MAME_DIR .. "src/osd/modules/input/input_rawinput.cpp",
		MAME_DIR .. "src/osd/modules/input/input_win32.cpp",
		MAME_DIR .. "src/osd/modules/input/input_sdl.cpp",
		MAME_DIR .. "src/osd/modules/input/input_sdlcommon.cpp",
		MAME_DIR .. "src/osd/modules/input/input_sdlcommon.h",
		MAME_DIR .. "src/osd/modules/input/input_x11.cpp",
		MAME_DIR .. "src/osd/modules/input/input_windows.cpp",
		MAME_DIR .. "src/osd/modules/input/input_windows.h",
		MAME_DIR .. "src/osd/modules/input/input_xinput.cpp",
	}

project ("ocore_" .. _OPTIONS["osd"])
	uuid (os.uuid("ocore_" .. _OPTIONS["osd"]))
	kind (LIBTYPE)

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

	files {
		MAME_DIR .. "src/osd/osdnet.cpp",
		MAME_DIR .. "src/osd/osdnet.h",
		MAME_DIR .. "src/osd/osdcore.cpp",
		MAME_DIR .. "src/osd/osdcore.h",
		MAME_DIR .. "src/osd/modules/osdmodule.cpp",
		MAME_DIR .. "src/osd/modules/osdmodule.h",
		MAME_DIR .. "src/osd/osdmini/minidir.cpp",
		MAME_DIR .. "src/osd/osdmini/minimisc.cpp",
		MAME_DIR .. "src/osd/osdmini/minitime.cpp",
		MAME_DIR .. "src/osd/modules/file/stdfile.cpp",
		MAME_DIR .. "src/osd/modules/sync/osdsync.cpp",
		MAME_DIR .. "src/osd/modules/sync/osdsync.h",
		MAME_DIR .. "src/osd/modules/sync/work_osd.cpp",
	}
