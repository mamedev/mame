// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Aquarius Memory Cartridges

**********************************************************************/

#include "emu.h"
#include "ram.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(AQUARIUS_RAM4, aquarius_ram4_device, "aquarius_ram4", "Aquarius 4K Memory Cartridge")
DEFINE_DEVICE_TYPE(AQUARIUS_RAM16, aquarius_ram16_device, "aquarius_ram16", "Aquarius 16K Memory Cartridge")
DEFINE_DEVICE_TYPE(AQUARIUS_RAM32, aquarius_ram32_device, "aquarius_ram32", "Aquarius 32K Memory Cartridge")
DEFINE_DEVICE_TYPE(AQUARIUS_RAM16P, aquarius_ram16p_device, "aquarius_ram16p", "Aquarius 16K+ Memory Cartridge")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  aquarius_ram_device - constructor
//-------------------------------------------------

aquarius_ram_device::aquarius_ram_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint16_t size)
	: device_t(mconfig, type, tag, owner, clock)
	, device_aquarius_cartridge_interface(mconfig, *this)
	, m_ram_size(size)
{
}

aquarius_ram4_device::aquarius_ram4_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: aquarius_ram_device(mconfig, AQUARIUS_RAM4, tag, owner, clock, 0x1000)
{
}

aquarius_ram16_device::aquarius_ram16_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: aquarius_ram_device(mconfig, AQUARIUS_RAM16, tag, owner, clock, 0x4000)
{
}

aquarius_ram32_device::aquarius_ram32_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: aquarius_ram_device(mconfig, AQUARIUS_RAM32, tag, owner, clock, 0x8000)
{
}

aquarius_ram16p_device::aquarius_ram16p_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, AQUARIUS_RAM16P, tag, owner, clock)
	, device_aquarius_cartridge_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void aquarius_ram_device::device_start()
{
	m_ram = make_unique_clear<uint8_t[]>(m_ram_size);

	save_pointer(NAME(m_ram), m_ram_size);
}

void aquarius_ram16p_device::device_start()
{
	m_ram = make_unique_clear<uint8_t[]>(0x4000);

	save_pointer(NAME(m_ram), 0x4000);
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

uint8_t aquarius_ram_device::mreq_r(offs_t offset)
{
	if (offset < m_ram_size)
	{
		return m_ram[offset];
	}

	return 0xff;
}

void aquarius_ram_device::mreq_w(offs_t offset, uint8_t data)
{
	if (offset < m_ram_size)
	{
		m_ram[offset] = data;
	}
}


uint8_t aquarius_ram16p_device::mreq_ce_r(offs_t offset)
{
	return m_ram[offset];
}

void aquarius_ram16p_device::mreq_ce_w(offs_t offset, uint8_t data)
{
	m_ram[offset] = data;
}
