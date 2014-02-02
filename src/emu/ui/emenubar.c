/***************************************************************************

    emenubar.h

    Internal MAME menu bar for the user interface.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#include "emu.h"
#include "ui/emenubar.h"
#include "delegate.h"
#include "emucore.h"

//**************************************************************************
//  MENUBAR IMPLEMENTATION
//**************************************************************************

//-------------------------------------------------
//  ctor
//-------------------------------------------------

ui_emu_menubar::ui_emu_menubar(running_machine &machine, render_container *container)
	: ui_menubar(machine, container)
{
}


//-------------------------------------------------
//  menubar_build_menus
//-------------------------------------------------

void ui_emu_menubar::menubar_build_menus()
{
	build_file_menu();
	build_snapshot_menu();
	build_options_menu();
	build_settings_menu();
	build_help_menu();
}


//-------------------------------------------------
//  build_file_menu
//-------------------------------------------------

void ui_emu_menubar::build_file_menu()
{
	// File
	menu_item &file_menu = root_menu().append("File");

	// Pause
	menu_item &pause_menu = file_menu.append("Pause", &running_machine::toggle_pause, machine());
	pause_menu.set_checked(machine().paused());

	// Reset
	menu_item &reset_menu = file_menu.append("Reset");
	reset_menu.append("Hard", &running_machine::schedule_hard_reset, machine());
	reset_menu.append("Soft", &running_machine::schedule_soft_reset, machine());

	// Exit
	file_menu.append("Exit", &running_machine::schedule_exit, machine());
}


//-------------------------------------------------
//  build_snapshot_menu
//-------------------------------------------------

void ui_emu_menubar::build_snapshot_menu()
{
	menu_item &snapshot_menu = root_menu().append("Snapshot");
	snapshot_menu.append("Load snapshot...");
	snapshot_menu.append("Save snapshot...");
}


//-------------------------------------------------
//  build_options_menu
//-------------------------------------------------

void ui_emu_menubar::build_options_menu()
{
	menu_item &options_menu = root_menu().append("Options");
	menu_item &throttle_menu = options_menu.append("Throttle");
	throttle_menu.append("200%", &ui_emu_menubar::throttle, *this, 2.0f);
	throttle_menu.append("100%", &ui_emu_menubar::throttle, *this, 1.0f);
	throttle_menu.append("50%", &ui_emu_menubar::throttle, *this, 0.5f);
	throttle_menu.append("20%", &ui_emu_menubar::throttle, *this, 0.2f);
	throttle_menu.append("10%", &ui_emu_menubar::throttle, *this, 0.1f);
	throttle_menu.append("Unthrottled", &ui_emu_menubar::throttle, *this, 0.0f);
}


//-------------------------------------------------
//  build_settings_menu
//-------------------------------------------------

void ui_emu_menubar::build_settings_menu()
{
	menu_item &settings_menu = root_menu().append("Settings");
	settings_menu.append("Foo");
}


//-------------------------------------------------
//  build_help_menu
//-------------------------------------------------

void ui_emu_menubar::build_help_menu()
{
	menu_item &help_menu = root_menu().append("Help");
	help_menu.append("About...");
}


//-------------------------------------------------
//  throttle
//-------------------------------------------------

void ui_emu_menubar::throttle(float rate)
{
}

