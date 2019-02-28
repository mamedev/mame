// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Cumana Floppy Disk Interface emulation

**********************************************************************/

#include "emu.h"
#include "cumana_fdi.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(CUMANA_FLOPPY_DISK_INTERFACE, cumana_floppy_disk_interface_device, "ql_cumanafdi", "Cumana Floppy Disk Interface")


//-------------------------------------------------
//  ROM( cumana_floppy_disk_interface )
//-------------------------------------------------

ROM_START( cumana_floppy_disk_interface )
	ROM_REGION( 0x4000, "rom", 0 )
	ROM_DEFAULT_BIOS("v116")
	ROM_SYSTEM_BIOS( 0, "v114", "v1.14" )
	ROMX_LOAD( "cumana114.rom", 0x0000, 0x4000, CRC(03baf164) SHA1(e487742c481be8c2771ab2c0fc5e3acd612dec54), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "v116", "v1.16" )
	ROMX_LOAD( "cumana116.rom", 0x0000, 0x4000, CRC(def02822) SHA1(323120cd3e1eaa38f6d0ae74367a4835a5a2a011), ROM_BIOS(1) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *cumana_floppy_disk_interface_device::device_rom_region() const
{
	return ROM_NAME( cumana_floppy_disk_interface );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  cumana_floppy_disk_interface_device - constructor
//-------------------------------------------------

cumana_floppy_disk_interface_device::cumana_floppy_disk_interface_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, CUMANA_FLOPPY_DISK_INTERFACE, tag, owner, clock),
	device_ql_expansion_card_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cumana_floppy_disk_interface_device::device_start()
{
}


//-------------------------------------------------
//  read -
//-------------------------------------------------

uint8_t cumana_floppy_disk_interface_device::read(offs_t offset, uint8_t data)
{
	return data;
}


//-------------------------------------------------
//  write -
//-------------------------------------------------

void cumana_floppy_disk_interface_device::write(offs_t offset, uint8_t data)
{
}
