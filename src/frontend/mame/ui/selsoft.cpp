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

#include "corestr.h"
#include "drivenum.h"
#include "emuopts.h"
#include "fileio.h"
#include "mame.h"
#include "rendutil.h"
#include "softlist_dev.h"
#include "uiinput.h"
#include "luaengine.h"
#include "unicode.h"

#include <algorithm>
#include <iterator>
#include <functional>
#include <thread>
#include <locale>


namespace ui {

struct menu_select_software::search_item
{
	search_item(search_item const &) = default;
	search_item(search_item &&) = default;
	search_item &operator=(search_item const &) = default;
	search_item &operator=(search_item &&) = default;

	search_item(ui_software_info const &s)
		: software(s)
		, ucs_shortname(ustr_from_utf8(normalize_unicode(s.shortname, unicode_normalization_form::D, true)))
		, ucs_longname(ustr_from_utf8(normalize_unicode(s.longname, unicode_normalization_form::D, true)))
		, ucs_alttitles()
		, penalty(1.0)
	{
		ucs_alttitles.reserve(s.alttitles.size());
		for (std::string const &alttitle : s.alttitles)
			ucs_alttitles.emplace_back(ustr_from_utf8(normalize_unicode(alttitle, unicode_normalization_form::D, true)));
	}

	void set_penalty(std::u32string const &search)
	{
		penalty = util::edit_distance(search, ucs_shortname);
		if (penalty)
			penalty = (std::min)(penalty, util::edit_distance(search, ucs_longname));
		auto it(ucs_alttitles.begin());
		while (penalty && (ucs_alttitles.end() != it))
			penalty = (std::min)(penalty, util::edit_distance(search, *it++));
	}

	std::reference_wrapper<ui_software_info const> software;
	std::u32string ucs_shortname;
	std::u32string ucs_longname;
	std::vector<std::u32string> ucs_alttitles;
	double penalty;
};



class menu_select_software::machine_data
{
public:
	machine_data(menu_select_software &menu)
		: m_icons(MAX_ICONS_RENDER)
		, m_has_empty_start(false)
		, m_filter_data()
		, m_filters()
		, m_filter_type(software_filter::ALL)
		, m_swinfo()
		, m_searchlist()
	{
		// add start empty item
		m_swinfo.emplace_back(*menu.m_system.driver);

		machine_config config(*menu.m_system.driver, menu.machine().options());

		// see if any media devices require an image to be loaded
		m_has_empty_start = true;
		for (device_image_interface &image : image_interface_enumerator(config.root_device()))
		{
			if (!image.filename() && image.must_be_loaded())
			{
				m_has_empty_start = false;
				break;
			}
		}

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
		for (software_list_device &swlist : software_list_device_enumerator(config.root_device()))
		{
			m_filter_data.add_list(swlist.list_name(), swlist.description());
			menu.check_for_icons(swlist.list_name().c_str());
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
					for (device_image_interface &image : image_interface_enumerator(config.root_device()))
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
						auto const ins(m_swinfo.emplace(m_swinfo.end(), swinfo, part, *menu.m_system.driver, swlist.list_name(), instance_name, type_name));
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
						for (software_info_item const &i : ins->info)
							m_filter_data.add_info(i);
						m_filter_data.add_device_type(ins->devicetype);
					}
				}
			}
		}

		std::string searchstr, curpath;
		for (auto &elem : m_filter_data.list_names())
		{
			path_iterator path(menu.machine().options().media_path());
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
						name = strmakelower(core_filename_extract_base(dir->name, true));
					else if (dir->type == osd::directory::entry::entry_type::DIR && strcmp(dir->name, ".") != 0)
						name = strmakelower(dir->name);
					else
						continue;

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
		std::collate<wchar_t> const &coll = std::use_facet<std::collate<wchar_t> >(std::locale());
		auto const compare_names =
				[&coll] (std::string const &x, std::string const &y) -> bool
				{
					std::wstring const wx = wstring_from_utf8(x);
					std::wstring const wy = wstring_from_utf8(y);
					return 0 > coll.compare(wx.data(), wx.data() + wx.size(), wy.data(), wy.data() + wy.size());
				};
		std::stable_sort(
				m_swinfo.begin() + 1,
				m_swinfo.end(),
				[&compare_names] (ui_software_info const &a, ui_software_info const &b) -> bool
				{
					bool const clonex = !a.parentname.empty() && !a.parentlongname.empty();
					bool const cloney = !b.parentname.empty() && !b.parentlongname.empty();

					if (!clonex && !cloney)
					{
						return compare_names(a.longname, b.longname);
					}
					else if (!clonex && cloney)
					{
						if ((a.shortname == b.parentname) && (a.instance == b.instance))
							return true;
						else
							return compare_names(a.longname, b.parentlongname);
					}
					else if (clonex && !cloney)
					{
						if ((a.parentname == b.shortname) && (a.instance == b.instance))
							return false;
						else
							return compare_names(a.parentlongname, b.longname);
					}
					else if ((a.parentname == b.parentname) && (a.instance == b.instance))
					{
						return compare_names(a.longname, b.longname);
					}
					else
					{
						return compare_names(a.parentlongname, b.parentlongname);
					}
				});

		// start populating search info in background
		m_search_thread = std::make_unique<std::thread>(
				[this] ()
				{
					m_searchlist.reserve(m_swinfo.size());
					for (ui_software_info const &sw : m_swinfo)
					{
						if (!sw.startempty)
							m_searchlist.emplace_back(sw);
					}
				});

		// build derivative filter data
		m_filter_data.finalise();

		// load custom filters info from file
		emu_file file(menu.ui().options().ui_path(), OPEN_FLAG_READ);
		if (!file.open(util::string_format("custom_%s_filter.ini", menu.m_system.driver->name)))
		{
			software_filter::ptr flt(software_filter::create(file, m_filter_data));
			if (flt)
				m_filters.emplace(flt->get_type(), std::move(flt));
			file.close();
		}
	}

	~machine_data()
	{
		if (m_search_thread)
			m_search_thread->join();
	}

	icon_cache &icons() { return m_icons; }

	bool has_empty_start() const noexcept { return m_has_empty_start; }

	filter_map const &filters() const noexcept { return m_filters; }

	software_filter::type filter_type() const noexcept { return m_filter_type; }
	void set_filter_type(software_filter::type type) noexcept { m_filter_type = type; }

	software_filter const *current_filter() const noexcept
	{
		auto const found(m_filters.find(m_filter_type));
		return (m_filters.end() != found) ? found->second.get() : nullptr;
	}

	software_filter &get_filter(software_filter::type type)
	{
		filter_map::const_iterator it(m_filters.find(type));
		if (m_filters.end() != it)
			return *it->second;
		else
			return *m_filters.emplace(type, software_filter::create(type, m_filter_data)).first->second;
	}

	std::vector<ui_software_info> const &swinfo() const noexcept { return m_swinfo; }

	std::vector<search_item> const &find_matches(std::string const &search)
	{
		// ensure search list is populated
		if (m_search_thread)
		{
			m_search_thread->join();
			m_search_thread.reset();
		}

		// update search
		const std::u32string ucs_search(ustr_from_utf8(normalize_unicode(search, unicode_normalization_form::D, true)));
		for (search_item &entry : m_searchlist)
			entry.set_penalty(ucs_search);

		// sort according to edit distance
		std::stable_sort(
				m_searchlist.begin(),
				m_searchlist.end(),
				[] (search_item const &lhs, search_item const &rhs) { return lhs.penalty < rhs.penalty; });

		// return reference to search results
		return m_searchlist;
	}

private:
	icon_cache                      m_icons;
	bool                            m_has_empty_start;
	software_filter_data            m_filter_data;
	filter_map                      m_filters;
	software_filter::type           m_filter_type;
	std::vector<ui_software_info>   m_swinfo;
	std::vector<search_item>        m_searchlist;

	std::unique_ptr<std::thread>    m_search_thread;
};


//-------------------------------------------------
//  ctor
//-------------------------------------------------

menu_select_software::menu_select_software(mame_ui_manager &mui, render_container &container, ui_system_info const &system)
	: menu_select_launch(mui, container, true)
	, m_icon_paths()
	, m_system(system)
	, m_displaylist()
{
	reselect_last::reselect(false);

	using machine_data_cache = util::lru_cache_map<game_driver const *, std::shared_ptr<machine_data> >;
	auto &cached(mui.get_session_data<menu_select_software, machine_data_cache>(8)[system.driver]);
	if (!cached)
		cached = std::make_shared<machine_data>(*this);
	m_data = cached;

	m_filter_highlight = m_data->filter_type();

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

void menu_select_software::handle(event const *ev)
{
	if (m_prev_selected == nullptr)
		m_prev_selected = item(0).ref();

	// FIXME: everything above here used run before events were processed

	// process the menu
	if (ev)
	{
		if (dismiss_error())
		{
			// reset the error on any subsequent menu event
		}
		else switch (ev->iptkey)
		{
		case IPT_UI_SELECT:
			if ((get_focus() == focused_menu::MAIN) && ev->itemref)
				inkey_select(ev);
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
			if (ev->itemref)
			{
				if (ev->iptkey == IPT_UI_FAVORITES)
				{
					// handle UI_FAVORITES
					ui_software_info *swinfo = (ui_software_info *)ev->itemref;

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
	for (auto &icon : m_data->icons()) // TODO: why is this here?  maybe better on resize or setting change?
		icon.second.texture.reset();

	int old_software = -1;

	// start with an empty list
	m_displaylist.clear();
	software_filter const *const flt(m_data->current_filter());

	// no active search
	if (m_search.empty())
	{
		// add an item to start empty or let the user use the file manager
		item_append(
				m_data->has_empty_start() ? _("[Start empty]") : _("[Use file manager]"),
				0,
				(void *)&m_data->swinfo()[0]);

		if (!flt)
			std::copy(std::next(m_data->swinfo().begin()), m_data->swinfo().end(), std::back_inserter(m_displaylist));
		else
			flt->apply(std::next(m_data->swinfo().begin()), m_data->swinfo().end(), std::back_inserter(m_displaylist));
	}
	else
	{
		std::vector<search_item> const &searchlist = m_data->find_matches(m_search);

		if (!flt)
		{
			std::transform(
					searchlist.begin(),
					std::next(searchlist.begin(), (std::min)(searchlist.size(), MAX_VISIBLE_SEARCH)),
					std::back_inserter(m_displaylist),
					[] (search_item const &entry) { return entry.software; });
		}
		else
		{
			for (auto it = searchlist.begin(); (searchlist.end() != it) && (MAX_VISIBLE_SEARCH > m_displaylist.size()); ++it)
			{
				if (flt->apply(it->software))
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
			old_software = curitem + 1;

		item_append(
				m_displaylist[curitem].get().longname, m_displaylist[curitem].get().devicetype,
				m_displaylist[curitem].get().parentname.empty() ? 0 : FLAG_INVERT, (void *)&m_displaylist[curitem].get());
	}

	// configure the custom rendering
	skip_main_items = 0;
	customtop = 4.0f * ui().get_line_height() + 5.0f * ui().box_tb_border();
	custombottom = 4.0f * ui().get_line_height() + 4.0f * ui().box_tb_border();

	if (old_software != -1)
	{
		set_selected_index(old_software);
		top_line = selected_index() - (ui_globals::visible_sw_lines / 2);
	}

	reselect_last::reset();
}


//-------------------------------------------------
//  handle select key event
//-------------------------------------------------

void menu_select_software::inkey_select(const event *menu_event)
{
	ui_software_info *ui_swinfo = (ui_software_info *)menu_event->itemref;
	driver_enumerator drivlist(machine().options(), *ui_swinfo->driver);
	media_auditor auditor(drivlist);
	drivlist.next();

	// audit the system ROMs first to see if we're going to work
	media_auditor::summary const sysaudit = auditor.audit_media(AUDIT_VALIDATE_FAST);
	if (!audit_passed(sysaudit))
	{
		set_error(reset_options::REMEMBER_REF, make_system_audit_fail_text(auditor, sysaudit));
	}
	else if (ui_swinfo->startempty == 1)
	{
		if (!select_bios(*ui_swinfo->driver, true))
		{
			reselect_last::reselect(true);
			launch_system(*ui_swinfo->driver, *ui_swinfo);
		}
	}
	else
	{
		// now audit the software
		software_list_device *swlist = software_list_device::find_by_name(*drivlist.config(), ui_swinfo->listname);
		const software_info *swinfo = swlist->find(ui_swinfo->shortname);
		media_auditor::summary const swaudit = auditor.audit_software(*swlist, *swinfo, AUDIT_VALIDATE_FAST);

		if (audit_passed(swaudit))
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
			set_error(reset_options::REMEMBER_REF, make_software_audit_fail_text(auditor, swaudit));
		}
	}
}


//-------------------------------------------------
//  draw left box
//-------------------------------------------------

float menu_select_software::draw_left_panel(float x1, float y1, float x2, float y2)
{
	return menu_select_launch::draw_left_panel<software_filter>(m_data->filter_type(), m_data->filters(), x1, y1, x2, y2);
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

	icon_cache::iterator icon(m_data->icons().find(swinfo));
	if ((m_data->icons().end() == icon) || !icon->second.texture)
	{
		std::map<std::string, std::string>::iterator paths(m_icon_paths.find(swinfo->listname));
		if (m_icon_paths.end() == paths)
			paths = m_icon_paths.emplace(swinfo->listname, make_icon_paths(swinfo->listname.c_str())).first;

		// allocate an entry or allocate a texture on forced redraw
		if (m_data->icons().end() == icon)
		{
			icon = m_data->icons().emplace(swinfo, texture_ptr(machine().render().texture_alloc(), machine().render())).first;
		}
		else
		{
			assert(!icon->second.texture);
			icon->second.texture.reset(machine().render().texture_alloc());
		}

		bitmap_argb32 tmp;
		emu_file snapfile(std::string(paths->second), OPEN_FLAG_READ);
		if (!snapfile.open(std::string(swinfo->shortname) + ".ico"))
		{
			render_load_ico_highest_detail(snapfile, tmp);
			snapfile.close();
		}
		if (!tmp.valid() && !swinfo->parentname.empty() && !snapfile.open(std::string(swinfo->parentname) + ".ico"))
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

void menu_select_software::get_selection(ui_software_info const *&software, ui_system_info const *&system) const
{
	software = reinterpret_cast<ui_software_info const *>(get_selection_ptr());
	system = &m_system;
}


void menu_select_software::make_topbox_text(std::string &line0, std::string &line1, std::string &line2) const
{
	// determine the text for the header
	int vis_item = !m_search.empty() ? m_available_items : (m_available_items - 1);
	line0 = string_format(_("%1$s %2$s ( %3$d / %4$d software packages )"), emulator_info::get_appname(), bare_build_version, vis_item, m_data->swinfo().size() - 1);
	line1 = string_format(_("Driver: \"%1$s\" software list "), m_system.description);

	software_filter const *const it(m_data->current_filter());
	char const *const filter(it ? it->filter_text() : nullptr);
	if (filter)
		line2 = string_format(_("%1$s: %2$s - Search: %3$s_"), it->display_name(), filter, m_search);
	else
		line2 = string_format(_("Search: %1$s_"), m_search);
}


std::string menu_select_software::make_software_description(ui_software_info const &software, ui_system_info const *system) const
{
	// show list/item to make it less confusing when there are multiple lists mixed
	return string_format(_("Software list/item: %1$s:%2$s"), software.listname, software.shortname);
}


void menu_select_software::filter_selected()
{
	if ((software_filter::FIRST <= m_filter_highlight) && (software_filter::LAST >= m_filter_highlight))
	{
		m_data->get_filter(software_filter::type(m_filter_highlight)).show_ui(
				ui(),
				container(),
				[this] (software_filter &filter)
				{
					software_filter::type const new_type(filter.get_type());
					if (software_filter::CUSTOM == new_type)
					{
						emu_file file(ui().options().ui_path(), OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS);
						if (!file.open(util::string_format("custom_%s_filter.ini", m_system.driver->name)))
						{
							filter.save_ini(file, 0);
							file.close();
						}
					}
					m_data->set_filter_type(new_type);
					reset(reset_options::REMEMBER_REF);
				});
	}
}

} // namespace ui
