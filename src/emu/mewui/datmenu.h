/***************************************************************************

    mewui/datmenu.h

    Internal MEWUI user interface.


***************************************************************************/

#pragma once

#ifndef __MEWUI_DATMENU_H__
#define __MEWUI_DATMENU_H__

#include "mewui/utils.h"
//-------------------------------------------------
//  class dats menu
//-------------------------------------------------

class ui_menu_dats : public ui_menu
{
public:
	ui_menu_dats(running_machine &machine, render_container *container, int _flags, const game_driver *driver = NULL);
	virtual ~ui_menu_dats();
	virtual void populate();
	virtual void handle();
	virtual void custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2);

private:
	const game_driver *ui_driver;
	int               flags;
	bool get_data(const game_driver *driver, int flags);
};

//-------------------------------------------------
//  class command data menu
//-------------------------------------------------

class ui_menu_command : public ui_menu
{
public:
	ui_menu_command(running_machine &machine, render_container *container, const game_driver *driver = NULL);
	virtual ~ui_menu_command();
	virtual void populate();
	virtual void handle();
	virtual void custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2);

private:
	const game_driver   *ui_driver;
};

//-------------------------------------------------
//  class command content data menu
//-------------------------------------------------

class ui_menu_command_content : public ui_menu
{
public:
	ui_menu_command_content(running_machine &machine, render_container *container, FPTR p_param, std::string title, const game_driver *driver = NULL);
	virtual ~ui_menu_command_content();
	virtual void populate();
	virtual void handle();
	virtual void custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2);

private:
	const game_driver   *ui_driver;
	FPTR                param;
	std::string         title;
};

//-------------------------------------------------
//  class software history menu
//-------------------------------------------------

class ui_menu_history_sw : public ui_menu
{
public:
	ui_menu_history_sw(running_machine &machine, render_container *container, ui_software_info *swinfo, const game_driver *driver = NULL);
	ui_menu_history_sw(running_machine &machine, render_container *container, const game_driver *driver = NULL);
	virtual ~ui_menu_history_sw();
	virtual void populate();
	virtual void handle();
	virtual void custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2);

private:
	const char *listname, *shortname, *longname;
	const game_driver *ui_driver;
};

#endif  /* __MEWUI_DATMENU_H__ */
