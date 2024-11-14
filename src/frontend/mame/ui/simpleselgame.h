// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles, Nathan Woods
/***************************************************************************

    ui/selgame.h

    Game selector

***************************************************************************/

#ifndef MAME_FRONTEND_UI_SIMPLESELGAME_H
#define MAME_FRONTEND_UI_SIMPLESELGAME_H

#pragma once

#include "menu.h"


class driver_enumerator;


namespace ui {

class simple_menu_select_game : public menu
{
public:
	simple_menu_select_game(mame_ui_manager &mui, render_container &container, const char *gamename);
	virtual ~simple_menu_select_game();

	// force game select menu
	static void force_game_select(mame_ui_manager &mui, render_container &container);

protected:
	virtual void recompute_metrics(uint32_t width, uint32_t height, float aspect) override;
	virtual void custom_render(uint32_t flags, void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2) override;
	virtual bool custom_ui_back() override { return !m_search.empty(); }
	virtual std::tuple<int, bool, bool> custom_pointer_updated(bool changed, ui_event const &uievt) override;

private:
	enum { VISIBLE_GAMES_IN_LIST = 15 };

	virtual void populate() override;
	virtual bool handle(event const *ev) override;

	// internal methods
	void build_driver_list();
	bool inkey_select(const event &menu_event);
	void inkey_cancel();
	void inkey_special(const event &menu_event);

	// internal state
	bool                    m_nomatch;
	bool                    m_error;
	bool                    m_rerandomize;
	std::string             m_search;
	int                     m_matchlist[VISIBLE_GAMES_IN_LIST];
	std::vector<const game_driver *>    m_driverlist;
	std::unique_ptr<driver_enumerator>  m_drivlist;

	// cached driver flags
	const game_driver *     m_cached_driver;
	machine_flags::type     m_cached_flags;
	device_t::feature_type  m_cached_unemulated;
	device_t::feature_type  m_cached_imperfect;
	rgb_t                   m_cached_color;
};

} // namespace ui

#endif // MAME_FRONTEND_UI_SIMPLESELGAME_H
