// license:BSD-3-Clause
// copyright-holders:bugjosh
#ifndef MAME_BUS_NEOGEO_LINK_H
#define MAME_BUS_NEOGEO_LINK_H

#pragma once

#include "rom.h"
#include "cpu/m6800/m6801.h"


/*************************************************
 link: cart with the HD6301V1 that runs the
 multi-play link (PROG-HERO)
 **************************************************/

class neogeo_link_cart_device : public neogeo_rom_device
{
public:
	neogeo_link_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint16_t protection_r(address_space &space, offs_t offset) override;
	virtual void protection_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;
	virtual void set_slot_selected(bool state) override { m_slot_selected = state; }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	TIMER_CALLBACK_MEMBER(to_mcu);
	int link_level() const;

	required_device<hd6301v1_cpu_device> m_mcu;
	required_memory_region m_mcu_rom;

	uint8_t m_to_mcu;
	uint8_t m_p1;
	uint8_t m_p4;
	bool m_slot_selected;
	uint8_t m_tx;
};

DECLARE_DEVICE_TYPE(NEOGEO_LINK_CART, neogeo_link_cart_device)


#endif // MAME_BUS_NEOGEO_LINK_H
