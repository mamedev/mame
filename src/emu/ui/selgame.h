/***************************************************************************

    ui/selgame.h

    Game selector

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __UI_SELGAME_H__
#define __UI_SELGAME_H__

#include "emu.h"
#include "emuopts.h"
#include "ui.h"
#include "ui/menu.h"
#include "drivenum.h"


//**************************************************************************
//  GAME SELECTOR
//**************************************************************************

class ui_menu_select_game : public ui_menu
{
public:
	ui_menu_select_game(running_machine &machine, render_container *container, const char *gamename);
	virtual ~ui_menu_select_game();
	virtual void populate();
	virtual void handle();
	virtual void custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2);

	// force game select menu
	static void force_game_select(running_machine &machine, render_container *container);

private:
	// private driver list
	enum { VISIBLE_GAMES_IN_LIST = 15 };
	const game_driver   **m_driver_list;
	int					m_driver_count;

	// the selection
	int					m_selection_up;
	int					m_selection_down;

	// other
	bool				m_error;
	char                m_search[40];

	// internal methods
	void build_driver_list();
	static int driver_list_compare(const void *p1, const void *p2);
	void select_searched_item();

	void inkey_select(const ui_menu_event *menu_event);
	void inkey_cancel(const ui_menu_event *menu_event);
	void inkey_special(const ui_menu_event *menu_event);
	void inkey_configure(const ui_menu_event *menu_event);
};

#endif  /* __UI_SELGAME_H__ */
