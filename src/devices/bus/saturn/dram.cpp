// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************

 Saturn cart emulation

 ***********************************************************************************************************/


#include "emu.h"
#include "dram.h"


//-------------------------------------------------
//  constructor
//-------------------------------------------------

const device_type SATURN_DRAM_8MB = &device_creator<saturn_dram8mb_device>;
const device_type SATURN_DRAM_32MB = &device_creator<saturn_dram32mb_device>;


saturn_dram_device::saturn_dram_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
					: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
						device_sat_cart_interface( mconfig, *this )
{
}

saturn_dram8mb_device::saturn_dram8mb_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: saturn_dram_device(mconfig, SATURN_DRAM_8MB, "Saturn Data RAM 8Mbit Cart", tag, owner, clock, "sat_dram_8mb", __FILE__)
{
	m_cart_type = 0x5a;
}

saturn_dram32mb_device::saturn_dram32mb_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: saturn_dram_device(mconfig, SATURN_DRAM_32MB, "Saturn Data RAM 32Mbit Cart", tag, owner, clock, "sat_dram_32mb", __FILE__)
{
	m_cart_type = 0x5c;
}


//-------------------------------------------------
//  mapper specific start/reset
//-------------------------------------------------

void saturn_dram_device::device_start()
{
}

void saturn_dram_device::device_reset()
{
}


/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

// RAM: two DRAM chips are present in the cart, thus accesses only go up to m_size/2!

READ32_MEMBER(saturn_dram_device::read_ext_dram0)
{
	if (offset < (0x400000/2)/4)
		return m_ext_dram0[offset % m_ext_dram0.size()];
	else
	{
		popmessage("DRAM0 read beyond its boundary! offs: %X\n", offset);
		return 0xffffffff;
	}
}

READ32_MEMBER(saturn_dram_device::read_ext_dram1)
{
	if (offset < (0x400000/2)/4)
		return m_ext_dram1[offset % m_ext_dram1.size()];
	else
	{
		popmessage("DRAM1 read beyond its boundary! offs: %X\n", offset);
		return 0xffffffff;
	}
}

WRITE32_MEMBER(saturn_dram_device::write_ext_dram0)
{
	if (offset < (0x400000/2)/4)
		COMBINE_DATA(&m_ext_dram0[offset % m_ext_dram0.size()]);
	else
		popmessage("DRAM0 write beyond its boundary! offs: %X data: %X\n", offset, data);
}

WRITE32_MEMBER(saturn_dram_device::write_ext_dram1)
{
	if (offset < (0x400000/2)/4)
		COMBINE_DATA(&m_ext_dram1[offset % m_ext_dram1.size()]);
	else
		popmessage("DRAM1 write beyond its boundary! offs: %X data: %X\n", offset, data);
}
