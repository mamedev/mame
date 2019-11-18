local lib = {}

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

-- Button being created/edited
local current_button = {}

-- Inputs that can be autofired (to list in BUTTON menu)
local inputs = {}

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
	return button.port and button.field and button.key and button.on_frames and button.off_frames and button.button and button.counter
end

local function is_supported_input(ioport_field)
	-- IPT_BUTTON1 through IPT_BUTTON16 in ioport_type enum (ioport.h)
	return ioport_field.type >= 64 and ioport_field.type <= 79
end

-- Main menu

local function populate_main_menu(buttons)
	local menu = {}
	menu[#menu + 1] = {_('Autofire buttons'), '', 'off'}
	menu[#menu + 1] = {'---', '', ''}
	header_height = #menu

	for index, button in ipairs(buttons) do
		-- Assume refresh rate of 60 Hz; maybe better to use screen_device refresh()?
		local rate = 60 / (button.on_frames + button.off_frames)
		-- Round to two decimal places
		rate = math.floor(rate * 100) / 100
		local text = button.button.name .. ' [' .. rate .. ' Hz]'
		local subtext = manager:machine():input():code_name(button.key)
		menu[#menu + 1] = {text, subtext, ''}
	end
	content_height = #menu

	menu[#menu + 1] = {'---', '', ''}
	menu[#menu + 1] = {_('Add autofire button'), '', ''}
	return menu
end

local function handle_main_menu(index, event, buttons)
	local section, adjusted_index = menu_section(index)
	if section == MENU_SECTIONS.CONTENT then
		if event == 'select' then
			current_button = buttons[adjusted_index]
			table.insert(menu_stack, MENU_TYPES.EDIT)
			return true
		elseif event == 'clear' then
			table.remove(buttons, adjusted_index)
			return true
		end
	elseif section == MENU_SECTIONS.FOOTER then
		if event == 'select' then
			current_button = create_new_button()
			table.insert(menu_stack, MENU_TYPES.ADD)
			return true
		end
	end
	return false
end

-- Add/edit menus (mostly identical)

local function populate_configure_menu(menu)
	local button_name = current_button.button and current_button.button.name or _('NOT SET')
	local key_name = current_button.key and manager:machine():input():code_name(current_button.key) or _('NOT SET')
	menu[#menu + 1] = {_('Input'), button_name, ''}
	menu[#menu + 1] = {_('Hotkey'), key_name, ''}
	menu[#menu + 1] = {_('On frames'), current_button.on_frames, current_button.on_frames > 1 and 'lr' or 'r'}
	menu[#menu + 1] = {_('Off frames'), current_button.off_frames, current_button.off_frames > 1 and 'lr' or 'r'}
end

-- Borrowed from the cheat plugin
local function poll_for_hotkey()
	local input = manager:machine():input()
	manager:machine():popmessage(_('Press button for hotkey or wait to leave unchanged'))
	manager:machine():video():frame_update(true)
	input:seq_poll_start('switch')
	local time = os.clock()
	while (not input:seq_poll()) and (os.clock() < time + 1) do end
	local tokens = input:seq_to_tokens(input:seq_poll_final())
	manager:machine():popmessage()
	manager:machine():video():frame_update(true)

	local final_token = nil
	for token in tokens:gmatch('%S+') do
		final_token = token
	end
	return final_token and input:code_from_token(final_token) or nil
end

local function handle_configure_menu(index, event)
	-- Input
	if index == 1 then
		if event == 'select' then
			table.insert(menu_stack, MENU_TYPES.BUTTON)
			return true
		else
			return false
		end
	-- Hotkey
	elseif index == 2 then
		if event == 'select' then
			local keycode = poll_for_hotkey()
			if keycode then
				current_button.key = keycode
				return true
			else
				return false
			end
		else
			return false
		end
	-- On frames
	elseif index == 3 then
		manager:machine():popmessage(_('Number of frames button will be pressed'))
		if event == 'left' then
			current_button.on_frames = current_button.on_frames - 1
		elseif event == 'right' then
			current_button.on_frames = current_button.on_frames + 1
		end
	-- Off frames
	elseif index == 4 then
		manager:machine():popmessage(_('Number of frames button will be released'))
		if event == 'left' then
			current_button.off_frames = current_button.off_frames - 1
		elseif event == 'right' then
			current_button.off_frames = current_button.off_frames + 1
		end
	end
	return true
end

local function populate_edit_menu()
	local menu = {}
	menu[#menu + 1] = {_('Edit autofire button'), '', 'off'}
	menu[#menu + 1] = {'---', '', ''}
	header_height = #menu

	populate_configure_menu(menu)
	content_height = #menu

	menu[#menu + 1] = {'---', '', ''}
	menu[#menu + 1] = {_('Done'), '', ''}
	return menu
end

local function handle_edit_menu(index, event, buttons)
	local section, adjusted_index = menu_section(index)
	if section == MENU_SECTIONS.CONTENT then
		return handle_configure_menu(adjusted_index, event)
	elseif section == MENU_SECTIONS.FOOTER then
		if event == 'select' then
			table.remove(menu_stack)
			return true
		end
	end
	return false
end

local function populate_add_menu()
	local menu = {}
	menu[#menu + 1] = {_('Add autofire button'), '', 'off'}
	menu[#menu + 1] = {'---', '', ''}
	header_height = #menu

	populate_configure_menu(menu)
	content_height = #menu

	menu[#menu + 1] = {'---', '', ''}
	if is_button_complete(current_button) then
		menu[#menu + 1] = {_('Create'), '', ''}
	else
		menu[#menu + 1] = {_('Cancel'), '', ''}
	end
	return menu
end

local function handle_add_menu(index, event, buttons)
	local section, adjusted_index = menu_section(index)
	if section == MENU_SECTIONS.CONTENT then
		return handle_configure_menu(adjusted_index, event)
	elseif section == MENU_SECTIONS.FOOTER then
		if event == 'select' then
			table.remove(menu_stack)
			if is_button_complete(current_button) then
				buttons[#buttons + 1] = current_button
			end
			return true
		end
	end
	return false
end

-- Button selection menu

local function populate_button_menu()
	menu = {}
	inputs = {}
	menu[#menu + 1] = {_('Select an input for autofire'), '', 'off'}
	menu[#menu + 1] = {'---', '', ''}
	header_height = #menu

	for port_key, port in pairs(manager:machine():ioport().ports) do
		for field_key, field in pairs(port.fields) do
			if is_supported_input(field) then
				menu[#menu + 1] = {field.name, '', ''}
				inputs[#inputs + 1] = {
					port_name = port_key,
					field_name = field_key,
					ioport_field = field
				}
			end
		end
	end
	content_height = #menu
	return menu
end

local function handle_button_menu(index, event)
	local section, adjusted_index = menu_section(index)
	if section == MENU_SECTIONS.CONTENT and event == 'select' then
		local selected_input = inputs[adjusted_index]
		current_button.port = selected_input.port_name
		current_button.field = selected_input.field_name
		current_button.button = selected_input.ioport_field
		table.remove(menu_stack)
		return true
	end
	return false
end

function lib:init_menu(buttons)
	header_height = 0
	content_height = 0
	menu_stack = { MENU_TYPES.MAIN }
	current_button = {}
	inputs = {}
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
	manager:machine():popmessage()
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
