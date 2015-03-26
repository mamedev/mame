---------------------------------------------------------------------------
--
--   ldplayer.lua
--
--   Small makefile to build a standalone laserdisc player
--
--   Copyright Nicola Salmoria and the MAME Team.
--   Visit http://mamedev.org for licensing and usage restrictions.
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

function createProjects(_target, _subtarget)
	project ("drvldplayer")
	targetsubdir(_target .."_" .. _subtarget)
	kind "StaticLib"
	uuid (os.uuid("drvldplayer"))
	
	options {
		"ForceCPP",
	}
	
	includedirs {
		MAME_DIR .. "src/emu",
		MAME_DIR .. "src/mame",
		MAME_DIR .. "src/lib",
		MAME_DIR .. "src/lib/util",
		MAME_DIR .. "3rdparty",
		MAME_DIR .. "3rdparty/zlib",
		GEN_DIR  .. "mame/layout",
	}	

	includeosd()

	files{
		MAME_DIR .. "src/emu/drivers/emudummy.c",
	}
end

function linkProjects(_target, _subtarget)
	links {
		"drvldplayer",
	}
end