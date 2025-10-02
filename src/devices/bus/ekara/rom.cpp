// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "rom.h"

//-------------------------------------------------
//  ekara_rom_device - constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(EKARA_ROM_PLAIN,         ekara_rom_plain_device,       "ekara_rom_plain",        "EKARA Cartridge")
DEFINE_DEVICE_TYPE(EKARA_ROM_I2C_BASE,      ekara_rom_i2c_base_device,    "ekara_rom_i2c_base",     "EKARA Cartridge with I2C")
DEFINE_DEVICE_TYPE(EKARA_ROM_I2C_24C08_EPITCH,     ekara_rom_i2c_24c08_epitch_device,   "ekara_rom_i2c_24c08",    "EKARA Cartridge with I2C 24C08 (e-pitch)")
DEFINE_DEVICE_TYPE(EKARA_ROM_I2C_24LC04,    ekara_rom_i2c_24lc04_device,  "ekara_rom_i2c_24lc04",   "EKARA Cartridge with I2C 24LC04")
DEFINE_DEVICE_TYPE(EKARA_ROM_I2C_24LC02,    ekara_rom_i2c_24lc02_device,  "ekara_rom_i2c_24lc02",   "EKARA Cartridge with I2C 24LC02")
DEFINE_DEVICE_TYPE(EKARA_ROM_I2C_24LC02_GC0010,    ekara_rom_i2c_24lc02_gc0010_device,  "ekara_rom_i2c_24lc02_gc0010",   "EKARA Cartridge with I2C 24LC02 (GC0010 direct access)")
DEFINE_DEVICE_TYPE(EKARA_ROM_I2C_24LC08_EVIO,    ekara_rom_i2c_24lc08_evio_device,  "ekara_rom_i2c_24lc08_evio",   "EKARA Cartridge with I2C 24LC08 (evio direct access)")


ekara_rom_plain_device::ekara_rom_plain_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock), device_ekara_cart_interface(mconfig, *this)
{
}

ekara_rom_plain_device::ekara_rom_plain_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	ekara_rom_plain_device(mconfig, EKARA_ROM_PLAIN, tag, owner, clock)
{
}



ekara_rom_i2c_base_device::ekara_rom_i2c_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	ekara_rom_plain_device(mconfig, type, tag, owner, clock),
	m_i2cmem(*this, "i2cmem")
{
	m_buscontrol[0] = m_buscontrol[1] = m_buscontrol[2] = 0x00;
}

ekara_rom_i2c_base_device::ekara_rom_i2c_base_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	ekara_rom_i2c_base_device(mconfig, EKARA_ROM_I2C_BASE, tag, owner, clock)
{
}

ekara_rom_i2c_24c08_epitch_device::ekara_rom_i2c_24c08_epitch_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	ekara_rom_i2c_base_device(mconfig, EKARA_ROM_I2C_24C08_EPITCH, tag, owner, clock)
{
}

ekara_rom_i2c_24lc04_device::ekara_rom_i2c_24lc04_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	ekara_rom_i2c_base_device(mconfig, EKARA_ROM_I2C_24LC04, tag, owner, clock)
{
}

ekara_rom_i2c_24lc02_device::ekara_rom_i2c_24lc02_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	ekara_rom_i2c_base_device(mconfig, EKARA_ROM_I2C_24LC02, tag, owner, clock)
{
}

ekara_rom_i2c_24lc02_gc0010_device::ekara_rom_i2c_24lc02_gc0010_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	ekara_rom_i2c_24lc02_gc0010_device(mconfig, EKARA_ROM_I2C_24LC02_GC0010, tag, owner, clock)
{
}

ekara_rom_i2c_24lc02_gc0010_device::ekara_rom_i2c_24lc02_gc0010_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	ekara_rom_i2c_base_device(mconfig, type, tag, owner, clock)
{
}

ekara_rom_i2c_24lc08_evio_device::ekara_rom_i2c_24lc08_evio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	ekara_rom_i2c_24lc02_gc0010_device(mconfig, EKARA_ROM_I2C_24LC08_EVIO, tag, owner, clock)
{
}

/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

// plain

uint8_t ekara_rom_plain_device::read_cart(offs_t offset)
{
	return read_rom(offset);
}

uint8_t ekara_rom_plain_device::read_rom(offs_t offset)
{
	return m_rom[offset & (m_rom_size-1)];
}

void ekara_rom_plain_device::write_cart(offs_t offset, uint8_t data)
{
	write_rom(offset, data);
}

void ekara_rom_plain_device::write_rom(offs_t offset, uint8_t data)
{
	logerror("ekara_rom_plain_device::write_rom %08x %02x\n", offset, data);
}

// i2c base

bool ekara_rom_i2c_base_device::is_read_access_not_rom(void)
{
	return (m_buscontrol[1] & 0x08) ? true : false;
}

bool ekara_rom_i2c_base_device::is_write_access_not_rom(void)
{
	return (m_buscontrol[0] & 0x08) ? true : false;
}

void ekara_rom_i2c_base_device::write_bus_control(offs_t offset, uint8_t data)
{
	logerror("ekara_rom_i2c_base_device::write_bus_control %08x %02x\n", offset, data);
	m_buscontrol[offset] = data;
}

void ekara_rom_i2c_base_device::write_rom(offs_t offset, uint8_t data)
{
	if (is_write_access_not_rom())
	{
		if (offset == 0x3fffff)
			write_extra(offset,data);
		else
			logerror("ekara_rom_i2c_base_device::write_rom %08x %02x\n", offset, data);
	}
	else
	{
		logerror("ekara_rom_i2c_base_device::write_rom %08x %02x\n", offset, data);
	}
}

uint8_t ekara_rom_i2c_base_device::read_rom(offs_t offset)
{
	if (is_read_access_not_rom())
	{
		if (offset == 0x5fffff)
			return read_extra(offset);
		else
			return m_rom[offset & (m_rom_size - 1)];
	}
	else
	{
		return m_rom[offset & (m_rom_size - 1)];
	}
}

uint8_t ekara_rom_i2c_base_device::read_extra(offs_t offset)
{
	logerror("ekara_rom_i2c_base_device::read_extra %08x\n", offset);

	return (m_i2cmem->read_sda() & 1) ? 0xff : 0x00;
}

void ekara_rom_i2c_base_device::write_extra(offs_t offset, uint8_t data)
{
	logerror("ekara_rom_i2c_base_device::write_extra %08x %02x\n", offset, data);

	m_i2cmem->write_sda((data & 0x01) >> 0);
	m_i2cmem->write_scl((data & 0x02) >> 1);
}

// i2c 24c08 (for epitch carts)

bool ekara_rom_i2c_24c08_epitch_device::is_read_access_not_rom(void)
{
	// write 0x08 before reading from SEEPROM
	// reads from 005fffff, cart does no accress decoding
	return (m_buscontrol[1] & 0x08) ? true : false;
}

bool ekara_rom_i2c_24c08_epitch_device::is_write_access_not_rom(void)
{
	// writes 0x05 before writing to SEEPROM
	// actually writes to address 3fffff which is where a mirror of the base ROM sits, but clearly all signals route through the cartridge too
	// cart does no accress decoding
	return (m_buscontrol[1] & 0x04) ? true : false;
}

void ekara_rom_i2c_24c08_epitch_device::device_add_mconfig(machine_config &config)
{
	I2C_24C08(config, "i2cmem", 0);
}

// i2c 24lc04

void ekara_rom_i2c_24lc04_device::device_add_mconfig(machine_config &config)
{
	I2C_24C04(config, "i2cmem", 0); // 24LC04
}

// i2c 24lc02

void ekara_rom_i2c_24lc02_device::device_add_mconfig(machine_config &config)
{
	I2C_24C02(config, "i2cmem", 0); // 24LC02
}

// i2c 24lc02 with direct IO port access

bool ekara_rom_i2c_24lc02_gc0010_device::is_read_access_not_rom(void)
{
	return false;
}

bool ekara_rom_i2c_24lc02_gc0010_device::is_write_access_not_rom(void)
{
	return false;
}

uint8_t ekara_rom_i2c_24lc02_gc0010_device::read_extra(offs_t offset)
{
	return 0;
}

void ekara_rom_i2c_24lc02_gc0010_device::write_extra(offs_t offset, uint8_t data)
{
}


void ekara_rom_i2c_24lc02_gc0010_device::write_sda(int state)
{
	m_i2cmem->write_sda(state);
}

void ekara_rom_i2c_24lc02_gc0010_device::write_scl(int state)
{
	m_i2cmem->write_scl(state);
}

int ekara_rom_i2c_24lc02_gc0010_device::read_sda()
{
	return m_i2cmem->read_sda();
}



void ekara_rom_i2c_24lc02_gc0010_device::device_add_mconfig(machine_config &config)
{
	I2C_24C02(config, "i2cmem", 0); // 24LC02
}


void ekara_rom_i2c_24lc08_evio_device::device_add_mconfig(machine_config &config)
{
	I2C_24C08(config, "i2cmem", 0);
}

/*-------------------------------------------------
 slot interface
 -------------------------------------------------*/

void ekara_cart(device_slot_interface &device)
{
	device.option_add_internal("plain",       EKARA_ROM_PLAIN);
	device.option_add_internal("rom_24c08_epitch",  EKARA_ROM_I2C_24C08_EPITCH);
	device.option_add_internal("rom_24lc04",  EKARA_ROM_I2C_24LC04);
	device.option_add_internal("rom_24lc02",  EKARA_ROM_I2C_24LC02);
	device.option_add_internal("rom_24lc02_gc0010",  EKARA_ROM_I2C_24LC02_GC0010);
	device.option_add_internal("rom_24lc08_evio",  EKARA_ROM_I2C_24LC08_EVIO);
}
