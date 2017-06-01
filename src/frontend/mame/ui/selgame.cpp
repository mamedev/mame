// license:BSD-3-Clause
// copyright-holders:Maurizio Petrarota
/*********************************************************************

    ui/selgame.cpp

    Main UI menu.

*********************************************************************/

#include "emu.h"

#include "ui/selgame.h"

#include "ui/ui.h"
#include "ui/miscmenu.h"
#include "ui/inifile.h"
#include "ui/datmenu.h"
#include "ui/optsmenu.h"
#include "ui/selector.h"
#include "ui/selsoft.h"
#include "ui/custmenu.h"
#include "ui/auditmenu.h"

#include "../info.h"

#include "audit.h"
#include "drivenum.h"
#include "emuopts.h"
#include "mame.h"
#include "rendfont.h"
#include "rendutil.h"
#include "softlist_dev.h"
#include "uiinput.h"
#include "luaengine.h"
extern const char UI_VERSION_TAG[];

namespace ui {
bool menu_select_game::first_start = true;
std::vector<const game_driver *> menu_select_game::m_sortedlist;
int menu_select_game::m_isabios = 0;

//-------------------------------------------------
//  ctor
//-------------------------------------------------

menu_select_game::menu_select_game(mame_ui_manager &mui, render_container &container, const char *gamename)
	: menu_select_launch(mui, container, false)
{
	highlight = 0;
	std::string error_string, last_filter, sub_filter;
	ui_options &moptions = mui.options();

	// load drivers cache
	init_sorted_list();

	// check if there are available icons
	ui_globals::has_icons = false;
	file_enumerator path(moptions.icons_directory());
	const osd::directory::entry *dir;
	while ((dir = path.next()) != nullptr)
	{
		std::string src(dir->name);
		if (src.find(".ico") != std::string::npos || src.find("icons") != std::string::npos)
		{
			ui_globals::has_icons = true;
			break;
		}
	}

	// build drivers list
	if (!load_available_machines())
		build_available_list();

	// load custom filter
	load_custom_filters();

	if (first_start)
	{
		reselect_last::driver = moptions.last_used_machine();
		std::string tmp(moptions.last_used_filter());
		std::size_t found = tmp.find_first_of(",");
		if (found == std::string::npos)
			last_filter = tmp;
		else
		{
			last_filter = tmp.substr(0, found);
			sub_filter = tmp.substr(found + 1);
		}

		main_filters::actual = FILTER_ALL;
		for (size_t ind = 0; ind < main_filters::length; ++ind)
			if (last_filter == main_filters::text[ind])
			{
				main_filters::actual = ind;
				break;
			}

		if (main_filters::actual == FILTER_CATEGORY)
			main_filters::actual = FILTER_ALL;
		else if (main_filters::actual == FILTER_MANUFACTURER)
		{
			for (size_t id = 0; id < c_mnfct::ui.size(); ++id)
				if (sub_filter == c_mnfct::ui[id])
					c_mnfct::actual = id;
		}
		else if (main_filters::actual == FILTER_YEAR)
		{
			for (size_t id = 0; id < c_year::ui.size(); ++id)
				if (sub_filter == c_year::ui[id])
					c_year::actual = id;
		}
		first_start = false;
	}

	if (!moptions.remember_last())
		reselect_last::reset();

	mui.machine().options().set_value(OPTION_SNAPNAME, "%g/%i", OPTION_PRIORITY_CMDLINE, error_string);
	mui.machine().options().set_value(OPTION_SOFTWARENAME, "", OPTION_PRIORITY_CMDLINE, error_string);

	ui_globals::curimage_view = FIRST_VIEW;
	ui_globals::curdats_view = 0;
	ui_globals::switch_image = false;
	ui_globals::default_image = true;
	ui_globals::panels_status = moptions.hide_panels();
	ui_globals::curdats_total = 1;
	m_searchlist[0] = nullptr;
}

//-------------------------------------------------
//  dtor
//-------------------------------------------------

menu_select_game::~menu_select_game()
{
	std::string error_string, last_driver;
	game_driver const *const driver(isfavorite() ? nullptr : reinterpret_cast<game_driver const *>(get_selection_ref()));
	ui_software_info *const swinfo(isfavorite() ? reinterpret_cast<ui_software_info *>(get_selection_ref()) : nullptr);

	if (reinterpret_cast<uintptr_t>(driver) > skip_main_items)
		last_driver = driver->name;
	else if (driver && m_prev_selected)
		last_driver = reinterpret_cast<game_driver const *>(m_prev_selected)->name;

	if (reinterpret_cast<uintptr_t>(swinfo) > skip_main_items)
		last_driver = swinfo->shortname;
	else if (swinfo && m_prev_selected)
		last_driver = reinterpret_cast<ui_software_info *>(m_prev_selected)->shortname;

	std::string filter(main_filters::text[main_filters::actual]);
	if (main_filters::actual == FILTER_MANUFACTURER)
		filter.append(",").append(c_mnfct::ui[c_mnfct::actual]);
	else if (main_filters::actual == FILTER_YEAR)
		filter.append(",").append(c_year::ui[c_year::actual]);

	ui_options &mopt = ui().options();
	mopt.set_value(OPTION_LAST_USED_FILTER, filter.c_str(), OPTION_PRIORITY_CMDLINE, error_string);
	mopt.set_value(OPTION_LAST_USED_MACHINE, last_driver.c_str(), OPTION_PRIORITY_CMDLINE, error_string);
	mopt.set_value(OPTION_HIDE_PANELS, ui_globals::panels_status, OPTION_PRIORITY_CMDLINE, error_string);
	ui().save_ui_options();
}

//-------------------------------------------------
//  handle
//-------------------------------------------------

void menu_select_game::handle()
{
	if (!m_prev_selected)
		m_prev_selected = item[0].ref;

	bool check_filter = false;

	// if i have to load datfile, performe an hard reset
	if (ui_globals::reset)
	{
		ui_globals::reset = false;
		machine().schedule_hard_reset();
		stack_reset();
		return;
	}

	// if i have to reselect a software, force software list submenu
	if (reselect_last::get())
	{
		const game_driver *const driver = reinterpret_cast<const game_driver *>(get_selection_ref());
		menu::stack_push<menu_select_software>(ui(), container(), driver);
		return;
	}

	// ignore pause keys by swallowing them before we process the menu
	machine().ui_input().pressed(IPT_UI_PAUSE);

	// process the menu
	const event *menu_event = process(PROCESS_LR_REPEAT);
	if (menu_event && menu_event->itemref)
	{
		if (m_ui_error)
		{
			// reset the error on any future menu_event
			m_ui_error = false;
			machine().ui_input().reset();
		}
		else if (menu_event->iptkey == IPT_UI_SELECT)
		{
			// handle selections
			if (get_focus() == focused_menu::main)
			{
				if (isfavorite())
					inkey_select_favorite(menu_event);
				else
					inkey_select(menu_event);
			}
			else if (get_focus() == focused_menu::left)
			{
				l_hover = highlight;
				check_filter = true;
				m_prev_selected = nullptr;
			}
		}
		else if (menu_event->iptkey == IPT_CUSTOM)
		{
			// handle IPT_CUSTOM (mouse right click)
			if (!isfavorite())
			{
				menu::stack_push<menu_machine_configure>(
						ui(), container(),
						reinterpret_cast<const game_driver *>(m_prev_selected),
						menu_event->mouse.x0, menu_event->mouse.y0);
			}
			else
			{
				ui_software_info *sw = reinterpret_cast<ui_software_info *>(m_prev_selected);
				menu::stack_push<menu_machine_configure>(
						ui(), container(),
						(const game_driver *)sw->driver,
						menu_event->mouse.x0, menu_event->mouse.y0);
			}
		}
		else if (menu_event->iptkey == IPT_UI_LEFT)
		{
			// handle UI_LEFT

			if (ui_globals::rpanel == RP_IMAGES && ui_globals::curimage_view > FIRST_VIEW)
			{
				// Images
				ui_globals::curimage_view--;
				ui_globals::switch_image = true;
				ui_globals::default_image = false;
			}
			else if (ui_globals::rpanel == RP_INFOS)
			{
				// Infos
				if (!isfavorite())
				{
					const game_driver *drv = (const game_driver *)menu_event->itemref;
					if ((uintptr_t)drv > skip_main_items && ui_globals::curdats_view > 0)
					{
						ui_globals::curdats_view--;
						m_topline_datsview = 0;
					}
				}
				else
				{
					ui_software_info *drv = (ui_software_info *)menu_event->itemref;
					if (drv->startempty == 1 && ui_globals::curdats_view > 0)
					{
						ui_globals::curdats_view--;
						m_topline_datsview = 0;
					}
					else if ((uintptr_t)drv > skip_main_items && ui_globals::cur_sw_dats_view > 0)
					{
						ui_globals::cur_sw_dats_view--;
						m_topline_datsview = 0;
					}
				}
			}
		}
		else if (menu_event->iptkey == IPT_UI_RIGHT)
		{
			// handle UI_RIGHT
			if (ui_globals::rpanel == RP_IMAGES && ui_globals::curimage_view < LAST_VIEW)
			{
				// Images
				ui_globals::curimage_view++;
				ui_globals::switch_image = true;
				ui_globals::default_image = false;
			}
			else if (ui_globals::rpanel == RP_INFOS)
			{
				// Infos
				if (!isfavorite())
				{
					const game_driver *drv = (const game_driver *)menu_event->itemref;
					if ((uintptr_t)drv > skip_main_items && ui_globals::curdats_view < (ui_globals::curdats_total - 1))
					{
						ui_globals::curdats_view++;
						m_topline_datsview = 0;
					}
				}
				else
				{
					ui_software_info *drv = (ui_software_info *)menu_event->itemref;
					if (drv->startempty == 1 && ui_globals::curdats_view < (ui_globals::curdats_total - 1))
					{
						ui_globals::curdats_view++;
						m_topline_datsview = 0;
					}
					else if ((uintptr_t)drv > skip_main_items && ui_globals::cur_sw_dats_view < (ui_globals::cur_sw_dats_total - 1))
					{
						ui_globals::cur_sw_dats_view++;
						m_topline_datsview = 0;
					}
				}
			}
		}
		else if (menu_event->iptkey == IPT_UI_UP_FILTER && highlight > FILTER_FIRST)
		{
			// handle UI_UP_FILTER
			highlight--;
		}
		else if (menu_event->iptkey == IPT_UI_DOWN_FILTER && highlight < FILTER_LAST)
		{
			// handle UI_DOWN_FILTER
			highlight++;
		}
		else if (menu_event->iptkey == IPT_UI_LEFT_PANEL)
		{
			// handle UI_LEFT_PANEL
			ui_globals::rpanel = RP_IMAGES;
		}
		else if (menu_event->iptkey == IPT_UI_RIGHT_PANEL)
		{
			// handle UI_RIGHT_PANEL
			ui_globals::rpanel = RP_INFOS;
		}
		else if (menu_event->iptkey == IPT_UI_CANCEL && !m_search.empty())
		{
			// escape pressed with non-empty text clears the text
			m_search.clear();
			reset(reset_options::SELECT_FIRST);
		}
		else if (menu_event->iptkey == IPT_UI_DATS)
		{
			// handle UI_DATS
			if (!isfavorite())
			{
				const game_driver *driver = (const game_driver *)menu_event->itemref;
				if ((uintptr_t)driver > skip_main_items && mame_machine_manager::instance()->lua()->call_plugin_check<const char *>("data_list", driver->name, true))
					menu::stack_push<menu_dats_view>(ui(), container(), driver);
			}
			else
			{
				ui_software_info *ui_swinfo  = (ui_software_info *)menu_event->itemref;

				if ((uintptr_t)ui_swinfo > skip_main_items)
				{
					if (ui_swinfo->startempty == 1 && mame_machine_manager::instance()->lua()->call_plugin_check<const char *>("data_list", ui_swinfo->driver->name, true))
						menu::stack_push<menu_dats_view>(ui(), container(), ui_swinfo->driver);
					else if (mame_machine_manager::instance()->lua()->call_plugin_check<const char *>("data_list", std::string(ui_swinfo->shortname).append(1, ',').append(ui_swinfo->listname).c_str()) || !ui_swinfo->usage.empty())
							menu::stack_push<menu_dats_view>(ui(), container(), ui_swinfo);
				}
			}
		}
		else if (menu_event->iptkey == IPT_UI_FAVORITES)
		{
			// handle UI_FAVORITES
			if (!isfavorite())
			{
				const game_driver *driver = (const game_driver *)menu_event->itemref;
				if ((uintptr_t)driver > skip_main_items)
				{
					favorite_manager &mfav = mame_machine_manager::instance()->favorite();
					if (!mfav.isgame_favorite(driver))
					{
						mfav.add_favorite_game(driver);
						machine().popmessage(_("%s\n added to favorites list."), driver->type.fullname());
					}

					else
					{
						mfav.remove_favorite_game();
						machine().popmessage(_("%s\n removed from favorites list."), driver->type.fullname());
					}
				}
			}
			else
			{
				ui_software_info *swinfo = (ui_software_info *)menu_event->itemref;
				if ((uintptr_t)swinfo > skip_main_items)
				{
					machine().popmessage(_("%s\n removed from favorites list."), swinfo->longname.c_str());
					mame_machine_manager::instance()->favorite().remove_favorite_game(*swinfo);
					reset(reset_options::SELECT_FIRST);
				}
			}
		}
		else if (menu_event->iptkey == IPT_UI_EXPORT)
		{
			// handle UI_EXPORT
			inkey_export();
		}
		else if (menu_event->iptkey == IPT_UI_AUDIT_FAST && !m_unavailsortedlist.empty())
		{
			// handle UI_AUDIT_FAST
			menu::stack_push<menu_audit>(ui(), container(), m_availsortedlist, m_unavailsortedlist, 1);
		}
		else if (menu_event->iptkey == IPT_UI_AUDIT_ALL)
		{
			// handle UI_AUDIT_ALL
			menu::stack_push<menu_audit>(ui(), container(), m_availsortedlist, m_unavailsortedlist, 2);
		}
		else if (menu_event->iptkey == IPT_SPECIAL)
		{
			// typed characters append to the buffer
			inkey_special(menu_event);
		}
		else if (menu_event->iptkey == IPT_UI_CONFIGURE)
		{
			inkey_navigation();
		}
		else if (menu_event->iptkey == IPT_OTHER)
		{
			m_prev_selected = nullptr;
			check_filter = true;
			highlight = l_hover;
		}
	}

	if (menu_event && !menu_event->itemref)
	{
		if (menu_event->iptkey == IPT_SPECIAL)
		{
			inkey_special(menu_event);
		}
		else if (menu_event->iptkey == IPT_UI_CONFIGURE)
		{
			inkey_navigation();
		}
		else if (menu_event->iptkey == IPT_OTHER)
		{
			set_focus(focused_menu::left);
			m_prev_selected = nullptr;
			l_hover = highlight;
			check_filter = true;
		}
		else if (menu_event->iptkey == IPT_UI_UP_FILTER && highlight > FILTER_FIRST)
		{
			// handle UI_UP_FILTER
			highlight--;
		}
		else if (menu_event->iptkey == IPT_UI_DOWN_FILTER && highlight < FILTER_LAST)
		{
			// handle UI_DOWN_FILTER
			highlight++;
		}
	}

	// if we're in an error state, overlay an error message
	if (m_ui_error)
		ui().draw_text_box(container(), _("The selected machine is missing one or more required ROM or CHD images. "
				"Please select a different machine.\n\nPress any key to continue."), ui::text_layout::CENTER, 0.5f, 0.5f, UI_RED_COLOR);

	// handle filters selection from key shortcuts
	if (check_filter)
	{
		m_search.clear();
		if (l_hover == FILTER_CATEGORY)
		{
			main_filters::actual = l_hover;
			menu::stack_push<menu_game_options>(ui(), container());
		}
		else if (l_hover == FILTER_CUSTOM)
		{
			main_filters::actual = l_hover;
			menu::stack_push<menu_custom_filter>(ui(), container(), true);
		}
		else if (l_hover == FILTER_MANUFACTURER)
			menu::stack_push<menu_selector>(ui(), container(), c_mnfct::ui, c_mnfct::actual, menu_selector::GAME, l_hover);
		else if (l_hover == FILTER_YEAR)
			menu::stack_push<menu_selector>(ui(), container(), c_year::ui, c_year::actual, menu_selector::GAME, l_hover);
		else
		{
			if (l_hover >= FILTER_ALL)
				main_filters::actual = l_hover;
			reset(reset_options::SELECT_FIRST);
		}
	}
}

//-------------------------------------------------
//  populate
//-------------------------------------------------

void menu_select_game::populate(float &customtop, float &custombottom)
{
	ui_globals::redraw_icon = true;
	ui_globals::switch_image = true;
	int old_item_selected = -1;
	uint32_t flags_ui = FLAG_LEFT_ARROW | FLAG_RIGHT_ARROW;

	if (!isfavorite())
	{
		// if search is not empty, find approximate matches
		if (!m_search.empty())
			populate_search();
		else
		{
			// reset search string
			m_search.clear();
			m_displaylist.clear();

			// if filter is set on category, build category list
			switch (main_filters::actual)
			{
				case FILTER_CATEGORY:
					build_category();
					break;
				case FILTER_MANUFACTURER:
					build_list(c_mnfct::ui[c_mnfct::actual].c_str());
					break;
				case FILTER_YEAR:
					build_list(c_year::ui[c_year::actual].c_str());
					break;
				case FILTER_CUSTOM:
					build_custom();
					break;
				default:
					build_list();
					break;
			}

			// iterate over entries
			int curitem = 0;
			for (auto & elem : m_displaylist)
			{
				if (old_item_selected == -1 && elem->name == reselect_last::driver)
					old_item_selected = curitem;

				bool cloneof = strcmp(elem->parent, "0");
				if (cloneof)
				{
					int cx = driver_list::find(elem->parent);
					if (cx != -1 && ((driver_list::driver(cx).flags & MACHINE_IS_BIOS_ROOT) != 0))
						cloneof = false;
				}

				item_append(elem->type.fullname(), "", (cloneof) ? (flags_ui | FLAG_INVERT) : flags_ui, (void *)elem);
				curitem++;
			}
		}
	}
	else
	{
		// populate favorites list
		m_search.clear();
		int curitem = 0;

		// iterate over entries
		for (auto & favmap : mame_machine_manager::instance()->favorite().m_list)
		{
			auto flags = flags_ui | FLAG_UI_FAVORITE;
			if (favmap.second.startempty == 1)
			{
				if (old_item_selected == -1 && favmap.second.shortname == reselect_last::driver)
					old_item_selected = curitem;

				bool cloneof = strcmp(favmap.second.driver->parent, "0");
				if (cloneof)
				{
					int cx = driver_list::find(favmap.second.driver->parent);
					if (cx != -1 && ((driver_list::driver(cx).flags & MACHINE_IS_BIOS_ROOT) != 0))
						cloneof = false;
				}

				item_append(favmap.second.longname, "", (cloneof) ? (flags | FLAG_INVERT) : flags, (void *)&favmap.second);
			}
			else
			{
				if (old_item_selected == -1 && favmap.second.shortname == reselect_last::driver)
					old_item_selected = curitem;
				item_append(favmap.second.longname, favmap.second.devicetype,
							favmap.second.parentname.empty() ? flags : (FLAG_INVERT | flags), (void *)&favmap.second);
			}
			curitem++;
		}
	}

	item_append(menu_item_type::SEPARATOR, flags_ui);

	// add special items
	if (stack_has_special_main_menu())
	{
		item_append(_("Configure Options"), "", flags_ui, (void *)(uintptr_t)CONF_OPTS);
		item_append(_("Configure Machine"), "", flags_ui, (void *)(uintptr_t)CONF_MACHINE);
		skip_main_items = 2;
		if (machine().options().plugins())
		{
			item_append(_("Plugins"), "", flags_ui, (void *)(uintptr_t)CONF_PLUGINS);
			skip_main_items++;
		}
	}
	else
		skip_main_items = 0;

	// configure the custom rendering
	customtop = 3.0f * ui().get_line_height() + 5.0f * UI_BOX_TB_BORDER;
	custombottom = 5.0f * ui().get_line_height() + 3.0f * UI_BOX_TB_BORDER;

	// reselect prior game launched, if any
	if (old_item_selected != -1)
	{
		selected = old_item_selected;
		if (ui_globals::visible_main_lines == 0)
			top_line = (selected != 0) ? selected - 1 : 0;
		else
			top_line = selected - (ui_globals::visible_main_lines / 2);

		if (reselect_last::software.empty())
			reselect_last::reset();
	}
	else
		reselect_last::reset();
}

//-------------------------------------------------
//  build a list of available drivers
//-------------------------------------------------

void menu_select_game::build_available_list()
{
	int m_total = driver_list::total();
	std::vector<bool> m_included(m_total, false);

	// open a path to the ROMs and find them in the array
	file_enumerator path(machine().options().media_path());
	const osd::directory::entry *dir;

	// iterate while we get new objects
	while ((dir = path.next()) != nullptr)
	{
		char drivername[50];
		char *dst = drivername;
		const char *src;

		// build a name for it
		for (src = dir->name; *src != 0 && *src != '.' && dst < &drivername[ARRAY_LENGTH(drivername) - 1]; ++src)
			*dst++ = tolower((uint8_t) * src);

		*dst = 0;
		int drivnum = driver_list::find(drivername);
		if (drivnum != -1 && !m_included[drivnum])
		{
			m_availsortedlist.push_back(&driver_list::driver(drivnum));
			m_included[drivnum] = true;
		}
	}

	// now check and include NONE_NEEDED
	if (!ui().options().hide_romless())
	{
		for (int x = 0; x < m_total; ++x)
		{
			auto driver = &driver_list::driver(x);
			if (!m_included[x] && driver != &GAME_NAME(___empty))
			{
				auto entries = rom_build_entries(driver->rom);
				const rom_entry *rom = entries.data();
				bool noroms = true;

				// check NO-DUMP
				for (; !ROMENTRY_ISEND(rom) && noroms == true; ++rom)
					if (ROMENTRY_ISFILE(rom))
					{
						util::hash_collection hashes(ROM_GETHASHDATA(rom));
						if (!hashes.flag(util::hash_collection::FLAG_NO_DUMP) && !ROM_ISOPTIONAL(rom))
							noroms = false;
					}

				if (!noroms)
				{
					// check if clone == parent
					auto cx = driver_list::clone(*driver);
					if (cx != -1 && m_included[cx])
					{
						auto drv = &driver_list::driver(cx);
						if (driver->rom == drv->rom)
							noroms = true;

						// check if clone < parent
						if (!noroms)
						{
							noroms = true;
							for (; !ROMENTRY_ISEND(rom) && noroms == true; ++rom)
							{
								if (ROMENTRY_ISFILE(rom))
								{
									util::hash_collection hashes(ROM_GETHASHDATA(rom));
									if (hashes.flag(util::hash_collection::FLAG_NO_DUMP) || ROM_ISOPTIONAL(rom))
										continue;

									uint64_t lenght = ROM_GETLENGTH(rom);
									auto found = false;
									auto parent_entries = rom_build_entries(drv->rom);
									for (auto parentrom = parent_entries.data(); !ROMENTRY_ISEND(parentrom) && found == false; ++parentrom)
									{
										if (ROMENTRY_ISFILE(parentrom) && ROM_GETLENGTH(parentrom) == lenght)
										{
											util::hash_collection parenthashes(ROM_GETHASHDATA(parentrom));
											if (parenthashes.flag(util::hash_collection::FLAG_NO_DUMP) || ROM_ISOPTIONAL(parentrom))
												continue;

											if (hashes == parenthashes)
												found = true;
										}
									}
									noroms = found;
								}
							}
						}
					}
				}

				if (noroms)
				{
					m_availsortedlist.push_back(&driver_list::driver(x));
					m_included[x] = true;
				}
			}
		}
	}
	// sort
	std::stable_sort(m_availsortedlist.begin(), m_availsortedlist.end(), sorted_game_list);

	// now build the unavailable list
	for (int x = 0; x < m_total; ++x)
		if (!m_included[x] && &driver_list::driver(x) != &GAME_NAME(___empty))
			m_unavailsortedlist.push_back(&driver_list::driver(x));

	// sort
	std::stable_sort(m_unavailsortedlist.begin(), m_unavailsortedlist.end(), sorted_game_list);
}


//-------------------------------------------------
//  force the game select menu to be visible
//  and inescapable
//-------------------------------------------------

void menu_select_game::force_game_select(mame_ui_manager &mui, render_container &container)
{
	// reset the menu stack
	menu::stack_reset(mui.machine());

	// add the quit entry followed by the game select entry
	menu::stack_push_special_main<menu_quit_game>(mui, container);
	menu::stack_push<menu_select_game>(mui, container, nullptr);

	// force the menus on
	mui.show_menu();

	// make sure MAME is paused
	mui.machine().pause();
}

//-------------------------------------------------
//  handle select key event
//-------------------------------------------------

void menu_select_game::inkey_select(const event *menu_event)
{
	const game_driver *driver = (const game_driver *)menu_event->itemref;

	// special case for configure options
	if ((uintptr_t)driver == CONF_OPTS)
		menu::stack_push<menu_game_options>(ui(), container());

	// special case for configure machine
	else if (uintptr_t(driver) == CONF_MACHINE)
	{
		if (m_prev_selected)
			menu::stack_push<menu_machine_configure>(ui(), container(), reinterpret_cast<const game_driver *>(m_prev_selected));
		return;
	}

	// special case for configure plugins
	else if ((uintptr_t)driver == CONF_PLUGINS)
	{
		menu::stack_push<menu_plugins_configure>(ui(), container());
	}
	// anything else is a driver
	else
	{
		// audit the game first to see if we're going to work
		driver_enumerator enumerator(machine().options(), *driver);
		enumerator.next();
		media_auditor auditor(enumerator);
		media_auditor::summary summary = auditor.audit_media(AUDIT_VALIDATE_FAST);

		// if everything looks good, schedule the new driver
		if (summary == media_auditor::CORRECT || summary == media_auditor::BEST_AVAILABLE || summary == media_auditor::NONE_NEEDED)
		{
			if ((driver->flags & MACHINE_TYPE_ARCADE) == 0)
			{
				for (software_list_device &swlistdev : software_list_device_iterator(enumerator.config()->root_device()))
					if (!swlistdev.get_info().empty())
					{
						menu::stack_push<menu_select_software>(ui(), container(), driver);
						return;
					}
			}

			s_bios biosname;
			if (!ui().options().skip_bios_menu() && has_multiple_bios(driver, biosname))
				menu::stack_push<bios_selection>(ui(), container(), biosname, (void *)driver, false, false);
			else
			{
				reselect_last::driver = driver->name;
				reselect_last::software.clear();
				reselect_last::swlist.clear();
				mame_machine_manager::instance()->schedule_new_driver(*driver);
				machine().schedule_hard_reset();
				stack_reset();
			}
		}
		// otherwise, display an error
		else
		{
			reset(reset_options::REMEMBER_REF);
			m_ui_error = true;
		}
	}
}

//-------------------------------------------------
//  handle select key event for favorites menu
//-------------------------------------------------

void menu_select_game::inkey_select_favorite(const event *menu_event)
{
	ui_software_info *ui_swinfo = (ui_software_info *)menu_event->itemref;
	ui_options &mopt = ui().options();

	if ((uintptr_t)ui_swinfo == CONF_OPTS)
	{
		// special case for configure options
		menu::stack_push<menu_game_options>(ui(), container());
	}
	else if ((uintptr_t)ui_swinfo == CONF_MACHINE)
	{
		// special case for configure machine
		if (m_prev_selected)
		{
			ui_software_info *swinfo = reinterpret_cast<ui_software_info *>(m_prev_selected);
			menu::stack_push<menu_machine_configure>(ui(), container(), (const game_driver *)swinfo->driver);
		}
		return;
	}
	else if ((uintptr_t)ui_swinfo == CONF_PLUGINS)
	{
		// special case for configure plugins
		menu::stack_push<menu_plugins_configure>(ui(), container());
	}
	else if (ui_swinfo->startempty == 1)
	{
		// audit the game first to see if we're going to work
		driver_enumerator enumerator(machine().options(), *ui_swinfo->driver);
		enumerator.next();
		media_auditor auditor(enumerator);
		media_auditor::summary summary = auditor.audit_media(AUDIT_VALIDATE_FAST);

		if (summary == media_auditor::CORRECT || summary == media_auditor::BEST_AVAILABLE || summary == media_auditor::NONE_NEEDED)
		{
			if ((ui_swinfo->driver->flags & MACHINE_TYPE_ARCADE) == 0)
			{
				for (software_list_device &swlistdev : software_list_device_iterator(enumerator.config()->root_device()))
					if (!swlistdev.get_info().empty())
					{
						menu::stack_push<menu_select_software>(ui(), container(), ui_swinfo->driver);
						return;
					}
			}

			// if everything looks good, schedule the new driver
			s_bios biosname;
			if (!mopt.skip_bios_menu() && has_multiple_bios(ui_swinfo->driver, biosname))
				menu::stack_push<bios_selection>(ui(), container(), biosname, (void *)ui_swinfo->driver, false, false);
			else
			{
				reselect_last::driver = ui_swinfo->driver->name;
				reselect_last::software.clear();
				reselect_last::swlist.clear();
				reselect_last::set(true);
				mame_machine_manager::instance()->schedule_new_driver(*ui_swinfo->driver);
				machine().schedule_hard_reset();
				stack_reset();
			}
		}
		else
		{
			// otherwise, display an error
			reset(reset_options::REMEMBER_REF);
			m_ui_error = true;
		}
	}
	else
	{
		// first validate
		driver_enumerator drv(machine().options(), *ui_swinfo->driver);
		media_auditor auditor(drv);
		drv.next();
		software_list_device *swlist = software_list_device::find_by_name(*drv.config(), ui_swinfo->listname.c_str());
		const software_info *swinfo = swlist->find(ui_swinfo->shortname.c_str());

		media_auditor::summary summary = auditor.audit_software(swlist->list_name(), swinfo, AUDIT_VALIDATE_FAST);

		if (summary == media_auditor::CORRECT || summary == media_auditor::BEST_AVAILABLE || summary == media_auditor::NONE_NEEDED)
		{
			s_bios biosname;
			if (!mopt.skip_bios_menu() && has_multiple_bios(ui_swinfo->driver, biosname))
			{
				menu::stack_push<bios_selection>(ui(), container(), biosname, (void *)ui_swinfo, true, false);
				return;
			}
			else if (!mopt.skip_parts_menu() && swinfo->has_multiple_parts(ui_swinfo->interface.c_str()))
			{
				s_parts parts;
				for (const software_part &swpart : swinfo->parts())
				{
					if (swpart.matches_interface(ui_swinfo->interface.c_str()))
					{
						std::string menu_part_name(swpart.name());
						if (swpart.feature("part_id") != nullptr)
							menu_part_name.assign("(").append(swpart.feature("part_id")).append(")");
						parts.emplace(swpart.name(), menu_part_name);
					}
				}
				menu::stack_push<software_parts>(ui(), container(), parts, ui_swinfo);
				return;
			}

			std::string error_string;
			std::string string_list = string_format("%s:%s:%s:%s", ui_swinfo->listname, ui_swinfo->shortname, ui_swinfo->part, ui_swinfo->instance);
			mopt.set_value(OPTION_SOFTWARENAME, string_list.c_str(), OPTION_PRIORITY_CMDLINE, error_string);
			std::string snap_list = std::string(ui_swinfo->listname).append(PATH_SEPARATOR).append(ui_swinfo->shortname);
			mopt.set_value(OPTION_SNAPNAME, snap_list.c_str(), OPTION_PRIORITY_CMDLINE, error_string);
			reselect_last::driver = drv.driver().name;
			reselect_last::software = ui_swinfo->shortname;
			reselect_last::swlist = ui_swinfo->listname;
			mame_machine_manager::instance()->schedule_new_driver(drv.driver());
			machine().schedule_hard_reset();
			stack_reset();
		}

		// otherwise, display an error
		else
		{
			reset(reset_options::REMEMBER_POSITION);
			m_ui_error = true;
		}
	}
}

//-------------------------------------------------
//  returns if the search can be activated
//-------------------------------------------------

inline bool menu_select_game::isfavorite() const
{
	return (main_filters::actual == FILTER_FAVORITE);
}

//-------------------------------------------------
//  handle special key event
//-------------------------------------------------

void menu_select_game::inkey_special(const event *menu_event)
{
	if (!isfavorite())
	{
		if (input_character(m_search, menu_event->unichar, uchar_is_printable))
			reset(reset_options::SELECT_FIRST);
	}
}


//-------------------------------------------------
//  build list
//-------------------------------------------------

void menu_select_game::build_list(const char *filter_text, int filter, bool bioscheck, std::vector<const game_driver *> s_drivers)
{
	if (s_drivers.empty())
	{
		filter = main_filters::actual;
		if (filter == FILTER_AVAILABLE)
			s_drivers = m_availsortedlist;
		else if (filter == FILTER_UNAVAILABLE)
			s_drivers = m_unavailsortedlist;
		else
			s_drivers = m_sortedlist;
	}

	for (auto & s_driver : s_drivers)
	{
		if (!bioscheck && filter != FILTER_BIOS && (s_driver->flags & MACHINE_IS_BIOS_ROOT) != 0)
			continue;

		switch (filter)
		{
		case FILTER_ALL:
		case FILTER_AVAILABLE:
		case FILTER_UNAVAILABLE:
			m_displaylist.push_back(s_driver);
			break;

		case FILTER_WORKING:
			if (!(s_driver->flags & MACHINE_NOT_WORKING))
				m_displaylist.push_back(s_driver);
			break;

		case FILTER_NOT_MECHANICAL:
			if (!(s_driver->flags & MACHINE_MECHANICAL))
				m_displaylist.push_back(s_driver);
			break;

		case FILTER_BIOS:
			if (s_driver->flags & MACHINE_IS_BIOS_ROOT)
				m_displaylist.push_back(s_driver);
			break;

		case FILTER_PARENT:
		case FILTER_CLONES:
			{
				bool cloneof = strcmp(s_driver->parent, "0");
				if (cloneof)
				{
					auto cx = driver_list::find(s_driver->parent);
					if (cx != -1 && ((driver_list::driver(cx).flags & MACHINE_IS_BIOS_ROOT) != 0))
						cloneof = false;
				}

				if (filter == FILTER_CLONES && cloneof)
					m_displaylist.push_back(s_driver);
				else if (filter == FILTER_PARENT && !cloneof)
					m_displaylist.push_back(s_driver);
			}
			break;
		case FILTER_NOT_WORKING:
			if (s_driver->flags & MACHINE_NOT_WORKING)
				m_displaylist.push_back(s_driver);
			break;

		case FILTER_MECHANICAL:
			if (s_driver->flags & MACHINE_MECHANICAL)
				m_displaylist.push_back(s_driver);
			break;

		case FILTER_SAVE:
			if (s_driver->flags & MACHINE_SUPPORTS_SAVE)
				m_displaylist.push_back(s_driver);
			break;

		case FILTER_NOSAVE:
			if (!(s_driver->flags & MACHINE_SUPPORTS_SAVE))
				m_displaylist.push_back(s_driver);
			break;

		case FILTER_YEAR:
			if (!core_stricmp(filter_text, s_driver->year))
				m_displaylist.push_back(s_driver);
			break;

		case FILTER_VERTICAL:
			if (s_driver->flags & ORIENTATION_SWAP_XY)
				m_displaylist.push_back(s_driver);
			break;

		case FILTER_HORIZONTAL:
			if (!(s_driver->flags & ORIENTATION_SWAP_XY))
				m_displaylist.push_back(s_driver);
			break;

		case FILTER_MANUFACTURER:
			{
				std::string name = c_mnfct::getname(s_driver->manufacturer);
				if (!core_stricmp(filter_text, name.c_str()))
					m_displaylist.push_back(s_driver);
			}
			break;
		case FILTER_CHD:
			{
				auto entries = rom_build_entries(s_driver->rom);
				for (const rom_entry &rom : entries)
					if (ROMENTRY_ISREGION(&rom) && ROMREGION_ISDISKDATA(&rom))
					{
						m_displaylist.push_back(s_driver);
						break;
					}
				}
			break;
		case FILTER_NOCHD:
			{
				auto entries = rom_build_entries(s_driver->rom);
				bool found = false;
				for (const rom_entry &rom : entries)
					if (ROMENTRY_ISREGION(&rom) && ROMREGION_ISDISKDATA(&rom))
					{
						found = true;
						break;
					}
				if (!found)
					m_displaylist.push_back(s_driver);
			}
			break;
		}
	}
}

//-------------------------------------------------
//  build custom display list
//-------------------------------------------------

void menu_select_game::build_custom()
{
	std::vector<const game_driver *> s_drivers;
	bool bioscheck = false;

	if (custfltr::main == FILTER_AVAILABLE)
		s_drivers = m_availsortedlist;
	else if (custfltr::main == FILTER_UNAVAILABLE)
		s_drivers = m_unavailsortedlist;
	else
		s_drivers = m_sortedlist;

	for (auto & elem : s_drivers)
	{
		m_displaylist.push_back(elem);
	}

	for (int count = 1; count <= custfltr::numother; ++count)
	{
		int filter = custfltr::other[count];
		if (filter == FILTER_BIOS)
			bioscheck = true;
	}

	for (int count = 1; count <= custfltr::numother; ++count)
	{
		int filter = custfltr::other[count];
		s_drivers = m_displaylist;
		m_displaylist.clear();

		switch (filter)
		{
			case FILTER_YEAR:
				build_list(c_year::ui[custfltr::year[count]].c_str(), filter, bioscheck, s_drivers);
				break;
			case FILTER_MANUFACTURER:
				build_list(c_mnfct::ui[custfltr::mnfct[count]].c_str(), filter, bioscheck, s_drivers);
				break;
			default:
				build_list(nullptr, filter, bioscheck, s_drivers);
				break;
		}
	}
}

//-------------------------------------------------
//  build category list
//-------------------------------------------------

void menu_select_game::build_category()
{
	m_displaylist.clear();
	std::vector<int> temp_filter;
	mame_machine_manager::instance()->inifile().load_ini_category(temp_filter);

	for (auto actual : temp_filter)
		m_displaylist.push_back(&driver_list::driver(actual));

	std::stable_sort(m_displaylist.begin(), m_displaylist.end(), sorted_game_list);
}

//-------------------------------------------------
//  populate search list
//-------------------------------------------------

void menu_select_game::populate_search()
{
	// allocate memory to track the penalty value
	std::vector<int> penalty(VISIBLE_GAMES_IN_SEARCH, 9999);
	int index = 0;
	for (; index < m_displaylist.size(); ++index)
	{
		// pick the best match between driver name and description
		int curpenalty = fuzzy_substring(m_search, m_displaylist[index]->type.fullname());
		int tmp = fuzzy_substring(m_search, m_displaylist[index]->name);
		curpenalty = std::min(curpenalty, tmp);

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

			m_searchlist[matchnum] = m_displaylist[index];
			penalty[matchnum] = curpenalty;
		}
	}

	(index < VISIBLE_GAMES_IN_SEARCH) ? m_searchlist[index] = nullptr : m_searchlist[VISIBLE_GAMES_IN_SEARCH] = nullptr;
	uint32_t flags_ui = FLAG_LEFT_ARROW | FLAG_RIGHT_ARROW;
	for (int curitem = 0; m_searchlist[curitem]; ++curitem)
	{
		bool cloneof = strcmp(m_searchlist[curitem]->parent, "0");
		if (cloneof)
		{
			int cx = driver_list::find(m_searchlist[curitem]->parent);
			if (cx != -1 && ((driver_list::driver(cx).flags & MACHINE_IS_BIOS_ROOT) != 0))
				cloneof = false;
		}
		item_append(m_searchlist[curitem]->type.fullname(), "", (!cloneof) ? flags_ui : (FLAG_INVERT | flags_ui),
			(void *)m_searchlist[curitem]);
	}
}

//-------------------------------------------------
//  generate general info
//-------------------------------------------------

void menu_select_game::general_info(const game_driver *driver, std::string &buffer)
{
	std::ostringstream str;

	util::stream_format(str, _("Romset: %1$-.100s\n"), driver->name);
	util::stream_format(str, _("Year: %1$s\n"), driver->year);
	util::stream_format(str, _("Manufacturer: %1$-.100s\n"), driver->manufacturer);

	int cloneof = driver_list::non_bios_clone(*driver);
	if (cloneof != -1)
		util::stream_format(str, _("Driver is Clone of: %1$-.100s\n"), driver_list::driver(cloneof).type.fullname());
	else
		str << _("Driver is Parent:\n");

	if (driver->flags & MACHINE_NOT_WORKING)
		str << _("Overall: NOT WORKING\n");
	else if (driver->flags & MACHINE_UNEMULATED_PROTECTION)
		str << _("Overall: Unemulated Protection\n");
	else
		str << _("Overall: Working\n");

	if (driver->flags & MACHINE_IMPERFECT_COLORS)
		str << _("Graphics: Imperfect Colors\n");
	else if (driver->flags & MACHINE_WRONG_COLORS)
		str << ("Graphics: Wrong Colors\n");
	else if (driver->flags & MACHINE_IMPERFECT_GRAPHICS)
		str << _("Graphics: Imperfect\n");
	else
		str << _("Graphics: OK\n");

	if (driver->flags & MACHINE_NO_SOUND)
		str << _("Sound: Unimplemented\n");
	else if (driver->flags & MACHINE_IMPERFECT_SOUND)
		str << _("Sound: Imperfect\n");
	else
		str << _("Sound: OK\n");

	util::stream_format(str, _("Driver is Skeleton: %1$s\n"), ((driver->flags & MACHINE_IS_SKELETON) ? _("Yes") : _("No")));
	util::stream_format(str, _("Game is Mechanical: %1$s\n"), ((driver->flags & MACHINE_MECHANICAL) ? _("Yes") : _("No")));
	util::stream_format(str, _("Requires Artwork: %1$s\n"), ((driver->flags & MACHINE_REQUIRES_ARTWORK) ? _("Yes") : _("No")));
	util::stream_format(str, _("Requires Clickable Artwork: %1$s\n"), ((driver->flags & MACHINE_CLICKABLE_ARTWORK) ? _("Yes") : _("No")));
	util::stream_format(str, _("Support Cocktail: %1$s\n"), ((driver->flags & MACHINE_NO_COCKTAIL) ? _("Yes") : _("No")));
	util::stream_format(str, _("Driver is Bios: %1$s\n"), ((driver->flags & MACHINE_IS_BIOS_ROOT) ? _("Yes") : _("No")));
	util::stream_format(str, _("Support Save: %1$s\n"), ((driver->flags & MACHINE_SUPPORTS_SAVE) ? _("Yes") : _("No")));
	util::stream_format(str, _("Screen Orientation: %1$s\n"), ((driver->flags & ORIENTATION_SWAP_XY) ? _("Vertical") : _("Horizontal")));
	bool found = false;
	auto entries = rom_build_entries(driver->rom);
	for (const rom_entry &rom : entries)
		if (ROMENTRY_ISREGION(&rom) && ROMREGION_ISDISKDATA(&rom))
		{
			found = true;
			break;
		}
	util::stream_format(str, _("Requires CHD: %1$s\n"), found ? _("Yes") : _("No"));

	// audit the game first to see if we're going to work
	if (ui().options().info_audit())
	{
		driver_enumerator enumerator(machine().options(), *driver);
		enumerator.next();
		media_auditor auditor(enumerator);
		media_auditor::summary summary = auditor.audit_media(AUDIT_VALIDATE_FAST);
		media_auditor::summary summary_samples = auditor.audit_samples();

		// if everything looks good, schedule the new driver
		if (summary == media_auditor::CORRECT || summary == media_auditor::BEST_AVAILABLE || summary == media_auditor::NONE_NEEDED)
			str << _("Roms Audit Pass: OK\n");
		else
			str << _("Roms Audit Pass: BAD\n");

		if (summary_samples == media_auditor::NONE_NEEDED)
			str << _("Samples Audit Pass: None Needed\n");
		else if (summary_samples == media_auditor::CORRECT || summary_samples == media_auditor::BEST_AVAILABLE)
			str << _("Samples Audit Pass: OK\n");
		else
			str << _("Samples Audit Pass: BAD\n");
	}
	else
		str << _("Roms Audit Pass: Disabled\nSamples Audit Pass: Disabled\n");

	std::istringstream istr(str.str());
	std::string line;
	float spacewid = ui().get_char_width(0x200a);
	str.clear();
	str.seekp(0);
	str << "#jp\n";
	while(std::getline(istr, line))
	{
		int nspace = floor((0.35 - ui().get_string_width(line.c_str())) / spacewid);
		if(nspace < 5)
			nspace = 5;
		std::string newstr;
		newstr.reserve((nspace * 3) + line.length());
		newstr.append(line.substr(0, line.find(':')));
		for(int i = 0; i < nspace; i++)
			newstr.append("\xE2\x80\x8A");
		str << newstr.append(line.substr(line.find(':') + 1, line.npos)).append("\n");
	}
	buffer = str.str();
}

void menu_select_game::inkey_export()
{
	std::vector<game_driver const *> list;
	if (!m_search.empty())
	{
		for (int curitem = 0; m_searchlist[curitem]; ++curitem)
			list.push_back(m_searchlist[curitem]);
	}
	else
	{
		if (isfavorite())
		{
			// iterate over favorites
			for (auto & favmap : mame_machine_manager::instance()->favorite().m_list)
			{
				if (favmap.second.startempty == 1)
					list.push_back(favmap.second.driver);
				else
					return;
			}
		}
		else
		{
			list = m_displaylist;
		}
	}

	menu::stack_push<menu_export>(ui(), container(), std::move(list));
}

//-------------------------------------------------
//  save drivers infos to file
//-------------------------------------------------

void menu_select_game::init_sorted_list()
{
	if (!m_sortedlist.empty())
		return;

	// generate full list
	for (int x = 0; x < driver_list::total(); ++x)
	{
		const game_driver *driver = &driver_list::driver(x);
		if (driver == &GAME_NAME(___empty))
			continue;
		if (driver->flags & MACHINE_IS_BIOS_ROOT)
			m_isabios++;

		m_sortedlist.push_back(driver);
		c_mnfct::set(driver->manufacturer);
		c_year::set(driver->year);
	}

	for (auto & e : c_mnfct::uimap)
		c_mnfct::ui.emplace_back(e.first);

	// sort manufacturers - years and driver
	std::stable_sort(c_mnfct::ui.begin(), c_mnfct::ui.end());
	std::stable_sort(c_year::ui.begin(), c_year::ui.end());
	std::stable_sort(m_sortedlist.begin(), m_sortedlist.end(), sorted_game_list);
}

//-------------------------------------------------
//  load drivers infos from file
//-------------------------------------------------

bool menu_select_game::load_available_machines()
{
	// try to load available drivers from file
	emu_file file(ui().options().ui_path(), OPEN_FLAG_READ);
	if (file.open(emulator_info::get_configname(), "_avail.ini") != osd_file::error::NONE)
		return false;

	std::string readbuf;
	char rbuf[MAX_CHAR_INFO];
	file.gets(rbuf, MAX_CHAR_INFO);
	file.gets(rbuf, MAX_CHAR_INFO);
	readbuf = chartrimcarriage(rbuf);
	std::string a_rev = string_format("%s%s", UI_VERSION_TAG, bare_build_version);

	// version not matching ? exit
	if (a_rev != readbuf)
	{
		file.close();
		return false;
	}

	file.gets(rbuf, MAX_CHAR_INFO);
	file.gets(rbuf, MAX_CHAR_INFO);
	file.gets(rbuf, MAX_CHAR_INFO);
	auto avsize = atoi(rbuf);
	file.gets(rbuf, MAX_CHAR_INFO);
	auto unavsize = atoi(rbuf);

	// load available list
	for (int x = 0; x < avsize; ++x)
	{
		file.gets(rbuf, MAX_CHAR_INFO);
		int find = atoi(rbuf);
		m_availsortedlist.push_back(&driver_list::driver(find));
	}

	// load unavailable list
	for (int x = 0; x < unavsize; ++x)
	{
		file.gets(rbuf, MAX_CHAR_INFO);
		int find = atoi(rbuf);
		m_unavailsortedlist.push_back(&driver_list::driver(find));
	}
	file.close();
	return true;
}

//-------------------------------------------------
//  load custom filters info from file
//-------------------------------------------------

void menu_select_game::load_custom_filters()
{
	// attempt to open the output file
	emu_file file(ui().options().ui_path(), OPEN_FLAG_READ);
	if (file.open("custom_", emulator_info::get_configname(), "_filter.ini") == osd_file::error::NONE)
	{
		char buffer[MAX_CHAR_INFO];

		// get number of filters
		file.gets(buffer, MAX_CHAR_INFO);
		char *pb = strchr(buffer, '=');
		custfltr::numother = atoi(++pb) - 1;

		// get main filter
		file.gets(buffer, MAX_CHAR_INFO);
		pb = strchr(buffer, '=') + 2;

		for (int y = 0; y < main_filters::length; ++y)
			if (!strncmp(pb, main_filters::text[y], strlen(main_filters::text[y])))
			{
				custfltr::main = y;
				break;
			}

		for (int x = 1; x <= custfltr::numother; ++x)
		{
			file.gets(buffer, MAX_CHAR_INFO);
			char *cb = strchr(buffer, '=') + 2;
			for (int y = 0; y < main_filters::length; ++y)
				if (!strncmp(cb, main_filters::text[y], strlen(main_filters::text[y])))
				{
					custfltr::other[x] = y;
					if (y == FILTER_MANUFACTURER)
					{
						file.gets(buffer, MAX_CHAR_INFO);
						char *ab = strchr(buffer, '=') + 2;
						for (size_t z = 0; z < c_mnfct::ui.size(); ++z)
							if (!strncmp(ab, c_mnfct::ui[z].c_str(), c_mnfct::ui[z].length()))
								custfltr::mnfct[x] = z;
					}
					else if (y == FILTER_YEAR)
					{
						file.gets(buffer, MAX_CHAR_INFO);
						char *db = strchr(buffer, '=') + 2;
						for (size_t z = 0; z < c_year::ui.size(); ++z)
							if (!strncmp(db, c_year::ui[z].c_str(), c_year::ui[z].length()))
								custfltr::year[x] = z;
					}
				}
		}
		file.close();
	}

}


//-------------------------------------------------
//  draw left box
//-------------------------------------------------

float menu_select_game::draw_left_panel(float x1, float y1, float x2, float y2)
{
	float line_height = ui().get_line_height();

	if (ui_globals::panels_status == SHOW_PANELS || ui_globals::panels_status == HIDE_RIGHT_PANEL)
	{
		float origy1 = y1;
		float origy2 = y2;
		float text_size = ui().options().infos_size();
		float line_height_max = line_height * text_size;
		float left_width = 0.0f;
		int text_lenght = main_filters::length;
		int afilter = main_filters::actual;
		int phover = HOVER_FILTER_FIRST;
		const char **text = main_filters::text;
		float sc = y2 - y1 - (2.0f * UI_BOX_TB_BORDER);

		if ((text_lenght * line_height_max) > sc)
		{
			float lm = sc / (text_lenght);
			text_size = lm / line_height;
			line_height_max = line_height * text_size;
		}

		float text_sign = ui().get_string_width("_# ", text_size);
		for (int x = 0; x < text_lenght; ++x)
		{
			float total_width;

			// compute width of left hand side
			total_width = ui().get_string_width(text[x], text_size);
			total_width += text_sign;

			// track the maximum
			if (total_width > left_width)
				left_width = total_width;
		}

		x2 = x1 + left_width + 2.0f * UI_BOX_LR_BORDER;
		ui().draw_outlined_box(container(), x1, y1, x2, y2, UI_BACKGROUND_COLOR);

		// take off the borders
		x1 += UI_BOX_LR_BORDER;
		x2 -= UI_BOX_LR_BORDER;
		y1 += UI_BOX_TB_BORDER;
		y2 -= UI_BOX_TB_BORDER;

		for (int filter = 0; filter < text_lenght; ++filter)
		{
			std::string str(text[filter]);
			rgb_t bgcolor = UI_TEXT_BG_COLOR;
			rgb_t fgcolor = UI_TEXT_COLOR;

			if (mouse_in_rect(x1, y1, x2, y1 + line_height_max))
			{
				bgcolor = UI_MOUSEOVER_BG_COLOR;
				fgcolor = UI_MOUSEOVER_COLOR;
				hover = phover + filter;
				menu::highlight(x1, y1, x2, y1 + line_height_max, bgcolor);
			}

			if (highlight == filter && get_focus() == focused_menu::left)
			{
				fgcolor = rgb_t(0xff, 0xff, 0xff, 0x00);
				bgcolor = rgb_t(0xff, 0xff, 0xff, 0xff);
				ui().draw_textured_box(container(), x1, y1, x2, y1 + line_height_max, bgcolor, rgb_t(255, 43, 43, 43),
						hilight_main_texture(), PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA) | PRIMFLAG_TEXWRAP(1));
			}

			float x1t = x1 + text_sign;
			if (afilter == FILTER_CUSTOM)
			{
				if (filter == custfltr::main)
				{
					str.assign("@custom1 ").append(text[filter]);
					x1t -= text_sign;
				}
				else
				{
					for (int count = 1; count <= custfltr::numother; ++count)
					{
						int cfilter = custfltr::other[count];
						if (cfilter == filter)
						{
							str = string_format("@custom%d %s", count + 1, text[filter]);
							x1t -= text_sign;
							break;
						}
					}
				}
				convert_command_glyph(str);
			}
			else if (filter == main_filters::actual)
			{
				str.assign("_> ").append(text[filter]);
				x1t -= text_sign;
				convert_command_glyph(str);
			}

			ui().draw_text_full(container(), str.c_str(), x1t, y1, x2 - x1, ui::text_layout::LEFT, ui::text_layout::NEVER,
					mame_ui_manager::NORMAL, fgcolor, bgcolor, nullptr, nullptr, text_size);
			y1 += line_height_max;
		}

		x1 = x2 + UI_BOX_LR_BORDER;
		x2 = x1 + 2.0f * UI_BOX_LR_BORDER;
		y1 = origy1;
		y2 = origy2;
		float space = x2 - x1;
		float lr_arrow_width = 0.4f * space * machine().render().ui_aspect();
		rgb_t fgcolor = UI_TEXT_COLOR;

		// set left-right arrows dimension
		float ar_x0 = 0.5f * (x2 + x1) - 0.5f * lr_arrow_width;
		float ar_y0 = 0.5f * (y2 + y1) + 0.1f * space;
		float ar_x1 = ar_x0 + lr_arrow_width;
		float ar_y1 = 0.5f * (y2 + y1) + 0.9f * space;

		ui().draw_outlined_box(container(), x1, y1, x2, y2, rgb_t(0xEF, 0x12, 0x47, 0x7B));

		if (mouse_in_rect(x1, y1, x2, y2))
		{
			fgcolor = UI_MOUSEOVER_COLOR;
			hover = HOVER_LPANEL_ARROW;
		}

		draw_arrow(ar_x0, ar_y0, ar_x1, ar_y1, fgcolor, ROT90 ^ ORIENTATION_FLIP_X);
		return x2 + UI_BOX_LR_BORDER;
	}
	else
	{
		float space = x2 - x1;
		float lr_arrow_width = 0.4f * space * machine().render().ui_aspect();
		rgb_t fgcolor = UI_TEXT_COLOR;

		// set left-right arrows dimension
		float ar_x0 = 0.5f * (x2 + x1) - 0.5f * lr_arrow_width;
		float ar_y0 = 0.5f * (y2 + y1) + 0.1f * space;
		float ar_x1 = ar_x0 + lr_arrow_width;
		float ar_y1 = 0.5f * (y2 + y1) + 0.9f * space;

		ui().draw_outlined_box(container(), x1, y1, x2, y2, rgb_t(0xEF, 0x12, 0x47, 0x7B));

		if (mouse_in_rect(x1, y1, x2, y2))
		{
			fgcolor = UI_MOUSEOVER_COLOR;
			hover = HOVER_LPANEL_ARROW;
		}

		draw_arrow(ar_x0, ar_y0, ar_x1, ar_y1, fgcolor, ROT90);
		return x2 + UI_BOX_LR_BORDER;
	}
}



//-------------------------------------------------
//  get selected software and/or driver
//-------------------------------------------------

void menu_select_game::get_selection(ui_software_info const *&software, game_driver const *&driver) const
{
	if (item[0].flags & FLAG_UI_FAVORITE) // TODO: work out why this doesn't use isfavorite()
	{
		software = reinterpret_cast<ui_software_info const *>(get_selection_ptr());
		driver = software ? software->driver : nullptr;
	}
	else
	{
		software = nullptr;
		driver = reinterpret_cast<game_driver const *>(get_selection_ptr());
	}
}

void menu_select_game::make_topbox_text(std::string &line0, std::string &line1, std::string &line2) const
{
	inifile_manager &inifile = mame_machine_manager::instance()->inifile();

	line0 = string_format(_("%1$s %2$s ( %3$d / %4$d machines (%5$d BIOS) )"),
			emulator_info::get_appname(),
			bare_build_version,
			visible_items,
			(driver_list::total() - 1),
			m_isabios);

	std::string filtered;
	if (main_filters::actual == FILTER_CATEGORY && inifile.total() > 0)
	{
		filtered = string_format(_("%1$s (%2$s - %3$s) - "),
				main_filters::text[main_filters::actual],
				inifile.get_file(),
				inifile.get_category());
	}
	else if (main_filters::actual == FILTER_MANUFACTURER)
	{
		filtered = string_format(_("%1$s (%2$s) - "),
				main_filters::text[main_filters::actual],
				c_mnfct::ui[c_mnfct::actual]);
	}
	else if (main_filters::actual == FILTER_YEAR)
	{
		filtered = string_format(_("%1$s (%2$s) - "),
				main_filters::text[main_filters::actual],
				c_year::ui[c_year::actual]);
	}

	// display the current typeahead
	if (isfavorite())
		line1.clear();
	else
		line1 = string_format(_("%1$s Search: %2$s_"), filtered, m_search);

	line2.clear();
}


std::string menu_select_game::make_driver_description(game_driver const &driver) const
{
	// first line is game name
	return string_format(_("Romset: %1$-.100s"), driver.name);
}


std::string menu_select_game::make_software_description(ui_software_info const &software) const
{
	// first line is system
	return string_format(_("System: %1$-.100s"), software.driver->type.fullname());
}

} // namespace ui
