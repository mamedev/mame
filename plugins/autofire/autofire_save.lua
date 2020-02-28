local lib = {}

local function get_settings_path()
	return lfs.env_replace(manager:machine():options().entries.homepath:value():match('([^;]+)')) .. '/autofire/'
end

local function get_settings_filename()
	return emu.romname() .. '.cfg'
end

local function initialize_button(settings)
	if settings.port and settings.field and settings.key and settings.on_frames and settings.off_frames then
		local new_button = {
			port = settings.port,
			field = settings.field,
			key = manager:machine():input():seq_from_tokens(settings.key),
			on_frames = settings.on_frames,
			off_frames = settings.off_frames,
			counter = 0
		}
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

local function serialize_settings(button_list)
	local settings = {}
	for index, button in ipairs(button_list) do
		setting = {
			port = button.port,
			field = button.field,
			key = manager:machine():input():seq_to_tokens(button.key),
			on_frames = button.on_frames,
			off_frames = button.off_frames
		}
		settings[#settings + 1] = setting
	end
	return settings
end

function lib:load_settings()
	local buttons = {}
	local json = require('json')
	local file = io.open(get_settings_path() .. get_settings_filename(), 'r')
	if not file then
		return buttons
	end
	local loaded_settings = json.parse(file:read('a'))
	file:close()
	if not loaded_settings then
		return buttons
	end
	for index, button_settings in ipairs(loaded_settings) do
		local new_button = initialize_button(button_settings)
		if new_button then
			buttons[#buttons + 1] = new_button
		end
	end
	return buttons
end

function lib:save_settings(buttons)
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

return lib
