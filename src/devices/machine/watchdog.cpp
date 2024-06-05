// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Watchdog timer device.

***************************************************************************/

#include "emu.h"
#include "watchdog.h"

#include "emuopts.h"
#include "screen.h"


//**************************************************************************
//  WATCHDOG TIMER DEVICE
//**************************************************************************

DEFINE_DEVICE_TYPE(WATCHDOG_TIMER, watchdog_timer_device, "watchdog", "Watchdog Timer")

//-------------------------------------------------
//  watchdog_timer_device - constructor
//-------------------------------------------------

watchdog_timer_device::watchdog_timer_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, WATCHDOG_TIMER, tag, owner, clock)
	, m_vblank_count(0)
	, m_time(attotime::zero)
	, m_screen(*this, finder_base::DUMMY_TAG)
	, m_enabled(false)
	, m_reset_line(0)
	, m_counter(0)
{
}


//-------------------------------------------------
//  device_validity_check - validate the device
//  configuration
//-------------------------------------------------

void watchdog_timer_device::device_validity_check(validity_checker &valid) const
{
	if (m_vblank_count != 0)
	{
		if (m_screen.finder_tag() == finder_base::DUMMY_TAG)
			osd_printf_error("VBLANK count set without setting screen tag\n");
		else if (!m_screen)
			osd_printf_error("Screen device %s not found\n", m_screen.finder_tag());
	}
}


//-------------------------------------------------
//  device_start - perform device-specific startup
//-------------------------------------------------

void watchdog_timer_device::device_start()
{
	// initialize the watchdog
	m_timer = timer_alloc(FUNC(watchdog_timer_device::watchdog_expired), this);

	if (m_vblank_count != 0)
	{
		// fetch the screen
		if (m_screen)
			m_screen->register_vblank_callback(vblank_state_delegate(&watchdog_timer_device::watchdog_vblank, this));
	}

	save_item(NAME(m_enabled));
	save_item(NAME(m_reset_line));
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
//  watchdog_expired - handle expired timer
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(watchdog_timer_device::watchdog_expired)
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
//  watchdog_enable - enable the watchdog timer
//-------------------------------------------------

void watchdog_timer_device::watchdog_enable(int state)
{
	const bool enable = bool(state);

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
	logerror("watchdog_fired: reset issued\n");

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

void watchdog_timer_device::reset_w(u8 data)
{
	watchdog_reset();
}

u8 watchdog_timer_device::reset_r(address_space &space)
{
	if (!machine().side_effects_disabled())
		watchdog_reset();

	return space.unmap();
}


//-------------------------------------------------
//  16-bit reset read/write handlers
//-------------------------------------------------

void watchdog_timer_device::reset16_w(u16 data)
{
	watchdog_reset();
}

u16 watchdog_timer_device::reset16_r(address_space &space)
{
	if (!machine().side_effects_disabled())
		watchdog_reset();

	return space.unmap();
}


//-------------------------------------------------
//  32-bit reset read/write handlers
//-------------------------------------------------

void watchdog_timer_device::reset32_w(u32 data)
{
	watchdog_reset();
}

u32 watchdog_timer_device::reset32_r(address_space &space)
{
	if (!machine().side_effects_disabled())
		watchdog_reset();

	return space.unmap();
}


//-------------------------------------------------
//  reset writeline handler
//-------------------------------------------------

void watchdog_timer_device::reset_line_w(int state)
{
	state = state ? 1 : 0;

	// reset watchdog on rising edge
	if (state && !m_reset_line)
		watchdog_reset();

	m_reset_line = state;
}
