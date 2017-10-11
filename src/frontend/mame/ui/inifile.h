// license:BSD-3-Clause
// copyright-holders:Maurizio Petrarota
/***************************************************************************

    ui/inifile.h

    UI INIs file manager.

***************************************************************************/

#ifndef MAME_FRONTEND_UI_INIFILE_H
#define MAME_FRONTEND_UI_INIFILE_H

#pragma once

#include "ui/utils.h"

#include <unordered_set>


//-------------------------------------------------
//  INIFILE MANAGER
//-------------------------------------------------

class ui_options;

class inifile_manager
{
public:
	// construction/destruction
	inifile_manager(running_machine &machine, ui_options &moptions);

	// load systems from category
	void load_ini_category(size_t file, size_t category, std::unordered_set<game_driver const *> &result) const;

	// getters
	size_t get_file_count() const { return m_ini_index.size(); }
	std::string const &get_file_name(size_t file) const { return m_ini_index[file].first; }
	size_t get_category_count(size_t file) const { return m_ini_index[file].second.size(); }
	std::string const &get_category_name(size_t file, size_t category) const { return m_ini_index[file].second[category].first; }

private:
	// ini file structure
	using categoryindex = std::vector<std::pair<std::string, int64_t>>;

	void init_category(std::string &&filename, emu_file &file);

	// internal state
	ui_options &m_options;
	std::vector<std::pair<std::string, categoryindex> > m_ini_index;

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
	void remove_favorite_game(ui_software_info const &swinfo);

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
