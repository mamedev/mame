-- license:BSD-3-Clause
-- copyright-holders:Vas Crabb


-- Helpers

local function settings_path()
	return emu.subst_env(manager.machine.options.entries.homepath:value():match('([^;]+)')) .. '/inputmacro'
end

local function settings_filename()
	return emu.romname() .. '.cfg'
end

local function make_macro(setting)
	if (setting.name == nil) or (setting.binding == nil) or (setting.earlycancel == nil) or (setting.loop == nil) or (setting.steps == nil) then
		return nil
	end

	local result = {
		name = setting.name,
		binding = manager.machine.input:seq_from_tokens(setting.binding),
		bindingcfg = setting.binding,
		earlycancel = setting.earlycancel,
		loop = setting.loop,
		steps = { } }

	local ioport = manager.machine.ioport
	for i, step in ipairs(setting.steps) do
		if step.inputs and step.delay and step.duration then
			local s = {
				inputs = { },
				delay = step.delay,
				duration = step.duration }
			for j, input in ipairs(step.inputs) do
				if input.port and input.mask and input.type then
					local ipt = {
						port = input.port,
						mask = input.mask,
						type = ioport:token_to_input_type(input.type) }
					local port = ioport.ports[input.port]
					if port then
						local field = port:field(input.mask)
						if field and (field.type == ipt.type) then
							ipt.field = field
						end
					end
					table.insert(s.inputs, ipt)
				end
			end
			if #s.inputs > 0 then
				table.insert(result.steps, s)
			end
		end
	end

	if result.loop > #result.steps then
		result.loop = -1
	end

	if #result.steps > 0 then
		return result
	else
		return nil
	end
end

local function make_settings(macros)
	local input = manager.machine.input
	local ioport = manager.machine.ioport
	local result = { }
	for i, macro in ipairs(macros) do
		local m = {
			name = macro.name,
			binding = macro.bindingcfg,
			earlycancel = macro.earlycancel,
			loop = macro.loop,
			steps = { } }
		table.insert(result, m)
		for j, step in ipairs(macro.steps) do
			local s = {
				inputs = { },
				delay = step.delay,
				duration = step.duration }
			table.insert(m.steps, s)
			for k, input in ipairs(step.inputs) do
				local b = {
					port = input.port,
					mask = input.mask,
					type = ioport:input_type_to_token(input.type) }
				table.insert(s.inputs, b)
			end
		end
	end
	return result
end


-- Entry points

local lib = { }

function lib:load_settings()
	filename = settings_path() .. '/' .. settings_filename()
	local file = io.open(filename, 'r')
	if not file then
		return { }
	end
	local json = require('json')
	local settings = json.parse(file:read('a'))
	file:close()
	if not settings then
		emu.print_error(string.format('Error loading input macros: error parsing file "%s" as JSON', filename))
		return { }
	end

	result = { }
	for index, setting in ipairs(settings) do
		local macro = make_macro(setting)
		if macro then
			table.insert(result, macro)
		end
	end
	return result
end

function lib:save_settings(macros)
	local path = settings_path()
	local stat = lfs.attributes(path)
	if not stat then
		lfs.mkdir(path)
	elseif stat.mode ~= 'directory' then
		emu.print_error(string.format('Error saving input macros: "%s" is not a directory', path))
		return
	end
	filename = path .. '/' .. settings_filename()

	if #macros == 0 then
		os.remove(filename)
		return
	end

	local json = require('json')
	local settings = make_settings(macros)
	local text = json.stringify(settings, { indent = true })
	local file = io.open(filename, 'w')
	if not file then
		emu.print_error(string.format('Error saving input macros: error opening file "%s" for writing', filename))
		return
	end
	file:write(text)
	file:close()
end

return lib
