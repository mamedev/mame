/*********************************************************************

	mewui/selector.c

	Internal MEWUI user interface.

*********************************************************************/

#include "emu.h"
#include "ui/ui.h"
#include "mewui/selector.h"
#include "mewui/inifile.h"
#include "mewui/utils.h"

//-------------------------------------------------
//  ctor / dtor
//-------------------------------------------------

ui_menu_selector::ui_menu_selector(running_machine &machine, render_container *container, std::vector<std::string> s_sel, UINT16 *s_actual, int category, int _hover) : ui_menu(machine, container)
{
	p_category = category;
	selector = s_actual;
	first_pass = true;
	hover = _hover;
	str_items = s_sel;
}

ui_menu_selector::~ui_menu_selector()
{
}

//-------------------------------------------------
//  handle
//-------------------------------------------------

void ui_menu_selector::handle()
{
	// process the menu
	const ui_menu_event *menu_event = process(0);

	if (menu_event != NULL && menu_event->itemref != NULL)
	{
		if (menu_event->iptkey == IPT_UI_SELECT)
		{
			for (int idx = 0; idx < str_items.size(); idx++)
				if ((void*)&str_items[idx] == menu_event->itemref)
					*selector = idx;

			switch (p_category)
			{
				case SELECTOR_INIFILE:
					machine().inifile().current_file = *selector;
					machine().inifile().current_category = 0;
					ui_menu::menu_stack->parent->reset(UI_MENU_RESET_REMEMBER_REF);
					break;

				case SELECTOR_CATEGORY:
					machine().inifile().current_category = *selector;
					ui_menu::menu_stack->parent->reset(UI_MENU_RESET_REMEMBER_REF);
					break;

				case SELECTOR_GAME:
					mewui_globals::actual_filter = hover;
					ui_menu::menu_stack->parent->reset(UI_MENU_RESET_SELECT_FIRST);
					break;

				case SELECTOR_SOFTWARE:
					mewui_globals::actual_sw_filter = hover;
					ui_menu::menu_stack->parent->reset(UI_MENU_RESET_SELECT_FIRST);
					break;

				default:
					ui_menu::menu_stack->parent->reset(UI_MENU_RESET_REMEMBER_REF);
					break;
			}

			mewui_globals::switch_image = true;
			ui_menu::stack_pop(machine());
		}
		else if (menu_event->iptkey == IPT_SPECIAL)
		{
			int buflen = strlen(m_search);

			// if it's a backspace and we can handle it, do so
			if ((menu_event->unichar == 8 || menu_event->unichar == 0x7f) && buflen > 0)
			{
				*(char *)utf8_previous_char(&m_search[buflen]) = 0;
				reset(UI_MENU_RESET_SELECT_FIRST);
			}

			// if it's any other key and we're not maxed out, update
			else if (menu_event->unichar >= ' ' && menu_event->unichar < 0x7f)
			{
				buflen += utf8_from_uchar(&m_search[buflen], ARRAY_LENGTH(m_search) - buflen, menu_event->unichar);
				m_search[buflen] = 0;
				reset(UI_MENU_RESET_SELECT_FIRST);
			}
		}

		// escape pressed with non-empty text clears the text
		else if (menu_event->iptkey == IPT_UI_CANCEL && m_search[0] != 0)
		{
			m_search[0] = '\0';
			reset(UI_MENU_RESET_SELECT_FIRST);
		}
	}
}

//-------------------------------------------------
//  populate
//-------------------------------------------------

void ui_menu_selector::populate()
{
	if (m_search[0] != 0)
	{
		find_matches(m_search);

		for (int curitem = 0; searchlist[curitem]; curitem++)
			item_append(searchlist[curitem]->c_str(), NULL, 0, (void *)searchlist[curitem]);
	}
	else
	{
		for (int index = 0, added = 0; index < str_items.size(); index++)
			if (str_items[index].compare("_skip_") != 0)
			{
				if (first_pass && *selector == index)
					selected = added;

				added++;
				item_append(str_items[index].c_str(), NULL, 0, (void *)&str_items[index]);
			}
	}

	item_append(MENU_SEPARATOR_ITEM, NULL, 0, NULL);
	customtop = machine().ui().get_line_height() + 3.0f * UI_BOX_TB_BORDER;
	custombottom = machine().ui().get_line_height() + 3.0f * UI_BOX_TB_BORDER;
	first_pass = false;
}

//-------------------------------------------------
//  perform our special rendering
//-------------------------------------------------

void ui_menu_selector::custom_render(void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
	float width;
	std::string tempbuf = std::string("Selection List - Search: ").append(m_search).append("_");

	// get the size of the text
	machine().ui().draw_text_full(container, tempbuf.c_str(), 0.0f, 0.0f, 1.0f, JUSTIFY_CENTER, WRAP_TRUNCATE,
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
	machine().ui().draw_text_full(container, tempbuf.c_str(), x1, y1, x2 - x1, JUSTIFY_CENTER, WRAP_TRUNCATE,
									DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, NULL, NULL);

	// bottom text
	// get the text for 'UI Select'
	std::string ui_select_text;
	machine().input().seq_name(ui_select_text, machine().ioport().type_seq(IPT_UI_SELECT, 0, SEQ_TYPE_STANDARD));
	tempbuf.assign("Double click or press ").append(ui_select_text.c_str()).append(" to select");

	machine().ui().draw_text_full(container, tempbuf.c_str(), 0.0f, 0.0f, 1.0f, JUSTIFY_CENTER, WRAP_NEVER,
									DRAW_NONE, ARGB_WHITE, ARGB_BLACK, &width, NULL);
	width += 2 * UI_BOX_LR_BORDER;
	maxwidth = MAX(maxwidth, width);

	// compute our bounds
	x1 = 0.5f - 0.5f * maxwidth;
	x2 = x1 + maxwidth;
	y1 = origy2 + UI_BOX_TB_BORDER;
	y2 = origy2 + bottom;

	// draw a box
	machine().ui().draw_outlined_box(container, x1, y1, x2, y2, UI_RED_COLOR);

	// take off the borders
	x1 += UI_BOX_LR_BORDER;
	x2 -= UI_BOX_LR_BORDER;
	y1 += UI_BOX_TB_BORDER;

	// draw the text within it
	machine().ui().draw_text_full(container, tempbuf.c_str(), x1, y1, x2 - x1, JUSTIFY_CENTER, WRAP_NEVER,
									DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, NULL, NULL);
}

//-------------------------------------------------
//  find approximate matches
//-------------------------------------------------

void ui_menu_selector::find_matches(const char *str)
{
	// allocate memory to track the penalty value
	std::vector<int> penalty(VISIBLE_GAMES_IN_SEARCH, 9999);
	int index = 0;

	for (; index < str_items.size(); index++)
	{
		if (str_items[index].compare("_skip_") == 0)
			continue;

		// pick the best match between driver name and description
		int curpenalty = fuzzy_substring(str, str_items[index].c_str());
		int tmp = fuzzy_substring(str, str_items[index].c_str());
		curpenalty = MIN(curpenalty, tmp);

		// insert into the sorted table of matches
		for (int matchnum = VISIBLE_GAMES_IN_SEARCH - 1; matchnum >= 0; matchnum--)
		{
			// stop if we're worse than the current entry
			if (curpenalty >= penalty[matchnum])
				break;

			// as long as this isn't the last entry, bump this one down
			if (matchnum < VISIBLE_GAMES_IN_SEARCH - 1)
			{
				penalty[matchnum + 1] = penalty[matchnum];
				searchlist[matchnum + 1] = searchlist[matchnum];
			}

			searchlist[matchnum] = &str_items[index];
			penalty[matchnum] = curpenalty;
		}
	}
	(index < VISIBLE_GAMES_IN_SEARCH) ? searchlist[index] = NULL : searchlist[VISIBLE_GAMES_IN_SEARCH] = NULL;
}

