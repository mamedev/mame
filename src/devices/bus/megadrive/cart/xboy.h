// license: BSD-3-Clause
// copyright-holders: Angelo Salese

#ifndef MAME_BUS_MEGADRIVE_CART_XBOY_H
#define MAME_BUS_MEGADRIVE_CART_XBOY_H

#pragma once

#include "rom.h"
#include "slot.h"

class megadrive_unl_kof98_device : public megadrive_rom_device
{
public:
	megadrive_unl_kof98_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void cart_map(address_map &map) override ATTR_COLD;
};


class megadrive_unl_bugslife_device : public megadrive_rom_device
{
public:
	megadrive_unl_bugslife_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void time_io_map(address_map &map) override ATTR_COLD;
};

class megadrive_unl_pokemona_device : public megadrive_rom_device
{
public:
	megadrive_unl_pokemona_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void time_io_map(address_map &map) override ATTR_COLD;
};

class megadrive_unl_kof99_device : public megadrive_rom_device
{
public:
	megadrive_unl_kof99_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void time_io_map(address_map &map) override ATTR_COLD;
};

class megadrive_unl_topf_device : public megadrive_rom_device
{
public:
	megadrive_unl_topf_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void cart_map(address_map &map) override ATTR_COLD;
protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	void prot_latch_w(u8 data);
	void prot_mode_w(u8 data);
	u8 prot_data_r();
private:
	memory_bank_array_creator<32> m_rom_bank;
	memory_view m_rom_view;

	u8 m_prot_latch;
	u8 m_prot_mode;
};

DECLARE_DEVICE_TYPE(MEGADRIVE_UNL_KOF98,     megadrive_unl_kof98_device)
DECLARE_DEVICE_TYPE(MEGADRIVE_UNL_BUGSLIFE,  megadrive_unl_bugslife_device)
DECLARE_DEVICE_TYPE(MEGADRIVE_UNL_POKEMONA,  megadrive_unl_pokemona_device)
DECLARE_DEVICE_TYPE(MEGADRIVE_UNL_KOF99,     megadrive_unl_kof99_device)
DECLARE_DEVICE_TYPE(MEGADRIVE_UNL_TOPF,      megadrive_unl_topf_device)


#endif // MAME_BUS_MEGADRIVE_CART_XBOY_H
