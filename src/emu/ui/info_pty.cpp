// license:BSD-3-Clause
// copyright-holders:F.Ulivi
/***************************************************************************

    ui/info_pty.c

    Information screen on pseudo terminals

***************************************************************************/

#include "emu.h"
#include "ui/menu.h"
#include "ui/info_pty.h"

ui_menu_pty_info::ui_menu_pty_info(running_machine &machine, render_container *container) :
	ui_menu(machine, container)
{
}

ui_menu_pty_info::~ui_menu_pty_info()
{
}

void ui_menu_pty_info::populate()
{
	item_append("Pseudo terminals", nullptr, MENU_FLAG_DISABLE, nullptr);
	item_append("", nullptr, MENU_FLAG_DISABLE, nullptr);

	pty_interface_iterator iter(machine().root_device());
	for (device_pty_interface *pty = iter.first(); pty != nullptr; pty = iter.next()) {
		const char *port_name = pty->device().owner()->tag().c_str() + 1;
		if (pty->is_open()) {
			item_append(port_name , pty->slave_name() , MENU_FLAG_DISABLE , nullptr);
		} else {
			item_append(port_name , "[failed]" , MENU_FLAG_DISABLE , nullptr);
		}
		item_append("", nullptr, MENU_FLAG_DISABLE, nullptr);
	}
}

void ui_menu_pty_info::handle()
{
	process(0);
}
