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

#include "lrucache.h"

#include <map>
#include <memory>
#include <string>
#include <vector>


namespace ui {

// Menu Class
class menu_select_software : public menu_select_launch
{
public:
	menu_select_software(mame_ui_manager &mui, render_container &container, ui_system_info const &system);
	virtual ~menu_select_software() override;

private:
	using filter_map = std::map<software_filter::type, software_filter::ptr>;
	using icon_cache = texture_lru<ui_software_info const *>;

	struct search_item;
	class machine_data;

	virtual void populate(float &customtop, float &custombottom) override;
	virtual void handle() override;

	// drawing
	virtual float draw_left_panel(float x1, float y1, float x2, float y2) override;
	virtual render_texture *get_icon_texture(int linenum, void *selectedref) override;

	// get selected software and/or driver
	virtual void get_selection(ui_software_info const *&software, ui_system_info const *&system) const override;

	// text for main top/bottom panels
	virtual void make_topbox_text(std::string &line0, std::string &line1, std::string &line2) const override;
	virtual std::string make_software_description(ui_software_info const &software, ui_system_info const *system) const override;

	// filter navigation
	virtual void filter_selected() override;

	// toolbar
	virtual void inkey_export() override { throw false; }

	// handlers
	void inkey_select(const event *menu_event);

	std::map<std::string, std::string>  m_icon_paths;
	ui_system_info const                &m_system;
	std::shared_ptr<machine_data>       m_data;

	std::vector<std::reference_wrapper<ui_software_info const> > m_displaylist;
};

} // namespace ui

#endif // MAME_FRONTEND_UI_SELSOFT_H
