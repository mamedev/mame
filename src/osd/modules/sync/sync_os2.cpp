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

// standard C headers
#include <math.h>
#include <unistd.h>

// MAME headers
#include "osdcore.h"
#include "osdsync.h"

#include "eminline.h"

#define INCL_DOS
#include <os2.h>

#include <stdlib.h>
#define pthread_t       int
#define pthread_self    _gettid

struct osd_event {
	HMTX                hmtx;
	HEV                 hev;
	volatile INT32      autoreset;
	INT8                padding[52];    // Fill a 64-byte cache line
};

//============================================================
//  TYPE DEFINITIONS
//============================================================

struct osd_thread {
	pthread_t           thread;
	osd_thread_callback callback;
	void *param;
};

//============================================================
//  osd_event_alloc
//============================================================

osd_event *osd_event_alloc(int manualreset, int initialstate)
{
	osd_event *ev;

	ev = (osd_event *)calloc(1, sizeof(osd_event));
	if (ev == NULL)
		return NULL;

	DosCreateMutexSem(NULL, &ev->hmtx, 0, FALSE);
	DosCreateEventSem(NULL, &ev->hev, 0, initialstate);
	ev->autoreset = !manualreset;

	return ev;
}

//============================================================
//  osd_event_free
//============================================================

void osd_event_free(osd_event *event)
{
	DosCloseMutexSem(event->hmtx);
	DosCloseEventSem(event->hev);
	free(event);
}

//============================================================
//  osd_event_set
//============================================================

void osd_event_set(osd_event *event)
{
	DosPostEventSem(event->hev);
}

//============================================================
//  osd_event_reset
//============================================================

void osd_event_reset(osd_event *event)
{
	ULONG ulCount;

	DosResetEventSem(event->hev, &ulCount);
}

//============================================================
//  osd_event_wait
//============================================================

int osd_event_wait(osd_event *event, osd_ticks_t timeout)
{
	ULONG rc;
	ULONG timeout_param;

	if (timeout == OSD_EVENT_WAIT_INFINITE)
		timeout_param = SEM_INDEFINITE_WAIT;
	else
		timeout_param = timeout * 1000 / osd_ticks_per_second();

	if(event->autoreset)
		DosRequestMutexSem(event->hmtx, -1);

	rc = DosWaitEventSem(event->hev, timeout_param);

	if(event->autoreset)
	{
		ULONG ulCount;

		if(rc == 0)
			DosResetEventSem(event->hev, &ulCount);

		DosReleaseMutexSem(event->hmtx);
	}

	return (rc == 0);
}

//============================================================
//  osd_thread_create
//============================================================

static void worker_thread_entry(void *param)
{
		osd_thread *thread = (osd_thread *) param;

		thread->callback(thread->param);
}

osd_thread *osd_thread_create(osd_thread_callback callback, void *cbparam)
{
	osd_thread *thread;

	thread = (osd_thread *)calloc(1, sizeof(osd_thread));
	if (thread == NULL)
		return NULL;
	thread->callback = callback;
	thread->param = cbparam;
	thread->thread = _beginthread(worker_thread_entry, NULL, 65535, thread);
	if ( thread->thread == -1 )
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
	PTIB ptib;

	DosGetInfoBlocks(&ptib, NULL);

	if ( DosSetPriority(PRTYS_THREAD, PRTYC_NOCHANGE,
						((BYTE)ptib->tib_ptib2->tib2_ulpri) + adjust, thread->thread ))
		return FALSE;


	return TRUE;
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
	TID tid = thread->thread;

	DosWaitThread(&tid, 0);
	free(thread);
}
