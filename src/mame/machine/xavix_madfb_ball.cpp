// license:BSD-3-Clause
// copyright-holders:David Haywood

/*
    Motion Football simulation for XaviX based games
    EA Sports Madden Football
    Play TV Football

    seems to register a pulse when you move it, and another when it stops moving
    game uses current timer register (without interrupt) to measure time between pulses
    to get 'throw' value

    need to verify with code and work out how best to handle this
*/

#include "emu.h"
#include "machine/xavix_madfb_ball.h"

DEFINE_DEVICE_TYPE(XAVIX_MADFB_BALL, xavix_madfb_ball_device, "xavix_madfb_ball", "XaviX / Radica Football Ball")

xavix_madfb_ball_device::xavix_madfb_ball_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, XAVIX_MADFB_BALL, tag, owner, clock),
	m_event_out_cb(*this),
	m_in(*this, "BALL")
{
}

TIMER_CALLBACK_MEMBER(xavix_madfb_ball_device::event_timer)
{
	m_event_out_cb(1);
	m_is_running = 0;
	check_ball();
}

void xavix_madfb_ball_device::check_ball()
{
	uint8_t ballval = m_in->read();

	if (ballval == 0x00)
	{
		m_event_timer->adjust(attotime::never, 0);
		m_is_running = 0;
	}
	else
	{
		// TODO: set frequency based on value
		m_event_timer->adjust(attotime::from_hz(800), 0);
		m_is_running = 1;
	}
}

INPUT_CHANGED_MEMBER( xavix_madfb_ball_device::changed )
{
	// this could happen while the timer is still active, which could end up cancelling it in flight
	// should probably calculate adjustment based on current expiry?
	// instead I'm just making sure the timer isn't running right now
	//printf("ball changed to %02x\n", m_in->read());
	if (!m_is_running)
	{
		check_ball();
	}
}


static INPUT_PORTS_START( ball )
	PORT_START("BALL")
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(20) PORT_CHANGED_MEMBER(DEVICE_SELF, xavix_madfb_ball_device, changed, nullptr)
INPUT_PORTS_END


ioport_constructor xavix_madfb_ball_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(ball);
}


void xavix_madfb_ball_device::device_start()
{
	m_event_out_cb.resolve_safe();
	m_event_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(xavix_madfb_ball_device::event_timer), this));
}

void xavix_madfb_ball_device::device_reset()
{
	m_event_timer->adjust(attotime::never, 0);
	m_is_running = 0;
}
