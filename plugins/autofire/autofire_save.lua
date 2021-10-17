local lib = {}

local function get_settings_path()
	return emu.subst_env(manager.machine.options.entries.homepath:value():match('([^;]+)')) .. '/autofire/'
end

local function get_settings_filename()
	return emu.romname() .. '.cfg'
end

local function initialize_button(settings)
	if settings.port and settings.mask and settings.type and settings.key and settings.on_frames and settings.off_frames then
		local new_button = {
			port = settings.port,
			key = manager.machine.input:seq_from_tokens(settings.key),
			on_frames = settings.on_frames,
			off_frames = settings.off_frames,
			counter = 0
		}
		local ioport = manager.machine.ioport
		local port = ioport.ports[settings.port]
		if port then
			local field = port:field(settings.mask)
			if field and (field.type == ioport:token_to_input_type(settings.type)) then
				new_button.field = field.name
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
			mask = button.button.mask,
			type = manager.machine.ioport:input_type_to_token(button.button.type),
			key = manager.machine.input:seq_to_tokens(button.key),
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
	if #buttons == 0 then
		os.remove(path .. get_settings_filename())
		return
	end
	local json = require('json')
	local settings = serialize_settings(buttons)
	local data = json.stringify(settings, {indent = true})
	local file = io.open(path .. get_settings_filename(), 'w')
	if file then
		file:write(data)
		file:close()
	end
end

return lib
