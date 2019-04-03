// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore Super Games cartridge emulation

**********************************************************************/

#include "emu.h"
#include "super_games.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(C64_SUPER_GAMES, c64_super_games_cartridge_device, "c64_super_games", "C64 Super Games cartridge")



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c64_super_games_cartridge_device - constructor
//-------------------------------------------------

c64_super_games_cartridge_device::c64_super_games_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, C64_SUPER_GAMES, tag, owner, clock),
	device_c64_expansion_card_interface(mconfig, *this),
	m_bank(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c64_super_games_cartridge_device::device_start()
{
	// state saving
	save_item(NAME(m_bank));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void c64_super_games_cartridge_device::device_reset()
{
	m_bank = 0;
}


//-------------------------------------------------
//  c64_cd_r - cartridge data read
//-------------------------------------------------

uint8_t c64_super_games_cartridge_device::c64_cd_r(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (!roml || !romh)
	{
		offs_t addr = (m_bank << 14) | (offset & 0x3fff);
		data = m_roml[addr];
	}

	return data;
}


//-------------------------------------------------
//  c64_cd_w - cartridge data write
//-------------------------------------------------

void c64_super_games_cartridge_device::c64_cd_w(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (!io2)
	{
		m_bank = data & 0x03;

		if (BIT(data, 2))
		{
			if (BIT(data, 3))
			{
				m_game = 1;
				m_exrom = 1;
			}
			else
			{
				m_game = 1;
				m_exrom = 0;
			}
		}
		else
		{
			m_game = 0;
			m_exrom = 0;
		}
	}
}
