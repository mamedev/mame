// license:BSD-3-Clause
// copyright-holders:Maurizio Petrarota
/***************************************************************************

    ui/utils.h

    Internal UI user interface.

***************************************************************************/

#pragma once

#ifndef __UI_UTILS_H__
#define __UI_UTILS_H__

#include "unicode.h"

#define MAX_CHAR_INFO            256
#define MAX_CUST_FILTER          8

// GLOBAL ENUMERATORS
enum : uint16_t
{
	FILTER_FIRST = 0,
	FILTER_ALL = FILTER_FIRST,
	FILTER_AVAILABLE,
	FILTER_UNAVAILABLE,
	FILTER_WORKING,
	FILTER_NOT_WORKING,
	FILTER_MECHANICAL,
	FILTER_NOT_MECHANICAL,
	FILTER_CATEGORY,
	FILTER_FAVORITE,
	FILTER_BIOS,
	FILTER_PARENT,
	FILTER_CLONES,
	FILTER_MANUFACTURER,
	FILTER_YEAR,
	FILTER_SAVE,
	FILTER_NOSAVE,
	FILTER_CHD,
	FILTER_NOCHD,
	FILTER_VERTICAL,
	FILTER_HORIZONTAL,
	FILTER_CUSTOM,
	FILTER_LAST = FILTER_CUSTOM
};

enum
{
	FIRST_VIEW = 0,
	SNAPSHOT_VIEW = FIRST_VIEW,
	CABINETS_VIEW,
	CPANELS_VIEW,
	PCBS_VIEW,
	FLYERS_VIEW,
	TITLES_VIEW,
	ENDS_VIEW,
	ARTPREV_VIEW,
	BOSSES_VIEW,
	LOGOS_VIEW,
	VERSUS_VIEW,
	GAMEOVER_VIEW,
	HOWTO_VIEW,
	SCORES_VIEW,
	SELECT_VIEW,
	MARQUEES_VIEW,
	LAST_VIEW = MARQUEES_VIEW
};

enum
{
	RP_FIRST = 0,
	RP_IMAGES = RP_FIRST,
	RP_INFOS,
	RP_LAST = RP_INFOS
};

enum
{
	SHOW_PANELS = 0,
	HIDE_LEFT_PANEL,
	HIDE_RIGHT_PANEL,
	HIDE_BOTH
};

enum : uint16_t
{
	UI_SW_FIRST = 0,
	UI_SW_ALL = UI_SW_FIRST,
	UI_SW_AVAILABLE,
	UI_SW_UNAVAILABLE,
	UI_SW_PARENTS,
	UI_SW_CLONES,
	UI_SW_YEARS,
	UI_SW_PUBLISHERS,
	UI_SW_SUPPORTED,
	UI_SW_PARTIAL_SUPPORTED,
	UI_SW_UNSUPPORTED,
	UI_SW_REGION,
	UI_SW_TYPE,
	UI_SW_LIST,
	UI_SW_CUSTOM,
	UI_SW_LAST = UI_SW_CUSTOM
};

enum
{
	HOVER_DAT_UP = -1000,
	HOVER_DAT_DOWN,
	HOVER_UI_LEFT,
	HOVER_UI_RIGHT,
	HOVER_ARROW_UP,
	HOVER_ARROW_DOWN,
	HOVER_B_FAV,
	HOVER_B_EXPORT,
	HOVER_B_DATS,
	HOVER_RPANEL_ARROW,
	HOVER_LPANEL_ARROW,
	HOVER_FILTER_FIRST,
	HOVER_FILTER_LAST = (HOVER_FILTER_FIRST) + 1 + FILTER_LAST,
	HOVER_SW_FILTER_FIRST,
	HOVER_SW_FILTER_LAST = (HOVER_SW_FILTER_FIRST) + 1 + UI_SW_LAST,
	HOVER_RP_FIRST,
	HOVER_RP_LAST = (HOVER_RP_FIRST) + 1 + RP_LAST
};

// GLOBAL STRUCTURES
struct ui_software_info
{
	ui_software_info() {}
	ui_software_info(std::string sname, std::string lname, std::string pname, std::string y, std::string pub,
		uint8_t s, std::string pa, const game_driver *d, std::string li, std::string i, std::string is, uint8_t em,
		std::string plong, std::string u, std::string de, bool av)
	{
		shortname = sname; longname = lname; parentname = pname; year = y; publisher = pub;
		supported = s; part = pa; driver = d; listname = li; interface = i; instance = is; startempty = em;
		parentlongname = plong; usage = u; devicetype = de; available = av;
	}
	std::string shortname;
	std::string longname;
	std::string parentname;
	std::string year;
	std::string publisher;
	uint8_t supported = 0;
	std::string part;
	const game_driver *driver = nullptr;
	std::string listname;
	std::string interface;
	std::string instance;
	uint8_t startempty = 0;
	std::string parentlongname;
	std::string usage;
	std::string devicetype;
	bool available = false;

	bool operator==(const ui_software_info& r)
	{
		return shortname == r.shortname && longname == r.longname && parentname == r.parentname
			   && year == r.year && publisher == r.publisher && supported == r.supported
			   && part == r.part && driver == r.driver && listname == r.listname
			   && interface == r.interface && instance == r.instance && startempty == r.startempty
			   && parentlongname == r.parentlongname && usage == r.usage && devicetype == r.devicetype;
	}
};

// Manufacturers
struct c_mnfct
{
	static void set(const char *str);
	static std::string getname(const char *str);
	static std::vector<std::string> ui;
	static std::unordered_map<std::string, int> uimap;
	static uint16_t actual;
};

// Years
struct c_year
{
	static void set(const char *str);
	static std::vector<std::string> ui;
	static uint16_t actual;
};

// GLOBAL CLASS
struct ui_globals
{
	static uint8_t        curimage_view, curdats_view, curdats_total, cur_sw_dats_view, cur_sw_dats_total, rpanel;
	static bool         switch_image, redraw_icon, default_image, reset;
	static int          visible_main_lines, visible_sw_lines;
	static uint16_t       panels_status;
	static bool         has_icons;
};

#define main_struct(name) \
struct name##_filters \
{ \
	static uint16_t actual; \
	static const char *text[]; \
	static size_t length; \
};

main_struct(main);
main_struct(sw);

// Custom filter
struct custfltr
{
	static uint16_t  main;
	static uint16_t  numother;
	static uint16_t  other[MAX_CUST_FILTER];
	static uint16_t  mnfct[MAX_CUST_FILTER];
	static uint16_t  screen[MAX_CUST_FILTER];
	static uint16_t  year[MAX_CUST_FILTER];
};

// Software custom filter
struct sw_custfltr
{
	static uint16_t  main;
	static uint16_t  numother;
	static uint16_t  other[MAX_CUST_FILTER];
	static uint16_t  mnfct[MAX_CUST_FILTER];
	static uint16_t  year[MAX_CUST_FILTER];
	static uint16_t  region[MAX_CUST_FILTER];
	static uint16_t  type[MAX_CUST_FILTER];
	static uint16_t  list[MAX_CUST_FILTER];
};

// GLOBAL FUNCTIONS
int fuzzy_substring(std::string needle, std::string haystack);
char* chartrimcarriage(char str[]);
const char* strensure(const char* s);
int getprecisionchr(const char* s);
std::vector<std::string> tokenize(const std::string &text, char sep);


//-------------------------------------------------
//  input_character - inputs a typed character
//  into a buffer
//-------------------------------------------------

template <typename F>
bool input_character(std::string &buffer, std::string::size_type size, char32_t unichar, F &&filter)
{
	bool result = false;
	auto buflen = buffer.size();

	if ((unichar == 8) || (unichar == 0x7f))
	{
		// backspace
		if (0 < buflen)
		{
			auto buffer_oldend = buffer.c_str() + buflen;
			auto buffer_newend = utf8_previous_char(buffer_oldend);
			buffer.resize(buffer_newend - buffer.c_str());
			result = true;
		}
	}
	else if ((unichar >= ' ') && filter(unichar))
	{
		// append this character - check against the size first
		std::string utf8_char = utf8_from_uchar(unichar);
		if ((buffer.size() + utf8_char.size()) <= size)
		{
			buffer += utf8_char;
			result = true;
		}
	}
	return result;
}


//-------------------------------------------------
//  input_character - inputs a typed character
//  into a buffer
//-------------------------------------------------

template <typename F>
bool input_character(std::string &buffer, char32_t unichar, F &&filter)
{
	auto size = std::numeric_limits<std::string::size_type>::max();
	return input_character(buffer, size, unichar, filter);
}


#endif /* __UI_UTILS_H__ */
