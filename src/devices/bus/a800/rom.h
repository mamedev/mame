// license:BSD-3-Clause
// copyright-holders:Fabio Priuli, Angelo Salese

#ifndef MAME_BUS_A800_ROM_H
#define MAME_BUS_A800_ROM_H

#pragma once

#include "a800_slot.h"
#include "machine/nvram.h"


class a800_rom_device : public device_t,
						public device_a800_cart_interface
{
public:
	a800_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void cart_map(address_map &map) override ATTR_COLD;
	virtual std::tuple<int, int> get_initial_rd_state() override { return std::make_tuple(0, 1); }

protected:
	a800_rom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
};

class a800_rom_right_device : public a800_rom_device
{
public:
	a800_rom_right_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void cart_map(address_map &map) override ATTR_COLD;
	virtual std::tuple<int, int> get_initial_rd_state() override { return std::make_tuple(1, 0); }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
};

class a800_rom_16kb_device : public a800_rom_device
{
public:
	a800_rom_16kb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void cart_map(address_map &map) override ATTR_COLD;
	virtual std::tuple<int, int> get_initial_rd_state() override { return std::make_tuple(1, 1); }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
};


class xegs_rom_device : public a800_rom_device
{
public:
	xegs_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void cart_map(address_map &map) override ATTR_COLD;
	virtual void cctl_map(address_map &map) override ATTR_COLD;
	virtual std::tuple<int, int> get_initial_rd_state() override { return std::make_tuple(1, 1); }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	int m_bank;
};

class a5200_rom_device : public device_t,
						public device_a5200_cart_interface
{
public:
	a5200_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void cart_map(address_map &map) override ATTR_COLD;

protected:
	a5200_rom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
};

class a5200_rom_2chips_device : public a5200_rom_device
{
public:
	a5200_rom_2chips_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void cart_map(address_map &map) override ATTR_COLD;
};

DECLARE_DEVICE_TYPE(A800_ROM,             a800_rom_device)
DECLARE_DEVICE_TYPE(A800_ROM_RIGHT,       a800_rom_right_device)
DECLARE_DEVICE_TYPE(A800_ROM_16KB,        a800_rom_16kb_device)
DECLARE_DEVICE_TYPE(XEGS_ROM,             xegs_rom_device)
DECLARE_DEVICE_TYPE(A5200_ROM,            a5200_rom_device)
DECLARE_DEVICE_TYPE(A5200_ROM_2CHIPS,     a5200_rom_2chips_device)


#endif // MAME_BUS_A800_ROM_H
