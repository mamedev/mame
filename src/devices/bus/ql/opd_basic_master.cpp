// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    OPD Basic Master emulation

**********************************************************************/

#include "opd_basic_master.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type OPD_BASIC_MASTER = &device_creator<opd_basic_master_t>;


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

const rom_entry *opd_basic_master_t::device_rom_region() const
{
	return ROM_NAME( opd_basic_master );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  opd_basic_master_t - constructor
//-------------------------------------------------

opd_basic_master_t::opd_basic_master_t(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, OPD_BASIC_MASTER, "OPD Basic Master", tag, owner, clock, "ql_opdbm", __FILE__),
	device_ql_expansion_card_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void opd_basic_master_t::device_start()
{
}


//-------------------------------------------------
//  read -
//-------------------------------------------------

UINT8 opd_basic_master_t::read(address_space &space, offs_t offset, UINT8 data)
{
	return data;
}


//-------------------------------------------------
//  write -
//-------------------------------------------------

void opd_basic_master_t::write(address_space &space, offs_t offset, UINT8 data)
{
}
