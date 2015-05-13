// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles, Nathan Woods
/***************************************************************************

    ui/miscmenu.h

    Internal MAME menus for the user interface.

***************************************************************************/

#pragma once

#ifndef __UI_MISCMENU_H__
#define __UI_MISCMENU_H__

#include "drivenum.h"
#include "crsshair.h"

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

class ui_menu_autofire : public ui_menu {
public:
	ui_menu_autofire(running_machine &machine, render_container *container);
	virtual ~ui_menu_autofire();
	virtual void populate();
	virtual void handle();

private:
	int refresh;
};

#endif  /* __UI_MISCMENU_H__ */
