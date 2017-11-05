// license:BSD-3-Clause
// copyright-holders:smf
/**********************************************************************

    Commodore PET User Port Diagnostic Connector emulation

**********************************************************************/

#include "emu.h"
#include "diag.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(PET_USERPORT_DIAGNOSTIC_CONNECTOR, pet_userport_diagnostic_connector_device, "pet_user_diag", "PET Userport Diagnostic Connector")



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  pet_userport_diagnostic_connector_t - constructor
//-------------------------------------------------

pet_userport_diagnostic_connector_device::pet_userport_diagnostic_connector_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, PET_USERPORT_DIAGNOSTIC_CONNECTOR, tag, owner, clock),
	device_pet_user_port_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void pet_userport_diagnostic_connector_device::device_start()
{
	output_5(0);
	output_e(0);
}
