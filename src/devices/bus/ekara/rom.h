// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_BUS_EKARA_ROM_H
#define MAME_BUS_EKARA_ROM_H

#pragma once

#include "slot.h"
#include "machine/i2cmem.h"

// ======================> ekara_rom_plain_device

class ekara_rom_plain_device : public device_t,
						public device_ekara_cart_interface
{
public:
	// construction/destruction
	ekara_rom_plain_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_cart) override;
	virtual DECLARE_WRITE8_MEMBER(write_cart) override;

	virtual DECLARE_READ8_MEMBER(read_extra) override { return 0xff; };
	virtual DECLARE_WRITE8_MEMBER(write_extra) override { };

	virtual READ8_MEMBER(read_rom);
	virtual WRITE8_MEMBER(write_rom);

protected:
	ekara_rom_plain_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override { }
	virtual void device_reset() override { }
};

// ======================> ekara_rom_i2c_base_device

class ekara_rom_i2c_base_device : public ekara_rom_plain_device
{
public:
	// construction/destruction
	ekara_rom_i2c_base_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	ekara_rom_i2c_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual READ8_MEMBER(read_rom) override;
	virtual WRITE8_MEMBER(write_rom) override;

	optional_device<i2cmem_device> m_i2cmem;

	virtual DECLARE_READ8_MEMBER(read_extra) override;
	virtual DECLARE_WRITE8_MEMBER(write_extra) override;

	virtual DECLARE_WRITE8_MEMBER(write_bus_control) override;

	virtual bool is_read_access_not_rom(void) override;
	virtual bool is_write_access_not_rom(void) override;

	uint8_t m_buscontrol[3];
};


// ======================> ekara_rom_i2c_24c08_epitch_device

class ekara_rom_i2c_24c08_epitch_device : public ekara_rom_i2c_base_device
{
public:
	// construction/destruction
	ekara_rom_i2c_24c08_epitch_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual bool is_read_access_not_rom(void) override;
	virtual bool is_write_access_not_rom(void) override;

	virtual void device_add_mconfig(machine_config &config) override;
};


// ======================> ekara_rom_i2c_24lc04_device

class ekara_rom_i2c_24lc04_device : public ekara_rom_i2c_base_device
{
public:
	// construction/destruction
	ekara_rom_i2c_24lc04_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override;
};

// ======================> ekara_rom_i2c_24lc02_device

class ekara_rom_i2c_24lc02_device : public ekara_rom_i2c_base_device
{
public:
	// construction/destruction
	ekara_rom_i2c_24lc02_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override;
};


// device type definition
DECLARE_DEVICE_TYPE(EKARA_ROM_PLAIN,       ekara_rom_plain_device)
DECLARE_DEVICE_TYPE(EKARA_ROM_I2C_BASE,    ekara_rom_i2c_base_device)
DECLARE_DEVICE_TYPE(EKARA_ROM_I2C_24C08_EPITCH,   ekara_rom_i2c_24c08_epitch_device)
DECLARE_DEVICE_TYPE(EKARA_ROM_I2C_24LC04,  ekara_rom_i2c_24lc04_device)
DECLARE_DEVICE_TYPE(EKARA_ROM_I2C_24LC02,  ekara_rom_i2c_24lc02_device)

#endif // MAME_BUS_EKARA_ROM_H
