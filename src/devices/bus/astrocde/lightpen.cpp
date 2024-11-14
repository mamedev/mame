// license:BSD-3-Clause
// copyright-holders:Ryan Holtz

#include "emu.h"
#include "lightpen.h"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(ASTROCADE_LIGHTPEN, astrocade_lightpen_device, "astrocade_lightpen", "Bally Astrocade Light Pen")


//**************************************************************************
//    Bally Astrocade light pen input
//**************************************************************************

astrocade_lightpen_device::astrocade_lightpen_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ASTROCADE_LIGHTPEN, tag, owner, clock)
	, device_astrocade_accessory_interface(mconfig, *this)
	, m_trigger(*this, "TRIGGER")
	, m_lightx(*this, "LIGHTX")
	, m_lighty(*this, "LIGHTY")
	, m_pen_timer(nullptr)
{
}

astrocade_lightpen_device::~astrocade_lightpen_device()
{
}

void astrocade_lightpen_device::device_start()
{
	m_pen_timer = timer_alloc(FUNC(astrocade_lightpen_device::trigger_tick), this);

	save_item(NAME(m_retrigger));
}

void astrocade_lightpen_device::device_reset()
{
	m_pen_timer->adjust(attotime::never);
	m_retrigger = false;
}

TIMER_CALLBACK_MEMBER(astrocade_lightpen_device::trigger_tick)
{
	write_ltpen(1);
	write_ltpen(0);
	if (m_retrigger)
		m_pen_timer->adjust(m_screen->time_until_pos(m_lighty->read(), m_lightx->read()));
	else
		m_pen_timer->adjust(attotime::never);
}

INPUT_CHANGED_MEMBER(astrocade_lightpen_device::trigger)
{
	if (newval)
	{
		m_retrigger = true;
		m_pen_timer->adjust(m_screen->time_until_pos(m_lighty->read(), m_lightx->read()));
	}
	else
	{
		m_retrigger = false;
		m_pen_timer->adjust(attotime::never);
	}
}

static INPUT_PORTS_START( astrocade_lightpen )
	PORT_START("TRIGGER")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(astrocade_lightpen_device::trigger), 0)
	PORT_BIT( 0xfe, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("LIGHTX")
	PORT_BIT(0x1ff, 0x000, IPT_LIGHTGUN_X) PORT_MINMAX(0x000, 0x15f) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(45) PORT_KEYDELTA(15)

	PORT_START("LIGHTY")
	PORT_BIT(0x0ff, 0x000, IPT_LIGHTGUN_Y) PORT_MINMAX(0x000, 0x0f0) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(45) PORT_KEYDELTA(15)
INPUT_PORTS_END

ioport_constructor astrocade_lightpen_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( astrocade_lightpen );
}
