// license:BSD-3-Clause
// copyright-holders:Maurizio Petrarota
/***************************************************************************

    ui/inifile.cpp

    UI INIs file manager.

***************************************************************************/

#include "emu.h"
#include "ui/moptions.h"
#include "ui/inifile.h"
#include "softlist.h"
#include "drivenum.h"

//-------------------------------------------------
//  GLOBAL VARIABLES
//-------------------------------------------------
UINT16 inifile_manager::c_cat = 0;
UINT16 inifile_manager::c_file = 0;

//-------------------------------------------------
//  ctor
//-------------------------------------------------

inifile_manager::inifile_manager(running_machine &machine, ui_options &moptions)
	: m_machine(machine)
	, m_options(moptions)
{
	ini_index.clear();
	directory_scan();
}

//-------------------------------------------------
//  scan directories and create index
//-------------------------------------------------

void inifile_manager::directory_scan()
{
	file_enumerator path(m_options.extraini_path());
	const osd::directory::entry *dir;

	while ((dir = path.next()) != nullptr)
		if (core_filename_ends_with(dir->name, ".ini") && parseopen(dir->name))
		{
			init_category(std::string(dir->name));
			parseclose();
		}

	// sort
	std::stable_sort(ini_index.begin(), ini_index.end());
}

//-------------------------------------------------
//  initialize category
//-------------------------------------------------

void inifile_manager::init_category(std::string filename)
{
	categoryindex index;
	char rbuf[MAX_CHAR_INFO];
	std::string readbuf;
	while (fgets(rbuf, MAX_CHAR_INFO, fp) != nullptr)
		if (rbuf[0] == '[')
		{
			readbuf = rbuf;
			auto name = readbuf.substr(1, readbuf.find("]") - 1);
			if (name == "FOLDER_SETTINGS") continue;
			index.emplace_back(name, ftell(fp));
		}

	// sort
	std::stable_sort(index.begin(), index.end());

	if (!index.empty())
		ini_index.emplace_back(strmakelower(filename), index);
}

//-------------------------------------------------
//  load and indexing ini files
//-------------------------------------------------

void inifile_manager::load_ini_category(std::vector<int> &temp_filter)
{
	if (ini_index.empty())
		return;

	auto search_clones = false;
	std::string filename(ini_index[c_file].first);
	auto offset = ini_index[c_file].second[c_cat].second;

	if (filename == "category.ini" || filename == "alltime.ini")
		search_clones = true;

	if (parseopen(filename.c_str()))
	{
		fseek(fp, offset, SEEK_SET);
		char rbuf[MAX_CHAR_INFO];
		std::string readbuf;
		while (fgets(rbuf, MAX_CHAR_INFO, fp) != nullptr)
		{
			readbuf = chartrimcarriage(rbuf);

			if (readbuf.empty() || readbuf[0] == '[')
				break;

			auto dfind = driver_list::find(readbuf.c_str());
			if (dfind != -1)
			{
				temp_filter.push_back(dfind);
				if (search_clones && driver_list::non_bios_clone(dfind) == -1)
					for (int x = 0; x < driver_list::total(); x++)
						if (readbuf == driver_list::driver(x).parent && readbuf != driver_list::driver(x).name)
							temp_filter.push_back(x);
			}
		}
		parseclose();
	}
}

//---------------------------------------------------------
//  parseopen - Open up file for reading
//---------------------------------------------------------

bool inifile_manager::parseopen(const char *filename)
{
	emu_file file(m_options.extraini_path(), OPEN_FLAG_READ);
	if (file.open(filename) != osd_file::error::NONE)
		return false;

	m_fullpath = file.fullpath();
	file.close();
	fp = fopen(m_fullpath.c_str(), "r");

	fgetc(fp);
	fseek(fp, 0, SEEK_SET);
	return true;
}

/**************************************************************************
    FAVORITE MANAGER
**************************************************************************/

//-------------------------------------------------
//  ctor
//-------------------------------------------------

favorite_manager::favorite_manager(running_machine &machine, ui_options &moptions)
	: m_machine(machine)
	, m_options(moptions)
{
	m_current = -1;
	parse_favorite();
}

//-------------------------------------------------
//  add a game
//-------------------------------------------------

void favorite_manager::add_favorite_game(const game_driver *driver)
{
	m_list.emplace_back(driver->name, driver->description, "", "", "", 0, "", driver, "", "", "", 1, "", "", "", true);
	save_favorite_games();
}

//-------------------------------------------------
//  add a system
//-------------------------------------------------

void favorite_manager::add_favorite_game(ui_software_info &swinfo)
{
	m_list.push_back(swinfo);
	save_favorite_games();
}

//-------------------------------------------------
//  add a game / system
//-------------------------------------------------

void favorite_manager::add_favorite_game()
{
	if ((machine().system().flags & MACHINE_TYPE_ARCADE) != 0)
	{
		add_favorite_game(&machine().system());
		return;
	}

	auto software_avail = false;
	for (device_image_interface &image : image_interface_iterator(machine().root_device()))
	{
		if (image.exists() && image.software_entry())
		{
			auto swinfo = image.software_entry();
			auto part = image.part_entry();
			ui_software_info tmpmatches;
			tmpmatches.shortname = swinfo->shortname();
			tmpmatches.longname = strensure(image.longname());
			tmpmatches.parentname = swinfo->parentname();
			tmpmatches.year = strensure(image.year());
			tmpmatches.publisher = strensure(image.manufacturer());
			tmpmatches.supported = image.supported();
			tmpmatches.part = part->name();
			tmpmatches.driver = &machine().system();
			tmpmatches.listname = strensure(image.software_list_name());
			tmpmatches.interface = part->interface();
			tmpmatches.instance = strensure(image.instance_name());
			tmpmatches.startempty = 0;
			tmpmatches.parentlongname.clear();
			if (!swinfo->parentname().empty())
			{
				auto swlist = software_list_device::find_by_name(machine().config(), image.software_list_name());
				for (const util::software_info &c_swinfo : swlist->get_info())
				{
					std::string c_parent(c_swinfo.parentname());
					if (!c_parent.empty() && c_parent == swinfo->shortname())
						{
							tmpmatches.parentlongname = c_swinfo.longname();
							break;
						}
				}
			}

			tmpmatches.usage.clear();
			for (const util::feature_list_item &flist : swinfo->other_info())
				if (!strcmp(flist.name().c_str(), "usage"))
					tmpmatches.usage = flist.value();

			tmpmatches.devicetype = strensure(image.image_type_name());
			tmpmatches.available = true;
			software_avail = true;
			m_list.push_back(tmpmatches);
			save_favorite_games();
		}
	}

	if (!software_avail)
		add_favorite_game(&machine().system());
}

//-------------------------------------------------
//  remove a favorite from list
//-------------------------------------------------

void favorite_manager::remove_favorite_game(ui_software_info &swinfo)
{
	m_list.erase(std::remove(m_list.begin(), m_list.end(), swinfo), m_list.end());
	save_favorite_games();
}

//-------------------------------------------------
//  remove a favorite from list
//-------------------------------------------------

void favorite_manager::remove_favorite_game()
{
	m_list.erase(m_list.begin() + m_current);
	save_favorite_games();
}

//-------------------------------------------------
//  check if game is already in favorite list
//-------------------------------------------------

bool favorite_manager::isgame_favorite()
{
	if ((machine().system().flags & MACHINE_TYPE_ARCADE) != 0)
		return isgame_favorite(&machine().system());

	auto image_loaded = false;

	for (device_image_interface &image : image_interface_iterator(machine().root_device()))
	{
		const util::software_info *swinfo = image.software_entry();
		if (image.exists() && swinfo != nullptr)
		{
			image_loaded = true;
			for (size_t current = 0; current < m_list.size(); current++)
				if (m_list[current].shortname == swinfo->shortname() &&
					m_list[current].listname == image.software_list_name())
				{
					m_current = current;
					return true;
				}
		}
	}

	if (!image_loaded)
		return isgame_favorite(&machine().system());

	m_current = -1;
	return false;
}

//-------------------------------------------------
//  check if game is already in favorite list
//-------------------------------------------------

bool favorite_manager::isgame_favorite(const game_driver *driver)
{
	for (size_t x = 0; x < m_list.size(); x++)
		if (m_list[x].driver == driver && m_list[x].shortname == driver->name)
		{
			m_current = x;
			return true;
		}

	m_current = -1;
	return false;
}

//-------------------------------------------------
//  check if game is already in favorite list
//-------------------------------------------------

bool favorite_manager::isgame_favorite(ui_software_info const &swinfo)
{
	for (size_t x = 0; x < m_list.size(); x++)
		if (m_list[x] == swinfo)
		{
			m_current = x;
			return true;
		}

	m_current = -1;
	return false;
}

//-------------------------------------------------
//  parse favorite file
//-------------------------------------------------

void favorite_manager::parse_favorite()
{
	emu_file file(m_options.ui_path(), OPEN_FLAG_READ);
	if (file.open(favorite_filename) == osd_file::error::NONE)
	{
		char readbuf[1024];
		file.gets(readbuf, 1024);

		while (readbuf[0] == '[')
			file.gets(readbuf, 1024);

		while (file.gets(readbuf, 1024))
		{
			ui_software_info tmpmatches;
			tmpmatches.shortname = chartrimcarriage(readbuf);
			file.gets(readbuf, 1024);
			tmpmatches.longname = chartrimcarriage(readbuf);
			file.gets(readbuf, 1024);
			tmpmatches.parentname = chartrimcarriage(readbuf);
			file.gets(readbuf, 1024);
			tmpmatches.year = chartrimcarriage(readbuf);
			file.gets(readbuf, 1024);
			tmpmatches.publisher = chartrimcarriage(readbuf);
			file.gets(readbuf, 1024);
			tmpmatches.supported = atoi(readbuf);
			file.gets(readbuf, 1024);
			tmpmatches.part = chartrimcarriage(readbuf);
			file.gets(readbuf, 1024);
			chartrimcarriage(readbuf);
			auto dx = driver_list::find(readbuf);
			if (dx == -1) continue;
			tmpmatches.driver = &driver_list::driver(dx);
			file.gets(readbuf, 1024);
			tmpmatches.listname = chartrimcarriage(readbuf);
			file.gets(readbuf, 1024);
			tmpmatches.interface = chartrimcarriage(readbuf);
			file.gets(readbuf, 1024);
			tmpmatches.instance = chartrimcarriage(readbuf);
			file.gets(readbuf, 1024);
			tmpmatches.startempty = atoi(readbuf);
			file.gets(readbuf, 1024);
			tmpmatches.parentlongname = chartrimcarriage(readbuf);
			file.gets(readbuf, 1024);
			tmpmatches.usage = chartrimcarriage(readbuf);
			file.gets(readbuf, 1024);
			tmpmatches.devicetype = chartrimcarriage(readbuf);
			file.gets(readbuf, 1024);
			tmpmatches.available = atoi(readbuf);
			m_list.push_back(tmpmatches);
		}
		file.close();
	}
}

//-------------------------------------------------
//  save favorite
//-------------------------------------------------

void favorite_manager::save_favorite_games()
{
	// attempt to open the output file
	emu_file file(m_options.ui_path(), OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS);
	if (file.open(favorite_filename) == osd_file::error::NONE)
	{
		if (m_list.empty())
		{
			file.remove_on_close();
			file.close();
			return;
		}

		// generate the favorite INI
		std::ostringstream text;
		text << "[ROOT_FOLDER]\n[Favorite]\n\n";
		for (auto & elem : m_list)
		{
			text << elem.shortname << '\n';
			text << elem.longname << '\n';
			text << elem.parentname << '\n';
			text << elem.year << '\n';
			text << elem.publisher << '\n';
			util::stream_format(text, "%d\n", elem.supported);
			text << elem.part << '\n';
			util::stream_format(text, "%s\n", elem.driver->name);
			text << elem.listname << '\n';
			text << elem.interface << '\n';
			text << elem.instance << '\n';
			util::stream_format(text, "%d\n", elem.startempty);
			text << elem.parentlongname << '\n';
			text << elem.usage << '\n';
			text << elem.devicetype << '\n';
			util::stream_format(text, "%d\n", elem.available);
		}
		file.puts(text.str().c_str());
		file.close();
	}
}
