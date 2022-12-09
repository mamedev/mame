// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef MAME_BUS_MSX_CART_KANJI_H
#define MAME_BUS_MSX_CART_KANJI_H

#pragma once

#include "bus/msx/slot/cartridge.h"


DECLARE_DEVICE_TYPE(MSX_CART_KANJI, msx_cart_kanji_device)
DECLARE_DEVICE_TYPE(MSX_CART_MSXWRITE, msx_cart_msxwrite_device)


class msx_cart_kanji_device : public device_t, public msx_cart_interface
{
public:
	msx_cart_kanji_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual image_init_result initialize_cartridge(std::string &message) override;

protected:
	msx_cart_kanji_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	image_init_result validate_kanji_regions(std::string &message);
	void install_kanji_handlers();

	u8 kanji_r(offs_t offset);
	void kanji_w(offs_t offset, u8 data);

	u32 m_kanji_mask;
	u32 m_kanji_address;
};


class msx_cart_msxwrite_device : public msx_cart_kanji_device
{
public:
	msx_cart_msxwrite_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual image_init_result initialize_cartridge(std::string &message) override;

protected:
	// device-level overrides
	virtual void device_reset() override;
	virtual ioport_constructor device_input_ports() const override;

private:
	static constexpr size_t BANK_SIZE = 0x4000;

	template <int Bank> void bank_w(u8 data);

	memory_bank_array_creator<2> m_rombank;
	required_ioport m_kanji_switch;
	u8 m_bank_mask;
};

#endif // MAME_BUS_MSX_CART_KANJI_H
