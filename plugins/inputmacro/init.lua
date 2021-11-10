-- license:BSD-3-Clause
-- copyright-holders:Vas Crabb
local exports = {
	name = 'inputmacro',
	version = '0.0.1',
	description = 'Input macro plugin',
	license = 'BSD-3-Clause',
	author = { name = 'Vas Crabb' } }


local inputmacro = exports

function inputmacro.startplugin()
	--[[
	  Configuration data:
	  * name: display name (string)
	  * binding: activation sequence (input sequence)
	  * earlycancel: cancel or complete on release (Boolean)
	  * loop: -1 = release, 0 = prolong, >0 = loop to step on hold (int)
	  * steps:
	    * inputs:
	      * port: port (I/O port)
	      * field: field (I/O port field)
	    * delay: delay before activating inputs in frames (integer)
	    * duration: duration to activate inputs for (integer)

	  Live state:
	  * step: current step (integer or nil)
	  * frame: frame of current step, starting at 1 (integer)
	]]
	local macros = { }
	local active_inputs = { }
	local shortname
	local menu
	local input

	local function activate_inputs(inputs)
		for index, input in ipairs(inputs) do
			active_inputs[string.format('%s.%d.%d', input.port.tag, input.field.mask, input.field.type)] = input.field
		end
	end

	local function process_frame()
		previous_inputs = active_inputs
		active_inputs = { }

		for index, macro in ipairs(macros) do
			if macro.step then
				if macro.earlycancel and (not input:seq_pressed(macro.binding)) then
					-- stop immediately on release if early cancel set
					macro.step = nil
				else
					-- advance frame
					macro.frame = macro.frame + 1
					local step = macro.steps[macro.step]
					if macro.frame > (step.delay + step.duration) then
						if macro.step < #macro.steps then
							-- not the last step, advance step
							macro.step = macro.step + 1
							macro.frame = 1
							step = macro.steps[macro.step]
						elseif not input:seq_pressed(macro.binding) then
							-- input released and macro completed
							macro.step = nil
							step = nil
						elseif macro.loop > 0 then
							-- loop to step
							macro.step = macro.loop
							macro.frame = 1
						elseif macro.loop < 0 then
							-- release if held
							step = nil
						end
					end
					if step and (macro.frame > step.delay) then
						activate_inputs(step.inputs)
					end
				end
			elseif input:seq_pressed(macro.binding) then
				-- initial activation
				macro.step = 1
				macro.frame = 1
				local step = macro.steps[1]
				if step.delay == 0 then
					-- no delay on first step, activate inputs
					activate_inputs(step.inputs)
				end
			end
		end

		for key, field in pairs(active_inputs) do
			field:set_value(1)
		end
		for key, field in pairs(previous_inputs) do
			if not active_inputs[key] then
				field:set_value(0)
			end
		end
	end

	local function start()
		input = manager.machine.input
		if shortname ~= emu.romname() then
			local persister = require('inputmacro/inputmacro_persist')
			macros = persister.load_settings()
			shortname = emu.romname()
		end
	end

	local function stop()
		local persister = require('inputmacro/inputmacro_persist')
		persister:save_settings(macros)
	end

	local function menu_callback(index, event)
		return menu:handle_event(index, event)
	end

	local function menu_populate()
		if not menu then
			menu = require('inputmacro/inputmacro_menu')
			menu:init(macros)
		end
		return menu:populate()
	end

	emu.register_frame_done(process_frame)
	emu.register_start(start)
	emu.register_stop(stop)
	emu.register_menu(menu_callback, menu_populate, _('Input Macros'))
end

return exports
