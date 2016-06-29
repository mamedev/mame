// license:BSD-3-Clause
// copyright-holders:Maurizio Petrarota
/*********************************************************************

    ui/selector.cpp

    Internal UI user interface.

*********************************************************************/

#include "emu.h"

#include "ui/selector.h"
#include "ui/ui.h"
#include "ui/inifile.h"

#include "mame.h"

namespace ui {
//-------------------------------------------------
//  ctor / dtor
//-------------------------------------------------

menu_selector::menu_selector(mame_ui_manager &mui, render_container *container, std::vector<std::string> const &s_sel, UINT16 &s_actual, int category, int _hover)
	: menu(mui, container)
	, m_selector(s_actual)
	, m_category(category)
	, m_hover(_hover)
	, m_first_pass(true)
	, m_str_items(s_sel)
{
	m_search[0] = '\0';
	m_searchlist[0] = nullptr;
}

menu_selector::menu_selector(mame_ui_manager &mui, render_container *container, std::vector<std::string> &&s_sel, UINT16 &s_actual, int category, int _hover)
	: menu(mui, container)
	, m_selector(s_actual)
	, m_category(category)
	, m_hover(_hover)
	, m_first_pass(true)
	, m_str_items(std::move(s_sel))
{
	m_search[0] = '\0';
	m_searchlist[0] = nullptr;
}

menu_selector::~menu_selector()
{
}

//-------------------------------------------------
//  handle
//-------------------------------------------------

void menu_selector::handle()
{
	// process the menu
	const event *menu_event = process(0);

	if (menu_event != nullptr && menu_event->itemref != nullptr)
	{
		if (menu_event->iptkey == IPT_UI_SELECT)
		{
			for (size_t idx = 0; idx < m_str_items.size(); ++idx)
				if ((void*)&m_str_items[idx] == menu_event->itemref)
					m_selector = idx;

			switch (m_category)
			{
			case INIFILE:
				mame_machine_manager::instance()->inifile().set_file(m_selector);
				mame_machine_manager::instance()->inifile().set_cat(0);
				reset_parent(reset_options::REMEMBER_REF);
				break;

			case CATEGORY:
				mame_machine_manager::instance()->inifile().set_cat(m_selector);
				reset_parent(reset_options::REMEMBER_REF);
				break;

			case GAME:
				main_filters::actual = m_hover;
				reset_parent(reset_options::SELECT_FIRST);
				break;

			case SOFTWARE:
				sw_filters::actual = m_hover;
				reset_parent(reset_options::SELECT_FIRST);
				break;

			default:
				reset_parent(reset_options::REMEMBER_REF);
				break;
			}

			ui_globals::switch_image = true;
			menu::stack_pop(machine());
		}
		else if (menu_event->iptkey == IPT_SPECIAL)
		{
			auto const buflen = strlen(m_search);
			if ((menu_event->unichar == 8) || (menu_event->unichar == 0x7f))
			{
				// if it's a backspace and we can handle it, do so
				if (0 < buflen)
				{
					*const_cast<char *>(utf8_previous_char(&m_search[buflen])) = 0;
					reset(reset_options::SELECT_FIRST);
				}
			}
			else if (menu_event->is_char_printable())
			{
				// if it's any other key and we're not maxed out, update
				if (menu_event->append_char(m_search, buflen))
					reset(reset_options::SELECT_FIRST);
			}
		}

		// escape pressed with non-empty text clears the text
		else if (menu_event->iptkey == IPT_UI_CANCEL && m_search[0] != 0)
		{
			m_search[0] = '\0';
			reset(reset_options::SELECT_FIRST);
		}
	}
}

//-------------------------------------------------
//  populate
//-------------------------------------------------

void menu_selector::populate()
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

	item_append(menu_item_type::SEPARATOR);
	customtop = custombottom = ui().get_line_height() + 3.0f * UI_BOX_TB_BORDER;
	m_first_pass = false;
}

//-------------------------------------------------
//  perform our special rendering
//-------------------------------------------------

void menu_selector::custom_render(void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
	float width;
	std::string tempbuf = std::string(_("Selection List - Search: ")).append(m_search).append("_");

	// get the size of the text
	ui().draw_text_full(container, tempbuf.c_str(), 0.0f, 0.0f, 1.0f, ui::text_layout::CENTER, ui::text_layout::TRUNCATE,
		mame_ui_manager::NONE, rgb_t::white, rgb_t::black, &width, nullptr);
	width += (2.0f * UI_BOX_LR_BORDER) + 0.01f;
	float maxwidth = MAX(width, origx2 - origx1);

	// compute our bounds
	float x1 = 0.5f - 0.5f * maxwidth;
	float x2 = x1 + maxwidth;
	float y1 = origy1 - top;
	float y2 = origy1 - UI_BOX_TB_BORDER;

	// draw a box
	ui().draw_outlined_box(container, x1, y1, x2, y2, UI_GREEN_COLOR);

	// take off the borders
	x1 += UI_BOX_LR_BORDER;
	x2 -= UI_BOX_LR_BORDER;
	y1 += UI_BOX_TB_BORDER;

	// draw the text within it
	ui().draw_text_full(container, tempbuf.c_str(), x1, y1, x2 - x1, ui::text_layout::CENTER, ui::text_layout::TRUNCATE,
		mame_ui_manager::NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, nullptr, nullptr);

	// bottom text
	// get the text for 'UI Select'
	std::string ui_select_text = machine().input().seq_name(machine().ioport().type_seq(IPT_UI_SELECT, 0, SEQ_TYPE_STANDARD));
	tempbuf = string_format(_("Double click or press %1$s to select"), ui_select_text);

	ui().draw_text_full(container, tempbuf.c_str(), 0.0f, 0.0f, 1.0f, ui::text_layout::CENTER, ui::text_layout::NEVER,
		mame_ui_manager::NONE, rgb_t::white, rgb_t::black, &width, nullptr);
	width += 2 * UI_BOX_LR_BORDER;
	maxwidth = MAX(maxwidth, width);

	// compute our bounds
	x1 = 0.5f - 0.5f * maxwidth;
	x2 = x1 + maxwidth;
	y1 = origy2 + UI_BOX_TB_BORDER;
	y2 = origy2 + bottom;

	// draw a box
	ui().draw_outlined_box(container, x1, y1, x2, y2, UI_RED_COLOR);

	// take off the borders
	x1 += UI_BOX_LR_BORDER;
	x2 -= UI_BOX_LR_BORDER;
	y1 += UI_BOX_TB_BORDER;

	// draw the text within it
	ui().draw_text_full(container, tempbuf.c_str(), x1, y1, x2 - x1, ui::text_layout::CENTER, ui::text_layout::NEVER,
		mame_ui_manager::NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, nullptr, nullptr);
}

//-------------------------------------------------
//  find approximate matches
//-------------------------------------------------

void menu_selector::find_matches(const char *str)
{
	// allocate memory to track the penalty value
	std::vector<int> penalty(VISIBLE_GAMES_IN_SEARCH, 9999);
	int index = 0;

	for (; index < m_str_items.size(); ++index)
	{
		if (m_str_items[index] == "_skip_")
			continue;

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

} // namespace ui
