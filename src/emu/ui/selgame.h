/***************************************************************************

    selgame.h

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

private:
	// private driver list
	enum { VISIBLE_GAMES_IN_LIST = 15 };
	const game_driver   **driver_list;
	int					driver_count;

	// the selection
	int					selection_up;
	int					selection_down;

	// other
	bool				error;
	char                search[40];

	void build_driver_list();
	static int driver_list_compare(const void *p1, const void *p2);
	void select_searched_item();

	void inkey_select(const ui_menu_event *menu_event);
	void inkey_cancel(const ui_menu_event *menu_event);
	void inkey_special(const ui_menu_event *menu_event);
	void inkey_toggle_ui(const ui_menu_event *menu_event);
};


#endif  /* __UI_SELGAME_H__ */
