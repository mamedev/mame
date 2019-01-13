// license:BSD-3-Clause
// copyright-holders:Maurizio Petrarota
/***************************************************************************

    ui/selsoft.cpp

    UI software menu.

***************************************************************************/

#include "emu.h"
#include "ui/selsoft.h"

#include "ui/ui.h"
#include "ui/inifile.h"
#include "ui/selector.h"

#include "audit.h"
#include "drivenum.h"
#include "emuopts.h"
#include "mame.h"
#include "rendutil.h"
#include "softlist_dev.h"
#include "uiinput.h"
#include "luaengine.h"


namespace ui {

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
	, m_filter_type(software_filter::ALL)
{
	reselect_last::reselect(false);

	m_driver = driver;
	build_software_list();
	load_sw_custom_filters();
	m_filter_highlight = m_filter_type;

	set_switch_image();
	ui_globals::cur_sw_dats_view = 0;
	ui_globals::cur_sw_dats_total = 1;
}

//-------------------------------------------------
//  dtor
//-------------------------------------------------

menu_select_software::~menu_select_software()
{
}

//-------------------------------------------------
//  handle
//-------------------------------------------------

void menu_select_software::handle()
{
	if (m_prev_selected == nullptr)
		m_prev_selected = item[0].ref;

	// ignore pause keys by swallowing them before we process the menu
	machine().ui_input().pressed(IPT_UI_PAUSE);

	// process the menu
	const event *menu_event = process(PROCESS_LR_REPEAT);
	if (menu_event)
	{
		if (dismiss_error())
		{
			// reset the error on any future event
		}
		else switch (menu_event->iptkey)
		{
		case IPT_UI_SELECT:
			if ((get_focus() == focused_menu::MAIN) && menu_event->itemref)
				inkey_select(menu_event);
			break;

		case IPT_UI_LEFT:
			if (ui_globals::rpanel == RP_IMAGES)
			{
				// Images
				previous_image_view();
			}
			else if (ui_globals::rpanel == RP_INFOS && ui_globals::cur_sw_dats_view > 0)
			{
				// Infos
				ui_globals::cur_sw_dats_view--;
				m_topline_datsview = 0;
			}
			break;

		case IPT_UI_RIGHT:
			if (ui_globals::rpanel == RP_IMAGES)
			{
				// Images
				next_image_view();
			}
			else if (ui_globals::rpanel == RP_INFOS && ui_globals::cur_sw_dats_view < (ui_globals::cur_sw_dats_total - 1))
			{
				// Infos
				ui_globals::cur_sw_dats_view++;
				m_topline_datsview = 0;
			}
			break;

		case IPT_UI_UP:
			if ((get_focus() == focused_menu::LEFT) && (software_filter::FIRST < m_filter_highlight))
				--m_filter_highlight;
			break;

		case IPT_UI_DOWN:
			if ((get_focus() == focused_menu::LEFT) && (software_filter::LAST > m_filter_highlight))
				++m_filter_highlight;
			break;

		case IPT_UI_HOME:
			if (get_focus() == focused_menu::LEFT)
				m_filter_highlight = software_filter::FIRST;
			break;

		case IPT_UI_END:
			if (get_focus() == focused_menu::LEFT)
				m_filter_highlight = software_filter::LAST;
			break;

		case IPT_UI_CONFIGURE:
			inkey_navigation();
			break;

		case IPT_UI_DATS:
			inkey_dats();
			break;

		default:
			if (menu_event->itemref)
			{
				if (menu_event->iptkey == IPT_UI_FAVORITES)
				{
					// handle UI_FAVORITES
					ui_software_info *swinfo = (ui_software_info *)menu_event->itemref;

					if ((uintptr_t)swinfo > 2)
					{
						favorite_manager &mfav = mame_machine_manager::instance()->favorite();
						if (!mfav.is_favorite_system_software(*swinfo))
						{
							mfav.add_favorite_software(*swinfo);
							machine().popmessage(_("%s\n added to favorites list."), swinfo->longname.c_str());
						}

						else
						{
							machine().popmessage(_("%s\n removed from favorites list."), swinfo->longname.c_str());
							mfav.remove_favorite_software(*swinfo);
						}
					}
				}
			}
		}
	}

	// if we're in an error state, overlay an error message
	draw_error_text();
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

		filter_map::const_iterator const it(m_filters.find(m_filter_type));
		if (m_filters.end() == it)
			m_displaylist = m_sortedlist;
		else
			it->second->apply(std::begin(m_sortedlist), std::end(m_sortedlist), std::back_inserter(m_displaylist));

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
		m_filter_data.add_list(swlist.list_name(), swlist.description());
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

				m_filter_data.add_region(tmpmatches.longname);
				m_filter_data.add_publisher(tmpmatches.publisher);
				m_filter_data.add_year(tmpmatches.year);
				m_filter_data.add_device_type(tmpmatches.devicetype);
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
	for (auto & elem : m_filter_data.list_names())
	{
		path_iterator path(machine().options().media_path());
		while (path.next(curpath))
		{
			searchstr.assign(curpath).append(PATH_SEPARATOR).append(elem).append(";");
			file_enumerator fpath(searchstr.c_str());

			// iterate while we get new objects
			osd::directory::entry const *dir;
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
	m_filter_data.finalise();

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
//  load custom filters info from file
//-------------------------------------------------

void menu_select_software::load_sw_custom_filters()
{
	// attempt to open the output file
	emu_file file(ui().options().ui_path(), OPEN_FLAG_READ);
	if (file.open("custom_", m_driver->name, "_filter.ini") == osd_file::error::NONE)
	{
		software_filter::ptr flt(software_filter::create(file, m_filter_data));
		if (flt)
			m_filters.emplace(flt->get_type(), std::move(flt));
		file.close();
	}
}

//-------------------------------------------------
//  find approximate matches
//-------------------------------------------------

void menu_select_software::find_matches(const char *str, int count)
{
	// allocate memory to track the penalty value
	std::vector<double> penalty(count, 1.0);
	std::u32string const search(ustr_from_utf8(normalize_unicode(str, unicode_normalization_form::D, true)));

	int index = 0;
	for ( ; index < m_displaylist.size(); ++index)
	{
		// pick the best match between shortname and longname
		// TODO: search alternate title as well
		double curpenalty(util::edit_distance(search, ustr_from_utf8(normalize_unicode(m_displaylist[index]->shortname, unicode_normalization_form::D, true))));
		if (curpenalty)
		{
			double const tmp(util::edit_distance(search, ustr_from_utf8(normalize_unicode(m_displaylist[index]->longname, unicode_normalization_form::D, true))));
			curpenalty = (std::min)(curpenalty, tmp);
		}

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
//  draw left box
//-------------------------------------------------

float menu_select_software::draw_left_panel(float x1, float y1, float x2, float y2)
{
	return menu_select_launch::draw_left_panel<software_filter>(m_filter_type, m_filters, x1, y1, x2, y2);
}


//-------------------------------------------------
//  get selected software and/or driver
//-------------------------------------------------

void menu_select_software::get_selection(ui_software_info const *&software, game_driver const *&driver) const
{
	software = reinterpret_cast<ui_software_info const *>(get_selection_ptr());
	driver = software ? software->driver : nullptr;
}


void menu_select_software::make_topbox_text(std::string &line0, std::string &line1, std::string &line2) const
{
	// determine the text for the header
	int vis_item = !m_search.empty() ? visible_items : (m_has_empty_start ? visible_items - 1 : visible_items);
	line0 = string_format(_("%1$s %2$s ( %3$d / %4$d software packages )"), emulator_info::get_appname(), bare_build_version, vis_item, m_swinfo.size() - 1);
	line1 = string_format(_("Driver: \"%1$s\" software list "), m_driver->type.fullname());

	filter_map::const_iterator const it(m_filters.find(m_filter_type));
	char const *const filter((m_filters.end() != it) ? it->second->filter_text() : nullptr);
	if (filter)
		line2 = string_format(_("%1$s: %2$s - Search: %3$s_"), it->second->display_name(), filter, m_search);
	else
		line2 = string_format(_("Search: %1$s_"), m_search);
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


void menu_select_software::filter_selected()
{
	if ((software_filter::FIRST <= m_filter_highlight) && (software_filter::LAST >= m_filter_highlight))
	{
		m_search.clear();
		filter_map::const_iterator it(m_filters.find(software_filter::type(m_filter_highlight)));
		if (m_filters.end() == it)
			it = m_filters.emplace(software_filter::type(m_filter_highlight), software_filter::create(software_filter::type(m_filter_highlight), m_filter_data)).first;
		it->second->show_ui(
				ui(),
				container(),
				[this, driver = m_driver] (software_filter &filter)
				{
					software_filter::type const new_type(filter.get_type());
					if (software_filter::CUSTOM == new_type)
					{
						emu_file file(ui().options().ui_path(), OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS);
						if (file.open("custom_", driver->name, "_filter.ini") == osd_file::error::NONE)
						{
							filter.save_ini(file, 0);
							file.close();
						}
					}
					m_filter_type = new_type;
					reset(reset_options::SELECT_FIRST);
				});
	}
}

} // namespace ui
