// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Miracle Hard Disk emulation

**********************************************************************/

#include "emu.h"
#include "miracle_hd.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(MIRACLE_HARD_DISK, miracle_hard_disk_device, "ql_mhd", "Miracle Hard Disk")


//-------------------------------------------------
//  ROM( miracle_hard_disk )
//-------------------------------------------------

ROM_START( miracle_hard_disk )
	ROM_REGION( 0x2000, "rom", 0 )
	ROM_DEFAULT_BIOS("v202")
	ROM_SYSTEM_BIOS( 0, "v202", "v2.02" )
	ROMX_LOAD( "miraculous_winny_v2.02_1989_qjump.rom", 0x0000, 0x2000, CRC(10982b35) SHA1(1beb87a207ecd0f47a43ed4b1bcc81d89ac75ffc), ROM_BIOS(0) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *miracle_hard_disk_device::device_rom_region() const
{
	return ROM_NAME( miracle_hard_disk );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  miracle_hard_disk_device - constructor
//-------------------------------------------------

miracle_hard_disk_device::miracle_hard_disk_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, MIRACLE_HARD_DISK, tag, owner, clock),
	device_ql_rom_cartridge_card_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void miracle_hard_disk_device::device_start()
{
}


//-------------------------------------------------
//  read -
//-------------------------------------------------

uint8_t miracle_hard_disk_device::read(offs_t offset, uint8_t data)
{
	return data;
}


//-------------------------------------------------
//  write -
//-------------------------------------------------

void miracle_hard_disk_device::write(offs_t offset, uint8_t data)
{
}
