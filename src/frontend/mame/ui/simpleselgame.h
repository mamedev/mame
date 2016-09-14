// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles, Nathan Woods
/***************************************************************************

    ui/selgame.h

    Game selector

***************************************************************************/

#pragma once

#ifndef MAME_FRONTEND_UI_SIMPLESELGAME_H
#define MAME_FRONTEND_UI_SIMPLESELGAME_H

#include "menu.h"

class driver_enumerator;

namespace ui {
class simple_menu_select_game : public menu {
public:
	simple_menu_select_game(mame_ui_manager &mui, render_container &container, const char *gamename);
	virtual ~simple_menu_select_game();

	// force game select menu
	static void force_game_select(mame_ui_manager &mui, render_container &container);

protected:
	virtual void custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2) override;
	virtual bool menu_has_search_active() override { return (m_search[0] != 0); }

private:
	enum { VISIBLE_GAMES_IN_LIST = 15 };

	virtual void populate() override;
	virtual void handle() override;

	// internal methods
	void build_driver_list();
	void inkey_select(const event *menu_event);
	void inkey_cancel();
	void inkey_special(const event *menu_event);

	// internal state
	UINT8                   m_error;
	bool                    m_rerandomize;
	char                    m_search[40];
	int                     m_matchlist[VISIBLE_GAMES_IN_LIST];
	std::vector<const game_driver *> m_driverlist;
	std::unique_ptr<driver_enumerator> m_drivlist;
};

} // namespace ui

#endif  /* MAME_FRONTEND_UI_SIMPLESELGAME_H */
