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

DEFINE_DEVICE_TYPE(CRVISION_ROM_4K,  crvision_rom_device,    "crvision_4k",  "CreatiVision 4K Carts")
DEFINE_DEVICE_TYPE(CRVISION_ROM_6K,  crvision_rom6k_device,  "crvision_6k",  "CreatiVision 6K Carts")
DEFINE_DEVICE_TYPE(CRVISION_ROM_8K,  crvision_rom8k_device,  "crvision_8k",  "CreatiVision 8K Carts")
DEFINE_DEVICE_TYPE(CRVISION_ROM_10K, crvision_rom10k_device, "crvision_10k", "CreatiVision 10K Carts")
DEFINE_DEVICE_TYPE(CRVISION_ROM_12K, crvision_rom12k_device, "crvision_12k", "CreatiVision 12K Carts")
DEFINE_DEVICE_TYPE(CRVISION_ROM_16K, crvision_rom16k_device, "crvision_16k", "CreatiVision 16K Carts")
DEFINE_DEVICE_TYPE(CRVISION_ROM_18K, crvision_rom18k_device, "crvision_18k", "CreatiVision 18K Carts")


crvision_rom_device::crvision_rom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_crvision_cart_interface(mconfig, *this)
{
}

crvision_rom_device::crvision_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: crvision_rom_device(mconfig, CRVISION_ROM_4K, tag, owner, clock)
{
}

crvision_rom6k_device::crvision_rom6k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: crvision_rom_device(mconfig, CRVISION_ROM_6K, tag, owner, clock)
{
}

crvision_rom8k_device::crvision_rom8k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: crvision_rom_device(mconfig, CRVISION_ROM_8K, tag, owner, clock)
{
}

crvision_rom10k_device::crvision_rom10k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: crvision_rom_device(mconfig, CRVISION_ROM_10K, tag, owner, clock)
{
}

crvision_rom12k_device::crvision_rom12k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: crvision_rom_device(mconfig, CRVISION_ROM_12K, tag, owner, clock)
{
}

crvision_rom16k_device::crvision_rom16k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: crvision_rom_device(mconfig, CRVISION_ROM_16K, tag, owner, clock)
{
}

crvision_rom18k_device::crvision_rom18k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: crvision_rom_device(mconfig, CRVISION_ROM_18K, tag, owner, clock)
{
}


/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

uint8_t crvision_rom_device::read_rom80(offs_t offset)
{
	offset &= 0x1fff;
	if (offset < 0x1000)
		return 0xff;

	return m_rom[offset & 0xfff];
}


uint8_t crvision_rom6k_device::read_rom80(offs_t offset)
{
	offset &= 0x1fff;
	if (offset < 0x1000)
		return m_rom[0x1000 + (offset & 0x7ff)];

	return m_rom[offset & 0xfff];
}


uint8_t crvision_rom8k_device::read_rom80(offs_t offset)
{
	return m_rom[offset & 0x1fff];
}


uint8_t crvision_rom10k_device::read_rom80(offs_t offset)
{
	return m_rom[offset & 0x1fff];
}

uint8_t crvision_rom10k_device::read_rom40(offs_t offset)
{
	return m_rom[0x2000 + (offset & 0x7ff)];
}


uint8_t crvision_rom12k_device::read_rom80(offs_t offset)
{
	return m_rom[offset & 0x1fff];
}

uint8_t crvision_rom12k_device::read_rom40(offs_t offset)
{
	return m_rom[0x2000 + (offset & 0xfff)];
}


uint8_t crvision_rom16k_device::read_rom80(offs_t offset)
{
	// lower 8K in 0xa000-0xbfff, higher 8K in 0x8000-0x9fff
	return m_rom[offset ^ 0x2000];
}


uint8_t crvision_rom18k_device::read_rom80(offs_t offset)
{
	// lower 8K in 0xa000-0xbfff, higher 8K in 0x8000-0x9fff
	return m_rom[offset ^ 0x2000];
}

uint8_t crvision_rom18k_device::read_rom40(offs_t offset)
{
	return m_rom[0x4000 + (offset & 0x7ff)];
}
