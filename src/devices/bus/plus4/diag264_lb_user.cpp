// license:BSD-3-Clause
// copyright-holders:smf
/**********************************************************************

    Diag264 User Port Loop Back Connector emulation

**********************************************************************/

#include "diag264_lb_user.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type DIAG264_USER_PORT_LOOPBACK = &device_creator<diag264_user_port_loopback_device>;



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  diag264_user_port_loopback_device - constructor
//-------------------------------------------------

diag264_user_port_loopback_device::diag264_user_port_loopback_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, DIAG264_USER_PORT_LOOPBACK, "Diag264 User Port Loopback", tag, owner, clock, "diag264_user_port_loopback", __FILE__),
	device_pet_user_port_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void diag264_user_port_loopback_device::device_start()
{
}
