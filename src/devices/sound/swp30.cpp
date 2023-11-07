// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Yamaha SWP30/30B, rompler/dsp combo

#include "emu.h"
#include "debugger.h"
#include "swp30.h"

static int scount = 0;

/*
  The SWP30 is the combination of a rompler called AWM2 (Advanced Wave
  Memory 2) and an effects DSP called MEG (Multiple Effects
  Generator).  It also includes some routing/mixing capabilities,
  moving data between AWM2, MEG and serial inputs and outputs with
  volume management capabilities everywhere.  Its clock is 33.9MHz and
  the output is at 44100Hz stereo (768 cycles per sample pair) per dac
  output.

  I/O wise, the chip has 8 generic audio serial inputs and 8 outputs
  for external plugins, and two dac outputs.  The DAC outputs are
  stereo, and so is the first generic input.  It's unclear whether the
  outputs and the other inputs are stereo.  The MU100 connects a
  stereo ADC to the first input, and routes the third input and output
  to the plugin boards, but not the left/right input clock, arguing
  for mono.


    Registers:

  The chip interface presents 4096 16-bits registers in a 64x64 grid.
  They all seem to be read/write.  Some of this grid is for
  per-channel values for AWM2, but parts are isolated and renumbered
  for MEG regisrers or for general control functions.

  Names we'll use in the rest of the text:
  - reg(y, x) is the register at address 2*(y*0x40 + x)
  - ch<nn>  is reg(channel, nn) for a given AWG2 channel
  - sy<nn>  is reg(nn/2, 0xe + (nn % 2))
  - fp<nnn> is reg(nn/6, 0x21 + 2*(nn % 6))
  - of<nn>  is reg(nn/2, 0x30 + (nn % 2))
  - lfo<nn> is reg(nn/2, 0x3e + (nn % 2)) for nn = 0..17


    AWM2:

  The AWM2 is in charge of handling the individual channels.  It
  manages reading the rom, decoding the samples, applying volume and
  pitch envelopes and lfos and filtering the result.  Each channel is
  then sent to the mixer for further processing.

  The sound data can be four formats (8 bits, 12 bits, 16 bits, and a
  8-bits log format with roughly 10 bits of dynamic).  The rom bus is
  25 bits address and 32 bits data wide.  It applies four filters to
  the sample data, two of fixed type (low pass then highpass) and two
  free 3-point FIR filters (used for yet another lowpass and
  highpass).  Envelopes are handled semi-automatically, and the final
  panned result is sent to the mixer.


  ch00       fixed LPF frequency cutoff index
  ch01       fixed LPF frequency cutoff index increment?
  ch02       fixed HPF frequency cutoff
  ch03       40ff at startup, 5010 always afterwards?
  ch04       fixed LPF resonance level
  ch05       unknown
  ch06       attack,  bit 14-8 = step, bit 7 = skip
  ch07       decay1,   bit 14-8 = step, bit 7-0 = target attenuation (top 8 bits)
  ch08       decay2, bit 14-8 = step, bit 7-0 = target attenuation (top 8 bits)
  ch09       base volume  bit 15 = activate decay2, bit 14-8 unknown, bit 7-0 = initial attenuation

  ch0a-0d    unknown, probably something to do with pitch eg
  ch10       unknown
  ch11       bit 15 = compressed 8-bits mode, 13-0 channel replay frequency, signed 3.10 fixed point,
             log2 scale, positive is higher resulting frequency.

  ch12-13    bit 31 unknown, 30 unknown, 29-0 = number of samples before the loop point
  ch14-15    bit 31 = play sample backwards, 30-0 = number of samples in the loop
  ch16-17    bit 31-30 = sample format, 29-25 = loop samples decimal part, 24-0 = loop start address in rom
  ch20,22,24 first FIR coefficients
  ch26,28,2a second FIR coefficients
  ch2c-2f    unknown
  ch32       pan left/right, 2x8 bits of attenuation

  sy02       internal register selector, msb = 0 or 6, lsb = channel
  sy03       internal register read port, used for envelope/keyoff management, 6 seems to be current volume
  sy0c-0f    keyon mask
  sy10       write something to trigger a keyon according to the mask


  The current attenuation (before panning) is on 26 bits, in 4.22
  floating point format, of which only probably the top 8 are used for
  actual volume computations (see the Mixer part).  The steps are in
  4.3 floating-point format, e.g. the value converts to linear as:

     step = (8 + bit 2..0) << (bit 7..4)

  giving a value between 8 and 0x78000.  This value is added or
  substracted after each sample.

  For attack the actual range of steps is 8..119, giving an increment
  of 0x10 to 0x3c000, and a full sweep from -96dB to 0 in 95s (8) to
  6.2ms (119).

  For decay1 and decay2 the range is 1..120, e.g. 9 to 0x40000, or
  169s to 5.8ms for a full sweep.


    MEG:

  The MEG is a DSP with 384 program steps connected to a 0x40000
  samples ram.  Instructions are 64 bits wide, and to each instruction
  is associated a 2.14 fixed point value, Every third instruction (pc
  multiple of 3) can initiate a memory access to the reverb buffer
  which will be completed two instructions later.  Each of those
  instructions is associated to a 16-bits address offset value.

  The DSP also sports 256 rotating registers (e.g. register 1 at run
  <n> becomes register 0 at run <n+1>) and 64 fixed registers.  The
  fixed registers are used to store the results of reading the samples
  ram and also communicate with the mixer.

  Every 44100th of a second the 384 program steps are run once in
  order (no branches) to compute everything.

  24 LFO registers are available (possibly more).  The LFO registers
  internal counters are 22 bits wide.  The LSB of the register gives
  the increment per sample, encoded in a special 3.5 format.
  With scale = 3bits and v = 5bits,
    step  = base[scale] + (v << shift[scale])
    base  = { 0, 32, 64, 128, 256, 512,  1024, 2048 }
    shift = { 0,  0,  1,   2,   3,   4,     5,    6 }

  The 21th bit of the counter inverts bits 20-0 on read, those are
  interpreted as a 0-1 value, giving a sawtooth wave.

  8 mappings can be setup, which allow to manage rotating buffers in
  the samples ram easily by automating masking and offset adding.  The
  register format is: tttttsss oooooooo.  't' is not understood
  yet. 's' is the sub-buffer size, defined as 1 << (10+s).  The base
  offset is o << 10.  There are no alignment issues, e.g. you can have
  a buffer at 0x28000 which is 0x10000 samples long.


  fp<nnn>    fixed point 2.14 value associated with instruction nnn
  of<nn>     16-bits offset associated with instruction 3*nn
  lfo<nn>    LFO registers

  sy21       MEG program write address
  sy22-25    MEG program opcode, msb-first, writing to 25 triggers an auto-increment
  sy30-3e    even slots only, MEG buffer mappings


    Mixer:

  The mixer gets the outputs of the AWM2, the MEG (for the previous
  sample) and the external inputs, attenuates and sums them according
  to its mapping instructions, and pushes the results to the MEG, the
  DACs and the external outputs.  The attenuations are 8-bits values
  in 4.4 floating point format (multiplies by (1-mant/2)*2**(-exp)).
  The routing is indicated through triplets of 16-bits values.

  ch33       dry (msb) and reverb (lsb) attenuation for an AWM2 channel
  ch34       chorus (msb) and variation (lsb) atternuation
  ch35-37    routing for an AWM2 channel


*/


DEFINE_DEVICE_TYPE(SWP30, swp30_device, "swp30", "Yamaha SWP30 sound chip")

bool swp30_device::istep(s32 &value, s32 limit, s32 step)
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

s32 swp30_device::fpadd(s32 value, s32 step)
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

s32 swp30_device::fpsub(s32 value, s32 step)
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

bool swp30_device::fpstep(s32 &value, s32 limit, s32 step)
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
s32 swp30_device::fpapply(s32 value, s32 sample)
{
	if(value >= 0x10000000)
		return 0;
	return (s64(sample) - ((s64(sample) * ((value >> 9) & 0x7fff)) >> 16)) >> (value >> 24);
}

// sample is signed 24.8
s32 swp30_device::lpffpapply(s32 value, s32 sample)
{
	return ((((value >> 7) & 0x7fff) | 0x8000) * s64(sample)) >> (31 - (value >> 22));
}

// Some tables we picked up from the swp00.  May be different, may not be.

const std::array<s32, 0x80> swp30_device::attack_linear_step = {
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

const std::array<s32, 0x20> swp30_device::decay_linear_step = {
	0x15083, 0x17ad2, 0x1a41a, 0x1cbe7, 0x1f16d, 0x22ef1, 0x26a44, 0x2a1e4,
	0x2da35, 0x34034, 0x3a197, 0x40000, 0x45b82, 0x4b809, 0x51833, 0x57262,
	0x5d9f7, 0x6483f, 0x6b15c, 0x71c72, 0x77976, 0x7d119, 0x83127, 0x88889,
	0x8d3dd, 0x939a8, 0x991f2, 0x9d89e, 0xa0a0a, 0xa57eb, 0xa72f0, 0xac769,
};

// Actual shape of the lfos unknown, since the hardware accepts 4 and
// 3 are in use (0, 1 and 3) and no recording are currently available

const std::array<u32, 4> swp30_device::lfo_shape_centered_saw = { 0x00000000, 0x00000000, 0xfff00000, 0xfff00000 }; // --////--
const std::array<u32, 4> swp30_device::lfo_shape_centered_tri = { 0x00000000, 0x0007ffff, 0xfff7ffff, 0xfff00000 }; // --/\/\--
const std::array<u32, 4> swp30_device::lfo_shape_offset_saw   = { 0x00000000, 0x00000000, 0x00000000, 0x00000000 }; // __////__
const std::array<u32, 4> swp30_device::lfo_shape_offset_tri   = { 0x00000000, 0x00000000, 0x000fffff, 0x000fffff }; // __/\/\__

swp30_device::swp30_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cpu_device(mconfig, SWP30, tag, owner, clock),
	  device_sound_interface(mconfig, *this),
	  m_program_config("meg_program", ENDIANNESS_LITTLE, 64, 9, -3, address_map_constructor(FUNC(swp30_device::meg_prg_map), this)),
	  m_rom_config("sample_rom", ENDIANNESS_LITTLE, 32, 25, -2)
{
}

void swp30_device::device_start()
{
	m_program = &space(AS_PROGRAM);
	m_rom     = &space(AS_DATA);
	m_rom->cache(m_rom_cache);

	state_add(STATE_GENPC,     "GENPC",     m_meg_pc).noshow();
	state_add(STATE_GENPCBASE, "CURPC",     m_meg_pc).noshow();
	state_add(0,               "PC",        m_meg_pc);

	set_icountptr(m_icount);

	m_stream = stream_alloc(0, 2, 44100, STREAM_SYNCHRONOUS);

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
		s16 base = ((i & 0x1f) << (7+(i >> 5))) + (((1 << (i >> 5))-1) << 12);
		m_dpcm[i | 0x80] = - base;
		m_dpcm[i]        = + base;
	}

	save_item(NAME(m_keyon_mask));

	save_item(NAME(m_sample_start));
	save_item(NAME(m_sample_end));
	save_item(NAME(m_sample_address));
	save_item(NAME(m_pitch));

	save_item(NAME(m_release_glo));
	save_item(NAME(m_pan));
	save_item(NAME(m_dry_rev));
	save_item(NAME(m_cho_var));

	save_item(NAME(m_lfo_step_pmod));
	save_item(NAME(m_lfo_amod));

	save_item(NAME(m_attack));
	save_item(NAME(m_decay1));
	save_item(NAME(m_decay2));

	save_item(NAME(m_lfo_phase));
	save_item(NAME(m_sample_pos));
	save_item(NAME(m_envelope_level));
	save_item(NAME(m_envelope_on_timer));
	save_item(NAME(m_envelope_timer));
	save_item(NAME(m_decay2_done));
	save_item(NAME(m_envelope_mode));
	save_item(NAME(m_glo_level_cur));
	save_item(NAME(m_pan_l));
	save_item(NAME(m_pan_r));

	save_item(NAME(m_dpcm_current));
	save_item(NAME(m_dpcm_next));
	save_item(NAME(m_dpcm_address));

	save_item(NAME(m_sample_history));

	save_item(NAME(m_lpf_cutoff));
	save_item(NAME(m_lpf_cutoff_inc));
	save_item(NAME(m_lpf_reso));
	save_item(NAME(m_hpf_cutoff));
	save_item(NAME(m_eq_filter));
	save_item(NAME(m_routing));

	save_item(NAME(m_internal_adr));

	save_item(NAME(m_meg_program_address));
	save_item(NAME(m_waverom_adr));
	save_item(NAME(m_waverom_mode));
	save_item(NAME(m_waverom_access));
	save_item(NAME(m_waverom_val));

	save_item(NAME(m_meg_program));
	save_item(NAME(m_meg_const));
	save_item(NAME(m_meg_offset));
	save_item(NAME(m_meg_lfo));
	save_item(NAME(m_meg_map));
}

void swp30_device::device_reset()
{
	m_keyon_mask = 0;

	std::fill(m_sample_start.begin(), m_sample_start.end(), 0);
	std::fill(m_sample_end.begin(), m_sample_end.end(), 0);
	std::fill(m_sample_address.begin(), m_sample_address.end(), 0);
	std::fill(m_pitch.begin(), m_pitch.end(), 0);

	std::fill(m_release_glo.begin(), m_release_glo.end(), 0);
	std::fill(m_pan.begin(), m_pan.end(), 0);
	std::fill(m_dry_rev.begin(), m_dry_rev.end(), 0);
	std::fill(m_cho_var.begin(), m_cho_var.end(), 0);

	std::fill(m_lfo_step_pmod.begin(), m_lfo_step_pmod.end(), 0);
	std::fill(m_lfo_amod.begin(), m_lfo_amod.end(), 0);

	std::fill(m_attack.begin(), m_attack.end(), 0);
	std::fill(m_decay1.begin(), m_decay1.end(), 0);
	std::fill(m_decay2.begin(), m_decay2.end(), 0);

	std::fill(m_lfo_phase.begin(), m_lfo_phase.end(), 0);
	std::fill(m_sample_pos.begin(), m_sample_pos.end(), 0);
	std::fill(m_envelope_level.begin(), m_envelope_level.end(), 0);
	std::fill(m_envelope_timer.begin(), m_envelope_timer.end(), 0);
	std::fill(m_envelope_on_timer.begin(), m_envelope_on_timer.end(), false);
	std::fill(m_decay2_done.begin(), m_decay2_done.end(), false);
	std::fill(m_envelope_mode.begin(), m_envelope_mode.end(), IDLE);
	std::fill(m_glo_level_cur.begin(), m_glo_level_cur.end(), 0);
	std::fill(m_pan_l.begin(), m_pan_l.end(), 0);
	std::fill(m_pan_r.begin(), m_pan_r.end(), 0);

	std::fill(m_dpcm_current.begin(), m_dpcm_current.end(), false);
	std::fill(m_dpcm_next.begin(), m_dpcm_next.end(), false);
	std::fill(m_dpcm_address.begin(), m_dpcm_address.end(), false);

	std::fill(m_meg_program.begin(), m_meg_program.end(), 0);
	std::fill(m_meg_const.begin(), m_meg_const.end(), 0);
	std::fill(m_meg_offset.begin(), m_meg_offset.end(), 0);
	std::fill(m_meg_lfo.begin(), m_meg_lfo.end(), 0);
	std::fill(m_meg_map.begin(), m_meg_map.end(), 0);

	memset(m_sample_history, 0, sizeof(m_sample_history));

	memset(m_lpf_cutoff, 0, sizeof(m_lpf_cutoff));
	memset(m_lpf_cutoff_inc, 0, sizeof(m_lpf_cutoff_inc));
	memset(m_lpf_reso, 0, sizeof(m_lpf_reso));
	memset(m_hpf_cutoff, 0, sizeof(m_hpf_cutoff));
	memset(m_eq_filter, 0, sizeof(m_eq_filter));
	memset(m_routing, 0, sizeof(m_routing));

	m_meg_program_address = 0;
	m_waverom_adr = 0;
	m_waverom_mode = 0;
	m_waverom_access = 0;
	m_waverom_val = 0;
}

void swp30_device::map(address_map &map)
{
	map(0x0000, 0x1fff).rw(FUNC(swp30_device::snd_r), FUNC(swp30_device::snd_w));

	rchan(map, 0x00).rw(FUNC(swp30_device::lpf_cutoff_r), FUNC(swp30_device::lpf_cutoff_w));
	rchan(map, 0x01).rw(FUNC(swp30_device::lpf_cutoff_inc_r), FUNC(swp30_device::lpf_cutoff_inc_w));
	rchan(map, 0x02).rw(FUNC(swp30_device::hpf_cutoff_r), FUNC(swp30_device::hpf_cutoff_w));
	// 03 seems to always get 5010 except at startup where it's 40ff
	rchan(map, 0x04).rw(FUNC(swp30_device::lpf_reso_r), FUNC(swp30_device::lpf_reso_w));
	rchan(map, 0x05).rw(FUNC(swp30_device::lfo_amod_r), FUNC(swp30_device::lfo_amod_w));
	rchan(map, 0x06).rw(FUNC(swp30_device::attack_r), FUNC(swp30_device::attack_w));
	rchan(map, 0x07).rw(FUNC(swp30_device::decay1_r), FUNC(swp30_device::decay1_w));
	rchan(map, 0x08).rw(FUNC(swp30_device::decay2_r), FUNC(swp30_device::decay2_w));
	rchan(map, 0x09).rw(FUNC(swp30_device::release_glo_r), FUNC(swp30_device::release_glo_w));
	rchan(map, 0x0a).rw(FUNC(swp30_device::lfo_step_pmod_r), FUNC(swp30_device::lfo_step_pmod_w));
	// 0b-0d missing
	// 10 missing
	rchan(map, 0x11).rw(FUNC(swp30_device::pitch_r), FUNC(swp30_device::pitch_w));
	rchan(map, 0x12).rw(FUNC(swp30_device::sample_start_h_r), FUNC(swp30_device::sample_start_h_w));
	rchan(map, 0x13).rw(FUNC(swp30_device::sample_start_l_r), FUNC(swp30_device::sample_start_l_w));
	rchan(map, 0x14).rw(FUNC(swp30_device::sample_end_h_r), FUNC(swp30_device::sample_end_h_w));
	rchan(map, 0x15).rw(FUNC(swp30_device::sample_end_l_r), FUNC(swp30_device::sample_end_l_w));
	rchan(map, 0x16).rw(FUNC(swp30_device::sample_address_h_r), FUNC(swp30_device::sample_address_h_w));
	rchan(map, 0x17).rw(FUNC(swp30_device::sample_address_l_r), FUNC(swp30_device::sample_address_l_w));
	rchan(map, 0x20).rw(FUNC(swp30_device::eq_filter_r<0>), FUNC(swp30_device::eq_filter_w<0>));
	rchan(map, 0x22).rw(FUNC(swp30_device::eq_filter_r<1>), FUNC(swp30_device::eq_filter_w<1>));
	rchan(map, 0x24).rw(FUNC(swp30_device::eq_filter_r<2>), FUNC(swp30_device::eq_filter_w<2>));
	rchan(map, 0x26).rw(FUNC(swp30_device::eq_filter_r<3>), FUNC(swp30_device::eq_filter_w<3>));
	rchan(map, 0x28).rw(FUNC(swp30_device::eq_filter_r<4>), FUNC(swp30_device::eq_filter_w<4>));
	rchan(map, 0x2a).rw(FUNC(swp30_device::eq_filter_r<5>), FUNC(swp30_device::eq_filter_w<5>));
	// 2c-2f missing
	rchan(map, 0x32).rw(FUNC(swp30_device::pan_r), FUNC(swp30_device::pan_w));
	rchan(map, 0x33).rw(FUNC(swp30_device::dry_rev_r), FUNC(swp30_device::dry_rev_w));
	rchan(map, 0x34).rw(FUNC(swp30_device::cho_var_r), FUNC(swp30_device::cho_var_w));
	rchan(map, 0x35).rw(FUNC(swp30_device::routing_r<0>), FUNC(swp30_device::routing_w<0>));
	rchan(map, 0x36).rw(FUNC(swp30_device::routing_r<1>), FUNC(swp30_device::routing_w<1>));
	rchan(map, 0x37).rw(FUNC(swp30_device::routing_r<2>), FUNC(swp30_device::routing_w<2>));
	// 38-3d missing, are special

	// Control registers
	// These appear as channel slots 0x0e and 0x0f
	// 00-01 missing
	rctrl(map, 0x02).rw(FUNC(swp30_device::internal_adr_r), FUNC(swp30_device::internal_adr_w));
	rctrl(map, 0x03).r (FUNC(swp30_device::internal_r));
	rctrl(map, 0x04).rw(FUNC(swp30_device::waverom_adr_r<1>), FUNC(swp30_device::waverom_adr_w<1>));
	rctrl(map, 0x05).rw(FUNC(swp30_device::waverom_adr_r<0>), FUNC(swp30_device::waverom_adr_w<0>));
	rctrl(map, 0x06).rw(FUNC(swp30_device::waverom_mode_r<1>), FUNC(swp30_device::waverom_mode_w<1>));
	rctrl(map, 0x07).rw(FUNC(swp30_device::waverom_mode_r<0>), FUNC(swp30_device::waverom_mode_w<0>));
	rctrl(map, 0x08).rw(FUNC(swp30_device::waverom_access_r), FUNC(swp30_device::waverom_access_w));
	rctrl(map, 0x09).r (FUNC(swp30_device::waverom_busy_r));
	rctrl(map, 0x0a).r (FUNC(swp30_device::waverom_val_r<1>));
	rctrl(map, 0x0b).r (FUNC(swp30_device::waverom_val_r<0>));
	rctrl(map, 0x0c).rw(FUNC(swp30_device::keyon_mask_r<3>), FUNC(swp30_device::keyon_mask_w<3>));
	rctrl(map, 0x0d).rw(FUNC(swp30_device::keyon_mask_r<2>), FUNC(swp30_device::keyon_mask_w<2>));
	rctrl(map, 0x0e).rw(FUNC(swp30_device::keyon_mask_r<1>), FUNC(swp30_device::keyon_mask_w<1>));
	rctrl(map, 0x0f).rw(FUNC(swp30_device::keyon_mask_r<0>), FUNC(swp30_device::keyon_mask_w<0>));
	rctrl(map, 0x10).rw(FUNC(swp30_device::keyon_r), FUNC(swp30_device::keyon_w));
	// 11-20 missing
	rctrl(map, 0x21).rw(FUNC(swp30_device::meg_prg_address_r), FUNC(swp30_device::meg_prg_address_w));
	rctrl(map, 0x22).rw(FUNC(swp30_device::meg_prg_r<0>), FUNC(swp30_device::meg_prg_w<0>));
	rctrl(map, 0x23).rw(FUNC(swp30_device::meg_prg_r<1>), FUNC(swp30_device::meg_prg_w<1>));
	rctrl(map, 0x24).rw(FUNC(swp30_device::meg_prg_r<2>), FUNC(swp30_device::meg_prg_w<2>));
	rctrl(map, 0x25).rw(FUNC(swp30_device::meg_prg_r<3>), FUNC(swp30_device::meg_prg_w<3>));
	// 26-7f missing
	rctrl(map, 0x30).rw(FUNC(swp30_device::meg_map_r<0>), FUNC(swp30_device::meg_map_w<0>));
	rctrl(map, 0x32).rw(FUNC(swp30_device::meg_map_r<1>), FUNC(swp30_device::meg_map_w<1>));
	rctrl(map, 0x34).rw(FUNC(swp30_device::meg_map_r<2>), FUNC(swp30_device::meg_map_w<2>));
	rctrl(map, 0x36).rw(FUNC(swp30_device::meg_map_r<3>), FUNC(swp30_device::meg_map_w<3>));
	rctrl(map, 0x38).rw(FUNC(swp30_device::meg_map_r<4>), FUNC(swp30_device::meg_map_w<4>));
	rctrl(map, 0x3a).rw(FUNC(swp30_device::meg_map_r<5>), FUNC(swp30_device::meg_map_w<5>));
	rctrl(map, 0x3c).rw(FUNC(swp30_device::meg_map_r<6>), FUNC(swp30_device::meg_map_w<6>));
	rctrl(map, 0x3e).rw(FUNC(swp30_device::meg_map_r<7>), FUNC(swp30_device::meg_map_w<7>));

	// MEG registers
	rchan(map, 0x21).rw(FUNC(swp30_device::meg_const_r<0>), FUNC(swp30_device::meg_const_w<0>));
	rchan(map, 0x23).rw(FUNC(swp30_device::meg_const_r<1>), FUNC(swp30_device::meg_const_w<1>));
	rchan(map, 0x25).rw(FUNC(swp30_device::meg_const_r<2>), FUNC(swp30_device::meg_const_w<2>));
	rchan(map, 0x27).rw(FUNC(swp30_device::meg_const_r<3>), FUNC(swp30_device::meg_const_w<3>));
	rchan(map, 0x29).rw(FUNC(swp30_device::meg_const_r<4>), FUNC(swp30_device::meg_const_w<4>));
	rchan(map, 0x2b).rw(FUNC(swp30_device::meg_const_r<5>), FUNC(swp30_device::meg_const_w<5>));
	rchan(map, 0x30).rw(FUNC(swp30_device::meg_offset_r<0>), FUNC(swp30_device::meg_offset_w<0>));
	rchan(map, 0x31).rw(FUNC(swp30_device::meg_offset_r<1>), FUNC(swp30_device::meg_offset_w<1>));
	rchan(map, 0x3e).rw(FUNC(swp30_device::meg_lfo_r<0>), FUNC(swp30_device::meg_lfo_w<0>));
	rchan(map, 0x3f).rw(FUNC(swp30_device::meg_lfo_r<1>), FUNC(swp30_device::meg_lfo_w<1>));
}

// Control registers
template<int sel> u16 swp30_device::keyon_mask_r()
{
	return m_keyon_mask >> (16*sel);
}

template<int sel> void swp30_device::keyon_mask_w(u16 data)
{
	m_keyon_mask = (m_keyon_mask & ~(u64(0xffff) << (16*sel))) | (u64(data) << (16*sel));
}

u16 swp30_device::keyon_r()
{
	return 0;
}

void swp30_device::keyon_w(u16)
{
	for(int chan=0; chan<64; chan++) {
		u64 mask = u64(1) << chan;
		if(m_keyon_mask & mask) {
			m_sample_pos[chan] = -(m_sample_start[chan] & 0xffffff) << 8;
			if(m_release_glo[chan] & 0x8000) {
				m_envelope_level[chan] = 0;
				m_envelope_on_timer[chan] = false;
				m_envelope_mode[chan] = RELEASE;
			} else if(m_attack[chan] & 0x80) {
				m_envelope_level[chan] = 0x8000000;
				m_envelope_on_timer[chan] = false;
				m_envelope_mode[chan] = ATTACK;
			} else {
				m_envelope_level[chan] = 0;
				m_envelope_on_timer[chan] = true;
				m_envelope_timer[chan] = 0x8000000;
				m_envelope_mode[chan] = ATTACK;
			}

			m_decay2_done[chan] = false;
			m_glo_level_cur[chan] = (m_release_glo[chan] & 0xff) << 4;
			m_pan_l[chan] = (m_pan[chan] & 0xff00) >> 4;
			m_pan_r[chan] = (m_pan[chan] & 0x00ff) << 4;

			m_dpcm_current[chan] = 0;
			m_dpcm_next[chan] = 0;
			s32 dt = m_sample_start[chan] & 0xffffff;
			if(m_sample_end[chan] & 0x80000000)
				dt = -dt;
			m_dpcm_address[chan] = ((m_sample_address[chan] & 0xffffff) << 2) - dt;

			m_lfo_phase[chan] = 0;

			if(1)
				logerror("[%08d] keyon %02x %08x %08x %08x vol %04x env %04x %04x %04x pan %04x disp %04x %04x\n", scount, chan, m_sample_start[chan], m_sample_end[chan], m_sample_address[chan], m_release_glo[chan], m_attack[chan], m_decay1[chan], m_decay2[chan], m_pan[chan], m_dry_rev[chan], m_cho_var[chan]);
		}
	}
	m_keyon_mask = 0;
}


u16 swp30_device::meg_prg_address_r()
{
	return m_meg_program_address;
}

void swp30_device::meg_prg_address_w(u16 data)
{
	m_meg_program_address = data;
	if(m_meg_program_address >= 0x180)
		m_meg_program_address = 0;
}

template<int sel> u16 swp30_device::meg_prg_r()
{
	constexpr offs_t shift = 48-16*sel;
	return m_meg_program[m_meg_program_address] >> shift;
}

template<int sel> void swp30_device::meg_prg_w(u16 data)
{
	constexpr offs_t shift = 48-16*sel;
	constexpr u64 mask = ~(u64(0xffff) << shift);
	m_meg_program[m_meg_program_address] = (m_meg_program[m_meg_program_address] & mask) | (u64(data) << shift);

	if(sel == 3) {
		if(0)
			logerror("program %03x %016x\n", m_meg_program_address, m_meg_program[m_meg_program_address]);
		m_meg_program_address ++;
		if(m_meg_program_address == 0x180)
			m_meg_program_address = 0;
	}
}


template<int sel> u16 swp30_device::meg_map_r()
{
	return m_meg_map[sel];
}

template<int sel> void swp30_device::meg_map_w(u16 data)
{
	m_meg_map[sel] = data;
}


template<int sel> void swp30_device::waverom_adr_w(u16 data)
{
	if(sel)
		m_waverom_adr = (m_waverom_adr & 0x0000ffff) | (data << 16);
	else
		m_waverom_adr = (m_waverom_adr & 0xffff0000) |  data;
}

template<int sel> u16 swp30_device::waverom_adr_r()
{
	return m_waverom_adr >> (16*sel);
}

template<int sel> void swp30_device::waverom_mode_w(u16 data)
{
	if(sel)
		m_waverom_mode = (m_waverom_mode & 0x0000ffff) | (data << 16);
	else
		m_waverom_mode = (m_waverom_mode & 0xffff0000) |  data;
}

template<int sel> u16 swp30_device::waverom_mode_r()
{
	return m_waverom_mode >> (16*sel);
}

void swp30_device::waverom_access_w(u16 data)
{
	m_waverom_access = data;
	if(data == 0x8000) {
		m_waverom_val = m_rom_cache.read_dword(m_waverom_adr);
		logerror("waverom read adr=%08x mode=%08x -> %08x\n", m_waverom_adr, m_waverom_mode, m_waverom_val);
	}
}

u16 swp30_device::waverom_access_r()
{
	return m_waverom_access;
}

u16 swp30_device::waverom_busy_r()
{
	// 0 = busy reading the rom, non-0 = finished
	return 0xffff;
}

template<int sel> u16 swp30_device::waverom_val_r()
{
	return m_waverom_val >> (16*sel);
}


// AWM2 per-channel registers
u16 swp30_device::lpf_cutoff_r(offs_t offset)
{
	return m_lpf_cutoff[offset >> 6];
}

void swp30_device::lpf_cutoff_w(offs_t offset, u16 data)
{
	u8 chan = offset >> 6;
	if(0 && m_lpf_cutoff[chan] != data)
		logerror("chan %02x lpf cutoff %04x\n", chan, data);
	m_lpf_cutoff[chan] = data;
}

u16 swp30_device::lpf_cutoff_inc_r(offs_t offset)
{
	return m_lpf_cutoff_inc[offset >> 6];
}

void swp30_device::lpf_cutoff_inc_w(offs_t offset, u16 data)
{
	u8 chan = offset >> 6;
	if(0 && m_lpf_cutoff_inc[chan] != data)
		logerror("chan %02x lpf cutoff increment %04x\n", chan, data);
	m_lpf_cutoff_inc[chan] = data;
}

u16 swp30_device::hpf_cutoff_r(offs_t offset)
{
	return m_hpf_cutoff[offset >> 6];
}

void swp30_device::hpf_cutoff_w(offs_t offset, u16 data)
{
	u8 chan = offset >> 6;
	if(0 && m_hpf_cutoff[chan] != data)
		logerror("chan %02x hpf cutoff %04x\n", chan, data);
	m_hpf_cutoff[chan] = data;
}

u16 swp30_device::lpf_reso_r(offs_t offset)
{
	return m_lpf_reso[offset >> 6];
}

void swp30_device::lpf_reso_w(offs_t offset, u16 data)
{
	u8 chan = offset >> 6;
	if(0 && m_lpf_reso[chan] != data)
		logerror("chan %02x lpf resonance %04x\n", chan, data);
	m_lpf_reso[chan] = data;
}

template<int coef> u16 swp30_device::eq_filter_r(offs_t offset)
{
	return m_eq_filter[offset >> 6][coef];
}

template<int coef> void swp30_device::eq_filter_w(offs_t offset, u16 data)
{
	m_eq_filter[offset >> 6][coef] = data;
}

template<int sel> u16 swp30_device::routing_r(offs_t offset)
{
	return m_routing[offset >> 6][sel];
}

template<int sel> void swp30_device::routing_w(offs_t offset, u16 data)
{
	m_routing[offset >> 6][sel] = data;
}

u16 swp30_device::release_glo_r(offs_t offset)
{
	return m_release_glo[offset >> 6];
}

void swp30_device::release_glo_w(offs_t offset, u16 data)
{
	u8 chan = offset >> 6;
	if(1 && m_release_glo[chan] != data)
		logerror("snd chan %02x rel/glo %02x %02x\n", chan, data >> 8, data & 0xff);
	m_release_glo[chan] = data;
	if((data & 0x8000) && m_envelope_mode[chan] != IDLE && m_envelope_mode[chan] != RELEASE)
		m_envelope_mode[chan] = RELEASE;
}


u16 swp30_device::pan_r(offs_t offset)
{
	return m_pan[offset >> 6];
}

void swp30_device::pan_w(offs_t offset, u16 data)
{
	u8 chan = offset >> 6;
	if(0 && m_pan[chan] != data)
		logerror("snd chan %02x pan l %02x r %02x\n", chan, data >> 8, data & 0xff);
	m_pan[chan] = data;
}

u16 swp30_device::dry_rev_r(offs_t offset)
{
	return m_dry_rev[offset >> 6];
}

void swp30_device::dry_rev_w(offs_t offset, u16 data)
{
	u8 chan = offset >> 6;
	if(0 && m_dry_rev[chan] != data)
		logerror("snd chan %02x dry %02x rev %02x\n", chan, data >> 8, data & 0xff);
	m_dry_rev[chan] = data;
}

u16 swp30_device::cho_var_r(offs_t offset)
{
	return m_cho_var[offset >> 6];
}

void swp30_device::cho_var_w(offs_t offset, u16 data)
{
	u8 chan = offset >> 6;
	if(0 && m_cho_var[chan] != data)
		logerror("snd chan %02x cho %02x var %02x\n", chan, data >> 8, data & 0xff);
	m_cho_var[chan] = data;
}

u16 swp30_device::pitch_r(offs_t offset)
{
	return m_pitch[offset >> 6];
}

void swp30_device::pitch_w(offs_t offset, u16 data)
{
	u8 chan = offset >> 6;
	//  delta is 4*256 per octave, positive means higher freq, e.g 4.10 format.
	s16 v = data & 0x2000 ? data | 0xc000 : data;
	if(0 && m_pitch[chan] != data)
		logerror("snd chan %02x pitch %c%c %d.%03x\n", chan, data & 0x8000 ? '#' : '.', data & 0x4000 ? '#' : '.', v / 1024, (v < 0 ? -v : v) & 0x3ff);
	m_pitch[chan] = data;
}

u16 swp30_device::attack_r(offs_t offset)
{
	return m_attack[offset >> 6];
}

void swp30_device::attack_w(offs_t offset, u16 data)
{
	if(data != m_attack[offset >> 6])
		logerror("attack[%02x] = %04x\n", offset >> 6, data);
	m_attack[offset >> 6] = data;
}

u16 swp30_device::decay1_r(offs_t offset)
{
	return m_decay1[offset >> 6];
}

void swp30_device::decay1_w(offs_t offset, u16 data)
{
	logerror("decay1[%02x] = %04x\n", offset >> 6, data);
	m_decay1[offset >> 6] = data;
}

u16 swp30_device::decay2_r(offs_t offset)
{
	return m_decay2[offset >> 6];
}

void swp30_device::decay2_w(offs_t offset, u16 data)
{
	logerror("decay2[%02x] = %04x\n", offset >> 6, data);
	m_decay2[offset >> 6] = data;
}

u16 swp30_device::lfo_step_pmod_r(offs_t offset)
{
	return m_lfo_step_pmod[offset >> 6];
}

void swp30_device::lfo_step_pmod_w(offs_t offset, u16 data)
{
	//	logerror("lfo_step_pmod[%02x] = %04x\n", offset >> 6, data);
	m_lfo_step_pmod[offset >> 6] = data;
}

u16 swp30_device::lfo_amod_r(offs_t offset)
{
	return m_lfo_amod[offset >> 6];
}

void swp30_device::lfo_amod_w(offs_t offset, u16 data)
{
	//	logerror("lfo_amod[%02x] = %04x\n", offset >> 6, data);
	m_lfo_amod[offset >> 6] = data;
}

u16 swp30_device::sample_start_h_r(offs_t offset)
{
	return m_sample_start[offset >> 6] >> 16;
}

u16 swp30_device::sample_start_l_r(offs_t offset)
{
	return m_sample_start[offset >> 6];
}

void swp30_device::sample_start_h_w(offs_t offset, u16 data)
{
	u8 chan = offset >> 6;
	m_sample_start[chan] = (m_sample_start[chan] & 0x0000ffff) | (data << 16);
}

void swp30_device::sample_start_l_w(offs_t offset, u16 data)
{
	u8 chan = offset >> 6;
	m_sample_start[chan] = (m_sample_start[chan] & 0xffff0000) | data;
}

u16 swp30_device::sample_end_h_r(offs_t offset)
{
	return m_sample_end[offset >> 6] >> 16;
}

u16 swp30_device::sample_end_l_r(offs_t offset)
{
	return m_sample_end[offset >> 6];
}

void swp30_device::sample_end_h_w(offs_t offset, u16 data)
{
	u8 chan = offset >> 6;
	m_sample_end[chan] = (m_sample_end[chan] & 0x0000ffff) | (data << 16);
}

void swp30_device::sample_end_l_w(offs_t offset, u16 data)
{
	u8 chan = offset >> 6;
	m_sample_end[chan] = (m_sample_end[chan] & 0xffff0000) | data;
	if(0)
		logerror("snd chan %02x post-size %02x %06x\n", chan, m_sample_end[chan] >> 24, m_sample_end[chan] & 0xffffff);
}

u16 swp30_device::sample_address_h_r(offs_t offset)
{
	return m_sample_address[offset >> 6] >> 16;
}

u16 swp30_device::sample_address_l_r(offs_t offset)
{
	return m_sample_address[offset >> 6];
}

void swp30_device::sample_address_h_w(offs_t offset, u16 data)
{
	u8 chan = offset >> 6;
	m_sample_address[chan] = (m_sample_address[chan] & 0x0000ffff) | (data << 16);
}

void swp30_device::sample_address_l_w(offs_t offset, u16 data)
{
	u8 chan = offset >> 6;
	static const char *const formats[4] = { "l16", "l12", "l8", "x8" };
	m_sample_address[chan] = (m_sample_address[chan] & 0xffff0000) | data;
	if(0)
		logerror("snd chan %02x format %s flags %02x address %06x\n", chan, formats[m_sample_address[chan] >> 30], (m_sample_address[chan] >> 24) & 0x3f, m_sample_address[chan] & 0xffffff);
}

u16 swp30_device::internal_adr_r()
{
	return m_internal_adr;
}

void swp30_device::internal_adr_w(u16 data)
{
	m_internal_adr = data;
}

u16 swp30_device::internal_r()
{
	u8 chan = m_internal_adr & 0x3f;
	switch(m_internal_adr >> 8) {
	case 0:
		// Not certain about the two top bits though, the code seems to only care about 0/non-0
		return m_envelope_mode[chan] == IDLE ? 0xffff : ((m_envelope_mode[chan] - 1) << 14) | (m_envelope_level[chan] >> (28-14));

	case 4:
		// used at 44c4
		// tests & 0x4000 only
		//      logerror("read %02x.4\n", chan);
		return 0x0000;

	case 6:
		return m_decay2_done[chan] ? 0x0000 : 0x8000;
	}

	logerror("%s internal_r port %x channel %02x sample %d\n", machine().time().to_string(), m_internal_adr >> 8, m_internal_adr & 0x1f, scount);
	machine().debug_break();

	return 0;
}


// MEG registers

template<int sel> u16 swp30_device::meg_const_r(offs_t offset)
{
	return m_meg_const[(offset >> 6)*6 + sel];
}

template<int sel> void swp30_device::meg_const_w(offs_t offset, u16 data)
{
	m_meg_const[(offset >> 6)*6 + sel] = data;
}

template<int sel> u16 swp30_device::meg_offset_r(offs_t offset)
{
	return m_meg_offset[(offset >> 6)*2 + sel];
}

template<int sel> void swp30_device::meg_offset_w(offs_t offset, u16 data)
{
	m_meg_offset[(offset >> 6)*2 + sel] =  data;
}

template<int sel> u16 swp30_device::meg_lfo_r(offs_t offset)
{
	return m_meg_lfo[(offset >> 6)*2 + sel];
}

template<int sel> void swp30_device::meg_lfo_w(offs_t offset, u16 data)
{
	int slot = (offset >> 6)*2 + sel; 
	m_meg_lfo[slot] = data;

	static const int dt[8] = { 0, 32, 64, 128, 256, 512,  1024, 2048 };
	static const int sh[8] = { 0,  0,  1,   2,   3,   4,     5,    6 };

	int scale = (data >> 5) & 7;
	int step = ((data & 31) << sh[scale]) + dt[scale];
	logerror("lfo_w %02x %04x freq=%5.2f phase=%6.4f\n", slot, m_meg_lfo[slot], step * 44100.0/4194304, (data >> 8)/256.0);
}



// Catch-all

static u16 rr[0x40*0x40];

u16 swp30_device::snd_r(offs_t offset)
{
	if(0) {
		int chan = (offset >> 6) & 0x3f;
		int slot = offset & 0x3f;
		std::string preg = "-";
		if(slot >= 0x21 && slot <= 0x2b && (slot & 1))
			preg = util::string_format("fp%03x", (slot-0x21)/2 + 6*chan);
		else if(slot == 0x30 || slot == 0x31)
			preg = util::string_format("dt%02x", (slot-0x30) + 2*chan);
		else if(slot == 0x0e || slot == 0x0f)
			preg = util::string_format("ct%02x", (slot-0x0e) + 2*chan);
		else
			preg = util::string_format("%02x.%02x", chan, slot);
		logerror("snd_r [%04x %04x] %-5s, %04x\n", offset, offset*2, preg, rr[offset]);
	}
	if(offset == 0x080f)
		return 0;
	return rr[offset];
}

void swp30_device::snd_w(offs_t offset, u16 data)
{
	if(rr[offset] == data)
		return;

	rr[offset] = data;

	int chan = (offset >> 6) & 0x3f;
	int slot = offset & 0x3f;

	if(offset == 0x04e)
		return;

	std::string preg = "-";
	if(slot >= 0x21 && slot <= 0x2b && (slot & 1))
		preg = util::string_format("fp%03x", (slot-0x21)/2 + 6*chan);
	else if(slot == 0x0e || slot == 0x0f)
		preg = util::string_format("sy%02x", (slot-0x0e) + 2*chan);
	else if(slot == 0x30 || slot == 0x31)
		preg = util::string_format("dt%02x", (slot-0x30) + 2*chan);
	else if(slot >= 0x38 && slot <= 0x3a)
		preg = util::string_format("mix[%x, %02x]", slot - 0x38, chan);
	else if(slot >= 0x3b && slot <= 0x3d)
		preg = util::string_format("route[%x, %02x]", slot - 0x3b, chan);
	else if(slot == 0x3e || slot == 0x3f)
		preg = util::string_format("lfo[%02x]", (slot-0x3e) + 2*chan);
	else
		preg = util::string_format("%02x.%02x", chan, slot);

	logerror("snd_w [%04x %04x] %-5s, %04x\n", offset, offset*2, preg, data);
}



// Synthesis

void swp30_device::sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs)
{
	if(outputs[0].samples() != 1)
		fatalerror("Sync stream not sync?\n");

	scount++;

	s32 dry_l = 0, dry_r = 0;

	// Loop on channels
	for(int chan = 0; chan < 64; chan++) {
		if(m_envelope_mode[chan] == IDLE)
			continue;

		// There actually are three shapes (0000, 4000 and c000) but
		// we're not sure what they are

		u32 lfo_phase = m_lfo_phase[chan] >> 7;
		s32 lfo_p_phase = lfo_phase ^ (m_lfo_step_pmod[chan] & 0xc000 ? lfo_shape_centered_tri : lfo_shape_centered_saw)[lfo_phase >> 18];
		s32 lfo_a_phase = lfo_phase ^ (m_lfo_step_pmod[chan] & 0xc000 ? lfo_shape_offset_tri   : lfo_shape_offset_saw  )[lfo_phase >> 18];

		// First, read the sample
		
		// - Find the base sample index and base address
		s32 sample_pos = m_sample_pos[chan];
		if(m_sample_end[chan] & 0x80000000)
			sample_pos = -sample_pos;

		s32 spos = sample_pos >> 8;
		offs_t base_address = m_sample_address[chan] & 0x1ffffff;
		
		// - Read/decompress the sample
		s16 val0, val1;
		switch(m_sample_address[chan] >> 30) {
		case 0: { // 16-bits linear
			offs_t adr = base_address + (spos >> 1);
			switch(spos & 1) {
			case 0: { // ABCDabcd ........
				u32 l0 = m_rom_cache.read_dword(adr);
				val0 = l0;
				val1 = l0 >> 16;
				break;
			}
			case 1: { // abcd.... ....ABCD
				u32 l0 = m_rom_cache.read_dword(adr);
				u32 l1 = m_rom_cache.read_dword(adr+1);
				val0 = l0 >> 16;
				val1 = l1;
				break;
			}
			}
			break;
		}

		case 1: { // 12-bits linear
			offs_t adr = base_address + (spos >> 3)*3;
			switch(spos & 7) {
			case 0: { // ..ABCabc ........ ........ ........
				u32 l0 = m_rom_cache.read_dword(adr);
				val0 =  (l0 & 0x00000fff) << 4;
				val1 =  (l0 & 0x00fff000) >> 8;
				break;
			}
			case 1: { // BCabc... .......A ........ ........
				u32 l0 = m_rom_cache.read_dword(adr);
				u32 l1 = m_rom_cache.read_dword(adr+1);
				val0 =  (l0 & 0x00fff000) >> 8;
				val1 = ((l0 & 0xff000000) >> 20) | ((l1 & 0x0000000f) << 12);
				break;
			}
			case 2: { // bc...... ....ABCa ........ ........
				u32 l0 = m_rom_cache.read_dword(adr);
				u32 l1 = m_rom_cache.read_dword(adr+1);
				val0 = ((l0 & 0xff000000) >> 20) | ((l1 & 0x0000000f) << 12);
				val1 =   l1 & 0x0000fff0;
				break;
			}
			case 3: { // ........ .ABCabc. ........ ........
				u32 l1 = m_rom_cache.read_dword(adr+1);
				val0 =   l1 & 0x0000fff0;
				val1 =  (l1 & 0x0fff0000) >> 12;
				break;
			}
			case 4: { // ........ Cabc.... ......AB ........
				u32 l1 = m_rom_cache.read_dword(adr+1);
				u32 l2 = m_rom_cache.read_dword(adr+2);
				val0 =  (l1 & 0x0fff0000) >> 12;
				val1 = ((l1 & 0xf0000000) >> 24) | ((l2 & 0x000000ff) << 8);
				break;
			}
			case 5: { // ........ c....... ...ABCab ........
				u32 l1 = m_rom_cache.read_dword(adr+1);
				u32 l2 = m_rom_cache.read_dword(adr+2);
				val0 = ((l1 & 0xf0000000) >> 24) | ((l2 & 0x000000ff) << 8);
				val1 =  (l2 & 0x000fff00) >> 4;
				break;
			}
			case 6: { // ........ ........ ABCabc.. ........
				u32 l2 = m_rom_cache.read_dword(adr+2);
				val0 =  (l2 & 0x000fff00) >> 4;
				val1 =  (l2 & 0xfff00000) >> 16;
				break;
			}
			case 7: { // ........ ........ abc..... .....ABC
				u32 l2 = m_rom_cache.read_dword(adr+2);
				u32 l3 = m_rom_cache.read_dword(adr+3);
				val0 =  (l2 & 0xfff00000) >> 16;
				val1 =  (l3 & 0x00000fff) << 4;
				break;
			}
			}
			break;
		}

		case 2: { // 8-bits linear
			offs_t adr = base_address + (spos >> 2);
			switch(spos & 3) {
			case 0: { // ....ABab ........
				u32 l0 = m_rom_cache.read_dword(adr);
				val0 = (l0 & 0x000000ff) << 8;
				val1 =  l0 & 0x0000ff00;
				break;
			}
			case 1: { // ..ABab.. ........
				u32 l0 = m_rom_cache.read_dword(adr);
				val0 =  l0 & 0x0000ff00;
				val1 = (l0 & 0x00ff0000) >> 8;
				break;
			}
			case 2: { // ABab.... ........
				u32 l0 = m_rom_cache.read_dword(adr);
				val0 = (l0 & 0x00ff0000) >> 8;
				val1 = (l0 & 0xff000000) >> 16;
				break;
			}
			case 3: { // ab...... ......AB
				u32 l0 = m_rom_cache.read_dword(adr);
				u32 l1 = m_rom_cache.read_dword(adr+1);
				val0 = (l0 & 0xff000000) >> 16;
				val1 = (l1 & 0x000000ff) << 8;
				break;
			}
			}
			break;
		}

		case 3: { // 8-bits delta-pcm
			offs_t adr = m_dpcm_address[chan];
			if(m_sample_end[chan] & 0x80000000) {
				u32 target_address = (base_address << 2) + spos - 1;
				while(adr >= target_address) {
					m_dpcm_current[chan] = m_dpcm_next[chan];
					s32 sample = m_dpcm_next[chan] + m_dpcm[(m_rom_cache.read_dword(adr >> 2) >> (8*(adr & 3))) & 0xff];
					adr --;
					if(sample < -0x8000)
						sample = -0x8000;
					else if(sample > 0x7fff)
						sample = 0x7fff;
					m_dpcm_next[chan] = sample;
				}
			} else {
				u32 target_address = (base_address << 2) + spos + 1;
				while(adr <= target_address) {
					m_dpcm_current[chan] = m_dpcm_next[chan];
					s32 sample = m_dpcm_next[chan] + m_dpcm[(m_rom_cache.read_dword(adr >> 2) >> (8*(adr & 3))) & 0xff];
					adr ++;
					if(sample < -0x8000)
						sample = -0x8000;
					else if(sample > 0x7fff)
						sample = 0x7fff;
					m_dpcm_next[chan] = sample;
				}
			}
			m_dpcm_address[chan] = adr;
			val0 = m_dpcm_current[chan];
			val1 = m_dpcm_next[chan];
			break;
		}
		}

		s32 mul = sample_pos & 0xff;
		s32 sample = val1 * mul + val0 * (0x100 - mul);

#if 0
		// Third, filter the sample
		// - missing lpf_cutoff, lpf_reso, hpf_cutoff

		// - eq lowpass
		s32 samp1 = (samp  * m_eq_filter[chan][2] + m_sample_history[chan][0][0] * m_eq_filter[chan][1] + m_sample_history[chan][0][1] * m_eq_filter[chan][0]) >> 13;
		m_sample_history[chan][0][1] = m_sample_history[chan][0][0];
		m_sample_history[chan][0][0] = samp;

		// - eq highpass
		s32 samp2 = (samp1 * m_eq_filter[chan][5] + m_sample_history[chan][1][0] * m_eq_filter[chan][4] + m_sample_history[chan][1][1] * m_eq_filter[chan][3]) >> 13;
		m_sample_history[chan][1][1] = m_sample_history[chan][1][0];
		m_sample_history[chan][1][0] = samp1;

#endif

		s32 tremolo_level = (lfo_a_phase * (m_lfo_amod[chan] & 0x1f)) << ((m_lfo_step_pmod[chan] & 0xc000) ? 3 : 2);

		dry_l += fpapply(m_envelope_level[chan] + (m_glo_level_cur[chan] << 16) + tremolo_level + ((m_dry_rev[chan] & 0xff00) << 12) + (m_pan_l[chan] << 16), sample);
		dry_r += fpapply(m_envelope_level[chan] + (m_glo_level_cur[chan] << 16) + tremolo_level + ((m_dry_rev[chan] & 0xff00) << 12) + (m_pan_r[chan] << 16), sample);

		istep(m_glo_level_cur[chan], (m_release_glo[chan] & 0x00ff) << 4, 1);
		istep(m_pan_l[chan], (m_pan[chan] & 0xff00) >> 4, 1);
		istep(m_pan_r[chan], (m_pan[chan] & 0x00ff) << 4, 1);

		m_lfo_phase[chan] = (m_lfo_phase[chan] + m_global_step[0x20 + ((m_lfo_step_pmod[chan] >> 8) & 0x3f)]) & 0x7ffffff;

		u32 sample_increment = (((m_pitch[chan] & 0x3ff) | 0x400) << (8 + (s16(m_pitch[chan] << 2) >> 12))) >> 10;
		m_sample_pos[chan] += (sample_increment * (0x800 + ((lfo_p_phase * (m_lfo_step_pmod[chan] & 0xff)) >> (m_lfo_step_pmod[chan] & 0xc000 ? 18 : 19)))) >> 11;
		if((m_sample_pos[chan] >> 8) >= (m_sample_end[chan] & 0xffffff)) {
			if(!(m_sample_end[chan] & 0xffffff))
				m_envelope_mode[chan] = IDLE;
			else {
				s32 prev = m_sample_pos[chan];
				do
					m_sample_pos[chan] = m_sample_pos[chan] - ((m_sample_end[chan] & 0xffffff) << 8) + ((m_sample_address[chan] >> 22) & 0xfc);
				while((m_sample_pos[chan] >> 8) >= (m_sample_end[chan] & 0xffffff));
				if(m_sample_end[chan] & 0x80000000)
					m_dpcm_address[chan] -= (m_sample_pos[chan] >> 8) - (prev >> 8);
				else
					m_dpcm_address[chan] += (m_sample_pos[chan] >> 8) - (prev >> 8);
			}
		}

		//		if(chan == 3)
		//			logerror("timer %d %08x\n", m_envelope_mode[chan], m_envelope_timer[chan]);
		switch(m_envelope_mode[chan]) {
		case ATTACK:
			if(m_envelope_on_timer[chan]) {
				if(istep(m_envelope_timer[chan], 0, m_global_step[(m_attack[chan] >> 8) & 0x7f] << 1))
					change_mode_attack_decay1(chan);
			} else {
				if(fpstep(m_envelope_level[chan], 0, attack_linear_step[(m_attack[chan] >> 8) & 0x7f]))
					change_mode_attack_decay1(chan);
			}
			break;

		case DECAY1:
			if(m_envelope_on_timer[chan]) {
				if(istep(m_envelope_timer[chan], 0, m_global_step[(m_decay1[chan] >> 8) & 0x7f] << 1))
					change_mode_decay1_decay2(chan);
			} else if((m_decay1[chan] & 0x6000) == 0x6000) {
				if(fpstep(m_envelope_level[chan], (m_decay1[chan] & 0xff) << 20, decay_linear_step[(m_decay1[chan] >> 8) & 0x1f]))
					change_mode_decay1_decay2(chan);
			} else {
				if(fpstep(m_envelope_level[chan], (m_decay1[chan] & 0xff) << 20, m_global_step[(m_decay1[chan] >> 8) & 0x7f]))
					change_mode_decay1_decay2(chan);
			}
			break;

		case DECAY2:
			if(m_envelope_on_timer[chan])
				m_decay2_done[chan] = istep(m_envelope_timer[chan], 0, m_global_step[(m_decay1[chan] >> 8) & 0x7f] << 1);
			else if((m_decay2[chan] & 0x6000) == 0x6000)
				m_decay2_done[chan] = fpstep(m_envelope_level[chan], (m_decay2[chan] & 0xff) << 20, decay_linear_step[(m_decay2[chan] >> 8) & 0x1f]);
			else
				m_decay2_done[chan] = fpstep(m_envelope_level[chan], (m_decay2[chan] & 0xff) << 20, m_global_step[(m_decay2[chan] >> 8) & 0x7f]);
			break;

		case RELEASE:
			if((m_release_glo[chan] & 0x6000) == 0x6000) {
				if(fpstep(m_envelope_level[chan], 0x8000000, decay_linear_step[(m_release_glo[chan] >> 8) & 0x1f]))
					m_envelope_mode[chan] = IDLE;
			} else {
				if(fpstep(m_envelope_level[chan], 0x8000000, m_global_step[(m_release_glo[chan] >> 8) & 0x7f]))
					m_envelope_mode[chan] = IDLE;
			}
			break;
		}
	}
		
	outputs[0].put_int_clamp(0, dry_l >> 8, 32768);
	outputs[1].put_int_clamp(0, dry_r >> 8, 32768);
}

void swp30_device::change_mode_attack_decay1(int chan)
{
	m_envelope_mode[chan] = DECAY1;
	m_envelope_timer[chan] = 0x8000000;
	m_envelope_on_timer[chan] = (m_decay1[chan] & 0xff) == 0;
}

void swp30_device::change_mode_decay1_decay2(int chan)
{
	m_envelope_mode[chan] = DECAY2;
	m_envelope_timer[chan] = 0x8000000;
	m_envelope_on_timer[chan] = (m_decay2[chan] & 0xff) == (m_decay1[chan] & 0xff);
}

uint32_t swp30_device::execute_min_cycles() const noexcept
{
	return 1;
}

uint32_t swp30_device::execute_max_cycles() const noexcept
{
	return 1;
}

uint32_t swp30_device::execute_input_lines() const noexcept
{
	return 0;
}

void swp30_device::execute_run()
{
	debugger_instruction_hook(m_meg_pc);
	m_icount = 0;
}

void swp30_device::meg_prg_map(address_map &map)
{
	map(0x000, 0x1bf).r(FUNC(swp30_device::meg_prg_map_r));
}

u64 swp30_device::meg_prg_map_r(offs_t address)
{
	return m_meg_program[address];
}

u16 swp30_device::swp30d_const_r(u16 address) const
{
	return m_meg_const[address];
}

u16 swp30_device::swp30d_offset_r(u16 address) const
{
	return m_meg_offset[address];
}

device_memory_interface::space_config_vector swp30_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config),
		std::make_pair(AS_DATA,    &m_rom_config),
	};
}

std::unique_ptr<util::disasm_interface> swp30_device::create_disassembler()
{
	return std::make_unique<swp30_disassembler>(this);
}

void swp30_device::state_import(const device_state_entry &entry)
{
}

void swp30_device::state_export(const device_state_entry &entry)
{
}

void swp30_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
}

