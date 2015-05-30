/*********************************************************************

	mewui/custmenu.c

	Internal MEWUI user interface.

*********************************************************************/

#include "emu.h"
#include "ui/ui.h"
#include "mewui/custmenu.h"
#include "mewui/utils.h"
#include "mewui/selector.h"
#include "mewui/inifile.h"
#include "rendfont.h"

/**************************************************
	MENU CUSTOM FILTER
**************************************************/
//-------------------------------------------------
//  ctor / dtor
//-------------------------------------------------
ui_menu_custom_filter::ui_menu_custom_filter(running_machine &machine, render_container *container, bool _single_menu) : ui_menu(machine, container)
{
	single_menu = _single_menu;
}

ui_menu_custom_filter::~ui_menu_custom_filter()
{
	if (single_menu)
		ui_menu::menu_stack->reset(UI_MENU_RESET_SELECT_FIRST);

	save_custom_filters();
}

//-------------------------------------------------
//  handle
//-------------------------------------------------
void ui_menu_custom_filter::handle()
{
	bool changed = false;

	// process the menu
	const ui_menu_event *menu_event = process(UI_MENU_PROCESS_LR_REPEAT);
	if (menu_event != NULL && menu_event->itemref != NULL)
	{
		switch ((FPTR)menu_event->itemref)
		{
			case MAIN_FILTER:
				if (menu_event->iptkey == IPT_UI_LEFT || menu_event->iptkey == IPT_UI_RIGHT)
				{
					(menu_event->iptkey == IPT_UI_RIGHT) ? custfltr::main_filter++ : custfltr::main_filter--;
					changed = true;
				}
				break;

			case ADD_FILTER:
				if (menu_event->iptkey == IPT_UI_SELECT)
				{
					custfltr::numother++;
					custfltr::other[custfltr::numother] = FILTER_UNAVAILABLE + 1;
					changed = true;
				}
				break;

			case REMOVE_FILTER:
				if (menu_event->iptkey == IPT_UI_SELECT)
				{
					custfltr::other[custfltr::numother] = FILTER_UNAVAILABLE + 1;
					custfltr::numother--;
					changed = true;
				}
				break;
		}

		if ((FPTR)menu_event->itemref >= OTHER_FILTER && (FPTR)menu_event->itemref < OTHER_FILTER + MAX_FILTER)
		{
			int pos = (int)((FPTR)menu_event->itemref - OTHER_FILTER);
			if (menu_event->iptkey == IPT_UI_LEFT && custfltr::other[pos] > FILTER_UNAVAILABLE + 1)
			{
				custfltr::other[pos]--;
				for ( ; custfltr::other[pos] > FILTER_UNAVAILABLE && (custfltr::other[pos] == FILTER_CATEGORY
						|| custfltr::other[pos] == FILTER_FAVORITE_GAME); custfltr::other[pos]--) ;
				changed = true;
			}

			else if (menu_event->iptkey == IPT_UI_RIGHT && custfltr::other[pos] < FILTER_LAST - 1)
			{
				custfltr::other[pos]++;
				for ( ; custfltr::other[pos] < FILTER_LAST && (custfltr::other[pos] == FILTER_CATEGORY
						|| custfltr::other[pos] == FILTER_FAVORITE_GAME); custfltr::other[pos]++) ;
				changed = true;
			}

			else if (menu_event->iptkey == IPT_UI_SELECT)
			{
				int total = mewui_globals::s_filter_text;
				std::vector<std::string> s_sel(total);
				for (int index = 0; index < total; index++)
					if (index <= FILTER_UNAVAILABLE || index == FILTER_CATEGORY || index == FILTER_FAVORITE_GAME || index == FILTER_CUSTOM)
						s_sel[index].assign("_skip_");
					else
						s_sel[index].assign(mewui_globals::filter_text[index]);

				ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_selector(machine(), container, s_sel, &custfltr::other[pos])));
			}
		}

		else if ((FPTR)menu_event->itemref >= YEAR_FILTER && (FPTR)menu_event->itemref < YEAR_FILTER + MAX_FILTER)
		{
			int pos = (int)((FPTR)menu_event->itemref - YEAR_FILTER);
			if (menu_event->iptkey == IPT_UI_LEFT && custfltr::year[pos] > 0)
			{
				custfltr::year[pos]--;
				changed = true;
			}

			else if (menu_event->iptkey == IPT_UI_RIGHT && custfltr::year[pos] < c_year::ui.size() - 1)
			{
				custfltr::year[pos]++;
				changed = true;
			}

			else if (menu_event->iptkey == IPT_UI_SELECT)
				ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_selector(machine(), container, c_year::ui, &custfltr::year[pos])));
		}

		else if ((FPTR)menu_event->itemref >= MNFCT_FILTER && (FPTR)menu_event->itemref < MNFCT_FILTER + MAX_FILTER)
		{
			int pos = (int)((FPTR)menu_event->itemref - MNFCT_FILTER);
			if (menu_event->iptkey == IPT_UI_LEFT && custfltr::mnfct[pos] > 0)
			{
				custfltr::mnfct[pos]--;
				changed = true;
			}

			else if (menu_event->iptkey == IPT_UI_RIGHT && custfltr::mnfct[pos] < c_mnfct::ui.size() - 1)
			{
				custfltr::mnfct[pos]++;
				changed = true;
			}

			else if (menu_event->iptkey == IPT_UI_SELECT)
				ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_selector(machine(), container, c_mnfct::ui, &custfltr::mnfct[pos])));
		}
	}

	if (changed)
		reset(UI_MENU_RESET_REMEMBER_REF);

}

//-------------------------------------------------
//  populate
//-------------------------------------------------
void ui_menu_custom_filter::populate()
{
	// add main filter
	UINT32 arrow_flags = get_arrow_flags((int)FILTER_ALL, (int)FILTER_UNAVAILABLE, custfltr::main_filter);
	item_append("Main filter", mewui_globals::filter_text[custfltr::main_filter], arrow_flags, (void *)MAIN_FILTER);

	// add other filters
	for (int x = 1; x <= custfltr::numother; x++)
	{
		item_append(MENU_SEPARATOR_ITEM, NULL, 0, NULL);

		// add filter items
		arrow_flags = get_arrow_flags((int)FILTER_UNAVAILABLE + 1, (int)FILTER_LAST - 1, custfltr::other[x]);
		item_append("Other filter", mewui_globals::filter_text[custfltr::other[x]], arrow_flags, (void *)(FPTR)(OTHER_FILTER + x));

		// add manufacturer subitem
		if (custfltr::other[x] == FILTER_MANUFACTURER && c_mnfct::ui.size() > 0)
		{
			if (c_mnfct::ui.size() == 1)
				arrow_flags = 0;
			else
				arrow_flags = get_arrow_flags(0, c_mnfct::ui.size() - 1, custfltr::mnfct[x]);

			std::string fbuff("^!Manufacturer");
			convert_command_glyph(fbuff);
			item_append(fbuff.c_str(), c_mnfct::ui[custfltr::mnfct[x]].c_str(), arrow_flags, (void *)(FPTR)(MNFCT_FILTER + x));
		}

		// add year subitem
		else if (custfltr::other[x] == FILTER_YEAR && c_year::ui.size() > 0)
		{
			if (c_year::ui.size() == 1)
				arrow_flags = 0;
			else
				arrow_flags = get_arrow_flags(0, c_year::ui.size() - 1, custfltr::year[x]);

			std::string fbuff("^!Year");
			convert_command_glyph(fbuff);
			item_append(fbuff.c_str(), c_year::ui[custfltr::year[x]].c_str(), arrow_flags, (void *)(FPTR)(YEAR_FILTER + x));
		}
	}

	item_append(MENU_SEPARATOR_ITEM, NULL, 0, NULL);

	if (custfltr::numother > 0)
		item_append("Remove last filter", NULL, 0, (void *)REMOVE_FILTER);

	if (custfltr::numother < MAX_FILTER)
		item_append("Add other filter", NULL, 0, (void *)ADD_FILTER);

	item_append(MENU_SEPARATOR_ITEM, NULL, 0, NULL);

	customtop = machine().ui().get_line_height() + 3.0f * UI_BOX_TB_BORDER;
}

//-------------------------------------------------
//  perform our special rendering
//-------------------------------------------------
void ui_menu_custom_filter::custom_render(void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
	float width;

	// get the size of the text
	machine().ui().draw_text_full(container, "Select custom filters:", 0.0f, 0.0f, 1.0f, JUSTIFY_CENTER, WRAP_NEVER,
			DRAW_NONE, ARGB_WHITE, ARGB_BLACK, &width, NULL);
	width += (2.0f * UI_BOX_LR_BORDER) + 0.01f;
	float maxwidth = MAX(width, origx2 - origx1);

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
	machine().ui().draw_text_full(container, "Select custom filters:", x1, y1, x2 - x1, JUSTIFY_CENTER, WRAP_NEVER,
			DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, NULL, NULL);
}
