// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef MAME_BUS_MSX_CART_ASCII_H
#define MAME_BUS_MSX_CART_ASCII_H

#pragma once

#include "bus/msx/slot/cartridge.h"


DECLARE_DEVICE_TYPE(MSX_CART_ASCII8,       msx_cart_ascii8_device)
DECLARE_DEVICE_TYPE(MSX_CART_ASCII16,      msx_cart_ascii16_device)
DECLARE_DEVICE_TYPE(MSX_CART_ASCII8_SRAM,  msx_cart_ascii8_sram_device)
DECLARE_DEVICE_TYPE(MSX_CART_ASCII16_SRAM, msx_cart_ascii16_sram_device)


class msx_cart_ascii8_device : public device_t, public msx_cart_interface
{
public:
	msx_cart_ascii8_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual image_init_result initialize_cartridge(std::string &message) override;

protected:
	// device-level overrides
	virtual void device_start() override { }
	virtual void device_reset() override;

private:
	static constexpr size_t BANK_SIZE = 0x2000;

	template <int Bank> void bank_w(u8 data);

	memory_bank_array_creator<4> m_rombank;
	u8 m_bank_mask;
};


class msx_cart_ascii16_device : public device_t, public msx_cart_interface
{
public:
	msx_cart_ascii16_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual image_init_result initialize_cartridge(std::string &message) override;

protected:
	// device-level overrides
	virtual void device_start() override { }
	virtual void device_reset() override;

private:
	static constexpr size_t BANK_SIZE = 0x4000;

	template <int Bank> void bank_w(u8 data);

	memory_bank_array_creator<2> m_rombank;
	u8 m_bank_mask;
};


class msx_cart_ascii8_sram_device : public device_t, public msx_cart_interface
{
public:
	msx_cart_ascii8_sram_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual image_init_result initialize_cartridge(std::string &message) override;

protected:
	// device-level overrides
	virtual void device_start() override { }
	virtual void device_reset() override;

private:
	static constexpr size_t BANK_SIZE = 0x2000;

	void mapper_write(offs_t offset, u8 data);

	memory_bank_array_creator<4> m_rombank;
	memory_view m_view2;
	memory_view m_view3;
	u8 m_bank_mask;
	u8 m_sram_select_mask;
};


class msx_cart_ascii16_sram_device : public device_t, public msx_cart_interface
{
public:
	msx_cart_ascii16_sram_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual image_init_result initialize_cartridge(std::string &message) override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	static constexpr size_t BANK_SIZE = 0x4000;

	void mapper_write_6000(u8 data);
	void mapper_write_7000(u8 data);

	memory_bank_array_creator<2> m_rombank;
	memory_view m_view;
	u8 m_bank_mask;
	u8 m_sram_select_mask;
};


#endif // MAME_BUS_MSX_CART_ASCII_H
