// license: BSD-3-Clause
// copyright-holders: Fabio Priuli, Damian Rogers

#ifndef MAME_BUS_MEGADRIVE_CART_SK_H
#define MAME_BUS_MEGADRIVE_CART_SK_H

#pragma once

#include "slot.h"

class megadrive_rom_sk_device : public device_t,
								public device_megadrive_cart_interface
{
public:
	megadrive_rom_sk_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void cart_map(address_map &map) override ATTR_COLD;
	virtual void time_io_map(address_map &map) override ATTR_COLD;

protected:
	virtual std::error_condition load() override ATTR_COLD;

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void device_start() override ATTR_COLD;

private:
	required_device<megadrive_cart_slot_device> m_lockon_cart;
	memory_bank_creator m_rom;
	memory_bank_creator m_patch_rom;
	memory_view m_patch_view;

	void time_w(offs_t offset, u16 data, u16 mem_mask);
};

DECLARE_DEVICE_TYPE(MEGADRIVE_ROM_SK, megadrive_rom_sk_device)


#endif // MAME_BUS_MEGADRIVE_CART_SK_H
