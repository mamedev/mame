// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles, Nathan Woods, Maurizio Petrarota
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
	virtual void populate() override;
	virtual void handle() override;
};

class ui_menu_network_devices : public ui_menu {
public:
	ui_menu_network_devices(running_machine &machine, render_container *container);
	virtual ~ui_menu_network_devices();
	virtual void populate() override;
	virtual void handle() override;
};

class ui_menu_bookkeeping : public ui_menu {
public:
	ui_menu_bookkeeping(running_machine &machine, render_container *container);
	virtual ~ui_menu_bookkeeping();
	virtual void populate() override;
	virtual void handle() override;

private:
	attotime prevtime;
};

class ui_menu_crosshair : public ui_menu {
public:
	ui_menu_crosshair(running_machine &machine, render_container *container);
	virtual ~ui_menu_crosshair();
	virtual void populate() override;
	virtual void handle() override;

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
	virtual void populate() override;
	virtual void handle() override;
};

class ui_menu_bios_selection : public ui_menu {
public:
	ui_menu_bios_selection(running_machine &machine, render_container *container);
	virtual ~ui_menu_bios_selection();
	virtual void populate() override;
	virtual void handle() override;
};

//-------------------------------------------------
//  class miscellaneous options menu
//-------------------------------------------------
class ui_menu_misc_options : public ui_menu
{
public:
	ui_menu_misc_options(running_machine &machine, render_container *container);
	virtual ~ui_menu_misc_options();
	virtual void populate() override;
	virtual void handle() override;
	virtual void custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2) override;

private:
	struct misc_option
	{
		bool        status;
		const char  *description;
		const char  *option;
	};

	static misc_option m_options[];
};

//-------------------------------------------------
//  export menu
//-------------------------------------------------

class ui_menu_export : public ui_menu
{
public:
	ui_menu_export(running_machine &machine, render_container *container, std::vector<const game_driver*> list);
	virtual ~ui_menu_export();
	virtual void populate() override;
	virtual void handle() override;

private:
	std::vector<const game_driver*> m_list;
};

//-------------------------------------------------
//  machine configure menu
//-------------------------------------------------

class ui_menu_machine_configure : public ui_menu
{
public:
	ui_menu_machine_configure(running_machine &machine, render_container *container, const game_driver *prev);
	virtual ~ui_menu_machine_configure();
	virtual void populate() override;
	virtual void handle() override;
	virtual void custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2) override;

private:
	const game_driver *m_drv;
};

//-------------------------------------------------
//  plugins configure menu
//-------------------------------------------------

class ui_menu_plugins_configure : public ui_menu
{
public:
	ui_menu_plugins_configure(running_machine &machine, render_container *container);
	virtual ~ui_menu_plugins_configure();
	virtual void populate() override;
	virtual void handle() override;
	virtual void custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2) override;
};

#endif  /* __UI_MISCMENU_H__ */
