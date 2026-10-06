// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "rom.h"

//-------------------------------------------------
//  leapster_rom_device - constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(LEAPSTER_ROM_PLAIN,         leapster_rom_plain_device,       "leapster_rom_plain",        "LeapFrog Leapster Cartridge")
DEFINE_DEVICE_TYPE(LEAPSTER_ROM_NVRAM,         leapster_rom_nvram_device,       "leapster_rom_nvram",        "LeapFrog Leapster Cartridge (NVRAM)")


leapster_rom_plain_device::leapster_rom_plain_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock), device_leapster_interface(mconfig, *this)
{
}

leapster_rom_plain_device::leapster_rom_plain_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	leapster_rom_plain_device(mconfig, LEAPSTER_ROM_PLAIN, tag, owner, clock)
{
}

leapster_rom_nvram_device::leapster_rom_nvram_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	leapster_rom_plain_device(mconfig, type, tag, owner, clock),
	m_nvram(*this, "nvram")
{
}

leapster_rom_nvram_device::leapster_rom_nvram_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	leapster_rom_nvram_device(mconfig, LEAPSTER_ROM_NVRAM, tag, owner, clock)
{
}

/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

// plain

uint16_t leapster_rom_plain_device::read_cart(offs_t offset)
{
	return read_rom(offset);
}

uint16_t leapster_rom_plain_device::read_rom(offs_t offset)
{
	return m_rom[offset & (m_rom_size-1)];
}

void leapster_rom_plain_device::write_cart(offs_t offset, uint16_t data)
{
	write_rom(offset, data);
}

void leapster_rom_plain_device::write_rom(offs_t offset, uint16_t data)
{
	logerror("leapster_rom_plain_device::write_rom %08x %04x\n", offset, data);
}

// i2c base

void leapster_rom_nvram_device::write_rom(offs_t offset, uint16_t data)
{
	logerror("leapster_rom_nvram_device::write_rom %08x %04x\n", offset, data);
}

uint16_t leapster_rom_nvram_device::read_rom(offs_t offset)
{
	return m_rom[offset & (m_rom_size - 1)];
}

#if 0
uint8_t leapster_rom_nvram_device::read_cart_seeprom(void)
{
	logerror("leapster_rom_nvram_device::read_cart_seeprom\n");

	return m_i2cmem->read_sda();
}

void leapster_rom_nvram_device::write_cart_seeprom(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (BIT(mem_mask, 1))
		m_i2cmem->write_scl(BIT(data, 1));
	if (BIT(mem_mask, 0))
		m_i2cmem->write_sda(BIT(data, 0));
}
#endif

void leapster_rom_nvram_device::device_add_mconfig(machine_config &config)
{
	NVRAM(config, m_nvram);
}

void leapster_rom_nvram_device::device_start()
{
	leapster_rom_plain_device::device_start();
	m_cartridge_eeprom = make_unique_clear<uint8_t[]>(2048);
	m_nvram->set_base(m_cartridge_eeprom.get(), 2048);
}

/*-------------------------------------------------
 slot interface
 -------------------------------------------------*/

void leapster_cart(device_slot_interface &device)
{
	device.option_add_internal("plain",       LEAPSTER_ROM_PLAIN);
	device.option_add_internal("rom_nvram",   LEAPSTER_ROM_NVRAM);
}
