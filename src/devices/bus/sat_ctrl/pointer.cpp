// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Sega Saturn Pointing Controller / Trackball emulation

    This is basically the same as a mouse, but it uses a different ID

**********************************************************************/

#include "pointer.h"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type SATURN_TRACK = &device_creator<saturn_track_device>;


static INPUT_PORTS_START( saturn_track )
	PORT_START("POINT_X")
	PORT_BIT(0xffff, 0x00, IPT_MOUSE_X) PORT_SENSITIVITY(100) PORT_MINMAX(0x000, 0xffff) PORT_KEYDELTA(2) PORT_RESET PORT_NAME("Pointer X")

	PORT_START("POINT_Y")
	PORT_BIT(0xffff, 0x00, IPT_MOUSE_Y) PORT_SENSITIVITY(100) PORT_MINMAX(0x000, 0xffff) PORT_KEYDELTA(2) PORT_RESET PORT_REVERSE PORT_NAME("Pointer Y")

	PORT_START("BUTTONS")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Left Button")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Right Button")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Middle Button")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_START ) PORT_NAME("Start Button")
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor saturn_track_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( saturn_track );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  saturn_track_device - constructor
//-------------------------------------------------

saturn_track_device::saturn_track_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
					device_t(mconfig, SATURN_TRACK, "Sega Saturn Pointing Controller / Trackball", tag, owner, clock, "saturn_track", __FILE__),
					device_saturn_control_port_interface(mconfig, *this),
					m_pointx(*this, "POINT_X"),
					m_pointy(*this, "POINT_Y"),
					m_buttons(*this, "BUTTONS")
{
	m_ctrl_id = 0x23;
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void saturn_track_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void saturn_track_device::device_reset()
{
}


//-------------------------------------------------
//  read_ctrl
//-------------------------------------------------

UINT8 saturn_track_device::read_ctrl(UINT8 offset)
{
	UINT8 res = 0;
	UINT8 mouse_ctrl = m_buttons->read();
	INT16 mouse_x = m_pointx->read();
	INT16 mouse_y = m_pointy->read();

	if (mouse_x < 0)
		mouse_ctrl |= 0x10;

	if (mouse_y < 0)
		mouse_ctrl |= 0x20;

	if ((mouse_x & 0xff00) != 0xff00 && (mouse_x & 0xff00) != 0x0000)
		mouse_ctrl |= 0x40;

	if ((mouse_y & 0xff00) != 0xff00 && (mouse_y & 0xff00) != 0x0000)
		mouse_ctrl |= 0x80;

	switch (offset)
	{
		case 0:
		default:
			res = mouse_ctrl;
			break;
		case 1:
			res = mouse_x & 0xff;
			break;
		case 2:
			res = mouse_y & 0xff;
			break;
	}
	return res;
}
