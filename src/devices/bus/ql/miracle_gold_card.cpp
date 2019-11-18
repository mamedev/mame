// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Miracle Systems Gold Card emulation

**********************************************************************/

#include "emu.h"
#include "miracle_gold_card.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(MIRACLE_GOLD_CARD, miracle_gold_card_device, "ql_gold", "Miracle Gold Card")


//-------------------------------------------------
//  ROM( miracle_gold_card )
//-------------------------------------------------

ROM_START( miracle_gold_card )
	ROM_REGION( 0x10000, "rom", 0 )
	ROM_DEFAULT_BIOS("v249")
	ROM_SYSTEM_BIOS( 0, "v228", "v2.28" )
	ROMX_LOAD( "goldcard228.bin", 0x00000, 0x10000, CRC(fee008de) SHA1(849f0a515ac32502f3b1a4f65ce957c0bef6e6d6), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "v249", "v2.49" )
	ROMX_LOAD( "sgcandgc249.bin", 0x00000, 0x10000, CRC(963c7bfc) SHA1(e80851fc536eef2b83c611e717e563b05bba8b3d), ROM_BIOS(1) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *miracle_gold_card_device::device_rom_region() const
{
	return ROM_NAME( miracle_gold_card );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  miracle_gold_card_device - constructor
//-------------------------------------------------

miracle_gold_card_device::miracle_gold_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, MIRACLE_GOLD_CARD, tag, owner, clock),
	device_ql_expansion_card_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void miracle_gold_card_device::device_start()
{
}


//-------------------------------------------------
//  read -
//-------------------------------------------------

uint8_t miracle_gold_card_device::read(offs_t offset, uint8_t data)
{
	return data;
}


//-------------------------------------------------
//  write -
//-------------------------------------------------

void miracle_gold_card_device::write(offs_t offset, uint8_t data)
{
}
