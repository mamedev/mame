// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Yamaha SWP30/30B, rompler/dsp combo

#include "emu.h"
#include "swp30.h"

/*
  The SWP30 is the combination of a rompler called AWM2 (Advanced Wave
  Memory 2) and an effects DSP called MEG (Multiple Effects
  Generator).  It also includes some routing/mixing capabilities,
  moving data between AWM2, MEG and serial inputs and outputs with
  volume management capabilities everywhere.  Its clock is 33.9MHz and
  the output is at 44100Hz stereo (768 cycles per sample pair) per dac
  output.

  I/O wise, the chip has 8 generic audio serial inputs and 8 outputs
  for external plugins, and two dac outputs.  The DAC output is
  stereo, and so is the first generic input.  It's unclear whether the
  outputs and the other inputs are stereo.  The MU100 connects a stereo ADC to the fist input, and routes 

  In practice the chip has the pin for a second DAC, so is
  probably 4-channel capable on output.

  The 


  The AWM2 manages 64 channels internally, and has inputs for 8
  external sources, one at least being stereo (they probably all are).
  The sound data can be four formats (8 bits, 12 bits, 16 bits, and a
  8-bits log format with roughly 10 bits of dynamic).  The rom bus is
  25 bits address and 32 bits data wide.  It applies four filters to
  the sample data, two of fixed type (low pass then highpass) and two
  free 3-point FIR filters (used for yet another lowpass and
  highpass).  Envelopes are handled automatically, and the final
  panned result is accumulated on four stereo accumulators which will
  be passed to the MEG.  Two of the channels (that includes the
  external ones) can also have their value sent to the MEG.




  The MEG is a DSP with 384 program steps.  Instructions are 64 bits
  wide, and to each instruction is associated a 2.14 fixed point value
  and, for every third instruction a 16-bit integer memory offset
  value.  In addition 24 LFOs are available, and possibly more.  It is
  connected to a dram of 262144 samples, theorically 18 bits but in
  practice only the top 16 bits are connected.

  The chip interface presents 4096 16-bits registers in a 64x64 grid.
  Some of this grid is for per-channel values for AWM2, but parts are
  isolated and renumbered for MEG regisrers or for general control
  functions.

  General register address: (64 * channel + slot) * 2 (16-bits values)
  MEG fixed point constants (n=0-383) : channel = n/6, slot = 0x21 + 2*(n%6)
  MEG integer constants (n=0-127): channel = n/2, slot = 0x30 + (n%2)
  MEG LFOs (n=0..23): channel = n/2, slot = 0x3e + (n%2)

  Control registers (n=0..127): channel = n/2, slot = 0x

  Note that the LFOs may be 128 instead of 24, but the mu100 code only
  reserves 24 values in its structures.  OTOH, the mu100 code never
  uses channel >= 12 slot 3e-3f either.


  AWM2 (per-channel) registers:
  slot(s)  function
  00       fixed LPF frequency cutoff index
  01       fixed LPF frequency cutoff index increment?
  02       fixed HPF frequency cutoff
  03       40ff at startup, 5010 always afterwards?
  04       fixed LPF resonane level
  05       unknown
  06-09    envelope information, not understood yet
  0a-0d    unknown, probably something to do with vibrato
  10       unknown
  11       channel replay frequency, signed 4.10 fixed point, log2 scale, positive is higher resulting frequency
  12-13    number of samples before the loop point
  14-15    number of samples in the loop
  16-17    bit 31-30 = sample format, bits 29-25 = loop samples decimal part, 24-0 = loop start address in rom
  20,22,24 first FIR coefficients
  26,28,2a second FIR coefficients
  2c-2f    unknown
  32       pan left/right, 2x8 bits of attenuation
  33-34    attenuation levels to add to the four accumulators (dry, reverb, chorus, variation for the mu100)
  35-37    routing, in particular for the taps.  Rather unclear

  Slots e-f are system control, 21,23,25,27,29,2b MEG fixed point
  registers, 30,31 MEG offset registers, 3e,3f MEG LFO registers.
  38-3d are special, not per-channel.

  Known system registers:
  number   function
  02       internal register selector, msb = 0 or 6, lsb = channel
  03       internal register read port, used for envelope/keyoff management
  0c-0f    keyon mask
  10       write something to trigger a keyon
  21       MEG program write address
  22-25    MEG program opcode, writing to 25 triggers an auto-increment
  30-3e    even slots only, MEG buffer mappings


  The LFO registers internal counters are 22 bits wide.  The LSB of
  the register gives the increment per sample, encoded in a special
  3.5 format.
  With scale = 3bits and v = 5bits, step = base[scale] + (v << shift[scale])
  base  = { 0, 32, 64, 128, 256, 512,  1024, 2048 }
  shift = { 0,  0,  1,   2,   3,   4,     5,    6 }

  The 21th bit of the counter inverts bits 20-0 on read, those are
  interpreted as a 0-1 value, giving a sawtooth wave.


  8 mappings can be setup, which allow to manage rotating buffers in
  the MEG-attached ram easily by automating masking and offset adding.
  The register format is: tttttsss oooooooo.  't' is not understood
  yet. 's' is the sub-buffer size, defined as 1 << (10+s).  The base
  offset is o << 10.  There are no alignment issues, e.g. you can have
  a buffer at 0x28000 which is 0x10000 samples long.


*/


DEFINE_DEVICE_TYPE(SWP30, swp30_device, "swp30", "Yamaha SWP30 sound chip")

swp30_device::swp30_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SWP30, tag, owner, clock),
	  device_sound_interface(mconfig, *this),
	  device_rom_interface(mconfig, *this, 25+2, ENDIANNESS_LITTLE, 32)
{
}

void swp30_device::device_start()
{
	m_stream = stream_alloc(0, 2, 44100);

	// Attenuantion for panning is 4.4 floating point.  That means 0
	// to -96.3dB.  Since it's a nice range, we assume it's the same
	// for other attenuation values.  Computed value is is 1.16
	// format, to avoid overflow

	for(int i=0; i<256; i++)
		m_linear_attenuation[i] = ((32 - (i & 15)) << (0xf ^ (i >> 4))) >> 4;

	// Relative playback frequency of a sample is encoded on signed 14
	// bits.  The scale is logarithmic, with 0x400 = 1 octave (e.g. *2
	// or /2).

	for(int i=-0x20000; i<0x2000; i++)
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
	save_item(NAME(m_active_mask));

	save_item(NAME(m_pre_size));
	save_item(NAME(m_post_size));
	save_item(NAME(m_address));

	save_item(NAME(m_sample_pos));
	save_item(NAME(m_sample_history));

	save_item(NAME(m_program_pfp));
	save_item(NAME(m_program_pint));
	save_item(NAME(m_program_plfo));

	save_item(NAME(m_volume));
	save_item(NAME(m_freq));
	save_item(NAME(m_pan));
	save_item(NAME(m_envelope));
	save_item(NAME(m_lpf_cutoff));
	save_item(NAME(m_lpf_cutoff_inc));
	save_item(NAME(m_lpf_reso));
	save_item(NAME(m_hpf_cutoff));
	save_item(NAME(m_eq_filter));
	save_item(NAME(m_routing));

	save_item(NAME(m_program_address));
}

void swp30_device::device_reset()
{
	memset(m_program, 0, sizeof(m_program));

	m_keyon_mask = 0;
	m_active_mask = 0;

	memset(m_pre_size, 0, sizeof(m_pre_size));
	memset(m_post_size, 0, sizeof(m_post_size));
	memset(m_address, 0, sizeof(m_address));

	memset(m_sample_pos, 0, sizeof(m_sample_pos));
	memset(m_sample_history, 0, sizeof(m_sample_history));

	memset(m_program_pfp, 0, sizeof(m_program_pfp));
	memset(m_program_pint, 0, sizeof(m_program_pint));
	memset(m_program_plfo, 0, sizeof(m_program_plfo));

	memset(m_volume, 0, sizeof(m_volume));
	memset(m_freq, 0, sizeof(m_freq));
	memset(m_pan, 0, sizeof(m_pan));
	memset(m_envelope, 0, sizeof(m_envelope));
	memset(m_lpf_cutoff, 0, sizeof(m_lpf_cutoff));
	memset(m_lpf_cutoff_inc, 0, sizeof(m_lpf_cutoff_inc));
	memset(m_lpf_reso, 0, sizeof(m_lpf_reso));
	memset(m_hpf_cutoff, 0, sizeof(m_hpf_cutoff));
	memset(m_eq_filter, 0, sizeof(m_eq_filter));
	memset(m_routing, 0, sizeof(m_routing));

	m_program_address = 0;
}

void swp30_device::rom_bank_updated()
{
	m_stream->update();
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
	rchan(map, 0x06).rw(FUNC(swp30_device::envelope_r<0>), FUNC(swp30_device::envelope_w<0>));
	rchan(map, 0x07).rw(FUNC(swp30_device::envelope_r<1>), FUNC(swp30_device::envelope_w<1>));
	rchan(map, 0x08).rw(FUNC(swp30_device::envelope_r<2>), FUNC(swp30_device::envelope_w<2>));
	rchan(map, 0x09).rw(FUNC(swp30_device::volume_r), FUNC(swp30_device::volume_w));
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
	// 00-0b missing
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
	rchan(map, 0x30).rw(FUNC(swp30_device::prg_int_r<0>), FUNC(swp30_device::prg_int_w<0>));
	rchan(map, 0x31).rw(FUNC(swp30_device::prg_int_r<1>), FUNC(swp30_device::prg_int_w<1>));
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
	m_stream->update();
	for(int i=0; i<64; i++) {
		u64 mask = u64(1) << i;
		if((m_keyon_mask & mask) && !(m_active_mask & mask) && !(m_volume[i] & 0x8000)) {
			m_sample_pos[i] = -s32(m_pre_size[i] << 8);
			logerror("keyon %02x %08x %08x %08x vol %04x pan %04x\n", i, m_pre_size[i], m_post_size[i], m_address[i], m_volume[i], m_pan[i]);
			m_active_mask |= mask;
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
	return m_program[m_program_address] >> shift;
}

template<int sel> void swp30_device::prg_w(u16 data)
{
	constexpr offs_t shift = 48-16*sel;
	constexpr u64 mask = ~(u64(0xffff) << shift);
	m_program[m_program_address] = (m_program[m_program_address] & mask) | (u64(data) << shift);

	if(sel == 3) {
		logerror("program %03x %016x\n", m_program_address, m_program[m_program_address]);
		m_program_address ++;
		if(m_program_address == 0x180)
			m_program_address = 0;
	}
}


template<int sel> u16 swp30_device::map_r()
{
	return m_map[sel];
}

template<int sel> void swp30_device::map_w(u16 data)
{
	m_map[sel] = data;
	logerror("map %d: type=%02x offset=%05x size=%05x\n", sel, data >> 11, (data & 0xff) << 10, 0x400 << ((data >> 8) & 7));
}


// AWM2 per-channel registers
u16 swp30_device::lpf_cutoff_r(offs_t offset)
{
	return m_lpf_cutoff[offset >> 6];
}

void swp30_device::lpf_cutoff_w(offs_t offset, u16 data)
{
	m_stream->update();
	u8 chan = offset >> 6;
	if(m_lpf_cutoff[chan] != data && 0)
		logerror("chan %02x lpf cutoff %04x\n", chan, data);
	m_lpf_cutoff[chan] = data;
}

u16 swp30_device::lpf_cutoff_inc_r(offs_t offset)
{
	return m_lpf_cutoff_inc[offset >> 6];
}

void swp30_device::lpf_cutoff_inc_w(offs_t offset, u16 data)
{
	m_stream->update();
	u8 chan = offset >> 6;
	if(m_lpf_cutoff_inc[chan] != data && 0)
		logerror("chan %02x lpf cutoff increment %04x\n", chan, data);
	m_lpf_cutoff_inc[chan] = data;
}

u16 swp30_device::hpf_cutoff_r(offs_t offset)
{
	return m_hpf_cutoff[offset >> 6];
}

void swp30_device::hpf_cutoff_w(offs_t offset, u16 data)
{
	m_stream->update();
	u8 chan = offset >> 6;
	if(m_hpf_cutoff[chan] != data)
		logerror("chan %02x hpf cutoff %04x\n", chan, data);
	m_hpf_cutoff[chan] = data;
}

u16 swp30_device::lpf_reso_r(offs_t offset)
{
	return m_lpf_reso[offset >> 6];
}

void swp30_device::lpf_reso_w(offs_t offset, u16 data)
{
	m_stream->update();
	u8 chan = offset >> 6;
	if(m_lpf_reso[chan] != data)
		logerror("chan %02x lpf resonance %04x\n", chan, data);
	m_lpf_reso[chan] = data;
}

template<int coef> u16 swp30_device::eq_filter_r(offs_t offset)
{
	return m_eq_filter[offset >> 6][coef];
}

template<int coef> void swp30_device::eq_filter_w(offs_t offset, u16 data)
{
	m_stream->update();
	m_eq_filter[offset >> 6][coef] = data;
}

template<int sel> u16 swp30_device::routing_r(offs_t offset)
{
	return m_routing[offset >> 6][sel];
}

template<int sel> void swp30_device::routing_w(offs_t offset, u16 data)
{
	m_stream->update();
	m_routing[offset >> 6][sel] = data;
}

u16 swp30_device::volume_r(offs_t offset)
{
	int chan = offset >> 6;
	return m_volume[chan];
}

void swp30_device::volume_w(offs_t offset, u16 data)
{
	m_stream->update();
	u8 chan = offset >> 6;
	if(m_volume[chan] != data)
		logerror("snd chan %02x volume %02x %02x\n", chan, data >> 8, data & 0xff);
	m_volume[chan] = data;
	if(data & 0x8000) {
		if(m_active_mask & (u64(1) << chan)) {
			if(m_post_size[chan])
				m_active_mask &= ~(u64(1) << chan);
		}
	}
}


u16 swp30_device::pan_r(offs_t offset)
{
	return m_pan[offset >> 6];
}

void swp30_device::pan_w(offs_t offset, u16 data)
{
	u8 chan = offset >> 6;
	if(m_pan[chan] != data)
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
	if(m_dry_rev[chan] != data)
		logerror("snd chan %02x dry %02x rev %02x\n", chan, data >> 8, data & 0xff);
	m_dry_rev[chan] = data;
}

u16 swp30_device::cho_var_r(offs_t offset)
{
	return m_dry_rev[offset >> 6];
}

void swp30_device::cho_var_w(offs_t offset, u16 data)
{
	u8 chan = offset >> 6;
	if(m_dry_rev[chan] != data)
		logerror("snd chan %02x cho %02x var %02x\n", chan, data >> 8, data & 0xff);
	m_dry_rev[chan] = data;
}

u16 swp30_device::freq_r(offs_t offset)
{
	return m_freq[offset >> 6];
}

void swp30_device::freq_w(offs_t offset, u16 data)
{
	u8 chan = offset >> 6;
	//	delta is 4*256 per octave, positive means higher freq, e.g 4.10 format.
	s16 v = data & 0x2000 ? data | 0xc000 : data;
	if(m_freq[chan] != data)
		logerror("snd chan %02x freq %c%c %d.%03x\n", chan, data & 0x8000 ? '#' : '.', data & 0x4000 ? '#' : '.', v / 1024, (v < 0 ? -v : v) & 0x3ff);
	m_freq[chan] = data;
}

template<int sel> u16 swp30_device::envelope_r(offs_t offset)
{
	return m_envelope[offset >> 6][sel];
}

template<int sel> void swp30_device::envelope_w(offs_t offset, u16 data)
{
	u8 chan = offset >> 6;
	bool ch = m_envelope[chan][sel] != data;
	m_envelope[chan][sel] = data;
	if(ch)
		logerror("snd chan %02x envelopes %04x %04x %04x\n", chan, m_envelope[chan][0], m_envelope[chan][1], m_envelope[chan][2]);
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
	// The address probably is 25 bits
	static const char *const formats[4] = { "l16", "l12", "l8", "x8" };
	m_address[chan] = (m_address[chan] & 0xffff0000) | data;
	logerror("snd chan %02x format %s flags %02x address %06x\n", chan, formats[m_address[chan] >> 30], (m_address[chan] >> 24) & 0x3f, m_address[chan] & 0xffffff);
}


// MEG registers (Multiple Effects Generator)

template<int sel> u16 swp30_device::prg_fp_r(offs_t offset)
{
	offs_t adr = (offset >> 6)*6 + sel;
	return m_program_pfp[adr];
}

template<int sel> void swp30_device::prg_fp_w(offs_t offset, u16 data)
{
	offs_t adr = (offset >> 6)*6 + sel;
	m_program_pfp[adr] = data;
	logerror("prg_fp_w %03x, %04x\n", adr, data);
}

template<int sel> u16 swp30_device::prg_int_r(offs_t offset)
{
	offs_t adr = (offset >> 6)*2 + sel;
	return m_program_pint[adr];
}

template<int sel> void swp30_device::prg_int_w(offs_t offset, u16 data)
{
	offs_t adr = (offset >> 6)*2 + sel;
	m_program_pint[adr] = data;
	logerror("prg_int_w %02x, %04x\n", adr, data);
}

template<int sel> u16 swp30_device::prg_lfo_r(offs_t offset)
{
	offs_t adr = (offset >> 6)*2 + sel;
	return m_program_plfo[adr];
}

template<int sel> void swp30_device::prg_lfo_w(offs_t offset, u16 data)
{
	offs_t adr = (offset >> 6)*2 + sel;
	m_program_plfo[adr] = data;

    static const int dt[8] = { 0, 32, 64, 128, 256, 512,  1024, 2048 };
    static const int sh[8] = { 0,  0,  1,   2,   3,   4,     5,    6 };

	int scale = (data >> 5) & 7;
	int step = ((data & 31) << sh[scale]) + dt[scale];
	logerror("prg_lfo_w %02x freq=%5.2f phase=%6.4f\n", adr, step * 44100.0/4194304, (data >> 8)/256.0);
}



// Catch-all

static u16 rr[0x40*0x40];

u16 swp30_device::snd_r(offs_t offset)
{
	if(1) {
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
	if((slot >= 0xa && slot <= 0xd) || (slot >= 0x2c && slot <= 0x2f))
		machine().debug_break();

	logerror("snd_w [%04x %04x] %-5s, %04x\n", offset, offset*2, preg, data);
}



// Synthesis

void swp30_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// Loop first on the samples and not on the channels otherwise
	// effects will be annoying to implement.

	for(int sample = 0; sample < samples; sample++) {
		// Accumulate on 64 bits, shift/clamp at the end
		s64 acc_left = 0, acc_right = 0;

		// Loop on channels
		for(int channel = 0; channel < 64; channel++)
			if(m_active_mask & (u64(1) << channel)) {
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
						m_active_mask &= ~((u64(1) << channel));
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

				// - anything else?

				// Fourth, volume (disabled) and pan, clamp the attenuation at -96dB
				s32 sampl = samp2 * m_linear_attenuation[std::min(0xff, (m_volume[channel] & 0x00) + (m_pan[channel] >> 8))];
				s32 sampr = samp2 * m_linear_attenuation[std::min(0xff, (m_volume[channel] & 0x00) + (m_pan[channel] & 0xff))];

				// Fifth, add to the accumulators
				acc_left  += sampl;
				acc_right += sampr;

				// Missing: reverb, chorus, effects in general
			}

		// Samples are 16 bits, there are up to 64 of them, and the accumulators are fixed-point signed 48.16
		// Global EQ is missing (it's done in the MEG)

		acc_left >>= (16+6);
		if(acc_left < -0x8000)
			acc_left = -0x8000;
		else if(acc_left > 0x7fff)
			acc_left = 0x7fff;
		outputs[0][sample] = acc_left;

		acc_right >>= (16+6);
		if(acc_right < -0x8000)
			acc_right = -0x8000;
		else if(acc_right > 0x7fff)
			acc_right = 0x7fff;
		outputs[1][sample] = acc_right;
	}
}
