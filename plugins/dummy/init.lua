-- license:BSD-3-Clause
-- copyright-holders:Miodrag Milanovic
local exports = {
	name = "dummy",
	version = "0.0.1",
	description = "A dummy example",
	license = "BSD-3-Clause",
	author = { name = "Miodrag Milanovic" }}

local dummy = exports

local reset_subscription, stop_subscription

function dummy.startplugin()
	reset_subscription = emu.add_machine_reset_notifier(
			function ()
				emu.print_info("Starting " .. emu.gamename())
			end)

	stop_subscription = emu.add_machine_stop_notifier(
			function ()
				emu.print_info("Exiting " .. emu.gamename())
			end)

	local function menu_populate()
		return {{ "This is a", "test", "off" }, { "Also a", "test", "" }}
	end

	local function menu_callback(index, event)
		emu.print_info("index: " .. index .. " event: " .. event)
		return false
	end

	emu.register_menu(menu_callback, menu_populate, "Dummy")
end

return exports
