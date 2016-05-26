// license:BSD-3-Clause
// copyright-holders:Maurizio Petrarota
/***************************************************************************

    ui/optsmenu.h

    UI main options menu manager.

***************************************************************************/

#pragma once

#ifndef MAME_FRONTEND_UI_OPTSMENU_H
#define MAME_FRONTEND_UI_OPTSMENU_H

#include "ui/menu.h"

namespace ui {

class menu_game_options : public menu
{
public:
	menu_game_options(mame_ui_manager &mui, render_container *container);
	virtual ~menu_game_options() override;
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

} // namespace ui

#endif /* MAME_FRONTEND_UI_OPTSMENU_H */
