local lib = {}

-- Common UI helper library
local commonui

-- Set of all menus
local MENU_TYPES = { MAIN = 0, EDIT = 1, ADD = 2, BUTTON = 3 }

-- Set of sections within a menu
local MENU_SECTIONS = { HEADER = 0, CONTENT = 1, FOOTER = 2 }

-- Last index of header items (above main content) in menu
local header_height = 0

-- Last index of content items (below header, above footer) in menu
local content_height = 0

-- Stack of menus (see MENU_TYPES)
local menu_stack = { MENU_TYPES.MAIN }

-- Button to select when showing the main menu (so newly added button can be selected)
local initial_button

-- Saved selection on main menu (to restore after configure menu is dismissed)
local main_selection_save

-- Whether configure menu is active (so first item can be selected initially)
local configure_menu_active = false

-- Saved selection on configure menu (to restore after button menu is dismissed)
local configure_selection_save

-- Helper for polling for hotkeys
local hotkey_poller

-- Button being created/edited
local current_button = {}

-- Initial button to select when opening buttons menu
local initial_input

-- Handler for BUTTON menu
local input_menu

-- Returns the section (from MENU_SECTIONS) and the index within that section
local function menu_section(index)
	if index <= header_height then
		return MENU_SECTIONS.HEADER, index
	elseif index <= content_height then
		return MENU_SECTIONS.CONTENT, index - header_height
	else
		return MENU_SECTIONS.FOOTER, index - content_height
	end
end

local function create_new_button()
	return {
		on_frames = 1,
		off_frames = 1,
		counter = 0
	}
end

local function is_button_complete(button)
	return button.port and button.mask and button.type and button.key and button.on_frames and button.off_frames and button.button and button.counter
end

-- Main menu

local function populate_main_menu(buttons)
	local ioport = manager.machine.ioport
	local input = manager.machine.input
	local menu = {}
	table.insert(menu, {_p('plugin-autofire', 'Autofire buttons'), '', 'off'})
	table.insert(menu, {string.format(_p('plugin-autofire', 'Press %s to delete'), manager.ui:get_general_input_setting(ioport:token_to_input_type('UI_CLEAR'))), '', 'off'})
	table.insert(menu, {'---', '', ''})
	header_height = #menu

	-- Use frame rate of first screen or 60Hz if no screens
	local freq = 60
	local screen = manager.machine.screens:at(1)
	if screen then
		freq = 1 / screen.frame_period
	end

	if #buttons > 0 then
		for index, button in ipairs(buttons) do
			-- Round rate to two decimal places
			local rate = freq / (button.on_frames + button.off_frames)
			rate = math.floor(rate * 100) / 100
			local text
			if button.button then
				text = string.format(_p('plugin-autofire', '%s [%g Hz]'), button.button.name, rate)
			else
				text = string.format(_p('plugin-autofire', 'n/a [%g Hz]'), rate)
			end
			table.insert(menu, {text, input:seq_name(button.key), ''})
			if index == initial_button then
				main_selection_save = #menu
			end
		end
	else
		table.insert(menu, {_p('plugin-autofire', '[no autofire buttons]'), '', 'off'})
	end
	initial_button = nil
	content_height = #menu

	table.insert(menu, {'---', '', ''})
	table.insert(menu, {_p('plugin-autofire', 'Add autofire button'), '', ''})

	local selection = main_selection_save
	main_selection_save = nil
	return menu, selection
end

local function handle_main_menu(index, event, buttons)
	local section, adjusted_index = menu_section(index)
	if section == MENU_SECTIONS.CONTENT then
		if event == 'select' then
			initial_button = adjusted_index
			main_selection_save = index
			current_button = buttons[adjusted_index]
			table.insert(menu_stack, MENU_TYPES.EDIT)
			return true
		elseif event == 'clear' then
			table.remove(buttons, adjusted_index)
			main_selection_save = index
			if adjusted_index > #buttons then
				main_selection_save = main_selection_save - 1
			end
			return true
		end
	elseif section == MENU_SECTIONS.FOOTER then
		if event == 'select' then
			main_selection_save = index
			current_button = create_new_button()
			table.insert(menu_stack, MENU_TYPES.ADD)
			return true
		end
	end
	return false
end

-- Add/edit menus (mostly identical)

local function populate_configure_menu(menu)
	local button_name
	if current_button.button then
		button_name = current_button.button.name
	elseif current_button.port then
		button_name = _p('plugin-autofire', 'n/a')
	else
		button_name = _p('plugin-autofire', '[not set]')
	end
	local key_name = current_button.key and manager.machine.input:seq_name(current_button.key) or _p('plugin-autofire', '[not set]')
	table.insert(menu, {_p('plugin-autofire', 'Input'), button_name, ''})
	if not (configure_menu_active or configure_selection_save) then
		configure_selection_save = #menu
	end
	table.insert(menu, {_p('plugin-autofire', 'Hotkey'), key_name, hotkey_poller and 'lr' or ''})
	table.insert(menu, {_p('plugin-autofire', 'On frames'), tostring(current_button.on_frames), current_button.on_frames > 1 and 'lr' or 'r'})
	table.insert(menu, {_p('plugin-autofire', 'Off frames'), tostring(current_button.off_frames), current_button.off_frames > 1 and 'lr' or 'r'})
	configure_menu_active = true
end

local function handle_configure_menu(index, event)
	if hotkey_poller then
		-- special handling for polling for hotkey
		if hotkey_poller:poll() then
			if hotkey_poller.sequence then
				current_button.key = hotkey_poller.sequence
				current_button.key_cfg = manager.machine.input:seq_to_tokens(hotkey_poller.sequence)
			end
			hotkey_poller = nil
			return true
		end
		return false
	end

	if index == 1 then
		-- Input
		if event == 'select' then
			configure_selection_save = header_height + index
			table.insert(menu_stack, MENU_TYPES.BUTTON)
			if current_button.port and current_button.button then
				initial_input = current_button.button
			end
			return true
		end
	elseif index == 2 then
		-- Hotkey
		if event == 'select' then
			if not commonui then
				commonui = require('commonui')
			end
			hotkey_poller = commonui.switch_polling_helper()
			return true
		end
	elseif index == 3 then
		-- On frames
		manager.machine:popmessage(_p('plugin-autofire', 'Number of frames button will be pressed'))
		if event == 'left' then
			current_button.on_frames = current_button.on_frames - 1
			return true
		elseif event == 'right' then
			current_button.on_frames = current_button.on_frames + 1
			return true
		elseif event == 'clear' then
			current_button.on_frames = 1
			return true
		end
	elseif index == 4 then
		-- Off frames
		manager.machine:popmessage(_p('plugin-autofire', 'Number of frames button will be released'))
		if event == 'left' then
			current_button.off_frames = current_button.off_frames - 1
			return true
		elseif event == 'right' then
			current_button.off_frames = current_button.off_frames + 1
			return true
		elseif event == 'clear' then
			current_button.off_frames = 1
			return true
		end
	end
	return false
end

local function populate_edit_menu()
	local menu = {}
	table.insert(menu, {_p('plugin-autofire', 'Edit autofire button'), '', 'off'})
	table.insert(menu, {'---', '', ''})
	header_height = #menu

	populate_configure_menu(menu)
	content_height = #menu

	table.insert(menu, {'---', '', ''})
	table.insert(menu, {_p('plugin-autofire', 'Delete'), '', ''})

	table.insert(menu, {'---', '', ''})
	table.insert(menu, {_p('plugin-autofire', 'Done'), '', ''})

	local selection = configure_selection_save
	configure_selection_save = nil
	if hotkey_poller then
		return hotkey_poller:overlay(menu, selection, 'lrrepeat')
	else
		return menu, selection, 'lrrepeat'
	end
end

local function handle_edit_menu(index, event, buttons)
	local section, adjusted_index = menu_section(index)
	if (section == MENU_SECTIONS.FOOTER) and (adjusted_index == 2) and (event == 'select') then
		table.remove(buttons, initial_button)
		if initial_button > #buttons then
			main_selection_save = main_selection_save - 1
		end
		initial_button = nil
		configure_menu_active = false
		table.remove(menu_stack)
		return true
	elseif ((section == MENU_SECTIONS.FOOTER) and (event == 'select')) or (event == 'back') then
		configure_menu_active = false
		initial_button = nil
		table.remove(menu_stack)
		return true
	elseif section == MENU_SECTIONS.CONTENT then
		return handle_configure_menu(adjusted_index, event)
	end
	return false
end

local function populate_add_menu()
	local menu = {}
	table.insert(menu, {_p('plugin-autofire', 'Add autofire button'), '', 'off'})
	table.insert(menu, {'---', '', ''})
	header_height = #menu

	populate_configure_menu(menu)
	content_height = #menu

	table.insert(menu, {'---', '', ''})
	if is_button_complete(current_button) then
		table.insert(menu, {_p('plugin-autofire', 'Create'), '', ''})
	else
		table.insert(menu, {_p('plugin-autofire', 'Cancel'), '', ''})
	end

	local selection = configure_selection_save
	configure_selection_save = nil
	if hotkey_poller then
		return hotkey_poller:overlay(menu, selection, 'lrrepeat')
	else
		return menu, selection, 'lrrepeat'
	end
end

local function handle_add_menu(index, event, buttons)
	local section, adjusted_index = menu_section(index)
	if ((section == MENU_SECTIONS.FOOTER) and (event == 'select')) or (event == 'back') then
		configure_menu_active = false
		table.remove(menu_stack)
		if is_button_complete(current_button) and (event == 'select') then
			table.insert(buttons, current_button)
			initial_button = #buttons
		end
		return true
	elseif section == MENU_SECTIONS.CONTENT then
		return handle_configure_menu(adjusted_index, event)
	end
	return false
end

-- Button selection menu

local function populate_button_menu()
	local function is_supported_input(ioport_field)
		if ioport_field.is_analog or ioport_field.is_toggle then
			return false
		elseif (ioport_field.type_class == 'config') or (ioport_field.type_class == 'dipswitch') then
			return false
		else
			return true
		end
	end

	local function action(field)
		if field then
			current_button.port = field.port.tag
			current_button.mask = field.mask
			current_button.type = field.type
			current_button.button = field
		end
		initial_input = nil
		input_menu = nil
		table.remove(menu_stack)
	end

	if not commonui then
		commonui = require('commonui')
	end
	input_menu = commonui.input_selection_menu(action, _p('plugin-autofire', 'Select an input for autofire'), is_supported_input)
	return input_menu:populate(initial_input)
end

local function handle_button_menu(index, event)
	return input_menu:handle(index, event)
end

function lib:init_menu(buttons)
	header_height = 0
	content_height = 0
	menu_stack = { MENU_TYPES.MAIN }
	current_button = {}
	input_menu = nil
end

function lib:populate_menu(buttons)
	local current_menu = menu_stack[#menu_stack]
	if current_menu == MENU_TYPES.MAIN then
		return populate_main_menu(buttons)
	elseif current_menu == MENU_TYPES.EDIT then
		return populate_edit_menu()
	elseif current_menu == MENU_TYPES.ADD then
		return populate_add_menu()
	elseif current_menu == MENU_TYPES.BUTTON then
		return populate_button_menu()
	end
end

function lib:handle_menu_event(index, event, buttons)
	manager.machine:popmessage()
	local current_menu = menu_stack[#menu_stack]
	if current_menu == MENU_TYPES.MAIN then
		return handle_main_menu(index, event, buttons)
	elseif current_menu == MENU_TYPES.EDIT then
		return handle_edit_menu(index, event, buttons)
	elseif current_menu == MENU_TYPES.ADD then
		return handle_add_menu(index, event, buttons)
	elseif current_menu == MENU_TYPES.BUTTON then
		return handle_button_menu(index, event)
	end
end

return lib
