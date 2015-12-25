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
		: "%cc"
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
		"   sync                          \n"
		"   stwcx.  %[tmp], 0, %[haslock] \n"
		"   bne-    1b                    \n"
		"   eieio                         \n"
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
	__asm__ __volatile__ ( " eieio " : : );
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
	if (prev != (size_t)NULL && prev != current)
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
				: "%cc"
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
		} while (osd_compare_exchange_pthread_t(&lock->holder, 0, current) != (size_t)NULL);
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
	if (prev == (size_t)NULL || prev == current)
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
		__asm__ __volatile__( " eieio " : : );
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
