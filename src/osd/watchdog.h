// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont
#ifndef _watchdog_h_
#define _watchdog_h_
//============================================================
//
//  watchdog.h - watchdog handling
//
//  SDLMAME by Olivier Galibert and R. Belmont
//
//============================================================

#include "osdsync.h"
#include <atomic>
#include <thread>

class osd_watchdog
{
public:
	osd_watchdog(void);
	~osd_watchdog(void);

	void reset() { m_event.set(); }

	osd_event &     event(void) { return m_event; }
	INT32           do_exit(void) const { return m_do_exit; }
	osd_ticks_t     getTimeout(void) const { return m_timeout; }
	void            setTimeout(int timeout);
private:
	osd_event      	m_event;
	std::thread*    m_thread;
	std::atomic<INT32>  m_do_exit;

	osd_ticks_t     m_timeout;
};
#endif
