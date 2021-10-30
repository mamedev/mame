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
	virtual void custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2) override;
	virtual bool custom_mouse_down() override;

	virtual void populate_text(std::optional<text_layout> &layout, float &width, int &lines) override;

private:
	struct list_items
	{
		list_items(std::string &&l, int i, std::string &&rev) : label(std::move(l)), option(i), revision(std::move(rev)) { }

		std::string label;
		int option;
		std::string revision;
	};

	virtual void populate(float &customtop, float &custombottom) override;
	virtual void handle(event const *ev) override;

	void get_data(std::string &buffer);
	void get_data_sw(std::string &buffer);

	ui_system_info const *const m_system;
	ui_software_info const *const m_swinfo;
	bool const m_issoft;
	int m_actual;
	std::string m_list, m_short, m_long, m_parent;
	std::vector<list_items> m_items_list;
};

} // namespace ui

#endif // MAME_FRONTEND_UI_DATMENU_H
