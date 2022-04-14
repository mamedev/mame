// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Yamaha SWP30/30B, rompler/dsp combo

#include "emu.h"
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

  Names we'll use in th rest of the text:
  - reg(y, x) is the register at address 2*(y*0x40 + x)
  - ch<nn>  is reg(channel, xx) for a given AWG2 channel
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
  ch07       decay,   bit 14-8 = step, bit 7-0 = target attenuation (top 8 bits)
  ch08       release, bit 14-8 = step, bit 7-0 = target attenuation (top 8 bits)
  ch09       base volume  bit 15 = activate release, bit 14-8 unknown, bit 7-0 = initial attenuation

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

  For decay and release the range is 1..120, e.g. 9 to 0x40000, or
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

swp30_device::swp30_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SWP30, tag, owner, clock),
	  device_sound_interface(mconfig, *this),
	  device_rom_interface(mconfig, *this),
	  m_meg(*this, "meg")
{
	(void)m_map;
}

void swp30_device::device_add_mconfig(machine_config &config)
{
	MEGEMB(config, m_meg);
}

void swp30_device::device_start()
{
	m_stream = stream_alloc(0, 2, 44100, STREAM_SYNCHRONOUS);

	// Attenuation for panning is 4.4 floating point.  That means 0
	// to -96.3dB.  Since it's a nice range, we assume it's the same
	// for other attenuation values.  Computed value is 1.16
	// format, to avoid overflow

	for(int i=0; i<256; i++)
		m_linear_attenuation[i] = ((32 - (i & 15)) << (0xf ^ (i >> 4))) >> 4;

	// Relative playback frequency of a sample is encoded on signed 14
	// bits.  The scale is logarithmic, with 0x400 = 1 octave (e.g. *2
	// or /2).

	for(int i=-0x2000; i<0x2000; i++)
		m_sample_increment[i & 0x3fff] = 256 * pow(2, i/1024.0);

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
		m_sample_log8[i] =  m_sample_log8[i] << 6;
		m_sample_log8[i | 0x80] = -m_sample_log8[i];
	}

	save_item(NAME(m_program));

	save_item(NAME(m_keyon_mask));

	save_item(NAME(m_pre_size));
	save_item(NAME(m_post_size));
	save_item(NAME(m_address));

	save_item(NAME(m_sample_pos));
	save_item(NAME(m_sample_history));

	save_item(NAME(m_program_pfp));
	save_item(NAME(m_program_pint));
	save_item(NAME(m_program_plfo));

	save_item(NAME(m_base_volume));
	save_item(NAME(m_current_volume));
	save_item(NAME(m_mode));
	save_item(NAME(m_freq));
	save_item(NAME(m_pan));
	save_item(NAME(m_attack));
	save_item(NAME(m_decay));
	save_item(NAME(m_release));
	save_item(NAME(m_lpf_cutoff));
	save_item(NAME(m_lpf_cutoff_inc));
	save_item(NAME(m_lpf_reso));
	save_item(NAME(m_hpf_cutoff));
	save_item(NAME(m_eq_filter));
	save_item(NAME(m_routing));

	save_item(NAME(m_internal_adr));

	save_item(NAME(m_program_address));
	save_item(NAME(m_waverom_adr));
	save_item(NAME(m_waverom_mode));
	save_item(NAME(m_waverom_access));
	save_item(NAME(m_waverom_val));
}

void swp30_device::device_reset()
{
	memset(m_program, 0, sizeof(m_program));

	m_keyon_mask = 0;

	memset(m_pre_size, 0, sizeof(m_pre_size));
	memset(m_post_size, 0, sizeof(m_post_size));
	memset(m_address, 0, sizeof(m_address));

	memset(m_sample_pos, 0, sizeof(m_sample_pos));
	memset(m_sample_history, 0, sizeof(m_sample_history));

	memset(m_program_pfp, 0, sizeof(m_program_pfp));
	memset(m_program_pint, 0, sizeof(m_program_pint));
	memset(m_program_plfo, 0, sizeof(m_program_plfo));

	memset(m_base_volume, 0, sizeof(m_base_volume));
	memset(m_current_volume, 0, sizeof(m_current_volume));
	memset(m_mode, IDLE, sizeof(m_mode));
	memset(m_freq, 0, sizeof(m_freq));
	memset(m_pan, 0, sizeof(m_pan));
	memset(m_attack, 0, sizeof(m_attack));
	memset(m_decay, 0, sizeof(m_decay));
	memset(m_release, 0, sizeof(m_release));
	memset(m_lpf_cutoff, 0, sizeof(m_lpf_cutoff));
	memset(m_lpf_cutoff_inc, 0, sizeof(m_lpf_cutoff_inc));
	memset(m_lpf_reso, 0, sizeof(m_lpf_reso));
	memset(m_hpf_cutoff, 0, sizeof(m_hpf_cutoff));
	memset(m_eq_filter, 0, sizeof(m_eq_filter));
	memset(m_routing, 0, sizeof(m_routing));

	m_program_address = 0;
	m_waverom_adr = 0;
	m_waverom_mode = 0;
	m_waverom_access = 0;
	m_waverom_val = 0;
}

void swp30_device::rom_bank_updated()
{
	// Nothing to do, stream is synchronous
}

void swp30_device::map(address_map &map)
{
	map(0x0000, 0x1fff).rw(FUNC(swp30_device::snd_r), FUNC(swp30_device::snd_w));

	rchan(map, 0x00).rw(FUNC(swp30_device::lpf_cutoff_r), FUNC(swp30_device::lpf_cutoff_w));
	rchan(map, 0x01).rw(FUNC(swp30_device::lpf_cutoff_inc_r), FUNC(swp30_device::lpf_cutoff_inc_w));
	rchan(map, 0x02).rw(FUNC(swp30_device::hpf_cutoff_r), FUNC(swp30_device::hpf_cutoff_w));
	// 03 seems to always get 5010 except at startup where it's 40ff
	rchan(map, 0x04).rw(FUNC(swp30_device::lpf_reso_r), FUNC(swp30_device::lpf_reso_w));
	// 05 missing
	rchan(map, 0x06).rw(FUNC(swp30_device::attack_r), FUNC(swp30_device::attack_w));
	rchan(map, 0x07).rw(FUNC(swp30_device::decay_r), FUNC(swp30_device::decay_w));
	rchan(map, 0x08).rw(FUNC(swp30_device::release_r), FUNC(swp30_device::release_w));
	rchan(map, 0x09).rw(FUNC(swp30_device::base_volume_r), FUNC(swp30_device::base_volume_w));
	// 0a-0d missing
	// 10 missing
	rchan(map, 0x11).rw(FUNC(swp30_device::freq_r), FUNC(swp30_device::freq_w));
	rchan(map, 0x12).rw(FUNC(swp30_device::pre_size_h_r), FUNC(swp30_device::pre_size_h_w));
	rchan(map, 0x13).rw(FUNC(swp30_device::pre_size_l_r), FUNC(swp30_device::pre_size_l_w));
	rchan(map, 0x14).rw(FUNC(swp30_device::post_size_h_r), FUNC(swp30_device::post_size_h_w));
	rchan(map, 0x15).rw(FUNC(swp30_device::post_size_l_r), FUNC(swp30_device::post_size_l_w));
	rchan(map, 0x16).rw(FUNC(swp30_device::address_h_r), FUNC(swp30_device::address_h_w));
	rchan(map, 0x17).rw(FUNC(swp30_device::address_l_r), FUNC(swp30_device::address_l_w));
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
	rctrl(map, 0x21).rw(FUNC(swp30_device::prg_address_r), FUNC(swp30_device::prg_address_w));
	rctrl(map, 0x22).rw(FUNC(swp30_device::prg_r<0>), FUNC(swp30_device::prg_w<0>));
	rctrl(map, 0x23).rw(FUNC(swp30_device::prg_r<1>), FUNC(swp30_device::prg_w<1>));
	rctrl(map, 0x24).rw(FUNC(swp30_device::prg_r<2>), FUNC(swp30_device::prg_w<2>));
	rctrl(map, 0x25).rw(FUNC(swp30_device::prg_r<3>), FUNC(swp30_device::prg_w<3>));
	// 26-7f missing
	rctrl(map, 0x30).rw(FUNC(swp30_device::map_r<0>), FUNC(swp30_device::map_w<0>));
	rctrl(map, 0x32).rw(FUNC(swp30_device::map_r<1>), FUNC(swp30_device::map_w<1>));
	rctrl(map, 0x34).rw(FUNC(swp30_device::map_r<2>), FUNC(swp30_device::map_w<2>));
	rctrl(map, 0x36).rw(FUNC(swp30_device::map_r<3>), FUNC(swp30_device::map_w<3>));
	rctrl(map, 0x38).rw(FUNC(swp30_device::map_r<4>), FUNC(swp30_device::map_w<4>));
	rctrl(map, 0x3a).rw(FUNC(swp30_device::map_r<5>), FUNC(swp30_device::map_w<5>));
	rctrl(map, 0x3c).rw(FUNC(swp30_device::map_r<6>), FUNC(swp30_device::map_w<6>));
	rctrl(map, 0x3e).rw(FUNC(swp30_device::map_r<7>), FUNC(swp30_device::map_w<7>));

	// MEG registers
	rchan(map, 0x21).rw(FUNC(swp30_device::prg_fp_r<0>), FUNC(swp30_device::prg_fp_w<0>));
	rchan(map, 0x23).rw(FUNC(swp30_device::prg_fp_r<1>), FUNC(swp30_device::prg_fp_w<1>));
	rchan(map, 0x25).rw(FUNC(swp30_device::prg_fp_r<2>), FUNC(swp30_device::prg_fp_w<2>));
	rchan(map, 0x27).rw(FUNC(swp30_device::prg_fp_r<3>), FUNC(swp30_device::prg_fp_w<3>));
	rchan(map, 0x29).rw(FUNC(swp30_device::prg_fp_r<4>), FUNC(swp30_device::prg_fp_w<4>));
	rchan(map, 0x2b).rw(FUNC(swp30_device::prg_fp_r<5>), FUNC(swp30_device::prg_fp_w<5>));
	rchan(map, 0x30).rw(FUNC(swp30_device::prg_off_r<0>), FUNC(swp30_device::prg_off_w<0>));
	rchan(map, 0x31).rw(FUNC(swp30_device::prg_off_r<1>), FUNC(swp30_device::prg_off_w<1>));
	rchan(map, 0x3e).rw(FUNC(swp30_device::prg_lfo_r<0>), FUNC(swp30_device::prg_lfo_w<0>));
	rchan(map, 0x3f).rw(FUNC(swp30_device::prg_lfo_r<1>), FUNC(swp30_device::prg_lfo_w<1>));
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
	for(int i=0; i<64; i++) {
		u64 mask = u64(1) << i;
		if(m_keyon_mask & mask) {
			m_sample_pos[i] = -s32(m_pre_size[i] << 8);
			m_current_volume[i] = (m_base_volume[i] & 0xff) << (26-8);
			change_mode(i, m_base_volume[i] & 0x8000 ? RELEASE : m_attack[i] & 0x80 ? DECAY : ATTACK);

			if(1)
				logerror("[%08d] keyon %02x %08x %08x %08x vol %04x env %04x %04x %04x pan %04x disp %04x %04x\n", scount, i, m_pre_size[i], m_post_size[i], m_address[i], m_base_volume[i], m_attack[i], m_decay[i], m_release[i], m_pan[i], m_dry_rev[i], m_cho_var[i]);
		}
	}
	m_keyon_mask = 0;
}


u16 swp30_device::prg_address_r()
{
	return m_program_address;
}

void swp30_device::prg_address_w(u16 data)
{
	m_program_address = data;
	if(m_program_address >= 0x180)
		m_program_address = 0;
}

template<int sel> u16 swp30_device::prg_r()
{
	constexpr offs_t shift = 48-16*sel;
	return m_meg->prg_r(m_program_address) >> shift;
}

template<int sel> void swp30_device::prg_w(u16 data)
{
	constexpr offs_t shift = 48-16*sel;
	constexpr u64 mask = ~(u64(0xffff) << shift);
	m_meg->prg_w(m_program_address, (m_meg->prg_r(m_program_address) & mask) | (u64(data) << shift));

	if(sel == 3) {
		if(0)
			logerror("program %03x %016x\n", m_program_address, m_program[m_program_address]);
		m_program_address ++;
		if(m_program_address == 0x180)
			m_program_address = 0;
	}
}


template<int sel> u16 swp30_device::map_r()
{
	return m_meg->map_r(sel);
}

template<int sel> void swp30_device::map_w(u16 data)
{
	m_meg->map_w(sel, data);
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
		m_waverom_val = read_dword(m_waverom_adr << 2);
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

u16 swp30_device::base_volume_r(offs_t offset)
{
	return m_base_volume[offset >> 6];
}

void swp30_device::base_volume_w(offs_t offset, u16 data)
{
	u8 chan = offset >> 6;
	if(1 && m_base_volume[chan] != data)
		logerror("snd chan %02x volume %02x %02x\n", chan, data >> 8, data & 0xff);
	m_base_volume[chan] = data;
	if((data & 0x8000) && m_mode[chan] != IDLE && m_mode[chan] != RELEASE)
		change_mode(chan, RELEASE);
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

u16 swp30_device::freq_r(offs_t offset)
{
	return m_freq[offset >> 6];
}

void swp30_device::freq_w(offs_t offset, u16 data)
{
	u8 chan = offset >> 6;
	//  delta is 4*256 per octave, positive means higher freq, e.g 4.10 format.
	s16 v = data & 0x2000 ? data | 0xc000 : data;
	if(0 && m_freq[chan] != data)
		logerror("snd chan %02x freq %c%c %d.%03x\n", chan, data & 0x8000 ? '#' : '.', data & 0x4000 ? '#' : '.', v / 1024, (v < 0 ? -v : v) & 0x3ff);
	m_freq[chan] = data;
}

u16 swp30_device::attack_r(offs_t offset)
{
	return m_attack[offset >> 6];
}

void swp30_device::attack_w(offs_t offset, u16 data)
{
	m_attack[offset >> 6] = data;
}

u16 swp30_device::decay_r(offs_t offset)
{
	return m_decay[offset >> 6];
}

void swp30_device::decay_w(offs_t offset, u16 data)
{
	m_decay[offset >> 6] = data;
}

u16 swp30_device::release_r(offs_t offset)
{
	return m_release[offset >> 6];
}

void swp30_device::release_w(offs_t offset, u16 data)
{
	m_release[offset >> 6] = data;
}

u16 swp30_device::pre_size_h_r(offs_t offset)
{
	return m_pre_size[offset >> 6] >> 16;
}

u16 swp30_device::pre_size_l_r(offs_t offset)
{
	return m_pre_size[offset >> 6];
}

void swp30_device::pre_size_h_w(offs_t offset, u16 data)
{
	u8 chan = offset >> 6;
	m_pre_size[chan] = (m_pre_size[chan] & 0x0000ffff) | (data << 16);
}

void swp30_device::pre_size_l_w(offs_t offset, u16 data)
{
	u8 chan = offset >> 6;
	m_pre_size[chan] = (m_pre_size[chan] & 0xffff0000) | data;
	if(0)
		logerror("snd chan %02x pre-size %02x %06x\n", chan, m_pre_size[chan] >> 24, m_pre_size[chan] & 0xffffff);
}

u16 swp30_device::post_size_h_r(offs_t offset)
{
	return m_post_size[offset >> 6] >> 16;
}

u16 swp30_device::post_size_l_r(offs_t offset)
{
	return m_post_size[offset >> 6];
}

void swp30_device::post_size_h_w(offs_t offset, u16 data)
{
	u8 chan = offset >> 6;
	m_post_size[chan] = (m_post_size[chan] & 0x0000ffff) | (data << 16);
}

void swp30_device::post_size_l_w(offs_t offset, u16 data)
{
	u8 chan = offset >> 6;
	m_post_size[chan] = (m_post_size[chan] & 0xffff0000) | data;
	if(0)
		logerror("snd chan %02x post-size %02x %06x\n", chan, m_post_size[chan] >> 24, m_post_size[chan] & 0xffffff);
}

u16 swp30_device::address_h_r(offs_t offset)
{
	return m_address[offset >> 6] >> 16;
}

u16 swp30_device::address_l_r(offs_t offset)
{
	return m_address[offset >> 6];
}

void swp30_device::address_h_w(offs_t offset, u16 data)
{
	u8 chan = offset >> 6;
	m_address[chan] = (m_address[chan] & 0x0000ffff) | (data << 16);
}

void swp30_device::address_l_w(offs_t offset, u16 data)
{
	u8 chan = offset >> 6;
	static const char *const formats[4] = { "l16", "l12", "l8", "x8" };
	m_address[chan] = (m_address[chan] & 0xffff0000) | data;
	if(0)
		logerror("snd chan %02x format %s flags %02x address %06x\n", chan, formats[m_address[chan] >> 30], (m_address[chan] >> 24) & 0x3f, m_address[chan] & 0xffffff);
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
		return m_mode[chan] == IDLE ? 0xffff : m_current_volume[chan] >> (26-14);

	case 4:
		// used at 44c4
		// tests & 0x4000 only
		//      logerror("read %02x.4\n", chan);
		return 0xffff;

	case 6:
		// used at 3e7c
		// tests & 0x8000 only, keyoff?
		logerror("read %02x.6\n", chan);
		return 0x0000;
	}

	logerror("%s internal_r port %x channel %02x sample %d\n", machine().time().to_string(), m_internal_adr >> 8, m_internal_adr & 0x1f, scount);
	machine().debug_break();

	return 0;
}


// MEG registers forwarding

template<int sel> u16 swp30_device::prg_fp_r(offs_t offset)
{
	return m_meg->fp_r((offset >> 6)*6 + sel);
}

template<int sel> void swp30_device::prg_fp_w(offs_t offset, u16 data)
{
	m_meg->fp_w((offset >> 6)*6 + sel, data);
}

template<int sel> u16 swp30_device::prg_off_r(offs_t offset)
{
	return m_meg->offset_r((offset >> 6)*2 + sel);
}

template<int sel> void swp30_device::prg_off_w(offs_t offset, u16 data)
{
	m_meg->offset_w((offset >> 6)*2 + sel, data);
}

template<int sel> u16 swp30_device::prg_lfo_r(offs_t offset)
{
	return m_meg->lfo_r((offset >> 6)*2 + sel);
}

template<int sel> void swp30_device::prg_lfo_w(offs_t offset, u16 data)
{
	m_meg->lfo_w((offset >> 6)*2 + sel, data);
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
	if(0 && offset == 0x080f)
		machine().debug_break();
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

	if(0 && slot == 0x03)
		machine().debug_break();

	std::string preg = "-";
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
		preg = util::string_format("%02x.%02x", chan, slot);
	//  if((slot >= 0xa && slot <= 0xd) || (slot >= 0x2c && slot <= 0x2f))
	//      machine().debug_break();

	logerror("snd_w [%04x %04x] %-5s, %04x\n", offset, offset*2, preg, data);
}



// Synthesis

void swp30_device::change_mode(int channel, u8 mode)
{
	if(1)
		logerror("[%08d] channel %02x mode %s\n", scount, channel,
				 mode == IDLE ? "idle" :
				 mode == ATTACK ? "attack" :
				 mode == DECAY ? "decay" :
				 mode == SUSTAIN ? "sustain" :
				 mode == RELEASE ? "release" :
				 "?");

	m_mode[channel] = mode;
	if(mode == IDLE || mode == SUSTAIN) {
		m_step_volume[channel] = 0;
		return;
	}

	u16 reg = mode == ATTACK ? m_attack[channel] & 0xff00 : mode == DECAY ? m_decay[channel] : m_release[channel];
	u32 target = (reg & 0xff) << (26-8);
	s32 step = (8 + ((reg >> 8) & 7)) << ((reg >> 11) & 15);
	if(mode != ATTACK)
		step *= 8;
	if(target < m_current_volume[channel])
		step = -step;
	m_target_volume[channel] = target;
	m_step_volume[channel] = step;

	if(1) {
		double delay = (double(target) - double(m_current_volume[channel])) / (44100*step);
		logerror("    -> time until hit %f seconds\n", delay);
	}
}

void swp30_device::sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs)
{
	if(outputs[0].samples() != 1)
		fatalerror("Sync stream not sync?\n");

	scount++;

	// Accumulate on 64 bits, shift/clamp at the end
	s64 dry_left = 0, dry_right = 0;

	// Loop on channels
	for(int channel = 0; channel < 64; channel++)
		if(m_mode[channel] != IDLE) {
			// First, read the sample

			// - Find the base sample index and base address
			s32 spos = m_sample_pos[channel] >> 8;
			offs_t base_address = (m_address[channel] & 0x1ffffff) << 2;
			// - Read/decompress the sample
			s16 samp = 0;
			switch(m_address[channel] >> 30) {
			case 0: { // 16-bits linear
				offs_t adr = base_address + (spos << 1);
				samp = read_word(adr);
				break;
			}

			case 1: { // 12-bits linear
				offs_t adr = base_address + (spos >> 2)*6;
				switch(spos & 3) {
				case 0: { // .abc .... ....
					u16 w0 = read_word(adr);
					samp = (w0 & 0x0fff) << 4;
					break;
				}
				case 1: { // C... ..AB ....
					u16 w0 = read_word(adr);
					u16 w1 = read_word(adr+2);
					samp = ((w0 & 0xf000) >> 8) | ((w1 & 0x00ff) << 8);
					break;
				}
				case 2: { // .... bc.. ...a
					u16 w0 = read_word(adr+2);
					u16 w1 = read_word(adr+4);
					samp = ((w0 & 0xff00) >> 4) | ((w1 & 0x000f) << 12);
					break;
				}
				case 3: { // .... .... ABC.
					u16 w1 = read_word(adr+4);
					samp = w1 & 0xfff0;
					break;
				}
				}
				break;
			}

			case 2:   // 8-bits linear
				samp = read_byte(base_address + spos) << 8;
				break;

			case 3:   // 8-bits logarithmic
				samp = m_sample_log8[read_byte(base_address + spos)];
				break;
			}

			//logerror("sample %02x %06x [%d] %+5d %04x  %04x %04x\n", channel, base_address >> 2, m_address[channel] >> 30, spos, samp & 0xffff, m_volume[channel], m_pan[channel]);

			// Second, step the sample pos, loop/deactivate as needed
			m_sample_pos[channel] += m_sample_increment[m_freq[channel] & 0x3fff];
			s32 loop_size = (m_post_size[channel] << 8) | ((m_address[channel] >> 22) & 0xf8);
			if(m_sample_pos[channel] >= loop_size) {
				// We reached the loop point, stop if loop size is zero,
				// otherwise loop
				if(!loop_size)
					change_mode(channel, IDLE);
				else
					do
						m_sample_pos[channel] -= loop_size;
					while(m_sample_pos[channel] >= loop_size);
			}

			// Third, filter the sample
			// - missing lpf_cutoff, lpf_reso, hpf_cutoff

			// - eq lowpass
			s32 samp1 = (samp  * m_eq_filter[channel][2] + m_sample_history[channel][0][0] * m_eq_filter[channel][1] + m_sample_history[channel][0][1] * m_eq_filter[channel][0]) >> 13;
			m_sample_history[channel][0][1] = m_sample_history[channel][0][0];
			m_sample_history[channel][0][0] = samp;

			// - eq highpass
			s32 samp2 = (samp1 * m_eq_filter[channel][5] + m_sample_history[channel][1][0] * m_eq_filter[channel][4] + m_sample_history[channel][1][1] * m_eq_filter[channel][3]) >> 13;
			m_sample_history[channel][1][1] = m_sample_history[channel][1][0];
			m_sample_history[channel][1][0] = samp1;

			// Fourth, establish the 8 volumes (only 2 for now, need the MEG) and update the envelope
			u32 raw_vol = m_current_volume[channel];
			u32 vol = raw_vol >> (26-8);

			u32 base_l = vol + (m_pan[channel] >> 8);
			u32 base_r = vol + (m_pan[channel] & 0xff);

			u32 dry_l = base_l + (m_dry_rev[channel] >> 8);
			u32 dry_r = base_r + (m_dry_rev[channel] >> 8);

			s32 step = m_step_volume[channel];
			u32 target = m_target_volume[channel];

			if(0) {
				u8 mode = m_mode[channel];
				logerror("[%08d] channel %02x state %s vol=%07x step=%c%07x target=%07x\n",
						 scount,
						 channel,
						 mode == IDLE ? "idle" :
						 mode == ATTACK ? "attack" :
						 mode == DECAY ? "decay" :
						 mode == SUSTAIN ? "sustain" :
						 mode == RELEASE ? "release" :
						 "?",
						 raw_vol,
						 step < 0 ? '-' : '+',
						 step < 0 ? -step : step,
						 target);
			}
			if(step) {
				raw_vol += step;
				if((step < 0 && (raw_vol <= target || raw_vol & 0x80000000)) ||
				   (step > 0 && raw_vol >= target)) {
					raw_vol = target;
					m_current_volume[channel] = raw_vol;

					// IDLE and SUSTAIN have zero step.
					// current volume must be updated before calling change_mode

					switch(m_mode[channel]) {
					case ATTACK:  change_mode(channel, DECAY);   break;
					case DECAY:   change_mode(channel, SUSTAIN); break;
					case RELEASE: change_mode(channel, IDLE);    break;
					}
				} else
					m_current_volume[channel] = raw_vol;
			}

			// Fifth, add to the (dry) accumulators
			dry_left  += samp2 * m_linear_attenuation[std::min(0xffu, dry_l)];
			dry_right += samp2 * m_linear_attenuation[std::min(0xffu, dry_r)];

			// Missing: reverb, chorus, effects in general
		}

	// Samples are 16 bits, there are up to 64 of them, and the accumulators are fixed-point signed 48.16
	// Global EQ is missing (it's done in the MEG)

	dry_left >>= 14;
	outputs[0].put_int_clamp(0, dry_left, 32768);

	dry_right >>= 14;
	outputs[1].put_int_clamp(0, dry_right, 32768);
}
