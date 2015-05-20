/***************************************************************************

    mewui/inifile.h

    MEWUI inifile system.

***************************************************************************/

#pragma once

#ifndef __INIFILE_H__
#define __INIFILE_H__

#include "mewui/utils.h"

/**************************************************************************
    CONSTANTS AND STRUCTURES
**************************************************************************/
// category structure
struct IniCategoryIndex
{
    char *name;
    long offset;
    IniCategoryIndex *prev, *next;
};

// ini file structure
struct IniFileIndex
{
    char *file_name;
    int category_number;
    IniFileIndex *prev, *next;
    IniCategoryIndex *first_category, *last_category, *current_category;
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
    IniFileIndex *getfile() { return current_file_idx; }
    IniCategoryIndex *getcategory() { return current_file_idx->current_category; }
    IniCategoryIndex *getfirstcategory() { return current_file_idx->first_category; }
    IniFileIndex *getfirstfile() { return first_file_idx; }
    IniFileIndex *getlastfile() { return last_file_idx; }
    int category_total() const { return current_file_idx->category_number; }
    int files_total() const { return file_total; }
    bool has_files() const { return (file_total > 0); }
    int getfileindex();
    int getcategoryindex();

    // setters
    void setcategory(int direction, int index = -1);
    void setfile(int direction, int index = -1);

    // load games from category
    void load_ini_category(std::vector<int> &temp_filter);

    // free global index
    void free_ini_index();

private:

    // files indices
    static IniFileIndex *first_file_idx, *last_file_idx, *current_file_idx;

    // init category index
    int init_category(IniFileIndex &index, const char *file_name);

    // init file index
    void directory_scan();

    // file open/close
    bool ParseOpen(const char *filename);
    void ParseClose();

    void add_item(IniFileIndex &index, const char *name, long *file_offset);
    IniFileIndex *allocate(const char *name);

    // internal state
    running_machine &m_machine;     // reference to our machine
    static int      file_total;
    emu_file        *fp;
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
    static std::vector<ui_software_info> favorite_list;

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
    static int current_favorite;

    // parse file mewui_favorite
    void parse_favorite();

    // file open/close
    bool parseOpen(const char *filename);
    void parseClose();

    // internal state
    running_machine &m_machine;     // reference to our machine
    emu_file        *fp;
};

#endif  /* __INIFILE_H__ */
