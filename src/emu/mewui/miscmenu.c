// license:BSD-3-Clause
// copyright-holders:Dankan1890
/***************************************************************************

    mewui/miscmenu.c

    MEWUI miscellaneous options menu.

***************************************************************************/

#include "emu.h"
#include "ui/ui.h"
#include "mewui/miscmenu.h"
#include "mewui/utils.h"

misc_option ui_menu_misc_options::m_options[] = {
	{ 0, NULL, NULL },
	{ 0, "Re-select last game / system played",             OPTION_REMEMBER_LAST },
	{ 0, "Enlarge images in the right panel",               OPTION_ENLARGE_SNAPS },
	{ 0, "DATs info",                                       OPTION_DATS_ENABLED },
	{ 0, "Cheats",                                          OPTION_CHEAT },
	{ 0, "Show mouse pointer",                              OPTION_UI_MOUSE },
	{ 0, "Confirm quit from machines",                      OPTION_CONFIRM_QUIT },
	{ 0, "Skip displaying information's screen at startup", OPTION_SKIP_GAMEINFO },
	{ 0, "Force 4:3 appearance for software snapshot",      OPTION_FORCED4X3 },
	{ 0, "Use image as background",                         OPTION_USE_BACKGROUND },
	{ 0, "Skip bios selection menu",                        OPTION_SKIP_BIOS_MENU },
	{ 0, "Skip software parts selection menu",              OPTION_SKIP_PARTS_MENU }
};

//-------------------------------------------------
//  ctor / dtor
//-------------------------------------------------

ui_menu_misc_options::ui_menu_misc_options(running_machine &machine, render_container *container) : ui_menu(machine, container)
{
	for (int d = 1; d < ARRAY_LENGTH(m_options); ++d)
		m_options[d].status = machine.options().bool_value(m_options[d].option);
}

ui_menu_misc_options::~ui_menu_misc_options()
{
	std::string error_string;
	for (int d = 1; d < ARRAY_LENGTH(m_options); ++d)
		machine().options().set_value(m_options[d].option, m_options[d].status, OPTION_PRIORITY_CMDLINE, error_string);
	mewui_globals::force_reset_main = true;
}

//-------------------------------------------------
//  handlethe options menu
//-------------------------------------------------

void ui_menu_misc_options::handle()
{
	bool changed = false;

	// process the menu
	const ui_menu_event *menu_event = process(0);
	if (menu_event != NULL && menu_event->itemref != NULL)
	{
		if (menu_event->iptkey == IPT_UI_LEFT || menu_event->iptkey == IPT_UI_RIGHT || menu_event->iptkey == IPT_UI_SELECT)
		{
			changed = true;
			int value = (FPTR)menu_event->itemref;
			if (!strcmp(m_options[value].option, OPTION_ENLARGE_SNAPS))
				mewui_globals::switch_image = true;
			m_options[value].status = !m_options[value].status;
		}
	}

	if (changed)
		reset(UI_MENU_RESET_REMEMBER_REF);
}

//-------------------------------------------------
//  populate
//-------------------------------------------------

void ui_menu_misc_options::populate()
{
	// add options items
	for (int opt = 1; opt < ARRAY_LENGTH(m_options); ++opt)
		item_append(m_options[opt].description, m_options[opt].status ? "On" : "Off", m_options[opt].status ? MENU_FLAG_RIGHT_ARROW : MENU_FLAG_LEFT_ARROW, (void *)(FPTR)opt);

	item_append(MENU_SEPARATOR_ITEM, NULL, 0, NULL);
	customtop = machine().ui().get_line_height() + (3.0f * UI_BOX_TB_BORDER);
}

//-------------------------------------------------
//  perform our special rendering
//-------------------------------------------------

void ui_menu_misc_options::custom_render(void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
	float width;

	machine().ui().draw_text_full(container, "Miscellaneous Options", 0.0f, 0.0f, 1.0f, JUSTIFY_CENTER, WRAP_TRUNCATE,
	                              DRAW_NONE, ARGB_WHITE, ARGB_BLACK, &width, NULL);
	width += 2 * UI_BOX_LR_BORDER;
	float maxwidth = MAX(origx2 - origx1, width);

	// compute our bounds
	float x1 = 0.5f - 0.5f * maxwidth;
	float x2 = x1 + maxwidth;
	float y1 = origy1 - top;
	float y2 = origy1 - UI_BOX_TB_BORDER;

	// draw a box
	machine().ui().draw_outlined_box(container, x1, y1, x2, y2, UI_GREEN_COLOR);

	// take off the borders
	x1 += UI_BOX_LR_BORDER;
	x2 -= UI_BOX_LR_BORDER;
	y1 += UI_BOX_TB_BORDER;

	// draw the text within it
	machine().ui().draw_text_full(container, "Miscellaneous Options", x1, y1, x2 - x1, JUSTIFY_CENTER, WRAP_TRUNCATE,
	                              DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, NULL, NULL);
}
