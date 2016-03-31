-- license:BSD-3-Clause
-- copyright-holders:MAMEdev Team

---------------------------------------------------------------------------
--
--   ldplayer.lua
--
--   Small makefile to build a standalone laserdisc player
--
---------------------------------------------------------------------------

--------------------------------------------------
-- specify required CPU cores (none)
--------------------------------------------------

CPUS["MCS48"] = true
CPUS["Z80"] = true



--------------------------------------------------
-- specify required sound cores
--------------------------------------------------

SOUNDS["WAVE"] = true


--------------------------------------------------
-- specify available video cores
--------------------------------------------------

--------------------------------------------------
-- specify available machine cores
--------------------------------------------------

MACHINES["LDV1000"] = true
MACHINES["LDPR8210"] = true

--------------------------------------------------
-- specify available bus cores
--
-- MIDI is here as dummy bus to allow libbus.a to
-- be created on OSX.
--------------------------------------------------

BUSES["MIDI"] = true

--------------------------------------------------
-- this is the list of driver libraries that
-- comprise MAME plus mamedriv.o which contains
-- the list of drivers
--------------------------------------------------

function createProjects_ldplayer_ldplayer(_target, _subtarget)
	project ("drvldplayer")
	targetsubdir(_target .."_" .. _subtarget)
	kind (LIBTYPE)
	uuid (os.uuid("drvldplayer"))

	includedirs {
		MAME_DIR .. "src/osd",
		MAME_DIR .. "src/emu",
		MAME_DIR .. "src/devices",
		MAME_DIR .. "src/mame",
		MAME_DIR .. "src/lib",
		MAME_DIR .. "src/lib/util",
		MAME_DIR .. "3rdparty",
		GEN_DIR  .. "mame/layout",
	}

	files{
		MAME_DIR .. "src/emu/drivers/emudummy.cpp",
	}

	dependency {
		{ MAME_DIR .. "src/emu/drivers/emudummy.cpp", GEN_DIR .. "ldplayer/layout/pr8210.lh" },
	}

	custombuildtask {
		layoutbuildtask("ldplayer/layout", "pr8210"),
	}
end

function linkProjects_ldplayer_ldplayer(_target, _subtarget)
	links {
		"drvldplayer",
	}
end
