-- license:BSD-3-Clause
-- copyright-holders:MAMEdev Team

---------------------------------------------------------------------------
--
--   virtual.lua
--
--   Virtual target makefile
--
---------------------------------------------------------------------------

--------------------------------------------------
-- specify available CPU cores
--------------------------------------------------

CPUS["M6502"] = true
CPUS["H6280"] = true
CPUS["MCS48"] = true
CPUS["Z80"] = true
CPUS["DSP16"] = true -- for qsound

--------------------------------------------------
-- specify available sound cores; some of these are
-- only for MAME and so aren't included
--------------------------------------------------

SOUNDS["NES_APU"] = true
SOUNDS["YM2612"] = true
SOUNDS["YM2151"] = true
SOUNDS["YM2413"] = true
SOUNDS["YM2608"] = true
SOUNDS["YM2203"] = true
SOUNDS["AY8910"] = true
SOUNDS["YM3526"] = true
SOUNDS["YM3812"] = true
SOUNDS["YMF271"] = true
SOUNDS["YMZ280B"] = true
SOUNDS["C6280"] = true
SOUNDS["SN76496"] = true
SOUNDS["K051649"] = true
SOUNDS["K053260"] = true
SOUNDS["K054539"] = true
SOUNDS["SEGAPCM"] = true
SOUNDS["MULTIPCM"] = true
SOUNDS["GB_SOUND"] = true
SOUNDS["POKEY"] = true
SOUNDS["C352"] = true
SOUNDS["OKIM6295"] = true
SOUNDS["QSOUND"] = true
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
--------------------------------------------------


--------------------------------------------------
-- this is the driver library that
-- comprise the virtual drivers
--------------------------------------------------
function linkProjects_mame_virtual(_target, _subtarget)
	links {
		"virtual",
	}
end

function createVirtualProjects(_target, _subtarget, _name)
	project (_name)
	targetsubdir(_target .."_" .. _subtarget)
	kind (LIBTYPE)
	uuid (os.uuid("drv-" .. _target .."_" .. _subtarget .. "_" .._name))
	addprojectflags()
	precompiledheaders()

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

	includedirs {
		ext_includedir("zlib"),
		ext_includedir("flac"),
	}
end

function createProjects_mame_virtual(_target, _subtarget)
	createVirtualProjects(_target, _subtarget, "virtual")
	files {
		MAME_DIR .. "src/mame/drivers/vgmplay.cpp",
		MAME_DIR .. "src/mame/drivers/ldplayer.cpp",
	}
end
