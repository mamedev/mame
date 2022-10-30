// license: BSD-3-Clause
// copyright-holders: Fabio Priuli, Angelo Salese

#ifndef MAME_BUS_A800_ULTRACART_H
#define MAME_BUS_A800_ULTRACART_H

#pragma once

#include "rom.h"

class a800_rom_ultracart_device : public a800_rom_device
{
public:
	// construction/destruction
	a800_rom_ultracart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void cart_map(address_map &map) override;
	virtual void cctl_map(address_map &map) override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	int m_bank;

	void binary_counter_access();
	u8 config_bank_r(offs_t offset);
	void config_bank_w(offs_t offset, u8 data);
};

DECLARE_DEVICE_TYPE(A800_ROM_ULTRACART,   a800_rom_ultracart_device)

#endif // MAME_BUS_A800_ULTRACART_H
