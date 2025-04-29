// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Yamaha SWP00, rompler/dsp combo

#include "emu.h"
#include "swp00.h"

/*

  Used in the MU50, the SWP00 is the combination of a rompler called
  AWM2 (Advanced Wave Memory 2) and an effects DSP called MEG
  (Multiple Effects Generator).  It is the simpler variant of those, a
  simplification and integration of the SWP20/SWD/MEG/EQ combo use in
  the MU80.

  Its clock is 33.9MHz and the output is at 44100Hz stereo (768 cycles
  per sample pair) per dac output.


    AWM2:

  The AWM2 is in charge of handling the individual channels.  It
  manages reading the rom, decoding the samples, applying volume and
  envelopes and lfos and filtering the result.  The channels are
  volume-modulated and summed into 7 outputs which are then processed
  by the MEG.

  As all the SWPs, the sound data can be four formats (8 bits, 12
  bits, 16 bits, and a 8-bits log format with roughly 10 bits of
  dynamic).  It's interesting to note that the 8-bits format is not
  used by the MU50.  The rom bus is 24 bits address and 8 bits data
  wide.  It applies a single, Chamberlin-configuration LPF to the
  sample data.  Envelopes are handled semi-automatically, and the
  final result volume-modulated (global volume, pan, tremolo, dispatch
  in dry/reverb/chorus/variation) in 7 output channels.


    MEG:

  The MEG in this case is an internal DSP with a fixed program in four
  selectable variants.  It has 192 steps of program, and can issue a
  memory access to the effects DRAM every 3 cycles.  The programs are
  internal and as far as we know not dumpable.  We managed a
  reimplementation though.

  The program does the effects "reverb", "chorus" and "variation" and
  mixing between all those.  The four variants only in practice impact
  the variation segment, in addresses 109-191 roughly.

  Each instruction is associated with a dynamically changeable 10-bit
  constant used as a fixed point value (either 1.9 or 3.7 depending on
  the instruction).  Every third instruction (pc multiple of 3) is
  also associated with a 16-bits offset for the potential memory
  access.


    Interface:

  The interface is 8-bits wide but would have wanted to be 16-bits, with
  11 address bits.  There are three address formats depending on the
  part of the chip one speaks to:
     000 0sss ssss  Global controls
     001 1ppp pppl  MEG, offsets (16-bits values, l=high/low byte, pc 00-bd, divided by 3)
     01p pppp pppl  MEG, constants (16-bits values, l=high/low byte, pc 00-bf)
     sss sscc cccs  AWM2, channel/slot combination (slot = 8-b and 20-37)
*/

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

const std::array<u8, 4> swp00_device::dpcm_offset = { 7, 6, 4, 0 };

bool swp00_device::istep(s32 &value, s32 limit, s32 step)
{
	//  fprintf(stderr, "istep(%x, %x, %x)\n", value, limit, step);
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

// sample is signed 24.8
s32 swp00_device::fpapply(s32 value, s32 sample)
{
	if(value >= 0x10000000)
		return 0;
	return (s64(sample) - ((s64(sample) * ((value >> 9) & 0x7fff)) >> 16)) >> (value >> 24);
}

// sample is signed 24.8
s32 swp00_device::lpffpapply(s32 value, s32 sample)
{
	return ((((value >> 7) & 0x7fff) | 0x8000) * s64(sample)) >> (31 - (value >> 22));
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
	save_item(NAME(m_meg_control));

	save_item(NAME(m_buffer_offset));
	save_item(NAME(m_rev_vol));
	save_item(NAME(m_cho_vol));
	save_item(NAME(m_var_vol));

	save_item(NAME(m_var_lfo_phase));
	save_item(NAME(m_var_lfo_h_1));
	save_item(NAME(m_var_lfo_h_2));
	save_item(NAME(m_var_lfo1a));
	save_item(NAME(m_var_lfo2a));
	save_item(NAME(m_var_lfo3a));
	save_item(NAME(m_var_lfo4a));

	save_item(NAME(m_var_filter_1));
	save_item(NAME(m_var_filter_2));
	save_item(NAME(m_var_filter_3));

	save_item(NAME(m_var_filter2_1));
	save_item(NAME(m_var_filter2_2a));
	save_item(NAME(m_var_filter2_2b));
	save_item(NAME(m_var_filter2_3a));
	save_item(NAME(m_var_filter2_3b));
	save_item(NAME(m_var_filter2_4));

	save_item(NAME(m_var_filterp_l_1));
	save_item(NAME(m_var_filterp_l_2));
	save_item(NAME(m_var_filterp_l_3));
	save_item(NAME(m_var_filterp_l_4));
	save_item(NAME(m_var_filterp_l_5));
	save_item(NAME(m_var_filterp_l_6));
	save_item(NAME(m_var_filterp_r_1));
	save_item(NAME(m_var_filterp_r_2));
	save_item(NAME(m_var_filterp_r_3));
	save_item(NAME(m_var_filterp_r_4));
	save_item(NAME(m_var_filterp_r_5));
	save_item(NAME(m_var_filterp_r_6));

	save_item(NAME(m_var_filter3_1));
	save_item(NAME(m_var_filter3_2));

	save_item(NAME(m_var_h1));
	save_item(NAME(m_var_h2));
	save_item(NAME(m_var_h3));
	save_item(NAME(m_var_h4));

	save_item(NAME(m_cho_lfo_phase));
	save_item(NAME(m_cho_filter_l_1));
	save_item(NAME(m_cho_filter_l_2));
	save_item(NAME(m_cho_filter_l_3));
	save_item(NAME(m_cho_filter_r_1));
	save_item(NAME(m_cho_filter_r_2));
	save_item(NAME(m_cho_filter_r_3));

	save_item(NAME(m_rev_filter_1));
	save_item(NAME(m_rev_filter_2));
	save_item(NAME(m_rev_filter_3));
	save_item(NAME(m_rev_hist_a));
	save_item(NAME(m_rev_hist_b));
	save_item(NAME(m_rev_hist_c));
	save_item(NAME(m_rev_hist_d));

	save_item(NAME(m_rev_buffer));
	save_item(NAME(m_cho_buffer));
	save_item(NAME(m_var_buffer));
	save_item(NAME(m_offset));
	save_item(NAME(m_const));
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
	save_item(NAME(m_sample_dpcm_and_format));
	save_item(NAME(m_sample_address));
	save_item(NAME(m_lfo_step));
	save_item(NAME(m_lfo_pmod_depth));

	save_item(NAME(m_lfo_phase));
	save_item(NAME(m_sample_pos));
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

	save_item(NAME(m_dpcm_current));
	save_item(NAME(m_dpcm_next));
	save_item(NAME(m_dpcm_address));
	save_item(NAME(m_dpcm_sum));

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

	// Delta-packed samples decompression.

	for(int i=0; i<128; i++) {
		s16 base = ((i & 0x1f) << (3+(i >> 5))) + (((1 << (i >> 5))-1) << 8);
		m_dpcm[i | 0x80] = - base;
		m_dpcm[i]        = + base;
	}
}

void swp00_device::device_reset()
{
	m_waverom_access = 0;
	m_waverom_val = 0;
	m_meg_control = 0;

	m_buffer_offset = 0;
	m_rev_vol = 0;
	m_cho_vol = 0;
	m_var_vol = 0;

	m_var_lfo_phase = 0;
	m_var_lfo_h_1 = 0;
	m_var_lfo_h_2 = 0;
	m_var_lfo1a = 0;
	m_var_lfo2a = 0;
	m_var_lfo3a = 0;
	m_var_lfo4a = 0;
	m_var_filter_1 = 0;
	m_var_filter_2 = 0;
	m_var_filter_3 = 0;
	m_var_filter2_1 = 0;
	m_var_filter2_2a = 0;
	m_var_filter2_2b = 0;
	m_var_filter2_3a = 0;
	m_var_filter2_3b = 0;
	m_var_filter2_4 = 0;
	m_var_filter3_1 = 0;
	m_var_filter3_2 = 0;
	m_var_filterp_l_1 = 0;
	m_var_filterp_l_2 = 0;
	m_var_filterp_l_3 = 0;
	m_var_filterp_l_4 = 0;
	m_var_filterp_l_5 = 0;
	m_var_filterp_l_6 = 0;
	m_var_filterp_r_1 = 0;
	m_var_filterp_r_2 = 0;
	m_var_filterp_r_3 = 0;
	m_var_filterp_r_4 = 0;
	m_var_filterp_r_5 = 0;
	m_var_filterp_r_6 = 0;

	m_var_h1 = 0;
	m_var_h2 = 0;
	m_var_h3 = 0;
	m_var_h4 = 0;

	m_cho_lfo_phase = 0;
	m_cho_filter_l_1 = 0;
	m_cho_filter_l_2 = 0;
	m_cho_filter_l_3 = 0;
	m_cho_filter_r_1 = 0;
	m_cho_filter_r_2 = 0;
	m_cho_filter_r_3 = 0;

	m_rev_filter_1 = 0;
	m_rev_filter_2 = 0;
	m_rev_filter_3 = 0;
	m_rev_hist_a = 0;
	m_rev_hist_b = 0;
	m_rev_hist_c = 0;
	m_rev_hist_d = 0;

	std::fill(m_rev_buffer.begin(), m_rev_buffer.end(), 0);
	std::fill(m_cho_buffer.begin(), m_cho_buffer.end(), 0);
	std::fill(m_var_buffer.begin(), m_var_buffer.end(), 0);
	std::fill(m_offset.begin(), m_offset.end(), 0);
	std::fill(m_const.begin(), m_const.end(), 0);
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
	std::fill(m_sample_dpcm_and_format.begin(), m_sample_dpcm_and_format.end(), 0);
	std::fill(m_sample_address.begin(), m_sample_address.end(), 0);
	std::fill(m_lfo_step.begin(), m_lfo_step.end(), 0);
	std::fill(m_lfo_pmod_depth.begin(), m_lfo_pmod_depth.end(), 0);

	std::fill(m_lfo_phase.begin(), m_lfo_phase.end(), 0);
	std::fill(m_sample_pos.begin(), m_sample_pos.end(), 0);
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

	std::fill(m_dpcm_current.begin(), m_dpcm_current.end(), false);
	std::fill(m_dpcm_next.begin(), m_dpcm_next.end(), false);
	std::fill(m_dpcm_address.begin(), m_dpcm_address.end(), false);
	std::fill(m_dpcm_sum.begin(), m_dpcm_sum.end(), 0);
}

void swp00_device::rom_bank_pre_change()
{
	m_stream->update();
}

void swp00_device::map(address_map &map)
{
	map(0x000, 0x7ff).rw(FUNC(swp00_device::snd_r), FUNC(swp00_device::snd_w));

	// 00-01: control

	rchan(map, 0x08).w(FUNC(swp00_device::slot8_w)); // always 80
	rchan(map, 0x09).w(FUNC(swp00_device::slot9_w)); // always 00
	rchan(map, 0x0a).rw(FUNC(swp00_device::sample_start_r<1>), FUNC(swp00_device::sample_start_w<1>));
	rchan(map, 0x0b).rw(FUNC(swp00_device::sample_start_r<0>), FUNC(swp00_device::sample_start_w<0>));

	// 0c-0f: meg offsets
	// 10-1b: meg values

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
	rchan(map, 0x30).rw(FUNC(swp00_device::sample_dpcm_and_format_r), FUNC(swp00_device::sample_dpcm_and_format_w));
	rchan(map, 0x31).rw(FUNC(swp00_device::sample_address_r<2>), FUNC(swp00_device::sample_address_w<2>));
	rchan(map, 0x32).rw(FUNC(swp00_device::sample_address_r<1>), FUNC(swp00_device::sample_address_w<1>));
	rchan(map, 0x33).rw(FUNC(swp00_device::sample_address_r<0>), FUNC(swp00_device::sample_address_w<0>));
	rchan(map, 0x34).rw(FUNC(swp00_device::pitch_r<1>), FUNC(swp00_device::pitch_w<1>));
	rchan(map, 0x35).rw(FUNC(swp00_device::pitch_r<0>), FUNC(swp00_device::pitch_w<0>));
	rchan(map, 0x36).rw(FUNC(swp00_device::sample_end_r<1>), FUNC(swp00_device::sample_end_w<1>));
	rchan(map, 0x37).rw(FUNC(swp00_device::sample_end_r<0>), FUNC(swp00_device::sample_end_w<0>));

	rctrl(map, 0x00); // 01 at startup
	rctrl(map, 0x01).rw(FUNC(swp00_device::state_r), FUNC(swp00_device::state_adr_w));
	rctrl(map, 0x02).rw(FUNC(swp00_device::waverom_access_r), FUNC(swp00_device::waverom_access_w));
	rctrl(map, 0x03).r(FUNC(swp00_device::waverom_val_r));
	rctrl(map, 0x04).rw(FUNC(swp00_device::meg_control_r), FUNC(swp00_device::meg_control_w));
	rctrl(map, 0x08).w(FUNC(swp00_device::keyon_w<3>));
	rctrl(map, 0x09).w(FUNC(swp00_device::keyon_w<2>));
	rctrl(map, 0x0a).w(FUNC(swp00_device::keyon_w<1>));
	rctrl(map, 0x0b).w(FUNC(swp00_device::keyon_w<0>));
	rctrl(map, 0x0c); // 00 at startup
	rctrl(map, 0x0d); // 00 at startup
	rctrl(map, 0x0e); // 00 at startup

	map(0x180, 0x1ff).rw(FUNC(swp00_device::offset_r), FUNC(swp00_device::offset_w));
	map(0x200, 0x37f).rw(FUNC(swp00_device::const_r), FUNC(swp00_device::const_w));
}


// Voice control

void swp00_device::slot8_w(offs_t offset, u8 data)
{
	if(data == 0x80)
		return;
	logerror("slot8[%02x] = %02x\n", offset >> 1, data);
}

void swp00_device::slot9_w(offs_t offset, u8 data)
{
	if(data == 0x00)
		return;
	logerror("slot9[%02x] = %02x\n", offset >> 1, data);
}

template<int sel> void swp00_device::lpf_info_w(offs_t offset, u8 data)
{
	int chan = offset >> 1;
	u16 old = m_lpf_info[chan];
	m_stream->update();

	m_lpf_info[chan] = (m_lpf_info[chan] & ~(0xff << (8*sel))) | (data << (8*sel));
	if(m_lpf_info[chan] == old)
		return;

	//  if(!sel)
	//      logerror("lpf_info[%02x] = %04x\n", chan, m_lpf_info[chan]);

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
	//  logerror("lpf_speed[%02x] = %02x\n", chan, m_lpf_speed[chan]);
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
	//  logerror("lfo_famod_depth[%02x] = %02x\n", chan, m_lfo_famod_depth[chan]);
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
	//  logerror("rev_level[%02x] = %02x\n", chan, m_rev_level[chan]);
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
	//  logerror("dry_level[%02x] = %02x\n", chan, m_dry_level[chan]);
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
	//  logerror("cho_level[%02x] = %02x\n", chan, m_cho_level[chan]);
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
	//  logerror("var_level[%02x] = %02x\n", chan, m_var_level[chan]);
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
	//  logerror("glo_level[%02x] = %02x\n", chan, m_glo_level[chan]);
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
	//  logerror("panning[%02x] = %02x\n", chan, m_panning[chan]);
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
	//  if(!sel)
	//      logerror("pitch[%02x] = %04x\n", chan, m_pitch[chan]);
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
	//  if(!sel)
	//      logerror("sample_start[%02x] = %04x\n", chan, m_sample_start[chan]);
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
	//  if(!sel)
	//      logerror("sample_end[%02x] = %04x\n", chan, m_sample_end[chan]);
}

template<int sel> u8 swp00_device::sample_end_r(offs_t offset)
{
	int chan = offset >> 1;
	return m_sample_end[chan] >> (8*sel);
}

void swp00_device::sample_dpcm_and_format_w(offs_t offset, u8 data)
{
	int chan = offset >> 1;
	m_stream->update();

	m_sample_dpcm_and_format[chan] = data;
	//  logerror("sample_dpcm_and_format[%02x] = %02x\n", chan, m_sample_dpcm_and_format[chan]);
}

u8 swp00_device::sample_dpcm_and_format_r(offs_t offset)
{
	int chan = offset >> 1;
	return m_sample_dpcm_and_format[chan];
}

template<int sel> void swp00_device::sample_address_w(offs_t offset, u8 data)
{
	int chan = offset >> 1;
	m_stream->update();

	m_sample_address[chan] = (m_sample_address[chan] & ~(0xff << (8*sel))) | (data << (8*sel));
	//  if(!sel)
	//      logerror("sample_address[%02x] = %04x\n", chan, m_sample_address[chan]);
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
	//  logerror("lfo_step[%02x] = %02x\n", chan, m_lfo_step[chan]);
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
	//  logerror("lfo_pmod_depth[%02x] = %02x\n", chan, m_lfo_pmod_depth[chan]);
}

u8 swp00_device::lfo_pmod_depth_r(offs_t offset)
{
	int chan = offset >> 1;
	return m_lfo_pmod_depth[chan];
}

void swp00_device::keyon(int chan)
{
	m_stream->update();
	logerror("keyon %02x a=%02x/%02x d=%02x/%02x glo=%02x pan=%02x [%x %x %x %x]\n", chan, m_attack_speed[chan], m_attack_level[chan], m_decay_speed[chan], m_decay_level[chan], m_glo_level[chan], m_panning[chan], m_sample_start[chan], m_sample_end[chan], m_sample_address[chan], m_sample_dpcm_and_format[chan]);
	m_lfo_phase[chan] = 0;
	m_sample_pos[chan] = -m_sample_start[chan] << 15;

	m_active[chan] = true;
	m_decay[chan] = false;
	m_decay_done[chan] = false;

	m_dpcm_current[chan] = 0;
	m_dpcm_next[chan] = 0;
	m_dpcm_address[chan] = m_sample_address[chan] - m_sample_start[chan];
	m_dpcm_sum[chan] = 0;

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

void swp00_device::offset_w(offs_t offset, u8 data)
{
	m_stream->update();

	if(offset & 1)
		m_offset[offset >> 1] = (m_offset[offset >> 1] & 0xff00) | data;
	else
		m_offset[offset >> 1] = (m_offset[offset >> 1] & 0x00ff) | (data << 8);
	if(0)
		if(offset & 1)
			logerror("offset[%02x] = %04x\n", 3*(offset >> 1), m_offset[offset >> 1]);
}

u8 swp00_device::offset_r(offs_t offset)
{
	if(offset & 1)
		return m_offset[offset >> 1];
	else
		return m_offset[offset >> 1] >> 8;
}

void swp00_device::const_w(offs_t offset, u8 data)
{
	m_stream->update();

	if(offset & 1)
		m_const[offset >> 1] = (m_const[offset >> 1] & 0xff00) | data;
	else
		m_const[offset >> 1] = (m_const[offset >> 1] & 0x00ff) | (data << 8);
	if(0)
		if(offset & 1)
			logerror("const[%02x] = %04x\n", offset >> 1, m_const[offset >> 1]);
}

u8 swp00_device::const_r(offs_t offset)
{
	if(offset & 1)
		return m_const[offset >> 1];
	else
		return m_const[offset >> 1] >> 8;
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

void swp00_device::meg_control_w(u8 data)
{
	m_meg_control = data;
	logerror("meg_control %02x (variation %x, %s)\n", m_meg_control, m_meg_control >> 6, m_meg_control & 2 ? "mute" : "on");
}

u8 swp00_device::meg_control_r()
{
	return m_meg_control;
}

// Counters state access
u8 swp00_device::state_r()
{
	m_stream->update();

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

u8 swp00_device::snd_r(offs_t offset)
{
	logerror("snd_r [%03x]\n", offset);
	return 0;
}

void swp00_device::snd_w(offs_t offset, u8 data)
{
	logerror("snd_w [%03x] %02x\n", offset, data);
}



// Synthesis

s32 swp00_device::rext(int reg) const
{
	s32 val = m_const[reg] & 0x3ff;
	if(val > 0x200) // Not 100% a real 2-complement fixed-point, e.g. the max value is positive, not negative
		val |= 0xfffffc00;
	return val;
}

s32 swp00_device::m7v(s32 value, s32 mult)
{
	return (s64(value) * mult) >> 7;
}

s32 swp00_device::m7(s32 value, int reg) const
{
	return m7v(value, rext(reg));
}

s32 swp00_device::m9v(s32 value, s32 mult)
{
	return (s64(value) * mult) >> 9;
}

s32 swp00_device::m9(s32 value, int reg) const
{
	return m9v(value, rext(reg));
}

template<size_t size> swp00_device::delay_block<size>::delay_block(swp00_device *swp, std::array<s32, size> &buffer) :
	m_swp(swp),
	m_buffer(buffer)
{
}

template<size_t size> s32 swp00_device::delay_block<size>::r(int offreg) const
{
	return m_buffer[(m_swp->m_buffer_offset + m_swp->m_offset[offreg/3]) & (size - 1)];
}

template<size_t size> void swp00_device::delay_block<size>::w(int offreg, s32 value) const
{
	m_buffer[(m_swp->m_buffer_offset + m_swp->m_offset[offreg/3]) & (size - 1)] = value;
}

template<size_t size> s32 swp00_device::delay_block<size>::rlfo(int offreg, u32 phase, s32 delta_phase, int levelreg) const
{
	// Phase is on 23 bits
	// Delta phase is on 10 bits shifts for a maximum of a full period (e.g. left shift of 13)
	// Phase is wrapped into a triangle on 22 bits
	// Level register is 10 bits where 1 = 4 samples of offset, for a maximum of 4096 samples

	u32 lfo_phase = lfo_wrap(phase, delta_phase);

	// Offset is 12.22
	u64 lfo_offset = lfo_phase * m_swp->rext(levelreg);
	u32 lfo_i_offset = lfo_offset >> 22;
	s32 lfo_i_frac = lfo_offset & 0x3fffff;

	// Uses in reality offreg and offreg+3 (which are offset by 1)
	u32 pos = m_swp->m_buffer_offset + m_swp->m_offset[offreg/3] + lfo_i_offset;
	s32 val0 = m_buffer[pos & (size - 1)];
	s32 val1 = m_buffer[(pos + 1) & (size - 1)];

	//  fprintf(stderr, "lfo %02x %x %x\n", offreg, val0, val1);
	return s32((val1 * s64(lfo_i_frac) + val0 * s64(0x400000 - lfo_i_frac)) >> 22);
}

template<size_t size> s32 swp00_device::delay_block<size>::rlfo2(int offreg, s32 offset) const
{
	// Offset is 12.11
	u32 lfo_i_offset = offset >> 11;
	s32 lfo_i_frac = offset & 0x7ff;

	// Uses in reality offreg and offreg+3 (which are offset by 1)
	u32 pos = m_swp->m_buffer_offset + m_swp->m_offset[offreg/3] + lfo_i_offset;
	s32 val0 = m_buffer[pos & (size - 1)];
	s32 val1 = m_buffer[(pos + 1) & (size - 1)];

	//  fprintf(stderr, "lfo %02x %x %x\n", offreg, val0, val1);
	return s32((val1 * s64(lfo_i_frac) + val0 * s64(0x800 - lfo_i_frac)) >> 11);
}

s32 swp00_device::lfo_get_step(int reg) const
{
	u32 e = (m_const[reg] >> 7) & 7;
	return (m_const[reg] & 0x7f) << (e == 7 ? 15 : e);
}

void swp00_device::lfo_step(u32 &phase, int reg) const
{
	phase = (phase + lfo_get_step(reg)) & 0x7fffff;
}

s32 swp00_device::lfo_saturate(s32 phase)
{
	if(phase < -0x400000)
		return -0x400000;
	if(phase >= 0x400000)
		return 0x3fffff;
	return phase;
}

u32 swp00_device::lfo_wrap(s32 phase, s32 delta_phase)
{
	s32 lfo_phase = (phase - (delta_phase << 13)) & 0x7fffff;
	if(lfo_phase & 0x400000)
		lfo_phase ^= 0x7fffff;
	return lfo_phase;
}

void swp00_device::filtered_lfo_step(s32 &position, s32 phase, int deltareg, int postdeltareg, int scalereg, int feedbackreg)
{
	s32 phase1 = lfo_saturate((deltareg == -1 ? phase : lfo_wrap(phase, deltareg)) - (rext(postdeltareg) << 13));
	s64 phase2 = s64(lfo_get_step(scalereg)) * phase1 + s64(0x400000 - lfo_get_step(feedbackreg)) * position;
	position = phase2 >> 22;
}

s32 swp00_device::alfo(u32 phase, s32 delta_phase, int levelreg, int offsetreg, bool sub) const
{
	u32 lfo_phase = lfo_wrap(phase, delta_phase);
	s32 offset = rext(offsetreg);
	if(sub)
		offset = -offset;
	s32 base = s32((s64(lfo_phase) * rext(levelreg)) >> 19) + (offset << 3);
	s32 bamp = ((base & 0x1ff) | 0x200) << ((base >> 9) & 15);
	bamp >>= 8;
	if(bamp <= -0x200)
		bamp = -0x1ff;
	else if(bamp >= 0x200)
		bamp = 0x200;
	return bamp;
}

s32 swp00_device::lfo_mod(s32 phase, int scalereg) const
{
	return (m9(phase, scalereg) >> 13) + 0x200;
}

s32 swp00_device::lfo_scale(s32 phase, int scalereg) const
{
	return lfo_saturate((phase - (rext(scalereg) << 13)) * 4);
}

s32 swp00_device::lfo_wrap_reg(s32 phase, int deltareg) const
{
	return lfo_wrap(phase, rext(deltareg));
}

s32 swp00_device::sx(int reg) const
{
	s32 mult = m_const[reg];
	if(mult & 0x200)
		mult |= 0xfffffc00;
	return mult;
}

double swp00_device::sx7(int reg) const
{
	return sx(reg) / 128.0;
}

double swp00_device::sx9(int reg) const
{
	return sx(reg) / 512.0;
}

s32 swp00_device::saturate(s32 value)
{
	if(value <= -0x20000)
		return -0x20000;
	else if(value > 0x1ffff)
		return 0x1ffff;
	else
		return value;
}

double v2f2(s32 value)
{
	return (1.0 - (value & 0xffffff) / 33554432.0) / (1 << (value >> 24));
}

void swp00_device::sound_stream_update(sound_stream &stream)
{
	const delay_block brev(this, m_rev_buffer);
	const delay_block bcho(this, m_cho_buffer);
	const delay_block bvar(this, m_var_buffer);

	for(int i=0; i != stream.samples(); i++) {
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
			switch(m_sample_dpcm_and_format[chan] >> 6) {
			case 0: { // 16-bits linear
				offs_t adr = base_address + (spos << 1);
				val0 = read_word(adr);
				val1 = read_word(adr+2);
				break;
			}

			case 1: { // 12-bits linear
				offs_t adr = base_address + (spos >> 2)*6;
				switch(spos & 3) {
				case 0: { // Cabc ..AB .... ....
					u16 w0 = read_word(adr);
					u16 w1 = read_word(adr+2);
					val0 = (w0 & 0x0fff) << 4;
					val1 = ((w0 & 0xf000) >> 8) | ((w1 & 0x00ff) << 8);
					break;
				}
				case 1: { // c... BCab ...A ....
					u16 w0 = read_word(adr);
					u16 w1 = read_word(adr+2);
					u16 w2 = read_word(adr+4);
					val0 = ((w0 & 0xf000) >> 8) | ((w1 & 0x00ff) << 8);
					val1 = ((w1 & 0xff00) >> 4) | ((w2 & 0x000f) << 12);
					break;
				}
				case 2: { // .... bc.. ABCa ....
					u16 w1 = read_word(adr+2);
					u16 w2 = read_word(adr+4);
					val0 = ((w1 & 0xff00) >> 4) | ((w2 & 0x000f) << 12);
					val1 = w2 & 0xfff0;
					break;
				}
				case 3: { // .... .... abc. .ABC
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

			case 3: { // 8-bits delta-pcm
				u8 offset = dpcm_offset[m_sample_dpcm_and_format[chan] & 3];
				u8 scale = (m_sample_dpcm_and_format[chan] >> 2) & 7;
				u32 target_address = base_address + spos + 1;
				while(m_dpcm_address[chan] <= target_address) {
					m_dpcm_current[chan] = m_dpcm_next[chan];
					m_dpcm_sum[chan] += m_dpcm[read_byte(m_dpcm_address[chan])] - offset;
					s32 sample = (m_dpcm_sum[chan] << scale) >> 3;
					m_dpcm_address[chan] ++;
					if(sample < -0x8000)
						sample = -0x8000;
					else if(sample > 0x7fff)
						sample = 0x7fff;
					m_dpcm_next[chan] = sample;
				}
				val0 = m_dpcm_current[chan];
				val1 = m_dpcm_next[chan];
				break;
			}
			}

			s32 mul = m_sample_pos[chan] & 0x7fff;
			s32 sample = (val1 * mul + val0 * (0x8000 - mul)) >> 7;

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
					s32 prev = m_sample_pos[chan];
					do
						m_sample_pos[chan] -= m_sample_end[chan] << 15;
					while((m_sample_pos[chan] >> 15) >= m_sample_end[chan]);
					m_dpcm_address[chan] += (m_sample_pos[chan] >> 15) - (prev >> 15);
					m_dpcm_sum[chan] = 0;
				}
			}

			if(m_lpf_speed[chan] & 0x80)
				m_lpf_done[chan] = istep(m_lpf_timer[chan], 0, m_global_step[m_lpf_speed[chan] & 0x7f] >> 1);
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

		dry_l >>= 8;
		dry_r >>= 8;
		rev   >>= 8;
		cho_l >>= 8;
		cho_r >>= 8;
		var_l >>= 8;
		var_r >>= 8;


		// Variation block
		//   Update the output volume
		m_var_vol = m9(m_var_vol, 0xbd) + m_const[0xbc];

		//   Scale the input
		var_l = m7(var_l, 0x04);
		var_r = m7(var_r, 0x07);

		//   Split depending on the variant selected
		s32 var_out_l = 0, var_out_r = 0;

		switch(m_meg_control & 0xc0) {
		case 0x00: {
			// Used by:
			// - 2-band EQ
			// - Auto Pan
			// - Celeste
			// - Chorus
			// - Delays
			// - Flanger
			// - Rotary Speaker
			// - Symphonic
			// - Tremolo

			// Two stages of filtering
			s32 var_filter_l_2 = m7(m_var_filter_l_1, 0x7e) + m7(var_l, 0x7f)          + m9(m_var_filter_l_2, 0x80);
			s32 var_filtered_l = m7(m_var_filter_l_2, 0xa7) + m7(var_filter_l_2, 0xa8) + m9(m_var_filter_l_3, 0xa9);

			m_var_filter_l_1 = var_l;
			m_var_filter_l_2 = var_filter_l_2;
			m_var_filter_l_3 = var_filtered_l;

			s32 var_filter_r_2 = m7(m_var_filter_r_1, 0x98) + m7(var_r, 0x99)          + m9(m_var_filter_r_2, 0x9a);
			s32 var_filtered_r = m7(m_var_filter_r_2, 0x9b) + m7(var_filter_r_2, 0x9c) + m9(m_var_filter_r_3, 0x9d);

			m_var_filter_r_1 = var_r;
			m_var_filter_r_2 = var_filter_r_2;
			m_var_filter_r_3 = var_filtered_r;

			// Rest is like, complex and stuff
			lfo_step(m_var_lfo_phase, 0x77);
			s32 var_lfo_phase_2 = m7(m7(m_var_lfo_phase, 0x6d), 0x70) & 0x7fffff;

			filtered_lfo_step(m_var_lfo1a, m_var_lfo_phase, 0x6e, 0x6f, 0x72, 0x71);
			filtered_lfo_step(m_var_lfo2a, m_var_lfo_phase, 0x79, 0x7a, 0x7c, 0x7b);
			filtered_lfo_step(m_var_lfo3a, m_var_lfo_phase, 0x88, 0x89, 0x8b, 0x8a);

			s32 lfo1b = lfo_scale(m_var_lfo1a, 0x73);
			s32 lfo2b = lfo_scale(m_var_lfo1a, 0x7d);
			s32 lfo3b = lfo_scale(m_var_lfo1a, 0x8c);

			s32 lfo1c = lfo_wrap_reg(var_lfo_phase_2, 0x74);
			s32 lfo2c = lfo_wrap_reg(var_lfo_phase_2, 0x84);
			s32 lfo3c = lfo_wrap_reg(var_lfo_phase_2, 0x8d);

			filtered_lfo_step(m_var_lfo4a, lfo3c, -1, 0x8e, 0x90, 0x8f);
			s32 lfo4b = lfo_scale(m_var_lfo4a, 0x91);

			s32 tap1 = bvar.rlfo2(0x78, m9(lfo1b, 0x75) + m9(lfo1c, 0x76));
			s32 tap2 = bvar.rlfo2(0x87, m9(lfo2b, 0x85) + m9(lfo2c, 0x86));
			s32 tap3 = bvar.rlfo2(0x99, m9(lfo3b, 0x95) + m9(lfo3c, 0x96));
			s32 tap4 = bvar.rlfo2(0xa8, m9(lfo4b, 0xa5));

			s32 mod1 = lfo_mod(lfo1b, 0x83);
			s32 mod2 = lfo_mod(lfo2b, 0x94);
			s32 mod3 = lfo_mod(lfo3b, 0xa4);

			m_var_lfo_h_1 = m9(m_var_lfo_h_1, 0x9e) + m9(tap1, 0x9f);
			m_var_lfo_h_2 = m9(m_var_lfo_h_2, 0xa0) + m9(tap1, 0xa1);

			bvar.w(0xae, var_filtered_l + m9(var_filtered_r, 0xaa) + m9(m_var_lfo_h_1, 0xab) + m9(m_var_lfo_h_2, 0xac));
			bvar.w(0xb1,                  m9(var_filtered_r, 0xad) + m9(m_var_lfo_h_1, 0xae) + m9(m_var_lfo_h_2, 0xaf));

			var_out_l = m9(var_filtered_l, 0xb2) + m9(var_filtered_r, 0xb3) + m9(m9v(tap2, mod1), 0xb4) + m9(m9v(tap3, mod3), 0xb5) + m9(tap4, 0xb6);
			var_out_r = m9(var_filtered_l, 0xb7) + m9(var_filtered_r, 0xb8) + m9(m9v(tap2, mod2), 0xb9) + m9(m9v(tap3, mod3), 0xba) + m9(tap4, 0xbb);

			break;
		}

		case 0x40: {
			// Used by:
			// - Phaser

			// Two stages of filtering
			s32 var_filter_l_2 = m7(m_var_filter_l_1, 0x6d) + m7(var_l, 0x6e)          + m9(m_var_filter_l_2, 0x6f);
			s32 var_filtered_l = m7(m_var_filter_l_2, 0x70) + m7(var_filter_l_2, 0x71) + m9(m_var_filter_l_3, 0x72);

			m_var_filter_l_1 = var_l;
			m_var_filter_l_2 = var_filter_l_2;
			m_var_filter_l_3 = var_filtered_l;
			s32 var_filter_r_2 = m7(m_var_filter_r_1, 0x73) + m7(var_r, 0x74)          + m9(m_var_filter_r_2, 0x75);
			s32 var_filtered_r = m7(m_var_filter_r_2, 0x76) + m7(var_filter_r_2, 0x77) + m9(m_var_filter_r_3, 0x78);

			m_var_filter_r_1 = var_r;
			m_var_filter_r_2 = var_filter_r_2;
			m_var_filter_r_3 = var_filtered_r;

			// A very funky amplitude lfo with a lot of stages
			s32 var_raw_l = m9(m_var_filterp_l_4, 0x7b) + m9(m_var_filterp_l_5, 0x7c) + m9(m_var_filterp_l_6, 0x7d);
			s32 var_raw_r = m9(m_var_filterp_r_4, 0x7e) + m9(m_var_filterp_r_5, 0x7f) + m9(m_var_filterp_r_6, 0x80);

			s32 var_o_l = m9(var_raw_l, 0xa3) + m9(m_var_filterp_r_3, 0xa4) + m9(m_var_filterp_r_5, 0xa5);
			s32 var_o_r = m9(var_raw_r, 0xa7);

			lfo_step(m_var_lfo_phase, 0x79);
			s32 alfo_l = 0x200 - alfo(m_var_lfo_phase, 0,             0x83, 0x82, false);
			s32 alfo_r = 0x200 - alfo(m_var_lfo_phase, m_const[0x9c], 0x9e, 0x9d, false);

			s32 var_l_1 = m9(var_filtered_l, 0x84) + m9(var_filtered_r, 0x85) + m9(var_raw_l, 0x86) + m9(var_raw_r, 0x87);
			s32 var_l_2 = m_var_filterp_l_1 + m9v(m_var_filterp_l_2 - var_l_1, alfo_l);
			m_var_filterp_l_1 = var_l_1;
			s32 var_l_3 = m_var_filterp_l_2 + m9v(m_var_filterp_l_3 - var_l_2, alfo_l);
			m_var_filterp_l_2 = var_l_2;
			s32 var_l_4 = m_var_filterp_l_3 + m9v(m_var_filterp_l_4 - var_l_3, alfo_l);
			m_var_filterp_l_3 = var_l_3;
			s32 var_l_5 = m_var_filterp_l_4 + m9v(m_var_filterp_l_5 - var_l_4, alfo_l);
			m_var_filterp_l_4 = var_l_4;
			m_var_filterp_l_6 = m_var_filterp_l_5 + m9v(m_var_filterp_l_6 - var_l_5, alfo_l);
			m_var_filterp_l_5 = var_l_5;

			s32 var_r_1 = m9(var_filtered_r, 0x9f) + m9(var_raw_l, 0xa0) + m9(var_raw_r, 0xa1);
			s32 var_r_2 = m_var_filterp_r_1 + m9v(m_var_filterp_r_2 - var_r_1, alfo_r);
			m_var_filterp_r_1 = var_r_1;
			s32 var_r_3 = m_var_filterp_r_2 + m9v(m_var_filterp_r_3 - var_r_2, alfo_r);
			m_var_filterp_r_2 = var_r_2;
			s32 var_r_4 = m_var_filterp_r_3 + m9v(m_var_filterp_r_4 - var_r_3, alfo_r);
			m_var_filterp_r_3 = var_r_3;
			s32 var_r_5 = m_var_filterp_r_4 + m9v(m_var_filterp_r_5 - var_r_4, alfo_r);
			m_var_filterp_r_4 = var_r_4;
			m_var_filterp_r_6 = m_var_filterp_r_5 + m9v(m_var_filterp_r_6 - var_r_5, alfo_r);
			m_var_filterp_r_5 = var_r_5;

			var_out_l = var_o_l + m9(var_filtered_l, 0xa2);
			var_out_r = var_o_r + m9(var_filtered_r, 0xa6);
			break;
		}

		case 0x80: {
			// Used by:
			// - 3-band EQ
			// - Amp simulation
			// - Distortion
			// - Gating

			// Compute a center value
			s32 var_m = m9(var_l, 0x6d) + m9(var_r, 0x6e);

			// Two stages of filtering on the center value
			s32 var_filter_2 = m7(m_var_filter_1, 0x6f) + m7(var_m, 0x70)        + m9(m_var_filter_2, 0x71);
			s32 var_filtered = m7(m_var_filter_2, 0x72) + m7(var_filter_2, 0x73) + m9(m_var_filter_3, 0x74);

			m_var_filter_1 = var_m;
			m_var_filter_2 = var_filter_2;
			m_var_filter_3 = var_filtered;

			// Gating/ER reverb injection with some filtering
			bvar.w(0x7e, m9(bvar.r(0x6c), 0x7b) + m9(var_m, 0x7c));
			s32 tap0 = m7(bvar.r(0x6c), 0x7e) + m7(var_m, 0x7f);
			bvar.w(0x84, m9(bvar.r(0x78), 0x81) + m9(tap0, 0x82));

			s32 var_f3_1 = bvar.r(0x6f);
			s32 var_f3_2 = m7(m_var_filter2_1, 0x77) + m7(var_f3_1, 0x78) + m9(m_var_filter3_2, 0x79);
			bvar.w(0x87, m7(bvar.r(0x78), 0x84) + m7(tap0, 0x85) + m9(var_f3_2, 0x86));

			m_var_filter3_1 = var_f3_1;
			m_var_filter3_2 = var_f3_2;

			// Multi-tap on reverb
			s32 tap1 = m9(bvar.r(0x6f), 0x99) + m9(bvar.r(0x72), 0x9a) + m9(bvar.r(0x75), 0x9b) + m9(bvar.r(0x8d), 0x9c) + m9(bvar.r(0x90), 0x9d) + m9(bvar.r(0x93), 0x9e) + m9(bvar.r(0x96), 0x9f);
			s32 tap2 = m9(bvar.r(0x9f), 0xb4) + m9(bvar.r(0xa2), 0xb5) + m9(bvar.r(0xa5), 0xb6) + m9(bvar.r(0xa8), 0xb7) + m9(bvar.r(0xab), 0xb8) + m9(bvar.r(0xae), 0xb9) + m9(bvar.r(0xb1), 0xba);

			bvar.w(0xb7, tap1);
			bvar.w(0xba, tap2);

			s32 tap2b = tap2 + m9(brev.r(0xb4), 0xbb);
			bvar.w(0x8a, m9(bvar.r(0x7b), 0x88) + m9(tap2b, 0x89));
			s32 var_gate_l = m7(bvar.r(0x7b), 0x8b) + m7(tap2b, 0x8c);

			s32 tap1b = tap1 + m9(brev.r(0x99), 0xa0);
			bvar.w(0x9c, m9(bvar.r(0x81), 0x8e) + m9(tap1b, 0x8f));
			s32 var_gate_r = m7(bvar.r(0x7b), 0x8b) + m7(tap1b, 0x8c);

			// Distortion stage
			s32 dist1 = saturate(m7(var_filtered, 0x76));
			s32 dist2 = saturate(m7(dist1,        0x83));
			s32 dist3 = saturate(m7(dist2,        0x87));
			s32 dist4 = saturate(m7(dist3,        0x8a));
			s32 dist5 = saturate(m7(dist4,        0x8d));
			s32 dist6 = saturate(m7(dist5,        0x90));
			s32 disto = m9(m9(dist1, 0x91) + m9(dist2, 0x92) + m9(dist3, 0x93) + m9(dist4, 0x94) + m9(dist5, 0x95) + m9(dist6, 0x96), 0xa1);

			// Filtering again, 3 stages
			s32 var_f2_2 = m7(m_var_filter2_1, 0xa2) + m7(disto, 0xa3) + m9(m_var_filter2_2a, 0xa4);
			s32 var_f2_3 = m7(m_var_filter2_3b, 0xa5) + m7(m_var_filter2_3a, 0xa6) + m7(m_var_filter2_2b, 0xa7) + m7(m_var_filter2_2a, 0xa8) + m7(var_f2_2, 0xa9);
			s32 var_f2_4 = m7(m_var_filter2_3a, 0xaa) + m7(var_f2_3, 0xab) + m9(m_var_filter2_4, 0xac);

			m_var_filter2_1 = disto;
			m_var_filter2_2b = m_var_filter2_2a;
			m_var_filter2_2a = var_f2_2;
			m_var_filter2_3b = m_var_filter2_3a;
			m_var_filter2_3a = var_f2_3;
			m_var_filter2_4 = var_f2_4;

			// Mix in both paths
			var_out_l = m9(var_l, 0xad) + m9(var_gate_l, 0xaf) + m9(var_f2_4, 0xb0);
			var_out_r = m9(var_r, 0xb1) + m9(var_gate_r, 0xb2) + m9(var_f2_4, 0xb3);

			break;
		}

		case 0xc0: {
			// Used by:
			// - Auto wah
			// - Hall
			// - Karaoke
			// - Plate
			// - Room
			// - Stage

			// Compute a center value
			s32 var_m   = m9(var_l, 0x6d) + m9(var_r, 0x6e);

			// Two stages of filtering on the center value
			s32 var_filter_2 = m7(m_var_filter_1, 0x6f) + m7(var_m, 0x70)        + m9(m_var_filter_2, 0x71);
			s32 var_filtered = m7(m_var_filter_2, 0x72) + m7(var_filter_2, 0x73) + m9(m_var_filter_3, 0x74);
			m_var_filter_1 = var_m;
			m_var_filter_2 = var_filter_2;
			m_var_filter_3 = var_filtered;

			// Inject in the reverb buffer and loop with filtering
			s32 tap1a = bvar.r(0x6c); // 36 v19
			s32 tap1b = bvar.r(0x6f); // 37 v21
			s32 tap1c = bvar.r(0x72); // 38 v27

			bvar.w(0x75, var_filtered    + m9(tap1a, 0x75));
			bvar.w(0x78, m9(tap1b, 0x76) + m9(tap1a, 0x77));

			s32 tap2a = m7(tap1b, 0x78) + m7(tap1a, 0x79);

			bvar.w(0x7b, m9(tap1b, 0x7a) + m9(tap2a, 0x7b));

			s32 tap2b = m7(tap1c, 0x7c) + m7(tap2a, 0x7d);

			s32 tap1d = bvar.r(0x9c);
			s32 tap1e = bvar.r(0x9f);

			bvar.w(0xa8, m9(m_var_h1, 0xa5) + m9(tap1d, 0xa6) + m9(tap2b, 0xa7));
			m_var_h1 = tap1d;

			bvar.w(0xae, m9(m_var_h2, 0xa8) + m9(tap1e, 0xa9) + m9(tap2b, 0xaa));
			m_var_h2 = tap1e;

			s32 tap1f = bvar.r(0xab);
			s32 tap1g = bvar.r(0xb1);

			bvar.w(0xb7, m9(m_var_h3, 0xb3) + m9(tap1f, 0xb4) + m9(tap2b, 0xb5));
			m_var_h3 = tap1f;

			bvar.w(0xba, m9(m_var_h4, 0xb6) + m9(tap1g, 0xb7) + m9(tap2b, 0xb8));
			m_var_h4 = tap1g;

			s32 tap1h = bvar.r(0x7e);

			s32 tap3a = m9(bvar.r(0x81) + bvar.r(0x84) + bvar.r(0x87) + bvar.r(0x8a), 0x8f) + m9(tap1h, 0x93);
			s32 tap3b = bvar.r(0xa5);
			bvar.w(0xb4, m9(tap3b, 0xaf) + m9(tap3a, 0xb0));
			s32 var_o_l = m7(tap3b, 0xb1) + m7(tap3a, 0xb2);

			s32 tap4a = m9(bvar.r(0x8d) + bvar.r(0x90) + bvar.r(0x93) + bvar.r(0x96), 0x9c) + m9(tap1h, 0xa0);
			s32 tap4b = bvar.r(0x99);
			bvar.w(0xa2, m9(tap4b, 0xa1) + m9(tap4a, 0xa2));
			s32 var_o_r = m7(tap4b, 0xa3) + m7(tap4a, 0xa4);

			//   auto-wah effect with lfo
			// Two stages of filtering
			s32 var_filter_l_2 = m7(m_var_filter_l_1, 0x80) + m7(var_l, 0x81)          + m9(m_var_filter_l_2, 0x82);
			s32 var_filtered_l = m7(m_var_filter_l_2, 0x83) + m7(var_filter_l_2, 0x84) + m9(m_var_filter_l_3, 0x85);

			m_var_filter_l_1 = var_l;
			m_var_filter_l_2 = var_filter_l_2;
			m_var_filter_l_3 = var_filtered_l;
			s32 var_filter_r_2 = m7(m_var_filter_r_1, 0x6f) + m7(var_r, 0x70)          + m9(m_var_filter_r_2, 0x71);
			s32 var_filtered_r = m7(m_var_filter_r_2, 0x72) + m7(var_filter_r_2, 0x73) + m9(m_var_filter_r_3, 0x74);

			m_var_filter_r_1 = var_r;
			m_var_filter_r_2 = var_filter_r_2;
			m_var_filter_r_3 = var_filtered_r;

			// Mixing
			s32 var_w_l = m7(var_filtered_l, 0x94) + m7(var_filtered_r, 0x95);
			s32 var_w_r = m7(var_filtered_r, 0x88);

			// Amplitude LFO and filtering
			lfo_step(m_var_lfo_phase, 0x7e);
			s32 amp = alfo(m_var_lfo_phase, 0, 0x86, 0x87, true);

			m_var_filterp_l_1 = m9v(m9(m_var_filterp_l_1, 0x89) + m9(m_var_filterp_l_2, 0x8a) + var_w_l, amp) + m9(m_var_filterp_l_1, 0x8b);
			m_var_filterp_l_2 = m9v(m_var_filterp_l_1, amp) + m9(m_var_filterp_l_2, 0x8d);

			m_var_filterp_r_1 = m9v(m9(m_var_filterp_r_1, 0x96) + m9(m_var_filterp_r_2, 0x97) + var_w_r, amp) + m9(m_var_filterp_r_1, 0x98);
			m_var_filterp_r_2 = m9v(m_var_filterp_r_1, amp) + m9(m_var_filterp_r_2, 0x9a);

			var_out_l = m9(var_filtered_l, 0xb9) +                   m9(m_var_filterp_l_1, 0xba) + m9(var_o_l, 0xbb);
			var_out_r = m9(var_filtered_r, 0xab) + m9(var_r, 0xac) + m9(m_var_filterp_r_1, 0xad) + m9(var_o_r, 0xae);
			break;
		}
		}


		// Chorus block
		//   Update the output volume
		m_cho_vol = m9(m_cho_vol, 0x58) + m_const[0x57];

		//   Scale the input
		cho_l = m7(cho_l, 0x02);
		cho_r = m7(cho_r, 0x05);

		// Add in the other channels
		cho_l += m9v(m7(var_out_l, 0x03), m_var_vol);
		cho_r += m9v(m7(var_out_r, 0x06), m_var_vol);

		//   A LFO with (up to) three phases to pick up the reverb
		lfo_step(m_cho_lfo_phase, 0x09);

		s32 cho_lfo_1 = bcho.rlfo(0x1b, m_cho_lfo_phase, 0,             0x1a);
		s32 cho_lfo_2 = bcho.rlfo(0x2a, m_cho_lfo_phase, m_const[0x25], 0x28);
		s32 cho_lfo_3 = bcho.rlfo(0x39, m_cho_lfo_phase, m_const[0x34], 0x37);

		//   Two stages of filtering
		s32 cho_filter_r_2 = m7(m_cho_filter_r_1, 0x3c) + m7(cho_r, 0x3d)          + m9(m_cho_filter_r_2, 0x3e);
		s32 cho_filtered_r = m7(m_cho_filter_r_2, 0x3f) + m7(cho_filter_r_2, 0x40) + m9(m_cho_filter_r_3, 0x41);

		m_cho_filter_r_1 = cho_r;
		m_cho_filter_r_2 = cho_filter_r_2;
		m_cho_filter_r_3 = cho_filtered_r;

		s32 cho_filter_l_2 = m7(m_cho_filter_l_1, 0x49) + m7(cho_l, 0x4a)          + m9(m_cho_filter_l_2, 0x4b);
		s32 cho_filtered_l = m7(m_cho_filter_l_2, 0x4c) + m7(cho_filter_l_2, 0x4d) + m9(m_cho_filter_l_3, 0x4e);

		m_cho_filter_l_1 = cho_l;
		m_cho_filter_l_2 = cho_filter_l_2;
		m_cho_filter_l_3 = cho_filtered_l;

		//   Reverb feedback from there, slighly assymetric to cover more possibilities
		bcho.w(0x42, m9(cho_lfo_2, 0x42) + cho_filtered_r);
		bcho.w(0x51, m9(cho_lfo_1, 0x4f) + cho_filtered_l + m9(cho_filtered_r, 0x50));

		//   Final value by combining the LFO-ed reverbs
		s32 cho_out_l = m9(cho_lfo_1, 0x60) + m9(cho_lfo_3, 0x61);
		s32 cho_out_r = m9(cho_lfo_2, 0x69) + m9(cho_lfo_3, 0x6a);



		// Reverb block
		//   Update the output volume
		m_rev_vol = m9(m_rev_vol, 0x0c) + m_const[0x0b];

		//   Scale the input
		rev = m7(rev, 0x11);

		//   Add in the other channels
		rev += m9v(m7(cho_out_l, 0x12) + m7(cho_out_r, 0x13), m_cho_vol);
		rev += m9v(m7(var_out_l, 0x14) + m7(var_out_r, 0x15), m_var_vol);

		//   Two stages of filtering (hpf then lpf)
		s32 rev_filter_2 = m7(m_rev_filter_1, 0x2d) + m7(rev, 0x2e)          + m9(m_rev_filter_2, 0x2f);
		s32 rev_filtered = m7(m_rev_filter_2, 0x30) + m7(rev_filter_2, 0x31) + m9(m_rev_filter_3, 0x32);

		m_rev_filter_1 = rev;
		m_rev_filter_2 = rev_filter_2;
		m_rev_filter_3 = rev_filtered;

		//   Main reverb
		brev.w(0x30, m9(brev.r(0x21), 0x29) + m9(brev.r(0x18), 0x2a));
		brev.w(0x33, m9(brev.r(0x1b), 0x33) + rev_filtered);

		//   Second dual reverb
		s32 rev_1 = m7(brev.r(0x33), 0x2b) + m7(brev.r(0x18), 0x2c);
		s32 rev_2 = m7(brev.r(0x27), 0x3a) + m7(rev_1, 0x3b);
		brev.w(0x3f, m9(brev.r(0x39), 0x38) + m9(rev_1, 0x39));

		//   Four more parallel layers with filtering
		brev.w(0x5d, m9(m_rev_hist_a, 0x59) + m9(brev.r(0x24), 0x5a) + m9(rev_2, 0x5b));
		m_rev_hist_a = brev.r(0x24);
		brev.w(0x63, m9(m_rev_hist_b, 0x5c) + m9(brev.r(0x54), 0x5d) + m9(rev_2, 0x5e));
		m_rev_hist_b = brev.r(0x54);
		brev.w(0x69, m9(m_rev_hist_c, 0x62) + m9(brev.r(0x5a), 0x63) + m9(rev_2, 0x64));
		m_rev_hist_c = brev.r(0x63);
		brev.w(0x6c, m9(m_rev_hist_d, 0x65) + m9(brev.r(0x60), 0x66) + m9(rev_2, 0x67));
		m_rev_hist_d = brev.r(0x66);

		//   Split final pick-up and injection
		s32 rev_base_l = m9(brev.r(0x00) + brev.r(0x03) + brev.r(0x06) + brev.r(0x09), 0x1c) + m9(brev.r(0xbd), 0x1b);
		brev.w(0x48, m9(brev.r(0x36), 0x45) + m9(rev_base_l, 0x46));
		s32 rev_out_l = m7(brev.r(0x36), 0x47) + m7(rev_base_l, 0x48);

		s32 rev_base_r = m9(brev.r(0x0c) + brev.r(0x0f) + brev.r(0x12) + brev.r(0x15), 0x21) + m9(brev.r(0xbd), 0x20);
		brev.w(0x48, m9(brev.r(0x36), 0x51) + m9(rev_base_r, 0x52));
		s32 rev_out_r = m7(brev.r(0x36), 0x53) + m7(rev_base_r, 0x54);


		// Scale the dry input
		dry_l = m7(dry_l, 0xbe);
		dry_r = m7(dry_r, 0x01);


		// Add in the other channels
		dry_l += m9v(rev_out_l, m_rev_vol) + m9v(m9(cho_out_l, 0x17), m_cho_vol) + m9v(m9(var_out_l, 0x18), m_var_vol);
		dry_r += m9v(rev_out_r, m_rev_vol) + m9v(m9(cho_out_r, 0x0e), m_cho_vol) + m9v(m9(var_out_r, 0x0f), m_var_vol);

		stream.put_int(0, i, dry_l, 32768);
		stream.put_int(1, i, dry_r, 32768);

		m_buffer_offset --;
	}
}
