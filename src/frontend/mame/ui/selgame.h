// license:BSD-3-Clause
// copyright-holders:Maurizio Petrarota
/***************************************************************************

    ui/selgame.h

    Main UI menu.

***************************************************************************/
#ifndef MAME_FRONTEND_UI_SELGAME_H
#define MAME_FRONTEND_UI_SELGAME_H

#pragma once

#include "ui/selmenu.h"
#include "ui/utils.h"


class media_auditor;

namespace ui {

class menu_select_game : public menu_select_launch
{
public:
	menu_select_game(mame_ui_manager &mui, render_container &container, const char *gamename);
	virtual ~menu_select_game();

	// force game select menu
	static void force_game_select(mame_ui_manager &mui, render_container &container);

private:
	enum
	{
		CONF_OPTS = 1,
		CONF_MACHINE,
		CONF_PLUGINS,
	};

	enum { VISIBLE_GAMES_IN_SEARCH = 200 };
	static bool first_start;
	static int m_isabios;

	static std::vector<const game_driver *> m_sortedlist;
	std::vector<ui_system_info> m_availsortedlist;
	std::vector<ui_system_info> m_displaylist;

	const game_driver *m_searchlist[VISIBLE_GAMES_IN_SEARCH + 1];

	virtual void populate(float &customtop, float &custombottom) override;
	virtual void handle() override;

	// draw left panel
	virtual float draw_left_panel(float x1, float y1, float x2, float y2) override;

	// get selected software and/or driver
	virtual void get_selection(ui_software_info const *&software, game_driver const *&driver) const override;
	virtual bool accept_search() const override { return !isfavorite(); }

	// text for main top/bottom panels
	virtual void make_topbox_text(std::string &line0, std::string &line1, std::string &line2) const override;
	virtual std::string make_driver_description(game_driver const &driver) const override;
	virtual std::string make_software_description(ui_software_info const &software) const override;

	// filter navigation
	virtual void filter_selected() override;

	// toolbar
	virtual void inkey_export() override;

	// internal methods
	void change_info_pane(int delta);

	void build_available_list();

	bool isfavorite() const;
	void populate_search();
	void init_sorted_list();
	bool load_available_machines();
	void load_custom_filters();

	static std::string make_error_text(bool summary, media_auditor const &auditor);

	// General info
	virtual void general_info(const game_driver *driver, std::string &buffer) override;

	// handlers
	void inkey_select(const event *menu_event);
	void inkey_select_favorite(const event *menu_event);
};

} // namespace ui

#endif  // MAME_FRONTEND_UI_SELGAME_H
