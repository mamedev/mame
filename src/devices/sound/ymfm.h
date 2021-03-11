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
//  GLOBAL HELPERS
//*********************************************************

//
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
//

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

// ======================> ymfm_registers_base

class ymfm_registers_base
{
protected:
	// constructor
	ymfm_registers_base(u8 *regdata) :
		m_regdata(regdata),
		m_chnum(0),
		m_opnum(0),
		m_choffs(0),
		m_opoffs(0)
	{
	}

public:
	// family type
	enum family_type : u8
	{
		FAMILY_OPM,
		FAMILY_OPN,
		FAMILY_OPL
	};

	// this value is returned from the is_keyon() function for rhythm channels
	static constexpr u8 YMFM_RHYTHM_CHANNEL = 15;

	//
	// the following constants need to be defined per family:
	//   family_type FAMILY: One of the family_types above
	//           u8 OUTPUTS: The number of outputs exposed (1-4)
	//          u8 CHANNELS: The number of channels on the chip
	//         u8 OPERATORS: The number of operators on the chip
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

	// common getters for current channel and operator number
	u8 chnum() const { return m_chnum; }
	u8 opnum() const { return m_opnum; }

	// default behavior is that operators and channels are mapped linearly
	void set_chnum(u8 chnum) { m_chnum = m_choffs = chnum; }
	void set_opnum(u8 opnum) { m_opnum = m_opoffs = opnum; }

	// helper to determine if the this channel is an active rhythm channel
	bool is_rhythm() const { return false; }

	// the following functions must be overriden
	static constexpr std::pair<u8,u8> opnum_to_chnum_and_index(u8 opnum) { assert(false); return std::make_pair(0, 0); }
	void reset() { assert(false); }
	void write(u16 index, u8 data) { assert(false); }
	static bool is_keyon(u16 regindex, u8 data, u8 &channel, u8 &opmask) { assert(false); return false; }
	u8 block_freq_to_keycode(u16 block_freq) const { assert(false); return 0; }
	u8 envelope_rate(u8 state) const { assert(false); return 0; }

	// system-wide register defaults
	u8 test() const               /*  8 bits */ { return 0; }
	u8 status_mask() const        /*  8 bits */ { return 0; } // OPL only
	u8 irq_reset() const          /*  8 bits */ { return 0; } // OPL only
	u16 timer_a_value() const     /* 10 bits */ { return 0; } // all but OPLL
	u8 timer_b_value() const      /*  8 bits */ { return 0; } // all but OPLL
	u8 reset_timer_a() const      /*  1 bit  */ { return 0; } // all but OPLL
	u8 reset_timer_b() const      /*  1 bit  */ { return 0; } // all but OPLL
	u8 enable_timer_a() const     /*  1 bit  */ { return 0; } // all but OPLL
	u8 enable_timer_b() const     /*  1 bit  */ { return 0; } // all but OPLL
	u8 load_timer_a() const       /*  1 bit  */ { return 0; } // all but OPLL
	u8 load_timer_b() const       /*  1 bit  */ { return 0; } // all but OPLL
	u8 csm() const                /*  1 bit  */ { return 0; } // all but OPL3
	u8 noise_frequency() const    /*  5 bits */ { return 0; } // OPM only
	u8 noise_enabled() const      /*  1 bit  */ { return 0; } // OPM only
	u8 lfo_enabled() const        /*  1 bit  */ { return 1; } // OPN(A) only
	u8 lfo_rate() const           /*3-8 bits */ { return 0; } // OPM,OPNA
	u8 lfo_waveform() const       /*  2 bits */ { return 0; } // OPM only
	u8 lfo_pm_depth() const       /*  7 bits */ { return 0; } // OPM,OPL only
	u8 lfo_am_depth() const       /*  7 bits */ { return 0; } // OPM,OPL only
	u8 multi_freq() const         /*  1 bit  */ { return 0; } // OPN(A) only
	u16 multi_block_freq0() const /* 14 bits */ { return 0; } // OPN(A) only
	u16 multi_block_freq1() const /* 14 bits */ { return 0; } // OPN(A) only
	u16 multi_block_freq2() const /* 14 bits */ { return 0; } // OPN(A) only
	u8 rhythm_enable() const      /*  1 bit  */ { return 0; } // OPL only
	u8 rhythm_keyon() const       /*  5 bits */ { return 0; } // OPL only
	u8 note_select() const        /*  1 bit  */ { return 0; } // OPL only
	u8 waveform_enable() const    /*  1 bits */ { return 0; } // OPL2+ only

	// per-channel register defaults
	u16 block_freq() const        /* 13 bits */ { return 0; }
	u8 algorithm() const          /*  3 bits */ { return 0; }
	u8 feedback() const           /*  3 bits */ { return 0; }
	u8 output0() const            /*  1 bit  */ { return 1; } // OPM,OPNA,OPL3+ only
	u8 output1() const            /*  1 bit  */ { return 0; } // OPM,OPNA,OPL3+ only
	u8 output2() const            /*  1 bit  */ { return 0; } // OPL3+ only
	u8 output3() const            /*  1 bit  */ { return 0; } // OPL3+ only
	u8 lfo_pm_sensitivity() const /*  3 bits */ { return 0; } // OPM,OPNA only
	u8 lfo_am_sensitivity() const /*  2 bits */ { return 0; } // OPM,OPNA only
	u8 instrument() const         /*  4 bits */ { return 0; } // OPLL only
	u8 sustain() const            /*  1 bit  */ { return 0; } // OPLL only

	// per-operator register defaults
	u8 attack_rate() const        /*  5 bits */ { return 0; }
	u8 decay_rate() const         /*  5 bits */ { return 0; }
	u8 sustain_level() const      /*  4 bits */ { return 0; }
	u8 release_rate() const       /*  4 bits */ { return 0; }
	u8 ksr() const                /*  2 bits */ { return 0; }
	u8 multiple() const           /*  4 bits */ { return 0; }
	u8 total_level() const        /*  7 bits */ { return 0; }
	u8 detune() const             /*  3 bits */ { return 0; } // OPM,OPN(A) only
	u8 detune2() const            /*  2 bits */ { return 0; } // OPM only
	u8 lfo_am_enabled() const     /*  1 bit  */ { return 0; } // OPM,OPNA,OPL only
	u8 lfo_pm_enabled() const     /*  1 bit  */ { return 0; } // OPL only
	u8 eg_sustain() const         /*  1 bit  */ { return 1; } // OPL only
	u8 ssg_eg_enabled() const     /*  1 bit  */ { return 0; } // OPN(A) only
	u8 ssg_eg_mode() const        /*  1 bit  */ { return 0; } // OPN(A) only
	u8 sustain_rate() const       /*  4 bits */ { return 0; } // OPM,OPN(A) only
	u8 key_scale_level() const    /*  2 bits */ { return 0; } // OPL only
	u8 waveform() const           /*  3 bits */ { return 0; } // OPL2+ only

protected:
	// return a bitfield extracted from a byte
	u8 sysbyte(u16 offset, u8 start, u8 count) const { return BIT(m_regdata[offset], start, count); }
	u8 chbyte(u16 offset, u8 start, u8 count) const { return sysbyte(m_choffs + offset, start, count); }
	u8 opbyte(u16 offset, u8 start, u8 count) const { return sysbyte(m_opoffs + offset, start, count); }

	// return a bitfield extracted from a pair of bytes, MSBs listed first
	u16 sysword(u16 offset1, u8 start1, u8 count1, u16 offset2, u8 start2, u8 count2) const
	{
		return (sysbyte(offset1, start1, count1) << count2) | sysbyte(offset2, start2, count2);
	}
	u16 chword(u16 offset1, u8 start1, u8 count1, u16 offset2, u8 start2, u8 count2) const
	{
		return (chbyte(offset1, start1, count1) << count2) | chbyte(offset2, start2, count2);
	}

	// internal state
	u8 *m_regdata;                 // pointer to the raw data
	u8 m_chnum;                    // channel index
	u8 m_opnum;                    // operator index
	u16 m_choffs;                  // channel offset for index
	u16 m_opoffs;                  // operator offset for index
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
//        20-27 xx------ Pan right
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
	static constexpr u8 OPERATORS = 8*4;
	static constexpr u16 REGISTERS = 0x100;
	static constexpr u16 REG_MODE = 0x14;
	static constexpr u8 DEFAULT_PRESCALE = 2;
	static constexpr u8 EG_CLOCK_DIVIDER = 3;
	static constexpr bool EG_HAS_DEPRESS = false;
	static constexpr bool MODULATOR_DELAY = false;
	static constexpr u32 CSM_TRIGGER_MASK = 0xff;
	static constexpr u8 STATUS_TIMERA = 0x01;
	static constexpr u8 STATUS_TIMERB = 0x02;
	static constexpr u8 STATUS_BUSY = 0x80;
	static constexpr u8 STATUS_IRQ = 0;

	// constructor
	ymopm_registers(u8 *regdata) :
		ymfm_registers_base(regdata)
	{
	}

	// return a mapping from linear operator to channel + index
	static constexpr std::pair<u8,u8> opnum_to_chnum_and_index(u8 opnum)
	{
		//    Op: 0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
		//    Ch: 0  1  2  3  4  5  6  7  0  1  2  3  4  5  6  7  0  1  2  3  4  5  6  7  0  1  2  3  4  5  6  7
		// Index: 0  0  0  0  0  0  0  0  2  2  2  2  2  2  2  2  1  1  1  1  1  1  1  1  3  3  3  3  3  3  3  3
		//
		// Note that the channel index order is 0,2,1,3, so we bitswap the index.
		//
		// This is because the order in the map is:
		//    carrier 1, carrier 2, modulator 1, modulator 2
		//
		// But when wiring up the connections, the more natural order is:
		//    carrier 1, modulator 1, carrier 2, modulator 2
		assert(opnum < OPERATORS);
		u8 chnum = opnum % CHANNELS;
		u8 index = opnum / CHANNELS;
		return std::make_pair(chnum, bitswap(index, 0, 1));
	}

	// reset state to default values
	void reset()
	{
		std::fill_n(&m_regdata[0], REGISTERS, 0);

		// enable output on both channels by default
		m_regdata[0x20] = m_regdata[0x21] = m_regdata[0x22] = m_regdata[0x23] = 0xc0;
		m_regdata[0x24] = m_regdata[0x25] = m_regdata[0x26] = m_regdata[0x27] = 0xc0;
	}

	// write access
	void write(u16 index, u8 data)
	{
		assert(index < REGISTERS);

		// LFO AM/PM depth are written to the same register (0x19);
		// redirect the PM depth to an unused neighbor (0x1a)
		if (index == 0x19)
			m_regdata[index + BIT(data, 7)] = data;
		else if (index != 0x1a)
			m_regdata[index] = data;
	}

	// determine if a given write is a keyon, and if so, for which channel/operators
	static bool is_keyon(u16 regindex, u8 data, u8 &channel, u8 &opmask)
	{
		if (regindex == 0x08)
		{
			channel = BIT(data, 0, 3);
			opmask = BIT(data, 3, 4);
			return true;
		}
		return false;
	}

	// compute the keycode from the given block_freq value
	u8 block_freq_to_keycode(u16 block_freq) const
	{
		// block_freq is block(3b):keycode(4b):keyfrac(6b); the 5-bit keycode
		// we want is just the top 5 bits here
		return BIT(block_freq, 8, 5);
	}

	// special helper for generically getting the envelope rates
	u8 envelope_rate(u8 state) const
	{
		// attack/decay/sustain are identical
		if (state < 3)
			return opbyte(0x80 + (state << 5), 0, 5);

		// release encodes 4 bits and expands them
		else
			return opbyte(0xe0, 0, 4) * 2 + 1;
	}

	// system-wide registers
	u8 test() const               /*  8 bits */ { return sysbyte(0x01, 0, 8); }
	u8 noise_frequency() const    /*  5 bits */ { return sysbyte(0x0f, 0, 5); }
	u8 noise_enabled() const      /*  1 bit  */ { return sysbyte(0x0f, 7, 1); }
	u16 timer_a_value() const     /* 10 bits */ { return sysword(0x10, 0, 8, 0x11, 0, 2); }
	u8 timer_b_value() const      /*  8 bits */ { return sysbyte(0x12, 0, 8); }
	u8 csm() const                /*  1 bit  */ { return sysbyte(0x14, 7, 1); }
	u8 reset_timer_b() const      /*  1 bit  */ { return sysbyte(0x14, 5, 1); }
	u8 reset_timer_a() const      /*  1 bit  */ { return sysbyte(0x14, 4, 1); }
	u8 enable_timer_b() const     /*  1 bit  */ { return sysbyte(0x14, 3, 1); }
	u8 enable_timer_a() const     /*  1 bit  */ { return sysbyte(0x14, 2, 1); }
	u8 load_timer_b() const       /*  1 bit  */ { return sysbyte(0x14, 1, 1); }
	u8 load_timer_a() const       /*  1 bit  */ { return sysbyte(0x14, 0, 1); }
	u8 lfo_rate() const           /*  8 bits */ { return sysbyte(0x18, 0, 8); }
	u8 lfo_am_depth() const       /*  7 bits */ { return sysbyte(0x19, 0, 7); }
	u8 lfo_pm_depth() const       /*  7 bits */ { return sysbyte(0x1a, 0, 7); }
	u8 lfo_waveform() const       /*  2 bits */ { return sysbyte(0x1b, 0, 2); }

	// per-channel registers
	u8 output1() const            /*  1 bit  */ { return chbyte(0x20, 7, 1); }
	u8 output0() const            /*  1 bit  */ { return chbyte(0x20, 6, 1); }
	u8 feedback() const           /*  3 bits */ { return chbyte(0x20, 3, 3); }
	u8 algorithm() const          /*  3 bits */ { return chbyte(0x20, 0, 3); }
	u16 block_freq() const        /* 13 bits */ { return chword(0x28, 0, 7, 0x30, 2, 6); }
	u8 lfo_pm_sensitivity() const /*  3 bits */ { return chbyte(0x38, 4, 3); }
	u8 lfo_am_sensitivity() const /*  2 bits */ { return chbyte(0x38, 0, 2); }

	// per-operator registers
	u8 detune() const             /*  3 bits */ { return opbyte(0x40, 4, 3); }
	u8 multiple() const           /*  4 bits */ { return opbyte(0x40, 0, 4); }
	u8 total_level() const        /*  7 bits */ { return opbyte(0x60, 0, 7); }
	u8 ksr() const                /*  2 bits */ { return opbyte(0x80, 6, 2); }
	u8 attack_rate() const        /*  5 bits */ { return opbyte(0x80, 0, 5); }
	u8 lfo_am_enabled() const     /*  1 bit  */ { return opbyte(0xa0, 7, 1); }
	u8 decay_rate() const         /*  5 bits */ { return opbyte(0xa0, 0, 5); }
	u8 detune2() const            /*  2 bits */ { return opbyte(0xc0, 6, 2); }
	u8 sustain_rate() const       /*  5 bits */ { return opbyte(0xc0, 0, 5); }
	u8 sustain_level() const      /*  4 bits */ { return opbyte(0xe0, 4, 4); }
	u8 release_rate() const       /*  4 bits */ { return opbyte(0xe0, 0, 4); }
};


// ======================> ymopn_registers

//
// OPN register map:
//
//      System-wide registers:
//           21 xxxxxxxx Test register
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
//        30-3F -xxx---- Detune value (0-7)
//              ----xxxx Multiple value (0-15)
//        40-4F -xxxxxxx Total level (0-127)
//        50-5F xx------ Key scale rate (0-3)
//              ---xxxxx Attack rate (0-31)
//        60-6F ---xxxxx Decay rate (0-31)
//        70-7F ---xxxxx Sustain rate (0-31)
//        80-8F xxxx---- Sustain level (0-15)
//              ----xxxx Release rate (0-15)
//        90-9F ----x--- SSG-EG enable
//              -----xxx SSG-EG envelope (0-7)
//
//     Per-channel registers (channel in address bits 0-1)
//        A0-A3 xxxxxxxx Frequency number lower 8 bits
//        A4-A7 --xxx--- Block (0-7)
//              -----xxx Frequency number upper 3 bits
//        B0-B3 --xxx--- Feedback level for operator 1 (0-7)
//              -----xxx Operator connection algorithm (0-7)
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

class ymopn_registers : public ymfm_registers_base
{
public:
	// constants
	static constexpr family_type FAMILY = FAMILY_OPN;
	static constexpr u8 OUTPUTS = 1;
	static constexpr u8 CHANNELS = 3;
	static constexpr u8 OPERATORS = 3*4;
	static constexpr u16 REGISTERS = 0x100;
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
	ymopn_registers(u8 *regdata) :
		ymfm_registers_base(regdata)
	{
	}

	// setters for operator number
	void set_opnum(u8 opnum)
	{
		assert(opnum < OPERATORS);
		m_opnum = opnum;

		// operator offset has gaps so the default linear mapping won't work
		m_opoffs = opnum % 3 + 4 * (opnum / 3);
	}

	// return a mapping from linear operator to channel + index
	static constexpr std::pair<u8,u8> opnum_to_chnum_and_index(u8 opnum)
	{
		//    Op: 0  1  2  3  4  5  6  7  8  9 10 11
		//    Ch: 0  1  2  0  1  2  0  1  2  0  1  2
		// Index: 0  0  0  2  2  2  1  1  1  3  3  3
		//
		// Note that the channel index order is 0,2,1,3, so we bitswap the index.
		//
		// This is because the order in the map is:
		//    carrier 1, carrier 2, modulator 1, modulator 2
		//
		// But when wiring up the connections, the more natural order is:
		//    carrier 1, modulator 1, carrier 2, modulator 2
		assert(opnum < OPERATORS);
		u8 chnum = opnum % CHANNELS;
		u8 index = opnum / CHANNELS;
		return std::make_pair(chnum, bitswap(index, 0, 1));
	}

	// reset state to default values
	void reset()
	{
		std::fill_n(&m_regdata[0], REGISTERS, 0);
	}

	// write access
	void write(u16 index, u8 data)
	{
		assert(index < REGISTERS);

		// writes in the 0xa0-af/0x1a0-af region are handled as latched pairs
		// borrow unused registers 0xb8-bf/0x1b8-bf as temporary holding locations
		if ((index & 0xf0) == 0xa0)
		{
			u16 latchindex = (index & 0x100) | 0xb8 | (BIT(index, 3) << 2) | BIT(index, 0, 2);

			// writes to the upper half just latch (only low 6 bits matter)
			if (BIT(index, 2))
				m_regdata[latchindex] = data | 0x80;

			// writes to the lower half only commit if the latch is there
			else if (BIT(m_regdata[latchindex], 7))
			{
				m_regdata[index | 4] = m_regdata[latchindex] & 0x3f;
				m_regdata[latchindex] = 0;
			}
		}

		// everything else is normal
		m_regdata[index] = data;
	}

	// determine if a given write is a keyon, and if so, for which channel/operators
	static bool is_keyon(u16 regindex, u8 data, u8 &channel, u8 &opmask)
	{
		if (regindex == 0x28)
		{
			channel = BIT(data, 0, 2);
			opmask = BIT(data, 4, 4);
			return true;
		}
		return false;
	}

	// compute the keycode from the given block_freq value
	u8 block_freq_to_keycode(u16 block_freq) const
	{
		// block_freq is block(3b):fnum(11b); the 5-bit keycode uses the top
		// 4 bits plus a magic formula for the final bit
		u8 keycode = BIT(block_freq, 10, 4) << 1;

		// lowest bit is determined by a mix of next lower FNUM bits
		// according to this equation from the YM2608 manual:
		//
		//   (F11 & (F10 | F9 | F8)) | (!F11 & F10 & F9 & F8)
		//
		// for speed, we just look it up in a 16-bit constant
		return keycode | BIT(0xfe80, BIT(block_freq, 7, 4));
	}

	// special helper for generically getting the envelope rates
	u8 envelope_rate(u8 state) const
	{
		// attack/decay/sustain are identical
		if (state < 3)
			return opbyte(0x50 + (state << 4), 0, 5);

		// release encodes 4 bits and expands them
		else
			return opbyte(0x80, 0, 4) * 2 + 1;
	}

	// system-wide registers
	u8 test() const               /*  8 bits */ { return sysbyte(0x21, 0, 8); }
	u16 timer_a_value() const     /* 10 bits */ { return sysword(0x24, 0, 8, 0x25, 0, 2); }
	u8 timer_b_value() const      /*  8 bits */ { return sysbyte(0x26, 0, 8); }
	u8 csm() const                /*  2 bits */ { return (sysbyte(0x27, 6, 2) == 2); }
	u8 multi_freq() const         /*  2 bits */ { return (sysbyte(0x27, 6, 2) != 0); }
	u8 reset_timer_b() const      /*  1 bit  */ { return sysbyte(0x27, 5, 1); }
	u8 reset_timer_a() const      /*  1 bit  */ { return sysbyte(0x27, 4, 1); }
	u8 enable_timer_b() const     /*  1 bit  */ { return sysbyte(0x27, 3, 1); }
	u8 enable_timer_a() const     /*  1 bit  */ { return sysbyte(0x27, 2, 1); }
	u8 load_timer_b() const       /*  1 bit  */ { return sysbyte(0x27, 1, 1); }
	u8 load_timer_a() const       /*  1 bit  */ { return sysbyte(0x27, 0, 1); }
	u16 multi_block_freq0() const /* 14 bits */ { return sysword(0xac, 0, 6, 0xa8, 0, 8); }
	u16 multi_block_freq1() const /* 14 bits */ { return sysword(0xad, 0, 6, 0xa9, 0, 8); }
	u16 multi_block_freq2() const /* 14 bits */ { return sysword(0xae, 0, 6, 0xaa, 0, 8); }

	// per-channel registers
	u16 block_freq() const        /* 14 bits */ { return chword(0xa4, 0, 6, 0xa0, 0, 8); }
	u8 feedback() const           /*  3 bits */ { return chbyte(0xb0, 3, 3); }
	u8 algorithm() const          /*  3 bits */ { return chbyte(0xb0, 0, 3); }

	// per-operator registers
	u8 detune() const             /*  3 bits */ { return opbyte(0x30, 4, 3); }
	u8 multiple() const           /*  4 bits */ { return opbyte(0x30, 0, 4); }
	u8 total_level() const        /*  8 bits */ { return opbyte(0x40, 0, 7); }
	u8 ksr() const                /*  2 bits */ { return opbyte(0x50, 6, 2); }
	u8 attack_rate() const        /*  5 bits */ { return opbyte(0x50, 0, 5); }
	u8 decay_rate() const         /*  5 bits */ { return opbyte(0x60, 0, 5); }
	u8 sustain_rate() const       /*  5 bits */ { return opbyte(0x70, 0, 5); }
	u8 sustain_level() const      /*  4 bits */ { return opbyte(0x80, 4, 4); }
	u8 release_rate() const       /*  4 bits */ { return opbyte(0x80, 0, 4); }
	u8 ssg_eg_enabled() const     /*  1 bit  */ { return opbyte(0x90, 3, 1); }
	u8 ssg_eg_mode() const        /*  3 bits */ { return opbyte(0x90, 0, 3); }

	// no LFO on OPN
	u8 lfo_enabled() const { return 0; }
};


// ======================> ymopna_registers

//
// OPNA/OPNB/OPNB2/OPN2 register map:
//
//      System-wide registers:
//           21 xxxxxxxx Test register
//           22 ----x--- LFO enable (new for OPNA)
//              -----xxx LFO rate (new for OPNA)
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
//              -----x-- Upper channel select (new for OPNA)
//              ------xx Channel select
//
//     Per-operator registers (channel in address bits 0-1, operator in bits 2-3)
//        30-3F -xxx---- Detune value (0-7)
//              ----xxxx Multiple value (0-15)
//        40-4F -xxxxxxx Total level (0-127)
//        50-5F xx------ Key scale rate (0-3)
//              ---xxxxx Attack rate (0-31)
//        60-6F x------- LFO AM enable (new for OPNA)
//              ---xxxxx Decay rate (0-31)
//        70-7F ---xxxxx Sustain rate (0-31)
//        80-8F xxxx---- Sustain level (0-15)
//              ----xxxx Release rate (0-15)
//        90-9F ----x--- SSG-EG enable
//              -----xxx SSG-EG envelope (0-7)
//
//     Per-channel registers (channel in address bits 0-1)
//        A0-A3 xxxxxxxx Frequency number lower 8 bits
//        A4-A7 --xxx--- Block (0-7)
//              -----xxx Frequency number upper 3 bits
//        B0-B3 --xxx--- Feedback level for operator 1 (0-7)
//              -----xxx Operator connection algorithm (0-7)
//        B4-B7 x------- Pan left (new for OPNA)
//              -x------ Pan right (new for OPNA)
//              --xx---- LFO AM shift (0-3) (new for OPNA)
//              -----xxx LFO PM depth (0-7) (new for OPNA)
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

class ymopna_registers : public ymopn_registers
{
public:
	// constants
	static constexpr u8 OUTPUTS = 2;
	static constexpr u8 CHANNELS = 6;
	static constexpr u8 OPERATORS = 6*4;
	static constexpr u16 REGISTERS = 0x200;

	// constructor
	ymopna_registers(u8 *regdata) :
		ymopn_registers(regdata)
	{
	}

	// setters for channel and operator base within the register file
	void set_chnum(u8 chnum)
	{
		assert(chnum < CHANNELS);
		m_chnum = chnum;
		m_choffs = chnum % 3 + 0x100 * (chnum / 3);
	}
	void set_opnum(u8 opnum)
	{
		assert(opnum < OPERATORS);
		m_opnum = opnum;
		m_opoffs = opnum % 3 + 4 * ((opnum % 12) / 3) + 0x100 * (opnum / 12);
	}

	// initial mapping of operator number to channel.opindex:
	static constexpr std::pair<u8,u8> opnum_to_chnum_and_index(u8 opnum)
	{
		//    Op: 0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23
		//    Ch: 0  1  2  0  1  2  0  1  2  0  1  2  3  4  5  3  4  5  3  4  5  3  4  5
		// Index: 0  0  0  2  2  2  1  1  1  3  3  3  0  0  0  2  2  2  1  1  1  3  3  3
		//
		// Note that the channel index order is 0,2,1,3, so we bitswap the index.
		//
		// This is because the order in the map is:
		//    carrier 1, carrier 2, modulator 1, modulator 2
		//
		// But when wiring up the connections, the more natural order is:
		//    carrier 1, modulator 1, carrier 2, modulator 2
		assert(opnum < OPERATORS);
		u8 chnum = opnum % (CHANNELS/2);
		u8 index = opnum / (CHANNELS/2);
		chnum += 3 * BIT(index, 2);
		return std::make_pair(chnum, bitswap(index, 0, 1));
	}

	// reset state to default values
	void reset()
	{
		std::fill_n(&m_regdata[0], REGISTERS, 0);

		// enable output on both channels by default
		m_regdata[0xb4] = m_regdata[0xb5] = m_regdata[0xb6] = 0xc0;
		m_regdata[0x1b4] = m_regdata[0x1b5] = m_regdata[0x1b6] = 0xc0;
	}

	// determine if a given write is a keyon, and if so, for which channel/operators
	static bool is_keyon(u16 regindex, u8 data, u8 &channel, u8 &opmask)
	{
		if (regindex == 0x28)
		{
			channel = BIT(data, 0, 2);
			if (channel == 3)
				return false;
			channel += BIT(data, 2, 1) * 3;
			opmask = BIT(data, 4, 4);
			return true;
		}
		return false;
	}

	// OPNA-specific system-wide registers
	u8 lfo_enabled() const        /*  3 bits */ { return sysbyte(0x22, 3, 1); }
	u8 lfo_rate() const           /*  3 bits */ { return sysbyte(0x22, 0, 3); }

	// OPNA-specific per-channel registers
	u8 output0() const            /*  1 bit  */ { return chbyte(0xb4, 7, 1); }
	u8 output1() const            /*  1 bit  */ { return chbyte(0xb4, 6, 1); }
	u8 lfo_am_sensitivity() const /*  2 bits */ { return chbyte(0xb4, 4, 2); }
	u8 lfo_pm_sensitivity() const /*  3 bits */ { return chbyte(0xb4, 0, 3); }

	// OPNA-specific per-operator registers
	u8 lfo_am_enabled() const     /*  1 bit  */ { return opbyte(0x60, 7, 1); }
};


// ======================> ymopl_registers, ymopl2_registers

//
// OPL/OPL2 register map:
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
//           08 x------- CSM mode
//              -x------ Note select
//           BD x------- AM depth
//              -x------ PM depth
//              --x----- Rhythm enable
//              ---x---- Bass drum key on
//              ----x--- Snare drum key on
//              -----x-- Tom key on
//              ------x- Top cymbal key on
//              -------x High hat key on
//
//     Per-channel registers (channel in address bits 0-3)
//        A0-A8 xxxxxxxx F-number (low 8 bits)
//        B0-B8 --x----- Key on
//              ---xxx-- Block (octvate, 0-7)
//              ------xx F-number (high two bits)
//        C0-C8 ----xxx- Feedback level for operator 1 (0-7)
//              -------x Operator connection algorithm
//
//     Per-operator registers (operator in bits 0-5)
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
//

class ymopl_registers : public ymfm_registers_base
{
public:
	// constants
	static constexpr family_type FAMILY = FAMILY_OPL;
	static constexpr u8 OUTPUTS = 1;
	static constexpr u8 CHANNELS = 9;
	static constexpr u8 OPERATORS = 9*2;
	static constexpr u16 REGISTERS = 0x100;
	static constexpr u16 REG_MODE = 0x04;
	static constexpr u8 DEFAULT_PRESCALE = 4;
	static constexpr u8 EG_CLOCK_DIVIDER = 1;
	static constexpr bool EG_HAS_DEPRESS = false;
	static constexpr bool MODULATOR_DELAY = true;
	static constexpr u32 CSM_TRIGGER_MASK = 0x1ff;
	static constexpr u8 STATUS_TIMERA = 0x40;
	static constexpr u8 STATUS_TIMERB = 0x20;
	static constexpr u8 STATUS_BUSY = 0;
	static constexpr u8 STATUS_IRQ = 0x80;

	// constructor
	ymopl_registers(u8 *regdata) :
		ymfm_registers_base(regdata)
	{
	}

	// setters for operator number
	void set_opnum(u8 opnum)
	{
		assert(opnum < OPERATORS);
		m_opnum = opnum;
		m_opoffs = opnum % 6 + 8 * (opnum / 6);
	}

	// return a mapping from linear operator to channel + index
	static constexpr std::pair<u8,u8> opnum_to_chnum_and_index(u8 opnum)
	{
		//    Op: 0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17
		//    Ch: 0  1  2  0  1  2  3  4  5  3  4  5  6  7  8  6  7  8
		// Index: 0  0  0  1  1  1  0  0  0  1  1  1  0  0  0  1  1  1
		assert(opnum < OPERATORS);
		u8 chnum = opnum % (CHANNELS/3);
		u8 index = opnum / (CHANNELS/3);
		chnum += (CHANNELS/3) * BIT(index, 1, 2);
		return std::make_pair(chnum, BIT(index, 0));
	}

	// helper to determine if the this channel is an active rhythm channel
	bool is_rhythm() const
	{
		return rhythm_enable() && (m_chnum >= 6);
	}

	// reset state to default values
	void reset()
	{
		std::fill_n(&m_regdata[0], REGISTERS, 0);
	}

	// write access
	void write(u16 index, u8 data)
	{
		assert(index < REGISTERS);

		// writes to the mode register with high bit set ignore the low bits
		if (index == REG_MODE && BIT(data, 7) != 0)
			m_regdata[index] |= 0x80;
		else
			m_regdata[index] = data;
	}

	// determine if a given write is a keyon, and if so, for which channel/operators
	static bool is_keyon(u16 regindex, u8 data, u8 &channel, u8 &opmask)
	{
		if ((regindex & 0xf0) == 0xb0)
		{
			channel = regindex & 0x0f;
			if (channel == 13)
			{
				channel = YMFM_RHYTHM_CHANNEL;
				opmask = BIT(data, 5) ? BIT(data, 0, 5) : 0;
				return true;
			}
			else if (channel < CHANNELS)
			{
				opmask = BIT(data, 5) ? 3 : 0;
				return true;
			}
		}
		return false;
	}

	// compute the keycode from the given block_freq value
	u8 block_freq_to_keycode(u16 block_freq) const
	{
		u8 keycode = BIT(block_freq, 11, 3) << 1;

		// block_freq is block(3b):fnum(10b):0; the 4-bit keycode uses the top
		// 3 bits plus either the MSB or MSB-1 of fnum, depending on note_select
		return keycode | BIT(block_freq, 10 - note_select(), 1);
	}

	// OPL-specific helper to handle the weird multiple mapping
	static constexpr u8 opl_multiple_map(u8 raw)
	{
		// replace the low bit with a table lookup; the equivalent
		// OPM/OPN values are: 0,1,2,3,4,5,6,7,8,9,10,10,12,12,15,15
		return (raw & 0xe) | BIT(0xc2aa, raw);
	}

	// special helper for generically getting the envelope rates
	u8 envelope_rate(u8 state) const
	{
		// attack/decay are 4 bits, expanded to 5 to match OPM/OPN
		if (state < 2)
			return opbyte(0x60, (state ^ 1) * 4, 4) * 2;

		// sustain rate doesn't exist in OPL, so effectively 0, unless in percussion mode
		else if (state == 2 && eg_sustain() != 0)
			return 0;

		// release is 4 bits, expanded to 5
		return opbyte(0x80, 0, 4) * 2;
	}

	// system-wide registers
	u8 test() const               /*  8 bits */ { return sysbyte(0x01, 0, 8); }
	u16 timer_a_value() const     /*  8 bits */ { return sysbyte(0x02, 0, 8) * 4; } // 8->10 bits
	u8 timer_b_value() const      /*  8 bits */ { return sysbyte(0x03, 0, 8); }
	u8 status_mask() const        /*  8 bits */ { return sysbyte(0x04, 0, 8) & 0x78; }
	u8 irq_reset() const          /*  1 bit  */ { return sysbyte(0x04, 7, 1); }
	u8 reset_timer_b() const      /*  1 bit  */ { return sysbyte(0x04, 7, 1) | sysbyte(0x04, 5, 1); }
	u8 reset_timer_a() const      /*  1 bit  */ { return sysbyte(0x04, 7, 1) | sysbyte(0x04, 6, 1); }
	u8 enable_timer_b() const     /*  1 bit  */ { return sysbyte(0x04, 5, 1) ^ 1; }
	u8 enable_timer_a() const     /*  1 bit  */ { return sysbyte(0x04, 6, 1) ^ 1; }
	u8 load_timer_b() const       /*  1 bit  */ { return sysbyte(0x04, 1, 1); }
	u8 load_timer_a() const       /*  1 bit  */ { return sysbyte(0x04, 0, 1); }
	u8 csm() const                /*  1 bit  */ { return sysbyte(0x08, 7, 1); }
	u8 note_select() const        /*  1 bit  */ { return sysbyte(0x08, 6, 1); }
	u8 lfo_am_depth() const       /*  1 bit  */ { return sysbyte(0xbd, 7, 1) * 2; } // 1->2 bits
	u8 lfo_pm_depth() const       /*  1 bit  */ { return sysbyte(0xbd, 6, 1); }
	u8 rhythm_enable() const      /*  1 bit  */ { return sysbyte(0xbd, 5, 1); }
	u8 rhythm_keyon() const       /*  5 bits */ { return sysbyte(0xbd, 4, 0); }

	// per-channel registers
	u16 block_freq() const        /* 13 bits */ { return chword(0xb0, 0, 5, 0xa0, 0, 8) * 2; } // 13->14 bits
	u8 feedback() const           /*  3 bits */ { return chbyte(0xc0, 1, 3); }
	u8 algorithm() const          /*  1 bit  */ { return chbyte(0xc0, 0, 1); }

	// per-operator registers
	u8 lfo_am_enabled() const     /*  1 bit  */ { return opbyte(0x20, 7, 1); }
	u8 lfo_pm_enabled() const     /*  1 bit  */ { return opbyte(0x20, 6, 1); }
	u8 eg_sustain() const         /*  1 bit  */ { return opbyte(0x20, 5, 1); }
	u8 ksr() const                /*  1 bit  */ { return opbyte(0x20, 4, 1) * 2 + 1; } // 1->2 bits
	u8 multiple() const           /*  4 bits */ { return opl_multiple_map(opbyte(0x20, 0, 4)); }
	u8 key_scale_level() const    /*  2 bits */ { return bitswap<2>(opbyte(0x40, 6, 2), 0, 1); }
	u8 total_level() const        /*  6 bits */ { return opbyte(0x40, 0, 6); } // 6->7 bits
	u8 attack_rate() const        /*  4 bits */ { return opbyte(0x60, 4, 4) * 2; } // 4->5 bits
	u8 decay_rate() const         /*  4 bits */ { return opbyte(0x60, 0, 4) * 2; } // 4->5 bits
	u8 sustain_level() const      /*  4 bits */ { return opbyte(0x80, 4, 4); }
	u8 release_rate() const       /*  4 bits */ { return opbyte(0x80, 0, 4); }
};

class ymopl2_registers : public ymopl_registers
{
public:
	// constructor
	ymopl2_registers(u8 *regdata) :
		ymopl_registers(regdata)
	{
	}

	// system-wide registers
	u8 waveform_enable() const    /*  1 bits */ { return sysbyte(0x01, 5, 1); }

	// per-operator registers
	u8 waveform() const           /*  2 bits */ { return opbyte(0xe0, 0, 2); }
};


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
	ymopll_registers(u8 *regdata) :
		ymopl_registers(regdata)
	{
	}

	// setters for operator number
	void set_opnum(u8 opnum)
	{
		assert(opnum < OPERATORS);

		// OPL overrides this so we need to put it back
		m_opnum = m_opoffs = opnum;
	}

	// return a mapping from linear operator to channel + index
	static constexpr std::pair<u8,u8> opnum_to_chnum_and_index(u8 opnum)
	{
		//    Op: 0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17
		//    Ch: 0  0  1  1  2  2  3  3  4  4  5  5  6  6  7  7  8  8
		// Index: 0  1  0  1  0  1  0  1  0  1  0  1  0  1  0  1  0  1
		assert(opnum < OPERATORS);
		return std::make_pair(opnum / 2, opnum % 2);
	}

	// reset state to default values
	void reset()
	{
		std::fill_n(&m_regdata[0], EXTERNAL_REGISTERS, 0);
	}

	// write access
	void write(u16 index, u8 data)
	{
		assert(index < EXTERNAL_REGISTERS);

		// write the new data
		u8 old = m_regdata[index];
		m_regdata[index] = data;

		// look for instrument changes
		if (index >= 0x30 && index <= 0x38 && BIT(old ^ data, 4, 4) != 0)
			update_instrument(index - 0x30);

		// also look for rhythm state change
		else if (index == 0x0e && BIT(old ^ data, 5, 1) != 0)
		{
			update_instrument(6);
			update_instrument(7);
			update_instrument(8);
		}
	}

	// determine if a given write is a keyon, and if so, for which channel/operators
	static bool is_keyon(u16 regindex, u8 data, u8 &channel, u8 &opmask)
	{
		if ((regindex & 0xf0) == 0x20)
		{
			channel = regindex & 0x0f;
			if (channel < CHANNELS)
			{
				opmask = BIT(data, 4) ? 3 : 0;
				return true;
			}
		}
		else if (regindex == 0x0e)
		{
			channel = YMFM_RHYTHM_CHANNEL;
			opmask = BIT(data, 5) ? BIT(data, 0, 5) : 0;
			return true;
		}
		return false;
	}

	// compute the keycode from the given block_freq value
	u8 block_freq_to_keycode(u16 block_freq) const
	{
		// block_freq is block(3b):fnum(9b):00; the 4-bit keycode uses the top
		// 3 bits plus the MSB of fnum
		return BIT(block_freq, 10, 4);
	}

	// special helper for generically getting the envelope rates
	u8 envelope_rate(u8 state) const
	{
		// The envelope diagram in the YM2413 datasheet gives values for these
		// in ms from 0->48dB. The attack/decay tables give values in ms from
		// 0->96dB, so to pick an equivalent decay rate, we want to find the
		// closest match that is 2x the 0->48dB value:
		//
		//     DP =   10ms (0->48db) ->   20ms (0->96db); decay of 12 gives   19.20ms
		//     RR =  310ms (0->48db) ->  620ms (0->96db); decay of  7 gives  613.76ms
		//     RS = 1200ms (0->48db) -> 2400ms (0->96db); decay of  5 gives 2455.04ms
		//
		// The envelope diagram for percussive sounds (eg_sustain() == 0) also uses
		// "RR" to mean both the constant RR above and the Release Rate specified in
		// the instrument data. In this case, Relief Pitcher's credit sound bears out
		// that the Release Rate is used during sustain, and that the constant RR
		// (or RS) is used during the release phase.
		constexpr u8 DP = 12;
		constexpr u8 RR = 7;
		constexpr u8 RS = 5;

		// attack/decay are 4 bits, expanded to 5 to match OPM/OPN
		if (state < 2)
			return instopbyte(0x04, (state ^ 1) * 4, 4) * 2;

		// sustain: depends on percussive state (EGTYP) specified by the instrument
		else if (state == 2)
			return eg_sustain() ? 0 : instopbyte(0x06, 0, 4) * 2;

		// release: depends on sustain flag and percussive state
		else if (state == 3)
			return sustain() ? RS * 2 : (eg_sustain() ? instopbyte(0x06, 0, 4) * 2 : RR * 2);

		// depress: hard-coded
		else
			return DP * 2;
	}

	// system-wide registers
	u8 rhythm_enable() const      /*  1 bit  */ { return sysbyte(0x0e, 5, 1); }
	u8 rhythm_keyon() const       /*  5 bits */ { return sysbyte(0x0e, 4, 0); }
	u8 test() const               /*  8 bits */ { return sysbyte(0x0f, 0, 8); }
	u8 lfo_am_depth() const       /*  1 bit  */ { return 1; } // 1->2 bits
	u8 waveform_enable() const    /*  1 bits */ { return 1; }

	// per-channel registers
	u16 block_freq() const        /* 12 bits */ { return chword(0x20, 0, 4, 0x10, 0, 8) * 4; } // 12->14 bits
	u8 sustain() const            /*  1 bit  */ { return chbyte(0x20, 5, 1); }
	u8 feedback() const           /*  3 bits */ { return instchbyte(0x03, 0, 3); }
	u8 algorithm() const          /*  1 bit  */ { return 0; }
	u8 instrument() const         /*  4 bits */ { return chbyte(0x30, 4, 4); }
	u8 output0() const            /*  1 bit  */ { return is_rhythm() ? 0 : 1; }
	u8 output1() const            /*  1 bit  */ { return is_rhythm() ? 1 : 0; }

	// per-operator registers
	u8 lfo_am_enabled() const     /*  1 bit  */ { return instopbyte(0x00, 7, 1); }
	u8 lfo_pm_enabled() const     /*  1 bit  */ { return instopbyte(0x00, 6, 1); }
	u8 eg_sustain() const         /*  1 bit  */ { return instopbyte(0x00, 5, 1); }
	u8 ksr() const                /*  1 bit  */ { return instopbyte(0x00, 4, 1) * 2 + 1; } // 1->2 bits
	u8 multiple() const           /*  4 bits */ { return opl_multiple_map(instopbyte(0x00, 0, 4)); }
	u8 key_scale_level() const    /*  2 bits */ { return instopbyte(0x02, 6, 2); }
	u8 total_level() const        /*  6 bits */ { return total_level_or_volume(); }
	u8 waveform() const           /*  1 bit */  { return instchbyte(0x03, 3 + BIT(opnum(), 0), 1); }
	u8 attack_rate() const        /*  4 bits */ { return instopbyte(0x04, 4, 4) * 2; } // 4->5 bits
	u8 decay_rate() const         /*  4 bits */ { return instopbyte(0x04, 0, 4) * 2; } // 4->5 bits
	u8 sustain_level() const      /*  4 bits */ { return instopbyte(0x06, 4, 4); }
	u8 release_rate() const       /*  4 bits */ { return instopbyte(0x06, 0, 4); }

	// set the instrument data
	void set_instrument_data(u8 const *data)
	{
		memcpy(&m_regdata[INSTDATA_BASE], data, INSTDATA_SIZE);
	}

private:
	// OPLL-specific helper to return either the total level or the volume
	u8 total_level_or_volume() const
	{
		int op = BIT(m_opnum, 0);
		if (op == 1 || is_rhythm())
			return chbyte(0x30, 4 * (op ^ 1), 4) * 4;
		else
			return instchbyte(0x02, 0, 6);
	}

	// helper to compute the ROM address of an instrument number
	static constexpr u8 rom_address(int instrument)
	{
		return (instrument == 0) ? 0 : (INSTDATA_BASE + 8 * (instrument - 1));
	}

	// helper to update the instrument
	void update_instrument(int chnum)
	{
		u8 baseaddr;
		if (rhythm_enable() && chnum >= 6)
			baseaddr = rom_address(16 + (chnum - 6));
		else
			baseaddr = rom_address(sysbyte(0x30 + chnum, 4, 4));

		m_regdata[CHANNEL_INSTBASE + chnum] = baseaddr;
		m_regdata[OPERATOR_INSTBASE + chnum * 2 + 0] = baseaddr + 0;
		m_regdata[OPERATOR_INSTBASE + chnum * 2 + 1] = baseaddr + 1;
	}

	// helpers to read from instrument channel/operator data
	u8 instchbyte(u16 offset, u8 start, u8 count) const { return BIT(m_regdata[offset + m_regdata[m_choffs + CHANNEL_INSTBASE]], start, count); }
	u8 instopbyte(u16 offset, u8 start, u8 count) const { return BIT(m_regdata[offset + m_regdata[m_opoffs + OPERATOR_INSTBASE]], start, count); }
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
	enum envelope_state : u8
	{
		ENV_ATTACK = 0,
		ENV_DECAY = 1,
		ENV_SUSTAIN = 2,
		ENV_RELEASE = 3,
		ENV_DEPRESS = 4
	};

public:
	// constructor
	ymfm_operator(ymfm_engine_base<RegisterType> &owner, RegisterType regs);

	// register for save states
	void save(device_t &device, u8 index);

	// reset the operator state
	void reset();

	// master clocking function
	void clock(u32 env_counter, s8 lfo_raw_pm, u16 block_freq);

	// return the current phase value
	u16 phase() const { return m_phase >> 10; }

	// compute operator volume
	s16 compute_volume(u16 phase, u16 am_offset) const;

	// compute volume for the OPM noise channel
	s16 compute_noise_volume(u8 noise_state, u16 am_offset) const;

	// key state control
	void keyonoff(u8 on, ymfm_keyon_type type);

	// return a pointer to our registers
	RegisterType &regs() { return m_regs; }

private:
	// return the effective 6-bit ADSR rate after adjustments
	u8 effective_rate(u8 rawrate, u8 keycode);

	// start the attack phase
	void start_attack(u8 keycode);

	// start the release phase
	void start_release();

	// clock phases
	void clock_keystate(u8 keystate, u8 keycode);
	void clock_ssg_eg_state(u8 keycode);
	void clock_envelope(u16 env_counter, u8 keycode);
	void clock_phase(s8 lfo_raw_pm, u16 block_freq, u8 keycode);

	// return effective attenuation of the envelope
	u16 envelope_attenuation(u8 am_offset) const;

	// internal state
	u32 m_phase;                     // current phase value (10.10 format)
	u16 m_env_attenuation;           // computed envelope attenuation (4.6 format)
	envelope_state m_env_state;      // current envelope state
	u8 m_ssg_inverted;               // non-zero if the output should be inverted (bit 0)
	u8 m_key_state;                  // current key state: on or off (bit 0)
	u8 m_keyon_live;                 // live key on state (bit 0 = direct, bit 1 = rhythm, bit 2 = CSM)
	RegisterType m_regs;             // operator-specific registers
	ymfm_engine_base<RegisterType> &m_owner; // reference to the owning engine
};


// ======================> ymfm_channel

template<class RegisterType>
class ymfm_channel
{
public:
	// constructor
	ymfm_channel(ymfm_engine_base<RegisterType> &owner, RegisterType regs);

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
			op->regs().set_chnum(m_regs.chnum());
	}

	// signal key on/off to our operators
	void keyonoff(u8 states, ymfm_keyon_type type);

	// master clocking function
	void clock(u32 env_counter, s8 lfo_raw_pm, bool is_multi_freq);

	// compute the channel output and add to the left/right output sums
	void output(u8 lfo_raw_am, u8 noise_state, s32 outputs[RegisterType::OUTPUTS], u8 rshift, s32 clipmax) const;

	// compute the special OPL rhythm channel outputs
	void output_rhythm_ch6(u8 lfo_raw_am, s32 outputs[RegisterType::OUTPUTS], u8 rshift, s32 clipmax) const;
	void output_rhythm_ch7(u8 lfo_raw_am, u8 noise_state, u8 phase_select, s32 outputs[RegisterType::OUTPUTS], u8 rshift, s32 clipmax) const;
	void output_rhythm_ch8(u8 lfo_raw_am, u8 phase_select, s32 outputs[RegisterType::OUTPUTS], u8 rshift, s32 clipmax) const;

	// return a pointer to our registers
	RegisterType &regs() { return m_regs; }

private:
	// convert a 6/8-bit raw AM value into an amplitude offset based on sensitivity
	u16 lfo_am_offset(u8 am_value) const;

	// helper to add values to the outputs based on channel enables
	void add_to_output(s32 *outputs, s32 value) const
	{
		if (RegisterType::OUTPUTS == 1 || m_regs.output0())
			outputs[0] += value;
		if (RegisterType::OUTPUTS >= 2 && m_regs.output1())
			outputs[1] += value;
		if (RegisterType::OUTPUTS >= 3 && m_regs.output2())
			outputs[2] += value;
		if (RegisterType::OUTPUTS >= 4 && m_regs.output3())
			outputs[3] += value;
	}

	// internal state
	s16 m_feedback[2];                    // feedback memory for operator 1
	mutable s16 m_feedback_in;            // next input value for op 1 feedback (set in output)
	ymfm_operator<RegisterType> *m_op[4]; // up to 4 operators
	RegisterType m_regs;                  // channel-specific registers
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
	void reset_lfo() { m_lfo_counter = 0; }

	// return the owning device
	device_t &device() const { return m_device; }

protected:
	// clock the LFO, updating m_lfo_am and return the signed PM value
	s8 clock_lfo();

	// clock the noise generator
	void clock_noise();

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
	u32 m_lfo_counter;               // LFO counter
	u32 m_noise_lfsr;                // noise LFSR state
	u8 m_noise_counter;              // noise counter
	u8 m_noise_state;                // latched noise state
	u8 m_noise_lfo;                  // latched LFO noise value
	u8 m_lfo_am;                     // current LFO AM value
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
	u8 m_regdata[RegisterType::REGISTERS]; // raw register data
};


// ======================> template instantiations

extern template class ymfm_engine_base<ymopm_registers>;
extern template class ymfm_engine_base<ymopn_registers>;
extern template class ymfm_engine_base<ymopna_registers>;
extern template class ymfm_engine_base<ymopl_registers>;
extern template class ymfm_engine_base<ymopl2_registers>;

using ymopm_engine = ymfm_engine_base<ymopm_registers>;
using ymopn_engine = ymfm_engine_base<ymopn_registers>;
using ymopna_engine = ymfm_engine_base<ymopna_registers>;
using ymopl_engine = ymfm_engine_base<ymopl_registers>;
using ymopl2_engine = ymfm_engine_base<ymopl2_registers>;


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
