// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef MAME_BUS_MSX_CART_HALNOTE_H
#define MAME_BUS_MSX_CART_HALNOTE_H

#pragma once

#include "bus/msx/slot/cartridge.h"


DECLARE_DEVICE_TYPE(MSX_CART_HALNOTE, msx_cart_halnote_device)


class msx_cart_halnote_device : public device_t, public msx_cart_interface
{
public:
	msx_cart_halnote_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual image_init_result initialize_cartridge(std::string &message) override;

protected:
	// device-level overrides
	virtual void device_start() override { }
	virtual void device_reset() override;

private:
	static constexpr u8 BANK_MASK = (0x100000 / 0x2000) - 1;

	void bank0_w(u8 data);
	void bank1_w(u8 data);
	void bank2_w(u8 data);
	void bank3_w(u8 data);
	void bank4_w(u8 data);
	void bank5_w(u8 data);

	memory_bank_array_creator<6> m_rombank;
	memory_view m_view0;
	memory_view m_view1;
};


#endif // MAME_BUS_MSX_CART_HALNOTE_H
