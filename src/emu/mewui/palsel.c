/*********************************************************************

    mewui/palsel.c

    Internal MEWUI user interface.

*********************************************************************/

#include "emu.h"
#include "ui/ui.h"
#include "mewui/utils.h"
#include "mewui/palsel.h"

static const palcolor m_palette[] = {
	{ "White",  "FFFFFFFF" },
	{ "Silver", "FFC0C0C0" },
	{ "Gray",   "FF808080" },
	{ "Black",  "FF000000" },
	{ "Red",    "FFFF0000" },
	{ "Orange", "FFFFA500" },
	{ "Yellow", "FFFFFF00" },
	{ "Green",  "FF00FF00" },
	{ "Blue",   "FF0000FF" },
	{ "Violet", "FF8F00FF" }
};

//-------------------------------------------------
//  ctor
//-------------------------------------------------

ui_menu_palette_sel::ui_menu_palette_sel(running_machine &machine, render_container *container, rgb_t &_color) : ui_menu(machine, container),
	m_original(_color)
{
}

//-------------------------------------------------
//  dtor
//-------------------------------------------------

ui_menu_palette_sel::~ui_menu_palette_sel()
{
}

//-------------------------------------------------
//  handle
//-------------------------------------------------

void ui_menu_palette_sel::handle()
{
	// process the menu
	const ui_menu_event *menu_event = process(MENU_FLAG_MEWUI_PALETTE);
	if (menu_event != NULL && menu_event->itemref != NULL)
	{
		if (menu_event->iptkey == IPT_UI_SELECT)
		{
			m_original = rgb_t((UINT32)strtoul(item[selected].subtext, NULL, 16));
			ui_menu::menu_stack->parent->reset(UI_MENU_RESET_SELECT_FIRST);
			ui_menu::stack_pop(machine());
		}
	}
}

//-------------------------------------------------
//  populate
//-------------------------------------------------

void ui_menu_palette_sel::populate()
{
	for (int x = 0; x < ARRAY_LENGTH(m_palette); ++x)
		item_append(m_palette[x].m_name, m_palette[x].m_argb, MENU_FLAG_MEWUI_PALETTE, (void *)(FPTR)(x + 1));

	item_append(MENU_SEPARATOR_ITEM, NULL, 0, NULL);
}

//-------------------------------------------------
//  perform our special rendering
//-------------------------------------------------

void ui_menu_palette_sel::custom_render(void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
}
