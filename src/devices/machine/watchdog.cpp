// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    watchdog.c

    Watchdog timer device.

***************************************************************************/

#include "emu.h"
#include "emuopts.h"
#include "watchdog.h"


//**************************************************************************
//  WATCHDOG TIMER DEVICE
//**************************************************************************

const device_type WATCHDOG_TIMER = &device_creator<watchdog_timer_device>;

//-------------------------------------------------
//  watchdog_timer_device - constructor
//-------------------------------------------------

watchdog_timer_device::watchdog_timer_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, WATCHDOG_TIMER, "Watchdog timer", tag, owner, clock, "watchdog", __FILE__),
		m_vblank_count(0),
		m_time(attotime::zero),
		m_screen_tag(nullptr)
{
}


//-------------------------------------------------
//  static_set_vblank_count - configuration helper
//  to set the number of VBLANKs
//-------------------------------------------------

void watchdog_timer_device::static_set_vblank_count(device_t &device, const char *screen_tag, INT32 count)
{
	watchdog_timer_device &watchdog = downcast<watchdog_timer_device &>(device);
	watchdog.m_screen_tag = screen_tag;
	watchdog.m_vblank_count = count;
}


//-------------------------------------------------
//  static_set_time - configuration helper to set
//  the time until reset
//-------------------------------------------------

void watchdog_timer_device::static_set_time(device_t &device, attotime time)
{
	watchdog_timer_device &watchdog = downcast<watchdog_timer_device &>(device);
	watchdog.m_time = time;
}


//-------------------------------------------------
//  device_validity_check - validate the device
//  configuration
//-------------------------------------------------

void watchdog_timer_device::device_validity_check(validity_checker &valid) const
{
	if (m_vblank_count != 0)
	{
		screen_device *screen = dynamic_cast<screen_device *>(siblingdevice(m_screen_tag));
		if (screen == nullptr)
			osd_printf_error("Invalid screen tag specified\n");
	}
}


//-------------------------------------------------
//  device_start - perform device-specific
//  startup
//-------------------------------------------------

void watchdog_timer_device::device_start()
{
	// initialize the watchdog
	m_counter = 0;
	m_timer = timer_alloc();

	if (m_vblank_count != 0)
	{
		// fetch the screen
		screen_device *screen = siblingdevice<screen_device>(m_screen_tag);
		if (screen != nullptr)
			screen->register_vblank_callback(vblank_state_delegate(FUNC(watchdog_timer_device::watchdog_vblank), this));
	}
	save_item(NAME(m_enabled));
	save_item(NAME(m_counter));
}


//-------------------------------------------------
//  device_reset - reset the device
//-------------------------------------------------

void watchdog_timer_device::device_reset()
{
	// set up the watchdog timer; only start off enabled if explicitly configured
	m_enabled = (m_vblank_count != 0 || m_time != attotime::zero);
	watchdog_reset();
	m_enabled = true;
}


//-------------------------------------------------
//  device_timer - handle timer expiration events
//-------------------------------------------------

void watchdog_timer_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	watchdog_fired();
}


//-------------------------------------------------
//  watchdog_reset - reset the watchdog timer
//-------------------------------------------------

void watchdog_timer_device::watchdog_reset()
{
	// if we're not enabled, skip it
	if (!m_enabled)
		m_timer->adjust(attotime::never);

	// VBLANK-based watchdog?
	else if (m_vblank_count != 0)
		m_counter = m_vblank_count;

	// timer-based watchdog?
	else if (m_time != attotime::zero)
		m_timer->adjust(m_time);

	// default to an obscene amount of time (3 seconds)
	else
		m_timer->adjust(attotime::from_seconds(3));
}


//-------------------------------------------------
//  watchdog_enable - reset the watchdog timer
//-------------------------------------------------

void watchdog_timer_device::watchdog_enable(bool enable)
{
	// when re-enabled, we reset our state
	if (m_enabled != enable)
	{
		m_enabled = enable;
		watchdog_reset();
	}
}


//-------------------------------------------------
//  watchdog_fired - trigger machine reset
//-------------------------------------------------

void watchdog_timer_device::watchdog_fired()
{
	logerror("Reset caused by the watchdog!!!\n");

	bool verbose = machine().options().verbose();
#ifdef MAME_DEBUG
	verbose = true;
#endif
	if (verbose)
		popmessage("Reset caused by the watchdog!!!\n");

	machine().schedule_soft_reset();
}


//-------------------------------------------------
//  watchdog_vblank - VBLANK state callback for
//  watchdog timers
//-------------------------------------------------

void watchdog_timer_device::watchdog_vblank(screen_device &screen, bool vblank_state)
{
	// VBLANK starting
	if (vblank_state && m_enabled)
	{
		// check the watchdog
		if (m_vblank_count != 0)
			if (--m_counter == 0)
				watchdog_fired();
	}
}


//**************************************************************************
//  WATCHDOG READ/WRITE HELPERS
//**************************************************************************

//-------------------------------------------------
//  8-bit reset read/write handlers
//-------------------------------------------------

WRITE8_MEMBER( watchdog_timer_device::reset_w ) { watchdog_reset(); }
READ8_MEMBER( watchdog_timer_device::reset_r ) { watchdog_reset(); return space.unmap(); }


//-------------------------------------------------
//  16-bit reset read/write handlers
//-------------------------------------------------

WRITE16_MEMBER( watchdog_timer_device::reset16_w ) { watchdog_reset(); }
READ16_MEMBER( watchdog_timer_device::reset16_r ) { watchdog_reset(); return space.unmap(); }


//-------------------------------------------------
//  32-bit reset read/write handlers
//-------------------------------------------------

WRITE32_MEMBER( watchdog_timer_device::reset32_w ) { watchdog_reset(); }
READ32_MEMBER( watchdog_timer_device::reset32_r ) { watchdog_reset(); return space.unmap(); }
