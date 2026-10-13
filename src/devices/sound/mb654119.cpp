// license:BSD-3-Clause
// copyright-holders:Giulio Zausa
/*
 Roland/Boss/Fujitsu MB654119 chorus/delay gate array, R15229844 (DEP-5, RCE-10 and more).

 Reverse engineered from silicon using Furrtek's R15229844 die photographs,
 verified with logic analyzer against a Roland DEP-5.
*/
#include "emu.h"
#include "mb654119.h"

DEFINE_DEVICE_TYPE(MB654119, mb654119_device, "mb654119", "Boss MB654119 chorus gate array")

mb654119_device::mb654119_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, MB654119, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
{
}

void mb654119_device::device_start()
{
	m_stream = stream_alloc(1, 2, clock() / 80);
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
	save_item(NAME(m_last_d0));
	save_item(NAME(m_last_d1));
}

void mb654119_device::device_reset()
{
	reset(false, true);
}

void mb654119_device::sound_stream_update(sound_stream &stream)
{
	if (m_external_clock)
	{
		stream.fill(0, 0);
		stream.fill(1, 0);
		return;
	}
	for (int i = 0; i < stream.samples(); ++i)
	{
		s16 a, b;
		process_frame(std::clamp<int>(stream.get(0, i) * 32768, -32768, 32767), a, b);
		stream.put_int(0, i, a, 32768);
		stream.put_int(1, i, b, 32768);
	}
}

void mb654119_device::reset(bool pin_long, bool pin_k64)
{
	m_delay_step = pin_long ? (pin_k64 ? 16u : 64u) : 8u;
	m_mask = (pin_k64 ? 16384u : 4096u) - 1u;
	m_buf.fill(0);
	m_w = 0;
	// Where the sweep is cut short. S29 carries an extra term when a
	// 64K DRAM is fitted, which makes it fire at 4096 samples of total
	// delay; with a 16K part it only fires when the 14 bit offset
	// field itself wraps, so the sweep is free to run the whole
	// (4096 sample) buffer and wrap the address.
	m_limit = (pin_k64 ? 4096u : 16384u) << 8;
	m_ticks_per_half = 32768u - rate_to_ticks(0);
	m_base_delay = 2u << 8;
	m_depth = 0;
	init_pulse();
	m_ramp = 0;
	m_par = false;
}

void mb654119_device::write_rate(u8 v)
{
	if (m_stream && !m_external_clock)
		m_stream->update();
	m_ticks_per_half = 32768u - rate_to_ticks(v);
}

void mb654119_device::write_delay(u8 v)
{
	if (m_stream && !m_external_clock)
		m_stream->update();
	m_base_delay = (u32(v) * m_delay_step + 2) << 8;
}

void mb654119_device::write_depth(u8 v)
{
	if (m_stream && !m_external_clock)
		m_stream->update();
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

	m_last_d0 = m_base_delay + (m_dir ? up : down);
	m_last_d1 = m_base_delay + (m_dir ? down : up);
	ch0 = s16(read(m_last_d0));
	ch1 = s16(read(m_last_d1));
	m_w = (m_w + 1) & m_mask;

	advance_lfo(m_base_delay >= (2u << 8) ? m_base_delay - (2u << 8) + up : up);
}

void mb654119_device::patch_input(const u32 periods_back, const s16 in)
{
	m_buf[(m_w - periods_back) & m_mask] = in;
}

u16 mb654119_device::rate_to_ticks(u8 r)
{
	auto b = [&](const int n) { return bool((r >> n) & 1); };
	const bool k99 = !((!b(7) && b(6) && b(2)) || b(7));
	const bool k78 = b(3) ? k99 : !k99;
	const bool k77 = !(k99 || !b(3));
	const bool k82 = b(4) ? !k77 : k77;
	const bool k84 = !(!k77 || !b(4));
	const bool j89 = b(5) ? !k84 : k84;
	const bool j88 = !(!k84 || !b(5));
	const bool j85 = b(6) ? !j88 : j88;
	const bool j74 = !(!j88 || !b(6));
	const bool g104 = !b(7) && b(6);   // 2nd segment
	const bool k101 = !(b(7) || b(6)); // 1st segment
	const bool g103 = !(!g104 && b(2));
	const bool g101 = !((!g104 && g103) || (g103 && b(2)));

	u16 t = 0;
	t |= u16(!(b(7) && j74)) << 14;
	t |= u16(!(b(7) && j85)) << 13;
	t |= u16(!(b(7) && j89)) << 12;
	t |= u16(!(b(7) && k82)) << 11;
	t |= u16(!((j88 && g104) || (k78 && b(7)))) << 10;
	t |= u16(!((j89 && g104) || (g101 && b(7)))) << 9;
	t |= u16(!((k82 && g104) || (b(1) && b(7)))) << 8;
	t |= u16(!((k78 && g104) || (b(0) && b(7)))) << 7;
	t |= u16(!(g104 && g101)) << 6;
	t |= u16(!((j89 && k101) || (b(1) && g104))) << 5;
	t |= u16(!((k82 && k101) || (b(0) && g104))) << 4;
	t |= u16(!(k101 && k78)) << 3;
	t |= u16(!(k101 && g101)) << 2;
	t |= u16(!(k101 && b(1))) << 1;
	t |= u16(!(k101 && b(0))) << 0;
	return t;
}

s32 mb654119_device::read(const u32 d256) const
{
	const u32 d = (d256 - 1) >> 8; // integer taps d, d+1
	const u32 lt = ((d + 1) << 8) - d256; // 0..255
	const s32 near_ = m_buf[(m_w - d) & m_mask];
	const s32 far_ = m_buf[(m_w - d - 1) & m_mask];

	s32 sum = 0;
	for (int k = 0; k < 8; ++k)
	{
		const s32 tap = ((lt >> k) & 1) ? near_ : far_;
		sum = ((sum >> 1) + (tap >> 1)) + ((sum | tap) & 1);
	}
	return sum;
}

void mb654119_device::advance_lfo(const u32 rising_offset)
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
