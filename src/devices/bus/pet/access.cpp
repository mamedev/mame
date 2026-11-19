// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Access Software cassette port dongle emulation

**********************************************************************/

#include "emu.h"
#include "access.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(ACCESS_DONGLE, access_dongle_device, "access", "Access Software cassette port dongle")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  access_dongle_device - constructor
//-------------------------------------------------

access_dongle_device::access_dongle_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ACCESS_DONGLE, tag, owner, clock),
	device_pet_datassette_port_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void access_dongle_device::device_start()
{
}
