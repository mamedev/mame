-- license:BSD-3-Clause
-- copyright-holders:Jack Li
local exports = {
	name = 'autofire',
	version = '0.0.4',
	description = 'Autofire plugin',
	license = 'BSD-3-Clause',
	author = { name = 'Jack Li' } }

local autofire = exports

function autofire.startplugin()

	-- List of autofire buttons, each being a table with keys:
	--   'port' - port name of the button being autofired
	--   'mask' - mask of the button field being autofired
	--   'type' - input type of the button being autofired
	--   'key' - input_seq of the keybinding
	--   'key_cfg' - configuration string for the keybinding
	--   'on_frames' - number of frames button is pressed
	--   'off_frames' - number of frames button is released
	--   'button' - reference to ioport_field
	--   'counter' - position in autofire cycle
	local buttons = {}

	local menu_handler

	local function process_frame()
		local input = manager.machine.input

		local function process_button(button)
			local pressed = input:seq_pressed(button.key)
			if pressed then
				local state = button.counter < button.on_frames and 1 or 0
				button.counter = (button.counter + 1) % (button.on_frames + button.off_frames)
				return state
			else
				button.counter = 0
				return 0
			end
		end

		-- Resolves conflicts between multiple autofire keybindings for the same button.
		local button_states = {}

		for i, button in ipairs(buttons) do
			if button.button then
				local key = button.port .. '\0' .. button.mask .. '.' .. button.type
				local state = button_states[key] or {0, button.button}
				state[1] = process_button(button) | state[1]
				button_states[key] = state
			end
		end
		for i, state in pairs(button_states) do
			state[2]:set_value(state[1])
		end
	end

	local function load_settings()
		local loader = require('autofire/autofire_save')
		if loader then
			buttons = loader:load_settings()
		end
	end

	local function save_settings()
		local saver = require('autofire/autofire_save')
		if saver then
			saver:save_settings(buttons)
		end

		menu_handler = nil
		buttons = {}
	end

	local function menu_callback(index, event)
		if menu_handler then
			return menu_handler:handle_menu_event(index, event, buttons)
		else
			return false
		end
	end

	local function menu_populate()
		if not menu_handler then
			menu_handler = require('autofire/autofire_menu')
			if menu_handler then
				menu_handler:init_menu(buttons)
			end
		end
		if menu_handler then
			return menu_handler:populate_menu(buttons)
		else
			return {{_p('plugin-autofire', 'Failed to load autofire menu'), '', 'off'}}
		end
	end

	emu.register_frame(process_frame)
	emu.register_prestart(load_settings)
	emu.register_stop(save_settings)
	emu.register_menu(menu_callback, menu_populate, _p('plugin-autofire', 'Autofire'))
end

return exports
