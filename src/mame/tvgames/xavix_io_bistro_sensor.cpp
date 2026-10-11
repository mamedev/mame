// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn
/***************************************************************************

    Bistro Kids ingredient-sensor interface. Responds to the console's
    excitation signal with a timed sequence of sense-line transitions that
    encodes the selected ingredient for the software's input sampler.

    Response periods are representative values that select the ingredient
    identities decoded by the ROM.

    TODO:
    - Measure original ingredient-sensor response periods.

***************************************************************************/

#include "emu.h"
#include "xavix_io_bistro_sensor.h"

DEFINE_DEVICE_TYPE(XAVIX_BISTRO_SENSOR, xavix_bistro_sensor_device, "xavix_bistro_sensor", "XaviX Bistro Kids Ingredient Sensor")

static INPUT_PORTS_START( xavix_bistro_sensor )
	PORT_START("INGREDIENT")
	PORT_CONFNAME( 0x0f, 0x00, "Ingredient" )
	PORT_CONFSETTING(    0x00, DEF_STR(None) )
	// Representative periods select the ROM's verified identities; original ingredient response timing is unmeasured.
	PORT_CONFSETTING(    0x01, "Potato" )
	PORT_CONFSETTING(    0x02, "Onion" )
	PORT_CONFSETTING(    0x03, "Green Pepper" )
	PORT_CONFSETTING(    0x04, "Tomato" )
	PORT_CONFSETTING(    0x05, "Carrot" )
	PORT_CONFSETTING(    0x06, "Green Onion" )
	PORT_CONFSETTING(    0x07, "Meat" )
	PORT_CONFSETTING(    0x08, "Egg" )
	PORT_CONFSETTING(    0x09, "Cucumber" )
INPUT_PORTS_END

xavix_bistro_sensor_device::xavix_bistro_sensor_device(
		const machine_config &mconfig,
		const char *tag,
		device_t *owner,
		u32 clock)
	: device_t(mconfig, XAVIX_BISTRO_SENSOR, tag, owner, clock)
	, m_ingredient(*this, "INGREDIENT")
	, m_sense(false)
{
}

ioport_constructor xavix_bistro_sensor_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( xavix_bistro_sensor );
}

void xavix_bistro_sensor_device::device_start()
{
	assert(clock());
	m_response_timer = timer_alloc(FUNC(xavix_bistro_sensor_device::response_tick), this);

	save_item(NAME(m_edges_remaining));
	save_item(NAME(m_excite));
	save_item(NAME(m_sense));
}

void xavix_bistro_sensor_device::device_reset()
{
	m_response_timer->adjust(attotime::never);
	m_edges_remaining = 0;
	m_excite = false;
	m_sense = false;
}

void xavix_bistro_sensor_device::excite_w(int state)
{
	bool const excite = bool(state);
	if (m_excite == excite)
		return;

	m_excite = excite;
	if (m_excite)
	{
		m_response_timer->adjust(attotime::never);
		m_edges_remaining = 0;
		m_sense = false;
		return;
	}

	u8 const ingredient = m_ingredient->read() & 0x0f;
	u8 const period = ingredient < std::size(RESPONSE_HALF_PERIOD_TICKS) ? RESPONSE_HALF_PERIOD_TICKS[ingredient] : 0;
	if (!period)
		return;

	// Sixteen transitions fill the firmware's sample buffer.
	m_edges_remaining = 16;
	m_sense = false;
	attotime const interval = clocks_to_attotime(period);
	m_response_timer->adjust(interval, 0, interval);
}

TIMER_CALLBACK_MEMBER(xavix_bistro_sensor_device::response_tick)
{
	m_sense = !m_sense;
	if (!--m_edges_remaining)
		m_response_timer->adjust(attotime::never);
}
