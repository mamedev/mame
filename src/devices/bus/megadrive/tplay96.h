// license:BSD-3-Clause
// copyright-holders:Angelo Salese
#ifndef MAME_BUS_MEGADRIVE_TPLAY96_H
#define MAME_BUS_MEGADRIVE_TPLAY96_H

#pragma once

#include "machine/nvram.h"

#include "md_slot.h"
#include "rom.h"

class md_rom_tplay96_device : public device_t,
						      public device_md_cart_interface
{
public:
	md_rom_tplay96_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
//	md_rom_titan_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual uint16_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<nvram_device> m_nvram;

	std::unique_ptr<uint8_t[]> m_nvram_ptr;
};

DECLARE_DEVICE_TYPE(MD_ROM_TPLAY96, md_rom_tplay96_device)

#endif
