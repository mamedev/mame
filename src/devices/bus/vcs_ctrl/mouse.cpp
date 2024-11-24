// license:BSD-3-Clause
// copyright-holders:Curt Coder, hap
/**********************************************************************

Mouse emulation (Commodore 1351 or compatible)

At power-on, holding the right mouse button will put it in joystick mode.
In proportional mode, left button goes to pin #6, right button to pin #1.
POT(X/Y) d1-d6 holds the current direction, d7 unused, d0 is a 'noise bit'.

TODO:
- joystick mode

**********************************************************************/

#include "emu.h"
#include "mouse.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(VCS_MOUSE, vcs_mouse_device, "vcs_mouse", "Atari / CBM Mouse")


static INPUT_PORTS_START( vcs_mouse )
	PORT_START("JOY")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_WRITE_LINE_MEMBER(FUNC(vcs_mouse_device::trigger_w))
	PORT_BIT( 0xde, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("POTX")
	PORT_BIT( 0x3f, 0x00, IPT_MOUSE_X) PORT_SENSITIVITY(15) PORT_KEYDELTA(10)

	PORT_START("POTY")
	PORT_BIT( 0x3f, 0x00, IPT_MOUSE_Y) PORT_SENSITIVITY(15) PORT_KEYDELTA(10) PORT_REVERSE
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor vcs_mouse_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( vcs_mouse );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  vcs_mouse_device - constructor
//-------------------------------------------------

vcs_mouse_device::vcs_mouse_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, VCS_MOUSE, tag, owner, clock),
	device_vcs_control_port_interface(mconfig, *this),
	m_joy(*this, "JOY"),
	m_potx(*this, "POTX"),
	m_poty(*this, "POTY")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vcs_mouse_device::device_start()
{
}


//-------------------------------------------------
//  vcs_joy_r - joystick read
//-------------------------------------------------

uint8_t vcs_mouse_device::vcs_joy_r()
{
	return m_joy->read();
}


//-------------------------------------------------
//  vcs_pot_x_r - potentiometer X read
//-------------------------------------------------

uint8_t vcs_mouse_device::vcs_pot_x_r()
{
	return m_potx->read() << 1;
}


//-------------------------------------------------
//  vcs_pot_y_r - potentiometer Y read
//-------------------------------------------------

uint8_t vcs_mouse_device::vcs_pot_y_r()
{
	return m_poty->read() << 1;
}
