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
	set_process_flags(PROCESS_CUSTOM_ONLY | PROCESS_NOINPUT);
}


menu_confirm_quit::~menu_confirm_quit()
{
}


void menu_confirm_quit::custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2)
{
	ui().draw_text_box(
			container(),
			util::string_format(
				_("Are you sure you want to quit?\n\n"
				"Press %1$s to quit\n"
				"Press %2$s to return to emulation"),
				ui().get_general_input_setting(IPT_UI_SELECT),
				ui().get_general_input_setting(IPT_UI_CANCEL)),
			text_layout::text_justify::CENTER,
			0.5f, 0.5f,
			UI_RED_COLOR);
}


void menu_confirm_quit::populate(float &customtop, float &custombottom)
{
}


void menu_confirm_quit::handle(event const *ev)
{
	if (machine().ui_input().pressed(IPT_UI_SELECT))
		machine().schedule_exit();
	else if (machine().ui_input().pressed(IPT_UI_CANCEL))
		stack_pop();
}

} // namespace ui
