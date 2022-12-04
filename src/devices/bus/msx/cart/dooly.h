// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef MAME_BUS_MSX_CART_DOOLY_H
#define MAME_BUS_MSX_CART_DOOLY_H

#pragma once

#include "bus/msx/slot/cartridge.h"


DECLARE_DEVICE_TYPE(MSX_CART_DOOLY, msx_cart_dooly_device)


class msx_cart_dooly_device : public device_t, public msx_cart_interface
{
public:
	msx_cart_dooly_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual image_init_result initialize_cartridge(std::string &message) override;

protected:
	// device-level overrides
	virtual void device_start() override { }
	virtual void device_reset() override;

private:
	void prot_w(u8 data);
	u8 mode4_page1_r(offs_t offset);
	u8 mode4_page2_r(offs_t offset);

	memory_view m_view1;
	memory_view m_view2;
};


#endif // MAME_BUS_MSX_CART_DOOLY_H
