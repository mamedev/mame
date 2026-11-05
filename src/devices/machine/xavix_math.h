// license:BSD-3-Clause
// copyright-holders:David Haywood, MagikalUnicorn

#ifndef MAME_MACHINE_XAVIX_MATH_H
#define MAME_MACHINE_XAVIX_MATH_H

#pragma once

class xavix_math_device : public device_t
{
public:
	xavix_math_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	void map(address_map &map) ATTR_COLD;

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	u8 shift_ctrl_r();
	void shift_ctrl_w(u8 data);
	u8 shift_result_r();
	u8 multiplier_ctrl_r();
	void multiplier_ctrl_w(u8 data);
	u8 multiplicand_r();
	void multiplicand_w(u8 data);
	u8 multiplier_r();
	void multiplier_w(u8 data);
	u8 product_r(offs_t offset);
	void product_w(offs_t offset, u8 data);

	void multiply();

	u8 m_multiplier_ctrl;
	u8 m_multiplicand;
	u8 m_multiplier;
	u16 m_result;
};

DECLARE_DEVICE_TYPE(XAVIX_MATH, xavix_math_device)

#endif // MAME_MACHINE_XAVIX_MATH_H
