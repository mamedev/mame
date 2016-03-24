// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    North Star MICRO-DISK System MDS-A-D (Double Density) emulation

**********************************************************************/

#include "nsmdsad.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type S100_MDS_AD = &device_creator<s100_mds_ad_device>;


//-------------------------------------------------
//  ROM( mds_ad )
//-------------------------------------------------

ROM_START( mds_ad )
	ROM_REGION( 0x100, "dsel", 0 )
	ROM_LOAD( "dsel.11c", 0x000, 0x100, NO_DUMP ) // 82S129

	ROM_REGION( 0x100, "dpgm", 0 )
	ROM_LOAD( "dpgm.9d", 0x000, 0x100, CRC(7aafa134) SHA1(bf1552c4818f30473798af4f54e65e1957e0db48) )

	ROM_REGION( 0x100, "dwe", 0 )
	ROM_LOAD( "dwe.4c", 0x000, 0x100, NO_DUMP ) // 82S129
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *s100_mds_ad_device::device_rom_region() const
{
	return ROM_NAME( mds_ad );
}


//-------------------------------------------------
//  SLOT_INTERFACE( mds_ad_floppies )
//-------------------------------------------------

static SLOT_INTERFACE_START( mds_ad_floppies )
	SLOT_INTERFACE( "525dd", FLOPPY_525_DD ) // Shugart SA-400
SLOT_INTERFACE_END


//-------------------------------------------------
//  MACHINE_CONFIG_FRAGMENT( mds_ad )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( mds_ad )
	MCFG_FLOPPY_DRIVE_ADD("floppy0", mds_ad_floppies, "525dd", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("floppy1", mds_ad_floppies, "525dd", floppy_image_device::default_floppy_formats)
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor s100_mds_ad_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( mds_ad );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  s100_mds_ad_device - constructor
//-------------------------------------------------

s100_mds_ad_device::s100_mds_ad_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, S100_MDS_AD, "MDS-A-D", tag, owner, clock, "nsmdsad", __FILE__),
	device_s100_card_interface(mconfig, *this),
	m_floppy0(*this, "floppy0"),
	m_floppy1(*this, "floppy1"),
	m_dsel_rom(*this, "dsel"),
	m_dpgm_rom(*this, "dpgm"),
	m_dwe_rom(*this, "dwe")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void s100_mds_ad_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void s100_mds_ad_device::device_reset()
{
}


//-------------------------------------------------
//  s100_smemr_r - memory read
//-------------------------------------------------

UINT8 s100_mds_ad_device::s100_smemr_r(address_space &space, offs_t offset)
{
	return 0;
}
