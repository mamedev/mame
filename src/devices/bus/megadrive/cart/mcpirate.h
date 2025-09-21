// license: BSD-3-Clause
// copyright-holders: Angelo Salese

#ifndef MAME_BUS_MEGADRIVE_CART_MCPIRATE_H
#define MAME_BUS_MEGADRIVE_CART_MCPIRATE_H

#pragma once

#include "slot.h"

class megadrive_mcpirate_device : public device_t,
								  public device_megadrive_cart_interface
{
public:
	megadrive_mcpirate_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void cart_map(address_map &map) override ATTR_COLD;
	virtual void time_io_map(address_map &map) override ATTR_COLD;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	memory_bank_array_creator<4> m_rom_bank;

	u8 m_bank_mask;
};

DECLARE_DEVICE_TYPE(MEGADRIVE_MCPIRATE, megadrive_mcpirate_device)


#endif // MAME_BUS_MEGADRIVE_CART_MCPIRATE_H
