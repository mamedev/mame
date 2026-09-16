// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
#ifndef MAME_BUS_Z88_FLASH_H
#define MAME_BUS_Z88_FLASH_H

#pragma once

#include "z88.h"
#include "machine/intelfsh.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> z88_1024k_flash_device

class z88_1024k_flash_device : public device_t,
								public device_z88cart_interface
{
public:
	// construction/destruction
	z88_1024k_flash_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	// z88cart_interface overrides
	virtual uint8_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint8_t data) override;
	virtual uint8_t* get_cart_base() override;
	virtual uint32_t get_cart_size() override { return 0x100000; }

private:
	required_device<intelfsh8_device> m_flash;
	required_memory_region   m_region;
};

// device type definition
DECLARE_DEVICE_TYPE(Z88_1024K_FLASH, z88_1024k_flash_device)

#endif // MAME_BUS_Z88_FLASH_H
