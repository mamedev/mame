// license:BSD-3-Clause
// copyright-holders:Aaron Giles

#ifndef MAME_SOUND_YMOPN_H
#define MAME_SOUND_YMOPN_H

#pragma once

#include "ay8910.h"


// special naming helper to keep our namespace isolated from other
// same-named objects in the device's namespace
#define YMOPN_NAME(x) x, "opn." #x


// ======================> ymopn_registers

//
// OPN register map:
//
//      System-wide registers:
//           21 xxxxxxxx Test register
//           22 ----x--- LFO enable (OPNA, OPNB, OPNB2, OPN2)
//              -----xxx LFO rate (OPNA, OPNB, OPNB2, OPN2)
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
//              -----x-- Upper channel select (OPNA, OPNB, OPNB2, OPN2)
//              ------xx Channel select
//
//     Per-operator registers (channel in address bits 0-1, operator in bits 2-3)
//        30-3F -xxx---- Detune value (0-7)
//              ----xxxx Multiple value (0-15)
//        40-4F -xxxxxxx Total level (0-127)
//        50-5F xx------ Key scale rate (0-3)
//              ---xxxxx Attack rate (0-31)
//        60-6F x------- LFO AM enable (OPNA, OPNB, OPNB2, OPN2)
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
//        B4-B7 x------- Pan left (OPNA, OPNB, OPNB2, OPN2)
//              -x------ Pan right (OPNA, OPNB, OPNB2, OPN2)
//              --xx---- LFO AM shift (0-3)
//              -----xxx LFO PM depth (0-7)
//
//     Special multi-frequency registers (channel implicitly #2; operator in address bits 0-1)
//        A8-AB xxxxxxxx Frequency number lower 8 bits
//        AC-AF --xxx--- Block (0-7)
//              -----xxx Frequency number upper 3 bits
//
class ymopn_registers
{
	// private constructor to directly specify channel/operator bases
	ymopn_registers(ymopn_registers const &src, u16 chbase, u16 opbase = 0) :
		m_chbase(chbase),
		m_opbase(opbase),
		m_regdata(src.m_regdata)
	{
	}

public:
	// constructor
	ymopn_registers(std::vector<u8> &regdata) :
		m_chbase(0),
		m_opbase(0),
		m_regdata(regdata)
	{
	}

u8 opbase() const { return m_opbase; }
u8 chbase() const { return m_chbase; }
u8 *regbase() const { return &m_regdata[0]; }

	// direct read/write access
	u8 read(u16 index) { return m_regdata[index]; }
	void write(u16 index, u8 data) { m_regdata[index] = data; }

	// create a new version of ourself with a different channel/operator base
	ymopn_registers channel_registers(u8 chnum) { return ymopn_registers(*this, channel_offset(chnum)); }
	ymopn_registers operator_registers(u8 opnum) { return ymopn_registers(*this, m_chbase, m_chbase + operator_offset(opnum)); }

	// system-wide registers
	u8 test() const           /*  8 bits */ { return m_regdata[0x21]; }
	u8 lfo_enable() const     /*  3 bits */ { return BIT(m_regdata[0x22], 3); }
	u8 lfo_rate() const       /*  3 bits */ { return BIT(m_regdata[0x22], 0, 3); }
	u16 timer_a_value() const /* 10 bits */ { return (m_regdata[0x24] << 2) | BIT(m_regdata[0x25], 0, 2); }
	u8 timer_b_value() const  /*  8 bits */ { return m_regdata[0x26]; }
	u8 csm_multi_freq() const /*  2 bits */ { return BIT(m_regdata[0x27], 6, 2); }
	u8 reset_timer_b() const  /*  1 bit  */ { return BIT(m_regdata[0x27], 5); }
	u8 reset_timer_a() const  /*  1 bit  */ { return BIT(m_regdata[0x27], 4); }
	u8 enable_timer_b() const /*  1 bit  */ { return BIT(m_regdata[0x27], 3); }
	u8 enable_timer_a() const /*  1 bit  */ { return BIT(m_regdata[0x27], 2); }
	u8 load_timer_b() const   /*  1 bit  */ { return BIT(m_regdata[0x27], 1); }
	u8 load_timer_a() const   /*  1 bit  */ { return BIT(m_regdata[0x27], 0); }
	u8 keyon_states() const   /*  4 bits */ { return BIT(m_regdata[0x28], 4, 4); }
	u8 keyon_channel2() const /*  1 bit  */ { return BIT(m_regdata[0x28], 2); }
	u8 keyon_channel() const  /*  2 bits */ { return BIT(m_regdata[0x28], 0, 2); }

	// per-channel registers
	u16 block_fnum() const    /* 14 bits */ { return block_fnum(m_chbase + 0xa0, m_chbase + 0xa4); }
	u8 feedback() const       /*  3 bits */ { return BIT(m_regdata[m_chbase + 0xb0], 3, 3); }
	u8 algorithm() const      /*  3 bits */ { return BIT(m_regdata[m_chbase + 0xb0], 0, 3); }
	u8 pan_left() const       /*  1 bit  */ { return BIT(m_regdata[m_chbase + 0xb4], 7); }
	u8 pan_right() const      /*  1 bit  */ { return BIT(m_regdata[m_chbase + 0xb4], 6); }
	u8 am_shift() const       /*  2 bits */ { return BIT(m_regdata[m_chbase + 0xb4], 4, 2); }
	u8 pm_depth() const       /*  3 bits */ { return BIT(m_regdata[m_chbase + 0xb4], 0, 3); }

	// per-operator registers
	u8 detune() const         /*  3 bits */ { return BIT(m_regdata[m_opbase + 0x30], 4, 3); }
	u8 multiple() const       /*  4 bits */ { return BIT(m_regdata[m_opbase + 0x30], 0, 4); }
	u8 total_level() const    /*  8 bits */ { return BIT(m_regdata[m_opbase + 0x40], 0, 7); }
	u8 ksr() const            /*  2 bits */ { return BIT(m_regdata[m_opbase + 0x50], 6, 2); }
	u8 attack_rate() const    /*  5 bits */ { return BIT(m_regdata[m_opbase + 0x50], 0, 5); }
	u8 lfo_am_enable() const  /*  1 bit  */ { return BIT(m_regdata[m_opbase + 0x60], 7); }
	u8 decay_rate() const     /*  5 bits */ { return BIT(m_regdata[m_opbase + 0x60], 0, 5); }
	u8 sustain_rate() const   /*  5 bits */ { return BIT(m_regdata[m_opbase + 0x70], 0, 5); }
	u8 sustain_level() const  /*  4 bits */ { return BIT(m_regdata[m_opbase + 0x80], 4, 4); }
	u8 release_rate() const   /*  4 bits */ { return BIT(m_regdata[m_opbase + 0x80], 0, 4); }
	u8 ssg_eg_enabled() const /*  1 bit  */ { return BIT(m_regdata[m_opbase + 0x90], 3); }
	u8 ssg_eg_mode() const    /*  3 bits */ { return BIT(m_regdata[m_opbase + 0x90], 0, 3); }

	// multi-frequency registers
	u16 multi_block_fnum(u8 op) const /* 14 bits */ { return block_fnum(op + 0xa8, op + 0xac); }

	// special helper for generically getting the attack/decay/statain/release rates
	u8 adsr_rate(u8 state) const
	{
		// attack/decay/sustain are identical
		if (state < 3)
			return BIT(m_regdata[m_opbase + 0x50 + (state << 4)], 0, 5);

		// release encodes 4 bits and expands them
		else
			return 2 * BIT(m_regdata[m_opbase + 0x80], 0, 4) + 1;
	}

private:
	// return a 14-bit block/fnum from two registers
	u16 block_fnum(u16 index0, u16 index1) const
	{
		return m_regdata[index0] | (BIT(m_regdata[index1], 0, 6) << 8);
	}

	// convert a channel number into a register offset
	static u16 channel_offset(u8 chnum)
	{
		// channels 3-5 are accessed at +0x100
		u16 offset = 0;
		if (chnum >= 3)
		{
			offset = 0x100;
			chnum -= 3;
		}

		// channel number goes into the low 2 bits
		return offset | BIT(chnum, 0, 2);
	}

	// convert an operator number into a register offset
	static u8 operator_offset(u8 opnum)
	{
		// operator index is swizzled, and goes into bits 2 & 3
		return (BIT(opnum, 0) << 3) | (BIT(opnum, 1) << 2);
	}

	// internal state
	u16 m_chbase;                  // base offset for channel-specific data
	u16 m_opbase;                  // base offset for operator-specific data
	std::vector<u8> &m_regdata;    // reference to the raw data
};


// ======================> ymopn_operator

class ymopn_operator
{
	enum envelope_state : u8
	{
		ENV_ATTACK = 0,
		ENV_DECAY = 1,
		ENV_SUSTAIN = 2,
		ENV_RELEASE = 3
	};

public:
	// constructor
	ymopn_operator(ymopn_registers regs);

	// register for save states
	void save(device_t &device, u8 index);

	// reset the operator state
	void reset();

	// master clocking function
	void clock(u32 env_counter, u8 lfo_counter, u8 pm_depth, u16 block_fnum);

	// compute operator volume
	s16 compute_volume(u16 modulation, u8 am_offset) const;

	// key state control
	void keyonoff(u8 on) { m_keyon = on; }
	void keyon_csm() { m_csm_triggered = 1; }

private:
	// return the effective 6-bit rate after adjustments
	u8 effective_rate(u8 rawrate, u8 keycode);

	// start the attack phase
	void start_attack(u8 keycode);

	// start the release phase
	void start_release();

	// clock phases
	void clock_keystate(u8 keystate, u8 keycode);
	void clock_ssg_eg_state(u8 keycode);
	void clock_envelope(u16 env_counter, u8 keycode);
	void clock_phase(u8 lfo_counter, u8 pm_depth, u16 block_fnum);

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
	ymopn_registers m_regs;          // operator-specific registers
};


// ======================> ymopn_channel

class ymopn_channel
{
public:
	// constructor
	ymopn_channel(ymopn_registers regs);

	// register for save states
	void save(device_t &device, u8 index);

	// reset the channel state
	void reset();

	// signal key on/off to our operators
	void keyonoff(u8 states);

	// signal CSM key on to our operators
	void keyon_csm();

	// master clocking function
	void clock(u32 env_counter, u8 lfo_counter, bool multi_freq);

	// compute the channel output and add to the left/right output sums
	void output(u8 lfo_counter, s32 &lsum, s32 &rsum, u8 rshift, s16 clipmax) const;

private:
	// internal state
	mutable s16 m_feedback[3];       // feedback memory for operator 1
	ymopn_operator m_op1;            // operator 1
	ymopn_operator m_op2;            // operator 2
	ymopn_operator m_op3;            // operator 3
	ymopn_operator m_op4;            // operator 4
	ymopn_registers m_regs;          // channel-specific registers
};


// ======================> ymopn_engine

class ymopn_engine
{
public:
	enum : u8
	{
		STATUS_TIMERA = 0x01,
		STATUS_TIMERB = 0x02,
		STATUS_BUSY = 0x80
	};

	// constructor
	ymopn_engine(device_t &device, bool six_channels);

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
	void set_reset_status(u8 set, u8 reset) { m_status = (m_status | set) & ~reset; check_interrupts(); }

	// set the IRQ mask
	void set_irq_mask(u8 mask) { m_irq_mask = mask; check_interrupts(); }

	// set the busy flag in the status register
	void set_busy();

	// return the current clock prescale
	u8 clock_prescale() const { return m_clock_prescale; }

	// set prescale factor (2/3/6)
	void set_clock_prescale(u8 prescale) { m_clock_prescale = prescale; }

	// configuration helpers
	auto irq_handler() { return m_irq_handler.bind(); }

private:
	// update the state of the given timer
	void update_timer(u8 which, u8 enable);

	// timer callback
	TIMER_CALLBACK_MEMBER(timer_handler);

	// check interrupts
	void check_interrupts();

	// internal state
	device_t &m_device;              // reference to the owning device
	u32 m_env_counter;               // envelope counter; low 2 bits are sub-counter
	u8 m_lfo_subcounter;             // LFO sub-counter
	u8 m_lfo_counter;                // LFO counter
	u8 m_status;                     // current status register
	u8 m_clock_prescale;             // prescale factor (2/3/6)
	u8 m_irq_mask;                   // mask of which bits signal IRQs
	u8 m_irq_state;                  // current IRQ state
	attotime m_busy_end;             // end of the busy time
	emu_timer *m_timer[2];           // our two timers
	devcb_write_line m_irq_handler;  // IRQ callback
 	std::vector<std::unique_ptr<ymopn_channel>> m_channel; // channel pointers
	std::vector<u8> m_regdata;       // raw register data
	u8 m_fnum_latches[16];           // latches for fnum data
	ymopn_registers m_regs;          // register accessor
};

#endif // MAME_SOUND_YMOPN_H
