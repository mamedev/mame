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

const device_type SATURN_BRAM_4MB = &device_creator<saturn_bram4mb_device>;
const device_type SATURN_BRAM_8MB = &device_creator<saturn_bram8mb_device>;
const device_type SATURN_BRAM_16MB = &device_creator<saturn_bram16mb_device>;
const device_type SATURN_BRAM_32MB = &device_creator<saturn_bram32mb_device>;


saturn_bram_device::saturn_bram_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
					: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
						device_sat_cart_interface( mconfig, *this ),
						device_nvram_interface(mconfig, *this)
{
}

saturn_bram4mb_device::saturn_bram4mb_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: saturn_bram_device(mconfig, SATURN_BRAM_4MB, "Saturn Battery RAM 4Mbit Cart", tag, owner, clock, "sat_bram_4mb", __FILE__)
{
	m_cart_type = 0x21;
}

saturn_bram8mb_device::saturn_bram8mb_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: saturn_bram_device(mconfig, SATURN_BRAM_8MB, "Saturn Battery RAM 8Mbit Cart", tag, owner, clock, "sat_bram_8mb", __FILE__)
{
	m_cart_type = 0x22;
}

saturn_bram16mb_device::saturn_bram16mb_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: saturn_bram_device(mconfig, SATURN_BRAM_16MB, "Saturn Battery RAM 16Mbit Cart", tag, owner, clock, "sat_bram_16mb", __FILE__)
{
	m_cart_type = 0x23;
}

saturn_bram32mb_device::saturn_bram32mb_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: saturn_bram_device(mconfig, SATURN_BRAM_32MB, "Saturn Battery RAM 32Mbit Cart", tag, owner, clock, "sat_bram_32mb", __FILE__)
{
	m_cart_type = 0x24;
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

void saturn_bram_device::nvram_default()
{
	static const UINT8 init[16] =
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

READ32_MEMBER(saturn_bram_device::read_ext_bram)
{
	if (offset < m_ext_bram.size()/2)
		return (m_ext_bram[offset * 2] << 16) | m_ext_bram[offset * 2 + 1];
	else
	{
		popmessage("Battery RAM read beyond its boundary! offs: %X\n", offset);
		return 0xffffffff;
	}
}

WRITE32_MEMBER(saturn_bram_device::write_ext_bram)
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
