-- license:BSD-3-Clause
-- copyright-holders:MAMEdev Team

---------------------------------------------------------------------------
--
--   schreibmaschine.lua
--
--   Brother LW-30
--   Brother LW-450
--   Brother LW-350
--   Brother LW-840ic
--
---------------------------------------------------------------------------

CPUS["Z180"] = true
CPUS["H8"] = true
VIDEOS["MC6845"] = true
SOUNDS["BEEP"] = true
MACHINES["UPD765"] = true
MACHINES["FDC_PLL"] = true
FORMATS["UPD765_DSK"] = true
FORMATS["BASICDSK"] = true
FORMATS["PC_DSK"] = true

function createProjects_mame_schreibmaschine(_target, _subtarget)
	project ("mame_schreibmaschine")
	targetsubdir(_target .."_" .. _subtarget)
	kind (LIBTYPE)
	uuid (os.uuid("drv-mame_schreibmaschine"))
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

files {
	MAME_DIR .. "src/mame/drivers/brother_lw30.cpp",
	MAME_DIR .. "src/mame/drivers/brother_lw350.cpp",
	MAME_DIR .. "src/mame/drivers/brother_lw840.cpp",
}
end

function linkProjects_mame_schreibmaschine(_target, _subtarget)
	links {
		"mame_schreibmaschine",
	}
end
