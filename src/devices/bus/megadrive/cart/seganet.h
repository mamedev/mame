// license: BSD-3-Clause
// copyright-holders: Angelo Salese

#ifndef MAME_BUS_MEGADRIVE_CART_SEGANET_H
#define MAME_BUS_MEGADRIVE_CART_SEGANET_H

#pragma once

#include "slot.h"

class megadrive_seganet_device : public device_t,
						         public device_megadrive_cart_interface
{
public:
	megadrive_seganet_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void cart_map(address_map &map) override ATTR_COLD;
	virtual void time_io_map(address_map &map) override ATTR_COLD;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	memory_bank_creator m_rom;
	memory_view m_ram_view;
};

DECLARE_DEVICE_TYPE(MEGADRIVE_SEGANET, megadrive_seganet_device)


#endif // MAME_BUS_MEGADRIVE_CART_SEGANET_H
