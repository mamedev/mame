// license:BSD-3-Clause
// copyright-holders:Maurizio Petrarota
/*********************************************************************

    ui/selector.cpp

    Internal UI user interface.

*********************************************************************/

#include "emu.h"
#include "ui/ui.h"
#include "ui/menu.h"
#include "ui/selector.h"
#include "ui/inifile.h"

//-------------------------------------------------
//  ctor / dtor
//-------------------------------------------------

ui_menu_selector::ui_menu_selector(running_machine &machine, render_container *container, std::vector<std::string> const &s_sel, UINT16 &s_actual, int category, int _hover)
	: ui_menu(machine, container)
	, m_selector(s_actual)
	, m_category(category)
	, m_hover(_hover)
	, m_first_pass(true)
	, m_str_items(s_sel)
{
	m_search[0] = '\0';
	m_searchlist[0] = nullptr;
}

ui_menu_selector::ui_menu_selector(running_machine &machine, render_container *container, std::vector<std::string> &&s_sel, UINT16 &s_actual, int category, int _hover)
	: ui_menu(machine, container)
	, m_selector(s_actual)
	, m_category(category)
	, m_hover(_hover)
	, m_first_pass(true)
	, m_str_items(std::move(s_sel))
{
	m_search[0] = '\0';
	m_searchlist[0] = nullptr;
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
	const ui_menu_event *m_event = process(0);

	if (m_event != nullptr && m_event->itemref != nullptr)
	{
		if (m_event->iptkey == IPT_UI_SELECT)
		{
			for (size_t idx = 0; idx < m_str_items.size(); ++idx)
				if ((void*)&m_str_items[idx] == m_event->itemref)
					m_selector = idx;

			switch (m_category)
			{
				case SELECTOR_INIFILE:
					machine().inifile().set_file(m_selector);
					machine().inifile().set_cat(0);
					ui_menu::menu_stack->parent->reset(UI_MENU_RESET_REMEMBER_REF);
					break;

				case SELECTOR_CATEGORY:
					machine().inifile().set_cat(m_selector);
					ui_menu::menu_stack->parent->reset(UI_MENU_RESET_REMEMBER_REF);
					break;

				case SELECTOR_GAME:
					main_filters::actual = m_hover;
					ui_menu::menu_stack->parent->reset(UI_MENU_RESET_SELECT_FIRST);
					break;

				case SELECTOR_SOFTWARE:
					sw_filters::actual = m_hover;
					ui_menu::menu_stack->parent->reset(UI_MENU_RESET_SELECT_FIRST);
					break;

				default:
					ui_menu::menu_stack->parent->reset(UI_MENU_RESET_REMEMBER_REF);
					break;
			}

			ui_globals::switch_image = true;
			ui_menu::stack_pop(machine());
		}
		else if (m_event->iptkey == IPT_SPECIAL)
		{
			int buflen = strlen(m_search);

			// if it's a backspace and we can handle it, do so
			if ((m_event->unichar == 8 || m_event->unichar == 0x7f) && buflen > 0)
			{
				*(char *)utf8_previous_char(&m_search[buflen]) = 0;
				reset(UI_MENU_RESET_SELECT_FIRST);
			}

			// if it's any other key and we're not maxed out, update
			else if (m_event->unichar >= ' ' && m_event->unichar < 0x7f)
			{
				buflen += utf8_from_uchar(&m_search[buflen], ARRAY_LENGTH(m_search) - buflen, m_event->unichar);
				m_search[buflen] = 0;
				reset(UI_MENU_RESET_SELECT_FIRST);
			}
		}

		// escape pressed with non-empty text clears the text
		else if (m_event->iptkey == IPT_UI_CANCEL && m_search[0] != 0)
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

		for (int curitem = 0; m_searchlist[curitem]; ++curitem)
			item_append(m_searchlist[curitem]->c_str(), nullptr, 0, (void *)m_searchlist[curitem]);
	}
	else
	{
		for (size_t index = 0, added = 0; index < m_str_items.size(); ++index)
			if (m_str_items[index] != "_skip_")
			{
				if (m_first_pass && m_selector == index)
					selected = added;

				added++;
				item_append(m_str_items[index].c_str(), nullptr, 0, (void *)&m_str_items[index]);
			}
	}

	item_append(MENU_SEPARATOR_ITEM, nullptr, 0, nullptr);
	customtop = custombottom = machine().ui().get_line_height() + 3.0f * UI_BOX_TB_BORDER;
	m_first_pass = false;
}

//-------------------------------------------------
//  perform our special rendering
//-------------------------------------------------

void ui_menu_selector::custom_render(void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
	float width;
	ui_manager &mui = machine().ui();
	std::string tempbuf = std::string(_("Selection List - Search: ")).append(m_search).append("_");

	// get the size of the text
	mui.draw_text_full(container, tempbuf.c_str(), 0.0f, 0.0f, 1.0f, JUSTIFY_CENTER, WRAP_TRUNCATE,
		DRAW_NONE, ARGB_WHITE, ARGB_BLACK, &width, nullptr);
	width += (2.0f * UI_BOX_LR_BORDER) + 0.01f;
	float maxwidth = MAX(width, origx2 - origx1);

	// compute our bounds
	float x1 = 0.5f - 0.5f * maxwidth;
	float x2 = x1 + maxwidth;
	float y1 = origy1 - top;
	float y2 = origy1 - UI_BOX_TB_BORDER;

	// draw a box
	mui.draw_outlined_box(container, x1, y1, x2, y2, UI_GREEN_COLOR);

	// take off the borders
	x1 += UI_BOX_LR_BORDER;
	x2 -= UI_BOX_LR_BORDER;
	y1 += UI_BOX_TB_BORDER;

	// draw the text within it
	mui.draw_text_full(container, tempbuf.c_str(), x1, y1, x2 - x1, JUSTIFY_CENTER, WRAP_TRUNCATE,
		DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, nullptr, nullptr);

	// bottom text
	// get the text for 'UI Select'
	std::string ui_select_text = machine().input().seq_name(machine().ioport().type_seq(IPT_UI_SELECT, 0, SEQ_TYPE_STANDARD));
	tempbuf = string_format(_("Double click or press %1$s to select"), ui_select_text);

	mui.draw_text_full(container, tempbuf.c_str(), 0.0f, 0.0f, 1.0f, JUSTIFY_CENTER, WRAP_NEVER,
		DRAW_NONE, ARGB_WHITE, ARGB_BLACK, &width, nullptr);
	width += 2 * UI_BOX_LR_BORDER;
	maxwidth = MAX(maxwidth, width);

	// compute our bounds
	x1 = 0.5f - 0.5f * maxwidth;
	x2 = x1 + maxwidth;
	y1 = origy2 + UI_BOX_TB_BORDER;
	y2 = origy2 + bottom;

	// draw a box
	mui.draw_outlined_box(container, x1, y1, x2, y2, UI_RED_COLOR);

	// take off the borders
	x1 += UI_BOX_LR_BORDER;
	x2 -= UI_BOX_LR_BORDER;
	y1 += UI_BOX_TB_BORDER;

	// draw the text within it
	mui.draw_text_full(container, tempbuf.c_str(), x1, y1, x2 - x1, JUSTIFY_CENTER, WRAP_NEVER,
		DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, nullptr, nullptr);
}

//-------------------------------------------------
//  find approximate matches
//-------------------------------------------------

void ui_menu_selector::find_matches(const char *str)
{
	// allocate memory to track the penalty value
	std::vector<int> penalty(VISIBLE_GAMES_IN_SEARCH, 9999);
	int index = 0;

	for (; index < m_str_items.size(); ++index)
	{
		if (m_str_items[index] == "_skip_")
			continue;

		// pick the best match between driver name and description
		int curpenalty = fuzzy_substring(str, m_str_items[index]);

		// insert into the sorted table of matches
		for (int matchnum = VISIBLE_GAMES_IN_SEARCH - 1; matchnum >= 0; --matchnum)
		{
			// stop if we're worse than the current entry
			if (curpenalty >= penalty[matchnum])
				break;

			// as long as this isn't the last entry, bump this one down
			if (matchnum < VISIBLE_GAMES_IN_SEARCH - 1)
			{
				penalty[matchnum + 1] = penalty[matchnum];
				m_searchlist[matchnum + 1] = m_searchlist[matchnum];
			}

			m_searchlist[matchnum] = &m_str_items[index];
			penalty[matchnum] = curpenalty;
		}
	}
	(index < VISIBLE_GAMES_IN_SEARCH) ? m_searchlist[index] = nullptr : m_searchlist[VISIBLE_GAMES_IN_SEARCH] = nullptr;
}
