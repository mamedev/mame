// license: BSD-3-Clause
// copyright-holders: Angelo Salese

#ifndef MAME_BUS_MEGADRIVE_CART_T5740_H
#define MAME_BUS_MEGADRIVE_CART_T5740_H

#pragma once

#include "machine/m95320.h"

#include "slot.h"

class megadrive_hb_psolar_device : public device_t
								 , public device_megadrive_cart_interface
{
public:
	megadrive_hb_psolar_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void cart_map(address_map &map) override ATTR_COLD;
	virtual void time_io_map(address_map &map) override ATTR_COLD;
protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
private:
	memory_bank_array_creator<8> m_rom_bank;
	required_device<m95320_eeprom_device> m_spi_eeprom;
};

DECLARE_DEVICE_TYPE(MEGADRIVE_HB_PSOLAR, megadrive_hb_psolar_device)


#endif // MAME_BUS_MEGADRIVE_CART_T5740_H
