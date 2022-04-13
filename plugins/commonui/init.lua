-- license:BSD-3-Clause
-- copyright-holders:Vas Crabb
local exports = {
	name = 'commonui',
	version = '0.0.1',
	description = 'Common plugin UI helpers',
	license = 'BSD-3-Clause',
	author = { name = 'Vas Crabb' } }


local commonui = exports


function commonui.input_selection_menu(action, title, filter)
	menu = { }

	local choices
	local index_first_choice
	local index_cancel

	local function populate_choices()
		local ioport = manager.machine.ioport

		local function compare(a, b)
			if a.device.tag < b.device.tag then
				return true
			elseif a.device.tag > b.device.tag then
				return false
			end
			groupa = ioport:type_group(a.type, a.player)
			groupb = ioport:type_group(b.type, b.player)
			if groupa < groupb then
				return true
			elseif groupa > groupb then
				return false
			elseif a.type < b.type then
				return true
			elseif a.type > b.type then
				return false
			else
				return a.name < b.name
			end
		end

		choices = { }
		for tag, port in pairs(manager.machine.ioport.ports) do
			for name, field in pairs(port.fields) do
				if (not filter) or filter(field) then
					table.insert(choices, field)
				end
			end
		end
		table.sort(choices, compare)

		local index = 1
		local prev
		while index <= #choices do
			local current = choices[index]
			if (not prev) or (prev.device.tag ~= current.device.tag) then
				table.insert(choices, index, false)
				index = index + 2
			else
				index = index + 1
			end
			prev = current
		end
	end

	function menu:populate(initial_selection)
		if not choices then
			populate_choices()
		end

		local items = { }

		if title then
			table.insert(items, { title, '', 'off' })
			table.insert(items, { '---', '', '' })
		end

		index_first_choice = #items + 1
		local selection = index_first_choice
		for index, field in ipairs(choices) do
			if field then
				table.insert(items, { field.name, '', '' })
				if initial_selection and (field.port.tag == initial_selection.port.tag) and (field.mask == initial_selection.mask) and (field.type == initial_selection.type) then
					selection = #items
					initial_selection = nil
				end
			else
				local device = choices[index + 1].device
				if device.owner then
					table.insert(items, { string.format(_p('plugin-commonui', '%s [root%s]'), device.name, device.tag), '', 'heading' })
				else
					table.insert(items, { string.format(_p('plugin-commonui', '[root%s]'), device.tag), '', 'heading' })
				end
			end
		end

		table.insert(items, { '---', '', '' })
		table.insert(items, { _p('plugin-commonui', 'Cancel'), '', '' })
		index_cancel = #items

		return items, selection
	end

	function menu:handle(index, event)
		local selection
		if (event == 'cancel') or ((index == input_item_cancel) and (event == 'select')) then
			action(nil)
			return true
		elseif event == 'select' then
			local field = choices[index - index_first_choice + 1]
			if field then
				action(field)
				return true
			end
		elseif event == 'prevgroup' then
			local found_break = false
			while (index > index_first_choice) and (not selection) do
				index = index - 1
				if not choices[index - index_first_choice + 1] then
					if found_break then
						selection = index + 1
					else
						found_break = true
					end
				end
			end
		elseif event == 'nextgroup' then
			while ((index - index_first_choice + 2) < #choices) and (not selection) do
				index = index + 1
				if not choices[index - index_first_choice + 1] then
					selection = index + 1
				end
			end
		end
		return false, selection
	end

	return menu
end


function commonui.switch_polling_helper(starting_sequence)
	helper = { }

	local machine = manager.machine
	local cancel = machine.ioport:token_to_input_type('UI_CANCEL')
	local cancel_prompt = manager.ui:get_general_input_setting(cancel)
	local input = machine.input
	local uiinput = machine.uiinput
	local poller = input:switch_sequence_poller()
	local modified_ticks = 0

	if starting_sequence then
		poller:start(starting_sequence)
	else
		poller:start()
	end

	function helper:overlay(items, selection, flags)
		if flags then
			flags = flags .. " nokeys"
		else
			flags = "nokeys"
		end
		return items, selection, flags
	end

	function helper:poll()
		-- prevent race condition between uiinput:pressed() and poll()
		if (modified_ticks == 0) and poller.modified then
			modified_ticks = emu.osd_ticks()
		end

		if uiinput:pressed(cancel) then
			-- UI_CANCEL pressed, abort
			machine:popmessage()
			if (not poller.modified) or (modified_ticks == emu.osd_ticks()) then
				-- cancelled immediately
				self.sequence = nil
				return true -- TODO: communicate this better?
			else
				-- entered something before cancelling
				self.sequence = nil
				return true
			end
		elseif poller:poll() then
			if poller.valid then
				-- valid sequence entered
				machine:popmessage()
				self.sequence = poller.sequence
				return true
			else
				-- invalid sequence entered
				machine:popmessage(_p('plugin-commonui', 'Invalid sequence entered'))
				self.sequence = nil
				return true
			end
		else
			machine:popmessage(string.format(
					_p('plugin-commonui', 'Enter sequence or press %s to cancel\n%s'),
					cancel_prompt,
					input:seq_name(poller.sequence)))
			return false
		end
	end

	return helper
end


return exports
