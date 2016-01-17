// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************


 V-Tech CreatiVision cart emulation


 ***********************************************************************************************************/


#include "emu.h"
#include "rom.h"


//-------------------------------------------------
//  crvision_rom_device - constructor
//-------------------------------------------------

const device_type CRVISION_ROM_4K = &device_creator<crvision_rom_device>;
const device_type CRVISION_ROM_6K = &device_creator<crvision_rom6k_device>;
const device_type CRVISION_ROM_8K = &device_creator<crvision_rom8k_device>;
const device_type CRVISION_ROM_10K = &device_creator<crvision_rom10k_device>;
const device_type CRVISION_ROM_12K = &device_creator<crvision_rom12k_device>;
const device_type CRVISION_ROM_16K = &device_creator<crvision_rom16k_device>;
const device_type CRVISION_ROM_18K = &device_creator<crvision_rom18k_device>;


crvision_rom_device::crvision_rom_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source)
					: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
						device_crvision_cart_interface(mconfig, *this)
{
}

crvision_rom_device::crvision_rom_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: device_t(mconfig, CRVISION_ROM_4K, "CreatiVision 4K Carts", tag, owner, clock, "crvision_4k", __FILE__),
						device_crvision_cart_interface(mconfig, *this)
{
}

crvision_rom6k_device::crvision_rom6k_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: crvision_rom_device(mconfig, CRVISION_ROM_6K, "CreatiVision 6K Carts", tag, owner, clock, "crvision_6k", __FILE__)
{
}

crvision_rom8k_device::crvision_rom8k_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: crvision_rom_device(mconfig, CRVISION_ROM_8K, "CreatiVision 8K Carts", tag, owner, clock, "crvision_8k", __FILE__)
{
}

crvision_rom10k_device::crvision_rom10k_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: crvision_rom_device(mconfig, CRVISION_ROM_10K, "CreatiVision 10K Carts", tag, owner, clock, "crvision_10k", __FILE__)
{
}

crvision_rom12k_device::crvision_rom12k_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: crvision_rom_device(mconfig, CRVISION_ROM_12K, "CreatiVision 12K Carts", tag, owner, clock, "crvision_12k", __FILE__)
{
}

crvision_rom16k_device::crvision_rom16k_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: crvision_rom_device(mconfig, CRVISION_ROM_16K, "CreatiVision 16K Carts", tag, owner, clock, "crvision_16k", __FILE__)
{
}

crvision_rom18k_device::crvision_rom18k_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: crvision_rom_device(mconfig, CRVISION_ROM_18K, "CreatiVision 18K Carts", tag, owner, clock, "crvision_18k", __FILE__)
{
}


/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

READ8_MEMBER(crvision_rom_device::read_rom80)
{
	offset &= 0x1fff;
	if (offset < 0x1000)
		return 0xff;

	return m_rom[offset & 0xfff];
}


READ8_MEMBER(crvision_rom6k_device::read_rom80)
{
	offset &= 0x1fff;
	if (offset < 0x1000)
		return m_rom[0x1000 + (offset & 0x7ff)];

	return m_rom[offset & 0xfff];
}


READ8_MEMBER(crvision_rom8k_device::read_rom80)
{
	return m_rom[offset & 0x1fff];
}


READ8_MEMBER(crvision_rom10k_device::read_rom80)
{
	return m_rom[offset & 0x1fff];
}

READ8_MEMBER(crvision_rom10k_device::read_rom40)
{
	return m_rom[0x2000 + (offset & 0x7ff)];
}


READ8_MEMBER(crvision_rom12k_device::read_rom80)
{
	return m_rom[offset & 0x1fff];
}

READ8_MEMBER(crvision_rom12k_device::read_rom40)
{
	return m_rom[0x2000 + (offset & 0xfff)];
}


READ8_MEMBER(crvision_rom16k_device::read_rom80)
{
	// lower 8K in 0xa000-0xbfff, higher 8K in 0x8000-0x9fff
	return m_rom[offset ^ 0x2000];
}


READ8_MEMBER(crvision_rom18k_device::read_rom80)
{
	// lower 8K in 0xa000-0xbfff, higher 8K in 0x8000-0x9fff
	return m_rom[offset ^ 0x2000];
}

READ8_MEMBER(crvision_rom18k_device::read_rom40)
{
	return m_rom[0x4000 + (offset & 0x7ff)];
}
