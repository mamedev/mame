// license:BSD-3-Clause
// copyright-holders:smf
/**********************************************************************

    Diag264 User Port Loop Back Connector emulation

**********************************************************************/

#include "emu.h"
#include "diag264_lb_user.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(DIAG264_USER_PORT_LOOPBACK, diag264_user_port_loopback_device, "diag264_user_port_loopback", "Diag264 User Port Loopback")



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  diag264_user_port_loopback_device - constructor
//-------------------------------------------------

diag264_user_port_loopback_device::diag264_user_port_loopback_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, DIAG264_USER_PORT_LOOPBACK, tag, owner, clock)
	, device_pet_user_port_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void diag264_user_port_loopback_device::device_start()
{
}
