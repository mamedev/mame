// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles, Nathan Woods
/***************************************************************************

    ui/simpleselgame.cpp

    Game selector

***************************************************************************/

#include "emu.h"

#include "ui/simpleselgame.h"

#include "ui/ui.h"
#include "ui/miscmenu.h"
#include "ui/optsmenu.h"

#include "audit.h"
#include "drivenum.h"
#include "emuopts.h"
#include "mame.h"
#include "uiinput.h"

#include <ctype.h>


namespace ui {
//-------------------------------------------------
//  ctor
//-------------------------------------------------

simple_menu_select_game::simple_menu_select_game(mame_ui_manager &mui, render_container &container, const char *gamename) : menu(mui, container), m_driverlist(driver_list::total() + 1)
{
	build_driver_list();
	if(gamename)
		strcpy(m_search, gamename);
	m_matchlist[0] = -1;
}


//-------------------------------------------------
//  dtor
//-------------------------------------------------

simple_menu_select_game::~simple_menu_select_game()
{
}



//-------------------------------------------------
//  build_driver_list - build a list of available
//  drivers
//-------------------------------------------------

void simple_menu_select_game::build_driver_list()
{
	// start with an empty list
	m_drivlist = std::make_unique<driver_enumerator>(machine().options());
	m_drivlist->exclude_all();

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
		for (src = dir->name; *src != 0 && *src != '.' && dst < &drivername[ARRAY_LENGTH(drivername) - 1]; src++)
			*dst++ = tolower((UINT8)*src);
		*dst = 0;

		int drivnum = m_drivlist->find(drivername);
		if (drivnum != -1)
			m_drivlist->include(drivnum);
	}

	// now build the final list
	m_drivlist->reset();
	int listnum = 0;
	while (m_drivlist->next())
		m_driverlist[listnum++] = &m_drivlist->driver();

	// NULL-terminate
	m_driverlist[listnum] = nullptr;
}



//-------------------------------------------------
//  handle - handle the game select menu
//-------------------------------------------------

void simple_menu_select_game::handle()
{
	// ignore pause keys by swallowing them before we process the menu
	machine().ui_input().pressed(IPT_UI_PAUSE);

	// process the menu
	const event *menu_event = process(0);
	if (menu_event && menu_event->itemref)
	{
		if (m_error)
		{
			// reset the error on any future menu_event
			m_error = false;
			machine().ui_input().reset();
		}
		else
		{
			// handle selections
			switch(menu_event->iptkey)
			{
			case IPT_UI_SELECT:
				inkey_select(menu_event);
				break;
			case IPT_UI_CANCEL:
				inkey_cancel();
				break;
			case IPT_SPECIAL:
				inkey_special(menu_event);
				break;
			}
		}
	}

	// if we're in an error state, overlay an error message
	if (m_error)
		ui().draw_text_box(container(),
							"The selected game is missing one or more required ROM or CHD images. "
							"Please select a different game.\n\nPress any key to continue.",
							ui::text_layout::CENTER, 0.5f, 0.5f, UI_RED_COLOR);
}


//-------------------------------------------------
//  inkey_select
//-------------------------------------------------

void simple_menu_select_game::inkey_select(const event *menu_event)
{
	const game_driver *driver = (const game_driver *)menu_event->itemref;

	// special case for configure inputs
	if ((FPTR)driver == 1)
		menu::stack_push<menu_game_options>(ui(), container());
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
			mame_machine_manager::instance()->schedule_new_driver(*driver);
			machine().schedule_hard_reset();
			stack_reset();
		}

		// otherwise, display an error
		else
		{
			reset(reset_options::REMEMBER_REF);
			m_error = true;
		}
	}
}


//-------------------------------------------------
//  inkey_cancel
//-------------------------------------------------

void simple_menu_select_game::inkey_cancel()
{
	// escape pressed with non-empty text clears the text
	if (m_search[0] != 0)
	{
		m_search[0] = '\0';
		reset(reset_options::SELECT_FIRST);
	}
}


//-------------------------------------------------
//  inkey_special - typed characters append to the buffer
//-------------------------------------------------

void simple_menu_select_game::inkey_special(const event *menu_event)
{
	// typed characters append to the buffer
	auto const buflen = std::strlen(m_search);

	if ((menu_event->unichar == 8) || (menu_event->unichar == 0x7f))
	{
		// if it's a backspace and we can handle it, do so
		if (0 < buflen)
		{
			*const_cast<char *>(utf8_previous_char(&m_search[buflen])) = 0;
			m_rerandomize = true;
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


//-------------------------------------------------
//  populate - populate the game select menu
//-------------------------------------------------

void simple_menu_select_game::populate()
{
	int matchcount;
	int curitem;

	for (curitem = matchcount = 0; m_driverlist[curitem] != nullptr && matchcount < VISIBLE_GAMES_IN_LIST; curitem++)
		if (!(m_driverlist[curitem]->flags & MACHINE_NO_STANDALONE))
			matchcount++;

	// if nothing there, add a single multiline item and return
	if (matchcount == 0)
	{
		std::string txt = string_format(
				_("No machines found. Please check the rompath specified in the %1$s.ini file.\n\n"
				"If this is your first time using %2$s, please see the config.txt file in "
				"the docs directory for information on configuring %2$s."),
				emulator_info::get_configname(),
				emulator_info::get_appname());
		item_append(txt, "", FLAG_MULTILINE | FLAG_REDTEXT, nullptr);
		return;
	}

	// otherwise, rebuild the match list
	assert(m_drivlist != nullptr);
	if (m_search[0] != 0 || m_matchlist[0] == -1 || m_rerandomize)
		m_drivlist->find_approximate_matches(m_search, matchcount, m_matchlist);
	m_rerandomize = false;

	// iterate over entries
	for (curitem = 0; curitem < matchcount; curitem++)
	{
		int curmatch = m_matchlist[curitem];
		if (curmatch != -1)
		{
			int cloneof = m_drivlist->non_bios_clone(curmatch);
			item_append(m_drivlist->driver(curmatch).name, m_drivlist->driver(curmatch).description, (cloneof == -1) ? 0 : FLAG_INVERT, (void *)&m_drivlist->driver(curmatch));
		}
	}

	// if we're forced into this, allow general input configuration as well
	if (stack_has_special_main_menu())
	{
		item_append(menu_item_type::SEPARATOR);
		item_append(_("Configure Options"), "", 0, (void *)1);
		skip_main_items = 1;
	}

	// configure the custom rendering
	customtop = ui().get_line_height() + 3.0f * UI_BOX_TB_BORDER;
	custombottom = 4.0f * ui().get_line_height() + 3.0f * UI_BOX_TB_BORDER;
}


//-------------------------------------------------
//  custom_render - perform our special rendering
//-------------------------------------------------

void simple_menu_select_game::custom_render(void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
	const game_driver *driver;
	float width, maxwidth;
	float x1, y1, x2, y2;
	std::string tempbuf[5];
	rgb_t color;
	int line;

	// display the current typeahead
	if (m_search[0] != 0)
		tempbuf[0] = string_format(_("Type name or select: %1$s_"), m_search);
	else
		tempbuf[0] = _("Type name or select: (random)");

	// get the size of the text
	ui().draw_text_full(container(), tempbuf[0].c_str(), 0.0f, 0.0f, 1.0f, ui::text_layout::CENTER, ui::text_layout::TRUNCATE,
						mame_ui_manager::NONE, rgb_t::white, rgb_t::black, &width, nullptr);
	width += 2 * UI_BOX_LR_BORDER;
	maxwidth = MAX(width, origx2 - origx1);

	// compute our bounds
	x1 = 0.5f - 0.5f * maxwidth;
	x2 = x1 + maxwidth;
	y1 = origy1 - top;
	y2 = origy1 - UI_BOX_TB_BORDER;

	// draw a box
	ui().draw_outlined_box(container(), x1, y1, x2, y2, UI_BACKGROUND_COLOR);

	// take off the borders
	x1 += UI_BOX_LR_BORDER;
	x2 -= UI_BOX_LR_BORDER;
	y1 += UI_BOX_TB_BORDER;

	// draw the text within it
	ui().draw_text_full(container(), tempbuf[0].c_str(), x1, y1, x2 - x1, ui::text_layout::CENTER, ui::text_layout::TRUNCATE,
						mame_ui_manager::NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, nullptr, nullptr);

	// determine the text to render below
	driver = ((FPTR)selectedref > skip_main_items) ? (const game_driver *)selectedref : nullptr;
	if (driver != nullptr)
	{
		const char *gfxstat, *soundstat;

		// first line is game name
		tempbuf[0] = string_format(_("%1$-.100s"), driver->description);

		// next line is year, manufacturer
		tempbuf[1] = string_format(_("%1$s, %2$-.100s"), driver->year, driver->manufacturer);

		// next line source path
		tempbuf[2] = string_format(_("Driver: %1$-.100s"), core_filename_extract_base(driver->source_file).c_str());

		// next line is overall driver status
		if (driver->flags & MACHINE_NOT_WORKING)
			tempbuf[3].assign(_("Overall: NOT WORKING"));
		else if (driver->flags & MACHINE_UNEMULATED_PROTECTION)
			tempbuf[3].assign(_("Overall: Unemulated Protection"));
		else
			tempbuf[3].assign(_("Overall: Working"));

		// next line is graphics, sound status
		if (driver->flags & (MACHINE_IMPERFECT_GRAPHICS | MACHINE_WRONG_COLORS | MACHINE_IMPERFECT_COLORS))
			gfxstat = _("Imperfect");
		else
			gfxstat = _("OK");

		if (driver->flags & MACHINE_NO_SOUND)
			soundstat = _("Unimplemented");
		else if (driver->flags & MACHINE_IMPERFECT_SOUND)
			soundstat = _("Imperfect");
		else
			soundstat = _("OK");

		tempbuf[4] = string_format(_("Gfx: %s, Sound: %s"), gfxstat, soundstat);
	}
	else
	{
		const char *s = emulator_info::get_copyright();
		line = 0;

		// first line is version string
		tempbuf[line++] = string_format("%s %s", emulator_info::get_appname(), build_version);

		// output message
		while (line < ARRAY_LENGTH(tempbuf))
		{
			if (!(*s == 0 || *s == '\n'))
				tempbuf[line].push_back(*s);

			if (*s == '\n')
			{
				line++;
				s++;
			} else if (*s != 0)
				s++;
			else
				line++;
		}
	}

	// get the size of the text
	maxwidth = origx2 - origx1;
	for (line = 0; line < 4; line++)
	{
		ui().draw_text_full(container(), tempbuf[line].c_str(), 0.0f, 0.0f, 1.0f, ui::text_layout::CENTER, ui::text_layout::TRUNCATE,
							mame_ui_manager::NONE, rgb_t::white, rgb_t::black, &width, nullptr);
		width += 2 * UI_BOX_LR_BORDER;
		maxwidth = MAX(maxwidth, width);
	}

	// compute our bounds
	x1 = 0.5f - 0.5f * maxwidth;
	x2 = x1 + maxwidth;
	y1 = origy2 + UI_BOX_TB_BORDER;
	y2 = origy2 + bottom;

	// draw a box
	color = UI_BACKGROUND_COLOR;
	if (driver != nullptr)
		color = UI_GREEN_COLOR;
	if (driver != nullptr && (driver->flags & (MACHINE_IMPERFECT_GRAPHICS | MACHINE_WRONG_COLORS | MACHINE_IMPERFECT_COLORS | MACHINE_NO_SOUND | MACHINE_IMPERFECT_SOUND)) != 0)
		color = UI_YELLOW_COLOR;
	if (driver != nullptr && (driver->flags & (MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION)) != 0)
		color = UI_RED_COLOR;
	ui().draw_outlined_box(container(), x1, y1, x2, y2, color);

	// take off the borders
	x1 += UI_BOX_LR_BORDER;
	x2 -= UI_BOX_LR_BORDER;
	y1 += UI_BOX_TB_BORDER;

	// draw all lines
	for (line = 0; line < 4; line++)
	{
		ui().draw_text_full(container(), tempbuf[line].c_str(), x1, y1, x2 - x1, ui::text_layout::CENTER, ui::text_layout::TRUNCATE,
							mame_ui_manager::NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, nullptr, nullptr);
		y1 += ui().get_line_height();
	}
}


//-------------------------------------------------
//  force_game_select - force the game
//  select menu to be visible and inescapable
//-------------------------------------------------

void simple_menu_select_game::force_game_select(mame_ui_manager &mui, render_container &container)
{
	char *gamename = (char *)mui.machine().options().system_name();

	// reset the menu stack
	menu::stack_reset(mui.machine());

	// add the quit entry followed by the game select entry
	menu::stack_push_special_main<menu_quit_game>(mui, container);
	menu::stack_push<simple_menu_select_game>(mui, container, gamename);

	// force the menus on
	mui.show_menu();

	// make sure MAME is paused
	mui.machine().pause();
}

} // namespace ui
