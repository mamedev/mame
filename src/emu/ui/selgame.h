// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles, Nathan Woods
/***************************************************************************

    ui/selgame.h

    Game selector

***************************************************************************/

#pragma once

#ifndef __UI_SELGAME_H__
#define __UI_SELGAME_H__

#include "drivenum.h"
#include "menu.h"

class ui_menu_select_game : public ui_menu {
public:
	ui_menu_select_game(running_machine &machine, render_container *container, const char *gamename);
	virtual ~ui_menu_select_game();
	virtual void populate();
	virtual void handle();
	virtual void custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2);

	// force game select menu
	static void force_game_select(running_machine &machine, render_container *container);

private:
	// internal state
	enum { VISIBLE_GAMES_IN_LIST = 15 };
	UINT8                   m_error;
	UINT8                   m_rerandomize;
	char                    m_search[40];
	int                     m_matchlist[VISIBLE_GAMES_IN_LIST];
	std::vector<const game_driver *> m_driverlist;
	auto_pointer<driver_enumerator> m_drivlist;

	// internal methods
	void build_driver_list();
	void inkey_select(const ui_menu_event *menu_event);
	void inkey_cancel(const ui_menu_event *menu_event);
	void inkey_special(const ui_menu_event *menu_event);
};

#endif  /* __UI_SELGAME_H__ */
