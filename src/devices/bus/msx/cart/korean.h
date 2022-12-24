// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef MAME_BUS_MSX_CART_KOREAN_H
#define MAME_BUS_MSX_CART_KOREAN_H

#pragma once

#include "bus/msx/slot/cartridge.h"


DECLARE_DEVICE_TYPE(MSX_CART_KOREAN_80IN1,  msx_cart_korean_80in1_device)
DECLARE_DEVICE_TYPE(MSX_CART_KOREAN_90IN1,  msx_cart_korean_90in1_device)
DECLARE_DEVICE_TYPE(MSX_CART_KOREAN_126IN1, msx_cart_korean_126in1_device)


class msx_cart_korean_80in1_device : public device_t, public msx_cart_interface
{
public:
	msx_cart_korean_80in1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual image_init_result initialize_cartridge(std::string &message) override;

protected:
	// device-level overrides
	virtual void device_start() override { }
	virtual void device_reset() override;

private:
	void bank_w(offs_t offset, u8 data);

	memory_bank_array_creator<4> m_rombank;

	u8 m_bank_mask;
};


class msx_cart_korean_90in1_device : public device_t, public msx_cart_interface
{
public:
	msx_cart_korean_90in1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual image_init_result initialize_cartridge(std::string &message) override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	void banking(u8 data);

	memory_bank_array_creator<3> m_rombank;
	memory_view m_view;
	u8 m_bank_mask;
};


class msx_cart_korean_126in1_device : public device_t, public msx_cart_interface
{
public:
	msx_cart_korean_126in1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual image_init_result initialize_cartridge(std::string &message) override;

protected:
	// device-level overrides
	virtual void device_start() override { }
	virtual void device_reset() override;

private:
	void bank_w(offs_t offset, u8 data);

	memory_bank_array_creator<2> m_rombank;
	u8 m_bank_mask;
};


#endif // MAME_BUS_MSX_CART_KOREAN_H
