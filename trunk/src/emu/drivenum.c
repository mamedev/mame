/***************************************************************************

    drivenum.c

    Driver enumeration helpers.

****************************************************************************

    Copyright Aaron Giles
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
          notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer in
          the documentation and/or other materials provided with the
          distribution.
        * Neither the name 'MAME' nor the names of its contributors may be
          used to endorse or promote products derived from this software
          without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

***************************************************************************/

#include "emu.h"
#include "drivenum.h"
#include <ctype.h>



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
	if (name == NULL)
		return -1;

	// create a dummy item for comparison purposes
	game_driver driver;
	driver.name = name;
	game_driver *driverptr = &driver;

	// binary search to find it
	const game_driver **result = reinterpret_cast<const game_driver **>(bsearch(&driverptr, s_drivers_sorted, s_driver_count, sizeof(*s_drivers_sorted), driver_sort_callback));
	return (result == NULL) ? -1 : result - s_drivers_sorted;
}


//-------------------------------------------------
//  matches - true if we match, taking into
//  account wildcards in the wildstring
//-------------------------------------------------

bool driver_list::matches(const char *wildstring, const char *string)
{
	// can only match internal drivers if the wildstring starts with an underscore
	if (string[0] == '_' && (wildstring == NULL || wildstring[0] != '_'))
		return false;

	// match everything else normally
	return (wildstring == NULL || mame_strwildcmp(wildstring, string) == 0);
}


//-------------------------------------------------
//  driver_sort_callback - compare two items in
//  an array of game_driver pointers
//-------------------------------------------------

int driver_list::driver_sort_callback(const void *elem1, const void *elem2)
{
	const game_driver * const *item1 = reinterpret_cast<const game_driver * const *>(elem1);
	const game_driver * const *item2 = reinterpret_cast<const game_driver * const *>(elem2);
	return mame_stricmp((*item1)->name, (*item2)->name);
}


//-------------------------------------------------
//  penalty_compare - compare two strings for
//  closeness and assign a score.
//-------------------------------------------------

int driver_list::penalty_compare(const char *source, const char *target)
{
	int gaps = 1;
	bool last = true;

	// scan the strings
	for ( ; *source && *target; target++)
	{
		// do a case insensitive match
		bool match = (tolower((UINT8)*source) == tolower((UINT8)*target));

		// if we matched, advance the source
		if (match)
			source++;

		// if the match state changed, count gaps
		if (match != last)
		{
			last = match;
			if (!match)
				gaps++;
		}
	}

	// penalty if short string does not completely fit in
	for ( ; *source; source++)
		gaps++;

	// if we matched perfectly, gaps == 0
	if (gaps == 1 && *source == 0 && *target == 0)
		gaps = 0;

	return gaps;
}



//**************************************************************************
//  DRIVER ENUMERATOR
//**************************************************************************

//-------------------------------------------------
//  driver_enumerator - constructor
//-------------------------------------------------

driver_enumerator::driver_enumerator(emu_options &options)
	: m_current(-1),
	  m_filtered_count(0),
	  m_options(options),
	  m_included(global_alloc_array(UINT8, s_driver_count)),
	  m_config(global_alloc_array_clear(machine_config *, s_driver_count))
{
	include_all();
}


driver_enumerator::driver_enumerator(emu_options &options, const char *string)
	: m_current(-1),
	  m_filtered_count(0),
	  m_options(options),
	  m_included(global_alloc_array(UINT8, s_driver_count)),
	  m_config(global_alloc_array_clear(machine_config *, s_driver_count))
{
	filter(string);
}


driver_enumerator::driver_enumerator(emu_options &options, const game_driver &driver)
	: m_current(-1),
	  m_filtered_count(0),
	  m_options(options),
	  m_included(global_alloc_array(UINT8, s_driver_count)),
	  m_config(global_alloc_array_clear(machine_config *, s_driver_count))
{
	filter(driver);
}


//-------------------------------------------------
//  ~driver_enumerator - destructor
//-------------------------------------------------

driver_enumerator::~driver_enumerator()
{
	// free any configs
	for (int index = 0; index < s_driver_count; index++)
		global_free(m_config[index]);

	// free the arrays
	global_free(m_included);
	global_free(m_config);
}


//-------------------------------------------------
//  config - return a machine_config for the given
//  driver, allocating on demand if needed
//-------------------------------------------------

machine_config &driver_enumerator::config(int index, emu_options &options) const
{
	assert(index >= 0 && index < s_driver_count);

	// if we don't have it cached, add it
	if (m_config[index] == NULL)
	{
		// if our cache is full, release the head entry
		if (m_config_cache.count() == CONFIG_CACHE_COUNT)
		{
			config_entry *first = m_config_cache.first();
			m_config[first->index()] = NULL;
			m_config_cache.remove(*first);
		}

		// allocate the config and add it to the end of the list
		machine_config *config = m_config[index] = global_alloc(machine_config(*s_drivers_sorted[index], options));
		m_config_cache.append(*global_alloc(config_entry(*config, index)));
	}
	return *m_config[index];
}


//-------------------------------------------------
//  filter - filter the driver list against the
//  given string
//-------------------------------------------------

int driver_enumerator::filter(const char *filterstring)
{
	// reset the count
	exclude_all();

	// match name against each driver in the list
	for (int index = 0; index < s_driver_count; index++)
		if (matches(filterstring, s_drivers_sorted[index]->name))
			include(index);

	return m_filtered_count;
}


//-------------------------------------------------
//  filter - filter the driver list against the
//  given driver
//-------------------------------------------------

int driver_enumerator::filter(const game_driver &driver)
{
	// reset the count
	exclude_all();

	// match name against each driver in the list
	for (int index = 0; index < s_driver_count; index++)
		if (s_drivers_sorted[index] == &driver)
			include(index);

	return m_filtered_count;
}


//-------------------------------------------------
//  include_all - include all non-internal drivers
//-------------------------------------------------

void driver_enumerator::include_all()
{
	memset(m_included, 1, sizeof(m_included[0]) * s_driver_count); m_filtered_count = s_driver_count;
	int empty = find("___empty");
	assert(empty != -1);
	m_included[empty] = 0;
}


//-------------------------------------------------
//  next - get the next driver matching the given
//  filter
//-------------------------------------------------

bool driver_enumerator::next()
{
	// always advance one
	m_current++;

	// if we have a filter, scan forward to the next match
	while (m_current < s_driver_count)
	{
		if (m_included[m_current])
			break;
		m_current++;
	}

	// return true if we end up in range
	return (m_current >= 0 && m_current < s_driver_count);
}


//-------------------------------------------------
//  next_excluded - get the next driver that is
//  not currently included in the list
//-------------------------------------------------

bool driver_enumerator::next_excluded()
{
	// always advance one
	m_current++;

	// if we have a filter, scan forward to the next match
	while (m_current < s_driver_count)
	{
		if (!m_included[m_current])
			break;
		m_current++;
	}

	// return true if we end up in range
	return (m_current >= 0 && m_current < s_driver_count);
}


//-------------------------------------------------
//  driver_sort_callback - compare two items in
//  an array of game_driver pointers
//-------------------------------------------------

void driver_enumerator::find_approximate_matches(const char *string, int count, int *results)
{
#undef rand

	// if no name, pick random entries
	if (string == NULL || string[0] == 0)
	{
		// seed the RNG first
		srand(osd_ticks());

		// allocate a temporary list
		int *templist = global_alloc_array(int, m_filtered_count);
		int arrayindex = 0;
		for (int index = 0; index < s_driver_count; index++)
			if (m_included[index])
				templist[arrayindex++] = index;
		assert(arrayindex == m_filtered_count);

		// shuffle
		for (int shufnum = 0; shufnum < 4 * s_driver_count; shufnum++)
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

		global_free(templist);
		return;
	}

	// allocate memory to track the penalty value
	int *penalty = global_alloc_array(int, count);

	// initialize everyone's states
	for (int matchnum = 0; matchnum < count; matchnum++)
	{
		penalty[matchnum] = 9999;
		results[matchnum] = -1;
	}

	// scan the entire drivers array
	for (int index = 0; index < s_driver_count; index++)
		if (m_included[index])
		{
			// skip things that can't run
			if ((s_drivers_sorted[index]->flags & GAME_NO_STANDALONE) != 0)
				continue;

			// pick the best match between driver name and description
			int curpenalty = penalty_compare(string, s_drivers_sorted[index]->description);
			int tmp = penalty_compare(string, s_drivers_sorted[index]->name);
			curpenalty = MIN(curpenalty, tmp);

			// insert into the sorted table of matches
			for (int matchnum = count - 1; matchnum >= 0; matchnum--)
			{
				// stop if we're worse than the current entry
				if (curpenalty >= penalty[matchnum])
					break;

				// as long as this isn't the last entry, bump this one down
				if (matchnum < count - 1)
				{
					penalty[matchnum + 1] = penalty[matchnum];
					results[matchnum + 1] = results[matchnum];
				}
				results[matchnum] = index;
				penalty[matchnum] = curpenalty;
			}
		}

	// free our temp memory
	global_free(penalty);
}


driver_enumerator::config_entry::config_entry(machine_config &config, int index)
	: m_next(NULL),
	  m_config(&config),
	  m_index(index)
{
}


driver_enumerator::config_entry::~config_entry()
{
	global_free(m_config);
}
