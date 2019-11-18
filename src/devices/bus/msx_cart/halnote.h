// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef MAME_BUS_MSX_CART_HALNOTE_H
#define MAME_BUS_MSX_CART_HALNOTE_H

#pragma once

#include "bus/msx_cart/cartridge.h"


DECLARE_DEVICE_TYPE(MSX_CART_HALNOTE, msx_cart_halnote_device)


class msx_cart_halnote_device : public device_t, public msx_cart_interface
{
public:
	msx_cart_halnote_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void initialize_cartridge() override;

	virtual uint8_t read_cart(offs_t offset) override;
	virtual void write_cart(offs_t offset, uint8_t data) override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_post_load() override;

	void restore_banks();

private:
	uint8_t m_selected_bank[8];
	uint8_t *m_bank_base[8];

	void map_bank(int bank);
};


#endif // MAME_BUS_MSX_CART_HALNOTE_H
