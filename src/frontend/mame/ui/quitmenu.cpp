// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

    ui/quitmenu.h

    Menus involved in quitting MAME.

***************************************************************************/

#include "emu.h"
#include "quitmenu.h"

#include "uiinput.h"


namespace ui {

menu_confirm_quit::menu_confirm_quit(mame_ui_manager &mui, render_container &container)
	: autopause_menu<>(mui, container)
{
	set_one_shot(true);
	set_needs_prev_menu_item(false);
	set_heading(_("menu-quit", "Are you sure you want to quit?"));
}


menu_confirm_quit::~menu_confirm_quit()
{
}


void menu_confirm_quit::populate()
{
	item_append(_("menu-quit", "Quit"), 0, nullptr);
	item_append(_("menu-quit", "Return to emulation"), 0, nullptr);
}


bool menu_confirm_quit::handle(event const *ev)
{
	if (ev && (IPT_UI_SELECT == ev->iptkey))
	{
		if (0 == selected_index())
			machine().schedule_exit();
		else
			stack_pop();
	}

	return false;
}

} // namespace ui
