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

// plain cartridge

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

// cartridge with NVRAM (exact NVRAM/EEPROM type unknown)

void leapster_rom_nvram_device::write_rom(offs_t offset, uint16_t data)
{
	logerror("leapster_rom_nvram_device::write_rom %08x %04x\n", offset, data);
}

uint16_t leapster_rom_nvram_device::read_rom(offs_t offset)
{
	return m_rom[offset & (m_rom_size - 1)];
}



uint8_t leapster_rom_nvram_device::read_nvram(uint16_t offset)
{
	return m_cartridge_eeprom.get()[offset & (NVRAM_SIZE-1)];
}

void leapster_rom_nvram_device::write_nvram(uint16_t offset, uint8_t data)
{
	m_cartridge_eeprom.get()[offset & (NVRAM_SIZE-1)] = data;
}


void leapster_rom_nvram_device::device_add_mconfig(machine_config &config)
{
	NVRAM(config, m_nvram);
}

void leapster_rom_nvram_device::device_start()
{
	leapster_rom_plain_device::device_start();

	m_cartridge_eeprom = make_unique_clear<uint8_t[]>(NVRAM_SIZE);
	m_nvram->set_base(m_cartridge_eeprom.get(), NVRAM_SIZE);
	save_pointer(NAME(m_cartridge_eeprom.get()), NVRAM_SIZE);
}

/*-------------------------------------------------
 slot interface
 -------------------------------------------------*/

void leapster_cart(device_slot_interface &device)
{
	device.option_add_internal("plain",       LEAPSTER_ROM_PLAIN);
	device.option_add_internal("rom_nvram",   LEAPSTER_ROM_NVRAM);
}
