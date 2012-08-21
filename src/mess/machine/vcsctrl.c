/**********************************************************************

    Atari Video Computer System controller port emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#include "machine/vcsctrl.h"



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type VCS_CONTROL_PORT = &device_creator<vcs_control_port_device>;



//**************************************************************************
//  CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_vcs_control_port_interface - constructor
//-------------------------------------------------

device_vcs_control_port_interface::device_vcs_control_port_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig,device)
{
	m_port = dynamic_cast<vcs_control_port_device *>(device.owner());
}


//-------------------------------------------------
//  ~device_vcs_control_port_interface - destructor
//-------------------------------------------------

device_vcs_control_port_interface::~device_vcs_control_port_interface()
{
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  vcs_control_port_device - constructor
//-------------------------------------------------

vcs_control_port_device::vcs_control_port_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
        device_t(mconfig, VCS_CONTROL_PORT, "Atari VCS control port", tag, owner, clock),
		device_slot_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  vcs_control_port_device - destructor
//-------------------------------------------------

vcs_control_port_device::~vcs_control_port_device()
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vcs_control_port_device::device_start()
{
	m_device = dynamic_cast<device_vcs_control_port_interface *>(get_card_device());
}


READ8_MEMBER( vcs_control_port_device::joy_r ) { UINT8 data = 0xff; if (m_device != NULL) data = m_device->vcs_joy_r(); return data; }
READ8_MEMBER( vcs_control_port_device::pot_x_r ) { UINT8 data = 0xff; if (m_device != NULL) data = m_device->vcs_pot_x_r(); return data; }
READ8_MEMBER( vcs_control_port_device::pot_y_r ) { UINT8 data = 0xff; if (m_device != NULL) data = m_device->vcs_pot_y_r(); return data; }
