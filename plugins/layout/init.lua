-- license:BSD-3-Clause
-- copyright-holders:Carl
-- Layout scripts should return a table and a string.  The table can have two optional keys reset and frame
-- which have functions for values called on reset and frame draw respectively and the string is a unique name.
local exports = {}
exports.name = "layout"
exports.version = "0.0.1"
exports.description = "Layout helper plugin"
exports.license = "BSD-3-Clause"
exports.author = { name = "Carl" }

local layout = exports

function layout.startplugin()
	local scripts = {}
	local function prepare_layout(file, script)
		local env = {
			machine = manager.machine,
			emu = {
				render_bounds = emu.render_bounds,
				render_color = emu.render_color,
				print_verbose = emu.print_verbose,
				print_error = emu.print_error,
				print_info = emu.print_info,
				print_debug = emu.print_debug },
			file = file,
			print = print,
			pairs = pairs,
			ipairs = ipairs,
			string = { format = string.format },
			tostring = tostring,
			table = { insert = table.insert, remove = table.remove } }
		local script, err = load(script, script, "t", env)
		if not script then
			emu.print_verbose("error loading layout script " .. err)
			return
		end
		local hooks = script()
		if hooks ~= nil then
			table.insert(scripts, hooks)
		end
	end

	emu.register_callback(prepare_layout, "layout")
	emu.register_frame(function()
		if manager.machine.paused then
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
