// license: BSD-3-Clause
// copyright-holders: Angelo Salese

#ifndef MAME_BUS_MEGADRIVE_CART_AVARTISAN_H
#define MAME_BUS_MEGADRIVE_CART_AVARTISAN_H

#pragma once

#include "rom.h"
#include "slot.h"

class megadrive_unl_avartisan_device : public device_t,
									   public device_megadrive_cart_interface
{
public:
	megadrive_unl_avartisan_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void cart_map(address_map &map) override ATTR_COLD;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	memory_bank_array_creator<0x40> m_rom_bank;
	u8 m_bank_size;
	u8 m_bank_sel;
	bool m_lock_config;
};

DECLARE_DEVICE_TYPE(MEGADRIVE_UNL_AVARTISAN, megadrive_unl_avartisan_device)


#endif // MAME_BUS_MEGADRIVE_CART_AVARTISAN_H
