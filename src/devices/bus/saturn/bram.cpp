// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************

 Saturn Battery RAM cart emulation

 ***********************************************************************************************************/


#include "emu.h"
#include "bram.h"


//-------------------------------------------------
//  constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(SATURN_BRAM_4MB,  saturn_bram4mb_device,  "sat_bram_4mb",  "Saturn Battery RAM 4Mbit Cart")
DEFINE_DEVICE_TYPE(SATURN_BRAM_8MB,  saturn_bram8mb_device,  "sat_bram_8mb",  "Saturn Battery RAM 8Mbit Cart")
DEFINE_DEVICE_TYPE(SATURN_BRAM_16MB, saturn_bram16mb_device, "sat_bram_16mb", "Saturn Battery RAM 16Mbit Cart")
DEFINE_DEVICE_TYPE(SATURN_BRAM_32MB, saturn_bram32mb_device, "sat_bram_32mb", "Saturn Battery RAM 32Mbit Cart")


saturn_bram_device::saturn_bram_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, int cart_type)
	: device_t(mconfig, type, tag, owner, clock)
	, device_sat_cart_interface(mconfig, *this, cart_type)
	, device_nvram_interface(mconfig, *this)
{
}

saturn_bram4mb_device::saturn_bram4mb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: saturn_bram_device(mconfig, SATURN_BRAM_4MB, tag, owner, clock, 0x21)
{
}

saturn_bram8mb_device::saturn_bram8mb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: saturn_bram_device(mconfig, SATURN_BRAM_8MB, tag, owner, clock, 0x22)
{
}

saturn_bram16mb_device::saturn_bram16mb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: saturn_bram_device(mconfig, SATURN_BRAM_16MB, tag, owner, clock, 0x23)
{
}

saturn_bram32mb_device::saturn_bram32mb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: saturn_bram_device(mconfig, SATURN_BRAM_32MB, tag, owner, clock, 0x24)
{
}


//-------------------------------------------------
//  start/reset
//-------------------------------------------------

void saturn_bram_device::device_start()
{
}

void saturn_bram_device::device_reset()
{
}

bool saturn_bram_device::nvram_read(util::read_stream &file)
{
	size_t actual;
	if (m_ext_bram.empty())
		return true;
	else
		return !file.read(&m_ext_bram[0], m_ext_bram.size(), actual) && actual == m_ext_bram.size();
}

bool saturn_bram_device::nvram_write(util::write_stream &file)
{
	size_t actual;
	return !file.write(&m_ext_bram[0], m_ext_bram.size(), actual) && actual == m_ext_bram.size();
}

bool saturn_bram_device::nvram_can_write()
{
	return !m_ext_bram.empty();
}

void saturn_bram_device::nvram_default()
{
	static const uint8_t init[16] =
	{ 'B', 'a', 'c', 'k', 'U', 'p', 'R', 'a', 'm', ' ', 'F', 'o', 'r', 'm', 'a', 't' };
	memset(&m_ext_bram[0], 0, m_ext_bram.size());

	for (int i = 0; i < 32; i++)
	{
		for (int j = 0; j < 16; j++)
			m_ext_bram[i * 16 + j] = init[j];
	}
}


/*-------------------------------------------------
 IO handlers
 -------------------------------------------------*/

// Battery RAM: single chip

uint32_t saturn_bram_device::read_ext_bram(offs_t offset)
{
	if (offset < m_ext_bram.size()/2)
		return (m_ext_bram[offset * 2] << 16) | m_ext_bram[offset * 2 + 1];
	else
	{
		popmessage("Battery RAM read beyond its boundary! offs: %X\n", offset);
		return 0xffffffff;
	}
}

void saturn_bram_device::write_ext_bram(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (offset < m_ext_bram.size()/2)
	{
		if (ACCESSING_BITS_16_23)
			m_ext_bram[offset * 2 + 0] = (data & 0x00ff0000) >> 16;
		if (ACCESSING_BITS_0_7)
			m_ext_bram[offset * 2 + 1] = (data & 0x000000ff) >> 0;
	}
	else
		popmessage("Battery RAM write beyond its boundary! offs: %X data: %X\n", offset, data);
}
