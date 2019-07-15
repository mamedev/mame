// license:BSD-3-Clause
// copyright-holders:Curt Coder, smf
/**********************************************************************

    geoCable Centronics Cable emulation

**********************************************************************/

#include "emu.h"
#include "geocable.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define CENTRONICS_TAG "centronics"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(C64_GEOCABLE, c64_geocable_device, "c64_geocable", "C64 geoCable")


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void c64_geocable_device::device_add_mconfig(machine_config &config)
{
	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->busy_handler().set(FUNC(c64_geocable_device::output_b));
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c64_geocable_device - constructor
//-------------------------------------------------

c64_geocable_device::c64_geocable_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, C64_GEOCABLE, tag, owner, clock),
	device_pet_user_port_interface(mconfig, *this),
	m_centronics(*this, CENTRONICS_TAG)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c64_geocable_device::device_start()
{
}
