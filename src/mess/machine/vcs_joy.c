/**********************************************************************

    Atari Video Computer System digital joystick emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#include "vcs_joy.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type VCS_JOYSTICK = &device_creator<vcs_joystick_device>;


static INPUT_PORTS_START( vcs_joystick )
	PORT_START("JOY")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0xd0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor vcs_joystick_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( vcs_joystick );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  vcs_joystick_device - constructor
//-------------------------------------------------

vcs_joystick_device::vcs_joystick_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, VCS_JOYSTICK, "Digital joystick", tag, owner, clock),
	device_vcs_control_port_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vcs_joystick_device::device_start()
{
}


//-------------------------------------------------
//  c64_pb_r - port B read
//-------------------------------------------------

UINT8 vcs_joystick_device::vcs_joy_r()
{
	return ioport("JOY")->read();
}
