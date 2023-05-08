-- license:BSD-3-Clause
-- copyright-holders:Carl
-- Layout scripts should return a table and a string.  The table can have two optional keys reset and frame
-- which have functions for values called on reset and frame draw respectively and the string is a unique name.
local exports = {
	name = "layout",
	version = "0.0.1",
	description = "Layout helper plugin",
	license = "BSD-3-Clause",
	author = { name = "Carl" } }

local layout = exports

local frame_subscription, stop_subscription

function layout.startplugin()
	local scripts = {}
	local function prepare_layout(file, script)
		local env = {
			machine = manager.machine,
			emu = {
				attotime = emu.attotime,
				render_bounds = emu.render_bounds,
				render_color = emu.render_color,
				print_verbose = emu.print_verbose,
				print_error = emu.print_error,
				print_warning = emu.print_warning,
				print_info = emu.print_info,
				print_debug = emu.print_debug },
			file = file,
			print = print,
			pairs = pairs,
			ipairs = ipairs,
			string = string,
			tonumber = tonumber,
			tostring = tostring,
			table = table }
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
	frame_subscription = emu.add_machine_frame_notifier(function ()
		if manager.machine.paused then
			return
		end
		for num, scr in pairs(scripts) do
			if scr.frame then
				scr.frame()
			end
		end
	end)
	emu.register_prestart(function ()
		for num, scr in pairs(scripts) do
			if scr.reset then
				scr.reset()
			end
		end
	end)
	stop_subscription = emu.add_machine_stop_notifier(function ()
		scripts = {}
	end)
end

return exports
