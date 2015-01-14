/***************************************************************************

    ui/miscmenu.h

    Internal MAME menus for the user interface.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __UI_MISCMENU_H__
#define __UI_MISCMENU_H__

#include "crsshair.h"
#include "drivenum.h"

class ui_menu_keyboard_mode : public ui_menu {
public:
	ui_menu_keyboard_mode(running_machine &machine, render_container *container);
	virtual ~ui_menu_keyboard_mode();
	virtual void populate();
	virtual void handle();
};

class ui_menu_network_devices : public ui_menu {
public:
	ui_menu_network_devices(running_machine &machine, render_container *container);
	virtual ~ui_menu_network_devices();
	virtual void populate();
	virtual void handle();
};

class ui_menu_bookkeeping : public ui_menu {
public:
	ui_menu_bookkeeping(running_machine &machine, render_container *container);
	virtual ~ui_menu_bookkeeping();
	virtual void populate();
	virtual void handle();

private:
	attotime prevtime;
};

class ui_menu_game_info : public ui_menu {
public:
	ui_menu_game_info(running_machine &machine, render_container *container);
	virtual ~ui_menu_game_info();
	virtual void populate();
	virtual void handle();
};

class ui_menu_cheat : public ui_menu {
public:
	ui_menu_cheat(running_machine &machine, render_container *container);
	virtual ~ui_menu_cheat();
	virtual void populate();
	virtual void handle();
};

class ui_menu_sliders : public ui_menu {
public:
	ui_menu_sliders(running_machine &machine, render_container *container, bool menuless_mode = false);
	virtual ~ui_menu_sliders();
	virtual void populate();
	virtual void handle();

	virtual void custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2);

	static UINT32 ui_handler(running_machine &machine, render_container *container, UINT32 state);

private:
	bool menuless_mode, hidden;
};

class ui_menu_video_targets : public ui_menu {
public:
	ui_menu_video_targets(running_machine &machine, render_container *container);
	virtual ~ui_menu_video_targets();
	virtual void populate();
	virtual void handle();
};

class ui_menu_video_options : public ui_menu {
public:
	ui_menu_video_options(running_machine &machine, render_container *container, render_target *target);
	virtual ~ui_menu_video_options();
	virtual void populate();
	virtual void handle();

private:
	enum {
		VIDEO_ITEM_ROTATE = 0x80000000,
		VIDEO_ITEM_BACKDROPS,
		VIDEO_ITEM_OVERLAYS,
		VIDEO_ITEM_BEZELS,
		VIDEO_ITEM_CPANELS,
		VIDEO_ITEM_MARQUEES,
		VIDEO_ITEM_ZOOM,
		VIDEO_ITEM_VIEW
	};

	render_target *target;
};

class ui_menu_crosshair : public ui_menu {
public:
	ui_menu_crosshair(running_machine &machine, render_container *container);
	virtual ~ui_menu_crosshair();
	virtual void populate();
	virtual void handle();

private:
	enum {
		CROSSHAIR_ITEM_VIS = 0,
		CROSSHAIR_ITEM_PIC,
		CROSSHAIR_ITEM_AUTO_TIME
	};

	/* internal crosshair menu item data */
	struct crosshair_item_data {
		UINT8               type;
		UINT8               player;
		UINT8               min, max;
		UINT8               cur;
		UINT8               defvalue;
		char                last_name[CROSSHAIR_PIC_NAME_LENGTH + 1];
		char                next_name[CROSSHAIR_PIC_NAME_LENGTH + 1];
	};
};

class ui_menu_quit_game : public ui_menu {
public:
	ui_menu_quit_game(running_machine &machine, render_container *container);
	virtual ~ui_menu_quit_game();
	virtual void populate();
	virtual void handle();
};

class ui_menu_bios_selection : public ui_menu {
public:
	ui_menu_bios_selection(running_machine &machine, render_container *container);
	virtual ~ui_menu_bios_selection();
	virtual void populate();
	virtual void handle();

private:
};

#endif  /* __UI_MISCMENU_H__ */
