// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef MAME_BUS_MSX_CART_RTYPE_H
#define MAME_BUS_MSX_CART_RTYPE_H

#pragma once

#include "bus/msx_cart/cartridge.h"


DECLARE_DEVICE_TYPE(MSX_CART_RTYPE, msx_cart_rtype_device)


class msx_cart_rtype_device : public device_t, public msx_cart_interface
{
public:
	msx_cart_rtype_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

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
	uint8_t m_selected_bank;
	uint8_t *m_bank_base[2];
};


#endif // MAME_BUS_MSX_CART_RTYPE_H
