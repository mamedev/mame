// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_BUS_JAKKS_GAMEKEY_ROM_H
#define MAME_BUS_JAKKS_GAMEKEY_ROM_H

#pragma once

#include "slot.h"
#include "machine/i2cmem.h"

// ======================> jakks_gamekey_rom_plain_device

class jakks_gamekey_rom_plain_device : public device_t,
						public device_jakks_gamekey_interface
{
public:
	// construction/destruction
	jakks_gamekey_rom_plain_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual DECLARE_READ16_MEMBER(read_cart) override;
	virtual DECLARE_WRITE16_MEMBER(write_cart) override;

	virtual uint8_t read_cart_seeprom(void) override { return 1; };
	virtual DECLARE_WRITE16_MEMBER(write_cart_seeprom) override { };

	virtual READ16_MEMBER(read_rom);
	virtual WRITE16_MEMBER(write_rom);

protected:
	jakks_gamekey_rom_plain_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override { }
	virtual void device_reset() override { }
};

// ======================> jakks_gamekey_rom_i2c_base_device

class jakks_gamekey_rom_i2c_base_device : public jakks_gamekey_rom_plain_device
{
public:
	// construction/destruction
	jakks_gamekey_rom_i2c_base_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	jakks_gamekey_rom_i2c_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual READ16_MEMBER(read_rom) override;
	virtual WRITE16_MEMBER(write_rom) override;

	optional_device<i2cmem_device> m_i2cmem;

	virtual uint8_t read_cart_seeprom(void) override;
	virtual DECLARE_WRITE16_MEMBER(write_cart_seeprom) override;
};


// ======================> jakks_gamekey_rom_i2c_24lc04_device

class jakks_gamekey_rom_i2c_24lc04_device : public jakks_gamekey_rom_i2c_base_device
{
public:
	// construction/destruction
	jakks_gamekey_rom_i2c_24lc04_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override;
};

// device type definition
DECLARE_DEVICE_TYPE(JAKKS_GAMEKEY_ROM_PLAIN,       jakks_gamekey_rom_plain_device)
DECLARE_DEVICE_TYPE(JAKKS_GAMEKEY_ROM_I2C_BASE,    jakks_gamekey_rom_i2c_base_device)
DECLARE_DEVICE_TYPE(JAKKS_GAMEKEY_ROM_I2C_24LC04,  jakks_gamekey_rom_i2c_24lc04_device)

#endif // MAME_BUS_JAKKS_GAMEKEY_ROM_H
