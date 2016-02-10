// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles, Nathan Woods
/***************************************************************************

    ui/mainmenu.h

    Internal MAME menus for the user interface.

***************************************************************************/

#pragma once

#ifndef __UI_MAINMENU_H__
#define __UI_MAINMENU_H__

#include "drivenum.h"

class ui_menu_main : public ui_menu {
public:
	ui_menu_main(running_machine &machine, render_container *container);
	virtual ~ui_menu_main();
	virtual void populate() override;
	virtual void handle() override;

private:
	enum {
		INPUT_GROUPS,
		INPUT_SPECIFIC,
		SETTINGS_DIP_SWITCHES,
		SETTINGS_DRIVER_CONFIG,
		ANALOG,
		BOOKKEEPING,
		GAME_INFO,
		IMAGE_MENU_IMAGE_INFO,
		IMAGE_MENU_FILE_MANAGER,
		TAPE_CONTROL,
		SLOT_DEVICES,
		NETWORK_DEVICES,
		KEYBOARD_MODE,
		SLIDERS,
		VIDEO_TARGETS,
		VIDEO_OPTIONS,
		CROSSHAIR,
		CHEAT,
		SELECT_GAME,
		BIOS_SELECTION,
		BARCODE_READ,
		PTY_INFO,
		HISTORY,
		MAMEINFO,
		SYSINFO,
		ADD_FAVORITE,
		REMOVE_FAVORITE,
		COMMAND,
		STORYINFO,
		SW_HISTORY,
		QUIT_GAME
	};
};

#endif  /* __UI_MAINMENU_H__ */
