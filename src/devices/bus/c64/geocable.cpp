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
//  MACHINE_CONFIG_START( c64_geocable )
//-------------------------------------------------

static MACHINE_CONFIG_START( c64_geocable )
	MCFG_CENTRONICS_ADD("centronics", centronics_devices, "printer")
	MCFG_CENTRONICS_BUSY_HANDLER(WRITELINE(c64_geocable_device, output_b))
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor c64_geocable_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( c64_geocable );
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
