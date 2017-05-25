// license:BSD-3-Clause
// copyright-holders:Maurizio Petrarota
/***************************************************************************

    ui/inifile.h

    UI INIs file manager.

***************************************************************************/

#ifndef MAME_FRONTEND_UI_INIFILE_H
#define MAME_FRONTEND_UI_INIFILE_H

#pragma once

#include "../frontend/mame/ui/utils.h"

//-------------------------------------------------
//  INIFILE MANAGER
//-------------------------------------------------

class ui_options;

class inifile_manager
{
public:
	// construction/destruction
	inifile_manager(running_machine &machine, ui_options &moptions);

	// getters
	running_machine &machine() const { return m_machine; }
	std::string get_file() { return ini_index[c_file].first; }
	std::string get_file(int file) { return ini_index[file].first; }
	std::string get_category(int cat) { return ini_index[c_file].second[cat].first; }
	std::string get_category() { return ini_index[c_file].second[c_cat].first; }
	size_t total() { return ini_index.size(); }
	size_t cat_total() { return ini_index[c_file].second.size(); }
	uint16_t &cur_file() { return c_file; }
	uint16_t &cur_cat() { return c_cat; }

	// load games from category
	void load_ini_category(std::vector<int> &temp_filter);

	// setters
	void move_file(int d) { c_file += d; c_cat = 0; }
	void move_cat(int d) { c_cat += d; }
	void set_cat(uint16_t i) { c_cat = i; }
	void set_file(uint16_t i) { c_file = i; }

private:

	// ini file structure
	using categoryindex = std::vector<std::pair<std::string, long>>;

	// files indices
	static uint16_t c_file, c_cat;
	std::vector<std::pair<std::string, categoryindex>> ini_index;

	// init category index
	void init_category(std::string filename);

	// init file index
	void directory_scan();

	// file open/close/seek
	bool parseopen(const char *filename);
	void parseclose() { if (fp != nullptr) fclose(fp); }

	// internal state
	running_machine &m_machine;  // reference to our machine
	ui_options      &m_options;
	std::string     m_fullpath;
	FILE            *fp = nullptr;
};

//-------------------------------------------------
//  FAVORITE MANAGER
//-------------------------------------------------

class favorite_manager
{
public:
	// construction/destruction
	favorite_manager(running_machine &machine, ui_options &moptions);

	// favorites comparator
	struct ci_less
	{
		bool operator() (const std::string &s1, const std::string &s2) const
		{
			return (core_stricmp(s1.c_str(), s2.c_str()) < 0);
		}
	};

	// favorite indices
	std::multimap<std::string, ui_software_info, ci_less> m_list;

	// getters
	running_machine &machine() const { return m_machine; }

	// add
	void add_favorite_game();
	void add_favorite_game(const game_driver *driver);
	void add_favorite_game(ui_software_info &swinfo);

	// check
	bool isgame_favorite();
	bool isgame_favorite(const game_driver *driver);
	bool isgame_favorite(ui_software_info const &swinfo);

	// save
	void save_favorite_games();

	// remove
	void remove_favorite_game();
	void remove_favorite_game(ui_software_info &swinfo);

private:
	const char *favorite_filename = "favorites.ini";

	// current
	std::multimap<std::string, ui_software_info>::iterator m_current;

	// parse file ui_favorite
	void parse_favorite();

	// internal state
	running_machine &m_machine;  // reference to our machine
	ui_options &m_options;
};

#endif  // MAME_FRONTEND_UI_INIFILE_H
