// license:BSD-3-Clause
// copyright-holders:Maurizio Petrarota
/***************************************************************************

    ui/selsoft.h

    UI software menu.

***************************************************************************/
#ifndef MAME_FRONTEND_UI_SELSOFT_H
#define MAME_FRONTEND_UI_SELSOFT_H

#pragma once

#include "ui/custmenu.h"
#include "ui/selmenu.h"

namespace ui {

// Menu Class
class menu_select_software : public menu_select_launch
{
public:
	menu_select_software(mame_ui_manager &mui, render_container &container, const game_driver *driver);
	virtual ~menu_select_software() override;

protected:
	virtual bool menu_has_search_active() override { return !m_search.empty(); }

private:
	enum { VISIBLE_GAMES_IN_SEARCH = 200 };

	std::string             m_search;
	const game_driver       *m_driver;
	bool                    m_has_empty_start;
	s_filter                m_filter;
	software_filter::type   m_filter_type;
	int                     highlight;

	virtual void populate(float &customtop, float &custombottom) override;
	virtual void handle() override;

	// draw left panel
	virtual float draw_left_panel(float x1, float y1, float x2, float y2) override;

	// get selected software and/or driver
	virtual void get_selection(ui_software_info const *&software, game_driver const *&driver) const override;

	// text for main top/bottom panels
	virtual void make_topbox_text(std::string &line0, std::string &line1, std::string &line2) const override;
	virtual std::string make_driver_description(game_driver const &driver) const override;
	virtual std::string make_software_description(ui_software_info const &software) const override;

	ui_software_info                  *m_searchlist[VISIBLE_GAMES_IN_SEARCH + 1];
	std::vector<ui_software_info *>   m_displaylist, m_tmp, m_sortedlist;
	std::vector<ui_software_info>     m_swinfo;

	void build_software_list();
	void build_list(std::vector<ui_software_info *> &vec, const char *filter_text = nullptr, int filter = -1);
	void build_custom();
	void find_matches(const char *str, int count);
	void load_sw_custom_filters();

	// handlers
	void inkey_select(const event *menu_event);
	void inkey_special(const event *menu_event);

	virtual void general_info(const game_driver *driver, std::string &buffer) override {}
};

} // namespace ui

#endif /* MAME_FRONTEND_UI_SELSOFT_H */
