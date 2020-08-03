// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_ODYSSEY2_ROM_H
#define MAME_BUS_ODYSSEY2_ROM_H

#pragma once

#include "slot.h"


// ======================> o2_rom_device

class o2_rom_device : public device_t,
						public device_o2_cart_interface
{
public:
	// construction/destruction
	o2_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint8_t read_rom04(offs_t offset) override;
	virtual uint8_t read_rom0c(offs_t offset) override;

	virtual void write_p1(uint8_t data) override { m_bank_base = data & 3; }

protected:
	o2_rom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;

	int m_bank_base;
};

// ======================> o2_rom12_device

class o2_rom12_device : public o2_rom_device
{
public:
	// construction/destruction
	o2_rom12_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint8_t read_rom04(offs_t offset) override;
	virtual uint8_t read_rom0c(offs_t offset) override;
};

// ======================> o2_rom16_device

class o2_rom16_device : public o2_rom_device
{
public:
	// construction/destruction
	o2_rom16_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint8_t read_rom04(offs_t offset) override;
	virtual uint8_t read_rom0c(offs_t offset) override;
};


// device type definition
DECLARE_DEVICE_TYPE(O2_ROM_STD, o2_rom_device)
DECLARE_DEVICE_TYPE(O2_ROM_12K, o2_rom12_device)
DECLARE_DEVICE_TYPE(O2_ROM_16K, o2_rom16_device)

#endif // MAME_BUS_ODYSSEY2_ROM_H
