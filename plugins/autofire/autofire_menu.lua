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

-- Button to select when showing the main menu (so newly added button can be selected)
local initial_button

-- Saved selection on main menu (to restore after configure menu is dismissed)
local main_selection_save

-- Whether configure menu is active (so first item can be selected initially)
local configure_menu_active = false

-- Saved selection on configure menu (to restore after button menu is dismissed)
local configure_selection_save

-- Button being created/edited
local current_button = {}

-- Initial button to select when opening buttons menu
local initial_input

-- Inputs that can be autofired (to list in BUTTON menu)
local inputs

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
	local ioport = manager.machine.ioport
	local input = manager.machine.input
	local menu = {}
	menu[#menu + 1] = {_('Autofire buttons'), '', 'off'}
	menu[#menu + 1] = {string.format(_('Press %s to delete'), input:seq_name(ioport:type_seq(ioport:token_to_input_type('UI_CLEAR')))), '', 'off'}
	menu[#menu + 1] = {'---', '', ''}
	header_height = #menu

	-- Use frame rate of first screen or 60Hz if no screens
	local freq = 60
	local screen = manager.machine.screens:at(1)
	if screen then
		freq = 1 / screen.frame_period
	end

	for index, button in ipairs(buttons) do
		-- Round rate to two decimal places
		local rate = freq / (button.on_frames + button.off_frames)
		rate = math.floor(rate * 100) / 100
		local text = string.format(_('%s [%g Hz]'), _p('input-name', button.button.name), rate)
		local subtext = input:seq_name(button.key)
		menu[#menu + 1] = {text, subtext, ''}
		if index == initial_button then
			main_selection_save = #menu
		end
	end
	initial_button = nil
	content_height = #menu

	menu[#menu + 1] = {'---', '', ''}
	menu[#menu + 1] = {_('Add autofire button'), '', ''}

	local selection = main_selection_save
	main_selection_save = nil
	return menu, selection
end

local function handle_main_menu(index, event, buttons)
	local section, adjusted_index = menu_section(index)
	if section == MENU_SECTIONS.CONTENT then
		if event == 'select' then
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
	local button_name = current_button.button and _p('input-name', current_button.button.name) or _('NOT SET')
	local key_name = current_button.key and manager.machine.input:seq_name(current_button.key) or _('NOT SET')
	menu[#menu + 1] = {_('Input'), button_name, ''}
	if not (configure_menu_active or configure_selection_save) then
		configure_selection_save = #menu
	end
	menu[#menu + 1] = {_('Hotkey'), key_name, ''}
	menu[#menu + 1] = {_('On frames'), current_button.on_frames, current_button.on_frames > 1 and 'lr' or 'r'}
	menu[#menu + 1] = {_('Off frames'), current_button.off_frames, current_button.off_frames > 1 and 'lr' or 'r'}
	configure_menu_active = true
end

-- Borrowed from the cheat plugin
local function poll_for_hotkey()
	local input = manager.machine.input
	local poller = input:switch_sequence_poller()
	manager.machine:popmessage(_('Press button for hotkey or wait to leave unchanged'))
	manager.machine.video:frame_update()
	poller:start()
	local time = os.clock()
	local clearmsg = true
	while (not poller:poll()) and (poller.modified or (os.clock() < time + 1)) do
		if poller.modified then
			if not poller.valid then
				manager.machine:popmessage(_('Invalid sequence entered'))
				clearmsg = false
				break
			end
			manager.machine:popmessage(input:seq_name(poller.sequence))
			manager.machine.video:frame_update()
		end
	end
	if clearmsg then
		manager.machine:popmessage()
	end
	return poller.valid and poller.sequence or nil
end

local function handle_configure_menu(index, event)
	if index == 1 then
		-- Input
		if event == 'select' then
			configure_selection_save = header_height + index
			table.insert(menu_stack, MENU_TYPES.BUTTON)
			if current_button.port and current_button.field then
				initial_input = {port_name = current_button.port, ioport_field = current_button.button}
			end
			return true
		end
	elseif index == 2 then
		-- Hotkey
		if event == 'select' then
			local keycode = poll_for_hotkey()
			if keycode then
				current_button.key = keycode
				return true
			end
		end
	elseif index == 3 then
		-- On frames
		manager.machine:popmessage(_('Number of frames button will be pressed'))
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
		manager.machine:popmessage(_('Number of frames button will be released'))
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
	menu[#menu + 1] = {_('Edit autofire button'), '', 'off'}
	menu[#menu + 1] = {'---', '', ''}
	header_height = #menu

	populate_configure_menu(menu)
	content_height = #menu

	menu[#menu + 1] = {'---', '', ''}
	menu[#menu + 1] = {_('Done'), '', ''}

	local selection = configure_selection_save
	configure_selection_save = nil
	return menu, selection, 'lrrepeat'
end

local function handle_edit_menu(index, event, buttons)
	local section, adjusted_index = menu_section(index)
	if ((section == MENU_SECTIONS.FOOTER) and (event == 'select')) or (event == 'cancel') then
		inputs = nil
		configure_menu_active = false
		table.remove(menu_stack)
		return true
	elseif section == MENU_SECTIONS.CONTENT then
		return handle_configure_menu(adjusted_index, event)
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

	local selection = configure_selection_save
	configure_selection_save = nil
	return menu, selection, 'lrrepeat'
end

local function handle_add_menu(index, event, buttons)
	local section, adjusted_index = menu_section(index)
	if ((section == MENU_SECTIONS.FOOTER) and (event == 'select')) or (event == 'cancel') then
		inputs = nil
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
	local ioport = manager.machine.ioport
	menu = {}
	menu[#menu + 1] = {_('Select an input for autofire'), '', 'off'}
	menu[#menu + 1] = {'---', '', ''}
	header_height = #menu

	if not inputs then
		inputs = {}

		for port_key, port in pairs(ioport.ports) do
			for field_key, field in pairs(port.fields) do
				if is_supported_input(field) then
					inputs[#inputs + 1] = {
						port_name = port_key,
						field_name = field_key,
						ioport_field = field
					}
				end
			end
		end

		local function compare(x, y)
			if x.ioport_field.device.tag < y.ioport_field.device.tag then
				return true
			elseif x.ioport_field.device.tag > y.ioport_field.device.tag then
				return false
			end
			groupx = ioport:type_group(x.ioport_field.type, x.ioport_field.player)
			groupy = ioport:type_group(y.ioport_field.type, y.ioport_field.player)
			if groupx < groupy then
				return true
			elseif groupx > groupy then
				return false
			elseif x.ioport_field.type < y.ioport_field.type then
				return true
			elseif x.ioport_field.type > y.ioport_field.type then
				return false
			else
				return x.ioport_field.name < y.ioport_field.name
			end
		end
		table.sort(inputs, compare)

		local i = 1
		local prev
		while i <= #inputs do
			local current = inputs[i]
			if (not prev) or (prev.ioport_field.device.tag ~= current.ioport_field.device.tag) then
				table.insert(inputs, i, false)
				i = i + 2
			else
				i = i + 1
			end
			prev = current
		end
	end

	local selection = header_height + 1
	for i, input in ipairs(inputs) do
		if input then
			menu[header_height + i] = { _p('input-name', input.ioport_field.name), '', '' }
			if initial_input and (initial_input.port_name == input.port_name) and (initial_input.ioport_field.mask == input.ioport_field.mask) and (initial_input.ioport_field.type == input.ioport_field.type) then
				selection = header_height + i
				initial_input = nil
			end
		else
			local device = inputs[i + 1].ioport_field.device
			if device.owner then
				menu[header_height + i] = {string.format(_('%s [root%s]'), device.name, device.tag), '', 'heading'}
			else
				menu[header_height + i] = {string.format(_('[root%s]'), device.tag), '', 'heading'}
			end
		end
	end
	content_height = #menu
	initial_input = nil

	menu[#menu + 1] = {'---', '', ''}
	menu[#menu + 1] = {_('Cancel'), '', ''}

	return menu, selection
end

local function handle_button_menu(index, event)
	local section, adjusted_index = menu_section(index)
	if ((section == MENU_SECTIONS.FOOTER) and (event == 'select')) or (event == 'cancel') then
		table.remove(menu_stack)
		return true
	elseif (section == MENU_SECTIONS.CONTENT) and (event == 'select') then
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
	inputs = nil
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
