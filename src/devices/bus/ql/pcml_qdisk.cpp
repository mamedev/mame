// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    PCML Q+ Disk Interface emulation

**********************************************************************/

#include "pcml_qdisk.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type PCML_Q_DISK_INTERFACE = &device_creator<pcml_q_disk_interface_t>;


//-------------------------------------------------
//  ROM( pcml_q_disk_interface )
//-------------------------------------------------

ROM_START( pcml_q_disk_interface )
	ROM_REGION( 0x4000, "rom", 0 )
	ROM_DEFAULT_BIOS("v114")
	ROM_SYSTEM_BIOS( 0, "v114", "v1.14" )
	ROMX_LOAD( "pcml_diskram system_v1.14_1984.rom", 0x0000, 0x4000, CRC(e38b41dd) SHA1(d2038f0b1a62e8e65ec86660d03c25489ce40274), ROM_BIOS(1) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *pcml_q_disk_interface_t::device_rom_region() const
{
	return ROM_NAME( pcml_q_disk_interface );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  pcml_q_disk_interface_t - constructor
//-------------------------------------------------

pcml_q_disk_interface_t::pcml_q_disk_interface_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, PCML_Q_DISK_INTERFACE, "PCML Q+ Disk Interface", tag, owner, clock, "ql_pcmlqdi", __FILE__),
	device_ql_expansion_card_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void pcml_q_disk_interface_t::device_start()
{
}


//-------------------------------------------------
//  read -
//-------------------------------------------------

UINT8 pcml_q_disk_interface_t::read(address_space &space, offs_t offset, UINT8 data)
{
	return data;
}


//-------------------------------------------------
//  write -
//-------------------------------------------------

void pcml_q_disk_interface_t::write(address_space &space, offs_t offset, UINT8 data)
{
}
