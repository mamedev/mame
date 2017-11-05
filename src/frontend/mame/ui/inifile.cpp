// license:BSD-3-Clause
// copyright-holders:Maurizio Petrarota
/***************************************************************************

    ui/inifile.cpp

    UI INIs file manager.

***************************************************************************/

#include "emu.h"
#include "ui/inifile.h"

#include "ui/moptions.h"

#include "drivenum.h"
#include "softlist_dev.h"


//-------------------------------------------------
//  ctor
//-------------------------------------------------

inifile_manager::inifile_manager(running_machine &machine, ui_options &moptions)
	: m_options(moptions)
	, m_ini_index()
{
	// scan directories and create index
	file_enumerator path(m_options.categoryini_path());
	for (osd::directory::entry const *dir = path.next(); dir; dir = path.next())
	{
		std::string name(dir->name);
		if (core_filename_ends_with(name, ".ini"))
		{
			emu_file file(m_options.categoryini_path(), OPEN_FLAG_READ);
			if (file.open(name) == osd_file::error::NONE)
			{
				init_category(std::move(name), file);
				file.close();
			}
		}
	}
	std::stable_sort(m_ini_index.begin(), m_ini_index.end(), [] (auto const &x, auto const &y) { return 0 > core_stricmp(x.first.c_str(), y.first.c_str()); });
}

//-------------------------------------------------
//  load and indexing ini files
//-------------------------------------------------

void inifile_manager::load_ini_category(size_t file, size_t category, std::unordered_set<game_driver const *> &result) const
{
	std::string const &filename(m_ini_index[file].first);
	emu_file fp(m_options.categoryini_path(), OPEN_FLAG_READ);
	if (fp.open(filename) != osd_file::error::NONE)
	{
		osd_printf_error("Failed to open category file %s for reading\n", filename.c_str());
		return;
	}

	int64_t const offset(m_ini_index[file].second[category].second);
	if (fp.seek(offset, SEEK_SET) || (fp.tell() != offset))
	{
		fp.close();
		osd_printf_error("Failed to seek to category offset in file %s\n", filename.c_str());
		return;
	}

	char rbuf[MAX_CHAR_INFO];
	while (fp.gets(rbuf, MAX_CHAR_INFO) && rbuf[0] && ('[' != rbuf[0]))
	{
		auto const tail(std::find_if(std::begin(rbuf), std::prev(std::end(rbuf)), [] (char ch) { return !ch || ('\r' == ch) || ('\n' == ch); }));
		*tail = '\0';
		int const dfind(driver_list::find(rbuf));
		if (0 <= dfind)
			result.emplace(&driver_list::driver(dfind));
	}

	fp.close();
}

//-------------------------------------------------
//  initialize category
//-------------------------------------------------

void inifile_manager::init_category(std::string &&filename, emu_file &file)
{
	categoryindex index;
	char rbuf[MAX_CHAR_INFO];
	std::string name;
	while (file.gets(rbuf, ARRAY_LENGTH(rbuf)))
	{
		if ('[' == rbuf[0])
		{
			auto const head(std::next(std::begin(rbuf)));
			auto const tail(std::find_if(head, std::end(rbuf), [] (char ch) { return !ch || (']' == ch); }));
			name.assign(head, tail);
			if ("FOLDER_SETTINGS" != name)
				index.emplace_back(std::move(name), file.tell());
		}
	}
	std::stable_sort(index.begin(), index.end(), [] (auto const &x, auto const &y) { return 0 > core_stricmp(x.first.c_str(), y.first.c_str()); });
	if (!index.empty())
		m_ini_index.emplace_back(std::move(filename), std::move(index));
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
	parse_favorite();
}

//-------------------------------------------------
//  add a game
//-------------------------------------------------

void favorite_manager::add_favorite_game(const game_driver *driver)
{
	m_list.emplace(driver->type.fullname(), *driver);
	save_favorite_games();
}

//-------------------------------------------------
//  add a system
//-------------------------------------------------

void favorite_manager::add_favorite_game(ui_software_info &swinfo)
{
	m_list.emplace(swinfo.longname, swinfo);
	save_favorite_games();
}

//-------------------------------------------------
//  add a game / system
//-------------------------------------------------

void favorite_manager::add_favorite_game()
{
	if ((machine().system().flags & machine_flags::MASK_TYPE) == machine_flags::TYPE_ARCADE)
	{
		add_favorite_game(&machine().system());
		return;
	}

	auto software_avail = false;
	for (device_image_interface &image : image_interface_iterator(machine().root_device()))
	{
		if (image.exists() && image.loaded_through_softlist())
		{
			const software_info *swinfo = image.software_entry();
			const software_part *part = image.part_entry();
			ui_software_info tmpmatches;
			tmpmatches.shortname = swinfo->shortname();
			tmpmatches.longname = image.longname();
			tmpmatches.parentname = swinfo->parentname();
			tmpmatches.year = image.year();
			tmpmatches.publisher = image.manufacturer();
			tmpmatches.supported = image.supported();
			tmpmatches.part = part->name();
			tmpmatches.driver = &machine().system();
			tmpmatches.listname = strensure(image.software_list_name());
			tmpmatches.interface = part->interface();
			tmpmatches.instance = image.instance_name();
			tmpmatches.startempty = 0;
			tmpmatches.parentlongname.clear();
			if (!swinfo->parentname().empty())
			{
				auto swlist = software_list_device::find_by_name(machine().config(), image.software_list_name());
				for (const software_info &c_swinfo : swlist->get_info())
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
			for (const feature_list_item &flist : swinfo->other_info())
				if (!strcmp(flist.name().c_str(), "usage"))
					tmpmatches.usage = flist.value();

			tmpmatches.devicetype = strensure(image.image_type_name());
			tmpmatches.available = true;
			software_avail = true;
			m_list.emplace(tmpmatches.longname, tmpmatches);
			save_favorite_games();
		}
	}

	if (!software_avail)
		add_favorite_game(&machine().system());
}

//-------------------------------------------------
//  remove a favorite from list
//-------------------------------------------------

void favorite_manager::remove_favorite_game(ui_software_info const &swinfo)
{
	for (auto e = m_list.begin(); e != m_list.end(); ++e)
		if (e->second == swinfo)
		{
			m_list.erase(e);
			break;
		}
	m_current = m_list.begin();
	save_favorite_games();
}

//-------------------------------------------------
//  remove a favorite from list
//-------------------------------------------------

void favorite_manager::remove_favorite_game()
{
	m_list.erase(m_current);
	m_current = m_list.begin();
	save_favorite_games();
}

//-------------------------------------------------
//  check if game is already in favorite list
//-------------------------------------------------

bool favorite_manager::isgame_favorite()
{
	if ((machine().system().flags & machine_flags::MASK_TYPE) == machine_flags::TYPE_ARCADE)
		return isgame_favorite(&machine().system());

	auto image_loaded = false;

	for (device_image_interface &image : image_interface_iterator(machine().root_device()))
	{
		const software_info *swinfo = image.software_entry();
		if (image.exists() && swinfo != nullptr)
		{
			image_loaded = true;
			for (auto current = m_list.begin(); current != m_list.end(); ++current)
				if (current->second.shortname == swinfo->shortname() &&
					current->second.listname == image.software_list_name())
				{
					m_current = current;
					return true;
				}
		}
	}

	if (!image_loaded)
		return isgame_favorite(&machine().system());

	m_current = m_list.begin();
	return false;
}

//-------------------------------------------------
//  check if game is already in favorite list
//-------------------------------------------------

bool favorite_manager::isgame_favorite(const game_driver *driver)
{
	for (auto current = m_list.begin(); current != m_list.end(); ++current)
		if (current->second.driver == driver && current->second.shortname == driver->name)
		{
			m_current = current;
			return true;
		}

	m_current = m_list.begin();
	return false;
}

//-------------------------------------------------
//  check if game is already in favorite list
//-------------------------------------------------

bool favorite_manager::isgame_favorite(ui_software_info const &swinfo)
{
	for (auto current = m_list.begin(); current != m_list.end(); ++current)
		if (current->second == swinfo)
		{
			m_current = current;
			return true;
		}

	m_current = m_list.begin();
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
			m_list.emplace(tmpmatches.longname, tmpmatches);
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
		for (auto & e : m_list)
		{
			auto elem = e.second;
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
