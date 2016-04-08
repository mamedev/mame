-- license:BSD-3-Clause
-- copyright-holders:MAMEdev Team

---------------------------------------------------------------------------
--
--   nl.lua
--
--   Compiles all drivers using netlist code
--   Use make SUBTARGET=nl to build
--
---------------------------------------------------------------------------


--------------------------------------------------
-- Specify all the CPU cores necessary for the
-- drivers referenced in nl.lst.
--------------------------------------------------

CPUS["Z80"] = true
CPUS["M6800"] = true
CPUS["M6803"] = true
CPUS["M6809"] = true
--CPUS["MCS48"] = true
--CPUS["MCS51"] = true
--CPUS["M6800"] = true
--CPUS["M6809"] = true
--CPUS["M680X0"] = true
--CPUS["TMS9900"] = true
--CPUS["COP400"] = true

--------------------------------------------------
-- Specify all the sound cores necessary for the
-- drivers referenced in nl.lst.
--------------------------------------------------

--SOUNDS["SAMPLES"] = true
SOUNDS["DAC"] = true
SOUNDS["DISCRETE"] = true
SOUNDS["AY8910"] = true
SOUNDS["MSM5205"] = true
--SOUNDS["ASTROCADE"] = true
--SOUNDS["TMS5220"] = true
--SOUNDS["OKIM6295"] = true
--SOUNDS["HC55516"] = true
--SOUNDS["YM3812"] = true
--SOUNDS["CEM3394"] = true
--SOUNDS["VOTRAX"] = true

--------------------------------------------------
-- specify available video cores
--------------------------------------------------

VIDEOS["FIXFREQ"] = true

--------------------------------------------------
-- specify available machine cores
--------------------------------------------------

MACHINES["NETLIST"] = true
--MACHINES["6821PIA"] = true
--MACHINES["TTL74148"] = true
--MACHINES["TTL74153"] = true
--MACHINES["TTL7474"] = true
--MACHINES["RIOT6532"] = true
--MACHINES["PIT8253"] = true
--MACHINES["Z80CTC"] = true
--MACHINES["68681"] = true
--MACHINES["BANKDEV"] = true


--------------------------------------------------
-- specify available bus cores
--------------------------------------------------

-- not needed by nl.lua but build system wants at least one bus
BUSES["CENTRONICS"] = true

--------------------------------------------------
-- This is the list of files that are necessary
-- for building all of the drivers referenced
-- in nl.lst
--------------------------------------------------

function createProjects_mame_nl(_target, _subtarget)
	project ("mame_netlist")
	targetsubdir(_target .."_" .. _subtarget)
	kind (LIBTYPE)
	uuid (os.uuid("drv-mame-nl"))
  addprojectflags()
  precompiledheaders()

	includedirs {
		MAME_DIR .. "src/osd",
		MAME_DIR .. "src/emu",
		MAME_DIR .. "src/devices",
		MAME_DIR .. "src/lib/netlist",
		MAME_DIR .. "src/mame",
		MAME_DIR .. "src/lib",
		MAME_DIR .. "src/lib/util",
		MAME_DIR .. "3rdparty",
		GEN_DIR  .. "mame/layout",
	}

files{
	MAME_DIR .. "src/mame/drivers/pong.cpp",
	MAME_DIR .. "src/mame/drivers/nl_pong.cpp",
	MAME_DIR .. "src/mame/drivers/nl_pongd.cpp",
	MAME_DIR .. "src/mame/drivers/nl_breakout.cpp",

	MAME_DIR .. "src/mame/drivers/1942.cpp",
	MAME_DIR .. "src/mame/includes/1942.h",
	MAME_DIR .. "src/mame/video/1942.cpp",
	MAME_DIR .. "src/mame/drivers/popeye.cpp",
	MAME_DIR .. "src/mame/includes/popeye.h",
	MAME_DIR .. "src/mame/video/popeye.cpp",
	
  MAME_DIR .. "src/mame/drivers/m62.cpp",
  MAME_DIR .. "src/mame/includes/m62.h",
  MAME_DIR .. "src/mame/video/m62.cpp",
  MAME_DIR .. "src/mame/audio/irem.cpp",
  MAME_DIR .. "src/mame/audio/irem.h",

}
end

function linkProjects_mame_nl(_target, _subtarget)
	links {
		"mame_netlist",
	}
end
