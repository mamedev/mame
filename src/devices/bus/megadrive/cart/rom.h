// license: BSD-3-Clause
// copyright-holders: Angelo Salese

#ifndef MAME_BUS_MEGADRIVE_CART_ROM_H
#define MAME_BUS_MEGADRIVE_CART_ROM_H

#pragma once

#include "slot.h"
//#include "machine/nvram.h"

class megadrive_rom_device : public device_t,
							 public device_megadrive_cart_interface
{
public:
	megadrive_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void cart_map(address_map &map) override ATTR_COLD;

protected:
	megadrive_rom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual std::error_condition load() override ATTR_COLD;

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

//  bool check_rom(std::string &message) ATTR_COLD;
	memory_bank_creator m_rom;

	u32 m_rom_mask;
	u32 m_rom_mirror;

//  void install_rom() ATTR_COLD;
};

class megadrive_rom_ssf2_device : public device_t,
								  public device_megadrive_cart_interface
{
public:
	megadrive_rom_ssf2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void cart_map(address_map &map) override ATTR_COLD;
	virtual void time_io_map(address_map &map) override ATTR_COLD;

	void cart_bank_map(address_map &map);
protected:
	megadrive_rom_ssf2_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	memory_bank_array_creator<8> m_rom_bank;
	memory_view m_sram_view;
};

DECLARE_DEVICE_TYPE(MEGADRIVE_ROM,      megadrive_rom_device)
DECLARE_DEVICE_TYPE(MEGADRIVE_ROM_SSF2, megadrive_rom_ssf2_device)

#endif // MAME_BUS_MEGADRIVE_CART_ROM_H
