-- license:BSD-3-Clause
-- copyright-holders:Carl
-- Layout scripts should return a table and a string.  The table can have two optional keys reset and frame 
-- which have functions for values called on reset and frame draw respectively and the string is a unique name.
local exports = {}
exports.name = "layout"
exports.version = "0.0.1"
exports.description = "Layout helper plugin"
exports.license = "The BSD 3-Clause License"
exports.author = { name = "Carl" }

local layout = exports

function layout.startplugin()
	local scripts = {}
	local function prepare_layout(script)
		local env = { machine = manager:machine(), pairs = pairs, ipairs = ipairs,
			      table = { insert = table.insert, remove = table.remove } }
		local script, err = load(script, script, "t", env)
		if not script then
			emu.print_verbose("error loading layout script " .. err)
			return
		end
		local name
		script, name = script()
		scripts[name] = script
	end

	emu.register_callback(prepare_layout, "layout")
	emu.register_frame(function()
		if manager:machine().paused then
			return
		end
		for num, scr in pairs(scripts) do
			if scr.frame then
				scr.frame()
			end
		end
	end)
	emu.register_start(function()
		for num, scr in pairs(scripts) do
			if scr.reset then
				scr.reset()
			end
		end
	end)
	emu.register_stop(function() scripts = {} end)
end

return exports
