-- license:BSD-3-Clause
-- copyright-holders:Jack Li
local exports = {
	name = 'autofire',
	version = '0.0.4',
	description = 'Autofire plugin',
	license = 'The BSD 3-Clause License',
	author = { name = 'Jack Li' } }

local autofire = exports

function autofire.startplugin()

	-- List of autofire buttons, each being a table with keys:
	--   'port' - port name of the button being autofired
	--   'field' - field name of the button being autofired
	--   'key' - input_code of the keybinding
	--   'on_frames' - number of frames button is pressed
	--   'off_frames' - number of frames button is released
	--   'button' - reference to ioport_field
	--   'counter' - position in autofire cycle
	local buttons = {}

	local current_rom = nil

	local function process_button(button)
		local pressed = manager.machine.input:seq_pressed(button.key)
		if pressed then
			local state = button.counter < button.on_frames and 1 or 0
			button.counter = (button.counter + 1) % (button.on_frames + button.off_frames)
			return state
		else
			button.counter = 0
			return 0
		end
	end

	local function button_states_key(button)
		return button.port .. '\0' .. button.field
	end

	local function process_frame()
		-- Resolves conflicts between multiple autofire keybindings for the same button.
		local button_states = {}

		for i, button in ipairs(buttons) do
			local state = button_states[button_states_key(button)]
			if not state then
				state = 0
			end
			state = process_button(button) | state
			button_states[button_states_key(button)] = state
		end
		for i, button in ipairs(buttons) do
			button.button:set_value(button_states[button_states_key(button)])
		end
	end

	local function reinit_buttons()
		for i, button in ipairs(buttons) do
			button.counter = 0
			button.button = manager.machine.ioport.ports[button.port].fields[button.field]
		end
	end

	local function load_settings()
		if current_rom == emu.romname() then
			reinit_buttons()
		else
			local loader = require('autofire/autofire_save')
			if loader then
				buttons = loader:load_settings()
			end
		end
		current_rom = emu.romname()
		local menu_handler = require('autofire/autofire_menu')
		if menu_handler then
			menu_handler:init_menu(buttons)
		end
	end

	local function save_settings()
		local saver = require('autofire/autofire_save')
		if saver then
			saver:save_settings(buttons)
		end
	end

	local function menu_callback(index, event)
		local menu_handler = require('autofire/autofire_menu')
		if menu_handler then
			return menu_handler:handle_menu_event(index, event, buttons)
		else
			return false
		end
	end

	local function menu_populate()
		local menu_handler = require('autofire/autofire_menu')
		if menu_handler then
			return menu_handler:populate_menu(buttons)
		else
			return {{_('Failed to load autofire menu'), '', ''}}
		end
	end

	emu.register_frame_done(process_frame)
	emu.register_start(load_settings)
	emu.register_stop(save_settings)
	emu.register_menu(menu_callback, menu_populate, _('Autofire'))
end

return exports
