// license:BSD-3-Clause
// copyright-holders:Maurizio Petrarota
/*********************************************************************

    ui/selgame.cpp

    Main UI menu.

*********************************************************************/

#include "emu.h"
#include "emuopts.h"
#include "ui/ui.h"
#include "ui/menu.h"
#include "uiinput.h"
#include "ui/selgame.h"
#include "ui/miscmenu.h"
#include "audit.h"
#include "ui/datfile.h"
#include "ui/inifile.h"
#include "rendfont.h"
#include "ui/datmenu.h"
#include "ui/optsmenu.h"
#include "ui/selector.h"
#include "ui/selsoft.h"
#include "sound/samples.h"
#include "ui/custmenu.h"
#include "../info.h"
#include "ui/auditmenu.h"
#include "rendutil.h"
#include "softlist.h"
#include <algorithm>

extern const char UI_VERSION_TAG[];

static bool first_start = true;
static const char *dats_info[] = {
	__("General Info"),
	__("History"),
	__("Mameinfo"),
	__("Sysinfo"),
	__("Messinfo"),
	__("Command"),
	__("Gameinit"),
	__("Mamescore") };

std::vector<const game_driver *> ui_menu_select_game::m_sortedlist;
int ui_menu_select_game::m_isabios = 0;

//-------------------------------------------------
//  ctor
//-------------------------------------------------

ui_menu_select_game::ui_menu_select_game(running_machine &machine, render_container *container, const char *gamename) : ui_menu(machine, container)
{
	m_focus = focused_menu::main;
	highlight = 0;
	std::string error_string, last_filter, sub_filter;
	ui_options &moptions = machine.ui().options();

	// load drivers cache
	init_sorted_list();

	// check if there are available icons
	ui_globals::has_icons = false;
	file_enumerator path(moptions.icons_directory());
	const osd_directory_entry *dir;
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

	machine.options().set_value(OPTION_SNAPNAME, "%g/%i", OPTION_PRIORITY_CMDLINE, error_string);
	machine.options().set_value(OPTION_SOFTWARENAME, "", OPTION_PRIORITY_CMDLINE, error_string);

	ui_globals::curimage_view = FIRST_VIEW;
	ui_globals::curdats_view = UI_FIRST_LOAD;
	ui_globals::switch_image = false;
	ui_globals::default_image = true;
	ui_globals::panels_status = moptions.hide_panels();
	m_searchlist[0] = nullptr;
}

//-------------------------------------------------
//  dtor
//-------------------------------------------------

ui_menu_select_game::~ui_menu_select_game()
{
	std::string error_string, last_driver;
	const game_driver *driver = nullptr;
	ui_software_info *swinfo = nullptr;
	ui_options &mopt = machine().ui().options();
	if (isfavorite())
		swinfo = (selected >= 0 && selected < item.size()) ? (ui_software_info *)item[selected].ref : nullptr;
	else
		driver = (selected >= 0 && selected < item.size()) ? (const game_driver *)item[selected].ref : nullptr;

	if ((FPTR)driver > skip_main_items)
		last_driver = driver->name;

	if ((FPTR)swinfo > skip_main_items)
		last_driver = swinfo->shortname;

	std::string filter(main_filters::text[main_filters::actual]);
	if (main_filters::actual == FILTER_MANUFACTURER)
		filter.append(",").append(c_mnfct::ui[c_mnfct::actual]);
	else if (main_filters::actual == FILTER_YEAR)
		filter.append(",").append(c_year::ui[c_year::actual]);

	mopt.set_value(OPTION_LAST_USED_FILTER, filter.c_str(), OPTION_PRIORITY_CMDLINE, error_string);
	mopt.set_value(OPTION_LAST_USED_MACHINE, last_driver.c_str(), OPTION_PRIORITY_CMDLINE, error_string);
	mopt.set_value(OPTION_HIDE_PANELS, ui_globals::panels_status, OPTION_PRIORITY_CMDLINE, error_string);
	save_ui_options(machine());
}

//-------------------------------------------------
//  handle
//-------------------------------------------------

void ui_menu_select_game::handle()
{
	if (m_prev_selected == nullptr)
		m_prev_selected = item[0].ref;

	bool check_filter = false;
	bool enabled_dats = machine().ui().options().enabled_dats();

	// if i have to load datfile, performe an hard reset
	if (ui_globals::reset)
	{
		ui_globals::reset = false;
		machine().schedule_hard_reset();
		ui_menu::stack_reset(machine());
		return;
	}

	// if i have to reselect a software, force software list submenu
	if (reselect_last::get())
	{
		const game_driver *driver = (const game_driver *)item[selected].ref;
		ui_menu::stack_push(global_alloc_clear<ui_menu_select_software>(machine(), container, driver));
		return;
	}

	// ignore pause keys by swallowing them before we process the menu
	machine().ui_input().pressed(IPT_UI_PAUSE);

	// process the menu
	const ui_menu_event *m_event = process(UI_MENU_PROCESS_LR_REPEAT);
	if (m_event != nullptr && m_event->itemref != nullptr)
	{
		// reset the error on any future m_event
		if (ui_error)
		{
			ui_error = false;
			machine().ui_input().reset();
		}

		// handle selections
		else if (m_event->iptkey == IPT_UI_SELECT)
		{
			if (m_focus == focused_menu::main)
			{
				if (isfavorite())
					inkey_select_favorite(m_event);
				else
					inkey_select(m_event);
			}
			else if (m_focus == focused_menu::left)
			{
				l_hover = highlight;
				check_filter = true;
				m_prev_selected = nullptr;
			}
		}

		// handle UI_LEFT
		else if (m_event->iptkey == IPT_UI_LEFT)
		{
			// Images
			if (ui_globals::rpanel == RP_IMAGES && ui_globals::curimage_view > FIRST_VIEW)
			{
				ui_globals::curimage_view--;
				ui_globals::switch_image = true;
				ui_globals::default_image = false;
			}

			// Infos
			else if (ui_globals::rpanel == RP_INFOS)
			{
				if (!isfavorite())
				{
					const game_driver *drv = (const game_driver *)m_event->itemref;
					if ((FPTR)drv > skip_main_items && ui_globals::curdats_view > UI_FIRST_LOAD)
					{
						ui_globals::curdats_view--;
						topline_datsview = 0;
					}
				}
				else
				{
					ui_software_info *drv = (ui_software_info *)m_event->itemref;
					if (drv->startempty == 1 && ui_globals::curdats_view > UI_FIRST_LOAD)
					{
						ui_globals::curdats_view--;
						topline_datsview = 0;
					}
					else if ((FPTR)drv > skip_main_items && ui_globals::cur_sw_dats_view > 0)
					{
						ui_globals::cur_sw_dats_view--;
						topline_datsview = 0;
					}
				}
			}
		}

		// handle UI_RIGHT
		else if (m_event->iptkey == IPT_UI_RIGHT)
		{
			// Images
			if (ui_globals::rpanel == RP_IMAGES && ui_globals::curimage_view < LAST_VIEW)
			{
				ui_globals::curimage_view++;
				ui_globals::switch_image = true;
				ui_globals::default_image = false;
			}

			// Infos
			else if (ui_globals::rpanel == RP_INFOS)
			{
				if (!isfavorite())
				{
					const game_driver *drv = (const game_driver *)m_event->itemref;
					if ((FPTR)drv > skip_main_items && ui_globals::curdats_view < UI_LAST_LOAD)
					{
						ui_globals::curdats_view++;
						topline_datsview = 0;
					}
				}
				else
				{
					ui_software_info *drv = (ui_software_info *)m_event->itemref;
					if (drv->startempty == 1 && ui_globals::curdats_view < UI_LAST_LOAD)
					{
						ui_globals::curdats_view++;
						topline_datsview = 0;
					}
					else if ((FPTR)drv > skip_main_items && ui_globals::cur_sw_dats_view < 1)
					{
						ui_globals::cur_sw_dats_view++;
						topline_datsview = 0;
					}
				}
			}
		}

		// handle UI_UP_FILTER
		else if (m_event->iptkey == IPT_UI_UP_FILTER && highlight > FILTER_FIRST)
		{
			highlight--;
		}

		// handle UI_DOWN_FILTER
		else if (m_event->iptkey == IPT_UI_DOWN_FILTER && highlight < FILTER_LAST)
		{
			highlight++;
		}

		// handle UI_LEFT_PANEL
		else if (m_event->iptkey == IPT_UI_LEFT_PANEL)
			ui_globals::rpanel = RP_IMAGES;

		// handle UI_RIGHT_PANEL
		else if (m_event->iptkey == IPT_UI_RIGHT_PANEL)
			ui_globals::rpanel = RP_INFOS;

		// escape pressed with non-empty text clears the text
		else if (m_event->iptkey == IPT_UI_CANCEL && m_search[0] != 0)
		{
			m_search[0] = '\0';
			reset(UI_MENU_RESET_SELECT_FIRST);
		}

		// handle UI_DATS
		else if (m_event->iptkey == IPT_UI_DATS && enabled_dats)
		{
			if (!isfavorite())
			{
				const game_driver *driver = (const game_driver *)m_event->itemref;
				if ((FPTR)driver > skip_main_items && machine().datfile().has_data(driver))
					ui_menu::stack_push(global_alloc_clear<ui_menu_dats_view>(machine(), container, driver));
			}
			else
			{
				ui_software_info *ui_swinfo  = (ui_software_info *)m_event->itemref;
				datfile_manager &mdat = machine().datfile();

				if ((FPTR)ui_swinfo > skip_main_items)
				{
					if (ui_swinfo->startempty == 1 && mdat.has_history(ui_swinfo->driver))
						ui_menu::stack_push(global_alloc_clear<ui_menu_dats_view>(machine(), container, ui_swinfo->driver));
					else if (mdat.has_software(ui_swinfo->listname, ui_swinfo->shortname, ui_swinfo->parentname) || !ui_swinfo->usage.empty())
						ui_menu::stack_push(global_alloc_clear<ui_menu_dats_view>(machine(), container, ui_swinfo));
				}
			}
		}

		// handle UI_FAVORITES
		else if (m_event->iptkey == IPT_UI_FAVORITES)
		{
			if (!isfavorite())
			{
				const game_driver *driver = (const game_driver *)m_event->itemref;
				if ((FPTR)driver > skip_main_items)
				{
					if (!machine().favorite().isgame_favorite(driver))
					{
						machine().favorite().add_favorite_game(driver);
						machine().popmessage(_("%s\n added to favorites list."), driver->description);
					}

					else
					{
						machine().favorite().remove_favorite_game();
						machine().popmessage(_("%s\n removed from favorites list."), driver->description);
					}
				}
			}
			else
			{
				ui_software_info *swinfo = (ui_software_info *)m_event->itemref;
				if ((FPTR)swinfo > skip_main_items)
				{
					machine().popmessage(_("%s\n removed from favorites list."), swinfo->longname.c_str());
					machine().favorite().remove_favorite_game(*swinfo);
					reset(UI_MENU_RESET_SELECT_FIRST);
				}
			}
		}

		// handle UI_EXPORT
		else if (m_event->iptkey == IPT_UI_EXPORT && !isfavorite())
			inkey_export();

		// handle UI_AUDIT_FAST
		else if (m_event->iptkey == IPT_UI_AUDIT_FAST && !m_unavailsortedlist.empty())
			ui_menu::stack_push(global_alloc_clear<ui_menu_audit>(machine(), container, m_availsortedlist, m_unavailsortedlist, 1));

		// handle UI_AUDIT_ALL
		else if (m_event->iptkey == IPT_UI_AUDIT_ALL)
			ui_menu::stack_push(global_alloc_clear<ui_menu_audit>(machine(), container, m_availsortedlist, m_unavailsortedlist, 2));

		// typed characters append to the buffer
		else if (m_event->iptkey == IPT_SPECIAL)
			inkey_special(m_event);

		else if (m_event->iptkey == IPT_UI_CONFIGURE)
			inkey_configure(m_event);

		else if (m_event->iptkey == IPT_OTHER)
		{
			m_prev_selected = nullptr;
			check_filter = true;
			highlight = l_hover;
		}
	}

	if (m_event != nullptr && m_event->itemref == nullptr)
	{
		if (m_event->iptkey == IPT_SPECIAL)
			inkey_special(m_event);
		else if (m_event->iptkey == IPT_UI_CONFIGURE)
			inkey_configure(m_event);
		else if (m_event->iptkey == IPT_UI_SELECT && m_focus == focused_menu::left)
		{
			m_prev_selected = nullptr;
			l_hover = highlight;
			check_filter = true;
		}
		// handle UI_UP_FILTER
		else if (m_event->iptkey == IPT_UI_UP_FILTER && highlight > FILTER_FIRST)
		{
			highlight--;
		}

		// handle UI_DOWN_FILTER
		else if (m_event->iptkey == IPT_UI_DOWN_FILTER && highlight < FILTER_LAST)
		{
			highlight++;
		}
	}

	// if we're in an error state, overlay an error message
	if (ui_error)
		machine().ui().draw_text_box(container, _("The selected machine is missing one or more required ROM or CHD images. "
			"Please select a different machine.\n\nPress any key to continue."), JUSTIFY_CENTER, 0.5f, 0.5f, UI_RED_COLOR);

	// handle filters selection from key shortcuts
	if (check_filter)
	{
		m_search[0] = '\0';
		if (l_hover == FILTER_CATEGORY)
		{
			main_filters::actual = l_hover;
			ui_menu::stack_push(global_alloc_clear<ui_menu_game_options>(machine(), container));
		}
		else if (l_hover == FILTER_CUSTOM)
		{
			main_filters::actual = l_hover;
			ui_menu::stack_push(global_alloc_clear<ui_menu_custom_filter>(machine(), container, true));
		}
		else if (l_hover == FILTER_MANUFACTURER)
			ui_menu::stack_push(global_alloc_clear<ui_menu_selector>(machine(), container, c_mnfct::ui, c_mnfct::actual, SELECTOR_GAME, l_hover));
		else if (l_hover == FILTER_YEAR)
			ui_menu::stack_push(global_alloc_clear<ui_menu_selector>(machine(), container, c_year::ui, c_year::actual, SELECTOR_GAME, l_hover));
		else
		{
			if (l_hover >= FILTER_ALL)
				main_filters::actual = l_hover;
			reset(UI_MENU_RESET_SELECT_FIRST);
		}
	}
}

//-------------------------------------------------
//  populate
//-------------------------------------------------

void ui_menu_select_game::populate()
{
	ui_globals::redraw_icon = true;
	ui_globals::switch_image = true;
	int old_item_selected = -1;

	if (!isfavorite())
	{
		// if search is not empty, find approximate matches
		if (m_search[0] != 0 && !isfavorite())
			populate_search();
		else
		{
			// reset search string
			m_search[0] = '\0';
			m_displaylist.clear();
			m_tmp.clear();

			// if filter is set on category, build category list
			switch (main_filters::actual)
			{
				case FILTER_CATEGORY:
					build_category();
					break;
				case FILTER_MANUFACTURER:
					build_list(m_tmp, c_mnfct::ui[c_mnfct::actual].c_str());
					break;
				case FILTER_YEAR:
					build_list(m_tmp, c_year::ui[c_year::actual].c_str());
					break;
				case FILTER_CUSTOM:
					build_custom();
					break;
				default:
					build_list(m_tmp);
					break;
			}

			// iterate over entries
			int curitem = 0;
			for (auto & elem : m_displaylist)
			{
				UINT32 flags_ui = MENU_FLAG_UI | MENU_FLAG_LEFT_ARROW | MENU_FLAG_RIGHT_ARROW;

				if (old_item_selected == -1 && elem->name == reselect_last::driver)
					old_item_selected = curitem;

				bool cloneof = strcmp(elem->parent, "0");
				if (cloneof)
				{
					int cx = driver_list::find(elem->parent);
					if (cx != -1 && ((driver_list::driver(cx).flags & MACHINE_IS_BIOS_ROOT) != 0))
						cloneof = false;
				}
				if (cloneof)
					flags_ui |= MENU_FLAG_INVERT;

				item_append(elem->description, nullptr, flags_ui, (void *)elem);
				curitem++;
			}
		}
	}
	// populate favorites list
	else
	{
		m_search[0] = '\0';
		int curitem = 0;
		// iterate over entries
		for (auto & mfavorite : machine().favorite().m_list)
		{
			UINT32 flags_ui = MENU_FLAG_UI | MENU_FLAG_LEFT_ARROW | MENU_FLAG_RIGHT_ARROW | MENU_FLAG_UI_FAVORITE;
			if (mfavorite.startempty == 1)
			{
				if (old_item_selected == -1 && mfavorite.shortname == reselect_last::driver)
					old_item_selected = curitem;

				bool cloneof = strcmp(mfavorite.driver->parent, "0");
				if (cloneof)
				{
					int cx = driver_list::find(mfavorite.driver->parent);
					if (cx != -1 && ((driver_list::driver(cx).flags & MACHINE_IS_BIOS_ROOT) != 0))
						cloneof = false;
				}
				if (cloneof)
					flags_ui |= MENU_FLAG_INVERT;

				item_append(mfavorite.longname.c_str(), nullptr, flags_ui, (void *)&mfavorite);
			}
			else
			{
				if (old_item_selected == -1 && mfavorite.shortname == reselect_last::driver)
					old_item_selected = curitem;
				item_append(mfavorite.longname.c_str(), mfavorite.devicetype.c_str(),
					mfavorite.parentname.empty() ? flags_ui : (MENU_FLAG_INVERT | flags_ui), (void *)&mfavorite);
			}
			curitem++;
		}
	}

	item_append(MENU_SEPARATOR_ITEM, nullptr, MENU_FLAG_UI, nullptr);

	// add special items
	if (ui_menu::stack_has_special_main_menu())
	{
		UINT32 flags_ui = MENU_FLAG_UI | MENU_FLAG_LEFT_ARROW | MENU_FLAG_RIGHT_ARROW;
		item_append(_("Configure Options"), nullptr, flags_ui, (void *)(FPTR)CONF_OPTS);
//      item_append(_("Configure Machine"), nullptr, flags_ui, (void *)(FPTR)CONF_MACHINE); TODO
		skip_main_items = 1;
		if (machine().options().plugins())
		{
			item_append(_("Plugins"), nullptr, flags_ui, (void *)(FPTR)CONF_PLUGINS);
			skip_main_items++;
		}
	}
	else
		skip_main_items = 0;

	// configure the custom rendering
	customtop = 3.0f * machine().ui().get_line_height() + 5.0f * UI_BOX_TB_BORDER;
	custombottom = 5.0f * machine().ui().get_line_height() + 3.0f * UI_BOX_TB_BORDER;

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

void ui_menu_select_game::build_available_list()
{
	int m_total = driver_list::total();
	std::vector<bool> m_included(m_total, false);

	// open a path to the ROMs and find them in the array
	file_enumerator path(machine().options().media_path());
	const osd_directory_entry *dir;

	// iterate while we get new objects
	while ((dir = path.next()) != nullptr)
	{
		char drivername[50];
		char *dst = drivername;
		const char *src;

		// build a name for it
		for (src = dir->name; *src != 0 && *src != '.' && dst < &drivername[ARRAY_LENGTH(drivername) - 1]; ++src)
			*dst++ = tolower((UINT8) * src);

		*dst = 0;
		int drivnum = driver_list::find(drivername);
		if (drivnum != -1 && !m_included[drivnum])
		{
			m_availsortedlist.push_back(&driver_list::driver(drivnum));
			m_included[drivnum] = true;
		}
	}

	// now check and include NONE_NEEDED
	for (int x = 0; x < m_total; ++x)
	{
		const game_driver *driver = &driver_list::driver(x);
		if (!m_included[x] && driver != &GAME_NAME(___empty))
		{
			const rom_entry *rom = driver->rom;
			bool noroms = true;

			// check NO-DUMP
			for (; !ROMENTRY_ISEND(rom) && noroms == true; ++rom)
				if (ROMENTRY_ISFILE(rom))
				{
					hash_collection hashes(ROM_GETHASHDATA(rom));
					if (!hashes.flag(hash_collection::FLAG_NO_DUMP) && !ROM_ISOPTIONAL(rom))
						noroms = false;
				}

			if (!noroms)
			{
				// check if clone == parent
				int cx = driver_list::clone(*driver);
				if (cx != -1 && m_included[cx])
				{
					const game_driver *drv = &driver_list::driver(cx);
					const rom_entry *parentrom = drv->rom;
					if ((rom = driver->rom) == parentrom)
						noroms = true;

					// check if clone < parent
					if (!noroms)
					{
						noroms = true;
						for (; !ROMENTRY_ISEND(rom) && noroms == true; ++rom)
						{
							if (ROMENTRY_ISFILE(rom))
							{
								hash_collection hashes(ROM_GETHASHDATA(rom));
								if (hashes.flag(hash_collection::FLAG_NO_DUMP) || ROM_ISOPTIONAL(rom))
									continue;

								UINT64 lenght = ROM_GETLENGTH(rom);
								bool found = false;
								for (parentrom = drv->rom; !ROMENTRY_ISEND(parentrom) && found == false; ++parentrom)
								{
									if (ROMENTRY_ISFILE(parentrom) && ROM_GETLENGTH(parentrom) == lenght)
									{
										hash_collection parenthashes(ROM_GETHASHDATA(parentrom));
										if (parenthashes.flag(hash_collection::FLAG_NO_DUMP) || ROM_ISOPTIONAL(parentrom))
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
//  perform our special rendering
//-------------------------------------------------

void ui_menu_select_game::custom_render(void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
	const game_driver *driver = nullptr;
	ui_software_info *swinfo = nullptr;
	float width, maxwidth = origx2 - origx1;
	std::string tempbuf[5];
	rgb_t color = UI_BACKGROUND_COLOR;
	bool isstar = false;
	ui_manager &mui = machine().ui();
	float tbarspace = mui.get_line_height();
	float text_size = 1.0f;

	tempbuf[0] = string_format(_("%1$s %2$s ( %3$d / %4$d machines (%5$d BIOS) )"),
			emulator_info::get_appname(),
			bare_build_version,
			visible_items,
			(driver_list::total() - 1),
			m_isabios);

	std::string filtered;
	if (main_filters::actual == FILTER_CATEGORY && machine().inifile().total() > 0)
	{
		filtered = string_format(_("%1$s (%2$s - %3$s) - "),
				main_filters::text[main_filters::actual],
				machine().inifile().get_file(),
				machine().inifile().get_category());
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
		tempbuf[1].clear();
	else
		tempbuf[1] = string_format(_("%1$s Search: %2$s_"), filtered, m_search);

	// get the size of the text
	for (int line = 0; line < 2; ++line)
	{
		mui.draw_text_full(container, tempbuf[line].c_str(), 0.0f, 0.0f, 1.0f, JUSTIFY_CENTER, WRAP_NEVER,
			DRAW_NONE, ARGB_WHITE, ARGB_BLACK, &width, nullptr);
		width += 2 * UI_BOX_LR_BORDER;
		maxwidth = MAX(width, maxwidth);
	}

	if (maxwidth > origx2 - origx1)
	{
		text_size = (origx2 - origx1) / maxwidth;
		maxwidth = origx2 - origx1;
	}

	// compute our bounds
	float x1 = 0.5f - 0.5f * maxwidth;
	float x2 = x1 + maxwidth;
	float y1 = origy1 - top;
	float y2 = origy1 - 3.0f * UI_BOX_TB_BORDER - tbarspace;

	// draw a box
	mui.draw_outlined_box(container, x1, y1, x2, y2, UI_BACKGROUND_COLOR);

	// take off the borders
	x1 += UI_BOX_LR_BORDER;
	x2 -= UI_BOX_LR_BORDER;
	y1 += UI_BOX_TB_BORDER;

	// draw the text within it
	for (int line = 0; line < 2; ++line)
	{
		mui.draw_text_full(container, tempbuf[line].c_str(), x1, y1, x2 - x1, JUSTIFY_CENTER, WRAP_NEVER,
			DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, nullptr, nullptr, text_size);
		y1 += mui.get_line_height();
	}

	// determine the text to render below
	if (!isfavorite())
		driver = ((FPTR)selectedref > skip_main_items) ? (const game_driver *)selectedref : ((m_prev_selected != nullptr) ? (const game_driver *)m_prev_selected : nullptr);
	else
	{
		swinfo = ((FPTR)selectedref > skip_main_items) ? (ui_software_info *)selectedref : ((m_prev_selected != nullptr) ? (ui_software_info *)m_prev_selected : nullptr);
		if (swinfo != nullptr && swinfo->startempty == 1)
			driver = swinfo->driver;
	}

	if (driver != nullptr)
	{
		isstar = machine().favorite().isgame_favorite(driver);

		// first line is game name
		tempbuf[0] = string_format(_("Romset: %1$-.100s"), driver->name);

		// next line is year, manufacturer
		tempbuf[1] = string_format(_("%1$s, %2$-.100s"), driver->year, driver->manufacturer);

		// next line is clone/parent status
		int cloneof = driver_list::non_bios_clone(*driver);

		if (cloneof != -1)
			tempbuf[2] = string_format(_("Driver is clone of: %1$-.100s"), driver_list::driver(cloneof).description);
		else
			tempbuf[2] = _("Driver is parent");

		// next line is overall driver status
		if (driver->flags & MACHINE_NOT_WORKING)
			tempbuf[3] = _("Overall: NOT WORKING");
		else if (driver->flags & MACHINE_UNEMULATED_PROTECTION)
			tempbuf[3] = _("Overall: Unemulated Protection");
		else
			tempbuf[3] = _("Overall: Working");

		// next line is graphics, sound status
		if (driver->flags & (MACHINE_IMPERFECT_GRAPHICS | MACHINE_WRONG_COLORS | MACHINE_IMPERFECT_COLORS))
			tempbuf[4] = _("Graphics: Imperfect, ");
		else
			tempbuf[4] = _("Graphics: OK, ");

		if (driver->flags & MACHINE_NO_SOUND)
			tempbuf[4].append(_("Sound: Unimplemented"));
		else if (driver->flags & MACHINE_IMPERFECT_SOUND)
			tempbuf[4].append(_("Sound: Imperfect"));
		else
			tempbuf[4].append(_("Sound: OK"));

		color = UI_GREEN_COLOR;

		if ((driver->flags & (MACHINE_IMPERFECT_GRAPHICS | MACHINE_WRONG_COLORS | MACHINE_IMPERFECT_COLORS
			| MACHINE_NO_SOUND | MACHINE_IMPERFECT_SOUND)) != 0)
			color = UI_YELLOW_COLOR;

		if ((driver->flags & (MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION)) != 0)
			color = UI_RED_COLOR;
	}

	else if (swinfo != nullptr)
	{
		isstar = machine().favorite().isgame_favorite(*swinfo);

		// first line is system
		tempbuf[0] = string_format(_("System: %1$-.100s"), swinfo->driver->description);

		// next line is year, publisher
		tempbuf[1] = string_format(_("%1$s, %2$-.100s"), swinfo->year.c_str(), swinfo->publisher.c_str());

		// next line is parent/clone
		if (!swinfo->parentname.empty())
			tempbuf[2] = string_format(_("Software is clone of: %1$-.100s"), !swinfo->parentlongname.empty() ? swinfo->parentlongname.c_str() : swinfo->parentname.c_str());
		else
			tempbuf[2] = _("Software is parent");

		// next line is supported status
		if (swinfo->supported == SOFTWARE_SUPPORTED_NO)
		{
			tempbuf[3] = _("Supported: No");
			color = UI_RED_COLOR;
		}
		else if (swinfo->supported == SOFTWARE_SUPPORTED_PARTIAL)
		{
			tempbuf[3] = _("Supported: Partial");
			color = UI_YELLOW_COLOR;
		}
		else
		{
			tempbuf[3] = _("Supported: Yes");
			color = UI_GREEN_COLOR;
		}

		// last line is romset name
		tempbuf[4] = string_format(_("romset: %1$-.100s"), swinfo->shortname.c_str());
	}
	else
	{
		std::string copyright(emulator_info::get_copyright());
		size_t found = copyright.find("\n");
		tempbuf[0].clear();
		tempbuf[1] = string_format(_("%1$s %2$s"), emulator_info::get_appname(), build_version);
		tempbuf[2] = copyright.substr(0, found);
		tempbuf[3] = copyright.substr(found + 1);
		tempbuf[4].clear();
	}

	// compute our bounds
	x1 = 0.5f - 0.5f * maxwidth;
	x2 = x1 + maxwidth;
	y1 = y2;
	y2 = origy1 - UI_BOX_TB_BORDER;

	// draw toolbar
	draw_toolbar(x1, y1, x2, y2);

	// get the size of the text
	maxwidth = origx2 - origx1;

	for (auto & elem : tempbuf)
	{
		mui.draw_text_full(container, elem.c_str(), 0.0f, 0.0f, 1.0f, JUSTIFY_CENTER, WRAP_NEVER,
			DRAW_NONE, ARGB_WHITE, ARGB_BLACK, &width, nullptr);
		width += 2 * UI_BOX_LR_BORDER;
		maxwidth = MAX(maxwidth, width);
	}

	if (maxwidth > origx2 - origx1)
	{
		text_size = (origx2 - origx1) / maxwidth;
		maxwidth = origx2 - origx1;
	}

	// compute our bounds
	x1 = 0.5f - 0.5f * maxwidth;
	x2 = x1 + maxwidth;
	y1 = origy2 + UI_BOX_TB_BORDER;
	y2 = origy2 + bottom;

	// draw a box
	mui.draw_outlined_box(container, x1, y1, x2, y2, color);

	// take off the borders
	x1 += UI_BOX_LR_BORDER;
	x2 -= UI_BOX_LR_BORDER;
	y1 += UI_BOX_TB_BORDER;

	// is favorite? draw the star
	if (isstar)
		draw_star(x1, y1);

	// draw all lines
	for (auto & elem : tempbuf)
	{
		mui.draw_text_full(container, elem.c_str(), x1, y1, x2 - x1, JUSTIFY_CENTER, WRAP_NEVER,
			DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, nullptr, nullptr, text_size);
		y1 += mui.get_line_height();
	}
}

//-------------------------------------------------
//  force the game select menu to be visible
//  and inescapable
//-------------------------------------------------

void ui_menu_select_game::force_game_select(running_machine &machine, render_container *container)
{
	// reset the menu stack
	ui_menu::stack_reset(machine);

	// add the quit entry followed by the game select entry
	ui_menu *quit = global_alloc_clear<ui_menu_quit_game>(machine, container);
	quit->set_special_main_menu(true);
	ui_menu::stack_push(quit);
	ui_menu::stack_push(global_alloc_clear<ui_menu_select_game>(machine, container, nullptr));

	// force the menus on
	machine.ui().show_menu();

	// make sure MAME is paused
	machine.pause();
}

//-------------------------------------------------
//  handle select key event
//-------------------------------------------------

void ui_menu_select_game::inkey_select(const ui_menu_event *m_event)
{
	const game_driver *driver = (const game_driver *)m_event->itemref;

	// special case for configure options
	if ((FPTR)driver == CONF_OPTS)
		ui_menu::stack_push(global_alloc_clear<ui_menu_game_options>(machine(), container));
	/* special case for configure machine TODO
	else if ((FPTR)driver == CONF_MACHINE)
	{
	    if (m_prev_selected != nullptr)
	        ui_menu::stack_push(global_alloc_clear<ui_menu_machine_configure>(machine(), container, (const game_driver *)m_prev_selected));
	    else
	        return;
	} */
	// special case for configure plugins
	else if ((FPTR)driver == CONF_PLUGINS)
	{
		ui_menu::stack_push(global_alloc_clear<ui_menu_plugins_configure>(machine(), container));
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
				software_list_device_iterator iter(enumerator.config().root_device());
				for (software_list_device *swlistdev = iter.first(); swlistdev != nullptr; swlistdev = iter.next())
					if (!swlistdev->get_info().empty())
					{
						ui_menu::stack_push(global_alloc_clear<ui_menu_select_software>(machine(), container, driver));
						return;
					}
			}

			s_bios biosname;
			if (!machine().ui().options().skip_bios_menu() && has_multiple_bios(driver, biosname))
				ui_menu::stack_push(global_alloc_clear<ui_bios_selection>(machine(), container, biosname, (void *)driver, false, false));
			else
			{
				reselect_last::driver = driver->name;
				reselect_last::software.clear();
				reselect_last::swlist.clear();
				machine().manager().schedule_new_driver(*driver);
				machine().schedule_hard_reset();
				ui_menu::stack_reset(machine());
			}
		}
		// otherwise, display an error
		else
		{
			reset(UI_MENU_RESET_REMEMBER_REF);
			ui_error = true;
		}
	}
}

//-------------------------------------------------
//  handle select key event for favorites menu
//-------------------------------------------------

void ui_menu_select_game::inkey_select_favorite(const ui_menu_event *m_event)
{
	ui_software_info *ui_swinfo = (ui_software_info *)m_event->itemref;
	ui_options &mopt = machine().ui().options();

	// special case for configure options
	if ((FPTR)ui_swinfo == CONF_OPTS)
		ui_menu::stack_push(global_alloc_clear<ui_menu_game_options>(machine(), container));
	/* special case for configure machine TODO
	else if ((FPTR)ui_swinfo == CONF_MACHINE)
	{
	    if (m_prev_selected != nullptr)
	    {
	        ui_software_info *swinfo = (ui_software_info *)m_prev_selected;
	        if (swinfo->startempty == 1)
	            ui_menu::stack_push(global_alloc_clear<ui_menu_machine_configure>(machine(), container, swinfo->driver));
	    }
	    else
	        return;
	} */
	// special case for configure plugins
	else if ((FPTR)ui_swinfo == CONF_PLUGINS)
	{
		ui_menu::stack_push(global_alloc_clear<ui_menu_plugins_configure>(machine(), container));
	}
	else if (ui_swinfo->startempty == 1)
	{
		// audit the game first to see if we're going to work
		driver_enumerator enumerator(machine().options(), *ui_swinfo->driver);
		enumerator.next();
		media_auditor auditor(enumerator);
		media_auditor::summary summary = auditor.audit_media(AUDIT_VALIDATE_FAST);

		// if everything looks good, schedule the new driver
		if (summary == media_auditor::CORRECT || summary == media_auditor::BEST_AVAILABLE || summary == media_auditor::NONE_NEEDED)
		{
			s_bios biosname;
			if (!mopt.skip_bios_menu() && has_multiple_bios(ui_swinfo->driver, biosname))
				ui_menu::stack_push(global_alloc_clear<ui_bios_selection>(machine(), container, biosname, (void *)ui_swinfo->driver, false, false));
			else
			{
				reselect_last::driver = ui_swinfo->driver->name;
				reselect_last::software.clear();
				reselect_last::swlist.clear();
				reselect_last::set(true);
				machine().manager().schedule_new_driver(*ui_swinfo->driver);
				machine().schedule_hard_reset();
				ui_menu::stack_reset(machine());
			}
		}

		// otherwise, display an error
		else
		{
			reset(UI_MENU_RESET_REMEMBER_REF);
			ui_error = true;
		}
	}
	else
	{
		// first validate
		driver_enumerator drv(machine().options(), *ui_swinfo->driver);
		media_auditor auditor(drv);
		drv.next();
		software_list_device *swlist = software_list_device::find_by_name(drv.config(), ui_swinfo->listname.c_str());
		software_info *swinfo = swlist->find(ui_swinfo->shortname.c_str());
		media_auditor::summary summary = auditor.audit_software(swlist->list_name(), swinfo, AUDIT_VALIDATE_FAST);
		if (summary == media_auditor::CORRECT || summary == media_auditor::BEST_AVAILABLE || summary == media_auditor::NONE_NEEDED)
		{
			s_bios biosname;
			if (!mopt.skip_bios_menu() && has_multiple_bios(ui_swinfo->driver, biosname))
			{
				ui_menu::stack_push(global_alloc_clear<ui_bios_selection>(machine(), container, biosname, (void *)ui_swinfo, true, false));
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
				ui_menu::stack_push(global_alloc_clear<ui_software_parts>(machine(), container, parts, ui_swinfo));
				return;
			}

			std::string error_string;
			std::string string_list = std::string(ui_swinfo->listname).append(":").append(ui_swinfo->shortname).append(":").append(ui_swinfo->part).append(":").append(ui_swinfo->instance);
			mopt.set_value(OPTION_SOFTWARENAME, string_list.c_str(), OPTION_PRIORITY_CMDLINE, error_string);
			std::string snap_list = std::string(ui_swinfo->listname).append(PATH_SEPARATOR).append(ui_swinfo->shortname);
			mopt.set_value(OPTION_SNAPNAME, snap_list.c_str(), OPTION_PRIORITY_CMDLINE, error_string);
			reselect_last::driver = drv.driver().name;
			reselect_last::software = ui_swinfo->shortname;
			reselect_last::swlist = ui_swinfo->listname;
			machine().manager().schedule_new_driver(drv.driver());
			machine().schedule_hard_reset();
			ui_menu::stack_reset(machine());
		}
		// otherwise, display an error
		else
		{
			reset(UI_MENU_RESET_REMEMBER_POSITION);
			ui_error = true;
		}
	}
}

//-------------------------------------------------
//  returns if the search can be activated
//-------------------------------------------------

inline bool ui_menu_select_game::isfavorite()
{
	return (main_filters::actual == FILTER_FAVORITE);
}

//-------------------------------------------------
//  handle special key event
//-------------------------------------------------

void ui_menu_select_game::inkey_special(const ui_menu_event *m_event)
{
	int buflen = strlen(m_search);

	// if it's a backspace and we can handle it, do so
	if (((m_event->unichar == 8 || m_event->unichar == 0x7f) && buflen > 0) && !isfavorite())
	{
		*(char *)utf8_previous_char(&m_search[buflen]) = 0;
		reset(UI_MENU_RESET_SELECT_FIRST);
	}
	else if ((m_event->unichar >= ' ' && m_event->unichar < 0x7f) && !isfavorite())
	{
		// if it's any other key and we're not maxed out, update
		buflen += utf8_from_uchar(&m_search[buflen], ARRAY_LENGTH(m_search) - buflen, m_event->unichar);
		m_search[buflen] = 0;
		reset(UI_MENU_RESET_SELECT_FIRST);
	}
}


void ui_menu_select_game::inkey_configure(const ui_menu_event *m_event)
{
	if (selected <= visible_items && m_focus == focused_menu::main)
	{
		m_prev_selected = item[selected].ref;
		selected = visible_items + 1;
	}
	else if (selected > visible_items && m_focus == focused_menu::main)
	{
		if (ui_globals::panels_status != HIDE_LEFT_PANEL)
			m_focus = focused_menu::left;

		else if (ui_globals::panels_status == HIDE_BOTH)
		{
			for (int x = 0; x < item.size(); ++x)
				if (item[x].ref == m_prev_selected)
					selected = x;
		}
		else
			m_focus = focused_menu::righttop;
	}
	else if (m_focus == focused_menu::left)
	{
		if (ui_globals::panels_status != HIDE_RIGHT_PANEL)
			m_focus = focused_menu::righttop;
		else
		{
			m_focus = focused_menu::main;
			if (m_prev_selected == nullptr)
			{
				selected = 0;
				return;
			}

			for (int x = 0; x < item.size(); ++x)
				if (item[x].ref == m_prev_selected)
					selected = x;
		}
	}
	else if (m_focus == focused_menu::righttop)
		m_focus = focused_menu::rightbottom;
	else if (m_focus == focused_menu::rightbottom)
	{
		m_focus = focused_menu::main;
		if (m_prev_selected == nullptr)
		{
			selected = 0;
			return;
		}

		for (int x = 0; x < item.size(); ++x)
			if (item[x].ref == m_prev_selected)
				selected = x;
	}
}

//-------------------------------------------------
//  build list
//-------------------------------------------------

void ui_menu_select_game::build_list(std::vector<const game_driver *> &s_drivers, const char *filter_text, int filter, bool bioscheck)
{
	int cx = 0;
	bool cloneof = false;

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
				cloneof = strcmp(s_driver->parent, "0");
				if (cloneof)
				{
					cx = driver_list::find(s_driver->parent);
					if (cx != -1 && ((driver_list::driver(cx).flags & MACHINE_IS_BIOS_ROOT) != 0))
						cloneof = false;
				}

				if (filter == FILTER_CLONES && cloneof)
					m_displaylist.push_back(s_driver);
				else if (filter == FILTER_PARENT && !cloneof)
					m_displaylist.push_back(s_driver);
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
				break;
			}
			case FILTER_CHD:
				for (const rom_entry *rom = s_driver->rom; !ROMENTRY_ISEND(rom); ++rom)
					if (ROMENTRY_ISREGION(rom) && ROMREGION_ISDISKDATA(rom))
					{
						m_displaylist.push_back(s_driver);
						break;
					}
				break;
			case FILTER_NOCHD:
			{
				bool found = false;
				for (const rom_entry *rom = s_driver->rom; !ROMENTRY_ISEND(rom); ++rom)
					if (ROMENTRY_ISREGION(rom) && ROMREGION_ISDISKDATA(rom))
					{
						found = true;
						break;
					}
				if (!found)
					m_displaylist.push_back(s_driver);
				break;
			}
		}
	}
}

//-------------------------------------------------
//  build custom display list
//-------------------------------------------------

void ui_menu_select_game::build_custom()
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
				build_list(s_drivers, c_year::ui[custfltr::year[count]].c_str(), filter, bioscheck);
				break;
			case FILTER_MANUFACTURER:
				build_list(s_drivers, c_mnfct::ui[custfltr::mnfct[count]].c_str(), filter, bioscheck);
				break;
			default:
				build_list(s_drivers, nullptr, filter, bioscheck);
				break;
		}
	}
}

//-------------------------------------------------
//  build category list
//-------------------------------------------------

void ui_menu_select_game::build_category()
{
	std::vector<int> temp_filter;
	machine().inifile().load_ini_category(temp_filter);

	for (auto actual : temp_filter)
		m_tmp.push_back(&driver_list::driver(actual));

	std::stable_sort(m_tmp.begin(), m_tmp.end(), sorted_game_list);
	m_displaylist = m_tmp;
}

//-------------------------------------------------
//  populate search list
//-------------------------------------------------

void ui_menu_select_game::populate_search()
{
	// allocate memory to track the penalty value
	std::vector<int> penalty(VISIBLE_GAMES_IN_SEARCH, 9999);
	int index = 0;
	for (; index < m_displaylist.size(); ++index)
	{
		// pick the best match between driver name and description
		int curpenalty = fuzzy_substring(m_search, m_displaylist[index]->description);
		int tmp = fuzzy_substring(m_search, m_displaylist[index]->name);
		curpenalty = MIN(curpenalty, tmp);

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
	UINT32 flags_ui = MENU_FLAG_UI | MENU_FLAG_LEFT_ARROW | MENU_FLAG_RIGHT_ARROW;
	for (int curitem = 0; m_searchlist[curitem]; ++curitem)
	{
		bool cloneof = strcmp(m_searchlist[curitem]->parent, "0");
		if (cloneof)
		{
			int cx = driver_list::find(m_searchlist[curitem]->parent);
			if (cx != -1 && ((driver_list::driver(cx).flags & MACHINE_IS_BIOS_ROOT) != 0))
				cloneof = false;
		}
		item_append(m_searchlist[curitem]->description, nullptr, (!cloneof) ? flags_ui : (MENU_FLAG_INVERT | flags_ui),
			(void *)m_searchlist[curitem]);
	}
}

//-------------------------------------------------
//  generate general info
//-------------------------------------------------

void ui_menu_select_game::general_info(const game_driver *driver, std::string &buffer)
{
	std::ostringstream str;

	util::stream_format(str, _("Romset: %1$-.100s\n"), driver->name);
	util::stream_format(str, _("Year: %1$s\n"), driver->year);
	util::stream_format(str, _("Manufacturer: %1$-.100s\n"), driver->manufacturer);

	int cloneof = driver_list::non_bios_clone(*driver);
	if (cloneof != -1)
		util::stream_format(str, _("Driver is Clone of: %1$-.100s\n"), driver_list::driver(cloneof).description);
	else
		str << _("Driver is Parent\n");

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
	for (const rom_entry *rom = driver->rom; !ROMENTRY_ISEND(rom); ++rom)
		if (ROMENTRY_ISREGION(rom) && ROMREGION_ISDISKDATA(rom))
		{
			found = true;
			break;
		}
	util::stream_format(str, _("Requires CHD: %1$s\n"), found ? _("Yes") : _("No"));

	// audit the game first to see if we're going to work
	if (machine().ui().options().info_audit())
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

	buffer = str.str();
}

void ui_menu_select_game::inkey_export()
{
	std::vector<const game_driver *> list;
	if (m_search[0] != 0)
	{
		for (int curitem = 0; m_searchlist[curitem]; ++curitem)
		{
			list.push_back(m_searchlist[curitem]);
		}
	}
	else
	{
		list = m_displaylist;
	}
	ui_menu::stack_push(global_alloc_clear<ui_menu_export>(machine(), container, list));
}

//-------------------------------------------------
//  save drivers infos to file
//-------------------------------------------------

void ui_menu_select_game::init_sorted_list()
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

	// sort manufacturers - years and driver
	std::stable_sort(c_mnfct::ui.begin(), c_mnfct::ui.end());
	std::stable_sort(c_year::ui.begin(), c_year::ui.end());
	std::stable_sort(m_sortedlist.begin(), m_sortedlist.end(), sorted_game_list);
}

//-------------------------------------------------
//  load drivers infos from file
//-------------------------------------------------

bool ui_menu_select_game::load_available_machines()
{
	// try to load available drivers from file
	emu_file file(machine().ui().options().ui_path(), OPEN_FLAG_READ);
	if (file.open(emulator_info::get_configname(), "_avail.ini") != osd_file::error::NONE)
		return false;

	std::string readbuf;
	char rbuf[MAX_CHAR_INFO];
	file.gets(rbuf, MAX_CHAR_INFO);
	file.gets(rbuf, MAX_CHAR_INFO);
	readbuf = chartrimcarriage(rbuf);
	std::string a_rev = std::string(UI_VERSION_TAG).append(bare_build_version);

	// version not matching ? exit
	if (a_rev != readbuf)
	{
		file.close();
		return false;
	}

	file.gets(rbuf, MAX_CHAR_INFO);
	file.gets(rbuf, MAX_CHAR_INFO);
	int avsize = 0, unavsize = 0;
	file.gets(rbuf, MAX_CHAR_INFO);
	avsize = atoi(rbuf);
	file.gets(rbuf, MAX_CHAR_INFO);
	unavsize = atoi(rbuf);

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

void ui_menu_select_game::load_custom_filters()
{
	// attempt to open the output file
	emu_file file(machine().ui().options().ui_path(), OPEN_FLAG_READ);
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

float ui_menu_select_game::draw_left_panel(float x1, float y1, float x2, float y2)
{
	ui_manager &mui = machine().ui();
	float line_height = mui.get_line_height();

	if (ui_globals::panels_status == SHOW_PANELS || ui_globals::panels_status == HIDE_RIGHT_PANEL)
	{
		float origy1 = y1;
		float origy2 = y2;
		float text_size = 0.75f;
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

		float text_sign = mui.get_string_width_ex("_# ", text_size);
		for (int x = 0; x < text_lenght; ++x)
		{
			float total_width;

			// compute width of left hand side
			total_width = mui.get_string_width_ex(text[x], text_size);
			total_width += text_sign;

			// track the maximum
			if (total_width > left_width)
				left_width = total_width;
		}

		x2 = x1 + left_width + 2.0f * UI_BOX_LR_BORDER;
		mui.draw_outlined_box(container, x1, y1, x2, y2, UI_BACKGROUND_COLOR);

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

			if (mouse_hit && x1 <= mouse_x && x2 > mouse_x && y1 <= mouse_y && y1 + line_height_max > mouse_y)
			{
				bgcolor = UI_MOUSEOVER_BG_COLOR;
				fgcolor = UI_MOUSEOVER_COLOR;
				hover = phover + filter;
				ui_menu::highlight(container, x1, y1, x2, y1+ line_height_max, bgcolor);
			}

			if (highlight == filter && m_focus == focused_menu::left)
			{
				fgcolor = rgb_t(0xff, 0xff, 0xff, 0x00);
				bgcolor = rgb_t(0xff, 0xff, 0xff, 0xff);
				mui.draw_textured_box(container, x1, y1, x2, y1 + line_height_max, bgcolor, rgb_t(255, 43, 43, 43),
					hilight_main_texture, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA) | PRIMFLAG_TEXWRAP(TRUE));
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

			mui.draw_text_full(container, str.c_str(), x1t, y1, x2 - x1, JUSTIFY_LEFT, WRAP_NEVER,
				DRAW_NORMAL, fgcolor, bgcolor, nullptr, nullptr, text_size);
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

		mui.draw_outlined_box(container, x1, y1, x2, y2, rgb_t(0xEF, 0x12, 0x47, 0x7B));

		if (mouse_hit && x1 <= mouse_x && x2 > mouse_x && y1 <= mouse_y && y2 > mouse_y)
		{
			fgcolor = UI_MOUSEOVER_COLOR;
			hover = HOVER_LPANEL_ARROW;
		}

		draw_arrow(container, ar_x0, ar_y0, ar_x1, ar_y1, fgcolor, ROT90 ^ ORIENTATION_FLIP_X);
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

		mui.draw_outlined_box(container, x1, y1, x2, y2, rgb_t(0xEF, 0x12, 0x47, 0x7B));

		if (mouse_hit && x1 <= mouse_x && x2 > mouse_x && y1 <= mouse_y && y2 > mouse_y)
		{
			fgcolor = UI_MOUSEOVER_COLOR;
			hover = HOVER_LPANEL_ARROW;
		}

		draw_arrow(container, ar_x0, ar_y0, ar_x1, ar_y1, fgcolor, ROT90);
		return x2 + UI_BOX_LR_BORDER;
	}
}

//-------------------------------------------------
//  draw infos
//-------------------------------------------------

void ui_menu_select_game::infos_render(void *selectedref, float origx1, float origy1, float origx2, float origy2)
{
	ui_manager &mui = machine().ui();
	float line_height = mui.get_line_height();
	static std::string buffer;
	std::vector<int> xstart;
	std::vector<int> xend;
	float text_size = machine().ui().options().infos_size();
	const game_driver *driver = nullptr;
	ui_software_info *soft = nullptr;
	bool is_favorites = ((item[0].flags & MENU_FLAG_UI_FAVORITE) != 0);
	static ui_software_info *oldsoft = nullptr;
	static const game_driver *olddriver = nullptr;
	static int oldview = -1;
	static int old_sw_view = -1;

	if (is_favorites)
	{
		soft = ((FPTR)selectedref > skip_main_items) ? (ui_software_info *)selectedref : ((m_prev_selected != nullptr) ? (ui_software_info *)m_prev_selected : nullptr);
		if (soft && soft->startempty == 1)
		{
			driver = soft->driver;
			oldsoft = nullptr;
		}
		else
			olddriver = nullptr;
	}
	else
	{
		driver = ((FPTR)selectedref > skip_main_items) ? (const game_driver *)selectedref : ((m_prev_selected != nullptr) ? (const game_driver *)m_prev_selected : nullptr);
		oldsoft = nullptr;
	}

	if (driver)
	{
		float gutter_width = 0.4f * line_height * machine().render().ui_aspect() * 1.3f;
		float ud_arrow_width = line_height * machine().render().ui_aspect();
		float oy1 = origy1 + line_height;

		// MAMESCORE? Full size text
		if (ui_globals::curdats_view == UI_STORY_LOAD)
			text_size = 1.0f;

		std::string snaptext(_(dats_info[ui_globals::curdats_view]));

		// apply title to right panel
		float title_size = 0.0f;
		float txt_length = 0.0f;

		for (int x = UI_FIRST_LOAD; x < UI_LAST_LOAD; ++x)
		{
			mui.draw_text_full(container, _(dats_info[x]), origx1, origy1, origx2 - origx1, JUSTIFY_CENTER,
				WRAP_NEVER, DRAW_NONE, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, &txt_length, nullptr);
			txt_length += 0.01f;
			title_size = (std::max)(txt_length, title_size);
		}

		rgb_t fgcolor = UI_TEXT_COLOR;
		rgb_t bgcolor = UI_TEXT_BG_COLOR;
		if (m_focus == focused_menu::rightbottom)
		{
			fgcolor = rgb_t(0xff, 0xff, 0xff, 0x00);
			bgcolor = rgb_t(0xff, 0xff, 0xff, 0xff);
		}

		float middle = origx2 - origx1;

		// check size
		float sc = title_size + 2.0f * gutter_width;
		float tmp_size = (sc > middle) ? ((middle - 2.0f * gutter_width) / sc) : 1.0f;
		title_size *= tmp_size;

		if (bgcolor != UI_TEXT_BG_COLOR)
			mui.draw_textured_box(container, origx1 + ((middle - title_size) * 0.5f), origy1, origx1 + ((middle + title_size) * 0.5f),
				origy1 + line_height, bgcolor, rgb_t(255, 43, 43, 43), hilight_main_texture, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA) | PRIMFLAG_TEXWRAP(TRUE));

		mui.draw_text_full(container, snaptext.c_str(), origx1, origy1, origx2 - origx1, JUSTIFY_CENTER,
			WRAP_NEVER, DRAW_NORMAL, fgcolor, bgcolor, nullptr, nullptr, tmp_size);

		draw_common_arrow(origx1, origy1, origx2, origy2, ui_globals::curdats_view, UI_FIRST_LOAD, UI_LAST_LOAD, title_size);

		if (driver != olddriver || ui_globals::curdats_view != oldview)
		{
			buffer.clear();
			olddriver = driver;
			oldview = ui_globals::curdats_view;
			topline_datsview = 0;
			totallines = 0;
			std::vector<std::string> m_item;

			if (ui_globals::curdats_view == UI_GENERAL_LOAD)
				general_info(driver, buffer);
			else if (ui_globals::curdats_view != UI_COMMAND_LOAD)
				machine().datfile().load_data_info(driver, buffer, ui_globals::curdats_view);
			else
				machine().datfile().command_sub_menu(driver, m_item);

			if (!m_item.empty() && ui_globals::curdats_view == UI_COMMAND_LOAD)
			{
				for (size_t x = 0; x < m_item.size(); ++x)
				{
					std::string t_buffer;
					buffer.append(m_item[x]).append("\n");
					machine().datfile().load_command_info(t_buffer, m_item[x]);
					if (!t_buffer.empty())
						buffer.append(t_buffer).append("\n");
				}
				convert_command_glyph(buffer);
			}
		}

		if (buffer.empty())
		{
			mui.draw_text_full(container, _("No Infos Available"), origx1, (origy2 + origy1) * 0.5f, origx2 - origx1, JUSTIFY_CENTER,
				WRAP_WORD, DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, nullptr, nullptr);
			return;
		}
		else if (ui_globals::curdats_view != UI_STORY_LOAD && ui_globals::curdats_view != UI_COMMAND_LOAD)
			totallines = mui.wrap_text(container, buffer.c_str(), origx1, origy1, origx2 - origx1 - (2.0f * gutter_width), xstart, xend, text_size);
		else
			totallines = mui.wrap_text(container, buffer.c_str(), 0.0f, 0.0f, 1.0f - (2.0f * gutter_width), xstart, xend, text_size);

		int r_visible_lines = floor((origy2 - oy1) / (line_height * text_size));
		if (totallines < r_visible_lines)
			r_visible_lines = totallines;
		if (topline_datsview < 0)
			topline_datsview = 0;
		if (topline_datsview + r_visible_lines >= totallines)
			topline_datsview = totallines - r_visible_lines;

		sc = origx2 - origx1 - (2.0f * UI_BOX_LR_BORDER);
		for (int r = 0; r < r_visible_lines; ++r)
		{
			int itemline = r + topline_datsview;
			std::string tempbuf(buffer.substr(xstart[itemline], xend[itemline] - xstart[itemline]));

			// up arrow
			if (r == 0 && topline_datsview != 0)
				info_arrow(0, origx1, origx2, oy1, line_height, text_size, ud_arrow_width);
			// bottom arrow
			else if (r == r_visible_lines - 1 && itemline != totallines - 1)
				info_arrow(1, origx1, origx2, oy1, line_height, text_size, ud_arrow_width);
			// special case for mamescore
			else if (ui_globals::curdats_view == UI_STORY_LOAD)
			{
				// check size
				float textlen = mui.get_string_width_ex(tempbuf.c_str(), text_size);
				float tmp_size = (textlen > sc) ? text_size * (sc / textlen) : text_size;

				size_t last_underscore = tempbuf.find_last_of("_");
				if (last_underscore == std::string::npos)
				{
					mui.draw_text_full(container, tempbuf.c_str(), origx1, oy1, origx2 - origx1, JUSTIFY_CENTER,
						WRAP_TRUNCATE, DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, nullptr, nullptr, tmp_size);
				}
				else
				{
					float effective_width = origx2 - origx1 - gutter_width;
					float effective_left = origx1 + gutter_width;
					std::string last_part(tempbuf.substr(last_underscore + 1));
					std::string first_part(tempbuf.substr(0, tempbuf.find("___")));
					float item_width;

					mui.draw_text_full(container, first_part.c_str(), effective_left, oy1, effective_width,
						JUSTIFY_LEFT, WRAP_TRUNCATE, DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, &item_width, nullptr, tmp_size);

					mui.draw_text_full(container, last_part.c_str(), effective_left + item_width, oy1,
						origx2 - origx1 - 2.0f * gutter_width - item_width, JUSTIFY_RIGHT, WRAP_TRUNCATE,
						DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, nullptr, nullptr, tmp_size);
				}
			}

			// special case for command
			else if (ui_globals::curdats_view == UI_COMMAND_LOAD || ui_globals::curdats_view == UI_GENERAL_LOAD)
			{
				// check size
				float textlen = mui.get_string_width_ex(tempbuf.c_str(), text_size);
				float tmp_size = (textlen > sc) ? text_size * (sc / textlen) : text_size;

				int first_dspace = (ui_globals::curdats_view == UI_COMMAND_LOAD) ? tempbuf.find("  ") : tempbuf.find(":");
				if (first_dspace > 0)
				{
					float effective_width = origx2 - origx1 - gutter_width;
					float effective_left = origx1 + gutter_width;
					std::string first_part(tempbuf.substr(0, first_dspace));
					std::string last_part(tempbuf.substr(first_dspace + 1));
					strtrimspace(last_part);
					mui.draw_text_full(container, first_part.c_str(), effective_left, oy1, effective_width, JUSTIFY_LEFT,
						WRAP_TRUNCATE, DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, nullptr, nullptr, tmp_size);

					mui.draw_text_full(container, last_part.c_str(), effective_left, oy1, origx2 - origx1 - 2.0f * gutter_width,
						JUSTIFY_RIGHT, WRAP_TRUNCATE, DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, nullptr, nullptr, tmp_size);
				}
				else
					mui.draw_text_full(container, tempbuf.c_str(), origx1 + gutter_width, oy1, origx2 - origx1, JUSTIFY_LEFT,
						WRAP_TRUNCATE, DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, nullptr, nullptr, tmp_size);
			}
			else
				mui.draw_text_full(container, tempbuf.c_str(), origx1 + gutter_width, oy1, origx2 - origx1, JUSTIFY_LEFT,
					WRAP_TRUNCATE, DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, nullptr, nullptr, text_size);

			oy1 += (line_height * text_size);
		}

		// return the number of visible lines, minus 1 for top arrow and 1 for bottom arrow
		right_visible_lines = r_visible_lines - (topline_datsview != 0) - (topline_datsview + r_visible_lines != totallines);
	}
	else if (soft)
	{
		float gutter_width = 0.4f * line_height * machine().render().ui_aspect() * 1.3f;
		float ud_arrow_width = line_height * machine().render().ui_aspect();
		float oy1 = origy1 + line_height;

		// apply title to right panel
		if (soft->usage.empty())
		{
			mui.draw_text_full(container, _("History"), origx1, origy1, origx2 - origx1, JUSTIFY_CENTER, WRAP_TRUNCATE,
				DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, nullptr, nullptr);
			ui_globals::cur_sw_dats_view = 0;
		}
		else
		{
			float title_size = 0.0f;
			float txt_length = 0.0f;
			std::string t_text[2];
			t_text[0] = _("History");
			t_text[1] = _("Usage");

			for (auto & elem: t_text)
			{
				mui.draw_text_full(container, elem.c_str(), origx1, origy1, origx2 - origx1, JUSTIFY_CENTER, WRAP_TRUNCATE,
					DRAW_NONE, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, &txt_length, nullptr);
				txt_length += 0.01f;
				title_size = (std::max)(txt_length, title_size);
			}

			rgb_t fgcolor = UI_TEXT_COLOR;
			rgb_t bgcolor = UI_TEXT_BG_COLOR;
			if (m_focus == focused_menu::rightbottom)
			{
				fgcolor = rgb_t(0xff, 0xff, 0xff, 0x00);
				bgcolor = rgb_t(0xff, 0xff, 0xff, 0xff);
			}

			float middle = origx2 - origx1;

			if (bgcolor != UI_TEXT_BG_COLOR)
				mui.draw_textured_box(container, origx1 + ((middle - title_size) * 0.5f), origy1, origx1 + ((middle + title_size) * 0.5f),
					origy1 + line_height, bgcolor, rgb_t(255, 43, 43, 43), hilight_main_texture, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA) | PRIMFLAG_TEXWRAP(TRUE));

			mui.draw_text_full(container, t_text[ui_globals::cur_sw_dats_view].c_str(), origx1, origy1, origx2 - origx1,
				JUSTIFY_CENTER, WRAP_TRUNCATE, DRAW_NORMAL, fgcolor, bgcolor, nullptr, nullptr);

			draw_common_arrow(origx1, origy1, origx2, origy2, ui_globals::cur_sw_dats_view, 0, 1, title_size);
		}

		if (oldsoft != soft || old_sw_view != ui_globals::cur_sw_dats_view)
		{
			buffer.clear();
			old_sw_view = ui_globals::cur_sw_dats_view;
			oldsoft = soft;
			if (ui_globals::cur_sw_dats_view == 0)
			{
				if (soft->startempty == 1)
					machine().datfile().load_data_info(soft->driver, buffer, UI_HISTORY_LOAD);
				else
					machine().datfile().load_software_info(soft->listname, buffer, soft->shortname, soft->parentname);
			}
			else
				buffer = soft->usage;
		}

		if (buffer.empty())
		{
			mui.draw_text_full(container, _("No Infos Available"), origx1, (origy2 + origy1) * 0.5f, origx2 - origx1, JUSTIFY_CENTER,
				WRAP_WORD, DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, nullptr, nullptr);
			return;
		}
		else
			totallines = mui.wrap_text(container, buffer.c_str(), origx1, origy1, origx2 - origx1 - (2.0f * gutter_width), xstart, xend, text_size);

		int r_visible_lines = floor((origy2 - oy1) / (line_height * text_size));
		if (totallines < r_visible_lines)
			r_visible_lines = totallines;
		if (topline_datsview < 0)
				topline_datsview = 0;
		if (topline_datsview + r_visible_lines >= totallines)
				topline_datsview = totallines - r_visible_lines;

		for (int r = 0; r < r_visible_lines; ++r)
		{
			int itemline = r + topline_datsview;
			std::string tempbuf(buffer.substr(xstart[itemline], xend[itemline] - xstart[itemline]));

			// up arrow
			if (r == 0 && topline_datsview != 0)
				info_arrow(0, origx1, origx2, oy1, line_height, text_size, ud_arrow_width);
			// bottom arrow
			else if (r == r_visible_lines - 1 && itemline != totallines - 1)
				info_arrow(1, origx1, origx2, oy1, line_height, text_size, ud_arrow_width);
			else
				mui.draw_text_full(container, tempbuf.c_str(), origx1 + gutter_width, oy1, origx2 - origx1, JUSTIFY_LEFT,
					WRAP_TRUNCATE, DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, nullptr, nullptr, text_size);
			oy1 += (line_height * text_size);
		}

		// return the number of visible lines, minus 1 for top arrow and 1 for bottom arrow
		right_visible_lines = r_visible_lines - (topline_datsview != 0) - (topline_datsview + r_visible_lines != totallines);
	}
}

//-------------------------------------------------
//  draw right panel
//-------------------------------------------------

void ui_menu_select_game::draw_right_panel(void *selectedref, float origx1, float origy1, float origx2, float origy2)
{
	ui_manager &mui = machine().ui();
	rgb_t fgcolor = UI_TEXT_COLOR;
	bool hide = (ui_globals::panels_status == HIDE_RIGHT_PANEL || ui_globals::panels_status == HIDE_BOTH);
	float x2 = (hide) ? origx2 : origx1 + 2.0f * UI_BOX_LR_BORDER;
	float space = x2 - origx1;
	float lr_arrow_width = 0.4f * space * machine().render().ui_aspect();

	// set left-right arrows dimension
	float ar_x0 = 0.5f * (x2 + origx1) - 0.5f * lr_arrow_width;
	float ar_y0 = 0.5f * (origy2 + origy1) + 0.1f * space;
	float ar_x1 = ar_x0 + lr_arrow_width;
	float ar_y1 = 0.5f * (origy2 + origy1) + 0.9f * space;

	//machine().ui().draw_outlined_box(container, origx1, origy1, origx2, origy2, UI_BACKGROUND_COLOR);
	mui.draw_outlined_box(container, origx1, origy1, origx2, origy2, rgb_t(0xEF, 0x12, 0x47, 0x7B));

	if (mouse_hit && origx1 <= mouse_x && x2 > mouse_x && origy1 <= mouse_y && origy2 > mouse_y)
	{
		fgcolor = UI_MOUSEOVER_COLOR;
		hover = HOVER_RPANEL_ARROW;
	}

	if (hide)
	{
		draw_arrow(container, ar_x0, ar_y0, ar_x1, ar_y1, fgcolor, ROT90 ^ ORIENTATION_FLIP_X);
		return;
	}

	draw_arrow(container, ar_x0, ar_y0, ar_x1, ar_y1, fgcolor, ROT90);
	origx1 = x2;
	origy1 = draw_right_box_title(origx1, origy1, origx2, origy2);

	if (ui_globals::rpanel == RP_IMAGES)
		arts_render(selectedref, origx1, origy1, origx2, origy2);
	else
		infos_render(selectedref, origx1, origy1, origx2, origy2);
}

//-------------------------------------------------
//  perform our special rendering
//-------------------------------------------------

void ui_menu_select_game::arts_render(void *selectedref, float origx1, float origy1, float origx2, float origy2)
{
	ui_manager &mui = machine().ui();
	float line_height = mui.get_line_height();
	bool is_favorites = ((item[0].flags & MENU_FLAG_UI_FAVORITE) != 0);
	static ui_software_info *oldsoft = nullptr;
	static const game_driver *olddriver = nullptr;
	const game_driver *driver = nullptr;
	ui_software_info *soft = nullptr;

	if (is_favorites)
	{
		soft = ((FPTR)selectedref > skip_main_items) ? (ui_software_info *)selectedref : ((m_prev_selected != nullptr) ? (ui_software_info *)m_prev_selected : nullptr);
		if (soft && soft->startempty == 1)
		{
			driver = soft->driver;
			oldsoft = nullptr;
		}
		else
			olddriver = nullptr;
	}
	else
	{
		driver = ((FPTR)selectedref > skip_main_items) ? (const game_driver *)selectedref : ((m_prev_selected != nullptr) ? (const game_driver *)m_prev_selected : nullptr);
		oldsoft = nullptr;
	}

	if (driver != nullptr)
	{
		if (ui_globals::default_image)
			((driver->flags & MACHINE_TYPE_ARCADE) == 0) ? ui_globals::curimage_view = CABINETS_VIEW : ui_globals::curimage_view = SNAPSHOT_VIEW;

		std::string searchstr;
		searchstr = arts_render_common(origx1, origy1, origx2, origy2);

		// loads the image if necessary
		if (driver != olddriver || !snapx_bitmap->valid() || ui_globals::switch_image)
		{
			emu_file snapfile(searchstr.c_str(), OPEN_FLAG_READ);
			snapfile.set_restrict_to_mediapath(true);
			bitmap_argb32 *tmp_bitmap;
			tmp_bitmap = auto_alloc(machine(), bitmap_argb32);

			// try to load snapshot first from saved "0000.png" file
			std::string fullname(driver->name);
			render_load_png(*tmp_bitmap, snapfile, fullname.c_str(), "0000.png");

			if (!tmp_bitmap->valid())
				render_load_jpeg(*tmp_bitmap, snapfile, fullname.c_str(), "0000.jpg");

			// if fail, attemp to load from standard file
			if (!tmp_bitmap->valid())
			{
				fullname.assign(driver->name).append(".png");
				render_load_png(*tmp_bitmap, snapfile, nullptr, fullname.c_str());

				if (!tmp_bitmap->valid())
				{
					fullname.assign(driver->name).append(".jpg");
					render_load_jpeg(*tmp_bitmap, snapfile, nullptr, fullname.c_str());
				}
			}

			// if fail again, attemp to load from parent file
			if (!tmp_bitmap->valid())
			{
				// set clone status
				bool cloneof = strcmp(driver->parent, "0");
				if (cloneof)
				{
					int cx = driver_list::find(driver->parent);
					if (cx != -1 && ((driver_list::driver(cx).flags & MACHINE_IS_BIOS_ROOT) != 0))
						cloneof = false;
				}

				if (cloneof)
				{
					fullname.assign(driver->parent).append(".png");
					render_load_png(*tmp_bitmap, snapfile, nullptr, fullname.c_str());

					if (!tmp_bitmap->valid())
					{
						fullname.assign(driver->parent).append(".jpg");
						render_load_jpeg(*tmp_bitmap, snapfile, nullptr, fullname.c_str());
					}
				}
			}

			olddriver = driver;
			ui_globals::switch_image = false;
			arts_render_images(tmp_bitmap, origx1, origy1, origx2, origy2, false);
			auto_free(machine(), tmp_bitmap);
		}

		// if the image is available, loaded and valid, display it
		if (snapx_bitmap->valid())
		{
			float x1 = origx1 + 0.01f;
			float x2 = origx2 - 0.01f;
			float y1 = origy1 + UI_BOX_TB_BORDER + line_height;
			float y2 = origy2 - UI_BOX_TB_BORDER - line_height;

			// apply texture
			container->add_quad( x1, y1, x2, y2, ARGB_WHITE, snapx_texture, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
		}
	}
	else if (soft != nullptr)
	{
		std::string fullname, pathname;

		if (ui_globals::default_image)
			(soft->startempty == 0) ? ui_globals::curimage_view = SNAPSHOT_VIEW : ui_globals::curimage_view = CABINETS_VIEW;

		// arts title and searchpath
		std::string searchstr;
		searchstr = arts_render_common(origx1, origy1, origx2, origy2);

		// loads the image if necessary
		if (soft != oldsoft || !snapx_bitmap->valid() || ui_globals::switch_image)
		{
			emu_file snapfile(searchstr.c_str(), OPEN_FLAG_READ);
			bitmap_argb32 *tmp_bitmap;
			tmp_bitmap = auto_alloc(machine(), bitmap_argb32);

			if (soft->startempty == 1)
			{
				// Load driver snapshot
				fullname.assign(soft->driver->name).append(".png");
				render_load_png(*tmp_bitmap, snapfile, nullptr, fullname.c_str());

				if (!tmp_bitmap->valid())
				{
					fullname.assign(soft->driver->name).append(".jpg");
					render_load_jpeg(*tmp_bitmap, snapfile, nullptr, fullname.c_str());
				}
			}
			else if (ui_globals::curimage_view == TITLES_VIEW)
			{
				// First attempt from name list
				pathname.assign(soft->listname).append("_titles");
				fullname.assign(soft->shortname).append(".png");
				render_load_png(*tmp_bitmap, snapfile, pathname.c_str(), fullname.c_str());

				if (!tmp_bitmap->valid())
				{
					fullname.assign(soft->shortname).append(".jpg");
					render_load_jpeg(*tmp_bitmap, snapfile, pathname.c_str(), fullname.c_str());
				}
			}
			else
			{
				// First attempt from name list
				pathname = soft->listname;
				fullname.assign(soft->shortname).append(".png");
				render_load_png(*tmp_bitmap, snapfile, pathname.c_str(), fullname.c_str());

				if (!tmp_bitmap->valid())
				{
					fullname.assign(soft->shortname).append(".jpg");
					render_load_jpeg(*tmp_bitmap, snapfile, pathname.c_str(), fullname.c_str());
				}

				if (!tmp_bitmap->valid())
				{
					// Second attempt from driver name + part name
					pathname.assign(soft->driver->name).append(soft->part.c_str());
					fullname.assign(soft->shortname).append(".png");
					render_load_png(*tmp_bitmap, snapfile, pathname.c_str(), fullname.c_str());

					if (!tmp_bitmap->valid())
					{
						fullname.assign(soft->shortname).append(".jpg");
						render_load_jpeg(*tmp_bitmap, snapfile, pathname.c_str(), fullname.c_str());
					}
				}
			}

			oldsoft = soft;
			ui_globals::switch_image = false;
			arts_render_images(tmp_bitmap, origx1, origy1, origx2, origy2, true);
			auto_free(machine(), tmp_bitmap);
		}

		// if the image is available, loaded and valid, display it
		if (snapx_bitmap->valid())
		{
			float x1 = origx1 + 0.01f;
			float x2 = origx2 - 0.01f;
			float y1 = origy1 + UI_BOX_TB_BORDER + line_height;
			float y2 = origy2 - UI_BOX_TB_BORDER - line_height;

			// apply texture
			container->add_quad(x1, y1, x2, y2, ARGB_WHITE, snapx_texture, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
		}
	}
}
