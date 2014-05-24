// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Sandy Super Disk emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#include "sandy_superdisk.h"



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type SANDY_SUPER_DISK = &device_creator<sandy_super_disk_t>;


//-------------------------------------------------
//  ROM( sandy_super_disk )
//-------------------------------------------------

ROM_START( sandy_super_disk )
	ROM_REGION( 0x4000, "rom", 0 )
	ROM_LOAD( "sandysuperdisk.rom", 0x0000, 0x4000, CRC(b52077da) SHA1(bf531758145ffd083e01c1cf9c45d0e9264a3b53) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *sandy_super_disk_t::device_rom_region() const
{
	return ROM_NAME( sandy_super_disk );
}


//-------------------------------------------------
//  MACHINE_CONFIG_FRAGMENT( sandy_super_disk )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( sandy_super_disk )
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor sandy_super_disk_t::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( sandy_super_disk );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  sandy_super_disk_t - constructor
//-------------------------------------------------

sandy_super_disk_t::sandy_super_disk_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, SANDY_SUPER_DISK, "Sandy Super Disk", tag, owner, clock, "sandy_super_disk", __FILE__),
	device_ql_expansion_card_interface(mconfig, *this),
	m_rom(*this, "rom")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sandy_super_disk_t::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void sandy_super_disk_t::device_reset()
{
}
