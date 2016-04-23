// license:BSD-3-Clause
// copyright-holders:Maurizio Petrarota
/***************************************************************************

    ui/custmenu.h

    Internal UI user interface.


***************************************************************************/

#pragma once

#ifndef __UI_CUSTMENU_H__
#define __UI_CUSTMENU_H__

#include "ui/utils.h"

// Software region
struct c_sw_region
{
	std::vector<std::string> ui;
	UINT16 actual;
	void set(std::string &str);
	std::string getname(std::string &str);
};

// Software publishers
struct c_sw_publisher
{
	std::vector<std::string> ui;
	UINT16 actual;
	void set(std::string &str);
	std::string getname(std::string &str);
};

// Software device type
struct c_sw_type
{
	std::vector<std::string> ui;
	UINT16 actual;
	void set(std::string &str);
};

// Software list
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
	void set(std::string &str);
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
	virtual void populate() override;
	virtual void handle() override;
	virtual void custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2) override;

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
	virtual void populate() override;
	virtual void handle() override;
	virtual void custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2) override;

private:
	enum
	{
		MAIN_FILTER = 1,
		ADD_FILTER,
		REMOVE_FILTER,
		MNFCT_FILTER,
		YEAR_FILTER   = MNFCT_FILTER  + MAX_CUST_FILTER + 1,
		SCREEN_FILTER = YEAR_FILTER   + MAX_CUST_FILTER + 1,
		OTHER_FILTER  = SCREEN_FILTER + MAX_CUST_FILTER + 1
	};

	bool m_single_menu, m_added;
	void save_custom_filters();
};

#endif  /* __UI_CUSTMENU_H__ */
