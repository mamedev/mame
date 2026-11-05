// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn
/***************************************************************************

    XaviX and Hasbro Mini Golf putter sensor. Tracks a swing's peak strength
    and returns a timed analogue pulse while the console enables the optical
    emitter, allowing its ADC polling loop to classify the stroke.

    Pulse durations are representative values for the six strength ranges
    decoded by the ROM.

    TODO:
    - Measure physical putter-sensor pulse durations.

***************************************************************************/

#include "emu.h"
#include "xavix_io_wamg_sensor.h"

DEFINE_DEVICE_TYPE(XAVIX_WAMG_SENSOR, xavix_wamg_sensor_device, "xavix_wamg_sensor", "XaviX / Hasbro Mini Golf Putter Sensor")

xavix_wamg_sensor_device::xavix_wamg_sensor_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, XAVIX_WAMG_SENSOR, tag, owner, clock)
	, m_strength(*this, "SWING")
{
}

static INPUT_PORTS_START( xavix_wamg_sensor )
	PORT_START("SWING")
	PORT_BIT( 0x07, 0x00, IPT_PEDAL ) PORT_NAME("Putter Swing Strength") PORT_MINMAX(0, 6) PORT_SENSITIVITY(3) PORT_KEYDELTA(1) PORT_CENTERDELTA(6) PORT_CODE_INC(KEYCODE_SPACE)
	PORT_BIT( 0xf8, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

ioport_constructor xavix_wamg_sensor_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(xavix_wamg_sensor);
}

void xavix_wamg_sensor_device::device_start()
{
	m_pulse_timer = timer_alloc(FUNC(xavix_wamg_sensor_device::pulse_end), this);

	save_item(NAME(m_peak));
	save_item(NAME(m_armed));
	save_item(NAME(m_pulse_active));
	save_item(NAME(m_emitter));
}

void xavix_wamg_sensor_device::device_reset()
{
	m_pulse_timer->adjust(attotime::never);
	m_peak = 0;
	m_armed = !(m_strength->read() & 0x07);
	m_pulse_active = false;
	m_emitter = false;
}

u8 xavix_wamg_sensor_device::adc_r()
{
	return (m_pulse_active && m_emitter) ? 0xc0 : 0x40;
}

void xavix_wamg_sensor_device::vblank_w(int state)
{
	if (!state)
		return;

	u8 const strength = m_strength->read() & 0x07;
	if (m_pulse_active)
		return;

	if (!m_armed)
	{
		if (!strength)
			m_armed = true;
		return;
	}

	if (strength)
	{
		if (strength > m_peak)
			m_peak = strength;
		return;
	}

	if (!m_peak)
		return;

	// These representative durations reach the six ROM strength bins; hardware timing is not yet measured.
	m_pulse_timer->adjust(attotime::from_ticks(u64(PULSE_COUNTS[m_peak]) * CLOCKS_PER_COUNT, clock()));
	m_peak = 0;
	m_armed = false;
	m_pulse_active = true;
}

TIMER_CALLBACK_MEMBER(xavix_wamg_sensor_device::pulse_end)
{
	m_pulse_active = false;
}
