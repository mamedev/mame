-- license:BSD-3-Clause
-- copyright-holders:Miodrag Milanovic
local exports = {
	name = "dummy",
	version = "0.0.1",
	description = "A dummy example",
	license = "BSD-3-Clause",
	author = { name = "Miodrag Milanovic" }}

local dummy = exports

function dummy.startplugin()
	emu.register_start(function()
		emu.print_verbose("Starting " .. emu.gamename())
	end)

	emu.register_stop(function()
		emu.print_verbose("Exiting " .. emu.gamename())
	end)

	local function menu_populate()
		return {{ "This is a", "test", "off" }, { "Also a", "test", "" }}
	end

	local function menu_callback(index, event)
		emu.print_verbose("index: " .. index .. " event: " .. event)
		return false
	end

	emu.register_menu(menu_callback, menu_populate, "Dummy")
end

return exports
