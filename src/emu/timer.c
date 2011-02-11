/***************************************************************************

    timer.c

    Timer devices.

****************************************************************************

    Copyright Aaron Giles
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
          notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer in
          the documentation and/or other materials provided with the
          distribution.
        * Neither the name 'MAME' nor the names of its contributors may be
          used to endorse or promote products derived from this software
          without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

***************************************************************************/

#include "emu.h"
#include "profiler.h"


/***************************************************************************
//  DEBUGGING
***************************************************************************/

#define VERBOSE 0

#define LOG(x)	do { if (VERBOSE) logerror x; } while (0)



/***************************************************************************
//  DEVICE DEFINITIONS
***************************************************************************/

const device_type TIMER = timer_device_config::static_alloc_device_config;




//**************************************************************************
//  TIMER DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  timer_device_config - constructor
//-------------------------------------------------

timer_device_config::timer_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock)
	: device_config(mconfig, static_alloc_device_config, "Timer", tag, owner, clock),
	  m_type(TIMER_TYPE_GENERIC),
	  m_callback(NULL),
	  m_ptr(NULL),
	  m_start_delay(attotime::zero),
	  m_period(attotime::zero),
	  m_param(0),
	  m_screen(NULL),
	  m_first_vpos(0),
	  m_increment(0)
{
}


//-------------------------------------------------
//  static_alloc_device_config - allocate a new
//  configuration object
//-------------------------------------------------

device_config *timer_device_config::static_alloc_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock)
{
	return global_alloc(timer_device_config(mconfig, tag, owner, clock));
}


//-------------------------------------------------
//  alloc_device - allocate a new device object
//-------------------------------------------------

device_t *timer_device_config::alloc_device(running_machine &machine) const
{
	return auto_alloc(&machine, timer_device(machine, *this));
}


//-------------------------------------------------
//  static_configure_generic - configuration
//  helper to set up a generic timer
//-------------------------------------------------

void timer_device_config::static_configure_generic(device_config *device, timer_device_fired_func callback)
{
	timer_device_config *timer = downcast<timer_device_config *>(device);
	timer->m_type = TIMER_TYPE_GENERIC;
	timer->m_callback = callback;
}


//-------------------------------------------------
//  static_configure_periodic - configuration
//  helper to set up a periodic timer
//-------------------------------------------------

void timer_device_config::static_configure_periodic(device_config *device, timer_device_fired_func callback, attotime period)
{
	timer_device_config *timer = downcast<timer_device_config *>(device);
	timer->m_type = TIMER_TYPE_PERIODIC;
	timer->m_callback = callback;
	timer->m_period = period;
}


//-------------------------------------------------
//  static_configure_scanline - configuration
//  helper to set up a scanline timer
//-------------------------------------------------

void timer_device_config::static_configure_scanline(device_config *device, timer_device_fired_func callback, const char *screen, int first_vpos, int increment)
{
	timer_device_config *timer = downcast<timer_device_config *>(device);
	timer->m_type = TIMER_TYPE_SCANLINE;
	timer->m_callback = callback;
	timer->m_screen = screen;
	timer->m_first_vpos = first_vpos;
	timer->m_increment = increment;
}


//-------------------------------------------------
//  static_set_callback - configuration helper
//  to set the callback
//-------------------------------------------------

void timer_device_config::static_set_callback(device_config *device, timer_device_fired_func callback)
{
	timer_device_config *timer = downcast<timer_device_config *>(device);
	timer->m_callback = callback;
}


//-------------------------------------------------
//  static_set_start_delay - configuration helper
//  to set the starting delay
//-------------------------------------------------

void timer_device_config::static_set_start_delay(device_config *device, attotime delay)
{
	timer_device_config *timer = downcast<timer_device_config *>(device);
	timer->m_start_delay = delay;
}


//-------------------------------------------------
//  static_set_param - configuration helper to set
//  the integer parameter
//-------------------------------------------------

void timer_device_config::static_set_param(device_config *device, int param)
{
	timer_device_config *timer = downcast<timer_device_config *>(device);
	timer->m_param = param;
}


//-------------------------------------------------
//  static_set_ptr - configuration helper to set
//  the pointer parameter
//-------------------------------------------------

void timer_device_config::static_set_ptr(device_config *device, void *ptr)
{
	timer_device_config *timer = downcast<timer_device_config *>(device);
	timer->m_ptr = ptr;
}


//-------------------------------------------------
//  device_validity_check - validate the device
//  configuration
//-------------------------------------------------

bool timer_device_config::device_validity_check(const game_driver &driver) const
{
	bool error = false;

	// type based configuration
	switch (m_type)
	{
		case TIMER_TYPE_GENERIC:
			if (m_screen != NULL || m_first_vpos != 0 || m_start_delay != attotime::zero)
				mame_printf_warning("%s: %s generic timer '%s' specified parameters for a scanline timer\n", driver.source_file, driver.name, tag());
			if (m_period != attotime::zero || m_start_delay != attotime::zero)
				mame_printf_warning("%s: %s generic timer '%s' specified parameters for a periodic timer\n", driver.source_file, driver.name, tag());
			break;

		case TIMER_TYPE_PERIODIC:
			if (m_screen != NULL || m_first_vpos != 0)
				mame_printf_warning("%s: %s periodic timer '%s' specified parameters for a scanline timer\n", driver.source_file, driver.name, tag());
			if (m_period <= attotime::zero)
			{
				mame_printf_error("%s: %s periodic timer '%s' specified invalid period\n", driver.source_file, driver.name, tag());
				error = true;
			}
			break;

		case TIMER_TYPE_SCANLINE:
			if (m_period != attotime::zero || m_start_delay != attotime::zero)
				mame_printf_warning("%s: %s scanline timer '%s' specified parameters for a periodic timer\n", driver.source_file, driver.name, tag());
			if (m_param != 0)
				mame_printf_warning("%s: %s scanline timer '%s' specified parameter which is ignored\n", driver.source_file, driver.name, tag());
			if (m_first_vpos < 0)
			{
				mame_printf_error("%s: %s scanline timer '%s' specified invalid initial position\n", driver.source_file, driver.name, tag());
				error = true;
			}
			if (m_increment < 0)
			{
				mame_printf_error("%s: %s scanline timer '%s' specified invalid increment\n", driver.source_file, driver.name, tag());
				error = true;
			}
			break;

		default:
			mame_printf_error("%s: %s timer '%s' has an invalid type\n", driver.source_file, driver.name, tag());
			error = true;
			break;
	}

	return error;
}



//**************************************************************************
//  LIVE TIMER DEVICE
//**************************************************************************

//-------------------------------------------------
//  timer_device - constructor
//-------------------------------------------------

timer_device::timer_device(running_machine &_machine, const timer_device_config &config)
	: device_t(_machine, config),
	  m_config(config),
	  m_timer(NULL),
	  m_ptr(m_config.m_ptr),
	  m_screen(NULL),
	  m_first_time(true)
{
}


//-------------------------------------------------
//  device_start - perform device-specific
//  startup
//-------------------------------------------------

void timer_device::device_start()
{
	// fetch the screen
	if (m_config.m_screen != NULL)
		m_screen = downcast<screen_device *>(machine->device(m_config.m_screen));

	// allocate the timer
	m_timer = timer_alloc();

	// register for save states
	save_item(NAME(m_first_time));
}


//-------------------------------------------------
//  device_reset - reset the device
//-------------------------------------------------

void timer_device::device_reset()
{
	// type based configuration
	switch (m_config.m_type)
	{
		case timer_device_config::TIMER_TYPE_GENERIC:
		case timer_device_config::TIMER_TYPE_PERIODIC:
		{
			// convert the period into attotime
			attotime period = attotime::never;
			if (m_config.m_period > attotime::zero)
			{
				period = m_config.m_period;

				// convert the start_delay into attotime
				attotime start_delay = attotime::zero;
				if (m_config.m_start_delay > attotime::zero)
					start_delay = m_config.m_start_delay;

				// allocate and start the backing timer
				m_timer->adjust(start_delay, m_config.m_param, period);
			}
			break;
		}

		case timer_device_config::TIMER_TYPE_SCANLINE:
			if (m_screen == NULL)
				fatalerror("timer '%s': unable to find screen '%s'\n", tag(), m_config.m_screen);

			// set the timer to to fire immediately
			m_first_time = true;
			m_timer->adjust(attotime::zero, m_config.m_param);
			break;
	}
}


//-------------------------------------------------
//  device_timer - handle timer expiration events
//-------------------------------------------------

void timer_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (m_config.m_type)
	{
		// general periodic timers just call through
		case timer_device_config::TIMER_TYPE_GENERIC:
		case timer_device_config::TIMER_TYPE_PERIODIC:
			if (m_config.m_type != timer_device_config::TIMER_TYPE_SCANLINE)
			{
				if (m_config.m_callback != NULL)
					(*m_config.m_callback)(*this, m_ptr, param);
			}
			break;


		// scanline timers have to do some additiona bookkeeping
		case timer_device_config::TIMER_TYPE_SCANLINE:
		{
			// by default, we fire at the first position
			int next_vpos = m_config.m_first_vpos;

			// the first time through we just go with the default position
			if (!m_first_time)
			{
				// call the real callback
				int vpos = m_screen->vpos();
				(*m_config.m_callback)(*this, m_ptr, vpos);

				// advance by the increment only if we will still be within the screen bounds
		        if (m_config.m_increment != 0 && (vpos + m_config.m_increment) < m_screen->height())
					next_vpos = vpos + m_config.m_increment;
			}
			m_first_time = false;

			// adjust the timer
			m_timer->adjust(m_screen->time_until_pos(next_vpos));
			break;
		}
	}
}
