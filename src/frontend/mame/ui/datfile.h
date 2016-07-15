// license:BSD-3-Clause
// copyright-holders:Maurizio Petrarota
/***************************************************************************

    ui/datfile.h

    UI DATs manager.

***************************************************************************/

#ifndef MAME_FRONTEND_UI_DATFILE_H
#define MAME_FRONTEND_UI_DATFILE_H

#pragma once

#include <string>
#include <unordered_map>

class ui_options;

namespace ui {
//-------------------------------------------------
//  Datafile Manager
//-------------------------------------------------
class datfile_manager
{
public:
	// construction/destruction
	datfile_manager(running_machine &machine, ui_options &moptions);

	// getters
	running_machine &machine() const { return m_machine; }

	// actions
	void load_data_info(const game_driver *drv, std::string &buffer, int type);
	void load_command_info(std::string &buffer, std::string &sel);
	void load_software_info(std::string const &softlist, std::string &buffer, std::string const &softname, std::string const &parentname);
	void command_sub_menu(const game_driver *drv, std::vector<std::string> &menuitems);
	void reset_run() { first_run = true; }

	std::string const &rev_history() const { return m_history_rev; }
	std::string const &rev_mameinfo() const { return m_mame_rev; }
	std::string const &rev_messinfo() const { return m_mess_rev; }
	std::string const &rev_sysinfo() const { return m_sysinfo_rev; }
	std::string const &rev_storyinfo() const { return m_story_rev; }
	std::string const &rev_ginitinfo() const { return m_ginit_rev; }

	bool has_history(game_driver const *driver) const { return m_histidx.find(driver) != m_histidx.end(); }
	bool has_mameinfo(game_driver const *driver) const { return m_mameidx.find(driver) != m_mameidx.end(); }
	bool has_messinfo(game_driver const *driver) const { return m_messidx.find(driver) != m_messidx.end(); }
	bool has_command(game_driver const *driver) const { return m_cmdidx.find(driver) != m_cmdidx.end(); }
	bool has_sysinfo(game_driver const *driver) const { return m_sysidx.find(driver) != m_sysidx.end(); }
	bool has_story(game_driver const *driver) const { return m_storyidx.find(driver) != m_storyidx.end(); }
	bool has_gameinit(game_driver const *driver) const { return m_ginitidx.find(driver) != m_ginitidx.end(); }

	// this isn't really just a getter - it affects some internal state
	bool has_software(std::string const &softlist, std::string const &softname, std::string const &parentname);

	bool has_data(game_driver const *a = nullptr) const
	{
		game_driver const *const d(a ? a : &machine().system());
		return has_history(d) || has_mameinfo(d) || has_messinfo(d) || has_command(d) || has_sysinfo(d) || has_story(d) || has_gameinit(d);
	}

private:
	using drvindex = std::unordered_map<std::string, long>;
	using dataindex = std::unordered_map<const game_driver *, long>;
	using swindex = std::unordered_map<std::string, drvindex>;

	// global index
	static dataindex m_histidx, m_mameidx, m_messidx, m_cmdidx, m_sysidx, m_storyidx, m_ginitidx;
	static drvindex m_drvidx, m_messdrvidx, m_menuidx;
	static swindex m_swindex;

	// internal helpers
	void init_history();
	void init_mameinfo();
	void init_messinfo();
	void init_command();
	void init_sysinfo();
	void init_story();
	void init_gameinit();

	// file open/close/seek
	bool parseopen(const char *filename);
	void parseclose() { if (fp != nullptr) fclose(fp); }

	int index_mame_mess_info(dataindex &index, drvindex &index_drv, int &drvcount);
	int index_datafile(dataindex &index, int &swcount, std::string &rev, std::string const &tag, char sep);
	void index_menuidx(const game_driver *drv, dataindex const &idx, drvindex &index);
	drvindex::const_iterator m_itemsiter;

	void load_data_text(const game_driver *drv, std::string &buffer, dataindex &idx, const std::string &tag);
	void load_driver_text(const game_driver *drv, std::string &buffer, drvindex &idx, const std::string &tag);

	// internal state
	running_machine     &m_machine;             // reference to our machine
	ui_options          &m_options;
	std::string         m_fullpath;
	static std::string  m_history_rev, m_mame_rev, m_mess_rev, m_sysinfo_rev, m_story_rev, m_ginit_rev;
	FILE                *fp = nullptr;
	static bool         first_run;
};

} // namespace ui

#endif  // MAME_FRONTEND_UI_DATFILE_H
