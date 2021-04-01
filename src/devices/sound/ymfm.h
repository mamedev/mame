// license:BSD-3-Clause
// copyright-holders:Aaron Giles

#ifndef MAME_SOUND_YMFM_H
#define MAME_SOUND_YMFM_H

#pragma once

//
// Implementation notes:
//
//
// REGISTER CLASSES
//
// OPM and OPN are very closely related, and thus share a common engine
// and implementation. Differentiation is provided by the various registers
// classes, which are specified as template parameters to the shared
// implementation.
//
// There are currently three register classes:
//
//    ymopm_registers: OPM (YM2151)
//    ymopn_registers: OPN (YM2203)
//    ymopna_registers: OPNA (YM2608) / OPNB (YM2610/B) / OPN2 (YM2612/YM3438)
//
//
// FREQUENCIES
//
// One major difference between OPM and OPN is in how frequencies are
// specified. OPM specifies frequency via a 3-bit 'block' (aka octave),
// combined with a 4-bit 'key code' (note number) and a 6-bit 'key
// fraction'. The key code and fraction are converted on the chip
// into an x.11 fixed-point value and then shifted by the block to
// produce the final step value for the phase.
//
// OPN, on the other hand, specifies frequencies via a 3-bit 'block'
// just as on OPM, but combined with an 11-bit 'frequency number' or
// 'fnum', which is directly shifted by the block to produce the step
// value. So essentially, OPN makes the user do the conversion from
// note value to phase increment, while OPM is programmed in a more
// 'musical' way, specifying notes and cents.
//
// Interally, this is abstracted away into a 'block_freq' value,
// which is a 16-bit value containing the block and frequency info
// concatenated together as follows:
//
//    OPM: [3-bit block]:[4-bit keycode]:[6-bit fraction] = 13 bits total
//
//    OPN: [3-bit block]:[11-bit fnum] = 14 bits total
//
// Template specialization in functions that interpret the 'block_freq'
// value is used to deconstruct it appropriately (specifically, see
// clock_phase).
//
//
// LOW FREQUENCY OSCILLATOR (LFO)
//
// The LFO engines are different in several key ways. The OPM LFO
// engine is fairly intricate. It has a 4.4 floating-point rate which
// allows for a huge range of frequencies, and can select between four
// different waveforms (sawtooth, square, triangle, or noise). Separate
// 7-bit depth controls for AM and PM control the amount of modulation
// applied in each case. This global LFO value is then further controlled
// at the channel level by a 2-bit AM sensitivity and a 3-bit PM
// sensitivity, and each operator has a 1-bit AM on/off switch.
//
// For OPN the LFO engine was removed entirely, but a limited version
// was put back in OPNA and later chips. This stripped-down version
// offered only a 3-bit rate setting (versus the 4.4 floating-point rate
// in OPN), and no depth control. It did bring back the channel-level
// sensitivity controls and the operator-level on/off control.
//


//*********************************************************
//  MACROS
//*********************************************************

// special naming helper to keep our namespace isolated from other
// same-named objects in the device's namespace (mostly necessary
// for chips which derive from AY-8910 classes and may have clashing
// names)
#define YMFM_NAME(x) x, "ymfm." #x



//*********************************************************
//  REGISTER CLASSES
//*********************************************************

// ======================> ymfm_registers_base

class ymfm_registers_base
{
protected:
	// constructor
	ymfm_registers_base(std::vector<u8> &regdata, u16 chbase = 0, u16 opbase = 0) :
		m_chbase(chbase),
		m_opbase(opbase),
		m_regdata(regdata)
	{
	}

public:
	// system-wide registers that aren't universally supported
	u8 noise_frequency() const    /*  5 bits */ { return 0; } // not on OPN,OPNA
	u8 noise_enabled() const      /*  1 bit  */ { return 0; } // not on OPN,OPNA
	u8 lfo_enabled() const        /*  1 bit  */ { return 0; } // not on OPM,OPN
	u8 lfo_rate() const           /*3-8 bits */ { return 0; } // not on OPN
	u8 lfo_waveform() const       /*  2 bits */ { return 0; } // not on OPN,OPNA
	u8 lfo_pm_depth() const       /*  7 bits */ { return 0; } // not on OPN,OPNA
	u8 lfo_am_depth() const       /*  7 bits */ { return 0; } // not on OPN,OPNA
	u8 multi_freq() const         /*  1 bit  */ { return 0; } // not on OPM
	u16 multi_block_freq0() const /* 14 bits */ { return 0; } // not on OPM
	u16 multi_block_freq1() const /* 14 bits */ { return 0; } // not on OPM
	u16 multi_block_freq2() const /* 14 bits */ { return 0; } // not on OPM

	// per-channel registers that aren't universally supported
	u8 pan_right() const          /*  1 bit  */ { return 1; } // not on OPN
	u8 pan_left() const           /*  1 bit  */ { return 1; } // not on OPN
	u8 lfo_pm_sensitivity() const /*  3 bits */ { return 0; } // not on OPN
	u8 lfo_am_sensitivity() const /*  2 bits */ { return 0; } // not on OPN

	// per-operator registers that aren't universally supported
	u8 lfo_am_enabled() const     /*  1 bit  */ { return 0; } // not on OPN
	u8 detune2() const            /*  2 bits */ { return 0; } // not on OPN,OPN2
	u8 ssg_eg_enabled() const     /*  1 bit  */ { return 0; } // not on OPM
	u8 ssg_eg_mode() const        /*  1 bit  */ { return 0; } // not on OPM

protected:
	// return a bitfield extracted from a byte
	u8 sysbyte(u16 offset, u8 start, u8 count) const
	{
		return BIT(m_regdata[offset], start, count);
	}
	u8 chbyte(u16 offset, u8 start, u8 count) const { return sysbyte(offset + m_chbase, start, count); }
	u8 opbyte(u16 offset, u8 start, u8 count) const { return sysbyte(offset + m_opbase, start, count); }

	// return a bitfield extracted from a pair of bytes, MSBs listed first
	u16 sysword(u16 offset1, u8 start1, u8 count1, u16 offset2, u8 start2, u8 count2) const
	{
		return (sysbyte(offset1, start1, count1) << count2) | sysbyte(offset2, start2, count2);
	}
	u16 chword(u16 offset1, u8 start1, u8 count1, u16 offset2, u8 start2, u8 count2) const { return sysword(offset1 + m_chbase, start1, count1, offset2 + m_chbase, start2, count2); }

	// internal state
	u16 m_chbase;                  // base offset for channel-specific data
	u16 m_opbase;                  // base offset for operator-specific data
	std::vector<u8> &m_regdata;    // reference to the raw data
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

class ymopm_registers : public ymfm_registers_base
{
public:
	// constants
	static constexpr u8 DEFAULT_PRESCALE = 2;
	static constexpr u8 CHANNELS = 8;
	static constexpr u8 CSM_TRIGGER_MASK = 0xff;
	static constexpr u16 REGISTERS = 0x100;
	static constexpr u16 REG_MODE = 0x14;
	static constexpr u16 REG_KEYON = 0x08;

	// constructor
	ymopm_registers(std::vector<u8> &regdata, u16 chbase = 0, u16 opbase = 0) :
		ymfm_registers_base(regdata, chbase, opbase)
	{
	}

	// return channel/operator number
	u8 chnum() const { return BIT(m_chbase, 0, 3); }
	u8 opnum() const { return BIT(m_opbase, 4) | (BIT(m_opbase, 3) << 1); }

	// reset state to default values
	void reset()
	{
		// enable output on both channels by default
		m_regdata[0x20] = m_regdata[0x21] = m_regdata[0x22] = m_regdata[0x23] = 0xc0;
		m_regdata[0x24] = m_regdata[0x25] = m_regdata[0x26] = m_regdata[0x27] = 0xc0;
	}

	// write access
	void write(u16 index, u8 data)
	{
		// LFO AM/PM depth are written to the same register (0x19);
		// redirect the PM depth to an unused neighbor (0x1a)
		if (index == 0x19)
			m_regdata[index + BIT(data, 7)] = data;
		else if (index != 0x1a)
			m_regdata[index] = data;
	}

	// create a new version of ourself with a different channel/operator base
	ymopm_registers channel_registers(u8 chnum) { return ymopm_registers(m_regdata, channel_offset(chnum)); }
	ymopm_registers operator_registers(u8 opnum) { return ymopm_registers(m_regdata, m_chbase, m_chbase + operator_offset(opnum)); }

	// system-wide registers
	u8 test() const               /*  8 bits */ { return sysbyte(0x01, 0, 8); }
	u8 keyon_states() const       /*  4 bits */ { return sysbyte(0x08, 3, 4); }
	u8 keyon_channel() const      /*  3 bits */ { return sysbyte(0x08, 0, 3); }
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
	u8 pan_right() const          /*  1 bit  */ { return chbyte(0x20, 7, 1); }
	u8 pan_left() const           /*  1 bit  */ { return chbyte(0x20, 6, 1); }
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

	// LFO is always enabled
	u8 lfo_enabled() const { return 1; }

	// special helper for generically getting the attack/decay/statain/release rates
	u8 adsr_rate(u8 state) const
	{
		// attack/decay/sustain are identical
		if (state < 3)
			return opbyte(0x80 + (state << 5), 0, 5);

		// release encodes 4 bits and expands them
		else
			return opbyte(0xe0, 0, 4) * 2 + 1;
	}

protected:
	// convert a channel number into a register offset; channel goes into the low 3 bits
	static constexpr u8 channel_offset(u8 chnum) { return BIT(chnum, 0, 3); }

	// convert an operator number into a register offset; operator goes into bits 3-4
	static constexpr u8 operator_offset(u8 opnum) { return (BIT(opnum, 0) << 4) | (BIT(opnum, 1) << 3); }
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

class ymopn_registers : public ymfm_registers_base
{
public:
	// constants
	static constexpr u8 DEFAULT_PRESCALE = 6;
	static constexpr u8 CHANNELS = 3;
	static constexpr u8 CSM_TRIGGER_MASK = 1 << 2;
	static constexpr u16 REGISTERS = 0x100;
	static constexpr u16 REG_MODE = 0x27;
	static constexpr u16 REG_KEYON = 0x28;

	// constructor
	ymopn_registers(std::vector<u8> &regdata, u16 chbase = 0, u16 opbase = 0) :
		ymfm_registers_base(regdata, chbase, opbase)
	{
	}

	// return channel/operator number
	u8 chnum() const { return BIT(m_chbase, 0, 2); }
	u8 opnum() const { return BIT(m_opbase, 3) | (BIT(m_opbase, 2) << 1); }

	// reset state to default values
	void reset()
	{
	}

	// write access
	void write(u16 index, u8 data)
	{
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

	// create a new version of ourself with a different channel/operator base
	ymopn_registers channel_registers(u8 chnum) { return ymopn_registers(m_regdata, channel_offset(chnum)); }
	ymopn_registers operator_registers(u8 opnum) { return ymopn_registers(m_regdata, m_chbase, m_chbase + operator_offset(opnum)); }

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
	u8 keyon_states() const       /*  4 bits */ { return sysbyte(0x28, 4, 4); }
	u8 keyon_channel() const      /*  2 bits */ { return sysbyte(0x28, 0, 2); }
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

	// special helper for generically getting the attack/decay/statain/release rates
	u8 adsr_rate(u8 state) const
	{
		// attack/decay/sustain are identical
		if (state < 3)
			return opbyte(0x50 + (state << 4), 0, 5);

		// release encodes 4 bits and expands them
		else
			return opbyte(0x80, 0, 4) * 2 + 1;
	}

protected:
	// convert a channel number into a register offset; channel goes in low 2 bits
	static constexpr u16 channel_offset(u8 chnum) { return BIT(chnum, 0, 2); }

	// convert an operator number into a register offset; operator goes into bits 2-3
	static constexpr u8 operator_offset(u8 opnum) { return (BIT(opnum, 0) << 3) | (BIT(opnum, 1) << 2); }
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

class ymopna_registers : public ymopn_registers
{
public:
	// constants
	static constexpr u8 CHANNELS = 6;
	static constexpr u16 REGISTERS = 0x200;

	// constructor
	ymopna_registers(std::vector<u8> &regdata, u16 chbase = 0, u16 opbase = 0) :
		ymopn_registers(regdata, chbase, opbase)
	{
	}

	// return channel/operator number
	u8 chnum() const { return BIT(m_chbase, 0, 2) + 3 * BIT(m_chbase, 8); }

	// reset state to default values
	void reset()
	{
		// enable output on both channels by default
		m_regdata[0xb4] = m_regdata[0xb5] = m_regdata[0xb6] = 0xc0;
		m_regdata[0x1b4] = m_regdata[0x1b5] = m_regdata[0x1b6] = 0xc0;
	}

	// create a new version of ourself with a different channel/operator base
	ymopna_registers channel_registers(u8 chnum) { return ymopna_registers(m_regdata, channel_offset(chnum)); }
	ymopna_registers operator_registers(u8 opnum) { return ymopna_registers(m_regdata, m_chbase, m_chbase + operator_offset(opnum)); }

	// OPNA-specific system-wide registers
	u8 lfo_enabled() const        /*  3 bits */ { return sysbyte(0x22, 3, 1); }
	u8 lfo_rate() const           /*  3 bits */ { return sysbyte(0x22, 0, 3); }
	u8 keyon_channel() const      /*  3 bits */
	{
		// ensure that both 3 and 7 return out-of-range values
		u8 temp = sysbyte(0x28, 0, 3);
		return (temp == 3) ? 6 : temp - BIT(temp, 2);
	}

	// OPNA-specific per-channel registers
	u8 pan_left() const           /*  1 bit  */ { return chbyte(0xb4, 7, 1); }
	u8 pan_right() const          /*  1 bit  */ { return chbyte(0xb4, 6, 1); }
	u8 lfo_am_sensitivity() const /*  2 bits */ { return chbyte(0xb4, 4, 2); }
	u8 lfo_pm_sensitivity() const /*  3 bits */ { return chbyte(0xb4, 0, 3); }

	// OPNA-specific per-operator registers
	u8 lfo_am_enabled() const     /*  1 bit  */ { return opbyte(0x60, 7, 1); }

protected:
	// convert a channel number into a register offset
	static constexpr u16 channel_offset(u8 chnum) { return chnum % 3 + ((chnum / 3) << 8); }
};



//*********************************************************
//  CORE ENGINE CLASSES
//*********************************************************

// ======================> ymfm_operator

template<class RegisterType>
class ymfm_operator
{
	enum envelope_state : u8
	{
		ENV_ATTACK = 0,
		ENV_DECAY = 1,
		ENV_SUSTAIN = 2,
		ENV_RELEASE = 3
	};
	static constexpr u16 ENV_QUIET = 0x200;

public:
	// constructor
	ymfm_operator(RegisterType regs);

	// register for save states
	void save(device_t &device, u8 index);

	// reset the operator state
	void reset();

	// master clocking function
	void clock(u32 env_counter, s8 lfo_raw_pm, u16 block_freq);

	// compute operator volume
	s16 compute_volume(u16 modulation, u16 am_offset) const;

	// compute volume for the OPM noise channel
	s16 compute_noise_volume(u8 noise_state, u16 am_offset) const;

	// key state control
	void keyonoff(u8 on) { m_keyon = on; }
	void keyon_csm() { m_csm_triggered = 1; }

	// are we active?
	bool active() const { return (m_env_state != ENV_RELEASE || m_env_attenuation < ENV_QUIET); }

private:
	// convert the generic block_freq into a 5-bit keycode
	u8 block_freq_to_keycode(u16 block_freq);

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
	void clock_phase(s8 lfo_raw_pm, u16 block_freq);

	// return effective attenuation of the envelope
	u16 envelope_attenuation(u8 am_offset) const;

	// internal state
	u32 m_phase;                     // current phase value (10.10 format)
	u16 m_env_attenuation;           // computed envelope attenuation (4.6 format)
	envelope_state m_env_state;      // current envelope state
	u8 m_ssg_inverted;               // non-zero if the output should be inverted (bit 0)
	u8 m_key_state;                  // current key state: on or off (bit 0)
	u8 m_keyon;                      // live key on state (bit 0)
	u8 m_csm_triggered;              // true if a CSM key on has been triggered (bit 0)
	RegisterType m_regs;             // operator-specific registers
};

template<>
u8 ymfm_operator<ymopm_registers>::block_freq_to_keycode(u16 block_freq);

template<>
void ymfm_operator<ymopm_registers>::clock_phase(s8 lfo_raw_pm, u16 block_freq);


// ======================> ymfm_channel

template<class RegisterType>
class ymfm_channel
{
public:
	// constructor
	ymfm_channel(RegisterType regs);

	// register for save states
	void save(device_t &device, u8 index);

	// reset the channel state
	void reset();

	// signal key on/off to our operators
	void keyonoff(u8 states);

	// signal CSM key on to our operators
	void keyon_csm();

	// master clocking function
	void clock(u32 env_counter, s8 lfo_raw_pm, bool is_multi_freq);

	// compute the channel output and add to the left/right output sums
	void output(u8 lfo_raw_am, u8 noise_state, s32 &lsum, s32 &rsum, u8 rshift, s16 clipmax) const;

	// is this channel active?
	bool active() const { return m_op1.active() || m_op2.active() || m_op3.active() || m_op4.active(); }

private:
	// convert a 6/8-bit raw AM value into an amplitude offset based on sensitivity
	u16 lfo_am_offset(u8 am_value) const;

	// internal state
	s16 m_feedback[2];                    // feedback memory for operator 1
	mutable s16 m_feedback_in;            // next input value for op 1 feedback (set in output)
	ymfm_operator<RegisterType> m_op1;    // operator 1
	ymfm_operator<RegisterType> m_op2;    // operator 2
	ymfm_operator<RegisterType> m_op3;    // operator 3
	ymfm_operator<RegisterType> m_op4;    // operator 4
	RegisterType m_regs;                  // channel-specific registers
};

template<>
u16 ymfm_channel<ymopm_registers>::lfo_am_offset(u8 lfo_raw_am) const;


// ======================> ymfm_engine_base

template<class RegisterType>
class ymfm_engine_base
{
public:
	enum : u8
	{
		STATUS_TIMERA = 0x01,
		STATUS_TIMERB = 0x02,
		STATUS_BUSY = 0x80
	};

	// constructor
	ymfm_engine_base(device_t &device);

	// register for save states
	void save(device_t &device);

	// reset the overall state
	void reset();

	// master clocking function
	u32 clock(u8 chanmask);

	// compute sum of channel outputs
	void output(s32 &lsum, s32 &rsum, u8 rshift, s16 clipmax, u8 chanmask) const;

	// write to the OPN registers
	void write(u16 regnum, u8 data);

	// return the current status
	u8 status() const;

	// set/reset bits in the status register, updating the IRQ status
	void set_reset_status(u8 set, u8 reset) { m_status = (m_status | set) & ~reset; schedule_check_interrupts(); }

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

	// configuration helpers
	auto irq_handler() { return m_irq_handler.bind(); }

	// reset the LFO state
	void reset_lfo() { m_lfo_counter = 0; }

private:
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
	u32 m_active_channels;           // mask of active channels (computed by prepare)
	u32 m_modified_channels;         // mask of channels that have been modified
	u32 m_prepare_count;             // counter to do periodic prepare sweeps
	attotime m_busy_end;             // end of the busy time
	emu_timer *m_timer[2];           // our two timers
	devcb_write_line m_irq_handler;  // IRQ callback
	std::unique_ptr<ymfm_channel<RegisterType>> m_channel[RegisterType::CHANNELS]; // channel pointers
	std::vector<u8> m_regdata;       // raw register data
	RegisterType m_regs;             // register accessor
};

template<>
s8 ymfm_engine_base<ymopm_registers>::clock_lfo();

template<>
void ymfm_engine_base<ymopm_registers>::clock_noise();


// ======================> template instantiations

extern template class ymfm_engine_base<ymopm_registers>;
extern template class ymfm_engine_base<ymopn_registers>;
extern template class ymfm_engine_base<ymopna_registers>;

using ymopm_engine = ymfm_engine_base<ymopm_registers>;
using ymopn_engine = ymfm_engine_base<ymopn_registers>;
using ymopna_engine = ymfm_engine_base<ymopna_registers>;

#endif // MAME_SOUND_YMFM_H
