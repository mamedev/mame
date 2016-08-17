// license:BSD-3-Clause
// copyright-holders:F.Ulivi
/***************************************************************************

    ui/info_pty.cpp

    Information screen on pseudo terminals

***************************************************************************/

#include "emu.h"

#include "ui/info_pty.h"


namespace ui {
menu_pty_info::menu_pty_info(mame_ui_manager &mui, render_container &container) :
	menu(mui, container)
{
}

menu_pty_info::~menu_pty_info()
{
}

void menu_pty_info::populate()
{
	item_append(_("Pseudo terminals"), "", FLAG_DISABLE, nullptr);
	item_append("", "", FLAG_DISABLE, nullptr);

	for (device_pty_interface &pty : pty_interface_iterator(machine().root_device()))
	{
		const char *port_name = pty.device().owner()->tag() + 1;
		if (pty.is_open())
			item_append(port_name, pty.slave_name(), FLAG_DISABLE, nullptr);
		else
			item_append(port_name, _("[failed]"), FLAG_DISABLE, nullptr);
		item_append("", "", FLAG_DISABLE, nullptr);
	}
}

void menu_pty_info::handle()
{
	process(0);
}

} // namespace ui
