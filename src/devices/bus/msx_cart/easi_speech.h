// license:BSD-3-Clause
// copyright-holders:hap
#ifndef MAME_BUS_MSX_CART_EASISPEECH_H
#define MAME_BUS_MSX_CART_EASISPEECH_H

#pragma once

#include "bus/msx_cart/cartridge.h"
#include "sound/sp0256.h"


DECLARE_DEVICE_TYPE(MSX_CART_EASISPEECH, msx_cart_easispeech_device)


class msx_cart_easispeech_device : public device_t, public msx_cart_interface
{
public:
	msx_cart_easispeech_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read_cart(offs_t offset) override;
	virtual void write_cart(offs_t offset, uint8_t data) override;

protected:
	// device-level overrides
	virtual void device_start() override { ; }
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

private:
	required_device<sp0256_device> m_speech;
};

#endif // MAME_BUS_MSX_CART_EASISPEECH_H
