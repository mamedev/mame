// license:BSD-3-Clause
// copyright-holders:Maurizio Petrarota
/***************************************************************************

    ui/datfile.cpp

    UI DATs manager.

***************************************************************************/

#include "emu.h"
#include "ui/datfile.h"

#include "drivenum.h"
#include "ui/moptions.h"
#include "ui/utils.h"

#include <utility>


namespace ui {
namespace {
//-------------------------------------------------
//  TAGS
//-------------------------------------------------
std::string const DATAFILE_TAG("$");
std::string const TAG_BIO("$bio");
std::string const TAG_INFO("$info");
std::string const TAG_MAME("$mame");
std::string const TAG_COMMAND("$cmd");
std::string const TAG_END("$end");
std::string const TAG_DRIVER("$drv");
std::string const TAG_STORY("$story");
std::string const TAG_HISTORY_R("## REVISION:");
std::string const TAG_MAMEINFO_R("# MAMEINFO.DAT");
std::string const TAG_MESSINFO_R("#     MESSINFO.DAT");
std::string const TAG_SYSINFO_R("# This file was generated on");
std::string const TAG_STORY_R("# version");
std::string const TAG_COMMAND_SEPARATOR("-----------------------------------------------");
std::string const TAG_GAMEINIT_R("# GAMEINIT.DAT");

} // anonymous namespace

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

#define opendatsfile(f) do { fileptr datfile = parseopen(#f".dat"); if (datfile) init_##f(std::move(datfile)); } while (false)

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
void datfile_manager::init_sysinfo(fileptr &&fp)
{
	int swcount = 0;
	auto count = index_datafile(std::move(fp), m_sysidx, swcount, m_sysinfo_rev, TAG_SYSINFO_R, '.');
	osd_printf_verbose("Sysinfo.dat games found = %i\n", count);
	osd_printf_verbose("Rev = %s\n", m_sysinfo_rev.c_str());
}

//-------------------------------------------------
//  initialize story.dat index
//-------------------------------------------------
void datfile_manager::init_story(fileptr &&fp)
{
	int swcount = 0;
	auto count = index_datafile(std::move(fp), m_storyidx, swcount, m_story_rev, TAG_STORY_R, 's');
	osd_printf_verbose("Story.dat games found = %i\n", count);
}

//-------------------------------------------------
//  initialize history.dat index
//-------------------------------------------------
void datfile_manager::init_history(fileptr &&fp)
{
	int swcount = 0;
	auto count = index_datafile(std::move(fp), m_histidx, swcount, m_history_rev, TAG_HISTORY_R, ' ');
	osd_printf_verbose("History.dat systems found = %i\n", count);
	osd_printf_verbose("History.dat software packages found = %i\n", swcount);
	osd_printf_verbose("Rev = %s\n", m_history_rev.c_str());
}

//-------------------------------------------------
//  initialize gameinit.dat index
//-------------------------------------------------
void datfile_manager::init_gameinit(fileptr &&fp)
{
	int swcount = 0;
	drvindex tmp;
	auto count = index_mame_mess_info(std::move(fp), m_ginitidx, tmp, swcount);
	osd_printf_verbose("Gameinit.dat games found = %i\n", count);
	osd_printf_verbose("Rev = %s\n", m_ginit_rev.c_str());
}

//-------------------------------------------------
//  initialize mameinfo.dat index
//-------------------------------------------------
void datfile_manager::init_mameinfo(fileptr &&fp)
{
	int drvcount = 0;
	auto count = index_mame_mess_info(std::move(fp), m_mameidx, m_drvidx, drvcount);
	osd_printf_verbose("Mameinfo.dat games found = %i\n", count);
	osd_printf_verbose("Mameinfo.dat drivers found = %d\n", drvcount);
	osd_printf_verbose("Rev = %s\n", m_mame_rev.c_str());
}

//-------------------------------------------------
//  initialize messinfo.dat index
//-------------------------------------------------
void datfile_manager::init_messinfo(fileptr &&fp)
{
	int drvcount = 0;
	auto count = index_mame_mess_info(std::move(fp), m_messidx, m_messdrvidx, drvcount);
	osd_printf_verbose("Messinfo.dat games found = %i\n", count);
	osd_printf_verbose("Messinfo.dat drivers found = %d\n", drvcount);
	osd_printf_verbose("Rev = %s\n", m_mess_rev.c_str());
}

//-------------------------------------------------
//  initialize command.dat index
//-------------------------------------------------
void datfile_manager::init_command(fileptr &&fp)
{
	int swcount = 0;
	std::string tmp;
	auto count = index_datafile(std::move(fp), m_cmdidx, swcount, tmp, std::string(), 'c');
	osd_printf_verbose("Command.dat games found = %i\n", count);
}

bool datfile_manager::has_software(std::string const &softlist, std::string const &softname, std::string const &parentname) const
{
	return bool(find_software(softlist, softname, parentname));
}

long const *datfile_manager::find_software(std::string const &softlist, std::string const &softname, std::string const &parentname) const
{
	// Find software in software list index
	auto const software(m_swindex.find(softlist));
	if (software == m_swindex.end())
		return nullptr;

	auto itemsiter = software->second.find(softname);
	if ((itemsiter == software->second.end()) && !parentname.empty())
		itemsiter = software->second.find(parentname);

	return (itemsiter != software->second.end()) ? &itemsiter->second : nullptr;
}

//-------------------------------------------------
//  load software info
//-------------------------------------------------
void datfile_manager::load_software_info(std::string const &softlist, std::string &buffer, std::string const &softname, std::string const &parentname)
{
	if (m_swindex.empty())
		return;

	// Load history text
	fileptr const datfile = parseopen("history.dat");
	if (datfile)
	{
		// Find software in software list index
		long const *const s_offset = find_software(softlist, softname, parentname);
		if (!s_offset)
			return;

		char rbuf[64 * 1024];
		std::fseek(datfile.get(), *s_offset, SEEK_SET);
		std::string readbuf;
		while (std::fgets(rbuf, 64 * 1024, datfile.get()) != nullptr)
		{
			readbuf = chartrimcarriage(rbuf);

			// end entry when a end tag is encountered
			if (readbuf == TAG_END)
				break;

			// add this string to the buffer
			buffer.append(readbuf).append("\n");
		}
	}
}

//-------------------------------------------------
//  load_data_info
//-------------------------------------------------
void datfile_manager::load_data_info(const game_driver *drv, std::string &buffer, int type)
{
	dataindex const *index_idx = nullptr;
	drvindex const *driver_idx = nullptr;
	std::string const *tag;
	std::string filename;

	switch (type)
	{
	case UI_HISTORY_LOAD:
		filename = "history.dat";
		tag = &TAG_BIO;
		index_idx = &m_histidx;
		break;
	case UI_MAMEINFO_LOAD:
		filename = "mameinfo.dat";
		tag = &TAG_MAME;
		index_idx = &m_mameidx;
		driver_idx = &m_drvidx;
		break;
	case UI_SYSINFO_LOAD:
		filename = "sysinfo.dat";
		tag = &TAG_BIO;
		index_idx = &m_sysidx;
		break;
	case UI_MESSINFO_LOAD:
		filename = "messinfo.dat";
		tag = &TAG_MAME;
		index_idx = &m_messidx;
		driver_idx = &m_messdrvidx;
		break;
	case UI_STORY_LOAD:
		filename = "story.dat";
		tag = &TAG_STORY;
		index_idx = &m_storyidx;
		break;
	case UI_GINIT_LOAD:
		filename = "gameinit.dat";
		tag = &TAG_MAME;
		index_idx = &m_ginitidx;
		break;
	default:
		assert(false);
		return;
	}

	fileptr const datfile = parseopen(filename.c_str());
	if (datfile)
	{
		load_data_text(datfile.get(), drv, buffer, *index_idx, *tag);

		// load driver info
		if (driver_idx && !driver_idx->empty())
			load_driver_text(datfile.get(), drv, buffer, *driver_idx, TAG_DRIVER);

		// cleanup mameinfo and sysinfo double line spacing
		if (((*tag == TAG_MAME) && (type != UI_GINIT_LOAD)) || (type == UI_SYSINFO_LOAD))
			strreplace(buffer, "\n\n", "\n");
	}
}

//-------------------------------------------------
//  load a game text into the buffer
//-------------------------------------------------
void datfile_manager::load_data_text(FILE *fp, game_driver const *drv, std::string &buffer, dataindex const &idx, std::string const &tag)
{
	auto itemsiter = idx.find(drv);
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
	std::fseek(fp, s_offset, SEEK_SET);
	char rbuf[64 * 1024];
	std::string readbuf;
	while (std::fgets(rbuf, 64 * 1024, fp) != nullptr)
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
void datfile_manager::load_driver_text(FILE *fp, game_driver const *drv, std::string &buffer, drvindex const &idx, std::string const &tag)
{
	std::string s(core_filename_extract_base(drv->source_file));
	auto index = idx.find(s);

	// if driver not found, return
	if (index == idx.end())
		return;

	buffer.append("\n--- DRIVER INFO ---\n").append("Driver: ").append(s).append("\n\n");
	auto s_offset = index->second;
	std::fseek(fp, s_offset, SEEK_SET);
	char rbuf[64 * 1024];
	std::string readbuf;
	while (std::fgets(rbuf, 64 * 1024, fp) != nullptr)
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
int datfile_manager::index_mame_mess_info(fileptr &&fp, dataindex &index, drvindex &index_drv, int &drvcount)
{
	size_t foundtag;
	auto t_mame = TAG_MAMEINFO_R.size();
	auto t_mess = TAG_MESSINFO_R.size();
	auto t_ginit = TAG_GAMEINIT_R.size();
	auto t_info = TAG_INFO.size();

	char rbuf[64 * 1024];
	std::string readbuf, xid, name;
	while (std::fgets(rbuf, 64 * 1024, fp.get()) != nullptr)
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
			std::fgets(rbuf, 64 * 1024, fp.get());
			xid = chartrimcarriage(rbuf);
			name = readbuf.substr(t_info + 1);
			if (xid == TAG_MAME)
			{
				// validate driver
				auto game_index = driver_list::find(name.c_str());
				if (game_index != -1)
					index.emplace(&driver_list::driver(game_index), std::ftell(fp.get()));
			}
			else if (xid == TAG_DRIVER)
			{
				index_drv.emplace(name, std::ftell(fp.get()));
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
int datfile_manager::index_datafile(fileptr &&fp, dataindex &index, int &swcount, std::string &rev, std::string const &tag, char sep)
{
	std::string readbuf;
	auto const tag_size = tag.size();
	auto const t_info = TAG_INFO.size();
	auto const t_bio = TAG_BIO.size();
	char rbuf[64 * 1024];
	while (std::fgets(rbuf, 64 * 1024, fp.get()) != nullptr)
	{
		readbuf = chartrimcarriage(rbuf);

		if (!tag.empty())
		{
			if (rev.empty() && readbuf.compare(0, tag_size, tag) == 0)
			{
				if (sep != 's')
					rev = readbuf.substr(tag_size + 1, readbuf.find(sep, tag_size + 1) - tag_size);
				else
					rev = readbuf.substr(tag_size + 1);
			}
		}

		if (readbuf.compare(0, t_info, TAG_INFO) == 0)
		{
			// search for game info
			auto rd = readbuf.substr(t_info + 1);
			std::vector<std::string> gamelist = tokenize(rd, ',');
			for (auto & e : gamelist)
			{
				auto game_index = driver_list::find(e.c_str());
				if (game_index != -1)
					index.emplace(&driver_list::driver(game_index), std::ftell(fp.get()));
			}
		}
		else if (!readbuf.empty() && readbuf[0] == DATAFILE_TAG[0])
		{
			// search for software info
			std::fgets(rbuf, 64 * 1024, fp.get());
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
						m_swindex[li].emplace(ro, std::ftell(fp.get()));
				swcount++;
			}
		}
	}
	return index.size();
}

//---------------------------------------------------------
//  parseopen - Open up file for reading
//---------------------------------------------------------
datfile_manager::fileptr datfile_manager::parseopen(const char *filename)
{
	emu_file file(m_options.history_path(), OPEN_FLAG_READ);
	if (file.open(filename) != osd_file::error::NONE)
		return fileptr(nullptr, &std::fclose);

	std::string const fullpath = file.fullpath();
	file.close();
	fileptr result(std::fopen(fullpath.c_str(), "rb"), &std::fclose);

	fgetc(result.get());
	fseek(result.get(), 0, SEEK_SET);
	return result;
}

//-------------------------------------------------
//  create the menu index
//-------------------------------------------------
void datfile_manager::index_menuidx(fileptr &&fp, const game_driver *drv, dataindex const &idx, drvindex &index)
{
	auto itemsiter = idx.find(drv);
	if (itemsiter == idx.end())
	{
		auto const cloneof = driver_list::non_bios_clone(*drv);
		if (cloneof == -1)
			return;

		auto const c_drv = &driver_list::driver(cloneof);
		if ((itemsiter = idx.find(c_drv)) == idx.end())
			return;
	}

	// seek to correct point in datafile
	auto const s_offset = itemsiter->second;
	std::fseek(fp.get(), s_offset, SEEK_SET);
	auto const tinfo = TAG_INFO.size();
	char rbuf[64 * 1024];
	std::string readbuf;
	while (std::fgets(rbuf, 64 * 1024, fp.get()) != nullptr)
	{
		readbuf = chartrimcarriage(rbuf);

		if (!core_strnicmp(TAG_INFO.c_str(), readbuf.c_str(), tinfo))
			break;

		// TAG_COMMAND identifies the driver
		if (readbuf == TAG_COMMAND)
		{
			std::fgets(rbuf, 64 * 1024, fp.get());
			chartrimcarriage(rbuf);
			index.emplace(rbuf, std::ftell(fp.get()));
		}
	}
}

//-------------------------------------------------
//  load command text into the buffer
//-------------------------------------------------
void datfile_manager::load_command_info(std::string &buffer, std::string const &sel)
{
	fileptr const datfile = parseopen("command.dat");
	if (datfile)
	{
		// open and seek to correct point in datafile
		auto const offset = m_menuidx.at(sel);
		std::fseek(datfile.get(), offset, SEEK_SET);
		char rbuf[64 * 1024];
		std::string readbuf;
		while (std::fgets(rbuf, 64 * 1024, datfile.get()) != nullptr)
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
	}
}

//-------------------------------------------------
//  load submenu item for command.dat
//-------------------------------------------------
void datfile_manager::command_sub_menu(const game_driver *drv, std::vector<std::string> &menuitems)
{
	fileptr datfile = parseopen("command.dat");
	if (datfile)
	{
		m_menuidx.clear();
		index_menuidx(std::move(datfile), drv, m_cmdidx, m_menuidx);
		menuitems.reserve(m_menuidx.size());
		for (auto const &elem : m_menuidx)
			menuitems.push_back(elem.first);
	}
}

} // namespace ui
