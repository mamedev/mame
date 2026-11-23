// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"

#include "c2color_cpu.h"

DEFINE_DEVICE_TYPE(C2_COLOR_CPU, c2_color_cpu_device, "c2_color_cpu", "C2 Color 8051-based CPU")

c2_color_cpu_device::c2_color_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: i8052_device(mconfig, C2_COLOR_CPU, tag, owner, clock, 0)
{
}

void c2_color_cpu_device::clear_state()
{
	std::fill(std::begin(m_math), std::end(m_math), 0);
	m_math_written = 0;
	m_unknown = 0;
}

void c2_color_cpu_device::device_start()
{
	i8052_device::device_start();
	clear_state();
	save_item(NAME(m_math));
	save_item(NAME(m_math_written));
	save_item(NAME(m_unknown));
}

void c2_color_cpu_device::device_reset()
{
	i8052_device::device_reset();
	clear_state();
}

void c2_color_cpu_device::sfr_map(address_map &map)
{
	i8052_device::sfr_map(map);
	map(0x8e, 0x8e).rw(FUNC(c2_color_cpu_device::unknown_r), FUNC(c2_color_cpu_device::unknown_w));
	map(0xe9, 0xee).rw(FUNC(c2_color_cpu_device::math_r), FUNC(c2_color_cpu_device::math_w));
}

void c2_color_cpu_device::math_w(offs_t offset, u8 data)
{
	if (!offset)
		m_math_written = 0;
	m_math[offset] = data;
	m_math_written |= 1U << offset;
	if (offset != 5)
		return;

	// Multiplication writes E9, ED, EA, EE.  Division writes all six bytes
	// in ascending order, supplying a 32-bit dividend and 16-bit divisor.
	u32 const operand = u32(m_math[0]) | (u32(m_math[1]) << 8);
	u32 const factor = u32(m_math[4]) | (u32(m_math[5]) << 8);
	u32 result = operand * factor;
	if (m_math_written == 0x3f)
	{
		u32 const dividend = operand | (u32(m_math[2]) << 16) | (u32(m_math[3]) << 24);
		result = factor ? dividend / factor : 0xffffffff;
		u16 const remainder = factor ? dividend % factor : 0;
		m_math[4] = remainder;
		m_math[5] = remainder >> 8;
	}
	for (unsigned i = 0; i != 4; ++i)
		m_math[i] = result >> (8 * i);
	// Arithmetic timing and overflow/division-by-zero status are unknown.
}
