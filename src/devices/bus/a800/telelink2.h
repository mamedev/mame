// license: BSD-3-Clause
// copyright-holders: Fabio Priuli, Angelo Salese

#ifndef MAME_BUS_A800_TELELINK2_H
#define MAME_BUS_A800_TELELINK2_H

#pragma once

#include "rom.h"

class a800_rom_telelink2_device : public a800_rom_device
{
public:
	// construction/destruction
	a800_rom_telelink2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void cart_map(address_map &map) override ATTR_COLD;
	virtual void cctl_map(address_map &map) override ATTR_COLD;

	// RD4 tied to +5V, assume always enabled
	virtual std::tuple<int, int> get_initial_rd_state() override { return std::make_tuple(1, 1); }

private:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	required_device<nvram_device> m_nvram;
	std::unique_ptr<uint8_t[]> m_nvram_ptr;
};

DECLARE_DEVICE_TYPE(A800_ROM_TELELINK2,   a800_rom_telelink2_device)

#endif // MAME_BUS_A800_TELELINK2_H
