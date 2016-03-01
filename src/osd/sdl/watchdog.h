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

#include "modules/sync/osdsync.h"
#include <atomic>
#include <thread>

class watchdog
{
public:
	watchdog(void);
	~watchdog(void);

	void reset() { osd_event_set(m_event); }

	osd_event *     event(void) { return m_event; }
	INT32           do_exit(void) { return m_do_exit; }
	osd_ticks_t     getTimeout(void) { return m_timeout; }
	void            setTimeout(int timeout);
private:
	osd_event *     m_event;
	std::thread*      m_thread;
	std::atomic<INT32>  m_do_exit;

	osd_ticks_t     m_timeout;
};
#endif
