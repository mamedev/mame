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

std::string reselect_last::driver;
std::string reselect_last::software;
std::string reselect_last::swlist;

std::vector<cache_info> mewui_globals::driver_cache(driver_list::total() + 1);

const char *mewui_globals::filter_text[] = { "All", "Available", "Unavailable", "Working", "Not Mechanical", "Category", "Favorites", "BIOS",
                                             "Originals", "Clones", "Not Working", "Mechanical", "Manufacturers", "Years", "Support Save",
                                             "Not Support Save", "CHD", "No CHD", "Use Samples", "Not Use Samples", "Stereo", "Vertical",
                                             "Horizontal", "Raster", "Vectors", "Custom" };

const char *mewui_globals::sw_filter_text[] = { "All", "Available", "Unavailable", "Originals", "Clones", "Years", "Publishers", "Supported",
                                                "Partial Supported", "Unsupported", "Region", "Device Type", "Custom" };

const char *mewui_globals::ume_text[] = { "ALL", "ARCADES", "SYSTEMS" };

size_t mewui_globals::s_filter_text = ARRAY_LENGTH(mewui_globals::filter_text);
size_t mewui_globals::sw_filter_len = ARRAY_LENGTH(mewui_globals::sw_filter_text);
size_t mewui_globals::s_ume_text = ARRAY_LENGTH(mewui_globals::ume_text);

//-------------------------------------------------
//  save game options
//-------------------------------------------------

void save_game_options(running_machine &machine)
{
	// attempt to open the output file
	emu_file file(machine.options().ini_path(), OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS);
	if (file.open(emulator_info::get_configname(), ".ini") == FILERR_NONE)
	{
		// generate the updated INI
		std::string initext;
		file.puts(machine.options().output_ini(initext));
		file.close();
	}
	else
		popmessage("**Error to save %s.ini**", emulator_info::get_configname());
}

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

	if (driver->flags & GAME_NOT_WORKING)
		buffer.append("Overall: NOT WORKING\n");
	else if (driver->flags & GAME_UNEMULATED_PROTECTION)
		buffer.append("Overall: Unemulated Protection\n");
	else
		buffer.append("Overall: Working\n");

	if (driver->flags & GAME_IMPERFECT_COLORS)
		buffer.append("Graphics: Imperfect Colors\n");
	else if (driver->flags & GAME_WRONG_COLORS)
		buffer.append("Graphics: Wrong Colors\n");
	else if (driver->flags & GAME_IMPERFECT_GRAPHICS)
		buffer.append("Graphics: Imperfect\n");
	else
		buffer.append("Graphics: OK\n");

	if (driver->flags & GAME_NO_SOUND)
		buffer.append("Sound: Unimplemented\n");
	else if (driver->flags & GAME_IMPERFECT_SOUND)
		buffer.append("Sound: Imperfect\n");
	else
		buffer.append("Sound: OK\n");

	strcatprintf(buffer, "Driver is Skeleton: %s\n", ((driver->flags & GAME_IS_SKELETON) ? "Yes" : "No"));
	strcatprintf(buffer, "Game is Mechanical: %s\n", ((driver->flags & GAME_MECHANICAL) ? "Yes" : "No"));
	strcatprintf(buffer, "Requires Artwork: %s\n", ((driver->flags & GAME_REQUIRES_ARTWORK) ? "Yes" : "No"));
	strcatprintf(buffer, "Requires Clickable Artwork: %s\n", ((driver->flags & GAME_CLICKABLE_ARTWORK) ? "Yes" : "No"));
	strcatprintf(buffer, "Support Cocktail: %s\n", ((driver->flags & GAME_NO_COCKTAIL) ? "Yes" : "No"));
	strcatprintf(buffer, "Driver is Bios: %s\n", ((driver->flags & GAME_IS_BIOS_ROOT) ? "Yes" : "No"));
	strcatprintf(buffer, "Support Save: %s\n", ((driver->flags & GAME_SUPPORTS_SAVE) ? "Yes" : "No"));

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
//  save custom filters info to file
//-------------------------------------------------

void save_custom_filters(running_machine &machine)
{
	// attempt to open the output file
	emu_file file(machine.options().mewui_path(), OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS);
	if (file.open("custom_", emulator_info::get_configname(), "_filter.ini") == FILERR_NONE)
	{
		// generate custom filters info
		std::string cinfo;
		strprintf(cinfo, "Total filters = %d\n", (custfltr::numother + 1));
		cinfo.append("Main filter = ").append(mewui_globals::filter_text[custfltr::main_filter]).append("\n");

		for (int x = 1; x <= custfltr::numother; x++)
		{
			cinfo.append("Other filter = ").append(mewui_globals::filter_text[custfltr::other[x]]).append("\n");
			if (custfltr::other[x] == FILTER_MANUFACTURER)
				cinfo.append("  Manufacturer filter = ").append(c_mnfct::ui[custfltr::mnfct[x]]).append("\n");
			else if (custfltr::other[x] == FILTER_YEAR)
				cinfo.append("  Year filter = ").append(c_year::ui[custfltr::year[x]]).append("\n");
		}
		file.puts(cinfo.c_str());
		file.close();
	}
}

//-------------------------------------------------
//  load custom filters info from file
//-------------------------------------------------

void load_custom_filters(running_machine &machine)
{
	// attempt to open the output file
	emu_file file(machine.options().mewui_path(), OPEN_FLAG_READ);
	if (file.open("custom_", emulator_info::get_configname(), "_filter.ini") == FILERR_NONE)
	{
		char buffer[MAX_CHAR_INFO];

		// get number of filters
		file.gets(buffer, MAX_CHAR_INFO);
		char *pb = strchr(buffer, '=');
		custfltr::numother = atoi(++pb) - 1;

		// get main filter
		file.gets(buffer, MAX_CHAR_INFO);
		pb = strchr(buffer, '=') + 2;

		for (int y = 0; y < mewui_globals::s_filter_text; y++)
			if (!strncmp(pb, mewui_globals::filter_text[y], strlen(mewui_globals::filter_text[y])))
			{
				custfltr::main_filter = y;
				break;
			}

		for (int x = 1; x <= custfltr::numother; x++)
		{
			file.gets(buffer, MAX_CHAR_INFO);
			char *cb = strchr(buffer, '=') + 2;
			for (int y = 0; y < mewui_globals::s_filter_text; y++)
				if (!strncmp(cb, mewui_globals::filter_text[y], strlen(mewui_globals::filter_text[y])))
				{
					custfltr::other[x] = y;
					if (y == FILTER_MANUFACTURER)
					{
						file.gets(buffer, MAX_CHAR_INFO);
						char *ab = strchr(buffer, '=') + 2;
						for (size_t z = 0; z < c_mnfct::ui.size(); z++)
							if (!strncmp(ab, c_mnfct::ui[z].c_str(), c_mnfct::ui[z].length()))
								custfltr::mnfct[x] = z;
					}
					else if (y == FILTER_YEAR)
					{
						file.gets(buffer, MAX_CHAR_INFO);
						char *db = strchr(buffer, '=') + 2;
						for (size_t z = 0; z < c_year::ui.size(); z++)
							if (!strncmp(db, c_year::ui[z].c_str(), c_year::ui[z].length()))
								custfltr::year[x] = z;
					}
				}
		}
		file.close();
	}

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

//-------------------------------------------------
//  save custom filters info to file
//-------------------------------------------------

void save_sw_custom_filters(running_machine &machine, const game_driver *driver, c_sw_region &m_region, c_sw_publisher &m_publisher, c_sw_year &m_year, c_sw_type &m_type)
{
	// attempt to open the output file
	emu_file file(machine.options().mewui_path(), OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS);
	if (file.open("custom_", driver->name, "_filter.ini") == FILERR_NONE)
	{
		// generate custom filters info
		std::string cinfo;
		strprintf(cinfo, "Total filters = %d\n", (sw_custfltr::numother + 1));
		cinfo.append("Main filter = ").append(mewui_globals::sw_filter_text[sw_custfltr::main_filter]).append("\n");

		for (int x = 1; x <= sw_custfltr::numother; x++)
		{
			cinfo.append("Other filter = ").append(mewui_globals::sw_filter_text[sw_custfltr::other[x]]).append("\n");
			if (sw_custfltr::other[x] == MEWUI_SW_PUBLISHERS)
				cinfo.append("  Manufacturer filter = ").append(m_publisher.ui[sw_custfltr::mnfct[x]]).append("\n");
			else if (sw_custfltr::other[x] == MEWUI_SW_YEARS)
				cinfo.append("  Year filter = ").append(m_year.ui[sw_custfltr::year[x]]).append("\n");
			else if (sw_custfltr::other[x] == MEWUI_SW_TYPE)
				cinfo.append("  Type filter = ").append(m_type.ui[sw_custfltr::type[x]]).append("\n");
			else if (sw_custfltr::other[x] == MEWUI_SW_REGION)
				cinfo.append("  Region filter = ").append(m_region.ui[sw_custfltr::region[x]]).append("\n");
		}
		file.puts(cinfo.c_str());
		file.close();
	}
}

//-------------------------------------------------
//  load custom filters info from file
//-------------------------------------------------

void load_sw_custom_filters(running_machine &machine, const game_driver *driver, c_sw_region &m_region, c_sw_publisher &m_publisher, c_sw_year &m_year, c_sw_type &m_type)
{
	// attempt to open the output file
	emu_file file(machine.options().mewui_path(), OPEN_FLAG_READ);
	if (file.open("custom_", driver->name, "_filter.ini") == FILERR_NONE)
	{
		char buffer[MAX_CHAR_INFO];

		// get number of filters
		file.gets(buffer, MAX_CHAR_INFO);
		char *pb = strchr(buffer, '=');
		sw_custfltr::numother = atoi(++pb) - 1;

		// get main filter
		file.gets(buffer, MAX_CHAR_INFO);
		pb = strchr(buffer, '=') + 2;

		for (int y = 0; y < mewui_globals::sw_filter_len; y++)
			if (!strncmp(pb, mewui_globals::sw_filter_text[y], strlen(mewui_globals::sw_filter_text[y])))
			{
				sw_custfltr::main_filter = y;
				break;
			}

		for (int x = 1; x <= sw_custfltr::numother; x++)
		{
			file.gets(buffer, MAX_CHAR_INFO);
			char *cb = strchr(buffer, '=') + 2;
			for (int y = 0; y < mewui_globals::sw_filter_len; y++)
				if (!strncmp(cb, mewui_globals::sw_filter_text[y], strlen(mewui_globals::sw_filter_text[y])))
				{
					sw_custfltr::other[x] = y;
					if (y == MEWUI_SW_PUBLISHERS)
					{
						file.gets(buffer, MAX_CHAR_INFO);
						char *ab = strchr(buffer, '=') + 2;
						for (size_t z = 0; z < m_publisher.ui.size(); z++)
							if (!strncmp(ab, m_publisher.ui[z].c_str(), m_publisher.ui[z].length()))
								sw_custfltr::mnfct[x] = z;
					}
					else if (y == MEWUI_SW_YEARS)
					{
						file.gets(buffer, MAX_CHAR_INFO);
						char *db = strchr(buffer, '=') + 2;
						for (size_t z = 0; z < m_year.ui.size(); z++)
							if (!strncmp(db, m_year.ui[z].c_str(), m_year.ui[z].length()))
								sw_custfltr::year[x] = z;
					}
					else if (y == MEWUI_SW_TYPE)
					{
						file.gets(buffer, MAX_CHAR_INFO);
						char *fb = strchr(buffer, '=') + 2;
						for (size_t z = 0; z < m_type.ui.size(); z++)
							if (!strncmp(fb, m_type.ui[z].c_str(), m_type.ui[z].length()))
								sw_custfltr::type[x] = z;
					}
					else if (y == MEWUI_SW_REGION)
					{
						file.gets(buffer, MAX_CHAR_INFO);
						char *eb = strchr(buffer, '=') + 2;
						for (size_t z = 0; z < m_region.ui.size(); z++)
							if (!strncmp(eb, m_region.ui[z].c_str(), m_region.ui[z].length()))
								sw_custfltr::region[x] = z;
					}
				}
		}
		file.close();
	}

}
