// license: BSD-3-Clause
// copyright-holders: Fabio Priuli, Angelo Salese

#ifndef MAME_BUS_A800_WILLIAMS_H
#define MAME_BUS_A800_WILLIAMS_H

#pragma once

#include "rom.h"

class a800_rom_williams_device : public a800_rom_device
{
public:
	a800_rom_williams_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
	a800_rom_williams_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void cart_map(address_map &map) override ATTR_COLD;
	virtual void cctl_map(address_map &map) override ATTR_COLD;
	virtual std::tuple<int, int> get_initial_rd_state() override { return std::make_tuple(0, 1); };

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	int m_bank;

	uint8_t disable_rom_r(offs_t offset);
	void disable_rom_w(offs_t offset, uint8_t data);
	virtual uint8_t rom_bank_r(offs_t offset);
	virtual void rom_bank_w(offs_t offset, uint8_t data);
};


class a800_rom_express_device : public a800_rom_williams_device
{
public:
	a800_rom_express_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
	a800_rom_express_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void cctl_map(address_map &map) override ATTR_COLD;

	virtual uint8_t rom_bank_r(offs_t offset) override;
	virtual void rom_bank_w(offs_t offset, uint8_t data) override;
};

class a800_rom_diamond_device : public a800_rom_express_device
{
public:
	a800_rom_diamond_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

private:
	virtual void cctl_map(address_map &map) override ATTR_COLD;
};

class a800_rom_turbo_device : public a800_rom_williams_device
{
public:
	a800_rom_turbo_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void cctl_map(address_map &map) override ATTR_COLD;
};

DECLARE_DEVICE_TYPE(A800_ROM_WILLIAMS,    a800_rom_williams_device)
DECLARE_DEVICE_TYPE(A800_ROM_EXPRESS,     a800_rom_express_device)
DECLARE_DEVICE_TYPE(A800_ROM_DIAMOND,     a800_rom_diamond_device)
DECLARE_DEVICE_TYPE(A800_ROM_TURBO,       a800_rom_turbo_device)

#endif // MAME_BUS_A800_WILLIAMS_H
