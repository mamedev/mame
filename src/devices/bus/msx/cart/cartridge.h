// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef MAME_BUS_MSX_CART_CARTRIDGE_H
#define MAME_BUS_MSX_CART_CARTRIDGE_H

#pragma once

#include "bus/msx/slot/cartridge.h"


void msx_cart(device_slot_interface &device, bool is_in_subslot);


DECLARE_DEVICE_TYPE(MSX_SLOT_CARTRIDGE, msx_slot_cartridge_device)


class msx_slot_cartridge_device : public msx_slot_cartridge_base_device
{
public:
	msx_slot_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual std::string get_default_card_software(get_default_card_software_hook &hook) const override;

protected:
	static char const *const get_cart_type(const u8 *rom, u32 length);
};

#endif // MAME_BUS_MSX_CART_CARTRIDGE_H
