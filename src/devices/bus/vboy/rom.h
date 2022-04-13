// license:BSD-3-Clause
// copyright-holders:Vas Crabb
#ifndef MAME_BUS_VBOY_ROM_H
#define MAME_BUS_VBOY_ROM_H

#pragma once

#include "slot.h"


//**************************************************************************
//  CLASS DECLARATIONS
//**************************************************************************

class vboy_flat_rom_device : public device_t, public device_vboy_cart_interface
{
public:
	vboy_flat_rom_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	// device_vboy_cart_interface implementation
	virtual image_init_result load() override ATTR_COLD;

protected:
	vboy_flat_rom_device(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, u32 clock);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
};


class vboy_flat_rom_sram_device : public vboy_flat_rom_device
{
public:
	vboy_flat_rom_sram_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	// device_vboy_cart_interface implementation
	virtual image_init_result load() override ATTR_COLD;
	virtual void unload() override ATTR_COLD;
};



//**************************************************************************
//  DEVICE TYPE DECLARATIONS
//**************************************************************************

DECLARE_DEVICE_TYPE(VBOY_FLAT_ROM, vboy_flat_rom_device)
DECLARE_DEVICE_TYPE(VBOY_FLAT_ROM_SRAM, vboy_flat_rom_sram_device)

#endif // MAME_BUS_VBOY_ROM_H
