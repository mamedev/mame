-- license:BSD-3-Clause
-- copyright-holders:Vas Crabb


-- Constants

local MENU_TYPES = { HELPERS = 0, ADD = 1, EDIT = 2, INPUT = 3 }


-- Globals

local commonui
local helpers
local menu_stack

local helpers_start_helper -- really for the helpers menu, but have to be declared local before edit menu functions
local helpers_item_first_helper
local helpers_selection_save


-- Helpers

local function new_helper()
	return {
		binding = nil,
		bindingcfg = '',
		axis = { port = nil, mask = nil, type = nil, field = nil },
		button = { port = nil, mask = nil, type = nil, field = nil } }
end

local function input_display_name(input)
	if input.field then
		return input.field.name
	elseif input.port then
		return _p('plugin-offscreenreload', 'n/a')
	else
		return _p('plugin-offscreenreload', '[not set]')
	end
end


-- Input menu

local input_menu
local input_start_field

local function start_input_menu(handler, title, filter, start_field)
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
	input_menu = commonui.input_selection_menu(action, title, filter)
	input_start_field = start_field
	table.insert(menu_stack, MENU_TYPES.INPUT)
end

local function handle_input(index, action)
	return input_menu:handle(index, action)
end

local function populate_input()
	return input_menu:populate(input_start_field)
end


-- Add/edit menus

local edit_current_helper
local edit_start_selection
local edit_menu_active
local edit_items
local edit_item_delete
local edit_item_exit
local edit_switch_poller

local function current_helper_complete()
	if not edit_current_helper.binding then
		return false
	end
	if not edit_current_helper.axis.field then
		return false
	end
	if not edit_current_helper.button.field then
		return false
	end
	return true
end

local function handle_edit_items(index, event)
	if edit_switch_poller then
		if edit_switch_poller:poll() then
			if edit_switch_poller.sequence then
				edit_current_helper.binding = edit_switch_poller.sequence
				edit_current_helper.bindingcfg = manager.machine.input:seq_to_tokens(edit_switch_poller.sequence)
			end
			edit_switch_poller = nil
			return true
		end
		return false
	end

	local command = edit_items[index]

	if (event ~= 'select') or (not command) then
		return false
	elseif command.action == 'binding' then
		if not commonui then
			commonui = require('commonui')
		end
		edit_switch_poller = commonui.switch_polling_helper()
		return true
	elseif command.action == 'axis' then
		local function handler(field)
			local device = field.device.tag
			local player = field.player
			local guessbutton = false
			if (not edit_current_helper.axis) or (not edit_current_helper.axis.field) then
				guessbutton = true
			elseif (edit_current_helper.axis.field.device.tag ~= field) or (edit_current_helper.axis.field.player ~= player) then
				guessbutton = true
			end

			edit_current_helper.axis.port = field.port.tag
			edit_current_helper.axis.mask = field.mask
			edit_current_helper.axis.type = field.type
			edit_current_helper.axis.field = field

			if guessbutton then
				local ioport = manager.machine.ioport
				local b1type = ioport:token_to_input_type('P1_BUTTON1')
				local done = false
				for i, port in pairs(ioport.ports) do
					if port.device.tag == device then
						for j, f in pairs(port.fields) do
							if (f.player == player) and (f.type == b1type) then
								edit_current_helper.button.port = f.port.tag
								edit_current_helper.button.mask = f.mask
								edit_current_helper.button.type = f.type
								edit_current_helper.button.field = f
								done = true
								break
							end
						end
					end
					if done then
						break
					end
				end
			end
		end
		local function filter(field)
			if (not field.is_analog) or field.analog_wraps then
				return false
			else
				return true
			end
		end
		start_input_menu(handler, _p('plugin-offscreenreload', 'Set Axis'), filter, edit_current_helper.axis.field)
		edit_start_selection = index
		return true
	elseif command.action == 'button' then
		local function handler(field)
			edit_current_helper.button.port = field.port.tag
			edit_current_helper.button.mask = field.mask
			edit_current_helper.button.type = field.type
			edit_current_helper.button.field = field
		end
		local function filter(field)
			if field.is_analog or field.is_toggle then
				return false
			elseif (field.type_class == 'config') or (field.type_class == 'dipswitch') then
				return false
			else
				return true
			end
		end
		start_input_menu(handler, _p('plugin-offscreenreload', 'Set Trigger'), filter, edit_current_helper.axis.button)
		edit_start_selection = index
		return true
	end

	return false
end

local function add_edit_items(items)
	edit_items = { }
	local input = manager.machine.input

	local binding = edit_current_helper.binding
	local activation = binding and input:seq_name(binding) or _p('plugin-offscreenreload', '[not set]')
	table.insert(items, { _p('plugin-offscreenreload', 'Reload combination'), activation, edit_switch_poller and 'lr' or '' })
	edit_items[#items] = { action = 'binding' }
	if not (edit_start_selection or edit_menu_active) then
		edit_start_selection = #items
	end
	edit_menu_active = true

	local axisname = input_display_name(edit_current_helper.axis)
	table.insert(items, { _p('plugin-offscreenreload', 'Axis input'), axisname, '' })
	edit_items[#items] = { action = 'axis' }

	local buttonname = input_display_name(edit_current_helper.button)
	table.insert(items, { _p('plugin-offscreenreload', 'Trigger input'), buttonname, '' })
	edit_items[#items] = { action = 'button' }
end

local function handle_add(index, event)
	local handled, selection = handle_edit_items(index, event)
	if handled then
		return true, selection
	elseif event == 'back' then
		edit_current_helper = nil
		edit_menu_active = false
		edit_items = nil
		table.remove(menu_stack)
		return true, selection
	elseif (index == edit_item_exit) and (event == 'select') then
		if current_helper_complete() then
			table.insert(helpers, edit_current_helper)
			helpers_start_helper = #helpers
		end
		edit_current_helper = nil
		edit_menu_active = false
		edit_items = nil
		table.remove(menu_stack)
		return true, selection
	end
	return false, selection
end

local function handle_edit(index, event)
	local handled, selection = handle_edit_items(index, event)
	if handled then
		return true, selection
	elseif (index == edit_item_delete) and (event == 'select') then
		local helper = helpers_selection_save - helpers_item_first_helper + 1
		table.remove(helpers, helper)
		if helper > #helpers then
			helpers_selection_save = helpers_selection_save - 1
		end
		edit_current_macro = nil
		edit_menu_active = false
		edit_items = nil
		table.remove(menu_stack)
		return true, selection
	elseif (event == 'back') or ((index == edit_item_exit) and (event == 'select')) then
		edit_current_helper = nil
		edit_menu_active = false
		edit_items = nil
		table.remove(menu_stack)
		return true, selection
	end
	return false, selection
end

local function populate_add()
	local items = { }

	table.insert(items, { _p('plugin-offscreenreload', 'Add Reload Helper'), '', 'off' })
	table.insert(items, { '---', '', '' })

	add_edit_items(items)

	table.insert(items, { '---', '', '' })
	if current_helper_complete() then
		table.insert(items, { _p('plugin-offscreenreload', 'Create'), '', '' })
	else
		table.insert(items, { _p('plugin-offscreenreload', 'Cancel'), '', '' })
	end
	edit_item_exit = #items

	local selection = edit_start_selection
	edit_start_selection = nil
	if edit_switch_poller then
		return edit_switch_poller:overlay(items, selection, '')
	else
		return items, selection, 'lrrepeat'
	end
end

local function populate_edit()
	local items = { }

	table.insert(items, { _p('plugin-offscreenreload', 'Edit Reload Helper'), '', 'off' })
	table.insert(items, { '---', '', '' })

	add_edit_items(items)

	table.insert(items, { '---', '', '' })
	table.insert(items, { _p('plugin-offscreenreload', 'Delete reload helper'), '', '' })
	edit_item_delete = #items

	table.insert(items, { '---', '', '' })
	table.insert(items, { _p('plugin-offscreenreload', 'Done'), '', '' })
	edit_item_exit = #items

	local selection = edit_start_selection
	edit_start_selection = nil
	if edit_switch_poller then
		return edit_switch_poller:overlay(items, selection, 'lrrepeat')
	else
		return items, selection, 'lrrepeat'
	end
end


-- Helpers menu

local helpers_item_add

function handle_helpers(index, event)
	if index == helpers_item_add then
		if event == 'select' then
			edit_current_helper = new_helper()
			helpers_selection_save = index
			table.insert(menu_stack, MENU_TYPES.ADD)
			return true
		end
	elseif index >= helpers_item_first_helper then
		local helper = index - helpers_item_first_helper + 1
		if event == 'select' then
			edit_current_helper = helpers[helper]
			helpers_selection_save = index
			table.insert(menu_stack, MENU_TYPES.EDIT)
			return true
		elseif event == 'clear' then
			table.remove(helpers, helper)
			if #helpers > 0 then
				helpers_selection_save = index
				if helper > #helpers then
					helpers_selection_save = helpers_selection_save - 1
				end
			end
			return true
		end
	end
	return false
end

local function populate_helpers()
	local input = manager.machine.input
	local ioport = manager.machine.ioport
	local items = { }

	table.insert(items, { _p('plugin-offscreenreload', 'Off-Screen Reload Helper'), '', 'off' })
	table.insert(items, { string.format(_p('plugin-offscreenreload', 'Press %s to delete'), manager.ui:get_general_input_setting(ioport:token_to_input_type('UI_CLEAR'))), '', 'off' })
	table.insert(items, { '---', '', '' })

	helpers_item_first_helper = #items + 1
	if #helpers > 0 then
		for index, helper in ipairs(helpers) do
			local bindingname = input:seq_name(helper.binding)
			local axisname = input_display_name(helper.axis)
			local buttonname = input_display_name(helper.button)
			table.insert(items, { string.format(_p('plugin-offscreenreload', '%s/%s'), axisname, buttonname), bindingname, '' })
			if helpers_start_helper == index then
				helpers_selection_save = #items
			end
		end
	else
		table.insert(items, { _p('plugin-offscreenreload', '[no reload helpers]'), '', 'off' })
	end
	helpers_start_helper = nil

	table.insert(items, { '---', '', '' })
	table.insert(items, { _p('plugin-offscreenreload', 'Add reload helper'), '', '' })
	helpers_item_add = #items

	local selection = helpers_selection_save
	helpers_selection_save = nil
	return items, selection
end


-- Entry points

local lib = { }

function lib:init(h)
	helpers = h
	menu_stack = { MENU_TYPES.HELPERS }
end

function lib:handle_event(index, event)
	local current = menu_stack[#menu_stack]
	if current == MENU_TYPES.HELPERS then
		return handle_helpers(index, event)
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
	if current == MENU_TYPES.HELPERS then
		return populate_helpers()
	elseif current == MENU_TYPES.ADD then
		return populate_add()
	elseif current == MENU_TYPES.EDIT then
		return populate_edit()
	elseif current == MENU_TYPES.INPUT then
		return populate_input()
	end
end

return lib
