// license:BSD-3-Clause
// copyright-holders:Maurizio Petrarota, Vas Crabb
/***************************************************************************

    ui/selgame.h

    Main UI menu.

***************************************************************************/
#ifndef MAME_FRONTEND_UI_SELGAME_H
#define MAME_FRONTEND_UI_SELGAME_H

#pragma once

#include "ui/selmenu.h"
#include "ui/utils.h"

#include <functional>


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
	};

	using icon_cache = texture_lru<game_driver const *>;

	class persistent_data;

	persistent_data &m_persistent_data;
	icon_cache m_icons;
	std::string m_icon_paths;
	std::vector<std::reference_wrapper<ui_system_info const> > m_displaylist;

	std::vector<std::pair<double, std::reference_wrapper<ui_system_info const> > > m_searchlist;
	unsigned m_searched_fields;
	bool m_populated_favorites;

	static bool s_first_start;

	virtual void populate(float &customtop, float &custombottom) override;
	virtual void handle() override;

	// drawing
	virtual float draw_left_panel(float x1, float y1, float x2, float y2) override;
	virtual render_texture *get_icon_texture(int linenum, void *selectedref) override;

	// get selected software and/or driver
	virtual void get_selection(ui_software_info const *&software, ui_system_info const *&system) const override;
	virtual bool accept_search() const override { return !isfavorite(); }

	// text for main top/bottom panels
	virtual void make_topbox_text(std::string &line0, std::string &line1, std::string &line2) const override;
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
	bool load_available_machines();
	void load_custom_filters();

	// General info
	virtual void general_info(ui_system_info const &system, std::string &buffer) override;

	// handlers
	void inkey_select(const event *menu_event);
	void inkey_select_favorite(const event *menu_event);
};

} // namespace ui

#endif  // MAME_FRONTEND_UI_SELGAME_H
