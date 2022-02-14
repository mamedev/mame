// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles, Nathan Woods
/***************************************************************************

    ui/simpleselgame.cpp

    Game selector

***************************************************************************/

#include "emu.h"

#include "ui/simpleselgame.h"

#include "ui/info.h"
#include "ui/optsmenu.h"
#include "ui/ui.h"
#include "ui/utils.h"

#include "audit.h"
#include "drivenum.h"
#include "emuopts.h"
#include "fileio.h"
#include "mame.h"
#include "uiinput.h"

#include <cctype>


namespace ui {

//-------------------------------------------------
//  ctor
//-------------------------------------------------

simple_menu_select_game::simple_menu_select_game(mame_ui_manager &mui, render_container &container, const char *gamename)
	: menu(mui, container)
	, m_nomatch(false), m_error(false), m_rerandomize(false)
	, m_search()
	, m_driverlist(driver_list::total() + 1)
	, m_drivlist()
	, m_cached_driver(nullptr)
	, m_cached_flags(machine_flags::NOT_WORKING)
	, m_cached_unemulated(device_t::feature::NONE), m_cached_imperfect(device_t::feature::NONE)
	, m_cached_color(ui().colors().background_color())
{
	set_process_flags(PROCESS_IGNOREPAUSE);
	set_needs_prev_menu_item(false);
	build_driver_list();
	if (gamename)
		m_search.assign(gamename);
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

	// iterate while we get new objects
	for (const osd::directory::entry *dir = path.next(); dir; dir = path.next())
	{
		char drivername[50];
		char *dst = drivername;
		const char *src;

		// build a name for it
		for (src = dir->name; *src != 0 && *src != '.' && dst < &drivername[std::size(drivername) - 1]; src++)
			*dst++ = tolower((uint8_t)*src);
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

void simple_menu_select_game::handle(event const *ev)
{
	// process the menu
	if (ev)
	{
		if (m_error)
		{
			// reset the error on any subsequent menu event
			m_error = false;
			machine().ui_input().reset();
		}
		else
		{
			// handle selections
			switch(ev->iptkey)
			{
			case IPT_UI_SELECT:
				inkey_select(*ev);
				break;
			case IPT_UI_CANCEL:
				inkey_cancel();
				break;
			case IPT_SPECIAL:
				inkey_special(*ev);
				break;
			}
		}
	}

	// if we're in an error state, overlay an error message
	if (m_error)
	{
		ui().draw_text_box(
				container(),
				_("The selected game is missing one or more required ROM or CHD images. "
				"Please select a different game.\n\nPress any key to continue."),
				text_layout::text_justify::CENTER, 0.5f, 0.5f, UI_RED_COLOR);
	}
}


//-------------------------------------------------
//  inkey_select
//-------------------------------------------------

void simple_menu_select_game::inkey_select(const event &menu_event)
{
	const game_driver *driver = (const game_driver *)menu_event.itemref;

	if ((uintptr_t)driver == 1) // special case for configure inputs
	{
		menu::stack_push<menu_simple_game_options>(
				ui(),
				container(),
				[this] () { reset(reset_options::SELECT_FIRST); });
	}
	else if (!driver) // special case for previous menu
	{
		stack_pop();
	}
	else // anything else is a driver
	{
		// audit the game first to see if we're going to work
		driver_enumerator enumerator(machine().options(), *driver);
		enumerator.next();
		media_auditor auditor(enumerator);
		media_auditor::summary summary = auditor.audit_media(AUDIT_VALIDATE_FAST);

		if (summary == media_auditor::CORRECT || summary == media_auditor::BEST_AVAILABLE || summary == media_auditor::NONE_NEEDED)
		{
			// if everything looks good, schedule the new driver
			mame_machine_manager::instance()->schedule_new_driver(*driver);
			machine().schedule_hard_reset();
			stack_reset();
		}
		else
		{
			// otherwise, display an error
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
	if (!m_search.empty())
	{
		m_search.clear();
		m_rerandomize = true;
		reset(reset_options::SELECT_FIRST);
	}
}


//-------------------------------------------------
//  inkey_special - typed characters append to the buffer
//-------------------------------------------------

void simple_menu_select_game::inkey_special(const event &menu_event)
{
	// typed characters append to the buffer
	size_t old_size = m_search.size();
	if (input_character(m_search, menu_event.unichar, uchar_is_printable))
	{
		if (m_search.size() < old_size)
			m_rerandomize = true;
		reset(reset_options::SELECT_FIRST);
	}
}


//-------------------------------------------------
//  populate - populate the game select menu
//-------------------------------------------------

void simple_menu_select_game::populate(float &customtop, float &custombottom)
{
	int matchcount;
	int curitem;

	for (curitem = matchcount = 0; m_driverlist[curitem] != nullptr && matchcount < VISIBLE_GAMES_IN_LIST; curitem++)
		matchcount++;

	// if nothing there, add a single multiline item and return
	m_nomatch = !matchcount;

	// otherwise, rebuild the match list
	if (matchcount)
	{
		assert(m_drivlist != nullptr);
		if (!m_search.empty() || m_matchlist[0] == -1 || m_rerandomize)
			m_drivlist->find_approximate_matches(m_search, matchcount, m_matchlist);
		m_rerandomize = false;

		// iterate over entries
		for (curitem = 0; curitem < matchcount; curitem++)
		{
			int curmatch = m_matchlist[curitem];
			if (curmatch != -1)
			{
				int cloneof = m_drivlist->non_bios_clone(curmatch);
				item_append(
						m_drivlist->driver(curmatch).type.fullname(),
						m_drivlist->driver(curmatch).name,
						(cloneof == -1) ? 0 : FLAG_INVERT,
						(void *)&m_drivlist->driver(curmatch));
			}
		}
		item_append(menu_item_type::SEPARATOR);
	}

	// if we're forced into this, allow general input configuration as well
	if (stack_has_special_main_menu())
	{
		item_append(_("Configure Options"), 0, (void *)1);
		item_append(_("Exit"), 0, nullptr);
	}
	else
	{
		item_append(_("Return to Previous Menu"), 0, nullptr);
	}

	// configure the custom rendering
	customtop = ui().get_line_height() + 3.0f * ui().box_tb_border();
	custombottom = 4.0f * ui().get_line_height() + 3.0f * ui().box_tb_border();
}


//-------------------------------------------------
//  custom_render - perform our special rendering
//-------------------------------------------------

void simple_menu_select_game::custom_render(void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
	// if no matches, display the error message
	if (m_nomatch)
	{
		ui().draw_text_box(
				container(),
				string_format(
						_("No machines found. Please check the rompath specified in the %1$s.ini file.\n\n"
						"If this is your first time using %2$s, please see the config.txt file in "
						"the docs directory for information on configuring %2$s."),
						emulator_info::get_configname(),
						emulator_info::get_appname()),
				text_layout::text_justify::CENTER,
				0.5f, origy2 + ui().box_tb_border() + (0.5f * (bottom - ui().box_tb_border())),
				UI_RED_COLOR);
		return;
	}

	const game_driver *driver;
	std::string tempbuf[5];

	// display the current typeahead
	if (!m_search.empty())
		tempbuf[0] = string_format(_("Type name or select: %1$s_"), m_search);
	else
		tempbuf[0] = _("Type name or select: (random)");

	// draw the top box
	draw_text_box(
			tempbuf, tempbuf + 1,
			origx1, origx2, origy1 - top, origy1 - ui().box_tb_border(),
			text_layout::text_justify::CENTER, text_layout::word_wrapping::TRUNCATE, false,
			ui().colors().text_color(), ui().colors().background_color(), 1.0f);

	// determine the text to render below
	driver = ((uintptr_t)selectedref > 1) ? (const game_driver *)selectedref : nullptr;
	if (driver)
	{
		// first line is game name
		tempbuf[0] = string_format(_("%1$-.100s"), driver->type.fullname());

		// next line is year, manufacturer
		tempbuf[1] = string_format(_("%1$s, %2$-.100s"), driver->year, driver->manufacturer);

		// next line source path
		tempbuf[2] = string_format(_("Driver: %1$s"), core_filename_extract_base(driver->type.source()));

		// update cached values if selection changed
		if (driver != m_cached_driver)
		{
			emu_options clean_options;
			machine_static_info const info(ui().options(), machine_config(*driver, clean_options));
			m_cached_driver = driver;
			m_cached_flags = info.machine_flags();
			m_cached_unemulated = info.unemulated_features();
			m_cached_imperfect = info.imperfect_features();
			m_cached_color = info.status_color();
		}

		// next line is overall driver status
		if (m_cached_flags & machine_flags::NOT_WORKING)
			tempbuf[3] = _("Overall: NOT WORKING");
		else if ((m_cached_unemulated | m_cached_imperfect) & device_t::feature::PROTECTION)
			tempbuf[3] = _("Overall: Unemulated Protection");
		else
			tempbuf[3] = _("Overall: Working");

		// next line is graphics, sound status
		if (m_cached_unemulated & device_t::feature::GRAPHICS)
			tempbuf[4] = _("Graphics: Unimplemented, ");
		else if ((m_cached_unemulated | m_cached_imperfect) & (device_t::feature::GRAPHICS | device_t::feature::PALETTE))
			tempbuf[4] = _("Graphics: Imperfect, ");
		else
			tempbuf[4] = _("Graphics: OK, ");

		if (m_cached_flags & machine_flags::NO_SOUND_HW)
			tempbuf[4].append(_("Sound: None"));
		else if (m_cached_unemulated & device_t::feature::SOUND)
			tempbuf[4].append(_("Sound: Unimplemented"));
		else if (m_cached_imperfect & device_t::feature::SOUND)
			tempbuf[4].append(_("Sound: Imperfect"));
		else
			tempbuf[4].append(_("Sound: OK"));
	}
	else
	{
		const char *s = emulator_info::get_copyright();
		unsigned line = 0;

		// first line is version string
		tempbuf[line++] = string_format("%s %s", emulator_info::get_appname(), build_version);

		// output message
		while (line < std::size(tempbuf))
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

	// draw the bottom box
	draw_text_box(
			tempbuf, tempbuf + 4,
			origx1, origx2, origy2 + ui().box_tb_border(), origy2 + bottom,
			text_layout::text_justify::CENTER, text_layout::word_wrapping::TRUNCATE, true,
			ui().colors().text_color(), driver ? m_cached_color : ui().colors().background_color(), 1.0f);
}


//-------------------------------------------------
//  force_game_select - force the game
//  select menu to be visible and inescapable
//-------------------------------------------------

void simple_menu_select_game::force_game_select(mame_ui_manager &mui, render_container &container)
{
	char *gamename = (char *)mui.machine().options().system_name();

	// reset the menu stack

	// drop any existing menus and start the system selection menu
	menu::stack_reset(mui);
	menu::stack_push_special_main<simple_menu_select_game>(mui, container, gamename);
	mui.show_menu();

	// make sure MAME is paused
	mui.machine().pause();
}

} // namespace ui
