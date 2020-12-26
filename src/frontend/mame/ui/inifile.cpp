// license:BSD-3-Clause
// copyright-holders:Maurizio Petrarota, Vas Crabb
/***************************************************************************

    ui/inifile.cpp

    UI INIs file manager.

***************************************************************************/

#include "emu.h"
#include "ui/inifile.h"

#include "ui/moptions.h"

#include "drivenum.h"
#include "softlist_dev.h"

#include <algorithm>
#include <cstring>
#include <iterator>


namespace {

char const FAVORITE_FILENAME[] = "favorites.ini";

} // anonymous namespace


//-------------------------------------------------
//  ctor
//-------------------------------------------------

inifile_manager::inifile_manager(ui_options &options)
	: m_options(options)
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
		osd_printf_error("Failed to open category file %s for reading\n", filename);
		return;
	}

	int64_t const offset(m_ini_index[file].second[category].second);
	if (fp.seek(offset, SEEK_SET) || (fp.tell() != offset))
	{
		fp.close();
		osd_printf_error("Failed to seek to category offset in file %s\n", filename);
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

bool favorite_manager::favorite_compare::operator()(ui_software_info const &lhs, ui_software_info const &rhs) const
{
	assert(lhs.driver);
	assert(rhs.driver);

	if (!lhs.startempty)
	{
		if (rhs.startempty)
			return false;
		else if (lhs.listname < rhs.listname)
			return true;
		else if (lhs.listname > rhs.listname)
			return false;
		else if (lhs.shortname < rhs.shortname)
			return true;
		else if (lhs.shortname > rhs.shortname)
			return false;
	}
	else if (!rhs.startempty)
	{
		return true;
	}

	return 0 > std::strncmp(lhs.driver->name, rhs.driver->name, ARRAY_LENGTH(lhs.driver->name));
}

bool favorite_manager::favorite_compare::operator()(ui_software_info const &lhs, game_driver const &rhs) const
{
	assert(lhs.driver);

	if (!lhs.startempty)
		return false;
	else
		return 0 > std::strncmp(lhs.driver->name, rhs.name, ARRAY_LENGTH(rhs.name));
}

bool favorite_manager::favorite_compare::operator()(game_driver const &lhs, ui_software_info const &rhs) const
{
	assert(rhs.driver);

	if (!rhs.startempty)
		return true;
	else
		return 0 > std::strncmp(lhs.name, rhs.driver->name, ARRAY_LENGTH(lhs.name));
}

bool favorite_manager::favorite_compare::operator()(ui_software_info const &lhs, running_software_key const &rhs) const
{
	assert(lhs.driver);
	assert(std::get<1>(rhs));

	if (lhs.startempty)
		return true;
	else if (lhs.listname < std::get<1>(rhs))
		return true;
	else if (lhs.listname > std::get<1>(rhs))
		return false;
	else if (lhs.shortname < std::get<2>(rhs))
		return true;
	else if (lhs.shortname > std::get<2>(rhs))
		return false;
	else
		return 0 > std::strncmp(lhs.driver->name, std::get<0>(rhs).name, ARRAY_LENGTH(lhs.driver->name));
}

bool favorite_manager::favorite_compare::operator()(running_software_key const &lhs, ui_software_info const &rhs) const
{
	assert(std::get<1>(lhs));
	assert(rhs.driver);

	if (rhs.startempty)
		return false;
	else if (std::get<1>(lhs) < rhs.listname)
		return true;
	else if (std::get<1>(lhs) > rhs.listname)
		return false;
	else if (std::get<2>(lhs) < rhs.shortname)
		return true;
	else if (std::get<2>(lhs) > rhs.shortname)
		return false;
	else
		return 0 > std::strncmp(std::get<0>(lhs).name, rhs.driver->name, ARRAY_LENGTH(rhs.driver->name));
}


//-------------------------------------------------
//  construction/destruction
//-------------------------------------------------

favorite_manager::favorite_manager(ui_options &options)
	: m_options(options)
	, m_favorites()
	, m_sorted()
	, m_need_sort(true)
{
	emu_file file(m_options.ui_path(), OPEN_FLAG_READ);
	if (file.open(FAVORITE_FILENAME) == osd_file::error::NONE)
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
			if (0 > dx)
				continue;
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
			m_favorites.emplace(std::move(tmpmatches));
		}
		file.close();
	}
}


//-------------------------------------------------
//  add
//-------------------------------------------------

void favorite_manager::add_favorite_system(game_driver const &driver)
{
	add_impl(driver);
}

void favorite_manager::add_favorite_software(ui_software_info const &swinfo)
{
	add_impl(swinfo);
}

void favorite_manager::add_favorite(running_machine &machine)
{
	apply_running_machine(
			machine,
			[this, &machine] (game_driver const &driver, device_image_interface *imagedev, software_info const *software, bool &done)
			{
				if (imagedev)
				{
					// creating this is fairly expensive, but we'll assume this usually succeeds
					ui_software_info info;
					software_part const *const part(imagedev->part_entry());
					assert(software);
					assert(part);

					// start with simple stuff that can just be copied
					info.shortname = software->shortname();
					info.longname = software->longname();
					info.parentname = software->parentname();
					info.year = software->year();
					info.publisher = software->publisher();
					info.supported = software->supported();
					info.part = part->name();
					info.driver = &driver;
					info.listname = imagedev->software_list_name();
					info.interface = part->interface();
					info.instance = imagedev->instance_name();
					info.startempty = 0;
					info.devicetype = strensure(imagedev->image_type_name());
					info.available = true;

					// look up the parent in the list if necessary (eugh, O(n) walk)
					if (!info.parentname.empty())
					{
						auto const listdev = software_list_device::find_by_name(machine.config(), info.listname);
						assert(listdev);
						for (software_info const &other : listdev->get_info())
						{
							if (other.shortname() == info.parentname)
							{
								info.parentlongname = other.longname();
								break;
							}
						}
					}

					// fill in with the first usage entry we find
					for (feature_list_item const &feature : software->other_info())
					{
						if (feature.name() == "usage")
						{
							info.usage = feature.value();
							break;
						}
					}

					// hooray for move semantics!
					add_impl(std::move(info));
				}
				else
				{
					add_impl(driver);
				}
			});
}

template <typename T> void favorite_manager::add_impl(T &&key)
{
	auto const ins(m_favorites.emplace(std::forward<T>(key)));
	if (ins.second)
	{
		if (!m_sorted.empty())
			m_sorted.emplace_back(std::ref(*ins.first));
		m_need_sort = true;
		save_favorites();
	}
}


//-------------------------------------------------
//  check
//-------------------------------------------------

bool favorite_manager::is_favorite_system(game_driver const &driver) const
{
	return check_impl(driver);
}

bool favorite_manager::is_favorite_software(ui_software_info const &swinfo) const
{
	auto found(m_favorites.lower_bound(swinfo));
	if ((m_favorites.end() != found) && (found->listname == swinfo.listname) && (found->shortname == swinfo.shortname))
		return true;
	else if (m_favorites.begin() == found)
		return false;

	// need to back up and check for matching software with lexically earlier driver
	--found;
	return (found->listname == swinfo.listname) && (found->shortname == swinfo.shortname);
}

bool favorite_manager::is_favorite(running_machine &machine) const
{
	bool result(false);
	apply_running_machine(
			machine,
			[this, &result] (game_driver const &driver, device_image_interface *imagedev, software_info const *software, bool &done)
			{
				assert(!result);
				result = imagedev
						? check_impl(running_software_key(driver, imagedev->software_list_name(), software->shortname()))
						: check_impl(driver);
				done = done || result;
			});
	return result;
}

bool favorite_manager::is_favorite_system_software(ui_software_info const &swinfo) const
{
	return check_impl(swinfo);
}

template <typename T> bool favorite_manager::check_impl(T const &key) const
{
	return m_favorites.find(key) != m_favorites.end();
}


//-------------------------------------------------
//  remove
//-------------------------------------------------

void favorite_manager::remove_favorite_system(game_driver const &driver)
{
	remove_impl(driver);
}

void favorite_manager::remove_favorite_software(ui_software_info const &swinfo)
{
	remove_impl(swinfo);
}

void favorite_manager::remove_favorite(running_machine &machine)
{
	apply_running_machine(
			machine,
			[this] (game_driver const &driver, device_image_interface *imagedev, software_info const *software, bool &done)
			{
				done = imagedev
						? remove_impl(running_software_key(driver, imagedev->software_list_name(), software->shortname()))
						: remove_impl(driver);
			});
}

template <typename T> bool favorite_manager::remove_impl(T const &key)
{
	auto const found(m_favorites.find(key));
	if (m_favorites.end() != found)
	{
		m_favorites.erase(found);
		m_sorted.clear();
		m_need_sort = true;
		save_favorites();
		return true;
	}
	else
	{
		return false;
	}
}


//-------------------------------------------------
//  implementation
//-------------------------------------------------

template <typename T>
void favorite_manager::apply_running_machine(running_machine &machine, T &&action)
{
	bool done(false);

	// TODO: this should be changed - it interacts poorly with cartslots on arcade systems
	if ((machine.system().flags & machine_flags::MASK_TYPE) == machine_flags::TYPE_ARCADE)
	{
		action(machine.system(), nullptr, nullptr, done);
	}
	else
	{
		bool have_software(false);
		for (device_image_interface &image_dev : image_interface_enumerator(machine.root_device()))
		{
			software_info const *const sw(image_dev.software_entry());
			if (image_dev.exists() && image_dev.loaded_through_softlist() && sw)
			{
				assert(image_dev.software_list_name());

				have_software = true;
				action(machine.system(), &image_dev, sw, done);
				if (done)
					return;
			}
		}

		if (!have_software)
			action(machine.system(), nullptr, nullptr, done);
	}
}

void favorite_manager::update_sorted()
{
	if (m_need_sort)
	{
		if (m_sorted.empty())
			std::copy(m_favorites.begin(), m_favorites.end(), std::back_inserter(m_sorted));

		assert(m_favorites.size() == m_sorted.size());
		std::stable_sort(
				m_sorted.begin(),
				m_sorted.end(),
				[] (ui_software_info const &lhs, ui_software_info const &rhs) -> bool
				{
					assert(lhs.driver);
					assert(rhs.driver);

					int cmp;

					cmp = core_stricmp(lhs.longname.c_str(), rhs.longname.c_str());
					if (0 > cmp)
						return true;
					else if (0 < cmp)
						return false;

					cmp = core_stricmp(lhs.driver->type.fullname(), rhs.driver->type.fullname());
					if (0 > cmp)
						return true;
					else if (0 < cmp)
						return false;

					cmp = std::strcmp(lhs.listname.c_str(), rhs.listname.c_str());
					if (0 > cmp)
						return true;
					else if (0 < cmp)
						return false;

					return false;
				});

		m_need_sort = false;
	}
}

void favorite_manager::save_favorites()
{
	// attempt to open the output file
	emu_file file(m_options.ui_path(), OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS);
	if (file.open(FAVORITE_FILENAME) == osd_file::error::NONE)
	{
		if (m_favorites.empty())
		{
			// delete it if there are no favorites
			file.remove_on_close();
		}
		else
		{
			// generate the favorite INI
			file.puts("[ROOT_FOLDER]\n[Favorite]\n\n");
			util::ovectorstream buf;
			for (ui_software_info const &info : m_favorites)
			{
				buf.clear();
				buf.rdbuf()->clear();

				buf << info.shortname << '\n';
				buf << info.longname << '\n';
				buf << info.parentname << '\n';
				buf << info.year << '\n';
				buf << info.publisher << '\n';
				util::stream_format(buf, "%d\n", info.supported);
				buf << info.part << '\n';
				util::stream_format(buf, "%s\n", info.driver->name);
				buf << info.listname << '\n';
				buf << info.interface << '\n';
				buf << info.instance << '\n';
				util::stream_format(buf, "%d\n", info.startempty);
				buf << info.parentlongname << '\n';
				buf << info.usage << '\n';
				buf << info.devicetype << '\n';
				util::stream_format(buf, "%d\n", info.available);

				buf.put('\0');
				file.puts(&buf.vec()[0]);
			}
		}
		file.close();
	}
}
