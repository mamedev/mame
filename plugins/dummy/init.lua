-- license:BSD-3-Clause
-- copyright-holders:Miodrag Milanovic
local exports = {}
exports.name = "dummy"
exports.version = "0.0.1"
exports.description = "A dummy example"
exports.license = "The BSD 3-Clause License"
exports.author = { name = "Miodrag Milanovic" }

local dummy = exports

function dummy.startplugin()
	emu.register_start(function()
		print("Starting " .. emu.gamename())
	end)
    
	emu.register_stop(function()
		print("Exiting " .. emu.gamename())
	end)

	local function menu_populate()
		return {{ "This is a", "test", 32 }, { "Also a", "test", 0 }} -- 32 is MENU_FLAG_DISABLE
	end

	local function menu_callback(index, event)
		print("index: " .. index .. " event: " .. event)
		return false
	end

	emu.register_menu(menu_callback, menu_populate, "Dummy")
end

return exports
