-- license:BSD-3-Clause
-- copyright-holders:Vas Crabb
local exports = {
	name = 'offscreenreload',
	version = '0.0.1',
	description = 'Off-screen reload helper plugin',
	license = 'BSD-3-Clause',
	author = { name = 'Vas Crabb' } }


local offscreenreload = exports

local frame_subscription, stop_subscription

function offscreenreload.startplugin()
	--[[
	  Configuration data:
	  * binding: activation sequence (input sequence)
	  * bindingcfg: activation sequence configuration (string)
	  * axis:
	    * port: port tag (string)
	    * mask: port field mask (integer)
	    * type: port field type (integer)
	    * field: field (I/O port field)
	  * button:
	    * port: port tag (string)
	    * mask: port field mask (integer)
	    * type: port field type (integer)
	    * field: field (I/O port field)

	  Live state:
	  * pressed: currently active (Boolean or nil)
	]]
	local helpers = { }
	local menu
	local input

	local function process_frame()
		for index, helper in ipairs(helpers) do
			if input:seq_pressed(helper.binding) then
				if not helper.pressed then
					if helper.axis.field then
						helper.axis.field:set_value(helper.axis.field.minvalue)
					end
					if helper.button.field then
						helper.button.field:set_value(1)
					end
					helper.pressed = true
				end
			else
				if helper.pressed then
					if helper.axis.field then
						helper.axis.field:clear_value()
					end
					if helper.button.field then
						helper.button.field:clear_value()
					end
					helper.pressed = nil
				end
			end
		end
	end

	local function start()
		input = manager.machine.input
		local persister = require('offscreenreload/offscreenreload_persist')
		helpers = persister.load_settings()
	end

	local function stop()
		local persister = require('offscreenreload/offscreenreload_persist')
		persister:save_settings(helpers)

		helpers = { }
		menu = nil
	end

	local function menu_callback(index, event)
		return menu:handle_event(index, event)
	end

	local function menu_populate()
		if not menu then
			menu = require('offscreenreload/offscreenreload_menu')
			menu:init(helpers)
		end
		return menu:populate()
	end

	frame_subscription = emu.add_machine_frame_notifier(process_frame)
	emu.register_prestart(start)
	stop_subscription = emu.add_machine_stop_notifier(stop)
	emu.register_menu(menu_callback, menu_populate, _p('plugin-offscreenreload', 'Off-Screen Reload Helper'))
end

return exports
