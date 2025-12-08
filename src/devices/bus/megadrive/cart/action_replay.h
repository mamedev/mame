// license: BSD-3-Clause
// copyright-holders: Angelo Salese

#ifndef MAME_BUS_MEGADRIVE_CART_ACTION_REPLAY_H
#define MAME_BUS_MEGADRIVE_CART_ACTION_REPLAY_H

#pragma once

#include "rom.h"
#include "slot.h"

class megadrive_action_replay_device : public megadrive_rom_device
{
public:
	megadrive_action_replay_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void cart_map(address_map &map) override ATTR_COLD;
	virtual void time_io_map(address_map &map) override ATTR_COLD;

protected:
	megadrive_action_replay_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	required_device<megadrive_cart_slot_device> m_lockon_cart;
	memory_view m_ar_view;

	void unlock_cart_w(offs_t offset, u16 data, u16 mem_mask);
};

DECLARE_DEVICE_TYPE(MEGADRIVE_ACTION_REPLAY, megadrive_action_replay_device)


#endif // MAME_BUS_MEGADRIVE_CART_ACTION_REPLAY_H
