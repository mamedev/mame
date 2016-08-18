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


--------------------------------------------------
-- specify available sound cores; some of these are
-- only for MAME and so aren't included
--------------------------------------------------


--------------------------------------------------
-- specify available video cores
--------------------------------------------------


--------------------------------------------------
-- specify available machine cores
--------------------------------------------------


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
		MAME_DIR .. "src/lib/netlist",
		MAME_DIR .. "3rdparty",
	}
end

function createProjects_mame_virtual(_target, _subtarget)
createVirtualProjects(_target, _subtarget, "virtual")
files {
	MAME_DIR .. "src/mame/drivers/vgmplay.cpp",
}
end
