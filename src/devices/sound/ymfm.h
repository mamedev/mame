// license:BSD-3-Clause
// copyright-holders:Aaron Giles

#ifndef MAME_SOUND_YMFM_H
#define MAME_SOUND_YMFM_H

#pragma once


//*********************************************************
//  MACROS
//*********************************************************

// special naming helper to keep our namespace isolated from other
// same-named objects in the device's namespace (mostly necessary
// for chips which derive from AY-8910 classes and may have clashing
// names)
#define YMFM_NAME(x) x, "ymfm." #x


//*********************************************************
//  GLOBAL ENUMERATORS
//*********************************************************

enum ymfm_envelope_state : u32
{
	YMFM_ENV_DEPRESS = 0,
	YMFM_ENV_ATTACK = 1,
	YMFM_ENV_DECAY = 2,
	YMFM_ENV_SUSTAIN = 3,
	YMFM_ENV_RELEASE = 4,
	YMFM_ENV_STATES = 5
};


//*********************************************************
//  GLOBAL HELPERS
//*********************************************************

// Many of the Yamaha FM chips emit a floating-point value, which is sent to
// a DAC for processing. The exact format of this floating-point value is
// documented below. This description only makes sense if the "internal"
// format treats sign as 1=positive and 0=negative, so the helpers below
// presume that.
//
// Internal OPx data      16-bit signed data     Exp Sign Mantissa
// =================      =================      === ==== ========
// 1 1xxxxxxxx------  ->  0 1xxxxxxxx------  ->  111   1  1xxxxxxx
// 1 01xxxxxxxx-----  ->  0 01xxxxxxxx-----  ->  110   1  1xxxxxxx
// 1 001xxxxxxxx----  ->  0 001xxxxxxxx----  ->  101   1  1xxxxxxx
// 1 0001xxxxxxxx---  ->  0 0001xxxxxxxx---  ->  100   1  1xxxxxxx
// 1 00001xxxxxxxx--  ->  0 00001xxxxxxxx--  ->  011   1  1xxxxxxx
// 1 000001xxxxxxxx-  ->  0 000001xxxxxxxx-  ->  010   1  1xxxxxxx
// 1 000000xxxxxxxxx  ->  0 000000xxxxxxxxx  ->  001   1  xxxxxxxx
// 0 111111xxxxxxxxx  ->  1 111111xxxxxxxxx  ->  001   0  xxxxxxxx
// 0 111110xxxxxxxx-  ->  1 111110xxxxxxxx-  ->  010   0  0xxxxxxx
// 0 11110xxxxxxxx--  ->  1 11110xxxxxxxx--  ->  011   0  0xxxxxxx
// 0 1110xxxxxxxx---  ->  1 1110xxxxxxxx---  ->  100   0  0xxxxxxx
// 0 110xxxxxxxx----  ->  1 110xxxxxxxx----  ->  101   0  0xxxxxxx
// 0 10xxxxxxxx-----  ->  1 10xxxxxxxx-----  ->  110   0  0xxxxxxx
// 0 0xxxxxxxx------  ->  1 0xxxxxxxx------  ->  111   0  0xxxxxxx

//-------------------------------------------------
//  ymfm_encode_fp - given a 32-bit signed input
//  value, convert it to a signed 3.10 floating-
//  point value
//-------------------------------------------------

inline s16 ymfm_encode_fp(s32 value)
{
	// handle overflows first
	if (value < -32768)
		return (7 << 10) | 0x000;
	if (value > 32767)
		return (7 << 10) | 0x3ff;

	// we need to count the number of leading sign bits after the sign
	// we can use count_leading_zeros if we invert negative values
	s32 scanvalue = value ^ (s32(value) >> 31);

	// exponent is related to the number of leading bits starting from bit 14
	int exponent = 7 - count_leading_zeros(scanvalue << 17);

	// smallest exponent value allowed is 1
	exponent = std::max(exponent, 1);

	// mantissa
	s32 mantissa = value >> (exponent - 1);

	// assemble into final form, inverting the sign
	return ((exponent << 10) | (mantissa & 0x3ff)) ^ 0x200;
}


//-------------------------------------------------
//  ymfm_decode_fp - given a 3.10 floating-point
//  value, convert it to a signed 16-bit value
//-------------------------------------------------

inline s16 ymfm_decode_fp(s16 value)
{
	// invert the sign and the exponent
	value ^= 0x1e00;

	// shift mantissa up to 16 bits then apply inverted exponent
	return s16(value << 6) >> BIT(value, 10, 3);
}


//-------------------------------------------------
//  ymfm_roundtrip_fp - compute the result of a
//  round trip through the encode/decode process
//  above
//-------------------------------------------------

inline s16 ymfm_roundtrip_fp(s32 value)
{
	// handle overflows first
	if (value < -32768)
		return -32768;
	if (value > 32767)
		return 32767;

	// we need to count the number of leading sign bits after the sign
	// we can use count_leading_zeros if we invert negative values
	s32 scanvalue = value ^ (s32(value) >> 31);

	// exponent is related to the number of leading bits starting from bit 14
	int exponent = 7 - count_leading_zeros(scanvalue << 17);

	// smallest exponent value allowed is 1
	exponent = std::max(exponent, 1);

	// apply the shift back and forth to zero out bits that are lost
	exponent -= 1;
	return (value >> exponent) << exponent;
}


//*********************************************************
//  REGISTER CLASSES
//*********************************************************

// ======================> ymfm_opdata_cache

// this class holds data that is computed once at the start of clocking
// and remains static during subsequent sound generation
struct ymfm_opdata_cache
{
	// set phase_step to this value to recalculate it each sample; needed
	// in the case of PM LFO changes
	static constexpr u32 PHASE_STEP_DYNAMIC = 1;

	u16 const *waveform;         // base of sine table
	u32 phase_step;              // phase step, or PHASE_STEP_DYNAMIC if PM is active
	u32 total_level;             // total level * 8 + KSL
	u32 block_freq;              // raw block frequency value (used to compute phase_step)
	s32 detune;                  // detuning value (used to compute phase_step)
	u32 multiple;                // multiple value (x.1, used to compute phase_step)
	u32 eg_sustain;              // sustain level, shifted up to envelope values
	u8 eg_rate[YMFM_ENV_STATES]; // envelope rate, including KSR
};


// ======================> ymfm_registers_base

// base class for family-specific register classes; this provides a few
// constants, common defaults, and helpers, but mostly each derived
// class is responsible for defining all commonly-called methods
class ymfm_registers_base
{
public:
	// this value is returned from the write() function for rhythm channels
	static constexpr u32 YMFM_RHYTHM_CHANNEL = 0xff;

	// this is the size of a full sin waveform
	static constexpr u32 WAVEFORM_LENGTH = 0x400;

	//
	// the following constants need to be defined per family:
	//          u32 OUTPUTS: The number of outputs exposed (1-4)
	//         u32 CHANNELS: The number of channels on the chip
	//     u32 ALL_CHANNELS: A bitmask of all channels
	//        u32 OPERATORS: The number of operators on the chip
	//     bool DYNAMIC_OPS: True if ops/channel can be changed at runtime
	//        u32 WAVEFORMS: The number of waveforms offered
	//        u32 REGISTERS: The number of 8-bit registers allocated
	//         u32 REG_MODE: The address of the "mode" register controlling timers
	// u32 DEFAULT_PRESCALE: The starting clock prescale
	// u32 EG_CLOCK_DIVIDER: The clock divider of the envelope generator
	//  bool EG_HAS_DEPRESS: True if the chip has a DP ("depress"?) envelope stage
	//      bool EG_HAS_SSG: True if the chip has SSG envelope support
	// bool MODULATOR_DELAY: True if the modulator is delayed by 1 sample (OPL pre-OPL3)
	// u32 CSM_TRIGGER_MASK: Mask of channels to trigger in CSM mode
	//     u8 STATUS_TIMERA: Status bit to set when timer A fires
	//     u8 STATUS_TIMERB: Status bit to set when tiemr B fires
	//       u8 STATUS_BUSY: Status bit to set when the chip is busy
	//        u8 STATUS_IRQ: Status bit to set when an IRQ is signalled
	//

	// system-wide register defaults
	u32 status_mask() const                { return 0; } // OPL only
	u32 irq_reset() const                  { return 0; } // OPL only
	u32 noise_enable() const               { return 0; } // OPM only
	u32 rhythm_enable() const              { return 0; } // OPL only

	// per-operator register defaults
	u32 op_ssg_eg_enable(u32 opoffs) const { return 0; } // OPN(A) only
	u32 op_ssg_eg_mode(u32 opoffs) const   { return 0; } // OPN(A) only

protected:
	// helper to encode four operator numbers into a 32-bit value in the
	// operator maps for each register class
	static constexpr u32 operator_list(u8 o1 = 0xff, u8 o2 = 0xff, u8 o3 = 0xff, u8 o4 = 0xff)
	{
		return o1 | (o2 << 8) | (o3 << 16) | (o4 << 24);
	}

	// helper to apply KSR to the raw ADSR rate, ignoring ksr if the
	// raw value is 0, and clamping to 63
	static constexpr u32 effective_rate(u32 rawrate, u32 ksr)
	{
		return (rawrate == 0) ? 0 : std::min<u32>(rawrate + ksr, 63);
	}
};


// ======================> ymopm_registers

//
// OPM register map:
//
//      System-wide registers:
//           01 xxxxxxxx Test register
//           08 x------- Key on/off operator 4
//              -x------ Key on/off operator 3
//              --x----- Key on/off operator 2
//              ---x---- Key on/off operator 1
//              -----xxx Channel select
//           0F x------- NE
//              ---xxxxx NFRQ
//           10 xxxxxxxx Timer A value (upper 8 bits)
//           11 ------xx Timer A value (lower 2 bits)
//           12 xxxxxxxx Timer B value
//           14 x------- CSM mode
//              --x----- Reset timer B
//              ---x---- Reset timer A
//              ----x--- Enable timer B
//              -----x-- Enable timer A
//              ------x- Load timer B
//              -------x Load timer A
//           18 xxxxxxxx LFO frequency
//           19 xxxxxxxx PM/AM LFO depth
//           1B xx------ CT
//              ------xx W
//
//     Per-channel registers (channel in address bits 0-2)
//        20-27 x------- Pan right
//              -x------ Pan left
//              --xxx--- Feedback level for operator 1 (0-7)
//              -----xxx Operator connection algorithm (0-7)
//        28-2F -xxxxxxx Key code
//        30-37 xxxxxx-- KF
//        38-3F -xxx---- PM sensitivity
//              ------xx AM shift
//
//     Per-operator registers (channel in address bits 0-2, operator in bits 3-4)
//        40-5F -xxx---- Detune value (0-7)
//              ----xxxx Multiple value (0-15)
//        60-7F -xxxxxxx Total level (0-127)
//        80-9F xx------ Key scale rate (0-3)
//              ---xxxxx Attack rate (0-31)
//        A0-BF x------- LFO AM enable
//              ---xxxxx Decay rate (0-31)
//        C0-DF xx------ Detune 2 value (0-3)
//              ---xxxxx Sustain rate (0-31)
//        E0-FF xxxx---- Sustain level (0-15)
//              ----xxxx Release rate (0-15)
//
//     Internal (fake) registers:
//           19 -xxxxxxx AM depth
//           1A -xxxxxxx PM depth
//

class ymopm_registers : public ymfm_registers_base
{
	// LFO waveforms are 256 entries long
	static constexpr u32 LFO_WAVEFORM_LENGTH = 256;

public:
	// constants
	static constexpr u32 OUTPUTS = 2;
	static constexpr u32 CHANNELS = 8;
	static constexpr u32 ALL_CHANNELS = (1 << CHANNELS) - 1;
	static constexpr u32 OPERATORS = CHANNELS * 4;
	static constexpr bool DYNAMIC_OPS = false;
	static constexpr u32 WAVEFORMS = 1;
	static constexpr u32 REGISTERS = 0x100;
	static constexpr u32 REG_MODE = 0x14;
	static constexpr u32 DEFAULT_PRESCALE = 2;
	static constexpr u32 EG_CLOCK_DIVIDER = 3;
	static constexpr bool EG_HAS_DEPRESS = false;
	static constexpr bool EG_HAS_SSG = false;
	static constexpr bool MODULATOR_DELAY = false;
	static constexpr u32 CSM_TRIGGER_MASK = ALL_CHANNELS;
	static constexpr u8 STATUS_TIMERA = 0x01;
	static constexpr u8 STATUS_TIMERB = 0x02;
	static constexpr u8 STATUS_BUSY = 0x80;
	static constexpr u8 STATUS_IRQ = 0;

	// constructor
	ymopm_registers();

	// register for save states
	void save(device_t &device);

	// reset to initial state
	void reset();

	// map channel number to register offset
	static constexpr u32 channel_offset(u32 chnum)
	{
		assert(chnum < CHANNELS);
		return chnum;
	}

	// map operator number to register offset
	static constexpr u32 operator_offset(u32 opnum)
	{
		assert(opnum < OPERATORS);
		return opnum;
	}

	// return an array of operator indices for each channel
	struct operator_mapping { u32 chan[CHANNELS]; };
	void operator_map(operator_mapping &dest) const;

	// handle writes to the register array
	bool write(u16 index, u8 data, u32 &chan, u32 &opmask);

	// clock the noise and LFO, if present, returning LFO PM value
	s32 clock_noise_and_lfo();

	// reset the LFO
	void reset_lfo() { m_lfo_counter = 0; }

	// return the AM offset from LFO for the given channel
	u32 lfo_am_offset(u32 choffs) const;

	// return the current noise state, gated by the noise clock
	u32 noise_state() const { return m_noise_state; }

	// caching helpers
	void cache_operator_data(u32 choffs, u32 opoffs, ymfm_opdata_cache &cache);

	// compute the phase step, given a PM value
	u32 compute_phase_step(u32 choffs, u32 opoffs, ymfm_opdata_cache const &cache, s32 lfo_raw_pm);

	// log a key-on event
	void log_keyon(u32 choffs, u32 opoffs);

	// system-wide registers
	u32 test() const                       { return byte(0x01, 0, 8); }
	u32 noise_frequency() const            { return byte(0x0f, 0, 5); }
	u32 noise_enable() const               { return byte(0x0f, 7, 1); }
	u32 timer_a_value() const              { return word(0x10, 0, 8, 0x11, 0, 2); }
	u32 timer_b_value() const              { return byte(0x12, 0, 8); }
	u32 csm() const                        { return byte(0x14, 7, 1); }
	u32 reset_timer_b() const              { return byte(0x14, 5, 1); }
	u32 reset_timer_a() const              { return byte(0x14, 4, 1); }
	u32 enable_timer_b() const             { return byte(0x14, 3, 1); }
	u32 enable_timer_a() const             { return byte(0x14, 2, 1); }
	u32 load_timer_b() const               { return byte(0x14, 1, 1); }
	u32 load_timer_a() const               { return byte(0x14, 0, 1); }
	u32 lfo_rate() const                   { return byte(0x18, 0, 8); }
	u32 lfo_am_depth() const               { return byte(0x19, 0, 7); }
	u32 lfo_pm_depth() const               { return byte(0x1a, 0, 7); }
	u32 lfo_waveform() const               { return byte(0x1b, 0, 2); }

	// per-channel registers
	u32 ch_output_any(u32 choffs) const    { return byte(0x20, 6, 2, choffs); }
	u32 ch_output_0(u32 choffs) const      { return byte(0x20, 6, 1, choffs); }
	u32 ch_output_1(u32 choffs) const      { return byte(0x20, 7, 1, choffs); }
	u32 ch_output_2(u32 choffs) const      { return 0; }
	u32 ch_output_3(u32 choffs) const      { return 0; }
	u32 ch_feedback(u32 choffs) const      { return byte(0x20, 3, 3, choffs); }
	u32 ch_algorithm(u32 choffs) const     { return byte(0x20, 0, 3, choffs); }
	u32 ch_block_freq(u32 choffs) const    { return word(0x28, 0, 7, 0x30, 2, 6, choffs); }
	u32 ch_lfo_pm_sens(u32 choffs) const   { return byte(0x38, 4, 3, choffs); }
	u32 ch_lfo_am_sens(u32 choffs) const   { return byte(0x38, 0, 2, choffs); }

	// per-operator registers
	u32 op_detune(u32 opoffs) const        { return byte(0x40, 4, 3, opoffs); }
	u32 op_multiple(u32 opoffs) const      { return byte(0x40, 0, 4, opoffs); }
	u32 op_total_level(u32 opoffs) const   { return byte(0x60, 0, 7, opoffs); }
	u32 op_ksr(u32 opoffs) const           { return byte(0x80, 6, 2, opoffs); }
	u32 op_attack_rate(u32 opoffs) const   { return byte(0x80, 0, 5, opoffs); }
	u32 op_lfo_am_enable(u32 opoffs) const { return byte(0xa0, 7, 1, opoffs); }
	u32 op_decay_rate(u32 opoffs) const    { return byte(0xa0, 0, 5, opoffs); }
	u32 op_detune2(u32 opoffs) const       { return byte(0xc0, 6, 2, opoffs); }
	u32 op_sustain_rate(u32 opoffs) const  { return byte(0xc0, 0, 5, opoffs); }
	u32 op_sustain_level(u32 opoffs) const { return byte(0xe0, 4, 4, opoffs); }
	u32 op_release_rate(u32 opoffs) const  { return byte(0xe0, 0, 4, opoffs); }

protected:
	// return a bitfield extracted from a byte
	u32 byte(u32 offset, u32 start, u32 count, u32 extra_offset = 0) const
	{
		return BIT(m_regdata[offset + extra_offset], start, count);
	}

	// return a bitfield extracted from a pair of bytes, MSBs listed first
	u32 word(u32 offset1, u32 start1, u32 count1, u32 offset2, u32 start2, u32 count2, u32 extra_offset = 0) const
	{
		return (byte(offset1, start1, count1, extra_offset) << count2) | byte(offset2, start2, count2, extra_offset);
	}

	// internal state
	u32 m_lfo_counter;               // LFO counter
	u32 m_noise_lfsr;                // noise LFSR state
	u8 m_noise_counter;              // noise counter
	u8 m_noise_state;                // latched noise state
	u8 m_noise_lfo;                  // latched LFO noise value
	u8 m_lfo_am;                     // current LFO AM value
	u8 m_regdata[REGISTERS];         // register data
	s16 m_lfo_waveform[4][LFO_WAVEFORM_LENGTH]; // LFO waveforms; AM in low 8, PM in upper 8
	u16 m_waveform[WAVEFORMS][WAVEFORM_LENGTH]; // waveforms
};


// ======================> ymopn_registers_base

//
// OPN register map:
//
//      System-wide registers:
//           21 xxxxxxxx Test register
//           22 ----x--- LFO enable [OPNA+ only]
//              -----xxx LFO rate [OPNA+ only]
//           24 xxxxxxxx Timer A value (upper 8 bits)
//           25 ------xx Timer A value (lower 2 bits)
//           26 xxxxxxxx Timer B value
//           27 xx------ CSM/Multi-frequency mode for channel #2
//              --x----- Reset timer B
//              ---x---- Reset timer A
//              ----x--- Enable timer B
//              -----x-- Enable timer A
//              ------x- Load timer B
//              -------x Load timer A
//           28 x------- Key on/off operator 4
//              -x------ Key on/off operator 3
//              --x----- Key on/off operator 2
//              ---x---- Key on/off operator 1
//              ------xx Channel select
//
//     Per-channel registers (channel in address bits 0-1)
//     Note that all these apply to address+100 as well on OPNA+
//        A0-A3 xxxxxxxx Frequency number lower 8 bits
//        A4-A7 --xxx--- Block (0-7)
//              -----xxx Frequency number upper 3 bits
//        B0-B3 --xxx--- Feedback level for operator 1 (0-7)
//              -----xxx Operator connection algorithm (0-7)
//        B4-B7 x------- Pan left [OPNA]
//              -x------ Pan right [OPNA]
//              --xx---- LFO AM shift (0-3) [OPNA+ only]
//              -----xxx LFO PM depth (0-7) [OPNA+ only]
//
//     Per-operator registers (channel in address bits 0-1, operator in bits 2-3)
//     Note that all these apply to address+100 as well on OPNA+
//        30-3F -xxx---- Detune value (0-7)
//              ----xxxx Multiple value (0-15)
//        40-4F -xxxxxxx Total level (0-127)
//        50-5F xx------ Key scale rate (0-3)
//              ---xxxxx Attack rate (0-31)
//        60-6F x------- LFO AM enable [OPNA]
//              ---xxxxx Decay rate (0-31)
//        70-7F ---xxxxx Sustain rate (0-31)
//        80-8F xxxx---- Sustain level (0-15)
//              ----xxxx Release rate (0-15)
//        90-9F ----x--- SSG-EG enable
//              -----xxx SSG-EG envelope (0-7)
//
//     Special multi-frequency registers (channel implicitly #2; operator in address bits 0-1)
//        A8-AB xxxxxxxx Frequency number lower 8 bits
//        AC-AF --xxx--- Block (0-7)
//              -----xxx Frequency number upper 3 bits
//
//     Internal (fake) registers:
//        B8-BB --xxxxxx Latched frequency number upper bits (from A4-A7)
//        BC-BF --xxxxxx Latched frequency number upper bits (from AC-AF)
//

template<bool IsOpnA>
class ymopn_registers_base : public ymfm_registers_base
{
public:
	// constants
	static constexpr u32 OUTPUTS = IsOpnA ? 2 : 1;
	static constexpr u32 CHANNELS = IsOpnA ? 6 : 3;
	static constexpr u32 ALL_CHANNELS = (1 << CHANNELS) - 1;
	static constexpr u32 OPERATORS = CHANNELS * 4;
	static constexpr bool DYNAMIC_OPS = false;
	static constexpr u32 WAVEFORMS = 1;
	static constexpr u32 REGISTERS = IsOpnA ? 0x200 : 0x100;
	static constexpr u32 REG_MODE = 0x27;
	static constexpr u32 DEFAULT_PRESCALE = 6;
	static constexpr u32 EG_CLOCK_DIVIDER = 3;
	static constexpr bool EG_HAS_DEPRESS = false;
	static constexpr bool EG_HAS_SSG = true;
	static constexpr bool MODULATOR_DELAY = false;
	static constexpr u32 CSM_TRIGGER_MASK = 1 << 2;
	static constexpr u8 STATUS_TIMERA = 0x01;
	static constexpr u8 STATUS_TIMERB = 0x02;
	static constexpr u8 STATUS_BUSY = 0x80;
	static constexpr u8 STATUS_IRQ = 0;

	// constructor
	ymopn_registers_base();

	// register for save states
	void save(device_t &device);

	// reset to initial state
	void reset();

	// map channel number to register offset
	static constexpr u32 channel_offset(u32 chnum)
	{
		assert(chnum < CHANNELS);
		if (!IsOpnA)
			return chnum;
		else
			return (chnum % 3) + 0x100 * (chnum / 3);
	}

	// map operator number to register offset
	static constexpr u32 operator_offset(u32 opnum)
	{
		assert(opnum < OPERATORS);
		if (!IsOpnA)
			return opnum + opnum / 3;
		else
			return (opnum % 12) + ((opnum % 12) / 3) + 0x100 * (opnum / 12);
	}

	// return an array of operator indices for each channel
	struct operator_mapping { u32 chan[CHANNELS]; };
	void operator_map(operator_mapping &dest) const;

	// handle writes to the register array
	bool write(u16 index, u8 data, u32 &chan, u32 &opmask);

	// clock the noise and LFO, if present, returning LFO PM value
	s32 clock_noise_and_lfo();

	// reset the LFO
	void reset_lfo() { m_lfo_counter = 0; }

	// return the AM offset from LFO for the given channel
	u32 lfo_am_offset(u32 choffs) const;

	// return LFO/noise states
	u32 noise_state() const { return 0; }

	// caching helpers
	void cache_operator_data(u32 choffs, u32 opoffs, ymfm_opdata_cache &cache);

	// compute the phase step, given a PM value
	u32 compute_phase_step(u32 choffs, u32 opoffs, ymfm_opdata_cache const &cache, s32 lfo_raw_pm);

	// log a key-on event
	void log_keyon(u32 choffs, u32 opoffs);

	// system-wide registers
	u32 test() const                       { return byte(0x21, 0, 8); }
	u32 lfo_enable() const                 { return IsOpnA ? byte(0x22, 3, 1) : 0; }
	u32 lfo_rate() const                   { return IsOpnA ? byte(0x22, 0, 3) : 0; }
	u32 timer_a_value() const              { return word(0x24, 0, 8, 0x25, 0, 2); }
	u32 timer_b_value() const              { return byte(0x26, 0, 8); }
	u32 csm() const                        { return (byte(0x27, 6, 2) == 2); }
	u32 multi_freq() const                 { return (byte(0x27, 6, 2) != 0); }
	u32 reset_timer_b() const              { return byte(0x27, 5, 1); }
	u32 reset_timer_a() const              { return byte(0x27, 4, 1); }
	u32 enable_timer_b() const             { return byte(0x27, 3, 1); }
	u32 enable_timer_a() const             { return byte(0x27, 2, 1); }
	u32 load_timer_b() const               { return byte(0x27, 1, 1); }
	u32 load_timer_a() const               { return byte(0x27, 0, 1); }
	u32 multi_block_freq(u32 num) const    { return word(0xac, 0, 6, 0xa8, 0, 8, num); }

	// per-channel registers
	u32 ch_block_freq(u32 choffs) const    { return word(0xa4, 0, 6, 0xa0, 0, 8, choffs); }
	u32 ch_feedback(u32 choffs) const      { return byte(0xb0, 3, 3, choffs); }
	u32 ch_algorithm(u32 choffs) const     { return byte(0xb0, 0, 3, choffs); }
	u32 ch_output_any(u32 choffs) const    { return IsOpnA ? byte(0xb4, 6, 2, choffs) : 1; }
	u32 ch_output_0(u32 choffs) const      { return IsOpnA ? byte(0xb4, 7, 1, choffs) : 1; }
	u32 ch_output_1(u32 choffs) const      { return IsOpnA ? byte(0xb4, 6, 1, choffs) : 0; }
	u32 ch_output_2(u32 choffs) const      { return 0; }
	u32 ch_output_3(u32 choffs) const      { return 0; }
	u32 ch_lfo_am_sens(u32 choffs) const   { return IsOpnA ? byte(0xb4, 4, 2, choffs) : 0; }
	u32 ch_lfo_pm_sens(u32 choffs) const   { return IsOpnA ? byte(0xb4, 0, 3, choffs) : 0; }

	// per-operator registers
	u32 op_detune(u32 opoffs) const        { return byte(0x30, 4, 3, opoffs); }
	u32 op_multiple(u32 opoffs) const      { return byte(0x30, 0, 4, opoffs); }
	u32 op_total_level(u32 opoffs) const   { return byte(0x40, 0, 7, opoffs); }
	u32 op_ksr(u32 opoffs) const           { return byte(0x50, 6, 2, opoffs); }
	u32 op_attack_rate(u32 opoffs) const   { return byte(0x50, 0, 5, opoffs); }
	u32 op_decay_rate(u32 opoffs) const    { return byte(0x60, 0, 5, opoffs); }
	u32 op_lfo_am_enable(u32 opoffs) const { return IsOpnA ? byte(0x60, 7, 1, opoffs) : 0; }
	u32 op_sustain_rate(u32 opoffs) const  { return byte(0x70, 0, 5, opoffs); }
	u32 op_sustain_level(u32 opoffs) const { return byte(0x80, 4, 4, opoffs); }
	u32 op_release_rate(u32 opoffs) const  { return byte(0x80, 0, 4, opoffs); }
	u32 op_ssg_eg_enable(u32 opoffs) const { return byte(0x90, 3, 1, opoffs); }
	u32 op_ssg_eg_mode(u32 opoffs) const   { return byte(0x90, 0, 3, opoffs); }

protected:
	// return a bitfield extracted from a byte
	u32 byte(u32 offset, u32 start, u32 count, u32 extra_offset = 0) const
	{
		return BIT(m_regdata[offset + extra_offset], start, count);
	}

	// return a bitfield extracted from a pair of bytes, MSBs listed first
	u32 word(u32 offset1, u32 start1, u32 count1, u32 offset2, u32 start2, u32 count2, u32 extra_offset = 0) const
	{
		return (byte(offset1, start1, count1, extra_offset) << count2) | byte(offset2, start2, count2, extra_offset);
	}

	// internal state
	u32 m_lfo_counter;               // LFO counter
	u8 m_lfo_am;                     // current LFO AM value
	u8 m_regdata[REGISTERS];         // register data
	u16 m_waveform[WAVEFORMS][WAVEFORM_LENGTH]; // waveforms
};

using ymopn_registers = ymopn_registers_base<false>;
using ymopna_registers = ymopn_registers_base<true>;


// ======================> ymopl_registers_base

//
// OPL/OPL2/OPL3/OPL4 register map:
//
//      System-wide registers:
//           01 xxxxxxxx Test register
//              --x----- Enable OPL compatibility mode [OPL2 only] (1 = enable)
//           02 xxxxxxxx Timer A value (4 * OPN)
//           03 xxxxxxxx Timer B value
//           04 x------- RST
//              -x------ Mask timer A
//              --x----- Mask timer B
//              ------x- Load timer B
//              -------x Load timer A
//           08 x------- CSM mode [OPL/OPL2 only]
//              -x------ Note select
//           BD x------- AM depth
//              -x------ PM depth
//              --x----- Rhythm enable
//              ---x---- Bass drum key on
//              ----x--- Snare drum key on
//              -----x-- Tom key on
//              ------x- Top cymbal key on
//              -------x High hat key on
//          101 --xxxxxx Test register 2 [OPL3 only]
//          104 --x----- Channel 6 4-operator mode [OPL3 only]
//              ---x---- Channel 5 4-operator mode [OPL3 only]
//              ----x--- Channel 4 4-operator mode [OPL3 only]
//              -----x-- Channel 3 4-operator mode [OPL3 only]
//              ------x- Channel 2 4-operator mode [OPL3 only]
//              -------x Channel 1 4-operator mode [OPL3 only]
//          105 -------x New [OPL3 only]
//              ------x- New2 [OPL4 only]
//
//     Per-channel registers (channel in address bits 0-3)
//     Note that all these apply to address+100 as well on OPL3+
//        A0-A8 xxxxxxxx F-number (low 8 bits)
//        B0-B8 --x----- Key on
//              ---xxx-- Block (octvate, 0-7)
//              ------xx F-number (high two bits)
//        C0-C8 x------- CHD output (to DO0 pin) [OPL3+ only]
//              -x------ CHC output (to DO0 pin) [OPL3+ only]
//              --x----- CHB output (mixed right, to DO2 pin) [OPL3+ only]
//              ---x---- CHA output (mixed left, to DO2 pin) [OPL3+ only]
//              ----xxx- Feedback level for operator 1 (0-7)
//              -------x Operator connection algorithm
//
//     Per-operator registers (operator in bits 0-5)
//     Note that all these apply to address+100 as well on OPL3+
//        20-35 x------- AM enable
//              -x------ PM enable (VIB)
//              --x----- EG type
//              ---x---- Key scale rate
//              ----xxxx Multiple value (0-15)
//        40-55 xx------ Key scale level (0-3)
//              --xxxxxx Total level (0-63)
//        60-75 xxxx---- Attack rate (0-15)
//              ----xxxx Decay rate (0-15)
//        80-95 xxxx---- Sustain level (0-15)
//              ----xxxx Release rate (0-15)
//        E0-F5 ------xx Wave select (0-3) [OPL2 only]
//              -----xxx Wave select (0-7) [OPL3+ only]
//

template<int Revision>
class ymopl_registers_base : public ymfm_registers_base
{
	static constexpr bool IsOpl2 = (Revision == 2);
	static constexpr bool IsOpl2Plus = (Revision >= 2);
	static constexpr bool IsOpl3Plus = (Revision >= 3);
	static constexpr bool IsOpl4Plus = (Revision >= 4);

public:
	// constants
	static constexpr u32 OUTPUTS = IsOpl3Plus ? 4 : 1;
	static constexpr u32 CHANNELS = IsOpl3Plus ? 18 : 9;
	static constexpr u32 ALL_CHANNELS = (1 << CHANNELS) - 1;
	static constexpr u32 OPERATORS = CHANNELS * 2;
	static constexpr bool DYNAMIC_OPS = IsOpl3Plus;
	static constexpr u32 WAVEFORMS = IsOpl3Plus ? 8 : (IsOpl2Plus ? 4 : 1);
	static constexpr u32 REGISTERS = IsOpl3Plus ? 0x200 : 0x100;
	static constexpr u32 REG_MODE = 0x04;
	static constexpr u32 DEFAULT_PRESCALE = IsOpl4Plus ? 19 : (IsOpl3Plus ? 8 : 4);
	static constexpr u32 EG_CLOCK_DIVIDER = 1;
	static constexpr bool EG_HAS_DEPRESS = false;
	static constexpr bool EG_HAS_SSG = false;
	static constexpr bool MODULATOR_DELAY = !IsOpl3Plus;
	static constexpr u32 CSM_TRIGGER_MASK = ALL_CHANNELS;
	static constexpr u8 STATUS_TIMERA = 0x40;
	static constexpr u8 STATUS_TIMERB = 0x20;
	static constexpr u8 STATUS_BUSY = 0;
	static constexpr u8 STATUS_IRQ = 0x80;

	// constructor
	ymopl_registers_base();

	// register for save states
	void save(device_t &device);

	// reset to initial state
	void reset();

	// map channel number to register offset
	static constexpr u32 channel_offset(u32 chnum)
	{
		assert(chnum < CHANNELS);
		if (!IsOpl3Plus)
			return chnum;
		else
			return (chnum % 9) + 0x100 * (chnum / 9);
	}

	// map operator number to register offset
	static constexpr u32 operator_offset(u32 opnum)
	{
		assert(opnum < OPERATORS);
		if (!IsOpl3Plus)
			return opnum + 2 * (opnum / 6);
		else
			return (opnum % 18) + 2 * ((opnum % 18) / 6) + 0x100 * (opnum / 18);
	}

	// return an array of operator indices for each channel
	struct operator_mapping { u32 chan[CHANNELS]; };
	void operator_map(operator_mapping &dest) const;

	// OPL4 apparently can read back FM registers?
	u8 read(u16 index) { return m_regdata[index]; }

	// handle writes to the register array
	bool write(u16 index, u8 data, u32 &chan, u32 &opmask);

	// clock the noise and LFO, if present, returning LFO PM value
	s32 clock_noise_and_lfo();

	// reset the LFO
	void reset_lfo() { m_lfo_am_counter = m_lfo_pm_counter = 0; }

	// return the AM offset from LFO for the given channel
	// on OPL this is just a fixed value
	u32 lfo_am_offset(u32 choffs) const { return m_lfo_am; }

	// return LFO/noise states
	u32 noise_state() const { return m_noise_lfsr >> 23; }

	// caching helpers
	void cache_operator_data(u32 choffs, u32 opoffs, ymfm_opdata_cache &cache);

	// compute the phase step, given a PM value
	u32 compute_phase_step(u32 choffs, u32 opoffs, ymfm_opdata_cache const &cache, s32 lfo_raw_pm);

	// log a key-on event
	void log_keyon(u32 choffs, u32 opoffs);

	// system-wide registers
	u32 test() const                       { return byte(0x01, 0, 8); }
	u32 waveform_enable() const            { return IsOpl2 ? byte(0x01, 5, 1) : (IsOpl3Plus ? 1 : 0); }
	u32 timer_a_value() const              { return byte(0x02, 0, 8) * 4; } // 8->10 bits
	u32 timer_b_value() const              { return byte(0x03, 0, 8); }
	u32 status_mask() const                { return byte(0x04, 0, 8) & 0x78; }
	u32 irq_reset() const                  { return byte(0x04, 7, 1); }
	u32 reset_timer_b() const              { return byte(0x04, 7, 1) | byte(0x04, 5, 1); }
	u32 reset_timer_a() const              { return byte(0x04, 7, 1) | byte(0x04, 6, 1); }
	u32 enable_timer_b() const             { return 1; }
	u32 enable_timer_a() const             { return 1; }
	u32 load_timer_b() const               { return byte(0x04, 1, 1); }
	u32 load_timer_a() const               { return byte(0x04, 0, 1); }
	u32 csm() const                        { return IsOpl3Plus ? 0 : byte(0x08, 7, 1); }
	u32 note_select() const                { return byte(0x08, 6, 1); }
	u32 lfo_am_depth() const               { return byte(0xbd, 7, 1); }
	u32 lfo_pm_depth() const               { return byte(0xbd, 6, 1); }
	u32 rhythm_enable() const              { return byte(0xbd, 5, 1); }
	u32 rhythm_keyon() const               { return byte(0xbd, 4, 0); }
	u32 newflag() const                    { return IsOpl3Plus ? byte(0x105, 0, 1) : 0; }
	u32 new2flag() const                   { return IsOpl4Plus ? byte(0x105, 1, 1) : 0; }
	u32 fourop_enable() const              { return IsOpl3Plus ? byte(0x104, 0, 6) : 0; }

	// per-channel registers
	u32 ch_block_freq(u32 choffs) const    { return word(0xb0, 0, 5, 0xa0, 0, 8, choffs); }
	u32 ch_feedback(u32 choffs) const      { return byte(0xc0, 1, 3, choffs); }
	u32 ch_algorithm(u32 choffs) const     { return byte(0xc0, 0, 1, choffs) | (IsOpl3Plus ? (8 | (byte(0xc3, 0, 1, choffs) << 1)) : 0); }
	u32 ch_output_any(u32 choffs) const    { return newflag() ? byte(0xc0 + choffs, 4, 4) : 1; }
	u32 ch_output_0(u32 choffs) const      { return newflag() ? byte(0xc0 + choffs, 4, 1) : 1; }
	u32 ch_output_1(u32 choffs) const      { return newflag() ? byte(0xc0 + choffs, 5, 1) : (IsOpl3Plus ? 1 : 0); }
	u32 ch_output_2(u32 choffs) const      { return newflag() ? byte(0xc0 + choffs, 6, 1) : 0; }
	u32 ch_output_3(u32 choffs) const      { return newflag() ? byte(0xc0 + choffs, 7, 1) : 0; }

	// per-operator registers
	u32 op_lfo_am_enable(u32 opoffs) const { return byte(0x20, 7, 1, opoffs); }
	u32 op_lfo_pm_enable(u32 opoffs) const { return byte(0x20, 6, 1, opoffs); }
	u32 op_eg_sustain(u32 opoffs) const    { return byte(0x20, 5, 1, opoffs); }
	u32 op_ksr(u32 opoffs) const           { return byte(0x20, 4, 1, opoffs); }
	u32 op_multiple(u32 opoffs) const      { return byte(0x20, 0, 4, opoffs); }
	u32 op_ksl(u32 opoffs) const           { return bitswap<2>(byte(0x40, 6, 2, opoffs), 0, 1); }
	u32 op_total_level(u32 opoffs) const   { return byte(0x40, 0, 6, opoffs); }
	u32 op_attack_rate(u32 opoffs) const   { return byte(0x60, 4, 4, opoffs); }
	u32 op_decay_rate(u32 opoffs) const    { return byte(0x60, 0, 4, opoffs); }
	u32 op_sustain_level(u32 opoffs) const { return byte(0x80, 4, 4, opoffs); }
	u32 op_release_rate(u32 opoffs) const  { return byte(0x80, 0, 4, opoffs); }
	u32 op_waveform(u32 opoffs) const      { return IsOpl2Plus ? byte(0xe0, 0, newflag() ? 3 : 2, opoffs) : 0; }

protected:
	// return a bitfield extracted from a byte
	u32 byte(u32 offset, u32 start, u32 count, u32 extra_offset = 0) const
	{
		return BIT(m_regdata[offset + extra_offset], start, count);
	}

	// return a bitfield extracted from a pair of bytes, MSBs listed first
	u32 word(u32 offset1, u32 start1, u32 count1, u32 offset2, u32 start2, u32 count2, u32 extra_offset = 0) const
	{
		return (byte(offset1, start1, count1, extra_offset) << count2) | byte(offset2, start2, count2, extra_offset);
	}

	// helper to determine if the this channel is an active rhythm channel
	bool is_rhythm(u32 choffs) const
	{
		return rhythm_enable() && (choffs >= 6 && choffs <= 8);
	}

	// internal state
	u16 m_lfo_am_counter;            // LFO AM counter
	u16 m_lfo_pm_counter;            // LFO PM counter
	u32 m_noise_lfsr;                // noise LFSR state
	u8 m_lfo_am;                     // current LFO AM value
	u8 m_regdata[REGISTERS];         // register data
	u16 m_waveform[WAVEFORMS][WAVEFORM_LENGTH]; // waveforms
};

using ymopl_registers = ymopl_registers_base<1>;
using ymopl2_registers = ymopl_registers_base<2>;
using ymopl3_registers = ymopl_registers_base<3>;
using ymopl4_registers = ymopl_registers_base<4>;


// ======================> ymopll_registers

//
// OPLL register map:
//
//      System-wide registers:
//           0E --x----- Rhythm enable
//              ---x---- Bass drum key on
//              ----x--- Snare drum key on
//              -----x-- Tom key on
//              ------x- Top cymbal key on
//              -------x High hat key on
//           0F xxxxxxxx Test register
//
//     Per-channel registers (channel in address bits 0-3)
//        10-18 xxxxxxxx F-number (low 8 bits)
//        20-28 --x----- Sustain on
//              ---x---- Key on
//              --- xxx- Block (octvate, 0-7)
//              -------x F-number (high bit)
//        30-38 xxxx---- Instrument selection
//              ----xxxx Volume
//
//     User instrument registers (for carrier, modulator operators)
//        00-01 x------- AM enable
//              -x------ PM enable (VIB)
//              --x----- EG type
//              ---x---- Key scale rate
//              ----xxxx Multiple value (0-15)
//           02 xx------ Key scale level (carrier, 0-3)
//              --xxxxxx Total level (modulator, 0-63)
//           03 xx------ Key scale level (modulator, 0-3)
//              ---x---- Rectified wave (carrier)
//              ----x--- Rectified wave (modulator)
//              -----xxx Feedback level for operator 1 (0-7)
//        04-05 xxxx---- Attack rate (0-15)
//              ----xxxx Decay rate (0-15)
//        06-07 xxxx---- Sustain level (0-15)
//              ----xxxx Release rate (0-15)
//
//     Internal (fake) registers:
//        40-48 xxxxxxxx Current instrument base address
//        4E-5F xxxxxxxx Current instrument base address + operator slot (0/1)
//        70-FF xxxxxxxx Data for instruments (1-16 plus 3 drums)
//

class ymopll_registers : public ymfm_registers_base
{
public:
	static constexpr u32 OUTPUTS = 2;
	static constexpr u32 CHANNELS = 9;
	static constexpr u32 ALL_CHANNELS = (1 << CHANNELS) - 1;
	static constexpr u32 OPERATORS = CHANNELS * 2;
	static constexpr bool DYNAMIC_OPS = false;
	static constexpr u32 WAVEFORMS = 2;
	static constexpr u32 REGISTERS = 0x40;
	static constexpr u32 REG_MODE = 0x3f;
	static constexpr u32 DEFAULT_PRESCALE = 4;
	static constexpr u32 EG_CLOCK_DIVIDER = 1;
	static constexpr bool EG_HAS_DEPRESS = true;
	static constexpr bool EG_HAS_SSG = false;
	static constexpr bool MODULATOR_DELAY = true;
	static constexpr u32 CSM_TRIGGER_MASK = 0;
	static constexpr u8 STATUS_TIMERA = 0;
	static constexpr u8 STATUS_TIMERB = 0;
	static constexpr u8 STATUS_BUSY = 0;
	static constexpr u8 STATUS_IRQ = 0;

	// OPLL-specific constants
	static constexpr u32 INSTDATA_SIZE = 0x90;

	// constructor
	ymopll_registers();

	// register for save states
	void save(device_t &device);

	// reset to initial state
	void reset();

	// map channel number to register offset
	static constexpr u32 channel_offset(u32 chnum)
	{
		assert(chnum < CHANNELS);
		return chnum;
	}

	// map operator number to register offset
	static constexpr u32 operator_offset(u32 opnum)
	{
		assert(opnum < OPERATORS);
		return opnum;
	}

	// return an array of operator indices for each channel
	struct operator_mapping { u32 chan[CHANNELS]; };
	void operator_map(operator_mapping &dest) const;

	// handle writes to the register array
	bool write(u16 index, u8 data, u32 &chan, u32 &opmask);

	// clock the noise and LFO, if present, returning LFO PM value
	s32 clock_noise_and_lfo();

	// reset the LFO
	void reset_lfo() { m_lfo_am_counter = m_lfo_pm_counter = 0; }

	// return the AM offset from LFO for the given channel
	// on OPL this is just a fixed value
	u32 lfo_am_offset(u32 choffs) const { return m_lfo_am; }

	// return LFO/noise states
	u32 noise_state() const { return m_noise_lfsr >> 23; }

	// caching helpers
	void cache_operator_data(u32 choffs, u32 opoffs, ymfm_opdata_cache &cache);

	// compute the phase step, given a PM value
	u32 compute_phase_step(u32 choffs, u32 opoffs, ymfm_opdata_cache const &cache, s32 lfo_raw_pm);

	// log a key-on event
	void log_keyon(u32 choffs, u32 opoffs);

	// set the instrument data
	void set_instrument_data(u8 const *data)
	{
		memcpy(&m_instdata[0], data, INSTDATA_SIZE);
	}

	// system-wide registers
	u32 rhythm_enable() const              { return byte(0x0e, 5, 1); }
	u32 rhythm_keyon() const               { return byte(0x0e, 4, 0); }
	u32 test() const                       { return byte(0x0f, 0, 8); }
	u32 waveform_enable() const            { return 1; }
	u32 timer_a_value() const              { return 0; }
	u32 timer_b_value() const              { return 0; }
	u32 status_mask() const                { return 0; }
	u32 irq_reset() const                  { return 0; }
	u32 reset_timer_b() const              { return 0; }
	u32 reset_timer_a() const              { return 0; }
	u32 enable_timer_b() const             { return 0; }
	u32 enable_timer_a() const             { return 0; }
	u32 load_timer_b() const               { return 0; }
	u32 load_timer_a() const               { return 0; }
	u32 csm() const                        { return 0; }

	// per-channel registers
	u32 ch_block_freq(u32 choffs) const    { return word(0x20, 0, 4, 0x10, 0, 8, choffs); }
	u32 ch_sustain(u32 choffs) const       { return byte(0x20, 5, 1, choffs); }
	u32 ch_total_level(u32 choffs) const   { return instchbyte(0x02, 0, 6, choffs); }
	u32 ch_feedback(u32 choffs) const      { return instchbyte(0x03, 0, 3, choffs); }
	u32 ch_algorithm(u32 choffs) const     { return 0; }
	u32 ch_instrument(u32 choffs) const    { return byte(0x30, 4, 4, choffs); }
	u32 ch_output_any(u32 choffs) const    { return 1; }
	u32 ch_output_0(u32 choffs) const      { return !is_rhythm(choffs); }
	u32 ch_output_1(u32 choffs) const      { return is_rhythm(choffs); }
	u32 ch_output_2(u32 choffs) const      { return 0; }
	u32 ch_output_3(u32 choffs) const      { return 0; }

	// per-operator registers
	u32 op_lfo_am_enable(u32 opoffs) const { return instopbyte(0x00, 7, 1, opoffs); }
	u32 op_lfo_pm_enable(u32 opoffs) const { return instopbyte(0x00, 6, 1, opoffs); }
	u32 op_eg_sustain(u32 opoffs) const    { return instopbyte(0x00, 5, 1, opoffs); }
	u32 op_ksr(u32 opoffs) const           { return instopbyte(0x00, 4, 1, opoffs); }
	u32 op_multiple(u32 opoffs) const      { return instopbyte(0x00, 0, 4, opoffs); }
	u32 op_ksl(u32 opoffs) const           { return instopbyte(0x02, 6, 2, opoffs); }
	u32 op_waveform(u32 opoffs) const      { return instchbyte(0x03, 3 + BIT(opoffs, 0), 1, opoffs >> 1); }
	u32 op_attack_rate(u32 opoffs) const   { return instopbyte(0x04, 4, 4, opoffs); }
	u32 op_decay_rate(u32 opoffs) const    { return instopbyte(0x04, 0, 4, opoffs); }
	u32 op_sustain_level(u32 opoffs) const { return instopbyte(0x06, 4, 4, opoffs); }
	u32 op_release_rate(u32 opoffs) const  { return instopbyte(0x06, 0, 4, opoffs); }
	u32 op_volume(u32 opoffs) const        { return byte(0x30, 4 * BIT(~opoffs, 0), 4, opoffs >> 1); }

private:
	// return a bitfield extracted from a byte
	u32 byte(u32 offset, u32 start, u32 count, u32 extra_offset = 0) const
	{
		return BIT(m_regdata[offset + extra_offset], start, count);
	}

	// return a bitfield extracted from a pair of bytes, MSBs listed first
	u32 word(u32 offset1, u32 start1, u32 count1, u32 offset2, u32 start2, u32 count2, u32 extra_offset = 0) const
	{
		return (byte(offset1, start1, count1, extra_offset) << count2) | byte(offset2, start2, count2, extra_offset);
	}

	// helpers to read from instrument channel/operator data
	u32 instchbyte(u32 offset, u32 start, u32 count, u32 choffs) const { return BIT(m_chinst[choffs][offset], start, count); }
	u32 instopbyte(u32 offset, u32 start, u32 count, u32 opoffs) const { return BIT(m_opinst[opoffs][offset], start, count); }

	// helper to determine if the this channel is an active rhythm channel
	bool is_rhythm(u32 choffs) const
	{
		return rhythm_enable() && choffs >= 6;
	}

	// internal state
	u16 m_lfo_am_counter;            // LFO AM counter
	u16 m_lfo_pm_counter;            // LFO PM counter
	u32 m_noise_lfsr;                // noise LFSR state
	u8 m_lfo_am;                     // current LFO AM value
	u8 const *m_chinst[CHANNELS];    // pointer to instrument data for each channel
	u8 const *m_opinst[OPERATORS];   // pointer to instrument data for each operator
	u8 m_regdata[REGISTERS];         // register data
	u8 m_instdata[INSTDATA_SIZE];    // instrument data
	u16 m_waveform[WAVEFORMS][WAVEFORM_LENGTH]; // waveforms
};


//*********************************************************
//  CORE ENGINE CLASSES
//*********************************************************

// forward declarations
template<class RegisterType> class ymfm_engine_base;

// three different keyon sources; actual keyon is an OR over all of these
enum ymfm_keyon_type : u32
{
	YMFM_KEYON_NORMAL = 0,
	YMFM_KEYON_RHYTHM = 1,
	YMFM_KEYON_CSM = 2
};


// ======================> ymfm_operator

// ymfm_operator represents an FM operator (or "slot" in FM parlance), which
// produces an output sine wave modulated by an envelope
template<class RegisterType>
class ymfm_operator
{
	// "quiet" value, used to optimize when we can skip doing working
	static constexpr u32 ENV_QUIET = 0x200;

public:
	// constructor
	ymfm_operator(ymfm_engine_base<RegisterType> &owner, u32 opoffs);

	// register for save states
	void save(device_t &device, u32 index);

	// reset the operator state
	void reset();

	// set the current channel
	void set_choffs(u32 choffs) { m_choffs = choffs; }

	// prepare prior to clocking
	bool prepare();

	// master clocking function
	void clock(u32 env_counter, s32 lfo_raw_pm);

	// return the current phase value
	u32 phase() const { return m_phase >> 10; }

	// compute operator volume
	s32 compute_volume(u32 phase, u32 am_offset) const;

	// compute volume for the OPM noise channel
	s32 compute_noise_volume(u32 am_offset) const;

	// key state control
	void keyonoff(u32 on, ymfm_keyon_type type);

	// return a reference to our registers
	RegisterType &regs() { return m_regs; }

private:
	// start the attack phase
	void start_attack();

	// start the release phase
	void start_release();

	// clock phases
	void clock_keystate(u32 keystate);
	void clock_ssg_eg_state();
	void clock_envelope(u32 env_counter);
	void clock_phase(s32 lfo_raw_pm);

	// return effective attenuation of the envelope
	u32 envelope_attenuation(u32 am_offset) const;

	// internal state
	u32 m_choffs;                    // channel offset in registers
	u32 m_opoffs;                    // operator offset in registers
	u32 m_phase;                     // current phase value (10.10 format)
	u16 m_env_attenuation;           // computed envelope attenuation (4.6 format)
	ymfm_envelope_state m_env_state; // current envelope state
	u8 m_ssg_inverted;               // non-zero if the output should be inverted (bit 0)
	u8 m_key_state;                  // current key state: on or off (bit 0)
	u8 m_keyon_live;                 // live key on state (bit 0 = direct, bit 1 = rhythm, bit 2 = CSM)
	ymfm_opdata_cache m_cache;       // cached values for performance
	RegisterType &m_regs;            // direct reference to registers
	ymfm_engine_base<RegisterType> &m_owner; // reference to the owning engine
};


// ======================> ymfm_channel

// ymfm_channel represents an FM channel which combines the output of 2 or 4
// operators into a final result
template<class RegisterType>
class ymfm_channel
{
public:
	// constructor
	ymfm_channel(ymfm_engine_base<RegisterType> &owner, u32 choffs);

	// register for save states
	void save(device_t &device, u32 index);

	// reset the channel state
	void reset();

	// assign operators
	void assign(int index, ymfm_operator<RegisterType> *op)
	{
		assert(index < std::size(m_op));
		m_op[index] = op;
		if (op != nullptr)
			op->set_choffs(m_choffs);
	}

	// signal key on/off to our operators
	void keyonoff(u32 states, ymfm_keyon_type type);

	// prepare prior to clocking
	bool prepare();

	// master clocking function
	void clock(u32 env_counter, s32 lfo_raw_pm);

	// specific 2-operator and 4-operator output handlers
	void output_2op(s32 outputs[RegisterType::OUTPUTS], u32 rshift, s32 clipmax) const;
	void output_4op(s32 outputs[RegisterType::OUTPUTS], u32 rshift, s32 clipmax) const;

	// compute the special OPL rhythm channel outputs
	void output_rhythm_ch6(s32 outputs[RegisterType::OUTPUTS], u32 rshift, s32 clipmax) const;
	void output_rhythm_ch7(u32 phase_select, s32 outputs[RegisterType::OUTPUTS], u32 rshift, s32 clipmax) const;
	void output_rhythm_ch8(u32 phase_select, s32 outputs[RegisterType::OUTPUTS], u32 rshift, s32 clipmax) const;

	// are we a 4-operator channel or a 2-operator one?
	bool is4op() const
	{
		if (RegisterType::DYNAMIC_OPS)
			return (m_op[2] != nullptr);
		return (RegisterType::OPERATORS / RegisterType::CHANNELS == 4);
	}

	// return a reference to our registers
	RegisterType &regs() { return m_regs; }

private:
	// helper to add values to the outputs based on channel enables
	void add_to_output(u32 choffs, s32 *outputs, s32 value) const
	{
		if (RegisterType::OUTPUTS == 1 || m_regs.ch_output_0(choffs))
			outputs[0] += value;
		if (RegisterType::OUTPUTS >= 2 && m_regs.ch_output_1(choffs))
			outputs[1] += value;
		if (RegisterType::OUTPUTS >= 3 && m_regs.ch_output_2(choffs))
			outputs[2] += value;
		if (RegisterType::OUTPUTS >= 4 && m_regs.ch_output_3(choffs))
			outputs[3] += value;
	}

	// internal state
	u32 m_choffs;                         // channel offset in registers
	s16 m_feedback[2];                    // feedback memory for operator 1
	mutable s16 m_feedback_in;            // next input value for op 1 feedback (set in output)
	ymfm_operator<RegisterType> *m_op[4]; // up to 4 operators
	RegisterType &m_regs;                 // direct reference to registers
	ymfm_engine_base<RegisterType> &m_owner; // reference to the owning engine
};


// ======================> ymfm_engine_base

// ymfm_engine_base represents a set of operators and channels which together
// form a Yamaha FM core; chips that implement other engines (ADPCM, wavetable,
// etc) take this output and combine it with the others externally
template<class RegisterType>
class ymfm_engine_base
{
public:
	// expose some constants from the registers
	static constexpr u32 OUTPUTS = RegisterType::OUTPUTS;
	static constexpr u32 CHANNELS = RegisterType::CHANNELS;
	static constexpr u32 ALL_CHANNELS = RegisterType::ALL_CHANNELS;
	static constexpr u32 OPERATORS = RegisterType::OPERATORS;

	// also expose status flags for consumers that inject additional bits
	static constexpr u8 STATUS_TIMERA = RegisterType::STATUS_TIMERA;
	static constexpr u8 STATUS_TIMERB = RegisterType::STATUS_TIMERB;
	static constexpr u8 STATUS_BUSY = RegisterType::STATUS_BUSY;
	static constexpr u8 STATUS_IRQ = RegisterType::STATUS_IRQ;

	// constructor
	ymfm_engine_base(device_t &device);

	// configuration helpers
	auto irq_handler() { return m_irq_handler.bind(); }

	// register for save states
	void save(device_t &device);

	// reset the overall state
	void reset();

	// master clocking function
	u32 clock(u32 chanmask);

	// compute sum of channel outputs
	void output(s32 outputs[RegisterType::OUTPUTS], u32 rshift, s32 clipmax, u32 chanmask) const;

	// write to the OPN registers
	void write(u16 regnum, u8 data);

	// return the current status
	u8 status() const;

	// set/reset bits in the status register, updating the IRQ status
	u8 set_reset_status(u8 set, u8 reset)
	{
		m_status = (m_status | set) & ~reset;
		schedule_check_interrupts();
		return m_status;
	}

	// set the IRQ mask
	void set_irq_mask(u8 mask) { m_irq_mask = mask; schedule_check_interrupts(); }

	// helper to compute the busy duration
	attotime compute_busy_duration(u32 cycles = 32)
	{
		return attotime::from_hz(m_device.clock()) * (cycles * m_clock_prescale);
	}

	// set the time when the busy flag in the status register should be cleared
	void set_busy_end(attotime end) { m_busy_end = end; }

	// return the current clock prescale
	u32 clock_prescale() const { return m_clock_prescale; }

	// set prescale factor (2/3/6)
	void set_clock_prescale(u32 prescale) { m_clock_prescale = prescale; }

	// compute sample rate
	u32 sample_rate(u32 baseclock) const { return baseclock / (m_clock_prescale * OPERATORS); }

	// reset the LFO state
	void reset_lfo() { m_regs.reset_lfo(); }

	// return the owning device
	device_t &device() const { return m_device; }

	// return a reference to our registers
	RegisterType &regs() { return m_regs; }

protected:
	// assign the current set of operators to channels
	void assign_operators();

	// update the state of the given timer
	void update_timer(u32 which, u32 enable);

	// timer callback
	TIMER_CALLBACK_MEMBER(timer_handler);

	// schedule an interrupt check
	void schedule_check_interrupts();

	// check interrupts
	TIMER_CALLBACK_MEMBER(check_interrupts);

	// handle a mode register write
	TIMER_CALLBACK_MEMBER(synced_mode_w);

	// internal state
	device_t &m_device;              // reference to the owning device
	u32 m_env_counter;               // envelope counter; low 2 bits are sub-counter
	u8 m_status;                     // current status register
	u8 m_clock_prescale;             // prescale factor (2/3/6)
	u8 m_irq_mask;                   // mask of which bits signal IRQs
	u8 m_irq_state;                  // current IRQ state
	u32 m_active_channels;           // mask of active channels (computed by prepare)
	u32 m_modified_channels;         // mask of channels that have been modified
	u32 m_prepare_count;             // counter to do periodic prepare sweeps
	attotime m_busy_end;             // end of the busy time
	emu_timer *m_timer[2];           // our two timers
	devcb_write_line m_irq_handler;  // IRQ callback
	RegisterType m_regs;             // register accessor
	std::unique_ptr<ymfm_channel<RegisterType>> m_channel[CHANNELS]; // channel pointers
	std::unique_ptr<ymfm_operator<RegisterType>> m_operator[OPERATORS]; // operator pointers
};


// ======================> template instantiations

extern template class ymfm_engine_base<ymopm_registers>;
extern template class ymfm_engine_base<ymopn_registers>;
extern template class ymfm_engine_base<ymopna_registers>;
extern template class ymfm_engine_base<ymopl_registers>;
extern template class ymfm_engine_base<ymopl2_registers>;
extern template class ymfm_engine_base<ymopl3_registers>;

using ymopm_engine = ymfm_engine_base<ymopm_registers>;
using ymopn_engine = ymfm_engine_base<ymopn_registers>;
using ymopna_engine = ymfm_engine_base<ymopna_registers>;
using ymopl_engine = ymfm_engine_base<ymopl_registers>;
using ymopl2_engine = ymfm_engine_base<ymopl2_registers>;
using ymopl3_engine = ymfm_engine_base<ymopl3_registers>;
using ymopl4_engine = ymfm_engine_base<ymopl4_registers>;


// ======================> ymopll_engine

// ymopll_engine is a special case because instrument data needs to be
// provided from an external source
class ymopll_engine : public ymfm_engine_base<ymopll_registers>
{
public:
	// constructor
	ymopll_engine(device_t &device) :
		ymfm_engine_base(device)
	{
	}

	// set the instrument data
	void set_instrument_data(u8 const *data)
	{
		m_regs.set_instrument_data(data);
	}
};


#endif // MAME_SOUND_YMFM_H
