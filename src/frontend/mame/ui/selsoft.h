// license:BSD-3-Clause
// copyright-holders:Maurizio Petrarota
/***************************************************************************

    ui/selsoft.h

    UI software menu.

***************************************************************************/
#pragma once

#ifndef MAME_FRONTEND_UI_SELSOFT_H
#define MAME_FRONTEND_UI_SELSOFT_H

#include "ui/custmenu.h"

namespace ui {

using s_bios = std::vector<std::pair<std::string, int>>;
using s_parts = std::unordered_map<std::string, std::string>;

// Menu Class
class menu_select_software : public menu
{
public:
	menu_select_software(mame_ui_manager &mui, render_container *container, const game_driver *driver);
	virtual ~menu_select_software() override;
	virtual void populate() override;
	virtual void handle() override;
	virtual void custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2) override;

	virtual bool menu_has_search_active() override { return (m_search[0] != 0); }

	// draw left panel
	virtual float draw_left_panel(float x1, float y1, float x2, float y2) override;

	// draw right panel
	virtual void draw_right_panel(void *selectedref, float origx1, float origy1, float origx2, float origy2) override;

private:
	enum { VISIBLE_GAMES_IN_SEARCH = 200 };
	char                m_search[40];
	const game_driver   *m_driver;
	bool                m_has_empty_start;
	s_filter            m_filter;
	int                 highlight;

	ui_software_info                  *m_searchlist[VISIBLE_GAMES_IN_SEARCH + 1];
	std::vector<ui_software_info *>   m_displaylist, m_tmp, m_sortedlist;
	std::vector<ui_software_info>     m_swinfo;

	void build_software_list();
	void build_list(std::vector<ui_software_info *> &vec, const char *filter_text = nullptr, int filter = -1);
	void build_custom();
	void find_matches(const char *str, int count);
	void load_sw_custom_filters();

	void arts_render(void *selectedref, float x1, float y1, float x2, float y2);
	void infos_render(void *selectedref, float x1, float y1, float x2, float y2);

	// handlers
	void inkey_select(const event *menu_event);
	void inkey_special(const event *menu_event);
	void inkey_navigation();
};

class software_parts : public menu
{
public:
	software_parts(mame_ui_manager &mui, render_container *container, s_parts parts, ui_software_info *ui_info);
	virtual ~software_parts() override;
	virtual void populate() override;
	virtual void handle() override;
	virtual void custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2) override;

private:
	ui_software_info *m_uiinfo;
	s_parts m_parts;
};

class bios_selection : public menu
{
public:
	bios_selection(mame_ui_manager &mui, render_container *container, s_bios biosname, void *driver, bool software, bool inlist);
	virtual ~bios_selection() override;
	virtual void populate() override;
	virtual void handle() override;
	virtual void custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2) override;

private:

	void    *m_driver;
	bool    m_software, m_inlist;
	s_bios  m_bios;
};

struct reselect_last
{
	static std::string driver, software, swlist;
	static void set(bool value) { m_reselect = value; }
	static bool get() { return m_reselect; }
	static void reset() { driver.clear(); software.clear(); swlist.clear(); set(false); }

private:
	static bool m_reselect;
};

// Getter
bool has_multiple_bios(const game_driver *driver, s_bios &biosname);

} // namespace ui

#endif /* MAME_FRONTEND_UI_SELSOFT_H */
