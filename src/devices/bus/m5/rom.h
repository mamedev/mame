// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_M5_ROM_H
#define MAME_BUS_M5_ROM_H

#pragma once

#include "slot.h"


// ======================> m5_rom_device

class m5_rom_device : public device_t,
						public device_m5_cart_interface
{
public:
	// construction/destruction
	m5_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override {}
	virtual void device_reset() override {}

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_rom) override;

protected:
	m5_rom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
};

// ======================> m5_ram_device

class m5_ram_device : public m5_rom_device
{
public:
	// construction/destruction
	m5_ram_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_ram) override;
	virtual DECLARE_WRITE8_MEMBER(write_ram) override;

protected:
	// device-level overrides
	virtual void device_start() override { }
	virtual void device_reset() override { }
};


// device type definition
DECLARE_DEVICE_TYPE(M5_ROM_STD, m5_rom_device)
DECLARE_DEVICE_TYPE(M5_ROM_RAM, m5_ram_device)


#endif // MAME_BUS_M5_ROM_H
