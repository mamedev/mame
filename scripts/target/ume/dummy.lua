---------------------------------------------------------------------------
--
--   ume.lua
--
--   Universal target makefile
--
---------------------------------------------------------------------------

dofile("../mess/mess.lua")
dofile("../mame/mame.lua")

function createProjects_ume_dummy(_target, _subtarget)
	project ("ume_dummy")
	targetsubdir(_target .."_" .. _subtarget)
	kind "StaticLib"
	uuid (os.uuid("drv-ume_dummy"))
	
	options {
		"ForceCPP",
	}
	
	includedirs {
		MAME_DIR .. "src/osd",
		MAME_DIR .. "src/emu",
		MAME_DIR .. "src/mess",
		MAME_DIR .. "src/lib",
		MAME_DIR .. "src/lib/util",
		MAME_DIR .. "3rdparty",
		MAME_DIR .. "3rdparty/zlib",
		GEN_DIR  .. "mess/layout",
	}	

	files{
		MAME_DIR .. "src/mess/drivers/coleco.c",
		MAME_DIR .. "src/mess/machine/coleco.c",
	}
end

function linkProjects_ume_dummy(_target, _subtarget)
	links {
		"ume_dummy",
	}
end