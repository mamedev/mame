// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    OPD Basic Master emulation

**********************************************************************/

#include "emu.h"
#include "opd_basic_master.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(OPD_BASIC_MASTER, opd_basic_master_device, "ql_opdbm", "OPD Basic Master")


//-------------------------------------------------
//  ROM( opd_basic_master )
//-------------------------------------------------

ROM_START( opd_basic_master )
	ROM_REGION( 0x10000, "rom", 0 )
	ROM_LOAD( "opd_basic_master_1984.rom", 0x00000, 0x10000, CRC(7e534c0d) SHA1(de485e89272e3b51086967333cda9de806ba3876) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *opd_basic_master_device::device_rom_region() const
{
	return ROM_NAME( opd_basic_master );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  opd_basic_master_device - constructor
//-------------------------------------------------

opd_basic_master_device::opd_basic_master_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, OPD_BASIC_MASTER, tag, owner, clock),
	device_ql_expansion_card_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void opd_basic_master_device::device_start()
{
}


//-------------------------------------------------
//  read -
//-------------------------------------------------

uint8_t opd_basic_master_device::read(offs_t offset, uint8_t data)
{
	return data;
}


//-------------------------------------------------
//  write -
//-------------------------------------------------

void opd_basic_master_device::write(offs_t offset, uint8_t data)
{
}
