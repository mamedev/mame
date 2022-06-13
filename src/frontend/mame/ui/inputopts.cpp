// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

    ui/inputopts.cpp

    Input options submenu.

***************************************************************************/

#include "emu.h"
#include "ui/inputopts.h"

#include "ui/analogipt.h"
#include "ui/inputdevices.h"
#include "ui/inputmap.h"
#include "ui/inputtoggle.h"
#include "ui/keyboard.h"

#include "natkeyboard.h"


namespace ui {

namespace {

enum : unsigned
{
	INPUTMAP_GENERAL = 1,
	INPUTMAP_MACHINE,
	ANALOG,
	KEYBOARD,
	TOGGLES,
	INPUTDEV
};


void scan_inputs(running_machine &machine, bool &inputmap, bool &analog, bool &toggle)
{
	inputmap = analog = toggle = false;
	for (auto &port : machine.ioport().ports())
	{
		for (ioport_field &field : port.second->fields())
		{
			if (field.enabled())
			{
				switch (field.type_class())
				{
				case INPUT_CLASS_CONTROLLER:
				case INPUT_CLASS_MISC:
				case INPUT_CLASS_KEYBOARD:
					inputmap = true;
					if (field.live().toggle)
						toggle = true;
					break;
				default:
					break;
				}
				if (field.is_analog())
					analog = true;

				if (inputmap && analog && toggle)
					return;
			}
		}
	}
}

} // anonymous namespace


menu_input_options::menu_input_options(mame_ui_manager &mui, render_container &container)
	: menu(mui, container)
{
	set_heading(_("menu-inputopts", "Input Settings"));
}


menu_input_options::~menu_input_options()
{
}


void menu_input_options::menu_activated()
{
	reset(reset_options::REMEMBER_REF);
}


void menu_input_options::populate(float &customtop, float &custombottom)
{
	bool inputmap, analog, toggle;
	scan_inputs(machine(), inputmap, analog, toggle);

	// system-specific stuff
	if (inputmap)
		item_append(_("menu-inputopts", "Input Assignments (this system)"), 0, (void *)INPUTMAP_MACHINE);
	if (analog)
		item_append(_("menu-inputopts", "Analog Input Adjustments"), 0, (void *)ANALOG);
	if (machine().natkeyboard().keyboard_count())
		item_append(_("menu-inputopts", "Keyboard Selection"), 0, (void *)KEYBOARD);
	if (toggle)
		item_append(_("menu-inputopts", "Toggle Inputs"), 0, (void *)TOGGLES);
	if (inputmap || analog || machine().natkeyboard().keyboard_count() || toggle)
		item_append(menu_item_type::SEPARATOR);

	// general stuff
	item_append(_("menu-inputopts", "Input Assignments (general)"), 0, (void *)INPUTMAP_GENERAL);
	item_append(_("menu-inputopts", "Input Devices"), 0, (void *)INPUTDEV);
	item_append(menu_item_type::SEPARATOR);
}


void menu_input_options::handle(event const *ev)
{
	if (ev && (IPT_UI_SELECT == ev->iptkey))
	{
		switch (uintptr_t(ev->itemref))
		{
		case INPUTMAP_GENERAL:
			stack_push<menu_input_groups>(ui(), container());
			break;
		case INPUTMAP_MACHINE:
			stack_push<menu_input_specific>(ui(), container());
			break;
		case ANALOG:
			stack_push<menu_analog>(ui(), container());
			break;
		case KEYBOARD:
			stack_push<menu_keyboard_mode>(ui(), container());
			break;
		case TOGGLES:
			stack_push<menu_input_toggles>(ui(), container());
			break;
		case INPUTDEV:
			stack_push<menu_input_devices>(ui(), container());
			break;
		}
	}
}

} // namespace ui
