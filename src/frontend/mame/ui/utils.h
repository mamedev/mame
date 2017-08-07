// license:BSD-3-Clause
// copyright-holders:Maurizio Petrarota
/***************************************************************************

    ui/utils.h

    Internal UI user interface.

***************************************************************************/
#ifndef MAME_FRONTEND_UI_UTILS_H
#define MAME_FRONTEND_UI_UTILS_H

#pragma once

#include "unicode.h"


namespace ui {

class machine_filter
{
public:
	enum type : uint16_t
	{
		ALL = 0,
		AVAILABLE,
		UNAVAILABLE,
		WORKING,
		NOT_WORKING,
		MECHANICAL,
		NOT_MECHANICAL,
		CATEGORY,
		FAVORITE,
		BIOS,
		PARENT,
		CLONES,
		MANUFACTURER,
		YEAR,
		SAVE,
		NOSAVE,
		CHD,
		NOCHD,
		VERTICAL,
		HORIZONTAL,
		CUSTOM,

		COUNT,
		FIRST = 0,
		LAST = COUNT - 1
	};

	static char const *config_name(type n);
	static char const *display_name(type n);
};

DECLARE_ENUM_INCDEC_OPERATORS(machine_filter::type)


class software_filter
{
public:
	enum type : uint16_t
	{
		ALL = 0,
		AVAILABLE,
		UNAVAILABLE,
		PARENTS,
		CLONES,
		YEARS,
		PUBLISHERS,
		SUPPORTED,
		PARTIAL_SUPPORTED,
		UNSUPPORTED,
		REGION,
		DEVICE_TYPE,
		LIST,
		CUSTOM,

		COUNT,
		FIRST = 0,
		LAST = COUNT - 1
	};

	static char const *config_name(type n);
	static char const *display_name(type n);
};

DECLARE_ENUM_INCDEC_OPERATORS(software_filter::type)

} // namespace ui

#define MAX_CHAR_INFO            256
#define MAX_CUST_FILTER          8

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
	HOVER_FILTER_LAST = (HOVER_FILTER_FIRST) + ui::machine_filter::COUNT,
	HOVER_SW_FILTER_FIRST,
	HOVER_SW_FILTER_LAST = (HOVER_SW_FILTER_FIRST) + ui::software_filter::COUNT,
	HOVER_RP_FIRST,
	HOVER_RP_LAST = (HOVER_RP_FIRST) + 1 + RP_LAST
};

// GLOBAL STRUCTURES
struct ui_software_info
{
	ui_software_info() { }

	// info for software list item
	ui_software_info(
			software_info const &info,
			software_part const &p,
			game_driver const &d,
			std::string const &li,
			std::string const &is,
			std::string const &de);

	// info for starting empty
	ui_software_info(game_driver const &d);

	// copyable/movable
	ui_software_info(ui_software_info const &) = default;
	ui_software_info(ui_software_info &&) = default;
	ui_software_info &operator=(ui_software_info const &) = default;
	ui_software_info &operator=(ui_software_info &&) = default;

	bool operator==(ui_software_info const &r)
	{
		return shortname == r.shortname && longname == r.longname && parentname == r.parentname
			   && year == r.year && publisher == r.publisher && supported == r.supported
			   && part == r.part && driver == r.driver && listname == r.listname
			   && interface == r.interface && instance == r.instance && startempty == r.startempty
			   && parentlongname == r.parentlongname && usage == r.usage && devicetype == r.devicetype;
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

struct main_filters { static ui::machine_filter::type actual; };

// Custom filter
struct custfltr
{
	static ui::machine_filter::type  main;
	static uint16_t  numother;
	static ui::machine_filter::type  other[MAX_CUST_FILTER];
	static uint16_t  mnfct[MAX_CUST_FILTER];
	static uint16_t  screen[MAX_CUST_FILTER];
	static uint16_t  year[MAX_CUST_FILTER];
};

// Software custom filter
struct sw_custfltr
{
	static ui::software_filter::type main;
	static uint16_t  numother;
	static ui::software_filter::type other[MAX_CUST_FILTER];
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


#endif // MAME_FRONTEND_UI_UTILS_H
