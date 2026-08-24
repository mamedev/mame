// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    Amiga Autoconfig

***************************************************************************/

#include "emu.h"
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
	memset(m_cfg, 0x0f, sizeof(m_cfg));

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
	m_cfg[0x00] &= 0x03;
	m_cfg[0x00] |= type << 2;

	// register 08 bit 4 is reserved and must read as one for zorro3 boards
	if (is_zorro3())
		m_cfg[0x04] &= 0x0e;
}

void amiga_autoconfig::autoconfig_board_size(board_size size)
{
	m_cfg[0x01] &= 0x08;
	m_cfg[0x01] |= size & 0x07;

	// set the extension bit for zorro3 board sizes
	if (size >= BOARD_SIZE_16M)
		m_cfg[0x04] &= 0x0d;
	else
		m_cfg[0x04] |= 0x02;
}

void amiga_autoconfig::autoconfig_board_subsize(board_subsize size)
{
	m_cfg[0x05] = ~size & 0x0f;
}

void amiga_autoconfig::autoconfig_rom_vector_valid(bool state)
{
	m_cfg[0x00] &= 0x0e;
	m_cfg[0x00] |= state ? 0x01 : 0x00;
}

void amiga_autoconfig::autoconfig_link_into_memory(bool state)
{
	m_cfg[0x00] &= 0x0d;
	m_cfg[0x00] |= state ? 0x02 : 0x00;
}

void amiga_autoconfig::autoconfig_multi_device(bool state)
{
	m_cfg[0x01] &= 0x07;
	m_cfg[0x01] |= state ? 0x08 : 0x00;
}

void amiga_autoconfig::autoconfig_8meg_preferred(bool state)
{
	m_cfg[0x04] &= 0x07;
	m_cfg[0x04] |= state ? 0x00 : 0x08;
}

void amiga_autoconfig::autoconfig_can_shutup(bool state)
{
	m_cfg[0x04] &= 0x0b;
	m_cfg[0x04] |= state ? 0x04 : 0x00;
}

void amiga_autoconfig::autoconfig_product(uint8_t data)
{
	m_cfg[0x02] = ~(data >> 4) & 0x0f;
	m_cfg[0x03] = ~(data >> 0) & 0x0f;
}

void amiga_autoconfig::autoconfig_manufacturer(uint16_t data)
{
	m_cfg[0x08] = ~(data >> 12) & 0x0f;
	m_cfg[0x09] = ~(data >> 8) & 0x0f;
	m_cfg[0x0a] = ~(data >> 4) & 0x0f;
	m_cfg[0x0b] = ~(data >> 0) & 0x0f;
}

void amiga_autoconfig::autoconfig_serial(uint32_t data)
{
	m_cfg[0x0c] = ~(data >> 28) & 0x0f;
	m_cfg[0x0d] = ~(data >> 24) & 0x0f;
	m_cfg[0x0e] = ~(data >> 20) & 0x0f;
	m_cfg[0x0f] = ~(data >> 16) & 0x0f;
	m_cfg[0x10] = ~(data >> 12) & 0x0f;
	m_cfg[0x11] = ~(data >> 8) & 0x0f;
	m_cfg[0x12] = ~(data >> 4) & 0x0f;
	m_cfg[0x13] = ~(data >> 0) & 0x0f;
}

void amiga_autoconfig::autoconfig_rom_vector(uint16_t data)
{
	m_cfg[0x14] = ~(data >> 12) & 0x0f;
	m_cfg[0x15] = ~(data >> 8) & 0x0f;
	m_cfg[0x16] = ~(data >> 4) & 0x0f;
	m_cfg[0x17] = ~(data >> 0) & 0x0f;
}


//**************************************************************************
//  ZORRO II CONFIG INTERFACE
//**************************************************************************

uint16_t amiga_autoconfig::autoconfig_read(address_space &space, offs_t offset, uint16_t mem_mask)
{
	uint16_t data = (uint16_t(m_cfg[offset]) << 12) | 0x0fff;

	if (VERBOSE && !dynamic_cast<device_t *>(this)->machine().side_effects_disabled())
		space.device().logerror("autoconfig_read %04x @ %02x [mask = %04x]\n", data, offset, mem_mask);

	return data;
}

void amiga_autoconfig::autoconfig_write(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (VERBOSE && !dynamic_cast<device_t *>(this)->machine().side_effects_disabled())
		space.device().logerror("autoconfig_write %04x @ %02x [mask = %04x]\n", data, offset, mem_mask);

	if (!ACCESSING_BITS_8_15)
		return;

	uint32_t const reg = offset << 1;

	switch (reg)
	{
	case 0x40:
		// user-definable
		break;

	case 0x42:
		// bit 0 = interrupt enable
		// bit 1 = user-definable
		// bit 2 = local reset
		// bit 3 = user-definable
		break;

	case 0x44:
		// zorro3 high order base address, high nibble
		if (is_zorro3())
			m_cfg[0x22] = data >> 12;
		break;

	case 0x46:
		// zorro3 high order base address, low nibble
		if (is_zorro3())
			m_cfg[0x23] = data >> 12;
		break;

	case 0x48:
		// base address register
		m_cfg[0x24] = data >> 12;

		// a write here completes the address
		{
			offs_t addr = 0;

			if (is_zorro3())
			{
				addr |= m_cfg[0x22] << 28;
				addr |= m_cfg[0x23] << 24;
			}

			addr |= m_cfg[0x24] << 20;
			addr |= m_cfg[0x25] << 16;

			autoconfig_base_address(addr);
		}
		break;

	case 0x4a:
		// latch low-nibble
		m_cfg[0x25] = data >> 12;
		break;

	case 0x4c:
		// shut-up register
		autoconfig_base_address(0);
		break;

	case 0x27:
		break;
	}
}


//**************************************************************************
//  ZORRO III CONFIG INTERFACE
//**************************************************************************

uint32_t amiga_autoconfig::autoconfig_read32(address_space &space, offs_t offset, uint32_t mem_mask)
{
	uint32_t data = 0xffffffff;
	uint32_t const reg = (offset << 2);
	int index = -1;

	if (reg < 0x80)
	{
		// first nibble of each byte is at 0x000 to 0x07c
		index = reg / 2;
	}
	else if ((reg >= 0x100) && (reg < 0x180))
	{
		// second nibble of each byte is at 0x100 to 0x17c
		index = (reg - 0x100) / 2 + 1;
	}

	if (index != -1)
		data = (uint32_t(m_cfg[index]) << 28) | 0x0fffffff;

	if (VERBOSE && !dynamic_cast<device_t *>(this)->machine().side_effects_disabled())
		space.device().logerror("autoconfig_read32 %08x @ %04x [mask = %08x]\n", data, reg, mem_mask);

	return data;
}

void amiga_autoconfig::autoconfig_write32(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask)
{
	uint32_t const reg = offset << 2;

	if (VERBOSE && !dynamic_cast<device_t *>(this)->machine().side_effects_disabled())
		space.device().logerror("autoconfig_write32 %08x @ %04x [mask = %08x]\n", data, reg, mem_mask);

	switch (reg)
	{
	case 0x44:
		if (ACCESSING_BITS_24_31)
		{
			m_cfg[0x22] = (data >> 28) & 0x0f;
			m_cfg[0x23] = (data >> 24) & 0x0f;
		}

		if (ACCESSING_BITS_16_23)
		{
			m_cfg[0x24] = (data >> 20) & 0x0f;
			m_cfg[0x25] = (data >> 16) & 0x0f;
		}

		// a write here completes the address
		if (ACCESSING_BITS_24_31)
		{
			offs_t addr = 0;

			addr |= m_cfg[0x22] << 28;
			addr |= m_cfg[0x23] << 24;
			addr |= m_cfg[0x24] << 20;
			addr |= m_cfg[0x25] << 16;

			autoconfig_base_address(addr);
		}

		break;

	case 0x48:
		if (ACCESSING_BITS_24_31)
		{
			m_cfg[0x24] = (data >> 28) & 0x0f;
			m_cfg[0x25] = (data >> 24) & 0x0f;
		}
		break;

	case 0x4c:
		// shut-up register
		if (ACCESSING_BITS_24_31)
			autoconfig_base_address(0);
		break;
	}
}
