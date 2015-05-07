// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Scanntronik Pagefox cartridge emulation

**********************************************************************/

/*

    PCB Layout
    ----------

    |===========================|
    |=|                         |
    |=|     RAM         LS11    |
    |=|                         |
    |=|                 LS139   |
    |=|     ROM0                |
    |=|                 LS273   |
    |=|                         |
    |=|     ROM1        LS00    |
    |===========================|

    RAM   - NEC D43256-12L 32Kx8 RAM
    ROM0  - SGS M27256-2FI 32Kx8 EPROM
    ROM1  - National Instruments NMC27C256Q 32Kx8 EPROM

*/

#include "pagefox.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type C64_PAGEFOX = &device_creator<c64_pagefox_cartridge_device>;



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c64_pagefox_cartridge_device - constructor
//-------------------------------------------------

c64_pagefox_cartridge_device::c64_pagefox_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, C64_PAGEFOX, "C64 Pagefox cartridge", tag, owner, clock, "c64_pagefox", __FILE__),
	device_c64_expansion_card_interface(mconfig, *this),
	m_ram(*this, "ram")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c64_pagefox_cartridge_device::device_start()
{
	// allocate memory
	m_ram.allocate(0x8000);

	// state saving
	save_item(NAME(m_bank));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void c64_pagefox_cartridge_device::device_reset()
{
	m_bank = 0;
}


//-------------------------------------------------
//  c64_cd_r - cartridge data read
//-------------------------------------------------

UINT8 c64_pagefox_cartridge_device::c64_cd_r(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (!roml || !romh)
	{
		if (BIT(m_bank, 3))
		{
			offs_t addr = (BIT(m_bank, 1) << 14) | (offset & 0x3fff);
			data = m_ram[addr];
		}
		else
		{
			int bank = (m_bank >> 1) & 0x07;
			offs_t addr = (bank << 14) | (offset & 0x3fff);
			data = m_roml[addr];
		}
	}

	return data;
}


//-------------------------------------------------
//  c64_cd_w - cartridge data write
//-------------------------------------------------

void c64_pagefox_cartridge_device::c64_cd_w(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (offset >= 0x8000 && offset < 0xc000)
	{
		if (BIT(m_bank, 3))
		{
			offs_t addr = (BIT(m_bank, 1) << 14) | (offset & 0x3fff);
			m_ram[addr] = data;
		}
	}
	else if (!io1 && BIT(offset, 7))
	{
		if (data == 0xff)
		{
			m_game = 1;
			m_exrom = 1;
		}
		else
		{
			m_game = 0;
			m_exrom = 0;
		}

		m_bank = data;
	}
}
