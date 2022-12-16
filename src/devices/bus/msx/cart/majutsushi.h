// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef MAME_BUS_MSX_CART_MAJUTSUSHI_H
#define MAME_BUS_MSX_CART_MAJUTSUSHI_H

#pragma once

#include "bus/msx/slot/cartridge.h"
#include "sound/dac.h"


DECLARE_DEVICE_TYPE(MSX_CART_MAJUTSUSHI, msx_cart_majutsushi_device)


class msx_cart_majutsushi_device : public device_t, public msx_cart_interface
{
public:
	msx_cart_majutsushi_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual image_init_result initialize_cartridge(std::string &message) override;

protected:
	// device-level overrides
	virtual void device_start() override { }

	virtual void device_add_mconfig(machine_config &config) override;

private:
	template <int Bank> void bank_w(u8 data);

	required_device<dac_byte_interface> m_dac;
	memory_bank_array_creator<3> m_rombank;
};


#endif // MAME_BUS_MSX_CART_MAJUTSUSHI_H
