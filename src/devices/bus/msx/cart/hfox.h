// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef MAME_BUS_MSX_CART_HFOX_H
#define MAME_BUS_MSX_CART_HFOX_H

#pragma once

#include "bus/msx/slot/cartridge.h"


DECLARE_DEVICE_TYPE(MSX_CART_HFOX, msx_cart_hfox_device)


class msx_cart_hfox_device : public device_t, public msx_cart_interface
{
public:
	msx_cart_hfox_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual image_init_result initialize_cartridge(std::string &message) override;

protected:
	// device-level overrides
	virtual void device_start() override { }
	virtual void device_reset() override;

private:
	template <int Bank> void bank_w(u8 data);

	memory_bank_array_creator<2> m_rombank;

	u8 m_bank_mask;
};


#endif // MAME_BUS_MSX_CART_HFOX_H
