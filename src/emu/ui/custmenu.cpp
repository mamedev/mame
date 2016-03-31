// license:BSD-3-Clause
// copyright-holders:Maurizio Petrarota
/*********************************************************************

    ui/custmenu.cpp

    Internal UI user interface.

*********************************************************************/

#include "emu.h"
#include "ui/ui.h"
#include "ui/menu.h"
#include "ui/custmenu.h"
#include "ui/selector.h"
#include "ui/inifile.h"
#include "rendfont.h"

/**************************************************
    MENU CUSTOM FILTER
**************************************************/
//-------------------------------------------------
//  ctor / dtor
//-------------------------------------------------
ui_menu_custom_filter::ui_menu_custom_filter(running_machine &machine, render_container *container, bool _single_menu)
	: ui_menu(machine, container)
	, m_single_menu(_single_menu)
	, m_added(false)
{
}

ui_menu_custom_filter::~ui_menu_custom_filter()
{
	if (m_single_menu)
		ui_menu::menu_stack->reset(UI_MENU_RESET_SELECT_FIRST);
	save_custom_filters();
}

//-------------------------------------------------
//  handle
//-------------------------------------------------
void ui_menu_custom_filter::handle()
{
	bool changed = false;
	m_added = false;

	// process the menu
	const ui_menu_event *m_event = process(UI_MENU_PROCESS_LR_REPEAT);
	if (m_event != nullptr && m_event->itemref != nullptr)
	{
		switch ((FPTR)m_event->itemref)
		{
			case MAIN_FILTER:
				if (m_event->iptkey == IPT_UI_LEFT || m_event->iptkey == IPT_UI_RIGHT)
				{
					(m_event->iptkey == IPT_UI_RIGHT) ? custfltr::main++ : custfltr::main--;
					changed = true;
				}
				break;

			case ADD_FILTER:
				if (m_event->iptkey == IPT_UI_SELECT)
				{
					custfltr::numother++;
					custfltr::other[custfltr::numother] = FILTER_UNAVAILABLE + 1;
					m_added = true;
				}
				break;

			case REMOVE_FILTER:
				if (m_event->iptkey == IPT_UI_SELECT)
				{
					custfltr::other[custfltr::numother] = FILTER_UNAVAILABLE + 1;
					custfltr::numother--;
					changed = true;
				}
				break;
		}

		if ((FPTR)m_event->itemref >= OTHER_FILTER && (FPTR)m_event->itemref < OTHER_FILTER + MAX_CUST_FILTER)
		{
			int pos = (int)((FPTR)m_event->itemref - OTHER_FILTER);
			if (m_event->iptkey == IPT_UI_LEFT && custfltr::other[pos] > FILTER_UNAVAILABLE + 1)
			{
				custfltr::other[pos]--;
				for ( ; custfltr::other[pos] > FILTER_UNAVAILABLE && (custfltr::other[pos] == FILTER_CATEGORY
						|| custfltr::other[pos] == FILTER_FAVORITE); custfltr::other[pos]--) ;
				changed = true;
			}
			else if (m_event->iptkey == IPT_UI_RIGHT && custfltr::other[pos] < FILTER_LAST - 1)
			{
				custfltr::other[pos]++;
				for ( ; custfltr::other[pos] < FILTER_LAST && (custfltr::other[pos] == FILTER_CATEGORY
						|| custfltr::other[pos] == FILTER_FAVORITE); custfltr::other[pos]++) ;
				changed = true;
			}
			else if (m_event->iptkey == IPT_UI_SELECT)
			{
				size_t total = main_filters::length;
				std::vector<std::string> s_sel(total);
				for (size_t index = 0; index < total; ++index)
					if (index <= FILTER_UNAVAILABLE || index == FILTER_CATEGORY || index == FILTER_FAVORITE || index == FILTER_CUSTOM)
						s_sel[index] = "_skip_";
					else
						s_sel[index] = main_filters::text[index];

				ui_menu::stack_push(global_alloc_clear<ui_menu_selector>(machine(), container, s_sel, custfltr::other[pos]));
			}
		}
		else if ((FPTR)m_event->itemref >= YEAR_FILTER && (FPTR)m_event->itemref < YEAR_FILTER + MAX_CUST_FILTER)
		{
			int pos = (int)((FPTR)m_event->itemref - YEAR_FILTER);
			if (m_event->iptkey == IPT_UI_LEFT && custfltr::year[pos] > 0)
			{
				custfltr::year[pos]--;
				changed = true;
			}
			else if (m_event->iptkey == IPT_UI_RIGHT && custfltr::year[pos] < c_year::ui.size() - 1)
			{
				custfltr::year[pos]++;
				changed = true;
			}
			else if (m_event->iptkey == IPT_UI_SELECT)
				ui_menu::stack_push(global_alloc_clear<ui_menu_selector>(machine(), container, c_year::ui, custfltr::year[pos]));
		}
		else if ((FPTR)m_event->itemref >= MNFCT_FILTER && (FPTR)m_event->itemref < MNFCT_FILTER + MAX_CUST_FILTER)
		{
			int pos = (int)((FPTR)m_event->itemref - MNFCT_FILTER);
			if (m_event->iptkey == IPT_UI_LEFT && custfltr::mnfct[pos] > 0)
			{
				custfltr::mnfct[pos]--;
				changed = true;
			}
			else if (m_event->iptkey == IPT_UI_RIGHT && custfltr::mnfct[pos] < c_mnfct::ui.size() - 1)
			{
				custfltr::mnfct[pos]++;
				changed = true;
			}
			else if (m_event->iptkey == IPT_UI_SELECT)
				ui_menu::stack_push(global_alloc_clear<ui_menu_selector>(machine(), container, c_mnfct::ui, custfltr::mnfct[pos]));
		}
	}

	if (changed)
		reset(UI_MENU_RESET_REMEMBER_REF);
	else if (m_added)
		reset(UI_MENU_RESET_SELECT_FIRST);
}

//-------------------------------------------------
//  populate
//-------------------------------------------------
void ui_menu_custom_filter::populate()
{
	// add main filter
	UINT32 arrow_flags = get_arrow_flags((int)FILTER_ALL, (int)FILTER_UNAVAILABLE, custfltr::main);
	item_append(_("Main filter"), main_filters::text[custfltr::main], arrow_flags, (void *)(FPTR)MAIN_FILTER);

	// add other filters
	for (int x = 1; x <= custfltr::numother; x++)
	{
		item_append(MENU_SEPARATOR_ITEM, nullptr, 0, nullptr);

		// add filter items
		arrow_flags = get_arrow_flags((int)FILTER_UNAVAILABLE + 1, (int)FILTER_LAST - 1, custfltr::other[x]);
		item_append(_("Other filter"), main_filters::text[custfltr::other[x]], arrow_flags, (void *)(FPTR)(OTHER_FILTER + x));

		if (m_added)
			selected = item.size() - 2;

		// add manufacturer subitem
		if (custfltr::other[x] == FILTER_MANUFACTURER && c_mnfct::ui.size() > 0)
		{
			arrow_flags = get_arrow_flags(0, c_mnfct::ui.size() - 1, custfltr::mnfct[x]);
			std::string fbuff(_("^!Manufacturer"));
			convert_command_glyph(fbuff);
			item_append(fbuff.c_str(), c_mnfct::ui[custfltr::mnfct[x]].c_str(), arrow_flags, (void *)(FPTR)(MNFCT_FILTER + x));
		}

		// add year subitem
		else if (custfltr::other[x] == FILTER_YEAR && c_year::ui.size() > 0)
		{
			arrow_flags = get_arrow_flags(0, c_year::ui.size() - 1, custfltr::year[x]);
			std::string fbuff(_("^!Year"));
			convert_command_glyph(fbuff);
			item_append(fbuff.c_str(), c_year::ui[custfltr::year[x]].c_str(), arrow_flags, (void *)(FPTR)(YEAR_FILTER + x));
		}
	}

	item_append(MENU_SEPARATOR_ITEM, nullptr, 0, nullptr);

	if (custfltr::numother > 0)
		item_append(_("Remove last filter"), nullptr, 0, (void *)(FPTR)REMOVE_FILTER);

	if (custfltr::numother < MAX_CUST_FILTER - 2)
		item_append(_("Add filter"), nullptr, 0, (void *)(FPTR)ADD_FILTER);

	item_append(MENU_SEPARATOR_ITEM, nullptr, 0, nullptr);
	customtop = machine().ui().get_line_height() + 3.0f * UI_BOX_TB_BORDER;
}

//-------------------------------------------------
//  perform our special rendering
//-------------------------------------------------
void ui_menu_custom_filter::custom_render(void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
	float width;
	ui_manager &mui = machine().ui();

	// get the size of the text
	mui.draw_text_full(container, _("Select custom filters:"), 0.0f, 0.0f, 1.0f, JUSTIFY_CENTER, WRAP_NEVER,
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
	mui.draw_text_full(container, _("Select custom filters:"), x1, y1, x2 - x1, JUSTIFY_CENTER, WRAP_NEVER,
		DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, nullptr, nullptr);
}

//-------------------------------------------------
//  save custom filters info to file
//-------------------------------------------------

void ui_menu_custom_filter::save_custom_filters()
{
	// attempt to open the output file
	emu_file file(machine().ui().options().ui_path(), OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS);
	if (file.open("custom_", emulator_info::get_configname(), "_filter.ini") == osd_file::error::NONE)
	{
		// generate custom filters info
		std::ostringstream cinfo;
		util::stream_format(cinfo, "Total filters = %d\n", (custfltr::numother + 1));
		util::stream_format(cinfo, "Main filter = %s\n", main_filters::text[custfltr::main]);

		for (int x = 1; x <= custfltr::numother; x++)
		{
			util::stream_format(cinfo, "Other filter = %s\n", main_filters::text[custfltr::other[x]]);
			if (custfltr::other[x] == FILTER_MANUFACTURER)
				util::stream_format(cinfo, "  Manufacturer filter = %s\n", c_mnfct::ui[custfltr::mnfct[x]]);
			else if (custfltr::other[x] == FILTER_YEAR)
				util::stream_format(cinfo, "  Year filter = %s\n", c_year::ui[custfltr::year[x]]);
		}
		file.puts(cinfo.str().c_str());
		file.close();
	}
}

/**************************************************
    MENU CUSTOM SOFTWARE FILTER
**************************************************/
//-------------------------------------------------
//  ctor / dtor
//-------------------------------------------------
ui_menu_swcustom_filter::ui_menu_swcustom_filter(running_machine &machine, render_container *container, const game_driver *_driver, s_filter &_filter) :
	ui_menu(machine, container)
	, m_added(false)
	, m_filter(_filter)
	, m_driver(_driver)
{
}

ui_menu_swcustom_filter::~ui_menu_swcustom_filter()
{
	ui_menu::menu_stack->reset(UI_MENU_RESET_SELECT_FIRST);
	save_sw_custom_filters();
}

//-------------------------------------------------
//  handle
//-------------------------------------------------
void ui_menu_swcustom_filter::handle()
{
	bool changed = false;
	m_added = false;

	// process the menu
	const ui_menu_event *m_event = process(UI_MENU_PROCESS_LR_REPEAT);
	if (m_event != nullptr && m_event->itemref != nullptr)
	{
		switch ((FPTR)m_event->itemref)
		{
			case MAIN_FILTER:
				if (m_event->iptkey == IPT_UI_LEFT || m_event->iptkey == IPT_UI_RIGHT)
				{
					(m_event->iptkey == IPT_UI_RIGHT) ? sw_custfltr::main++ : sw_custfltr::main--;
					changed = true;
				}
				break;

			case ADD_FILTER:
				if (m_event->iptkey == IPT_UI_SELECT)
				{
					sw_custfltr::numother++;
					sw_custfltr::other[sw_custfltr::numother] = UI_SW_UNAVAILABLE + 1;
					m_added = true;
				}
				break;

			case REMOVE_FILTER:
				if (m_event->iptkey == IPT_UI_SELECT)
				{
					sw_custfltr::other[sw_custfltr::numother] = UI_SW_UNAVAILABLE + 1;
					sw_custfltr::numother--;
					changed = true;
				}
				break;
		}

		if ((FPTR)m_event->itemref >= OTHER_FILTER && (FPTR)m_event->itemref < OTHER_FILTER + MAX_CUST_FILTER)
		{
			int pos = (int)((FPTR)m_event->itemref - OTHER_FILTER);
			if (m_event->iptkey == IPT_UI_LEFT && sw_custfltr::other[pos] > UI_SW_UNAVAILABLE + 1)
			{
				sw_custfltr::other[pos]--;
				changed = true;
			}
			else if (m_event->iptkey == IPT_UI_RIGHT && sw_custfltr::other[pos] < UI_SW_LAST - 1)
			{
				sw_custfltr::other[pos]++;
				changed = true;
			}
			else if (m_event->iptkey == IPT_UI_SELECT)
			{
				size_t total = sw_filters::length;
				std::vector<std::string> s_sel(total);
				for (size_t index = 0; index < total; ++index)
					if (index <= UI_SW_UNAVAILABLE|| index == UI_SW_CUSTOM)
						s_sel[index] = "_skip_";
					else
						s_sel[index] = sw_filters::text[index];

				ui_menu::stack_push(global_alloc_clear<ui_menu_selector>(machine(), container, s_sel, sw_custfltr::other[pos]));
			}
		}
		else if ((FPTR)m_event->itemref >= YEAR_FILTER && (FPTR)m_event->itemref < YEAR_FILTER + MAX_CUST_FILTER)
		{
			int pos = (int)((FPTR)m_event->itemref - YEAR_FILTER);
			if (m_event->iptkey == IPT_UI_LEFT && sw_custfltr::year[pos] > 0)
			{
				sw_custfltr::year[pos]--;
				changed = true;
			}
			else if (m_event->iptkey == IPT_UI_RIGHT && sw_custfltr::year[pos] < m_filter.year.ui.size() - 1)
			{
				sw_custfltr::year[pos]++;
				changed = true;
			}
			else if (m_event->iptkey == IPT_UI_SELECT)
				ui_menu::stack_push(global_alloc_clear<ui_menu_selector>(machine(), container, m_filter.year.ui, sw_custfltr::year[pos]));
		}
		else if ((FPTR)m_event->itemref >= TYPE_FILTER && (FPTR)m_event->itemref < TYPE_FILTER + MAX_CUST_FILTER)
		{
			int pos = (int)((FPTR)m_event->itemref - TYPE_FILTER);
			if (m_event->iptkey == IPT_UI_LEFT && sw_custfltr::type[pos] > 0)
			{
				sw_custfltr::type[pos]--;
				changed = true;
			}
			else if (m_event->iptkey == IPT_UI_RIGHT && sw_custfltr::type[pos] < m_filter.type.ui.size() - 1)
			{
				sw_custfltr::type[pos]++;
				changed = true;
			}
			else if (m_event->iptkey == IPT_UI_SELECT)
				ui_menu::stack_push(global_alloc_clear<ui_menu_selector>(machine(), container, m_filter.type.ui, sw_custfltr::type[pos]));
		}
		else if ((FPTR)m_event->itemref >= MNFCT_FILTER && (FPTR)m_event->itemref < MNFCT_FILTER + MAX_CUST_FILTER)
		{
			int pos = (int)((FPTR)m_event->itemref - MNFCT_FILTER);
			if (m_event->iptkey == IPT_UI_LEFT && sw_custfltr::mnfct[pos] > 0)
			{
				sw_custfltr::mnfct[pos]--;
				changed = true;
			}
			else if (m_event->iptkey == IPT_UI_RIGHT && sw_custfltr::mnfct[pos] < m_filter.publisher.ui.size() - 1)
			{
				sw_custfltr::mnfct[pos]++;
				changed = true;
			}
			else if (m_event->iptkey == IPT_UI_SELECT)
				ui_menu::stack_push(global_alloc_clear<ui_menu_selector>(machine(), container, m_filter.publisher.ui, sw_custfltr::mnfct[pos]));
		}
		else if ((FPTR)m_event->itemref >= REGION_FILTER && (FPTR)m_event->itemref < REGION_FILTER + MAX_CUST_FILTER)
		{
			int pos = (int)((FPTR)m_event->itemref - REGION_FILTER);
			if (m_event->iptkey == IPT_UI_LEFT && sw_custfltr::region[pos] > 0)
			{
				sw_custfltr::region[pos]--;
				changed = true;
			}
			else if (m_event->iptkey == IPT_UI_RIGHT && sw_custfltr::region[pos] < m_filter.region.ui.size() - 1)
			{
				sw_custfltr::region[pos]++;
				changed = true;
			}
			else if (m_event->iptkey == IPT_UI_SELECT)
				ui_menu::stack_push(global_alloc_clear<ui_menu_selector>(machine(), container, m_filter.region.ui, sw_custfltr::region[pos]));
		}
		else if ((FPTR)m_event->itemref >= LIST_FILTER && (FPTR)m_event->itemref < LIST_FILTER + MAX_CUST_FILTER)
		{
			int pos = (int)((FPTR)m_event->itemref - LIST_FILTER);
			if (m_event->iptkey == IPT_UI_LEFT && sw_custfltr::list[pos] > 0)
			{
				sw_custfltr::list[pos]--;
				changed = true;
			}
			else if (m_event->iptkey == IPT_UI_RIGHT && sw_custfltr::list[pos] < m_filter.swlist.name.size() - 1)
			{
				sw_custfltr::list[pos]++;
				changed = true;
			}
			else if (m_event->iptkey == IPT_UI_SELECT)
				ui_menu::stack_push(global_alloc_clear<ui_menu_selector>(machine(), container, m_filter.swlist.description, sw_custfltr::list[pos]));
		}
	}

	if (changed)
		reset(UI_MENU_RESET_REMEMBER_REF);
	else if (m_added)
		reset(UI_MENU_RESET_SELECT_FIRST);
}

//-------------------------------------------------
//  populate
//-------------------------------------------------
void ui_menu_swcustom_filter::populate()
{
	// add main filter
	UINT32 arrow_flags = get_arrow_flags((int)UI_SW_ALL, (int)UI_SW_UNAVAILABLE, sw_custfltr::main);
	item_append(_("Main filter"), sw_filters::text[sw_custfltr::main], arrow_flags, (void *)(FPTR)MAIN_FILTER);

	// add other filters
	for (int x = 1; x <= sw_custfltr::numother; x++)
	{
		item_append(MENU_SEPARATOR_ITEM, nullptr, 0, nullptr);

		// add filter items
		arrow_flags = get_arrow_flags((int)UI_SW_UNAVAILABLE + 1, (int)UI_SW_LAST - 1, sw_custfltr::other[x]);
		item_append(_("Other filter"), sw_filters::text[sw_custfltr::other[x]], arrow_flags, (void *)(FPTR)(OTHER_FILTER + x));

		if (m_added)
			selected = item.size() - 2;

		// add publisher subitem
		if (sw_custfltr::other[x] == UI_SW_PUBLISHERS && m_filter.publisher.ui.size() > 0)
		{
			arrow_flags = get_arrow_flags(0, m_filter.publisher.ui.size() - 1, sw_custfltr::mnfct[x]);
			std::string fbuff(_("^!Publisher"));
			convert_command_glyph(fbuff);
			item_append(fbuff.c_str(), m_filter.publisher.ui[sw_custfltr::mnfct[x]].c_str(), arrow_flags, (void *)(FPTR)(MNFCT_FILTER + x));
		}

		// add year subitem
		else if (sw_custfltr::other[x] == UI_SW_YEARS && m_filter.year.ui.size() > 0)
		{
			arrow_flags = get_arrow_flags(0, m_filter.year.ui.size() - 1, sw_custfltr::year[x]);
			std::string fbuff(_("^!Year"));
			convert_command_glyph(fbuff);
			item_append(fbuff.c_str(), m_filter.year.ui[sw_custfltr::year[x]].c_str(), arrow_flags, (void *)(FPTR)(YEAR_FILTER + x));
		}

		// add year subitem
		else if (sw_custfltr::other[x] == UI_SW_LIST && m_filter.swlist.name.size() > 0)
		{
			arrow_flags = get_arrow_flags(0, m_filter.swlist.name.size() - 1, sw_custfltr::list[x]);
			std::string fbuff(_("^!Software List"));
			convert_command_glyph(fbuff);
			item_append(fbuff.c_str(), m_filter.swlist.description[sw_custfltr::list[x]].c_str(), arrow_flags, (void *)(FPTR)(LIST_FILTER + x));
		}

		// add device type subitem
		else if (sw_custfltr::other[x] == UI_SW_TYPE && m_filter.type.ui.size() > 0)
		{
			arrow_flags = get_arrow_flags(0, m_filter.type.ui.size() - 1, sw_custfltr::type[x]);
			std::string fbuff(_("^!Device type"));
			convert_command_glyph(fbuff);
			item_append(fbuff.c_str(), m_filter.type.ui[sw_custfltr::type[x]].c_str(), arrow_flags, (void *)(FPTR)(TYPE_FILTER + x));
		}

		// add region subitem
		else if (sw_custfltr::other[x] == UI_SW_REGION && m_filter.region.ui.size() > 0)
		{
			arrow_flags = get_arrow_flags(0, m_filter.region.ui.size() - 1, sw_custfltr::region[x]);
			std::string fbuff(_("^!Region"));
			convert_command_glyph(fbuff);
			item_append(fbuff.c_str(), m_filter.region.ui[sw_custfltr::region[x]].c_str(), arrow_flags, (void *)(FPTR)(REGION_FILTER + x));
		}
	}

	item_append(MENU_SEPARATOR_ITEM, nullptr, 0, nullptr);

	if (sw_custfltr::numother > 0)
		item_append(_("Remove last filter"), nullptr, 0, (void *)(FPTR)REMOVE_FILTER);

	if (sw_custfltr::numother < MAX_CUST_FILTER - 2)
		item_append(_("Add filter"), nullptr, 0, (void *)(FPTR)ADD_FILTER);

	item_append(MENU_SEPARATOR_ITEM, nullptr, 0, nullptr);

	customtop = machine().ui().get_line_height() + 3.0f * UI_BOX_TB_BORDER;
}

//-------------------------------------------------
//  perform our special rendering
//-------------------------------------------------
void ui_menu_swcustom_filter::custom_render(void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
	float width;
	ui_manager &mui = machine().ui();

	// get the size of the text
	mui.draw_text_full(container, _("Select custom filters:"), 0.0f, 0.0f, 1.0f, JUSTIFY_CENTER, WRAP_NEVER,
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
	mui.draw_text_full(container, _("Select custom filters:"), x1, y1, x2 - x1, JUSTIFY_CENTER, WRAP_NEVER,
		DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, nullptr, nullptr);
}

//-------------------------------------------------
//  save custom filters info to file
//-------------------------------------------------

void ui_menu_swcustom_filter::save_sw_custom_filters()
{
	// attempt to open the output file
	emu_file file(machine().ui().options().ui_path(), OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS);
	if (file.open("custom_", m_driver->name, "_filter.ini") == osd_file::error::NONE)
	{
		// generate custom filters info
		std::ostringstream cinfo;
		util::stream_format(cinfo, "Total filters = %d\n", (sw_custfltr::numother + 1));
		util::stream_format(cinfo, "Main filter = %s\n", sw_filters::text[sw_custfltr::main]);

		for (int x = 1; x <= sw_custfltr::numother; x++)
		{
			util::stream_format(cinfo, "Other filter = %s\n", sw_filters::text[sw_custfltr::other[x]]);
			if (sw_custfltr::other[x] == UI_SW_PUBLISHERS)
				util::stream_format(cinfo, "  Manufacturer filter = %s\n", m_filter.publisher.ui[sw_custfltr::mnfct[x]]);
			else if (sw_custfltr::other[x] == UI_SW_LIST)
				util::stream_format(cinfo, "  Software List filter = %s\n", m_filter.swlist.name[sw_custfltr::list[x]]);
			else if (sw_custfltr::other[x] == UI_SW_YEARS)
				util::stream_format(cinfo, "  Year filter = %s\n", m_filter.year.ui[sw_custfltr::year[x]]);
			else if (sw_custfltr::other[x] == UI_SW_TYPE)
				util::stream_format(cinfo, "  Type filter = %s\n", m_filter.type.ui[sw_custfltr::type[x]]);
			else if (sw_custfltr::other[x] == UI_SW_REGION)
				util::stream_format(cinfo, "  Region filter = %s\n", m_filter.region.ui[sw_custfltr::region[x]]);
		}
		file.puts(cinfo.str().c_str());
		file.close();
	}
}
