// license:BSD-3-Clause
// copyright-holders:Maurizio Petrarota, Vas Crabb
/*********************************************************************

    ui/selgame.cpp

    Main UI menu.

*********************************************************************/

#include "emu.h"
#include "ui/selgame.h"

#include "ui/auditmenu.h"
#include "ui/icorender.h"
#include "ui/inifile.h"
#include "ui/miscmenu.h"
#include "ui/optsmenu.h"
#include "ui/selector.h"
#include "ui/selsoft.h"
#include "ui/systemlist.h"
#include "ui/ui.h"

#include "infoxml.h"
#include "luaengine.h"
#include "mame.h"

#include "corestr.h"
#include "drivenum.h"
#include "emuopts.h"
#include "fileio.h"
#include "rendutil.h"
#include "romload.h"
#include "softlist_dev.h"
#include "uiinput.h"
#include "unicode.h"

#include <cstring>
#include <iterator>
#include <memory>


extern const char UI_VERSION_TAG[];

namespace ui {

bool menu_select_game::s_first_start = true;


//-------------------------------------------------
//  ctor
//-------------------------------------------------

menu_select_game::menu_select_game(mame_ui_manager &mui, render_container &container, const char *gamename)
	: menu_select_launch(mui, container, false)
	, m_persistent_data(system_list::instance())
	, m_icons(MAX_ICONS_RENDER)
	, m_icon_paths()
	, m_displaylist()
	, m_searchlist()
	, m_searched_fields(system_list::AVAIL_NONE)
	, m_populated_favorites(false)
{
	std::string error_string, last_filter, sub_filter;
	ui_options &moptions = mui.options();

	// load drivers cache
	m_persistent_data.cache_data(mui.options());

	// check if there are available system icons
	check_for_icons(nullptr);

	// build drivers list
	if (!load_available_machines())
		build_available_list();

	if (s_first_start)
	{
		//s_first_start = false; TODO: why wasn't it ever clearing the first start flag?
		reselect_last::set_driver(moptions.last_used_machine());

		std::string tmp(moptions.last_used_filter());
		std::size_t const found = tmp.find_first_of(',');
		std::string fake_ini;
		if (found == std::string::npos)
		{
			fake_ini = util::string_format(u8"\uFEFF%s = 1\n", tmp);
		}
		else
		{
			std::string const sub_filter(tmp.substr(found + 1));
			tmp.resize(found);
			fake_ini = util::string_format(u8"\uFEFF%s = %s\n", tmp, sub_filter);
		}

		util::core_file::ptr file;
		if (!util::core_file::open_ram(fake_ini.c_str(), fake_ini.size(), OPEN_FLAG_READ, file))
			m_persistent_data.filter_data().load_ini(*file);
	}

	// do this after processing the last used filter setting so it overwrites the placeholder
	load_custom_filters();
	m_filter_highlight = m_persistent_data.filter_data().get_current_filter_type();

	if (!moptions.remember_last())
		reselect_last::reset();

	mui.machine().options().set_value(OPTION_SNAPNAME, "%g/%i", OPTION_PRIORITY_CMDLINE);

	// restore last right panel settings
	set_right_panel(moptions.system_right_panel());
	set_right_image(moptions.system_right_image());

	ui_globals::curdats_view = 0;
	ui_globals::curdats_total = 1;
}

//-------------------------------------------------
//  dtor
//-------------------------------------------------

menu_select_game::~menu_select_game()
{
	// TODO: reconsider when to do this
	ui().save_ui_options();
}


//-------------------------------------------------
//  recompute metrics
//-------------------------------------------------

void menu_select_game::recompute_metrics(uint32_t width, uint32_t height, float aspect)
{
	menu_select_launch::recompute_metrics(width, height, aspect);

	m_icons.clear();

	// configure the custom rendering
	set_custom_space(3.0F * line_height() + 5.0F * tb_border(), 4.0F * line_height() + 3.0F * tb_border());
}


//-------------------------------------------------
//  menu_activated
//-------------------------------------------------

void menu_select_game::menu_activated()
{
	menu_select_launch::menu_activated();

	// if I have to load datfile, perform a hard reset
	if (ui_globals::reset)
	{
		// dumb workaround for not being able to add an exit notifier
		struct cache_reset { ~cache_reset() { system_list::instance().reset_cache(); } };
		ui().get_session_data<cache_reset, cache_reset>();

		ui_globals::reset = false;
		machine().schedule_hard_reset();
		stack_reset();
	}
}

//-------------------------------------------------
//  menu_deactivated
//-------------------------------------------------

void menu_select_game::menu_deactivated()
{
	menu_select_launch::menu_deactivated();

	// get the "last selected system" string
	ui_system_info const *system;
	ui_software_info const *swinfo;
	get_selection(swinfo, system);
	std::string last_driver;
	if (swinfo)
		last_driver = swinfo->shortname;
	else if (system)
		last_driver = system->driver->name;

	// serialise the selected filter settings
	std::string const filter(m_persistent_data.filter_data().get_config_string());

	ui_options &mopt = ui().options();
	mopt.set_value(OPTION_LAST_USED_MACHINE,  last_driver,                 OPTION_PRIORITY_CMDLINE);
	mopt.set_value(OPTION_LAST_USED_FILTER,   filter,                      OPTION_PRIORITY_CMDLINE);
	mopt.set_value(OPTION_SYSTEM_RIGHT_PANEL, right_panel_config_string(), OPTION_PRIORITY_CMDLINE);
	mopt.set_value(OPTION_SYSTEM_RIGHT_IMAGE, right_image_config_string(), OPTION_PRIORITY_CMDLINE);
}

//-------------------------------------------------
//  handle
//-------------------------------------------------

bool menu_select_game::handle(event const *ev)
{
	if (!m_prev_selected && (item_count() > 0))
		m_prev_selected = item(0).ref();

	// if I have to select software, force software list submenu
	if (reselect_last::get())
	{
		// FIXME: this is never hit, need a better way to return to software selection if necessary
		const ui_system_info *system;
		const ui_software_info *software;
		get_selection(software, system);
		menu::stack_push<menu_select_software>(ui(), container(), *system);
		return false;
	}

	// FIXME: everything above here used to run before events were processed

	// process the menu
	bool changed = false;
	if (ev)
	{
		if (dismiss_error())
		{
			// reset the error on any subsequent menu event
			changed = true;
		}
		else switch (ev->iptkey)
		{
		case IPT_UI_EXPORT:
			inkey_export();
			break;

		case IPT_UI_DATS:
			inkey_dats();
			break;

		default:
			if (ev->itemref)
			{
				switch (ev->iptkey)
				{
				case IPT_UI_SELECT:
					if (get_focus() == focused_menu::MAIN)
					{
						if (m_populated_favorites)
							changed = inkey_select_favorite(ev);
						else
							changed = inkey_select(ev);
					}
					break;

				case IPT_UI_FAVORITES:
					if (uintptr_t(ev->itemref) > m_skip_main_items)
					{
						favorite_manager &mfav(mame_machine_manager::instance()->favorite());
						if (!m_populated_favorites)
						{
							auto const &info(*reinterpret_cast<ui_system_info const *>(ev->itemref));
							auto const &driver(*info.driver);
							if (!mfav.is_favorite_system(driver))
							{
								mfav.add_favorite_system(driver);
								machine().popmessage(_("%s\n added to favorites list."), info.description);
							}
							else
							{
								mfav.remove_favorite_system(driver);
								machine().popmessage(_("%s\n removed from favorites list."), info.description);
							}
							changed = true;
						}
						else
						{
							ui_software_info const *const swinfo(reinterpret_cast<ui_software_info const *>(ev->itemref));
							machine().popmessage(_("%s\n removed from favorites list."), swinfo->longname);
							mfav.remove_favorite_software(*swinfo);
							reset(reset_options::SELECT_FIRST);
						}
					}
					break;

				case IPT_UI_AUDIT:
					menu::stack_push<menu_audit>(ui(), container());
					break;
				}
			}
		}
	}

	return changed;
}

//-------------------------------------------------
//  populate
//-------------------------------------------------

void menu_select_game::populate()
{
	for (auto &icon : m_icons) // TODO: why is this here?  maybe better on resize or setting change?
		icon.second.texture.reset();

	set_switch_image();

	bool have_prev_selected = false;
	int old_item_selected = -1;
	if (!isfavorite())
	{
		if (m_populated_favorites)
			m_prev_selected = nullptr;
		m_populated_favorites = false;
		m_displaylist.clear();
		machine_filter const *const flt(m_persistent_data.filter_data().get_current_filter());

		// if search is not empty, find approximate matches
		if (!m_search.empty())
		{
			populate_search();
			if (flt)
			{
				for (auto it = m_searchlist.begin(); (m_searchlist.end() != it) && (MAX_VISIBLE_SEARCH > m_displaylist.size()); ++it)
				{
					if (flt->apply(it->second))
						m_displaylist.emplace_back(it->second);
				}
			}
			else
			{
				std::transform(
						m_searchlist.begin(),
						std::next(m_searchlist.begin(), (std::min)(m_searchlist.size(), MAX_VISIBLE_SEARCH)),
						std::back_inserter(m_displaylist),
						[] (auto const &entry) { return entry.second; });
			}
		}
		else
		{
			// if filter is set on category, build category list
			auto const &sorted(m_persistent_data.sorted_list());
			if (!flt)
			{
				for (ui_system_info const &sysinfo : sorted)
					m_displaylist.emplace_back(sysinfo);
			}
			else
			{
				for (ui_system_info const &sysinfo : sorted)
				{
					if (flt->apply(sysinfo))
						m_displaylist.emplace_back(sysinfo);
				}
			}
		}

		// iterate over entries
		int curitem = 0;
		for (ui_system_info const &elem : m_displaylist)
		{
			have_prev_selected = have_prev_selected || (&elem == m_prev_selected);
			if ((old_item_selected == -1) && (elem.driver->name == reselect_last::driver()))
				old_item_selected = curitem;

			item_append(elem.description, elem.is_clone ? FLAG_INVERT : 0, (void *)&elem);
			curitem++;
		}
	}
	else
	{
		// populate favorites list
		if (!m_populated_favorites)
			m_prev_selected = nullptr;
		m_populated_favorites = true;
		m_search.clear();
		mame_machine_manager::instance()->favorite().apply_sorted(
				[this, &have_prev_selected, &old_item_selected, curitem = 0] (ui_software_info const &info) mutable
				{
					have_prev_selected = have_prev_selected || (&info == m_prev_selected);
					if (info.startempty)
					{
						if (old_item_selected == -1 && info.shortname == reselect_last::driver())
							old_item_selected = curitem;

						bool cloneof = strcmp(info.driver->parent, "0");
						if (cloneof)
						{
							int const cx = driver_list::find(info.driver->parent);
							if ((0 <= cx) && ((driver_list::driver(cx).flags & machine_flags::IS_BIOS_ROOT) != 0))
								cloneof = false;
						}

						ui_system_info const &sysinfo = m_persistent_data.systems()[driver_list::find(info.driver->name)];
						item_append(sysinfo.description, cloneof ? FLAG_INVERT : 0, (void *)&info);
					}
					else
					{
						if (old_item_selected == -1 && info.shortname == reselect_last::driver())
							old_item_selected = curitem;
						item_append(info.longname, info.devicetype, info.parentname.empty() ? 0 : FLAG_INVERT, (void *)&info);
					}
					curitem++;
				});
	}

	// add special items
	if (stack_has_special_main_menu())
	{
		item_append(menu_item_type::SEPARATOR, 0);
		item_append(_("General Settings"), 0, (void *)(uintptr_t)CONF_OPTS);
		item_append(_("System Settings"), 0, (void *)(uintptr_t)CONF_MACHINE);
		m_skip_main_items = 3;

		if (m_prev_selected && !have_prev_selected && (item_count() > 0))
			m_prev_selected = item(0).ref();
	}
	else
	{
		m_skip_main_items = 0;
	}

	// reselect prior game launched, if any
	if (old_item_selected != -1)
	{
		set_selected_index(old_item_selected);
		centre_selection();

		if (reselect_last::software().empty())
			reselect_last::reset();
	}
	else
	{
		reselect_last::reset();
	}
}

//-------------------------------------------------
//  build a list of available drivers
//-------------------------------------------------

void menu_select_game::build_available_list()
{
	std::size_t const total = driver_list::total();
	std::vector<bool> included(total, false);

	// iterate over ROM directories and look for potential ROMs
	file_enumerator path(machine().options().media_path());
	for (osd::directory::entry const *dir = path.next(); dir; dir = path.next())
	{
		char drivername[50];
		char *dst = drivername;
		char const *src;

		// build a name for it
		for (src = dir->name; *src != 0 && *src != '.' && dst < &drivername[std::size(drivername) - 1]; ++src)
			*dst++ = tolower(uint8_t(*src));

		*dst = 0;
		int const drivnum = driver_list::find(drivername);
		if (0 <= drivnum)
			included[drivnum] = true;
	}

	// now check and include NONE_NEEDED
	if (!ui().options().hide_romless())
	{
		// FIXME: can't use the convenience macros with tiny ROM entries
		auto const is_required_rom =
				[] (tiny_rom_entry const &rom) { return ROMENTRY_ISFILE(rom) && !ROM_ISOPTIONAL(rom) && !std::strchr(rom.hashdata, '!'); };
		for (std::size_t x = 0; total > x; ++x)
		{
			game_driver const &driver(driver_list::driver(x));
			if (!included[x] && (&GAME_NAME(___empty) != &driver))
			{
				bool noroms(true);
				tiny_rom_entry const *rom;
				for (rom = driver.rom; !ROMENTRY_ISEND(rom); ++rom)
				{
					// check optional and NO_DUMP
					if (is_required_rom(*rom))
					{
						noroms = false;
						break; // break before incrementing, or it will subtly break the check for all ROMs belonging to parent
					}
				}

				if (!noroms)
				{
					// check if clone == parent
					auto const cx(driver_list::clone(driver));
					if ((0 <= cx) && included[cx])
					{
						game_driver const &parent(driver_list::driver(cx));
						if (driver.rom == parent.rom)
						{
							noroms = true;
						}
						else
						{
							// check if clone < parent
							noroms = true;
							for ( ; noroms && !ROMENTRY_ISEND(rom); ++rom)
							{
								if (is_required_rom(*rom))
								{
									util::hash_collection const hashes(rom->hashdata);

									bool found(false);
									for (tiny_rom_entry const *parentrom = parent.rom; !found && !ROMENTRY_ISEND(parentrom); ++parentrom)
									{
										if (is_required_rom(*parentrom) && (rom->length == parentrom->length))
										{
											util::hash_collection const parenthashes(parentrom->hashdata);
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
					included[x] = true;
			}
		}
	}

	// copy into the persistent sorted list
	for (ui_system_info &info : m_persistent_data.sorted_list())
		info.available = included[info.index];
}


//-------------------------------------------------
//  force the game select menu to be visible
//  and inescapable
//-------------------------------------------------

void menu_select_game::force_game_select(mame_ui_manager &mui, render_container &container)
{
	// drop any existing menus and start the system selection menu
	menu::stack_reset(mui);
	menu::stack_push_special_main<menu_select_game>(mui, container, nullptr);
	mui.show_menu();

	// make sure MAME is paused
	mui.machine().pause();
}

//-------------------------------------------------
//  handle select key event
//-------------------------------------------------

bool menu_select_game::inkey_select(const event *menu_event)
{
	auto const system = reinterpret_cast<ui_system_info const *>(menu_event->itemref);

	if (uintptr_t(system) == CONF_OPTS)
	{
		// special case for configure options
		menu::stack_push<menu_game_options>(
				ui(),
				container(),
				m_persistent_data.filter_data(),
				[this] () { reset(reset_options::SELECT_FIRST); });
		return false;
	}
	else if (uintptr_t(system) == CONF_MACHINE)
	{
		// special case for configure machine
		if (m_prev_selected)
			menu::stack_push<menu_machine_configure>(ui(), container(), *reinterpret_cast<const ui_system_info *>(m_prev_selected));
		return false;
	}
	else
	{
		// anything else is a driver
		driver_enumerator enumerator(machine().options(), *system->driver);
		enumerator.next();

		// if there are software entries, show a software selection menu
		for (software_list_device &swlistdev : software_list_device_enumerator(enumerator.config()->root_device()))
		{
			if (!swlistdev.get_info().empty())
			{
				menu::stack_push<menu_select_software>(ui(), container(), *system);
				return false;
			}
		}

		// audit the system ROMs first to see if we're going to work
		media_auditor auditor(enumerator);
		media_auditor::summary const summary = auditor.audit_media(AUDIT_VALIDATE_FAST);

		// if everything looks good, schedule the new driver
		if (audit_passed(summary))
		{
			if (!select_bios(*system->driver, false))
				launch_system(*system->driver);
			return false;
		}
		else
		{
			// otherwise, display an error
			set_error(reset_options::REMEMBER_REF, make_system_audit_fail_text(auditor, summary));
			return true;
		}
	}
}

//-------------------------------------------------
//  handle select key event for favorites menu
//-------------------------------------------------

bool menu_select_game::inkey_select_favorite(const event *menu_event)
{
	ui_software_info *ui_swinfo = (ui_software_info *)menu_event->itemref;

	if ((uintptr_t)ui_swinfo == CONF_OPTS)
	{
		// special case for configure options
		menu::stack_push<menu_game_options>(
				ui(),
				container(),
				m_persistent_data.filter_data(),
				[this] () { reset(reset_options::SELECT_FIRST); });
		return false;
	}
	else if ((uintptr_t)ui_swinfo == CONF_MACHINE)
	{
		// special case for configure machine
		if (m_prev_selected)
		{
			ui_software_info *swinfo = reinterpret_cast<ui_software_info *>(m_prev_selected);
			ui_system_info const &sysinfo = m_persistent_data.systems()[driver_list::find(swinfo->driver->name)];
			menu::stack_push<menu_machine_configure>(
					ui(),
					container(),
					sysinfo,
					[this, empty = swinfo->startempty] (bool fav, bool changed)
					{
						if (changed)
							reset(empty ? reset_options::SELECT_FIRST : reset_options::REMEMBER_REF);
					});
		}
		return false;
	}
	else if (ui_swinfo->startempty)
	{
		driver_enumerator enumerator(machine().options(), *ui_swinfo->driver);
		enumerator.next();

		// if there are software entries, show a software selection menu
		for (software_list_device &swlistdev : software_list_device_enumerator(enumerator.config()->root_device()))
		{
			if (!swlistdev.get_info().empty())
			{
				ui_system_info const &system(m_persistent_data.systems()[driver_list::find(ui_swinfo->driver->name)]);
				menu::stack_push<menu_select_software>(ui(), container(), system);
				return false;
			}
		}

		// audit the system ROMs first to see if we're going to work
		media_auditor auditor(enumerator);
		media_auditor::summary const summary = auditor.audit_media(AUDIT_VALIDATE_FAST);

		if (audit_passed(summary))
		{
			// if everything looks good, schedule the new driver
			if (!select_bios(*ui_swinfo->driver, false))
			{
				reselect_last::reselect(true);
				launch_system(*ui_swinfo->driver);
			}
			return false;
		}
		else
		{
			// otherwise, display an error
			set_error(reset_options::REMEMBER_REF, make_system_audit_fail_text(auditor, summary));
			return true;
		}
	}
	else
	{
		// first audit the system ROMs
		driver_enumerator drv(machine().options(), *ui_swinfo->driver);
		media_auditor auditor(drv);
		drv.next();
		media_auditor::summary const sysaudit = auditor.audit_media(AUDIT_VALIDATE_FAST);
		if (!audit_passed(sysaudit))
		{
			set_error(reset_options::REMEMBER_REF, make_system_audit_fail_text(auditor, sysaudit));
			return true;
		}
		else
		{
			// now audit the software
			software_list_device *swlist = software_list_device::find_by_name(*drv.config(), ui_swinfo->listname);
			const software_info *swinfo = swlist->find(ui_swinfo->shortname);

			media_auditor::summary const swaudit = auditor.audit_software(*swlist, *swinfo, AUDIT_VALIDATE_FAST);

			if (audit_passed(swaudit))
			{
				reselect_last::reselect(true);
				if (!select_bios(*ui_swinfo, false) && !select_part(*swinfo, *ui_swinfo))
					launch_system(drv.driver(), *ui_swinfo, ui_swinfo->part);
				return false;
			}
			else
			{
				// otherwise, display an error
				set_error(reset_options::REMEMBER_REF, make_software_audit_fail_text(auditor, swaudit));
				return true;
			}
		}
	}
}

//-------------------------------------------------
//  returns if the search can be activated
//-------------------------------------------------

bool menu_select_game::isfavorite() const
{
	return machine_filter::FAVORITE == m_persistent_data.filter_data().get_current_filter_type();
}


//-------------------------------------------------
//  populate search list
//-------------------------------------------------

void menu_select_game::populate_search()
{
	// ensure search list is populated
	if (m_searchlist.empty())
	{
		auto const &sorted(m_persistent_data.sorted_list());
		m_searchlist.reserve(sorted.size());
		for (ui_system_info const &info : sorted)
			m_searchlist.emplace_back(1.0, std::ref(info));
	}

	// keep track of what we matched against
	const std::u32string ucs_search(ustr_from_utf8(normalize_unicode(m_search, unicode_normalization_form::D, true)));

	// check available search data
	if (m_persistent_data.is_available(system_list::AVAIL_UCS_SHORTNAME))
		m_searched_fields |= system_list::AVAIL_UCS_SHORTNAME;
	if (m_persistent_data.is_available(system_list::AVAIL_UCS_DESCRIPTION))
		m_searched_fields |= system_list::AVAIL_UCS_DESCRIPTION;
	if (m_persistent_data.is_available(system_list::AVAIL_UCS_MANUF_DESC))
		m_searched_fields |= system_list::AVAIL_UCS_MANUF_DESC;
	if (m_persistent_data.is_available(system_list::AVAIL_UCS_DFLT_DESC))
		m_searched_fields |= system_list::AVAIL_UCS_DFLT_DESC;
	if (m_persistent_data.is_available(system_list::AVAIL_UCS_MANUF_DFLT_DESC))
		m_searched_fields |= system_list::AVAIL_UCS_MANUF_DFLT_DESC;

	for (std::pair<double, std::reference_wrapper<ui_system_info const> > &info : m_searchlist)
	{
		info.first = 1.0;
		ui_system_info const &sys(info.second);

		// match shortnames
		if (m_searched_fields & system_list::AVAIL_UCS_SHORTNAME)
			info.first = util::edit_distance(ucs_search, sys.ucs_shortname);

		// match reading
		if (info.first && !sys.ucs_reading_description.empty())
		{
			info.first = (std::min)(util::edit_distance(ucs_search, sys.ucs_reading_description), info.first);

			// match "<manufacturer> <reading>"
			if (info.first)
				info.first = (std::min)(util::edit_distance(ucs_search, sys.ucs_manufacturer_reading_description), info.first);
		}

		// match descriptions
		if (info.first && (m_searched_fields & system_list::AVAIL_UCS_DESCRIPTION))
			info.first = (std::min)(util::edit_distance(ucs_search, sys.ucs_description), info.first);

		// match "<manufacturer> <description>"
		if (info.first && (m_searched_fields & system_list::AVAIL_UCS_MANUF_DESC))
			info.first = (std::min)(util::edit_distance(ucs_search, sys.ucs_manufacturer_description), info.first);

		// match default description
		if (info.first && (m_searched_fields & system_list::AVAIL_UCS_DFLT_DESC) && !sys.ucs_default_description.empty())
		{
			info.first = (std::min)(util::edit_distance(ucs_search, sys.ucs_default_description), info.first);

			// match "<manufacturer> <default description>"
			if (info.first && (m_searched_fields & system_list::AVAIL_UCS_MANUF_DFLT_DESC))
				info.first = (std::min)(util::edit_distance(ucs_search, sys.ucs_manufacturer_default_description), info.first);
		}
	}

	// sort according to edit distance
	std::stable_sort(
			m_searchlist.begin(),
			m_searchlist.end(),
			[] (auto const &lhs, auto const &rhs) { return lhs.first < rhs.first; });
}


//-------------------------------------------------
//  get (possibly cached) icon texture
//-------------------------------------------------

render_texture *menu_select_game::get_icon_texture(int linenum, void *selectedref)
{
	game_driver const *const driver(m_populated_favorites
			? reinterpret_cast<ui_software_info const *>(selectedref)->driver
			: reinterpret_cast<ui_system_info const *>(selectedref)->driver);
	assert(driver);

	icon_cache::iterator icon(m_icons.find(driver));
	if ((m_icons.end() == icon) || !icon->second.texture)
	{
		if (m_icon_paths.empty())
			m_icon_paths = make_icon_paths(nullptr);

		// allocate an entry or allocate a texture on forced redraw
		if (m_icons.end() == icon)
		{
			icon = m_icons.emplace(driver, texture_ptr(machine().render().texture_alloc(), machine().render())).first;
		}
		else
		{
			assert(!icon->second.texture);
			icon->second.texture.reset(machine().render().texture_alloc());
		}

		// set clone status
		bool cloneof = strcmp(driver->parent, "0");
		if (cloneof)
		{
			auto cx = driver_list::find(driver->parent);
			if ((cx >= 0) && (driver_list::driver(cx).flags & machine_flags::IS_BIOS_ROOT))
				cloneof = false;
		}

		bitmap_argb32 tmp;
		emu_file snapfile(std::string(m_icon_paths), OPEN_FLAG_READ);
		if (!snapfile.open(std::string(driver->name) + ".ico"))
		{
			render_load_ico_highest_detail(snapfile, tmp);
			snapfile.close();
		}
		if (!tmp.valid() && cloneof && !snapfile.open(std::string(driver->parent) + ".ico"))
		{
			render_load_ico_highest_detail(snapfile, tmp);
			snapfile.close();
		}

		scale_icon(std::move(tmp), icon->second);
	}

	return icon->second.bitmap.valid() ? icon->second.texture.get() : nullptr;
}


//-------------------------------------------------
//  export displayed list
//-------------------------------------------------

void menu_select_game::inkey_export()
{
	std::vector<game_driver const *> list;
	if (m_populated_favorites)
	{
		// iterate over favorites
		mame_machine_manager::instance()->favorite().apply(
				[&list] (ui_software_info const &info)
				{
					assert(info.driver);
					if (info.startempty)
						list.push_back(info.driver);
				});
	}
	else
	{
		list.reserve(m_displaylist.size());
		for (ui_system_info const &info : m_displaylist)
			list.emplace_back(info.driver);
	}

	menu::stack_push<menu_export>(ui(), container(), std::move(list));
}

//-------------------------------------------------
//  load drivers infos from file
//-------------------------------------------------

bool menu_select_game::load_available_machines()
{
	// try to load available drivers from file
	emu_file file(ui().options().ui_path(), OPEN_FLAG_READ);
	if (file.open(std::string(emulator_info::get_configname()) + "_avail.ini"))
		return false;

	char rbuf[MAX_CHAR_INFO];
	std::string readbuf;
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

	// load available list
	std::unordered_set<std::string> available;
	while (file.gets(rbuf, MAX_CHAR_INFO))
	{
		readbuf = strtrimspace(rbuf);

		if (readbuf.empty() || ('#' == readbuf[0])) // ignore empty lines and line comments
			;
		else if ('[' == readbuf[0]) // throw out the rest of the file if we find a section heading
			break;
		else
			available.emplace(std::move(readbuf));
	}
	file.close();

	// turn it into the sorted system list we all love
	for (ui_system_info &info : m_persistent_data.sorted_list())
	{
		std::unordered_set<std::string>::iterator const it(available.find(&info.driver->name[0]));
		bool const found(available.end() != it);
		info.available = found;
		if (found)
			available.erase(it);
	}

	return true;
}

//-------------------------------------------------
//  load custom filters info from file
//-------------------------------------------------

void menu_select_game::load_custom_filters()
{
	emu_file file(ui().options().ui_path(), OPEN_FLAG_READ);
	if (!file.open(util::string_format("custom_%s_filter.ini", emulator_info::get_configname())))
	{
		machine_filter::ptr flt(machine_filter::create(file, m_persistent_data.filter_data()));
		if (flt)
			m_persistent_data.filter_data().set_filter(std::move(flt)); // not emplace/insert - could replace bogus filter from ui.ini line
		file.close();
	}

}


//-------------------------------------------------
//  draw left box
//-------------------------------------------------

void menu_select_game::draw_left_panel(u32 flags)
{
	machine_filter_data &filter_data(m_persistent_data.filter_data());
	menu_select_launch::draw_left_panel<machine_filter>(flags, filter_data.get_current_filter_type(), filter_data.get_filters());
}


//-------------------------------------------------
//  get selected software and/or driver
//-------------------------------------------------

void menu_select_game::get_selection(ui_software_info const *&software, ui_system_info const *&system) const
{
	if (m_populated_favorites)
	{
		software = reinterpret_cast<ui_software_info const *>(get_selection_ptr());
		system = software ? &m_persistent_data.systems()[driver_list::find(software->driver->name)] : nullptr;
	}
	else
	{
		software = nullptr;
		system = reinterpret_cast<ui_system_info const *>(get_selection_ptr());
	}
}

void menu_select_game::show_config_menu(int index)
{
	if (!m_populated_favorites)
	{
		menu::stack_push<menu_machine_configure>(
				ui(),
				container(),
				*reinterpret_cast<ui_system_info const *>(item(index).ref()),
				nullptr);
	}
	else
	{
		ui_software_info *sw = reinterpret_cast<ui_software_info *>(item(index).ref());
		ui_system_info const &sys = m_persistent_data.systems()[driver_list::find(sw->driver->name)];
		menu::stack_push<menu_machine_configure>(
				ui(),
				container(),
				sys,
				[this, empty = sw->startempty] (bool fav, bool changed)
				{
					if (changed)
						reset(empty ? reset_options::SELECT_FIRST : reset_options::REMEMBER_REF);
				});
	}
}

void menu_select_game::make_topbox_text(std::string &line0, std::string &line1, std::string &line2) const
{
	line0 = string_format(_("%1$s %2$s ( %3$d / %4$d systems (%5$d BIOS) )"),
			emulator_info::get_appname(),
			bare_build_version,
			m_available_items,
			(driver_list::total() - 1),
			m_persistent_data.bios_count());

	if (m_populated_favorites)
	{
		line1.clear();
	}
	else
	{
		machine_filter const *const it(m_persistent_data.filter_data().get_current_filter());
		char const *const filter(it ? it->filter_text() : nullptr);
		if (filter)
			line1 = string_format(_("%1$s: %2$s - Search: %3$s_"), it->display_name(), filter, m_search);
		else
			line1 = string_format(_("Search: %1$s_"), m_search);
	}

	line2.clear();
}


std::string menu_select_game::make_software_description(ui_software_info const &software, ui_system_info const *system) const
{
	// first line is system
	return string_format(_("System: %1$-.100s"), system->description);
}


void menu_select_game::filter_selected(int index)
{
	assert((machine_filter::FIRST <= index) && (machine_filter::LAST >= index));

	m_persistent_data.filter_data().get_filter(machine_filter::type(index)).show_ui(
			ui(),
			container(),
			[this] (machine_filter &filter)
			{
				set_switch_image();
				machine_filter::type const new_type(filter.get_type());
				if (machine_filter::CUSTOM == new_type)
				{
					emu_file file(ui().options().ui_path(), OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS);
					if (!file.open(util::string_format("custom_%s_filter.ini", emulator_info::get_configname())))
					{
						filter.save_ini(file, 0);
						file.close();
					}
				}
				m_persistent_data.filter_data().set_current_filter_type(new_type);
				reset(reset_options::REMEMBER_REF);
			});
}

} // namespace ui
