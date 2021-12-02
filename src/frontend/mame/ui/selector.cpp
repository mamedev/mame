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

#include "rapidfuzz/fuzz.hpp"
#include "rapidfuzz/utils.hpp"


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

void menu_selector::handle(event const *ev)
{
	// process the menu
	if (ev && ev->itemref)
	{
		if (ev->iptkey == IPT_UI_SELECT)
		{
			int selection(-1);
			for (size_t idx = 0; (m_str_items.size() > idx) && (0 > selection); ++idx)
				if ((void*)&m_str_items[idx] == ev->itemref)
					selection = int(unsigned(idx));

			m_handler(selection);

			stack_pop();
		}
		else if (ev->iptkey == IPT_SPECIAL)
		{
			if (input_character(m_search, ev->unichar, uchar_is_printable))
				reset(reset_options::SELECT_FIRST);
		}

		// escape pressed with non-empty text clears the text
		else if (ev->iptkey == IPT_UI_CANCEL && !m_search.empty())
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
			text_layout::text_justify::CENTER, text_layout::word_wrapping::TRUNCATE, false,
			ui().colors().text_color(), UI_GREEN_COLOR, 1.0f);

	// get the text for 'UI Select'
	tempbuf[0] = string_format(_("Double-click or press %1$s to select"), ui().get_general_input_setting(IPT_UI_SELECT));
	draw_text_box(
			std::begin(tempbuf), std::end(tempbuf),
			origx1, origx2, origy2 + ui().box_tb_border(), origy2 + bottom,
			text_layout::text_justify::CENTER, text_layout::word_wrapping::NEVER, false,
			ui().colors().text_color(), ui().colors().background_color(), 1.0f);
}

//-------------------------------------------------
//  find approximate matches
//-------------------------------------------------

void menu_selector::find_matches(const char *str)
{
	// allocate memory to track the ratio value
	m_ucs_items.reserve(m_str_items.size());
	std::vector<double> ratio(VISIBLE_ITEMS_IN_SEARCH, 0.0);
	std::u32string const search(ustr_from_utf8(normalize_unicode(str, unicode_normalization_form::D, true)));
	auto scorer = rapidfuzz::fuzz::CachedRatio<std::u32string>(search);

	m_searchlist[0] = nullptr;
	int index = 0;
	for ( ; index < m_str_items.size(); ++index)
	{
		assert(m_ucs_items.size() >= index);
		if (m_ucs_items.size() == index)
			m_ucs_items.emplace_back(ustr_from_utf8(normalize_unicode(m_str_items[index], unicode_normalization_form::D, true)));
		double const curratio(scorer.ratio(m_ucs_items[index]));

		// insert into the sorted table of matches
		auto pos = std::lower_bound(ratio.begin(), ratio.end(), curratio, std::greater<double>());
		if (ratio.end() != pos)
		{
			std::copy(pos, std::prev(ratio.end()), std::next(pos));
			std::copy(
					std::begin(m_searchlist) + std::distance(ratio.begin(), pos),
					std::prev(std::end(m_searchlist)),
					std::begin(m_searchlist) + std::distance(ratio.begin(), pos) + 1);
			*pos = curratio;
			m_searchlist[std::distance(ratio.begin(), pos)] = &m_str_items[index];
		}
	}
}

} // namespace ui
