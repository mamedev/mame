// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    COMX-35 F&M EPROM Card emulation

**********************************************************************/

#include "eprom.h"



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type COMX_EPR = &device_creator<comx_epr_device>;


//-------------------------------------------------
//  ROM( comx_epr )
//-------------------------------------------------

ROM_START( comx_epr )
	ROM_REGION( 0x800, "f800", 0 )
	ROM_LOAD( "f&m.eprom.board.1.1.bin", 0x0000, 0x0800, CRC(a042a31a) SHA1(13831a1350aa62a87988bfcc99c4b7db8ef1cf62) )

	ROM_REGION( 0x10000, "eprom", 0 )
	ROM_LOAD( "f&m.basic.bin",      0x0000, 0x2000, CRC(22ab7b61) SHA1(68b5770bca37b1ba94083f944086884e612b5a1b) )
	ROM_LOAD( "disk.utilities.bin", 0x2000, 0x2000, CRC(2576c945) SHA1(e80481054c6997a5f418d8a5872ac0110ae7b75a) )
	ROM_LOAD( "tennismania.bin",    0x4000, 0x2000, CRC(a956cc74) SHA1(8bc914f52f0dd2cf792da74ec4e9e333365619ef) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *comx_epr_device::device_rom_region() const
{
	return ROM_NAME( comx_epr );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  comx_epr_device - constructor
//-------------------------------------------------

comx_epr_device::comx_epr_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, COMX_EPR, "COMX-35 F&M EPROM Switchboard", tag, owner, clock, "comx_epr", __FILE__),
	device_comx_expansion_card_interface(mconfig, *this),
	m_rom(*this, "f800"),
	m_eprom(*this, "eprom"),
	m_select(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void comx_epr_device::device_start()
{
	// state saving
	save_item(NAME(m_select));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void comx_epr_device::device_reset()
{
}


//-------------------------------------------------
//  comx_mrd_r - memory read
//-------------------------------------------------

UINT8 comx_epr_device::comx_mrd_r(address_space &space, offs_t offset, int *extrom)
{
	UINT8 data = 0;

	if (offset >= 0xc000 && offset < 0xe000)
	{
		offs_t address = (m_select << 13) | (offset & 0x1fff);
		data = m_eprom->base()[address];
	}
	else if (offset >= 0xf800)
	{
		data = m_rom->base()[offset & 0x7ff];
	}

	return data;
}


//-------------------------------------------------
//  comx_io_w - I/O write
//-------------------------------------------------

void comx_epr_device::comx_io_w(address_space &space, offs_t offset, UINT8 data)
{
	if (offset == 1)
	{
		m_select = data >> 5;
	}
}
