// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood,Fabio Priuli
#ifndef MAME_BUS_NEOGEO_ROM_H
#define MAME_BUS_NEOGEO_ROM_H

#pragma once

#include "slot.h"
#include "machine/nvram.h"

// ======================> neogeo_rom_device

class neogeo_rom_device : public device_t, public device_neogeo_cart_interface
{
public:
	// construction/destruction
	neogeo_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint16_t clock);

	// reading and writing
	virtual uint16_t rom_r(offs_t offset) override;
	virtual void banksel_w(uint16_t data) override;

protected:
	neogeo_rom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint16_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
};



// device type definition
DECLARE_DEVICE_TYPE(NEOGEO_ROM, neogeo_rom_device)



/*************************************************
 vliner
 **************************************************/

class neogeo_vliner_cart_device : public neogeo_rom_device
{
public:
	neogeo_vliner_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint16_t ram_r(offs_t offset) override { return m_cart_ram[offset]; }
	virtual void ram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override { COMBINE_DATA(&m_cart_ram[offset]); }

	virtual int get_fixed_bank_type() override { return 0; }

protected:
	virtual void device_start() override ATTR_COLD;

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	std::unique_ptr<uint16_t[]> m_cart_ram;

	required_device<nvram_device> m_nvram;
};

DECLARE_DEVICE_TYPE(NEOGEO_VLINER_CART, neogeo_vliner_cart_device)


#endif // MAME_BUS_NEOGEO_ROM_H
