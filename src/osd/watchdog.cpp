// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont
//============================================================
//
//  watchdog.c - watchdog handling
//
//  SDLMAME by Olivier Galibert and R. Belmont
//
//============================================================

#include "osdcomm.h"
#include "osdcore.h"
#include "eminline.h"

#include "watchdog.h"
#include "modules/lib/osdlib.h"

static void *watchdog_thread(void *param)
{
	osd_watchdog *thiz = (osd_watchdog *) param;

	while (TRUE)
	{
		if (thiz->event().wait(thiz->getTimeout()))
		{
			if (thiz->do_exit())
				break;
			else
			{
				thiz->event().reset();
				continue;
			}
		}
		else
		{
			fprintf(stderr, "Terminating due to watchdog timeout\n");

			osd_process_kill();
		}
	}
	return nullptr;
}

osd_watchdog::osd_watchdog(void)
: m_event(1,0)
{
	m_do_exit = 0;
	m_thread = new std::thread(watchdog_thread, this);
	m_timeout = 60 * osd_ticks_per_second();
}

osd_watchdog::~osd_watchdog(void)
{
	m_do_exit = 1;
	m_event.set();
	m_thread->join();
	delete m_thread;
}

void osd_watchdog::setTimeout(int timeout)
{
	m_timeout = timeout * osd_ticks_per_second();
	this->reset();
}
