//============================================================
//
//  watchdog.c - watchdog handling
//
//  Copyright (c) 1996-2011, Nicola Salmoria and the MAME Team.
//  Visit http://mamedev.org for licensing and usage restrictions.
//
//  SDLMAME by Olivier Galibert and R. Belmont
//
//============================================================

#include "osdcomm.h"
#include "osdcore.h"
#ifdef SDLMAME_NOASM
#include "eminline.h"
#else
#include "osinline.h"
#endif

#include "watchdog.h"

static void *watchdog_thread(void *param)
{
	watchdog *thiz = (watchdog *) param;

	while (TRUE)
	{
		if (osd_event_wait(thiz->event(), thiz->getTimeout()))
		{
			if (thiz->do_exit())
				break;
			else
			{
				osd_event_reset(thiz->event());
				continue;
			}
		}
		else
		{
			fprintf(stderr, "Terminating due to watchdog timeout\n");

			osd_process_kill();
		}
	}
	return NULL;
}

watchdog::watchdog(void)
{
	m_do_exit = 0;
	m_event = osd_event_alloc(1, 0);
	m_thread = osd_thread_create(watchdog_thread, this);
	m_timeout = 60 * osd_ticks_per_second();
}

watchdog::~watchdog(void)
{
	atomic_exchange32(&m_do_exit, 1);
	osd_event_set(m_event);
	osd_thread_wait_free(m_thread);
	osd_event_free(m_event);
}

void watchdog::setTimeout(int timeout)
{
	m_timeout = timeout * osd_ticks_per_second();
	this->reset();
}
