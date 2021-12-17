// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont
//============================================================
//
//  osdsync.h - Core synchronization functions
//
//============================================================
#ifndef MAME_OSD_OSDSYNC_H
#define MAME_OSD_OSDSYNC_H

#pragma once

// C++ headers
#include <mutex>
#include <atomic>
#include <condition_variable>

#include "osdcomm.h"

/***************************************************************************
    SYNCHRONIZATION INTERFACES - Events
***************************************************************************/

#define OSD_EVENT_WAIT_INFINITE (~(osd_ticks_t)0)

/* osd_event is an opaque type which represents a setable/resettable event */

class osd_event
{
public:
	/*-----------------------------------------------------------------------------
	    constructor: allocate a new event

	    Parameters:

	        manualreset  - boolean. If true, the event will be automatically set
	                       to non-signalled after a thread successfully waited for
	                       it.
	        initialstate - boolean. If true, the event is signalled initially.

	    Return value:

	        A pointer to the allocated event.
	-----------------------------------------------------------------------------*/
	osd_event(int manualreset, int initialstate)
	{
		m_signalled = initialstate;
		m_autoreset = !manualreset;
	}

	~osd_event()
	{
	}

	/*-----------------------------------------------------------------------------
	    wait:   wait for an event to be signalled
	            If the event is in signalled state, the
	            function returns immediately. If not it will wait for the event
	            to become signalled.

	    Parameters:

	        timeout - timeout in osd_ticks

	    Return value:

	        true:  The event was signalled
	        false: A timeout occurred
	-----------------------------------------------------------------------------*/

	bool wait(osd_ticks_t timeout)
	{
		if (timeout == OSD_EVENT_WAIT_INFINITE)
			timeout = osd_ticks_per_second() * (osd_ticks_t)10000;

		std::unique_lock<std::mutex> lock(m_mutex);
		if (!timeout)
		{
			if (!m_signalled)
			{
				return false;
			}
		}
		else
		{
			if (!m_signalled)
			{
				uint64_t msec = timeout * 1000 / osd_ticks_per_second();

				do {
					if (m_cond.wait_for(lock, std::chrono::milliseconds(msec)) == std::cv_status::timeout)
					{
						if (!m_signalled)
						{
							return false;
						}
						else
							break;
					} else
						break;

				} while (true);
			}
		}

		if (m_autoreset)
			m_signalled = 0;

		return true;
	}

	/*-----------------------------------------------------------------------------
	    osd_event_reset: reset an event to non-signalled state

	    Parameters:

	        None

	    Return value:

	        None
	-----------------------------------------------------------------------------*/
	void reset()
	{
		m_mutex.lock();
		m_signalled = false;
		m_mutex.unlock();
	}

	/*-----------------------------------------------------------------------------
	    osd_event_set: set an event to signalled state

	    Parameters:

	        None

	    Return value:

	        Whether or not the event was actually signalled (false if the event had already been signalled)

	    Notes:

	        All threads waiting for the event will be signalled.
	-----------------------------------------------------------------------------*/
	bool set()
	{
		m_mutex.lock();
		bool needs_signal = !m_signalled;
		if (needs_signal)
		{
			m_signalled = true;
			if (m_autoreset)
				m_cond.notify_one();
			else
				m_cond.notify_all();
		}
		m_mutex.unlock();
		return needs_signal;
	}

private:
	std::mutex               m_mutex;
	std::condition_variable  m_cond;
	int32_t                  m_autoreset;
	int32_t                  m_signalled;

};

#endif // MAME_OSD_OSDSYNC_H
