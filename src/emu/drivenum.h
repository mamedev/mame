// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    drivenum.h

    Driver enumeration helpers.

***************************************************************************/

#ifndef MAME_EMU_DRIVENUM_H
#define MAME_EMU_DRIVENUM_H

#pragma once

#include <algorithm>
#include <cassert>
#include <memory>


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

GAME_EXTERN(___empty);


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
	static std::size_t total() { return s_driver_count; }

	// any item by index
	static const game_driver &driver(std::size_t index) { assert(index < total()); return *s_drivers_sorted[index]; }
	static int clone(std::size_t index) { return find(driver(index).parent); }
	static int non_bios_clone(std::size_t index) { int const result = find(driver(index).parent); return ((result >= 0) && !(driver(result).flags & MACHINE_IS_BIOS_ROOT)) ? result : -1; }
	static int compatible_with(std::size_t index) { return find(driver(index).compatible_with); }

	// any item by driver
	static int clone(const game_driver &driver) { int const index = find(driver); assert(index >= 0); return clone(index); }
	static int non_bios_clone(const game_driver &driver) { int const index = find(driver); assert(index >= 0); return non_bios_clone(index); }
	static int compatible_with(const game_driver &driver) { int const index = find(driver); assert(index >= 0); return compatible_with(index); }

	// general helpers
	static int find(const char *name);
	static int find(const game_driver &driver) { return find(driver.name); }

	// static helpers
	static bool matches(const char *wildstring, const char *string);

protected:
	static std::size_t const            s_driver_count;
	static game_driver const * const    s_drivers_sorted[];
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
	std::size_t count() const { return m_filtered_count; }
	int current() const { return m_current; }
	emu_options &options() const { return m_options; }

	// current item
	const game_driver &driver() const { return driver_list::driver(m_current); }
	std::shared_ptr<machine_config> const &config() const { return config(m_current, m_options); }
	int clone() const { return driver_list::clone(m_current); }
	int non_bios_clone() const { return driver_list::non_bios_clone(m_current); }
	int compatible_with() const { return driver_list::compatible_with(m_current); }
	void include() { include(m_current); }
	void exclude() { exclude(m_current); }

	// any item by index
	bool included(std::size_t index) const { assert(index < m_included.size()); return m_included[index]; }
	bool excluded(std::size_t index) const { assert(index < m_included.size()); return !m_included[index]; }
	std::shared_ptr<machine_config> const &config(std::size_t index) const { return config(index, m_options); }
	std::shared_ptr<machine_config> const &config(std::size_t index, emu_options &options) const;
	void include(std::size_t index) { assert(index < m_included.size()); if (!m_included[index]) { m_included[index] = true; m_filtered_count++; }  }
	void exclude(std::size_t index) { assert(index < m_included.size()); if (m_included[index]) { m_included[index] = false; m_filtered_count--; } }
	using driver_list::driver;
	using driver_list::clone;
	using driver_list::non_bios_clone;
	using driver_list::compatible_with;

	// filtering/iterating
	std::size_t filter(const char *string = nullptr);
	std::size_t filter(const game_driver &driver);
	void include_all();
	void exclude_all() { std::fill(m_included.begin(), m_included.end(), false); m_filtered_count = 0; }
	void reset() { m_current = -1; }
	bool next();
	bool next_excluded();

	// general helpers
	void set_current(std::size_t index) { assert(index < s_driver_count); m_current = index; }
	void find_approximate_matches(std::string const &string, std::size_t count, int *results);

private:
	static constexpr std::size_t CONFIG_CACHE_COUNT = 100;

	typedef util::lru_cache_map<std::size_t, std::shared_ptr<machine_config> > machine_config_cache;

	// internal helpers
	void release_current() const;

	// internal state
	int                             m_current;
	std::size_t                     m_filtered_count;
	emu_options &                   m_options;
	std::vector<bool>               m_included;
	mutable machine_config_cache    m_config;
};

#endif // MAME_EMU_DRIVENUM_H
