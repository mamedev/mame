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


static INPUT_PORTS_START( xavix_madfb_ball )
	PORT_START("BALL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Ball Function") PORT_PLAYER(1) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(xavix_madfb_ball_device::changed), 0) PORT_CODE(KEYCODE_LALT) PORT_CODE(MOUSECODE_BUTTON3_INDEXED(0)) PORT_CODE(GUNCODE_BUTTON2_INDEXED(0))
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Ball Motion") PORT_PLAYER(1) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(xavix_madfb_ball_device::changed), 0) PORT_CODE(KEYCODE_SPACE) PORT_CODE(MOUSECODE_BUTTON2_INDEXED(0))
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

ioport_constructor xavix_madfb_ball_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(xavix_madfb_ball);
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
