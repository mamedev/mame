// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_MONONCOL_ROM_H
#define MAME_BUS_MONONCOL_ROM_H

#pragma once

#include "slot.h"


// ======================> mononcol_rom_device

class mononcol_rom_device : public device_t, public device_mononcol_cart_interface
{
protected:
	// construction/destruction
	mononcol_rom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override { }
};


// ======================> mononcol_rom_plain_device

class mononcol_rom_plain_device : public mononcol_rom_device
{
public:
	// construction/destruction
	mononcol_rom_plain_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint8_t read_rom(offs_t offset) override;
	virtual uint16_t read16_rom(offs_t offset, uint16_t mem_mask) override;
	virtual uint32_t read32_rom(offs_t offset, uint32_t mem_mask) override;

protected:
	mononcol_rom_plain_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
};

// device type definition
DECLARE_DEVICE_TYPE(MONONCOL_ROM_PLAIN,    mononcol_rom_plain_device)


#endif // MAME_BUS_MONONCOL_ROM_H
