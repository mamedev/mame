// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef MAME_BUS_MSX_CART_MSXDOS2_H
#define MAME_BUS_MSX_CART_MSXDOS2_H

#pragma once

#include "bus/msx_cart/cartridge.h"


DECLARE_DEVICE_TYPE(MSX_CART_MSXDOS2, msx_cart_msxdos2_device)


class msx_cart_msxdos2_device : public device_t, public msx_cart_interface
{
public:
	msx_cart_msxdos2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void initialize_cartridge() override;

	virtual DECLARE_READ8_MEMBER(read_cart) override;
	virtual DECLARE_WRITE8_MEMBER(write_cart) override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	void restore_banks();

private:
	uint8_t m_selected_bank;
	uint8_t *m_bank_base;
};


#endif // MAME_BUS_MSX_CART_MSXDOS2_H
