// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Sega Saturn MD Joypad (3 buttons & 6 buttons) emulation

**********************************************************************/

#include "joy_md.h"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type SATURN_JOYMD3B = &device_creator<saturn_joymd3b_device>;
const device_type SATURN_JOYMD6B = &device_creator<saturn_joymd6b_device>;

static INPUT_PORTS_START( saturn_md3b )
	PORT_START("JOY")
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_START )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("A")
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("C")
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("B")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED ) //reads '1' when direct mode is polled
INPUT_PORTS_END

static INPUT_PORTS_START( saturn_md6b )
	PORT_START("JOY")
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_START )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("A")
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("C")
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("B")
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_NAME("Mode")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("X")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Y")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("Z")
	PORT_BIT( 0x000f, IP_ACTIVE_LOW, IPT_UNUSED ) //reads '1' when direct mode is polled
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor saturn_joymd3b_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( saturn_md3b );
}

ioport_constructor saturn_joymd6b_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( saturn_md6b );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  constructors
//-------------------------------------------------

saturn_joymd3b_device::saturn_joymd3b_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
					device_t(mconfig, SATURN_JOYMD3B, "Sega Saturn Joypad MD 3buttons", tag, owner, clock, "saturn_md3b", __FILE__),
					device_saturn_control_port_interface(mconfig, *this),
					m_joy(*this, "JOY")
{
	m_ctrl_id = 0xe1;
}


saturn_joymd6b_device::saturn_joymd6b_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
					device_t(mconfig, SATURN_JOYMD6B, "Sega Saturn Joypad MD 6buttons", tag, owner, clock, "saturn_md6b", __FILE__),
					device_saturn_control_port_interface(mconfig, *this),
					m_joy(*this, "JOY")
{
	m_ctrl_id = 0xe2;
}


//-------------------------------------------------
//  read_ctrl
//-------------------------------------------------

UINT8 saturn_joymd3b_device::read_ctrl(UINT8 offset)
{
	UINT8 res = 0;
	switch (offset)
	{
		case 0:
		default:
			res = m_joy->read() >> 8;
			break;
	}
	return res;
}

UINT8 saturn_joymd6b_device::read_ctrl(UINT8 offset)
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
	}
	return res;
}
