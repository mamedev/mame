// license: BSD-3-Clause
// copyright-holders: Angelo Salese

#ifndef MAME_BUS_MEGADRIVE_CART_TEKKENSP_H
#define MAME_BUS_MEGADRIVE_CART_TEKKENSP_H

#pragma once

#include "rom.h"
#include "slot.h"

class megadrive_unl_tekkensp_device : public megadrive_rom_device
{
public:
	megadrive_unl_tekkensp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void cart_map(address_map &map) override ATTR_COLD;
protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
private:
	template <unsigned N> void prot_shift_w(u8 data);
	u8 m_prot_latch;
};

DECLARE_DEVICE_TYPE(MEGADRIVE_UNL_TEKKENSP,     megadrive_unl_tekkensp_device)


#endif // MAME_BUS_MEGADRIVE_CART_TEKKENSP_H
