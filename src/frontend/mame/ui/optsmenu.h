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

class menu_simple_game_options : public menu
{
public:
	menu_simple_game_options(
			mame_ui_manager &mui,
			render_container &container,
			std::function<void ()> &&handler);
	virtual ~menu_simple_game_options() override;

protected:
	virtual void custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2) override;
	virtual void handle() override;
	virtual void populate(float &customtop, float &custombottom) override;

	void handle_item_event(event const &menu_event);

private:
	enum
	{
		DISPLAY_MENU = 1001,
		SOUND_MENU,
		CONTROLLER_MENU,
		MISC_MENU,
		ADVANCED_MENU,
		SAVE_OPTIONS,
		CGI_MENU,
		SAVE_CONFIG
	};

	std::function<void ()> const m_handler;
};


class menu_game_options : public menu_simple_game_options
{
public:
	menu_game_options(
			mame_ui_manager &mui,
			render_container &container,
			machine_filter_data &filter_data,
			std::function<void ()> &&handler);
	virtual ~menu_game_options() override;

protected:
	virtual void handle() override;
	virtual void populate(float &customtop, float &custombottom) override;

	void handle_item_event(event const &menu_event);

private:
	enum
	{
		FILTER_MENU = 2001,
		FILTER_ADJUST,
		CONF_DIR,
		CUSTOM_MENU
	};

	machine_filter_data &m_filter_data;
	machine_filter::type m_main_filter;
};

} // namespace ui

#endif // MAME_FRONTEND_UI_OPTSMENU_H
