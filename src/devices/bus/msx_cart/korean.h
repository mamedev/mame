// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef MAME_BUS_MSX_CART_KOREAN_H
#define MAME_BUS_MSX_CART_KOREAN_H

#pragma once

#include "bus/msx_cart/cartridge.h"


DECLARE_DEVICE_TYPE(MSX_CART_KOREAN_80IN1,  msx_cart_korean_80in1_device)
DECLARE_DEVICE_TYPE(MSX_CART_KOREAN_90IN1,  msx_cart_korean_90in1_device)
DECLARE_DEVICE_TYPE(MSX_CART_KOREAN_126IN1, msx_cart_korean_126in1_device)


class msx_cart_korean_80in1_device : public device_t, public msx_cart_interface
{
public:
	msx_cart_korean_80in1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void initialize_cartridge() override;

	virtual uint8_t read_cart(offs_t offset) override;
	virtual void write_cart(offs_t offset, uint8_t data) override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_post_load() override;

private:
	void restore_banks();
	void setup_bank(uint8_t bank);

	uint8_t m_bank_mask;
	uint8_t m_selected_bank[4];
	uint8_t *m_bank_base[4];
};


class msx_cart_korean_90in1_device : public device_t, public msx_cart_interface
{
public:
	msx_cart_korean_90in1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void initialize_cartridge() override;

	virtual uint8_t read_cart(offs_t offset) override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_post_load() override;

private:
	void banking(uint8_t data);

	void restore_banks();

	uint8_t m_bank_mask;
	uint8_t m_selected_bank;
	uint8_t *m_bank_base[4];
};


class msx_cart_korean_126in1_device : public device_t, public msx_cart_interface
{
public:
	msx_cart_korean_126in1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void initialize_cartridge() override;

	virtual uint8_t read_cart(offs_t offset) override;
	virtual void write_cart(offs_t offset, uint8_t data) override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_post_load() override;

private:
	void restore_banks();
	void setup_bank(uint8_t bank);

	uint8_t m_bank_mask;
	uint8_t m_selected_bank[2];
	uint8_t *m_bank_base[2];
};


#endif // MAME_BUS_MSX_CART_KOREAN_H
