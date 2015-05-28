/***************************************************************************

	mewui/selsoft.c

	Internal MEWUI user interface.

***************************************************************************/

#include "emu.h"
#include "emuopts.h"
#include "ui/ui.h"
#include "uiinput.h"
#include "audit.h"
#include "mewui/utils.h"
#include "mewui/selsoft.h"
#include "mewui/datmenu.h"
#include "mewui/datfile.h"
#include "mewui/inifile.h"
#include "mewui/selector.h"

static const char *region_lists[] = { "arg", "asia", "aus", "aut", "bel", "blr", "bra", "can", "chi", "chn", "cze", "den",
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
//  ctor
//-------------------------------------------------

ui_menu_select_software::ui_menu_select_software(running_machine &machine, render_container *container, const game_driver *driver) : ui_menu(machine, container)
{
	if (mewui_globals::force_reselect_software)
		mewui_globals::force_reselect_software = false;

	mewui_globals::actual_sw_filter = 0;

	ui_driver = driver;
	build_software_list();

	mewui_globals::curimage_view = SNAPSHOT_VIEW;
	mewui_globals::switch_image = true;
	mewui_globals::cur_sw_dats_view = MEWUI_FIRST_LOAD;

	std::string error_string;
	machine.options().set_value(OPTION_SOFTWARENAME, "", OPTION_PRIORITY_CMDLINE, error_string);
}

//-------------------------------------------------
//  dtor
//-------------------------------------------------

ui_menu_select_software::~ui_menu_select_software()
{
	mewui_globals::curimage_view = CABINETS_VIEW;
	mewui_globals::switch_image = true;

	// free softwares list
	ui_swlist.clear();

	// free regions data
	m_region.ui.clear();

	// free years data
	m_year.ui.clear();

	// free publishers data
	m_publisher.ui.clear();
}

//-------------------------------------------------
//  handle
//-------------------------------------------------

void ui_menu_select_software::handle()
{
	bool check_filter = false;

	// ignore pause keys by swallowing them before we process the menu
	ui_input_pressed(machine(), IPT_UI_PAUSE);

	// process the menu
	const ui_menu_event *menu_event = process(UI_MENU_PROCESS_LR_REPEAT);

	if (menu_event != NULL && menu_event->itemref != NULL)
	{
		// reset the error on any future menu_event
		if (ui_error)
			ui_error = false;

		// handle selections
		else if (menu_event->iptkey == IPT_UI_SELECT)
			inkey_select(menu_event);

		// handle UI_LEFT
		else if (menu_event->iptkey == IPT_UI_LEFT)
		{
			// Images
			if (mewui_globals::rpanel_infos == RP_IMAGES && mewui_globals::curimage_view > FIRST_VIEW)
			{
				mewui_globals::curimage_view--;
				mewui_globals::switch_image = true;
				mewui_globals::default_image = false;
			}

			// Infos
			else if (mewui_globals::rpanel_infos == RP_INFOS && mewui_globals::cur_sw_dats_view > 0)
			{
				mewui_globals::cur_sw_dats_view--;
				topline_datsview = 0;
			}
		}

		// handle UI_RIGHT
		else if (menu_event->iptkey == IPT_UI_RIGHT)
		{
			// Images
			if (mewui_globals::rpanel_infos == RP_IMAGES && mewui_globals::curimage_view < LAST_VIEW)
			{
				mewui_globals::curimage_view++;
				mewui_globals::switch_image = true;
				mewui_globals::default_image = false;
			}

			// Infos
			else if (mewui_globals::rpanel_infos == RP_INFOS && mewui_globals::cur_sw_dats_view < 1)
			{
				mewui_globals::cur_sw_dats_view++;
				topline_datsview = 0;
			}
		}

		// handle UI_HISTORY
		else if (menu_event->iptkey == IPT_UI_HISTORY && machine().options().enabled_dats())
		{
			ui_software_info *ui_swinfo = (ui_software_info *)menu_event->itemref;

			if ((FPTR)ui_swinfo > 1)
				ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_history_sw(machine(), container, ui_swinfo, ui_driver)));
		}

		// handle UI_UP_FILTER
		else if (menu_event->iptkey == IPT_UI_UP_FILTER && mewui_globals::actual_sw_filter > MEWUI_SW_FIRST)
		{
			l_sw_hover = mewui_globals::actual_sw_filter - 1;
			check_filter = true;
		}

		// handle UI_DOWN_FILTER
		else if (menu_event->iptkey == IPT_UI_DOWN_FILTER && mewui_globals::actual_sw_filter < MEWUI_SW_LAST)
		{
			l_sw_hover = mewui_globals::actual_sw_filter + 1;
			check_filter = true;
		}

		// handle UI_LEFT_PANEL
		else if (menu_event->iptkey == IPT_UI_LEFT_PANEL)
			mewui_globals::rpanel_infos = RP_IMAGES;

		// handle UI_RIGHT_PANEL
		else if (menu_event->iptkey == IPT_UI_RIGHT_PANEL)
			mewui_globals::rpanel_infos = RP_INFOS;

		// escape pressed with non-empty text clears the text
		else if (menu_event->iptkey == IPT_UI_CANCEL && m_search[0] != 0)
		{
			m_search[0] = '\0';
			reset(UI_MENU_RESET_SELECT_FIRST);
		}

		// handle UI_FAVORITES
		else if (menu_event->iptkey == IPT_UI_FAVORITES)
		{
			ui_software_info *swinfo = (ui_software_info *)menu_event->itemref;

			if ((FPTR)swinfo > 2)
			{
				if (!machine().favorite().isgame_favorite(*swinfo))
				{
					machine().favorite().add_favorite_game(*swinfo);
					popmessage("%s\n added to favorites list.", swinfo->longname.c_str());
				}

				else
				{
					popmessage("%s\n removed from favorites list.", swinfo->longname.c_str());
					machine().favorite().remove_favorite_game();
				}
			}
		}

		// typed characters append to the buffer
		else if (menu_event->iptkey == IPT_SPECIAL)
			inkey_special(menu_event);

		else if (menu_event->iptkey == IPT_OTHER)
			check_filter = true;
	}

	if (menu_event != NULL && menu_event->itemref == NULL)
	{
		// reset the error on any future menu_event
		if (ui_error)
			ui_error = false;

		else if (menu_event->iptkey == IPT_OTHER)
			check_filter = true;

		// handle UI_UP_FILTER
		else if (menu_event->iptkey == IPT_UI_UP_FILTER && mewui_globals::actual_sw_filter > MEWUI_SW_FIRST)
		{
			l_sw_hover = mewui_globals::actual_sw_filter - 1;
			check_filter = true;
		}

		// handle UI_DOWN_FILTER
		else if (menu_event->iptkey == IPT_UI_DOWN_FILTER && mewui_globals::actual_sw_filter < MEWUI_SW_LAST)
		{
			l_sw_hover = mewui_globals::actual_sw_filter + 1;
			check_filter = true;
		}
	}

	// if we're in an error state, overlay an error message
	if (ui_error)
		machine().ui().draw_text_box(container,
									"The selected software is missing one or more required files. "
									"Please select a different software.\n\nPress any key (except ESC) to continue.",
									JUSTIFY_CENTER, 0.5f, 0.5f, UI_RED_COLOR);

	// handle filters selection from key shortcuts
	if (check_filter)
	{
		m_search[0] = '\0';

		if (l_sw_hover == MEWUI_SW_REGION)
			ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_selector(machine(), container, m_region.ui,
													&m_region.actual, SELECTOR_SOFTWARE, l_sw_hover)));
		else if (l_sw_hover == MEWUI_SW_YEARS)
			ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_selector(machine(), container, m_year.ui,
													&m_year.actual, SELECTOR_SOFTWARE, l_sw_hover)));
		else if (l_sw_hover == MEWUI_SW_PUBLISHERS)
			ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_selector(machine(), container, m_publisher.ui,
													&m_publisher.actual, SELECTOR_SOFTWARE, l_sw_hover)));
		else
		{
			mewui_globals::actual_sw_filter = l_sw_hover;
			reset(UI_MENU_RESET_SELECT_FIRST);
		}
	}
}

//-------------------------------------------------
//  populate
//-------------------------------------------------

void ui_menu_select_software::populate()
{
	UINT32 flags_mewui = MENU_FLAG_MEWUI_SWLIST | MENU_FLAG_LEFT_ARROW | MENU_FLAG_RIGHT_ARROW;
	has_empty_start = true;
	int old_software = -1;

	machine_config config(*ui_driver, machine().options());
	image_interface_iterator iter(config.root_device());

	for (device_image_interface *image = iter.first(); image != NULL; image = iter.next())
	{
		if (image->filename() == NULL && image->must_be_loaded())
		{
				has_empty_start = false;
				break;
		}
	}

	// no active search
	if (m_search[0] == 0)
	{
		// if the device can be loaded empty, add an item
		if (has_empty_start)
			item_append("[Start empty]", NULL, flags_mewui, (void *)&ui_swlist[0]);

		build_list();

		// iterate over entries
		for (int curitem = 0; m_displaylist[curitem]; curitem++)
		{
			if (reselect_last::software.compare("[Start empty]") == 0 && !reselect_last::driver.empty())
				old_software = 0;

			else if (!reselect_last::software.empty() && m_displaylist[curitem]->shortname.compare(reselect_last::software) == 0
							&& m_displaylist[curitem]->part.compare(reselect_last::part) == 0)
					old_software = has_empty_start ? curitem + 1 : curitem;

			item_append(m_displaylist[curitem]->longname.c_str(), m_displaylist[curitem]->devicetype.c_str(),
						m_displaylist[curitem]->parentname.empty() ? flags_mewui : (MENU_FLAG_INVERT | flags_mewui), (void *)m_displaylist[curitem]);
		}
	}

	else
	{
		find_matches(m_search, VISIBLE_GAMES_IN_SEARCH);

		for (int curitem = 0; searchlist[curitem]; curitem++)
			item_append(searchlist[curitem]->longname.c_str(), searchlist[curitem]->devicetype.c_str(),
						searchlist[curitem]->parentname.empty() ? flags_mewui : (MENU_FLAG_INVERT | flags_mewui),
						(void *)searchlist[curitem]);
	}

	item_append(MENU_SEPARATOR_ITEM, NULL, flags_mewui, NULL);

	// configure the custom rendering
	customtop = 3.0f * machine().ui().get_line_height() + 3.0f * UI_BOX_TB_BORDER;
	custombottom = 5.0f * machine().ui().get_line_height() + 3.0f * UI_BOX_TB_BORDER;

	if (old_software != -1)
	{
		selected = old_software;
		top_line = selected - (mewui_globals::visible_sw_lines / 2);
	}

	reselect_last::driver.clear();
	reselect_last::software.clear();
	reselect_last::part.clear();
}

//-------------------------------------------------
//  build a list of softwares
//-------------------------------------------------

void ui_menu_select_software::build_software_list()
{
	// add start empty item
	ui_software_info first_swlist;
	first_swlist.shortname.assign(ui_driver->name);
	first_swlist.longname.assign(ui_driver->description);
	first_swlist.parentname.clear();
	first_swlist.year.clear();
	first_swlist.publisher.clear();
	first_swlist.supported = 0;
	first_swlist.part.clear();
	first_swlist.driver = ui_driver;
	first_swlist.listname.clear();
	first_swlist.interface.clear();
	first_swlist.instance.clear();
	first_swlist.startempty = 1;
	first_swlist.parentlongname.clear();
	first_swlist.usage.clear();
	first_swlist.devicetype.clear();
	first_swlist.available = true;
	ui_swlist.push_back(first_swlist);

	machine_config config(*ui_driver, machine().options());
	software_list_device_iterator deviter(config.root_device());

	// iterate thru all software lists
	for (software_list_device *swlist = deviter.first(); swlist != NULL; swlist = deviter.next())
	{
		for (software_info *swinfo = swlist->first_software_info(); swinfo != NULL; swinfo = swinfo->next())
		{
			software_part *part = swinfo->first_part();
			if (part->is_compatible(*swlist))
			{
				const char *instance_name = NULL;
				const char *type_name = NULL;
				ui_software_info tmpmatches;
				image_interface_iterator imgiter(config.root_device());
				for (device_image_interface *image = imgiter.first(); image != NULL; image = imgiter.next())
				{
					const char *interface = image->image_interface();
					if (interface != NULL && part->matches_interface(interface))
					{
						instance_name = image->instance_name();
						if (instance_name != NULL)
							tmpmatches.instance.assign(image->instance_name());

						type_name = image->image_type_name();
						if (type_name != NULL)
							tmpmatches.devicetype.assign(type_name);
						break;
					}
				}

				if (instance_name == NULL || type_name == NULL)
					continue;

				if (swinfo->shortname()) tmpmatches.shortname.assign(swinfo->shortname());
				if (swinfo->longname()) tmpmatches.longname.assign(swinfo->longname());
				if (swinfo->parentname()) tmpmatches.parentname.assign(swinfo->parentname());
				if (swinfo->year()) tmpmatches.year.assign(swinfo->year());
				if (swinfo->publisher()) tmpmatches.publisher.assign(swinfo->publisher());
				tmpmatches.supported = swinfo->supported();
				if (part->name()) tmpmatches.part.assign(part->name());
				tmpmatches.driver = ui_driver;
				if (swlist->list_name()) tmpmatches.listname.assign(swlist->list_name());
				if (part->interface()) tmpmatches.interface.assign(part->interface());
				tmpmatches.startempty = 0;
				tmpmatches.parentlongname.clear();
				tmpmatches.usage.clear();
				tmpmatches.available = true;

				for (feature_list_item *flist = swinfo->other_info(); flist != NULL; flist = flist->next())
					if (!strcmp(flist->name(), "usage"))
						tmpmatches.usage.assign(flist->value());

				ui_swlist.push_back(tmpmatches);
				m_region.set(tmpmatches.longname.c_str());
				m_publisher.set(tmpmatches.publisher.c_str());
				m_year.set(tmpmatches.year.c_str());
			}
		}
	}

	// no software found? start directly the driver
	if (ui_swlist.size() == 1)
	{
		reselect_last::driver.assign(ui_swlist[0].driver->name);
		reselect_last::software.clear();
		reselect_last::part.clear();
		mewui_globals::force_reselect_software = true;
		machine().manager().schedule_new_driver(*ui_swlist[0].driver);
		machine().schedule_hard_reset();
		ui_menu::stack_reset(machine());
		return;
	}

	m_displaylist.resize(ui_swlist.size() + 1);

	// retrieve and set the long name of software for parents
	for (int y = 0; y < ui_swlist.size(); y++)
		if (ui_swlist[y].parentname[0] != '\0')
		{
			bool found = false;

			// first scan backward
			for (int x = y; x > 0; x--)
				if (ui_swlist[y].parentname.compare(ui_swlist[x].shortname) == 0 && ui_swlist[y].instance.compare(ui_swlist[x].instance) == 0)
				{
					ui_swlist[y].parentlongname.assign(ui_swlist[x].longname);
					found = true;
					break;
				}

			// not found? then scan forward
			for (int x = y; !found && x < ui_swlist.size(); x++)
				if (ui_swlist[y].parentname.compare(ui_swlist[x].shortname) == 0 && ui_swlist[y].instance.compare(ui_swlist[x].instance) == 0)
				{
					ui_swlist[y].parentlongname.assign(ui_swlist[x].longname);
					break;
				}
		}

	// sort array
	if (machine().options().ui_grouped())
		std::stable_sort(ui_swlist.begin() + 1, ui_swlist.end(), compare_software);

	std::sort(m_region.ui.begin(), m_region.ui.end());
	std::sort(m_year.ui.begin(), m_year.ui.end());
	std::sort(m_publisher.ui.begin(), m_publisher.ui.end());
}

//-------------------------------------------------
//  perform our special rendering
//-------------------------------------------------

void ui_menu_select_software::custom_render(void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
	ui_software_info *swinfo = (FPTR)selectedref > 1 ? (ui_software_info *)selectedref : NULL;
	const game_driver *driver = NULL;
	float width, maxwidth;
	float x1, y1, x2, y2;
	std::string tempbuf[5], filtered;
	rgb_t color = UI_BACKGROUND_COLOR;
	bool isstar = false;

	// determine the text for the header
	int vis_item = (m_search[0] != 0) ? visible_items : (has_empty_start ? visible_items - 1 : visible_items);
	strprintf(tempbuf[0], "MEWUI %s (%s) (%d / %d software)", mewui_version, bare_build_version, vis_item, (int)ui_swlist.size() - 1);
	tempbuf[1].assign("Driver: \"").append(ui_driver->description).append("\" software list ");

	if (mewui_globals::actual_sw_filter == MEWUI_SW_REGION && m_region.ui.size() != 0)
		filtered.assign("Region: ").append(m_region.ui[m_region.actual]).append(" - ");
	else if (mewui_globals::actual_sw_filter == MEWUI_SW_PUBLISHERS)
		filtered.assign("Publisher: ").append(m_publisher.ui[m_publisher.actual]).append(" - ");
	else if (mewui_globals::actual_sw_filter == MEWUI_SW_YEARS)
		filtered.assign("Year: ").append(m_year.ui[m_year.actual]).append(" - ");

	tempbuf[2].assign(filtered).append("Search: ").append(m_search).append("_");

	// get the size of the text
	maxwidth = origx2 - origx1;

	for (int line = 0; line < 3; line++)
	{
		machine().ui().draw_text_full(container, tempbuf[line].c_str(), 0.0f, 0.0f, 1.0f, JUSTIFY_CENTER, WRAP_NEVER,
										DRAW_NONE, ARGB_WHITE, ARGB_BLACK, &width, NULL);
		width += 2 * UI_BOX_LR_BORDER;
		maxwidth = MAX(width, origx2 - origx1);
	}

	// compute our bounds
	x1 = 0.5f - 0.5f * maxwidth;
	x2 = x1 + maxwidth;
	y1 = origy1 - top;
	y2 = origy1 - UI_BOX_TB_BORDER;

	// draw a box
	machine().ui().draw_outlined_box(container, x1, y1, x2, y2, UI_BACKGROUND_COLOR);

	// take off the borders
	x1 += UI_BOX_LR_BORDER;
	x2 -= UI_BOX_LR_BORDER;
	y1 += UI_BOX_TB_BORDER;
	y2 -= UI_BOX_TB_BORDER;

	// draw the text within it
	for (int line = 0; line < 3; line++)
	{
		machine().ui().draw_text_full(container, tempbuf[line].c_str(), x1, y1, x2 - x1, JUSTIFY_CENTER, WRAP_NEVER,
										DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, NULL, NULL);
		y1 += machine().ui().get_line_height();
	}

	// determine the text to render below
	if (swinfo && swinfo->startempty == 1)
		driver = swinfo->driver;

	if ((FPTR)driver > 1)
	{
		isstar = machine().favorite().isgame_favorite(driver);

		// first line is game description
		strprintf(tempbuf[0], "%-.100s", driver->description);

		// next line is year, manufacturer
		strprintf(tempbuf[1], "%s, %-.100s", driver->year, driver->manufacturer);

		// next line is clone/parent status
		int cloneof = driver_list::non_bios_clone(*driver);

		if (cloneof != -1)
			strprintf(tempbuf[2], "Driver is clone of: %-.100s", driver_list::driver(cloneof).description);
		else
			tempbuf[2].assign("Driver is parent");

		// next line is overall driver status
		if (driver->flags & GAME_NOT_WORKING)
			tempbuf[3].assign("Overall: NOT WORKING");
		else if (driver->flags & GAME_UNEMULATED_PROTECTION)
			tempbuf[3].assign("Overall: Unemulated Protection");
		else
			tempbuf[3].assign("Overall: Working");

		// next line is graphics, sound status
		if (driver->flags & (GAME_IMPERFECT_GRAPHICS | GAME_WRONG_COLORS | GAME_IMPERFECT_COLORS))
			tempbuf[4].assign("Graphics: Imperfect, ");
		else
			tempbuf[4].assign("Graphics: OK, ");

		if (driver->flags & GAME_NO_SOUND)
			tempbuf[4].append("Sound: Unimplemented");
		else if (driver->flags & GAME_IMPERFECT_SOUND)
			tempbuf[4].append("Sound: Imperfect");
		else
			tempbuf[4].append("Sound: OK");

		if (driver != NULL)
			color = UI_GREEN_COLOR;

		if (driver != NULL && (driver->flags & (GAME_IMPERFECT_GRAPHICS | GAME_WRONG_COLORS | GAME_IMPERFECT_COLORS
												| GAME_NO_SOUND | GAME_IMPERFECT_SOUND)) != 0)
			color = UI_YELLOW_COLOR;

		if (driver != NULL && (driver->flags & (GAME_NOT_WORKING | GAME_UNEMULATED_PROTECTION)) != 0)
			color = UI_RED_COLOR;

	}

	else if ((FPTR)swinfo > 1)
	{
		isstar = machine().favorite().isgame_favorite(*swinfo);

		// first line is long name
		strprintf(tempbuf[0], "%-.100s", swinfo->longname.c_str());

		// next line is year, publisher
		strprintf(tempbuf[1], "%s, %-.100s", swinfo->year.c_str(), swinfo->publisher.c_str());

		// next line is parent/clone
		if (swinfo->parentname[0] != '\0')
			strprintf(tempbuf[2], "Software is clone of: %-.100s", swinfo->parentlongname[0] != '\0' ? swinfo->parentlongname.c_str() : swinfo->parentname.c_str());
		else
			tempbuf[2].assign("Software is parent");

		// next line is supported status
		if (swinfo->supported == SOFTWARE_SUPPORTED_NO)
		{
			tempbuf[3].assign("Supported: No");
			color = UI_RED_COLOR;
		}

		else if (swinfo->supported == SOFTWARE_SUPPORTED_PARTIAL)
		{
			tempbuf[3].assign("Supported: Partial");
			color = UI_YELLOW_COLOR;
		}

		else
		{
			tempbuf[3].assign("Supported: Yes");
			color = UI_GREEN_COLOR;
		}

		// last line is romset name
		strprintf(tempbuf[4], "romset: %-.100s", swinfo->shortname.c_str());
	}

	else
	{
		std::string copyright(emulator_info::get_copyright());
		size_t found = copyright.find("\n");

		tempbuf[0].assign(emulator_info::get_applongname()).append(" ").append(build_version);
		tempbuf[1].assign(copyright.substr(0, found));
		tempbuf[2].assign(copyright.substr(found + 1));
		tempbuf[3].clear();
		tempbuf[4].assign("MEWUI by dankan1890 http://sourceforge.net/projects/mewui");
	}

	// get the size of the text
	maxwidth = origx2 - origx1;

	for (int line = 0; line < 5; line++)
	{
		machine().ui().draw_text_full(container, tempbuf[line].c_str(), 0.0f, 0.0f, 1.0f, JUSTIFY_CENTER, WRAP_NEVER,
										DRAW_NONE, ARGB_WHITE, ARGB_BLACK, &width, NULL);
		width += 2 * UI_BOX_LR_BORDER;
		maxwidth = MAX(maxwidth, width);
	}

	// compute our bounds
	x1 = 0.5f - 0.5f * maxwidth;
	x2 = x1 + maxwidth;
	y1 = origy2 + UI_BOX_TB_BORDER;
	y2 = origy2 + bottom;

	// draw a box
	machine().ui().draw_outlined_box(container, x1, y1, x2, y2, color);

	// take off the borders
	x1 += UI_BOX_LR_BORDER;
	x2 -= UI_BOX_LR_BORDER;
	y1 += UI_BOX_TB_BORDER;

	// is favorite? draw the star
	if (isstar)
		draw_star(container, x1, y1);

	// draw all lines
	for (int line = 0; line < 5; line++)
	{
		machine().ui().draw_text_full(container, tempbuf[line].c_str(), x1, y1, x2 - x1, JUSTIFY_CENTER, WRAP_NEVER,
										DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, NULL, NULL);
		y1 += machine().ui().get_line_height();
	}
}

//-------------------------------------------------
//  handle select key event
//-------------------------------------------------

void ui_menu_select_software::inkey_select(const ui_menu_event *menu_event)
{
	ui_software_info *ui_swinfo = (ui_software_info *)menu_event->itemref;

	if (ui_swinfo->startempty == 1)
	{
		reselect_last::driver.assign(ui_swinfo->driver->name);
		reselect_last::software.assign("[Start empty]");
		reselect_last::part.clear();
		mewui_globals::force_reselect_software = true;
		machine().manager().schedule_new_driver(*ui_swinfo->driver);
		machine().schedule_hard_reset();
		ui_menu::stack_reset(machine());
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

		if (summary == media_auditor::CORRECT || summary == media_auditor::BEST_AVAILABLE)
		{
			std::string error_string;
			std::string string_list = std::string(ui_swinfo->listname).append(":").append(ui_swinfo->shortname).append(":").append(ui_swinfo->part).append(":").append(ui_swinfo->instance);
			machine().options().set_value(OPTION_SOFTWARENAME, string_list.c_str(), OPTION_PRIORITY_CMDLINE, error_string);

			reselect_last::driver.assign(drivlist.driver().name);
			reselect_last::software.assign(ui_swinfo->shortname.c_str());
			reselect_last::part.assign(ui_swinfo->part.c_str());
			mewui_globals::force_reselect_software = true;

			std::string snap_list = std::string(ui_swinfo->listname).append("/").append(ui_swinfo->shortname);
			machine().options().set_value(OPTION_SNAPNAME, snap_list.c_str(), OPTION_PRIORITY_CMDLINE, error_string);

			machine().manager().schedule_new_driver(drivlist.driver());
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
//  handle special key event
//-------------------------------------------------

void ui_menu_select_software::inkey_special(const ui_menu_event *menu_event)
{
	int buflen = strlen(m_search);

	// if it's a backspace and we can handle it, do so
	if ((menu_event->unichar == 8 || menu_event->unichar == 0x7f) && buflen > 0)
	{
		*(char *)utf8_previous_char(&m_search[buflen]) = 0;
		reset(UI_MENU_RESET_SELECT_FIRST);
	}

	// if it's any other key and we're not maxed out, update
	else if (menu_event->unichar >= ' ' && menu_event->unichar < 0x7f)
	{
		buflen += utf8_from_uchar(&m_search[buflen], ARRAY_LENGTH(m_search) - buflen, menu_event->unichar);
		m_search[buflen] = 0;
		reset(UI_MENU_RESET_SELECT_FIRST);
	}
}

//-------------------------------------------------
//  set software regions
//-------------------------------------------------

void c_sw_region::set(const char *str)
{
	std::string name = getname(str);
	if (std::find(ui.begin(), ui.end(), name) != ui.end())
		return;

	ui.push_back(name);
}

std::string c_sw_region::getname(const char *str)
{
	std::string fullname(str), name(str);
	strmakelower(fullname);
	size_t found = fullname.find("(");

	if (found != std::string::npos)
	{
		size_t ends = fullname.find_first_not_of("abcdefghijklmnopqrstuvwxyz", found + 1);
		std::string temp(fullname.substr(found + 1, ends - found - 1));

		for (int x = 0; x < ARRAY_LENGTH(region_lists); x++)
			if (temp.compare(region_lists[x]) == 0)
				return (name.substr(found + 1, ends - found - 1));
	}
	return std::string("<none>");
}

//-------------------------------------------------
//  set software years
//-------------------------------------------------

void c_sw_year::set(const char *str)
{
	std::string name(str);
	if (std::find(ui.begin(), ui.end(), name) != ui.end())
		return;

	ui.push_back(name);
}

//-------------------------------------------------
//  set software publishers
//-------------------------------------------------

void c_sw_publisher::set(const char *str)
{
	std::string name = getname(str);
	if (std::find(ui.begin(), ui.end(), name) != ui.end())
		return;

	ui.push_back(name);
}

std::string c_sw_publisher::getname(const char *str)
{
	std::string name(str);
	size_t found = name.find("(");

	if (found != std::string::npos)
		return (name.substr(0, found - 1));
	else
		return name;
}

//-------------------------------------------------
//  build display list
//-------------------------------------------------

void ui_menu_select_software::build_list()
{
	int arrayindex = 0;

	// iterate over entries
	for (int x = 1; x < ui_swlist.size(); x++)
	{
		switch (mewui_globals::actual_sw_filter)
		{
			case MEWUI_SW_PARENTS:
				if (ui_swlist[x].parentname.empty())
					m_displaylist[arrayindex++] = &ui_swlist[x];
				break;

			case MEWUI_SW_CLONES:
				if (!ui_swlist[x].parentname.empty())
					m_displaylist[arrayindex++] = &ui_swlist[x];
				break;

			case MEWUI_SW_AVAILABLE:
				if (ui_swlist[x].available)
					m_displaylist[arrayindex++] = &ui_swlist[x];
				 break;

			case MEWUI_SW_UNAVAILABLE:
				if (!ui_swlist[x].available)
					m_displaylist[arrayindex++] = &ui_swlist[x];
				 break;

			case MEWUI_SW_SUPPORTED:
				if (ui_swlist[x].supported != SOFTWARE_SUPPORTED_NO && ui_swlist[x].supported != SOFTWARE_SUPPORTED_PARTIAL)
					m_displaylist[arrayindex++] = &ui_swlist[x];
				break;

			case MEWUI_SW_PARTIAL_SUPPORTED:
				if (ui_swlist[x].supported == SOFTWARE_SUPPORTED_PARTIAL)
					m_displaylist[arrayindex++] = &ui_swlist[x];
				break;

			case MEWUI_SW_UNSUPPORTED:
				if (ui_swlist[x].supported == SOFTWARE_SUPPORTED_NO)
					m_displaylist[arrayindex++] = &ui_swlist[x];
				break;

			case MEWUI_SW_REGION:
			{
				std::string name = m_region.getname(ui_swlist[x].longname.c_str());

				if(!name.empty() && m_region.ui[m_region.actual].compare(name) == 0)
					m_displaylist[arrayindex++] = &ui_swlist[x];
				break;
			}

			case MEWUI_SW_PUBLISHERS:
			{
				std::string name = m_publisher.getname(ui_swlist[x].publisher.c_str());

				if(!name.empty() && m_publisher.ui[m_publisher.actual].compare(name) == 0)
					m_displaylist[arrayindex++] = &ui_swlist[x];
				break;
			}

			case MEWUI_SW_YEARS:
				if(ui_swlist[x].year == m_year.ui[m_year.actual])
					m_displaylist[arrayindex++] = &ui_swlist[x];
				break;

			default:
				m_displaylist[arrayindex++] = &ui_swlist[x];
				break;
		}
	}
	m_displaylist[arrayindex] = NULL;
}

//-------------------------------------------------
//  find approximate matches
//-------------------------------------------------

void ui_menu_select_software::find_matches(const char *str, int count)
{
	// allocate memory to track the penalty value
	std::vector<int> penalty(count, 9999);
	int index = 0;

	for (; m_displaylist[index]; index++)
	{
		// pick the best match between driver name and description
		int curpenalty = driver_list::penalty_compare(str, m_displaylist[index]->longname.c_str());
		int tmp = driver_list::penalty_compare(str, m_displaylist[index]->shortname.c_str());
		curpenalty = MIN(curpenalty, tmp);

		// insert into the sorted table of matches
		for (int matchnum = count - 1; matchnum >= 0; matchnum--)
		{
			// stop if we're worse than the current entry
			if (curpenalty >= penalty[matchnum])
				break;

			// as long as this isn't the last entry, bump this one down
			if (matchnum < count - 1)
			{
				penalty[matchnum + 1] = penalty[matchnum];
				searchlist[matchnum + 1] = searchlist[matchnum];
			}

			searchlist[matchnum] = m_displaylist[index];
			penalty[matchnum] = curpenalty;
		}
	}
	(index < count) ? searchlist[index] = NULL : searchlist[count] = NULL;
}
