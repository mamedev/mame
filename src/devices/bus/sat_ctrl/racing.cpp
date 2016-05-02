// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Sega Saturn Racing Wheel emulation

**********************************************************************/

#include "racing.h"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type SATURN_WHEEL = &device_creator<saturn_wheel_device>;


static INPUT_PORTS_START( saturn_racing )
	PORT_START("JOY")
	PORT_BIT(0x8000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT)
	PORT_BIT(0x4000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT)
	PORT_BIT(0x2000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN)
	PORT_BIT(0x1000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP)
	PORT_BIT(0x0800, IP_ACTIVE_LOW, IPT_START)
	PORT_BIT(0x0400, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("A")
	PORT_BIT(0x0200, IP_ACTIVE_LOW, IPT_BUTTON3) PORT_NAME("C")
	PORT_BIT(0x0100, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_NAME("B")
	PORT_BIT(0x0080, IP_ACTIVE_LOW, IPT_BUTTON8) PORT_NAME("R")
	PORT_BIT(0x0040, IP_ACTIVE_LOW, IPT_BUTTON4) PORT_NAME("X")
	PORT_BIT(0x0020, IP_ACTIVE_LOW, IPT_BUTTON5) PORT_NAME("Y")
	PORT_BIT(0x0010, IP_ACTIVE_LOW, IPT_BUTTON6) PORT_NAME("Z")
	PORT_BIT(0x0008, IP_ACTIVE_LOW, IPT_BUTTON7) PORT_NAME("L")
	// Note: unused bits must stay high, Bug 2 relies on this.
	PORT_BIT(0x0007, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("ANALOG_X")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_MINMAX(0x00, 0xff) PORT_SENSITIVITY(25) PORT_KEYDELTA(200) PORT_NAME("Racing Stick X")
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor saturn_wheel_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( saturn_racing );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  saturn_wheel_device - constructor
//-------------------------------------------------

saturn_wheel_device::saturn_wheel_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
					device_t(mconfig, SATURN_WHEEL, "Sega Saturn Racing Wheel", tag, owner, clock, "saturn_racing", __FILE__),
					device_saturn_control_port_interface(mconfig, *this),
					m_joy(*this, "JOY"),
					m_anx(*this, "ANALOG_X")
{
	m_ctrl_id = 0x13;
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void saturn_wheel_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void saturn_wheel_device::device_reset()
{
}


//-------------------------------------------------
//  read_ctrl
//-------------------------------------------------

UINT8 saturn_wheel_device::read_ctrl(UINT8 offset)
{
	UINT8 res = 0;
	switch (offset)
	{
		case 0:
		default:
			res = m_joy->read() >> 8;
			break;
		case 1:
			res = m_joy->read() & 0xff;
			break;
		case 2:
			res = m_anx->read();
			break;
	}
	return res;
}
