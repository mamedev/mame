// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Fun Play / Power Play cartridge emulation

**********************************************************************/

#include "emu.h"
#include "fun_play.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(C64_FUN_PLAY, c64_fun_play_cartridge_device, "c64_fun_play", "C64 Fun Play cartridge")



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c64_fun_play_cartridge_device - constructor
//-------------------------------------------------

c64_fun_play_cartridge_device::c64_fun_play_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, C64_FUN_PLAY, tag, owner, clock),
	device_c64_expansion_card_interface(mconfig, *this),
	m_bank(0)
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

uint8_t c64_fun_play_cartridge_device::c64_cd_r(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2)
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

void c64_fun_play_cartridge_device::c64_cd_w(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2)
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
