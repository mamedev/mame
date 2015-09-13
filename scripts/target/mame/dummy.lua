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
	
	options {
		"ForceCPP",
	}
	
	includedirs {
		MAME_DIR .. "src/osd",
		MAME_DIR .. "src/emu",
		MAME_DIR .. "src/devices",
		MAME_DIR .. "src/mess",
		MAME_DIR .. "src/lib",
		MAME_DIR .. "src/lib/util",
		MAME_DIR .. "3rdparty",
		GEN_DIR  .. "mess/layout",
	}

	files{
		MAME_DIR .. "src/mess/drivers/coleco.c",
		MAME_DIR .. "src/mess/machine/coleco.c",
	}
end

function linkProjects_mame_dummy(_target, _subtarget)
	links {
		"mame_dummy",
	}
end
