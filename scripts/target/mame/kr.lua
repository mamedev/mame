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
CPUS["UPD7810"] = true
CPUS["M6805"] = true
CPUS["NEC"] = true

--------------------------------------------------
-- Specify all the sound cores necessary for the
-- drivers referenced in tiny.lst.
--------------------------------------------------

SOUNDS["SAMPLES"] = true
SOUNDS["DAC"] = true
SOUNDS["DISCRETE"] = true
SOUNDS["AY8910"] = true
SOUNDS["WAVE"] = true
SOUNDS["BEEP"] = true
SOUNDS["SPEAKER"] = true
SOUNDS["YM2151"] = true

--------------------------------------------------
-- specify available video cores
--------------------------------------------------

VIDEOS["MC6847"] = true
VIDEOS["MC6845"] = true
VIDEOS["TMS9928A"] = true

--------------------------------------------------
-- specify available machine cores
--------------------------------------------------

MACHINES["SPC1000"] = true
MACHINES["SPC1500"] = true
MACHINES["I8255"] = true
MACHINES["E05A03"] = true
MACHINES["E05A30"] = true
MACHINES["UPD765"] = true
MACHINES["EEPROMDEV"] = true
MACHINES["STEPPERS"] = true
MACHINES["WD_FDC"] = true
MACHINES["Z80CTC"] = true
MACHINES["Z80DART"] = true
MACHINES["Z80PIO"] = true 
MACHINES["Z80SIO"] = true 
MACHINES["Z80DMA"] = true 

--------------------------------------------------
-- specify available bus cores
--------------------------------------------------

BUSES["CENTRONICS"] = true
BUSES["SPC1000"] = true
BUSES["GENERIC"] = true

--------------------------------------------------
-- This is the list of files that are necessary
-- for building all of the drivers referenced
-- in tiny.lst
--------------------------------------------------

function createProjects_mame_kr(_target, _subtarget)
	project ("mame_kr")
	targetsubdir(_target .."_" .. _subtarget)
	kind (LIBTYPE)
	uuid (os.uuid("drv-mame-kry"))
	addprojectflags()
	
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
	MAME_DIR .. "src/mame/drivers/spc1000.cpp",
	MAME_DIR .. "src/mame/drivers/spc1500.cpp",
	MAME_DIR .. "src/mame/drivers/x1.cpp",
	MAME_DIR .. "src/mame/includes/x1.h",
	MAME_DIR .. "src/mame/machine/x1.cpp",	
}
end

function linkProjects_mame_kr(_target, _subtarget)
	links {
		"mame_kr",
	}
end
