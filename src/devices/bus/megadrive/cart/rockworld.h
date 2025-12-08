// license: BSD-3-Clause
// copyright-holders: Angelo Salese

#ifndef MAME_BUS_MEGADRIVE_CART_ROCKWORLD_H
#define MAME_BUS_MEGADRIVE_CART_ROCKWORLD_H

#pragma once

#include "rom.h"
#include "slot.h"

class megadrive_unl_rockheaven_device : public megadrive_rom_device
{
public:
	megadrive_unl_rockheaven_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void cart_map(address_map &map) override ATTR_COLD;
};

class megadrive_unl_rockworld_device : public megadrive_rom_device
{
public:
	megadrive_unl_rockworld_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void cart_map(address_map &map) override ATTR_COLD;
};

DECLARE_DEVICE_TYPE(MEGADRIVE_UNL_ROCKHEAVEN,   megadrive_unl_rockheaven_device)
DECLARE_DEVICE_TYPE(MEGADRIVE_UNL_ROCKWORLD,    megadrive_unl_rockworld_device)

#endif // MAME_BUS_MEGADRIVE_CART_ROCKWORLD_H
