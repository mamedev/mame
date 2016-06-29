// license:BSD-3-Clause
// copyright-holders:Maurizio Petrarota
/***************************************************************************

    ui/datmenu.h

    Internal UI user interface.


***************************************************************************/

#pragma once

#ifndef MAME_FRONTEND_UI_DATMENU_H
#define MAME_FRONTEND_UI_DATMENU_H

#include "ui/menu.h"

#include <string>
#include <vector>


struct ui_software_info;

namespace ui {
//-------------------------------------------------
//  class dats menu
//-------------------------------------------------

class menu_dats_view : public menu
{
public:
	menu_dats_view(mame_ui_manager &mui, render_container *container, ui_software_info *swinfo, const game_driver *driver = nullptr);
	menu_dats_view(mame_ui_manager &mui, render_container *container, const game_driver *driver = nullptr);
	virtual ~menu_dats_view() override;
	virtual void populate() override;
	virtual void handle() override;
	virtual void custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2) override;

private:
	int m_actual;
	const game_driver  *m_driver;
	ui_software_info *m_swinfo;
	std::string m_list, m_short, m_long, m_parent;
	void get_data();
	void get_data_sw();
	void init_items();
	bool m_issoft;
	struct list_items
	{
		list_items(std::string l, int i, std::string rev) { label = l; option = i; revision = rev; }
		std::string label;
		int option;
		std::string revision;
	};
	std::vector<list_items> m_items_list;
};

} // namespace ui

#endif  /* MAME_FRONTEND_UI_DATMENU_H */
