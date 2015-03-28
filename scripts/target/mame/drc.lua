---------------------------------------------------------------------------
--
--   tiny.lua
--
--   Small driver-specific example makefile
--   Use make SUBTARGET=tiny to build
--
--   Copyright Nicola Salmoria and the MAME Team.
--   Visit  http://mamedev.org for licensing and usage restrictions.
--
---------------------------------------------------------------------------


--------------------------------------------------
-- Specify all the CPU cores necessary for the
-- drivers referenced in tiny.c.
--------------------------------------------------

CPUS["Z80"] = true
CPUS["MCS48"] = true
CPUS["MCS51"] = true

CPUS["SH2"] = true
CPUS["M680X0"] = true
CPUS["ADSP21XX"] = true
CPUS["SCUDSP"] = true


--------------------------------------------------
-- Specify all the sound cores necessary for the
-- drivers referenced in tiny.c.
--------------------------------------------------

SOUNDS["SCSP"] = true
SOUNDS["CDDA"] = true
SOUNDS["DMADAC"] = true

--------------------------------------------------
-- specify available video cores
--------------------------------------------------
VIDEOS["STVVDP"] = true

--------------------------------------------------
-- specify available machine cores
--------------------------------------------------

MACHINES["SMPC"] = true
MACHINES["STVCD"] = true
MACHINES["SATURN"] = true
MACHINES["SERFLASH"] = true
MACHINES["EEPROMDEV"] = true

--------------------------------------------------
-- specify available bus cores
--------------------------------------------------
BUSES["GENERIC"] = true
--------------------------------------------------
-- This is the list of files that are necessary
-- for building all of the drivers referenced
-- in drc.c
--------------------------------------------------

function createProjects(_target, _subtarget)
	project ("drc")
	targetsubdir(_target .."_" .. _subtarget)
	kind "StaticLib"
	uuid (os.uuid("drv-mame-drc"))
	
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
	MAME_DIR .. "src/mame/drivers/stv.c",
	MAME_DIR .. "src/mame/machine/stvprot.c",
	MAME_DIR .. "src/mame/machine/315-5838_317-0229_comp.c",
	MAME_DIR .. "src/mame/machine/315-5881_crypt.c",
	}
end

function linkProjects(_target, _subtarget)
	links {
		"drc",
	}
end