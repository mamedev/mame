// license:BSD-3-Clause
// copyright-holders:Maurizio Petrarota, Vas Crabb
/***************************************************************************

    ui/selsoft.cpp

    UI software menu.

***************************************************************************/

#include "emu.h"
#include "ui/selsoft.h"

#include "ui/ui.h"
#include "ui/icorender.h"
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

#include <algorithm>
#include <iterator>
#include <functional>


namespace ui {

namespace {

//-------------------------------------------------
//  compares two items in the software list and
//  sort them by parent-clone
//-------------------------------------------------

bool compare_software(ui_software_info const &a, ui_software_info const &b)
{
	bool const clonex = !a.parentname.empty() && !a.parentlongname.empty();
	bool const cloney = !b.parentname.empty() && !b.parentlongname.empty();

	if (!clonex && !cloney)
	{
		return 0 > core_stricmp(a.longname.c_str(), b.longname.c_str());
	}
	else if (!clonex && cloney)
	{
		if ((a.shortname == b.parentname) && (a.instance == b.instance))
			return true;
		else
			return 0 > core_stricmp(a.longname.c_str(), b.parentlongname.c_str());
	}
	else if (clonex && !cloney)
	{
		if ((a.parentname == b.shortname) && (a.instance == b.instance))
			return false;
		else
			return 0 > core_stricmp(a.parentlongname.c_str(), b.longname.c_str());
	}
	else if ((a.parentname == b.parentname) && (a.instance == b.instance))
	{
		return 0 > core_stricmp(a.longname.c_str(), b.longname.c_str());
	}
	else
	{
		return 0 > core_stricmp(a.parentlongname.c_str(), b.parentlongname.c_str());
	}
}

} // anonymous namespace


menu_select_software::search_item::search_item(ui_software_info const &s)
	: software(s)
	, ucs_shortname(ustr_from_utf8(normalize_unicode(s.shortname, unicode_normalization_form::D, true)))
	, ucs_longname(ustr_from_utf8(normalize_unicode(s.longname, unicode_normalization_form::D, true)))
	, penalty(1.0)
{
}

void menu_select_software::search_item::set_penalty(std::u32string const &search)
{
	// TODO: search alternate title as well
	penalty = util::edit_distance(search, ucs_shortname);
	if (penalty)
		penalty = (std::min)(penalty, util::edit_distance(search, ucs_longname));
}


//-------------------------------------------------
//  ctor
//-------------------------------------------------

menu_select_software::menu_select_software(mame_ui_manager &mui, render_container &container, game_driver const &driver)
	: menu_select_launch(mui, container, true)
	, m_icon_paths()
	, m_icons(MAX_ICONS_RENDER)
	, m_driver(driver)
	, m_has_empty_start(false)
	, m_filter_data()
	, m_filters()
	, m_filter_type(software_filter::ALL)
	, m_swinfo()
	, m_searchlist()
	, m_displaylist()
{
	reselect_last::reselect(false);

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
		m_prev_selected = item(0).ref;

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
							machine().popmessage(_("%s\n added to favorites list."), swinfo->longname);
						}

						else
						{
							machine().popmessage(_("%s\n removed from favorites list."), swinfo->longname);
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
	for (auto &icon : m_icons) // TODO: why is this here?  maybe better on resize or setting change?
		icon.second.texture.reset();

	uint32_t flags_ui = FLAG_LEFT_ARROW | FLAG_RIGHT_ARROW;
	m_has_empty_start = true;
	int old_software = -1;

	// FIXME: why does it do this relatively expensive operation every time?
	machine_config config(m_driver, machine().options());
	for (device_image_interface &image : image_interface_iterator(config.root_device()))
	{
		if (!image.filename() && image.must_be_loaded())
		{
			m_has_empty_start = false;
			break;
		}
	}

	// start with an empty list
	m_displaylist.clear();
	filter_map::const_iterator const flt(m_filters.find(m_filter_type));

	// no active search
	if (m_search.empty())
	{
		// if the device can be loaded empty, add an item
		if (m_has_empty_start)
			item_append("[Start empty]", "", flags_ui, (void *)&m_swinfo[0]);

		if (m_filters.end() == flt)
			std::copy(std::next(m_swinfo.begin()), m_swinfo.end(), std::back_inserter(m_displaylist));
		else
			flt->second->apply(std::next(m_swinfo.begin()), m_swinfo.end(), std::back_inserter(m_displaylist));
	}
	else
	{
		find_matches();

		if (m_filters.end() == flt)
		{
			std::transform(
					m_searchlist.begin(),
					std::next(m_searchlist.begin(), (std::min)(m_searchlist.size(), MAX_VISIBLE_SEARCH)),
					std::back_inserter(m_displaylist),
					[] (search_item const &entry) { return entry.software; });
		}
		else
		{
			for (auto it = m_searchlist.begin(); (m_searchlist.end() != it) && (MAX_VISIBLE_SEARCH > m_displaylist.size()); ++it)
			{
				if (flt->second->apply(it->software))
					m_displaylist.emplace_back(it->software);
			}
		}
	}

	// iterate over entries
	for (size_t curitem = 0; curitem < m_displaylist.size(); ++curitem)
	{
		if (reselect_last::software() == "[Start empty]" && !reselect_last::driver().empty())
			old_software = 0;
		else if (m_displaylist[curitem].get().shortname == reselect_last::software() && m_displaylist[curitem].get().listname == reselect_last::swlist())
			old_software = m_has_empty_start ? curitem + 1 : curitem;

		item_append(
				m_displaylist[curitem].get().longname, m_displaylist[curitem].get().devicetype,
				m_displaylist[curitem].get().parentname.empty() ? flags_ui : (FLAG_INVERT | flags_ui), (void *)&m_displaylist[curitem].get());
	}

	item_append(menu_item_type::SEPARATOR, flags_ui);

	// configure the custom rendering
	customtop = 4.0f * ui().get_line_height() + 5.0f * ui().box_tb_border();
	custombottom = 5.0f * ui().get_line_height() + 4.0f * ui().box_tb_border();

	if (old_software != -1)
	{
		set_selected_index(old_software);
		top_line = selected_index() - (ui_globals::visible_sw_lines / 2);
	}

	reselect_last::reset();
}

//-------------------------------------------------
//  build a list of software
//-------------------------------------------------

void menu_select_software::build_software_list()
{
	// add start empty item
	m_swinfo.emplace_back(m_driver);

	machine_config config(m_driver, machine().options());

	// iterate through all software lists
	std::vector<std::size_t> orphans;
	struct orphan_less
	{
		std::vector<ui_software_info> &swinfo;
		bool operator()(std::string const &a, std::string const &b) const { return a < b; };
		bool operator()(std::string const &a, std::size_t b) const { return a < swinfo[b].parentname; };
		bool operator()(std::size_t a, std::string const &b) const { return swinfo[a].parentname < b; };
		bool operator()(std::size_t a, std::size_t b) const { return swinfo[a].parentname < swinfo[b].parentname; };
	};
	orphan_less const orphan_cmp{ m_swinfo };
	for (software_list_device &swlist : software_list_device_iterator(config.root_device()))
	{
		m_filter_data.add_list(swlist.list_name(), swlist.description());
		check_for_icons(swlist.list_name().c_str());
		orphans.clear();
		std::map<std::string, std::string> parentnames;
		std::map<std::string, std::string>::const_iterator prevparent(parentnames.end());
		for (const software_info &swinfo : swlist.get_info())
		{
			// check for previously-encountered clones
			if (swinfo.parentname().empty())
			{
				if (parentnames.emplace(swinfo.shortname(), swinfo.longname()).second)
				{
					auto const clones(std::equal_range(orphans.begin(), orphans.end(), swinfo.shortname(), orphan_cmp));
					for (auto it = clones.first; clones.second != it; ++it)
						m_swinfo[*it].parentlongname = swinfo.longname();
					orphans.erase(clones.first, clones.second);
				}
				else
				{
					assert([] (auto const x) { return x.first == x.second; } (std::equal_range(orphans.begin(), orphans.end(), swinfo.shortname(), orphan_cmp)));
				}
			}

			const software_part &part = swinfo.parts().front();
			if (swlist.is_compatible(part) == SOFTWARE_IS_COMPATIBLE)
			{
				char const *instance_name(nullptr);
				char const *type_name(nullptr);
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

				if (instance_name && type_name)
				{
					// add to collection and try to resolve parent if applicable
					auto const ins(m_swinfo.emplace(m_swinfo.end(), swinfo, part, m_driver, swlist.list_name(), instance_name, type_name));
					if (!swinfo.parentname().empty())
					{
						if ((parentnames.end() == prevparent) || (swinfo.parentname() != prevparent->first))
							prevparent = parentnames.find(swinfo.parentname());

						if (parentnames.end() != prevparent)
						{
							ins->parentlongname = prevparent->second;
						}
						else
						{
							orphans.emplace(
									std::upper_bound(orphans.begin(), orphans.end(), swinfo.parentname(), orphan_cmp),
									std::distance(m_swinfo.begin(), ins));
						}
					}

					// populate filter choices
					m_filter_data.add_region(ins->longname);
					m_filter_data.add_publisher(ins->publisher);
					m_filter_data.add_year(ins->year);
					m_filter_data.add_device_type(ins->devicetype);
				}
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
	if (file.open("custom_", m_driver.name, "_filter.ini") == osd_file::error::NONE)
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

void menu_select_software::find_matches()
{
	// ensure search list is populated
	if (m_searchlist.empty())
	{
		m_searchlist.reserve(m_swinfo.size());
		std::copy(m_swinfo.begin(), m_swinfo.end(), std::back_inserter(m_searchlist));
	}

	// update search
	const std::u32string ucs_search(ustr_from_utf8(normalize_unicode(m_search, unicode_normalization_form::D, true)));
	for (search_item &entry : m_searchlist)
		entry.set_penalty(ucs_search);

	// sort according to edit distance
	std::stable_sort(
			m_searchlist.begin(),
			m_searchlist.end(),
			[] (search_item const &lhs, search_item const &rhs) { return lhs.penalty < rhs.penalty; });
}

//-------------------------------------------------
//  draw left box
//-------------------------------------------------

float menu_select_software::draw_left_panel(float x1, float y1, float x2, float y2)
{
	return menu_select_launch::draw_left_panel<software_filter>(m_filter_type, m_filters, x1, y1, x2, y2);
}


//-------------------------------------------------
//  get (possibly cached) icon texture
//-------------------------------------------------

render_texture *menu_select_software::get_icon_texture(int linenum, void *selectedref)
{
	ui_software_info const *const swinfo(reinterpret_cast<ui_software_info const *>(selectedref));
	assert(swinfo);

	if (swinfo->startempty)
		return nullptr;

	icon_cache::iterator icon(m_icons.find(swinfo));
	if ((m_icons.end() == icon) || !icon->second.texture)
	{
		std::map<std::string, std::string>::iterator paths(m_icon_paths.find(swinfo->listname));
		if (m_icon_paths.end() == paths)
			paths = m_icon_paths.emplace(swinfo->listname, make_icon_paths(swinfo->listname.c_str())).first;

		// allocate an entry or allocate a texture on forced redraw
		if (m_icons.end() == icon)
		{
			icon = m_icons.emplace(swinfo, texture_ptr(machine().render().texture_alloc(), machine().render())).first;
		}
		else
		{
			assert(!icon->second.texture);
			icon->second.texture.reset(machine().render().texture_alloc());
		}

		bitmap_argb32 tmp;
		emu_file snapfile(std::string(paths->second), OPEN_FLAG_READ);
		if (snapfile.open(std::string(swinfo->shortname), ".ico") == osd_file::error::NONE)
		{
			render_load_ico_highest_detail(snapfile, tmp);
			snapfile.close();
		}
		if (!tmp.valid() && !swinfo->parentname.empty() && (snapfile.open(std::string(swinfo->parentname), ".ico") == osd_file::error::NONE))
		{
			render_load_ico_highest_detail(snapfile, tmp);
			snapfile.close();
		}

		scale_icon(std::move(tmp), icon->second);
	}

	return icon->second.bitmap.valid() ? icon->second.texture.get() : nullptr;
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
	int vis_item = !m_search.empty() ? m_available_items : (m_has_empty_start ? m_available_items - 1 : m_available_items);
	line0 = string_format(_("%1$s %2$s ( %3$d / %4$d software packages )"), emulator_info::get_appname(), bare_build_version, vis_item, m_swinfo.size() - 1);
	line1 = string_format(_("Driver: \"%1$s\" software list "), m_driver.type.fullname());

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
		filter_map::const_iterator it(m_filters.find(software_filter::type(m_filter_highlight)));
		if (m_filters.end() == it)
			it = m_filters.emplace(software_filter::type(m_filter_highlight), software_filter::create(software_filter::type(m_filter_highlight), m_filter_data)).first;
		it->second->show_ui(
				ui(),
				container(),
				[this] (software_filter &filter)
				{
					software_filter::type const new_type(filter.get_type());
					if (software_filter::CUSTOM == new_type)
					{
						emu_file file(ui().options().ui_path(), OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS);
						if (file.open("custom_", m_driver.name, "_filter.ini") == osd_file::error::NONE)
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
