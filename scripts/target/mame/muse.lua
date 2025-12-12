-- license:BSD-3-Clause
-- copyright-holders:MAMEdev Team

---------------------------------------------------------------------------
--
--   muse.lua
--
--   MUSE target makefile
--
---------------------------------------------------------------------------

-- CPUS["COP400"] = true
-- CPUS["M6502"] = true
-- CPUS["M6800"] = true
-- CPUS["M6805"] = true
CPUS["M6809"] = true
-- CPUS["M680X0"] = true
-- CPUS["MCS48"] = true
CPUS["MCS51"] = true
-- CPUS["TMS9900"] = true
-- CPUS["Z80"] = true
CPUS["I8085"] = true
CPUS["I86"] = true

VIDEOS["HD44780"] = true
VIDEOS["HD61830"] = true

SOUNDS["DAC"] = true
SOUNDS["HOHNER"] = true

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
-- MACHINES["TTL74148"] = true
-- MACHINES["TTL74153"] = true
-- MACHINES["TTL74157"] = true
-- MACHINES["TTL74259"] = true
-- MACHINES["TTL7474"] = true
MACHINES["WATCHDOG"] = true
MACHINES["ACIA6850"] = true

BUSES["MIDI"] = true

-- Set all the device flag setting commands from the block headers
-- NOTE: This was enabling ALL devices! Commented out for minimal build.

--[[
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
--]]


function createProjects_mame_muse(_target, _subtarget)
	project ("mame_muse")
	targetsubdir(_target .."_" .. _subtarget)
	kind (LIBTYPE)
	uuid (os.uuid("drv-mame_muse"))
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

files {
	-- MAME_DIR .. "src/mame/hohner/vox5.cpp",
	-- MAME_DIR .. "src/mame/hohner/d86.cpp",
	-- MAME_DIR .. "src/mame/hohner/gp98.cpp",
	MAME_DIR .. "src/mame/hohner/ustudio.cpp",
	MAME_DIR .. "src/mame/hohner/xe9.cpp",
	MAME_DIR .. "src/mame/wersi/keyfox10.cpp",
	-- MAME_DIR .. "src/mame/hohner/wersidx.cpp",
    -- MAME_DIR .. "src/mame/hohner/boehm1224.cpp",
    -- MAME_DIR .. "src/mame/hohner/boehm_sinus.cpp",
    -- MAME_DIR .. "src/mame/casio/rz1.cpp",
    -- MAME_DIR .. "src/mame/casio/cz101.cpp",
	-- MAME_DIR .. "src/mame/hohner/wilgamat3.cpp",
}

end

function linkProjects_mame_muse(_target, _subtarget)
	links {
		"mame_muse",
	}
end
