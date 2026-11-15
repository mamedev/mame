// license:BSD-3-Clause
// copyright-holders:David Haywood, MagikalUnicorn
/***************************************************************************

    XaviX arithmetic unit. Provides signed or unsigned 8-bit multiplication,
    optional accumulation into a 16-bit result, and shifts through shared
    operand registers. Software can also write the result bytes directly.

    Arithmetic completes immediately in this model; result latency is not
    modelled.

    TODO:
    - Model result latency if it becomes observable.

***************************************************************************/

#include "emu.h"
#include "xavix_math.h"

#define VERBOSE 0
#include "logmacro.h"

DEFINE_DEVICE_TYPE(XAVIX_MATH, xavix_math_device, "xavix_math", "XaviX Math Unit")

xavix_math_device::xavix_math_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, XAVIX_MATH, tag, owner, clock)
{
}

void xavix_math_device::map(address_map &map)
{
	map(0x0, 0x0).rw(FUNC(xavix_math_device::shift_ctrl_r), FUNC(xavix_math_device::shift_ctrl_w));
	map(0x1, 0x1).rw(FUNC(xavix_math_device::shift_result_r), FUNC(xavix_math_device::multiplicand_w));
	map(0x2, 0x2).rw(FUNC(xavix_math_device::multiplier_ctrl_r), FUNC(xavix_math_device::multiplier_ctrl_w));
	map(0x3, 0x3).rw(FUNC(xavix_math_device::multiplicand_r), FUNC(xavix_math_device::multiplicand_w));
	map(0x4, 0x4).rw(FUNC(xavix_math_device::multiplier_r), FUNC(xavix_math_device::multiplier_w));
	map(0x5, 0x6).rw(FUNC(xavix_math_device::product_r), FUNC(xavix_math_device::product_w));
}

void xavix_math_device::device_start()
{
	save_item(NAME(m_multiplier_ctrl));
	save_item(NAME(m_multiplicand));
	save_item(NAME(m_multiplier));
	save_item(NAME(m_result));
}

void xavix_math_device::device_reset()
{
	m_multiplier_ctrl = 0;
	m_multiplicand = 0;
	m_multiplier = 0;
	m_result = 0;
}

u8 xavix_math_device::shift_ctrl_r()
{
	return (BIT(m_multiplier_ctrl, 1) << 7) | (m_multiplier & 0x4f);
}

// epo_guru uses the shifter for ground movement in its 3D stages. They still
// flicker heavily, but that seems unrelated to these calculations and may
// instead be a raster-timing issue.
void xavix_math_device::shift_ctrl_w(u8 data)
{
	// The shift count and input share the multiplier's operand registers.
	// Starting a shift selects its input sign and disables accumulation.
	LOG("%s: shift_ctrl_w %02x\n", machine().describe_context(), data);
	m_multiplier_ctrl = BIT(data, 7) << 1;
	m_multiplier = data;
	// A signed four-bit count selects the low or high byte of this result.
	s16 const value = BIT(m_multiplier_ctrl, 1) ? s8(m_multiplicand) : m_multiplicand;
	m_result = u16(value * (1 << BIT(m_multiplier, 0, 3)));
}

u8 xavix_math_device::shift_result_r()
{
	u8 const lower = m_result;
	u8 const upper = m_result >> 8;
	u8 const result = BIT(m_multiplier, 6) ? upper ^ lower : BIT(m_multiplier, 3) ? upper : lower;

	if (!machine().side_effects_disabled())
		LOG("%s: shift_result_r %02x\n", machine().describe_context(), result);
	return result;
}

u8 xavix_math_device::multiplier_ctrl_r()
{
	return m_multiplier_ctrl;
}

void xavix_math_device::multiplier_ctrl_w(u8 data)
{
	// Bit 7 adds to the previous result; bits 1/0 select signed operands.
	LOG("%s: multiplier_ctrl_w %02x\n", machine().describe_context(), data);
	m_multiplier_ctrl = data & 0x83;
}

u8 xavix_math_device::multiplicand_r()
{
	return m_multiplicand;
}

void xavix_math_device::multiplicand_w(u8 data)
{
	LOG("%s: multiplicand_w %02x\n", machine().describe_context(), data);
	m_multiplicand = data;
}

u8 xavix_math_device::multiplier_r()
{
	return m_multiplier;
}

void xavix_math_device::multiplier_w(u8 data)
{
	LOG("%s: multiplier_w %02x\n", machine().describe_context(), data);
	m_multiplier = data;
	// TODO: Model result latency if it becomes observable.
	// Monster Truck's self-test inserts four NOPs before reading the product.
	multiply();
}

u8 xavix_math_device::product_r(offs_t offset)
{
	u8 const result = BIT(m_result, 8 * offset, 8);
	if (!machine().side_effects_disabled())
		LOG("%s: product_r %d %02x\n", machine().describe_context(), offset, result);
	return result;
}

void xavix_math_device::product_w(offs_t offset, u8 data)
{
	// rad_madf writes here to set the base value which the multiplication result gets added to
	LOG("%s: product_w %d %02x\n", machine().describe_context(), offset, data);
	if (offset)
		m_result = (m_result & 0x00ff) | (u16(data) << 8);
	else
		m_result = (m_result & 0xff00) | data;
}

void xavix_math_device::multiply()
{
	s16 const multiplicand = BIT(m_multiplier_ctrl, 1) ? s8(m_multiplicand) : m_multiplicand;
	s16 const multiplier = BIT(m_multiplier_ctrl, 0) ? s8(m_multiplier) : m_multiplier;
	u16 const result = u16(multiplicand * multiplier);

	if (BIT(m_multiplier_ctrl, 7))
		m_result += result;
	else
		m_result = result;
}
