// license:BSD-3-Clause
// copyright-holders:Dankan1890
/***************************************************************************

    mewui/datfile.h

    MEWUI DATs manager.

***************************************************************************/

#pragma once

#ifndef __MEWUI_DATFILE_H__
#define __MEWUI_DATFILE_H__

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
	void load_data_info(const game_driver *drv, std::string &buffer, int type);
	void load_command_info(std::string &buffer, std::string &sel);
	void load_software_info(std::string &softlist, std::string &buffer, std::string &softname, std::string &parentname);
	void command_sub_menu(const game_driver *drv, std::vector<std::string> &menuitems);
	void reset_run() { first_run = true; }

	std::string rev_history() const { return m_history_rev; }
	std::string rev_mameinfo() const { return m_mame_rev; }
	std::string rev_messinfo() const { return m_mess_rev; }
	std::string rev_sysinfo() const { return m_sysinfo_rev; }
	std::string rev_storyinfo() const { return m_story_rev; }

private:
	using drvindex = std::unordered_map<std::string, long>;
	using dataindex = std::unordered_map<const game_driver *, long>;
	using swindex = std::unordered_map<std::string, drvindex>;

	// global index
	static dataindex m_histidx, m_mameidx, m_messidx, m_cmdidx, m_sysidx, m_storyidx;
	static drvindex m_drvidx, m_messdrvidx, m_menuidx;
	static swindex m_swindex;

	// internal helpers
	void init_history();
	void init_mameinfo();
	void init_messinfo();
	void init_command();
	void init_sysinfo();
	void init_storyinfo();

	// file open/close/seek
	bool parseopen(const char *filename);
	void parseclose() { if (fp != nullptr) fclose(fp); }

	int index_mame_mess_info(dataindex &index, drvindex &index_drv, int &drvcount);
	int index_datafile(dataindex &index, int &swcount);
	void index_menuidx(const game_driver *drv, dataindex &idx, drvindex &index);

	void load_data_text(const game_driver *drv, std::string &buffer, dataindex &idx, std::string &tag);
	void load_driver_text(const game_driver *drv, std::string &buffer, drvindex &idx, std::string &tag);

	// internal state
	running_machine     &m_machine;             // reference to our machine
	std::string         m_fullpath;
	static std::string  m_history_rev, m_mame_rev, m_mess_rev, m_sysinfo_rev, m_story_rev;
	FILE				*fp = nullptr;
	static bool			first_run;
};


#endif  /* __MEWUI_DATFILE_H__ */
