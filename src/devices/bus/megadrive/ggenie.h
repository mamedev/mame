// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_MEGADRIVE_GGENIE_H
#define MAME_BUS_MEGADRIVE_GGENIE_H

#pragma once

#include "md_slot.h"


// ======================> md_rom_ggenie_device

class md_rom_ggenie_device : public device_t,
						public device_md_cart_interface
{
public:
	// construction/destruction
	md_rom_ggenie_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// reading and writing
	virtual uint16_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;

private:
	required_device<md_cart_slot_device> m_exp;
	uint16_t m_gg_regs[0x20];
	int m_gg_bypass;
	int m_reg_enable;
	uint16_t m_gg_addr[6];
	uint16_t m_gg_data[6];
};


// device type definition
DECLARE_DEVICE_TYPE(MD_ROM_GAMEGENIE, md_rom_ggenie_device)

#endif // MAME_BUS_MEGADRIVE_GGENIE_H
