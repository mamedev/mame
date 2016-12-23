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

#include <cstdio>



osd_watchdog::osd_watchdog()
	: m_timeout(60 * osd_ticks_per_second())
	, m_event(1, 0)
	, m_do_exit()
	, m_thread()
{
	m_thread.reset(new std::thread(&osd_watchdog::watchdog_thread, this));
}

osd_watchdog::~osd_watchdog()
{
	m_do_exit = 1;
	m_event.set();
	m_thread->join();
}

void osd_watchdog::setTimeout(int timeout)
{
	m_timeout = timeout * osd_ticks_per_second();
	reset();
}

void *osd_watchdog::watchdog_thread(void *param)
{
	osd_watchdog *const thiz(reinterpret_cast<osd_watchdog *>(param));

	while (true)
	{
		if (thiz->wait())
		{
			if (thiz->do_exit())
				break;
			else
				thiz->clear_event();
		}
		else
		{
			std::fflush(stdout);
			std::fprintf(stderr, "Terminating due to watchdog timeout\n");
			std::fflush(stderr);

			osd_process_kill();
		}
	}
	return nullptr;
}
