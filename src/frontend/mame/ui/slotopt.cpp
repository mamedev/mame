// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles, Nathan Woods
/*********************************************************************

    ui/slotopt.cpp

    Internal menu for the slot options.

*********************************************************************/

#include "emu.h"

#include "ui/ui.h"
#include "ui/slotopt.h"
#include "ui/devopt.h"

#include "emuopts.h"
#include "mameopts.h"


/***************************************************************************
    UTILITY
***************************************************************************/

namespace {

// constants
void *ITEMREF_RESET = ((void *)1);
char DIVIDER[] = "------";


//-------------------------------------------------
//  get_slot_length
//-------------------------------------------------

int get_slot_length(const device_slot_interface &slot)
{
	int val = 0;
	for (auto &option : slot.option_list())
		if (option.second->selectable())
			val++;

	return val;
}


//-------------------------------------------------
//  get_slot_option
//-------------------------------------------------

const char *get_slot_option(device_slot_interface &slot, int index)
{
	if (index >= 0)
	{
		int val = 0;
		for (auto &option : slot.option_list())
		{
			if (val == index)
				return option.second->name();

			if (option.second->selectable())
				val++;
		}
	}

	return "";
}


};
/***************************************************************************
    SLOT MENU
***************************************************************************/

namespace ui {

//-------------------------------------------------
//  menu_slot_devices constructor
//-------------------------------------------------

menu_slot_devices::menu_slot_devices(mame_ui_manager &mui, render_container &container) : menu(mui, container)
{
}

//-------------------------------------------------
//  menu_slot_devices destructor
//-------------------------------------------------

menu_slot_devices::~menu_slot_devices()
{
}


//-------------------------------------------------
//  get_current_option - returns the current
//  slot option
//-------------------------------------------------

device_slot_option *menu_slot_devices::get_current_option(device_slot_interface &slot) const
{
	std::string current;

	if (!slot.fixed())
	{
		const char *slot_option_name = slot.slot_name();
		current = machine().options().slot_option(slot_option_name).value();
	}
	else
	{
		if (slot.default_option() == nullptr)
			return nullptr;
		current.assign(slot.default_option());
	}

	return slot.option(current.c_str());
}


//-------------------------------------------------
//  get_current_index
//-------------------------------------------------

int menu_slot_devices::get_current_index(device_slot_interface &slot) const
{
	const device_slot_option *current = get_current_option(slot);

	if (current != nullptr)
	{
		int val = 0;
		for (auto &option : slot.option_list())
		{
			if (option.second.get() == current)
				return val;

			if (option.second->selectable())
				val++;
		}
	}

	return -1;
}


//-------------------------------------------------
//  get_next_slot
//-------------------------------------------------

const char *menu_slot_devices::get_next_slot(device_slot_interface &slot) const
{
	int idx = get_current_index(slot);
	if (idx < 0)
		idx = 0;
	else
		idx++;

	if (idx >= get_slot_length(slot))
		return "";

	return get_slot_option(slot, idx);
}


//-------------------------------------------------
//  get_previous_slot
//-------------------------------------------------

const char *menu_slot_devices::get_previous_slot(device_slot_interface &slot) const
{
	int idx = get_current_index(slot);
	if (idx < 0)
		idx = get_slot_length(slot) - 1;
	else
		idx--;

	if (idx < 0)
		return "";

	return get_slot_option(slot, idx);
}


//-------------------------------------------------
//  set_slot_device
//-------------------------------------------------

void menu_slot_devices::set_slot_device(device_slot_interface &slot, const char *val)
{
	// we might change slot options; in the spirit of user friendliness, we should record all current
	// options
	record_current_options();

	// find the slot option
	slot_option &opt(machine().options().slot_option(slot.slot_name()));

	// specify it
	opt.specify(val);

	// erase this from our recorded options list - this is the slot we're trying to change!
	m_slot_options.erase(slot.slot_name());

	// refresh any options that we might have annotated earlier
	while (try_refresh_current_options())
		;

	// changing the options may result in options changing; we need to reset
	reset(reset_options::REMEMBER_POSITION);
}


//-------------------------------------------------
//  record_current_options
//-------------------------------------------------

void menu_slot_devices::record_current_options()
{
	for (device_slot_interface &slot : slot_interface_iterator(m_config->root_device()))
	{
		// we're doing this out of a desire to honor user-selectable options; therefore it only
		// makes sense to record values for selectable options
		if (slot.has_selectable_options())
		{
			// get the slot option
			const slot_option &opt(machine().options().slot_option(slot.slot_name()));

			// and record the value in our local cache
			m_slot_options[slot.slot_name()] = opt.specified_value();
		}
	}
}


//-------------------------------------------------
//  try_refresh_current_options
//-------------------------------------------------

bool menu_slot_devices::try_refresh_current_options()
{
	// enumerate through all slot options that we've tracked
	for (const auto &opt : m_slot_options)
	{
		// do we have a value different than what we're tracking?
		slot_option *slotopt = machine().options().find_slot_option(opt.first);
		if (slotopt && slotopt->specified_value() != opt.second)
		{
			// specify this option (but catch errors)
			try
			{
				slotopt->specify(opt.second);

				// the option was successfully specified; it isn't safe to continue
				// checking slots as the act of specifying the slot may have drastically
				// changed the options list
				return true;
			}
			catch (options_exception &)
			{
				// this threw an exception - that is fine; we can just proceed
			}
		}
	}

	// we've went through all options without changing anything
	return false;
}


//-------------------------------------------------
//  populate
//-------------------------------------------------

void menu_slot_devices::populate(float &customtop, float &custombottom)
{
	// we need to keep our own copy of the machine_config because we
	// can change this out from under the caller
	m_config = std::make_unique<machine_config>(machine().system(), machine().options());

	// cycle through all devices for this system
	for (device_slot_interface &slot : slot_interface_iterator(m_config->root_device()))
	{
		// does this slot have any selectable options?
		bool has_selectable_options = slot.has_selectable_options();

		// name this option
		std::string opt_name(DIVIDER);
		const device_slot_option *option = get_current_option(slot);
		if (option)
		{
			opt_name = has_selectable_options
				? option->name()
				: string_format(_("%s [internal]"), option->name());
		}

		// choose item flags
		uint32_t item_flags = has_selectable_options
			? FLAG_LEFT_ARROW | FLAG_RIGHT_ARROW
			: FLAG_DISABLE;

		item_append(slot.slot_name(), opt_name, item_flags, (void *)&slot);
	}
	item_append(menu_item_type::SEPARATOR);
	item_append(_("Reset"), "", 0, ITEMREF_RESET);
}


//-------------------------------------------------
//  handle
//-------------------------------------------------

void menu_slot_devices::handle()
{
	// process the menu
	const event *menu_event = process(0);

	if (menu_event != nullptr && menu_event->itemref != nullptr)
	{
		if (menu_event->itemref == ITEMREF_RESET && menu_event->iptkey == IPT_UI_SELECT)
		{
			machine().schedule_hard_reset();
		}
		else if (menu_event->iptkey == IPT_UI_LEFT || menu_event->iptkey == IPT_UI_RIGHT)
		{
			device_slot_interface *slot = (device_slot_interface *)menu_event->itemref;
			const char *val = (menu_event->iptkey == IPT_UI_LEFT) ? get_previous_slot(*slot) : get_next_slot(*slot);
			set_slot_device(*slot, val);
		}
		else if (menu_event->iptkey == IPT_UI_SELECT)
		{
			device_slot_interface *slot = (device_slot_interface *)menu_event->itemref;
			device_slot_option *option = get_current_option(*slot);
			if (option)
				menu::stack_push<menu_device_config>(ui(), container(), slot, option);
		}
	}
}

} // namespace ui
