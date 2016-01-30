// license:BSD-3-Clause
// copyright-holders:Aaron Giles
//============================================================
//
//  winsync.c - Win32 OSD core synchronization functions
//
//============================================================

// standard windows headers
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdlib.h>
#include <process.h>

// MAME headers
#include "osdcore.h"
#include "eminline.h"
#include "osdsync.h"


//============================================================
//  DEBUGGING
//============================================================

#define DEBUG_SLOW_LOCKS    0


//============================================================
//  TYPE DEFINITIONS
//============================================================

typedef BOOL (WINAPI *try_enter_critical_section_ptr)(LPCRITICAL_SECTION lpCriticalSection);

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
//  win_compare_exchange32
//============================================================

INT32 win_compare_exchange32(INT32 volatile *ptr, INT32 compare, INT32 exchange)
{
	return InterlockedCompareExchange((LPLONG)ptr, (LONG)exchange, (LONG)compare);
}


//============================================================
//  win_compare_exchange64
//============================================================

#ifdef PTR64
INT64 win_compare_exchange64(INT64 volatile *ptr, INT64 compare, INT64 exchange)
{
	return InterlockedCompareExchange64((LONGLONG*)ptr, (LONGLONG)exchange, (LONGLONG)compare);
}
#endif


//============================================================
//  win_atomic_exchange32
//============================================================

INT32 win_atomic_exchange32(INT32 volatile *ptr, INT32 exchange)
{
	return InterlockedExchange((LONG *) ptr, exchange);
}


//============================================================
//  win_atomic_add32
//============================================================

INT32 win_atomic_add32(INT32 volatile *ptr, INT32 delta)
{
	return InterlockedExchangeAdd((LONG *) ptr, delta) + delta;
}

//============================================================
//  osd_event_alloc
//============================================================

osd_event *osd_event_alloc(int manualreset, int initialstate)
{
	return (osd_event *) CreateEvent(nullptr, manualreset, initialstate, nullptr);
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
	DWORD timeout_param;
	if (timeout == OSD_EVENT_WAIT_INFINITE)
		timeout_param = INFINITE;
	else
		timeout_param = timeout * 1000 / osd_ticks_per_second();

	int ret = WaitForSingleObject((HANDLE) event, timeout_param);
	return (ret == WAIT_OBJECT_0);
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
	if (thread == nullptr)
		return nullptr;
	thread->callback = callback;
	thread->param = cbparam;
	handle = _beginthreadex(nullptr, 0, worker_thread_entry, thread, 0, nullptr);
	if (handle == 0)
	{
		free(thread);
		return nullptr;
	}
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
