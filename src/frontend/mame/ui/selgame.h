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

class system_list;


class menu_select_game : public menu_select_launch
{
public:
	menu_select_game(mame_ui_manager &mui, render_container &container, const char *gamename);
	virtual ~menu_select_game();

	// force game select menu
	static void force_game_select(mame_ui_manager &mui, render_container &container);

protected:
	virtual void recompute_metrics(uint32_t width, uint32_t height, float aspect) override;

	void menu_activated() override;
	void menu_deactivated() override;

private:
	enum
	{
		CONF_OPTS = 1,
		CONF_MACHINE,
	};

	using icon_cache = texture_lru<game_driver const *>;

	system_list &m_persistent_data;
	icon_cache m_icons;
	std::string m_icon_paths;
	std::vector<std::reference_wrapper<ui_system_info const> > m_displaylist;

	std::vector<std::pair<double, std::reference_wrapper<ui_system_info const> > > m_searchlist;
	unsigned m_searched_fields;
	bool m_populated_favorites;

	static bool s_first_start;

	virtual void populate() override;
	virtual bool handle(event const *ev) override;

	// drawing
	virtual void draw_left_panel(u32 flags) override;
	virtual render_texture *get_icon_texture(int linenum, void *selectedref) override;

	// get selected software and/or driver
	virtual void get_selection(ui_software_info const *&software, ui_system_info const *&system) const override;
	virtual void show_config_menu(int index) override;
	virtual bool accept_search() const override { return !isfavorite(); }

	// text for main top/bottom panels
	virtual void make_topbox_text(std::string &line0, std::string &line1, std::string &line2) const override;
	virtual std::string make_software_description(ui_software_info const &software, ui_system_info const *system) const override;

	// filter navigation
	virtual void filter_selected(int index) override;

	// toolbar
	virtual void inkey_export() override;

	void build_available_list();

	bool isfavorite() const;
	void populate_search();
	bool load_available_machines();
	void load_custom_filters();

	// handlers
	bool inkey_select(const event *menu_event);
	bool inkey_select_favorite(const event *menu_event);
};

} // namespace ui

#endif  // MAME_FRONTEND_UI_SELGAME_H
