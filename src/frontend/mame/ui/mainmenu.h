// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles, Nathan Woods
/***************************************************************************

    ui/mainmenu.h

    Internal MAME menus for the user interface.

***************************************************************************/

#pragma once

#ifndef MAME_FRONTEND_UI_MAINMENU_H
#define MAME_FRONTEND_UI_MAINMENU_H

namespace ui {
class menu_main : public menu
{
public:
	menu_main(mame_ui_manager &mui, render_container &container);
	virtual ~menu_main();

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
		PLUGINS,
		SELECT_GAME,
		BIOS_SELECTION,
		BARCODE_READ,
		PTY_INFO,
		EXTERNAL_DATS,
		ADD_FAVORITE,
		REMOVE_FAVORITE,
		QUIT_GAME
	};

	virtual void populate() override;
	virtual void handle() override;
};

} // namespace ui

#endif  /* MAME_FRONTEND_UI_MAINMENU_H */
