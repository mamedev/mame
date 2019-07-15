// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Kempston Disk Interface emulation

**********************************************************************/

#include "emu.h"
#include "kempston_di.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(KEMPSTON_DISK_INTERFACE, kempston_disk_interface_device, "ql_kdi", "Kempston Disk Interface")


//-------------------------------------------------
//  ROM( kempston_disk_system )
//-------------------------------------------------

ROM_START( kempston_disk_system )
	ROM_REGION( 0x2000, "rom", 0 )
	ROM_DEFAULT_BIOS("v114")
	ROM_SYSTEM_BIOS( 0, "v114", "v1.14" )
	ROMX_LOAD( "kempston_disk_system_v1.14_1984.rom", 0x0000, 0x2000, CRC(0b70ad2e) SHA1(ff8158d25864d920f3f6df259167e91c2784692c), ROM_BIOS(0) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *kempston_disk_interface_device::device_rom_region() const
{
	return ROM_NAME( kempston_disk_system );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  kempston_disk_interface_device - constructor
//-------------------------------------------------

kempston_disk_interface_device::kempston_disk_interface_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, KEMPSTON_DISK_INTERFACE, tag, owner, clock),
	device_ql_expansion_card_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void kempston_disk_interface_device::device_start()
{
}


//-------------------------------------------------
//  read -
//-------------------------------------------------

uint8_t kempston_disk_interface_device::read(offs_t offset, uint8_t data)
{
	return data;
}


//-------------------------------------------------
//  write -
//-------------------------------------------------

void kempston_disk_interface_device::write(offs_t offset, uint8_t data)
{
}
