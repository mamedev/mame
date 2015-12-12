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

struct osd_lock {
	volatile pthread_t  holder;
	INT32               count;
#ifdef PTR64
	INT8                padding[52];    // Fill a 64-byte cache line
#else
	INT8                padding[56];    // A bit more padding
#endif
};

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

struct osd_scalable_lock
{
	struct
	{
		volatile INT32  haslock;        // do we have the lock?
		INT32           filler[64/4-1]; // assumes a 64-byte cache line
	} slot[WORK_MAX_THREADS];           // one slot per thread
	volatile INT32      nextindex;      // index of next slot to use
};


//============================================================
//  Scalable Locks
//============================================================

osd_scalable_lock *osd_scalable_lock_alloc(void)
{
	osd_scalable_lock *lock;

	lock = (osd_scalable_lock *)calloc(1, sizeof(*lock));
	if (lock == NULL)
		return NULL;

	memset(lock, 0, sizeof(*lock));
	lock->slot[0].haslock = TRUE;
	return lock;
}


INT32 osd_scalable_lock_acquire(osd_scalable_lock *lock)
{
	INT32 myslot = (atomic_increment32(&lock->nextindex) - 1) & (WORK_MAX_THREADS - 1);

#if defined(__i386__) || defined(__x86_64__)
	register INT32 tmp;
	__asm__ __volatile__ (
		"1: clr    %[tmp]             ;"
		"   xchg   %[haslock], %[tmp] ;"
		"   test   %[tmp], %[tmp]     ;"
		"   jne    3f                 ;"
		"2: mov    %[haslock], %[tmp] ;"
		"   test   %[tmp], %[tmp]     ;"
		"   jne    1b                 ;"
		"   pause                     ;"
		"   jmp    2b                 ;"
		"3:                            "
		: [haslock] "+m"  (lock->slot[myslot].haslock)
		, [tmp]     "=&r" (tmp)
		:
		: "cc"
	);
#elif defined(__ppc__) || defined (__PPC__) || defined(__ppc64__) || defined(__PPC64__)
	register INT32 tmp;
	__asm__ __volatile__ (
		"1: lwarx   %[tmp], 0, %[haslock] \n"
		"   cmpwi   %[tmp], 0             \n"
		"   bne     3f                    \n"
		"2: lwzx    %[tmp], 0, %[haslock] \n"
		"   cmpwi   %[tmp], 0             \n"
		"   bne     1b                    \n"
		"   nop                           \n"
		"   nop                           \n"
		"   b       2b                    \n"
		"3: li      %[tmp], 0             \n"
		"   stwcx.  %[tmp], 0, %[haslock] \n"
		"   bne-    1b                    \n"
		"   lwsync                        \n"
		: [tmp]     "=&r" (tmp)
		: [haslock] "r"   (&lock->slot[myslot].haslock)
		: "cr0"
	);
#else
	INT32 backoff = 1;
	while (!osd_compare_exchange32(&lock->slot[myslot].haslock, TRUE, FALSE))
	{
		INT32 backcount;
		for (backcount = 0; backcount < backoff; backcount++)
			osd_yield_processor();
		backoff <<= 1;
	}
#endif
	return myslot;
}


void osd_scalable_lock_release(osd_scalable_lock *lock, INT32 myslot)
{
#if defined(__i386__) || defined(__x86_64__)
	register INT32 tmp = TRUE;
	__asm__ __volatile__ (
		" xchg   %[haslock], %[tmp] ;"
		: [haslock] "+m" (lock->slot[(myslot + 1) & (WORK_MAX_THREADS - 1)].haslock)
		, [tmp]     "+r" (tmp)
		:
	);
#elif defined(__ppc__) || defined (__PPC__) || defined(__ppc64__) || defined(__PPC64__)
	lock->slot[(myslot + 1) & (WORK_MAX_THREADS - 1)].haslock = TRUE;
	__asm__ __volatile__ ( " lwsync " : : );
#else
	osd_exchange32(&lock->slot[(myslot + 1) & (WORK_MAX_THREADS - 1)].haslock, TRUE);
#endif
}

void osd_scalable_lock_free(osd_scalable_lock *lock)
{
	free(lock);
}

static inline pthread_t osd_compare_exchange_pthread_t(pthread_t volatile *ptr, pthread_t compare, pthread_t exchange)
{
#ifdef PTR64
	INT64 result = compare_exchange64((INT64 volatile *)ptr, (INT64)compare, (INT64)exchange);
#else
	INT32 result = compare_exchange32((INT32 volatile *)ptr, (INT32)compare, (INT32)exchange);
#endif
	return (pthread_t)result;
}

static inline pthread_t osd_exchange_pthread_t(pthread_t volatile *ptr, pthread_t exchange)
{
#ifdef PTR64
	INT64 result = osd_exchange64((INT64 volatile *)ptr, (INT64)exchange);
#else
	INT32 result = atomic_exchange32((INT32 volatile *)ptr, (INT32)exchange);
#endif
	return (pthread_t)result;
}


//============================================================
//  osd_lock_alloc
//============================================================

osd_lock *osd_lock_alloc(void)
{
	osd_lock *lock;

	lock = (osd_lock *)calloc(1, sizeof(osd_lock));
	if (lock == NULL)
		return NULL;

	lock->holder = 0;
	lock->count = 0;

	return lock;
}

//============================================================
//  osd_lock_acquire
//============================================================

void osd_lock_acquire(osd_lock *lock)
{
	pthread_t current, prev;

	current = pthread_self();
	prev = osd_compare_exchange_pthread_t(&lock->holder, 0, current);
	if (prev != nullptr && prev != current)
	{
		do {
			register INT32 spin = 10000; // Convenient spin count
			register pthread_t tmp;
#if defined(__i386__) || defined(__x86_64__)
			__asm__ __volatile__ (
				"1: pause                    ;"
				"   mov    %[holder], %[tmp] ;"
				"   test   %[tmp], %[tmp]    ;"
				"   loopne 1b                ;"
				: [spin]   "+c"  (spin)
				, [tmp]    "=&r" (tmp)
				: [holder] "m"   (lock->holder)
				: "cc"
			);
#elif defined(__ppc__) || defined(__PPC__)
			__asm__ __volatile__ (
				"1: nop                        \n"
				"   nop                        \n"
				"   lwzx  %[tmp], 0, %[holder] \n"
				"   cmpwi %[tmp], 0            \n"
				"   bdnzt eq, 1b               \n"
				: [spin]   "+c"  (spin)
				, [tmp]    "=&r" (tmp)
				: [holder] "r"   (&lock->holder)
				: "cr0"
			);
#elif defined(__ppc64__) || defined(__PPC64__)
			__asm__ __volatile__ (
				"1: nop                        \n"
				"   nop                        \n"
				"   ldx   %[tmp], 0, %[holder] \n"
				"   cmpdi %[tmp], 0            \n"
				"   bdnzt eq, 1b               \n"
				: [spin]   "+c"  (spin)
				, [tmp]    "=&r" (tmp)
				: [holder] "r"   (&lock->holder)
				: "cr0"
			);
#else
			while (--spin > 0 && lock->holder != NULL)
				osd_yield_processor();
#endif
#if 0
			/* If you mean to use locks as a blocking mechanism for extended
			 * periods of time, you should do something like this.  However,
			 * it kills the performance of gaelco3d.
			 */
			if (spin == 0)
			{
				struct timespec sleep = { 0, 100000 }, remaining;
				nanosleep(&sleep, &remaining); // sleep for 100us
			}
#endif
		} while (osd_compare_exchange_pthread_t(&lock->holder, 0, current) != nullptr);
	}
	lock->count++;
}

//============================================================
//  osd_lock_try
//============================================================

int osd_lock_try(osd_lock *lock)
{
	pthread_t current, prev;

	current = pthread_self();
	prev = osd_compare_exchange_pthread_t(&lock->holder, 0, current);
	if (prev == nullptr || prev == current)
	{
		lock->count++;
		return 1;
	}
	return 0;
}

//============================================================
//  osd_lock_release
//============================================================

void osd_lock_release(osd_lock *lock)
{
	pthread_t current;

	current = pthread_self();
	if (lock->holder == current)
	{
		if (--lock->count == 0)
#if defined(__ppc__) || defined(__PPC__) || defined(__ppc64__) || defined(__PPC64__)
		lock->holder = 0;
		__asm__ __volatile__( " lwsync " : : );
#else
		osd_exchange_pthread_t(&lock->holder, 0);
#endif
		return;
	}

	// trying to release a lock you don't hold is bad!
//  assert(lock->holder == pthread_self());
}

//============================================================
//  osd_lock_free
//============================================================

void osd_lock_free(osd_lock *lock)
{
	free(lock);
}

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
