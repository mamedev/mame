// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Sprow 24bBC/RAM Disc Board

    http://www.sprow.co.uk/bbc/ramdisc.htm

**********************************************************************/


#include "emu.h"
#include "24bbc.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(BBC_24BBC, bbc_24bbc_device, "bbc_24bbc", "Sprow 24bBC/RAM Disc Board");


//-------------------------------------------------
//  ROM( ramfs )
//-------------------------------------------------

ROM_START(ramfs)
	ROM_REGION(0x4000, "exp_rom", 0)
	ROM_LOAD("ramfs120.rom", 0x0000, 0x4000, CRC(782e6386) SHA1(65493cc1f6ae31cf39298d67dad42ea23be4cae6))
ROM_END


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void bbc_24bbc_device::device_add_mconfig(machine_config &config)
{
	BBC_1MHZBUS_SLOT(config, m_1mhzbus, DERIVED_CLOCK(1, 1), bbc_1mhzbus_devices, nullptr);
	m_1mhzbus->irq_handler().set(DEVICE_SELF_OWNER, FUNC(bbc_1mhzbus_slot_device::irq_w));
	m_1mhzbus->nmi_handler().set(DEVICE_SELF_OWNER, FUNC(bbc_1mhzbus_slot_device::nmi_w));
}


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry* bbc_24bbc_device::device_rom_region() const
{
	return ROM_NAME(ramfs);
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbc_24bbc_device - constructor
//-------------------------------------------------

bbc_24bbc_device::bbc_24bbc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, BBC_24BBC, tag, owner, clock)
	, device_bbc_1mhzbus_interface(mconfig, *this)
	, m_1mhzbus(*this, "1mhzbus")
	, m_ram_addr(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_24bbc_device::device_start()
{
	m_ram = make_unique_clear<uint8_t[]>(0x100000);

	/* register for save states */
	save_item(NAME(m_ram_addr));
	save_pointer(NAME(m_ram), 0x100000);
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

uint8_t bbc_24bbc_device::fred_r(offs_t offset)
{
	uint8_t data = 0xff;

	switch (offset)
	{
	case 0x00:
		data = (m_ram_addr & 0x0000ff);
		break;
	case 0x01:
		data = (m_ram_addr & 0x00ff00) >> 8;
		break;
	case 0x02:
		data = (m_ram_addr & 0xff0000) >> 16;
		break;
	case 0x03:
		data = m_ram[m_ram_addr & 0xfffff];
		break;
	}

	data &= m_1mhzbus->fred_r(offset);

	return data;
}

void bbc_24bbc_device::fred_w(offs_t offset, uint8_t data)
{
	switch (offset)
	{
	case 0x00:
		m_ram_addr = (m_ram_addr & 0xffff00) | data;
		break;
	case 0x01:
		m_ram_addr = (m_ram_addr & 0xff00ff) | (data << 8);
		break;
	case 0x02:
		m_ram_addr = (m_ram_addr & 0x00ffff) | (data << 16);
		break;
	case 0x03:
		m_ram[m_ram_addr & 0xfffff] = data;
		break;
	}

	m_1mhzbus->fred_w(offset, data);
}

uint8_t bbc_24bbc_device::jim_r(offs_t offset)
{
	return m_1mhzbus->jim_r(offset);
}

void bbc_24bbc_device::jim_w(offs_t offset, uint8_t data)
{
	m_1mhzbus->jim_w(offset, data);
}
