// license:BSD-3-Clause
// copyright-holders:cam900
/***************************************************************************

    SNK NEO-ZMC Z80 bankswitch controller

***************************************************************************/
#ifndef MAME_MACHINE_NEO_ZMC_H
#define MAME_MACHINE_NEO_ZMC_H

#pragma once

#include "dirom.h"

class neo_zmc_device : public device_t, public device_rom_interface<22> // or 19?
{
public:
	// no clock pin
	neo_zmc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);
	~neo_zmc_device() { }

	template <unsigned Which> u8 banked_r(offs_t offset)
	{
		const u32 bank_shift = 11 + Which;
		return read_byte((u32(m_banksel[Which]) << bank_shift) | (offset & ((1 << bank_shift) - 1)));
	}

	u8 bank_r(offs_t offset);

	void bank_map(address_map &map) ATTR_COLD;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	// internal state
	u8 m_banksel[4];
};

DECLARE_DEVICE_TYPE(NEO_ZMC, neo_zmc_device)

#endif // MAME_MACHINE_NEO_ZMC_H
