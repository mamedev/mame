-- license:BSD-3-Clause
-- copyright-holders:Vas Crabb


-- Constants

local MENU_TYPES = { MACROS = 0, ADD = 1, EDIT = 2, INPUT = 3 }


-- Globals

local commonui
local macros
local menu_stack

local macros_start_macro -- really for the macros menu, but has to be declared local before edit menu functions


-- Helpers

local function new_macro()
	local function check_name(n)
		for index, macro in ipairs(macros) do
			if macro.name == n then
				return false
			end
		end
		return true
	end

	local name = _p('plugin-inputmacro', 'New macro')
	local number = 1
	while not check_name(name) do
		number = number + 1
		name = string.format(_p('plugin-inputmacro', 'New macro %d'), number)
	end
	return {
		name = name,
		binding = nil,
		earlycancel = true,
		loop = -1,
		steps = {
			{
				inputs = {
					{
						port = nil,
						field = nil } },
				delay = 0,
				duration = 1 } } }
end


-- Input menu

local input_menu
local input_start_field

function start_input_menu(handler, start_field)
	local function supported(f)
		if f.is_analog or f.is_toggle then
			return false
		elseif (f.type_class == 'config') or (f.type_class == 'dipswitch') then
			return false
		else
			return true
		end
	end

	local function action(field)
		if field then
			handler(field)
		end
		table.remove(menu_stack)
		input_menu = nil
		input_start_field = nil
	end

	if not commonui then
		commonui = require('commonui')
	end
	input_menu = commonui.input_selection_menu(action, _p('plugin-inputmacro', 'Set Input'), supported)
	input_start_field = start_field
	table.insert(menu_stack, MENU_TYPES.INPUT)
end

function handle_input(index, action)
	return input_menu:handle(index, action)
end

function populate_input()
	return input_menu:populate(input_start_field)
end


-- Add/edit menus

local edit_current_macro
local edit_start_selection
local edit_start_step
local edit_menu_active
local edit_insert_position
local edit_name_buffer
local edit_items
local edit_item_exit
local edit_switch_poller

local function current_macro_complete()
	if not edit_current_macro.binding then
		return false
	end
	local laststep = edit_current_macro.steps[#edit_current_macro.steps]
	if not laststep.inputs[#laststep.inputs].field then
		return false
	end
	return true
end

local function handle_edit_items(index, event)
	if edit_switch_poller then
		if edit_switch_poller:poll() then
			if edit_switch_poller.sequence then
				edit_current_macro.binding = edit_switch_poller.sequence
			end
			edit_switch_poller = nil
			return true
		end
		return false
	end

	local command = edit_items[index]

	local namecancel = false
	if edit_name_buffer and ((not command) or (command.action ~= 'name')) then
		edit_name_buffer = nil
		namecancel = true
	end

	if not command then
		return namecancel
	elseif command.action == 'name' then
		local function namechar()
			local ch = tonumber(event)
			if not ch then
				return nil
			elseif (ch >= 0x100) or ((ch & 0x7f) >= 0x20) or (ch == 0x08) then
				return utf8.char(ch)
			else
				return nil
			end
		end

		if edit_name_buffer then
			if event == 'select' then
				if #edit_name_buffer > 0 then
					edit_current_macro.name = edit_name_buffer
				end
				edit_name_buffer = nil
				return true
			elseif event == 'cancel' then
				edit_name_buffer = nil
				return true
			else
				local char = namechar()
				if char == '\b' then
					edit_name_buffer = edit_name_buffer:gsub('[%z\1-\127\192-\255][\128-\191]*$', '')
					return true
				elseif char then
					edit_name_buffer = edit_name_buffer .. char
					return true
				end
			end
		elseif event == 'select' then
			edit_name_buffer = edit_current_macro.name
			return true
		else
			local char = namechar()
			if char == '\b' then
				edit_name_buffer = ''
				return true
			elseif char then
				edit_name_buffer = char
				return true
			end
		end
	elseif command.action == 'binding' then
		if event == 'select' then
			if not commonui then
				commonui = require('commonui')
			end
			edit_switch_poller = commonui.switch_polling_helper()
			return true
		end
	elseif command.action == 'releaseaction' then
		if (event == 'select') or (event == 'left') or (event == 'right') then
			edit_current_macro.earlycancel = not edit_current_macro.earlycancel
			return true
		end
	elseif command.action == 'holdaction' then
		if event == 'left' then
			edit_current_macro.loop = edit_current_macro.loop - 1
			return true
		elseif event == 'right' then
			edit_current_macro.loop = edit_current_macro.loop + 1
			return true
		elseif event == 'clear' then
			edit_current_macro.loop = -1
			return true
		end
	elseif command.action == 'delay' then
		local step = edit_current_macro.steps[command.step]
		if event == 'left' then
			step.delay = step.delay - 1
			return true
		elseif event == 'right' then
			step.delay = step.delay + 1
			return true
		elseif event == 'clear' then
			step.delay = 0
			return true
		end
	elseif command.action == 'duration' then
		local step = edit_current_macro.steps[command.step]
		if event == 'left' then
			step.duration = step.duration - 1
			return true
		elseif event == 'right' then
			step.duration = step.duration + 1
			return true
		elseif event == 'clear' then
			step.duration = 1
			return true
		end
	elseif command.action == 'input' then
		local inputs = edit_current_macro.steps[command.step].inputs
		if event == 'select' then
			local hanlder =
				function(field)
					inputs[command.input].port = field.port
					inputs[command.input].field = field
				end
			start_input_menu(hanlder, inputs[command.input].field)
			edit_start_selection = index
			return true
		elseif event == 'clear' then
			if #inputs > 1 then
				table.remove(inputs, command.input)
				return true
			end
		end
	elseif command.action == 'addinput' then
		if event == 'select' then
			local inputs = edit_current_macro.steps[command.step].inputs
			local handler =
				function(field)
					inputs[#inputs + 1] = {
						port = field.port,
						field = field }
				end
			start_input_menu(handler)
			edit_start_selection = index
			return true
		end
	elseif command.action == 'deletestep' then
		if event == 'select' then
			table.remove(edit_current_macro.steps, command.step)
			if edit_current_macro.loop > #edit_current_macro.steps then
				edit_current_macro.loop = -1
			elseif edit_current_macro.loop > command.step then
				edit_current_macro.loop = edit_current_macro.loop - 1
			end
			if edit_insert_position > command.step then
				edit_insert_position = edit_insert_position - 1
			end
			edit_start_step = command.step
			if edit_start_step > #edit_current_macro.steps then
				edit_start_step = edit_start_step - 1
			end
			return true
		end
	elseif command.action == 'addstep' then
		if event == 'select' then
			local steps = edit_current_macro.steps
			local handler =
				function(field)
					local newstep = {
						inputs = {
							{
								port = field.port,
								field = field } },
						delay = 0,
						duration = 1 }
					table.insert(steps, edit_insert_position, newstep)
					if edit_current_macro.loop >= edit_insert_position then
						edit_current_macro.loop = edit_current_macro.loop + 1
					end
					edit_start_step = edit_insert_position
					edit_insert_position = edit_insert_position + 1
				end
			start_input_menu(handler)
			edit_start_selection = index
			return true
		elseif event == 'left' then
			edit_insert_position = edit_insert_position - 1
			return true
		elseif event == 'right' then
			edit_insert_position = edit_insert_position + 1
			return true
		end
	end

	return namecancel
end

local function add_edit_items(items)
	edit_items = { }
	local input = manager.machine.input
	local arrows

	items[#items + 1] = { _p('plugin-inputmacro', 'Name'), edit_name_buffer and (edit_name_buffer .. '_') or edit_current_macro.name, '' }
	edit_items[#items] = { action = 'name' }
	if not (edit_start_selection or edit_start_step or edit_menu_active) then
		edit_start_selection = #items
	end
	edit_menu_active = true

	local binding = edit_current_macro.binding
	local activation = binding and input:seq_name(binding) or _p('plugin-inputmacro', '[not set]')
	items[#items + 1] = { _p('plugin-inputmacro', 'Activation sequence'), activation, edit_switch_poller and 'lr' or '' }
	edit_items[#items] = { action = 'binding' }

	local releaseaction = edit_current_macro.earlycancel and _p('plugin-inputmacro', 'Stop immediately') or _p('plugin-inputmacro', 'Complete macro')
	items[#items + 1] = { _p('plugin-inputmacro', 'On release'), releaseaction, edit_current_macro.earlycancel and 'r' or 'l' }
	edit_items[#items] = { action = 'releaseaction' }

	local holdaction
	arrows = 'lr'
	if edit_current_macro.loop < 0 then
		holdaction = _p('plugin-inputmacro', 'Release')
		arrows = 'r'
	elseif edit_current_macro.loop > 0 then
		holdaction = string.format(_p('plugin-inputmacro', 'Loop to step %d'), edit_current_macro.loop)
		if edit_current_macro.loop >= #edit_current_macro.steps then
			arrows = 'l'
		end
	else
		holdaction = string.format(_p('plugin-inputmacro', 'Prolong step %d'), #edit_current_macro.steps)
	end
	items[#items + 1] = { _p('plugin-inputmacro', 'When held'), holdaction, arrows }
	edit_items[#items] = { action = 'holdaction' }

	for i, step in ipairs(edit_current_macro.steps) do
		items[#items + 1] = { string.format(_p('plugin-inputmacro', 'Step %d'), i), '', 'heading' }
		items[#items + 1] = { _p('plugin-inputmacro', 'Delay (frames)'), step.delay, (step.delay > 0) and 'lr' or 'r' }
		edit_items[#items] = { action = 'delay', step = i }
		if edit_start_step == i then
			edit_start_selection = #items
		end

		items[#items + 1] = { _p('plugin-inputmacro', 'Duration (frames)'), step.duration, (step.duration > 1) and 'lr' or 'r' }
		edit_items[#items] = { action = 'duration', step = i }

		for j, input in ipairs(step.inputs) do
			local inputname = input.field and _p('input-name', input.field.name) or _p('plugin-inputmacro', '[not set]')
			items[#items + 1] = { string.format(_p('plugin-inputmacro', 'Input %d'), j), inputname, '' }
			edit_items[#items] = { action = 'input', step = i, input = j }
		end

		if step.inputs[#step.inputs].field then
			items[#items + 1] = { _p('plugin-inputmacro', 'Add input'), '', '' }
			edit_items[#items] = { action = 'addinput', step = i }
		end

		if #edit_current_macro.steps > 1 then
			items[#items + 1] = { _p('plugin-inputmacro', 'Delete step'), '', '' }
			edit_items[#items] = { action = 'deletestep', step = i }
		end
	end
	edit_start_step = nil

	local laststep = edit_current_macro.steps[#edit_current_macro.steps]
	if laststep.inputs[#laststep.inputs].field then
		items[#items + 1] = { '---', '', '' }

		arrows = 'lr'
		if edit_insert_position > #edit_current_macro.steps then
			arrows = 'l'
		elseif edit_insert_position < 2 then
			arrows = 'r'
		end
		items[#items + 1] = { _p('plugin-inputmacro', 'Add step at position'), edit_insert_position, arrows }
		edit_items[#items] = { action = 'addstep', step = i }
	end
end

local function handle_add(index, event)
	if handle_edit_items(index, event) then
		return true
	elseif event == 'cancel' then
		edit_current_macro = nil
		edit_menu_active = false
		edit_items = nil
		table.remove(menu_stack)
		return true
	elseif (index == edit_item_exit) and (event == 'select') then
		if current_macro_complete() then
			table.insert(macros, edit_current_macro)
			macros_start_macro = #macros
		end
		edit_menu_active = false
		edit_current_macro = nil
		edit_items = nil
		table.remove(menu_stack)
		return true
	end
	return false
end

local function handle_edit(index, event)
	if handle_edit_items(index, event) then
		return true
	elseif (event == 'cancel') or ((index == edit_item_exit) and (event == 'select')) then
		edit_current_macro = nil
		edit_menu_active = false
		edit_items = nil
		table.remove(menu_stack)
		return true
	end
	return false
end

local function populate_add()
	local items = { }

	items[#items + 1] = { _p('plugin-inputmacro', 'Add Input Macro'), '', 'off' }
	items[#items + 1] = { '---', '', '' }

	add_edit_items(items)

	items[#items + 1] = { '---', '', '' }
	if current_macro_complete() then
		items[#items + 1] = { _p('plugin-inputmacro', 'Create'), '', '' }
	else
		items[#items + 1] = { _p('plugin-inputmacro', 'Cancel'), '', '' }
	end
	edit_item_exit = #items

	local selection = edit_start_selection
	edit_start_selection = nil
	if edit_switch_poller then
		return edit_switch_poller:overlay(items, selection, 'lrrepeat')
	else
		return items, selection, 'lrrepeat'
	end
end

local function populate_edit()
	local items = { }

	items[#items + 1] = { _p('plugin-inputmacro', 'Edit Input Macro'), '', 'off' }
	items[#items + 1] = { '---', '', '' }

	add_edit_items(items)

	items[#items + 1] = { '---', '', '' }
	items[#items + 1] = { _p('plugin-inputmacro', 'Done'), '', '' }
	edit_item_exit = #items

	local selection = edit_start_selection
	edit_start_selection = nil
	if edit_switch_poller then
		return edit_switch_poller:overlay(items, selection, 'lrrepeat')
	else
		return items, selection, 'lrrepeat'
	end
end


-- Macros menu

local macros_item_first_macro
local macros_selection_save
local macros_item_add

function handle_macros(index, event)
	if index == macros_item_add then
		if event == 'select' then
			edit_current_macro = new_macro()
			edit_insert_position = #edit_current_macro.steps + 1
			macros_selection_save = index
			table.insert(menu_stack, MENU_TYPES.ADD)
			return true
		end
	elseif index >= macros_item_first_macro then
		macro = index - macros_item_first_macro + 1
		if event == 'select' then
			edit_current_macro = macros[macro]
			edit_insert_position = #edit_current_macro.steps + 1
			macros_selection_save = index
			table.insert(menu_stack, MENU_TYPES.EDIT)
			return true
		elseif event == 'clear' then
			table.remove(macros, macro)
			if #macros > 0 then
				macros_selection_save = index
				if macro > #macros then
					macros_selection_save = macros_selection_save - 1
				end
			end
			return true
		end
	end
	return false
end

function populate_macros()
	local input = manager.machine.input
	local ioport = manager.machine.ioport
	local items = { }

	items[#items + 1] = { _p('plugin-inputmacro', 'Input Macros'), '', 'off' }
	items[#items + 1] = { string.format(_p('plugin-inputmacro', 'Press %s to delete'), manager.ui:get_general_input_setting(ioport:token_to_input_type('UI_CLEAR'))), '', 'off' }
	items[#items + 1] = { '---', '', '' }

	macros_item_first_macro = #items + 1
	if #macros > 0 then
		for index, macro in ipairs(macros) do
			items[#items + 1] = { macro.name, input:seq_name(macro.binding), '' }
			if macros_start_macro == index then
				macros_selection_save = #items
			end
		end
	else
		items[#items + 1] = { _p('plugin-inputmacro', '[no macros]'), '', 'off' }
	end
	macros_start_macro = nil

	items[#items + 1] = { '---', '', '' }
	items[#items + 1] = { _p('plugin-inputmacro', 'Add macro'), '', '' }
	macros_item_add = #items

	local selection = macros_selection_save
	macros_selection_save = nil
	return items, selection
end


-- Entry points

local lib = { }

function lib:init(m)
	macros = m
	menu_stack = { MENU_TYPES.MACROS }
end

function lib:handle_event(index, event)
	local current = menu_stack[#menu_stack]
	if current == MENU_TYPES.MACROS then
		return handle_macros(index, event)
	elseif current == MENU_TYPES.ADD then
		return handle_add(index, event)
	elseif current == MENU_TYPES.EDIT then
		return handle_edit(index, event)
	elseif current == MENU_TYPES.INPUT then
		return handle_input(index, event)
	end
end

function lib:populate()
	local current = menu_stack[#menu_stack]
	if current == MENU_TYPES.MACROS then
		return populate_macros()
	elseif current == MENU_TYPES.ADD then
		return populate_add()
	elseif current == MENU_TYPES.EDIT then
		return populate_edit()
	elseif current == MENU_TYPES.INPUT then
		return populate_input()
	end
end

return lib
