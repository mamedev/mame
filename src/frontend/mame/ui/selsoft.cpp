// license:BSD-3-Clause
// copyright-holders:Maurizio Petrarota
/***************************************************************************

    ui/selsoft.cpp

    UI software menu.

***************************************************************************/

#include "emu.h"

#include "ui/selsoft.h"

#include "ui/ui.h"
#include "ui/datmenu.h"
#include "ui/inifile.h"
#include "ui/selector.h"

#include "audit.h"
#include "drivenum.h"
#include "emuopts.h"
#include "mame.h"
#include "rendfont.h"
#include "rendutil.h"
#include "softlist_dev.h"
#include "uiinput.h"
#include "luaengine.h"


namespace ui {

static const char *region_lists[] = { "arab", "arg", "asia", "aus", "aut", "bel", "blr", "bra", "can", "chi", "chn", "cze", "den",
										"ecu", "esp", "euro", "fin", "fra", "gbr", "ger", "gre", "hkg", "hun", "irl", "isr",
										"isv", "ita", "jpn", "kaz", "kor", "lat", "lux", "mex", "ned", "nld", "nor", "nzl",
										"pol", "rus", "slo", "spa", "sui", "swe", "tha", "tpe", "tw", "uk", "ukr", "usa" };

//-------------------------------------------------
//  compares two items in the software list and
//  sort them by parent-clone
//-------------------------------------------------

bool compare_software(ui_software_info a, ui_software_info b)
{
	ui_software_info *x = &a;
	ui_software_info *y = &b;

	bool clonex = !x->parentname.empty();
	bool cloney = !y->parentname.empty();

	if (!clonex && !cloney)
		return (strmakelower(x->longname) < strmakelower(y->longname));

	std::string cx(x->parentlongname), cy(y->parentlongname);

	if (cx.empty())
		clonex = false;

	if (cy.empty())
		cloney = false;

	if (!clonex && !cloney)
		return (strmakelower(x->longname) < strmakelower(y->longname));
	else if (clonex && cloney)
	{
		if (!core_stricmp(x->parentname.c_str(), y->parentname.c_str()) && !core_stricmp(x->instance.c_str(), y->instance.c_str()))
			return (strmakelower(x->longname) < strmakelower(y->longname));
		else
			return (strmakelower(cx) < strmakelower(cy));
	}
	else if (!clonex && cloney)
	{
		if (!core_stricmp(x->shortname.c_str(), y->parentname.c_str()) && !core_stricmp(x->instance.c_str(), y->instance.c_str()))
			return true;
		else
			return (strmakelower(x->longname) < strmakelower(cy));
	}
	else
	{
		if (!core_stricmp(x->parentname.c_str(), y->shortname.c_str()) && !core_stricmp(x->instance.c_str(), y->instance.c_str()))
			return false;
		else
			return (strmakelower(cx) < strmakelower(y->longname));
	}
}

//-------------------------------------------------
//  ctor
//-------------------------------------------------

menu_select_software::menu_select_software(mame_ui_manager &mui, render_container &container, const game_driver *driver)
	: menu_select_launch(mui, container, true)
{
	reselect_last::reselect(false);

	sw_filters::actual = 0;
	highlight = 0;

	m_driver = driver;
	build_software_list();
	load_sw_custom_filters();

	ui_globals::curimage_view = SNAPSHOT_VIEW;
	ui_globals::switch_image = true;
	ui_globals::cur_sw_dats_view = 0;
	ui_globals::cur_sw_dats_total = 1;
}

//-------------------------------------------------
//  dtor
//-------------------------------------------------

menu_select_software::~menu_select_software()
{
	ui_globals::curimage_view = CABINETS_VIEW;
	ui_globals::switch_image = true;
}

//-------------------------------------------------
//  handle
//-------------------------------------------------

void menu_select_software::handle()
{
	if (m_prev_selected == nullptr)
		m_prev_selected = item[0].ref;

	bool check_filter = false;

	// ignore pause keys by swallowing them before we process the menu
	machine().ui_input().pressed(IPT_UI_PAUSE);

	// process the menu
	const event *menu_event = process(PROCESS_LR_REPEAT);

	if (menu_event && menu_event->itemref)
	{
		if (dismiss_error())
		{
			// reset the error on any future event
		}
		else if (menu_event->iptkey == IPT_UI_SELECT)
		{
			// handle selections
			if (get_focus() == focused_menu::main)
			{
				inkey_select(menu_event);
			}
			else if (get_focus() == focused_menu::left)
			{
				l_sw_hover = highlight;
				check_filter = true;
				m_prev_selected = nullptr;
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
			else if (ui_globals::rpanel == RP_INFOS && ui_globals::cur_sw_dats_view > 0)
			{
				// Infos
				ui_globals::cur_sw_dats_view--;
				m_topline_datsview = 0;
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
			else if (ui_globals::rpanel == RP_INFOS && ui_globals::cur_sw_dats_view < ui_globals::cur_sw_dats_total)
			{
				// Infos
				ui_globals::cur_sw_dats_view++;
				m_topline_datsview = 0;
			}
		}
		else if (menu_event->iptkey == IPT_UI_UP_FILTER && highlight > UI_SW_FIRST)
		{
			// handle UI_UP_FILTER
			highlight--;
		}
		else if (menu_event->iptkey == IPT_UI_DOWN_FILTER && highlight < UI_SW_LAST)
		{
			// handle UI_DOWN_FILTER
			highlight++;
		}
		else if (menu_event->iptkey == IPT_UI_DATS)
		{
			// handle UI_DATS
			ui_software_info *ui_swinfo = (ui_software_info *)menu_event->itemref;

			if (ui_swinfo->startempty == 1 && mame_machine_manager::instance()->lua()->call_plugin_check<const char *>("data_list", ui_swinfo->driver->name, true))
				menu::stack_push<menu_dats_view>(ui(), container(), ui_swinfo->driver);
			else if (mame_machine_manager::instance()->lua()->call_plugin_check<const char *>("data_list", std::string(ui_swinfo->shortname).append(1, ',').append(ui_swinfo->listname).c_str()) || !ui_swinfo->usage.empty())
				menu::stack_push<menu_dats_view>(ui(), container(), ui_swinfo);
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
		else if (menu_event->iptkey == IPT_UI_FAVORITES)
		{
			// handle UI_FAVORITES
			ui_software_info *swinfo = (ui_software_info *)menu_event->itemref;

			if ((uintptr_t)swinfo > 2)
			{
				favorite_manager &mfav = mame_machine_manager::instance()->favorite();
				if (!mfav.isgame_favorite(*swinfo))
				{
					mfav.add_favorite_game(*swinfo);
					machine().popmessage(_("%s\n added to favorites list."), swinfo->longname.c_str());
				}

				else
				{
					machine().popmessage(_("%s\n removed from favorites list."), swinfo->longname.c_str());
					mfav.remove_favorite_game();
				}
			}
		}
		else if (menu_event->iptkey == IPT_SPECIAL)
		{
			// typed characters append to the buffer
			inkey_special(menu_event);
		}
		else if (menu_event->iptkey == IPT_OTHER)
		{
			highlight = l_sw_hover;
			check_filter = true;
			m_prev_selected = nullptr;
		}
		else if (menu_event->iptkey == IPT_UI_CONFIGURE)
		{
			inkey_navigation();
		}
	}

	if (menu_event && !menu_event->itemref)
	{
		if (menu_event->iptkey == IPT_UI_CONFIGURE)
		{
			inkey_navigation();
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
			else if (ui_globals::rpanel == RP_INFOS && ui_globals::cur_sw_dats_view > 0)
			{
				// Infos
				ui_globals::cur_sw_dats_view--;
				m_topline_datsview = 0;
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
			else if (ui_globals::rpanel == RP_INFOS && ui_globals::cur_sw_dats_view < ui_globals::cur_sw_dats_total)
			{
				// Infos
				ui_globals::cur_sw_dats_view++;
				m_topline_datsview = 0;
			}
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
		else if (menu_event->iptkey == IPT_UI_UP_FILTER && highlight > UI_SW_FIRST)
		{
			// handle UI_UP_FILTER
			highlight--;
		}
		else if (menu_event->iptkey == IPT_UI_DOWN_FILTER && highlight < UI_SW_LAST)
		{
			// handle UI_DOWN_FILTER
			highlight++;
		}
		else if (menu_event->iptkey == IPT_OTHER && get_focus() == focused_menu::left)
		{
			l_sw_hover = highlight;
			check_filter = true;
			m_prev_selected = nullptr;
		}
	}

	// if we're in an error state, overlay an error message
	draw_error_text();

	// handle filters selection from key shortcuts
	if (check_filter)
	{
		m_search.clear();
		switch (l_sw_hover)
		{
		case UI_SW_REGION:
			menu::stack_push<menu_selector>(ui(), container(), m_filter.region.ui, m_filter.region.actual, menu_selector::SOFTWARE, l_sw_hover);
			break;
		case UI_SW_YEARS:
			menu::stack_push<menu_selector>(ui(), container(), m_filter.year.ui, m_filter.year.actual, menu_selector::SOFTWARE, l_sw_hover);
			break;
		case UI_SW_LIST:
			menu::stack_push<menu_selector>(ui(), container(), m_filter.swlist.description, m_filter.swlist.actual, menu_selector::SOFTWARE, l_sw_hover);
			break;
		case UI_SW_TYPE:
			menu::stack_push<menu_selector>(ui(), container(), m_filter.type.ui, m_filter.type.actual, menu_selector::SOFTWARE, l_sw_hover);
			break;
		case UI_SW_PUBLISHERS:
			menu::stack_push<menu_selector>(ui(), container(), m_filter.publisher.ui, m_filter.publisher.actual, menu_selector::SOFTWARE, l_sw_hover);
			break;
		case UI_SW_CUSTOM:
			sw_filters::actual = l_sw_hover;
			menu::stack_push<menu_swcustom_filter>(ui(), container(), m_driver, m_filter);
			break;
		default:
			sw_filters::actual = l_sw_hover;
			reset(reset_options::SELECT_FIRST);
			break;
		}
	}
}

//-------------------------------------------------
//  populate
//-------------------------------------------------

void menu_select_software::populate(float &customtop, float &custombottom)
{
	uint32_t flags_ui = FLAG_LEFT_ARROW | FLAG_RIGHT_ARROW;
	m_has_empty_start = true;
	int old_software = -1;

	machine_config config(*m_driver, machine().options());
	for (device_image_interface &image : image_interface_iterator(config.root_device()))
		if (image.filename() == nullptr && image.must_be_loaded())
		{
			m_has_empty_start = false;
			break;
		}

	// no active search
	if (m_search.empty())
	{
		// if the device can be loaded empty, add an item
		if (m_has_empty_start)
			item_append("[Start empty]", "", flags_ui, (void *)&m_swinfo[0]);

		m_displaylist.clear();
		m_tmp.clear();

		switch (sw_filters::actual)
		{
			case UI_SW_PUBLISHERS:
				build_list(m_tmp, m_filter.publisher.ui[m_filter.publisher.actual].c_str());
				break;

			case UI_SW_LIST:
				build_list(m_tmp, m_filter.swlist.name[m_filter.swlist.actual].c_str());
				break;

			case UI_SW_YEARS:
				build_list(m_tmp, m_filter.year.ui[m_filter.year.actual].c_str());
				break;

			case UI_SW_TYPE:
				build_list(m_tmp, m_filter.type.ui[m_filter.type.actual].c_str());
				break;

			case UI_SW_REGION:
				build_list(m_tmp, m_filter.region.ui[m_filter.region.actual].c_str());
				break;

			case UI_SW_CUSTOM:
				build_custom();
				break;

			default:
				build_list(m_tmp);
				break;
		}

		// iterate over entries
		for (size_t curitem = 0; curitem < m_displaylist.size(); ++curitem)
		{
			if (reselect_last::software() == "[Start empty]" && !reselect_last::driver().empty())
				old_software = 0;

			else if (m_displaylist[curitem]->shortname == reselect_last::software() && m_displaylist[curitem]->listname == reselect_last::swlist())
				old_software = m_has_empty_start ? curitem + 1 : curitem;

			item_append(m_displaylist[curitem]->longname, m_displaylist[curitem]->devicetype,
						m_displaylist[curitem]->parentname.empty() ? flags_ui : (FLAG_INVERT | flags_ui), (void *)m_displaylist[curitem]);
		}
	}

	else
	{
		find_matches(m_search.c_str(), VISIBLE_GAMES_IN_SEARCH);

		for (int curitem = 0; m_searchlist[curitem] != nullptr; ++curitem)
			item_append(m_searchlist[curitem]->longname, m_searchlist[curitem]->devicetype,
						m_searchlist[curitem]->parentname.empty() ? flags_ui : (FLAG_INVERT | flags_ui),
						(void *)m_searchlist[curitem]);
	}

	item_append(menu_item_type::SEPARATOR, flags_ui);

	// configure the custom rendering
	customtop = 4.0f * ui().get_line_height() + 5.0f * UI_BOX_TB_BORDER;
	custombottom = 5.0f * ui().get_line_height() + 4.0f * UI_BOX_TB_BORDER;

	if (old_software != -1)
	{
		selected = old_software;
		top_line = selected - (ui_globals::visible_sw_lines / 2);
	}

	reselect_last::reset();
}

//-------------------------------------------------
//  build a list of software
//-------------------------------------------------

void menu_select_software::build_software_list()
{
	// add start empty item
	m_swinfo.emplace_back(*m_driver);

	machine_config config(*m_driver, machine().options());

	// iterate thru all software lists
	for (software_list_device &swlist : software_list_device_iterator(config.root_device()))
	{
		m_filter.swlist.name.push_back(swlist.list_name());
		m_filter.swlist.description.push_back(swlist.description());
		for (const software_info &swinfo : swlist.get_info())
		{
			const software_part &part = swinfo.parts().front();
			if (swlist.is_compatible(part) == SOFTWARE_IS_COMPATIBLE)
			{
				const char *instance_name = nullptr;
				const char *type_name = nullptr;
				for (device_image_interface &image : image_interface_iterator(config.root_device()))
				{
					char const *const interface = image.image_interface();
					if (interface && part.matches_interface(interface))
					{
						instance_name = image.instance_name().c_str();
						type_name = image.image_type_name();
						break;
					}
				}
				if (!instance_name || !type_name)
					continue;

				ui_software_info tmpmatches(swinfo, part, *m_driver, swlist.list_name(), instance_name, type_name);

				m_filter.region.set(tmpmatches.longname);
				m_filter.publisher.set(tmpmatches.publisher);
				m_filter.year.set(tmpmatches.year);
				m_filter.type.set(tmpmatches.devicetype);
				m_swinfo.emplace_back(std::move(tmpmatches));
			}
		}
	}
	m_displaylist.resize(m_swinfo.size() + 1);

	// retrieve and set the long name of software for parents
	for (size_t y = 1; y < m_swinfo.size(); ++y)
	{
		if (!m_swinfo[y].parentname.empty())
		{
			std::string lparent(m_swinfo[y].parentname);
			bool found = false;

			// first scan backward
			for (int x = y; x > 0; --x)
				if (lparent == m_swinfo[x].shortname && m_swinfo[y].listname == m_swinfo[x].listname)
				{
					m_swinfo[y].parentlongname = m_swinfo[x].longname;
					found = true;
					break;
				}

			// not found? then scan forward
			for (size_t x = y; !found && x < m_swinfo.size(); ++x)
				if (lparent == m_swinfo[x].shortname && m_swinfo[y].listname == m_swinfo[x].listname)
				{
					m_swinfo[y].parentlongname = m_swinfo[x].longname;
					break;
				}
		}
	}

	std::string searchstr, curpath;
	const osd::directory::entry *dir;
	for (auto & elem : m_filter.swlist.name)
	{
		path_iterator path(machine().options().media_path());
		while (path.next(curpath))
		{
			searchstr.assign(curpath).append(PATH_SEPARATOR).append(elem).append(";");
			file_enumerator fpath(searchstr.c_str());

			// iterate while we get new objects
			while ((dir = fpath.next()) != nullptr)
			{
				std::string name;
				if (dir->type == osd::directory::entry::entry_type::FILE)
					name = core_filename_extract_base(dir->name, true);
				else if (dir->type == osd::directory::entry::entry_type::DIR && strcmp(dir->name, ".") != 0)
					name = dir->name;
				else
					continue;

				strmakelower(name);
				for (auto & yelem : m_swinfo)
					if (yelem.shortname == name && yelem.listname == elem)
					{
						yelem.available = true;
						break;
					}
			}
		}
	}

	// sort array
	std::stable_sort(m_swinfo.begin() + 1, m_swinfo.end(), compare_software);
	std::stable_sort(m_filter.region.ui.begin(), m_filter.region.ui.end());
	std::stable_sort(m_filter.year.ui.begin(), m_filter.year.ui.end());
	std::stable_sort(m_filter.type.ui.begin(), m_filter.type.ui.end());
	std::stable_sort(m_filter.publisher.ui.begin(), m_filter.publisher.ui.end());

	for (size_t x = 1; x < m_swinfo.size(); ++x)
		m_sortedlist.push_back(&m_swinfo[x]);
}


//-------------------------------------------------
//  handle select key event
//-------------------------------------------------

void menu_select_software::inkey_select(const event *menu_event)
{
	ui_software_info *ui_swinfo = (ui_software_info *)menu_event->itemref;

	if (ui_swinfo->startempty == 1)
	{
		if (!select_bios(*ui_swinfo->driver, true))
		{
			reselect_last::reselect(true);
			launch_system(*ui_swinfo->driver, *ui_swinfo);
		}
	}
	else
	{
		// first validate
		driver_enumerator drivlist(machine().options(), *ui_swinfo->driver);
		media_auditor auditor(drivlist);
		drivlist.next();
		software_list_device *swlist = software_list_device::find_by_name(*drivlist.config(), ui_swinfo->listname.c_str());
		const software_info *swinfo = swlist->find(ui_swinfo->shortname.c_str());

		media_auditor::summary const summary = auditor.audit_software(swlist->list_name(), swinfo, AUDIT_VALIDATE_FAST);

		if (summary == media_auditor::CORRECT || summary == media_auditor::BEST_AVAILABLE || summary == media_auditor::NONE_NEEDED)
		{
			if (!select_bios(*ui_swinfo, false) && !select_part(*swinfo, *ui_swinfo))
			{
				reselect_last::reselect(true);
				launch_system(drivlist.driver(), *ui_swinfo);
			}
		}
		else
		{
			// otherwise, display an error
			std::ostringstream str;
			str << _("The selected software is missing one or more required files. Please select a different software.\n\n");
			if (media_auditor::NOTFOUND != summary)
			{
				auditor.summarize(nullptr, &str);
				str << "\n";
			}
			str << _("Press any key to continue."),
			set_error(reset_options::REMEMBER_POSITION, str.str());
		}
	}
}

//-------------------------------------------------
//  handle special key event
//-------------------------------------------------

void menu_select_software::inkey_special(const event *menu_event)
{
	if (input_character(m_search, menu_event->unichar, uchar_is_printable))
		reset(reset_options::SELECT_FIRST);
}


//-------------------------------------------------
//  load custom filters info from file
//-------------------------------------------------

void menu_select_software::load_sw_custom_filters()
{
	// attempt to open the output file
	emu_file file(ui().options().ui_path(), OPEN_FLAG_READ);
	if (file.open("custom_", m_driver->name, "_filter.ini") == osd_file::error::NONE)
	{
		char buffer[MAX_CHAR_INFO];

		// get number of filters
		file.gets(buffer, MAX_CHAR_INFO);
		char *pb = strchr(buffer, '=');
		sw_custfltr::numother = atoi(++pb) - 1;

		// get main filter
		file.gets(buffer, MAX_CHAR_INFO);
		pb = strchr(buffer, '=') + 2;

		for (int y = 0; y < sw_filters::length; ++y)
			if (!strncmp(pb, sw_filters::text[y], strlen(sw_filters::text[y])))
			{
				sw_custfltr::main = y;
				break;
			}

		for (int x = 1; x <= sw_custfltr::numother; ++x)
		{
			file.gets(buffer, MAX_CHAR_INFO);
			char *cb = strchr(buffer, '=') + 2;
			for (int y = 0; y < sw_filters::length; y++)
			{
				if (!strncmp(cb, sw_filters::text[y], strlen(sw_filters::text[y])))
				{
					sw_custfltr::other[x] = y;
					if (y == UI_SW_PUBLISHERS)
					{
						file.gets(buffer, MAX_CHAR_INFO);
						char *ab = strchr(buffer, '=') + 2;
						for (size_t z = 0; z < m_filter.publisher.ui.size(); ++z)
							if (!strncmp(ab, m_filter.publisher.ui[z].c_str(), m_filter.publisher.ui[z].length()))
								sw_custfltr::mnfct[x] = z;
					}
					else if (y == UI_SW_YEARS)
					{
						file.gets(buffer, MAX_CHAR_INFO);
						char *db = strchr(buffer, '=') + 2;
						for (size_t z = 0; z < m_filter.year.ui.size(); ++z)
							if (!strncmp(db, m_filter.year.ui[z].c_str(), m_filter.year.ui[z].length()))
								sw_custfltr::year[x] = z;
					}
					else if (y == UI_SW_LIST)
					{
						file.gets(buffer, MAX_CHAR_INFO);
						char *gb = strchr(buffer, '=') + 2;
						for (size_t z = 0; z < m_filter.swlist.name.size(); ++z)
							if (!strncmp(gb, m_filter.swlist.name[z].c_str(), m_filter.swlist.name[z].length()))
								sw_custfltr::list[x] = z;
					}
					else if (y == UI_SW_TYPE)
					{
						file.gets(buffer, MAX_CHAR_INFO);
						char *fb = strchr(buffer, '=') + 2;
						for (size_t z = 0; z < m_filter.type.ui.size(); ++z)
							if (!strncmp(fb, m_filter.type.ui[z].c_str(), m_filter.type.ui[z].length()))
								sw_custfltr::type[x] = z;
					}
					else if (y == UI_SW_REGION)
					{
						file.gets(buffer, MAX_CHAR_INFO);
						char *eb = strchr(buffer, '=') + 2;
						for (size_t z = 0; z < m_filter.region.ui.size(); ++z)
							if (!strncmp(eb, m_filter.region.ui[z].c_str(), m_filter.region.ui[z].length()))
								sw_custfltr::region[x] = z;
					}
				}
			}
		}
		file.close();
	}
}

//-------------------------------------------------
//  set software regions
//-------------------------------------------------

void c_sw_region::set(std::string &str)
{
	std::string name = getname(str);
	if (std::find(ui.begin(), ui.end(), name) != ui.end())
		return;

	ui.push_back(name);
}

std::string c_sw_region::getname(std::string &str)
{
	std::string fullname(str);
	strmakelower(fullname);
	size_t found = fullname.find("(");

	if (found != std::string::npos)
	{
		size_t ends = fullname.find_first_not_of("abcdefghijklmnopqrstuvwxyz", found + 1);
		std::string temp(fullname.substr(found + 1, ends - found - 1));

		for (auto & elem : region_lists)
			if (temp == elem)
				return (str.substr(found + 1, ends - found - 1));
	}
	return std::string("<none>");
}

//-------------------------------------------------
//  set software device type
//-------------------------------------------------

void c_sw_type::set(std::string &str)
{
	if (std::find(ui.begin(), ui.end(), str) != ui.end())
		return;

	ui.push_back(str);
}

//-------------------------------------------------
//  set software years
//-------------------------------------------------

void c_sw_year::set(std::string &str)
{
	if (std::find(ui.begin(), ui.end(), str) != ui.end())
		return;

	ui.push_back(str);
}

//-------------------------------------------------
//  set software publishers
//-------------------------------------------------

void c_sw_publisher::set(std::string &str)
{
	std::string name = getname(str);
	if (std::find(ui.begin(), ui.end(), name) != ui.end())
		return;

	ui.push_back(name);
}

std::string c_sw_publisher::getname(std::string &str)
{
	size_t found = str.find("(");

	if (found != std::string::npos)
		return (str.substr(0, found - 1));
	else
		return str;
}

//-------------------------------------------------
//  build display list
//-------------------------------------------------
void menu_select_software::build_list(std::vector<ui_software_info *> &s_drivers, const char *filter_text, int filter)
{
	if (s_drivers.empty() && filter == -1)
	{
		filter = sw_filters::actual;
		s_drivers = m_sortedlist;
	}

	// iterate over entries
	for (auto & s_driver : s_drivers)
	{
		switch (filter)
		{
		case UI_SW_PARENTS:
			if (s_driver->parentname.empty())
				m_displaylist.push_back(s_driver);
			break;

		case UI_SW_CLONES:
			if (!s_driver->parentname.empty())
				m_displaylist.push_back(s_driver);
			break;

		case UI_SW_AVAILABLE:
			if (s_driver->available)
				m_displaylist.push_back(s_driver);
			break;

		case UI_SW_UNAVAILABLE:
			if (!s_driver->available)
				m_displaylist.push_back(s_driver);
			break;

		case UI_SW_SUPPORTED:
			if (s_driver->supported == SOFTWARE_SUPPORTED_YES)
				m_displaylist.push_back(s_driver);
			break;

		case UI_SW_PARTIAL_SUPPORTED:
			if (s_driver->supported == SOFTWARE_SUPPORTED_PARTIAL)
				m_displaylist.push_back(s_driver);
			break;

		case UI_SW_UNSUPPORTED:
			if (s_driver->supported == SOFTWARE_SUPPORTED_NO)
				m_displaylist.push_back(s_driver);
			break;

		case UI_SW_REGION:
			{
				std::string name = m_filter.region.getname(s_driver->longname);

				if(!name.empty() && name == filter_text)
					m_displaylist.push_back(s_driver);
			}
			break;

		case UI_SW_PUBLISHERS:
			{
				std::string name = m_filter.publisher.getname(s_driver->publisher);

				if(!name.empty() && name == filter_text)
					m_displaylist.push_back(s_driver);
			}
			break;

		case UI_SW_YEARS:
			if(s_driver->year == filter_text)
				m_displaylist.push_back(s_driver);
			break;

		case UI_SW_LIST:
			if(s_driver->listname == filter_text)
				m_displaylist.push_back(s_driver);
			break;

		case UI_SW_TYPE:
			if(s_driver->devicetype == filter_text)
				m_displaylist.push_back(s_driver);
			break;

		default:
			m_displaylist.push_back(s_driver);
			break;
		}
	}
}

//-------------------------------------------------
//  find approximate matches
//-------------------------------------------------

void menu_select_software::find_matches(const char *str, int count)
{
	// allocate memory to track the penalty value
	std::vector<int> penalty(count, 9999);
	int index = 0;

	for (; index < m_displaylist.size(); ++index)
	{
		// pick the best match between driver name and description
		int curpenalty = fuzzy_substring(str, m_displaylist[index]->longname);
		int tmp = fuzzy_substring(str, m_displaylist[index]->shortname);
		curpenalty = std::min(curpenalty, tmp);

		// insert into the sorted table of matches
		for (int matchnum = count - 1; matchnum >= 0; --matchnum)
		{
			// stop if we're worse than the current entry
			if (curpenalty >= penalty[matchnum])
				break;

			// as long as this isn't the last entry, bump this one down
			if (matchnum < count - 1)
			{
				penalty[matchnum + 1] = penalty[matchnum];
				m_searchlist[matchnum + 1] = m_searchlist[matchnum];
			}

			m_searchlist[matchnum] = m_displaylist[index];
			penalty[matchnum] = curpenalty;
		}
	}
	(index < count) ? m_searchlist[index] = nullptr : m_searchlist[count] = nullptr;
}

//-------------------------------------------------
//  build custom display list
//-------------------------------------------------

void menu_select_software::build_custom()
{
	std::vector<ui_software_info *> s_drivers;

	build_list(m_sortedlist, nullptr, sw_custfltr::main);

	for (int count = 1; count <= sw_custfltr::numother; ++count)
	{
		int filter = sw_custfltr::other[count];
		s_drivers = m_displaylist;
		m_displaylist.clear();

		switch (filter)
		{
			case UI_SW_YEARS:
				build_list(s_drivers, m_filter.year.ui[sw_custfltr::year[count]].c_str(), filter);
				break;
			case UI_SW_LIST:
				build_list(s_drivers, m_filter.swlist.name[sw_custfltr::list[count]].c_str(), filter);
				break;
			case UI_SW_TYPE:
				build_list(s_drivers, m_filter.type.ui[sw_custfltr::type[count]].c_str(), filter);
				break;
			case UI_SW_PUBLISHERS:
				build_list(s_drivers, m_filter.publisher.ui[sw_custfltr::mnfct[count]].c_str(), filter);
				break;
			case UI_SW_REGION:
				build_list(s_drivers, m_filter.region.ui[sw_custfltr::region[count]].c_str(), filter);
				break;
			default:
				build_list(s_drivers, nullptr, filter);
				break;
		}
	}
}

//-------------------------------------------------
//  draw left box
//-------------------------------------------------

float menu_select_software::draw_left_panel(float x1, float y1, float x2, float y2)
{
	if (ui_globals::panels_status == SHOW_PANELS || ui_globals::panels_status == HIDE_RIGHT_PANEL)
	{
		float origy1 = y1;
		float origy2 = y2;
		float text_size = 0.75f;
		float l_height = ui().get_line_height();
		float line_height = l_height * text_size;
		float left_width = 0.0f;
		int text_lenght = sw_filters::length;
		int afilter = sw_filters::actual;
		int phover = HOVER_SW_FILTER_FIRST;
		const char **text = sw_filters::text;
		float sc = y2 - y1 - (2.0f * UI_BOX_TB_BORDER);

		if ((text_lenght * line_height) > sc)
		{
			float lm = sc / (text_lenght);
			text_size = lm / l_height;
			line_height = l_height * text_size;
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

			if (mouse_in_rect(x1, y1, x2, y1 + line_height))
			{
				bgcolor = UI_MOUSEOVER_BG_COLOR;
				fgcolor = UI_MOUSEOVER_COLOR;
				hover = phover + filter;
			}

			if (highlight == filter && get_focus() == focused_menu::left)
			{
				fgcolor = rgb_t(0xff, 0xff, 0xff, 0x00);
				bgcolor = rgb_t(0xff, 0xff, 0xff, 0xff);
			}

			if (bgcolor != UI_TEXT_BG_COLOR)
			{
				ui().draw_textured_box(container(), x1, y1, x2, y1 + line_height, bgcolor, rgb_t(255, 43, 43, 43),
						hilight_main_texture(), PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA) | PRIMFLAG_TEXWRAP(1));
			}

			float x1t = x1 + text_sign;
			if (afilter == UI_SW_CUSTOM)
			{
				if (filter == sw_custfltr::main)
				{
					str.assign("@custom1 ").append(text[filter]);
					x1t -= text_sign;
				}
				else
				{
					for (int count = 1; count <= sw_custfltr::numother; ++count)
					{
						int cfilter = sw_custfltr::other[count];
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
			else if (filter == sw_filters::actual)
			{
				str.assign("_> ").append(text[filter]);
				x1t -= text_sign;
				convert_command_glyph(str);
			}

			ui().draw_text_full(container(), str.c_str(), x1t, y1, x2 - x1, ui::text_layout::LEFT, ui::text_layout::NEVER,
					mame_ui_manager::NORMAL, fgcolor, bgcolor, nullptr, nullptr, text_size);
			y1 += line_height;
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

void menu_select_software::get_selection(ui_software_info const *&software, game_driver const *&driver) const
{
	software = reinterpret_cast<ui_software_info const *>(get_selection_ref());
	driver = software ? software->driver : nullptr;
}


void menu_select_software::make_topbox_text(std::string &line0, std::string &line1, std::string &line2) const
{
	// determine the text for the header
	int vis_item = !m_search.empty() ? visible_items : (m_has_empty_start ? visible_items - 1 : visible_items);
	line0 = string_format(_("%1$s %2$s ( %3$d / %4$d software packages )"), emulator_info::get_appname(), bare_build_version, vis_item, m_swinfo.size() - 1);
	line1 = string_format(_("Driver: \"%1$s\" software list "), m_driver->type.fullname());

	std::string filtered;
	if (sw_filters::actual == UI_SW_REGION && m_filter.region.ui.size() != 0)
		filtered = string_format(_("Region: %1$s -"), m_filter.region.ui[m_filter.region.actual]);
	else if (sw_filters::actual == UI_SW_PUBLISHERS)
		filtered = string_format(_("Publisher: %1$s -"), m_filter.publisher.ui[m_filter.publisher.actual]);
	else if (sw_filters::actual == UI_SW_YEARS)
		filtered = string_format(_("Year: %1$s -"), m_filter.year.ui[m_filter.year.actual]);
	else if (sw_filters::actual == UI_SW_LIST)
		filtered = string_format(_("Software List: %1$s -"), m_filter.swlist.description[m_filter.swlist.actual]);
	else if (sw_filters::actual == UI_SW_TYPE)
		filtered = string_format(_("Device type: %1$s -"), m_filter.type.ui[m_filter.type.actual]);

	line2 = string_format(_("%s Search: %s_"), filtered, m_search);
}


std::string menu_select_software::make_driver_description(game_driver const &driver) const
{
	// first line is game description
	return string_format(_("%1$-.100s"), driver.type.fullname());
}


std::string menu_select_software::make_software_description(ui_software_info const &software) const
{
	// first line is long name
	return string_format(_("%1$-.100s"), software.longname);
}

} // namespace ui
