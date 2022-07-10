// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

    ui/systemlist.h

    Persistent system list data.

***************************************************************************/

#include "emu.h"
#include "ui/systemlist.h"

#include "ui/moptions.h"

#include "drivenum.h"
#include "fileio.h"

#include "util/corestr.h"
#include "util/unicode.h"

#include <algorithm>
#include <cassert>
#include <functional>
#include <locale>
#include <string_view>


namespace ui {

void system_list::cache_data(ui_options const &options)
{
	std::unique_lock<std::mutex> lock(m_mutex);
	if (!m_started)
	{
		m_started = true;
#if defined(__EMSCRIPTEN__)
		std::invoke(
#else
		m_thread = std::make_unique<std::thread>(
#endif
				[this, datpath = std::string(options.history_path()), titles = std::string(options.system_names())]
				{
					do_cache_data(datpath, titles);
				});
	}
}


void system_list::reset_cache()
{
	std::unique_lock<std::mutex> lock(m_mutex);
	if (m_thread)
		m_thread->join();
	m_thread.reset();
	m_started = false;
	m_available = AVAIL_NONE;
	m_systems.clear();
	m_sorted_list.clear();
	m_filter_data = machine_filter_data();
	m_bios_count = 0;
}


void system_list::wait_available(available desired)
{
	if (!is_available(desired))
	{
		assert(m_started);
		std::unique_lock<std::mutex> lock(m_mutex);
		if (!is_available(desired))
			m_condition.wait(lock, [this, desired] () { return is_available(desired); });
	}
}


system_list &system_list::instance()
{
	static system_list data;
	return data;
}


system_list::system_list()
	: m_started(false)
	, m_available(AVAIL_NONE)
	, m_bios_count(0)
{
}


system_list::~system_list()
{
	if (m_thread)
		m_thread->join();
}


void system_list::notify_available(available value)
{
	std::unique_lock<std::mutex> lock(m_mutex);
	m_available.fetch_or(value, std::memory_order_release);
	m_condition.notify_all();
}


void system_list::do_cache_data(std::string const &datpath, std::string const &titles)
{
	// try to open the titles file for optimisation reasons
	emu_file titles_file(datpath, OPEN_FLAG_READ);
	bool const try_titles(!titles.empty() && !titles_file.open(titles));

	// generate full list - initially ordered by shortname
	populate_list(!try_titles);

	// notify that BIOS count is valie
	notify_available(AVAIL_BIOS_COUNT);

	// try to load localised descriptions
	if (try_titles)
	{
		load_titles(titles_file);

		// populate parent descriptions while still ordered by shortname
		// already done on the first pass if built-in titles are used
		populate_parents();
	}

	// system names are finalised now
	notify_available(AVAIL_SYSTEM_NAMES);

	// get rid of the "empty" driver - we don't need positions to line up any more
	m_sorted_list.reserve(m_systems.size() - 1);
	auto const empty(driver_list::find(GAME_NAME(___empty)));
	for (ui_system_info &info : m_systems)
	{
		if (info.index != empty)
			m_sorted_list.emplace_back(info);
	}

	// sort drivers and notify
	std::collate<wchar_t> const &coll = std::use_facet<std::collate<wchar_t> >(std::locale());
	auto const compare_names =
			[&coll] (std::wstring const &wx, std::wstring const &wy) -> bool
			{
				return 0 > coll.compare(wx.data(), wx.data() + wx.size(), wy.data(), wy.data() + wy.size());
			};
	std::stable_sort(
			m_sorted_list.begin(),
			m_sorted_list.end(),
			[&compare_names] (ui_system_info const &lhs, ui_system_info const &rhs)
			{
				game_driver const &x(*lhs.driver);
				game_driver const &y(*rhs.driver);

				if (!lhs.is_clone && !rhs.is_clone)
				{
					return compare_names(
							lhs.reading_description.empty() ? wstring_from_utf8(lhs.description) : lhs.reading_description,
							rhs.reading_description.empty() ? wstring_from_utf8(rhs.description) : rhs.reading_description);
				}
				else if (lhs.is_clone && rhs.is_clone)
				{
					if (!std::strcmp(x.parent, y.parent))
					{
						return compare_names(
								lhs.reading_description.empty() ? wstring_from_utf8(lhs.description) : lhs.reading_description,
								rhs.reading_description.empty() ? wstring_from_utf8(rhs.description) : rhs.reading_description);
					}
					else
					{
						return compare_names(
								lhs.reading_parent.empty() ? wstring_from_utf8(lhs.parent) : lhs.reading_parent,
								rhs.reading_parent.empty() ? wstring_from_utf8(rhs.parent) : rhs.reading_parent);
					}
				}
				else if (!lhs.is_clone && rhs.is_clone)
				{
					if (!std::strcmp(x.name, y.parent))
					{
						return true;
					}
					else
					{
						return compare_names(
								lhs.reading_description.empty() ? wstring_from_utf8(lhs.description) : lhs.reading_description,
								rhs.reading_parent.empty() ? wstring_from_utf8(rhs.parent) : rhs.reading_parent);
					}
				}
				else
				{
					if (!std::strcmp(x.parent, y.name))
					{
						return false;
					}
					else
					{
						return compare_names(
								lhs.reading_parent.empty() ? wstring_from_utf8(lhs.parent) : lhs.reading_parent,
								rhs.reading_description.empty() ? wstring_from_utf8(rhs.description) : rhs.reading_description);
					}
				}
			});
	notify_available(AVAIL_SORTED_LIST);

	// sort manufacturers and years
	m_filter_data.finalise();
	notify_available(AVAIL_FILTER_DATA);

	// convert shortnames to UCS-4
	for (ui_system_info &info : m_sorted_list)
		info.ucs_shortname = ustr_from_utf8(normalize_unicode(info.driver->name, unicode_normalization_form::D, true));
	notify_available(AVAIL_UCS_SHORTNAME);

	// convert descriptions to UCS-4
	for (ui_system_info &info : m_sorted_list)
		info.ucs_description = ustr_from_utf8(normalize_unicode(info.description, unicode_normalization_form::D, true));
	notify_available(AVAIL_UCS_DESCRIPTION);

	// convert "<manufacturer> <description>" to UCS-4
	std::string buf;
	for (ui_system_info &info : m_sorted_list)
	{
		buf.assign(info.driver->manufacturer);
		buf.append(1, ' ');
		buf.append(info.description);
		info.ucs_manufacturer_description = ustr_from_utf8(normalize_unicode(buf, unicode_normalization_form::D, true));
	}
	notify_available(AVAIL_UCS_MANUF_DESC);

	// convert default descriptions to UCS-4
	if (try_titles)
	{
		for (ui_system_info &info : m_sorted_list)
		{
			std::string_view const fullname(info.driver->type.fullname());
			if (info.description != fullname)
				info.ucs_default_description = ustr_from_utf8(normalize_unicode(fullname, unicode_normalization_form::D, true));
		}
	}
	notify_available(AVAIL_UCS_DFLT_DESC);

	// convert "<manufacturer> <default description>" to UCS-4
	if (try_titles)
	{
		for (ui_system_info &info : m_sorted_list)
		{
			std::string_view const fullname(info.driver->type.fullname());
			if (info.description != fullname)
			{
				buf.assign(info.driver->manufacturer);
				buf.append(1, ' ');
				buf.append(fullname);
				info.ucs_manufacturer_default_description = ustr_from_utf8(normalize_unicode(buf, unicode_normalization_form::D, true));
			}
		}
	}
	notify_available(AVAIL_UCS_MANUF_DFLT_DESC);
}


void system_list::populate_list(bool copydesc)
{
	m_systems.reserve(driver_list::total());
	std::unordered_set<std::string> manufacturers, years;
	for (int x = 0; x < driver_list::total(); ++x)
	{
		game_driver const &driver(driver_list::driver(x));
		ui_system_info &ins(m_systems.emplace_back(driver, x, false));
		if (&driver != &GAME_NAME(___empty))
		{
			if (driver.flags & machine_flags::IS_BIOS_ROOT)
				++m_bios_count;

			if ((driver.parent[0] != '0') || driver.parent[1])
			{
				auto const parentindex(driver_list::find(driver.parent));
				if (copydesc)
				{
					if (0 <= parentindex)
					{
						game_driver const &parentdriver(driver_list::driver(parentindex));
						ins.is_clone = !(parentdriver.flags & machine_flags::IS_BIOS_ROOT);
						ins.parent = parentdriver.type.fullname();
					}
					else
					{
						ins.is_clone = false;
						ins.parent = driver.parent;
					}
				}
				else
				{
					ins.is_clone = (0 <= parentindex) && !(driver_list::driver(parentindex).flags & machine_flags::IS_BIOS_ROOT);
				}
			}

			if (copydesc)
				ins.description = driver.type.fullname();

			m_filter_data.add_manufacturer(driver.manufacturer);
			m_filter_data.add_year(driver.year);
		}
	}
}


void system_list::load_titles(util::core_file &file)
{
	char readbuf[1024];
	std::string convbuf;
	while (file.gets(readbuf, std::size(readbuf)))
	{
		// shortname, description, and description reading separated by tab
		auto const eoln(
				std::find_if(
					std::begin(readbuf),
					std::end(readbuf),
					[] (char ch) { return !ch || ('\n' == ch) || ('\r' == ch); }));
		auto const split(std::find(std::begin(readbuf), eoln, '\t'));
		if (eoln == split)
			continue;
		std::string_view const shortname(readbuf, split - readbuf);

		// find matching system - still sorted by shortname at this point
		auto const found(
				std::lower_bound(
					m_systems.begin(),
					m_systems.end(),
					shortname,
					[] (ui_system_info const &a, std::string_view const &b)
					{
						return a.driver->name < b;
					}));
		if ((m_systems.end() == found) || (shortname != found->driver->name))
		{
			//osd_printf_verbose("System '%s' not found\n", shortname); very spammy for single-driver builds
			continue;
		}

		// find the end of the description
		auto const descstart(std::next(split));
		auto const descend(std::find(descstart, eoln, '\t'));
		auto const description(strtrimspace(std::string_view(descstart, descend - descstart)));
		if (description.empty())
		{
			osd_printf_warning("Empty translated description for system '%s'\n", shortname);
		}
		else if (!found->description.empty())
		{
			osd_printf_warning(
					"Multiple translated descriptions for system '%s' ('%s' and '%s')\n",
					shortname,
					found->description,
					description);
		}
		else
		{
			found->description = description;
		}

		// populate the reading if it's present
		if (eoln == descend)
			continue;
		auto const readstart(std::next(descend));
		auto const readend(std::find(readstart, eoln, '\t'));
		auto const reading(strtrimspace(std::string_view(readstart, readend - readstart)));
		if (reading.empty())
		{
			osd_printf_warning("Empty translated description reading for system '%s'\n", shortname);
		}
		else
		{
			found->reading_description = wstring_from_utf8(reading);
			found->ucs_reading_description = ustr_from_utf8(normalize_unicode(reading, unicode_normalization_form::D, true));
			convbuf.assign(found->driver->manufacturer);
			convbuf.append(1, ' ');
			convbuf.append(reading);
			found->ucs_manufacturer_reading_description = ustr_from_utf8(normalize_unicode(convbuf, unicode_normalization_form::D, true));
		}
	}

	// fill in untranslated descriptions
	for (ui_system_info &info : m_systems)
	{
		if (info.description.empty())
			info.description = info.driver->type.fullname();
	}
}


void system_list::populate_parents()
{
	for (ui_system_info &info : m_systems)
	{
		if ((info.driver->parent[0] != '0') || info.driver->parent[1])
		{
			auto const found(
					std::lower_bound(
						m_systems.begin(),
						m_systems.end(),
						std::string_view(info.driver->parent),
						[] (ui_system_info const &a, std::string_view const &b)
						{
							return a.driver->name < b;
						}));
			if (m_systems.end() != found)
			{
				info.parent = found->description;
				info.reading_parent = found->reading_description;
			}
			else
			{
				info.parent = info.driver->parent;
			}
		}
	}
}

} // namespace ui
