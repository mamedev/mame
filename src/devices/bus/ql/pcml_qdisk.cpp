// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    PCML Q+ Disk Interface emulation

**********************************************************************/

#include "emu.h"
#include "pcml_qdisk.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(PCML_Q_DISK_INTERFACE, pcml_q_disk_interface_device, "ql_pcmlqdi", "PCML Q+ Disk Interface")


//-------------------------------------------------
//  ROM( pcml_q_disk_interface )
//-------------------------------------------------

ROM_START( pcml_q_disk_interface )
	ROM_REGION( 0x4000, "rom", 0 )
	ROM_DEFAULT_BIOS("v114")
	ROM_SYSTEM_BIOS( 0, "v114", "v1.14" )
	ROMX_LOAD( "pcml_diskram system_v1.14_1984.rom", 0x0000, 0x4000, CRC(e38b41dd) SHA1(d2038f0b1a62e8e65ec86660d03c25489ce40274), ROM_BIOS(0) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *pcml_q_disk_interface_device::device_rom_region() const
{
	return ROM_NAME( pcml_q_disk_interface );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  pcml_q_disk_interface_device - constructor
//-------------------------------------------------

pcml_q_disk_interface_device::pcml_q_disk_interface_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, PCML_Q_DISK_INTERFACE, tag, owner, clock),
	device_ql_expansion_card_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void pcml_q_disk_interface_device::device_start()
{
}


//-------------------------------------------------
//  read -
//-------------------------------------------------

uint8_t pcml_q_disk_interface_device::read(offs_t offset, uint8_t data)
{
	return data;
}


//-------------------------------------------------
//  write -
//-------------------------------------------------

void pcml_q_disk_interface_device::write(offs_t offset, uint8_t data)
{
}
