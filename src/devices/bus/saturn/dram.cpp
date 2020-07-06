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

DEFINE_DEVICE_TYPE(SATURN_DRAM_8MB,  saturn_dram8mb_device,  "sat_dram_8mb",  "Saturn Data RAM 8Mbit Cart")
DEFINE_DEVICE_TYPE(SATURN_DRAM_32MB, saturn_dram32mb_device, "sat_dram_32mb", "Saturn Data RAM 32Mbit Cart")


saturn_dram_device::saturn_dram_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, int cart_type)
	: device_t(mconfig, type, tag, owner, clock)
	, device_sat_cart_interface(mconfig, *this, cart_type)
{
}

saturn_dram8mb_device::saturn_dram8mb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: saturn_dram_device(mconfig, SATURN_DRAM_8MB, tag, owner, clock, 0x5a)
{
}

saturn_dram32mb_device::saturn_dram32mb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: saturn_dram_device(mconfig, SATURN_DRAM_32MB, tag, owner, clock, 0x5c)
{
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

uint32_t saturn_dram_device::read_ext_dram0(offs_t offset)
{
	if (offset < (0x400000/2)/4)
		return m_ext_dram0[offset % m_ext_dram0.size()];
	else
	{
		popmessage("DRAM0 read beyond its boundary! offs: %X\n", offset);
		return 0xffffffff;
	}
}

uint32_t saturn_dram_device::read_ext_dram1(offs_t offset)
{
	if (offset < (0x400000/2)/4)
		return m_ext_dram1[offset % m_ext_dram1.size()];
	else
	{
		popmessage("DRAM1 read beyond its boundary! offs: %X\n", offset);
		return 0xffffffff;
	}
}

void saturn_dram_device::write_ext_dram0(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (offset < (0x400000/2)/4)
		COMBINE_DATA(&m_ext_dram0[offset % m_ext_dram0.size()]);
	else
		popmessage("DRAM0 write beyond its boundary! offs: %X data: %X\n", offset, data);
}

void saturn_dram_device::write_ext_dram1(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (offset < (0x400000/2)/4)
		COMBINE_DATA(&m_ext_dram1[offset % m_ext_dram1.size()]);
	else
		popmessage("DRAM1 write beyond its boundary! offs: %X data: %X\n", offset, data);
}
