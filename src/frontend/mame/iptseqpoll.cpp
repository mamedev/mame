// license:BSD-3-Clause
// copyright-holders:Vas Crabb, Aaron Giles

#include "emu.h"
#include "iptseqpoll.h"

#include "inputdev.h"

#include <cassert>
#include <algorithm>


input_code_poller::input_code_poller(input_manager &manager) noexcept :
	m_manager(manager),
	m_axis_memory(),
	m_switch_memory()
{
}


input_code_poller::~input_code_poller()
{
}


void input_code_poller::reset()
{
	// iterate over device classes and devices
	m_axis_memory.clear();
	m_switch_memory.clear();
	for (input_device_class classno = DEVICE_CLASS_FIRST_VALID; DEVICE_CLASS_LAST_VALID >= classno; ++classno)
	{
		input_class &devclass(m_manager.device_class(classno));
		if (devclass.enabled())
		{
			for (int devnum = 0; devclass.maxindex() >= devnum; ++devnum)
			{
				// fetch the device; ignore if nullptr
				input_device *const device(devclass.device(devnum));
				if (device)
				{
					// iterate over items within each device
					for (input_item_id itemid = ITEM_ID_FIRST_VALID; device->maxitem() >= itemid; ++itemid)
					{
						// for any non-switch items, set memory to the current value
						input_device_item *const item(device->item(itemid));
						if (item && (item->itemclass() != ITEM_CLASS_SWITCH))
							m_axis_memory.emplace_back(item, m_manager.code_value(item->code()));
					}
				}
			}
		}
	}
	std::sort(m_axis_memory.begin(), m_axis_memory.end());
}


bool input_code_poller::code_pressed_once(input_code code, bool moved)
{
	// look for the code in the memory
	bool const pressed(m_manager.code_pressed(code));
	auto const found(std::lower_bound(m_switch_memory.begin(), m_switch_memory.end(), code));
	if ((m_switch_memory.end() != found) && (*found == code))
	{
		// if no longer pressed, clear entry
		if (!pressed)
			m_switch_memory.erase(found);

		// always return false
		return false;
	}

	// if we get here, we were not previously pressed; if still not pressed, return false
	if (!pressed || !moved)
		return false;

	// otherwise, add the code to the memory and return true
	m_switch_memory.emplace(found, code);
	return true;
}



axis_code_poller::axis_code_poller(input_manager &manager) noexcept :
	input_code_poller(manager),
	m_axis_active()
{
}


void axis_code_poller::reset()
{
	input_code_poller::reset();
	m_axis_active.clear();
	m_axis_active.resize(m_axis_memory.size(), false);
}


input_code axis_code_poller::poll()
{
	// iterate over the axis items we found
	for (std::size_t i = 0; m_axis_memory.size() > i; ++i)
	{
		auto &memory = m_axis_memory[i];
		input_code code = memory.first->code();
		if (!memory.first->check_axis(code.item_modifier(), memory.second))
		{
			m_axis_active[i] = false;
		}
		else if (!m_axis_active[i])
		{
			if (code.item_class() == ITEM_CLASS_ABSOLUTE)
			{
				m_axis_active[i] = true;
			}
			else
			{
				// can only cycle modifiers on a relative item with append
				m_axis_memory.erase(m_axis_memory.begin() + i);
				m_axis_active.erase(m_axis_active.begin() + i);
			}
			if (!m_manager.device_class(memory.first->device().devclass()).multi())
				code.set_device_index(0);
			return code;
		}
	}

	// iterate over device classes and devices, skipping disabled classes
	for (input_device_class classno = DEVICE_CLASS_FIRST_VALID; DEVICE_CLASS_LAST_VALID >= classno; ++classno)
	{
		input_class &devclass(m_manager.device_class(classno));
		if (!devclass.enabled())
			continue;

		for (int devnum = 0; devclass.maxindex() >= devnum; ++devnum)
		{
			// fetch the device; ignore if nullptr
			input_device *const device(devclass.device(devnum));
			if (!device)
				continue;

			// iterate over items within each device
			for (input_item_id itemid = ITEM_ID_FIRST_VALID; device->maxitem() >= itemid; ++itemid)
			{
				input_device_item *const item(device->item(itemid));
				if (!item)
					continue;

				input_code code = item->code();
				if (item->itemclass() == ITEM_CLASS_SWITCH)
				{
					// item is natively a switch, poll it
					if (code_pressed_once(code, true))
						return code;
					else
						continue;
				}
			}
		}
	}

	// if nothing, return an invalid code
	return INPUT_CODE_INVALID;
}



switch_code_poller::switch_code_poller(input_manager &manager) noexcept :
	input_code_poller(manager)
{
}


input_code switch_code_poller::poll()
{
	// iterate over device classes and devices, skipping disabled classes
	for (input_device_class classno = DEVICE_CLASS_FIRST_VALID; DEVICE_CLASS_LAST_VALID >= classno; ++classno)
	{
		input_class &devclass(m_manager.device_class(classno));
		if (!devclass.enabled())
			continue;

		for (int devnum = 0; devclass.maxindex() >= devnum; ++devnum)
		{
			// fetch the device; ignore if nullptr
			input_device *const device(devclass.device(devnum));
			if (!device)
				continue;

			// iterate over items within each device
			for (input_item_id itemid = ITEM_ID_FIRST_VALID; device->maxitem() >= itemid; ++itemid)
			{
				input_device_item *const item(device->item(itemid));
				if (!item)
					continue;

				input_code code = item->code();
				if (item->itemclass() == ITEM_CLASS_SWITCH)
				{
					// item is natively a switch, poll it
					if (code_pressed_once(code, true))
						return code;
					else
						continue;
				}

				auto const memory(std::lower_bound(
							m_axis_memory.begin(),
							m_axis_memory.end(),
							item,
							[] (auto const &x, auto const &y) { return x.first < y; }));
				if ((m_axis_memory.end() == memory) || (item != memory->first))
					continue;

				// poll axes digitally
				bool const moved(item->check_axis(code.item_modifier(), memory->second));
				code.set_item_class(ITEM_CLASS_SWITCH);
				if ((classno == DEVICE_CLASS_JOYSTICK) && (code.item_id() == ITEM_ID_XAXIS))
				{
					// joystick X axis - check with left/right modifiers
					code.set_item_modifier(ITEM_MODIFIER_LEFT);
					if (code_pressed_once(code, moved))
						return code;
					code.set_item_modifier(ITEM_MODIFIER_RIGHT);
					if (code_pressed_once(code, moved))
						return code;
				}
				else if ((classno == DEVICE_CLASS_JOYSTICK) && (code.item_id() == ITEM_ID_YAXIS))
				{
					// if this is a joystick Y axis, check with up/down modifiers
					code.set_item_modifier(ITEM_MODIFIER_UP);
					if (code_pressed_once(code, moved))
						return code;
					code.set_item_modifier(ITEM_MODIFIER_DOWN);
					if (code_pressed_once(code, moved))
						return code;
				}
				else
				{
					// any other axis, check with pos/neg modifiers
					code.set_item_modifier(ITEM_MODIFIER_POS);
					if (code_pressed_once(code, moved))
						return code;
					code.set_item_modifier(ITEM_MODIFIER_NEG);
					if (code_pressed_once(code, moved))
						return code;
				}
			}
		}
	}

	// if nothing, return an invalid code
	return INPUT_CODE_INVALID;
}



keyboard_code_poller::keyboard_code_poller(input_manager &manager) noexcept :
	input_code_poller(manager)
{
}


input_code keyboard_code_poller::poll()
{
	// iterate over devices in keyboard class
	input_class &devclass = m_manager.device_class(DEVICE_CLASS_KEYBOARD);
	for (int devnum = 0; devclass.maxindex() >= devnum; ++devnum)
	{
		// fetch the device; ignore if nullptr
		input_device *const device(devclass.device(devnum));
		if (device)
		{
			// iterate over items within each device
			for (input_item_id itemid = ITEM_ID_FIRST_VALID; itemid <= device->maxitem(); ++itemid)
			{
				// iterate over items within each device
				for (input_item_id itemid = ITEM_ID_FIRST_VALID; device->maxitem() >= itemid; ++itemid)
				{
					input_device_item *const item = device->item(itemid);
					if (item && (item->itemclass() == ITEM_CLASS_SWITCH))
					{
						input_code const code = item->code();
						if (code_pressed_once(code, true))
							return code;
					}
				}
			}
		}
	}

	// if nothing, return an invalid code
	return INPUT_CODE_INVALID;
}



input_sequence_poller::input_sequence_poller() noexcept :
	m_sequence(),
	m_last_ticks(0),
	m_modified(false)
{
}


input_sequence_poller::~input_sequence_poller()
{
}


void input_sequence_poller::start()
{
	// start with an empty sequence
	m_sequence.reset();

	// reset the recording count and the clock
	m_last_ticks = 0;
	m_modified = false;
	do_start();
}


void input_sequence_poller::start(input_seq const &startseq)
{
	// grab the starting sequence to append to, and append an OR if it isn't empty
	m_sequence = startseq;
	if (input_seq::end_code != m_sequence[0])
		m_sequence += input_seq::or_code;

	// reset the recording count and the clock
	m_last_ticks = 0;
	m_modified = false;
	do_start();
}


bool input_sequence_poller::poll()
{
	// if we got a new code to append it, append it and reset the timer
	input_code const newcode = do_poll();
	osd_ticks_t const newticks = osd_ticks();
	if (INPUT_CODE_INVALID != newcode)
	{
		m_sequence += newcode;
		m_last_ticks = newticks;
		m_modified = true;
	}

	// if we've recorded at least one item and one second has passed, we're done
	if (m_last_ticks && ((m_last_ticks + osd_ticks_per_second()) < newticks))
		return true;

	// return false to indicate we are still polling
	return false;
}



axis_sequence_poller::axis_sequence_poller(input_manager &manager) noexcept :
	input_sequence_poller(),
	m_code_poller(manager)
{
}


void axis_sequence_poller::do_start()
{
	// wait for any inputs that are already active to be released
	m_code_poller.reset();
	for (input_code dummycode = KEYCODE_ENTER; INPUT_CODE_INVALID != dummycode; )
		dummycode = m_code_poller.poll();
}


input_code axis_sequence_poller::do_poll()
{
	// absolute/relative case: see if we have an analog change of sufficient amount
	int const curlen = m_sequence.length();
	input_code lastcode = m_sequence[curlen - 1];
	bool const has_or = input_seq::or_code == lastcode;
	if (has_or)
		lastcode = m_sequence[curlen - 2];
	input_code newcode = m_code_poller.poll();

	// if not empty, see if it's the same control again to cycle modifiers
	if ((INPUT_CODE_INVALID != newcode) && curlen)
	{
		input_item_class const newclass = newcode.item_class();
		input_code last_nomodifier = lastcode;
		last_nomodifier.set_item_modifier(ITEM_MODIFIER_NONE);
		if (newcode == last_nomodifier)
		{
			if (ITEM_CLASS_ABSOLUTE == newclass)
			{
				// increment the modifier, wrapping back to none
				switch (lastcode.item_modifier())
				{
				case ITEM_MODIFIER_NONE:
					newcode.set_item_modifier(ITEM_MODIFIER_POS);
					break;
				case ITEM_MODIFIER_POS:
					newcode.set_item_modifier(ITEM_MODIFIER_NEG);
					break;
				case ITEM_MODIFIER_NEG:
					newcode.set_item_modifier(ITEM_MODIFIER_REVERSE);
					break;
				default:
				case ITEM_MODIFIER_REVERSE:
					newcode.set_item_modifier(ITEM_MODIFIER_NONE);
					break;
				}

				// back up over the previous code so we can re-append
				if (has_or)
					m_sequence.backspace();
				m_sequence.backspace();
			}
			else if (ITEM_CLASS_RELATIVE == newclass)
			{
				// increment the modifier, wrapping back to none
				switch (lastcode.item_modifier())
				{
				case ITEM_MODIFIER_NONE:
					newcode.set_item_modifier(ITEM_MODIFIER_REVERSE);
					break;
				default:
				case ITEM_MODIFIER_REVERSE:
					newcode.set_item_modifier(ITEM_MODIFIER_NONE);
					break;
				}

				// back up over the previous code so we can re-append
				if (has_or)
					m_sequence.backspace();
				m_sequence.backspace();
			}
			else if (!has_or && (ITEM_CLASS_SWITCH == newclass))
			{
				// back up over the existing code
				m_sequence.backspace();

				// if there was a NOT preceding it, delete it as well, otherwise append a fresh one
				if (m_sequence[curlen - 2] == input_seq::not_code)
					m_sequence.backspace();
				else
					m_sequence += input_seq::not_code;
			}
		}
		else if (!has_or && (ITEM_CLASS_SWITCH == newclass))
		{
			// ignore switches following axes
			if (ITEM_CLASS_SWITCH != lastcode.item_class())
			{
				// hack to stop it timing out so user can cancel
				m_sequence.backspace();
				newcode = lastcode;
			}
		}
	}

	// hack to stop it timing out before assigning an axis
	if (ITEM_CLASS_SWITCH == newcode.item_class())
	{
		m_sequence += newcode;
		set_modified();
		return INPUT_CODE_INVALID;
	}
	else
	{
		return newcode;
	}
}



switch_sequence_poller::switch_sequence_poller(input_manager &manager) noexcept :
	input_sequence_poller(),
	m_code_poller(manager)
{
}


void switch_sequence_poller::do_start()
{
	// wait for any inputs that are already active to be released
	m_code_poller.reset();
	for (input_code dummycode = KEYCODE_ENTER; INPUT_CODE_INVALID != dummycode; )
		dummycode = m_code_poller.poll();
}


input_code switch_sequence_poller::do_poll()
{
	// switch case: see if we have a new code to process
	int const curlen = m_sequence.length();
	input_code lastcode = m_sequence[curlen - 1];
	input_code newcode = m_code_poller.poll();
	if (INPUT_CODE_INVALID != newcode)
	{
		// if code is duplicate, toggle the NOT state on the code
		if (curlen && (newcode == lastcode))
		{
			// back up over the existing code
			m_sequence.backspace();

			// if there was a NOT preceding it, delete it as well, otherwise append a fresh one
			if (m_sequence[curlen - 2] == input_seq::not_code)
				m_sequence.backspace();
			else
				m_sequence += input_seq::not_code;
		}
	}
	return newcode;
}
