project ("emu")
uuid ("e6fa15e4-a354-4526-acef-13c8e80fcacf")
kind "StaticLib"
options {
	"ForceCPP",
}

includedirs {
	MAME_DIR .. "src/emu",
	MAME_DIR .. "src/lib",
	MAME_DIR .. "src/lib/util",
	MAME_DIR .. "3rdparty",
	MAME_DIR .. "3rdparty/expat/lib",
	MAME_DIR .. "3rdparty/lua/src",
	MAME_DIR .. "3rdparty/zlib",
	GEN_DIR  .. "emu",
	GEN_DIR  .. "emu/layout",
}

includeosd()

files {
	MAME_DIR .. "src/emu/*.c",
	MAME_DIR .. "src/emu/*.h",
	MAME_DIR .. "src/emu/*.png",
	MAME_DIR .. "src/emu/ui/*.c",
	MAME_DIR .. "src/emu/ui/*.h",
	MAME_DIR .. "src/emu/debug/*.c",
	MAME_DIR .. "src/emu/debug/*.h",
	MAME_DIR .. "src/emu/layout/*.lay",
	MAME_DIR .. "src/emu/sound/filter.*",
	MAME_DIR .. "src/emu/sound/flt_vol.*",
	MAME_DIR .. "src/emu/sound/flt_rc.*",
	MAME_DIR .. "src/emu/sound/wavwrite.*",
	MAME_DIR .. "src/emu/sound/samples.*",
	MAME_DIR .. "src/emu/drivers/*.*",
	MAME_DIR .. "src/emu/machine/bcreader.*",
	MAME_DIR .. "src/emu/machine/buffer.*",
	MAME_DIR .. "src/emu/machine/clock.*",
	MAME_DIR .. "src/emu/machine/generic.*",
	MAME_DIR .. "src/emu/machine/keyboard.*",
	MAME_DIR .. "src/emu/machine/laserdsc.*",
	MAME_DIR .. "src/emu/machine/latch.*",
	MAME_DIR .. "src/emu/machine/netlist.*",
	MAME_DIR .. "src/emu/machine/nvram.*",
	MAME_DIR .. "src/emu/machine/ram.*",
	MAME_DIR .. "src/emu/machine/legscsi.*",
	MAME_DIR .. "src/emu/machine/terminal.*",
	MAME_DIR .. "src/emu/imagedev/*.*",
	MAME_DIR .. "src/emu/video/generic.*",
	MAME_DIR .. "src/emu/video/resnet.*",
	MAME_DIR .. "src/emu/video/rgbutil.*",
	MAME_DIR .. "src/emu/video/vector.*",
	MAME_DIR .. "src/osd/osdnet.*",
}


function emuProject(_target, _subtarget)

	disasm_files = { }

	project ("optional")
	uuid (os.uuid("optional-" .. _target .."_" .. _subtarget))
	kind "StaticLib"
	targetsubdir(_target .."_" .. _subtarget)
	options {
		"ForceCPP",
		"ArchiveSplit",
	}

	includedirs {
		MAME_DIR .. "src/emu",
		MAME_DIR .. "src/mame", -- used for sound amiga
		MAME_DIR .. "src/lib",
		MAME_DIR .. "src/lib/util",
		MAME_DIR .. "3rdparty",
		MAME_DIR .. "3rdparty/expat/lib",
		MAME_DIR .. "3rdparty/lua/src",
		MAME_DIR .. "3rdparty/zlib",
		GEN_DIR  .. "emu",
		GEN_DIR  .. "emu/layout",
		MAME_DIR .. "src/emu/cpu/m68000",
		GEN_DIR .. "emu/cpu/m68000",
	}
	includeosd()
	
	dofile(path.join("src", "cpu.lua"))

	dofile(path.join("src", "sound.lua"))
	
	files {
		MAME_DIR .. "src/emu/netlist/**.*",
	}
	
	dofile(path.join("src", "video.lua"))

	dofile(path.join("src", "machine.lua"))

	
	project ("bus")
	uuid ("5d782c89-cf7e-4cfe-8f9f-0d4bfc16c91d")
	kind "StaticLib"
	targetsubdir(_target .."_" .. _subtarget)
	options {
		"ForceCPP",
		"ArchiveSplit",
	}

	includedirs {
		MAME_DIR .. "src/emu",
		MAME_DIR .. "src/lib",
		MAME_DIR .. "src/lib/util",
		MAME_DIR .. "3rdparty",
		MAME_DIR .. "3rdparty/expat/lib",
		MAME_DIR .. "3rdparty/lua/src",
		MAME_DIR .. "3rdparty/zlib",
		MAME_DIR .. "src/mess", -- some mess bus devices need this
		MAME_DIR .. "src/mame", -- used for nes bus devices
		GEN_DIR  .. "emu",
		GEN_DIR  .. "emu/layout",
	}

	includeosd()
	
	dofile(path.join("src", "bus.lua"))
	
	
	project ("dasm")
	uuid ("f2d28b0a-6da5-4f78-b629-d834aa00429d")
	kind "StaticLib"
	targetsubdir(_target .."_" .. _subtarget)
	options {
		"ForceCPP",
	}

	includedirs {
		MAME_DIR .. "src/emu",
		MAME_DIR .. "src/lib",
		MAME_DIR .. "src/lib/util",
		MAME_DIR .. "3rdparty",
		MAME_DIR .. "3rdparty/expat/lib",
		MAME_DIR .. "3rdparty/lua/src",
		MAME_DIR .. "3rdparty/zlib",
		GEN_DIR  .. "emu",
	}
	
	includeosd()
	
	files {
		disasm_files
	}
end