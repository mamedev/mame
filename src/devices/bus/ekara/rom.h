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
	virtual uint8_t read_cart(offs_t offset) override;
	virtual void write_cart(offs_t offset, uint8_t data) override;

	virtual uint8_t read_extra(offs_t offset) override { return 0xff; }
	virtual void write_extra(offs_t offset, uint8_t data) override { }

	virtual uint8_t read_rom(offs_t offset);
	virtual void write_rom(offs_t offset, uint8_t data);

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
	virtual uint8_t read_rom(offs_t offset) override;
	virtual void write_rom(offs_t offset, uint8_t data) override;

	optional_device<i2cmem_device> m_i2cmem;

	virtual uint8_t read_extra(offs_t offset) override;
	virtual void write_extra(offs_t offset, uint8_t data) override;

	virtual void write_bus_control(offs_t offset, uint8_t data) override;

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

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};


// ======================> ekara_rom_i2c_24lc04_device

class ekara_rom_i2c_24lc04_device : public ekara_rom_i2c_base_device
{
public:
	// construction/destruction
	ekara_rom_i2c_24lc04_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};

// ======================> ekara_rom_i2c_24lc02_device

class ekara_rom_i2c_24lc02_device : public ekara_rom_i2c_base_device
{
public:
	// construction/destruction
	ekara_rom_i2c_24lc02_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};


// ======================> ekara_rom_i2c_24lc02_gc0010_device

class ekara_rom_i2c_24lc02_gc0010_device : public ekara_rom_i2c_base_device
{
public:
	// construction/destruction
	ekara_rom_i2c_24lc02_gc0010_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	ekara_rom_i2c_24lc02_gc0010_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	bool is_read_access_not_rom(void) override;
	bool is_write_access_not_rom(void) override;
	uint8_t read_extra(offs_t offset) override;
	void write_extra(offs_t offset, uint8_t data) override;
	void write_sda(int state) override;
	void write_scl(int state) override;
	int read_sda() override;
};

class ekara_rom_i2c_24lc08_evio_device : public ekara_rom_i2c_24lc02_gc0010_device
{
public:
	// construction/destruction
	ekara_rom_i2c_24lc08_evio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

};


// device type definition
DECLARE_DEVICE_TYPE(EKARA_ROM_PLAIN,       ekara_rom_plain_device)
DECLARE_DEVICE_TYPE(EKARA_ROM_I2C_BASE,    ekara_rom_i2c_base_device)
DECLARE_DEVICE_TYPE(EKARA_ROM_I2C_24C08_EPITCH,   ekara_rom_i2c_24c08_epitch_device)
DECLARE_DEVICE_TYPE(EKARA_ROM_I2C_24LC04,  ekara_rom_i2c_24lc04_device)
DECLARE_DEVICE_TYPE(EKARA_ROM_I2C_24LC02,  ekara_rom_i2c_24lc02_device)
DECLARE_DEVICE_TYPE(EKARA_ROM_I2C_24LC02_GC0010,  ekara_rom_i2c_24lc02_gc0010_device)
DECLARE_DEVICE_TYPE(EKARA_ROM_I2C_24LC08_EVIO,  ekara_rom_i2c_24lc08_evio_device)

#endif // MAME_BUS_EKARA_ROM_H
