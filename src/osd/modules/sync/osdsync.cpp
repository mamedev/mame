// license:BSD-3-Clause
// copyright-holders:Aaron Giles,Miodrag Milanovic
//============================================================
//
//  osdsync.cpp -OSD core synchronization functions
//
//============================================================
// MAME headers
#include "osdcore.h"
#include "osdsync.h"

#include <stdlib.h>

// C++ headers
#include <mutex>
#include <atomic>
#include <condition_variable>
//============================================================
//  TYPE DEFINITIONS
//============================================================

struct osd_event {
	std::mutex               *mutex;
	std::condition_variable  *cond;
	std::atomic<INT32>       autoreset;
	std::atomic<INT32>       signalled;
};

//============================================================
//  osd_event_alloc
//============================================================

osd_event *osd_event_alloc(int manualreset, int initialstate)
{
	osd_event *ev;

	ev = (osd_event *)calloc(1, sizeof(osd_event));
	if (ev == nullptr)
		return nullptr;

	ev->mutex = new std::mutex();
	ev->cond = new std::condition_variable();
	ev->signalled = initialstate;
	ev->autoreset = !manualreset;

	return ev;
}

//============================================================
//  osd_event_free
//============================================================

void osd_event_free(osd_event *event)
{
	delete event->mutex;
	delete event->cond;
	free(event);
}

//============================================================
//  osd_event_set
//============================================================

void osd_event_set(osd_event *event)
{
	event->mutex->lock();
	if (event->signalled == FALSE)
	{
		event->signalled = TRUE;
		if (event->autoreset)
			event->cond->notify_one();
		else
			event->cond->notify_all();
	}
	event->mutex->unlock();
}

//============================================================
//  osd_event_reset
//============================================================

void osd_event_reset(osd_event *event)
{
	event->mutex->lock();
	event->signalled = FALSE;
	event->mutex->unlock();
}

//============================================================
//  osd_event_wait
//============================================================

int osd_event_wait(osd_event *event, osd_ticks_t timeout)
{
	if (timeout == OSD_EVENT_WAIT_INFINITE)
		timeout = osd_ticks_per_second() * (osd_ticks_t)10000;

	std::unique_lock<std::mutex> lock(*event->mutex);
	if (!timeout)
	{
		if (!event->signalled)
		{
			return FALSE;
		}
	}
	else
	{
		if (!event->signalled)
		{
			UINT64 msec = timeout * 1000 / osd_ticks_per_second();

			do {
				if (event->cond->wait_for(lock, std::chrono::milliseconds(msec)) == std::cv_status::timeout)
				{
					if (!event->signalled)
					{
						return FALSE;
					}
					else
						break;
				} else
					break;

			} while (TRUE);
		}
	}

	if (event->autoreset)
		event->signalled = 0;

	return TRUE;
}
