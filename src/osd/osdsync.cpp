// license:BSD-3-Clause
// copyright-holders:Aaron Giles
//============================================================
//
//  osdsync.c - OSD core work item functions
//
//============================================================
#if defined(OSD_WINDOWS) || defined(SDLMAME_WIN32)
// standard windows headers
#include <windows.h>
#include <process.h>
#include <tchar.h>
#include <cstdlib>

#ifdef __GNUC__
#include <cstdint>
#endif
#endif
#include <mutex>
#include <atomic>
#include <thread>
#include <vector>
#include <algorithm>
// MAME headers
#include "osdcore.h"
#include "osdsync.h"

#include "eminline.h"

#if defined(SDLMAME_LINUX) || defined(SDLMAME_BSD) || defined(SDLMAME_HAIKU) || defined(SDLMAME_EMSCRIPTEN) || defined(SDLMAME_MACOSX)
#include <pthread.h>
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
#define add_to_stat(v,x)        do { (v) += (x); } while (0)
#define begin_timing(v)         do { (v) -= get_profile_ticks(); } while (0)
#define end_timing(v)           do { (v) += get_profile_ticks(); } while (0)
#else
#define add_to_stat(v,x)        do { } while (0)
#define begin_timing(v)         do { } while (0)
#define end_timing(v)           do { } while (0)
#endif

template<typename _AtomType, typename _MainType>
static void spin_while(const volatile _AtomType * volatile atom, const _MainType val, const osd_ticks_t timeout, const int invert = 0)
{
	osd_ticks_t stopspin = osd_ticks() + timeout;

	do {
		int spin = 10000;
		while (--spin)
		{
			if ((*atom != val) ^ invert)
				return;
		}
	} while (((*atom == val) ^ invert) && osd_ticks() < stopspin);
}

template<typename _AtomType, typename _MainType>
static void spin_while_not(const volatile _AtomType * volatile atom, const _MainType val, const osd_ticks_t timeout)
{
	spin_while<_AtomType, _MainType>(atom, val, timeout, 1);
}

//============================================================
//  osd_num_processors
//============================================================

int osd_get_num_processors(bool heavy_mt)
{
#if defined(SDLMAME_EMSCRIPTEN)
	// multithreading is not supported at this time
	return 1;
#else
	unsigned int threads = std::thread::hardware_concurrency();
	// max out at 4 for now since scaling above that seems to do poorly
	return heavy_mt ? threads : std::min(std::thread::hardware_concurrency(), 4U);
#endif
}

//============================================================
//  TYPE DEFINITIONS
//============================================================

struct work_thread_info
{
	work_thread_info(uint32_t aid, osd_work_queue &aqueue)
	: queue(aqueue)
	, handle(nullptr)
	, wakeevent(true, false)  // manual reset, not signalled
	, id(aid)
#if KEEP_STATISTICS
	, itemsdone(0)
	, actruntime(0)
	, runtime(0)
	, spintime(0)
	, waittime(0)
#endif
	{
	}
	osd_work_queue &    queue;          // pointer back to the queue
	std::thread *       handle;         // handle to the thread
	osd_event           wakeevent;      // wake event for the thread
	uint32_t              id;

#if KEEP_STATISTICS
	int32_t               itemsdone;
	osd_ticks_t         actruntime;
	osd_ticks_t         runtime;
	osd_ticks_t         spintime;
	osd_ticks_t         waittime;
#endif
};


struct osd_work_queue
{
	osd_work_queue()
	: list(nullptr)
	, tailptr(nullptr)
	, free(nullptr)
	, items(0)
	, livethreads(0)
	, waiting(0)
	, exiting(0)
	, threads(0)
	, flags(0)
	, doneevent(true, true)     // manual reset, signalled
#if KEEP_STATISTICS
	, itemsqueued(0)
	, setevents(0)
	, extraitems(0)
	, spinloops(0)
#endif
	{
	}

	std::mutex          lock;           // lock for protecting the queue
	std::atomic<osd_work_item *> list;  // list of items in the queue
	osd_work_item ** volatile tailptr;  // pointer to the tail pointer of work items in the queue
	std::atomic<osd_work_item *> free;  // free list of work items
	std::atomic<int32_t>  items;          // items in the queue
	std::atomic<int32_t>  livethreads;    // number of live threads
	std::atomic<int32_t>  waiting;        // is someone waiting on the queue to complete?
	std::atomic<int32_t>  exiting;        // should the threads exit on their next opportunity?
	uint32_t              threads;        // number of threads in this queue
	uint32_t              flags;          // creation flags
	std::vector<work_thread_info *>  thread;         // array of thread information
	osd_event           doneevent;      // event signalled when work is complete

#if KEEP_STATISTICS
	std::atomic<int32_t>  itemsqueued;    // total items queued
	std::atomic<int32_t>  setevents;      // number of times we called SetEvent
	std::atomic<int32_t>  extraitems;     // how many extra items we got after the first in the queue loop
	std::atomic<int32_t>  spinloops;      // how many times spinning bought us more items
#endif
};


struct osd_work_item
{
	osd_work_item(osd_work_queue &aqueue)
	: next(nullptr)
	, queue(aqueue)
	, callback(nullptr)
	, param(nullptr)
	, result(nullptr)
	, event(nullptr)                // manual reset, not signalled
	, flags(0)
	, done(false)
	{
	}

	osd_work_item *     next;           // pointer to next item
	osd_work_queue &    queue;          // pointer back to the owning queue
	osd_work_callback   callback;       // callback function
	void *              param;          // callback parameter
	void *              result;         // callback result
	osd_event *         event;          // event signalled when complete
	uint32_t              flags;          // creation flags
	std::atomic<int32_t>  done;           // is the item done?
};

//============================================================
//  GLOBAL VARIABLES
//============================================================

int osd_num_processors = 0;

//============================================================
//  FUNCTION PROTOTYPES
//============================================================

static int effective_num_processors(bool heavy_mt);
static void * worker_thread_entry(void *param);
static void worker_thread_process(osd_work_queue *queue, work_thread_info *thread);
static bool queue_has_list_items(osd_work_queue *queue);

//============================================================
//  osd_thread_adjust_priority
//============================================================

int thread_adjust_priority(std::thread *thread, int adjust)
{
#if defined(OSD_WINDOWS) || defined(SDLMAME_WIN32)
	if (adjust)
		SetThreadPriority((HANDLE)thread->native_handle(), THREAD_PRIORITY_ABOVE_NORMAL);
	else
		SetThreadPriority((HANDLE)thread->native_handle(), GetThreadPriority(GetCurrentThread()));
#endif
#if defined(SDLMAME_LINUX) || defined(SDLMAME_BSD) || defined(SDLMAME_HAIKU) || defined(SDLMAME_DARWIN)
	struct sched_param  sched;
	int                 policy;

	if (pthread_getschedparam(thread->native_handle(), &policy, &sched) == 0)
	{
		sched.sched_priority += adjust;
		if (pthread_setschedparam(thread->native_handle(), policy, &sched) == 0)
			return true;
		else
			return false;
	}
#endif
	return true;
}

//============================================================
//  osd_work_queue_alloc
//============================================================

osd_work_queue *osd_work_queue_alloc(int flags)
{
	int threadnum;
	int numprocs = effective_num_processors(!(flags & WORK_QUEUE_FLAG_HIGH_FREQ));
	osd_work_queue *queue;
	int osdthreadnum = 0;
	int allocthreadnum;
	const char *osdworkqueuemaxthreads = osd_getenv(ENV_WORKQUEUEMAXTHREADS);

	// allocate a new queue
	queue = new osd_work_queue();

	// initialize basic queue members
	queue->tailptr = (osd_work_item **)&queue->list;
	queue->flags = flags;

	// determine how many threads to create...
	// on a single-CPU system, create 1 thread for I/O queues, and 0 threads for everything else
	if (numprocs == 1)
		threadnum = (flags & WORK_QUEUE_FLAG_IO) ? 1 : 0;
	// on an n-CPU system, create n-1 threads for multi queues, and 1 thread for everything else
	else
		threadnum = (flags & WORK_QUEUE_FLAG_MULTI) ? (numprocs - 1) : 1;

	if (osdworkqueuemaxthreads != nullptr && sscanf(osdworkqueuemaxthreads, "%d", &osdthreadnum) == 1 && threadnum > osdthreadnum)
		threadnum = osdthreadnum;

#if defined(SDLMAME_EMSCRIPTEN)
	// threads are not supported at all
	threadnum = 0;
#endif

	// clamp to the maximum
	queue->threads = std::min(threadnum, WORK_MAX_THREADS);

	// allocate memory for thread array (+1 to count the calling thread if WORK_QUEUE_FLAG_MULTI)
	if (flags & WORK_QUEUE_FLAG_MULTI)
		allocthreadnum = queue->threads + 1;
	else
		allocthreadnum = queue->threads;

#if KEEP_STATISTICS
	printf("osdprocs: %d effecprocs: %d threads: %d allocthreads: %d osdthreads: %d maxthreads: %d queuethreads: %d\n", osd_num_processors, numprocs, threadnum, allocthreadnum, osdthreadnum, WORK_MAX_THREADS, queue->threads);
#endif

	for (threadnum = 0; threadnum < allocthreadnum; threadnum++)
		queue->thread.push_back(new work_thread_info(threadnum, *queue));

	// iterate over threads
	for (threadnum = 0; threadnum < queue->threads; threadnum++)
	{
		work_thread_info *thread = queue->thread[threadnum];

		// create the thread
		thread->handle = new std::thread(worker_thread_entry, thread);
		if (thread->handle == nullptr)
			goto error;

		// set its priority: I/O threads get high priority because they are assumed to be
		// blocked most of the time; other threads just match the creator's priority
		if (flags & WORK_QUEUE_FLAG_IO)
			thread_adjust_priority(thread->handle, 1);
		else
			thread_adjust_priority(thread->handle, 0);
	}

	// start a timer going for "waittime" on the main thread
	if (flags & WORK_QUEUE_FLAG_MULTI)
	{
		begin_timing(queue->thread[queue->threads]->waittime);
	}
	return queue;

error:
	osd_work_queue_free(queue);
	return nullptr;
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

bool osd_work_queue_wait(osd_work_queue *queue, osd_ticks_t timeout)
{
	// if no threads, no waiting
	if (queue->threads == 0)
		return true;

	// if no items, we're done
	if (queue->items == 0)
		return true;

	// if this is a multi queue, help out rather than doing nothing
	if (queue->flags & WORK_QUEUE_FLAG_MULTI)
	{
		work_thread_info *thread = queue->thread[queue->threads];

		end_timing(thread->waittime);

		// process what we can as a worker thread
		worker_thread_process(queue, thread);

		// if we're a high frequency queue, spin until done
		if (queue->flags & WORK_QUEUE_FLAG_HIGH_FREQ && queue->items != 0)
		{
			// spin until we're done
			begin_timing(thread->spintime);
			spin_while_not<std::atomic<int>,int>(&queue->items, 0, timeout);
			end_timing(thread->spintime);

			begin_timing(thread->waittime);
			return (queue->items == 0);
		}
		begin_timing(thread->waittime);
	}

	// reset our done event and double-check the items before waiting
	queue->doneevent.reset();
	queue->waiting = true;
	if (queue->items != 0)
		queue->doneevent.wait(timeout);
	queue->waiting = false;

	// return true if we actually hit 0
	return (queue->items == 0);
}


//============================================================
//  osd_work_queue_free
//============================================================

void osd_work_queue_free(osd_work_queue *queue)
{
	// stop the timer for "waittime" on the main thread
	if (queue->flags & WORK_QUEUE_FLAG_MULTI)
	{
		end_timing(queue->thread[queue->threads]->waittime);
	}

	// signal all the threads to exit
	queue->exiting = true;
	for (int threadnum = 0; threadnum < queue->threads; threadnum++)
	{
		work_thread_info *thread = queue->thread[threadnum];
		thread->wakeevent.set();
	}

	// wait for all the threads to go away
	for (int threadnum = 0; threadnum < queue->threads; threadnum++)
	{
		work_thread_info *thread = queue->thread[threadnum];

		// block on the thread going away, then close the handle
		if (thread->handle != nullptr)
		{
			thread->handle->join();
			delete thread->handle;
		}

	}

#if KEEP_STATISTICS
	// output per-thread statistics
	for (work_thread_info *thread : queue->thread)
	{
		osd_ticks_t total = thread->runtime + thread->waittime + thread->spintime;
		printf("Thread %d:  items=%9d run=%5.2f%% (%5.2f%%)  spin=%5.2f%%  wait/other=%5.2f%% total=%9d\n",
				thread->id, thread->itemsdone,
				(double)thread->runtime * 100.0 / (double)total,
				(double)thread->actruntime * 100.0 / (double)total,
				(double)thread->spintime * 100.0 / (double)total,
				(double)thread->waittime * 100.0 / (double)total,
				(uint32_t) total);
	}
#endif

	// free the list
	for (auto & th : queue->thread)
		delete th;
	queue->thread.clear();

	// free all items in the free list
	while (queue->free.load() != nullptr)
	{
		auto *item = (osd_work_item *)queue->free;
		queue->free = item->next;
		delete item->event;
		delete item;
	}

	// free all items in the active list
	while (queue->list.load() != nullptr)
	{
		auto *item = (osd_work_item *)queue->list;
		queue->list = item->next;
		delete item->event;
		delete item;
	}

#if KEEP_STATISTICS
	printf("Items queued   = %9d\n", queue->itemsqueued.load());
	printf("SetEvent calls = %9d\n", queue->setevents.load());
	printf("Extra items    = %9d\n", queue->extraitems.load());
	printf("Spin loops     = %9d\n", queue->spinloops.load());
#endif

	// free the queue itself
	delete queue;
}


//============================================================
//  osd_work_item_queue_multiple
//============================================================

osd_work_item *osd_work_item_queue_multiple(osd_work_queue *queue, osd_work_callback callback, int32_t numitems, void *parambase, int32_t paramstep, uint32_t flags)
{
	osd_work_item *itemlist = nullptr, *lastitem = nullptr;
	osd_work_item **item_tailptr = &itemlist;
	int itemnum;

	// loop over items, building up a local list of work
	for (itemnum = 0; itemnum < numitems; itemnum++)
	{
		osd_work_item *item;

		// first allocate a new work item; try the free list first
		{
			std::lock_guard<std::mutex> lock(queue->lock);
			do
			{
				item = (osd_work_item *)queue->free;
			} while (item != nullptr && !queue->free.compare_exchange_weak(item, item->next, std::memory_order_release, std::memory_order_relaxed));
		}

		// if nothing, allocate something new
		if (item == nullptr)
		{
			// allocate the item
			item = new osd_work_item(*queue);
			if (item == nullptr)
				return nullptr;
		}
		else
		{
			item->done = false; // needs to be set this way to prevent data race/usage of uninitialized memory on Linux
		}

		// fill in the basics
		item->next = nullptr;
		item->callback = callback;
		item->param = parambase;
		item->result = nullptr;
		item->flags = flags;

		// advance to the next
		lastitem = item;
		*item_tailptr = item;
		item_tailptr = &item->next;
		parambase = (uint8_t *)parambase + paramstep;
	}

	// enqueue the whole thing within the critical section
	{
		std::lock_guard<std::mutex> lock(queue->lock);
		*queue->tailptr = itemlist;
		queue->tailptr = item_tailptr;
	}

	// increment the number of items in the queue
	queue->items += numitems;
	add_to_stat(queue->itemsqueued, numitems);

	// look for free threads to do the work
	if (queue->livethreads < queue->threads)
	{
		int threadnum;

		// iterate over all the threads
		for (threadnum = 0; threadnum < queue->threads; threadnum++)
		{
			work_thread_info *thread = queue->thread[threadnum];

			// Attempt to wake the thread
			if (thread->wakeevent.set())
			{
				add_to_stat(queue->setevents, 1);

				// for non-shared, the first one we find is good enough
				if (--numitems == 0)
					break;
			}
		}
	}

	// if no threads, run the queue now on this thread
	if (queue->threads == 0)
	{
		end_timing(queue->thread[0]->waittime);
		worker_thread_process(queue, queue->thread[0]);
		begin_timing(queue->thread[0]->waittime);
	}
	// only return the item if it won't get released automatically
	return (flags & WORK_ITEM_FLAG_AUTO_RELEASE) ? nullptr : lastitem;
}


//============================================================
//  osd_work_item_wait
//============================================================

bool osd_work_item_wait(osd_work_item *item, osd_ticks_t timeout)
{
	// if we're done already, just return
	if (item->done)
		return true;

	// if we don't have an event, create one
	if (item->event == nullptr)
	{
		std::lock_guard<std::mutex> lock(item->queue.lock);
		item->event = new osd_event(true, false);     // manual reset, not signalled
	}
	else
		item->event->reset();

	// if we don't have an event, we need to spin (shouldn't ever really happen)
	if (item->event == nullptr)
	{
		// TODO: do we need to measure the spin time here as well? and how can we do it?
		spin_while<std::atomic<int>,int>(&item->done, 0, timeout);
	}

	// otherwise, block on the event until done
	else if (!item->done)
		item->event->wait(timeout);

	// return true if the refcount actually hit 0
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
	std::lock_guard<std::mutex> lock(item->queue.lock);
	do
	{
		next = (osd_work_item *) item->queue.free;
		item->next = next;
	} while (!item->queue.free.compare_exchange_weak(next, item, std::memory_order_release, std::memory_order_relaxed));
}


//============================================================
//  effective_num_processors
//============================================================

static int effective_num_processors(bool heavy_mt)
{
	int physprocs = osd_get_num_processors(heavy_mt);

	// osd_num_processors == 0 for 'auto'
	if (osd_num_processors > 0)
	{
		return std::min(4 * physprocs, osd_num_processors);
	}
	else
	{
		int numprocs = 0;

		// if the OSDPROCESSORS environment variable is set, use that value if valid
		// note that we permit more than the real number of processors for testing
		const char *procsoverride = osd_getenv(ENV_PROCESSORS);
		if (procsoverride != nullptr && sscanf(procsoverride, "%d", &numprocs) == 1 && numprocs > 0)
			return std::min(4 * physprocs, numprocs);

		// otherwise, return the info from the system
		return physprocs;
	}
}


//============================================================
//  worker_thread_entry
//============================================================

static void *worker_thread_entry(void *param)
{
	auto *thread = (work_thread_info *)param;
	osd_work_queue &queue = thread->queue;

	// loop until we exit
	for ( ;; )
	{
		// block waiting for work or exit
		// bail on exit, and only wait if there are no pending items in queue
		if (queue.exiting)
			break;

		if (!queue_has_list_items(&queue))
		{
			begin_timing(thread->waittime);
			thread->wakeevent.wait( OSD_EVENT_WAIT_INFINITE);
			end_timing(thread->waittime);
		}

		if (queue.exiting)
			break;

		// indicate that we are live
		++queue.livethreads;

		// process work items
		for ( ;; )
		{
			// process as much as we can
			worker_thread_process(&queue, thread);

			// if we're a high frequency queue, spin for a while before giving up
			if (queue.flags & WORK_QUEUE_FLAG_HIGH_FREQ && queue.list.load() == nullptr)
			{
				// spin for a while looking for more work
				begin_timing(thread->spintime);
				spin_while<std::atomic<osd_work_item *>, osd_work_item *>(&queue.list, (osd_work_item *)nullptr, SPIN_LOOP_TIME);
				end_timing(thread->spintime);
			}

			// if nothing more, release the processor
			if (!queue_has_list_items(&queue))
				break;
			add_to_stat(queue.spinloops, 1);
		}

		// decrement the live thread count
		thread->wakeevent.reset();
		--queue.livethreads;
	}

	return nullptr;
}


//============================================================
//  worker_thread_process
//============================================================

static void worker_thread_process(osd_work_queue *queue, work_thread_info *thread)
{
	int threadid = thread->id;

	begin_timing(thread->runtime);

	// loop until everything is processed
	while (true)
	{
		osd_work_item *item = nullptr;

		bool end_loop = false;

		// use a critical section to synchronize the removal of items
		{
			std::lock_guard<std::mutex> lock(queue->lock);

			if (queue->list.load() == nullptr)
			{
				end_loop = true;
			}
			else
			{
				// pull the item from the queue
				item = (osd_work_item *)queue->list;
				if (item != nullptr)
				{
					queue->list = item->next;
					if (queue->list.load() == nullptr)
						queue->tailptr = (osd_work_item **)&queue->list;
				}
			}
		}

		if (end_loop)
			break;

		// process non-NULL items
		if (item != nullptr)
		{
			// call the callback and stash the result
			begin_timing(thread->actruntime);
			item->result = (*item->callback)(item->param, threadid);
			end_timing(thread->actruntime);

			// decrement the item count after we are done
			--queue->items;
			item->done = true;
			add_to_stat(thread->itemsdone, 1);

			// if it's an auto-release item, release it
			if (item->flags & WORK_ITEM_FLAG_AUTO_RELEASE)
				osd_work_item_release(item);

			// set the result and signal the event
			else
			{
				std::lock_guard<std::mutex> lock(queue->lock);

				if (item->event != nullptr)
				{
					item->event->set();
					add_to_stat(item->queue.setevents, 1);
				}
			}

			// if we removed an item and there's still work to do, bump the stats
			if (queue_has_list_items(queue))
				add_to_stat(queue->extraitems, 1);
		}
	}

	// we don't need to set the doneevent for multi queues because they spin
	if (queue->waiting)
	{
		queue->doneevent.set();
		add_to_stat(queue->setevents, 1);
	}

	end_timing(thread->runtime);
}

bool queue_has_list_items(osd_work_queue *queue)
{
	std::lock_guard<std::mutex> lock(queue->lock);
	bool has_list_items = (queue->list.load() != nullptr);
	return has_list_items;
}
