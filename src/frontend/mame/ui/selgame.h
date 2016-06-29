// license:BSD-3-Clause
// copyright-holders:Maurizio Petrarota
/***************************************************************************

    ui/selgame.h

    Main UI menu.

***************************************************************************/

#pragma once

#ifndef MAME_FRONTEND_UI_SELGAME_H
#define MAME_FRONTEND_UI_SELGAME_H

#include "ui/menu.h"


namespace ui {
class menu_select_game : public menu
{
public:
	menu_select_game(mame_ui_manager &mui, render_container *container, const char *gamename);
	virtual ~menu_select_game();
	virtual void populate() override;
	virtual void handle() override;
	virtual void custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2) override;

	// force game select menu
	static void force_game_select(mame_ui_manager &mui, render_container *container);

	virtual bool menu_has_search_active() override { return (m_search[0] != 0); }

	// draw left panel
	virtual float draw_left_panel(float x1, float y1, float x2, float y2) override;

	// draw right panel
	virtual void draw_right_panel(void *selectedref, float origx1, float origy1, float origx2, float origy2) override;

private:
	enum
	{
		CONF_OPTS = 1,
		CONF_MACHINE,
		CONF_PLUGINS,
	};

	enum { VISIBLE_GAMES_IN_SEARCH = 200 };
	char m_search[40];
	static int m_isabios;
	int highlight;

	static std::vector<const game_driver *> m_sortedlist;
	std::vector<const game_driver *> m_availsortedlist;
	std::vector<const game_driver *> m_unavailsortedlist;
	std::vector<const game_driver *> m_displaylist;

	const game_driver *m_searchlist[VISIBLE_GAMES_IN_SEARCH + 1];

	// internal methods
	void build_custom();
	void build_category();
	void build_available_list();
	void build_list(const char *filter_text = nullptr, int filter = 0, bool bioscheck = false, std::vector<const game_driver *> vec = {});

	bool isfavorite();
	void populate_search();
	void init_sorted_list();
	bool load_available_machines();
	void load_custom_filters();

	// General info
	void general_info(const game_driver *driver, std::string &buffer);

	void arts_render(void *selectedref, float x1, float y1, float x2, float y2);
	void infos_render(void *selectedref, float x1, float y1, float x2, float y2);

	// handlers
	void inkey_select(const event *menu_event);
	void inkey_select_favorite(const event *menu_event);
	void inkey_special(const event *menu_event);
	void inkey_export();
	void inkey_navigation();
};

} // namespace ui

#endif  // MAME_FRONTEND_UI_SELGAME_H
