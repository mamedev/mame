// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Miracle Systems Gold Card emulation

**********************************************************************/

#include "miracle_gold_card.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type MIRACLE_GOLD_CARD = &device_creator<miracle_gold_card_t>;


//-------------------------------------------------
//  ROM( miracle_gold_card )
//-------------------------------------------------

ROM_START( miracle_gold_card )
	ROM_REGION( 0x10000, "rom", 0 )
	ROM_DEFAULT_BIOS("v249")
	ROM_SYSTEM_BIOS( 0, "v228", "v2.28" )
	ROMX_LOAD( "goldcard228.bin", 0x00000, 0x10000, CRC(fee008de) SHA1(849f0a515ac32502f3b1a4f65ce957c0bef6e6d6), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "v249", "v2.49" )
	ROMX_LOAD( "sgcandgc249.bin", 0x00000, 0x10000, CRC(963c7bfc) SHA1(e80851fc536eef2b83c611e717e563b05bba8b3d), ROM_BIOS(2) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *miracle_gold_card_t::device_rom_region() const
{
	return ROM_NAME( miracle_gold_card );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  miracle_gold_card_t - constructor
//-------------------------------------------------

miracle_gold_card_t::miracle_gold_card_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, MIRACLE_GOLD_CARD, "Miracle Gold Card", tag, owner, clock, "ql_gold", __FILE__),
	device_ql_expansion_card_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void miracle_gold_card_t::device_start()
{
}


//-------------------------------------------------
//  read -
//-------------------------------------------------

UINT8 miracle_gold_card_t::read(address_space &space, offs_t offset, UINT8 data)
{
	return data;
}


//-------------------------------------------------
//  write -
//-------------------------------------------------

void miracle_gold_card_t::write(address_space &space, offs_t offset, UINT8 data)
{
}
