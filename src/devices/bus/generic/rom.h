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
	virtual DECLARE_READ8_MEMBER(read_rom) override;
	virtual DECLARE_READ16_MEMBER(read16_rom) override;
	virtual DECLARE_READ32_MEMBER(read32_rom) override;

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
	virtual DECLARE_READ8_MEMBER(read_ram) override;
	virtual DECLARE_WRITE8_MEMBER(write_ram) override;
};


// ======================> generic_rom_linear_device

class generic_rom_linear_device : public generic_rom_device
{
public:
	// construction/destruction
	generic_rom_linear_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_rom) override;
	virtual DECLARE_READ16_MEMBER(read16_rom) override;
	virtual DECLARE_READ32_MEMBER(read32_rom) override;
};



// device type definition
DECLARE_DEVICE_TYPE(GENERIC_ROM_PLAIN,    generic_rom_plain_device)
DECLARE_DEVICE_TYPE(GENERIC_ROM_LINEAR,   generic_rom_linear_device)
DECLARE_DEVICE_TYPE(GENERIC_ROMRAM_PLAIN, generic_romram_plain_device)


#endif // MAME_BUS_GENERIC_ROM_H
