-- license:BSD-3-Clause
-- copyright-holders:Vas Crabb


-- Helpers

local function settings_path()
	return manager.machine.options.entries.homepath:value():match('([^;]+)') .. '/offscreenreload'
end

local function settings_filename()
	return emu.romname() .. '.cfg'
end

local function make_helper(setting)
	if (setting.binding == nil) or (setting.axis == nil) or (setting.button == nil) then
		return nil
	elseif (setting.axis.port == nil) or (setting.axis.mask == nil) or (setting.axis.type == nil) then
		return nil
	elseif (setting.button.port == nil) or (setting.button.mask == nil) or (setting.button.type == nil) then
		return nil
	end

	local ioport = manager.machine.ioport
	local result = {
		binding = manager.machine.input:seq_from_tokens(setting.binding),
		bindingcfg = setting.binding,
		axis = {
			port = setting.axis.port,
			mask = setting.axis.mask,
			type = ioport:token_to_input_type(setting.axis.type) },
		button = {
			port = setting.button.port,
			mask = setting.button.mask,
			type = ioport:token_to_input_type(setting.button.type) } }

	local axisport = ioport.ports[result.axis.port]
	if axisport then
		local field = axisport:field(result.axis.mask)
		if field and (field.type == result.axis.type) then
			result.axis.field = field
		end
	end

	local buttonport = ioport.ports[result.button.port]
	if buttonport then
		local field = buttonport:field(result.button.mask)
		if field and (field.type == result.button.type) then
			result.button.field = field
		end
	end

	return result
end

local function make_settings(helpers)
	local input = manager.machine.input
	local ioport = manager.machine.ioport
	local result = { }
	for i, helper in ipairs(helpers) do
		local h = {
			binding = helper.bindingcfg,
			axis = {
				port = helper.axis.port,
				mask = helper.axis.mask,
				type = ioport:input_type_to_token(helper.axis.type) },
			button = {
				port = helper.button.port,
				mask = helper.button.mask,
				type = ioport:input_type_to_token(helper.button.type) } }
		table.insert(result, h)
	end
	return result
end


-- Entry points

local lib = { }

function lib:load_settings()
	local filename = settings_path() .. '/' .. settings_filename()
	local file = io.open(filename, 'r')
	if not file then
		return { }
	end
	local json = require('json')
	local settings = json.parse(file:read('a'))
	file:close()
	if not settings then
		emu.print_error(string.format('Error loading off-screen reload helpers: error parsing file "%s" as JSON', filename))
		return { }
	end

	result = { }
	for index, setting in ipairs(settings) do
		local helper = make_helper(setting)
		if helper then
			table.insert(result, helper)
		end
	end
	return result
end

function lib:save_settings(helpers)
	local path = settings_path()
	local stat = lfs.attributes(path)
	if stat and (stat.mode ~= 'directory') then
		emu.print_error(string.format('Error saving off-screen reload helpers: "%s" is not a directory', path))
		return
	end
	local filename = path .. '/' .. settings_filename()

	if #helpers == 0 then
		os.remove(filename)
		return
	elseif not stat then
		lfs.mkdir(path)
	end

	local json = require('json')
	local settings = make_settings(helpers)
	local text = json.stringify(settings, { indent = true })
	local file = io.open(filename, 'w')
	if not file then
		emu.print_error(string.format('Error saving off-screen reload helpers: error opening file "%s" for writing', filename))
		return
	end
	file:write(text)
	file:close()
end

return lib
