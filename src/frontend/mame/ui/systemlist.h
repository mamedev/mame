// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

    ui/systemlist.h

    Persistent system list data.

***************************************************************************/
#ifndef MAME_FRONTEND_UI_SYSTEMLIST_H
#define MAME_FRONTEND_UI_SYSTEMLIST_H

#pragma once

#include "ui/utils.h"

#include <atomic>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>


class ui_options;


namespace ui {

class system_list
{
public:
	enum available : unsigned
	{
		AVAIL_NONE                  = 0U,
		AVAIL_SYSTEM_NAMES          = 1U << 0,
		AVAIL_SORTED_LIST           = 1U << 1,
		AVAIL_BIOS_COUNT            = 1U << 2,
		AVAIL_UCS_SHORTNAME         = 1U << 3,
		AVAIL_UCS_DESCRIPTION       = 1U << 4,
		AVAIL_UCS_MANUF_DESC        = 1U << 5,
		AVAIL_UCS_DFLT_DESC         = 1U << 6,
		AVAIL_UCS_MANUF_DFLT_DESC   = 1U << 7,
		AVAIL_FILTER_DATA           = 1U << 8
	};

	using system_vector = std::vector<ui_system_info>;
	using system_reference = std::reference_wrapper<ui_system_info>;
	using system_reference_vector = std::vector<system_reference>;

	void cache_data(ui_options const &options);

	void reset_cache();

	bool is_available(available desired) const
	{
		return (m_available.load(std::memory_order_acquire) & desired) == desired;
	}

	void wait_available(available desired);

	system_vector const &systems()
	{
		wait_available(AVAIL_SYSTEM_NAMES);
		return m_systems;
	}

	system_reference_vector const &sorted_list()
	{
		wait_available(AVAIL_SORTED_LIST);
		return m_sorted_list;
	}

	int bios_count()
	{
		wait_available(AVAIL_BIOS_COUNT);
		return m_bios_count;
	}

	bool unavailable_systems()
	{
		wait_available(AVAIL_SORTED_LIST);
		return std::find_if(m_sorted_list.begin(), m_sorted_list.end(), [] (ui_system_info const &info) { return !info.available; }) != m_sorted_list.end();
	}

	machine_filter_data &filter_data()
	{
		wait_available(AVAIL_FILTER_DATA);
		return m_filter_data;
	}

	static system_list &instance();

private:
	system_list();
	~system_list();

	void notify_available(available value);
	void do_cache_data(std::string const &datpath, std::string const &titles);
	void populate_list(bool copydesc);
	void load_titles(util::core_file &file);
	void populate_parents();

	// synchronisation
	std::mutex                      m_mutex;
	std::condition_variable         m_condition;
	std::unique_ptr<std::thread>    m_thread;
	std::atomic<bool>               m_started;
	std::atomic<unsigned>           m_available;

	// data
	system_vector                   m_systems;
	system_reference_vector         m_sorted_list;
	machine_filter_data             m_filter_data;
	int                             m_bios_count;
};

} // namespace ui

#endif // MAME_FRONTEND_UI_SYSTEMLIST_H
