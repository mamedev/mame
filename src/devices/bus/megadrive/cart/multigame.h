// license: BSD-3-Clause
// copyright-holders: Angelo Salese

#ifndef MAME_BUS_MEGADRIVE_CART_MULTIGAME_H
#define MAME_BUS_MEGADRIVE_CART_MULTIGAME_H

#pragma once

#include "rom.h"
#include "slot.h"

class megadrive_tectoy_sports_device : public megadrive_rom_device
{
public:
	megadrive_tectoy_sports_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void cart_map(address_map &map) override ATTR_COLD;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	u8 m_game_sel;
};

class megadrive_cm2in1_device : public megadrive_rom_device
{
public:
	megadrive_cm2in1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void cart_map(address_map &map) override ATTR_COLD;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	u8 m_game_sel;
};

DECLARE_DEVICE_TYPE(MEGADRIVE_TECTOY_SPORTS, megadrive_tectoy_sports_device)
DECLARE_DEVICE_TYPE(MEGADRIVE_CM2IN1, megadrive_cm2in1_device)


#endif // MAME_BUS_MEGADRIVE_CART_MULTIGAME_H
