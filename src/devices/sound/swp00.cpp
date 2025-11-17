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

// Interpolation speed subblock

// A generic interpolaton subblock is used by envelopes, filter and
// lfo.  Its function is to provide a step to add or substract to a
// counter.  This step depends on a 00-7f speed value and the current
// sample number.  It goes from adding 1 every 2048 samples to adding
// 31 every sample.

// Specifically, for speeds between 00 and 57 seen as level*8 +
// duty_cycle (level=0..a, duty_cycle=0..7), 1 is potentially added
// every 2**(a-level) (0..1024) samples following a 16-steps pattern
// with a duty cycle of 8/16th (duty_cycle=0) to 15/16th
// (duty_cycle=7).  For speeds between 58 and 77 (level=b..e) on every
// sample either 2**(level-a)-1 or 2**(level-9)-1 is added following a
// 8-step pattern with a duty cycle going from 0/8th to 7/8th.  For
// 78+, 1f is added every sample.

u16 swp00_device::interpolation_step(u32 speed, u32 sample_counter)
{
	// Phase is incorrect, and very weird

	if(speed >= 0x78)
        return 0x7f;

    u32 k0 = speed >> 3;
    u32 k1 = speed & 7;

    if(speed >= 0x58) {
        k0 -= 10;
		u32 a = (4 << k0) - 1;
        u32 b = (2 << k0) - 1;
        static const u8 mx[8] = { 0x00, 0x80, 0x88, 0xa8, 0xaa, 0xea, 0xee, 0xfe };
        return ((mx[k1] << (sample_counter & 7)) & 0x80) ? a : b;
	}

    k0 = 10 - k0;

	if(sample_counter & util::make_bitmask<u32>(k0))
		return 0;

	static const u16 mx[8] = { 0xaaaa, 0xeaaa, 0xeaea, 0xeeea, 0xeeee, 0xfeee, 0xfefe, 0xfffe };
    return (mx[k1] << ((sample_counter >> k0) & 0xf)) & 0x8000 ? 1 : 0;
}



// Streaming block
//
//
//   00100 ccccc *   MMpp pppp pppp pppp            Memory access, initial phase
//   00101 ccccc *   ssss ssss ssss ssss            Number of samples before the loop point
//   11000 ccccc 0   ff0S SSmm/ffll llll            Format (3), Scaling, Compressor mode / Format (0-2). Loop size adjust.
//   1100* ccccc *   aaaa aaaa aaaa aaaa aaaa aaaa  Sample address (starts after format)
//   11010 ccccc *   eeee mmmm mmmm mmmm            Pitch
//   11011 ccccc *   ssss ssss ssss ssss            Number of samples in the loop

// Simplified version of the swp30 streaming block.
//
// Uses 2-point linear interpolation (9-bits multiplier, 15-bits
// counter) rather than 4-point cubic.
//
// Pitch is linear rather than exponential.
//
// Address resolution is byte.
//
// The same register is shared for per-loop size adjustment and
// parameters of the compressed format.  As a consequence the
// compressed format does not have the loop size adjustment active.
//
// There is an "initial phase" register available, which sets 14 of
// the 15-bits sub-sample position at start.  There are two bits in
// that register that seem to set some kind of memory access mode,
// with the result that putting anything else than 10 gives somewhat
// unpredictable results (00 weirdly seems to be "add 0x48 to the
// address", the other values return random values as raw sample
// data).


// Dpcm delta expansion table
const std::array<s16, 256> swp00_device::streaming_block::dpcm_expand = []() {
	std::array<s16, 256> deltas;
	static const s16 offset[4] = { 0, 0x20, 0x60, 0xe0 };
	for(u32 i=0; i != 128; i++) {
		u32 e = i >> 5;
		s16 base = ((i & 0x1f) << e) + offset[e];
		deltas[i] = base;
		deltas[i+128] = -base;
	}
	deltas[0x80] = 0x88; // Not actually used by samples, but tested on swp30 hardware
	return deltas;
}();

const std::array<s32, 8> swp00_device::streaming_block::max_value = {
	0x7fff, 0x7ffe, 0x7ffc, 0x7ff8, 0x7ff0, 0x7fe0, 0x7fc0, 0x7f80
};

void swp00_device::streaming_block::clear()
{
	m_phase = 0x8000;
	m_start = 0;
	m_loop = 0;
	m_address = 0;
	m_pitch = 0;
	m_format = 0;
	m_pos = 0;
	m_pos_dec = 0;
	m_dpcm_s0 = m_dpcm_s1 = 0;
	m_dpcm_pos = 0;
	m_dpcm_delta = 0;
	m_first = false;
	m_done = false;
	m_last = 0;
}

void swp00_device::streaming_block::keyon()
{
	m_pos = -m_start;
	m_pos_dec = (m_phase << 1) & 0x7ffe;
	m_dpcm_s0 = m_dpcm_s1 = 0;
	m_dpcm_pos = m_pos+1;
	m_dpcm_delta = 0;
	m_first = true;
	m_done = false;
}

void swp00_device::streaming_block::read_16(memory_access<24, 0, 0, ENDIANNESS_LITTLE>::cache &wave, s16 &val0, s16 &val1)
{
	offs_t adr = m_address + (m_pos << 1);
	val0 = wave.read_word(adr);
	val1 = wave.read_word(adr+2);
}

void swp00_device::streaming_block::read_12(memory_access<24, 0, 0, ENDIANNESS_LITTLE>::cache &wave, s16 &val0, s16 &val1)
{
	offs_t adr = m_address + (m_pos >> 2)*6;

	switch(m_pos & 3) {
	case 0: { // bcCa AB.. -> Cabc ..AB
		u16 w0 = wave.read_word(adr);
		u16 w1 = wave.read_word(adr+2);
		val0 = w0 << 4;
		val1 = ((w0 >> 8) | (w1 << 8)) & 0xfff0;
		break;
	}
	case 1: { // ..c. abBC .A.. -> c... BCab ...A
		u16 w0 = wave.read_word(adr);
		u16 w1 = wave.read_word(adr+2);
		u16 w2 = wave.read_word(adr+4);
		val0 = ((w0 >> 8) | (w1 << 8)) & 0xfff0;
		val1 = ((w1 >> 4) & 0x0ff0) | (w2 << 12);
		break;
	}
	case 2: { // ..bc CaAB -> bc.. ABCa
		u16 w1 = wave.read_word(adr+2);
		u16 w2 = wave.read_word(adr+4);
		val0 = ((w1 >> 4) & 0x0ff0) | (w2 << 12);
		val1 = w2 & 0xfff0;
		break;
	}
	case 3: { // c.ab BC.A -> abc. .ABC
		u16 w2 = wave.read_word(adr+4);
		u16 w3 = wave.read_word(adr+6);
		val0 = w2 & 0xfff0;
		val1 = w3 << 4;
		break;
	}
	}
}

void swp00_device::streaming_block::read_8(memory_access<24, 0, 0, ENDIANNESS_LITTLE>::cache &wave, s16 &val0, s16 &val1)
{
	offs_t adr = m_address + m_pos;
	val0 = wave.read_byte(adr  ) << 8;
	val1 = wave.read_byte(adr+1) << 8;
}

void swp00_device::streaming_block::dpcm_step(u8 input)
{
	u32 mode = m_format & 3;
	u32 scale = (m_format >> 2) & 7;
	s32 limit = max_value[scale];

	m_dpcm_s0 = m_dpcm_s1;

	s32 delta = m_dpcm_delta + dpcm_expand[input];
	s32 sample = m_dpcm_s1 + (delta << scale);

	if(sample < -0x8000) {
		sample = -0x8000;
		delta = 0;
	} else if(sample > limit) {
		sample = limit;
		delta = 0;
	}
	m_dpcm_s1 = sample;

	switch(mode) {
	case 0: delta = delta * 7 / 8; break;
	case 1: delta = delta * 3 / 4; break;
	case 2: delta = delta     / 2; break;
	case 3: delta = 0; break;
	}
	m_dpcm_delta = delta;
}

void swp00_device::streaming_block::read_8c(memory_access<24, 0, 0, ENDIANNESS_LITTLE>::cache &wave, s16 &val0, s16 &val1)
{
	while(m_dpcm_pos != m_pos + 2) {
		dpcm_step(wave.read_byte(m_address + m_dpcm_pos));
		m_dpcm_pos++;
	}
		
	val0 = m_dpcm_s0;
	val1 = m_dpcm_s1;
}

std::pair<s16, bool> swp00_device::streaming_block::step(memory_access<24, 0, 0, ENDIANNESS_LITTLE>::cache &wave, s32 fmod)
{
	if(m_done)
		return std::make_pair(m_last, false);

	s16 val0, val1;

	switch(m_format >> 6) {
	case 0: read_16(wave, val0, val1); break;
	case 1: read_12(wave, val0, val1); break;
	case 2: read_8 (wave, val0, val1); break;
	case 3: read_8c(wave, val0, val1); break;
	}

	u32 step = ((m_pitch & 0xfff) << (8 + (s16(m_pitch) >> 12))) >> 4;
	s32 interp = (m_pos_dec >> 6) & 0x1ff;
	// The multiplier has a precision limited to 15 bits
	s32 result = val0 + ((((val1-val0) * interp) >> 10) << 1);

	m_pos_dec += step + (fmod << 4);
	if(m_pos_dec >= 0x8000) {
		m_first = false;
		m_pos += m_pos_dec >> 15;
		m_pos_dec &= 0x7fff;
		if(m_pos >= m_loop) {
			if(m_loop) {
				m_pos -= m_loop;
				if((m_format & 0xc0) != 0xc0) {
					m_pos_dec += (m_format << 9) & 0x7e00;
					if(m_pos_dec >= 0x8000)
						m_pos ++;
					m_pos_dec &= 0x7fff;
				}
				m_dpcm_pos = 1;
			} else {
				m_done = true;
				m_last = result;
				return std::make_pair(m_last, true);
			}
		}
	}
	return std::make_pair(result, false);				
}

template<int sel> void swp00_device::streaming_block::phase_w(u8 data)
{
	constexpr int shift = 8*sel;
	m_phase = (m_phase & ~(0xff << shift)) | (data << shift);
}

template<int sel> void swp00_device::streaming_block::start_w(u8 data)
{
	constexpr int shift = 8*sel;
	m_start = (m_start & ~(0xff << shift)) | (data << shift);
}

template<int sel> void swp00_device::streaming_block::loop_w(u8 data)
{
	constexpr int shift = 8*sel;
	m_loop = (m_loop & ~(0xff << shift)) | (data << shift);
}

template<int sel> void swp00_device::streaming_block::address_w(u8 data)
{
	constexpr int shift = 8*sel;
	m_address = (m_address & ~(0xff << shift)) | (data << shift);
}

template<int sel> void swp00_device::streaming_block::pitch_w(u8 data)
{
	constexpr int shift = 8*sel;
	m_pitch = (m_pitch & ~(0xff << shift)) | (data << shift);
}

void swp00_device::streaming_block::format_w(u8 data)
{
	m_format = data;
}

template<int sel> u8 swp00_device::streaming_block::phase_r() const
{
	return m_phase >> (8*sel);
}

template<int sel> u8 swp00_device::streaming_block::start_r() const
{
	return m_start >> (8*sel);
}

template<int sel> u8 swp00_device::streaming_block::loop_r() const
{
	return m_loop >> (8*sel);
}

template<int sel> u8 swp00_device::streaming_block::address_r() const
{
	return m_address >> (8*sel);
}

template<int sel> u8 swp00_device::streaming_block::pitch_r() const
{
	return m_pitch >> (8*sel);
}

u8 swp00_device::streaming_block::format_r() const
{
	return m_format;
}

u32 swp00_device::streaming_block::sample_address_get()
{
	u32 result = m_address;
	m_address = (m_address + 1) & 0xffffff;
	return result;
}

std::string swp00_device::streaming_block::describe() const
{
	std::string desc;
	desc = util::string_format("[%04x %08x %08x %08x] ", m_pitch, m_start, m_loop, m_address) + util::string_format("sample %04x-%04x @ %06x ", m_start, m_loop, m_address);
	switch(m_format >> 6) {
	case 0: desc += "16"; break;
	case 1: desc += "12"; break;
	case 2: desc += "8 "; break;
	case 3: desc += util::string_format("c%x", m_format & 3); break;
	}
	if((m_format & 0xc0) == 0xc0)
		desc += util::string_format(" scale %x", (m_format >> 2) & 7);
	if(!m_loop)
		desc += " fwd ";
	else
		desc += " loop";
	if((m_format & 0xc0) != 0xc0 && (m_format & 0x3f))
		desc += util::string_format(" loop-adjust %02x", m_format & 0x3f);
	if(m_pitch & 0x8000) {
		u32 p = 0x10000 - m_pitch;
		desc += util::string_format(" pitch -%x.%03x", p >> 12, p & 0xfff);
	} else if(m_pitch)
		desc += util::string_format(" pitch +%x.%03x", (m_pitch >> 12) & 7, m_pitch & 0xfff);

	return desc;
}


//--------------------------------------------------------------------------------

// Envelope block
//
//
//   10011 ccccc 0   msss ssss                      Attack speed
//   10011 ccccc 1   tttt tttt                      Attack starting point/target
//   10100 ccccc 0   isss ssss                      Update decay, Decay speed
//   10100 ccccc 1   tttt tttt                      Decay target

// Simplified version of the swp30 envelope block.

// There is no decay2 or release.  Release is done by writing a new
// values to decay speed with bit 7 set and setting the target to ff.
// Writing a value to decay speed after keyon is ignored when bit 7 is
// not set.  Decay target otoh is always taken into account live.

// The attenuation is on 12 bits (4 exponent, 8 mantissa) and targets
// are left-shifted and zero-padded by 4 bits.

// Attack can happen in three modes:

//   - normal attack when m=1, where attenuation goes from starting point
//     to 0 at variable speed so that it's linear in amplitude space

//   - timed attack when m=0 and starting point = 0, where attenuation
//     stays at 0 for a time depending on the speed

//   - alternative decay when m=0 and target != 0, where
//     attenuation goes from 0 to target at constant speed, like
//     the decay

// Decay can happen in two modes:

//   - normal decay where the attenuation goes from current to target
//     at constant speed

//   - alternative decay, when bit 7 is set and speed >= 0x60, where
//     the speed is variable to be linear in amplitude space

// The value to add (or substract) on every cycle is given by the
// common interpolation subblock.  When in constant speed mode (attack
// mode 3, decay mode 1), the speed value is directly used.  When in
// variable speed mode, the actual speed is the speed (for attack) or
// speed - 0x10 (for decay) with 4*(attenuation >> 7) added.

// Timed attack mode counts at the given speed until 0x800 is reached.

void swp00_device::envelope_block::clear()
{
	m_attack_speed = 0;
	m_attack_level = 0;
	m_decay_speed = 0;
	m_decay_level = 0;
	m_envelope_level = 0xfff;
	m_envelope_mode = DECAY_DONE;
}

void swp00_device::envelope_block::keyon()
{
	if(m_decay_speed & 0x80) {
		// Immediate release
		m_envelope_level = 0;
		m_envelope_mode = DECAY;

	} else {
		if(m_attack_speed & 0x80)
			// normal mode
			m_envelope_level = m_attack_level << 4;
		else
			// decay-like or timed mode
			m_envelope_level = 0;
		m_envelope_mode = ATTACK;
	}
}

u8 swp00_device::envelope_block::status() const
{
	return m_envelope_mode == DECAY_DONE ? 0xff : (m_envelope_mode << 6) | (m_envelope_level >> 6);
}

bool swp00_device::envelope_block::active() const
{
	return m_envelope_mode != DECAY_DONE;
}

u16 swp00_device::envelope_block::step(u32 sample_counter)
{
	u16 result = m_envelope_level;
	switch(m_envelope_mode) {
	case ATTACK: {
		if(m_attack_speed & 0x80) {
			// normal mode
			s32 level = m_envelope_level - swp00_device::interpolation_step((m_attack_speed & 0x7f) + ((m_envelope_level >> 7) << 2), sample_counter);
			if(level <= 0) {
				level = 0;
				m_envelope_mode = DECAY;
			}
			m_envelope_level = level;
		} else {
			s32 level = m_envelope_level + swp00_device::interpolation_step(m_attack_speed, sample_counter);
			if(m_attack_level) {
				// decay-like mode
				s32 limit = m_attack_level << 4;
				if(level >= limit) {
					m_envelope_mode = DECAY;
					if(level >= 0xfff)
						level = 0xfff;
				}
			} else {
				// timed mode
				if(level >= 0x800) {
					level = 0;
					m_envelope_mode = DECAY;
				}
				result = 0;
			}
		}
		break;
	}

	case DECAY: {
		u32 key = m_decay_speed >= 0xe0 ? m_decay_speed - 0x90 + ((m_envelope_level >> 7) << 2) : m_decay_speed & 0x7f;
		s32 limit = m_decay_level << 4;
		s32 level = m_envelope_level;
		if(level < limit) {
			level += swp00_device::interpolation_step(key, sample_counter);
			if(level > limit) {
				m_envelope_mode = DECAY_DONE;
				if(level > 0xfff)
					level = 0xfff;
			}
		} else if(level > limit) {
			level -= swp00_device::interpolation_step(key, sample_counter);
			if(level < limit) {
				m_envelope_mode = DECAY_DONE;
				if(level < 0)
					level = 0;
			}
		} else
			m_envelope_mode = DECAY_DONE;
		m_envelope_level = level;
		break;
	}

	case DECAY_DONE:
		break;
	}
	return result;
}

void swp00_device::envelope_block::trigger_release()
{
	m_envelope_mode = DECAY;
}

u8 swp00_device::envelope_block::attack_speed_r() const
{
	return m_attack_speed;
}

void swp00_device::envelope_block::attack_speed_w(u8 data)
{
	if(m_envelope_mode == DECAY_DONE)
		m_attack_speed = data;
}

u8 swp00_device::envelope_block::attack_level_r() const
{
	return m_attack_level;
}

void swp00_device::envelope_block::attack_level_w(u8 data)
{
	m_attack_level = data;
}

u8 swp00_device::envelope_block::decay_speed_r() const
{
	return m_decay_speed;
}

void swp00_device::envelope_block::decay_speed_w(u8 data)
{
	if(data & 0x80) {
		m_decay_speed = data;
		m_envelope_mode = DECAY;

	} else if(m_envelope_mode == DECAY_DONE) 
		m_decay_speed = data;
}

u8 swp00_device::envelope_block::decay_level_r() const
{
	return m_decay_level;
}

void swp00_device::envelope_block::decay_level_w(u8 data)
{
	m_decay_level = data;
}

//--------------------------------------------------------------------------------

// Filter block

//   10000 ccccc *   qqqq qkkk kkkk kkkk            LPF Q and K coefficients
//   10001 ccccc 0   wsss ssss                      Sweep, speed

// This block takes samples from the streaming block and applies a
// Chamberlin configuration low pass filter to them.

//    encoded q is fp 2.3 (with a 4 offset), k is fp 4.8 but the top
//    bit is an added 1 except when it's all zero.
//
//    k = ((0x101 + k.m) << k.e) / (2**24)
//    q = ((0x10 - (q+4).m) << (4 - (q+4).e)) / (2**7)
//
//    B(0) = L(0) = 0
//    H' = x0 - L - q*B
//    B' = B + k * H'
//    L' = L + k * B'
//    y0 = L'

// The chip interpolates between consecutive values of k.  When
// updating the coefficients registers Q is updated immediatly and K
// uses the interpolation block with the configured speed to reach the
// new value.

// When w=1 at keyon, the behaviour is different.  The filter sweeps
// from 800 to fff at the configured speed then jumps to the
// programmed level.  All subsequent changes are ignored until the
// next keyon.  The value of w is only checked on keyon.

void swp00_device::filter_block::clear()
{
	m_info = 0x27ff; // Actual reset value unknown
	m_speed = 0x00;
	m_sweep = SWEEP_NONE;

	m_k_target = 0x7ff;
	m_k = 0xfff;
	m_q = 0x80;

	m_b = 0;
	m_l = 0;
}

void swp00_device::filter_block::keyon()
{
	m_b = 0;
	m_l = 0;

	if(m_speed & 0x80) {
		m_sweep = SWEEP_UP;
		m_k = 0x800;
	} else
		m_k = m_k_target;
}

s32 swp00_device::filter_block::step(s16 input, s32 lmod, u32 sample_counter)
{
	s32 km = std::max(m_k - lmod, 0);
	s64 k = (0x101 + (km & 0xff)) << (km >> 8);
	s32 h = (input << 6) - m_l - ((s64(m_q) * m_b) >> 7);
	m_b = m_b + ((k *   h) >> 24);
	m_l = m_l + ((k * m_b) >> 24);

	switch(m_sweep) {
	case SWEEP_NONE:
		if(m_k < m_k_target) {
			m_k += swp00_device::interpolation_step(m_speed & 0x7f, sample_counter);
			if(m_k > m_k_target)
				m_k = m_k_target;
		
		} else if(m_k > m_k_target) {
			m_k -= swp00_device::interpolation_step(m_speed & 0x7f, sample_counter);
			if(m_k < m_k_target)
				m_k = m_k_target;
		}
		break;
	case SWEEP_UP:
		m_k += swp00_device::interpolation_step(m_speed & 0x7f, sample_counter);
		if(m_k >= 0xfff) {
			m_k = m_k_target;
			m_sweep = SWEEP_DONE;
		}
		break;
	}

	return m_l;
}

u8 swp00_device::filter_block::status() const
{
	return (m_sweep == SWEEP_DONE || (m_sweep == SWEEP_NONE && m_k == m_k_target) ? 0xc0 : 0x00) | ((m_k >> 6) ^ 0x3f);
}

template<int sel> void swp00_device::filter_block::info_w(u8 data)
{
	if(sel) {
		m_info = (m_info & 0xff) | (data << 8);
		s32 q = (m_info >> 11) + 4;
		m_q = (0x10 - (q & 7)) << (4 - (q >> 3));
	} else
		m_info = (m_info & 0xff00) | data;

	m_k_target = m_info & 0x7ff;
	if(m_k_target)
		m_k_target |= 0x800;
}

template<int sel> u8 swp00_device::filter_block::info_r()
{
	return m_info >> (8*sel);
}

void swp00_device::filter_block::speed_w(u8 data)
{
	m_speed = data;
}

u8 swp00_device::filter_block::speed_r()
{
	return m_speed;
}

//--------------------------------------------------------------------------------

// LFO block

//   10001 ccccc 1   llla aaaa                      LPF level, amplitude level
//   10010 ccccc 0   dhss ssss                      Disable, shape, speed
//   10010 ccccc 1   pppp pppp                      Pitch level

// The LFO block is based on a 22-bits counter which is incremented on
// each sample.  It is then optionally shaped as a triangle, then
// multiplied by the different levels to compute the impact.  The LFO
// output can be disabled, zeroing the output.

// The increment is encoded in a variant of fp 3.3, when for e<7 the
// value is (8+m) << e, and for e=7 (8+2*m) << e, giving a cycle time
// of 1489 to 523288 cycles, or 0.08 to 30Hz.

void swp00_device::lfo_block::clear()
{
	m_counter = 0;
	m_lamod = 0;
	m_fmod = 0;
	m_speed = 0x80;
}

void swp00_device::lfo_block::keyon()
{
}

std::tuple<u32, s32, s32> swp00_device::lfo_block::step()
{
	u32 e = (m_speed >> 3) & 7;
	u32 step;
	if(e < 7)
		step = (8 | (m_speed & 7)) << e;
	else
		step = (8 + 2*(m_speed & 7)) << 7;

	m_counter = (m_counter + step) & 0x3fffff;

	u32 amod = 0;
	s32 fmod = 0;
	s32 lmod = 0;

	if(!(m_speed & 0x80)) {
		s32 shaped;
		if(m_speed & 0x40)
			shaped = ((m_counter << 1) & 0x3fffff) ^ (m_counter & 0x200000 ? 0x3fffff : 0);
		else
			shaped = m_counter;

		amod = (shaped * (m_lamod & 0x1f)) >> 16;
		fmod = ((shaped - 0x200000) * m_fmod) >> 21;
		lmod = (shaped * (m_lamod & 0xe0)) >> 20;
	}

	return std::tie(amod, fmod, lmod);
}

void swp00_device::lfo_block::lamod_w(u8 data)
{
	m_lamod = data;
}

u8 swp00_device::lfo_block::lamod_r()
{
	return m_lamod;
}

void swp00_device::lfo_block::speed_w(u8 data)
{
	m_speed = data;
}

u8 swp00_device::lfo_block::speed_r()
{
	return m_speed;
}

void swp00_device::lfo_block::fmod_w(u8 data)
{
	m_fmod = data;
}

u8 swp00_device::lfo_block::fmod_r()
{
	return m_fmod;
}


//--------------------------------------------------------------------------------

// Mixer block

//   10101 ccccc 0   RRRR RRRR                      Reverb level
//   10101 ccccc 1   DDDD DDDD                      Dry level
//   10110 ccccc 0   CCCC CCCC                      Chorus level
//   10110 ccccc 1   VVVV VVVV                      Variation level
//   10111 ccccc 0   gggg gggg                      Global level
//   10111 ccccc 1   llll rrrr                      Pan levels

// The mixer block function is to to get the samples from the filter
// and attenuate them as wanted given the envelope, tremolo, levels
// and pans to create the inputs to the effects block.  Those inputs
// are three stereo streams (dry, chorus and variation) and one mono
// stream (reverb).

// An attenuation is a 12-bits floating-point 4.8 value which combines
// additively.  The 8-bits levels are left-shifted by 4, the 4-bits
// (pan) by 6.  A pan of 0xf has a special mapped value of 0xfff
// (e.g. off).

// Envelope and tremolo are given externally, this block manages all
// the other volumes.  The four levels dry, reverb, chorus and
// variation are applied immediatly when changed.  The global and pan
// left/right levels are interpolated through time.  When a new level
// is written it becomes a new 12-bits target value through shifting,
// and that value is reached by changing the current value by 1 on
// each sample.

// The current value of the interpolators and whether they reached
// their target can be read out.

void swp00_device::mixer_block::clear()
{
	m_cglo = m_cpanl = m_cpanr = 0xfff;
	m_tglo = m_tpanl = m_tpanr = 0xfff;
	m_glo = m_pan = m_dry = m_rev = m_cho = m_var = 0xff;
}

void swp00_device::mixer_block::keyon()
{
	m_cglo = m_tglo;
	m_cpanl = m_tpanl;
	m_cpanr = m_tpanr;
}

u8 swp00_device::mixer_block::status_glo() const
{
	return (m_cglo == m_tglo ? 0xc0 : 0x00) | (m_cglo >> 6);
}

u8 swp00_device::mixer_block::status_panl() const
{
	return (m_cpanl == m_tpanl ? 0xc0 : 0x00) | (m_cpanl >> 6);
}

u8 swp00_device::mixer_block::status_panr() const
{
	return (m_cpanr == m_tpanr ? 0xc0 : 0x00) | (m_cpanr >> 6);
}

void swp00_device::mixer_block::step(s32 sample, u16 envelope, u16 tremolo,
									 s32 &dry_l, s32 &dry_r, s32 &rev, s32 &cho_l, s32 &cho_r, s32 &var_l, s32 &var_r)
{
	u16 base = envelope + tremolo + m_cglo;
	dry_l += volume_apply(base + (m_dry << 4) + m_cpanl, sample);
	dry_r += volume_apply(base + (m_dry << 4) + m_cpanr, sample);
	rev   += volume_apply(base + (m_rev << 4),           sample);
	cho_l += volume_apply(base + (m_cho << 4) + m_cpanl, sample);
	cho_r += volume_apply(base + (m_cho << 4) + m_cpanr, sample);
	var_l += volume_apply(base + (m_var << 4) + m_cpanl, sample);
	var_r += volume_apply(base + (m_var << 4) + m_cpanr, sample);

	if(m_cglo < m_tglo)
		m_cglo++;
	else if(m_cglo > m_tglo)
		m_cglo--;
	if(m_cpanl < m_tpanl)
		m_cpanl++;
	else if(m_cpanl > m_tpanl)
		m_cpanl--;
	if(m_cpanr < m_tpanr)
		m_cpanr++;
	else if(m_cpanr > m_tpanr)
		m_cpanr--;
}

s32 swp00_device::mixer_block::volume_apply(s32 level, s32 sample)
{
	// Level is 4.8 floating point positive, and represents an attenuation
	// Sample is 16.6 signed and the result is in the same format

	// Passed-in value may have overflowed
	if(level >= 0xfff)
		return 0;

	s32 e = level >> 8;
	s32 m = level & 0xff; 
	s64 mul = (0x1000000 - (m << 15)) >> e;
	return (sample * mul) >> 24;
}


void swp00_device::mixer_block::glo_w(u8 data)
{
	m_glo = data;
	m_tglo = data << 4;
}

void swp00_device::mixer_block::pan_w(u8 data)
{
	m_pan = data;
	m_tpanl = (data << 2) & 0x3c0;
	m_tpanr = (data << 6) & 0x3c0;
	if(m_tpanl == 0x3c0)
		m_tpanl = 0xfff;
	if(m_tpanr == 0x3c0)
		m_tpanr = 0xfff;
}

void swp00_device::mixer_block::dry_w(u8 data)
{
	m_dry = data;
}

void swp00_device::mixer_block::rev_w(u8 data)
{
	m_rev = data;
}

void swp00_device::mixer_block::cho_w(u8 data)
{
	m_cho = data;
}

void swp00_device::mixer_block::var_w(u8 data)
{
	m_var = data;
}

u8 swp00_device::mixer_block::glo_r()
{
	return m_glo;
}

u8 swp00_device::mixer_block::pan_r()
{
	return m_pan;
}

u8 swp00_device::mixer_block::dry_r()
{
	return m_dry;
}

u8 swp00_device::mixer_block::rev_r()
{
	return m_rev;
}

u8 swp00_device::mixer_block::cho_r()
{
	return m_cho;
}

u8 swp00_device::mixer_block::var_r()
{
	return m_var;
}


//--------------------------------------------------------------------------------

DEFINE_DEVICE_TYPE(SWP00, swp00_device, "swp00", "Yamaha SWP00 (TC170C120SF / XQ036A00) sound chip")

swp00_device::swp00_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SWP00, tag, owner, clock),
	  device_sound_interface(mconfig, *this),
	  device_rom_interface(mconfig, *this),
	  m_require_sync(false)
{
}

void swp00_device::require_sync()
{
	m_require_sync = true;
}

bool swp00_device::istep(s32 &value, s32 limit, s32 step)
{
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

void swp00_device::device_start()
{
	m_stream = stream_alloc(0, 2, 44100, m_require_sync ? STREAM_SYNCHRONOUS : STREAM_DEFAULT_FLAGS);

	save_item(STRUCT_MEMBER(m_streaming, m_phase));
	save_item(STRUCT_MEMBER(m_streaming, m_start));
	save_item(STRUCT_MEMBER(m_streaming, m_loop));
	save_item(STRUCT_MEMBER(m_streaming, m_address));
	save_item(STRUCT_MEMBER(m_streaming, m_pitch));
	save_item(STRUCT_MEMBER(m_streaming, m_format));
	save_item(STRUCT_MEMBER(m_streaming, m_pos));
	save_item(STRUCT_MEMBER(m_streaming, m_pos_dec));
	save_item(STRUCT_MEMBER(m_streaming, m_dpcm_s0));
	save_item(STRUCT_MEMBER(m_streaming, m_dpcm_s1));
	save_item(STRUCT_MEMBER(m_streaming, m_dpcm_pos));
	save_item(STRUCT_MEMBER(m_streaming, m_dpcm_delta));
	save_item(STRUCT_MEMBER(m_streaming, m_first));
	save_item(STRUCT_MEMBER(m_streaming, m_done));
	save_item(STRUCT_MEMBER(m_streaming, m_last));

	save_item(STRUCT_MEMBER(m_envelope, m_attack_speed));
	save_item(STRUCT_MEMBER(m_envelope, m_attack_level));
	save_item(STRUCT_MEMBER(m_envelope, m_decay_speed));
	save_item(STRUCT_MEMBER(m_envelope, m_decay_level));
	save_item(STRUCT_MEMBER(m_envelope, m_envelope_level));
	save_item(STRUCT_MEMBER(m_envelope, m_envelope_mode));

	save_item(STRUCT_MEMBER(m_filter, m_q));
	save_item(STRUCT_MEMBER(m_filter, m_b));
	save_item(STRUCT_MEMBER(m_filter, m_l));
	save_item(STRUCT_MEMBER(m_filter, m_k));
	save_item(STRUCT_MEMBER(m_filter, m_k_target));
	save_item(STRUCT_MEMBER(m_filter, m_info));
	save_item(STRUCT_MEMBER(m_filter, m_speed));

	save_item(STRUCT_MEMBER(m_mixer, m_cglo));
	save_item(STRUCT_MEMBER(m_mixer, m_cpanl));
	save_item(STRUCT_MEMBER(m_mixer, m_cpanr));
	save_item(STRUCT_MEMBER(m_mixer, m_tglo));
	save_item(STRUCT_MEMBER(m_mixer, m_tpanl));
	save_item(STRUCT_MEMBER(m_mixer, m_tpanr));
	save_item(STRUCT_MEMBER(m_mixer, m_glo));
	save_item(STRUCT_MEMBER(m_mixer, m_pan));
	save_item(STRUCT_MEMBER(m_mixer, m_dry));
	save_item(STRUCT_MEMBER(m_mixer, m_rev));
	save_item(STRUCT_MEMBER(m_mixer, m_cho));
	save_item(STRUCT_MEMBER(m_mixer, m_var));

	save_item(NAME(m_sample_counter));
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
}

void swp00_device::device_reset()
{
	for(auto &s : m_streaming)
		s.clear();
	for(auto &e : m_envelope)
		e.clear();
	for(auto &f : m_filter)
		f.clear();
	for(auto &l : m_lfo)
		l.clear();
	for(auto &m : m_mixer)
		m.clear();

	m_sample_counter = 0;
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
}

void swp00_device::rom_bank_pre_change()
{
	m_stream->update();
}

void swp00_device::map(address_map &map)
{
	map(0x000, 0x7ff).rw(FUNC(swp00_device::snd_r), FUNC(swp00_device::snd_w));

	// 00-01: control

	rchan(map, 0x08).rw(FUNC(swp00_device::phase_r<1>), FUNC(swp00_device::phase_w<1>));
	rchan(map, 0x09).rw(FUNC(swp00_device::phase_r<0>), FUNC(swp00_device::phase_w<0>));
	rchan(map, 0x0a).rw(FUNC(swp00_device::start_r<1>), FUNC(swp00_device::start_w<1>));
	rchan(map, 0x0b).rw(FUNC(swp00_device::start_r<0>), FUNC(swp00_device::start_w<0>));

	// 0c-0f: meg offsets
	// 10-1b: meg values

	rchan(map, 0x20).rw(FUNC(swp00_device::lpf_info_r<1>), FUNC(swp00_device::lpf_info_w<1>));
	rchan(map, 0x21).rw(FUNC(swp00_device::lpf_info_r<0>), FUNC(swp00_device::lpf_info_w<0>));
	rchan(map, 0x22).rw(FUNC(swp00_device::lpf_speed_r), FUNC(swp00_device::lpf_speed_w));
	rchan(map, 0x23).rw(FUNC(swp00_device::lfo_lamod_r), FUNC(swp00_device::lfo_lamod_w));
	rchan(map, 0x24).rw(FUNC(swp00_device::lfo_speed_r), FUNC(swp00_device::lfo_speed_w));
	rchan(map, 0x25).rw(FUNC(swp00_device::lfo_fmod_r), FUNC(swp00_device::lfo_fmod_w));
	rchan(map, 0x26).rw(FUNC(swp00_device::attack_speed_r), FUNC(swp00_device::attack_speed_w));
	rchan(map, 0x27).rw(FUNC(swp00_device::attack_level_r), FUNC(swp00_device::attack_level_w));
	rchan(map, 0x28).rw(FUNC(swp00_device::decay_speed_r), FUNC(swp00_device::decay_speed_w));
	rchan(map, 0x29).rw(FUNC(swp00_device::decay_level_r), FUNC(swp00_device::decay_level_w));
	rchan(map, 0x2a).rw(FUNC(swp00_device::rev_r), FUNC(swp00_device::rev_w));
	rchan(map, 0x2b).rw(FUNC(swp00_device::dry_r), FUNC(swp00_device::dry_w));
	rchan(map, 0x2c).rw(FUNC(swp00_device::cho_r), FUNC(swp00_device::cho_w));
	rchan(map, 0x2d).rw(FUNC(swp00_device::var_r), FUNC(swp00_device::var_w));
	rchan(map, 0x2e).rw(FUNC(swp00_device::glo_r), FUNC(swp00_device::glo_w));
	rchan(map, 0x2f).rw(FUNC(swp00_device::pan_r), FUNC(swp00_device::pan_w));
	rchan(map, 0x30).rw(FUNC(swp00_device::format_r), FUNC(swp00_device::format_w));
	rchan(map, 0x31).rw(FUNC(swp00_device::address_r<2>), FUNC(swp00_device::address_w<2>));
	rchan(map, 0x32).rw(FUNC(swp00_device::address_r<1>), FUNC(swp00_device::address_w<1>));
	rchan(map, 0x33).rw(FUNC(swp00_device::address_r<0>), FUNC(swp00_device::address_w<0>));
	rchan(map, 0x34).rw(FUNC(swp00_device::pitch_r<1>), FUNC(swp00_device::pitch_w<1>));
	rchan(map, 0x35).rw(FUNC(swp00_device::pitch_r<0>), FUNC(swp00_device::pitch_w<0>));
	rchan(map, 0x36).rw(FUNC(swp00_device::loop_r<1>), FUNC(swp00_device::loop_w<1>));
	rchan(map, 0x37).rw(FUNC(swp00_device::loop_r<0>), FUNC(swp00_device::loop_w<0>));

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
template<int sel> void swp00_device::lpf_info_w(offs_t offset, u8 data)
{
	m_stream->update();
	m_filter[offset >> 1].info_w<sel>(data);
}

template<int sel> u8 swp00_device::lpf_info_r(offs_t offset)
{
	return m_filter[offset >> 1].info_r<sel>();
}

void swp00_device::lpf_speed_w(offs_t offset, u8 data)
{
	m_stream->update();
	m_filter[offset >> 1].speed_w(data);
}

u8 swp00_device::lpf_speed_r(offs_t offset)
{
	return m_filter[offset >> 1].speed_r();
}

void swp00_device::lfo_lamod_w(offs_t offset, u8 data)
{
	m_stream->update();
	m_lfo[offset >> 1].lamod_w(data);
}

u8 swp00_device::lfo_lamod_r(offs_t offset)
{
	return m_lfo[offset >> 1].lamod_r();
}

void swp00_device::lfo_speed_w(offs_t offset, u8 data)
{
	m_stream->update();
	m_lfo[offset >> 1].speed_w(data);
}

u8 swp00_device::lfo_speed_r(offs_t offset)
{
	return m_lfo[offset >> 1].speed_r();
}

void swp00_device::lfo_fmod_w(offs_t offset, u8 data)
{
	m_stream->update();
	m_lfo[offset >> 1].fmod_w(data);
}

u8 swp00_device::lfo_fmod_r(offs_t offset)
{
	return m_lfo[offset >> 1].fmod_r();
}

void swp00_device::glo_w(offs_t offset, u8 data)
{
	m_stream->update();
	m_mixer[offset >> 1].glo_w(data);
}

u8 swp00_device::glo_r(offs_t offset)
{
	return m_mixer[offset >> 1].glo_r();
}

void swp00_device::pan_w(offs_t offset, u8 data)
{
	m_stream->update();
	m_mixer[offset >> 1].pan_w(data);
}

u8 swp00_device::pan_r(offs_t offset)
{
	return m_mixer[offset >> 1].pan_r();
}

void swp00_device::dry_w(offs_t offset, u8 data)
{
	m_stream->update();
	m_mixer[offset >> 1].dry_w(data);
}

u8 swp00_device::dry_r(offs_t offset)
{
	return m_mixer[offset >> 1].dry_r();
}

void swp00_device::rev_w(offs_t offset, u8 data)
{
	m_stream->update();
	m_mixer[offset >> 1].rev_w(data);
}

u8 swp00_device::rev_r(offs_t offset)
{
	return m_mixer[offset >> 1].rev_r();
}

void swp00_device::cho_w(offs_t offset, u8 data)
{
	m_stream->update();
	m_mixer[offset >> 1].cho_w(data);
}

u8 swp00_device::cho_r(offs_t offset)
{
	return m_mixer[offset >> 1].cho_r();
}
void swp00_device::var_w(offs_t offset, u8 data)
{
	m_stream->update();
	m_mixer[offset >> 1].var_w(data);
}

u8 swp00_device::var_r(offs_t offset)
{
	return m_mixer[offset >> 1].var_r();
}

void swp00_device::attack_speed_w(offs_t offset, u8 data)
{
	m_stream->update();
	m_envelope[offset >> 1].attack_speed_w(data);
}

u8 swp00_device::attack_speed_r(offs_t offset)
{
	return m_envelope[offset >> 1].attack_speed_r();
}

void swp00_device::attack_level_w(offs_t offset, u8 data)
{
	m_stream->update();
	m_envelope[offset >> 1].attack_level_w(data);
}

u8 swp00_device::attack_level_r(offs_t offset)
{
	return m_envelope[offset >> 1].attack_level_r();
}

void swp00_device::decay_speed_w(offs_t offset, u8 data)
{
	m_stream->update();
	m_envelope[offset >> 1].decay_speed_w(data);
}

u8 swp00_device::decay_speed_r(offs_t offset)
{
	return m_envelope[offset >> 1].decay_speed_r();
}

void swp00_device::decay_level_w(offs_t offset, u8 data)
{
	m_stream->update();
	m_envelope[offset >> 1].decay_level_w(data);
}

u8 swp00_device::decay_level_r(offs_t offset)
{
	return m_envelope[offset >> 1].decay_level_r();
}

template<int sel> void swp00_device::pitch_w(offs_t offset, u8 data)
{
	m_stream->update();
	m_streaming[offset >> 1].pitch_w<sel>(data);
}

template<int sel> u8 swp00_device::pitch_r(offs_t offset)
{
	return m_streaming[offset >> 1].pitch_r<sel>();
}

template<int sel> void swp00_device::start_w(offs_t offset, u8 data)
{
	m_stream->update();
	m_streaming[offset >> 1].start_w<sel>(data);
}

template<int sel> u8 swp00_device::start_r(offs_t offset)
{
	return m_streaming[offset >> 1].start_r<sel>();
}

template<int sel> void swp00_device::phase_w(offs_t offset, u8 data)
{
	m_stream->update();
	m_streaming[offset >> 1].phase_w<sel>(data);
}

template<int sel> u8 swp00_device::phase_r(offs_t offset)
{
	return m_streaming[offset >> 1].phase_r<sel>();
}

template<int sel> void swp00_device::loop_w(offs_t offset, u8 data)
{
	m_stream->update();
	m_streaming[offset >> 1].loop_w<sel>(data);
}

template<int sel> u8 swp00_device::loop_r(offs_t offset)
{
	return m_streaming[offset >> 1].loop_r<sel>();
}

void swp00_device::format_w(offs_t offset, u8 data)
{
	m_stream->update();
	m_streaming[offset >> 1].format_w(data);
}

u8 swp00_device::format_r(offs_t offset)
{
	return m_streaming[offset >> 1].format_r();
}

template<int sel> void swp00_device::address_w(offs_t offset, u8 data)
{
	m_stream->update();
	m_streaming[offset >> 1].address_w<sel>(data);
}

template<int sel> u8 swp00_device::address_r(offs_t offset)
{
	return m_streaming[offset >> 1].address_r<sel>();
}

void swp00_device::keyon(int chan)
{
	m_stream->update();

	logerror("keyon %02x: %s\n", chan, m_streaming[chan].describe());
	m_streaming[chan].keyon();
	m_envelope [chan].keyon();
	m_filter   [chan].keyon();
	m_lfo      [chan].keyon();
	m_mixer    [chan].keyon();
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
	return read_byte(m_streaming[0x1f].sample_address_get());
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
	switch(m_state_adr >> 5) {
	case 0:  // lpf k
		return m_filter[chan].status();

	case 1:  // sample counter
		return 0xc0 | (m_sample_counter >> 13);

	case 2:  // Envelope state
		return m_envelope[chan].status();

	case 3:  // global level
		return m_mixer[chan].status_glo();

	case 4:  // panning l
		return m_mixer[chan].status_panl();
									   
	case 5:  // panning r
		return m_mixer[chan].status_panr();
	}

	// 6 and 7 are semi-random values between c0 and ff with a lot of c0 and ff

	logerror("state %d unsupported\n", m_state_adr >> 5);
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
			auto [amod, fmod, lmod] = m_lfo[chan].step();

			if(!m_envelope[chan].active())
				continue;

			auto [sample1, trigger_release] = m_streaming[chan].step(m_rom_cache, fmod);
			if(trigger_release)
				m_envelope[chan].trigger_release();

			s32 sample2 = m_filter[chan].step(sample1, lmod, m_sample_counter);
			u32 envelope_level = m_envelope[chan].step(m_sample_counter);
			m_mixer[chan].step(sample2, envelope_level, amod, dry_l, dry_r, rev, cho_l, cho_r, var_l, var_r);
		}

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

		// 18-bits ADCs
		stream.put_int_clamp(0, i, dry_l >> 6, 0x20000);
		stream.put_int_clamp(1, i, dry_r >> 6, 0x20000);

		m_buffer_offset --;
		m_sample_counter ++;
	}
}
