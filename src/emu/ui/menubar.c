/***************************************************************************

    menubar.h

    Internal MAME menu bar for the user interface.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#include "emu.h"
#include "ui/menubar.h"


/*-------------------------------------------------
    ctor
-------------------------------------------------*/

ui_menubar::ui_menubar(running_machine &machine, render_container *container)
	: ui_stackable(machine, container)
{
}


/*-------------------------------------------------
    reset
-------------------------------------------------*/

void ui_menubar::reset()
{
}


/*-------------------------------------------------
    do_handle
-------------------------------------------------*/

void ui_menubar::do_handle()
{
	const char *menubar_text = "File\0Snapshot\0Settings\0Help\0";

	float text_height = get_line_height();
	float spacing = text_height / 10;
	float x = spacing;
	float y = spacing;

	for(const char *s = menubar_text; *s; s += strlen(s) + 1)
	{
		float width = get_string_width(s);

		draw_outlined_box(
			x,
			y,
			x + width + (spacing * 2),
			y + text_height + (spacing * 2),
			UI_RED_COLOR);

		draw_text(s, x + spacing, y + spacing);		

		x += width + spacing * 4;
	}
}

