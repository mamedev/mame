// license:BSD-3-Clause
// copyright-holders:David Haywood, MagikalUnicorn
/***************************************************************************

    Motion Football input for XaviX-based EA Sports Madden Football and Play
    TV Football systems. The ball's infrared LEDs encode Function and Motion
    as three event rates, delivered to the console's I/O event input.

***************************************************************************/

#include "emu.h"
#include "xavix_io_madf_ball.h"

DEFINE_DEVICE_TYPE(XAVIX_MADFB_BALL, xavix_madfb_ball_device, "xavix_madfb_ball", "XaviX / Radica Football Ball")

xavix_madfb_ball_device::xavix_madfb_ball_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, XAVIX_MADFB_BALL, tag, owner, clock),
	m_event_out_cb(*this),
	m_in(*this, "BALL")
{
}


void xavix_madfb_ball_device::set_input_source(device_t &source)
{
	m_in.set_tag(source, m_in.finder_tag());
}

void xavix_madfb_ball_device::device_start()
{
	m_event_timer = timer_alloc(FUNC(xavix_madfb_ball_device::event_timer), this);
}

void xavix_madfb_ball_device::device_reset()
{
	update_event_rate();
}

TIMER_CALLBACK_MEMBER(xavix_madfb_ball_device::event_timer)
{
	m_event_out_cb(1);
}

void xavix_madfb_ball_device::update_event_rate()
{
	u8 const ball = m_in->read() & 3;

	if (!ball)
		m_event_timer->adjust(attotime::never, 0);
	else
	{
		// Long, middle and short periods encode Function, Motion and both; Motion also calibrates the receiver.
		attotime const period = attotime::from_ticks(PULSE_PERIODS[ball] * 2048, clock());
		m_event_timer->adjust(period, 0, period);
	}
}

INPUT_CHANGED_MEMBER(xavix_madfb_ball_device::changed)
{
	update_event_rate();
}
