// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef MAME_BUS_MSX_CART_KANJI_H
#define MAME_BUS_MSX_CART_KANJI_H

#pragma once

#include "bus/msx_cart/cartridge.h"


DECLARE_DEVICE_TYPE(MSX_CART_HXM200, msx_cart_kanji_hxm200_device)


class msx_cart_kanji_hxm200_device : public device_t, public msx_cart_interface
{
public:
	msx_cart_kanji_hxm200_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual const tiny_rom_entry *device_rom_region() const override;

private:
	u8 kanji_r(offs_t offset);
	void kanji_w(offs_t offset, u8 data);

	required_memory_region m_region_kanji;
	u32 m_kanji_address;
};


#endif // MAME_BUS_MSX_CART_KANJI_H
