/**********************************************************************

    Atari Video Computer System lightpen emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#include "vcs_lightpen.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type VCS_LIGHTPEN = &device_creator<vcs_lightpen_device>;


INPUT_CHANGED_MEMBER( vcs_lightpen_device::trigger )
{
	// TODO trigger timer at correct screen position
}


static INPUT_PORTS_START( vcs_lightpen )
	PORT_START("JOY")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, vcs_lightpen_device, trigger, 0)
	PORT_BIT( 0xdf, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("LIGHTX")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X) PORT_NAME("Lightpen X Axis") PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(30) PORT_KEYDELTA(20) PORT_PLAYER(1)

	PORT_START("LIGHTY")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y) PORT_NAME("Lightpen Y Axis") PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(30) PORT_KEYDELTA(20) PORT_PLAYER(1)
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

vcs_lightpen_device::vcs_lightpen_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, VCS_LIGHTPEN, "Light Pen", tag, owner, clock),
	device_vcs_control_port_interface(mconfig, *this)
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

UINT8 vcs_lightpen_device::vcs_joy_r()
{
	return ioport("JOY")->read();
}
