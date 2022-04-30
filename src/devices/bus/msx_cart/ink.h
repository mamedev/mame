// license:BSD-3-Clause
// copyright-holders:hap
#ifndef MAME_BUS_MSX_CART_INK_H
#define MAME_BUS_MSX_CART_INK_H

#pragma once

#include "bus/msx_cart/cartridge.h"
#include "machine/intelfsh.h"


DECLARE_DEVICE_TYPE(MSX_CART_INK, msx_cart_ink_device)


class msx_cart_ink_device : public device_t, public msx_cart_interface
{
public:
	msx_cart_ink_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void initialize_cartridge() override;

	virtual uint8_t read_cart(offs_t offset) override;
	virtual void write_cart(offs_t offset, uint8_t data) override;

protected:
	// device-level overrides
	virtual void device_start() override { ; }
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

private:
	required_device<amd_29f040_device> m_flash;
};


#endif // MAME_BUS_MSX_CART_INK_H
