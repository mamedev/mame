// license: BSD-3-Clause
// copyright-holders: Angelo Salese

#ifndef MAME_BUS_A800_PHOENIX_H
#define MAME_BUS_A800_PHOENIX_H

#pragma once

#include "rom.h"

class a800_rom_phoenix_device : public a800_rom_device
{
public:
	a800_rom_phoenix_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
	a800_rom_phoenix_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void cart_map(address_map &map) override;
	virtual void cctl_map(address_map &map) override;

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	u32 m_rom_mask;
	virtual u8 disable_rom_r(offs_t offset);
	virtual void disable_rom_w(offs_t offset, u8 data);
};

class a800_rom_blizzard_16kb_device : public a800_rom_phoenix_device
{
public:
	a800_rom_blizzard_16kb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void cart_map(address_map &map) override;

protected:
	virtual void device_reset() override;

	virtual u8 disable_rom_r(offs_t offset) override;
	virtual void disable_rom_w(offs_t offset, u8 data) override;
};



DECLARE_DEVICE_TYPE(A800_ROM_PHOENIX,         a800_rom_phoenix_device)
DECLARE_DEVICE_TYPE(A800_ROM_BLIZZARD_16KB,   a800_rom_blizzard_16kb_device)


#endif // MAME_BUS_A800_PHOENIX_H
