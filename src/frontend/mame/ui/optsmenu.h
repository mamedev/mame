// license:BSD-3-Clause
// copyright-holders:Maurizio Petrarota
/***************************************************************************

    ui/optsmenu.h

    UI main options menu manager.

***************************************************************************/
#ifndef MAME_FRONTEND_UI_OPTSMENU_H
#define MAME_FRONTEND_UI_OPTSMENU_H

#pragma once

#include "ui/menu.h"
#include "ui/utils.h"


namespace ui {
class menu_game_options : public menu
{
public:
	menu_game_options(mame_ui_manager &mui, render_container &container);
	virtual ~menu_game_options() override;

protected:
	virtual void custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2) override;

private:
	enum
	{
		FILTER_MENU = 1,
		FILTER_ADJUST,
		CONF_DIR,
		DISPLAY_MENU,
		CUSTOM_MENU,
		SOUND_MENU,
		CONTROLLER_MENU,
		MISC_MENU,
		ADVANCED_MENU,
		SAVE_OPTIONS,
		CGI_MENU,
		SAVE_CONFIG
	};

	virtual void populate(float &customtop, float &custombottom) override;
	virtual void handle() override;

	machine_filter::type m_main;
};

} // namespace ui

#endif /* MAME_FRONTEND_UI_OPTSMENU_H */
