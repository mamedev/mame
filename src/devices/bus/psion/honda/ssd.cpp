// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Psion Siena SSD Drive

**********************************************************************/

#include "emu.h"
#include "ssd.h"
#include "bus/rs232/rs232.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(PSION_SIENA_SSD, psion_siena_ssd_device, "psion_siena_ssd", "Psion Siena SSD Drive")


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void psion_siena_ssd_device::device_add_mconfig(machine_config &config)
{
	PSION_SSD(config, m_ssd);
	m_ssd->door_cb().set(DEVICE_SELF_OWNER, FUNC(psion_honda_slot_device::write_sdoe));

	RS232_PORT(config, "rs232", default_rs232_devices, nullptr);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  psion_siena_ssd_device - constructor
//-------------------------------------------------

psion_siena_ssd_device::psion_siena_ssd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, PSION_SIENA_SSD, tag, owner, clock)
	, device_psion_honda_interface(mconfig, *this)
	, m_ssd(*this, "ssd")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void psion_siena_ssd_device::device_start()
{
}
