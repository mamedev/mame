/***************************************************************************

    mewui/miscmenu.c

    Internal MEWUI user interface.

***************************************************************************/

#include "emu.h"
#include "ui/ui.h"
#include "mewui/miscmenu.h"
#include "mewui/utils.h"

/**************************************************
    MENU MISCELLANEOUS OPTIONS
**************************************************/
//-------------------------------------------------
//  ctor / dtor
//-------------------------------------------------

ui_menu_misc_options::ui_menu_misc_options(running_machine &machine, render_container *container) : ui_menu(machine, container)
{
	m_options[REMEMBER_LAST_GAME] = machine.options().remember_last();
	m_options[ENLARGE_ARTS] = machine.options().enlarge_snaps();
	m_options[DATS_ENABLED] = machine.options().enabled_dats();
	m_options[CHEAT_ENABLED] = machine.options().cheat();
	m_options[MOUSE_ENABLED] = machine.options().ui_mouse();
	m_options[CONFIRM_QUIT_ENABLED] = machine.options().confirm_quit();
	m_options[SKIP_GAMEINFO_ENABLED] = machine.options().skip_gameinfo();
	m_options[FORCED_4X3] = machine.options().forced_4x3_snapshot();
	m_options[USE_BGRND] = machine.options().use_background_image();
}

ui_menu_misc_options::~ui_menu_misc_options()
{
	std::string error_string;
	machine().options().set_value(OPTION_REMEMBER_LAST, m_options[REMEMBER_LAST_GAME], OPTION_PRIORITY_CMDLINE, error_string);
	machine().options().set_value(OPTION_ENLARGE_SNAPS, m_options[ENLARGE_ARTS], OPTION_PRIORITY_CMDLINE, error_string);
	machine().options().set_value(OPTION_DATS_ENABLED, m_options[DATS_ENABLED], OPTION_PRIORITY_CMDLINE, error_string);
	machine().options().set_value(OPTION_CHEAT, m_options[CHEAT_ENABLED], OPTION_PRIORITY_CMDLINE, error_string);
	machine().options().set_value(OPTION_UI_MOUSE, m_options[MOUSE_ENABLED], OPTION_PRIORITY_CMDLINE, error_string);
	machine().options().set_value(OPTION_CONFIRM_QUIT, m_options[CONFIRM_QUIT_ENABLED], OPTION_PRIORITY_CMDLINE, error_string);
	machine().options().set_value(OPTION_SKIP_GAMEINFO, m_options[SKIP_GAMEINFO_ENABLED], OPTION_PRIORITY_CMDLINE, error_string);
	machine().options().set_value(OPTION_FORCED4X3, m_options[FORCED_4X3], OPTION_PRIORITY_CMDLINE, error_string);
	machine().options().set_value(OPTION_USE_BACKGROUND, m_options[USE_BGRND], OPTION_PRIORITY_CMDLINE, error_string);
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
			if (value == ENLARGE_ARTS)
				mewui_globals::switch_image = true;
			m_options[value] = !m_options[value];
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
	item_append("Re-select last game / system played", m_options[REMEMBER_LAST_GAME] ? "On" : "Off",
				m_options[REMEMBER_LAST_GAME] ? MENU_FLAG_RIGHT_ARROW : MENU_FLAG_LEFT_ARROW, (void *)REMEMBER_LAST_GAME);

	item_append("Enlarge images in the right panel", m_options[ENLARGE_ARTS] ? "On" : "Off",
				m_options[ENLARGE_ARTS] ? MENU_FLAG_RIGHT_ARROW : MENU_FLAG_LEFT_ARROW, (void *)ENLARGE_ARTS);

	item_append("Loading DATs info", m_options[DATS_ENABLED] ? "On" : "Off",
				m_options[DATS_ENABLED] ? MENU_FLAG_RIGHT_ARROW : MENU_FLAG_LEFT_ARROW, (void *)DATS_ENABLED);

	item_append("Cheats", m_options[CHEAT_ENABLED] ? "On" : "Off",
				m_options[CHEAT_ENABLED] ? MENU_FLAG_RIGHT_ARROW : MENU_FLAG_LEFT_ARROW, (void *)CHEAT_ENABLED);

	item_append("Show mouse pointer", m_options[MOUSE_ENABLED] ? "On" : "Off",
				m_options[MOUSE_ENABLED] ? MENU_FLAG_RIGHT_ARROW : MENU_FLAG_LEFT_ARROW, (void *)MOUSE_ENABLED);

	item_append("Confirm quit from game / system", m_options[CONFIRM_QUIT_ENABLED] ? "On" : "Off",
				m_options[CONFIRM_QUIT_ENABLED] ? MENU_FLAG_RIGHT_ARROW : MENU_FLAG_LEFT_ARROW, (void *)CONFIRM_QUIT_ENABLED);

	item_append("Skip displaying information's screen at startup", m_options[SKIP_GAMEINFO_ENABLED] ? "On" : "Off",
				m_options[SKIP_GAMEINFO_ENABLED] ? MENU_FLAG_RIGHT_ARROW : MENU_FLAG_LEFT_ARROW, (void *)SKIP_GAMEINFO_ENABLED);

	item_append("Force Min 4:3 appearance for snapshot", m_options[FORCED_4X3] ? "On" : "Off",
				m_options[FORCED_4X3] ? MENU_FLAG_RIGHT_ARROW : MENU_FLAG_LEFT_ARROW, (void *)FORCED_4X3);

	item_append("Use image as background", m_options[USE_BGRND] ? "On" : "Off",
				m_options[USE_BGRND] ? MENU_FLAG_RIGHT_ARROW : MENU_FLAG_LEFT_ARROW, (void *)USE_BGRND);

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
