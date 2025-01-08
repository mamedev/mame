// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_BUS_GAMATE_ROM_H
#define MAME_BUS_GAMATE_ROM_H

#pragma once

#include "slot.h"
#include "gamate_protection.h"

// ======================> gamate_rom_plain_device

class gamate_rom_plain_device : public device_t,
						public device_gamate_cart_interface
{
public:
	// construction/destruction
	gamate_rom_plain_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint8_t read_cart(offs_t offset) override;
	virtual void write_cart(offs_t offset, uint8_t data) override;
	virtual uint8_t read_rom(offs_t offset);
	virtual void write_rom(offs_t offset, uint8_t data);

protected:
	gamate_rom_plain_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device-level overrides
	virtual void device_start() override { }
	virtual void device_reset() override { }

	optional_device<gamate_protection_device> m_protection;
};

// ======================> gamate_rom_banked_device

class gamate_rom_banked_device : public gamate_rom_plain_device
{
public:
	// construction/destruction
	gamate_rom_banked_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	gamate_rom_banked_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint8_t read_rom(offs_t offset) override;
	virtual void write_rom(offs_t offset, uint8_t data) override;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	int m_bank;
};


// ======================> gamate_rom_4in1_device

class gamate_rom_4in1_device : public gamate_rom_banked_device
{
public:
	// construction/destruction
	gamate_rom_4in1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint8_t read_rom(offs_t offset) override;
	virtual void write_rom(offs_t offset, uint8_t data) override;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	int m_multibank;
};

// device type definition
DECLARE_DEVICE_TYPE(GAMATE_ROM_PLAIN,       gamate_rom_plain_device)
DECLARE_DEVICE_TYPE(GAMATE_ROM_BANKED,      gamate_rom_banked_device)
DECLARE_DEVICE_TYPE(GAMATE_ROM_4IN1,        gamate_rom_4in1_device)

#endif // MAME_BUS_GAMATE_ROM_H
