// license:BSD-3-Clause
// copyright-holders:Dankan1890
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
	m_single_menu = _single_menu;
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
					m_added = true;
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

		if ((FPTR)menu_event->itemref >= OTHER_FILTER && (FPTR)menu_event->itemref < OTHER_FILTER + MAX_CUST_FILTER)
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
		else if ((FPTR)menu_event->itemref >= YEAR_FILTER && (FPTR)menu_event->itemref < YEAR_FILTER + MAX_CUST_FILTER)
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
		else if ((FPTR)menu_event->itemref >= MNFCT_FILTER && (FPTR)menu_event->itemref < MNFCT_FILTER + MAX_CUST_FILTER)
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
	else if (m_added)
		reset(UI_MENU_RESET_SELECT_FIRST);
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

		if (m_added)
			selected = item.size() - 2;

		// add manufacturer subitem
		if (custfltr::other[x] == FILTER_MANUFACTURER && c_mnfct::ui.size() > 0)
		{
			arrow_flags = get_arrow_flags(0, c_mnfct::ui.size() - 1, custfltr::mnfct[x]);
			std::string fbuff("^!Manufacturer");
			convert_command_glyph(fbuff);
			item_append(fbuff.c_str(), c_mnfct::ui[custfltr::mnfct[x]].c_str(), arrow_flags, (void *)(FPTR)(MNFCT_FILTER + x));
		}

		// add year subitem
		else if (custfltr::other[x] == FILTER_YEAR && c_year::ui.size() > 0)
		{
			arrow_flags = get_arrow_flags(0, c_year::ui.size() - 1, custfltr::year[x]);
			std::string fbuff("^!Year");
			convert_command_glyph(fbuff);
			item_append(fbuff.c_str(), c_year::ui[custfltr::year[x]].c_str(), arrow_flags, (void *)(FPTR)(YEAR_FILTER + x));
		}
	}

	item_append(MENU_SEPARATOR_ITEM, NULL, 0, NULL);

	if (custfltr::numother > 0)
		item_append("Remove last filter", NULL, 0, (void *)REMOVE_FILTER);

	if (custfltr::numother < MAX_CUST_FILTER - 2)
		item_append("Add filter", NULL, 0, (void *)ADD_FILTER);

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

//-------------------------------------------------
//  save custom filters info to file
//-------------------------------------------------

void ui_menu_custom_filter::save_custom_filters()
{
	// attempt to open the output file
	emu_file file(machine().options().mewui_path(), OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS);
	if (file.open("custom_", emulator_info::get_configname(), "_filter.ini") == FILERR_NONE)
	{
		// generate custom filters info
		std::string cinfo;
		strprintf(cinfo, "Total filters = %d\n", (custfltr::numother + 1));
		cinfo.append("Main filter = ").append(mewui_globals::filter_text[custfltr::main_filter]).append("\n");

		for (int x = 1; x <= custfltr::numother; x++)
		{
			cinfo.append("Other filter = ").append(mewui_globals::filter_text[custfltr::other[x]]).append("\n");
			if (custfltr::other[x] == FILTER_MANUFACTURER)
				cinfo.append("  Manufacturer filter = ").append(c_mnfct::ui[custfltr::mnfct[x]]).append("\n");
			else if (custfltr::other[x] == FILTER_YEAR)
				cinfo.append("  Year filter = ").append(c_year::ui[custfltr::year[x]]).append("\n");
		}
		file.puts(cinfo.c_str());
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
	ui_menu(machine, container), m_filter(_filter), m_driver(_driver)
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
	const ui_menu_event *menu_event = process(UI_MENU_PROCESS_LR_REPEAT);
	if (menu_event != NULL && menu_event->itemref != NULL)
	{
		switch ((FPTR)menu_event->itemref)
		{
			case MAIN_FILTER:
				if (menu_event->iptkey == IPT_UI_LEFT || menu_event->iptkey == IPT_UI_RIGHT)
				{
					(menu_event->iptkey == IPT_UI_RIGHT) ? sw_custfltr::main_filter++ : sw_custfltr::main_filter--;
					changed = true;
				}
				break;

			case ADD_FILTER:
				if (menu_event->iptkey == IPT_UI_SELECT)
				{
					sw_custfltr::numother++;
					sw_custfltr::other[sw_custfltr::numother] = MEWUI_SW_UNAVAILABLE + 1;
					m_added = true;
				}
				break;

			case REMOVE_FILTER:
				if (menu_event->iptkey == IPT_UI_SELECT)
				{
					sw_custfltr::other[sw_custfltr::numother] = MEWUI_SW_UNAVAILABLE + 1;
					sw_custfltr::numother--;
					changed = true;
				}
				break;
		}

		if ((FPTR)menu_event->itemref >= OTHER_FILTER && (FPTR)menu_event->itemref < OTHER_FILTER + MAX_CUST_FILTER)
		{
			int pos = (int)((FPTR)menu_event->itemref - OTHER_FILTER);
			if (menu_event->iptkey == IPT_UI_LEFT && sw_custfltr::other[pos] > MEWUI_SW_UNAVAILABLE + 1)
			{
				sw_custfltr::other[pos]--;
				changed = true;
			}
			else if (menu_event->iptkey == IPT_UI_RIGHT && sw_custfltr::other[pos] < MEWUI_SW_LAST - 1)
			{
				sw_custfltr::other[pos]++;
				changed = true;
			}
			else if (menu_event->iptkey == IPT_UI_SELECT)
			{
				int total = mewui_globals::sw_filter_len;
				std::vector<std::string> s_sel(total);
				for (int index = 0; index < total; index++)
					if (index <= MEWUI_SW_UNAVAILABLE|| index == MEWUI_SW_CUSTOM)
						s_sel[index].assign("_skip_");
					else
						s_sel[index].assign(mewui_globals::sw_filter_text[index]);

				ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_selector(machine(), container, s_sel, &sw_custfltr::other[pos])));
			}
		}
		else if ((FPTR)menu_event->itemref >= YEAR_FILTER && (FPTR)menu_event->itemref < YEAR_FILTER + MAX_CUST_FILTER)
		{
			int pos = (int)((FPTR)menu_event->itemref - YEAR_FILTER);
			if (menu_event->iptkey == IPT_UI_LEFT && sw_custfltr::year[pos] > 0)
			{
				sw_custfltr::year[pos]--;
				changed = true;
			}
			else if (menu_event->iptkey == IPT_UI_RIGHT && sw_custfltr::year[pos] < m_filter.year.ui.size() - 1)
			{
				sw_custfltr::year[pos]++;
				changed = true;
			}
			else if (menu_event->iptkey == IPT_UI_SELECT)
				ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_selector(machine(), container, m_filter.year.ui, &sw_custfltr::year[pos])));
		}
		else if ((FPTR)menu_event->itemref >= TYPE_FILTER && (FPTR)menu_event->itemref < TYPE_FILTER + MAX_CUST_FILTER)
		{
			int pos = (int)((FPTR)menu_event->itemref - TYPE_FILTER);
			if (menu_event->iptkey == IPT_UI_LEFT && sw_custfltr::type[pos] > 0)
			{
				sw_custfltr::type[pos]--;
				changed = true;
			}
			else if (menu_event->iptkey == IPT_UI_RIGHT && sw_custfltr::type[pos] < m_filter.type.ui.size() - 1)
			{
				sw_custfltr::type[pos]++;
				changed = true;
			}
			else if (menu_event->iptkey == IPT_UI_SELECT)
				ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_selector(machine(), container, m_filter.type.ui, &sw_custfltr::type[pos])));
		}
		else if ((FPTR)menu_event->itemref >= MNFCT_FILTER && (FPTR)menu_event->itemref < MNFCT_FILTER + MAX_CUST_FILTER)
		{
			int pos = (int)((FPTR)menu_event->itemref - MNFCT_FILTER);
			if (menu_event->iptkey == IPT_UI_LEFT && sw_custfltr::mnfct[pos] > 0)
			{
				sw_custfltr::mnfct[pos]--;
				changed = true;
			}
			else if (menu_event->iptkey == IPT_UI_RIGHT && sw_custfltr::mnfct[pos] < m_filter.publisher.ui.size() - 1)
			{
				sw_custfltr::mnfct[pos]++;
				changed = true;
			}
			else if (menu_event->iptkey == IPT_UI_SELECT)
				ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_selector(machine(), container, m_filter.publisher.ui, &sw_custfltr::mnfct[pos])));
		}
		else if ((FPTR)menu_event->itemref >= REGION_FILTER && (FPTR)menu_event->itemref < REGION_FILTER + MAX_CUST_FILTER)
		{
			int pos = (int)((FPTR)menu_event->itemref - REGION_FILTER);
			if (menu_event->iptkey == IPT_UI_LEFT && sw_custfltr::region[pos] > 0)
			{
				sw_custfltr::region[pos]--;
				changed = true;
			}
			else if (menu_event->iptkey == IPT_UI_RIGHT && sw_custfltr::region[pos] < m_filter.region.ui.size() - 1)
			{
				sw_custfltr::region[pos]++;
				changed = true;
			}
			else if (menu_event->iptkey == IPT_UI_SELECT)
				ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_selector(machine(), container, m_filter.region.ui, &sw_custfltr::region[pos])));
		}
		else if ((FPTR)menu_event->itemref >= LIST_FILTER && (FPTR)menu_event->itemref < LIST_FILTER + MAX_CUST_FILTER)
		{
			int pos = (int)((FPTR)menu_event->itemref - LIST_FILTER);
			if (menu_event->iptkey == IPT_UI_LEFT && sw_custfltr::list[pos] > 0)
			{
				sw_custfltr::list[pos]--;
				changed = true;
			}
			else if (menu_event->iptkey == IPT_UI_RIGHT && sw_custfltr::list[pos] < m_filter.swlist.name.size() - 1)
			{
				sw_custfltr::list[pos]++;
				changed = true;
			}
			else if (menu_event->iptkey == IPT_UI_SELECT)
				ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_selector(machine(), container, m_filter.swlist.description, &sw_custfltr::list[pos])));
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
	UINT32 arrow_flags = get_arrow_flags((int)MEWUI_SW_ALL, (int)MEWUI_SW_UNAVAILABLE, sw_custfltr::main_filter);
	item_append("Main filter", mewui_globals::sw_filter_text[sw_custfltr::main_filter], arrow_flags, (void *)MAIN_FILTER);

	// add other filters
	for (int x = 1; x <= sw_custfltr::numother; x++)
	{
		item_append(MENU_SEPARATOR_ITEM, NULL, 0, NULL);

		// add filter items
		arrow_flags = get_arrow_flags((int)MEWUI_SW_UNAVAILABLE + 1, (int)MEWUI_SW_LAST - 1, sw_custfltr::other[x]);
		item_append("Other filter", mewui_globals::sw_filter_text[sw_custfltr::other[x]], arrow_flags, (void *)(FPTR)(OTHER_FILTER + x));

		if (m_added)
			selected = item.size() - 2;

		// add publisher subitem
		if (sw_custfltr::other[x] == MEWUI_SW_PUBLISHERS && m_filter.publisher.ui.size() > 0)
		{
			arrow_flags = get_arrow_flags(0, m_filter.publisher.ui.size() - 1, sw_custfltr::mnfct[x]);
			std::string fbuff("^!Publisher");
			convert_command_glyph(fbuff);
			item_append(fbuff.c_str(), m_filter.publisher.ui[sw_custfltr::mnfct[x]].c_str(), arrow_flags, (void *)(FPTR)(MNFCT_FILTER + x));
		}

		// add year subitem
		else if (sw_custfltr::other[x] == MEWUI_SW_YEARS && m_filter.year.ui.size() > 0)
		{
			arrow_flags = get_arrow_flags(0, m_filter.year.ui.size() - 1, sw_custfltr::year[x]);
			std::string fbuff("^!Year");
			convert_command_glyph(fbuff);
			item_append(fbuff.c_str(), m_filter.year.ui[sw_custfltr::year[x]].c_str(), arrow_flags, (void *)(FPTR)(YEAR_FILTER + x));
		}

		// add year subitem
		else if (sw_custfltr::other[x] == MEWUI_SW_LIST && m_filter.swlist.name.size() > 0)
		{
			arrow_flags = get_arrow_flags(0, m_filter.swlist.name.size() - 1, sw_custfltr::list[x]);
			std::string fbuff("^!Software List");
			convert_command_glyph(fbuff);
			item_append(fbuff.c_str(), m_filter.swlist.description[sw_custfltr::list[x]].c_str(), arrow_flags, (void *)(FPTR)(LIST_FILTER + x));
		}

		// add device type subitem
		else if (sw_custfltr::other[x] == MEWUI_SW_TYPE && m_filter.type.ui.size() > 0)
		{
			arrow_flags = get_arrow_flags(0, m_filter.type.ui.size() - 1, sw_custfltr::type[x]);
			std::string fbuff("^!Device type");
			convert_command_glyph(fbuff);
			item_append(fbuff.c_str(), m_filter.type.ui[sw_custfltr::type[x]].c_str(), arrow_flags, (void *)(FPTR)(TYPE_FILTER + x));
		}

		// add region subitem
		else if (sw_custfltr::other[x] == MEWUI_SW_REGION && m_filter.region.ui.size() > 0)
		{
			arrow_flags = get_arrow_flags(0, m_filter.region.ui.size() - 1, sw_custfltr::region[x]);
			std::string fbuff("^!Region");
			convert_command_glyph(fbuff);
			item_append(fbuff.c_str(), m_filter.region.ui[sw_custfltr::region[x]].c_str(), arrow_flags, (void *)(FPTR)(REGION_FILTER + x));
		}
	}

	item_append(MENU_SEPARATOR_ITEM, NULL, 0, NULL);

	if (sw_custfltr::numother > 0)
		item_append("Remove last filter", NULL, 0, (void *)REMOVE_FILTER);

	if (sw_custfltr::numother < MAX_CUST_FILTER - 2)
		item_append("Add filter", NULL, 0, (void *)ADD_FILTER);

	item_append(MENU_SEPARATOR_ITEM, NULL, 0, NULL);

	customtop = machine().ui().get_line_height() + 3.0f * UI_BOX_TB_BORDER;
}

//-------------------------------------------------
//  perform our special rendering
//-------------------------------------------------
void ui_menu_swcustom_filter::custom_render(void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
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

//-------------------------------------------------
//  save custom filters info to file
//-------------------------------------------------

void ui_menu_swcustom_filter::save_sw_custom_filters()
{
	// attempt to open the output file
	emu_file file(machine().options().mewui_path(), OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS);
	if (file.open("custom_", m_driver->name, "_filter.ini") == FILERR_NONE)
	{
		// generate custom filters info
		std::string cinfo;
		strprintf(cinfo, "Total filters = %d\n", (sw_custfltr::numother + 1));
		cinfo.append("Main filter = ").append(mewui_globals::sw_filter_text[sw_custfltr::main_filter]).append("\n");

		for (int x = 1; x <= sw_custfltr::numother; x++)
		{
			cinfo.append("Other filter = ").append(mewui_globals::sw_filter_text[sw_custfltr::other[x]]).append("\n");
			if (sw_custfltr::other[x] == MEWUI_SW_PUBLISHERS)
				cinfo.append("  Manufacturer filter = ").append(m_filter.publisher.ui[sw_custfltr::mnfct[x]]).append("\n");
			else if (sw_custfltr::other[x] == MEWUI_SW_YEARS)
				cinfo.append("  Software List filter = ").append(m_filter.swlist.name[sw_custfltr::list[x]]).append("\n");
			else if (sw_custfltr::other[x] == MEWUI_SW_YEARS)
				cinfo.append("  Year filter = ").append(m_filter.year.ui[sw_custfltr::year[x]]).append("\n");
			else if (sw_custfltr::other[x] == MEWUI_SW_TYPE)
				cinfo.append("  Type filter = ").append(m_filter.type.ui[sw_custfltr::type[x]]).append("\n");
			else if (sw_custfltr::other[x] == MEWUI_SW_REGION)
				cinfo.append("  Region filter = ").append(m_filter.region.ui[sw_custfltr::region[x]]).append("\n");
		}
		file.puts(cinfo.c_str());
		file.close();
	}
}

