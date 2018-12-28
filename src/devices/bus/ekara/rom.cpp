// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "rom.h"

//-------------------------------------------------
//  ekara_rom_device - constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(EKARA_ROM_PLAIN,         ekara_rom_plain_device,       "ekara_rom_plain",        "EKARA Cartridge")
DEFINE_DEVICE_TYPE(EKARA_ROM_I2C_BASE,      ekara_rom_i2c_base_device,    "ekara_rom_i2c_base",     "EKARA Cartridge with I2C")
DEFINE_DEVICE_TYPE(EKARA_ROM_I2C_24LC04,    ekara_rom_i2c_24lc04_device,  "ekara_rom_i2c_24lc04",   "EKARA Cartridge with I2C 24LC04")
DEFINE_DEVICE_TYPE(EKARA_ROM_I2C_24LC02,    ekara_rom_i2c_24lc02_device,  "ekara_rom_i2c_24lc02",   "EKARA Cartridge with I2C 24LC02")


ekara_rom_plain_device::ekara_rom_plain_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock), device_ekara_cart_interface(mconfig, *this)
{
}

ekara_rom_plain_device::ekara_rom_plain_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	ekara_rom_plain_device(mconfig, EKARA_ROM_PLAIN, tag, owner, clock)
{
}

ekara_rom_i2c_base_device::ekara_rom_i2c_base_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	ekara_rom_plain_device(mconfig, EKARA_ROM_I2C_BASE, tag, owner, clock),
	m_i2cmem(*this, "i2cmem")
{
}

ekara_rom_i2c_base_device::ekara_rom_i2c_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	ekara_rom_plain_device(mconfig, type, tag, owner, clock),
	m_i2cmem(*this, "i2cmem")
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

/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

// plain

READ8_MEMBER(ekara_rom_plain_device::read_cart)
{
	return read_rom(space, offset, mem_mask);
}

READ8_MEMBER(ekara_rom_plain_device::read_rom)
{
	return m_rom[offset & (m_rom_size-1)];
}

WRITE8_MEMBER(ekara_rom_plain_device::write_cart)
{
	write_rom(space, offset, data, mem_mask);
}

WRITE8_MEMBER(ekara_rom_plain_device::write_rom)
{
	logerror("ekara_rom_plain_device::write_rom %08x %02x\n", offset, data);
}

// i2c base

WRITE8_MEMBER(ekara_rom_i2c_base_device::write_rom)
{
	logerror("ekara_rom_i2c_base_device::write_rom %08x %02x\n", offset, data);
}

READ8_MEMBER(ekara_rom_i2c_base_device::read_rom)
{
	return m_rom[offset & (m_rom_size - 1)];
}

READ8_MEMBER(ekara_rom_i2c_base_device::read_extra)
{
	logerror("ekara_rom_i2c_base_device::read_extra %08x\n", offset);

	return (m_i2cmem->read_sda() & 1) << 7;
}

WRITE8_MEMBER(ekara_rom_i2c_base_device::write_extra)
{
	logerror("ekara_rom_i2c_base_device::write_extra %08x %02x\n", offset, data);

	m_i2cmem->write_sda((data & 0x01) >> 0);
	m_i2cmem->write_scl((data & 0x02) >> 1);
}


// i2c 24lc04

MACHINE_CONFIG_START(ekara_rom_i2c_24lc04_device::device_add_mconfig)
	I2CMEM(config, "i2cmem", 0)/*.set_page_size(16)*/.set_data_size(0x200); // 24LC04
MACHINE_CONFIG_END

// i2c 24lc02

MACHINE_CONFIG_START(ekara_rom_i2c_24lc02_device::device_add_mconfig)
	I2CMEM(config, "i2cmem", 0)/*.set_page_size(16)*/.set_data_size(0x100); // 24LC02
MACHINE_CONFIG_END


/*-------------------------------------------------
 slot interface
 -------------------------------------------------*/

void ekara_cart(device_slot_interface &device)
{
	device.option_add_internal("plain",       EKARA_ROM_PLAIN);
	device.option_add_internal("rom_24lc04",  EKARA_ROM_I2C_24LC04);
	device.option_add_internal("rom_24lc02",  EKARA_ROM_I2C_24LC02);	
}
