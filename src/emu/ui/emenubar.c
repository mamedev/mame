/***************************************************************************

    emenubar.h

    Internal MAME menu bar for the user interface.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#include "emu.h"
#include "ui/emenubar.h"

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
//  build_menus
//-------------------------------------------------

void ui_emu_menubar::build_menus()
{
	menu_item &file_menu = root_menu().append("File");
	file_menu.append("Pause");
	file_menu.append("Exit");

	menu_item &snapshot_menu = root_menu().append("Snapshot");
	snapshot_menu.append("Load snapshot...");
	snapshot_menu.append("Save snapshot...");

	menu_item &options_menu = root_menu().append("Options");
	menu_item &throttle_menu = options_menu.append("Throttle");
	throttle_menu.append("200%");
	throttle_menu.append("100%");
	throttle_menu.append("50%");
	throttle_menu.append("20%");
	throttle_menu.append("10%");
	throttle_menu.append("Unthrottled");

	menu_item &settings_menu = root_menu().append("Settings");
	settings_menu.append("Foo");

	menu_item &help_menu = root_menu().append("Help");
	help_menu.append("About...");
}
