// license:BSD-3-Clause
// copyright-holders:David Haywood, MagikalUnicorn
/***************************************************************************

    XaviX and Radica Monster Truck steering-wheel interface. Exposes steering
    direction as a GPIO level and generates periodic I/O events at a rate
    proportional to displacement from centre. The model uses one event per
    second per input step.

    TODO:
    - Measure the physical event-rate scale.

***************************************************************************/

#include "emu.h"
#include "xavix_io_mtrk_wheel.h"

DEFINE_DEVICE_TYPE(XAVIX_MTRK_WHEEL, xavix_mtrk_wheel_device, "xavix_mtrk_wheel", "XaviX / Radica Monster Truck Steering Wheel")

xavix_mtrk_wheel_device::xavix_mtrk_wheel_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, XAVIX_MTRK_WHEEL, tag, owner, clock),
	m_event_out_cb(*this),
	m_in(*this, "WHEEL"),
	m_direction(0),
	m_rate(0)
{
}

static INPUT_PORTS_START( wheel )
	PORT_START("WHEEL")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_NAME("Steering Wheel") PORT_MINMAX(0x20,0xe0) PORT_SENSITIVITY(100) PORT_KEYDELTA(4) PORT_CENTERDELTA(20) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(xavix_mtrk_wheel_device::changed), 0)
INPUT_PORTS_END




ioport_constructor xavix_mtrk_wheel_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(wheel);
}


void xavix_mtrk_wheel_device::device_start()
{
	m_event_timer = timer_alloc(FUNC(xavix_mtrk_wheel_device::event_timer), this);

	save_item(NAME(m_direction));
	save_item(NAME(m_rate));
}

void xavix_mtrk_wheel_device::device_reset()
{
	m_direction = 0;
	m_rate = 0;
	update_event_rate();
}

TIMER_CALLBACK_MEMBER(xavix_mtrk_wheel_device::event_timer)
{
	m_event_out_cb(1);
}

void xavix_mtrk_wheel_device::update_event_rate()
{
	u8 const wheel = m_in->read();
	u8 const rate = (wheel < 0x80) ? (0x80 - wheel) : (wheel - 0x80);

	if (!rate)
	{
		m_event_timer->adjust(attotime::never, 0);
		m_direction = 0;
		m_rate = 0;
		return;
	}

	// Direction is a GPIO level; distance from centre controls the event rate.
	m_direction = wheel > 0x80;
	// One event per second per input step is provisional pending measurements from original hardware.
	attotime const period = attotime::from_hz(rate);
	attotime remaining = period;
	if (m_rate)
	{
		attotime const old_period = attotime::from_hz(m_rate);
		remaining *= m_event_timer->remaining().as_double() / old_period.as_double();
	}
	m_event_timer->adjust(remaining, 0, period);
	m_rate = rate;
}

INPUT_CHANGED_MEMBER(xavix_mtrk_wheel_device::changed)
{
	update_event_rate();
}

int xavix_mtrk_wheel_device::direction_r()
{
	return m_direction;
}
