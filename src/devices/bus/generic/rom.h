// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_GENERIC_ROM_H
#define MAME_BUS_GENERIC_ROM_H

#pragma once

#include "slot.h"


// ======================> generic_rom_device

class generic_rom_device : public device_t, public device_generic_cart_interface
{
protected:
	// construction/destruction
	generic_rom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override { }
};


// ======================> generic_rom_plain_device

class generic_rom_plain_device : public generic_rom_device
{
public:
	// construction/destruction
	generic_rom_plain_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint8_t read_rom(offs_t offset) override;
	virtual uint16_t read16_rom(offs_t offset, uint16_t mem_mask) override;
	virtual uint32_t read32_rom(offs_t offset, uint32_t mem_mask) override;

protected:
	generic_rom_plain_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
};


// ======================> generic_romram_plain_device

class generic_romram_plain_device : public generic_rom_plain_device
{
public:
	// construction/destruction
	generic_romram_plain_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint8_t read_ram(offs_t offset) override;
	virtual void write_ram(offs_t offset, uint8_t data) override;
};


// ======================> generic_rom_linear_device

class generic_rom_linear_device : public generic_rom_device
{
public:
	// construction/destruction
	generic_rom_linear_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint8_t read_rom(offs_t offset) override;
	virtual uint16_t read16_rom(offs_t offset, uint16_t mem_mask) override;
	virtual uint32_t read32_rom(offs_t offset, uint32_t mem_mask) override;
};



// device type definition
DECLARE_DEVICE_TYPE(GENERIC_ROM_PLAIN,    generic_rom_plain_device)
DECLARE_DEVICE_TYPE(GENERIC_ROM_LINEAR,   generic_rom_linear_device)
DECLARE_DEVICE_TYPE(GENERIC_ROMRAM_PLAIN, generic_romram_plain_device)


#endif // MAME_BUS_GENERIC_ROM_H
