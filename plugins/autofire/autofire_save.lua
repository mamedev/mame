local lib = {}

local function get_settings_path()
	return emu.subst_env(manager.machine.options.entries.homepath:value():match('([^;]+)')) .. '/autofire'
end

local function get_settings_filename()
	return emu.romname() .. '.cfg'
end

local function initialize_button(settings)
	if settings.port and settings.mask and settings.type and settings.key and settings.on_frames and settings.off_frames then
		local ioport = manager.machine.ioport
		local new_button = {
			port = settings.port,
			mask = settings.mask,
			type = ioport:token_to_input_type(settings.type),
			key = manager.machine.input:seq_from_tokens(settings.key),
			key_cfg = settings.key,
			on_frames = settings.on_frames,
			off_frames = settings.off_frames,
			counter = 0
		}
		local port = ioport.ports[settings.port]
		if port then
			local field = port:field(settings.mask)
			if field and (field.type == new_button.type) then
				new_button.button = field
			end
		end
		return new_button
	end
	return nil
end

local function serialize_settings(button_list)
	local settings = {}
	for index, button in ipairs(button_list) do
		local setting = {
			port = button.port,
			mask = button.mask,
			type = manager.machine.ioport:input_type_to_token(button.type),
			key = button.key_cfg,
			on_frames = button.on_frames,
			off_frames = button.off_frames
		}
		table.insert(settings, setting)
	end
	return settings
end

function lib:load_settings()
	local buttons = {}
	local json = require('json')
	local filename = get_settings_path() .. '/' .. get_settings_filename()
	local file = io.open(filename, 'r')
	if not file then
		return buttons
	end
	local loaded_settings = json.parse(file:read('a'))
	file:close()
	if not loaded_settings then
		emu.print_error(string.format('Error loading autofire settings: error parsing file "%s" as JSON', filename))
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
		emu.print_error(string.format('Error saving autofire settings: "%s" is not a directory', path))
		return
	end
	local filename = path .. '/' .. get_settings_filename()
	if #buttons == 0 then
		os.remove(filename)
		return
	end
	local json = require('json')
	local settings = serialize_settings(buttons)
	local data = json.stringify(settings, {indent = true})
	local file = io.open(filename, 'w')
	if not file then
		emu.print_error(string.format('Error saving autofire settings: error opening file "%s" for writing', filename))
		return
	end
	file:write(data)
	file:close()
end

return lib
