// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Kempston Disk Interface emulation

**********************************************************************/

#include "kempston_di.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type KEMPSTON_DISK_INTERFACE = &device_creator<kempston_disk_interface_t>;


//-------------------------------------------------
//  ROM( kempston_disk_system )
//-------------------------------------------------

ROM_START( kempston_disk_system )
	ROM_REGION( 0x2000, "rom", 0 )
	ROM_DEFAULT_BIOS("v114")
	ROM_SYSTEM_BIOS( 0, "v114", "v1.14" )
	ROMX_LOAD( "kempston_disk_system_v1.14_1984.rom", 0x0000, 0x2000, CRC(0b70ad2e) SHA1(ff8158d25864d920f3f6df259167e91c2784692c), ROM_BIOS(1) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *kempston_disk_interface_t::device_rom_region() const
{
	return ROM_NAME( kempston_disk_system );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  kempston_disk_interface_t - constructor
//-------------------------------------------------

kempston_disk_interface_t::kempston_disk_interface_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, KEMPSTON_DISK_INTERFACE, "Kempston Disk Interface", tag, owner, clock, "ql_kdi", __FILE__),
	device_ql_expansion_card_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void kempston_disk_interface_t::device_start()
{
}


//-------------------------------------------------
//  read -
//-------------------------------------------------

UINT8 kempston_disk_interface_t::read(address_space &space, offs_t offset, UINT8 data)
{
	return data;
}


//-------------------------------------------------
//  write -
//-------------------------------------------------

void kempston_disk_interface_t::write(address_space &space, offs_t offset, UINT8 data)
{
}
