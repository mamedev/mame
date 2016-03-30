// license:BSD-3-Clause
// copyright-holders:Maurizio Petrarota
/***************************************************************************

    ui/selgame.h

    Main UI menu.

***************************************************************************/

#pragma once

#ifndef __UI_MAIN_H__
#define __UI_MAIN_H__

#include "drivenum.h"
#include "ui/menu.h"

class ui_menu_select_game : public ui_menu
{
public:
	ui_menu_select_game(running_machine &machine, render_container *container, const char *gamename);
	virtual ~ui_menu_select_game();
	virtual void populate() override;
	virtual void handle() override;
	virtual void custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2) override;

	// force game select menu
	static void force_game_select(running_machine &machine, render_container *container);

	virtual bool menu_has_search_active() override { return (m_search[0] != 0); }

	// draw left panel
	virtual float draw_left_panel(float x1, float y1, float x2, float y2) override;

	// draw right panel
	virtual void draw_right_panel(void *selectedref, float origx1, float origy1, float origx2, float origy2) override;

private:
	enum
	{
		CONF_OPTS = 1,
//      CONF_MACHINE,
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
	std::vector<const game_driver *> m_tmp;

	const game_driver *m_searchlist[VISIBLE_GAMES_IN_SEARCH + 1];

	// internal methods
	void build_custom();
	void build_category();
	void build_available_list();
	void build_list(std::vector<const game_driver *> &vec, const char *filter_text = nullptr, int filter = 0, bool bioscheck = false);

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
	void inkey_select(const ui_menu_event *menu_event);
	void inkey_select_favorite(const ui_menu_event *menu_event);
	void inkey_special(const ui_menu_event *menu_event);
	void inkey_export();
	void inkey_configure(const ui_menu_event *menu_event);
};


#endif  /* __UI_MAIN_H__ */
