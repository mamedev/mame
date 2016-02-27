// license:BSD-3-Clause
// copyright-holders:Dankan1890
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
	// category structure
	struct IniCategoryIndex
	{
		IniCategoryIndex(std::string _name, long _offset) { name = _name; offset = _offset; }
		std::string name;
		long offset;
	};

	using categoryindex = std::vector<IniCategoryIndex>;

	// ini file structure
	struct IniFileIndex
	{
		IniFileIndex(std::string _name, categoryindex _category) { name = _name; category = _category; }
		std::string name;
		categoryindex category;
	};

	// construction/destruction
	inifile_manager(running_machine &machine);

	// getters
	running_machine &machine() const { return m_machine; }

	// load games from category
	void load_ini_category(std::vector<int> &temp_filter);

	// files indices
	std::vector<IniFileIndex> ini_index;
	static UINT16 current_file, current_category;

	std::string actual_file() { return ini_index[current_file].name; }
	std::string actual_category() { return ini_index[current_file].category[current_category].name; }

private:
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
