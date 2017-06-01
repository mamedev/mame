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

	const char *slot_option_name = slot.slot_name();
	if (!slot.fixed() && machine().options().slot_options().count(slot_option_name) > 0)
	{
		current = machine().options().slot_options()[slot_option_name].value();
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
	std::string error;
	machine().options().set_value(slot.slot_name(), val, OPTION_PRIORITY_CMDLINE, error);
	assert(error.empty());
}


//-------------------------------------------------
//  populate
//-------------------------------------------------

void menu_slot_devices::populate(float &customtop, float &custombottom)
{
	// cycle through all devices for this system
	for (device_slot_interface &slot : slot_interface_iterator(machine().root_device()))
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
			mame_options::add_slot_options(machine().options());
			machine().schedule_hard_reset();
		}
		else if (menu_event->iptkey == IPT_UI_LEFT || menu_event->iptkey == IPT_UI_RIGHT)
		{
			device_slot_interface *slot = (device_slot_interface *)menu_event->itemref;
			const char *val = (menu_event->iptkey == IPT_UI_LEFT) ? get_previous_slot(*slot) : get_next_slot(*slot);
			set_slot_device(*slot, val);
			reset(reset_options::REMEMBER_REF);
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
