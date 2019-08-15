// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Atari Video Computer System lightpen emulation

**********************************************************************/

#include "emu.h"
#include "lightpen.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(VCS_LIGHTPEN, vcs_lightpen_device, "vcs_lightpen", "Atari / CBM Light Pen")


INPUT_CHANGED_MEMBER( vcs_lightpen_device::trigger )
{
	// TODO trigger timer at correct screen position
	m_port->trigger_w(newval);
}


static INPUT_PORTS_START( vcs_lightpen )
	PORT_START("JOY")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, vcs_lightpen_device, trigger, 0)
	PORT_BIT( 0xdf, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("LIGHTX")
	PORT_BIT( 0xff, 0x00, IPT_LIGHTGUN_X) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(45) PORT_KEYDELTA(15)

	PORT_START("LIGHTY")
	PORT_BIT( 0xff, 0x00, IPT_LIGHTGUN_Y) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(45) PORT_KEYDELTA(15)
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor vcs_lightpen_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( vcs_lightpen );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  vcs_lightpen_device - constructor
//-------------------------------------------------

vcs_lightpen_device::vcs_lightpen_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, VCS_LIGHTPEN, tag, owner, clock),
	device_vcs_control_port_interface(mconfig, *this),
	m_joy(*this, "JOY"),
	m_lightx(*this, "LIGHTX"),
	m_lighty(*this, "LIGHTY")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vcs_lightpen_device::device_start()
{
}


//-------------------------------------------------
//  vcs_joy_r - lightpen read
//-------------------------------------------------

uint8_t vcs_lightpen_device::vcs_joy_r()
{
	return m_joy->read();
}
