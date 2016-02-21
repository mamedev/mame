// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont
//============================================================
//
//  osdsync.h - Core synchronization functions
//
//============================================================

#ifndef __OSDSYNC__
#define __OSDSYNC__

#include "osdcomm.h"
#include "osdcore.h"

/***************************************************************************
    SYNCHRONIZATION INTERFACES - Events
***************************************************************************/

#define OSD_EVENT_WAIT_INFINITE (~(osd_ticks_t)0)

/* osd_event is an opaque type which represents a setable/resetable event */

struct osd_event;


/*-----------------------------------------------------------------------------
    osd_event_alloc: allocate a new event

    Parameters:

        manualreset  - boolean. If true, the event will be automatically set
                       to non-signalled after a thread successfully waited for
                       it.
        initialstate - boolean. If true, the event is signalled initially.

    Return value:

        A pointer to the allocated event.
-----------------------------------------------------------------------------*/
osd_event *osd_event_alloc(int manualreset, int initialstate);


/*-----------------------------------------------------------------------------
    osd_event_wait: wait for an event to be signalled

    Parameters:

        event - The event to wait for. If the event is in signalled state, the
                function returns immediately. If not it will wait for the event
                to become signalled.
        timeout - timeout in osd_ticks

    Return value:

        TRUE:  The event was signalled
        FALSE: A timeout occurred
-----------------------------------------------------------------------------*/
int osd_event_wait(osd_event *event, osd_ticks_t timeout);


/*-----------------------------------------------------------------------------
    osd_event_reset: reset an event to non-signalled state

    Parameters:

        event - The event to set to non-signalled state

    Return value:

        None
-----------------------------------------------------------------------------*/
void osd_event_reset(osd_event *event);


/*-----------------------------------------------------------------------------
    osd_event_set: set an event to signalled state

    Parameters:

        event - The event to set to signalled state

    Return value:

        None

    Notes:

        All threads waiting for the event will be signalled.
-----------------------------------------------------------------------------*/
void osd_event_set(osd_event *event);


/*-----------------------------------------------------------------------------
    osd_event_free: free the memory and resources associated with an osd_event

    Parameters:

        event - a pointer to a previously allocated osd_event.

    Return value:

        None.
-----------------------------------------------------------------------------*/
void osd_event_free(osd_event *event);

/***************************************************************************
    SYNCHRONIZATION INTERFACES - Threads
***************************************************************************/

/* osd_thread is an opaque type which represents a thread */
struct osd_thread;


/* osd_thread_callback is a callback function that will be called from the thread */
typedef void *(*osd_thread_callback)(void *param);


/*-----------------------------------------------------------------------------
    osd_thread_create: create a new thread

    Parameters:

        callback - The callback function to be called once the thread is up
        cbparam  - The parameter to pass to the callback function.

    Return value:

        A pointer to the created thread.
-----------------------------------------------------------------------------*/
osd_thread *osd_thread_create(osd_thread_callback callback, void *cbparam);

/*-----------------------------------------------------------------------------
    osd_thread_adjust_priority: adjust priority of a thread

    Parameters:

        thread - A pointer to a previously created thread.
        adjust - signed integer to add to the thread priority

    Return value:

        TRUE on success, FALSE on failure
-----------------------------------------------------------------------------*/
int osd_thread_adjust_priority(osd_thread *thread, int adjust);


/*-----------------------------------------------------------------------------
    osd_thread_cpu_affinity: change cpu affinity of a thread

    Parameters:

        thread - A pointer to a previously created thread
                 or NULL for main thread
        mask   - bitmask to which cpus to bind
                 i.e. 0x01 1st cpu, 0x02, 2nd cpu, 0x04 3rd cpu

    Return value:

        TRUE on success, FALSE on failure
-----------------------------------------------------------------------------*/
int osd_thread_cpu_affinity(osd_thread *thread, UINT32 mask);


/*-----------------------------------------------------------------------------
    osd_thread_wait_free: wait for thread to finish and free resources

    Parameters:

        thread - A pointer to a previously created thread.

    Return value:

        None.
-----------------------------------------------------------------------------*/
void osd_thread_wait_free(osd_thread *thread);

#endif  /* __OSDSYNC__ */
