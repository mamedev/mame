// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_MEGADRIVE_SK_H
#define MAME_BUS_MEGADRIVE_SK_H

#pragma once

#include "md_slot.h"


// ======================> md_rom_sk_device

class md_rom_sk_device : public device_t,
						public device_md_cart_interface
{
public:
	// construction/destruction
	md_rom_sk_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	md_rom_sk_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// reading and writing
	virtual uint16_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;
	virtual uint16_t read_a13(offs_t offset) override;
	virtual void write_a13(offs_t offset, uint16_t data) override;

private:
	required_device<md_cart_slot_device> m_exp;
	bool m_map_upper;
};


// device type definition
DECLARE_DEVICE_TYPE(MD_ROM_SK, md_rom_sk_device)

#endif // MAME_BUS_MEGADRIVE_SK_H
