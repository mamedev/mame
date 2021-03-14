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

enum ymfm_envelope_state : u8
{
	YMFM_ENV_ATTACK = 0,
	YMFM_ENV_DECAY = 1,
	YMFM_ENV_SUSTAIN = 2,
	YMFM_ENV_RELEASE = 3,
	YMFM_ENV_DEPRESS = 4
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

struct ymfm_opdata_cache
{
	u32 phase_step;		// phase step, or 0 if PM is active
	u16 block_freq;		// block frequency value
	u16 total_level;	// total level, shifted up by 3 bits
	s8 detune;			// detuning value
	u8 eg_sustain;		// sustain level
	u8 eg_rate[5];		// envelope rate for each step
};


// ======================> ymfm_registers_base

class ymfm_registers_base
{
public:
	// family type
	enum family_type : u8
	{
		FAMILY_OPM,
		FAMILY_OPN,
		FAMILY_OPL
	};

	// this value is returned from the is_keyon() function for rhythm channels
	static constexpr u8 YMFM_RHYTHM_CHANNEL = 0xff;

	//
	// the following constants need to be defined per family:
	//   family_type FAMILY: One of the family_types above
	//           u8 OUTPUTS: The number of outputs exposed (1-4)
	//          u8 CHANNELS: The number of channels on the chip
	//     u32 ALL_CHANNELS: A bitmask of all channels
	//         u8 OPERATORS: The number of operators on the chip
	//     bool DYNAMIC_OPS: True if ops/channel can be changed at runtime
	//        u16 REGISTERS: The number of 8-bit registers allocated
	//         u16 REG_MODE: The address of the "mode" register controlling timers
	//  u8 DEFAULT_PRESCALE: The starting clock prescale
	//  u8 EG_CLOCK_DIVIDER: The clock divider of the envelope generator
	//  bool EG_HAS_DEPRESS: True if the chip has a DP ("depress"?) envelope stage
	// bool MODULATOR_DELAY: True if the modulator is delayed by 1 sample (OPL pre-OPL3)
	// u32 CSM_TRIGGER_MASK: Mask of channels to trigger in CSM mode
	//     u8 STATUS_TIMERA: Status bit to set when timer A fires
	//     u8 STATUS_TIMERB: Status bit to set when tiemr B fires
	//       u8 STATUS_BUSY: Status bit to set when the chip is busy
	//        u8 STATUS_IRQ: Status bit to set when an IRQ is signalled
	//

	// system-wide register defaults
	u8 status_mask() const        /*  8 bits */ { return 0; } // OPL only
	u8 irq_reset() const          /*  8 bits */ { return 0; } // OPL only
	u8 noise_frequency() const    /*  5 bits */ { return 0; } // OPM only
	u8 noise_enabled() const      /*  1 bit  */ { return 0; } // OPM only
	u8 lfo_enabled() const        /*  1 bit  */ { return 1; } // OPN(A) only
	u8 lfo_rate() const           /*3-8 bits */ { return 0; } // OPM,OPNA
	u8 lfo_waveform() const       /*  2 bits */ { return 0; } // OPM only
	u8 lfo_pm_depth() const       /*  7 bits */ { return 0; } // OPM,OPL only
	u8 lfo_am_depth() const       /*  7 bits */ { return 0; } // OPM,OPL only
	u8 multi_freq() const         /*  1 bit  */ { return 0; } // OPN(A) only
	u8 rhythm_enable() const      /*  1 bit  */ { return 0; } // OPL only
	u8 waveform_enable() const    /*  1 bits */ { return 0; } // OPL2+ only

	// per-channel register defaults
	u8 ch_lfo_am_sens(u16 choffs) const /*  2 bits */ { return 0; } // OPM,OPNA only

	// per-operator register defaults
	u8 op_ssg_eg_enabled(u16 opoffs) const     /*  1 bit  */ { return 0; } // OPN(A) only
	u8 op_ssg_eg_mode(u16 opoffs) const        /*  1 bit  */ { return 0; } // OPN(A) only
	u8 op_waveform(u16 opoffs) const           /*  3 bits */ { return 0; } // OPL2+ only

protected:
	static constexpr u8 effective_rate(u8 rawrate, u8 ksr)
	{
		return (rawrate == 0) ? 0 : std::min(rawrate + ksr, 63);
	}

	// internal state
	u8 *m_regdata;                 // pointer to the raw data
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
public:
	// constants
	static constexpr family_type FAMILY = FAMILY_OPM;
	static constexpr u8 OUTPUTS = 2;
	static constexpr u8 CHANNELS = 8;
	static constexpr u32 ALL_CHANNELS = (1 << CHANNELS) - 1;
	static constexpr u8 OPERATORS = CHANNELS * 4;
	static constexpr u8 DYNAMIC_OPS = false;
	static constexpr u16 REGISTERS = 0x100;
	static constexpr u16 REG_MODE = 0x14;
	static constexpr u8 DEFAULT_PRESCALE = 2;
	static constexpr u8 EG_CLOCK_DIVIDER = 3;
	static constexpr bool EG_HAS_DEPRESS = false;
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
	static constexpr u16 channel_offset(u8 chnum)
	{
		assert(chnum < CHANNELS);
		return chnum;
	}

	// map operator number to register offset
	static constexpr u16 operator_offset(u8 opnum)
	{
		assert(opnum < OPERATORS);
		return opnum;
	}

	// return an array of operator indices for each channel
	struct operator_mapping { u32 chan[CHANNELS]; };
	void operator_map(operator_mapping &dest) const;

	// handle writes to the register array
	bool write(u16 index, u8 data, u8 &chan, u8 &opmask);

	// compute the phase step, given a PM value
	u32 compute_phase_step(u16 choffs, u16 opoffs, u16 block_freq, s8 detuneval, s8 lfo_raw_pm);

	// caching helpers
	void cache_operator_data(u16 choffs, u16 opoffs, ymfm_opdata_cache &cache);

	// clock the noise and LFO, if present, returning LFO PM value
	s8 clock_noise_and_lfo();

	// reset the LFO
	void reset_lfo() { m_lfo_counter = 0; }

	// return the AM offset from LFO for the given channel
	u16 lfo_am_offset(u16 choffs) const;

	// return the current noise state, gated by the noise clock
	u8 noise_state() const { return m_noise_state; }

	// log a key-on event
	void log_keyon(u16 choffs, u16 opoffs);

	// system-wide registers
	u8 test() const                        { return byte(0x01, 0, 8); }
	u8 noise_frequency() const             { return byte(0x0f, 0, 5); }
	u8 noise_enabled() const               { return byte(0x0f, 7, 1); }
	u16 timer_a_value() const              { return word(0x10, 0, 8, 0x11, 0, 2); }
	u8 timer_b_value() const               { return byte(0x12, 0, 8); }
	u8 csm() const                         { return byte(0x14, 7, 1); }
	u8 reset_timer_b() const               { return byte(0x14, 5, 1); }
	u8 reset_timer_a() const               { return byte(0x14, 4, 1); }
	u8 enable_timer_b() const              { return byte(0x14, 3, 1); }
	u8 enable_timer_a() const              { return byte(0x14, 2, 1); }
	u8 load_timer_b() const                { return byte(0x14, 1, 1); }
	u8 load_timer_a() const                { return byte(0x14, 0, 1); }
	u8 lfo_rate() const                    { return byte(0x18, 0, 8); }
	u8 lfo_am_depth() const                { return byte(0x19, 0, 7); }
	u8 lfo_pm_depth() const                { return byte(0x1a, 0, 7); }
	u8 lfo_waveform() const                { return byte(0x1b, 0, 2); }

	// per-channel registers
	u8 ch_output_mask(u16 choffs) const    { return byte(0x20, 6, 2, choffs); }
	u8 ch_feedback(u16 choffs) const       { return byte(0x20, 3, 3, choffs); }
	u8 ch_algorithm(u16 choffs) const      { return byte(0x20, 0, 3, choffs); }
	u16 ch_block_freq(u16 choffs) const    { return word(0x28, 0, 7, 0x30, 2, 6, choffs); }
	u8 ch_lfo_pm_sens(u16 choffs) const    { return byte(0x38, 4, 3, choffs); }
	u8 ch_lfo_am_sens(u16 choffs) const    { return byte(0x38, 0, 2, choffs); }

	// per-operator registers
	u8 op_detune(u16 opoffs) const         { return byte(0x40, 4, 3, opoffs); }
	u8 op_multiple(u16 opoffs) const       { return byte(0x40, 0, 4, opoffs); }
	u8 op_total_level(u16 opoffs) const    { return byte(0x60, 0, 7, opoffs); }
	u8 op_ksr(u16 opoffs) const            { return byte(0x80, 6, 2, opoffs); }
	u8 op_attack_rate(u16 opoffs) const    { return byte(0x80, 0, 5, opoffs); }
	u8 op_lfo_am_enabled(u16 opoffs) const { return byte(0xa0, 7, 1, opoffs); }
	u8 op_decay_rate(u16 opoffs) const     { return byte(0xa0, 0, 5, opoffs); }
	u8 op_detune2(u16 opoffs) const        { return byte(0xc0, 6, 2, opoffs); }
	u8 op_sustain_rate(u16 opoffs) const   { return byte(0xc0, 0, 5, opoffs); }
	u8 op_sustain_level(u16 opoffs) const  { return byte(0xe0, 4, 4, opoffs); }
	u8 op_release_rate(u16 opoffs) const   { return byte(0xe0, 0, 4, opoffs); }

protected:
	// return a bitfield extracted from a byte
	u8 byte(u16 offset, u8 start, u8 count, u16 extra_offset = 0) const
	{
		return BIT(m_regdata[offset + extra_offset], start, count);
	}

	// return a bitfield extracted from a pair of bytes, MSBs listed first
	u16 word(u16 offset1, u8 start1, u8 count1, u16 offset2, u8 start2, u8 count2, u16 extra_offset = 0) const
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
	static constexpr family_type FAMILY = FAMILY_OPN;
	static constexpr u8 OUTPUTS = IsOpnA ? 2 : 1;
	static constexpr u8 CHANNELS = IsOpnA ? 6 : 3;
	static constexpr u32 ALL_CHANNELS = (1 << CHANNELS) - 1;
	static constexpr u8 OPERATORS = CHANNELS * 4;
	static constexpr u8 DYNAMIC_OPS = false;
	static constexpr u16 REGISTERS = IsOpnA ? 0x200 : 0x100;
	static constexpr u16 REG_MODE = 0x27;
	static constexpr u8 DEFAULT_PRESCALE = 6;
	static constexpr u8 EG_CLOCK_DIVIDER = 3;
	static constexpr bool EG_HAS_DEPRESS = false;
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
	static constexpr u16 channel_offset(u8 chnum)
	{
		assert(chnum < CHANNELS);
		if (!IsOpnA)
			return chnum;
		else
			return (chnum % 3) + 0x100 * (chnum / 3);
	}

	// map operator number to register offset
	static constexpr u16 operator_offset(u8 opnum)
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
	bool write(u16 index, u8 data, u8 &chan, u8 &opmask);

	// compute the phase step, given a PM value
	u32 compute_phase_step(u16 choffs, u16 opoffs, u16 block_freq, s8 detuneval, s8 lfo_raw_pm);

	// caching helpers
	void cache_operator_data(u16 choffs, u16 opoffs, ymfm_opdata_cache &cache);

	// clock the noise and LFO, if present, returning LFO PM value
	s8 clock_noise_and_lfo();

	// reset the LFO
	void reset_lfo() { m_lfo_counter = 0; }

	// return the AM offset from LFO for the given channel
	u16 lfo_am_offset(u16 choffs) const;

	// return LFO/noise states
	u8 noise_state() const { return 0; }

	// log a key-on event
	void log_keyon(u16 choffs, u16 opoffs);

	// system-wide registers
	u8 test() const                        { return byte(0x21, 0, 8); }
	u8 lfo_enabled() const                 { return IsOpnA ? byte(0x22, 3, 1) : 0; }
	u8 lfo_rate() const                    { return IsOpnA ? byte(0x22, 0, 3) : 0; }
	u16 timer_a_value() const              { return word(0x24, 0, 8, 0x25, 0, 2); }
	u8 timer_b_value() const               { return byte(0x26, 0, 8); }
	u8 csm() const                         { return (byte(0x27, 6, 2) == 2); }
	u8 multi_freq() const                  { return (byte(0x27, 6, 2) != 0); }
	u8 reset_timer_b() const               { return byte(0x27, 5, 1); }
	u8 reset_timer_a() const               { return byte(0x27, 4, 1); }
	u8 enable_timer_b() const              { return byte(0x27, 3, 1); }
	u8 enable_timer_a() const              { return byte(0x27, 2, 1); }
	u8 load_timer_b() const                { return byte(0x27, 1, 1); }
	u8 load_timer_a() const                { return byte(0x27, 0, 1); }
	u16 multi_block_freq(u8 num) const     { return word(0xac, 0, 6, 0xa8, 0, 8, num); }

	// per-channel registers
	u16 ch_block_freq(u16 choffs) const    { return word(0xa4, 0, 6, 0xa0, 0, 8, choffs); }
	u8 ch_feedback(u16 choffs) const       { return byte(0xb0, 3, 3, choffs); }
	u8 ch_algorithm(u16 choffs) const      { return byte(0xb0, 0, 3, choffs); }
	u8 ch_output_mask(u16 choffs) const    { return IsOpnA ? bitswap<2>(byte(0xb4, 6, 2, choffs), 0, 1) : 1; }
	u8 ch_lfo_am_sens(u16 choffs) const    { return IsOpnA ? byte(0xb4, 4, 2, choffs) : 0; }
	u8 ch_lfo_pm_sens(u16 choffs) const    { return IsOpnA ? byte(0xb4, 0, 3, choffs) : 0; }

	// per-operator registers
	u8 op_detune(u16 opoffs) const         { return byte(0x30, 4, 3, opoffs); }
	u8 op_multiple(u16 opoffs) const       { return byte(0x30, 0, 4, opoffs); }
	u8 op_total_level(u16 opoffs) const    { return byte(0x40, 0, 7, opoffs); }
	u8 op_ksr(u16 opoffs) const            { return byte(0x50, 6, 2, opoffs); }
	u8 op_attack_rate(u16 opoffs) const    { return byte(0x50, 0, 5, opoffs); }
	u8 op_decay_rate(u16 opoffs) const     { return byte(0x60, 0, 5, opoffs); }
	u8 op_lfo_am_enabled(u16 opoffs) const { return IsOpnA ? byte(0x60, 7, 1, opoffs) : 0; }
	u8 op_sustain_rate(u16 opoffs) const   { return byte(0x70, 0, 5, opoffs); }
	u8 op_sustain_level(u16 opoffs) const  { return byte(0x80, 4, 4, opoffs); }
	u8 op_release_rate(u16 opoffs) const   { return byte(0x80, 0, 4, opoffs); }
	u8 op_ssg_eg_enabled(u16 opoffs) const { return byte(0x90, 3, 1, opoffs); }
	u8 op_ssg_eg_mode(u16 opoffs) const    { return byte(0x90, 0, 3, opoffs); }

protected:
	// return a bitfield extracted from a byte
	u8 byte(u16 offset, u8 start, u8 count, u16 extra_offset = 0) const
	{
		return BIT(m_regdata[offset + extra_offset], start, count);
	}

	// return a bitfield extracted from a pair of bytes, MSBs listed first
	u16 word(u16 offset1, u8 start1, u8 count1, u16 offset2, u8 start2, u8 count2, u16 extra_offset = 0) const
	{
		return (byte(offset1, start1, count1, extra_offset) << count2) | byte(offset2, start2, count2, extra_offset);
	}

	// internal state
	u32 m_lfo_counter;               // LFO counter
	u8 m_lfo_am;                     // current LFO AM value
	u8 m_regdata[REGISTERS];         // register data
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
//     Per-channel registers (channel in address bits 0-3)
//     Note that all these apply to address+100 as well on OPL3+
//        A0-A8 xxxxxxxx F-number (low 8 bits)
//        B0-B8 --x----- Key on
//              ---xxx-- Block (octvate, 0-7)
//              ------xx F-number (high two bits)
//        C0-C8 ----xxx- Feedback level for operator 1 (0-7)
//              -------x Operator connection algorithm
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
	static constexpr family_type FAMILY = FAMILY_OPL;
	static constexpr u8 OUTPUTS = IsOpl3Plus ? 4 : 1;
	static constexpr u8 CHANNELS = IsOpl3Plus ? 18 : 9;
	static constexpr u32 ALL_CHANNELS = (1 << CHANNELS) - 1;
	static constexpr u8 OPERATORS = CHANNELS * 2;
	static constexpr u8 DYNAMIC_OPS = false;
	static constexpr u16 REGISTERS = IsOpl3Plus ? 0x200 : 0x100;
	static constexpr u16 REG_MODE = 0x04;
	static constexpr u8 DEFAULT_PRESCALE = IsOpl3Plus ? 8 : 4;
	static constexpr u8 EG_CLOCK_DIVIDER = 1;
	static constexpr bool EG_HAS_DEPRESS = false;
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
	static constexpr u16 channel_offset(u8 chnum)
	{
		assert(chnum < CHANNELS);
		if (!IsOpl3Plus)
			return chnum;
		else
			return (chnum % 9) + 0x100 * (chnum / 9);
	}

	// map operator number to register offset
	static constexpr u16 operator_offset(u8 opnum)
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

	// handle writes to the register array
	bool write(u16 index, u8 data, u8 &chan, u8 &opmask);

	// compute the phase step, given a PM value
	u32 compute_phase_step(u16 choffs, u16 opoffs, u16 block_freq, s8 detuneval, s8 lfo_raw_pm);

	// caching helpers
	void cache_operator_data(u16 choffs, u16 opoffs, ymfm_opdata_cache &cache);

	// clock the noise and LFO, if present, returning LFO PM value
	s8 clock_noise_and_lfo();

	// reset the LFO
	void reset_lfo() { m_lfo_am_counter = m_lfo_pm_counter = 0; }

	// return the AM offset from LFO for the given channel
	// on OPL this is just a fixed value
	u16 lfo_am_offset(u16 choffs) const { return m_lfo_am; }

	// return LFO/noise states
	u8 noise_state() const { return m_noise_lfsr >> 23; }

	// log a key-on event
	void log_keyon(u16 choffs, u16 opoffs);

	// helper to determine if the this channel is an active rhythm channel
	bool is_rhythm(u16 choffs) const
	{
		return rhythm_enable() && (choffs >= 6 && choffs <= 8);
	}

	// OPL-specific helper to handle the weird multiple mapping
	static constexpr u8 opl_multiple_map(u8 raw)
	{
		// replace the low bit with a table lookup; the equivalent
		// OPM/OPN values are: 0,1,2,3,4,5,6,7,8,9,10,10,12,12,15,15
		return (raw & 0xe) | BIT(0xc2aa, raw);
	}

	// system-wide registers
	u8 test() const               /*  8 bits */ { return byte(0x01, 0, 8); }
	u8 waveform_enable() const    /*  1 bits */ { return IsOpl2 ? byte(0x01, 5, 1) : (IsOpl3Plus ? 1 : 0); }
	u16 timer_a_value() const     /*  8 bits */ { return byte(0x02, 0, 8) * 4; } // 8->10 bits
	u8 timer_b_value() const      /*  8 bits */ { return byte(0x03, 0, 8); }
	u8 status_mask() const        /*  8 bits */ { return byte(0x04, 0, 8) & 0x78; }
	u8 irq_reset() const          /*  1 bit  */ { return byte(0x04, 7, 1); }
	u8 reset_timer_b() const      /*  1 bit  */ { return byte(0x04, 7, 1) | byte(0x04, 5, 1); }
	u8 reset_timer_a() const      /*  1 bit  */ { return byte(0x04, 7, 1) | byte(0x04, 6, 1); }
	u8 enable_timer_b() const     /*  1 bit  */ { return byte(0x04, 5, 1) ^ 1; }
	u8 enable_timer_a() const     /*  1 bit  */ { return byte(0x04, 6, 1) ^ 1; }
	u8 load_timer_b() const       /*  1 bit  */ { return byte(0x04, 1, 1); }
	u8 load_timer_a() const       /*  1 bit  */ { return byte(0x04, 0, 1); }
	u8 csm() const                /*  1 bit  */ { return IsOpl3Plus ? 0 : byte(0x08, 7, 1); }
	u8 note_select() const        /*  1 bit  */ { return byte(0x08, 6, 1); }
	u8 lfo_am_depth() const       /*  1 bit  */ { return byte(0xbd, 7, 1) * 2; } // 1->2 bits
	u8 lfo_pm_depth() const       /*  1 bit  */ { return byte(0xbd, 6, 1); }
	u8 rhythm_enable() const      /*  1 bit  */ { return byte(0xbd, 5, 1); }
	u8 rhythm_keyon() const       /*  5 bits */ { return byte(0xbd, 4, 0); }
	u8 newflag() const            /*  1 bit  */ { return IsOpl3Plus ? byte(0x105, 0, 1) : 0; }
	u8 fourop_enable() const      /*  6 bits */ { return IsOpl3Plus ? byte(0x104, 0, 6) : 0; }

	// per-channel registers
	u16 ch_block_freq(u16 choffs) const        /* 13 bits */ { return word(0xb0, 0, 5, 0xa0, 0, 8, choffs) * 2; } // 13->14 bits
	u8 ch_feedback(u16 choffs) const           /*  3 bits */ { return byte(0xc0, 1, 3, choffs); }
	u8 ch_algorithm(u16 choffs) const          /*  1 bit  */ { return byte(0xc0, 0, 1, choffs) | (IsOpl3Plus ? (8 | (byte(0xc3, 0, 1, choffs) << 1)) : 0); }
	u8 ch_output_mask(u16 choffs) const        /*  4 bits */ { return IsOpl3Plus ? bitswap<4>(byte(0xc0 + choffs, 5, 1), 3,2,0,1) : 1; }

	// per-operator registers
	u8 op_lfo_am_enabled(u16 opoffs) const   { return byte(0x20, 7, 1, opoffs); }
	u8 op_lfo_pm_enabled(u16 opoffs) const   { return byte(0x20, 6, 1, opoffs); }
	u8 op_eg_sustain(u16 opoffs) const       { return byte(0x20, 5, 1, opoffs); }
	u8 op_ksr(u16 opoffs) const              { return byte(0x20, 4, 1, opoffs) * 2 + 1; } // 1->2 bits
	u8 op_multiple(u16 opoffs) const         { return opl_multiple_map(byte(0x20, 0, 4, opoffs)); }
	u8 op_key_scale_level(u16 opoffs) const  { return bitswap<2>(byte(0x40, 6, 2, opoffs), 0, 1); }
	u8 op_total_level(u16 opoffs) const      { return byte(0x40, 0, 6, opoffs); }
	u8 op_attack_rate(u16 opoffs) const      { return byte(0x60, 4, 4, opoffs); }
	u8 op_decay_rate(u16 opoffs) const       { return byte(0x60, 0, 4, opoffs); }
	u8 op_sustain_level(u16 opoffs) const    { return byte(0x80, 4, 4, opoffs); }
	u8 op_release_rate(u16 opoffs) const     { return byte(0x80, 0, 4, opoffs); }
	u8 op_waveform(u16 opoffs) const         { return IsOpl2Plus ? byte(0xe0, 0, IsOpl3Plus ? 3 : 2, opoffs) : 0; }

protected:
	// return a bitfield extracted from a byte
	u8 byte(u16 offset, u8 start, u8 count, u16 extra_offset = 0) const
	{
		return BIT(m_regdata[offset + extra_offset], start, count);
	}

	// return a bitfield extracted from a pair of bytes, MSBs listed first
	u16 word(u16 offset1, u8 start1, u8 count1, u16 offset2, u8 start2, u8 count2, u16 extra_offset = 0) const
	{
		return (byte(offset1, start1, count1, extra_offset) << count2) | byte(offset2, start2, count2, extra_offset);
	}

	// internal state
	u16 m_lfo_am_counter;            // LFO AM counter
	u16 m_lfo_pm_counter;            // LFO PM counter
	u32 m_noise_lfsr;                // noise LFSR state
	u8 m_lfo_am;                     // current LFO AM value
	u8 m_regdata[REGISTERS];         // register data
};

using ymopl_registers = ymopl_registers_base<1>;
using ymopl2_registers = ymopl_registers_base<2>;
using ymopl3_registers = ymopl_registers_base<3>;


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

class ymopll_registers : public ymopl_registers
{
public:
	static constexpr u8 OUTPUTS = 2;
	static constexpr u16 REG_MODE = 0xff;
	static constexpr bool EG_HAS_DEPRESS = true;
	static constexpr u8 STATUS_TIMERA = 0;
	static constexpr u8 STATUS_TIMERB = 0;
	static constexpr u8 STATUS_BUSY = 0;
	static constexpr u8 STATUS_IRQ = 0;

	// OPLL-specific constants
	static constexpr u16 EXTERNAL_REGISTERS = 0x40;
	static constexpr u16 CHANNEL_INSTBASE = 0x40;
	static constexpr u16 OPERATOR_INSTBASE = 0x4e;
	static constexpr u16 INSTDATA_BASE = 0x70;
	static constexpr u16 INSTDATA_SIZE = 0x90;

	// constructor
	ymopll_registers();

	// reset to initial state
	void reset();

	// map channel number to register offset
	static constexpr u16 channel_offset(u8 chnum)
	{
		assert(chnum < CHANNELS);
		return chnum;
	}

	// map operator number to register offset
	static constexpr u16 operator_offset(u8 opnum)
	{
		assert(opnum < OPERATORS);
		return opnum;
	}

	// return an array of operator indices for each channel
	struct operator_mapping { u32 chan[CHANNELS]; };
	void operator_map(operator_mapping &dest) const;

	// handle writes to the register array
	bool write(u16 index, u8 data, u8 &chan, u8 &opmask);

	// compute the phase step, given a PM value
	u32 compute_phase_step(u16 choffs, u16 opoffs, u16 block_freq, s8 detuneval, s8 lfo_raw_pm);

	// caching helpers
	void cache_operator_data(u16 choffs, u16 opoffs, ymfm_opdata_cache &cache);

	// clock the noise and LFO, if present, returning LFO PM value
	s8 clock_noise_and_lfo();

	// log a key-on event
	void log_keyon(u16 choffs, u16 opoffs);

	// helper to determine if the this channel is an active rhythm channel
	bool is_rhythm(u16 choffs) const
	{
		return rhythm_enable() && (choffs >= 6 && choffs <= 8);
	}

	// system-wide registers
	u8 rhythm_enable() const                 { return byte(0x0e, 5, 1); }
	u8 rhythm_keyon() const                  { return byte(0x0e, 4, 0); }
	u8 test() const                          { return byte(0x0f, 0, 8); }
	u8 waveform_enable() const               { return 1; }

	// per-channel registers
	u16 ch_block_freq(u16 choffs) const      { return word(0x20, 0, 4, 0x10, 0, 8, choffs) * 4; } // 12->14 bits
	u8 ch_sustain(u16 choffs) const          { return byte(0x20, 5, 1, choffs); }
	u8 ch_feedback(u16 choffs) const         { return instchbyte(0x03, 0, 3, choffs); }
	u8 ch_algorithm(u16 choffs) const        { return 0; }
	u8 ch_instrument(u16 choffs) const       { return byte(0x30, 4, 4, choffs); }
	u8 ch_output_mask(u16 choffs) const      { return 1 << is_rhythm(choffs); }

	// per-operator registers
	u8 op_lfo_am_enabled(u16 opoffs) const   { return instopbyte(0x00, 7, 1, opoffs); }
	u8 op_lfo_pm_enabled(u16 opoffs) const   { return instopbyte(0x00, 6, 1, opoffs); }
	u8 op_eg_sustain(u16 opoffs) const       { return instopbyte(0x00, 5, 1, opoffs); }
	u8 op_ksr(u16 opoffs) const              { return instopbyte(0x00, 4, 1, opoffs) * 2 + 1; } // 1->2 bits
	u8 op_multiple(u16 opoffs) const         { return opl_multiple_map(instopbyte(0x00, 0, 4, opoffs)); }
	u8 op_key_scale_level(u16 opoffs) const  { return instopbyte(0x02, 6, 2, opoffs); }
	u8 op_total_level(u16 opoffs) const      { return total_level_or_volume(opoffs); }
	u8 op_waveform(u16 opoffs) const         { return instchbyte(0x03, 3 + BIT(opoffs, 0), 1, opoffs >> 1); }
	u8 op_attack_rate(u16 opoffs) const      { return instopbyte(0x04, 4, 4, opoffs); }
	u8 op_decay_rate(u16 opoffs) const       { return instopbyte(0x04, 0, 4, opoffs); }
	u8 op_sustain_level(u16 opoffs) const    { return instopbyte(0x06, 4, 4, opoffs); }
	u8 op_release_rate(u16 opoffs) const     { return instopbyte(0x06, 0, 4, opoffs); }

	// set the instrument data
	void set_instrument_data(u8 const *data)
	{
		memcpy(&m_regdata[INSTDATA_BASE], data, INSTDATA_SIZE);
	}

private:
	// OPLL-specific helper to return either the total level or the volume
	u8 total_level_or_volume(u16 opoffs) const
	{
		u16 choffs = opoffs >> 1;
		int opindex = BIT(opoffs, 0);
		if (opindex == 1 || is_rhythm(choffs))
			return byte(0x30, 4 * (opindex ^ 1), 4, choffs) * 4;
		else
			return instchbyte(0x02, 0, 6, choffs);
	}

	// helper to compute the ROM address of an instrument number
	static constexpr u8 rom_address(int instrument)
	{
		return (instrument == 0) ? 0 : (INSTDATA_BASE + 8 * (instrument - 1));
	}

	// helper to update the instrument
	void update_instrument(u16 choffs)
	{
		u8 baseaddr;
		if (rhythm_enable() && choffs >= 6)
			baseaddr = rom_address(16 + (choffs - 6));
		else
			baseaddr = rom_address(ch_instrument(choffs));

		m_regdata[CHANNEL_INSTBASE + choffs] = baseaddr;
		m_regdata[OPERATOR_INSTBASE + choffs * 2 + 0] = baseaddr + 0;
		m_regdata[OPERATOR_INSTBASE + choffs * 2 + 1] = baseaddr + 1;
	}

	// helpers to read from instrument channel/operator data
	u8 instchbyte(u16 offset, u8 start, u8 count, u16 choffs) const { return BIT(m_regdata[offset + m_regdata[CHANNEL_INSTBASE + choffs]], start, count); }
	u8 instopbyte(u16 offset, u8 start, u8 count, u16 opoffs) const { return BIT(m_regdata[offset + m_regdata[OPERATOR_INSTBASE + opoffs]], start, count); }
};


//*********************************************************
//  CORE ENGINE CLASSES
//*********************************************************

template<class RegisterType> class ymfm_engine_base;

enum ymfm_keyon_type : u8
{
	YMFM_KEYON_NORMAL = 0,
	YMFM_KEYON_RHYTHM = 1,
	YMFM_KEYON_CSM = 2
};


// ======================> ymfm_operator

template<class RegisterType>
class ymfm_operator
{
public:
	// constructor
	ymfm_operator(ymfm_engine_base<RegisterType> &owner, u8 opnum);

	// register for save states
	void save(device_t &device, u8 index);

	// reset the operator state
	void reset();

	// set the current channel
	void set_chnum(u8 chnum) { m_chnum = chnum; m_choffs = RegisterType::channel_offset(chnum); }

	// prepare prior to clocking
	void prepare();

	// master clocking function
	void clock(u32 env_counter, s8 lfo_raw_pm);

	// return the current phase value
	u16 phase() const { return m_phase >> 10; }

	// compute operator volume
	s16 compute_volume(u16 phase, u16 am_offset) const;

	// compute volume for the OPM noise channel
	s16 compute_noise_volume(u16 am_offset) const;

	// key state control
	void keyonoff(u8 on, ymfm_keyon_type type);

	// return a reference to our registers
	RegisterType &regs() { return m_regs; }

private:
	// return the effective 6-bit ADSR rate after adjustments
	u8 effective_rate(u8 rawrate);

	// start the attack phase
	void start_attack();

	// start the release phase
	void start_release();

	// clock phases
	void clock_keystate(u8 keystate);
	void clock_ssg_eg_state();
	void clock_envelope(u16 env_counter);
	void clock_phase(s8 lfo_raw_pm);

	// return effective attenuation of the envelope
	u16 envelope_attenuation(u8 am_offset) const;

	// internal state
	u8 m_chnum;                      // channel number
	u16 m_choffs;                    // channel offset in registers
	u16 m_opoffs;                    // operator offset in registers
	u32 m_phase;                     // current phase value (10.10 format)
	u16 m_env_attenuation;           // computed envelope attenuation (4.6 format)
	ymfm_envelope_state m_env_state; // current envelope state
	u8 m_ssg_inverted;               // non-zero if the output should be inverted (bit 0)
	u8 m_key_state;                  // current key state: on or off (bit 0)
	u8 m_keyon_live;                 // live key on state (bit 0 = direct, bit 1 = rhythm, bit 2 = CSM)
	ymfm_opdata_cache m_cache;       // cached values for performance
	RegisterType &m_regs;            // operator-specific registers
	ymfm_engine_base<RegisterType> &m_owner; // reference to the owning engine
};


// ======================> ymfm_channel

template<class RegisterType>
class ymfm_channel
{
public:
	// constructor
	ymfm_channel(ymfm_engine_base<RegisterType> &owner, u8 chnum);

	// register for save states
	void save(device_t &device, u8 index);

	// reset the channel state
	void reset();

	// assign operators
	void assign(int index, ymfm_operator<RegisterType> *op)
	{
		assert(index < std::size(m_op));
		m_op[index] = op;
		if (op != nullptr)
			op->set_chnum(m_chnum);
	}

	// signal key on/off to our operators
	void keyonoff(u8 states, ymfm_keyon_type type);

	// prepare prior to clocking
	void prepare();

	// master clocking function
	void clock(u32 env_counter, s8 lfo_raw_pm, bool is_multi_freq);

	// compute the channel output and add to the left/right output sums
	void output(s32 outputs[RegisterType::OUTPUTS], u8 rshift, s32 clipmax) const;

	// specific 2-operator and 4-operator output handlers
	void output_2op(s32 outputs[RegisterType::OUTPUTS], u8 rshift, s32 clipmax) const;
	void output_4op(s32 outputs[RegisterType::OUTPUTS], u8 rshift, s32 clipmax) const;

	// compute the special OPL rhythm channel outputs
	void output_rhythm_ch6(s32 outputs[RegisterType::OUTPUTS], u8 rshift, s32 clipmax) const;
	void output_rhythm_ch7(u8 phase_select, s32 outputs[RegisterType::OUTPUTS], u8 rshift, s32 clipmax) const;
	void output_rhythm_ch8(u8 phase_select, s32 outputs[RegisterType::OUTPUTS], u8 rshift, s32 clipmax) const;

	// return a reference to our registers
	RegisterType &regs() { return m_regs; }

private:
	// helper to add values to the outputs based on channel enables
	void add_to_output(u8 outmask, s32 *outputs, s32 value) const
	{
		if (RegisterType::OUTPUTS == 1 || BIT(outmask, 0))
			outputs[0] += value;
		if (RegisterType::OUTPUTS >= 2 && BIT(outmask, 1))
			outputs[1] += value;
		if (RegisterType::OUTPUTS >= 3 && BIT(outmask, 2))
			outputs[2] += value;
		if (RegisterType::OUTPUTS >= 4 && BIT(outmask, 3))
			outputs[3] += value;
	}

	// internal state
	u8 m_chnum;                           // channel number
	u16 m_choffs;                         // channel offset in registers
	s16 m_feedback[2];                    // feedback memory for operator 1
	mutable s16 m_feedback_in;            // next input value for op 1 feedback (set in output)
	ymfm_operator<RegisterType> *m_op[4]; // up to 4 operators
	RegisterType &m_regs;                 // channel-specific registers
	ymfm_engine_base<RegisterType> &m_owner; // reference to the owning engine
};


// ======================> ymfm_engine_base

template<class RegisterType>
class ymfm_engine_base
{
public:
	// constructor
	ymfm_engine_base(device_t &device);

	// configuration helpers
	auto irq_handler() { return m_irq_handler.bind(); }

	// register for save states
	void save(device_t &device);

	// reset the overall state
	void reset();

	// prepare prior to clocking
	void prepare(u32 chanmask);

	// master clocking function
	u32 clock(u32 chanmask);

	// compute sum of channel outputs
	void output(s32 outputs[RegisterType::OUTPUTS], u8 rshift, s32 clipmax, u32 chanmask) const;

	// write to the OPN registers
	void write(u16 regnum, u8 data);

	// return the current status
	u8 status() const;

	// set/reset bits in the status register, updating the IRQ status
	u8 set_reset_status(u8 set, u8 reset)
	{
		m_status = (m_status | set) & ~reset & ~m_regs.status_mask();
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
	u8 clock_prescale() const { return m_clock_prescale; }

	// set prescale factor (2/3/6)
	void set_clock_prescale(u8 prescale) { m_clock_prescale = prescale; }

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
	void update_timer(u8 which, u8 enable);

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
	attotime m_busy_end;             // end of the busy time
	emu_timer *m_timer[2];           // our two timers
	devcb_write_line m_irq_handler;  // IRQ callback
	RegisterType m_regs;             // register accessor
 	std::unique_ptr<ymfm_channel<RegisterType>> m_channel[RegisterType::CHANNELS]; // channel pointers
 	std::unique_ptr<ymfm_operator<RegisterType>> m_operator[RegisterType::OPERATORS]; // operator pointers
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


// ======================> ymopll_engine

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
