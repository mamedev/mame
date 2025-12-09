// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*
** Model 1 coprocessor TGP simulation
*/

#include "emu.h"
#include "debugger.h"
#include "cpu/mb86233/mb86233.h"
#include "cpu/v60/v60.h"
#include "model1.h"

void model1_state::machine_start()
{
	m_digits.resolve();
	m_outs.resolve();

	m_copro_ram_data = std::make_unique<u32[]>(0x2000);

	save_pointer(NAME(m_copro_ram_data), 0x2000);
	save_item(NAME(m_v60_copro_ram_adr));
	save_item(NAME(m_v60_copro_ram_latch));

	m_copro_fifo_in->setup(16,
						   [this]() { m_tgp_copro->stall(); },
						   [this]() { m_tgp_copro->set_input_line(INPUT_LINE_HALT, ASSERT_LINE); },
						   [this]() { m_tgp_copro->set_input_line(INPUT_LINE_HALT, CLEAR_LINE); },
						   [this]() { m_maincpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE); },
						   [this]() { m_maincpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE); },
						   [    ]() { },
						   [    ]() { });
	m_copro_fifo_out->setup(16,
							[this]() { m_maincpu->stall(); },
							[this]() { m_maincpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE); },
							[this]() { m_maincpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE); },
							[this]() { m_tgp_copro->set_input_line(INPUT_LINE_HALT, ASSERT_LINE); },
							[this]() { m_tgp_copro->set_input_line(INPUT_LINE_HALT, CLEAR_LINE); },
							[    ]() { },
							[    ]() { });
}

void model1_state::copro_reset()
{
	m_v60_copro_ram_adr = 0;
	m_copro_ram_adr[0] = 0;
	m_copro_ram_adr[1] = 0;
	m_copro_ram_adr[2] = 0;
	m_copro_ram_adr[3] = 0;
	m_copro_sincos_base = 0;
	m_copro_inv_base = 0;
	m_copro_isqrt_base = 0;
	std::fill(std::begin(m_copro_atan_base), std::end(m_copro_atan_base), 0);
	std::fill(std::begin(m_v60_copro_ram_latch), std::end(m_v60_copro_ram_latch), 0);
	memset(m_copro_ram_data.get(), 0, 0x2000*4);
}

u16 model1_state::v60_copro_ram_adr_r()
{
	return m_v60_copro_ram_adr;
}

void model1_state::v60_copro_ram_adr_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_v60_copro_ram_adr);
}

u16 model1_state::v60_copro_ram_r(offs_t offset)
{
	u16 r;
	if (!offset)
		r = m_copro_ram_data[m_v60_copro_ram_adr & 0x1fff];

	else {
		r = m_copro_ram_data[m_v60_copro_ram_adr & 0x1fff] >> 16;

		if(m_v60_copro_ram_adr & 0x8000)
			m_v60_copro_ram_adr ++;
	}

	return r;
}

void model1_state::v60_copro_ram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(m_v60_copro_ram_latch + offset);

	if (offset) {
		u32 v = m_v60_copro_ram_latch[0] | (m_v60_copro_ram_latch[1] << 16);
		m_copro_ram_data[m_v60_copro_ram_adr & 0x1fff] = v;
		if(m_v60_copro_ram_adr & 0x8000)
			m_v60_copro_ram_adr++;
	}
}

u16 model1_state::v60_copro_fifo_r(offs_t offset)
{
	if (!offset) {
		m_v60_copro_fifo_r = m_copro_fifo_out->pop();
		return m_v60_copro_fifo_r;

	} else
		return m_v60_copro_fifo_r >> 16;
}

void model1_state::v60_copro_fifo_w(offs_t offset, u16 data)
{
	if(offset) {
		m_v60_copro_fifo_w = (m_v60_copro_fifo_w & 0x0000ffff) | (data << 16);
		m_copro_fifo_in->push(u32(m_v60_copro_fifo_w));

	} else
		m_v60_copro_fifo_w = (m_v60_copro_fifo_w & 0xffff0000) | data;
}

/* Coprocessor TGP memory map */
void model1_state::copro_prog_map(address_map &map)
{
	map(0x000, 0x7ff).rom();
}

void model1_state::copro_data_map(address_map &map)
{
	map(0x0000, 0x00ff).ram();
	map(0x0100, 0x0100).r(m_copro_fifo_in, FUNC(generic_fifo_u32_device::read));

	map(0x0200, 0x03ff).ram();
	map(0x0400, 0x0400).w(m_copro_fifo_out, FUNC(generic_fifo_u32_device::write));
}

void model1_state::copro_io_map(address_map &map)
{
	map(0x0000, 0x0000).rw(FUNC(model1_state::copro_ramadr_r), FUNC(model1_state::copro_ramadr_w)).select(0x18);
	map(0x0001, 0x0001).rw(FUNC(model1_state::copro_ramdata_r), FUNC(model1_state::copro_ramdata_w)).select(0x18);
	map(0x0020, 0x0023).rw(FUNC(model1_state::copro_sincos_r), FUNC(model1_state::copro_sincos_w));
	map(0x0024, 0x0027).rw(FUNC(model1_state::copro_atan_r), FUNC(model1_state::copro_atan_w));
	map(0x0028, 0x0029).rw(FUNC(model1_state::copro_inv_r), FUNC(model1_state::copro_inv_w));
	map(0x002a, 0x002b).rw(FUNC(model1_state::copro_isqrt_r), FUNC(model1_state::copro_isqrt_w));
	map(0x002e, 0x002e).w(FUNC(model1_state::copro_data_w));
	map(0x8000, 0xffff).r(FUNC(model1_state::copro_data_r));
}

void model1_state::copro_rf_map(address_map &map)
{
	map(0x0, 0x0).nopw(); // leds
}

void model1_state::copro_ramadr_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_copro_ram_adr[offset >> 3]);
}

u32 model1_state::copro_ramadr_r(offs_t offset)
{
	return m_copro_ram_adr[offset >> 3];
}

void model1_state::copro_ramdata_w(offs_t offset, u32 data, u32 mem_mask)
{
	if(m_copro_ram_adr[offset >> 3] & 0x40000) {
		COMBINE_DATA(&m_copro_ram_data[0x1000 | (m_copro_ram_adr[offset >> 3] & 0x1fff)]);
	} else {
		COMBINE_DATA(&m_copro_ram_data[m_copro_ram_adr[offset >> 3] & 0x1fff]);
	}
	m_copro_ram_adr[offset >> 3] ++;
}

u32 model1_state::copro_ramdata_r(offs_t offset)
{
	u32 val = (m_copro_ram_adr[offset >> 3] & 0x40000) ? m_copro_ram_data[0x1000 | (m_copro_ram_adr[offset >> 3] & 0x1fff)] : m_copro_ram_data[m_copro_ram_adr[offset >> 3] & 0x1fff];
	if(!machine().side_effects_disabled())
		m_copro_ram_adr[offset >> 3] ++;
	return val;
}



void model1_state::copro_sincos_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_copro_sincos_base);
}

u32 model1_state::copro_sincos_r(offs_t offset)
{
	offs_t ang = m_copro_sincos_base + offset * 0x4000;
	offs_t index = ang & 0x3fff;
	if (ang & 0x4000)
		index = std::min(0x4000 - (int)index, 0x3fff);
	u32 result = m_copro_tables[index];
	if(ang & 0x8000)
		result ^= 0x80000000;
	return result;
}

void model1_state::copro_inv_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_copro_inv_base);
}

u32 model1_state::copro_inv_r(offs_t offset)
{
	offs_t index = ((m_copro_inv_base >> 9) & 0x3ffe) | (offset & 1);
	u32 result = m_copro_tables[index | 0x8000];
	u8 bexp = (m_copro_inv_base >> 23) & 0xff;
	u8 exp = (result >> 23) + (0x7f - bexp);
	result = (result & 0x807fffff) | (exp << 23);
	if(m_copro_inv_base & 0x80000000)
		result ^= 0x80000000;
	return result;
}

void model1_state::copro_isqrt_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_copro_isqrt_base);
}

u32 model1_state::copro_isqrt_r(offs_t offset)
{
	offs_t index = 0x2000 ^ (((m_copro_isqrt_base>> 10) & 0x3ffe) | (offset & 1));
	u32 result = m_copro_tables[index | 0xc000];
	u8 bexp = (m_copro_isqrt_base >> 24) & 0x7f;
	u8 exp = (result >> 23) + (0x3f - bexp);
	result = (result & 0x807fffff) | (exp << 23);
	if(!(offset & 1))
		result &= 0x7fffffff;
	return result;
}

void model1_state::copro_atan_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_copro_atan_base[offset]);
}

u32 model1_state::copro_atan_r()
{
	u32 idx = m_copro_atan_base[3] & 0xffff;
	if(idx & 0xc000)
		idx = 0x3fff;
	u32 result = m_copro_tables[idx | 0x4000];

	// Correct for table bug, it seems that the hardware does something equivalent somehow,
	// or maybe there are boards with updated opr roms

	u16 dt = (result >> 16) + result;
	if(dt & 0x001) {
		if((result & 0x00f) == 0x00e)
			result -= 0x00000001;
		else
			result -= 0x00010000;
	}
	if(dt & 0x010) {
		if((result & 0x0f0) == 0x0e0)
			result -= 0x00000010;
		else
			result -= 0x00100000;
	}
	if(dt & 0x100) {
		if((result & 0xf00) == 0xe00)
			result -= 0x00000100;
		else
			result -= 0x01000000;
	}

	bool s0 = m_copro_atan_base[0] & 0x80000000;
	bool s1 = m_copro_atan_base[1] & 0x80000000;
	bool s2 = m_copro_atan_base[2] & 0x80000000;

	if(s0 ^ s1 ^ s2)
		result >>= 16;
	if(s2)
		result += 0x4000;
	if((s0 && !s2) || (s1 && s2))
		result += 0x8000;

	return result & 0xffff;
}

void model1_state::copro_data_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_copro_data_base);
}

u32 model1_state::copro_data_r(offs_t offset)
{
	offs_t index = (m_copro_data_base & ~0x7fff) | offset;
	index &= (m_copro_data->bytes() >> 2) - 1;
	return m_copro_data->as_u32(index);
}
