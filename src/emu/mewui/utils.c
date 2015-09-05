// license:BSD-3-Clause
// copyright-holders:Dankan1890
/***************************************************************************

    mewui/utils.c

    Internal MEWUI user interface.

***************************************************************************/

#include "emu.h"
#include "mewui/utils.h"
#include "mewui/inifile.h"
#include "sound/samples.h"
#include "audit.h"
#include <fstream>

// Years index
UINT16 c_year::actual = 0;
std::vector<std::string> c_year::ui;

// Manufacturers index
UINT16 c_mnfct::actual = 0;
std::vector<std::string> c_mnfct::ui;

// Globals
UINT16 mewui_globals::actual_filter = 0;
UINT16 mewui_globals::actual_sw_filter = 0;
UINT8 mewui_globals::rpanel_infos = 0;
UINT8 mewui_globals::curimage_view = 0;
UINT8 mewui_globals::curdats_view = 0;
UINT8 mewui_globals::cur_sw_dats_view = 0;
bool mewui_globals::switch_image = false;
bool mewui_globals::default_image = true;
bool mewui_globals::force_reselect_software = false;
bool mewui_globals::force_reset_main = false;
bool mewui_globals::redraw_icon = false;
int mewui_globals::visible_main_lines = 0;
int mewui_globals::visible_sw_lines = 0;
UINT8 mewui_globals::ume_system = 0;

// Custom filter
UINT16 custfltr::main_filter = 0;
UINT16 custfltr::numother = 0;
UINT16 custfltr::other[MAX_CUST_FILTER];
UINT16 custfltr::mnfct[MAX_CUST_FILTER];
UINT16 custfltr::year[MAX_CUST_FILTER];

// Custom filter
UINT16 sw_custfltr::main_filter = 0;
UINT16 sw_custfltr::numother = 0;
UINT16 sw_custfltr::other[MAX_CUST_FILTER];
UINT16 sw_custfltr::mnfct[MAX_CUST_FILTER];
UINT16 sw_custfltr::year[MAX_CUST_FILTER];
UINT16 sw_custfltr::region[MAX_CUST_FILTER];
UINT16 sw_custfltr::type[MAX_CUST_FILTER];
UINT16 sw_custfltr::list[MAX_CUST_FILTER];

std::vector<cache_info> mewui_globals::driver_cache(driver_list::total() + 1);

const char *mewui_globals::filter_text[] = { "All", "Available", "Unavailable", "Working", "Not Mechanical", "Category", "Favorites", "BIOS",
                                             "Originals", "Clones", "Not Working", "Mechanical", "Manufacturers", "Years", "Support Save",
                                             "Not Support Save", "CHD", "No CHD", "Use Samples", "Not Use Samples", "Stereo", "Vertical",
                                             "Horizontal", "Raster", "Vectors", "Custom" };

const char *mewui_globals::sw_filter_text[] = { "All", "Available", "Unavailable", "Originals", "Clones", "Years", "Publishers", "Supported",
                                                "Partial Supported", "Unsupported", "Region", "Device Type", "Software List", "Custom" };

const char *mewui_globals::ume_text[] = { "ALL", "ARCADES", "SYSTEMS" };

size_t mewui_globals::s_filter_text = ARRAY_LENGTH(mewui_globals::filter_text);
size_t mewui_globals::sw_filter_len = ARRAY_LENGTH(mewui_globals::sw_filter_text);
size_t mewui_globals::s_ume_text = ARRAY_LENGTH(mewui_globals::ume_text);

//-------------------------------------------------
//  generate general info
//-------------------------------------------------

void general_info(running_machine &machine, const game_driver *driver, std::string &buffer)
{
	strprintf(buffer, "Romset: %-.100s\n", driver->name);
	buffer.append("Year: ").append(driver->year).append("\n");
	strcatprintf(buffer, "Manufacturer: %-.100s\n", driver->manufacturer);

	int cloneof = driver_list::non_bios_clone(*driver);
	if (cloneof != -1)
		strcatprintf(buffer, "Driver is Clone of: %-.100s\n", driver_list::driver(cloneof).description);
	else
		buffer.append("Driver is Parent\n");

	if (driver->flags & MACHINE_NOT_WORKING)
		buffer.append("Overall: NOT WORKING\n");
	else if (driver->flags & MACHINE_UNEMULATED_PROTECTION)
		buffer.append("Overall: Unemulated Protection\n");
	else
		buffer.append("Overall: Working\n");

	if (driver->flags & MACHINE_IMPERFECT_COLORS)
		buffer.append("Graphics: Imperfect Colors\n");
	else if (driver->flags & MACHINE_WRONG_COLORS)
		buffer.append("Graphics: Wrong Colors\n");
	else if (driver->flags & MACHINE_IMPERFECT_GRAPHICS)
		buffer.append("Graphics: Imperfect\n");
	else
		buffer.append("Graphics: OK\n");

	if (driver->flags & MACHINE_NO_SOUND)
		buffer.append("Sound: Unimplemented\n");
	else if (driver->flags & MACHINE_IMPERFECT_SOUND)
		buffer.append("Sound: Imperfect\n");
	else
		buffer.append("Sound: OK\n");

	strcatprintf(buffer, "Driver is Skeleton: %s\n", ((driver->flags & MACHINE_IS_SKELETON) ? "Yes" : "No"));
	strcatprintf(buffer, "Game is Mechanical: %s\n", ((driver->flags & MACHINE_MECHANICAL) ? "Yes" : "No"));
	strcatprintf(buffer, "Requires Artwork: %s\n", ((driver->flags & MACHINE_REQUIRES_ARTWORK) ? "Yes" : "No"));
	strcatprintf(buffer, "Requires Clickable Artwork: %s\n", ((driver->flags & MACHINE_CLICKABLE_ARTWORK) ? "Yes" : "No"));
	strcatprintf(buffer, "Support Cocktail: %s\n", ((driver->flags & MACHINE_NO_COCKTAIL) ? "Yes" : "No"));
	strcatprintf(buffer, "Driver is Bios: %s\n", ((driver->flags & MACHINE_IS_BIOS_ROOT) ? "Yes" : "No"));
	strcatprintf(buffer, "Support Save: %s\n", ((driver->flags & MACHINE_SUPPORTS_SAVE) ? "Yes" : "No"));

	int idx = driver_list::find(driver->name);
	strcatprintf(buffer, "Screen Type: %s\n", (mewui_globals::driver_cache[idx].b_vector ? "Vector" : "Raster"));
	strcatprintf(buffer, "Screen Orentation: %s\n", ((driver->flags & ORIENTATION_SWAP_XY) ? "Vertical" : "Horizontal"));
	strcatprintf(buffer, "Requires Samples: %s\n", (mewui_globals::driver_cache[idx].b_samples ? "Yes" : "No"));
	strcatprintf(buffer, "Sound Channel: %s\n", (mewui_globals::driver_cache[idx].b_stereo ? "Stereo" : "Mono"));
	strcatprintf(buffer, "Requires CHD: %s\n", (mewui_globals::driver_cache[idx].b_chd ? "Yes" : "No"));

	// audit the game first to see if we're going to work
	driver_enumerator enumerator(machine.options(), *driver);
	enumerator.next();
	media_auditor auditor(enumerator);
	media_auditor::summary summary = auditor.audit_media(AUDIT_VALIDATE_FAST);
	media_auditor::summary summary_samples = auditor.audit_samples();

	// if everything looks good, schedule the new driver
	if (summary == media_auditor::CORRECT || summary == media_auditor::BEST_AVAILABLE || summary == media_auditor::NONE_NEEDED)
		buffer.append("Roms Audit Pass: OK\n");
	else
		buffer.append("Roms Audit Pass: BAD\n");

	if (summary_samples == media_auditor::NONE_NEEDED)
		buffer.append("Samples Audit Pass: None Needed\n");
	else if (summary_samples == media_auditor::CORRECT || summary_samples == media_auditor::BEST_AVAILABLE)
		buffer.append("Samples Audit Pass: OK\n");
	else
		buffer.append("Samples Audit Pass: BAD\n");
}

//-------------------------------------------------
//  search a substring with even partial matching
//-------------------------------------------------

int fuzzy_substring(const char *needle, const char *haystack)
{
	const int nlen = strlen(needle);
	const int hlen = strlen(haystack);

	if (hlen == 0) return nlen;
	if (nlen == 0) return hlen;

	std::string s_needle(needle);
	std::string s_haystack(haystack);

	strmakelower(s_needle);
	strmakelower(s_haystack);

	if (s_needle == s_haystack)
		return 0;
	if (s_haystack.find(s_needle) != std::string::npos)
		return 0;

	int *row1 = global_alloc_array_clear(int, hlen + 2);
	int *row2 = global_alloc_array_clear(int, hlen + 2);

	for (int i = 0; i < nlen; ++i)
	{
		row2[0] = i + 1;
		for (int j = 0; j < hlen; ++j)
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
	int *last = row1 + hlen;

	while (++first != last)
		if (*first < *smallest)
			smallest = first;

	int rv = *smallest;
	global_free_array(row1);
	global_free_array(row2);

	return rv;
}
//-------------------------------------------------
//  search a substring with even partial matching
//-------------------------------------------------

int fuzzy_substring2(const char *needle, const char *haystack)
{
	std::string s1(needle);
	std::string s2(haystack);
	strmakelower(s1);
	strmakelower(s2);
	const size_t m(s1.size());
	const size_t n(s2.size());

	if (m == 0)
		return n;
	if (n == 0)
		return m;

	if (s1 == s2)
		return 0;
	size_t it = s2.find(s1);
	if (it != std::string::npos)
		return it;

	size_t *costs = global_alloc_array(size_t, n + 1);
	for(size_t k = 0; k <= n; ++k)
		costs[k] = k;
	size_t i = 0;
	for (std::string::const_iterator it1 = s1.begin(); it1 != s1.end(); ++it1, ++i)
	{
		costs[0] = i+1;
		size_t corner = i;
		size_t j = 0;
		for (std::string::const_iterator it2 = s2.begin(); it2 != s2.end(); ++it2, ++j)
		{
			size_t upper = costs[j+1];
			if (*it1 == *it2)
				costs[j+1] = corner;
			else
			{
				size_t t(upper < corner ? upper : corner);
				costs[j+1] = (costs[j] < t ? costs[j] : t) + 1;
			}
			corner = upper;
		}
	}

	size_t result = costs[n];
	global_free_array(costs);
	return result;
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

