// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef MAME_BUS_MSX_CART_ARC_H
#define MAME_BUS_MSX_CART_ARC_H

#pragma once

#include "bus/msx/slot/cartridge.h"


DECLARE_DEVICE_TYPE(MSX_CART_ARC, msx_cart_arc_device)


class msx_cart_arc_device : public device_t, public msx_cart_interface
{
public:
	msx_cart_arc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual image_init_result initialize_cartridge(std::string &message) override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	void io_7f_w(u8 data);
	u8 io_7f_r();

	u8 m_7f;
};

#endif // MAME_BUS_MSX_CART_ARC_H
