// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    drivenum.h

    Driver enumeration helpers.

***************************************************************************/

#pragma once

#ifndef __DRIVENUM_H__
#define __DRIVENUM_H__


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> driver_list

// driver_list is a purely static class that wraps the global driver list
class driver_list
{
	DISABLE_COPYING(driver_list);

protected:
	// construction/destruction
	driver_list();

public:
	// getters
	static int total() { return s_driver_count; }

	// any item by index
	static const game_driver &driver(int index) { assert(index >= 0 && index < s_driver_count); return *s_drivers_sorted[index]; }
	static int clone(int index) { return find(driver(index).parent); }
	static int non_bios_clone(int index) { int result = find(driver(index).parent); return (result != -1 && (driver(result).flags & MACHINE_IS_BIOS_ROOT) == 0) ? result : -1; }
	static int compatible_with(int index) { return find(driver(index).compatible_with); }

	// any item by driver
	static int clone(const game_driver &driver) { int index = find(driver); assert(index != -1); return clone(index); }
	static int non_bios_clone(const game_driver &driver) { int index = find(driver); assert(index != -1); return non_bios_clone(index); }
	static int compatible_with(const game_driver &driver) { int index = find(driver); assert(index != -1); return compatible_with(index); }

	// general helpers
	static int find(const char *name);
	static int find(const game_driver &driver) { return find(driver.name); }

	// static helpers
	static bool matches(const char *wildstring, const char *string);
	static int penalty_compare(const char *source, const char *target);

protected:
	// internal helpers
	static int driver_sort_callback(const void *elem1, const void *elem2);

	// internal state
	static int                          s_driver_count;
	static const game_driver * const    s_drivers_sorted[];
};


// ======================> driver_enumerator

// driver_enumerator enables efficient iteration through the driver list
class driver_enumerator : public driver_list
{
	DISABLE_COPYING(driver_enumerator);

public:
	// construction/destruction
	driver_enumerator(emu_options &options);
	driver_enumerator(emu_options &options, const char *filter);
	driver_enumerator(emu_options &options, const game_driver &filter);
	~driver_enumerator();

	// getters
	int count() const { return m_filtered_count; }
	int current() const { return m_current; }
	emu_options &options() const { return m_options; }

	// current item
	const game_driver &driver() const { return driver_list::driver(m_current); }
	machine_config &config() const { return config(m_current, m_options); }
	int clone() { return driver_list::clone(m_current); }
	int non_bios_clone() { return driver_list::non_bios_clone(m_current); }
	int compatible_with() { return driver_list::compatible_with(m_current); }
	void include() { include(m_current); }
	void exclude() { exclude(m_current); }

	// any item by index
	bool included(int index) const { assert(index >= 0 && index < s_driver_count); return m_included[index]; }
	bool excluded(int index) const { assert(index >= 0 && index < s_driver_count); return !m_included[index]; }
	machine_config &config(int index) const { return config(index,m_options); }
	machine_config &config(int index, emu_options &options) const;
	void include(int index) { assert(index >= 0 && index < s_driver_count); if (!m_included[index]) { m_included[index] = true; m_filtered_count++; }  }
	void exclude(int index) { assert(index >= 0 && index < s_driver_count); if (m_included[index]) { m_included[index] = false; m_filtered_count--; } }
	using driver_list::driver;
	using driver_list::clone;
	using driver_list::non_bios_clone;
	using driver_list::compatible_with;

	// filtering/iterating
	int filter(const char *string = nullptr);
	int filter(const game_driver &driver);
	void include_all();
	void exclude_all() { memset(&m_included[0], 0, sizeof(m_included[0]) * s_driver_count); m_filtered_count = 0; }
	void reset() { m_current = -1; }
	bool next();
	bool next_excluded();

	// general helpers
	void set_current(int index) { assert(index >= -1 && index <= s_driver_count); m_current = index; }
	void find_approximate_matches(const char *string, int count, int *results);

private:
	// internal helpers
	void release_current() const;

	static const int CONFIG_CACHE_COUNT = 100;
	
	// internal state
	int                 m_current;
	int                 m_filtered_count;
	emu_options &       m_options;
	std::vector<UINT8> m_included;
	mutable std::vector<std::unique_ptr<machine_config>> m_config;
	mutable std::vector<int> m_config_cache;
};

#endif
