// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles, Nathan Woods
/***************************************************************************

    ui/selgame.c

    Game selector

***************************************************************************/

#include "emu.h"
#include "emuopts.h"
#include "ui/ui.h"
#include "ui/menu.h"
#include "uiinput.h"
#include "ui/selgame.h"
#include "ui/inputmap.h"
#include "ui/miscmenu.h"
#include "audit.h"
#include <ctype.h>


//-------------------------------------------------
//  ctor
//-------------------------------------------------

ui_menu_select_game::ui_menu_select_game(running_machine &machine, render_container *container, const char *gamename) : ui_menu(machine, container), m_driverlist(driver_list::total() + 1)
{
	build_driver_list();
	if(gamename)
		strcpy(m_search, gamename);
	m_matchlist[0] = -1;
}


//-------------------------------------------------
//  dtor
//-------------------------------------------------

ui_menu_select_game::~ui_menu_select_game()
{
}



//-------------------------------------------------
//  build_driver_list - build a list of available
//  drivers
//-------------------------------------------------

void ui_menu_select_game::build_driver_list()
{
	// start with an empty list
	m_drivlist.reset(global_alloc(driver_enumerator(machine().options())));
	m_drivlist->exclude_all();

	// open a path to the ROMs and find them in the array
	file_enumerator path(machine().options().media_path());
	const osd_directory_entry *dir;

	// iterate while we get new objects
	while ((dir = path.next()) != NULL)
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
	m_driverlist[listnum] = NULL;
}



//-------------------------------------------------
//  handle - handle the game select menu
//-------------------------------------------------

void ui_menu_select_game::handle()
{
	// ignore pause keys by swallowing them before we process the menu
	ui_input_pressed(machine(), IPT_UI_PAUSE);

	// process the menu
	const ui_menu_event *menu_event = process(0);
	if (menu_event != NULL && menu_event->itemref != NULL)
	{
		// reset the error on any future menu_event
		if (m_error)
			m_error = false;

		// handle selections
		else
		{
			switch(menu_event->iptkey)
			{
				case IPT_UI_SELECT:
					inkey_select(menu_event);
					break;
				case IPT_UI_CANCEL:
					inkey_cancel(menu_event);
					break;
				case IPT_SPECIAL:
					inkey_special(menu_event);
					break;
			}
		}
	}

	// if we're in an error state, overlay an error message
	if (m_error)
		machine().ui().draw_text_box(container,
							"The selected game is missing one or more required ROM or CHD images. "
							"Please select a different game.\n\nPress any key to continue.",
							JUSTIFY_CENTER, 0.5f, 0.5f, UI_RED_COLOR);
}


//-------------------------------------------------
//  inkey_select
//-------------------------------------------------

void ui_menu_select_game::inkey_select(const ui_menu_event *menu_event)
{
	const game_driver *driver = (const game_driver *)menu_event->itemref;

	// special case for configure inputs
	if ((FPTR)driver == 1)
		ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_input_groups(machine(), container)));

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
			machine().manager().schedule_new_driver(*driver);
			machine().schedule_hard_reset();
			ui_menu::stack_reset(machine());
		}

		// otherwise, display an error
		else
		{
			reset(UI_MENU_RESET_REMEMBER_REF);
			m_error = true;
		}
	}
}


//-------------------------------------------------
//  inkey_cancel
//-------------------------------------------------

void ui_menu_select_game::inkey_cancel(const ui_menu_event *menu_event)
{
	// escape pressed with non-empty text clears the text
	if (m_search[0] != 0)
	{
		// since we have already been popped, we must recreate ourself from scratch
		ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_select_game(machine(), container, NULL)));
	}
}


//-------------------------------------------------
//  inkey_special - typed characters append to the buffer
//-------------------------------------------------

void ui_menu_select_game::inkey_special(const ui_menu_event *menu_event)
{
	// typed characters append to the buffer
	int buflen = strlen(m_search);

	// if it's a backspace and we can handle it, do so
	if ((menu_event->unichar == 8 || menu_event->unichar == 0x7f) && buflen > 0)
	{
		*(char *)utf8_previous_char(&m_search[buflen]) = 0;
		m_rerandomize = true;
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
//  populate - populate the game select menu
//-------------------------------------------------

void ui_menu_select_game::populate()
{
	int matchcount;
	int curitem;

	for (curitem = matchcount = 0; m_driverlist[curitem] != NULL && matchcount < VISIBLE_GAMES_IN_LIST; curitem++)
		if (!(m_driverlist[curitem]->flags & MACHINE_NO_STANDALONE))
			matchcount++;

	// if nothing there, add a single multiline item and return
	if (matchcount == 0)
	{
		std::string txt;
		strprintf(txt, "No %s found. Please check the rompath specified in the %s.ini file.\n\n"
					"If this is your first time using %s, please see the config.txt file in "
					"the docs directory for information on configuring %s.",
					emulator_info::get_gamesnoun(),
					emulator_info::get_configname(),
					emulator_info::get_appname(),emulator_info::get_appname() );
		item_append(txt.c_str(), NULL, MENU_FLAG_MULTILINE | MENU_FLAG_REDTEXT, NULL);
		return;
	}

	// otherwise, rebuild the match list
	assert(m_drivlist != NULL);
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
			item_append(m_drivlist->driver(curmatch).name, m_drivlist->driver(curmatch).description, (cloneof == -1) ? 0 : MENU_FLAG_INVERT, (void *)&m_drivlist->driver(curmatch));
		}
	}

	// if we're forced into this, allow general input configuration as well
	if (ui_menu::stack_has_special_main_menu())
	{
		item_append(MENU_SEPARATOR_ITEM, NULL, 0, NULL);
		item_append("Configure General Inputs", NULL, 0, (void *)1);
	}

	// configure the custom rendering
	customtop = machine().ui().get_line_height() + 3.0f * UI_BOX_TB_BORDER;
	custombottom = 4.0f * machine().ui().get_line_height() + 3.0f * UI_BOX_TB_BORDER;
}


//-------------------------------------------------
//  custom_render - perform our special rendering
//-------------------------------------------------

void ui_menu_select_game::custom_render(void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
	const game_driver *driver;
	float width, maxwidth;
	float x1, y1, x2, y2;
	std::string tempbuf[5];
	rgb_t color;
	int line;

	// display the current typeahead
	if (m_search[0] != 0)
		strprintf(tempbuf[0], "Type name or select: %s_", m_search);
	else
		strprintf(tempbuf[0],"Type name or select: (random)");

	// get the size of the text
	machine().ui().draw_text_full(container, tempbuf[0].c_str(), 0.0f, 0.0f, 1.0f, JUSTIFY_CENTER, WRAP_TRUNCATE,
						DRAW_NONE, ARGB_WHITE, ARGB_BLACK, &width, NULL);
	width += 2 * UI_BOX_LR_BORDER;
	maxwidth = MAX(width, origx2 - origx1);

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

	// draw the text within it
	machine().ui().draw_text_full(container, tempbuf[0].c_str(), x1, y1, x2 - x1, JUSTIFY_CENTER, WRAP_TRUNCATE,
						DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, NULL, NULL);

	// determine the text to render below
	driver = ((FPTR)selectedref > 1) ? (const game_driver *)selectedref : NULL;
	if ((FPTR)driver > 1)
	{
		const char *gfxstat, *soundstat;

		// first line is game name
		strprintf(tempbuf[0],"%-.100s", driver->description);

		// next line is year, manufacturer
		strprintf(tempbuf[1], "%s, %-.100s", driver->year, driver->manufacturer);

		// next line source path
		strprintf(tempbuf[2],"Driver: %-.100s", core_filename_extract_base(tempbuf[3], driver->source_file).c_str());

		// next line is overall driver status
		if (driver->flags & MACHINE_NOT_WORKING)
			tempbuf[3].assign("Overall: NOT WORKING");
		else if (driver->flags & MACHINE_UNEMULATED_PROTECTION)
			tempbuf[3].assign("Overall: Unemulated Protection");
		else
			tempbuf[3].assign("Overall: Working");

		// next line is graphics, sound status
		if (driver->flags & (MACHINE_IMPERFECT_GRAPHICS | MACHINE_WRONG_COLORS | MACHINE_IMPERFECT_COLORS))
			gfxstat = "Imperfect";
		else
			gfxstat = "OK";

		if (driver->flags & MACHINE_NO_SOUND)
			soundstat = "Unimplemented";
		else if (driver->flags & MACHINE_IMPERFECT_SOUND)
			soundstat = "Imperfect";
		else
			soundstat = "OK";

		strprintf(tempbuf[4], "Gfx: %s, Sound: %s", gfxstat, soundstat);
	}
	else
	{
		const char *s = emulator_info::get_copyright();
		line = 0;

		// first line is version string
		strprintf(tempbuf[line++], "%s %s", emulator_info::get_applongname(), build_version);

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
		machine().ui().draw_text_full(container, tempbuf[line].c_str(), 0.0f, 0.0f, 1.0f, JUSTIFY_CENTER, WRAP_TRUNCATE,
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
	color = UI_BACKGROUND_COLOR;
	if (driver != NULL)
		color = UI_GREEN_COLOR;
	if (driver != NULL && (driver->flags & (MACHINE_IMPERFECT_GRAPHICS | MACHINE_WRONG_COLORS | MACHINE_IMPERFECT_COLORS | MACHINE_NO_SOUND | MACHINE_IMPERFECT_SOUND)) != 0)
		color = UI_YELLOW_COLOR;
	if (driver != NULL && (driver->flags & (MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION)) != 0)
		color = UI_RED_COLOR;
	machine().ui().draw_outlined_box(container, x1, y1, x2, y2, color);

	// take off the borders
	x1 += UI_BOX_LR_BORDER;
	x2 -= UI_BOX_LR_BORDER;
	y1 += UI_BOX_TB_BORDER;

	// draw all lines
	for (line = 0; line < 4; line++)
	{
		machine().ui().draw_text_full(container, tempbuf[line].c_str(), x1, y1, x2 - x1, JUSTIFY_CENTER, WRAP_TRUNCATE,
							DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, NULL, NULL);
		y1 += machine().ui().get_line_height();
	}
}


//-------------------------------------------------
//  force_game_select - force the game
//  select menu to be visible and inescapable
//-------------------------------------------------

void ui_menu_select_game::force_game_select(running_machine &machine, render_container *container)
{
	char *gamename = (char *)machine.options().system_name();

	// reset the menu stack
	ui_menu::stack_reset(machine);

	// add the quit entry followed by the game select entry
	ui_menu *quit = auto_alloc_clear(machine, ui_menu_quit_game(machine, container));
	quit->set_special_main_menu(true);
	ui_menu::stack_push(quit);
	ui_menu::stack_push(auto_alloc_clear(machine, ui_menu_select_game(machine, container, gamename)));

	// force the menus on
	machine.ui().show_menu();

	// make sure MAME is paused
	machine.pause();
}
