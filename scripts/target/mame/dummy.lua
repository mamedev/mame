-- license:BSD-3-Clause
-- copyright-holders:MAMEdev Team

---------------------------------------------------------------------------
--
--   dummy.lua
--
--   Dummy target makefile
--
---------------------------------------------------------------------------

-- Set all the device flag setting commands from the block headers

local function selectors_get(path)
	local selector = ""
	for l in io.lines(path) do
		if l:sub(1, 3) == "--@" then
			local pos = l:find(",")
			selector = selector .. l:sub(pos+1) .. "\n"
		end
	end
	return selector
end

local selectors =
		selectors_get(MAME_DIR .. "scripts/src/cpu.lua") ..
		selectors_get(MAME_DIR .. "scripts/src/sound.lua") ..
		selectors_get(MAME_DIR .. "scripts/src/video.lua") ..
		selectors_get(MAME_DIR .. "scripts/src/machine.lua") ..
		selectors_get(MAME_DIR .. "scripts/src/bus.lua") ..
		selectors_get(MAME_DIR .. "scripts/src/formats.lua")

load(selectors)()


function createProjects_mame_dummy(_target, _subtarget)
	project ("mame_dummy")
	targetsubdir(_target .."_" .. _subtarget)
	kind (LIBTYPE)
	uuid (os.uuid("drv-mame_dummy"))
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
	MAME_DIR .. "src/mame/coleco/coleco.cpp",
	MAME_DIR .. "src/mame/coleco/coleco.h",
	MAME_DIR .. "src/mame/coleco/coleco_m.cpp",
	MAME_DIR .. "src/mame/coleco/coleco_m.h",
}
end

function linkProjects_mame_dummy(_target, _subtarget)
	links {
		"mame_dummy",
	}
end
