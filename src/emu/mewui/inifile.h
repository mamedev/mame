/***************************************************************************

    mewui/inifile.h

    MEWUI inifile system.

***************************************************************************/

#pragma once

#ifndef __MEWUI_INIFILE_H__
#define __MEWUI_INIFILE_H__

#include "mewui/utils.h"

/**************************************************************************
    CONSTANTS AND STRUCTURES
**************************************************************************/
// category structure
struct IniCategoryIndex
{
	std::string name;
	long offset;
};

// ini file structure
struct IniFileIndex
{
	std::string name;
	std::vector<IniCategoryIndex> category;
};

/**************************************************************************
    INIFILE MANAGER
**************************************************************************/

class inifile_manager
{
public:

	// construction/destruction
	inifile_manager(running_machine &machine);

	// getters
	running_machine &machine() const { return m_machine; }

	// load games from category
	void load_ini_category(std::vector<int> &temp_filter);

	// files indices
	std::vector<IniFileIndex> ini_index;
	static UINT16 current_file, current_category;

private:

	// init category index
	void init_category(std::vector<IniCategoryIndex> &index, std::string &filename);

	// init file index
	void directory_scan();

	// file open/close
	bool ParseOpen(const char *filename);

	// internal state
	running_machine &m_machine;  // reference to our machine
	std::string     fullpath;
};

/**************************************************************************
    FAVORITE MANAGER
**************************************************************************/

class favorite_manager
{
public:

	// construction/destruction
	favorite_manager(running_machine &machine);

	// favorite indices
	std::vector<ui_software_info> favorite_list;

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

	// current
	int current_favorite;

	// parse file mewui_favorite
	void parse_favorite();

	// internal state
	running_machine &m_machine;  // reference to our machine
};

#endif  /* __MEWUI_INIFILE_H__ */
