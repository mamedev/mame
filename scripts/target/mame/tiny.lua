-- license:BSD-3-Clause
-- copyright-holders:MAMEdev Team

---------------------------------------------------------------------------
--
--   tiny.lua
--
--   Small driver-specific example makefile
--   Use make SUBTARGET=tiny to build
--
---------------------------------------------------------------------------


--------------------------------------------------
-- Specify all the CPU cores necessary for the
-- drivers referenced in tiny.lst.
--------------------------------------------------

CPUS["Z80"] = true
CPUS["M6502"] = true
CPUS["MCS48"] = true
CPUS["MCS51"] = true
CPUS["M6800"] = true
CPUS["M6809"] = true
CPUS["M680X0"] = true
CPUS["TMS9900"] = true
CPUS["COP400"] = true

--------------------------------------------------
-- Specify all the sound cores necessary for the
-- drivers referenced in tiny.lst.
--------------------------------------------------

SOUNDS["SAMPLES"] = true
SOUNDS["DAC"] = true
SOUNDS["DISCRETE"] = true
SOUNDS["AY8910"] = true
SOUNDS["YM2151"] = true
SOUNDS["ASTROCADE"] = true
SOUNDS["TMS5220"] = true
SOUNDS["OKIM6295"] = true
SOUNDS["HC55516"] = true
SOUNDS["YM3812"] = true
SOUNDS["CEM3394"] = true
SOUNDS["VOTRAX"] = true

--------------------------------------------------
-- specify available video cores
--------------------------------------------------

--------------------------------------------------
-- specify available machine cores
--------------------------------------------------

MACHINES["6821PIA"] = true
MACHINES["TTL74148"] = true
MACHINES["TTL74153"] = true
MACHINES["TTL7474"] = true
MACHINES["RIOT6532"] = true
MACHINES["PIT8253"] = true
MACHINES["Z80CTC"] = true
MACHINES["68681"] = true
MACHINES["BANKDEV"] = true


--------------------------------------------------
-- specify available bus cores
--------------------------------------------------

BUSES["CENTRONICS"] = true

--------------------------------------------------
-- This is the list of files that are necessary
-- for building all of the drivers referenced
-- in tiny.lst
--------------------------------------------------

function createProjects_mame_tiny(_target, _subtarget)
	project ("mame_tiny")
	targetsubdir(_target .."_" .. _subtarget)
	kind (LIBTYPE)
	uuid (os.uuid("drv-mame-tiny"))
	
	options {
		"ForceCPP",
	}
	
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
		MAME_DIR .. "src/mame/machine/ticket.c",
		MAME_DIR .. "src/mame/drivers/carpolo.c",
		MAME_DIR .. "src/mame/machine/carpolo.c",
		MAME_DIR .. "src/mame/video/carpolo.c",
		MAME_DIR .. "src/mame/drivers/circus.c",
		MAME_DIR .. "src/mame/audio/circus.c",
		MAME_DIR .. "src/mame/video/circus.c",
		MAME_DIR .. "src/mame/drivers/exidy.c",
		MAME_DIR .. "src/mame/audio/exidy.c",
		MAME_DIR .. "src/mame/video/exidy.c",
		MAME_DIR .. "src/mame/audio/exidy440.c",
		MAME_DIR .. "src/mame/drivers/starfire.c",
		MAME_DIR .. "src/mame/video/starfire.c",
		MAME_DIR .. "src/mame/drivers/vertigo.c",
		MAME_DIR .. "src/mame/machine/vertigo.c",
		MAME_DIR .. "src/mame/video/vertigo.c",
		MAME_DIR .. "src/mame/drivers/victory.c",
		MAME_DIR .. "src/mame/video/victory.c",
		MAME_DIR .. "src/mame/audio/targ.c",
		MAME_DIR .. "src/mame/drivers/astrocde.c",
		MAME_DIR .. "src/mame/video/astrocde.c",
		MAME_DIR .. "src/mame/drivers/gridlee.c",
		MAME_DIR .. "src/mame/audio/gridlee.c",
		MAME_DIR .. "src/mame/video/gridlee.c",
		MAME_DIR .. "src/mame/drivers/williams.c",
		MAME_DIR .. "src/mame/machine/williams.c",
		MAME_DIR .. "src/mame/audio/williams.c",
		MAME_DIR .. "src/mame/video/williams.c",
		MAME_DIR .. "src/mame/audio/gorf.c",
		MAME_DIR .. "src/mame/audio/wow.c",
		MAME_DIR .. "src/mame/drivers/gaelco.c",
		MAME_DIR .. "src/mame/video/gaelco.c",
		MAME_DIR .. "src/mame/machine/gaelcrpt.c",
		MAME_DIR .. "src/mame/drivers/wrally.c",
		MAME_DIR .. "src/mame/machine/wrally.c",
		MAME_DIR .. "src/mame/video/wrally.c",
		MAME_DIR .. "src/mame/drivers/looping.c",
		MAME_DIR .. "src/mame/drivers/supertnk.c",
	}
end

function linkProjects_mame_tiny(_target, _subtarget)
	links {
		"mame_tiny",
	}
end
