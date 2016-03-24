// license:BSD-3-Clause
// copyright-holders:Maurizio Petrarota
/***************************************************************************

    ui/datmenu.h

    Internal UI user interface.


***************************************************************************/

#pragma once

#ifndef __UI_DATMENU_H__
#define __UI_DATMENU_H__

struct ui_software_info;

//-------------------------------------------------
//  class dats menu
//-------------------------------------------------

class ui_menu_dats_view : public ui_menu
{
public:
	ui_menu_dats_view(running_machine &machine, render_container *container, ui_software_info *swinfo, const game_driver *driver = nullptr);
	ui_menu_dats_view(running_machine &machine, render_container *container, const game_driver *driver = nullptr);
	virtual ~ui_menu_dats_view();
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

#endif  /* __UI_DATMENU_H__ */
