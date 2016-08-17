// license:BSD-3-Clause
// copyright-holders:smf
/**********************************************************************

    Commodore PET User Port Diagnostic Connector emulation

**********************************************************************/

#include "diag.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type PET_USERPORT_DIAGNOSTIC_CONNECTOR = &device_creator<pet_userport_diagnostic_connector_t>;



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  pet_userport_diagnostic_connector_t - constructor
//-------------------------------------------------

pet_userport_diagnostic_connector_t::pet_userport_diagnostic_connector_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, PET_USERPORT_DIAGNOSTIC_CONNECTOR, "PET Userport Diagnostic Connector", tag, owner, clock, "pet_user_diag", __FILE__),
	device_pet_user_port_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void pet_userport_diagnostic_connector_t::device_start()
{
	output_5(0);
	output_e(0);
}
