// license:BSD-3-Clause
// copyright-holders:Maurizio Petrarota
/*********************************************************************

    ui/custmenu.cpp

    Internal UI user interface.

*********************************************************************/

#include "emu.h"
#include "ui/custmenu.h"

#include "ui/ui.h"
#include "ui/selector.h"
#include "ui/inifile.h"

#include "rendfont.h"


namespace ui {

/**************************************************
    MENU CUSTOM FILTER
**************************************************/
//-------------------------------------------------
//  ctor / dtor
//-------------------------------------------------
menu_custom_filter::menu_custom_filter(mame_ui_manager &mui, render_container &container, bool _single_menu)
	: menu(mui, container)
	, m_single_menu(_single_menu)
	, m_added(false)
{
}

menu_custom_filter::~menu_custom_filter()
{
	if (m_single_menu)
		reset_topmost(reset_options::SELECT_FIRST);
	save_custom_filters();
}

//-------------------------------------------------
//  handle
//-------------------------------------------------
void menu_custom_filter::handle()
{
	bool changed = false;
	m_added = false;

	// process the menu
	const event *menu_event = process(PROCESS_LR_REPEAT);
	if (menu_event != nullptr && menu_event->itemref != nullptr)
	{
		switch ((uintptr_t)menu_event->itemref)
		{
		case MAIN_FILTER:
			if (menu_event->iptkey == IPT_UI_LEFT || menu_event->iptkey == IPT_UI_RIGHT)
			{
				(menu_event->iptkey == IPT_UI_RIGHT) ? custfltr::main++ : custfltr::main--;
				changed = true;
			}
			break;

		case ADD_FILTER:
			if (menu_event->iptkey == IPT_UI_SELECT)
			{
				custfltr::numother++;
				custfltr::other[custfltr::numother] = machine_filter::UNAVAILABLE;
				++custfltr::other[custfltr::numother];
				m_added = true;
			}
			break;

		case REMOVE_FILTER:
			if (menu_event->iptkey == IPT_UI_SELECT)
			{
				custfltr::other[custfltr::numother] = machine_filter::UNAVAILABLE;
				++custfltr::other[custfltr::numother];
				custfltr::numother--;
				changed = true;
			}
			break;
		}

		if ((uintptr_t)menu_event->itemref >= OTHER_FILTER && (uintptr_t)menu_event->itemref < OTHER_FILTER + MAX_CUST_FILTER)
		{
			int pos = (int)((uintptr_t)menu_event->itemref - OTHER_FILTER);
			if (menu_event->iptkey == IPT_UI_LEFT && custfltr::other[pos] > machine_filter::UNAVAILABLE + 1)
			{
				custfltr::other[pos]--;
				for ( ; custfltr::other[pos] > machine_filter::UNAVAILABLE && (custfltr::other[pos] == machine_filter::CATEGORY
						|| custfltr::other[pos] == machine_filter::FAVORITE); custfltr::other[pos]--) { };
				changed = true;
			}
			else if (menu_event->iptkey == IPT_UI_RIGHT && custfltr::other[pos] < machine_filter::LAST - 1)
			{
				custfltr::other[pos]++;
				for ( ; custfltr::other[pos] < machine_filter::LAST && (custfltr::other[pos] == machine_filter::CATEGORY
						|| custfltr::other[pos] == machine_filter::FAVORITE); custfltr::other[pos]++) { };
				changed = true;
			}
			else if (menu_event->iptkey == IPT_UI_SELECT)
			{
				std::vector<machine_filter::type> types;
				std::vector<std::string> names;
				types.reserve(machine_filter::COUNT);
				names.reserve(machine_filter::COUNT);
				int sel(-1);
				for (machine_filter::type index = machine_filter::FIRST; index < machine_filter::COUNT; ++index)
				{
					if ((index > machine_filter::UNAVAILABLE) && (index != machine_filter::CATEGORY) && (index != machine_filter::FAVORITE) && (index != machine_filter::CUSTOM))
					{
						if (custfltr::other[pos] == index)
							sel = types.size();
						types.emplace_back(index);
						names.emplace_back(machine_filter::display_name(index));
					}
				}
				menu::stack_push<menu_selector>(ui(), container(), std::move(names), sel, [this, pos, t = std::move(types)] (int selection) { custfltr::other[pos] = t[selection]; reset(reset_options::REMEMBER_REF); });
			}
		}
		else if ((uintptr_t)menu_event->itemref >= YEAR_FILTER && (uintptr_t)menu_event->itemref < YEAR_FILTER + MAX_CUST_FILTER)
		{
			int pos = (int)((uintptr_t)menu_event->itemref - YEAR_FILTER);
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
			{
				menu::stack_push<menu_selector>(ui(), container(), std::vector<std::string>(c_year::ui), custfltr::year[pos], [this, pos] (int selection) { custfltr::year[pos] = selection; reset(reset_options::REMEMBER_REF); });
			}
		}
		else if ((uintptr_t)menu_event->itemref >= MNFCT_FILTER && (uintptr_t)menu_event->itemref < MNFCT_FILTER + MAX_CUST_FILTER)
		{
			int pos = (int)((uintptr_t)menu_event->itemref - MNFCT_FILTER);
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
			{
				menu::stack_push<menu_selector>(ui(), container(), std::vector<std::string>(c_mnfct::ui), custfltr::mnfct[pos], [this, pos] (int selection) { custfltr::mnfct[pos] = selection; reset(reset_options::REMEMBER_REF); });
			}
		}
	}

	if (changed)
		reset(reset_options::REMEMBER_REF);
	else if (m_added)
		reset(reset_options::SELECT_FIRST);
}

//-------------------------------------------------
//  populate
//-------------------------------------------------
void menu_custom_filter::populate(float &customtop, float &custombottom)
{
	// add main filter
	uint32_t arrow_flags = get_arrow_flags<uint16_t>(machine_filter::ALL, machine_filter::UNAVAILABLE, custfltr::main);
	item_append(_("Main filter"), machine_filter::display_name(custfltr::main), arrow_flags, (void *)(uintptr_t)MAIN_FILTER);

	// add other filters
	for (int x = 1; x <= custfltr::numother; x++)
	{
		item_append(menu_item_type::SEPARATOR);

		// add filter items
		arrow_flags = get_arrow_flags<uint16_t>(machine_filter::UNAVAILABLE + 1, machine_filter::LAST - 1, custfltr::other[x]);
		item_append(_("Other filter"), machine_filter::display_name(custfltr::other[x]), arrow_flags, (void *)(uintptr_t)(OTHER_FILTER + x));

		if (m_added)
			selected = item.size() - 2;

		// add manufacturer subitem
		if (custfltr::other[x] == machine_filter::MANUFACTURER && c_mnfct::ui.size() > 0)
		{
			arrow_flags = get_arrow_flags<uint16_t>(0, c_mnfct::ui.size() - 1, custfltr::mnfct[x]);
			std::string fbuff(_("^!Manufacturer"));
			convert_command_glyph(fbuff);
			item_append(fbuff, c_mnfct::ui[custfltr::mnfct[x]], arrow_flags, (void *)(uintptr_t)(MNFCT_FILTER + x));
		}

		// add year subitem
		else if (custfltr::other[x] == machine_filter::YEAR && c_year::ui.size() > 0)
		{
			arrow_flags = get_arrow_flags<uint16_t>(0, c_year::ui.size() - 1, custfltr::year[x]);
			std::string fbuff(_("^!Year"));
			convert_command_glyph(fbuff);
			item_append(fbuff, c_year::ui[custfltr::year[x]], arrow_flags, (void *)(uintptr_t)(YEAR_FILTER + x));
		}
	}

	item_append(menu_item_type::SEPARATOR);

	if (custfltr::numother > 0)
		item_append(_("Remove last filter"), "", 0, (void *)(uintptr_t)REMOVE_FILTER);

	if (custfltr::numother < MAX_CUST_FILTER - 2)
		item_append(_("Add filter"), "", 0, (void *)(uintptr_t)ADD_FILTER);

	item_append(menu_item_type::SEPARATOR);
	customtop = ui().get_line_height() + 3.0f * UI_BOX_TB_BORDER;
}

//-------------------------------------------------
//  perform our special rendering
//-------------------------------------------------
void menu_custom_filter::custom_render(void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
	char const *const text[] = { _("Select custom filters:") };
	draw_text_box(
			std::begin(text), std::end(text),
			origx1, origx2, origy1 - top, origy1 - UI_BOX_TB_BORDER,
			ui::text_layout::CENTER, ui::text_layout::NEVER, false,
			UI_TEXT_COLOR, UI_GREEN_COLOR, 1.0f);
}

//-------------------------------------------------
//  save custom filters info to file
//-------------------------------------------------

void menu_custom_filter::save_custom_filters()
{
	// attempt to open the output file
	emu_file file(ui().options().ui_path(), OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS);
	if (file.open("custom_", emulator_info::get_configname(), "_filter.ini") == osd_file::error::NONE)
	{
		// generate custom filters info
		std::ostringstream cinfo;
		util::stream_format(cinfo, "Total filters = %d\n", (custfltr::numother + 1));
		util::stream_format(cinfo, "Main filter = %s\n", machine_filter::config_name(custfltr::main));

		for (int x = 1; x <= custfltr::numother; x++)
		{
			util::stream_format(cinfo, "Other filter = %s\n", machine_filter::config_name(custfltr::other[x]));
			if (custfltr::other[x] == machine_filter::MANUFACTURER)
				util::stream_format(cinfo, "  Manufacturer filter = %s\n", c_mnfct::ui[custfltr::mnfct[x]]);
			else if (custfltr::other[x] == machine_filter::YEAR)
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
menu_swcustom_filter::menu_swcustom_filter(mame_ui_manager &mui, render_container &container, const game_driver *_driver, s_filter &_filter)
	: menu(mui, container)
	, m_added(false)
	, m_filter(_filter)
	, m_driver(_driver)
{
}

menu_swcustom_filter::~menu_swcustom_filter()
{
	reset_topmost(reset_options::SELECT_FIRST);
	save_sw_custom_filters();
}

//-------------------------------------------------
//  handle
//-------------------------------------------------
void menu_swcustom_filter::handle()
{
	bool changed = false;
	m_added = false;

	// process the menu
	const event *menu_event = process(PROCESS_LR_REPEAT);
	if (menu_event != nullptr && menu_event->itemref != nullptr)
	{
		switch ((uintptr_t)menu_event->itemref)
		{
			case MAIN_FILTER:
				if (menu_event->iptkey == IPT_UI_LEFT || menu_event->iptkey == IPT_UI_RIGHT)
				{
					(menu_event->iptkey == IPT_UI_RIGHT) ? sw_custfltr::main++ : sw_custfltr::main--;
					changed = true;
				}
				break;

			case ADD_FILTER:
				if (menu_event->iptkey == IPT_UI_SELECT)
				{
					sw_custfltr::numother++;
					sw_custfltr::other[sw_custfltr::numother] = software_filter::UNAVAILABLE;
					++sw_custfltr::other[sw_custfltr::numother];
					m_added = true;
				}
				break;

			case REMOVE_FILTER:
				if (menu_event->iptkey == IPT_UI_SELECT)
				{
					sw_custfltr::other[sw_custfltr::numother] = software_filter::UNAVAILABLE;
					++sw_custfltr::other[sw_custfltr::numother];
					sw_custfltr::numother--;
					changed = true;
				}
				break;
		}

		if ((uintptr_t)menu_event->itemref >= OTHER_FILTER && (uintptr_t)menu_event->itemref < OTHER_FILTER + MAX_CUST_FILTER)
		{
			int pos = (int)((uintptr_t)menu_event->itemref - OTHER_FILTER);
			if (menu_event->iptkey == IPT_UI_LEFT && sw_custfltr::other[pos] > software_filter::UNAVAILABLE + 1)
			{
				sw_custfltr::other[pos]--;
				changed = true;
			}
			else if (menu_event->iptkey == IPT_UI_RIGHT && sw_custfltr::other[pos] < software_filter::LAST - 1)
			{
				sw_custfltr::other[pos]++;
				changed = true;
			}
			else if (menu_event->iptkey == IPT_UI_SELECT)
			{
				std::vector<software_filter::type> types;
				std::vector<std::string> names;
				types.reserve(software_filter::COUNT);
				names.reserve(software_filter::COUNT);
				uint16_t sel(0);
				for (software_filter::type index = software_filter::FIRST; index < software_filter::COUNT; ++index)
				{
					if ((index >= software_filter::UNAVAILABLE) && (index != software_filter::CUSTOM))
					{
						if (sw_custfltr::other[pos] == index)
							sel = types.size();
						types.emplace_back(index);
						names.emplace_back(software_filter::display_name(index));
					}
				}
				menu::stack_push<menu_selector>(ui(), container(), std::move(names), sel, [this, pos, t = std::move(types)] (int selection) { sw_custfltr::other[pos] = t[selection]; reset(reset_options::REMEMBER_REF); });
			}
		}
		else if ((uintptr_t)menu_event->itemref >= YEAR_FILTER && (uintptr_t)menu_event->itemref < YEAR_FILTER + MAX_CUST_FILTER)
		{
			int pos = (int)((uintptr_t)menu_event->itemref - YEAR_FILTER);
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
			{
				menu::stack_push<menu_selector>(ui(), container(), std::vector<std::string>(m_filter.year.ui), sw_custfltr::year[pos], [this, pos] (int selection) { sw_custfltr::year[pos] = selection; reset(reset_options::REMEMBER_REF); });
			}
		}
		else if ((uintptr_t)menu_event->itemref >= TYPE_FILTER && (uintptr_t)menu_event->itemref < TYPE_FILTER + MAX_CUST_FILTER)
		{
			int pos = (int)((uintptr_t)menu_event->itemref - TYPE_FILTER);
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
			{
				menu::stack_push<menu_selector>(ui(), container(), std::vector<std::string>(m_filter.type.ui), sw_custfltr::type[pos], [this, pos] (int selection) { sw_custfltr::type[pos] = selection; reset(reset_options::REMEMBER_REF); });
			}
		}
		else if ((uintptr_t)menu_event->itemref >= MNFCT_FILTER && (uintptr_t)menu_event->itemref < MNFCT_FILTER + MAX_CUST_FILTER)
		{
			int pos = (int)((uintptr_t)menu_event->itemref - MNFCT_FILTER);
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
			{
				menu::stack_push<menu_selector>(ui(), container(), std::vector<std::string>(m_filter.publisher.ui), sw_custfltr::mnfct[pos], [this, pos] (int selection) { sw_custfltr::mnfct[pos] = selection; reset(reset_options::REMEMBER_REF); });
			}
		}
		else if ((uintptr_t)menu_event->itemref >= REGION_FILTER && (uintptr_t)menu_event->itemref < REGION_FILTER + MAX_CUST_FILTER)
		{
			int pos = (int)((uintptr_t)menu_event->itemref - REGION_FILTER);
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
			{
				menu::stack_push<menu_selector>(ui(), container(), std::vector<std::string>(m_filter.region.ui), sw_custfltr::region[pos], [this, pos] (int selection) { sw_custfltr::region[pos] = selection; reset(reset_options::REMEMBER_REF); });
			}
		}
		else if ((uintptr_t)menu_event->itemref >= LIST_FILTER && (uintptr_t)menu_event->itemref < LIST_FILTER + MAX_CUST_FILTER)
		{
			int pos = (int)((uintptr_t)menu_event->itemref - LIST_FILTER);
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
			{
				menu::stack_push<menu_selector>(ui(), container(), std::vector<std::string>(m_filter.swlist.description), sw_custfltr::list[pos], [this, pos] (int selection) { sw_custfltr::list[pos] = selection; reset(reset_options::REMEMBER_REF); });
			}
		}
	}

	if (changed)
		reset(reset_options::REMEMBER_REF);
	else if (m_added)
		reset(reset_options::SELECT_FIRST);
}

//-------------------------------------------------
//  populate
//-------------------------------------------------
void menu_swcustom_filter::populate(float &customtop, float &custombottom)
{
	// add main filter
	uint32_t arrow_flags = get_arrow_flags<uint16_t>(software_filter::ALL, software_filter::UNAVAILABLE, sw_custfltr::main);
	item_append(_("Main filter"), software_filter::display_name(sw_custfltr::main), arrow_flags, (void *)(uintptr_t)MAIN_FILTER);

	// add other filters
	for (int x = 1; x <= sw_custfltr::numother; x++)
	{
		item_append(menu_item_type::SEPARATOR);

		// add filter items
		arrow_flags = get_arrow_flags<uint16_t>(software_filter::UNAVAILABLE + 1, software_filter::LAST - 1, sw_custfltr::other[x]);
		item_append(_("Other filter"), software_filter::display_name(sw_custfltr::other[x]), arrow_flags, (void *)(uintptr_t)(OTHER_FILTER + x));

		if (m_added)
			selected = item.size() - 2;

		// add publisher subitem
		if (sw_custfltr::other[x] == software_filter::PUBLISHERS && m_filter.publisher.ui.size())
		{
			arrow_flags = get_arrow_flags<uint16_t>(0, m_filter.publisher.ui.size() - 1, sw_custfltr::mnfct[x]);
			std::string fbuff(_("^!Publisher"));
			convert_command_glyph(fbuff);
			item_append(fbuff, m_filter.publisher.ui[sw_custfltr::mnfct[x]], arrow_flags, (void *)(uintptr_t)(MNFCT_FILTER + x));
		}

		// add year subitem
		else if (sw_custfltr::other[x] == software_filter::YEARS && m_filter.year.ui.size())
		{
			arrow_flags = get_arrow_flags<uint16_t>(0, m_filter.year.ui.size() - 1, sw_custfltr::year[x]);
			std::string fbuff(_("^!Year"));
			convert_command_glyph(fbuff);
			item_append(fbuff, m_filter.year.ui[sw_custfltr::year[x]], arrow_flags, (void *)(uintptr_t)(YEAR_FILTER + x));
		}

		// add year subitem
		else if (sw_custfltr::other[x] == software_filter::LIST && m_filter.swlist.name.size())
		{
			arrow_flags = get_arrow_flags<uint16_t>(0, m_filter.swlist.name.size() - 1, sw_custfltr::list[x]);
			std::string fbuff(_("^!Software List"));
			convert_command_glyph(fbuff);
			item_append(fbuff, m_filter.swlist.description[sw_custfltr::list[x]], arrow_flags, (void *)(uintptr_t)(LIST_FILTER + x));
		}

		// add device type subitem
		else if (sw_custfltr::other[x] == software_filter::DEVICE_TYPE && m_filter.type.ui.size())
		{
			arrow_flags = get_arrow_flags<uint16_t>(0, m_filter.type.ui.size() - 1, sw_custfltr::type[x]);
			std::string fbuff(_("^!Device type"));
			convert_command_glyph(fbuff);
			item_append(fbuff, m_filter.type.ui[sw_custfltr::type[x]], arrow_flags, (void *)(uintptr_t)(TYPE_FILTER + x));
		}

		// add region subitem
		else if (sw_custfltr::other[x] == software_filter::REGION && m_filter.region.ui.size())
		{
			arrow_flags = get_arrow_flags<uint16_t>(0, m_filter.region.ui.size() - 1, sw_custfltr::region[x]);
			std::string fbuff(_("^!Region"));
			convert_command_glyph(fbuff);
			item_append(fbuff, m_filter.region.ui[sw_custfltr::region[x]], arrow_flags, (void *)(uintptr_t)(REGION_FILTER + x));
		}
	}

	item_append(menu_item_type::SEPARATOR);

	if (sw_custfltr::numother > 0)
		item_append(_("Remove last filter"), "", 0, (void *)(uintptr_t)REMOVE_FILTER);

	if (sw_custfltr::numother < MAX_CUST_FILTER - 2)
		item_append(_("Add filter"), "", 0, (void *)(uintptr_t)ADD_FILTER);

	item_append(menu_item_type::SEPARATOR);

	customtop = ui().get_line_height() + 3.0f * UI_BOX_TB_BORDER;
}

//-------------------------------------------------
//  perform our special rendering
//-------------------------------------------------
void menu_swcustom_filter::custom_render(void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
	char const *const text[] = { _("Select custom filters:") };
	draw_text_box(
			std::begin(text), std::end(text),
			origx1, origx2, origy1 - top, origy1 - UI_BOX_TB_BORDER,
			ui::text_layout::CENTER, ui::text_layout::NEVER, false,
			UI_TEXT_COLOR, UI_GREEN_COLOR, 1.0f);
}

//-------------------------------------------------
//  save custom filters info to file
//-------------------------------------------------

void menu_swcustom_filter::save_sw_custom_filters()
{
	// attempt to open the output file
	emu_file file(ui().options().ui_path(), OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS);
	if (file.open("custom_", m_driver->name, "_filter.ini") == osd_file::error::NONE)
	{
		// generate custom filters info
		std::ostringstream cinfo;
		util::stream_format(cinfo, "Total filters = %d\n", (sw_custfltr::numother + 1));
		util::stream_format(cinfo, "Main filter = %s\n", software_filter::config_name(sw_custfltr::main));

		for (int x = 1; x <= sw_custfltr::numother; x++)
		{
			util::stream_format(cinfo, "Other filter = %s\n", software_filter::config_name(sw_custfltr::other[x]));
			if (sw_custfltr::other[x] == software_filter::PUBLISHERS)
				util::stream_format(cinfo, "  Manufacturer filter = %s\n", m_filter.publisher.ui[sw_custfltr::mnfct[x]]);
			else if (sw_custfltr::other[x] == software_filter::LIST)
				util::stream_format(cinfo, "  Software List filter = %s\n", m_filter.swlist.name[sw_custfltr::list[x]]);
			else if (sw_custfltr::other[x] == software_filter::YEARS)
				util::stream_format(cinfo, "  Year filter = %s\n", m_filter.year.ui[sw_custfltr::year[x]]);
			else if (sw_custfltr::other[x] == software_filter::DEVICE_TYPE)
				util::stream_format(cinfo, "  Type filter = %s\n", m_filter.type.ui[sw_custfltr::type[x]]);
			else if (sw_custfltr::other[x] == software_filter::REGION)
				util::stream_format(cinfo, "  Region filter = %s\n", m_filter.region.ui[sw_custfltr::region[x]]);
		}
		file.puts(cinfo.str().c_str());
		file.close();
	}
}

} // namespace ui
