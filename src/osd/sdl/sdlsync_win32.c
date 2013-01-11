//============================================================
//
//  sdlsync.c - SDL core synchronization functions
//
//  Copyright (c) 1996-2010, Nicola Salmoria and the MAME Team.
//  Visit http://mamedev.org for licensing and usage restrictions.
//
//  SDLMAME by Olivier Galibert and R. Belmont
//
//============================================================

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <process.h>
#include <tchar.h>

#ifdef __GNUC__
#include <stdint.h>
#endif

// MAME headers
#include "osdcore.h"
#include "osinline.h"
#include "sdlsync.h"

#include "../windows/winsync.c"


//============================================================
//  DEBUGGING
//============================================================

#define USE_SCALABLE_LOCKS      (0)

struct osd_event
{
	void *  ptr;
};

struct osd_thread {
	HANDLE handle;
	osd_thread_callback callback;
	void *param;
};

//============================================================
//  osd_event_alloc
//============================================================

osd_event *osd_event_alloc(int manualreset, int initialstate)
{
	return (osd_event *) CreateEvent(NULL, manualreset, initialstate, NULL);
}

//============================================================
//  osd_event_free
//============================================================

void osd_event_free(osd_event *event)
{
	CloseHandle((HANDLE) event);
}

//============================================================
//  osd_event_set
//============================================================

void osd_event_set(osd_event *event)
{
	SetEvent((HANDLE) event);
}

//============================================================
//  osd_event_reset
//============================================================

void osd_event_reset(osd_event *event)
{
	ResetEvent((HANDLE) event);
}

//============================================================
//  osd_event_wait
//============================================================

int osd_event_wait(osd_event *event, osd_ticks_t timeout)
{
	int ret = WaitForSingleObject((HANDLE) event, timeout * 1000 / osd_ticks_per_second());
	return ( ret == WAIT_OBJECT_0);
}

//============================================================
//  Scalable Locks
//============================================================

struct osd_scalable_lock
{
#if USE_SCALABLE_LOCKS
	struct
	{
		volatile INT32  haslock;        // do we have the lock?
		INT32           filler[64/4-1]; // assumes a 64-byte cache line
	} slot[WORK_MAX_THREADS];           // one slot per thread
	volatile INT32      nextindex;      // index of next slot to use
#else
	CRITICAL_SECTION    section;
#endif
};

osd_scalable_lock *osd_scalable_lock_alloc(void)
{
	osd_scalable_lock *lock;

	lock = (osd_scalable_lock *)calloc(1, sizeof(*lock));

	memset(lock, 0, sizeof(*lock));
#if USE_SCALABLE_LOCKS
	lock->slot[0].haslock = TRUE;
#else
	InitializeCriticalSection(&lock->section);
#endif
	return lock;
}


INT32 osd_scalable_lock_acquire(osd_scalable_lock *lock)
{
#if USE_SCALABLE_LOCKS
	INT32 myslot = (interlocked_increment(&lock->nextindex) - 1) & (WORK_MAX_THREADS - 1);
	INT32 backoff = 1;

	while (!lock->slot[myslot].haslock)
	{
		INT32 backcount;
		for (backcount = 0; backcount < backoff; backcount++)
			YieldProcessor();
		backoff <<= 1;
	}
	lock->slot[myslot].haslock = FALSE;
	return myslot;
#else
	EnterCriticalSection(&lock->section);
	return 0;
#endif
}


void osd_scalable_lock_release(osd_scalable_lock *lock, INT32 myslot)
{
#if USE_SCALABLE_LOCKS
	interlocked_exchange32(&lock->slot[(myslot + 1) & (WORK_MAX_THREADS - 1)].haslock, TRUE);
#else
	LeaveCriticalSection(&lock->section);
#endif
}

void osd_scalable_lock_free(osd_scalable_lock *lock)
{
	free(lock);
}

//============================================================
//  osd_thread_create
//============================================================

static unsigned __stdcall worker_thread_entry(void *param)
{
	osd_thread *thread = (osd_thread *) param;
	void *res;
	res = thread->callback(thread->param);
#ifdef PTR64
	return (unsigned) (long long) res;
#else
	return (unsigned) res;
#endif
}

osd_thread *osd_thread_create(osd_thread_callback callback, void *cbparam)
{
	osd_thread *thread;
	uintptr_t handle;

	thread = (osd_thread *)calloc(1, sizeof(osd_thread));
	thread->callback = callback;
	thread->param = cbparam;
	handle = _beginthreadex(NULL, 0, worker_thread_entry, thread, 0, NULL);
	thread->handle = (HANDLE) handle;
	return thread;
}

//============================================================
//  osd_thread_wait_free
//============================================================

void osd_thread_wait_free(osd_thread *thread)
{
	WaitForSingleObject(thread->handle, INFINITE);
	CloseHandle(thread->handle);
	free(thread);
}

//============================================================
//  osd_thread_adjust_priority
//============================================================

int osd_thread_adjust_priority(osd_thread *thread, int adjust)
{
	if (adjust)
		SetThreadPriority(thread->handle, THREAD_PRIORITY_ABOVE_NORMAL);
	else
		SetThreadPriority(thread->handle, GetThreadPriority(GetCurrentThread()));
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
//  osd_process_kill
//============================================================

void osd_process_kill(void)
{
	TerminateProcess(GetCurrentProcess(), -1);
}
