// license:BSD-3-Clause
// copyright-holders:David Haywood, MagikalUnicorn
#ifndef MAME_BUS_XAVIX_CART_ROM_H
#define MAME_BUS_XAVIX_CART_ROM_H

#pragma once

#include "cart.h"
#include "machine/i2cmem.h"

class xavix_rom_plain_device : public device_t,
						public device_xavix_cart_interface
{
public:
	xavix_rom_plain_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device_xavix_cart_interface implementation
	virtual uint8_t read_cart(offs_t offset) override;
	virtual void write_cart(offs_t offset, uint8_t data) override;

protected:
	xavix_rom_plain_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device_t implementation
	virtual void device_start() override ATTR_COLD { }
};

class xavix_rom_i2c_base_device : public xavix_rom_plain_device
{
public:
	// device_xavix_cart_interface implementation
	virtual uint8_t read_cart(offs_t offset) override;
	virtual void write_cart(offs_t offset, uint8_t data) override;
	virtual void write_bus_control(offs_t offset, uint8_t data) override;

protected:
	xavix_rom_i2c_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint8_t bus_write_control_index = 0, uint8_t bus_write_control_mask = 0x08);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	uint8_t read_i2c(offs_t offset);
	void write_i2c(offs_t offset, uint8_t data);

	required_device<i2cmem_device> m_i2cmem;
	uint8_t m_buscontrol[3];
	uint8_t const m_bus_write_control_index;
	uint8_t const m_bus_write_control_mask;
};

class xavix_rom_24c04_control1_device : public xavix_rom_i2c_base_device
{
public:
	xavix_rom_24c04_control1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device_t implementation
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};

class xavix_rom_24c08_control1_device : public xavix_rom_i2c_base_device
{
public:
	xavix_rom_24c08_control1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device_t implementation
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};

class xavix_rom_24c04_control0_device : public xavix_rom_i2c_base_device
{
public:
	xavix_rom_24c04_control0_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device_t implementation
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};

class xavix_rom_24c02_control0_device : public xavix_rom_i2c_base_device
{
public:
	xavix_rom_24c02_control0_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device_t implementation
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};

class xavix_rom_i2c_direct_device : public xavix_rom_plain_device
{
public:
	// device_xavix_cart_interface implementation
	virtual void write_sda(int state) override;
	virtual void write_scl(int state) override;
	virtual int read_sda() override;

protected:
	xavix_rom_i2c_direct_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	required_device<i2cmem_device> m_i2cmem;
};

class xavix_rom_24c02_direct_device : public xavix_rom_i2c_direct_device
{
public:
	xavix_rom_24c02_direct_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device_t implementation
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};

class xavix_rom_24c08_direct_device : public xavix_rom_i2c_direct_device
{
public:
	xavix_rom_24c08_direct_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device_t implementation
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};

DECLARE_DEVICE_TYPE(XAVIX_ROM_PLAIN,              xavix_rom_plain_device)
DECLARE_DEVICE_TYPE(XAVIX_ROM_24C04_CONTROL1,     xavix_rom_24c04_control1_device)
DECLARE_DEVICE_TYPE(XAVIX_ROM_24C08_CONTROL1,     xavix_rom_24c08_control1_device)
DECLARE_DEVICE_TYPE(XAVIX_ROM_24C04_CONTROL0,     xavix_rom_24c04_control0_device)
DECLARE_DEVICE_TYPE(XAVIX_ROM_24C02_CONTROL0,     xavix_rom_24c02_control0_device)
DECLARE_DEVICE_TYPE(XAVIX_ROM_24C02_DIRECT,       xavix_rom_24c02_direct_device)
DECLARE_DEVICE_TYPE(XAVIX_ROM_24C08_DIRECT,       xavix_rom_24c08_direct_device)

#endif // MAME_BUS_XAVIX_CART_ROM_H
