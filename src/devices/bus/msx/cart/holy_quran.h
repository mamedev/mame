// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef MAME_BUS_MSX_CART_HOLY_QURAN_H
#define MAME_BUS_MSX_CART_HOLY_QURAN_H

#pragma once

#include "bus/msx/slot/cartridge.h"


DECLARE_DEVICE_TYPE(MSX_CART_HOLY_QURAN, msx_cart_holy_quran_device)


class msx_cart_holy_quran_device : public device_t, public msx_cart_interface
{
public:
	msx_cart_holy_quran_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual image_init_result initialize_cartridge(std::string &message) override;

protected:
	// device-level overrides
	virtual void device_start() override { }
	virtual void device_reset() override;

private:
	static constexpr size_t BANK_SIZE = 0x2000;

	u8 read(offs_t offset);
	u8 read2(offs_t offset);
	template <int Bank> void bank_w(u8 data);

	memory_bank_array_creator<4> m_rombank;
	memory_view m_view1;
	memory_view m_view2;

	std::vector<u8> m_decrypted;
	u8 m_bank_mask;
};


#endif // MAME_BUS_MSX_CART_HOLY_QURAN_H
