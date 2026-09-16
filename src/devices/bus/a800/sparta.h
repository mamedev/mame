// license:BSD-3-Clause
// copyright-holders:Fabio Priuli, Angelo Salese
#ifndef MAME_BUS_A800_SPARTA_H
#define MAME_BUS_A800_SPARTA_H

#pragma once

#include "rom.h"
#include "a800_slot.h"

class a800_rom_spartados_device : public a800_rom_device
{
public:
	a800_rom_spartados_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
	a800_rom_spartados_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void cart_map(address_map &map) override ATTR_COLD;
	virtual void cctl_map(address_map &map) override ATTR_COLD;
	virtual std::tuple<int, int> get_initial_rd_state() override { return std::make_tuple(0, 1); }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void bank_config_access(offs_t offset);

	int m_bank;

	u8 rom_bank_r(offs_t offset);
	void rom_bank_w(offs_t offset, u8 data);

	u8 subslot_r(offs_t offset);
	void subslot_w(offs_t offset, u8 data);
private:
	required_device<a800_cart_slot_device> m_subcart;
	memory_view m_cart_view;

	void subslot_config_access(offs_t offset);
	void subcart_rd4_w( int state );
	void subcart_rd5_w( int state );

	bool m_subcart_enabled;
	int m_subcart_rd4_enabled = 0, m_subcart_rd5_enabled = 0;
};

class a800_rom_spartados_128kb_device : public a800_rom_spartados_device
{
public:
	a800_rom_spartados_128kb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void cctl_map(address_map &map) override ATTR_COLD;

protected:
	virtual void bank_config_access(offs_t offset) override;
};

DECLARE_DEVICE_TYPE(A800_ROM_SPARTADOS,       a800_rom_spartados_device)
DECLARE_DEVICE_TYPE(A800_ROM_SPARTADOS_128KB, a800_rom_spartados_128kb_device)


#endif // MAME_BUS_A800_SPARTA_H
