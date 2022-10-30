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

	virtual void cart_map(address_map &map) override;
	virtual void cctl_map(address_map &map) override;

private:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;

	required_device<nvram_device> m_nvram;
	std::unique_ptr<uint8_t[]> m_nvram_ptr;
};

#endif // MAME_BUS_A800_TELELINK2_H
