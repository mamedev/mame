// license: BSD-3-Clause
// copyright-holders: Angelo Salese

#ifndef MAME_BUS_MEGADRIVE_CART_SEGACH_H
#define MAME_BUS_MEGADRIVE_CART_SEGACH_H

#pragma once

#include "slot.h"

class megadrive_segach_jp_device : public device_t,
								 public device_megadrive_cart_interface
{
public:
	megadrive_segach_jp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void cart_map(address_map &map) override ATTR_COLD;
	virtual void time_io_map(address_map &map) override ATTR_COLD;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	memory_bank_creator m_rom;
	memory_view m_ram_view;
	std::vector<u16> m_ram;
};

DECLARE_DEVICE_TYPE(MEGADRIVE_SEGACH_JP, megadrive_segach_jp_device)


#endif // MAME_BUS_MEGADRIVE_CART_SEGACH_H
