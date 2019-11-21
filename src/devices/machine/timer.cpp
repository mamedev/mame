// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    timer.cpp

    Timer devices.

***************************************************************************/

#include "emu.h"
#include "machine/timer.h"


/***************************************************************************
//  DEBUGGING
***************************************************************************/

#define VERBOSE 0

#define LOG(x)  do { if (VERBOSE) logerror x; } while (0)




//**************************************************************************
//  LIVE TIMER DEVICE
//**************************************************************************

DEFINE_DEVICE_TYPE(TIMER, timer_device, "timer", "Timer")

//-------------------------------------------------
//  timer_device - constructor
//-------------------------------------------------

timer_device::timer_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, TIMER, tag, owner, clock),
		m_type(TIMER_TYPE_GENERIC),
		m_callback(*this),
		m_ptr(nullptr),
		m_start_delay(attotime::zero),
		m_period(attotime::zero),
		m_param(0),
		m_screen(*this, finder_base::DUMMY_TAG),
		m_first_vpos(0),
		m_increment(0),
		m_timer(nullptr),
		m_first_time(true)
{
}


//-------------------------------------------------
//  device_validity_check - validate the device
//  configuration
//-------------------------------------------------

void timer_device::device_validity_check(validity_checker &valid) const
{
	// type based configuration
	switch (m_type)
	{
		case TIMER_TYPE_GENERIC:
			if (m_screen.finder_tag() != finder_base::DUMMY_TAG || m_first_vpos != 0 || m_start_delay != attotime::zero)
				osd_printf_warning("Generic timer specified parameters for a scanline timer\n");
			if (m_period != attotime::zero || m_start_delay != attotime::zero)
				osd_printf_warning("Generic timer specified parameters for a periodic timer\n");
			break;

		case TIMER_TYPE_PERIODIC:
			if (m_screen.finder_tag() != finder_base::DUMMY_TAG || m_first_vpos != 0)
				osd_printf_warning("Periodic timer specified parameters for a scanline timer\n");
			if (m_period <= attotime::zero)
				osd_printf_error("Periodic timer specified invalid period\n");
			break;

		case TIMER_TYPE_SCANLINE:
			if (m_period != attotime::zero || m_start_delay != attotime::zero)
				osd_printf_warning("Scanline timer specified parameters for a periodic timer\n");
			if (m_param != 0)
				osd_printf_warning("Scanline timer specified parameter which is ignored\n");
			if (m_screen.finder_tag() == finder_base::DUMMY_TAG)
				osd_printf_error("Scanline timer has no screen specified\n");
			else if (!m_screen)
				osd_printf_error("Scanline timer specifies nonexistent screen %s\n", m_screen.finder_tag());
//          if (m_first_vpos < 0)
//              osd_printf_error("Scanline timer specified invalid initial position\n");
//          if (m_increment < 0)
//              osd_printf_error("Scanline timer specified invalid increment\n");
			break;

		default:
			osd_printf_error("Invalid type specified\n");
			break;
	}
}


//-------------------------------------------------
//  device_start - perform device-specific
//  startup
//-------------------------------------------------

void timer_device::device_start()
{
	// allocate the timer
	m_timer = timer_alloc();

	m_callback.resolve();

	// register for save states
	save_item(NAME(m_first_time));
}


//-------------------------------------------------
//  device_reset - reset the device
//-------------------------------------------------

void timer_device::device_reset()
{
	// type based configuration
	switch (m_type)
	{
		case TIMER_TYPE_GENERIC:
		case TIMER_TYPE_PERIODIC:
		{
			// convert the period into attotime
			attotime period;
			if (m_period > attotime::zero)
			{
				period = m_period;

				// convert the start_delay into attotime
				attotime start_delay = attotime::zero;
				if (m_start_delay > attotime::zero)
					start_delay = m_start_delay;

				// allocate and start the backing timer
				m_timer->adjust(start_delay, m_param, period);
			}
			break;
		}

		case TIMER_TYPE_SCANLINE:
			if (!m_screen)
				fatalerror("timer '%s': unable to find screen '%s'\n", tag(), m_screen.finder_tag());

			// set the timer to fire immediately
			m_first_time = true;
			m_timer->adjust(attotime::zero, m_param);
			break;
	}
}


//-------------------------------------------------
//  device_timer - handle timer expiration events
//-------------------------------------------------

void timer_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (m_type)
	{
		// general periodic timers just call through
		case TIMER_TYPE_GENERIC:
		case TIMER_TYPE_PERIODIC:
			if (!m_callback.isnull())
				(m_callback)(*this, m_ptr, param);
			break;

		// scanline timers have to do some additional bookkeeping
		case TIMER_TYPE_SCANLINE:
		{
			// by default, we fire at the first position
			int next_vpos = m_first_vpos;

			// the first time through we just go with the default position
			if (!m_first_time)
			{
				// call the real callback
				int vpos = m_screen->vpos();
				if (!m_callback.isnull())
					(m_callback)(*this, m_ptr, vpos);

				// advance by the increment only if we will still be within the screen bounds
				if (m_increment != 0 && (vpos + m_increment) < m_screen->height())
					next_vpos = vpos + m_increment;
			}
			m_first_time = false;

			// adjust the timer
			m_timer->adjust(m_screen->time_until_pos(next_vpos));
			break;
		}
	}
}
