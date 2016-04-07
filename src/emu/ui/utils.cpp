// license:BSD-3-Clause
// copyright-holders:Maurizio Petrarota
/***************************************************************************

    ui/utils.cpp

    Internal UI user interface.

***************************************************************************/

#include "emu.h"
#include "ui/utils.h"
#include <algorithm>

extern const char UI_VERSION_TAG[];
const char UI_VERSION_TAG[] = "# UI INFO ";

// Years index
UINT16 c_year::actual = 0;
std::vector<std::string> c_year::ui;

// Manufacturers index
UINT16 c_mnfct::actual = 0;
std::vector<std::string> c_mnfct::ui;

// Main filters
UINT16 main_filters::actual = 0;
const char *main_filters::text[] = { "All", "Available", "Unavailable", "Working", "Not Working", "Mechanical", "Not Mechanical",
	"Category", "Favorites", "BIOS", "Parents", "Clones", "Manufacturers", "Years", "Support Save",
	"Not Support Save", "CHD", "No CHD", "Vertical", "Horizontal", "Custom" };
size_t main_filters::length = ARRAY_LENGTH(main_filters::text);

// Software filters
UINT16 sw_filters::actual = 0;
const char *sw_filters::text[] = { "All", "Available", "Unavailable", "Parents", "Clones", "Years", "Publishers", "Supported",
	"Partial Supported", "Unsupported", "Region", "Device Type", "Software List", "Custom" };
size_t sw_filters::length = ARRAY_LENGTH(sw_filters::text);

// Globals
UINT8 ui_globals::rpanel = 0;
UINT8 ui_globals::curimage_view = 0;
UINT8 ui_globals::curdats_view = 0;
UINT8 ui_globals::cur_sw_dats_view = 0;
bool ui_globals::switch_image = false;
bool ui_globals::default_image = true;
bool ui_globals::reset = false;
bool ui_globals::redraw_icon = false;
int ui_globals::visible_main_lines = 0;
int ui_globals::visible_sw_lines = 0;
UINT16 ui_globals::panels_status = 0;
bool ui_globals::has_icons = false;

// Custom filter
UINT16 custfltr::main = 0;
UINT16 custfltr::numother = 0;
UINT16 custfltr::other[MAX_CUST_FILTER];
UINT16 custfltr::mnfct[MAX_CUST_FILTER];
UINT16 custfltr::year[MAX_CUST_FILTER];
UINT16 custfltr::screen[MAX_CUST_FILTER];

// Custom filter
UINT16 sw_custfltr::main = 0;
UINT16 sw_custfltr::numother = 0;
UINT16 sw_custfltr::other[MAX_CUST_FILTER];
UINT16 sw_custfltr::mnfct[MAX_CUST_FILTER];
UINT16 sw_custfltr::year[MAX_CUST_FILTER];
UINT16 sw_custfltr::region[MAX_CUST_FILTER];
UINT16 sw_custfltr::type[MAX_CUST_FILTER];
UINT16 sw_custfltr::list[MAX_CUST_FILTER];

char* chartrimcarriage(char str[])
{
	char *pstr = strrchr(str, '\n');
	if (pstr)
		str[pstr - str] = '\0';
	pstr = strrchr(str, '\r');
	if (pstr)
		str[pstr - str] = '\0';
	return str;
}

const char* strensure(const char* s)
{
	return s == nullptr ? "" : s;
}

int getprecisionchr(const char* s)
{
	int precision = 1;
	char *dp = const_cast<char *>(strchr(s, '.'));
	if (dp != nullptr)
		precision = strlen(s) - (dp - s) - 1;
	return precision;
}

//-------------------------------------------------
//  search a substring with even partial matching
//-------------------------------------------------

int fuzzy_substring(std::string s_needle, std::string s_haystack)
{
	if (s_needle.empty())
		return s_haystack.size();
	if (s_haystack.empty())
		return s_needle.size();

	strmakelower(s_needle);
	strmakelower(s_haystack);

	if (s_needle == s_haystack)
		return 0;
	if (s_haystack.find(s_needle) != std::string::npos)
		return 0;

	auto *row1 = global_alloc_array_clear<int>(s_haystack.size() + 2);
	auto *row2 = global_alloc_array_clear<int>(s_haystack.size() + 2);

	for (int i = 0; i < s_needle.size(); ++i)
	{
		row2[0] = i + 1;
		for (int j = 0; j < s_haystack.size(); ++j)
		{
			int cost = (s_needle[i] == s_haystack[j]) ? 0 : 1;
			row2[j + 1] = MIN(row1[j + 1] + 1, MIN(row2[j] + 1, row1[j] + cost));
		}

		int *tmp = row1;
		row1 = row2;
		row2 = tmp;
	}

	int *first, *smallest;
	first = smallest = row1;
	int *last = row1 + s_haystack.size();

	while (++first != last)
		if (*first < *smallest)
			smallest = first;

	int rv = *smallest;
	global_free_array(row1);
	global_free_array(row2);

	return rv;
}

//-------------------------------------------------
//  set manufacturers
//-------------------------------------------------

void c_mnfct::set(const char *str)
{
	std::string name = getname(str);
	if (std::find(ui.begin(), ui.end(), name) != ui.end())
		return;

	ui.push_back(name);
}

std::string c_mnfct::getname(const char *str)
{
	std::string name(str);
	size_t found = name.find("(");

	if (found != std::string::npos)
		return (name.substr(0, found - 1));
	else
		return name;
}

//-------------------------------------------------
//  set years
//-------------------------------------------------

void c_year::set(const char *str)
{
	std::string name(str);
	if (std::find(ui.begin(), ui.end(), name) != ui.end())
		return;

	ui.push_back(name);
}
