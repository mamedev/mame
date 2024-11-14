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

	virtual void cart_map(address_map &map) override ATTR_COLD;
	virtual void cctl_map(address_map &map) override ATTR_COLD;
	virtual std::tuple<int, int> get_initial_rd_state() override { return std::make_tuple(0, 1); }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	u32 m_rom_mask;
	virtual u8 disable_rom_r(offs_t offset);
	virtual void disable_rom_w(offs_t offset, u8 data);
};

class a800_rom_blizzard_16kb_device : public a800_rom_phoenix_device
{
public:
	a800_rom_blizzard_16kb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void cart_map(address_map &map) override ATTR_COLD;
	virtual std::tuple<int, int> get_initial_rd_state() override { return std::make_tuple(1, 1); }

protected:
	virtual void device_reset() override ATTR_COLD;

	virtual u8 disable_rom_r(offs_t offset) override;
	virtual void disable_rom_w(offs_t offset, u8 data) override;
};

class a800_rom_phoenix_ast2k_device : public a800_rom_phoenix_device
{
	public:
	a800_rom_phoenix_ast2k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void cart_map(address_map &map) override ATTR_COLD;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	required_ioport m_dsw;
	u32 m_rom_select;

	emu_timer *m_rd5_disarm_timer;

	TIMER_CALLBACK_MEMBER(rd5_disarm_cb);
};

DECLARE_DEVICE_TYPE(A800_ROM_PHOENIX,         a800_rom_phoenix_device)
DECLARE_DEVICE_TYPE(A800_ROM_BLIZZARD_16KB,   a800_rom_blizzard_16kb_device)
DECLARE_DEVICE_TYPE(A800_ROM_PHOENIX_AST2K,   a800_rom_phoenix_ast2k_device)

#endif // MAME_BUS_A800_PHOENIX_H
