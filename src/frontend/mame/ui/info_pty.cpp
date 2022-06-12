// license:BSD-3-Clause
// copyright-holders:F.Ulivi
/***************************************************************************

    ui/info_pty.cpp

    Information screen on pseudo terminals

***************************************************************************/

#include "emu.h"
#include "ui/info_pty.h"

#include "dipty.h"


namespace ui {

menu_pty_info::menu_pty_info(mame_ui_manager &mui, render_container &container) :
	menu(mui, container)
{
	set_heading(_("Pseudo Terminals"));
}

menu_pty_info::~menu_pty_info()
{
}

void menu_pty_info::populate(float &customtop, float &custombottom)
{
	for (device_pty_interface &pty : pty_interface_enumerator(machine().root_device()))
	{
		const char *port_name = pty.device().owner()->tag() + 1;
		if (pty.is_open())
			item_append(port_name, pty.slave_name(), FLAG_DISABLE, nullptr);
		else
			item_append(port_name, _("[failed]"), FLAG_DISABLE, nullptr);
		item_append(std::string(), FLAG_DISABLE, nullptr);
	}
}

void menu_pty_info::handle(event const *ev)
{
}

} // namespace ui
