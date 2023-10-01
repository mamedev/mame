// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Yamaha SWP00, rompler/dsp combo

#include "emu.h"
#include "swp00.h"

DEFINE_DEVICE_TYPE(SWP00, swp00_device, "swp00", "Yamaha SWP00 (TC170C120SF / XQ036A00) sound chip")

swp00_device::swp00_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SWP00, tag, owner, clock),
	  device_sound_interface(mconfig, *this),
	  device_rom_interface(mconfig, *this)
{
}

void swp00_device::device_add_mconfig(machine_config &config)
{
}

const std::array<u32, 4> swp00_device::lfo_shape_centered_saw = { 0x00000000, 0x00000000, 0xfff00000, 0xfff00000 }; // --////--
const std::array<u32, 4> swp00_device::lfo_shape_centered_tri = { 0x00000000, 0x0007ffff, 0xfff7ffff, 0xfff00000 }; // --/\/\--
const std::array<u32, 4> swp00_device::lfo_shape_offset_saw   = { 0x00000000, 0x00000000, 0x00000000, 0x00000000 }; // __////__
const std::array<u32, 4> swp00_device::lfo_shape_offset_tri   = { 0x00000000, 0x00000000, 0x000fffff, 0x000fffff }; // __/\/\__

const std::array<s32, 16> swp00_device::panmap = {
	0x000, 0x040, 0x080, 0x0c0,
	0x100, 0x140, 0x180, 0x1c0,
	0x200, 0x240, 0x280, 0x2c0,
	0x300, 0x340, 0x380, 0xfff
};

bool swp00_device::istep(s32 &value, s32 limit, s32 step)
{
	//	fprintf(stderr, "istep(%x, %x, %x)\n", value, limit, step);
	if(value < limit) {
		value += step;
		if(value >= limit) {
			value = limit;
			return true;
		}
		return false;
	}

	if(value > limit) {
		value -= step;
		if(value <= limit) {
			value = limit;
			return true;
		}
		return false;
	}

	return true;
}

s32 swp00_device::fpadd(s32 value, s32 step)
{
	s32 e = value >> 24;
	s32 m = value & 0xffffff;

	m += step << e;
	if(m & 0xfe000000)
		return 0xfffffff;

	while(m & 0x01000000) {
		m <<= 1;
		e ++;
	}
	if(e >= 16)
		return 0xfffffff;
	return (e << 24) | (m & 0xffffff);
}

s32 swp00_device::fpsub(s32 value, s32 step)
{
	s32 e = value >> 24;
	s32 m = (value & 0xffffff) | 0xfe000000;
	m = e < 0xc ? m - (step << e) : (m >> (e - 0xb)) - (step << 0xb);
	//	fprintf(stderr, "%07x %05x -> %x %08x\n", value, step, e, m);
	if(m >= 0)
		return 0;
	if(e >= 0xc)
		e = 0xb;
	while(m < 0xfe000000) {
		if(!e)
			return 0;
		e --;
		m >>= 1;
	}
	while(e != 0xf && (m >= 0xff000000)) {
		e ++;
		m <<= 1;
	}
	
	return (e << 24) | (m & 0xffffff);
}

bool swp00_device::fpstep(s32 &value, s32 limit, s32 step)
{
	//	fprintf(stderr, "fpstep(%x, %x, %x)\n", value, limit, step);

	// value, limit and step are 4.24 but step has its exponent and
	// top four bits zero

	if(value == limit)
		return true;
	if(value < limit) {
		value = fpadd(value, step);
		if(value >= limit) {
			value = limit;
			return true;
		}
		return false;
	}

	value = fpsub(value, step);
	if(value <= limit) {
		value = limit;
		return true;
	}
	return false;
}

s16 swp00_device::fpapply(s32 value, s16 sample)
{
	if(value >= 0x10000000)
		return 0;
	return (sample - ((sample * ((value >> 9) & 0x7fff)) >> 16)) >> (value >> 24);
}

s16 swp00_device::lpffpapply(s32 value, s16 sample)
{
	return ((((value >> 7) & 0x7fff) | 0x8000) * sample) >> (31 - (value >> 22));
}

// Some tables we need.  Maybe they're in roms inside the chip,
// maybe they're logic.  Probably slightly inexact too, would need
// a complicated hardware setup to really test them.

const std::array<s32, 0x80> swp00_device::attack_linear_step = {
	0x00027, 0x0002b, 0x0002f, 0x00033, 0x00037, 0x0003d, 0x00042, 0x00048,
	0x0004d, 0x00056, 0x0005e, 0x00066, 0x0006f, 0x0007a, 0x00085, 0x00090,
	0x0009b, 0x000ac, 0x000bd, 0x000cc, 0x000de, 0x000f4, 0x00109, 0x00120,
	0x00135, 0x00158, 0x00179, 0x00199, 0x001bc, 0x001e7, 0x00214, 0x00240,
	0x0026b, 0x002af, 0x002f2, 0x00332, 0x00377, 0x003d0, 0x0042c, 0x00480,
	0x004dc, 0x0055e, 0x005e9, 0x0066e, 0x006f4, 0x007a4, 0x00857, 0x0090b,
	0x009c3, 0x00acb, 0x00bd6, 0x00ce6, 0x00e00, 0x00f5e, 0x010d2, 0x01234,
	0x0139e, 0x015d0, 0x017f3, 0x01a20, 0x01c4a, 0x01f52, 0x02232, 0x0250f,
	0x027ff, 0x02c72, 0x03109, 0x0338b, 0x039c4, 0x04038, 0x04648, 0x04c84,
	0x05262, 0x05c1c, 0x065af, 0x06f5c, 0x07895, 0x0866f, 0x09470, 0x0a19e,
	0x0ae4c, 0x0c566, 0x0db8d, 0x0f00f, 0x10625, 0x12937, 0x14954, 0x16c17,
	0x1886e, 0x1c71c, 0x20000, 0x239e1, 0x2647c, 0x2aaab, 0x2ecfc, 0x3241f,
	0x35e51, 0x3a83b, 0x40000, 0x4325c, 0x47dc1, 0x4c8f9, 0x50505, 0x55555,
	0x58160, 0x5d174, 0x60606, 0x62b2e, 0x67b24, 0x6a63c, 0x6d3a0, 0x6eb3e,
	0x71c72, 0x73616, 0x75075, 0x76b98, 0x78788, 0x78788, 0x7a44c, 0x7a44c,
	0x7a44c, 0x7a44c, 0x7a44c, 0x7a44c, 0x7a44c, 0x7a44c, 0x7a44c, 0x7a44c,
};

const std::array<s32, 0x20> swp00_device::decay_linear_step = {
	0x15083, 0x17ad2, 0x1a41a, 0x1cbe7, 0x1f16d, 0x22ef1, 0x26a44, 0x2a1e4,
	0x2da35, 0x34034, 0x3a197, 0x40000, 0x45b82, 0x4b809, 0x51833, 0x57262,
	0x5d9f7, 0x6483f, 0x6b15c, 0x71c72, 0x77976, 0x7d119, 0x83127, 0x88889,
	0x8d3dd, 0x939a8, 0x991f2, 0x9d89e, 0xa0a0a, 0xa57eb, 0xa72f0, 0xac769,
};

void swp00_device::device_start()
{
	m_stream = stream_alloc(0, 2, 44100);

	save_item(NAME(m_waverom_access));
	save_item(NAME(m_waverom_val));
	save_item(NAME(m_intreg));
	save_item(NAME(m_fpreg));
	save_item(NAME(m_lpf_info));
	save_item(NAME(m_lpf_speed));
	save_item(NAME(m_lfo_famod_depth));
	save_item(NAME(m_rev_level));
	save_item(NAME(m_dry_level));
	save_item(NAME(m_cho_level));
	save_item(NAME(m_var_level));
	save_item(NAME(m_glo_level));
	save_item(NAME(m_panning));
	save_item(NAME(m_attack_speed));
	save_item(NAME(m_attack_level));
	save_item(NAME(m_decay_speed));
	save_item(NAME(m_decay_level));
	save_item(NAME(m_pitch));
	save_item(NAME(m_sample_start));
	save_item(NAME(m_sample_end));
	save_item(NAME(m_sample_dec_and_format));
	save_item(NAME(m_sample_address));
	save_item(NAME(m_lfo_step));
	save_item(NAME(m_lfo_pmod_depth));

	save_item(NAME(m_lfo_phase));
	save_item(NAME(m_sample_pos));
	save_item(NAME(m_sample_increment));
	save_item(NAME(m_envelope_level));

	save_item(NAME(m_glo_level_cur));
	save_item(NAME(m_pan_l));
	save_item(NAME(m_pan_r));

	save_item(NAME(m_lpf_feedback));
	save_item(NAME(m_lpf_target_value));
	save_item(NAME(m_lpf_value));
	save_item(NAME(m_lpf_timer));
	save_item(NAME(m_lpf_ha));
	save_item(NAME(m_lpf_hb));

	save_item(NAME(m_active));
	save_item(NAME(m_decay));
	save_item(NAME(m_decay_done));
	save_item(NAME(m_lpf_done));

	for(int i=0; i != 128; i++) {
		u32 v = 0;
		switch(i >> 3) {
		default:  v = ((i & 7) + 8) << (1 + (i >> 3)); break;
		case 0xb: v = ((i & 7) + 4) << 13; break;
		case 0xc: v = ((i & 6) + 6) << 14; break;
		case 0xd: v = ((i & 4) + 7) << 15; break;
		case 0xe: v = 15 << 15; break;
		case 0xf: v = 31 << 15; break;
		}
		m_global_step[i] = v;
	}

	// Log to linear 8-bits sample decompression.  Statistics say
	// that's what it should look like.  Note that 0 can be encoded
	// both as 0x00 and 0x80, and as it happens 0x80 is never used in
	// these samples.  Ends up with a 55dB dynamic range, to compare
	// with 8bits 48dB, 12bits 72dB and 16bits 96dB.

	//  Rescale so that it's roughly 16 bits.  Range ends up being +/- 78c0.

	for(int i=0; i<32; i++) {
		m_sample_log8[     i] =  i << 0;
		m_sample_log8[0x20|i] = (i << 1) + 0x21;
		m_sample_log8[0x40|i] = (i << 2) + 0x62;
		m_sample_log8[0x60|i] = (i << 3) + 0xe3;
	}
	for(int i=0; i<128; i++) {
		s16 base = m_sample_log8[i] << 6;
		m_sample_log8[i | 0x80] = - base;
		m_sample_log8[i]        = + base;
	}
}

void swp00_device::device_reset()
{
	m_waverom_access = 0;
	m_waverom_val = 0;

	std::fill(m_intreg.begin(), m_intreg.end(), 0);
	std::fill(m_fpreg.begin(), m_fpreg.end(), 0);
	std::fill(m_lpf_info.begin(), m_lpf_info.end(), 0);
	std::fill(m_lpf_speed.begin(), m_lpf_speed.end(), 0);
	std::fill(m_lfo_famod_depth.begin(), m_lfo_famod_depth.end(), 0);
	std::fill(m_rev_level.begin(), m_rev_level.end(), 0);
	std::fill(m_dry_level.begin(), m_dry_level.end(), 0);
	std::fill(m_cho_level.begin(), m_cho_level.end(), 0);
	std::fill(m_var_level.begin(), m_var_level.end(), 0);
	std::fill(m_glo_level.begin(), m_glo_level.end(), 0);
	std::fill(m_panning.begin(), m_panning.end(), 0);
	std::fill(m_attack_speed.begin(), m_attack_speed.end(), 0);
	std::fill(m_attack_level.begin(), m_attack_level.end(), 0);
	std::fill(m_decay_speed.begin(), m_decay_speed.end(), 0);
	std::fill(m_decay_level.begin(), m_decay_level.end(), 0);
	std::fill(m_pitch.begin(), m_pitch.end(), 0);
	std::fill(m_sample_start.begin(), m_sample_start.end(), 0);
	std::fill(m_sample_end.begin(), m_sample_end.end(), 0);
	std::fill(m_sample_dec_and_format.begin(), m_sample_dec_and_format.end(), 0);
	std::fill(m_sample_address.begin(), m_sample_address.end(), 0);
	std::fill(m_lfo_step.begin(), m_lfo_step.end(), 0);
	std::fill(m_lfo_pmod_depth.begin(), m_lfo_pmod_depth.end(), 0);

	std::fill(m_lfo_phase.begin(), m_lfo_phase.end(), 0);
	std::fill(m_sample_pos.begin(), m_sample_pos.end(), 0);
	std::fill(m_sample_increment.begin(), m_sample_increment.end(), 0);
	std::fill(m_envelope_level.begin(), m_envelope_level.end(), 0);

	std::fill(m_glo_level_cur.begin(), m_glo_level_cur.end(), 0);
	std::fill(m_pan_l.begin(), m_pan_l.end(), 0);
	std::fill(m_pan_r.begin(), m_pan_r.end(), 0);

	std::fill(m_lpf_feedback.begin(), m_lpf_feedback.end(), 0);
	std::fill(m_lpf_target_value.begin(), m_lpf_target_value.end(), 0);
	std::fill(m_lpf_value.begin(), m_lpf_value.end(), 0);
	std::fill(m_lpf_timer.begin(), m_lpf_timer.end(), 0);
	std::fill(m_lpf_ha.begin(), m_lpf_ha.end(), 0);
	std::fill(m_lpf_hb.begin(), m_lpf_hb.end(), 0);

	std::fill(m_active.begin(), m_active.end(), false);
	std::fill(m_decay.begin(), m_decay.end(), false);
	std::fill(m_decay_done.begin(), m_decay_done.end(), false);
	std::fill(m_lpf_done.begin(), m_lpf_done.end(), false);
}

void swp00_device::rom_bank_pre_change()
{
	m_stream->update();
}

void swp00_device::map(address_map &map)
{
	map(0x000, 0x7ff).rw(FUNC(swp00_device::snd_r), FUNC(swp00_device::snd_w));

	rchan(map, 0x0a).rw(FUNC(swp00_device::sample_start_r<1>), FUNC(swp00_device::sample_start_w<1>));
	rchan(map, 0x0b).rw(FUNC(swp00_device::sample_start_r<0>), FUNC(swp00_device::sample_start_w<0>));
	rchan(map, 0x20).rw(FUNC(swp00_device::lpf_info_r<1>), FUNC(swp00_device::lpf_info_w<1>));
	rchan(map, 0x21).rw(FUNC(swp00_device::lpf_info_r<0>), FUNC(swp00_device::lpf_info_w<0>));
	rchan(map, 0x22).rw(FUNC(swp00_device::lpf_speed_r), FUNC(swp00_device::lpf_speed_w));
	rchan(map, 0x23).rw(FUNC(swp00_device::lfo_famod_depth_r), FUNC(swp00_device::lfo_famod_depth_w));
	rchan(map, 0x24).rw(FUNC(swp00_device::lfo_step_r), FUNC(swp00_device::lfo_step_w));
	rchan(map, 0x25).rw(FUNC(swp00_device::lfo_pmod_depth_r), FUNC(swp00_device::lfo_pmod_depth_w));
	rchan(map, 0x26).rw(FUNC(swp00_device::attack_speed_r), FUNC(swp00_device::attack_speed_w));
	rchan(map, 0x27).rw(FUNC(swp00_device::attack_level_r), FUNC(swp00_device::attack_level_w));
	rchan(map, 0x28).rw(FUNC(swp00_device::decay_speed_r), FUNC(swp00_device::decay_speed_w));
	rchan(map, 0x29).rw(FUNC(swp00_device::decay_level_r), FUNC(swp00_device::decay_level_w));
	rchan(map, 0x2a).rw(FUNC(swp00_device::rev_level_r), FUNC(swp00_device::rev_level_w));
	rchan(map, 0x2b).rw(FUNC(swp00_device::dry_level_r), FUNC(swp00_device::dry_level_w));
	rchan(map, 0x2c).rw(FUNC(swp00_device::cho_level_r), FUNC(swp00_device::cho_level_w));
	rchan(map, 0x2d).rw(FUNC(swp00_device::var_level_r), FUNC(swp00_device::var_level_w));
	rchan(map, 0x2e).rw(FUNC(swp00_device::glo_level_r), FUNC(swp00_device::glo_level_w));
	rchan(map, 0x2f).rw(FUNC(swp00_device::panning_r), FUNC(swp00_device::panning_w));
	rchan(map, 0x30).rw(FUNC(swp00_device::sample_dec_and_format_r), FUNC(swp00_device::sample_dec_and_format_w));
	rchan(map, 0x31).rw(FUNC(swp00_device::sample_address_r<2>), FUNC(swp00_device::sample_address_w<2>));
	rchan(map, 0x32).rw(FUNC(swp00_device::sample_address_r<1>), FUNC(swp00_device::sample_address_w<1>));
	rchan(map, 0x33).rw(FUNC(swp00_device::sample_address_r<0>), FUNC(swp00_device::sample_address_w<0>));
	rchan(map, 0x34).rw(FUNC(swp00_device::pitch_r<1>), FUNC(swp00_device::pitch_w<1>));
	rchan(map, 0x35).rw(FUNC(swp00_device::pitch_r<0>), FUNC(swp00_device::pitch_w<0>));
	rchan(map, 0x36).rw(FUNC(swp00_device::sample_end_r<1>), FUNC(swp00_device::sample_end_w<1>));
	rchan(map, 0x37).rw(FUNC(swp00_device::sample_end_r<0>), FUNC(swp00_device::sample_end_w<0>));

	rany(map, 0x00, 0x02).rw(FUNC(swp00_device::waverom_access_r), FUNC(swp00_device::waverom_access_w));
	rany(map, 0x00, 0x03).r(FUNC(swp00_device::waverom_val_r));
	rany(map, 0x00, 0x08).w(FUNC(swp00_device::keyon_w<3>));
	rany(map, 0x00, 0x09).w(FUNC(swp00_device::keyon_w<2>));
	rany(map, 0x00, 0x0a).w(FUNC(swp00_device::keyon_w<1>));
	rany(map, 0x00, 0x0b).w(FUNC(swp00_device::keyon_w<0>));

	map(0x001, 0x001).rw(FUNC(swp00_device::state_r), FUNC(swp00_device::state_adr_w));

	map(0x180, 0x1ff).rw(FUNC(swp00_device::intreg_r), FUNC(swp00_device::intreg_w));
	map(0x200, 0x3ff).rw(FUNC(swp00_device::fpreg_r), FUNC(swp00_device::fpreg_w));
}


// Voice control

#if 0
static double exp_mant_to_double(signed int a1)
{
  signed int v1; // eax@1
  a1 >>= 6;
  v1 = a1;
  if ( a1 < 0 )
    v1 = 0;
  return (double)((unsigned short)v1 | 0x10000u) / ((double)(1 << (24 - (v1 >> 16))) * 256.0);
}

static double v2f(s32 value)
{
	return 1.0 - (1.0 - (value & 0xffffff) / 33554432.0) / (1 << (value >> 24));
}
#endif

template<int sel> void swp00_device::lpf_info_w(offs_t offset, u8 data)
{
	int chan = offset >> 1;
	u16 old = m_lpf_info[chan];
	m_stream->update();

	m_lpf_info[chan] = (m_lpf_info[chan] & ~(0xff << (8*sel))) | (data << (8*sel));
	if(m_lpf_info[chan] == old)
		return;

	if(!sel)
		logerror("lpf_info[%02x] = %04x\n", chan, m_lpf_info[chan]);

	u32 fb = m_lpf_info[chan] >> 11;
	u32 level = m_lpf_info[chan] & 0x7ff;
	if(fb < 4 && level > 0x7c0)
		level = 0x7c0;
	if(level)
		level |= 0x800;
	m_lpf_feedback[chan] = (fb + 4) << 21;
	m_lpf_target_value[chan] = level << 14;
}

template<int sel> u8 swp00_device::lpf_info_r(offs_t offset)
{
	int chan = offset >> 1;
	return m_lpf_info[chan] >> (8*sel);
}

void swp00_device::lpf_speed_w(offs_t offset, u8 data)
{
	int chan = offset >> 1;
	if(m_lpf_speed[chan] == data)
		return;
	m_stream->update();
	m_lpf_speed[chan] = data;
	logerror("lpf_speed[%02x] = %02x\n", chan, m_lpf_speed[chan]);
}

u8 swp00_device::lpf_speed_r(offs_t offset)
{
	int chan = offset >> 1;
	return m_lpf_speed[chan];
}

void swp00_device::lfo_famod_depth_w(offs_t offset, u8 data)
{
	int chan = offset >> 1;
	if(m_lfo_famod_depth[chan] == data)
		return;
	m_stream->update();
	m_lfo_famod_depth[chan] = data;
	logerror("lfo_famod_depth[%02x] = %02x\n", chan, m_lfo_famod_depth[chan]);
}

u8 swp00_device::lfo_famod_depth_r(offs_t offset)
{
	int chan = offset >> 1;
	return m_lfo_famod_depth[chan];
}

void swp00_device::rev_level_w(offs_t offset, u8 data)
{
	int chan = offset >> 1;
	if(m_rev_level[chan] == data)
		return;
	m_stream->update();
	m_rev_level[chan] = data;
	logerror("rev_level[%02x] = %02x\n", chan, m_rev_level[chan]);
}

u8 swp00_device::rev_level_r(offs_t offset)
{
	int chan = offset >> 1;
	return m_rev_level[chan];
}

void swp00_device::dry_level_w(offs_t offset, u8 data)
{
	int chan = offset >> 1;
	if(m_dry_level[chan] == data)
		return;
	m_stream->update();
	m_dry_level[chan] = data;
	logerror("dry_level[%02x] = %02x\n", chan, m_dry_level[chan]);
}

u8 swp00_device::dry_level_r(offs_t offset)
{
	int chan = offset >> 1;
	return m_dry_level[chan];
}

void swp00_device::cho_level_w(offs_t offset, u8 data)
{
	int chan = offset >> 1;
	if(m_cho_level[chan] == data)
		return;
	m_stream->update();
	m_cho_level[chan] = data;
	logerror("cho_level[%02x] = %02x\n", chan, m_cho_level[chan]);
}

u8 swp00_device::cho_level_r(offs_t offset)
{
	int chan = offset >> 1;
	return m_cho_level[chan];
}

void swp00_device::var_level_w(offs_t offset, u8 data)
{
	int chan = offset >> 1;
	if(m_var_level[chan] == data)
		return;
	m_stream->update();
	m_var_level[chan] = data;
	logerror("var_level[%02x] = %02x\n", chan, m_var_level[chan]);
}

u8 swp00_device::var_level_r(offs_t offset)
{
	int chan = offset >> 1;
	return m_var_level[chan];
}

void swp00_device::glo_level_w(offs_t offset, u8 data)
{
	int chan = offset >> 1;
	if(m_glo_level[chan] == data)
		return;
	m_glo_level[chan] = data;
	logerror("glo_level[%02x] = %02x\n", chan, m_glo_level[chan]);
}

u8 swp00_device::glo_level_r(offs_t offset)
{
	int chan = offset >> 1;
	return m_glo_level[chan];
}

void swp00_device::panning_w(offs_t offset, u8 data)
{
	int chan = offset >> 1;
	if(m_panning[chan] == data)
		return;
	m_stream->update();
	m_panning[chan] = data;
	logerror("panning[%02x] = %02x\n", chan, m_panning[chan]);
}

u8 swp00_device::panning_r(offs_t offset)
{
	int chan = offset >> 1;
	return m_panning[chan];
}

void swp00_device::attack_speed_w(offs_t offset, u8 data)
{
	int chan = offset >> 1;
	if(m_attack_speed[chan] == data)
		return;
	m_stream->update();
	m_attack_speed[chan] = data;
	logerror("attack_speed[%02x] = %02x\n", chan, m_attack_speed[chan]);
}

u8 swp00_device::attack_speed_r(offs_t offset)
{
	int chan = offset >> 1;
	return m_attack_speed[chan];
}

void swp00_device::attack_level_w(offs_t offset, u8 data)
{
	int chan = offset >> 1;
	if(m_attack_level[chan] == data)
		return;
	m_stream->update();
	m_attack_level[chan] = data;
	logerror("attack_level[%02x] = %02x\n", chan, m_attack_level[chan]);
}

u8 swp00_device::attack_level_r(offs_t offset)
{
	int chan = offset >> 1;
	return m_attack_level[chan];
}

void swp00_device::decay_speed_w(offs_t offset, u8 data)
{
	int chan = offset >> 1;
	if(m_decay_speed[chan] == data)
		return;

	m_stream->update();
	m_decay_speed[chan] = data;
	
	if(data & 0x80)
		m_decay[chan] = true;

	logerror("decay_speed[%02x] = %02x\n", chan, m_decay_speed[chan]);
}

u8 swp00_device::decay_speed_r(offs_t offset)
{
	int chan = offset >> 1;
	return m_decay_speed[chan];
}

void swp00_device::decay_level_w(offs_t offset, u8 data)
{
	int chan = offset >> 1;
	if(m_decay_level[chan] == data)
		return;
	m_stream->update();
	m_decay_level[chan] = data;
	logerror("decay_level[%02x] = %02x\n", chan, m_decay_level[chan]);
}

u8 swp00_device::decay_level_r(offs_t offset)
{
	int chan = offset >> 1;
	return m_decay_level[chan];
}

template<int sel> void swp00_device::pitch_w(offs_t offset, u8 data)
{
	int chan = offset >> 1;
	u16 old = m_pitch[chan];
	m_stream->update();
	m_pitch[chan] = (m_pitch[chan] & ~(0xff << (8*sel))) | (data << (8*sel));
	if(m_pitch[chan] == old)
		return;
	if(!sel)
		logerror("pitch[%02x] = %04x\n", chan, m_pitch[chan]);
}

template<int sel> u8 swp00_device::pitch_r(offs_t offset)
{
	int chan = offset >> 1;
	return m_pitch[chan] >> (8*sel);
}

template<int sel> void swp00_device::sample_start_w(offs_t offset, u8 data)
{
	int chan = offset >> 1;
	m_stream->update();

	m_sample_start[chan] = (m_sample_start[chan] & ~(0xff << (8*sel))) | (data << (8*sel));
	if(!sel)
		logerror("sample_start[%02x] = %04x\n", chan, m_sample_start[chan]);
}

template<int sel> u8 swp00_device::sample_start_r(offs_t offset)
{
	int chan = offset >> 1;
	return m_sample_start[chan] >> (8*sel);
}

template<int sel> void swp00_device::sample_end_w(offs_t offset, u8 data)
{
	int chan = offset >> 1;
	m_stream->update();

	m_sample_end[chan] = (m_sample_end[chan] & ~(0xff << (8*sel))) | (data << (8*sel));
	if(!sel)
		logerror("sample_end[%02x] = %04x\n", chan, m_sample_end[chan]);
}

template<int sel> u8 swp00_device::sample_end_r(offs_t offset)
{
	int chan = offset >> 1;
	return m_sample_end[chan] >> (8*sel);
}

void swp00_device::sample_dec_and_format_w(offs_t offset, u8 data)
{
	int chan = offset >> 1;
	m_stream->update();

	m_sample_dec_and_format[chan] = data;
	logerror("sample_dec_and_format[%02x] = %02x\n", chan, m_sample_dec_and_format[chan]);
}

u8 swp00_device::sample_dec_and_format_r(offs_t offset)
{
	int chan = offset >> 1;
	return m_sample_dec_and_format[chan];
}

template<int sel> void swp00_device::sample_address_w(offs_t offset, u8 data)
{
	int chan = offset >> 1;
	m_stream->update();

	m_sample_address[chan] = (m_sample_address[chan] & ~(0xff << (8*sel))) | (data << (8*sel));
	if(!sel)
		logerror("sample_address[%02x] = %04x\n", chan, m_sample_address[chan]);
}

template<int sel> u8 swp00_device::sample_address_r(offs_t offset)
{
	int chan = offset >> 1;
	return m_sample_address[chan] >> (8*sel);
}

void swp00_device::lfo_step_w(offs_t offset, u8 data)
{
	int chan = offset >> 1;
	if(m_lfo_step[chan] == data)
		return;
	m_stream->update();

	m_lfo_step[chan] = data;
	logerror("lfo_step[%02x] = %02x\n", chan, m_lfo_step[chan]);
}

u8 swp00_device::lfo_step_r(offs_t offset)
{
	int chan = offset >> 1;
	return m_lfo_step[chan];
}

void swp00_device::lfo_pmod_depth_w(offs_t offset, u8 data)
{
	int chan = offset >> 1;
	if(m_lfo_pmod_depth[chan] == data)
		return;
	m_stream->update();

	m_lfo_pmod_depth[chan] = data;
	logerror("lfo_pmod_depth[%02x] = %02x\n", chan, m_lfo_pmod_depth[chan]);
}

u8 swp00_device::lfo_pmod_depth_r(offs_t offset)
{
	int chan = offset >> 1;
	return m_lfo_pmod_depth[chan];
}

void swp00_device::keyon(int chan)
{
	m_stream->update();
	logerror("keyon %02x a=%02x/%02x d=%02x/%02x\n", chan, m_attack_speed[chan], m_attack_level[chan], m_decay_speed[chan], m_decay_level[chan]);
	m_lfo_phase[chan] = 0;
	m_sample_pos[chan] = -m_sample_start[chan] << 15;

	m_active[chan] = true;
	m_decay[chan] = false;
	m_decay_done[chan] = false;

	m_lpf_value[chan] = m_lpf_target_value[chan];
	m_lpf_timer[chan] = 0x4000000;
	m_lpf_ha[chan] = 0;
	m_lpf_hb[chan] = 0;

	m_glo_level_cur[chan] = m_glo_level[chan] << 4;
	m_pan_l[chan] = panmap[m_panning[chan] >> 4];
	m_pan_r[chan] = panmap[m_panning[chan] & 15];

	if(m_decay_speed[chan] & 0x80) {
		m_envelope_level[chan] = 0;
		m_decay[chan] = true;
	} else if((m_attack_speed[chan] & 0x80) || m_attack_level[chan])
		m_envelope_level[chan] = m_attack_level[chan] << 20;
	else
		m_envelope_level[chan] = 0x8000000;
}

template<int sel> void swp00_device::keyon_w(u8 data)
{
	for(int i=0; i < 8; i++)
		if(BIT(data, i))
			keyon(8*sel+i);
}

void swp00_device::intreg_w(offs_t offset, u8 data)
{
	m_stream->update();

	if(offset & 1)
		m_intreg[offset >> 1] = (m_intreg[offset >> 1] & 0xff00) | data;
	else
		m_intreg[offset >> 1] = (m_intreg[offset >> 1] & 0x00ff) | (data << 8);
	if(0)
		if(offset & 1)
			logerror("intreg[%02x] = %04x\n", offset >> 1, m_intreg[offset >> 1]);
}

u8 swp00_device::intreg_r(offs_t offset)
{
	if(offset & 1)
		return m_intreg[offset >> 1];
	else
		return m_intreg[offset >> 1] >> 8;
}

void swp00_device::fpreg_w(offs_t offset, u8 data)
{
	m_stream->update();

	if(offset & 1)
		m_fpreg[offset >> 1] = (m_fpreg[offset >> 1] & 0xff00) | data;
	else
		m_fpreg[offset >> 1] = (m_fpreg[offset >> 1] & 0x00ff) | (data << 8);
	if(0)
		if(offset & 1)
			logerror("fpreg[%03x] = %04x\n", offset >> 1, m_fpreg[offset >> 1]);
}

u8 swp00_device::fpreg_r(offs_t offset)
{
	if(offset & 1)
		return m_fpreg[offset >> 1];
	else
		return m_fpreg[offset >> 1] >> 8;
}

void swp00_device::waverom_access_w(u8 data)
{
	m_waverom_access = data;
}

u8 swp00_device::waverom_access_r()
{
	return 0x00; // non-zero = busy reading the rom
}

u8 swp00_device::waverom_val_r()
{
	u8 val = read_byte(m_sample_address[0x1f]);
	logerror("waverom read adr=%08x -> %02x\n", m_sample_address[0x1f], val);
	m_sample_address[0x1f] = (m_sample_address[0x1f] + 1) & 0xffffff;
	return val;
}

// Counters state access
u8 swp00_device::state_r()
{
	m_stream->update();

	//	logerror("state_r %x.%02x\n", m_state_adr >> 5, m_state_adr & 0x1f);
	int chan = m_state_adr & 0x1f;
	switch(m_state_adr & 0xe0) {
	case 0x00:  // lpf value
		return (m_lpf_value[chan] >> 20) | (m_lpf_done[chan] ? 0x80 : 0x00);

	case 0x40: { // Envelope state
		if(!m_active[chan])
			return 0xff;

		u8 vol;
		if(m_decay[chan] || m_attack_level[chan] || (m_attack_speed[chan] & 0x80))
			vol = m_envelope_level[chan] >> 22;
		else
			vol = 0;

		if(m_decay_done[chan])
			vol |= 0x40;
		if(m_decay[chan])
			vol |= 0x80;

		return vol;
	}

	case 0x60:   // global level
		return (m_glo_level_cur[chan] >> 6) | ((m_glo_level_cur[chan] == (m_glo_level[chan] << 4)) ? 0x80 : 0x00);

	case 0x80:   // panning l
		return (m_pan_l[chan] >> 6) | ((m_pan_l[chan] == panmap[m_panning[chan] >> 4]) ? 0x80 : 0x00);

	case 0xa0:   // panning r
		return (m_pan_r[chan] >> 6) | ((m_pan_r[chan] == panmap[m_panning[chan] & 15]) ? 0x80 : 0x00);
	}

	logerror("state %02x unsupported\n");
	return 0;
}

void swp00_device::state_adr_w(u8 data)
{
	m_state_adr = data;
}


// Catch-all

static u8 rr[0x20*0x40];

u8 swp00_device::snd_r(offs_t offset)
{
	if(1) {
		int chan = (offset >> 1) & 0x1f;
		int slot = ((offset >> 5) & 0x3e) | (offset & 1);
		std::string preg = "-";
#if 0
		if(slot >= 0x21 && slot <= 0x2b && (slot & 1))
			preg = util::string_format("fp%03x", (slot-0x21)/2 + 6*chan);
		else if(slot == 0x30 || slot == 0x31)
			preg = util::string_format("dt%02x", (slot-0x30) + 2*chan);
		else if(slot == 0x0e || slot == 0x0f)
			preg = util::string_format("ct%02x", (slot-0x0e) + 2*chan);
		else
#endif
			preg = util::string_format("%02x.%02x", chan, slot);
		if(offset != 1)
			logerror("snd_r [%03x] %-5s, %02x\n", offset, preg, rr[offset]);
	}
	return rr[offset];
}

void swp00_device::snd_w(offs_t offset, u8 data)
{
	//	if(rr[offset] == data)
	//		return;

	rr[offset] = data;

	int chan = (offset >> 1) & 0x1f;
	int slot = ((offset >> 5) & 0x3e) | (offset & 1);

	std::string preg = "-";
#if 0
	if(slot >= 0x21 && slot <= 0x2b && (slot & 1))
		preg = util::string_format("fp%03x", (slot-0x21)/2 + 6*chan);
	else if(slot == 0x0e || slot == 0x0f)
		preg = util::string_format("sy%02x", (slot-0x0e) + 2*chan);
	else if(slot == 0x30 || slot == 0x31)
		preg = util::string_format("dt%02x", (slot-0x30) + 2*chan);
	else if(slot == 0x38)
		preg = util::string_format("vl%02x", chan);
	else if(slot == 0x3e || slot == 0x3f)
		preg = util::string_format("lf%02x", (slot-0x3e) + 2*chan);
	else
#endif
		preg = util::string_format("%02x.%02x", chan, slot);

	if(offset == 1)
		return;

	logerror("snd_w [%03x] %-5s, %02x\n", offset, preg, data);
}



// Synthesis

void swp00_device::sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs)
{
	for(int i=0; i != outputs[0].samples(); i++) {
		s32 dry_l = 0, dry_r = 0;
		s32 rev   = 0;
		s32 cho_l = 0, cho_r = 0;
		s32 var_l = 0, var_r = 0;

		for(int chan = 0; chan != 32; chan++) {
			if(!m_active[chan])
				continue;

			u32 lfo_phase = m_lfo_phase[chan] >> 7;
			s32 lfo_p_phase  = lfo_phase ^ (m_lfo_step[chan] & 0x40 ? lfo_shape_centered_tri : lfo_shape_centered_saw)[lfo_phase >> 18];
			s32 lfo_fa_phase = lfo_phase ^ (m_lfo_step[chan] & 0x40 ? lfo_shape_offset_tri   : lfo_shape_offset_saw  )[lfo_phase >> 18];

			s16 val0, val1;
			u32 base_address = m_sample_address[chan];
			s32 spos = m_sample_pos[chan] >> 15;
			switch(m_sample_dec_and_format[chan] >> 6) {
			case 0: { // 16-bits linear
				offs_t adr = base_address + (spos << 1);
				val0 = read_word(adr);
				val1 = read_word(adr+1);
				break;
			}

			case 1: { // 12-bits linear
				offs_t adr = base_address + (spos >> 2)*6;
				switch(spos & 3) {
				case 0: { // .abc .... ....
					u16 w0 = read_word(adr);
					u16 w1 = read_word(adr+2);
					val0 = (w0 & 0x0fff) << 4;
					val1 = ((w0 & 0xf000) >> 8) | ((w1 & 0x00ff) << 8);
					break;
				}
				case 1: { // C... ..AB ....
					u16 w0 = read_word(adr);
					u16 w1 = read_word(adr+2);
					u16 w2 = read_word(adr+4);
					val0 = ((w0 & 0xf000) >> 8) | ((w1 & 0x00ff) << 8);
					val1 = ((w1 & 0xff00) >> 4) | ((w2 & 0x000f) << 12);
					break;
				}
				case 2: { // .... bc.. ...a
					u16 w1 = read_word(adr+2);
					u16 w2 = read_word(adr+4);
					val0 = ((w1 & 0xff00) >> 4) | ((w2 & 0x000f) << 12);
					val1 = w2 & 0xfff0;
					break;
				}
				case 3: { // .... .... ABC.
					u16 w2 = read_word(adr+4);
					u16 w3 = read_word(adr+6);
					val0 = w2 & 0xfff0;
					val1 = (w3 & 0x0fff) << 4;
					break;
				}
				}
				break;
			}

			case 2:   // 8-bits linear
				val0 = (read_byte(base_address + spos)     << 8);
				val1 = (read_byte(base_address + spos + 1) << 8);
				break;

			case 3:   // 8-bits logarithmic
				val0 = m_sample_log8[read_byte(base_address + spos)];
				val1 = m_sample_log8[read_byte(base_address + spos + 1)];
				break;
			}

			s32 mul = m_sample_pos[chan] & 0x7fff;
			s16 sample = (val1 * mul + val0 * (0x8000 - mul)) >> 15;

			s32 lpf_value = m_lpf_value[chan] + ((lfo_fa_phase * (m_lfo_famod_depth[chan] >> 5)) << (m_lfo_step[chan] & 0x40 ? 2 : 1));

			m_lpf_ha[chan] += lpffpapply(lpf_value, sample - 2*fpapply(m_lpf_feedback[chan], m_lpf_ha[chan]) - m_lpf_hb[chan]);
			m_lpf_hb[chan] += lpffpapply(lpf_value, m_lpf_ha[chan]);

			sample = m_lpf_hb[chan];

			s32 envelope_level;
			if(m_decay[chan] || m_attack_level[chan] || (m_attack_speed[chan] & 0x80))
				envelope_level = m_envelope_level[chan];
			else
				envelope_level = 0;

			s32 tremolo_level = (lfo_fa_phase * (m_lfo_famod_depth[chan] & 0x1f)) << ((m_lfo_step[chan] & 0x40) ? 3 : 2);

			dry_l += fpapply(envelope_level + (m_glo_level_cur[chan] << 16) + tremolo_level + (m_dry_level[chan] << 20) + (m_pan_l[chan] << 16), sample);
			dry_r += fpapply(envelope_level + (m_glo_level_cur[chan] << 16) + tremolo_level + (m_dry_level[chan] << 20) + (m_pan_r[chan] << 16), sample);
			rev   += fpapply(envelope_level + (m_glo_level_cur[chan] << 16) + tremolo_level + (m_rev_level[chan] << 20),                         sample);
			cho_l += fpapply(envelope_level + (m_glo_level_cur[chan] << 16) + tremolo_level + (m_cho_level[chan] << 20) + (m_pan_l[chan] << 16), sample);
			cho_r += fpapply(envelope_level + (m_glo_level_cur[chan] << 16) + tremolo_level + (m_cho_level[chan] << 20) + (m_pan_r[chan] << 16), sample);
			var_l += fpapply(envelope_level + (m_glo_level_cur[chan] << 16) + tremolo_level + (m_var_level[chan] << 20) + (m_pan_l[chan] << 16), sample);
			var_r += fpapply(envelope_level + (m_glo_level_cur[chan] << 16) + tremolo_level + (m_var_level[chan] << 20) + (m_pan_r[chan] << 16), sample);

			m_lfo_phase[chan] = (m_lfo_phase[chan] + m_global_step[0x20 + (m_lfo_step[chan] & 0x3f)]) & 0x7ffffff;

			u32 sample_increment = ((m_pitch[chan] & 0xfff) << (8 + (s16(m_pitch[chan]) >> 12))) >> 4;
			m_sample_pos[chan] += (sample_increment * (0x800 + ((lfo_p_phase * m_lfo_pmod_depth[chan]) >> (m_lfo_step[chan] & 0x40 ? 18 : 19)))) >> 11;
			if((m_sample_pos[chan] >> 15) >= m_sample_end[chan]) {
				if(!m_sample_end[chan])
					m_active[chan] = false;
				else {
					do
						m_sample_pos[chan] -= (m_sample_end[chan] << 15) | ((m_sample_dec_and_format[chan] & 0x3f) << 9);
					while((m_sample_pos[chan] >> 15) >= m_sample_end[chan]);
				}
			}

			if(m_lpf_speed[chan] & 0x80)
				m_lpf_done[chan] = istep(m_lpf_timer[chan], 0, m_global_step[m_lpf_speed[chan]] >> 1);
			else
				m_lpf_done[chan] = istep(m_lpf_value[chan], m_lpf_target_value[chan], m_global_step[m_lpf_speed[chan]] >> 1);

			istep(m_glo_level_cur[chan], m_glo_level[chan] << 4, 1);
			istep(m_pan_l[chan], panmap[m_panning[chan] >> 4], 1);
			istep(m_pan_r[chan], panmap[m_panning[chan] & 15], 1);

			if(m_decay[chan]) {
				if((m_decay_speed[chan] & 0x60) == 0x60)
					m_decay_done[chan] = fpstep(m_envelope_level[chan], m_decay_level[chan] << 20, decay_linear_step[m_decay_speed[chan] & 0x1f]);
				else
					m_decay_done[chan] = istep(m_envelope_level[chan], m_decay_level[chan] << 20, m_global_step[m_decay_speed[chan]] << 1);
				if(m_envelope_level[chan] & 0x8000000)
					m_active[chan] = false;

			} else if(m_attack_speed[chan] & 0x80)
				m_decay[chan] = fpstep(m_envelope_level[chan], 0, attack_linear_step[m_attack_speed[chan] & 0x7f]);
			else
				m_decay[chan] = istep(m_envelope_level[chan], 0, m_global_step[m_attack_speed[chan]] << 1);
		}

		outputs[0].put_int(i, dry_l, 32768);
		outputs[1].put_int(i, dry_r, 32768);
		(void)rev;
		(void)cho_l;
		(void)cho_r;
		(void)var_l;
		(void)var_r;
	}
}
