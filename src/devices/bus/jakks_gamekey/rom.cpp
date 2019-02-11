// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "rom.h"

//-------------------------------------------------
//  jakks_gamekey_rom_device - constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(JAKKS_GAMEKEY_ROM_PLAIN,         jakks_gamekey_rom_plain_device,       "jakks_gamekey_rom_plain",        "JAKKS Pacific GameKey")
DEFINE_DEVICE_TYPE(JAKKS_GAMEKEY_ROM_I2C_BASE,      jakks_gamekey_rom_i2c_base_device,    "jakks_gamekey_rom_i2c_base",     "JAKKS Pacific GameKey with I2C")
DEFINE_DEVICE_TYPE(JAKKS_GAMEKEY_ROM_I2C_24LC04,    jakks_gamekey_rom_i2c_24lc04_device,  "jakks_gamekey_rom_i2c_24lc04",   "JAKKS Pacific GameKey with I2C 24LC04")


jakks_gamekey_rom_plain_device::jakks_gamekey_rom_plain_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock), device_jakks_gamekey_interface(mconfig, *this)
{
}

jakks_gamekey_rom_plain_device::jakks_gamekey_rom_plain_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	jakks_gamekey_rom_plain_device(mconfig, JAKKS_GAMEKEY_ROM_PLAIN, tag, owner, clock)
{
}

jakks_gamekey_rom_i2c_base_device::jakks_gamekey_rom_i2c_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	jakks_gamekey_rom_plain_device(mconfig, type, tag, owner, clock),
	m_i2cmem(*this, "i2cmem")
{
}

jakks_gamekey_rom_i2c_base_device::jakks_gamekey_rom_i2c_base_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	jakks_gamekey_rom_i2c_base_device(mconfig, JAKKS_GAMEKEY_ROM_I2C_BASE, tag, owner, clock)
{
}

jakks_gamekey_rom_i2c_24lc04_device::jakks_gamekey_rom_i2c_24lc04_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	jakks_gamekey_rom_i2c_base_device(mconfig, JAKKS_GAMEKEY_ROM_I2C_24LC04, tag, owner, clock)
{
}

/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

// plain

READ16_MEMBER(jakks_gamekey_rom_plain_device::read_cart)
{
	return read_rom(space, offset, mem_mask);
}

READ16_MEMBER(jakks_gamekey_rom_plain_device::read_rom)
{
	return m_rom[offset & (m_rom_size-1)];
}

WRITE16_MEMBER(jakks_gamekey_rom_plain_device::write_cart)
{
	write_rom(space, offset, data, mem_mask);
}

WRITE16_MEMBER(jakks_gamekey_rom_plain_device::write_rom)
{
	logerror("jakks_gamekey_rom_plain_device::write_rom %08x %02x\n", offset, data);
}

// i2c base

WRITE16_MEMBER(jakks_gamekey_rom_i2c_base_device::write_rom)
{
	logerror("jakks_gamekey_rom_i2c_base_device::write_rom %08x %02x\n", offset, data);
}

READ16_MEMBER(jakks_gamekey_rom_i2c_base_device::read_rom)
{
	return m_rom[offset & (m_rom_size - 1)];
}

uint8_t jakks_gamekey_rom_i2c_base_device::read_cart_seeprom(void)
{
	logerror("jakks_gamekey_rom_i2c_base_device::read_cart_seeprom\n");

	return m_i2cmem->read_sda();
}

WRITE16_MEMBER(jakks_gamekey_rom_i2c_base_device::write_cart_seeprom)
{
	if (BIT(mem_mask, 1))
		m_i2cmem->write_scl(BIT(data, 1));
	if (BIT(mem_mask, 0))
		m_i2cmem->write_sda(BIT(data, 0));
}

// i2c 24lc04

void jakks_gamekey_rom_i2c_24lc04_device::device_add_mconfig(machine_config &config)
{
	I2CMEM(config, "i2cmem", 0)/*.set_page_size(16)*/.set_data_size(0x200); // 24LC04
}


/*-------------------------------------------------
 slot interface
 -------------------------------------------------*/

void jakks_gamekey(device_slot_interface &device)
{
	device.option_add_internal("plain",       JAKKS_GAMEKEY_ROM_PLAIN);
	device.option_add_internal("rom_24lc04",  JAKKS_GAMEKEY_ROM_I2C_24LC04);
}
