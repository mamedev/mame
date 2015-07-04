/***************************************************************************

    mewui/datfile.h

    MEWUI data file.

***************************************************************************/

#pragma once

#ifndef __MEWUI_DATFILE_H__
#define __MEWUI_DATFILE_H__

//-------------------------------------------------
//  STRUCTURES
//-------------------------------------------------
struct tDatafileIndex
{
	long              offset;
	const game_driver *driver;
};

struct sDataDrvIndex
{
	long        offset;
	std::string name;
};

struct tMenuIndex
{
	UINT64 offset;
	std::string menuitem;
};

struct SoftwareItem
{
	std::string softname;
	long        offset;
};

struct SoftwareListIndex
{
	std::string               listname;
	std::vector<SoftwareItem> item_list;
};

//-------------------------------------------------
//  Datafile Manager
//-------------------------------------------------
class datfile_manager
{
public:

	// construction/destruction
	datfile_manager(running_machine &machine);

	// getters
	running_machine &machine() const { return m_machine; }

	// actions
	void load_data_info(const game_driver *drv, std::string &buffer, int hm_type);
	void load_command_info(std::string &buffer, const int menu_sel);
	void load_software_info(const char *soft_list, std::string &buffer, const char *soft_name);
	void command_sub_menu(const game_driver *drv, std::vector<std::string> &menu_item);

	std::string rev_history() const { return history_revision; }
	std::string rev_mameinfo() const { return mame_revision; }
	std::string rev_messinfo() const { return mess_revision; }
	std::string rev_sysinfo() const { return sysinfo_revision; }
	std::string rev_storyinfo() const { return story_revision; }

private:

	// global index
	std::vector<tDatafileIndex> hist_idx, mame_idx, mess_idx, cmd_idx, sysi_idx, story_idx;
	std::vector<sDataDrvIndex> drv_idx, drvmess_idx;
	std::vector<tMenuIndex> menu_idx;
	std::vector<SoftwareListIndex> sListIndex;

	// internal helpers
	void init_history();
	void init_mameinfo();
	void init_messinfo();
	void init_command();
	void init_sysinfo();
	void init_storyinfo();

	bool ParseOpen(const char *filename);

	int index_mame_mess_info(std::vector<tDatafileIndex> &index, std::vector<sDataDrvIndex> &index_drv, int &drvcount);
	int index_datafile(std::vector<tDatafileIndex> &index, int &swcount);
	void index_menuidx(const game_driver *drv, std::vector<tDatafileIndex> &d_idx, std::vector<tMenuIndex> &index);

	void load_command_text(std::string &buffer, std::vector<tMenuIndex> &m_idx, const int menu_sel);
	void load_data_text(const game_driver *drv, std::string &buffer, std::vector<tDatafileIndex> &idx, const char *tag);
	void load_driver_text(const game_driver *drv, std::string &buffer, std::vector<sDataDrvIndex> &idx, const char *tag);

	int find_or_allocate(std::string name);

	// internal state
	running_machine  &m_machine;             // reference to our machine
	std::string         fullpath;
	static std::string  history_revision, mame_revision, mess_revision, sysinfo_revision, story_revision;
};


#endif  /* __MEWUI_DATFILE_H__ */
