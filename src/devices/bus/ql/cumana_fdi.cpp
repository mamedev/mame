// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Cumana Floppy Disk Interface emulation

**********************************************************************/

#include "cumana_fdi.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type CUMANA_FLOPPY_DISK_INTERFACE = &device_creator<cumana_floppy_disk_interface_t>;


//-------------------------------------------------
//  ROM( cumana_floppy_disk_interface )
//-------------------------------------------------

ROM_START( cumana_floppy_disk_interface )
	ROM_REGION( 0x4000, "rom", 0 )
	ROM_DEFAULT_BIOS("v116")
	ROM_SYSTEM_BIOS( 0, "v114", "v1.14" )
	ROMX_LOAD( "cumana114.rom", 0x0000, 0x4000, CRC(03baf164) SHA1(e487742c481be8c2771ab2c0fc5e3acd612dec54), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "v116", "v1.16" )
	ROMX_LOAD( "cumana116.rom", 0x0000, 0x4000, CRC(def02822) SHA1(323120cd3e1eaa38f6d0ae74367a4835a5a2a011), ROM_BIOS(2) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *cumana_floppy_disk_interface_t::device_rom_region() const
{
	return ROM_NAME( cumana_floppy_disk_interface );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  cumana_floppy_disk_interface_t - constructor
//-------------------------------------------------

cumana_floppy_disk_interface_t::cumana_floppy_disk_interface_t(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, CUMANA_FLOPPY_DISK_INTERFACE, "Cumana Floppy Disk Interface", tag, owner, clock, "ql_cumanafdi", __FILE__),
	device_ql_expansion_card_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cumana_floppy_disk_interface_t::device_start()
{
}


//-------------------------------------------------
//  read -
//-------------------------------------------------

UINT8 cumana_floppy_disk_interface_t::read(address_space &space, offs_t offset, UINT8 data)
{
	return data;
}


//-------------------------------------------------
//  write -
//-------------------------------------------------

void cumana_floppy_disk_interface_t::write(address_space &space, offs_t offset, UINT8 data)
{
}
