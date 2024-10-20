// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_VECTREX_ROM_H
#define MAME_BUS_VECTREX_ROM_H

#pragma once

#include "slot.h"


// ======================> vectrex_rom_device

class vectrex_rom_device : public device_t,
						public device_vectrex_cart_interface
{
public:
	// construction/destruction
	vectrex_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint8_t read_rom(offs_t offset) override;

protected:
	vectrex_rom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override { }
	virtual void device_reset() override { }
};

// ======================> vectrex_rom64k_device

class vectrex_rom64k_device : public vectrex_rom_device
{
public:
	// construction/destruction
	vectrex_rom64k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint8_t read_rom(offs_t offset) override;
	virtual void write_bank(uint8_t data) override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	int m_bank;
};

// ======================> vectrex_sram_device

class vectrex_sram_device : public vectrex_rom_device
{
public:
	// construction/destruction
	vectrex_sram_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual void write_ram(offs_t offset, uint8_t data) override;
};



// device type definition
DECLARE_DEVICE_TYPE(VECTREX_ROM_STD,  vectrex_rom_device)
DECLARE_DEVICE_TYPE(VECTREX_ROM_64K,  vectrex_rom64k_device)
DECLARE_DEVICE_TYPE(VECTREX_ROM_SRAM, vectrex_sram_device)

#endif // MAME_BUS_VECTREX_ROM_H
