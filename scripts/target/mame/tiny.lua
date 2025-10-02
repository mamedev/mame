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
SOUNDS["SN76496"] = true
SOUNDS["TMS5220"] = true
SOUNDS["VOTRAX_SC01"] = true
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
MACHINES["MOS6530"] = true
MACHINES["NETLIST"] = true
MACHINES["OUTPUT_LATCH"] = true
MACHINES["PIT8253"] = true
MACHINES["SEGACRPT"] = true
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
		MAME_DIR .. "src/mame/shared",
		MAME_DIR .. "src/lib",
		MAME_DIR .. "src/lib/util",
		MAME_DIR .. "3rdparty",
		GEN_DIR  .. "mame/layout",
	}

files{
	MAME_DIR .. "src/mame/exidy/nl_carpolo.cpp",
	MAME_DIR .. "src/mame/exidy/nl_carpolo.h",
	MAME_DIR .. "src/mame/exidy/carpolo.cpp",
	MAME_DIR .. "src/mame/exidy/carpolo.h",
	MAME_DIR .. "src/mame/exidy/carpolo_m.cpp",
	MAME_DIR .. "src/mame/exidy/carpolo_v.cpp",
	MAME_DIR .. "src/mame/exidy/circus_a.cpp",
	MAME_DIR .. "src/mame/exidy/circus.cpp",
	MAME_DIR .. "src/mame/exidy/circus.h",
	MAME_DIR .. "src/mame/exidy/circus_v.cpp",
	MAME_DIR .. "src/mame/exidy/exidy.cpp",
	MAME_DIR .. "src/mame/shared/exidysound.cpp",
	MAME_DIR .. "src/mame/shared/exidysound.h",
	MAME_DIR .. "src/mame/exidy/exidy440_a.cpp",
	MAME_DIR .. "src/mame/exidy/exidy440_a.h",
	MAME_DIR .. "src/mame/exidy/nl_fireone.h",
	MAME_DIR .. "src/mame/exidy/nl_fireone.cpp",
	MAME_DIR .. "src/mame/exidy/nl_starfire.h",
	MAME_DIR .. "src/mame/exidy/nl_starfire.cpp",
	MAME_DIR .. "src/mame/exidy/starfire.cpp",
	MAME_DIR .. "src/mame/exidy/starfire.h",
	MAME_DIR .. "src/mame/exidy/starfire_v.cpp",
	MAME_DIR .. "src/mame/exidy/vertigo.cpp",
	MAME_DIR .. "src/mame/exidy/vertigo.h",
	MAME_DIR .. "src/mame/exidy/vertigo_m.cpp",
	MAME_DIR .. "src/mame/exidy/vertigo_v.cpp",
	MAME_DIR .. "src/mame/exidy/victory.cpp",
	MAME_DIR .. "src/mame/exidy/victory.h",
	MAME_DIR .. "src/mame/exidy/victory_v.cpp",
	MAME_DIR .. "src/mame/midway/astrocde.cpp",
	MAME_DIR .. "src/mame/midway/astrocde.h",
	MAME_DIR .. "src/mame/midway/astrocde_v.cpp",
	MAME_DIR .. "src/mame/midway/gridlee.cpp",
	MAME_DIR .. "src/mame/midway/gridlee.h",
	MAME_DIR .. "src/mame/midway/gridlee_a.cpp",
	MAME_DIR .. "src/mame/midway/gridlee_v.cpp",
	MAME_DIR .. "src/mame/shared/s11c_bg.cpp",
	MAME_DIR .. "src/mame/shared/s11c_bg.h",
	MAME_DIR .. "src/mame/shared/williamssound.cpp",
	MAME_DIR .. "src/mame/shared/williamssound.h",
	MAME_DIR .. "src/mame/midway/williams.cpp",
	MAME_DIR .. "src/mame/midway/williams.h",
	MAME_DIR .. "src/mame/midway/williams_m.cpp",
	MAME_DIR .. "src/mame/midway/williams_v.cpp",
	MAME_DIR .. "src/mame/midway/williamsblitter.cpp",
	MAME_DIR .. "src/mame/midway/williamsblitter.h",
	MAME_DIR .. "src/mame/gaelco/gaelco.cpp",
	MAME_DIR .. "src/mame/gaelco/gaelco.h",
	MAME_DIR .. "src/mame/gaelco/gaelco_v.cpp",
	MAME_DIR .. "src/mame/gaelco/gaelcrpt.cpp",
	MAME_DIR .. "src/mame/gaelco/wrally.cpp",
	MAME_DIR .. "src/mame/gaelco/gaelco_wrally_sprites.cpp",
	MAME_DIR .. "src/mame/gaelco/gaelco_wrally_sprites.h",
	MAME_DIR .. "src/mame/gaelco/gaelco_ds5002fp.cpp",
	MAME_DIR .. "src/mame/gaelco/gaelco_ds5002fp.h",
	MAME_DIR .. "src/mame/misc/goldnpkr.cpp",
	MAME_DIR .. "src/mame/videogames/looping.cpp",
	MAME_DIR .. "src/mame/videogames/supertnk.cpp",
}
end

function linkProjects_mame_tiny(_target, _subtarget)
	links {
		"mame_tiny",
	}
end
