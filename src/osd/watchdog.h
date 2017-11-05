// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont
#ifndef MAME_OSD_WATCHDOG_H
#define MAME_OSD_WATCHDOG_H
#pragma once

//============================================================
//
//  watchdog.h - watchdog handling
//
//  SDLMAME by Olivier Galibert and R. Belmont
//
//============================================================

#include "osdsync.h"

#include <atomic>
#include <cstdint>
#include <memory>
#include <thread>


class osd_watchdog
{
public:
	osd_watchdog();
	~osd_watchdog();

	osd_ticks_t     getTimeout(void) const { return m_timeout; }
	void            setTimeout(int timeout);

	void            reset() { m_event.set(); }

private:
	static void *watchdog_thread(void *param);

	void clear_event() { m_event.reset(); }
	bool wait() { return m_event.wait(getTimeout()); }
	bool do_exit(void) const { return m_do_exit != 0; }

	osd_ticks_t                     m_timeout;
	osd_event                       m_event;
	std::atomic<std::int32_t>       m_do_exit;
	std::unique_ptr<std::thread>    m_thread;
};
#endif // MAME_OSD_WATCHDOG_H
