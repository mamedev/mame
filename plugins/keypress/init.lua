-- license:BSD-3-Clause
-- copyright-holders:Andrei I. Holub
local exports = {
	name = 'keypress',
	version = '0.0.1',
	description = 'Display currently pressed keys',
	license = 'BSD-3-Clause',
	author = { name = 'Andrei I. Holub' } }

local keypress = exports

local frame_subscription, stop_subscription

function keypress.startplugin()
	local shown = false

	local function process_frame()
		local pressed = {}
		local input = manager.machine.input

		for name, devclass in pairs(input.device_classes) do
			if devclass.enabled then
				for _, dev in ipairs(devclass.devices) do
					for id, item in pairs(dev.items) do
						if item.current ~= 0 then
							pressed[#pressed + 1] = item.name
						end
					end
				end
			end
		end

		if #pressed > 0 then
			manager.machine:popmessage(table.concat(pressed, '  '))
			shown = true
		elseif shown then
			manager.machine:popmessage()
			shown = false
		end
	end

	local function stop()
		if shown then
			manager.machine:popmessage()
			shown = false
		end
	end

	frame_subscription = emu.add_machine_frame_notifier(process_frame)
	stop_subscription = emu.add_machine_stop_notifier(stop)
end

return exports
