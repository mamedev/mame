// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    Amiga Autoconfig

***************************************************************************/

#include "autoconfig.h"


//**************************************************************************
//  CONSTANTS & MACROS
//**************************************************************************

#define VERBOSE 1


//**************************************************************************
//  CONSTRUCTOR
//**************************************************************************

amiga_autoconfig::amiga_autoconfig()
{
	memset(m_cfg, 0xff, sizeof(m_cfg));

	// interrupt status register, not inverted
	m_cfg[0x20] = 0;
	m_cfg[0x21] = 0;
}

amiga_autoconfig::~amiga_autoconfig()
{
}


//**************************************************************************
//  AUTOCONFIG INFO SETTERS
//**************************************************************************

void amiga_autoconfig::autoconfig_board_type(board_type type)
{
	m_cfg[0x00] &= 0x3000;
	m_cfg[0x00] |= (type << 2) << 12;
}

void amiga_autoconfig::autoconfig_board_size(board_size size)
{
	m_cfg[0x01] &= 0x8000;
	m_cfg[0x01] |= (size << 0) << 12;
}

void amiga_autoconfig::autoconfig_rom_vector_valid(bool state)
{
	m_cfg[0x00] &= 0xe000;
	m_cfg[0x00] |= (state ? 0x01 : 0x00) << 12;
}

void amiga_autoconfig::autoconfig_link_into_memory(bool state)
{
	m_cfg[0x00] &= 0xd000;
	m_cfg[0x00] |= (state ? 0x02 : 0x00) << 12;
}

void amiga_autoconfig::autoconfig_multi_device(bool state)
{
	m_cfg[0x01] &= 0x7000;
	m_cfg[0x01] |= (state ? 0x08 : 0x00) << 12;
}

void amiga_autoconfig::autoconfig_8meg_preferred(bool state)
{
	m_cfg[0x04] &= 0x7000;
	m_cfg[0x04] |= (state ? 0x08 : 0x00) << 12;
}

void amiga_autoconfig::autoconfig_can_shutup(bool state)
{
	m_cfg[0x04] &= 0xb000;
	m_cfg[0x04] |= (state ? 0x04 : 0x00) << 12;
}

void amiga_autoconfig::autoconfig_product(UINT8 data)
{
	m_cfg[0x02] = ~((data & 0xf0) >> 4) << 12;
	m_cfg[0x03] = ~((data & 0x0f) >> 0) << 12;
}

void amiga_autoconfig::autoconfig_manufacturer(UINT16 data)
{
	m_cfg[0x08] = ~((data & 0xf000) >> 12) << 12;
	m_cfg[0x09] = ~((data & 0x0f00) >> 8) << 12;
	m_cfg[0x0a] = ~((data & 0x00f0) >> 4) << 12;
	m_cfg[0x0b] = ~((data & 0x000f) >> 0) << 12;
}

void amiga_autoconfig::autoconfig_serial(UINT32 data)
{
	m_cfg[0x0c] = ~((data & 0xf0000000) >> 28) << 12;
	m_cfg[0x0d] = ~((data & 0x0f000000) >> 24) << 12;
	m_cfg[0x0e] = ~((data & 0x00f00000) >> 20) << 12;
	m_cfg[0x0f] = ~((data & 0x000f0000) >> 16) << 12;
	m_cfg[0x10] = ~((data & 0x0000f000) >> 12) << 12;
	m_cfg[0x11] = ~((data & 0x00000f00) >> 8) << 12;
	m_cfg[0x12] = ~((data & 0x000000f0) >> 4) << 12;
	m_cfg[0x13] = ~((data & 0x0000000f) >> 0) << 12;
}

void amiga_autoconfig::autoconfig_rom_vector(UINT16 data)
{
	m_cfg[0x14] = ~((data & 0xf000) >> 12) << 12;
	m_cfg[0x15] = ~((data & 0x0f00) >> 8) << 12;
	m_cfg[0x16] = ~((data & 0x00f0) >> 4) << 12;
	m_cfg[0x17] = ~((data & 0x000f) >> 0) << 12;
}


//**************************************************************************
//  MEMORY INTERFACE
//**************************************************************************

READ16_MEMBER( amiga_autoconfig::autoconfig_read )
{
	UINT16 data = m_cfg[offset] | 0x0fff;

	if (VERBOSE && !space.debugger_access())
		space.device().logerror("autoconfig_read %04x @ %02x [mask = %04x]\n", data, offset, mem_mask);

	return data;
}

WRITE16_MEMBER( amiga_autoconfig::autoconfig_write )
{
	if (VERBOSE && !space.debugger_access())
		space.device().logerror("autoconfig_write %04x @ %02x [mask = %04x]\n", data, offset, mem_mask);

	switch (offset)
	{
	case 0x20:
		// user-definable
		break;

	case 0x21:
		// bit 0 = interrupt enable
		// bit 1 = user-definable
		// bit 2 = local reset
		// bit 3 = user-definable
		break;

	case 0x24:
		// base address register
		m_cfg[0x24] = data & 0xf000;
		autoconfig_base_address((m_cfg[0x24] << 8) | (m_cfg[0x25] << 4));
		break;

	case 0x25:
		// latch low-nibble
		m_cfg[0x25] = data & 0xf000;
		break;

	case 0x26:
		// shut-up register
		autoconfig_base_address(0);
		break;

	case 0x27:
		break;
	}
}
