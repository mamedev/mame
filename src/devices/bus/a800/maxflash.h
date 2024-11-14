// license: BSD-3-Clause
// copyright-holders: Angelo Salese

#ifndef MAME_BUS_A800_MAXFLASH_H
#define MAME_BUS_A800_MAXFLASH_H

#pragma once

#include "rom.h"
#include "machine/intelfsh.h"

class a800_maxflash_128kb_device : public a800_rom_device
{
public:
	a800_maxflash_128kb_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
	a800_maxflash_128kb_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void cart_map(address_map &map) override ATTR_COLD;
	virtual void cctl_map(address_map &map) override ATTR_COLD;
	virtual std::tuple<int, int> get_initial_rd_state() override { return std::make_tuple(0, 1); }

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	required_device<intelfsh8_device> m_flash;

	uint8_t disable_rom_r(offs_t offset);
	void disable_rom_w(offs_t offset, uint8_t data);
	uint8_t rom_bank_r(offs_t offset);
	void rom_bank_w(offs_t offset, uint8_t data);

private:
	int m_bank;
};

class a800_maxflash_1mb_device : public a800_maxflash_128kb_device
{
public:
	a800_maxflash_1mb_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void cctl_map(address_map &map) override ATTR_COLD;

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};

DECLARE_DEVICE_TYPE(A800_MAXFLASH_128KB, a800_maxflash_128kb_device)
DECLARE_DEVICE_TYPE(A800_MAXFLASH_1MB, a800_maxflash_1mb_device)

#endif // MAME_BUS_A800_MAXFLASH_H
