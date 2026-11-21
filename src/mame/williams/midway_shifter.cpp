// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

    Midway driving game gear shifter

***************************************************************************/

#include "emu.h"
#include "midway_shifter.h"


DEFINE_DEVICE_TYPE(MIDWAY_SHIFTER, midway_shifter_device, "midway_shifter", "Midway gear shifter")


midway_shifter_device::midway_shifter_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MIDWAY_SHIFTER, tag, owner, clock)
	, m_conf(*this, "^CONF")
	, m_initial_gear(0)
	, m_gear(0)
	, m_held(0)
{
}


INPUT_PORTS_START( midway_shifter )
	PORT_START("CONF")
	PORT_CONFNAME( 0x07, 0x01, "Shifter Type" )
	PORT_CONFSETTING(    0x01, "Buttons (sticky)" )   // for convenience for people without a shifter
	PORT_CONFSETTING(    0x03, "Buttons (toggling)" ) // avoids the need for the neutral button
	PORT_CONFSETTING(    0x05, "Sequential" )         // for spring-return shifter, paddles or buttons
	PORT_CONFSETTING(    0x00, "H-Pattern" )          // for use with a real shifter
INPUT_PORTS_END

void midway_shifter_device::device_start()
{
	save_item(NAME(m_gear));
	save_item(NAME(m_held));
}

void midway_shifter_device::device_reset()
{
	// the buttons held down don't change across a reset
	m_gear = m_initial_gear;
}


ioport_value midway_shifter_device::gears_r()
{
	// a real shifter closes the switches directly
	if (!BIT(m_conf->read(), 0))
		return m_held;

	return m_gear ? (1 << (m_gear - 1)) : 0;
}

INPUT_CHANGED_MEMBER(midway_shifter_device::gear_button)
{
	if (param)
	{
		if (newval)
			m_held |= 1 << (param - 1);
		else
			m_held &= ~(1 << (param - 1));
	}

	if (newval)
	{
		if ((param != m_gear) || !BIT(m_conf->read(), 1))
			m_gear = param;
		else
			m_gear = 0;
	}
}

INPUT_CHANGED_MEMBER(midway_shifter_device::shift_button)
{
	if (newval)
	{
		if (!param)
		{
			if (m_gear)
				--m_gear;
		}
		else if (m_gear < 4)
		{
			++m_gear;
		}
	}
}
