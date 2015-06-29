function maintargetosdoptions(_target)
end

newoption {
	trigger = "NO_USE_MIDI",
	description = "Disable MIDI I/O",
	allowed = {
	--	{ "0",  "Enable MIDI"  },
		{ "1",  "Disable MIDI" },
	},
}

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
		MAME_DIR .. "src/osd/retro/retromain.c",

		-- The public API in libretro.c is "unused" and tends to get
		-- stripped by the "helpful" linker, so we compile it into
		-- the top-level shared lib to avoid having to jump through
		-- quite as many hoops.
		-- MAME_DIR .. "src/osd/retro/libretro.c",
	}

	-- We don't support MIDI at present
	--if _OPTIONS["NO_USE_MIDI"]=="1" then
		defines {
			"NO_USE_MIDI",
		}

		files {
			MAME_DIR .. "src/osd/modules/midi/none.c",
		}
	--else
	--	files {
	--		MAME_DIR .. "src/osd/modules/midi/portmidi.c",
	--	}
	--end

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
