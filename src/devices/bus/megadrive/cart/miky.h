// license: BSD-3-Clause
// copyright-holders: Angelo Salese

#ifndef MAME_BUS_MEGADRIVE_CART_MIKY_H
#define MAME_BUS_MEGADRIVE_CART_MIKY_H

#pragma once

#include "rom.h"
#include "sram.h"
#include "slot.h"

class megadrive_unl_tc2000_device : public megadrive_rom_device
{
public:
	megadrive_unl_tc2000_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void cart_map(address_map &map) override ATTR_COLD;
protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
private:
	u8 m_prot_latch;
};

class megadrive_unl_futbol_arg96_device : public megadrive_rom_tplay96_device
{
public:
	megadrive_unl_futbol_arg96_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void cart_map(address_map &map) override ATTR_COLD;

protected:
	virtual u16 get_nvram_length() override ATTR_COLD;

};


DECLARE_DEVICE_TYPE(MEGADRIVE_UNL_TC2000,        megadrive_unl_tc2000_device)
DECLARE_DEVICE_TYPE(MEGADRIVE_UNL_FUTBOL_ARG96,  megadrive_unl_futbol_arg96_device)


#endif // MAME_BUS_MEGADRIVE_CART_TEKKENSP_H
