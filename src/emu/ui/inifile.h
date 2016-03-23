// license:BSD-3-Clause
// copyright-holders:Maurizio Petrarota
/***************************************************************************

    ui/inifile.h

    UI INIs file manager.

***************************************************************************/

#pragma once

#ifndef __UI_INIFILE_H__
#define __UI_INIFILE_H__

#include "ui/utils.h"

//-------------------------------------------------
//  INIFILE MANAGER
//-------------------------------------------------

class inifile_manager
{
public:
	// construction/destruction
	inifile_manager(running_machine &machine);

	// getters
	running_machine &machine() const { return m_machine; }
	std::string get_file(int file = -1) { return ((file == -1) ? ini_index[c_file].first : ini_index[file].first); }
	std::string get_category(int cat = -1) { return ((cat == -1) ? ini_index[c_file].second[c_cat].first : ini_index[c_file].second[cat].first); }
	size_t total() { return ini_index.size(); }
	size_t cat_total() { return ini_index[c_file].second.size(); }
	UINT16 &cur_file() { return c_file; }
	UINT16 &cur_cat() { return c_cat; }

	// load games from category
	void load_ini_category(std::vector<int> &temp_filter);

	// setters
	void move_file(int d) { c_file += d; c_cat = 0; }
	void move_cat(int d) { c_cat += d; }
	void set_cat(int i = -1) { (i == -1) ? c_cat = 0 : c_cat = i; }
	void set_file(int i = -1) { (i == -1) ? c_file = 0 : c_file = i; }

private:

	// ini file structure
	using categoryindex = std::vector<std::pair<std::string, long>>;

	// files indices
	static UINT16 c_file, c_cat;
	std::vector<std::pair<std::string, categoryindex>> ini_index;

	// init category index
	void init_category(std::string &filename);

	// init file index
	void directory_scan();

	// file open/close/seek
	bool parseopen(const char *filename);
	void parseclose() { if (fp != nullptr) fclose(fp); }

	// internal state
	running_machine &m_machine;  // reference to our machine
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
	favorite_manager(running_machine &machine);

	// favorite indices
	std::vector<ui_software_info> m_list;

	// getters
	running_machine &machine() const { return m_machine; }

	// add
	void add_favorite_game();
	void add_favorite_game(const game_driver *driver);
	void add_favorite_game(ui_software_info &swinfo);

	// check
	bool isgame_favorite();
	bool isgame_favorite(const game_driver *driver);
	bool isgame_favorite(ui_software_info &swinfo);

	// save
	void save_favorite_games();

	// remove
	void remove_favorite_game();
	void remove_favorite_game(ui_software_info &swinfo);

private:
	const char *favorite_filename = "favorites.ini";

	// current
	int m_current;

	// parse file ui_favorite
	void parse_favorite();

	// internal state
	running_machine &m_machine;  // reference to our machine
};

#endif  /* __UI_INIFILE_H__ */
