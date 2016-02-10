// license:BSD-3-Clause
// copyright-holders:Aaron Giles
//============================================================
//
//  sdlwork.c - SDL OSD core work item functions
//
//============================================================

#if defined(OSD_WINDOWS)
// standard windows headers
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <process.h>
#include <tchar.h>
#include <stdlib.h>

#ifdef __GNUC__
#include <stdint.h>
#endif
#endif
#include <mutex>

// MAME headers
#include "osdcore.h"

#include "modules/sync/osdsync.h"
#include "modules/lib/osdlib.h"

#include "eminline.h"

#if defined(SDLMAME_MACOSX)
#include "osxutils.h"
#endif

#if defined(OSD_SDL)
typedef void *PVOID;
#endif

//============================================================
//  DEBUGGING
//============================================================

#define KEEP_STATISTICS         (0)

//============================================================
//  PARAMETERS
//============================================================

#define ENV_PROCESSORS               "OSDPROCESSORS"
#define ENV_WORKQUEUEMAXTHREADS      "OSDWORKQUEUEMAXTHREADS"

#define SPIN_LOOP_TIME          (osd_ticks_per_second() / 10000)

//============================================================
//  MACROS
//============================================================

#if KEEP_STATISTICS
#define add_to_stat(v,x)        do { atomic_add32((v), (x)); } while (0)
#define begin_timing(v)         do { (v) -= get_profile_ticks(); } while (0)
#define end_timing(v)           do { (v) += get_profile_ticks(); } while (0)
#else
#define add_to_stat(v,x)        do { } while (0)
#define begin_timing(v)         do { } while (0)
#define end_timing(v)           do { } while (0)
#endif

template<typename _PtrType>
static void spin_while(const volatile _PtrType * volatile ptr, const _PtrType val, const osd_ticks_t timeout, const int invert = 0)
{
	osd_ticks_t stopspin = osd_ticks() + timeout;

	do {
		int spin = 10000;
		while (--spin)
		{
			if ((*ptr != val) ^ invert)
				return;
		}
	} while (((*ptr == val) ^ invert) && osd_ticks() < stopspin);
}

template<typename _PtrType>
static void spin_while_not(const volatile _PtrType * volatile ptr, const _PtrType val, const osd_ticks_t timeout)
{
	spin_while(ptr, val, timeout, 1);
}


//============================================================
//  TYPE DEFINITIONS
//============================================================

struct work_thread_info
{
	osd_work_queue *    queue;          // pointer back to the queue
	osd_thread *        handle;         // handle to the thread
	osd_event *         wakeevent;      // wake event for the thread
	volatile INT32      active;         // are we actively processing work?

#if KEEP_STATISTICS
	INT32               itemsdone;
	osd_ticks_t         actruntime;
	osd_ticks_t         runtime;
	osd_ticks_t         spintime;
	osd_ticks_t         waittime;
#endif
};


struct osd_work_queue
{
	std::mutex			*lock;           // lock for protecting the queue
	osd_work_item * volatile list;      // list of items in the queue
	osd_work_item ** volatile tailptr;  // pointer to the tail pointer of work items in the queue
	osd_work_item * volatile free;      // free list of work items
	volatile INT32      items;          // items in the queue
	volatile INT32      livethreads;    // number of live threads
	volatile INT32      waiting;        // is someone waiting on the queue to complete?
	volatile INT32      exiting;        // should the threads exit on their next opportunity?
	UINT32              threads;        // number of threads in this queue
	UINT32              flags;          // creation flags
	work_thread_info *  thread;         // array of thread information
	osd_event   *       doneevent;      // event signalled when work is complete

#if KEEP_STATISTICS
	volatile INT32      itemsqueued;    // total items queued
	volatile INT32      setevents;      // number of times we called SetEvent
	volatile INT32      extraitems;     // how many extra items we got after the first in the queue loop
	volatile INT32      spinloops;      // how many times spinning bought us more items
#endif
};


struct osd_work_item
{
	osd_work_item *     next;           // pointer to next item
	osd_work_queue *    queue;          // pointer back to the owning queue
	osd_work_callback   callback;       // callback function
	void *              param;          // callback parameter
	void *              result;         // callback result
	osd_event *         event;          // event signalled when complete
	UINT32              flags;          // creation flags
	volatile INT32      done;           // is the item done?
};

//============================================================
//  GLOBAL VARIABLES
//============================================================

int osd_num_processors = 0;

//============================================================
//  FUNCTION PROTOTYPES
//============================================================

static int effective_num_processors(void);
static void * worker_thread_entry(void *param);
static void worker_thread_process(osd_work_queue *queue, work_thread_info *thread);
static bool queue_has_list_items(osd_work_queue *queue);


//============================================================
//  osd_work_queue_alloc
//============================================================

osd_work_queue *osd_work_queue_alloc(int flags)
{
	int threadnum;
	int numprocs = effective_num_processors();
	osd_work_queue *queue;
	int osdthreadnum = 0;
	int allocthreadnum;
	const char *osdworkqueuemaxthreads = osd_getenv(ENV_WORKQUEUEMAXTHREADS);

	// allocate a new queue
	queue = (osd_work_queue *)osd_malloc(sizeof(*queue));
	if (queue == NULL)
		goto error;
	memset(queue, 0, sizeof(*queue));

	// initialize basic queue members
	queue->tailptr = (osd_work_item **)&queue->list;
	queue->flags = flags;

	// allocate events for the queue
	queue->doneevent = osd_event_alloc(TRUE, TRUE);     // manual reset, signalled
	if (queue->doneevent == NULL)
		goto error;

	// initialize the critical section
	queue->lock = new std::mutex();
	if (queue->lock == NULL)
		goto error;

	// determine how many threads to create...
	// on a single-CPU system, create 1 thread for I/O queues, and 0 threads for everything else
	if (numprocs == 1)
		threadnum = (flags & WORK_QUEUE_FLAG_IO) ? 1 : 0;
	// on an n-CPU system, create n-1 threads for multi queues, and 1 thread for everything else
	else
		threadnum = (flags & WORK_QUEUE_FLAG_MULTI) ? (numprocs - 1) : 1;

	if (osdworkqueuemaxthreads != NULL && sscanf(osdworkqueuemaxthreads, "%d", &osdthreadnum) == 1 && threadnum > osdthreadnum)
		threadnum = osdthreadnum;

	// clamp to the maximum
	queue->threads = MIN(threadnum, WORK_MAX_THREADS);

	// allocate memory for thread array (+1 to count the calling thread if WORK_QUEUE_FLAG_MULTI)
	if (flags & WORK_QUEUE_FLAG_MULTI)
		allocthreadnum = queue->threads + 1;
	else
		allocthreadnum = queue->threads;

#if KEEP_STATISTICS
	printf("osdprocs: %d effecprocs: %d threads: %d allocthreads: %d osdthreads: %d maxthreads: %d queuethreads: %d\n", osd_num_processors, numprocs, threadnum, allocthreadnum, osdthreadnum, WORK_MAX_THREADS, queue->threads);
#endif

	queue->thread = (work_thread_info *)osd_malloc_array(allocthreadnum * sizeof(queue->thread[0]));
	if (queue->thread == NULL)
		goto error;
	memset(queue->thread, 0, allocthreadnum * sizeof(queue->thread[0]));

	// iterate over threads
	for (threadnum = 0; threadnum < queue->threads; threadnum++)
	{
		work_thread_info *thread = &queue->thread[threadnum];

		// set a pointer back to the queue
		thread->queue = queue;

		// create the per-thread wake event
		thread->wakeevent = osd_event_alloc(FALSE, FALSE);  // auto-reset, not signalled
		if (thread->wakeevent == NULL)
			goto error;

		// create the thread
		thread->handle = osd_thread_create(worker_thread_entry, thread);
		if (thread->handle == NULL)
			goto error;

		// set its priority: I/O threads get high priority because they are assumed to be
		// blocked most of the time; other threads just match the creator's priority
		if (flags & WORK_QUEUE_FLAG_IO)
			osd_thread_adjust_priority(thread->handle, 1);
		else
			osd_thread_adjust_priority(thread->handle, 0);
	}

	// start a timer going for "waittime" on the main thread
	if (flags & WORK_QUEUE_FLAG_MULTI)
	{
		begin_timing(queue->thread[queue->threads].waittime);
	}
	return queue;

error:
	osd_work_queue_free(queue);
	return NULL;
}


//============================================================
//  osd_work_queue_items
//============================================================

int osd_work_queue_items(osd_work_queue *queue)
{
	// return the number of items currently in the queue
	return queue->items;
}


//============================================================
//  osd_work_queue_wait
//============================================================

int osd_work_queue_wait(osd_work_queue *queue, osd_ticks_t timeout)
{
	// if no threads, no waiting
	if (queue->threads == 0)
		return TRUE;

	// if no items, we're done
	if (queue->items == 0)
		return TRUE;

	// if this is a multi queue, help out rather than doing nothing
	if (queue->flags & WORK_QUEUE_FLAG_MULTI)
	{
		work_thread_info *thread = &queue->thread[queue->threads];

		end_timing(thread->waittime);

		// process what we can as a worker thread
		worker_thread_process(queue, thread);

		// if we're a high frequency queue, spin until done
		if (queue->flags & WORK_QUEUE_FLAG_HIGH_FREQ && queue->items != 0)
		{
			// spin until we're done
			begin_timing(thread->spintime);
			spin_while_not(&queue->items, 0, timeout);
			end_timing(thread->spintime);

			begin_timing(thread->waittime);
			return (queue->items == 0);
		}
		begin_timing(thread->waittime);
	}

	// reset our done event and double-check the items before waiting
	osd_event_reset(queue->doneevent);
	atomic_exchange32(&queue->waiting, TRUE);
	if (queue->items != 0)
		osd_event_wait(queue->doneevent, timeout);
	atomic_exchange32(&queue->waiting, FALSE);

	// return TRUE if we actually hit 0
	return (queue->items == 0);
}


//============================================================
//  osd_work_queue_free
//============================================================

void osd_work_queue_free(osd_work_queue *queue)
{
	// if we have threads, clean them up
	if (queue->thread != NULL)
	{
		int threadnum;

		// stop the timer for "waittime" on the main thread
		if (queue->flags & WORK_QUEUE_FLAG_MULTI)
		{
			end_timing(queue->thread[queue->threads].waittime);
		}

		// signal all the threads to exit
		atomic_exchange32(&queue->exiting, TRUE);
		for (threadnum = 0; threadnum < queue->threads; threadnum++)
		{
			work_thread_info *thread = &queue->thread[threadnum];
			if (thread->wakeevent != NULL)
				osd_event_set(thread->wakeevent);
		}

		// wait for all the threads to go away
		for (threadnum = 0; threadnum < queue->threads; threadnum++)
		{
			work_thread_info *thread = &queue->thread[threadnum];

			// block on the thread going away, then close the handle
			if (thread->handle != NULL)
			{
				osd_thread_wait_free(thread->handle);
			}

			// clean up the wake event
			if (thread->wakeevent != NULL)
				osd_event_free(thread->wakeevent);
		}

#if KEEP_STATISTICS
		int allocthreadnum;
		if (queue->flags & WORK_QUEUE_FLAG_MULTI)
			allocthreadnum = queue->threads + 1;
		else
			allocthreadnum = queue->threads;

		// output per-thread statistics
		for (threadnum = 0; threadnum < allocthreadnum; threadnum++)
		{
			work_thread_info *thread = &queue->thread[threadnum];
			osd_ticks_t total = thread->runtime + thread->waittime + thread->spintime;
			printf("Thread %d:  items=%9d run=%5.2f%% (%5.2f%%)  spin=%5.2f%%  wait/other=%5.2f%% total=%9d\n",
					threadnum, thread->itemsdone,
					(double)thread->runtime * 100.0 / (double)total,
					(double)thread->actruntime * 100.0 / (double)total,
					(double)thread->spintime * 100.0 / (double)total,
					(double)thread->waittime * 100.0 / (double)total,
					(UINT32) total);
		}
#endif
	}

	// free the list
	if (queue->thread != NULL)
		osd_free(queue->thread);

	// free all the events
	if (queue->doneevent != NULL)
		osd_event_free(queue->doneevent);

	// free all items in the free list
	while (queue->free != NULL)
	{
		osd_work_item *item = (osd_work_item *)queue->free;
		queue->free = item->next;
		if (item->event != NULL)
			osd_event_free(item->event);
		osd_free(item);
	}

	// free all items in the active list
	while (queue->list != NULL)
	{
		osd_work_item *item = (osd_work_item *)queue->list;
		queue->list = item->next;
		if (item->event != NULL)
			osd_event_free(item->event);
		osd_free(item);
	}

#if KEEP_STATISTICS
	printf("Items queued   = %9d\n", queue->itemsqueued);
	printf("SetEvent calls = %9d\n", queue->setevents);
	printf("Extra items    = %9d\n", queue->extraitems);
	printf("Spin loops     = %9d\n", queue->spinloops);
#endif

	delete queue->lock;
	// free the queue itself
	osd_free(queue);
}


//============================================================
//  osd_work_item_queue_multiple
//============================================================

osd_work_item *osd_work_item_queue_multiple(osd_work_queue *queue, osd_work_callback callback, INT32 numitems, void *parambase, INT32 paramstep, UINT32 flags)
{
	osd_work_item *itemlist = NULL, *lastitem = NULL;
	osd_work_item **item_tailptr = &itemlist;
	int itemnum;

	// loop over items, building up a local list of work
	for (itemnum = 0; itemnum < numitems; itemnum++)
	{
		osd_work_item *item;

		// first allocate a new work item; try the free list first
		{
			queue->lock->lock();
			do
			{
				item = (osd_work_item *)queue->free;
			} while (item != NULL && compare_exchange_ptr((PVOID volatile *)&queue->free, item, item->next) != item);
			queue->lock->unlock();
		}

		// if nothing, allocate something new
		if (item == NULL)
		{
			// allocate the item
			item = (osd_work_item *)osd_malloc(sizeof(*item));
			if (item == NULL)
				return NULL;
			item->event = NULL;
			item->queue = queue;
			item->done = FALSE;
		}
		else
		{
			atomic_exchange32(&item->done, FALSE); // needs to be set this way to prevent data race/usage of uninitialized memory on Linux
		}

		// fill in the basics
		item->next = NULL;
		item->callback = callback;
		item->param = parambase;
		item->result = NULL;
		item->flags = flags;

		// advance to the next
		lastitem = item;
		*item_tailptr = item;
		item_tailptr = &item->next;
		parambase = (UINT8 *)parambase + paramstep;
	}

	// enqueue the whole thing within the critical section
	{
		queue->lock->lock();
		*queue->tailptr = itemlist;
		queue->tailptr = item_tailptr;
		queue->lock->unlock();
	}

	// increment the number of items in the queue
	atomic_add32(&queue->items, numitems);
	add_to_stat(&queue->itemsqueued, numitems);

	// look for free threads to do the work
	if (queue->livethreads < queue->threads)
	{
		int threadnum;

		// iterate over all the threads
		for (threadnum = 0; threadnum < queue->threads; threadnum++)
		{
			work_thread_info *thread = &queue->thread[threadnum];

			// if this thread is not active, wake him up
			if (!thread->active)
			{
				osd_event_set(thread->wakeevent);
				add_to_stat(&queue->setevents, 1);

				// for non-shared, the first one we find is good enough
				if (--numitems == 0)
					break;
			}
		}
	}

	// if no threads, run the queue now on this thread
	if (queue->threads == 0)
	{
		end_timing(queue->thread[0].waittime);
		worker_thread_process(queue, &queue->thread[0]);
		begin_timing(queue->thread[0].waittime);
	}
	// only return the item if it won't get released automatically
	return (flags & WORK_ITEM_FLAG_AUTO_RELEASE) ? NULL : lastitem;
}


//============================================================
//  osd_work_item_wait
//============================================================

int osd_work_item_wait(osd_work_item *item, osd_ticks_t timeout)
{
	// if we're done already, just return
	if (item->done)
		return TRUE;

	// if we don't have an event, create one
	if (item->event == NULL)
	{
		item->queue->lock->lock();
		item->event = osd_event_alloc(TRUE, FALSE);     // manual reset, not signalled
		item->queue->lock->unlock();
	}
	else
		osd_event_reset(item->event);

	// if we don't have an event, we need to spin (shouldn't ever really happen)
	if (item->event == NULL)
	{
		// TODO: do we need to measure the spin time here as well? and how can we do it?
		spin_while(&item->done, 0, timeout);
	}

	// otherwise, block on the event until done
	else if (!item->done)
		osd_event_wait(item->event, timeout);

	// return TRUE if the refcount actually hit 0
	return item->done;
}


//============================================================
//  osd_work_item_result
//============================================================

void *osd_work_item_result(osd_work_item *item)
{
	return item->result;
}


//============================================================
//  osd_work_item_release
//============================================================

void osd_work_item_release(osd_work_item *item)
{
	osd_work_item *next;

	// make sure we're done first
	osd_work_item_wait(item, 100 * osd_ticks_per_second());

	// add us to the free list on our queue
	item->queue->lock->lock();
	do
	{
		next = (osd_work_item *)item->queue->free;
		item->next = next;
	} while (compare_exchange_ptr((PVOID volatile *)&item->queue->free, next, item) != next);
	item->queue->lock->unlock();
}


//============================================================
//  effective_num_processors
//============================================================

static int effective_num_processors(void)
{
	int physprocs = osd_get_num_processors();

	// osd_num_processors == 0 for 'auto'
	if (osd_num_processors > 0)
	{
		return MIN(4 * physprocs, osd_num_processors);
	}
	else
	{
		int numprocs = 0;

		// if the OSDPROCESSORS environment variable is set, use that value if valid
		// note that we permit more than the real number of processors for testing
		const char *procsoverride = osd_getenv(ENV_PROCESSORS);
		if (procsoverride != NULL && sscanf(procsoverride, "%d", &numprocs) == 1 && numprocs > 0)
			return MIN(4 * physprocs, numprocs);

		// otherwise, return the info from the system
		return physprocs;
	}
}


//============================================================
//  worker_thread_entry
//============================================================

static void *worker_thread_entry(void *param)
{
	work_thread_info *thread = (work_thread_info *)param;
	osd_work_queue *queue = thread->queue;

#if defined(SDLMAME_MACOSX)
	void *arp = NewAutoreleasePool();
#endif

	// loop until we exit
	for ( ;; )
	{
		// block waiting for work or exit
		// bail on exit, and only wait if there are no pending items in queue
		if (queue->exiting)
			break;

		if (!queue_has_list_items(queue))
		{
			begin_timing(thread->waittime);
			osd_event_wait(thread->wakeevent, OSD_EVENT_WAIT_INFINITE);
			end_timing(thread->waittime);
		}

		if (queue->exiting)
			break;

		// indicate that we are live
		atomic_exchange32(&thread->active, TRUE);
		atomic_increment32(&queue->livethreads);

		// process work items
		for ( ;; )
		{
			// process as much as we can
			worker_thread_process(queue, thread);

			// if we're a high frequency queue, spin for a while before giving up
			if (queue->flags & WORK_QUEUE_FLAG_HIGH_FREQ && queue->list == NULL)
			{
				// spin for a while looking for more work
				begin_timing(thread->spintime);
				spin_while(&queue->list, (osd_work_item *)NULL, SPIN_LOOP_TIME);
				end_timing(thread->spintime);
			}

			// if nothing more, release the processor
			if (!queue_has_list_items(queue))
				break;
			add_to_stat(&queue->spinloops, 1);
		}

		// decrement the live thread count
		atomic_exchange32(&thread->active, FALSE);
		atomic_decrement32(&queue->livethreads);
	}

#if defined(SDLMAME_MACOSX)
	ReleaseAutoreleasePool(arp);
#endif

	return NULL;
}


//============================================================
//  worker_thread_process
//============================================================

static void worker_thread_process(osd_work_queue *queue, work_thread_info *thread)
{
	int threadid = thread - queue->thread;

	begin_timing(thread->runtime);

	// loop until everything is processed
	while (true)
	{
		osd_work_item *item = NULL;

		bool end_loop = false;

		// use a critical section to synchronize the removal of items
		{
			queue->lock->lock();
			if (queue->list == NULL)
			{
				end_loop = true;
			}
			else
			{
				// pull the item from the queue
				item = (osd_work_item *)queue->list;
				if (item != NULL)
				{
					queue->list = item->next;
					if (queue->list == NULL)
						queue->tailptr = (osd_work_item **)&queue->list;
				}
			}
			queue->lock->unlock();
		}

		if (end_loop)
			break;

		// process non-NULL items
		if (item != NULL)
		{
			// call the callback and stash the result
			begin_timing(thread->actruntime);
			item->result = (*item->callback)(item->param, threadid);
			end_timing(thread->actruntime);

			// decrement the item count after we are done
			atomic_decrement32(&queue->items);
			atomic_exchange32(&item->done, TRUE);
			add_to_stat(&thread->itemsdone, 1);

			// if it's an auto-release item, release it
			if (item->flags & WORK_ITEM_FLAG_AUTO_RELEASE)
				osd_work_item_release(item);

			// set the result and signal the event
			else
			{
				queue->lock->lock();
				if (item->event != NULL)
				{
					osd_event_set(item->event);
					add_to_stat(&item->queue->setevents, 1);
				}
				queue->lock->unlock();
			}

			// if we removed an item and there's still work to do, bump the stats
			if (queue_has_list_items(queue))
				add_to_stat(&queue->extraitems, 1);
		}
	}

	// we don't need to set the doneevent for multi queues because they spin
	if (queue->waiting)
	{
		osd_event_set(queue->doneevent);
		add_to_stat(&queue->setevents, 1);
	}

	end_timing(thread->runtime);
}

bool queue_has_list_items(osd_work_queue *queue)
{
	queue->lock->lock();
	bool has_list_items = (queue->list != NULL);
	queue->lock->unlock();
	return has_list_items;
}
