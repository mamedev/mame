// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Acorn Archimedes RTFM Joystick Interface

**********************************************************************/

#include "emu.h"
#include "rtfmjoy.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(ARC_RTFM_JOY, arc_rtfm_joystick_device, "arc_rtfmjoy", "Acorn Archimedes RTFM Joystick Interface");


//-------------------------------------------------
//  INPUT_PORTS( rtfm_joystick )
//-------------------------------------------------

static INPUT_PORTS_START( rtfm_joystick )
	PORT_START("JOY0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("Fire") PORT_PLAYER(1)

	PORT_START("JOY1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("Fire") PORT_PLAYER(2)
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor arc_rtfm_joystick_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( rtfm_joystick );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  archimedes_rtfm_joystick_device - constructor
//-------------------------------------------------

arc_rtfm_joystick_device::arc_rtfm_joystick_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, ARC_RTFM_JOY, tag, owner, clock)
	, device_archimedes_econet_interface(mconfig, *this)
	, m_joy(*this, "JOY%u", 0U)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void arc_rtfm_joystick_device::device_start()
{
}

//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

u8 arc_rtfm_joystick_device::read(offs_t offset)
{
	u8 data = 0xff;

	switch (offset & 0x03)
	{
	case 0x01:
		data = ~m_joy[0]->read() & 0x1f;
		break;
	case 0x02:
		data = ~m_joy[1]->read() & 0x1f;
		break;
	}

	return data;
}
