// license:BSD-3-Clause
// copyright-holders:Dankan1890
/***************************************************************************

    mewui/optsmenu.h

    MEWUI main options menu manager.

***************************************************************************/

#pragma once

#ifndef __MEWUI_OPTSMENU_H__
#define __MEWUI_OPTSMENU_H__

#include "mewui/utils.h"

class ui_menu_game_options : public ui_menu
{
public:
	ui_menu_game_options(running_machine &machine, render_container *container);
	virtual ~ui_menu_game_options();
	virtual void populate();
	virtual void handle();
	virtual void custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2);

private:
	enum
	{
		FILTER_MENU = 1,
		FILE_CATEGORY_FILTER,
		MANUFACT_CAT_FILTER,
		YEAR_CAT_FILTER,
		CATEGORY_FILTER,
		MISC_MENU,
		DISPLAY_MENU,
		CUSTOM_MENU,
		SOUND_MENU,
		CONTROLLER_MENU,
		SAVE_OPTIONS,
		CGI_MENU,
		CUSTOM_FILTER,
		UME_SYSTEM
	};
};

// save options to file
void save_game_options(running_machine &machine);

#endif /* __MEWUI_OPTSMENU_H__ */
