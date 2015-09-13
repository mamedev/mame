// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************


 Epoch Super Cassette Vision cart emulation


 ***********************************************************************************************************/


#include "emu.h"
#include "rom.h"


//-------------------------------------------------
//  scv_rom_device - constructor
//-------------------------------------------------

const device_type SCV_ROM8K = &device_creator<scv_rom8_device>;
const device_type SCV_ROM16K = &device_creator<scv_rom16_device>;
const device_type SCV_ROM32K = &device_creator<scv_rom32_device>;
const device_type SCV_ROM32K_RAM8K = &device_creator<scv_rom32ram8_device>;
const device_type SCV_ROM64K = &device_creator<scv_rom64_device>;
const device_type SCV_ROM128K = &device_creator<scv_rom128_device>;
const device_type SCV_ROM128K_RAM4K = &device_creator<scv_rom128ram4_device>;


scv_rom8_device::scv_rom8_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
					: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
						device_scv_cart_interface( mconfig, *this )
{
}

scv_rom8_device::scv_rom8_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: device_t(mconfig, SCV_ROM8K, "SCV 8K Carts", tag, owner, clock, "scv_rom8", __FILE__),
						device_scv_cart_interface( mconfig, *this )
{
}

scv_rom16_device::scv_rom16_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: scv_rom8_device(mconfig, SCV_ROM16K, "SCV 16K Carts", tag, owner, clock, "scv_rom16", __FILE__)
{
}

scv_rom32_device::scv_rom32_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: scv_rom8_device(mconfig, SCV_ROM32K, "SCV 32K Carts", tag, owner, clock, "scv_rom32", __FILE__)
{
}

scv_rom32ram8_device::scv_rom32ram8_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: scv_rom8_device(mconfig, SCV_ROM32K_RAM8K, "SCV 32K + RAM 8K Carts", tag, owner, clock, "scv_rom32_ram8", __FILE__)
{
}

scv_rom64_device::scv_rom64_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: scv_rom8_device(mconfig, SCV_ROM16K, "SCV 64K Carts", tag, owner, clock, "scv_rom64", __FILE__)
{
}

scv_rom128_device::scv_rom128_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: scv_rom8_device(mconfig, SCV_ROM32K, "SCV 128K Carts", tag, owner, clock, "scv_rom128", __FILE__)
{
}

scv_rom128ram4_device::scv_rom128ram4_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: scv_rom8_device(mconfig, SCV_ROM128K_RAM4K, "SCV 128K + RAM 4K Carts", tag, owner, clock, "scv_rom128_ram4", __FILE__)
{
}


//-------------------------------------------------
//  mapper specific start/reset
//-------------------------------------------------

void scv_rom32ram8_device::device_start()
{
	save_item(NAME(m_ram_enabled));
}

void scv_rom32ram8_device::device_reset()
{
	m_ram_enabled = 1;
}


void scv_rom64_device::device_start()
{
	save_item(NAME(m_bank_base));
}

void scv_rom64_device::device_reset()
{
	m_bank_base = 0;
}


void scv_rom128_device::device_start()
{
	save_item(NAME(m_bank_base));
}

void scv_rom128_device::device_reset()
{
	m_bank_base = 0;
}


void scv_rom128ram4_device::device_start()
{
	save_item(NAME(m_bank_base));
	save_item(NAME(m_ram_enabled));
}

void scv_rom128ram4_device::device_reset()
{
	m_bank_base = 0;
	m_ram_enabled = 1;
}



/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

READ8_MEMBER(scv_rom8_device::read_cart)
{
	return m_rom[offset & 0x1fff];
}


READ8_MEMBER(scv_rom16_device::read_cart)
{
	return m_rom[offset & 0x3fff];
}


READ8_MEMBER(scv_rom32_device::read_cart)
{
	return m_rom[offset];
}


READ8_MEMBER(scv_rom32ram8_device::read_cart)
{
	if (m_ram_enabled && offset >= 0x6000)
		return m_ram[offset & 0x1fff];

	return m_rom[offset];
}

WRITE8_MEMBER(scv_rom32ram8_device::write_cart)
{
	if (m_ram_enabled && offset >= 0x6000)
		m_ram[offset & 0x1fff] = data;
}

WRITE8_MEMBER(scv_rom32ram8_device::write_bank)
{
	m_ram_enabled = BIT(data, 5);
}


READ8_MEMBER(scv_rom64_device::read_cart)
{
	return m_rom[offset + (m_bank_base * 0x8000)];
}

WRITE8_MEMBER(scv_rom64_device::write_bank)
{
	m_bank_base = BIT(data, 5);
}


READ8_MEMBER(scv_rom128_device::read_cart)
{
	return m_rom[offset + (m_bank_base * 0x8000)];
}

WRITE8_MEMBER(scv_rom128_device::write_bank)
{
	m_bank_base = (data >> 5) & 0x03;
}


READ8_MEMBER(scv_rom128ram4_device::read_cart)
{
	if (m_ram_enabled && offset >= 0x7000)
		return m_ram[offset & 0xfff];

	return m_rom[offset + (m_bank_base * 0x8000)];
}

WRITE8_MEMBER(scv_rom128ram4_device::write_cart)
{
	if (m_ram_enabled && offset >= 0x7000)
		m_ram[offset & 0xfff] = data;
}

WRITE8_MEMBER(scv_rom128ram4_device::write_bank)
{
	m_bank_base = (data >> 5) & 0x03;
	m_ram_enabled = BIT(data, 6);
}
