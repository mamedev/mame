#ifndef _watchdog_h_
#define _watchdog_h_
//============================================================
//
//  watchdog.h - watchdog handling
//
//  Copyright (c) 1996-2011, Nicola Salmoria and the MAME Team.
//  Visit http://mamedev.org for licensing and usage restrictions.
//
//  SDLMAME by Olivier Galibert and R. Belmont
//
//============================================================

#include "sdlsync.h"

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
	osd_thread *    m_thread;
	volatile INT32  m_do_exit;

	osd_ticks_t     m_timeout;
};
#endif
