//============================================================
//
//  winsync.h - Windows core synchronization functions
//
//  Copyright (c) 1996-2014, Nicola Salmoria and the MAME Team.
//  Visit http://mamedev.org for licensing and usage restrictions.
//
//============================================================

#ifndef __WINSYNC__
#define __WINSYNC__

/***************************************************************************
    SYNCHRONIZATION INTERFACES - Events
***************************************************************************/

/* osd_event is an opaque type which represents a setable/resetable event */

struct osd_event;


/*-----------------------------------------------------------------------------
    osd_lock_event_alloc: allocate a new event

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

//============================================================
//  Scalable Locks
//============================================================

struct osd_scalable_lock;

osd_scalable_lock *osd_scalable_lock_alloc(void);

INT32 osd_scalable_lock_acquire(osd_scalable_lock *lock);

void osd_scalable_lock_release(osd_scalable_lock *lock, INT32 myslot);

void osd_scalable_lock_free(osd_scalable_lock *lock);

#endif  /* __WINSYNC__ */