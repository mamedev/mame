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

CPUS["COP400"] = true
CPUS["M6502"] = true
CPUS["M6800"] = true
CPUS["M6805"] = true
CPUS["M6809"] = true
CPUS["M680X0"] = true
CPUS["MCS48"] = true
CPUS["MCS51"] = true
CPUS["TMS9900"] = true
CPUS["Z80"] = true

--------------------------------------------------
-- Specify all the sound cores necessary for the
-- drivers referenced in tiny.lst.
--------------------------------------------------

SOUNDS["ASTROCADE"] = true
SOUNDS["AY8910"] = true
SOUNDS["CEM3394"] = true
SOUNDS["DAC"] = true
SOUNDS["DISCRETE"] = true
SOUNDS["HC55516"] = true
SOUNDS["OKIM6295"] = true
SOUNDS["SAMPLES"] = true
SOUNDS["TMS5220"] = true
SOUNDS["VOTRAX"] = true
SOUNDS["YM2151"] = true
SOUNDS["YM3812"] = true


--------------------------------------------------
-- specify available video cores
--------------------------------------------------

VIDEOS["MC6845"] = true


--------------------------------------------------
-- specify available machine cores
--------------------------------------------------

MACHINES["6821PIA"] = true
MACHINES["68681"] = true
MACHINES["ADC0808"] = true
MACHINES["BANKDEV"] = true
MACHINES["GEN_LATCH"] = true
MACHINES["INPUT_MERGER"] = true
MACHINES["NETLIST"] = true
MACHINES["OUTPUT_LATCH"] = true
MACHINES["PIT8253"] = true
MACHINES["RIOT6532"] = true
MACHINES["TICKET"] = true
MACHINES["TIMEKPR"] = true
MACHINES["TTL74148"] = true
MACHINES["TTL74153"] = true
MACHINES["TTL74157"] = true
MACHINES["TTL74259"] = true
MACHINES["TTL7474"] = true
MACHINES["WATCHDOG"] = true
MACHINES["Z80CTC"] = true
MACHINES["Z80DAISY"] = true
MACHINES["Z80PIO"] = true


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
	addprojectflags()
	precompiledheaders_novs()

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
	MAME_DIR .. "src/mame/audio/nl_carpolo.cpp",
	MAME_DIR .. "src/mame/audio/nl_carpolo.h",
	MAME_DIR .. "src/mame/drivers/carpolo.cpp",
	MAME_DIR .. "src/mame/includes/carpolo.h",
	MAME_DIR .. "src/mame/machine/carpolo.cpp",
	MAME_DIR .. "src/mame/video/carpolo.cpp",
	MAME_DIR .. "src/mame/audio/circus.cpp",
	MAME_DIR .. "src/mame/drivers/circus.cpp",
	MAME_DIR .. "src/mame/includes/circus.h",
	MAME_DIR .. "src/mame/video/circus.cpp",
	MAME_DIR .. "src/mame/drivers/exidy.cpp",
	MAME_DIR .. "src/mame/audio/exidy.cpp",
	MAME_DIR .. "src/mame/audio/exidy.h",
	MAME_DIR .. "src/mame/audio/exidy440.cpp",
	MAME_DIR .. "src/mame/audio/exidy440.h",
	MAME_DIR .. "src/mame/audio/nl_fireone.h",
	MAME_DIR .. "src/mame/audio/nl_fireone.cpp",
	MAME_DIR .. "src/mame/audio/nl_starfire.h",
	MAME_DIR .. "src/mame/audio/nl_starfire.cpp",
	MAME_DIR .. "src/mame/drivers/starfire.cpp",
	MAME_DIR .. "src/mame/includes/starfire.h",
	MAME_DIR .. "src/mame/video/starfire.cpp",
	MAME_DIR .. "src/mame/drivers/vertigo.cpp",
	MAME_DIR .. "src/mame/includes/vertigo.h",
	MAME_DIR .. "src/mame/machine/vertigo.cpp",
	MAME_DIR .. "src/mame/video/vertigo.cpp",
	MAME_DIR .. "src/mame/drivers/victory.cpp",
	MAME_DIR .. "src/mame/includes/victory.h",
	MAME_DIR .. "src/mame/video/victory.cpp",
	MAME_DIR .. "src/mame/drivers/astrocde.cpp",
	MAME_DIR .. "src/mame/includes/astrocde.h",
	MAME_DIR .. "src/mame/video/astrocde.cpp",
	MAME_DIR .. "src/mame/drivers/gridlee.cpp",
	MAME_DIR .. "src/mame/includes/gridlee.h",
	MAME_DIR .. "src/mame/audio/gridlee.cpp",
	MAME_DIR .. "src/mame/video/gridlee.cpp",
	MAME_DIR .. "src/mame/audio/s11c_bg.cpp",
	MAME_DIR .. "src/mame/audio/s11c_bg.h",
	MAME_DIR .. "src/mame/audio/williams.cpp",
	MAME_DIR .. "src/mame/audio/williams.h",
	MAME_DIR .. "src/mame/drivers/williams.cpp",
	MAME_DIR .. "src/mame/includes/williams.h",
	MAME_DIR .. "src/mame/machine/williams.cpp",
	MAME_DIR .. "src/mame/video/williams.cpp",
	MAME_DIR .. "src/mame/drivers/gaelco.cpp",
	MAME_DIR .. "src/mame/includes/gaelco.h",
	MAME_DIR .. "src/mame/video/gaelco.cpp",
	MAME_DIR .. "src/mame/machine/gaelcrpt.cpp",
	MAME_DIR .. "src/mame/drivers/wrally.cpp",
	MAME_DIR .. "src/mame/includes/wrally.h",
	MAME_DIR .. "src/mame/video/wrally.cpp",
	MAME_DIR .. "src/mame/video/gaelco_wrally_sprites.cpp",
	MAME_DIR .. "src/mame/video/gaelco_wrally_sprites.h",
	MAME_DIR .. "src/mame/machine/gaelco_ds5002fp.cpp",
	MAME_DIR .. "src/mame/machine/gaelco_ds5002fp.h",
	MAME_DIR .. "src/mame/drivers/looping.cpp",
	MAME_DIR .. "src/mame/drivers/supertnk.cpp",
	MAME_DIR .. "src/mame/drivers/goldnpkr.cpp",
}
end

function linkProjects_mame_tiny(_target, _subtarget)
	links {
		"mame_tiny",
	}
end
