-- license:BSD-3-Clause
-- copyright-holders:Jack Li
local exports = {}
exports.name = "autofire"
exports.version = "0.0.1"
exports.description = "Autofire plugin"
exports.license = "The BSD 3-Clause License"
exports.author = { name = "Jack Li" }

local autofire = exports

function autofire.startplugin()

	-- List of autofire buttons, each being a table with keys:
	--   'port' - port name of the button being autofired
	--   'field' - field name of the button being autofired
	--   'key' - token of the keybinding
	--   'on_frames' - number of frames button is pressed
	--   'off_frames' - number of frames button is released
	--   'button' - reference to ioport_field (not persisted)
	--   'counter' - position in autofire cycle (not persisted)
	-- port, field, key, on_frames, and off_frames are loaded from/saved to a file on start/stop.
	local buttons = {}

	local function process_button(button)
		local keycode = manager:machine():input():code_from_token(button.key)
		local pressed = manager:machine():input():code_pressed(keycode)
		if pressed then
			button.button:set_value(button.counter < button.on_frames and 1 or 0)
			button.counter = (button.counter + 1) % (button.on_frames + button.off_frames)
		else
			button.counter = 0
			button.button:set_value(0)
		end
	end

	local function process_frame()
		for i, button in ipairs(buttons) do
			process_button(button)
		end
	end

	local function initialize_button(settings)
		if settings.port and settings.field and settings.key and settings.on_frames and settings.off_frames then
			local new_button = {}
			new_button.port = settings.port
			new_button.field = settings.field
			new_button.key = settings.key
			new_button.on_frames = settings.on_frames
			new_button.off_frames = settings.off_frames
			new_button.counter = 0
			local port = manager:machine():ioport().ports[settings.port]
			if port then
				local field = port.fields[settings.field]
				if field then
					new_button.button = field
					return new_button
				end
			end
		end
		return nil
	end

	local function get_settings_path()
		return lfs.env_replace(manager:machine():options().entries.pluginspath:value():match("([^;]+)")) .. "/autofire/cfg/"
	end

	local function get_settings_filename()
		return emu.romname() .. '.cfg'
	end

	local function load_settings()
		buttons = {}
		local json = require('json')
		local file = io.open(get_settings_path() .. get_settings_filename(), 'r')
		if not file then
			return
		end
		local loaded_settings = json.parse(file:read('a'))
		file:close()
		if not loaded_settings then
			return
		end
		for index, button_settings in ipairs(loaded_settings) do
			local new_button = initialize_button(button_settings)
			if new_button then
				buttons[#buttons + 1] = new_button
			end
		end
	end

	local function serialize_settings(button_list)
		local settings = {}
		for index, button in ipairs(button_list) do
			setting = {}
			setting.port = button.port
			setting.field = button.field
			setting.key = button.key
			setting.on_frames = button.on_frames
			setting.off_frames = button.off_frames
			settings[#settings + 1] = setting
		end
		return settings
	end

	local function save_settings()
		local path = get_settings_path()
		local attr = lfs.attributes(path)
		if not attr then
			lfs.mkdir(path)
		elseif attr.mode ~= 'directory' then
			return
		end
		local json = require('json')
		local settings = serialize_settings(buttons)
		local file = io.open(path .. get_settings_filename(), 'w')
		if file then
			file:write(json.stringify(settings, {indent = true}))
			file:close()
		end
	end

	emu.register_frame_done(process_frame)
	emu.register_start(load_settings)
	emu.register_stop(save_settings)
end

return exports
