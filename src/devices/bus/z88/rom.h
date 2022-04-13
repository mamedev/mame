// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
#ifndef MAME_BUS_Z88_ROM_H
#define MAME_BUS_Z88_ROM_H

#pragma once

#include "z88.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> z88_32k_rom_device

class z88_32k_rom_device : public device_t,
							public device_z88cart_interface
{
public:
	// construction/destruction
	z88_32k_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	z88_32k_rom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;

	// z88cart_interface overrides
	virtual uint8_t read(offs_t offset) override;
	virtual uint8_t* get_cart_base() override;
	virtual uint32_t get_cart_size() override { return 0x8000; }

protected:
	// internal state
	uint8_t *     m_rom;
};

// ======================> z88_128k_rom_device

class z88_128k_rom_device : public z88_32k_rom_device
{
public:
	// construction/destruction
	z88_128k_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// z88cart_interface overrides
	virtual uint32_t get_cart_size() override { return 0x20000; }
};

// ======================> z88_256k_rom_device

class z88_256k_rom_device : public z88_32k_rom_device
{
public:
	// construction/destruction
	z88_256k_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// z88cart_interface overrides
	virtual uint32_t get_cart_size() override { return 0x200000; }
};

// device type definition
DECLARE_DEVICE_TYPE(Z88_32K_ROM,  z88_32k_rom_device)
DECLARE_DEVICE_TYPE(Z88_128K_ROM, z88_128k_rom_device)
DECLARE_DEVICE_TYPE(Z88_256K_ROM, z88_256k_rom_device)

#endif // MAME_BUS_Z88_ROM_H
