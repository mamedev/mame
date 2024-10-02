// license: BSD-3-Clause
// copyright-holders: Angelo Salese

#ifndef MAME_BUS_A5200_SUPERCART_H
#define MAME_BUS_A5200_SUPERCART_H

#pragma once

#include "rom.h"


class a5200_rom_supercart_device : public a5200_rom_device
{
public:
	a5200_rom_supercart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void cart_map(address_map &map) override ATTR_COLD;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	u8 m_bank;
	u8 m_bank_mask;

	u8 bank_r(offs_t offset);
};

DECLARE_DEVICE_TYPE(A5200_ROM_SUPERCART,       a5200_rom_supercart_device)

#endif // MAME_BUS_A5200_SUPERCART_H

