// license:BSD-3-Clause
// copyright-holders:Maurizio Petrarota
/***************************************************************************

    ui/optsmenu.h

    UI main options menu manager.

***************************************************************************/

#pragma once

#ifndef __UI_OPTSMENU_H__
#define __UI_OPTSMENU_H__

class ui_menu_game_options : public ui_menu
{
public:
	ui_menu_game_options(running_machine &machine, render_container *container);
	virtual ~ui_menu_game_options();
	virtual void populate() override;
	virtual void handle() override;
	virtual void custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2) override;

private:
	UINT16 m_main;

	enum
	{
		FILTER_MENU = 1,
		FILE_CATEGORY_FILTER,
		MANUFACT_CAT_FILTER,
		YEAR_CAT_FILTER,
		CATEGORY_FILTER,
		CONF_DIR,
		DISPLAY_MENU,
		CUSTOM_MENU,
		SOUND_MENU,
		CONTROLLER_MENU,
		MISC_MENU,
		ADVANCED_MENU,
		SAVE_OPTIONS,
		CGI_MENU,
		CUSTOM_FILTER,
		SAVE_CONFIG
	};
};

// save options to file
void save_ui_options(running_machine &machine);
void save_main_option(running_machine &machine);

#endif /* __UI_OPTSMENU_H__ */
