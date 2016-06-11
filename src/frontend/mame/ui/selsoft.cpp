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
#include "ui/datfile.h"
#include "ui/inifile.h"
#include "ui/selector.h"

#include "audit.h"
#include "drivenum.h"
#include "emuopts.h"
#include "mame.h"
#include "rendfont.h"
#include "rendutil.h"
#include "softlist.h"
#include "uiinput.h"


namespace ui {

std::string reselect_last::driver;
std::string reselect_last::software;
std::string reselect_last::swlist;
bool reselect_last::m_reselect = false;
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

	bool clonex = (x->parentname[0] != '\0');
	bool cloney = (y->parentname[0] != '\0');

	if (!clonex && !cloney)
		return (strmakelower(x->longname) < strmakelower(y->longname));

	std::string cx(x->parentlongname), cy(y->parentlongname);

	if (clonex && cx[0] == '\0')
		clonex = false;

	if (cloney && cy[0] == '\0')
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
//  get bios count
//-------------------------------------------------

bool has_multiple_bios(const game_driver *driver, s_bios &biosname)
{
	if (driver->rom == nullptr)
		return false;

	std::string default_name;
	for (const rom_entry *rom = driver->rom; !ROMENTRY_ISEND(rom); ++rom)
		if (ROMENTRY_ISDEFAULT_BIOS(rom))
			default_name = ROM_GETNAME(rom);

	for (const rom_entry *rom = driver->rom; !ROMENTRY_ISEND(rom); ++rom)
	{
		if (ROMENTRY_ISSYSTEM_BIOS(rom))
		{
			std::string name(ROM_GETHASHDATA(rom));
			std::string bname(ROM_GETNAME(rom));
			int bios_flags = ROM_GETBIOSFLAGS(rom);

			if (bname == default_name)
			{
				name.append(_(" (default)"));
				biosname.emplace(biosname.begin(), name, bios_flags - 1);
			}
			else
				biosname.emplace_back(name, bios_flags - 1);
		}
	}
	return (biosname.size() > 1);
}

//-------------------------------------------------
//  ctor
//-------------------------------------------------

menu_select_software::menu_select_software(mame_ui_manager &mui, render_container *container, const game_driver *driver) : menu(mui, container)
{
	if (reselect_last::get())
		reselect_last::set(false);

	sw_filters::actual = 0;
	highlight = 0;

	m_driver = driver;
	build_software_list();
	load_sw_custom_filters();

	ui_globals::curimage_view = SNAPSHOT_VIEW;
	ui_globals::switch_image = true;
	ui_globals::cur_sw_dats_view = UI_FIRST_LOAD;

	std::string error_string;
	mui.machine().options().set_value(OPTION_SOFTWARENAME, "", OPTION_PRIORITY_CMDLINE, error_string);
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
		if (ui_error)
		{
			// reset the error on any future event
			ui_error = false;
			machine().ui_input().reset();
		}
		else if (menu_event->iptkey == IPT_UI_SELECT)
		{
			// handle selections
			if (is_focus(focused_menu::main))
			{
				inkey_select(menu_event);
			}
			else if (is_focus(focused_menu::left))
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
				topline_datsview = 0;
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
			else if (ui_globals::rpanel == RP_INFOS && ui_globals::cur_sw_dats_view < 1)
			{
				// Infos
				ui_globals::cur_sw_dats_view++;
				topline_datsview = 0;
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
		else if (menu_event->iptkey == IPT_UI_DATS && ui().options().enabled_dats())
		{
			// handle UI_DATS
			ui_software_info *ui_swinfo = (ui_software_info *)menu_event->itemref;
			datfile_manager &mdat = mame_machine_manager::instance()->datfile();

			if (ui_swinfo->startempty == 1 && mdat.has_history(ui_swinfo->driver))
				menu::stack_push<menu_dats_view>(ui(), container, ui_swinfo->driver);
			else if (mdat.has_software(ui_swinfo->listname, ui_swinfo->shortname, ui_swinfo->parentname) || !ui_swinfo->usage.empty())
				menu::stack_push<menu_dats_view>(ui(), container, ui_swinfo);
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
		else if (menu_event->iptkey == IPT_UI_CANCEL && m_search[0] != 0)
		{
			// escape pressed with non-empty text clears the text
			m_search[0] = '\0';
			reset(reset_options::SELECT_FIRST);
		}
		else if (menu_event->iptkey == IPT_UI_FAVORITES)
		{
			// handle UI_FAVORITES
			ui_software_info *swinfo = (ui_software_info *)menu_event->itemref;

			if ((FPTR)swinfo > 2)
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
			inkey_configure(menu_event);
	}

	if (menu_event && !menu_event->itemref)
	{
		if (menu_event->iptkey == IPT_UI_CONFIGURE)
		{
			inkey_configure(menu_event);
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
				topline_datsview = 0;
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
			else if (ui_globals::rpanel == RP_INFOS && ui_globals::cur_sw_dats_view < 1)
			{
				// Infos
				ui_globals::cur_sw_dats_view++;
				topline_datsview = 0;
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
		else if (menu_event->iptkey == IPT_OTHER && is_focus(focused_menu::left))
		{
			l_sw_hover = highlight;
			check_filter = true;
			m_prev_selected = nullptr;
		}
	}

	// if we're in an error state, overlay an error message
	if (ui_error)
		ui().draw_text_box(container, _("The selected software is missing one or more required files. "
									"Please select a different software.\n\nPress any key to continue."),
									JUSTIFY_CENTER, 0.5f, 0.5f, UI_RED_COLOR);

	// handle filters selection from key shortcuts
	if (check_filter)
	{
		m_search[0] = '\0';
		switch (l_sw_hover)
		{
		case UI_SW_REGION:
			menu::stack_push<menu_selector>(ui(), container, m_filter.region.ui, m_filter.region.actual, menu_selector::SOFTWARE, l_sw_hover);
			break;
		case UI_SW_YEARS:
			menu::stack_push<menu_selector>(ui(), container, m_filter.year.ui, m_filter.year.actual, menu_selector::SOFTWARE, l_sw_hover);
			break;
		case UI_SW_LIST:
			menu::stack_push<menu_selector>(ui(), container, m_filter.swlist.description, m_filter.swlist.actual, menu_selector::SOFTWARE, l_sw_hover);
			break;
		case UI_SW_TYPE:
			menu::stack_push<menu_selector>(ui(), container, m_filter.type.ui, m_filter.type.actual, menu_selector::SOFTWARE, l_sw_hover);
			break;
		case UI_SW_PUBLISHERS:
			menu::stack_push<menu_selector>(ui(), container, m_filter.publisher.ui, m_filter.publisher.actual, menu_selector::SOFTWARE, l_sw_hover);
			break;
		case UI_SW_CUSTOM:
			sw_filters::actual = l_sw_hover;
			menu::stack_push<menu_swcustom_filter>(ui(), container, m_driver, m_filter);
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

void menu_select_software::populate()
{
	UINT32 flags_ui = FLAG_UI_SWLIST | FLAG_LEFT_ARROW | FLAG_RIGHT_ARROW;
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
	if (m_search[0] == 0)
	{
		// if the device can be loaded empty, add an item
		if (m_has_empty_start)
			item_append("[Start empty]", nullptr, flags_ui, (void *)&m_swinfo[0]);

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
			if (reselect_last::software == "[Start empty]" && !reselect_last::driver.empty())
				old_software = 0;

			else if (m_displaylist[curitem]->shortname == reselect_last::software && m_displaylist[curitem]->listname == reselect_last::swlist)
				old_software = m_has_empty_start ? curitem + 1 : curitem;

			item_append(m_displaylist[curitem]->longname.c_str(), m_displaylist[curitem]->devicetype.c_str(),
						m_displaylist[curitem]->parentname.empty() ? flags_ui : (FLAG_INVERT | flags_ui), (void *)m_displaylist[curitem]);
		}
	}

	else
	{
		find_matches(m_search, VISIBLE_GAMES_IN_SEARCH);

		for (int curitem = 0; m_searchlist[curitem] != nullptr; ++curitem)
			item_append(m_searchlist[curitem]->longname.c_str(), m_searchlist[curitem]->devicetype.c_str(),
						m_searchlist[curitem]->parentname.empty() ? flags_ui : (FLAG_INVERT | flags_ui),
						(void *)m_searchlist[curitem]);
	}

	item_append(menu_item_type::SEPARATOR);

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
	m_swinfo.emplace_back(m_driver->name, m_driver->description, "", "", "", 0, "", m_driver, "", "", "", 1, "", "", "", true);

	machine_config config(*m_driver, machine().options());

	// iterate thru all software lists
	for (software_list_device &swlist : software_list_device_iterator(config.root_device()))
	{
		m_filter.swlist.name.push_back(swlist.list_name());
		m_filter.swlist.description.push_back(swlist.description());
		for (software_info &swinfo : swlist.get_info())
		{
			software_part *part = swinfo.first_part();
			if (part->is_compatible(swlist) == SOFTWARE_IS_COMPATIBLE)
			{
				const char *instance_name = nullptr;
				const char *type_name = nullptr;
				ui_software_info tmpmatches;
				for (device_image_interface &image : image_interface_iterator(config.root_device()))
				{
					const char *interface = image.image_interface();
					if (interface != nullptr && part->matches_interface(interface))
					{
						instance_name = image.instance_name();
						if (instance_name != nullptr)
							tmpmatches.instance = image.instance_name();

						type_name = image.image_type_name();
						if (type_name != nullptr)
							tmpmatches.devicetype = type_name;
						break;
					}
				}

				if (instance_name == nullptr || type_name == nullptr)
					continue;

				tmpmatches.shortname = strensure(swinfo.shortname());
				tmpmatches.longname = strensure(swinfo.longname());
				tmpmatches.parentname = strensure(swinfo.parentname());
				tmpmatches.year = strensure(swinfo.year());
				tmpmatches.publisher = strensure(swinfo.publisher());
				tmpmatches.supported = swinfo.supported();
				tmpmatches.part = strensure(part->name());
				tmpmatches.driver = m_driver;
				tmpmatches.listname = strensure(swlist.list_name());
				tmpmatches.interface = strensure(part->interface());
				tmpmatches.startempty = 0;
				tmpmatches.parentlongname.clear();
				tmpmatches.usage.clear();
				tmpmatches.available = false;

				for (feature_list_item &flist : swinfo.other_info())
					if (!strcmp(flist.name(), "usage"))
						tmpmatches.usage = flist.value();

				m_swinfo.push_back(tmpmatches);
				m_filter.region.set(tmpmatches.longname);
				m_filter.publisher.set(tmpmatches.publisher);
				m_filter.year.set(tmpmatches.year);
				m_filter.type.set(tmpmatches.devicetype);
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
	const osd_directory_entry *dir;
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
				if (dir->type == ENTTYPE_FILE)
					name = core_filename_extract_base(dir->name, true);
				else if (dir->type == ENTTYPE_DIR && strcmp(dir->name, ".") != 0)
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
//  perform our special rendering
//-------------------------------------------------

void menu_select_software::custom_render(void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
	ui_software_info *swinfo = (selectedref != nullptr) ? (ui_software_info *)selectedref : ((m_prev_selected != nullptr) ? (ui_software_info *)m_prev_selected : nullptr);
	const game_driver *driver = nullptr;
	float width;
	std::string tempbuf[5], filtered;
	rgb_t color = UI_BACKGROUND_COLOR;
	bool isstar = false;
	float tbarspace = ui().get_line_height();
	float text_size = 1.0f;

	// determine the text for the header
	int vis_item = (m_search[0] != 0) ? visible_items : (m_has_empty_start ? visible_items - 1 : visible_items);
	tempbuf[0] = string_format(_("%1$s %2$s ( %3$d / %4$d software packages )"), emulator_info::get_appname(), bare_build_version, vis_item, m_swinfo.size() - 1);
	tempbuf[1] = string_format(_("Driver: \"%1$s\" software list "), m_driver->description);

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

	tempbuf[2] = string_format(_("%s Search: %s_"), filtered, m_search);

	// get the size of the text
	float maxwidth = origx2 - origx1;

	for (int line = 0; line < 3; ++line)
	{
		ui().draw_text_full(container, tempbuf[line].c_str(), 0.0f, 0.0f, 1.0f, JUSTIFY_CENTER, WRAP_NEVER,
			DRAW_NONE, rgb_t::white, rgb_t::black, &width, nullptr);
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
	ui().draw_outlined_box(container, x1, y1, x2, y2, UI_BACKGROUND_COLOR);

	// take off the borders
	x1 += UI_BOX_LR_BORDER;
	x2 -= UI_BOX_LR_BORDER;
	y1 += UI_BOX_TB_BORDER;

	// draw the text within it
	for (int line = 0; line < 3; ++line)
	{
		ui().draw_text_full(container, tempbuf[line].c_str(), x1, y1, x2 - x1, JUSTIFY_CENTER, WRAP_NEVER,
			DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, nullptr, nullptr, text_size);
		y1 += ui().get_line_height();
	}

	// determine the text to render below
	if (swinfo != nullptr && swinfo->startempty == 1)
		driver = swinfo->driver;

	if (driver != nullptr)
	{
		isstar = mame_machine_manager::instance()->favorite().isgame_favorite(driver);

		// first line is game description
		tempbuf[0] = string_format(_("%1$-.100s"), driver->description);

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
		isstar = mame_machine_manager::instance()->favorite().isgame_favorite(*swinfo);

		// first line is long name
		tempbuf[0] = string_format(_("%1$-.100s"), swinfo->longname.c_str());

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
		tempbuf[1] = string_format("%s %s", emulator_info::get_appname(), build_version);
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
	draw_toolbar(x1, y1, x2, y2, true);

	// get the size of the text
	maxwidth = origx2 - origx1;

	for (auto & elem : tempbuf)
	{
		ui().draw_text_full(container, elem.c_str(), 0.0f, 0.0f, 1.0f, JUSTIFY_CENTER, WRAP_NEVER,
			DRAW_NONE, rgb_t::white, rgb_t::black, &width, nullptr);
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
	ui().draw_outlined_box(container, x1, y1, x2, y2, color);

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
		ui().draw_text_full(container, elem.c_str(), x1, y1, x2 - x1, JUSTIFY_CENTER, WRAP_NEVER,
			DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, nullptr, nullptr, text_size);
		y1 += ui().get_line_height();
	}
}

//-------------------------------------------------
//  handle select key event
//-------------------------------------------------

void menu_select_software::inkey_select(const event *menu_event)
{
	ui_software_info *ui_swinfo = (ui_software_info *)menu_event->itemref;
	ui_options &mopt = ui().options();

	if (ui_swinfo->startempty == 1)
	{
		s_bios biosname;
		if (!mopt.skip_bios_menu() && has_multiple_bios(ui_swinfo->driver, biosname))
			menu::stack_push<bios_selection>(ui(), container, biosname, (void *)ui_swinfo->driver, false, true);
		else
		{
			reselect_last::driver = ui_swinfo->driver->name;
			reselect_last::software = "[Start empty]";
			reselect_last::swlist.clear();
			reselect_last::set(true);
			mame_machine_manager::instance()->schedule_new_driver(*ui_swinfo->driver);
			machine().schedule_hard_reset();
			menu::stack_reset(machine());
		}
	}

	else
	{
		// first validate
		driver_enumerator drivlist(machine().options(), *ui_swinfo->driver);
		media_auditor auditor(drivlist);
		drivlist.next();
		software_list_device *swlist = software_list_device::find_by_name(drivlist.config(), ui_swinfo->listname.c_str());
		software_info *swinfo = swlist->find(ui_swinfo->shortname.c_str());

		media_auditor::summary summary = auditor.audit_software(swlist->list_name(), swinfo, AUDIT_VALIDATE_FAST);

		if (summary == media_auditor::CORRECT || summary == media_auditor::BEST_AVAILABLE || summary == media_auditor::NONE_NEEDED)
		{
			s_bios biosname;
			if (!mopt.skip_bios_menu() && has_multiple_bios(ui_swinfo->driver, biosname))
			{
				menu::stack_push<bios_selection>(ui(), container, biosname, (void *)ui_swinfo, true, false);
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
				menu::stack_push<software_parts>(ui(), container, parts, ui_swinfo);
				return;
			}

			std::string error_string;
			std::string string_list = std::string(ui_swinfo->listname).append(":").append(ui_swinfo->shortname).append(":").append(ui_swinfo->part).append(":").append(ui_swinfo->instance);
			machine().options().set_value(OPTION_SOFTWARENAME, string_list.c_str(), OPTION_PRIORITY_CMDLINE, error_string);
			std::string snap_list = std::string(ui_swinfo->listname).append(PATH_SEPARATOR).append(ui_swinfo->shortname);
			machine().options().set_value(OPTION_SNAPNAME, snap_list.c_str(), OPTION_PRIORITY_CMDLINE, error_string);
			reselect_last::driver = drivlist.driver().name;
			reselect_last::software = ui_swinfo->shortname;
			reselect_last::swlist = ui_swinfo->listname;
			reselect_last::set(true);
			mame_machine_manager::instance()->schedule_new_driver(drivlist.driver());
			machine().schedule_hard_reset();
			menu::stack_reset(machine());
		}

		// otherwise, display an error
		else
		{
			reset(reset_options::REMEMBER_POSITION);
			ui_error = true;
		}
	}
}

//-------------------------------------------------
//  handle special key event
//-------------------------------------------------

void menu_select_software::inkey_special(const event *menu_event)
{
	auto const buflen = std::strlen(m_search);

	if ((menu_event->unichar == 8) || (menu_event->unichar == 0x7f))
	{
		// if it's a backspace and we can handle it, do so
		if (0 < buflen)
		{
			*const_cast<char *>(utf8_previous_char(&m_search[buflen])) = 0;
			reset(reset_options::SELECT_FIRST);
		}
	}
	else if (menu_event->is_char_printable())
	{
		// if it's any other key and we're not maxed out, update
		if (menu_event->append_char(m_search, buflen))
			reset(reset_options::SELECT_FIRST);
	}
}


void menu_select_software::inkey_configure(const event *menu_event)
{
	if (is_focus(focused_menu::main))
	{
		if (selected <= visible_items)
		{
			m_prev_selected = item[selected].ref;
			selected = visible_items + 1;
		}
		else
		{
			if (ui_globals::panels_status != HIDE_LEFT_PANEL)
				set_focus(focused_menu::left);

			else if (ui_globals::panels_status == HIDE_BOTH)
			{
				for (int x = 0; x < item.size(); ++x)
					if (item[x].ref == m_prev_selected)
						selected = x;
			}
			else
			{
				set_focus(focused_menu::righttop);
			}
		}
	}
	else if (is_focus(focused_menu::left))
	{
		if (ui_globals::panels_status != HIDE_RIGHT_PANEL)
		{
			set_focus(focused_menu::righttop);
		}
		else
		{
			set_focus(focused_menu::main);
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
	else if (is_focus(focused_menu::righttop))
	{
		set_focus(focused_menu::rightbottom);
	}
	else if (is_focus(focused_menu::rightbottom))
	{
		set_focus(focused_menu::main);
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
		curpenalty = MIN(curpenalty, tmp);

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
		ui().draw_outlined_box(container, x1, y1, x2, y2, UI_BACKGROUND_COLOR);

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

			if (mouse_hit && x1 <= mouse_x && x2 > mouse_x && y1 <= mouse_y && y1 + line_height > mouse_y)
			{
				bgcolor = UI_MOUSEOVER_BG_COLOR;
				fgcolor = UI_MOUSEOVER_COLOR;
				hover = phover + filter;
			}

			if (highlight == filter && is_focus(focused_menu::left))
			{
				fgcolor = rgb_t(0xff, 0xff, 0xff, 0x00);
				bgcolor = rgb_t(0xff, 0xff, 0xff, 0xff);
			}

			if (bgcolor != UI_TEXT_BG_COLOR)
				ui().draw_textured_box(container, x1, y1, x2, y1 + line_height, bgcolor, rgb_t(255, 43, 43, 43),
					hilight_main_texture, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA) | PRIMFLAG_TEXWRAP(TRUE));

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

			ui().draw_text_full(container, str.c_str(), x1t, y1, x2 - x1, JUSTIFY_LEFT, WRAP_NEVER,
				DRAW_NORMAL, fgcolor, bgcolor, nullptr, nullptr, text_size);
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

		ui().draw_outlined_box(container, x1, y1, x2, y2, rgb_t(0xEF, 0x12, 0x47, 0x7B));

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

		ui().draw_outlined_box(container, x1, y1, x2, y2, rgb_t(0xEF, 0x12, 0x47, 0x7B));

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

void menu_select_software::infos_render(void *selectedref, float origx1, float origy1, float origx2, float origy2)
{
	float line_height = ui().get_line_height();
	static std::string buffer;
	std::vector<int> xstart;
	std::vector<int> xend;
	float text_size = ui().options().infos_size();
	ui_software_info *soft = (selectedref != nullptr) ? (ui_software_info *)selectedref : ((m_prev_selected != nullptr) ? (ui_software_info *)m_prev_selected : nullptr);
	static ui_software_info *oldsoft = nullptr;
	static int old_sw_view = -1;

	float gutter_width = 0.4f * line_height * machine().render().ui_aspect() * 1.3f;
	float ud_arrow_width = line_height * machine().render().ui_aspect();
	float oy1 = origy1 + line_height;

	// apply title to right panel
	if (soft != nullptr && soft->usage.empty())
	{
		float title_size = 0.0f;

		ui().draw_text_full(container, _("History"), origx1, origy1, origx2 - origx1, JUSTIFY_CENTER, WRAP_TRUNCATE,
			DRAW_NONE, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, &title_size, nullptr);
		title_size += 0.01f;

		rgb_t fgcolor = UI_TEXT_COLOR;
		rgb_t bgcolor = UI_TEXT_BG_COLOR;
		if (is_focus(focused_menu::rightbottom))
		{
			fgcolor = rgb_t(0xff, 0xff, 0xff, 0x00);
			bgcolor = rgb_t(0xff, 0xff, 0xff, 0xff);
		}

		float middle = origx2 - origx1;

		if (bgcolor != UI_TEXT_BG_COLOR)
			ui().draw_textured_box(container, origx1 + ((middle - title_size) * 0.5f), origy1, origx1 + ((middle + title_size) * 0.5f),
				origy1 + line_height, bgcolor, rgb_t(255, 43, 43, 43), hilight_main_texture, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA) | PRIMFLAG_TEXWRAP(TRUE));

		ui().draw_text_full(container, _("History"), origx1, origy1, origx2 - origx1, JUSTIFY_CENTER, WRAP_NEVER,
			DRAW_NORMAL, fgcolor, bgcolor, nullptr, nullptr);
		ui_globals::cur_sw_dats_view = 0;
	}
	else
	{
		float title_size = 0.0f;
		float txt_lenght = 0.0f;
		std::string t_text[2];
		t_text[0] = _("History");
		t_text[1] = _("Usage");

		for (auto & elem : t_text)
		{
			ui().draw_text_full(container, elem.c_str(), origx1, origy1, origx2 - origx1, JUSTIFY_CENTER, WRAP_NEVER,
				DRAW_NONE, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, &txt_lenght, nullptr);
			txt_lenght += 0.01f;
			title_size = MAX(txt_lenght, title_size);
		}

		rgb_t fgcolor = UI_TEXT_COLOR;
		rgb_t bgcolor = UI_TEXT_BG_COLOR;
		if (is_focus(focused_menu::rightbottom))
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
			ui().draw_textured_box(container, origx1 + ((middle - title_size) * 0.5f), origy1, origx1 + ((middle + title_size) * 0.5f),
				origy1 + line_height, bgcolor, rgb_t(255, 43, 43, 43), hilight_main_texture, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA) | PRIMFLAG_TEXWRAP(TRUE));

		ui().draw_text_full(container, t_text[ui_globals::cur_sw_dats_view].c_str(), origx1, origy1, origx2 - origx1,
			JUSTIFY_CENTER, WRAP_NEVER, DRAW_NORMAL, fgcolor, bgcolor, nullptr, nullptr, tmp_size);

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
				mame_machine_manager::instance()->datfile().load_data_info(soft->driver, buffer, UI_HISTORY_LOAD);
			else
				mame_machine_manager::instance()->datfile().load_software_info(soft->listname, buffer, soft->shortname, soft->parentname);
		}
		else
			buffer = soft->usage;
	}

	if (buffer.empty())
	{
		ui().draw_text_full(container, _("No Infos Available"), origx1, (origy2 + origy1) * 0.5f, origx2 - origx1, JUSTIFY_CENTER,
			WRAP_WORD, DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, nullptr, nullptr);
		return;
	}
	else
		totallines = ui().wrap_text(container, buffer.c_str(), origx1, origy1, origx2 - origx1 - (2.0f * gutter_width), xstart, xend, text_size);

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
		std::string tempbuf;
		tempbuf.assign(buffer.substr(xstart[itemline], xend[itemline] - xstart[itemline]));

		// up arrow
		if (r == 0 && topline_datsview != 0)
			info_arrow(0, origx1, origx2, oy1, line_height, text_size, ud_arrow_width);
		// bottom arrow
		else if (r == r_visible_lines - 1 && itemline != totallines - 1)
			info_arrow(1, origx1, origx2, oy1, line_height, text_size, ud_arrow_width);
		else
			ui().draw_text_full(container, tempbuf.c_str(), origx1 + gutter_width, oy1, origx2 - origx1,
				JUSTIFY_LEFT, WRAP_TRUNCATE, DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR,
				nullptr, nullptr, text_size);
		oy1 += (line_height * text_size);
	}

	// return the number of visible lines, minus 1 for top arrow and 1 for bottom arrow
	right_visible_lines = r_visible_lines - (topline_datsview != 0) - (topline_datsview + r_visible_lines != totallines);
}

//-------------------------------------------------
//  perform our special rendering
//-------------------------------------------------

void menu_select_software::arts_render(void *selectedref, float origx1, float origy1, float origx2, float origy2)
{
	float line_height = ui().get_line_height();
	static ui_software_info *oldsoft = nullptr;
	static const game_driver *olddriver = nullptr;
	const game_driver *driver = nullptr;
	ui_software_info *soft = (selectedref != nullptr) ? (ui_software_info *)selectedref : ((m_prev_selected != nullptr) ? (ui_software_info *)m_prev_selected : nullptr);

	if (soft != nullptr && soft->startempty == 1)
	{
		driver = soft->driver;
		oldsoft = nullptr;
	}
	else
		olddriver = nullptr;

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
			container->add_quad( x1, y1, x2, y2, rgb_t::white, snapx_texture, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
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
					pathname.assign(soft->driver->name).append(soft->part);
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
			container->add_quad(x1, y1, x2, y2, rgb_t::white, snapx_texture, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
		}
	}
}

void menu_select_software::draw_right_panel(void *selectedref, float origx1, float origy1, float origx2, float origy2)
{
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

	ui().draw_outlined_box(container, origx1, origy1, origx2, origy2, rgb_t(0xEF, 0x12, 0x47, 0x7B));

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
//  ctor
//-------------------------------------------------

software_parts::software_parts(mame_ui_manager &mui, render_container *container, s_parts parts, ui_software_info *ui_info) : menu(mui, container)
{
	m_parts = parts;
	m_uiinfo = ui_info;
}

//-------------------------------------------------
//  dtor
//-------------------------------------------------

software_parts::~software_parts()
{
}

//-------------------------------------------------
//  populate
//-------------------------------------------------

void software_parts::populate()
{
	for (auto & elem : m_parts)
		item_append(elem.first.c_str(), elem.second.c_str(), 0, (void *)&elem);

	item_append(menu_item_type::SEPARATOR);
	customtop = ui().get_line_height() + (3.0f * UI_BOX_TB_BORDER);
}

//-------------------------------------------------
//  handle
//-------------------------------------------------

void software_parts::handle()
{
	// process the menu
	const event *menu_event = process(0);
	if (menu_event && (menu_event->iptkey) == IPT_UI_SELECT && menu_event->itemref)
	{
		for (auto & elem : m_parts)
		{
			if ((void*)&elem == menu_event->itemref)
			{
				std::string error_string;
				std::string string_list = std::string(m_uiinfo->listname).append(":").append(m_uiinfo->shortname).append(":").append(elem.first).append(":").append(m_uiinfo->instance);
				machine().options().set_value(OPTION_SOFTWARENAME, string_list.c_str(), OPTION_PRIORITY_CMDLINE, error_string);

				reselect_last::driver = m_uiinfo->driver->name;
				reselect_last::software = m_uiinfo->shortname;
				reselect_last::swlist = m_uiinfo->listname;
				reselect_last::set(true);

				std::string snap_list = std::string(m_uiinfo->listname).append("/").append(m_uiinfo->shortname);
				machine().options().set_value(OPTION_SNAPNAME, snap_list.c_str(), OPTION_PRIORITY_CMDLINE, error_string);

				mame_machine_manager::instance()->schedule_new_driver(*m_uiinfo->driver);
				machine().schedule_hard_reset();
				menu::stack_reset(machine());
			}
		}
	}
}

//-------------------------------------------------
//  perform our special rendering
//-------------------------------------------------

void software_parts::custom_render(void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
	float width;
	ui().draw_text_full(container, _("Software part selection:"), 0.0f, 0.0f, 1.0f, JUSTIFY_CENTER, WRAP_TRUNCATE,
									DRAW_NONE, rgb_t::white, rgb_t::black, &width, nullptr);
	width += 2 * UI_BOX_LR_BORDER;
	float maxwidth = MAX(origx2 - origx1, width);

	// compute our bounds
	float x1 = 0.5f - 0.5f * maxwidth;
	float x2 = x1 + maxwidth;
	float y1 = origy1 - top;
	float y2 = origy1 - UI_BOX_TB_BORDER;

	// draw a box
	ui().draw_outlined_box(container, x1, y1, x2, y2, UI_GREEN_COLOR);

	// take off the borders
	x1 += UI_BOX_LR_BORDER;
	x2 -= UI_BOX_LR_BORDER;
	y1 += UI_BOX_TB_BORDER;

	// draw the text within it
	ui().draw_text_full(container, _("Software part selection:"), x1, y1, x2 - x1, JUSTIFY_CENTER, WRAP_TRUNCATE,
									DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, nullptr, nullptr);
}

//-------------------------------------------------
//  ctor
//-------------------------------------------------

bios_selection::bios_selection(mame_ui_manager &mui, render_container *container, s_bios biosname, void *_driver, bool _software, bool _inlist) : menu(mui, container)
{
	m_bios = biosname;
	m_driver = _driver;
	m_software = _software;
	m_inlist = _inlist;
}

//-------------------------------------------------
//  dtor
//-------------------------------------------------

bios_selection::~bios_selection()
{
}

//-------------------------------------------------
//  populate
//-------------------------------------------------

void bios_selection::populate()
{
	for (auto & elem : m_bios)
		item_append(elem.first.c_str(), nullptr, 0, (void *)&elem.first);

	item_append(menu_item_type::SEPARATOR);
	customtop = ui().get_line_height() + (3.0f * UI_BOX_TB_BORDER);
}

//-------------------------------------------------
//  handle
//-------------------------------------------------

void bios_selection::handle()
{
	// process the menu
	const event *menu_event = process(0);
	emu_options &moptions = machine().options();
	if (menu_event && menu_event->iptkey == IPT_UI_SELECT && menu_event->itemref)
	{
		for (auto & elem : m_bios)
		{
			if ((void*)&elem.first == menu_event->itemref)
			{
				if (!m_software)
				{
					const game_driver *s_driver = (const game_driver *)m_driver;
					reselect_last::driver = s_driver->name;
					if (m_inlist)
						reselect_last::software = "[Start empty]";
					else
					{
						reselect_last::software.clear();
						reselect_last::swlist.clear();
						reselect_last::set(true);
					}

					std::string error;
					moptions.set_value(OPTION_BIOS, elem.second, OPTION_PRIORITY_CMDLINE, error);
					mame_machine_manager::instance()->schedule_new_driver(*s_driver);
					machine().schedule_hard_reset();
					menu::stack_reset(machine());
				}
				else
				{
					ui_software_info *ui_swinfo = (ui_software_info *)m_driver;
					std::string error;
					machine().options().set_value(OPTION_BIOS, elem.second, OPTION_PRIORITY_CMDLINE, error);
					driver_enumerator drivlist(machine().options(), *ui_swinfo->driver);
					drivlist.next();
					software_list_device *swlist = software_list_device::find_by_name(drivlist.config(), ui_swinfo->listname.c_str());
					software_info *swinfo = swlist->find(ui_swinfo->shortname.c_str());
					if (!ui().options().skip_parts_menu() && swinfo->has_multiple_parts(ui_swinfo->interface.c_str()))
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
						menu::stack_push<software_parts>(ui(), container, parts, ui_swinfo);
						return;
					}
					std::string error_string;
					std::string string_list = std::string(ui_swinfo->listname).append(":").append(ui_swinfo->shortname).append(":").append(ui_swinfo->part).append(":").append(ui_swinfo->instance);
					moptions.set_value(OPTION_SOFTWARENAME, string_list.c_str(), OPTION_PRIORITY_CMDLINE, error_string);
					std::string snap_list = std::string(ui_swinfo->listname).append(PATH_SEPARATOR).append(ui_swinfo->shortname);
					moptions.set_value(OPTION_SNAPNAME, snap_list.c_str(), OPTION_PRIORITY_CMDLINE, error_string);
					reselect_last::driver = drivlist.driver().name;
					reselect_last::software = ui_swinfo->shortname;
					reselect_last::swlist = ui_swinfo->listname;
					reselect_last::set(true);
					mame_machine_manager::instance()->schedule_new_driver(drivlist.driver());
					machine().schedule_hard_reset();
					menu::stack_reset(machine());
				}
			}
		}
	}
}

//-------------------------------------------------
//  perform our special rendering
//-------------------------------------------------

void bios_selection::custom_render(void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
	float width;
	ui().draw_text_full(container, _("Bios selection:"), 0.0f, 0.0f, 1.0f, JUSTIFY_CENTER, WRAP_TRUNCATE,
									DRAW_NONE, rgb_t::white, rgb_t::black, &width, nullptr);
	width += 2 * UI_BOX_LR_BORDER;
	float maxwidth = MAX(origx2 - origx1, width);

	// compute our bounds
	float x1 = 0.5f - 0.5f * maxwidth;
	float x2 = x1 + maxwidth;
	float y1 = origy1 - top;
	float y2 = origy1 - UI_BOX_TB_BORDER;

	// draw a box
	ui().draw_outlined_box(container, x1, y1, x2, y2, UI_GREEN_COLOR);

	// take off the borders
	x1 += UI_BOX_LR_BORDER;
	x2 -= UI_BOX_LR_BORDER;
	y1 += UI_BOX_TB_BORDER;

	// draw the text within it
	ui().draw_text_full(container, _("Bios selection:"), x1, y1, x2 - x1, JUSTIFY_CENTER, WRAP_TRUNCATE,
									DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, nullptr, nullptr);
}

} // namespace ui
