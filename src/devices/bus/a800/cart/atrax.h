// license: BSD-3-Clause
// copyright-holders: Angelo Salese

#ifndef MAME_BUS_A800_ATRAX_H
#define MAME_BUS_A800_ATRAX_H

#pragma once

#include "rom.h"


class a800_rom_atrax_device : public a800_rom_device
{
public:
	a800_rom_atrax_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void cart_map(address_map &map) override ATTR_COLD;
	virtual void cctl_map(address_map &map) override ATTR_COLD;
	virtual std::tuple<int, int> get_initial_rd_state() override { return std::make_tuple(0, 1); }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	void config_bank_w(offs_t offset, u8 data);

private:
	int m_bank;
};


DECLARE_DEVICE_TYPE(A800_ROM_ATRAX,        a800_rom_atrax_device)

#endif // MAME_BUS_A800_ATRAX_H
