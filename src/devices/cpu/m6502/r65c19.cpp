// license:BSD-3-Clause
// copyright-holders:AJR
/**********************************************************************

    Rockwell R65C19 Microcomputer (MCU)

    TODO: fully describe this MCU and its successors (C29, C39) and
    emulate their internal peripherals (only core emulation now)

**********************************************************************/

#include "emu.h"
#include "r65c19.h"
#include "r65c19d.h"

DEFINE_DEVICE_TYPE(R65C19, r65c19_device, "r65c19", "Rockwell R65C19 MCU")

r65c19_device::r65c19_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, address_map_constructor internal_map)
	: r65c02_device(mconfig, type, tag, owner, clock)
	, m_w(0)
	, m_i(0)
{
	program_config.m_internal_map = std::move(internal_map);
}

r65c19_device::r65c19_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: r65c19_device(mconfig, R65C19, tag, owner, clock, address_map_constructor())
{
}

std::unique_ptr<util::disasm_interface> r65c19_device::create_disassembler()
{
	return std::make_unique<r65c19_disassembler>();
}

void r65c19_device::do_add(u8 v)
{
	P &= ~F_C;
	do_adc(v);
}

u16 r65c19_device::do_accumulate(u16 v, u16 w)
{
	// Compute the sum
	s32 result = s16(v) + s16(w);

	// Determine flags and saturate result upon overflow
	P &= ~(F_N | F_V);
	if (result > 32767)
	{
		P |= F_V;
		result = 32767;
	}
	else if (result < 0)
	{
		P |= F_N;
		if (result < -32768)
		{
			P |= F_V;
			result = -32768;
		}
	}

	// MPA and MPY always destroy the old value of Y, as does RND when it overflows
	Y = result & 0xff;

	// 16-bit result to W, or high byte to A
	return result & 0xffff;
}

u16 r65c19_device::get_irq_vector()
{
	// TODO: this is a stub
	return 0xfffc;
}

void r65c19_device::device_start()
{
	mintf = std::make_unique<mi_default>();

	c19_init();
}

void r65c19_device::c19_init()
{
	init();

	state_add(R65C19_W, "W", m_w);
	state_add<u8>(R65C19_WL, "WL",
		[this]() { return m_w & 0xff; },
		[this](u8 data) { m_w = set_l(m_w, data); }).noshow();
	state_add<u8>(R65C19_WH, "WH",
		[this]() { return m_w >> 8; },
		[this](u8 data) { m_w = set_h(m_w, data); }).noshow();
	state_add(R65C19_I, "I", m_i);

	save_item(NAME(m_w));
	save_item(NAME(m_i));
}

void r65c19_device::device_reset()
{
	r65c02_device::device_reset();
}

#include "cpu/m6502/r65c19.hxx"
