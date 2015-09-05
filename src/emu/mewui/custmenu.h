// license:BSD-3-Clause
// copyright-holders:Dankan1890
/***************************************************************************

    mewui/custmenu.h

    Internal MEWUI user interface.


***************************************************************************/

#pragma once

#ifndef __MEWUI_CUSTMENU_H__
#define __MEWUI_CUSTMENU_H__

#include "mewui/utils.h"

// Software region
struct c_sw_region
{
	std::vector<std::string> ui;
	UINT16 actual;
	void set(const char *str);
	std::string getname(const char *str);
};

// Software publishers
struct c_sw_publisher
{
	std::vector<std::string> ui;
	UINT16 actual;
	void set(const char *str);
	std::string getname(const char *str);
};

// Software device type
struct c_sw_type
{
	std::vector<std::string> ui;
	UINT16 actual;
	void set(const char *str);
};

// Software device type
struct c_sw_list
{
	std::vector<std::string> name;
	std::vector<std::string> description;
	UINT16 actual;
};

// Software years
struct c_sw_year
{
	std::vector<std::string> ui;
	UINT16 actual;
	void set(const char *str);
};

struct s_filter
{
	c_sw_region     region;
	c_sw_publisher  publisher;
	c_sw_year       year;
	c_sw_type       type;
	c_sw_list       swlist;
};

//-------------------------------------------------
//  custom software filter menu class
//-------------------------------------------------
class ui_menu_swcustom_filter : public ui_menu
{
public:
	ui_menu_swcustom_filter(running_machine &machine, render_container *container, const game_driver *_driver, s_filter &_filter);
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
		YEAR_FILTER   = MNFCT_FILTER  + MAX_CUST_FILTER + 1,
		REGION_FILTER = YEAR_FILTER   + MAX_CUST_FILTER + 1,
		TYPE_FILTER   = REGION_FILTER + MAX_CUST_FILTER + 1,
		LIST_FILTER   = TYPE_FILTER   + MAX_CUST_FILTER + 1,
		OTHER_FILTER  = LIST_FILTER   + MAX_CUST_FILTER + 1
	};

	bool               m_added;
	s_filter           &m_filter;
	const game_driver  *m_driver;

	void save_sw_custom_filters();
};

//-------------------------------------------------
//  custom filter menu class
//-------------------------------------------------
class ui_menu_custom_filter : public ui_menu
{
public:
	ui_menu_custom_filter(running_machine &machine, render_container *container, bool _single_menu = false);
	virtual ~ui_menu_custom_filter();
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
		YEAR_FILTER  = MNFCT_FILTER + MAX_CUST_FILTER + 1,
		OTHER_FILTER = YEAR_FILTER  + MAX_CUST_FILTER + 1
	};

	bool m_single_menu, m_added;
	void save_custom_filters();
};

#endif  /* __MEWUI_CUSTMENU_H__ */
