// license:GPL-2.0
// copyright-holders:Giulio Zausa
/*
 Roland/Boss/Fujitsu MB654119 chorus/delay gate array, R15229844 (DEP-5, RCE-10 and more).

 Reverse engineered from silicon using Furrtek's R15229844 die photographs,
 verified with a logic analyzer against a Roland DEP-5.
 Ported from the Gearmulator project (The Usual Suspects).
*/
#include "emu.h"
#include "mb654119.h"

DEFINE_DEVICE_TYPE(MB654119, mb654119_device, "mb654119", "Boss MB654119 chorus gate array")

mb654119_device::mb654119_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, MB654119, tag, owner, clock)
	, m_pin_long(false)
	, m_pin_k64(true)
	, m_base_delay(0)
	, m_depth(0)
	, m_ticks_per_half(1U << 20)
	, m_limit(0xffffffff)
	, m_delay_step(8)
	, m_acc(0)
	, m_peak(0)
	, m_ramp(0)
	, m_ticks(0)
	, m_dir(false)
	, m_par(false)
	, m_buf{}
	, m_w(0)
	, m_mask(0)
{
}

void mb654119_device::device_start()
{
	save_item(NAME(m_base_delay));
	save_item(NAME(m_depth));
	save_item(NAME(m_ticks_per_half));
	save_item(NAME(m_limit));
	save_item(NAME(m_delay_step));
	save_item(NAME(m_acc));
	save_item(NAME(m_peak));
	save_item(NAME(m_ramp));
	save_item(NAME(m_ticks));
	save_item(NAME(m_dir));
	save_item(NAME(m_par));
	save_item(NAME(m_buf));
	save_item(NAME(m_w));
	save_item(NAME(m_mask));
}

void mb654119_device::device_reset()
{
	m_delay_step = m_pin_long ? (m_pin_k64 ? 16u : 64u) : 8u;
	m_mask = (m_pin_k64 ? 16384u : 4096u) - 1u;
	m_buf.fill(0);
	m_w = 0;
	// Where the sweep is cut short. S29 carries an extra term when a
	// 64K DRAM is fitted, which makes it fire at 4096 samples of total
	// delay; with a 16K part it only fires when the 14 bit offset
	// field itself wraps, so the sweep is free to run the whole
	// (4096 sample) buffer and wrap the address.
	m_limit = (m_pin_k64 ? 4096u : 16384u) << 8;
	m_ticks_per_half = 32768u - rate_to_ticks(0);
	m_base_delay = 2u << 8;
	m_depth = 0;
	init_pulse();
	m_ramp = 0;
	m_par = false;
}

void mb654119_device::write_rate(u8 v)
{
	m_ticks_per_half = 32768u - rate_to_ticks(v);
}

void mb654119_device::write_delay(u8 v)
{
	m_base_delay = (u32(v) * m_delay_step + 2) << 8;
}

void mb654119_device::write_depth(u8 v)
{
	m_depth = v;
}

void mb654119_device::init_pulse()
{
	m_acc = 0;
	m_peak = 0;
	m_ticks = 0;
	m_dir = false;
}

void mb654119_device::process_frame(s16 in, s16 &ch0, s16 &ch1)
{
	m_buf[m_w] = in;

	const u32 up = (m_acc >> 4) + 1;

	const u32 a = (m_acc >> 10) > (m_peak >> 10) ? 0 : ((m_peak >> 10) - (m_acc >> 10)) & 0xffff;
	const u32 down = (a << 6) + 64 - ((m_acc >> 4) & 0x3f);

	ch0 = s16(read(m_base_delay + (m_dir ? up : down)));
	ch1 = s16(read(m_base_delay + (m_dir ? down : up)));
	m_w = (m_w + 1) & m_mask;

	advance_lfo(m_base_delay >= (2u << 8) ? m_base_delay - (2u << 8) + up : up);
}

void mb654119_device::patch_input(u32 periods_back, s16 in)
{
	m_buf[(m_w - periods_back) & m_mask] = in;
}

u16 mb654119_device::rate_to_ticks(u8 r)
{
	const bool k99 = !((!BIT(r, 7) && BIT(r, 6) && BIT(r, 2)) || BIT(r, 7));
	const bool k78 = BIT(r, 3) ? k99 : !k99;
	const bool k77 = !(k99 || !BIT(r, 3));
	const bool k82 = BIT(r, 4) ? !k77 : k77;
	const bool k84 = !(!k77 || !BIT(r, 4));
	const bool j89 = BIT(r, 5) ? !k84 : k84;
	const bool j88 = !(!k84 || !BIT(r, 5));
	const bool j85 = BIT(r, 6) ? !j88 : j88;
	const bool j74 = !(!j88 || !BIT(r, 6));
	const bool g104 = !BIT(r, 7) && BIT(r, 6);   // 2nd segment
	const bool k101 = !(BIT(r, 7) || BIT(r, 6)); // 1st segment
	const bool g103 = !(!g104 && BIT(r, 2));
	const bool g101 = !((!g104 && g103) || (g103 && BIT(r, 2)));

	u16 t = 0;
	t |= u16(!(BIT(r, 7) && j74)) << 14;
	t |= u16(!(BIT(r, 7) && j85)) << 13;
	t |= u16(!(BIT(r, 7) && j89)) << 12;
	t |= u16(!(BIT(r, 7) && k82)) << 11;
	t |= u16(!((j88 && g104) || (k78 && BIT(r, 7)))) << 10;
	t |= u16(!((j89 && g104) || (g101 && BIT(r, 7)))) << 9;
	t |= u16(!((k82 && g104) || (BIT(r, 1) && BIT(r, 7)))) << 8;
	t |= u16(!((k78 && g104) || (BIT(r, 0) && BIT(r, 7)))) << 7;
	t |= u16(!(g104 && g101)) << 6;
	t |= u16(!((j89 && k101) || (BIT(r, 1) && g104))) << 5;
	t |= u16(!((k82 && k101) || (BIT(r, 0) && g104))) << 4;
	t |= u16(!(k101 && k78)) << 3;
	t |= u16(!(k101 && g101)) << 2;
	t |= u16(!(k101 && BIT(r, 1))) << 1;
	t |= u16(!(k101 && BIT(r, 0))) << 0;
	return t;
}

s32 mb654119_device::read(u32 d256) const
{
	const u32 d = (d256 - 1) >> 8; // integer taps d, d+1
	const u32 lt = ((d + 1) << 8) - d256; // 0..255
	const s32 near_ = m_buf[(m_w - d) & m_mask];
	const s32 far_ = m_buf[(m_w - d - 1) & m_mask];
	s32 sum = 0;
	for (int k = 0; k < 8; ++k)
	{
		const s32 tap = BIT(lt, k) ? near_ : far_;
		sum = ((sum >> 1) + (tap >> 1)) + ((sum | tap) & 1);
	}
	return sum;
}

void mb654119_device::advance_lfo(u32 rising_offset)
{
	m_par = !m_par;
	if (m_par)
		m_acc += m_depth;

	++m_ramp;
	if ((m_ramp & 127) == 0)
		++m_ticks;

	const bool underflow = !m_dir && (m_acc >> 10) > (m_peak >> 10);
	if (2 * m_ticks + ((m_ramp >> 6) & 1) >= m_ticks_per_half || underflow || rising_offset > m_limit)
	{
		m_ticks = 0;
		m_peak = m_acc & ~0x3ffu; // the snapshot drops the low bits
		m_acc = 0;
		m_dir = !m_dir;
	}
}
