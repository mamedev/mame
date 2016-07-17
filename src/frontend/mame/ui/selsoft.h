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
#include "ui/selmenu.h"

namespace ui {
using s_bios = std::vector<std::pair<std::string, int>>;
using s_parts = std::unordered_map<std::string, std::string>;

// Menu Class
class menu_select_software : public menu_select_launch
{
public:
	menu_select_software(mame_ui_manager &mui, render_container &container, const game_driver *driver);
	virtual ~menu_select_software() override;

protected:
	virtual bool menu_has_search_active() override { return (m_search[0] != 0); }

private:
	enum { VISIBLE_GAMES_IN_SEARCH = 200 };
	char                m_search[40];
	const game_driver   *m_driver;
	bool                m_has_empty_start;
	s_filter            m_filter;
	int                 highlight;

	virtual void populate() override;
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

	virtual void infos_render(float x1, float y1, float x2, float y2) override;

	// handlers
	void inkey_select(const event *menu_event);
	void inkey_special(const event *menu_event);
};

class software_parts : public menu
{
public:
	software_parts(mame_ui_manager &mui, render_container &container, s_parts parts, ui_software_info *ui_info);
	virtual ~software_parts() override;

protected:
	virtual void custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2) override;

private:
	virtual void populate() override;
	virtual void handle() override;

	ui_software_info *m_uiinfo;
	s_parts m_parts;
};

class bios_selection : public menu
{
public:
	bios_selection(mame_ui_manager &mui, render_container &container, s_bios biosname, void *driver, bool software, bool inlist);
	virtual ~bios_selection() override;

protected:
	virtual void custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2) override;

private:
	virtual void populate() override;
	virtual void handle() override;

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
