// license:BSD-3-Clause
// copyright-holders:Maurizio Petrarota
/*********************************************************************

    ui/custmenu.cpp

    Internal UI user interface.

*********************************************************************/

#include "emu.h"
#include "ui/custmenu.h"

#include <algorithm>


namespace ui {

namespace {

constexpr char const *region_lists[] = {
		"arab", "arg", "asia", "aus", "aut",
		"bel", "blr", "bra",
		"can", "chi", "chn", "cze",
		"den",
		"ecu", "esp", "euro",
		"fin", "fra",
		"gbr", "ger", "gre",
		"hkg", "hun",
		"irl", "isr", "isv", "ita",
		"jpn",
		"kaz", "kor",
		"lat", "lux",
		"mex",
		"ned", "nld", "nor", "nzl",
		"pol",
		"rus",
		"slo", "spa", "sui", "swe",
		"tha", "tpe", "tw",
		"uk", "ukr", "usa" };

} // anonymous namespace

//-------------------------------------------------
//  set software regions
//-------------------------------------------------

void c_sw_region::set(std::string const &str)
{
	std::string name(getname(str));
	std::vector<std::string>::iterator const pos(std::lower_bound(ui.begin(), ui.end(), name));
	if ((ui.end() == pos) || (*pos != str))
		ui.emplace(pos, std::move(name));
}

std::string c_sw_region::getname(std::string const &str) const
{
	std::string fullname(str);
	strmakelower(fullname);
	size_t found = fullname.find("(");

	if (found != std::string::npos)
	{
		size_t ends = fullname.find_first_not_of("abcdefghijklmnopqrstuvwxyz", found + 1);
		std::string temp(fullname.substr(found + 1, ends - found - 1));

		for (auto & elem : region_lists)
			if (temp == elem)
				return (str.substr(found + 1, ends - found - 1));
	}
	return std::string("<none>");
}

//-------------------------------------------------
//  set software device type
//-------------------------------------------------

void c_sw_type::set(std::string &str)
{
	std::vector<std::string>::iterator const pos(std::lower_bound(ui.begin(), ui.end(), str));
	if ((ui.end() == pos) || (*pos != str))
		ui.emplace(pos, str);
}

//-------------------------------------------------
//  set software years
//-------------------------------------------------

void c_sw_year::set(std::string &str)
{
	std::vector<std::string>::iterator const pos(std::lower_bound(ui.begin(), ui.end(), str));
	if ((ui.end() == pos) || (*pos != str))
		ui.emplace(pos, str);
}

//-------------------------------------------------
//  set software publishers
//-------------------------------------------------

void c_sw_publisher::set(std::string const &str)
{
	std::string name(getname(str));
	std::vector<std::string>::iterator const pos(std::lower_bound(ui.begin(), ui.end(), name));
	if ((ui.end() == pos) || (*pos != name))
		ui.emplace(pos, std::move(name));
}

std::string c_sw_publisher::getname(std::string const &str) const
{
	size_t found = str.find("(");

	if (found != std::string::npos)
		return (str.substr(0, found - 1));
	else
		return str;
}

} // namespace ui
