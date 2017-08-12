// license:BSD-3-Clause
// copyright-holders:Maurizio Petrarota
/***************************************************************************

    ui/custmenu.h

    Internal UI user interface.


***************************************************************************/

#ifndef MAME_FRONTEND_UI_CUSTMENU_H
#define MAME_FRONTEND_UI_CUSTMENU_H

#pragma once


namespace ui {

// Software region
struct c_sw_region
{
	std::vector<std::string> ui;
	void set(std::string const &str);
	std::string getname(std::string const &str) const;
};

// Software publishers
struct c_sw_publisher
{
	std::vector<std::string> ui;
	void set(std::string const &str);
	std::string getname(std::string const &str) const;
};

// Software device type
struct c_sw_type
{
	std::vector<std::string> ui;
	void set(std::string &str);
};

// Software list
struct c_sw_list
{
	std::vector<std::string> name;
	std::vector<std::string> description;
};

// Software years
struct c_sw_year
{
	std::vector<std::string> ui;
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

} // namespace ui

#endif  /* MAME_FRONTEND_UI_CUSTMENU_H */
