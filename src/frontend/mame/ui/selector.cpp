// license:BSD-3-Clause
// copyright-holders:Maurizio Petrarota
/*********************************************************************

    ui/selector.cpp

    Internal UI user interface.

*********************************************************************/

#include "emu.h"
#include "ui/selector.h"

#include "ui/ui.h"
#include "ui/utils.h"

#include "corestr.h"
#include "unicode.h"


namespace ui {

//-------------------------------------------------
//  ctor / dtor
//-------------------------------------------------

menu_selector::menu_selector(
		mame_ui_manager &mui,
		render_container &container,
		std::vector<std::string> &&sel,
		int initial,
		std::function<void (int)> &&handler)
	: menu(mui, container)
	, m_search()
	, m_str_items(std::move(sel))
	, m_handler(std::move(handler))
	, m_initial(initial)
{
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
			int selection(-1);
			for (size_t idx = 0; (m_str_items.size() > idx) && (0 > selection); ++idx)
				if ((void*)&m_str_items[idx] == menu_event->itemref)
					selection = int(unsigned(idx));

			m_handler(selection);

			stack_pop();
		}
		else if (menu_event->iptkey == IPT_SPECIAL)
		{
			if (input_character(m_search, menu_event->unichar, uchar_is_printable))
				reset(reset_options::SELECT_FIRST);
		}

		// escape pressed with non-empty text clears the text
		else if (menu_event->iptkey == IPT_UI_CANCEL && !m_search.empty())
		{
			m_search.clear();
			reset(reset_options::SELECT_FIRST);
		}
	}
}

//-------------------------------------------------
//  populate
//-------------------------------------------------

void menu_selector::populate(float &customtop, float &custombottom)
{
	if (!m_search.empty())
	{
		find_matches(m_search.c_str());

		for (int curitem = 0; m_searchlist[curitem]; ++curitem)
			item_append(*m_searchlist[curitem], 0, (void *)m_searchlist[curitem]);
	}
	else
	{
		for (size_t index = 0; index < m_str_items.size(); ++index)
		{
			if ((0 <= m_initial) && (unsigned(m_initial) == index))
				set_selected_index(index);

			item_append(m_str_items[index], 0, (void *)&m_str_items[index]);
		}
	}

	item_append(menu_item_type::SEPARATOR);
	customtop = custombottom = ui().get_line_height() + 3.0f * ui().box_tb_border();
	m_initial = -1;
}

//-------------------------------------------------
//  perform our special rendering
//-------------------------------------------------

void menu_selector::custom_render(void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
	std::string tempbuf[1] = { std::string(_("Selection List - Search: ")).append(m_search).append("_") };
	draw_text_box(
			std::begin(tempbuf), std::end(tempbuf),
			origx1, origx2, origy1 - top, origy1 - ui().box_tb_border(),
			ui::text_layout::CENTER, ui::text_layout::TRUNCATE, false,
			ui().colors().text_color(), UI_GREEN_COLOR, 1.0f);

	// get the text for 'UI Select'
	tempbuf[0] = string_format(_("Double-click or press %1$s to select"), ui().get_general_input_setting(IPT_UI_SELECT));
	draw_text_box(
			std::begin(tempbuf), std::end(tempbuf),
			origx1, origx2, origy2 + ui().box_tb_border(), origy2 + bottom,
			ui::text_layout::CENTER, ui::text_layout::NEVER, false,
			ui().colors().text_color(), ui().colors().background_color(), 1.0f);
}

//-------------------------------------------------
//  find approximate matches
//-------------------------------------------------

void menu_selector::find_matches(const char *str)
{
	// allocate memory to track the penalty value
	m_ucs_items.reserve(m_str_items.size());
	std::vector<double> penalty(VISIBLE_GAMES_IN_SEARCH, 1.0);
	std::u32string const search(ustr_from_utf8(normalize_unicode(str, unicode_normalization_form::D, true)));

	int index = 0;
	for ( ; index < m_str_items.size(); ++index)
	{
		assert(m_ucs_items.size() >= index);
		if (m_ucs_items.size() == index)
			m_ucs_items.emplace_back(ustr_from_utf8(normalize_unicode(m_str_items[index], unicode_normalization_form::D, true)));
		double const curpenalty(util::edit_distance(search, m_ucs_items[index]));

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
