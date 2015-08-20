// license:BSD-3-Clause
// copyright-holders:Dankan1890
/***************************************************************************

    mewui/swcustmenu.h

    Internal MEWUI user interface.


***************************************************************************/

#pragma once

#ifndef __MEWUI_SWCUSTMENU_H__
#define __MEWUI_SWCUSTMENU_H__

#include "mewui/utils.h"

//-------------------------------------------------
//  custom filter menu class
//-------------------------------------------------
class ui_menu_swcustom_filter : public ui_menu
{
public:
	ui_menu_swcustom_filter(running_machine &machine, render_container *container, const game_driver *_driver,
							c_sw_region &_region, c_sw_publisher &_publisher, c_sw_year &_year, c_sw_type &_type, c_sw_list &_list);
	virtual ~ui_menu_swcustom_filter();
	virtual void populate();
	virtual void handle();
	virtual void custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2);

private:
	enum
	{
		MAIN_FILTER = 1,
		ADD_FILTER,
		REMOVE_FILTER,
		MNFCT_FILTER,
		YEAR_FILTER = MNFCT_FILTER + MAX_CUST_FILTER + 1,
		REGION_FILTER = YEAR_FILTER + MAX_CUST_FILTER + 1,
		TYPE_FILTER = REGION_FILTER + MAX_CUST_FILTER + 1,
		LIST_FILTER = TYPE_FILTER + MAX_CUST_FILTER + 1,
		OTHER_FILTER = LIST_FILTER + MAX_CUST_FILTER + 1
	};

	bool added;
	const game_driver *driver;
	c_sw_region &m_region;
	c_sw_publisher &m_publisher;
	c_sw_year &m_year;
	c_sw_type &m_type;
	c_sw_list &m_list;
	void save_sw_custom_filters();
};

#endif  /* __MEWUI_SWCUSTMENU_H__ */
