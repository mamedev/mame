// license:BSD-3-Clause
// copyright-holders:Maurizio Petrarota, Vas Crabb
/***************************************************************************

    ui/selsoft.h

    UI software menu.

***************************************************************************/
#ifndef MAME_FRONTEND_UI_SELSOFT_H
#define MAME_FRONTEND_UI_SELSOFT_H

#pragma once

#include "ui/selmenu.h"
#include "ui/utils.h"

#include <map>
#include <string>
#include <vector>


namespace ui {

// Menu Class
class menu_select_software : public menu_select_launch
{
public:
	menu_select_software(mame_ui_manager &mui, render_container &container, game_driver const &driver);
	virtual ~menu_select_software() override;

private:
	using filter_map = std::map<software_filter::type, software_filter::ptr>;
	using icon_cache = texture_lru<ui_software_info const *>;

	struct search_item
	{
		search_item(ui_software_info const &s);
		search_item(search_item const &) = default;
		search_item(search_item &&) = default;
		search_item &operator=(search_item const &) = default;
		search_item &operator=(search_item &&) = default;
		void set_penalty(std::u32string const &search);

		std::reference_wrapper<ui_software_info const> software;
		std::u32string ucs_shortname;
		std::u32string ucs_longname;
		double penalty;
	};

	virtual void populate(float &customtop, float &custombottom) override;
	virtual void handle() override;

	// drawing
	virtual float draw_left_panel(float x1, float y1, float x2, float y2) override;
	virtual render_texture *get_icon_texture(int linenum, void *selectedref) override;

	// get selected software and/or driver
	virtual void get_selection(ui_software_info const *&software, game_driver const *&driver) const override;

	// text for main top/bottom panels
	virtual void make_topbox_text(std::string &line0, std::string &line1, std::string &line2) const override;
	virtual std::string make_driver_description(game_driver const &driver) const override;
	virtual std::string make_software_description(ui_software_info const &software) const override;

	// filter navigation
	virtual void filter_selected() override;

	// toolbar
	virtual void inkey_export() override { throw false; }

	void build_software_list();
	void find_matches();
	void load_sw_custom_filters();

	// handlers
	void inkey_select(const event *menu_event);

	virtual void general_info(const game_driver *driver, std::string &buffer) override { }

	std::map<std::string, std::string>  m_icon_paths;
	icon_cache                          m_icons;
	game_driver const                   &m_driver;
	bool                                m_has_empty_start;
	software_filter_data                m_filter_data;
	filter_map                          m_filters;
	software_filter::type               m_filter_type;

	std::vector<ui_software_info>       m_swinfo;
	std::vector<search_item>            m_searchlist;
	std::vector<std::reference_wrapper<ui_software_info const> > m_displaylist;
};

} // namespace ui

#endif // MAME_FRONTEND_UI_SELSOFT_H
