// license:BSD-3-Clause
// copyright-holders:Angelo Salese
#ifndef MAME_BUS_MEGADRIVE_TITAN_H
#define MAME_BUS_MEGADRIVE_TITAN_H

#pragma once

#include "md_slot.h"

class md_rom_titan_device : public device_t,
							public device_md_cart_interface
{
public:
	md_rom_titan_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
//  md_rom_titan_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual uint16_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;
	virtual void write_a13(offs_t offset, uint16_t data) override;

private:
	u8 m_bank[8];
};

DECLARE_DEVICE_TYPE(MD_ROM_TITAN, md_rom_titan_device)

#endif
