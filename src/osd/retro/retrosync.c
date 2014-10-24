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

#ifndef _GNU_SOURCE
#define _GNU_SOURCE     // for PTHREAD_MUTEX_RECURSIVE; needs to be here before other glibc headers are included
#endif

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <process.h>
#endif

// standard C headers
#include <math.h>
#include <stdint.h>

#ifndef WIN32
#include <unistd.h>
#endif

// MAME headers
#include "osdcomm.h"
#include "osdcore.h"

#include "retrosync.h"
#include <stdlib.h>

#ifndef WIN32
#include <pthread.h>
#endif

#include <errno.h>
#include <signal.h>
#include <sys/time.h>

#ifndef WIN32
struct hidden_mutex_t {
	pthread_mutex_t id;
};
#endif

struct osd_event {
#ifndef WIN32
	pthread_mutex_t     mutex;
	pthread_cond_t      cond;
	volatile INT32      autoreset;
	volatile INT32      signalled;
#ifdef PTR64
	INT8                padding[40];    // Fill a 64-byte cache line
#else
	INT8                padding[48];    // A bit more padding
#endif

#else
	void *  ptr;
#endif

};

//============================================================
//  TYPE DEFINITIONS
//============================================================
#ifdef WIN32
typedef BOOL (WINAPI *try_enter_critical_section_ptr)(LPCRITICAL_SECTION lpCriticalSection);
static try_enter_critical_section_ptr try_enter_critical_section = NULL;
static int checked_for_try_enter = FALSE;

struct osd_lock
{
	CRITICAL_SECTION    critsect;
};
#endif

struct osd_thread {
#ifdef WIN32
   HANDLE handle;
   osd_thread_callback callback;
   void *params;
#else
	pthread_t           thread;
#endif
};

struct osd_scalable_lock
{
#ifdef WIN32
	CRITICAL_SECTION    critsect;
#else
	osd_lock            *lock;
#endif
};

//============================================================
//  Scalable Locks
//============================================================

osd_scalable_lock *osd_scalable_lock_alloc(void)
{
	osd_scalable_lock *lock;

	lock = (osd_scalable_lock *)calloc(1, sizeof(*lock));

#ifdef WIN32
	memset(lock, 0, sizeof(*lock));
	InitializeCriticalSection(&lock->critsect);
#else
	lock->lock = osd_lock_alloc();
#endif
	return lock;
}


INT32 osd_scalable_lock_acquire(osd_scalable_lock *lock)
{
#if defined(WIN32)
   EnterCriticalSection(&lock->critsect);
#else
	osd_lock_acquire(lock->lock);
#endif
	return 0;
}


void osd_scalable_lock_release(osd_scalable_lock *lock, INT32 myslot)
{
#ifdef WIN32
	LeaveCriticalSection(&lock->critsect);
#else
	osd_lock_release(lock->lock);
#endif
}

void osd_scalable_lock_free(osd_scalable_lock *lock)
{
#ifdef WIN32
	DeleteCriticalSection(&lock->critsect);
#else
	osd_lock_free(lock->lock);
#endif
	free(lock);
}


//============================================================
//  osd_lock_alloc
//============================================================

osd_lock *osd_lock_alloc(void)
{
#ifdef WIN32
	osd_lock *lock = (osd_lock *)malloc(sizeof(*lock));
	if (lock == NULL)
		return NULL;
	InitializeCriticalSection(&lock->critsect);
	return lock;
#else
	hidden_mutex_t *mutex;
	pthread_mutexattr_t mtxattr;

	mutex = (hidden_mutex_t *)calloc(1, sizeof(hidden_mutex_t));

	pthread_mutexattr_init(&mtxattr);
	pthread_mutexattr_settype(&mtxattr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&mutex->id, &mtxattr);

	return (osd_lock *)mutex;
#endif
}

//============================================================
//  osd_lock_acquire
//============================================================

void osd_lock_acquire(osd_lock *lock)
{
#ifdef WIN32
	// block until we can acquire the lock
	EnterCriticalSection(&lock->critsect);
#else
	hidden_mutex_t *mutex = (hidden_mutex_t *) lock;
	int r;

	r = pthread_mutex_lock(&mutex->id);
	if (r==0)
		return;
#endif
}

//============================================================
//  osd_lock_try
//============================================================

int osd_lock_try(osd_lock *lock)
{
#ifdef WIN32
	int result = TRUE;

	// if we haven't yet checked for the TryEnter API, do it now
	if (!checked_for_try_enter)
	{
		// see if we can use TryEnterCriticalSection
		HMODULE library = LoadLibrary(TEXT("kernel32.dll"));
		if (library != NULL)
			try_enter_critical_section = (try_enter_critical_section_ptr)GetProcAddress(library, "TryEnterCriticalSection");
		checked_for_try_enter = TRUE;
	}

	// if we have it, use it, otherwise just block
	if (try_enter_critical_section != NULL)
		result = (*try_enter_critical_section)(&lock->critsect);
	else
		EnterCriticalSection(&lock->critsect);
	return result;
#else
	hidden_mutex_t *mutex = (hidden_mutex_t *) lock;
	int r;

	r = pthread_mutex_trylock(&mutex->id);
	if (r==0)
		return 1;
	return 0;
#endif
}

//============================================================
//  osd_lock_release
//============================================================

void osd_lock_release(osd_lock *lock)
{
#ifdef WIN32
	LeaveCriticalSection(&lock->critsect);
#else
	hidden_mutex_t *mutex = (hidden_mutex_t *) lock;

	pthread_mutex_unlock(&mutex->id);
#endif
}

//============================================================
//  osd_lock_free
//============================================================

void osd_lock_free(osd_lock *lock)
{
#ifdef WIN32
	DeleteCriticalSection(&lock->critsect);
	free(lock);
#else
	hidden_mutex_t *mutex = (hidden_mutex_t *) lock;

	pthread_mutex_unlock(&mutex->id);
	pthread_mutex_destroy(&mutex->id);
	free(mutex);
#endif
}

//============================================================
//  osd_event_alloc
//============================================================

osd_event *osd_event_alloc(int manualreset, int initialstate)
{
#ifdef WIN32
	return (osd_event *) CreateEvent(NULL, manualreset, initialstate, NULL);
#else
	osd_event *ev;
	pthread_mutexattr_t mtxattr;

	ev = (osd_event *)calloc(1, sizeof(osd_event));

	pthread_mutexattr_init(&mtxattr);
	pthread_mutex_init(&ev->mutex, &mtxattr);
	pthread_cond_init(&ev->cond, NULL);
	ev->signalled = initialstate;
	ev->autoreset = !manualreset;

	return ev;
#endif
}

//============================================================
//  osd_event_free
//============================================================

void osd_event_free(osd_event *event)
{
#ifndef WIN32
	pthread_mutex_destroy(&event->mutex);
	pthread_cond_destroy(&event->cond);
	free(event);
#else
	CloseHandle((HANDLE) event);
#endif
}

//============================================================
//  osd_event_set
//============================================================

void osd_event_set(osd_event *event)
{
#ifndef WIN32
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
#else
	SetEvent((HANDLE) event);
#endif
	
	
}

//============================================================
//  osd_event_reset
//============================================================

void osd_event_reset(osd_event *event)
{
#ifndef WIN32
	pthread_mutex_lock(&event->mutex);
	event->signalled = FALSE;
	pthread_mutex_unlock(&event->mutex);
#else
	ResetEvent((HANDLE) event);	
#endif
}

//============================================================
//  osd_event_wait
//============================================================

int osd_event_wait(osd_event *event, osd_ticks_t timeout)
{
#ifdef WIN32
	int ret = WaitForSingleObject((HANDLE) event, timeout * 1000 / osd_ticks_per_second());
	return ( ret == WAIT_OBJECT_0);
#else

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
#endif
}

//============================================================
//  osd_thread_create
//============================================================
#ifdef WIN32
static unsigned __stdcall worker_thread_entry(void *param)
{
   osd_thread *thread = (osd_thread *) param;
   void *res;
   res = thread->callback(thread->params);
#ifdef PTR64
   return (unsigned) (long long) res;
#else
   return (unsigned) res;
#endif
}
#endif

osd_thread *osd_thread_create(osd_thread_callback callback, void *cbparam)
{
	osd_thread *thread;
#ifndef WIN32
	pthread_attr_t  attr;
#else
	uintptr_t handle;
#endif

	thread = (osd_thread *)calloc(1, sizeof(osd_thread));
#ifdef WIN32
   thread->callback = callback;
   thread->params = cbparam;
   handle = _beginthreadex(NULL, 0, worker_thread_entry, thread, 0, NULL);
   thread->handle = (HANDLE)handle;
#else
	pthread_attr_init(&attr);
#ifndef RETRO_AND
	pthread_attr_setinheritsched(&attr, PTHREAD_INHERIT_SCHED);
#endif
	if ( pthread_create(&thread->thread, &attr, callback, cbparam) != 0 )
	{
		free(thread);
		return NULL;
	}
#endif
	return thread;
}

//============================================================
//  osd_thread_adjust_priority
//============================================================

int osd_thread_adjust_priority(osd_thread *thread, int adjust)
{
#ifdef WIN32
   if (adjust)
      SetThreadPriority(thread->handle, THREAD_PRIORITY_ABOVE_NORMAL);
   else
      SetThreadPriority(thread->handle, GetThreadPriority(GetCurrentThread()));
   return TRUE;
#else
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
#endif
}

//============================================================
//  osd_thread_cpu_affinity
//============================================================

int osd_thread_cpu_affinity(osd_thread *thread, UINT32 mask)
{
#if defined(__GNUC__) && defined(WIN32)
   return TRUE; /* stub */
#elif !defined(NO_AFFINITY_NP) && !defined(__MACH__) && !defined(RETRO_AND)
	cpu_set_t   cmask;
	pthread_t   lthread;
	int         bitnum;

	CPU_ZERO(&cmask);
	for (bitnum=0; bitnum<32; bitnum++)
		if (mask & (1<<bitnum))
			CPU_SET(bitnum, &cmask);

	if (thread == NULL)
		lthread = pthread_self();
	else
		lthread = thread->thread;

	if (pthread_setaffinity_np(lthread, sizeof(cmask), &cmask) <0)
	{
		/* Not available during link in all targets */
		fprintf(stderr, "error %d setting cpu affinity to mask %08x", errno, mask);
		return FALSE;
	}
	else
		return TRUE;
#else
	return TRUE;
#endif
}

//============================================================
//  osd_thread_wait_free
//============================================================

void osd_thread_wait_free(osd_thread *thread)
{
#if defined(WIN32)
   WaitForSingleObject(thread->handle, INFINITE);
   CloseHandle(thread->handle);
   free(thread);
#else
	pthread_join(thread->thread, NULL);
	free(thread);
#endif
}

//============================================================
//  osd_process_kill
//============================================================

void osd_process_kill(void)
{
#if defined(WIN32)
   TerminateProcess(GetCurrentProcess(), -1);
#else
	kill(getpid(), SIGKILL);
#endif
}
