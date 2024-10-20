// license:BSD-3-Clause
// copyright-holders:David Haywood
#include "emu.h"
#include "xavix_mtrk_wheel.h"

DEFINE_DEVICE_TYPE(XAVIX_MTRK_WHEEL, xavix_mtrk_wheel_device, "xavix_mtrk_wheel", "XaviX / Radica Monster Truck Steering Wheel")

xavix_mtrk_wheel_device::xavix_mtrk_wheel_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, XAVIX_MTRK_WHEEL, tag, owner, clock),
	m_event_out_cb(*this),
	m_in(*this, "WHEEL")
{
}

TIMER_CALLBACK_MEMBER(xavix_mtrk_wheel_device::event_timer)
{
	m_event_out_cb(1);
	m_is_running = 0;
	check_wheel();
}

void xavix_mtrk_wheel_device::check_wheel()
{
	uint8_t wheelval = m_in->read();

	if (wheelval == 0x80)
	{
		m_event_timer->adjust(attotime::never, 0);
		m_direction = 0;
		m_is_running = 0;
	}
	else if (wheelval > 0x80)
	{
		wheelval = wheelval - 0x80;
		m_direction = 1;
		// TODO: set frequency based on value
		m_event_timer->adjust(attotime::from_hz(20), 0);
		m_is_running = 1;
	}
	else
	{
		wheelval = 0x7f - wheelval;
		m_direction = 0;
		// TODO: set frequency based on value
		m_event_timer->adjust(attotime::from_hz(20), 0);
		m_is_running = 1;
	}
}

INPUT_CHANGED_MEMBER( xavix_mtrk_wheel_device::changed )
{
	// this could happen while the timer is still active, which could end up cancelling it in flight
	// should probably calculate adjustment based on current expiry?
	// instead I'm just making sure the timer isn't running right now
	//printf("wheel changed to %02x\n", m_in->read());
	if (!m_is_running)
	{
		check_wheel();
	}
}


static INPUT_PORTS_START( wheel )
	PORT_START("WHEEL")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_MINMAX(0x20,0xe0) PORT_SENSITIVITY(100) PORT_KEYDELTA(4) PORT_CENTERDELTA(20) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(xavix_mtrk_wheel_device::changed), 0)
INPUT_PORTS_END


ioport_constructor xavix_mtrk_wheel_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(wheel);
}


void xavix_mtrk_wheel_device::device_start()
{
	m_event_timer = timer_alloc(FUNC(xavix_mtrk_wheel_device::event_timer), this);
}

void xavix_mtrk_wheel_device::device_reset()
{
	m_direction = 0;
	m_event_timer->adjust(attotime::never, 0);
	m_is_running = 0;
}

int xavix_mtrk_wheel_device::read_direction()
{
	return m_direction;
}
