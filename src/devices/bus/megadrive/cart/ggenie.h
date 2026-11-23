// license: BSD-3-Clause
// copyright-holders: Fabio Priuli, Damian Rogers

#ifndef MAME_BUS_MEGADRIVE_CART_GGENIE_H
#define MAME_BUS_MEGADRIVE_CART_GGENIE_H

#pragma once

#include "rom.h"
#include "slot.h"

class megadrive_ggenie_device : public megadrive_rom_device
{
public:
	megadrive_ggenie_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void cart_map(address_map &map) override ATTR_COLD;
	virtual void time_io_map(address_map &map) override ATTR_COLD;

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	required_device<megadrive_cart_slot_device> m_lockon_cart;
	memory_view m_gg_view;

	u16 m_regs[20];

	bool locked() const { return BIT(m_regs[0], 8); }
	u32 patch_address(u32 code) const { return ((m_regs[2 + code * 3] & 0xff) << 16) | m_regs[3 + code * 3]; }
	int patch_match(offs_t offset) const;
	void update_view();

	u16 regs_r(offs_t offset);
	void regs_w(offs_t offset, u16 data, u16 mem_mask);
	u16 cart_r(offs_t offset, u16 mem_mask);
	void cart_w(offs_t offset, u16 data, u16 mem_mask);
};

DECLARE_DEVICE_TYPE(MEGADRIVE_GGENIE, megadrive_ggenie_device)


#endif // MAME_BUS_MEGADRIVE_CART_GGENIE_H
