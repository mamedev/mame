// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Fun Play / Power Play cartridge emulation

**********************************************************************/

#include "fun_play.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type C64_FUN_PLAY = &device_creator<c64_fun_play_cartridge_device>;



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c64_fun_play_cartridge_device - constructor
//-------------------------------------------------

c64_fun_play_cartridge_device::c64_fun_play_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, C64_FUN_PLAY, "C64 Fun Play cartridge", tag, owner, clock, "c64_fun_play", __FILE__),
	device_c64_expansion_card_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c64_fun_play_cartridge_device::device_start()
{
	// state saving
	save_item(NAME(m_bank));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void c64_fun_play_cartridge_device::device_reset()
{
	m_bank = 0;
}


//-------------------------------------------------
//  c64_cd_r - cartridge data read
//-------------------------------------------------

UINT8 c64_fun_play_cartridge_device::c64_cd_r(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (!roml)
	{
		offs_t addr = (m_bank << 13) | (offset & 0x1fff);
		data = m_roml[addr];
	}

	return data;
}


//-------------------------------------------------
//  c64_cd_w - cartridge data write
//-------------------------------------------------

void c64_fun_play_cartridge_device::c64_cd_w(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (!io1)
	{
		/*

		    bit     description

		    0       A16
		    1
		    2
		    3       A13
		    4       A14
		    5       A15
		    6
		    7

		*/

		m_bank = ((data & 0x01) << 3) | ((data >> 3) & 0x07);
	}
}
