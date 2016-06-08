// license:BSD-3-Clause
// copyright-holders:Maurizio Petrarota
/***************************************************************************

    ui/datfile.cpp

    UI DATs manager.

***************************************************************************/

#include "emu.h"
#include "drivenum.h"
#include "ui/moptions.h"
#include "ui/datfile.h"
#include "ui/utils.h"

//-------------------------------------------------
//  TAGS
//-------------------------------------------------
static std::string DATAFILE_TAG("$");
static std::string TAG_BIO("$bio");
static std::string TAG_INFO("$info");
static std::string TAG_MAME("$mame");
static std::string TAG_COMMAND("$cmd");
static std::string TAG_END("$end");
static std::string TAG_DRIVER("$drv");
static std::string TAG_STORY("$story");
static std::string TAG_HISTORY_R("## REVISION:");
static std::string TAG_MAMEINFO_R("# MAMEINFO.DAT");
static std::string TAG_MESSINFO_R("#     MESSINFO.DAT");
static std::string TAG_SYSINFO_R("# This file was generated on");
static std::string TAG_STORY_R("# version");
static std::string TAG_COMMAND_SEPARATOR("-----------------------------------------------");
static std::string TAG_GAMEINIT_R("# GAMEINIT.DAT");

//-------------------------------------------------
//  Statics
//-------------------------------------------------
datfile_manager::dataindex datfile_manager::m_histidx;
datfile_manager::dataindex datfile_manager::m_mameidx;
datfile_manager::dataindex datfile_manager::m_messidx;
datfile_manager::dataindex datfile_manager::m_cmdidx;
datfile_manager::dataindex datfile_manager::m_sysidx;
datfile_manager::dataindex datfile_manager::m_storyidx;
datfile_manager::dataindex datfile_manager::m_ginitidx;
datfile_manager::drvindex datfile_manager::m_drvidx;
datfile_manager::drvindex datfile_manager::m_messdrvidx;
datfile_manager::drvindex datfile_manager::m_menuidx;
datfile_manager::swindex datfile_manager::m_swindex;
std::string datfile_manager::m_history_rev;
std::string datfile_manager::m_mame_rev;
std::string datfile_manager::m_mess_rev;
std::string datfile_manager::m_sysinfo_rev;
std::string datfile_manager::m_story_rev;
std::string datfile_manager::m_ginit_rev;
bool datfile_manager::first_run = true;

#define opendatsfile(f) if (parseopen(#f".dat")) { init_##f(); parseclose(); }

//-------------------------------------------------
// ctor
//-------------------------------------------------
datfile_manager::datfile_manager(running_machine &machine, ui_options &moptions) 
	: m_machine(machine)
	, m_options(moptions)
{
	if (m_options.enabled_dats() && first_run)
	{
		first_run = false;
		opendatsfile(mameinfo);
		opendatsfile(command);
		opendatsfile(story);
		opendatsfile(messinfo);
		opendatsfile(sysinfo);
		opendatsfile(history);
		opendatsfile(gameinit);
	}
}

//-------------------------------------------------
//  initialize sysinfo.dat index
//-------------------------------------------------
void datfile_manager::init_sysinfo()
{
	int swcount = 0;
	auto count = index_datafile(m_sysidx, swcount);
	osd_printf_verbose("Sysinfo.dat games found = %i\n", count);
	osd_printf_verbose("Rev = %s\n", m_sysinfo_rev.c_str());
}

//-------------------------------------------------
//  initialize story.dat index
//-------------------------------------------------
void datfile_manager::init_story()
{
	int swcount = 0;
	auto count = index_datafile(m_storyidx, swcount);
	osd_printf_verbose("Story.dat games found = %i\n", count);
}

//-------------------------------------------------
//  initialize history.dat index
//-------------------------------------------------
void datfile_manager::init_history()
{
	int swcount = 0;
	auto count = index_datafile(m_histidx, swcount);
	osd_printf_verbose("History.dat systems found = %i\n", count);
	osd_printf_verbose("History.dat software packages found = %i\n", swcount);
	osd_printf_verbose("Rev = %s\n", m_history_rev.c_str());
}

//-------------------------------------------------
//  initialize gameinit.dat index
//-------------------------------------------------
void datfile_manager::init_gameinit()
{
	int swcount = 0;
	drvindex tmp;
	auto count = index_mame_mess_info(m_ginitidx, tmp, swcount);
	osd_printf_verbose("Gameinit.dat games found = %i\n", count);
	osd_printf_verbose("Rev = %s\n", m_ginit_rev.c_str());
}

//-------------------------------------------------
//  initialize mameinfo.dat index
//-------------------------------------------------
void datfile_manager::init_mameinfo()
{
	int drvcount = 0;
	auto count = index_mame_mess_info(m_mameidx, m_drvidx, drvcount);
	osd_printf_verbose("Mameinfo.dat games found = %i\n", count);
	osd_printf_verbose("Mameinfo.dat drivers found = %d\n", drvcount);
	osd_printf_verbose("Rev = %s\n", m_mame_rev.c_str());
}

//-------------------------------------------------
//  initialize messinfo.dat index
//-------------------------------------------------
void datfile_manager::init_messinfo()
{
	int drvcount = 0;
	auto count = index_mame_mess_info(m_messidx, m_messdrvidx, drvcount);
	osd_printf_verbose("Messinfo.dat games found = %i\n", count);
	osd_printf_verbose("Messinfo.dat drivers found = %d\n", drvcount);
	osd_printf_verbose("Rev = %s\n", m_mess_rev.c_str());
}

//-------------------------------------------------
//  initialize command.dat index
//-------------------------------------------------
void datfile_manager::init_command()
{
	int swcount = 0;
	auto count = index_datafile(m_cmdidx, swcount);
	osd_printf_verbose("Command.dat games found = %i\n", count);
}

bool datfile_manager::has_software(std::string &softlist, std::string &softname, std::string &parentname)
{
	// Find software in software list index
	if (m_swindex.find(softlist) == m_swindex.end())
		return false;

	m_itemsiter = m_swindex[softlist].find(softname);
	if (m_itemsiter == m_swindex[softlist].end() && !parentname.empty())
		m_itemsiter = m_swindex[softlist].find(parentname);

	if (m_itemsiter == m_swindex[softlist].end())
		return false;

	return true;
}
//-------------------------------------------------
//  load software info
//-------------------------------------------------
void datfile_manager::load_software_info(std::string &softlist, std::string &buffer, std::string &softname, std::string &parentname)
{
	// Load history text
	if (!m_swindex.empty() && parseopen("history.dat"))
	{
		// Find software in software list index
		if (!has_software(softlist, softname, parentname))
			return;

		auto s_offset = m_itemsiter->second;
		char rbuf[64 * 1024];
		fseek(fp, s_offset, SEEK_SET);
		std::string readbuf;
		while (fgets(rbuf, 64 * 1024, fp) != nullptr)
		{
			readbuf = chartrimcarriage(rbuf);

			// end entry when a end tag is encountered
			if (readbuf == TAG_END)
				break;

			// add this string to the buffer
			buffer.append(readbuf).append("\n");
		}
		parseclose();
	}
}

//-------------------------------------------------
//  load_data_info
//-------------------------------------------------
void datfile_manager::load_data_info(const game_driver *drv, std::string &buffer, int type)
{
	dataindex index_idx;
	drvindex driver_idx;
	std::string tag;
	std::string filename;

	switch (type)
	{
		case UI_HISTORY_LOAD:
			filename = "history.dat";
			tag = TAG_BIO;
			index_idx = m_histidx;
			break;
		case UI_MAMEINFO_LOAD:
			filename = "mameinfo.dat";
			tag = TAG_MAME;
			index_idx = m_mameidx;
			driver_idx = m_drvidx;
			break;
		case UI_SYSINFO_LOAD:
			filename = "sysinfo.dat";
			tag = TAG_BIO;
			index_idx = m_sysidx;
			break;
		case UI_MESSINFO_LOAD:
			filename = "messinfo.dat";
			tag = TAG_MAME;
			index_idx = m_messidx;
			driver_idx = m_messdrvidx;
			break;
		case UI_STORY_LOAD:
			filename = "story.dat";
			tag = TAG_STORY;
			index_idx = m_storyidx;
			break;
		case UI_GINIT_LOAD:
			filename = "gameinit.dat";
			tag = TAG_MAME;
			index_idx = m_ginitidx;
			break;
	}

	if (parseopen(filename.c_str()))
	{
		load_data_text(drv, buffer, index_idx, tag);

		// load driver info
		if (!driver_idx.empty())
			load_driver_text(drv, buffer, driver_idx, TAG_DRIVER);

		// cleanup mameinfo and sysinfo double line spacing
		if ((tag == TAG_MAME && type != UI_GINIT_LOAD) || type == UI_SYSINFO_LOAD)
			strreplace(buffer, "\n\n", "\n");

		parseclose();
	}
}

//-------------------------------------------------
//  load a game text into the buffer
//-------------------------------------------------
void datfile_manager::load_data_text(const game_driver *drv, std::string &buffer, dataindex &idx, std::string &tag)
{
	dataindex::iterator itemsiter = idx.find(drv);
	if (itemsiter == idx.end())
	{
		auto cloneof = driver_list::non_bios_clone(*drv);
		if (cloneof == -1)
			return;
		else
		{
			auto c_drv = &driver_list::driver(cloneof);
			itemsiter = idx.find(c_drv);
			if (itemsiter == idx.end())
				return;
		}
	}

	auto s_offset = itemsiter->second;
	fseek(fp, s_offset, SEEK_SET);
	char rbuf[64 * 1024];
	std::string readbuf;
	while (fgets(rbuf, 64 * 1024, fp) != nullptr)
	{
		readbuf = chartrimcarriage(rbuf);

		// end entry when a end tag is encountered
		if (readbuf == TAG_END)
			break;

		// continue if a specific tag is encountered
		if (readbuf == tag)
			continue;

		// add this string to the buffer
		buffer.append(readbuf).append("\n");
	}
}

//-------------------------------------------------
//  load a driver name and offset into an
//  indexed array
//-------------------------------------------------
void datfile_manager::load_driver_text(const game_driver *drv, std::string &buffer, drvindex &idx, std::string &tag)
{
	std::string s(core_filename_extract_base(drv->source_file));
	drvindex::const_iterator index = idx.find(s);

	// if driver not found, return
	if (index == idx.end())
		return;

	buffer.append("\n--- DRIVER INFO ---\n").append("Driver: ").append(s).append("\n\n");
	auto s_offset = index->second;
	fseek(fp, s_offset, SEEK_SET);
	char rbuf[64 * 1024];
	std::string readbuf;
	while (fgets(rbuf, 64 * 1024, fp) != nullptr)
	{
		readbuf = chartrimcarriage(rbuf);

		// end entry when a end tag is encountered
		if (readbuf == TAG_END)
			break;

		// continue if a specific tag is encountered
		if (readbuf == tag)
			continue;

		// add this string to the buffer
		buffer.append(readbuf).append("\n");
	}
}

//-------------------------------------------------
//  load a game name and offset into an
//  indexed array (mameinfo)
//-------------------------------------------------
int datfile_manager::index_mame_mess_info(dataindex &index, drvindex &index_drv, int &drvcount)
{
	std::string name;
	size_t foundtag;
	auto t_mame = TAG_MAMEINFO_R.size();
	auto t_mess = TAG_MESSINFO_R.size();
	auto t_ginit = TAG_GAMEINIT_R.size();
	auto t_info = TAG_INFO.size();

	char rbuf[64 * 1024];
	std::string readbuf, xid;
	while (fgets(rbuf, 64 * 1024, fp) != nullptr)
	{
		readbuf = chartrimcarriage(rbuf);
		if (m_mame_rev.empty() && readbuf.compare(0, t_mame, TAG_MAMEINFO_R) == 0)
		{
			auto found = readbuf.find(" ", t_mame + 1);
			m_mame_rev = readbuf.substr(t_mame + 1, found - t_mame);
		}
		else if (m_mess_rev.empty() && (foundtag = readbuf.find(TAG_MESSINFO_R)) != std::string::npos)
		{
			auto found = readbuf.find(" ", foundtag + t_mess + 1);
			m_mess_rev = readbuf.substr(foundtag + t_mess + 1, found - t_mess - foundtag);
		}
		else if (m_ginit_rev.empty() && readbuf.compare(0, t_ginit, TAG_GAMEINIT_R) == 0)
		{
			auto found = readbuf.find(" ", t_ginit + 1);
			m_ginit_rev = readbuf.substr(t_ginit + 1, found - t_ginit);
		}
		else if (readbuf.compare(0, t_info, TAG_INFO) == 0)
		{
			// TAG_INFO
			fgets(rbuf, 64 * 1024, fp);
			xid = chartrimcarriage(rbuf);
			name = readbuf.substr(t_info + 1);
			if (xid == TAG_MAME)
			{
				// validate driver
				auto game_index = driver_list::find(name.c_str());
				if (game_index != -1)
					index.emplace(&driver_list::driver(game_index), ftell(fp));
			}
			else if (xid == TAG_DRIVER)
			{
				index_drv.emplace(name, ftell(fp));
				drvcount++;
			}
		}
	}
	return index.size();
}

//-------------------------------------------------
//  load a game name and offset into an
//  indexed array
//-------------------------------------------------
int datfile_manager::index_datafile(dataindex &index, int &swcount)
{
	std::string readbuf;
	auto t_hist = TAG_HISTORY_R.size();
	auto t_story = TAG_STORY_R.size();
	auto t_sysinfo = TAG_SYSINFO_R.size();
	auto t_info = TAG_INFO.size();
	auto t_bio = TAG_BIO.size();
	char rbuf[64 * 1024];
	while (fgets(rbuf, 64 * 1024, fp) != nullptr)
	{
		readbuf = chartrimcarriage(rbuf);

		if (m_history_rev.empty() && readbuf.compare(0, t_hist, TAG_HISTORY_R) == 0)
		{
			auto found = readbuf.find(" ", t_hist + 1);
			m_history_rev = readbuf.substr(t_hist + 1, found - t_hist);
		}
		else if (m_sysinfo_rev.empty() && readbuf.compare(0, t_sysinfo, TAG_SYSINFO_R) == 0)
		{
			auto found = readbuf.find(".", t_sysinfo + 1);
			m_sysinfo_rev = readbuf.substr(t_sysinfo + 1, found - t_sysinfo);
		}
		else if (m_story_rev.empty() && readbuf.compare(0, t_story, TAG_STORY_R) == 0)
			m_story_rev = readbuf.substr(t_story + 1);
		else if (readbuf.compare(0, t_info, TAG_INFO) == 0)
		{
			// search for game info
			auto rd = readbuf.substr(t_info + 1);
			std::vector<std::string> gamelist = tokenize(rd, ',');
			for (auto & e : gamelist)
			{
				auto game_index = driver_list::find(e.c_str());
				if (game_index != -1)
					index.emplace(&driver_list::driver(game_index), ftell(fp));
			}
		}
		else if (!readbuf.empty() && readbuf[0] == DATAFILE_TAG[0])
		{
			// search for software info
			fgets(rbuf, 64 * 1024, fp);
			std::string readbuf_2(chartrimcarriage(rbuf));
			if (readbuf_2.compare(0, t_bio, TAG_BIO) == 0)
			{
				auto eq_sign = readbuf.find('=');
				std::string s_list(readbuf.substr(1, eq_sign - 1));
				std::string s_roms(readbuf.substr(eq_sign + 1));
				std::vector<std::string> token_list = tokenize(s_list, ',');
				std::vector<std::string> token_roms = tokenize(s_roms, ',');
				for (auto & li : token_list)
					for (auto & ro : token_roms)
						m_swindex[li].emplace(ro, ftell(fp));
			}
		}
	}
	return index.size();
}

//---------------------------------------------------------
//  parseopen - Open up file for reading
//---------------------------------------------------------
bool datfile_manager::parseopen(const char *filename)
{
	emu_file file(m_options.history_path(), OPEN_FLAG_READ);
	if (file.open(filename) != osd_file::error::NONE)
		return false;

	m_fullpath = file.fullpath();
	file.close();
	fp = fopen(m_fullpath.c_str(), "rb");

	fgetc(fp);
	fseek(fp, 0, SEEK_SET);
	return true;
}

//-------------------------------------------------
//  create the menu index
//-------------------------------------------------
void datfile_manager::index_menuidx(const game_driver *drv, dataindex &idx, drvindex &index)
{
	dataindex::iterator itemsiter = idx.find(drv);
	if (itemsiter == idx.end())
	{
		auto cloneof = driver_list::non_bios_clone(*drv);
		if (cloneof == -1)
			return;
		else
		{
			auto c_drv = &driver_list::driver(cloneof);
			if ((itemsiter = idx.find(c_drv)) == idx.end())
				return;
		}
	}

	// seek to correct point in datafile
	auto s_offset = itemsiter->second;
	fseek(fp, s_offset, SEEK_SET);
	auto tinfo = TAG_INFO.size();
	char rbuf[64 * 1024];
	std::string readbuf;
	while (fgets(rbuf, 64 * 1024, fp) != nullptr)
	{
		readbuf = chartrimcarriage(rbuf);

		if (!core_strnicmp(TAG_INFO.c_str(), readbuf.c_str(), tinfo))
			break;

		// TAG_COMMAND identifies the driver
		if (readbuf == TAG_COMMAND)
		{
			fgets(rbuf, 64 * 1024, fp);
			chartrimcarriage(rbuf);
			index.emplace(rbuf, ftell(fp));
		}
	}
}

//-------------------------------------------------
//  load command text into the buffer
//-------------------------------------------------
void datfile_manager::load_command_info(std::string &buffer, std::string &sel)
{
	if (parseopen("command.dat"))
	{
		// open and seek to correct point in datafile
		auto offset = m_menuidx.at(sel);
		fseek(fp, offset, SEEK_SET);
		char rbuf[64 * 1024];
		std::string readbuf;
		while (fgets(rbuf, 64 * 1024, fp) != nullptr)
		{
			readbuf = chartrimcarriage(rbuf);

			// skip separator lines
			if (readbuf == TAG_COMMAND_SEPARATOR)
				continue;

			// end entry when a tag is encountered
			if (readbuf == TAG_END)
				break;

			// add this string to the buffer
			buffer.append(readbuf).append("\n");;
		}
		parseclose();
	}
}

//-------------------------------------------------
//  load submenu item for command.dat
//-------------------------------------------------
void datfile_manager::command_sub_menu(const game_driver *drv, std::vector<std::string> &menuitems)
{
	if (parseopen("command.dat"))
	{
		m_menuidx.clear();
		index_menuidx(drv, m_cmdidx, m_menuidx);
		for (auto & elem : m_menuidx)
			menuitems.push_back(elem.first);
		parseclose();
	}
}
