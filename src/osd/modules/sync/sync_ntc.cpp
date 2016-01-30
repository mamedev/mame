// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont
//============================================================
//
//  sdlsync.c - SDL core synchronization functions
//
//  SDLMAME by Olivier Galibert and R. Belmont
//
//============================================================

#ifndef _GNU_SOURCE
#define _GNU_SOURCE     // for PTHREAD_MUTEX_RECURSIVE; needs to be here before other glibc headers are included
#endif

#include "sdlinc.h"

#ifdef SDLMAME_MACOSX
#include <mach/mach.h>
#include <signal.h>
#endif

#if defined(SDLMAME_NETBSD) || defined(SDLMAME_OPENBSD)
/* for SIGKILL */
#include <signal.h>
#endif

// standard C headers
#include <math.h>
#include <unistd.h>

// MAME headers
#include "osdcore.h"
#include "osdsync.h"

#include "eminline.h"

#include <pthread.h>
#include <errno.h>
#include <sys/time.h>

struct osd_event {
	pthread_mutex_t     mutex;
	pthread_cond_t      cond;
	volatile INT32      autoreset;
	volatile INT32      signalled;
#ifdef PTR64
	INT8                padding[40];    // Fill a 64-byte cache line
#else
	INT8                padding[48];    // A bit more padding
#endif
};


//============================================================
//  TYPE DEFINITIONS
//============================================================

struct osd_thread {
	pthread_t           thread;
};

//============================================================
//  osd_event_alloc
//============================================================

osd_event *osd_event_alloc(int manualreset, int initialstate)
{
	osd_event *ev;
	pthread_mutexattr_t mtxattr;

	ev = (osd_event *)calloc(1, sizeof(osd_event));
	if (ev == NULL)
		return NULL;

	pthread_mutexattr_init(&mtxattr);
	pthread_mutex_init(&ev->mutex, &mtxattr);
	pthread_cond_init(&ev->cond, NULL);
	ev->signalled = initialstate;
	ev->autoreset = !manualreset;

	return ev;
}

//============================================================
//  osd_event_free
//============================================================

void osd_event_free(osd_event *event)
{
	pthread_mutex_destroy(&event->mutex);
	pthread_cond_destroy(&event->cond);
	free(event);
}

//============================================================
//  osd_event_set
//============================================================

void osd_event_set(osd_event *event)
{
	pthread_mutex_lock(&event->mutex);
	if (event->signalled == FALSE)
	{
		event->signalled = TRUE;
		if (event->autoreset)
			pthread_cond_signal(&event->cond);
		else
			pthread_cond_broadcast(&event->cond);
	}
	pthread_mutex_unlock(&event->mutex);
}

//============================================================
//  osd_event_reset
//============================================================

void osd_event_reset(osd_event *event)
{
	pthread_mutex_lock(&event->mutex);
	event->signalled = FALSE;
	pthread_mutex_unlock(&event->mutex);
}

//============================================================
//  osd_event_wait
//============================================================

int osd_event_wait(osd_event *event, osd_ticks_t timeout)
{
	if (timeout == OSD_EVENT_WAIT_INFINITE)
		timeout = osd_ticks_per_second() * (osd_ticks_t)10000;

	pthread_mutex_lock(&event->mutex);
	if (!timeout)
	{
		if (!event->signalled)
		{
				pthread_mutex_unlock(&event->mutex);
				return FALSE;
		}
	}
	else
	{
		if (!event->signalled)
		{
			struct timespec   ts;
			struct timeval    tp;
			UINT64 msec = timeout * 1000 / osd_ticks_per_second();
			UINT64 nsec;

			gettimeofday(&tp, NULL);

			ts.tv_sec  = tp.tv_sec;
			nsec = (UINT64) tp.tv_usec * (UINT64) 1000 + (msec * (UINT64) 1000000);
			ts.tv_nsec = nsec % (UINT64) 1000000000;
			ts.tv_sec += nsec / (UINT64) 1000000000;

			do {
				int ret = pthread_cond_timedwait(&event->cond, &event->mutex, &ts);
				if ( ret == ETIMEDOUT )
				{
					if (!event->signalled)
					{
						pthread_mutex_unlock(&event->mutex);
						return FALSE;
					}
					else
						break;
				}
				if (ret == 0)
					break;
				if ( ret != EINTR)
				{
					printf("Error %d while waiting for pthread_cond_timedwait:  %s\n", ret, strerror(ret));
				}

			} while (TRUE);
		}
	}

	if (event->autoreset)
		event->signalled = 0;

	pthread_mutex_unlock(&event->mutex);

	return TRUE;
}

//============================================================
//  osd_thread_create
//============================================================

osd_thread *osd_thread_create(osd_thread_callback callback, void *cbparam)
{
	osd_thread *thread;
	pthread_attr_t  attr;

	thread = (osd_thread *)calloc(1, sizeof(osd_thread));
	if (thread == NULL)
		return NULL;
	pthread_attr_init(&attr);
#ifndef SDLMAME_HAIKU
	pthread_attr_setinheritsched(&attr, PTHREAD_INHERIT_SCHED);
#endif
	if ( pthread_create(&thread->thread, &attr, callback, cbparam) != 0 )
	{
		free(thread);
		return NULL;
	}
	return thread;
}

//============================================================
//  osd_thread_adjust_priority
//============================================================

int osd_thread_adjust_priority(osd_thread *thread, int adjust)
{
	struct sched_param  sched;
	int                 policy;

	if ( pthread_getschedparam( thread->thread, &policy, &sched ) == 0 )
	{
		sched.sched_priority += adjust;
		if ( pthread_setschedparam(thread->thread, policy, &sched ) == 0)
			return TRUE;
		else
			return FALSE;
	}
	else
		return FALSE;
}

//============================================================
//  osd_thread_cpu_affinity
//============================================================

int osd_thread_cpu_affinity(osd_thread *thread, UINT32 mask)
{
	return TRUE;
}

//============================================================
//  osd_thread_wait_free
//============================================================

void osd_thread_wait_free(osd_thread *thread)
{
	pthread_join(thread->thread, NULL);
	free(thread);
}
