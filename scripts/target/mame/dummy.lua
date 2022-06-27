-- license:BSD-3-Clause
-- copyright-holders:MAMEdev Team

---------------------------------------------------------------------------
--
--   dummy.lua
--
--   Dummy target makefile
--
---------------------------------------------------------------------------

dofile("arcade.lua")
dofile("mess.lua")

function createProjects_mame_dummy(_target, _subtarget)
	project ("mame_dummy")
	targetsubdir(_target .."_" .. _subtarget)
	kind (LIBTYPE)
	uuid (os.uuid("drv-mame_dummy"))
	addprojectflags()
	precompiledheaders_novs()

	includedirs {
		MAME_DIR .. "src/osd",
		MAME_DIR .. "src/emu",
		MAME_DIR .. "src/devices",
		MAME_DIR .. "src/mame/shared",
		MAME_DIR .. "src/lib",
		MAME_DIR .. "src/lib/util",
		MAME_DIR .. "3rdparty",
		GEN_DIR  .. "mame/layout",
	}

files{
	MAME_DIR .. "src/mame/coleco/coleco.cpp",
	MAME_DIR .. "src/mame/coleco/coleco.h",
	MAME_DIR .. "src/mame/coleco/coleco_m.cpp",
	MAME_DIR .. "src/mame/coleco/coleco_m.h",
}
end

function linkProjects_mame_dummy(_target, _subtarget)
	links {
		"mame_dummy",
	}
end
