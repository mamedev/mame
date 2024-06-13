// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    drivenum.cpp

    Driver enumeration helpers.

***************************************************************************/

#include "emu.h"
#include "drivenum.h"

#include "corestr.h"
#include "softlist_dev.h"
#include "unicode.h"

#include <algorithm>

#include <cctype>



//**************************************************************************
//  DRIVER LIST
//**************************************************************************

//-------------------------------------------------
//  driver_list - constructor
//-------------------------------------------------

driver_list::driver_list()
{
}


//-------------------------------------------------
//  find - find a driver by name
//-------------------------------------------------

int driver_list::find(const char *name)
{
	// if no name, bail
	if (!name)
		return -1;

	// binary search to find it
	game_driver const *const *const begin = s_drivers_sorted;
	game_driver const *const *const end = begin + s_driver_count;
	auto const cmp = [] (game_driver const *driver, char const *name) { return core_stricmp(driver->name, name) < 0; };
	game_driver const *const *const result = std::lower_bound(begin, end, name, cmp);
	return ((result == end) || core_stricmp((*result)->name, name)) ? -1 : std::distance(begin, result);
}


//-------------------------------------------------
//  matches - true if we match, taking into
//  account wildcards in the wildstring
//-------------------------------------------------

bool driver_list::matches(const char *wildstring, const char *string)
{
	// can only match internal drivers if the wildstring starts with an underscore
	if (string[0] == '_' && (wildstring == nullptr || wildstring[0] != '_'))
		return false;

	// match everything else normally
	return (wildstring == nullptr || core_strwildcmp(wildstring, string) == 0);
}



//**************************************************************************
//  DRIVER ENUMERATOR
//**************************************************************************

//-------------------------------------------------
//  driver_enumerator - constructor
//-------------------------------------------------

driver_enumerator::driver_enumerator(emu_options &options)
	: m_current(-1)
	, m_filtered_count(0)
	, m_options(options)
	, m_included(s_driver_count)
	, m_config(CONFIG_CACHE_COUNT)
{
	include_all();
}


driver_enumerator::driver_enumerator(emu_options &options, const char *string)
	: driver_enumerator(options)
{
	filter(string);
}


driver_enumerator::driver_enumerator(emu_options &options, const game_driver &driver)
	: driver_enumerator(options)
{
	filter(driver);
}


//-------------------------------------------------
//  ~driver_enumerator - destructor
//-------------------------------------------------

driver_enumerator::~driver_enumerator()
{
	// configs are freed by the cache
}


//-------------------------------------------------
//  config - return a machine_config for the given
//  driver, allocating on demand if needed
//-------------------------------------------------

std::shared_ptr<machine_config> const &driver_enumerator::config(std::size_t index, emu_options &options) const
{
	assert(index < s_driver_count);

	// if we don't have it cached, add it
	std::shared_ptr<machine_config> &config = m_config[index];
	if (!config)
		config = std::make_shared<machine_config>(*s_drivers_sorted[index], options);

	return config;
}


//-------------------------------------------------
//  filter - filter the driver list against the
//  given string
//-------------------------------------------------

std::size_t driver_enumerator::filter(const char *filterstring)
{
	// reset the count
	exclude_all();

	// match name against each driver in the list
	for (std::size_t index = 0; index < s_driver_count; index++)
		if (matches(filterstring, s_drivers_sorted[index]->name))
			include(index);

	return m_filtered_count;
}


//-------------------------------------------------
//  filter - filter the driver list against the
//  given driver
//-------------------------------------------------

std::size_t driver_enumerator::filter(const game_driver &driver)
{
	// reset the count
	exclude_all();

	// match name against each driver in the list
	for (std::size_t index = 0; index < s_driver_count; index++)
		if (s_drivers_sorted[index] == &driver)
			include(index);

	return m_filtered_count;
}


//-------------------------------------------------
//  include_all - include all non-internal drivers
//-------------------------------------------------

void driver_enumerator::include_all()
{
	std::fill(m_included.begin(), m_included.end(), true);
	m_filtered_count = m_included.size();

	// always exclude the empty driver
	exclude(find("___empty"));
}


//-------------------------------------------------
//  next - get the next driver matching the given
//  filter
//-------------------------------------------------

bool driver_enumerator::next()
{
	release_current();

	// always advance one
	// if we have a filter, scan forward to the next match
	for (m_current++; (m_current < s_driver_count) && !m_included[m_current]; m_current++) { }

	// return true if we end up in range
	return (m_current >= 0) && (m_current < s_driver_count);
}


//-------------------------------------------------
//  next_excluded - get the next driver that is
//  not currently included in the list
//-------------------------------------------------

bool driver_enumerator::next_excluded()
{
	release_current();

	// always advance one
	// if we have a filter, scan forward to the next match
	for (m_current++; (m_current < s_driver_count) && m_included[m_current]; m_current++) { }

	// return true if we end up in range
	return (m_current >= 0) && (m_current < s_driver_count);
}


//-------------------------------------------------
//  driver_sort_callback - compare two items in
//  an array of game_driver pointers
//-------------------------------------------------

void driver_enumerator::find_approximate_matches(std::string const &string, std::size_t count, int *results)
{
#undef rand

	// if no name, pick random entries
	if (string.empty())
	{
		// seed the RNG first
		srand(osd_ticks());

		// allocate a temporary list
		std::vector<int> templist(m_filtered_count);
		int arrayindex = 0;
		for (int index = 0; index < s_driver_count; index++)
			if (m_included[index])
				templist[arrayindex++] = index;
		assert(arrayindex == m_filtered_count);

		// shuffle
		for (int shufnum = 0; shufnum < (4 * s_driver_count); shufnum++)
		{
			int item1 = rand() % m_filtered_count;
			int item2 = rand() % m_filtered_count;
			int temp = templist[item1];
			templist[item1] = templist[item2];
			templist[item2] = temp;
		}

		// copy out the first few entries
		for (int matchnum = 0; matchnum < count; matchnum++)
			results[matchnum] = templist[matchnum % m_filtered_count];
	}
	else
	{
		// allocate memory to track the penalty value
		std::vector<std::pair<double, int> > penalty;
		penalty.reserve(count + 1);
		std::u32string const search(ustr_from_utf8(normalize_unicode(string, unicode_normalization_form::D, true)));
		std::string composed;
		std::u32string candidate;

		// scan the entire drivers array
		for (int index = 0; index < s_driver_count; index++)
		{
			// skip things that can't run
			if (m_included[index])
			{
				// cheat on the shortname as it's always lowercase ASCII
				game_driver const &drv(*s_drivers_sorted[index]);
				std::size_t const namelen(std::strlen(drv.name));
				candidate.resize(namelen);
				std::copy_n(drv.name, namelen, candidate.begin());
				double curpenalty(util::edit_distance(search, candidate));

				// if it's not a perfect match, try the description
				if (curpenalty)
				{
					candidate = ustr_from_utf8(normalize_unicode(drv.type.fullname(), unicode_normalization_form::D, true));
					double p(util::edit_distance(search, candidate));
					if (p < curpenalty)
						curpenalty = p;
				}

				// also check "<manufacturer> <description>"
				if (curpenalty)
				{
					composed.assign(drv.manufacturer);
					composed.append(1, ' ');
					composed.append(drv.type.fullname());
					candidate = ustr_from_utf8(normalize_unicode(composed, unicode_normalization_form::D, true));
					double p(util::edit_distance(search, candidate));
					if (p < curpenalty)
						curpenalty = p;
				}

				// insert into the sorted table of matches
				auto const it(std::upper_bound(penalty.begin(), penalty.end(), std::make_pair(curpenalty, index)));
				if (penalty.end() != it)
				{
					penalty.emplace(it, curpenalty, index);
					if (penalty.size() > count)
						penalty.pop_back();
				}
				else if (penalty.size() < count)
				{
					penalty.emplace(it, curpenalty, index);
				}
			}
		}

		// copy to output and pad with -1
		std::fill(
				std::transform(
					penalty.begin(),
					penalty.end(),
					results,
					[] (std::pair<double, int> const &x) { return x.second; }),
				results + count,
				-1);
	}
}


//-------------------------------------------------
//  release_current - release bulky memory
//  structures from the current entry because
//  we're done with it
//-------------------------------------------------

void driver_enumerator::release_current() const
{
	// skip if no current entry
	if ((m_current >= 0) && (m_current < s_driver_count))
	{
		// skip if we haven't cached a config
		auto const cached = m_config.find(m_current);
		if (cached != m_config.end())
		{
			// iterate over software lists in this entry and reset
			for (software_list_device &swlistdev : software_list_device_enumerator(cached->second->root_device()))
				swlistdev.release();
		}
	}
}
