// license: BSD-3-Clause
// copyright-holders: Fabio Priuli, Angelo Salese

#ifndef MAME_BUS_A800_BBSB_H
#define MAME_BUS_A800_BBSB_H

#pragma once

#include "rom.h"


class a800_rom_bbsb_device : public a800_rom_device
{
public:
	a800_rom_bbsb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void cart_map(address_map &map) override ATTR_COLD;
	virtual std::tuple<int, int> get_initial_rd_state() override { return std::make_tuple(1, 1); }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	int m_banks[2];

	template <unsigned BankNum> u8 read_bank(offs_t offset);
	u8 bank_r(offs_t offset);
	void bank_w(offs_t offset, u8 data);
};

class a5200_rom_bbsb_device : public a5200_rom_device
{
public:
	a5200_rom_bbsb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void cart_map(address_map &map) override ATTR_COLD;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	int m_banks[2];

	template <unsigned BankNum> u8 read_bank(offs_t offset);
	u8 bank_r(offs_t offset);
	void bank_w(offs_t offset, u8 data);
};

DECLARE_DEVICE_TYPE(A800_ROM_BBSB,        a800_rom_bbsb_device)
DECLARE_DEVICE_TYPE(A5200_ROM_BBSB,       a5200_rom_bbsb_device)

#endif // MAME_BUS_A800_BBSB_H
