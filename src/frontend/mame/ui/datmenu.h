// license:BSD-3-Clause
// copyright-holders:Maurizio Petrarota, Vas Crabb
/***************************************************************************

    ui/datmenu.h

    Internal UI user interface.


***************************************************************************/

#ifndef MAME_FRONTEND_UI_DATMENU_H
#define MAME_FRONTEND_UI_DATMENU_H

#pragma once

#include "ui/text.h"
#include "ui/textbox.h"

#include <optional>
#include <string>
#include <utility>
#include <vector>


struct ui_software_info;
struct ui_system_info;


namespace ui {

//-------------------------------------------------
//  class dats menu
//-------------------------------------------------

class menu_dats_view : public menu_textbox
{
public:
	menu_dats_view(mame_ui_manager &mui, render_container &container, const ui_software_info &swinfo);
	menu_dats_view(mame_ui_manager &mui, render_container &container, const ui_system_info *system = nullptr);
	virtual ~menu_dats_view() override;

	static void add_info_text(text_layout &layout, std::string_view text, rgb_t color, float size = 1.0f);

protected:
	virtual void recompute_metrics(uint32_t width, uint32_t height, float aspect) override;
	virtual void custom_render(uint32_t flags, void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2) override;
	virtual std::tuple<int, bool, bool> custom_pointer_updated(bool changed, ui_event const &uievt) override;

	virtual void populate_text(std::optional<text_layout> &layout, float &width, int &lines) override;

private:
	struct list_items
	{
		list_items(std::string &&l, int i, std::string &&rev) : label(std::move(l)), revision(std::move(rev)), option(i), bounds(1.0F, 0.0F) { }

		std::string label;
		std::string revision;
		int option;

		std::pair<float, float> bounds;
	};

	virtual void populate() override;
	virtual bool handle(event const *ev) override;

	ui_system_info const *const m_system;
	ui_software_info const *const m_swinfo;
	bool const m_issoft;
	int m_current_tab;
	std::string m_list, m_short, m_long, m_parent;
	std::vector<list_items> m_items_list;

	std::pair<float, float> m_tab_line;
	int m_clicked_tab;
};

} // namespace ui

#endif // MAME_FRONTEND_UI_DATMENU_H
