// license:BSD-3-Clause
// copyright-holders:Aaron Giles

#ifndef MAME_SOUND_YMFM_H
#define MAME_SOUND_YMFM_H

#pragma once


namespace ymfm
{


//*********************************************************
//  INTERFACE CLASSES
//*********************************************************

// forward declarations
template<class RegisterType> class fm_engine_base;

class fm_engine_callbacks
{
public:
	// timer callback; called by the interface when a timer fires
	virtual void intf_timer_expired(uint32_t tnum) = 0;

	// check interrupts; called by the interface after synchronization
	virtual void intf_check_interrupts() = 0;

	// mode register write; called by the interface after synchronization
	virtual void intf_mode_write(uint8_t data) = 0;
};


class fm_interface
{
	template<typename RegisterType> friend class fm_engine_base;

public:
	// constructor
	fm_interface() { }

	// destructor
	virtual ~fm_interface() { }

	// save types:
	//  uint8_t
	//  uint8_t[]
	//  uint32_t
	//  unique_ptr<fm_operator>[]
	//  unique_ptr<fm_channel>[]

	template<typename T>
	void save_item(T &item, char const *name, int index = 0)
	{
	}

	template<typename... Params>
	void log(char const *fmt, Params &&... args)
	{
//		LOG("%s: ", m_device.tag());
//		LOG(fmt, std::forward<Params>(args)...);
	}

protected:
	// the engine calls this to set one of two timers which should fire after
	// the given number of clocks; when the timer fires, it must call the
	// m_callbacks->intf_timer_handler() method; a duration_in_clocks that is
	// negative means to disable the timer
	virtual void set_timer(uint32_t tnum, int32_t duration_in_clocks) = 0;

	// the engine calls this when the external IRQ state changes; in
	// response, the interface should perform any IRQ signaling that needs
	// to happen as a result
	virtual void update_irq(bool asserted) = 0;

	// the engine calls this to schedule a synchronized write to the mode
	// register with the provided data; in response, the interface should
	// clock through any unprocessed stream data and then call the
	// m_callbacks->intf_mode_write() method with the provided data
	virtual void synchronized_mode_write(uint8_t data) = 0;

	// the engine calls this to schedule a synchronized interrupt check;
	// in response, the interface should clock through any unprocessed
	// stream data and then call the m_callbacks->intf_check_interrupts() method
	virtual void synchronized_check_interrupts() = 0;

	// internal state
	fm_engine_callbacks *m_callbacks;
};

}



// ======================> fm_interface

// this class abstracts out system-dependent behaviors and interfaces
#ifdef __EMU_H__
class mame_fm_interface : public ymfm::fm_interface
{
public:
	// construction
	mame_fm_interface(device_t &device) :
		m_device(device),
		m_irq_handler(device)
	{
	}

	// startup; this is not called by the engine, but by the owner of the
	// interface, at device_start time
	void start()
	{
		// allocate our timers
		for (int tnum = 0; tnum < 2; tnum++)
			m_timer[tnum] = m_device.machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(mame_fm_interface::timer_handler), this));

		// resolve the IRQ handler while we're here
		m_irq_handler.resolve();
	}

	// configuration helpers
	auto irq_handler() { return m_irq_handler.bind(); }

	// set a timer to go off after the given number of clocks; when it fires,
	// it must call timer_handler on the target object
	virtual void set_timer(uint32_t tnum, int32_t duration_in_clocks) override
	{
		if (duration_in_clocks >= 0)
			m_timer[tnum]->adjust(attotime::from_ticks(duration_in_clocks, m_device.clock()), tnum);
		else
			m_timer[tnum]->enable(false);
	}

	virtual void update_irq(bool asserted) override
	{
		m_irq_handler(asserted ? ASSERT_LINE : CLEAR_LINE);
	}

	virtual void synchronized_mode_write(uint8_t data) override
	{
		m_device.machine().scheduler().synchronize(timer_expired_delegate(FUNC(mame_fm_interface::mode_write), this), data);
	}

	virtual void synchronized_check_interrupts() override
	{
		// if we're currently executing a CPU, schedule the interrupt check;
		// otherwise, do it directly
		auto &scheduler = m_device.machine().scheduler();
		if (scheduler.currently_executing())
			scheduler.synchronize(timer_expired_delegate(FUNC(mame_fm_interface::check_interrupts), this));
		else
			m_callbacks->intf_check_interrupts();
	}

	template<typename T>
	void save_item(T &item, char const *name, int index = 0)
	{
		m_device.save_item(item, name, index);
	}

	template<typename... Params>
	void log(char const *fmt, Params &&... args)
	{
//		LOG("%s: ", m_device.tag());
//		LOG(fmt, std::forward<Params>(args)...);
	}

private:
	// timer handler
	void timer_handler(void *ptr, int param)
	{
		m_callbacks->intf_timer_expired(param);
	}

	void mode_write(void *ptr, int param)
	{
		m_callbacks->intf_mode_write(param);
	}

	void check_interrupts(void *ptr, int param)
	{
		m_callbacks->intf_check_interrupts();
	}

	device_t &m_device;
	emu_timer *m_timer[2];
	devcb_write_line m_irq_handler;
};
#endif



namespace ymfm
{


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

enum envelope_state : uint32_t
{
	EG_DEPRESS = 0,
	EG_ATTACK = 1,
	EG_DECAY = 2,
	EG_SUSTAIN = 3,
	EG_RELEASE = 4,
	EG_STATES = 5
};


//*********************************************************
//  GLOBAL HELPERS
//*********************************************************

//-------------------------------------------------
//  bitfield - extract a bitfield from the given
//  value, starting at bit 'start' for a length of
//  'length' bits
//-------------------------------------------------

inline uint32_t bitfield(uint32_t value, int start, int length = 1)
{
	return (value >> start) & ((1 << length) - 1);
}


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
//  encode_fp - given a 32-bit signed input value
//  convert it to a signed 3.10 floating-point
//  value
//-------------------------------------------------

inline int16_t encode_fp(int32_t value)
{
	// handle overflows first
	if (value < -32768)
		return (7 << 10) | 0x000;
	if (value > 32767)
		return (7 << 10) | 0x3ff;

	// we need to count the number of leading sign bits after the sign
	// we can use count_leading_zeros if we invert negative values
	int32_t scanvalue = value ^ (int32_t(value) >> 31);

	// exponent is related to the number of leading bits starting from bit 14
	int exponent = 7 - count_leading_zeros(scanvalue << 17);

	// smallest exponent value allowed is 1
	exponent = std::max(exponent, 1);

	// mantissa
	int32_t mantissa = value >> (exponent - 1);

	// assemble into final form, inverting the sign
	return ((exponent << 10) | (mantissa & 0x3ff)) ^ 0x200;
}


//-------------------------------------------------
//  decode_fp - given a 3.10 floating-point value,
//  convert it to a signed 16-bit value
//-------------------------------------------------

inline int16_t decode_fp(int16_t value)
{
	// invert the sign and the exponent
	value ^= 0x1e00;

	// shift mantissa up to 16 bits then apply inverted exponent
	return int16_t(value << 6) >> bitfield(value, 10, 3);
}


//-------------------------------------------------
//  roundtrip_fp - compute the result of a round
//  trip through the encode/decode process above
//-------------------------------------------------

inline int16_t roundtrip_fp(int32_t value)
{
	// handle overflows first
	if (value < -32768)
		return -32768;
	if (value > 32767)
		return 32767;

	// we need to count the number of leading sign bits after the sign
	// we can use count_leading_zeros if we invert negative values
	int32_t scanvalue = value ^ (int32_t(value) >> 31);

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


// ======================> opdata_cache

// this class holds data that is computed once at the start of clocking
// and remains static during subsequent sound generation
struct opdata_cache
{
	// set phase_step to this value to recalculate it each sample; needed
	// in the case of PM LFO changes
	static constexpr uint32_t PHASE_STEP_DYNAMIC = 1;

	uint16_t const *waveform;         // base of sine table
	uint32_t phase_step;              // phase step, or PHASE_STEP_DYNAMIC if PM is active
	uint32_t total_level;             // total level * 8 + KSL
	uint32_t block_freq;              // raw block frequency value (used to compute phase_step)
	int32_t detune;                  // detuning value (used to compute phase_step)
	uint32_t multiple;                // multiple value (x.1, used to compute phase_step)
	uint32_t eg_sustain;              // sustain level, shifted up to envelope values
	uint8_t eg_rate[EG_STATES];       // envelope rate, including KSR
};


// ======================> fm_registers_base

// base class for family-specific register classes; this provides a few
// constants, common defaults, and helpers, but mostly each derived
// class is responsible for defining all commonly-called methods
class fm_registers_base
{
public:
	// this value is returned from the write() function for rhythm channels
	static constexpr uint32_t RHYTHM_CHANNEL = 0xff;

	// this is the size of a full sin waveform
	static constexpr uint32_t WAVEFORM_LENGTH = 0x400;

	//
	// the following constants need to be defined per family:
	//          uint32_t OUTPUTS: The number of outputs exposed (1-4)
	//         uint32_t CHANNELS: The number of channels on the chip
	//     uint32_t ALL_CHANNELS: A bitmask of all channels
	//        uint32_t OPERATORS: The number of operators on the chip
	//     bool DYNAMIC_OPS: True if ops/channel can be changed at runtime
	//        uint32_t WAVEFORMS: The number of waveforms offered
	//        uint32_t REGISTERS: The number of 8-bit registers allocated
	//         uint32_t REG_MODE: The address of the "mode" register controlling timers
	// uint32_t DEFAULT_PRESCALE: The starting clock prescale
	// uint32_t EG_CLOCK_DIVIDER: The clock divider of the envelope generator
	//  bool EG_HAS_DEPRESS: True if the chip has a DP ("depress"?) envelope stage
	//      bool EG_HAS_SSG: True if the chip has SSG envelope support
	// bool MODULATOR_DELAY: True if the modulator is delayed by 1 sample (OPL pre-OPL3)
	// uint32_t CSM_TRIGGER_MASK: Mask of channels to trigger in CSM mode
	//     uint8_t STATUS_TIMERA: Status bit to set when timer A fires
	//     uint8_t STATUS_TIMERB: Status bit to set when tiemr B fires
	//       uint8_t STATUS_BUSY: Status bit to set when the chip is busy
	//        uint8_t STATUS_IRQ: Status bit to set when an IRQ is signalled
	//

	// system-wide register defaults
	uint32_t status_mask() const                { return 0; } // OPL only
	uint32_t irq_reset() const                  { return 0; } // OPL only
	uint32_t noise_enable() const               { return 0; } // OPM only
	uint32_t rhythm_enable() const              { return 0; } // OPL only

	// per-operator register defaults
	uint32_t op_ssg_eg_enable(uint32_t opoffs) const { return 0; } // OPN(A) only
	uint32_t op_ssg_eg_mode(uint32_t opoffs) const   { return 0; } // OPN(A) only

protected:
	// helper to encode four operator numbers into a 32-bit value in the
	// operator maps for each register class
	static constexpr uint32_t operator_list(uint8_t o1 = 0xff, uint8_t o2 = 0xff, uint8_t o3 = 0xff, uint8_t o4 = 0xff)
	{
		return o1 | (o2 << 8) | (o3 << 16) | (o4 << 24);
	}

	// helper to apply KSR to the raw ADSR rate, ignoring ksr if the
	// raw value is 0, and clamping to 63
	static constexpr uint32_t effective_rate(uint32_t rawrate, uint32_t ksr)
	{
		return (rawrate == 0) ? 0 : std::min<uint32_t>(rawrate + ksr, 63);
	}
};


// ======================> opm_registers

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

class opm_registers : public fm_registers_base
{
	// LFO waveforms are 256 entries long
	static constexpr uint32_t LFO_WAVEFORM_LENGTH = 256;

public:
	// constants
	static constexpr uint32_t OUTPUTS = 2;
	static constexpr uint32_t CHANNELS = 8;
	static constexpr uint32_t ALL_CHANNELS = (1 << CHANNELS) - 1;
	static constexpr uint32_t OPERATORS = CHANNELS * 4;
	static constexpr bool DYNAMIC_OPS = false;
	static constexpr uint32_t WAVEFORMS = 1;
	static constexpr uint32_t REGISTERS = 0x100;
	static constexpr uint32_t REG_MODE = 0x14;
	static constexpr uint32_t DEFAULT_PRESCALE = 2;
	static constexpr uint32_t EG_CLOCK_DIVIDER = 3;
	static constexpr bool EG_HAS_DEPRESS = false;
	static constexpr bool EG_HAS_SSG = false;
	static constexpr bool MODULATOR_DELAY = false;
	static constexpr uint32_t CSM_TRIGGER_MASK = ALL_CHANNELS;
	static constexpr uint8_t STATUS_TIMERA = 0x01;
	static constexpr uint8_t STATUS_TIMERB = 0x02;
	static constexpr uint8_t STATUS_BUSY = 0x80;
	static constexpr uint8_t STATUS_IRQ = 0;

	// constructor
	opm_registers();

	// register for save states
	void save(fm_interface &intf);

	// reset to initial state
	void reset();

	// map channel number to register offset
	static constexpr uint32_t channel_offset(uint32_t chnum)
	{
		assert(chnum < CHANNELS);
		return chnum;
	}

	// map operator number to register offset
	static constexpr uint32_t operator_offset(uint32_t opnum)
	{
		assert(opnum < OPERATORS);
		return opnum;
	}

	// return an array of operator indices for each channel
	struct operator_mapping { uint32_t chan[CHANNELS]; };
	void operator_map(operator_mapping &dest) const;

	// handle writes to the register array
	bool write(uint16_t index, uint8_t data, uint32_t &chan, uint32_t &opmask);

	// clock the noise and LFO, if present, returning LFO PM value
	int32_t clock_noise_and_lfo();

	// reset the LFO
	void reset_lfo() { m_lfo_counter = 0; }

	// return the AM offset from LFO for the given channel
	uint32_t lfo_am_offset(uint32_t choffs) const;

	// return the current noise state, gated by the noise clock
	uint32_t noise_state() const { return m_noise_state; }

	// caching helpers
	void cache_operator_data(uint32_t choffs, uint32_t opoffs, opdata_cache &cache);

	// compute the phase step, given a PM value
	uint32_t compute_phase_step(uint32_t choffs, uint32_t opoffs, opdata_cache const &cache, int32_t lfo_raw_pm);

	// log a key-on event
	std::string log_keyon(uint32_t choffs, uint32_t opoffs);

	// system-wide registers
	uint32_t test() const                       { return byte(0x01, 0, 8); }
	uint32_t noise_frequency() const            { return byte(0x0f, 0, 5); }
	uint32_t noise_enable() const               { return byte(0x0f, 7, 1); }
	uint32_t timer_a_value() const              { return word(0x10, 0, 8, 0x11, 0, 2); }
	uint32_t timer_b_value() const              { return byte(0x12, 0, 8); }
	uint32_t csm() const                        { return byte(0x14, 7, 1); }
	uint32_t reset_timer_b() const              { return byte(0x14, 5, 1); }
	uint32_t reset_timer_a() const              { return byte(0x14, 4, 1); }
	uint32_t enable_timer_b() const             { return byte(0x14, 3, 1); }
	uint32_t enable_timer_a() const             { return byte(0x14, 2, 1); }
	uint32_t load_timer_b() const               { return byte(0x14, 1, 1); }
	uint32_t load_timer_a() const               { return byte(0x14, 0, 1); }
	uint32_t lfo_rate() const                   { return byte(0x18, 0, 8); }
	uint32_t lfo_am_depth() const               { return byte(0x19, 0, 7); }
	uint32_t lfo_pm_depth() const               { return byte(0x1a, 0, 7); }
	uint32_t lfo_waveform() const               { return byte(0x1b, 0, 2); }

	// per-channel registers
	uint32_t ch_output_any(uint32_t choffs) const    { return byte(0x20, 6, 2, choffs); }
	uint32_t ch_output_0(uint32_t choffs) const      { return byte(0x20, 6, 1, choffs); }
	uint32_t ch_output_1(uint32_t choffs) const      { return byte(0x20, 7, 1, choffs); }
	uint32_t ch_output_2(uint32_t choffs) const      { return 0; }
	uint32_t ch_output_3(uint32_t choffs) const      { return 0; }
	uint32_t ch_feedback(uint32_t choffs) const      { return byte(0x20, 3, 3, choffs); }
	uint32_t ch_algorithm(uint32_t choffs) const     { return byte(0x20, 0, 3, choffs); }
	uint32_t ch_block_freq(uint32_t choffs) const    { return word(0x28, 0, 7, 0x30, 2, 6, choffs); }
	uint32_t ch_lfo_pm_sens(uint32_t choffs) const   { return byte(0x38, 4, 3, choffs); }
	uint32_t ch_lfo_am_sens(uint32_t choffs) const   { return byte(0x38, 0, 2, choffs); }

	// per-operator registers
	uint32_t op_detune(uint32_t opoffs) const        { return byte(0x40, 4, 3, opoffs); }
	uint32_t op_multiple(uint32_t opoffs) const      { return byte(0x40, 0, 4, opoffs); }
	uint32_t op_total_level(uint32_t opoffs) const   { return byte(0x60, 0, 7, opoffs); }
	uint32_t op_ksr(uint32_t opoffs) const           { return byte(0x80, 6, 2, opoffs); }
	uint32_t op_attack_rate(uint32_t opoffs) const   { return byte(0x80, 0, 5, opoffs); }
	uint32_t op_lfo_am_enable(uint32_t opoffs) const { return byte(0xa0, 7, 1, opoffs); }
	uint32_t op_decay_rate(uint32_t opoffs) const    { return byte(0xa0, 0, 5, opoffs); }
	uint32_t op_detune2(uint32_t opoffs) const       { return byte(0xc0, 6, 2, opoffs); }
	uint32_t op_sustain_rate(uint32_t opoffs) const  { return byte(0xc0, 0, 5, opoffs); }
	uint32_t op_sustain_level(uint32_t opoffs) const { return byte(0xe0, 4, 4, opoffs); }
	uint32_t op_release_rate(uint32_t opoffs) const  { return byte(0xe0, 0, 4, opoffs); }

protected:
	// return a bitfield extracted from a byte
	uint32_t byte(uint32_t offset, uint32_t start, uint32_t count, uint32_t extra_offset = 0) const
	{
		return bitfield(m_regdata[offset + extra_offset], start, count);
	}

	// return a bitfield extracted from a pair of bytes, MSBs listed first
	uint32_t word(uint32_t offset1, uint32_t start1, uint32_t count1, uint32_t offset2, uint32_t start2, uint32_t count2, uint32_t extra_offset = 0) const
	{
		return (byte(offset1, start1, count1, extra_offset) << count2) | byte(offset2, start2, count2, extra_offset);
	}

	// internal state
	uint32_t m_lfo_counter;               // LFO counter
	uint32_t m_noise_lfsr;                // noise LFSR state
	uint8_t m_noise_counter;              // noise counter
	uint8_t m_noise_state;                // latched noise state
	uint8_t m_noise_lfo;                  // latched LFO noise value
	uint8_t m_lfo_am;                     // current LFO AM value
	uint8_t m_regdata[REGISTERS];         // register data
	int16_t m_lfo_waveform[4][LFO_WAVEFORM_LENGTH]; // LFO waveforms; AM in low 8, PM in upper 8
	uint16_t m_waveform[WAVEFORMS][WAVEFORM_LENGTH]; // waveforms
};


// ======================> opn_registers_base

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
class opn_registers_base : public fm_registers_base
{
public:
	// constants
	static constexpr uint32_t OUTPUTS = IsOpnA ? 2 : 1;
	static constexpr uint32_t CHANNELS = IsOpnA ? 6 : 3;
	static constexpr uint32_t ALL_CHANNELS = (1 << CHANNELS) - 1;
	static constexpr uint32_t OPERATORS = CHANNELS * 4;
	static constexpr bool DYNAMIC_OPS = false;
	static constexpr uint32_t WAVEFORMS = 1;
	static constexpr uint32_t REGISTERS = IsOpnA ? 0x200 : 0x100;
	static constexpr uint32_t REG_MODE = 0x27;
	static constexpr uint32_t DEFAULT_PRESCALE = 6;
	static constexpr uint32_t EG_CLOCK_DIVIDER = 3;
	static constexpr bool EG_HAS_DEPRESS = false;
	static constexpr bool EG_HAS_SSG = true;
	static constexpr bool MODULATOR_DELAY = false;
	static constexpr uint32_t CSM_TRIGGER_MASK = 1 << 2;
	static constexpr uint8_t STATUS_TIMERA = 0x01;
	static constexpr uint8_t STATUS_TIMERB = 0x02;
	static constexpr uint8_t STATUS_BUSY = 0x80;
	static constexpr uint8_t STATUS_IRQ = 0;

	// constructor
	opn_registers_base();

	// register for save states
	void save(fm_interface &intf);

	// reset to initial state
	void reset();

	// map channel number to register offset
	static constexpr uint32_t channel_offset(uint32_t chnum)
	{
		assert(chnum < CHANNELS);
		if (!IsOpnA)
			return chnum;
		else
			return (chnum % 3) + 0x100 * (chnum / 3);
	}

	// map operator number to register offset
	static constexpr uint32_t operator_offset(uint32_t opnum)
	{
		assert(opnum < OPERATORS);
		if (!IsOpnA)
			return opnum + opnum / 3;
		else
			return (opnum % 12) + ((opnum % 12) / 3) + 0x100 * (opnum / 12);
	}

	// return an array of operator indices for each channel
	struct operator_mapping { uint32_t chan[CHANNELS]; };
	void operator_map(operator_mapping &dest) const;

	// handle writes to the register array
	bool write(uint16_t index, uint8_t data, uint32_t &chan, uint32_t &opmask);

	// clock the noise and LFO, if present, returning LFO PM value
	int32_t clock_noise_and_lfo();

	// reset the LFO
	void reset_lfo() { m_lfo_counter = 0; }

	// return the AM offset from LFO for the given channel
	uint32_t lfo_am_offset(uint32_t choffs) const;

	// return LFO/noise states
	uint32_t noise_state() const { return 0; }

	// caching helpers
	void cache_operator_data(uint32_t choffs, uint32_t opoffs, opdata_cache &cache);

	// compute the phase step, given a PM value
	uint32_t compute_phase_step(uint32_t choffs, uint32_t opoffs, opdata_cache const &cache, int32_t lfo_raw_pm);

	// log a key-on event
	std::string log_keyon(uint32_t choffs, uint32_t opoffs);

	// system-wide registers
	uint32_t test() const                       { return byte(0x21, 0, 8); }
	uint32_t lfo_enable() const                 { return IsOpnA ? byte(0x22, 3, 1) : 0; }
	uint32_t lfo_rate() const                   { return IsOpnA ? byte(0x22, 0, 3) : 0; }
	uint32_t timer_a_value() const              { return word(0x24, 0, 8, 0x25, 0, 2); }
	uint32_t timer_b_value() const              { return byte(0x26, 0, 8); }
	uint32_t csm() const                        { return (byte(0x27, 6, 2) == 2); }
	uint32_t multi_freq() const                 { return (byte(0x27, 6, 2) != 0); }
	uint32_t reset_timer_b() const              { return byte(0x27, 5, 1); }
	uint32_t reset_timer_a() const              { return byte(0x27, 4, 1); }
	uint32_t enable_timer_b() const             { return byte(0x27, 3, 1); }
	uint32_t enable_timer_a() const             { return byte(0x27, 2, 1); }
	uint32_t load_timer_b() const               { return byte(0x27, 1, 1); }
	uint32_t load_timer_a() const               { return byte(0x27, 0, 1); }
	uint32_t multi_block_freq(uint32_t num) const    { return word(0xac, 0, 6, 0xa8, 0, 8, num); }

	// per-channel registers
	uint32_t ch_block_freq(uint32_t choffs) const    { return word(0xa4, 0, 6, 0xa0, 0, 8, choffs); }
	uint32_t ch_feedback(uint32_t choffs) const      { return byte(0xb0, 3, 3, choffs); }
	uint32_t ch_algorithm(uint32_t choffs) const     { return byte(0xb0, 0, 3, choffs); }
	uint32_t ch_output_any(uint32_t choffs) const    { return IsOpnA ? byte(0xb4, 6, 2, choffs) : 1; }
	uint32_t ch_output_0(uint32_t choffs) const      { return IsOpnA ? byte(0xb4, 7, 1, choffs) : 1; }
	uint32_t ch_output_1(uint32_t choffs) const      { return IsOpnA ? byte(0xb4, 6, 1, choffs) : 0; }
	uint32_t ch_output_2(uint32_t choffs) const      { return 0; }
	uint32_t ch_output_3(uint32_t choffs) const      { return 0; }
	uint32_t ch_lfo_am_sens(uint32_t choffs) const   { return IsOpnA ? byte(0xb4, 4, 2, choffs) : 0; }
	uint32_t ch_lfo_pm_sens(uint32_t choffs) const   { return IsOpnA ? byte(0xb4, 0, 3, choffs) : 0; }

	// per-operator registers
	uint32_t op_detune(uint32_t opoffs) const        { return byte(0x30, 4, 3, opoffs); }
	uint32_t op_multiple(uint32_t opoffs) const      { return byte(0x30, 0, 4, opoffs); }
	uint32_t op_total_level(uint32_t opoffs) const   { return byte(0x40, 0, 7, opoffs); }
	uint32_t op_ksr(uint32_t opoffs) const           { return byte(0x50, 6, 2, opoffs); }
	uint32_t op_attack_rate(uint32_t opoffs) const   { return byte(0x50, 0, 5, opoffs); }
	uint32_t op_decay_rate(uint32_t opoffs) const    { return byte(0x60, 0, 5, opoffs); }
	uint32_t op_lfo_am_enable(uint32_t opoffs) const { return IsOpnA ? byte(0x60, 7, 1, opoffs) : 0; }
	uint32_t op_sustain_rate(uint32_t opoffs) const  { return byte(0x70, 0, 5, opoffs); }
	uint32_t op_sustain_level(uint32_t opoffs) const { return byte(0x80, 4, 4, opoffs); }
	uint32_t op_release_rate(uint32_t opoffs) const  { return byte(0x80, 0, 4, opoffs); }
	uint32_t op_ssg_eg_enable(uint32_t opoffs) const { return byte(0x90, 3, 1, opoffs); }
	uint32_t op_ssg_eg_mode(uint32_t opoffs) const   { return byte(0x90, 0, 3, opoffs); }

protected:
	// return a bitfield extracted from a byte
	uint32_t byte(uint32_t offset, uint32_t start, uint32_t count, uint32_t extra_offset = 0) const
	{
		return bitfield(m_regdata[offset + extra_offset], start, count);
	}

	// return a bitfield extracted from a pair of bytes, MSBs listed first
	uint32_t word(uint32_t offset1, uint32_t start1, uint32_t count1, uint32_t offset2, uint32_t start2, uint32_t count2, uint32_t extra_offset = 0) const
	{
		return (byte(offset1, start1, count1, extra_offset) << count2) | byte(offset2, start2, count2, extra_offset);
	}

	// internal state
	uint32_t m_lfo_counter;               // LFO counter
	uint8_t m_lfo_am;                     // current LFO AM value
	uint8_t m_regdata[REGISTERS];         // register data
	uint16_t m_waveform[WAVEFORMS][WAVEFORM_LENGTH]; // waveforms
};

using opn_registers = opn_registers_base<false>;
using opna_registers = opn_registers_base<true>;


// ======================> opl_registers_base

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
class opl_registers_base : public fm_registers_base
{
	static constexpr bool IsOpl2 = (Revision == 2);
	static constexpr bool IsOpl2Plus = (Revision >= 2);
	static constexpr bool IsOpl3Plus = (Revision >= 3);
	static constexpr bool IsOpl4Plus = (Revision >= 4);

public:
	// constants
	static constexpr uint32_t OUTPUTS = IsOpl3Plus ? 4 : 1;
	static constexpr uint32_t CHANNELS = IsOpl3Plus ? 18 : 9;
	static constexpr uint32_t ALL_CHANNELS = (1 << CHANNELS) - 1;
	static constexpr uint32_t OPERATORS = CHANNELS * 2;
	static constexpr bool DYNAMIC_OPS = IsOpl3Plus;
	static constexpr uint32_t WAVEFORMS = IsOpl3Plus ? 8 : (IsOpl2Plus ? 4 : 1);
	static constexpr uint32_t REGISTERS = IsOpl3Plus ? 0x200 : 0x100;
	static constexpr uint32_t REG_MODE = 0x04;
	static constexpr uint32_t DEFAULT_PRESCALE = IsOpl4Plus ? 19 : (IsOpl3Plus ? 8 : 4);
	static constexpr uint32_t EG_CLOCK_DIVIDER = 1;
	static constexpr bool EG_HAS_DEPRESS = false;
	static constexpr bool EG_HAS_SSG = false;
	static constexpr bool MODULATOR_DELAY = !IsOpl3Plus;
	static constexpr uint32_t CSM_TRIGGER_MASK = ALL_CHANNELS;
	static constexpr uint8_t STATUS_TIMERA = 0x40;
	static constexpr uint8_t STATUS_TIMERB = 0x20;
	static constexpr uint8_t STATUS_BUSY = 0;
	static constexpr uint8_t STATUS_IRQ = 0x80;

	// constructor
	opl_registers_base();

	// register for save states
	void save(fm_interface &intf);

	// reset to initial state
	void reset();

	// map channel number to register offset
	static constexpr uint32_t channel_offset(uint32_t chnum)
	{
		assert(chnum < CHANNELS);
		if (!IsOpl3Plus)
			return chnum;
		else
			return (chnum % 9) + 0x100 * (chnum / 9);
	}

	// map operator number to register offset
	static constexpr uint32_t operator_offset(uint32_t opnum)
	{
		assert(opnum < OPERATORS);
		if (!IsOpl3Plus)
			return opnum + 2 * (opnum / 6);
		else
			return (opnum % 18) + 2 * ((opnum % 18) / 6) + 0x100 * (opnum / 18);
	}

	// return an array of operator indices for each channel
	struct operator_mapping { uint32_t chan[CHANNELS]; };
	void operator_map(operator_mapping &dest) const;

	// OPL4 apparently can read back FM registers?
	uint8_t read(uint16_t index) { return m_regdata[index]; }

	// handle writes to the register array
	bool write(uint16_t index, uint8_t data, uint32_t &chan, uint32_t &opmask);

	// clock the noise and LFO, if present, returning LFO PM value
	int32_t clock_noise_and_lfo();

	// reset the LFO
	void reset_lfo() { m_lfo_am_counter = m_lfo_pm_counter = 0; }

	// return the AM offset from LFO for the given channel
	// on OPL this is just a fixed value
	uint32_t lfo_am_offset(uint32_t choffs) const { return m_lfo_am; }

	// return LFO/noise states
	uint32_t noise_state() const { return m_noise_lfsr >> 23; }

	// caching helpers
	void cache_operator_data(uint32_t choffs, uint32_t opoffs, opdata_cache &cache);

	// compute the phase step, given a PM value
	uint32_t compute_phase_step(uint32_t choffs, uint32_t opoffs, opdata_cache const &cache, int32_t lfo_raw_pm);

	// log a key-on event
	std::string log_keyon(uint32_t choffs, uint32_t opoffs);

	// system-wide registers
	uint32_t test() const                       { return byte(0x01, 0, 8); }
	uint32_t waveform_enable() const            { return IsOpl2 ? byte(0x01, 5, 1) : (IsOpl3Plus ? 1 : 0); }
	uint32_t timer_a_value() const              { return byte(0x02, 0, 8) * 4; } // 8->10 bits
	uint32_t timer_b_value() const              { return byte(0x03, 0, 8); }
	uint32_t status_mask() const                { return byte(0x04, 0, 8) & 0x78; }
	uint32_t irq_reset() const                  { return byte(0x04, 7, 1); }
	uint32_t reset_timer_b() const              { return byte(0x04, 7, 1) | byte(0x04, 5, 1); }
	uint32_t reset_timer_a() const              { return byte(0x04, 7, 1) | byte(0x04, 6, 1); }
	uint32_t enable_timer_b() const             { return 1; }
	uint32_t enable_timer_a() const             { return 1; }
	uint32_t load_timer_b() const               { return byte(0x04, 1, 1); }
	uint32_t load_timer_a() const               { return byte(0x04, 0, 1); }
	uint32_t csm() const                        { return IsOpl3Plus ? 0 : byte(0x08, 7, 1); }
	uint32_t note_select() const                { return byte(0x08, 6, 1); }
	uint32_t lfo_am_depth() const               { return byte(0xbd, 7, 1); }
	uint32_t lfo_pm_depth() const               { return byte(0xbd, 6, 1); }
	uint32_t rhythm_enable() const              { return byte(0xbd, 5, 1); }
	uint32_t rhythm_keyon() const               { return byte(0xbd, 4, 0); }
	uint32_t newflag() const                    { return IsOpl3Plus ? byte(0x105, 0, 1) : 0; }
	uint32_t new2flag() const                   { return IsOpl4Plus ? byte(0x105, 1, 1) : 0; }
	uint32_t fourop_enable() const              { return IsOpl3Plus ? byte(0x104, 0, 6) : 0; }

	// per-channel registers
	uint32_t ch_block_freq(uint32_t choffs) const    { return word(0xb0, 0, 5, 0xa0, 0, 8, choffs); }
	uint32_t ch_feedback(uint32_t choffs) const      { return byte(0xc0, 1, 3, choffs); }
	uint32_t ch_algorithm(uint32_t choffs) const     { return byte(0xc0, 0, 1, choffs) | (IsOpl3Plus ? (8 | (byte(0xc3, 0, 1, choffs) << 1)) : 0); }
	uint32_t ch_output_any(uint32_t choffs) const    { return newflag() ? byte(0xc0 + choffs, 4, 4) : 1; }
	uint32_t ch_output_0(uint32_t choffs) const      { return newflag() ? byte(0xc0 + choffs, 4, 1) : 1; }
	uint32_t ch_output_1(uint32_t choffs) const      { return newflag() ? byte(0xc0 + choffs, 5, 1) : (IsOpl3Plus ? 1 : 0); }
	uint32_t ch_output_2(uint32_t choffs) const      { return newflag() ? byte(0xc0 + choffs, 6, 1) : 0; }
	uint32_t ch_output_3(uint32_t choffs) const      { return newflag() ? byte(0xc0 + choffs, 7, 1) : 0; }

	// per-operator registers
	uint32_t op_lfo_am_enable(uint32_t opoffs) const { return byte(0x20, 7, 1, opoffs); }
	uint32_t op_lfo_pm_enable(uint32_t opoffs) const { return byte(0x20, 6, 1, opoffs); }
	uint32_t op_eg_sustain(uint32_t opoffs) const    { return byte(0x20, 5, 1, opoffs); }
	uint32_t op_ksr(uint32_t opoffs) const           { return byte(0x20, 4, 1, opoffs); }
	uint32_t op_multiple(uint32_t opoffs) const      { return byte(0x20, 0, 4, opoffs); }
	uint32_t op_ksl(uint32_t opoffs) const           { uint32_t temp = byte(0x40, 6, 2, opoffs); return bitfield(temp, 1) | (bitfield(temp, 0) << 1); }
	uint32_t op_total_level(uint32_t opoffs) const   { return byte(0x40, 0, 6, opoffs); }
	uint32_t op_attack_rate(uint32_t opoffs) const   { return byte(0x60, 4, 4, opoffs); }
	uint32_t op_decay_rate(uint32_t opoffs) const    { return byte(0x60, 0, 4, opoffs); }
	uint32_t op_sustain_level(uint32_t opoffs) const { return byte(0x80, 4, 4, opoffs); }
	uint32_t op_release_rate(uint32_t opoffs) const  { return byte(0x80, 0, 4, opoffs); }
	uint32_t op_waveform(uint32_t opoffs) const      { return IsOpl2Plus ? byte(0xe0, 0, newflag() ? 3 : 2, opoffs) : 0; }

protected:
	// return a bitfield extracted from a byte
	uint32_t byte(uint32_t offset, uint32_t start, uint32_t count, uint32_t extra_offset = 0) const
	{
		return bitfield(m_regdata[offset + extra_offset], start, count);
	}

	// return a bitfield extracted from a pair of bytes, MSBs listed first
	uint32_t word(uint32_t offset1, uint32_t start1, uint32_t count1, uint32_t offset2, uint32_t start2, uint32_t count2, uint32_t extra_offset = 0) const
	{
		return (byte(offset1, start1, count1, extra_offset) << count2) | byte(offset2, start2, count2, extra_offset);
	}

	// helper to determine if the this channel is an active rhythm channel
	bool is_rhythm(uint32_t choffs) const
	{
		return rhythm_enable() && (choffs >= 6 && choffs <= 8);
	}

	// internal state
	uint16_t m_lfo_am_counter;            // LFO AM counter
	uint16_t m_lfo_pm_counter;            // LFO PM counter
	uint32_t m_noise_lfsr;                // noise LFSR state
	uint8_t m_lfo_am;                     // current LFO AM value
	uint8_t m_regdata[REGISTERS];         // register data
	uint16_t m_waveform[WAVEFORMS][WAVEFORM_LENGTH]; // waveforms
};

using opl_registers = opl_registers_base<1>;
using opl2_registers = opl_registers_base<2>;
using opl3_registers = opl_registers_base<3>;
using opl4_registers = opl_registers_base<4>;


// ======================> opll_registers

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

class opll_registers : public fm_registers_base
{
public:
	static constexpr uint32_t OUTPUTS = 2;
	static constexpr uint32_t CHANNELS = 9;
	static constexpr uint32_t ALL_CHANNELS = (1 << CHANNELS) - 1;
	static constexpr uint32_t OPERATORS = CHANNELS * 2;
	static constexpr bool DYNAMIC_OPS = false;
	static constexpr uint32_t WAVEFORMS = 2;
	static constexpr uint32_t REGISTERS = 0x40;
	static constexpr uint32_t REG_MODE = 0x3f;
	static constexpr uint32_t DEFAULT_PRESCALE = 4;
	static constexpr uint32_t EG_CLOCK_DIVIDER = 1;
	static constexpr bool EG_HAS_DEPRESS = true;
	static constexpr bool EG_HAS_SSG = false;
	static constexpr bool MODULATOR_DELAY = true;
	static constexpr uint32_t CSM_TRIGGER_MASK = 0;
	static constexpr uint8_t STATUS_TIMERA = 0;
	static constexpr uint8_t STATUS_TIMERB = 0;
	static constexpr uint8_t STATUS_BUSY = 0;
	static constexpr uint8_t STATUS_IRQ = 0;

	// OPLL-specific constants
	static constexpr uint32_t INSTDATA_SIZE = 0x90;

	// constructor
	opll_registers();

	// register for save states
	void save(fm_interface &intf);

	// reset to initial state
	void reset();

	// map channel number to register offset
	static constexpr uint32_t channel_offset(uint32_t chnum)
	{
		assert(chnum < CHANNELS);
		return chnum;
	}

	// map operator number to register offset
	static constexpr uint32_t operator_offset(uint32_t opnum)
	{
		assert(opnum < OPERATORS);
		return opnum;
	}

	// return an array of operator indices for each channel
	struct operator_mapping { uint32_t chan[CHANNELS]; };
	void operator_map(operator_mapping &dest) const;

	// handle writes to the register array
	bool write(uint16_t index, uint8_t data, uint32_t &chan, uint32_t &opmask);

	// clock the noise and LFO, if present, returning LFO PM value
	int32_t clock_noise_and_lfo();

	// reset the LFO
	void reset_lfo() { m_lfo_am_counter = m_lfo_pm_counter = 0; }

	// return the AM offset from LFO for the given channel
	// on OPL this is just a fixed value
	uint32_t lfo_am_offset(uint32_t choffs) const { return m_lfo_am; }

	// return LFO/noise states
	uint32_t noise_state() const { return m_noise_lfsr >> 23; }

	// caching helpers
	void cache_operator_data(uint32_t choffs, uint32_t opoffs, opdata_cache &cache);

	// compute the phase step, given a PM value
	uint32_t compute_phase_step(uint32_t choffs, uint32_t opoffs, opdata_cache const &cache, int32_t lfo_raw_pm);

	// log a key-on event
	std::string log_keyon(uint32_t choffs, uint32_t opoffs);

	// set the instrument data
	void set_instrument_data(uint8_t const *data)
	{
		memcpy(&m_instdata[0], data, INSTDATA_SIZE);
	}

	// system-wide registers
	uint32_t rhythm_enable() const              { return byte(0x0e, 5, 1); }
	uint32_t rhythm_keyon() const               { return byte(0x0e, 4, 0); }
	uint32_t test() const                       { return byte(0x0f, 0, 8); }
	uint32_t waveform_enable() const            { return 1; }
	uint32_t timer_a_value() const              { return 0; }
	uint32_t timer_b_value() const              { return 0; }
	uint32_t status_mask() const                { return 0; }
	uint32_t irq_reset() const                  { return 0; }
	uint32_t reset_timer_b() const              { return 0; }
	uint32_t reset_timer_a() const              { return 0; }
	uint32_t enable_timer_b() const             { return 0; }
	uint32_t enable_timer_a() const             { return 0; }
	uint32_t load_timer_b() const               { return 0; }
	uint32_t load_timer_a() const               { return 0; }
	uint32_t csm() const                        { return 0; }

	// per-channel registers
	uint32_t ch_block_freq(uint32_t choffs) const    { return word(0x20, 0, 4, 0x10, 0, 8, choffs); }
	uint32_t ch_sustain(uint32_t choffs) const       { return byte(0x20, 5, 1, choffs); }
	uint32_t ch_total_level(uint32_t choffs) const   { return instchbyte(0x02, 0, 6, choffs); }
	uint32_t ch_feedback(uint32_t choffs) const      { return instchbyte(0x03, 0, 3, choffs); }
	uint32_t ch_algorithm(uint32_t choffs) const     { return 0; }
	uint32_t ch_instrument(uint32_t choffs) const    { return byte(0x30, 4, 4, choffs); }
	uint32_t ch_output_any(uint32_t choffs) const    { return 1; }
	uint32_t ch_output_0(uint32_t choffs) const      { return !is_rhythm(choffs); }
	uint32_t ch_output_1(uint32_t choffs) const      { return is_rhythm(choffs); }
	uint32_t ch_output_2(uint32_t choffs) const      { return 0; }
	uint32_t ch_output_3(uint32_t choffs) const      { return 0; }

	// per-operator registers
	uint32_t op_lfo_am_enable(uint32_t opoffs) const { return instopbyte(0x00, 7, 1, opoffs); }
	uint32_t op_lfo_pm_enable(uint32_t opoffs) const { return instopbyte(0x00, 6, 1, opoffs); }
	uint32_t op_eg_sustain(uint32_t opoffs) const    { return instopbyte(0x00, 5, 1, opoffs); }
	uint32_t op_ksr(uint32_t opoffs) const           { return instopbyte(0x00, 4, 1, opoffs); }
	uint32_t op_multiple(uint32_t opoffs) const      { return instopbyte(0x00, 0, 4, opoffs); }
	uint32_t op_ksl(uint32_t opoffs) const           { return instopbyte(0x02, 6, 2, opoffs); }
	uint32_t op_waveform(uint32_t opoffs) const      { return instchbyte(0x03, 3 + bitfield(opoffs, 0), 1, opoffs >> 1); }
	uint32_t op_attack_rate(uint32_t opoffs) const   { return instopbyte(0x04, 4, 4, opoffs); }
	uint32_t op_decay_rate(uint32_t opoffs) const    { return instopbyte(0x04, 0, 4, opoffs); }
	uint32_t op_sustain_level(uint32_t opoffs) const { return instopbyte(0x06, 4, 4, opoffs); }
	uint32_t op_release_rate(uint32_t opoffs) const  { return instopbyte(0x06, 0, 4, opoffs); }
	uint32_t op_volume(uint32_t opoffs) const        { return byte(0x30, 4 * bitfield(~opoffs, 0), 4, opoffs >> 1); }

private:
	// return a bitfield extracted from a byte
	uint32_t byte(uint32_t offset, uint32_t start, uint32_t count, uint32_t extra_offset = 0) const
	{
		return bitfield(m_regdata[offset + extra_offset], start, count);
	}

	// return a bitfield extracted from a pair of bytes, MSBs listed first
	uint32_t word(uint32_t offset1, uint32_t start1, uint32_t count1, uint32_t offset2, uint32_t start2, uint32_t count2, uint32_t extra_offset = 0) const
	{
		return (byte(offset1, start1, count1, extra_offset) << count2) | byte(offset2, start2, count2, extra_offset);
	}

	// helpers to read from instrument channel/operator data
	uint32_t instchbyte(uint32_t offset, uint32_t start, uint32_t count, uint32_t choffs) const { return bitfield(m_chinst[choffs][offset], start, count); }
	uint32_t instopbyte(uint32_t offset, uint32_t start, uint32_t count, uint32_t opoffs) const { return bitfield(m_opinst[opoffs][offset], start, count); }

	// helper to determine if the this channel is an active rhythm channel
	bool is_rhythm(uint32_t choffs) const
	{
		return rhythm_enable() && choffs >= 6;
	}

	// internal state
	uint16_t m_lfo_am_counter;            // LFO AM counter
	uint16_t m_lfo_pm_counter;            // LFO PM counter
	uint32_t m_noise_lfsr;                // noise LFSR state
	uint8_t m_lfo_am;                     // current LFO AM value
	uint8_t const *m_chinst[CHANNELS];    // pointer to instrument data for each channel
	uint8_t const *m_opinst[OPERATORS];   // pointer to instrument data for each operator
	uint8_t m_regdata[REGISTERS];         // register data
	uint8_t m_instdata[INSTDATA_SIZE];    // instrument data
	uint16_t m_waveform[WAVEFORMS][WAVEFORM_LENGTH]; // waveforms
};


// ======================> opq_registers

//
// OPQ register map:
//
//      System-wide registers:
//           03 xxxxxxxx Timer control (unknown; 0x71 causes interrupts at ~10ms)
//           04 ----x--- LFO disable
//              -----xxx LFO frequency (0=~4Hz, 6=~10Hz, 7=~47Hz)
//           05 -x------ Key on/off operator 4
//              --x----- Key on/off operator 3
//              ---x---- Key on/off operator 2
//              ----x--- Key on/off operator 1
//              -----xxx Channel select
//
//     Per-channel registers (channel in address bits 0-2)
//        10-17 x------- Pan right
//              -x------ Pan left
//              --xxx--- Feedback level for operator 1 (0-7)
//              -----xxx Operator connection algorithm (0-7)
//        18-1F x------- Echo
//              -xxx---- PM sensitivity
//              ------xx AM shift
//        20-27 -xxx---- Block (0-7), Operator 2 & 4
//              ----xxxx Frequency number upper 4 bits, Operator 2 & 4
//        28-2F -xxx---- Block (0-7), Operator 1 & 3
//              ----xxxx Frequency number upper 4 bits, Operator 1 & 3
//        30-37 xxxxxxxx Frequency number lower 8 bits, Operator 2 & 4
//        38-3F xxxxxxxx Frequency number lower 8 bits, Operator 1 & 3
//
//     Per-operator registers (channel in address bits 0-2, operator in bits 3-4)
//        40-5F 0-xxxxxx Detune value (0-63)
//              1---xxxx Multiple value (0-15)
//        60-7F -xxxxxxx Total level (0-127)
//        80-9F xxx----- Key scale rate (0-7)
//              ---xxxxx Attack rate (0-31)
//        A0-BF x------- LFO AM enable, retrigger disable
//               x------ Waveform select
//              ---xxxxx Decay rate (0-31)
//        C0-DF ---xxxxx Sustain rate (0-31)
//        E0-FF xxxx---- Sustain level (0-15)
//              ----xxxx Release rate (0-15)
//
// Diffs from OPM:
//  - 2 frequencies/channel
//  - KSR is 3 bits?
//  - retrigger disable
//  - 2 waveforms
//  - uses FNUM
//  - echo behavior
//  - larger detune range
//
// Questions:
//  - timer information is pretty light
//  - how does echo work?
//  -

class opq_registers : public fm_registers_base
{
public:
	// constants
	static constexpr uint32_t OUTPUTS = 2;
	static constexpr uint32_t CHANNELS = 8;
	static constexpr uint32_t ALL_CHANNELS = (1 << CHANNELS) - 1;
	static constexpr uint32_t OPERATORS = CHANNELS * 4;
	static constexpr bool DYNAMIC_OPS = false;
	static constexpr uint32_t WAVEFORMS = 2;
	static constexpr uint32_t REGISTERS = 0x120;
	static constexpr uint32_t REG_MODE = 0x03;
	static constexpr uint32_t DEFAULT_PRESCALE = 2;
	static constexpr uint32_t EG_CLOCK_DIVIDER = 3;
	static constexpr bool EG_HAS_DEPRESS = false;
	static constexpr bool EG_HAS_SSG = false;
	static constexpr bool MODULATOR_DELAY = false;
	static constexpr uint32_t CSM_TRIGGER_MASK = ALL_CHANNELS;
	static constexpr uint8_t STATUS_TIMERA = 0;
	static constexpr uint8_t STATUS_TIMERB = 0x04;
	static constexpr uint8_t STATUS_BUSY = 0x80;
	static constexpr uint8_t STATUS_IRQ = 0;

	// constructor
	opq_registers();

	// register for save states
	void save(fm_interface &intf);

	// reset to initial state
	void reset();

	// map channel number to register offset
	static constexpr uint32_t channel_offset(uint32_t chnum)
	{
		assert(chnum < CHANNELS);
		return chnum;
	}

	// map operator number to register offset
	static constexpr uint32_t operator_offset(uint32_t opnum)
	{
		assert(opnum < OPERATORS);
		return opnum;
	}

	// return an array of operator indices for each channel
	struct operator_mapping { uint32_t chan[CHANNELS]; };
	void operator_map(operator_mapping &dest) const;

	// handle writes to the register array
	bool write(uint16_t index, uint8_t data, uint32_t &chan, uint32_t &opmask);

	// clock the noise and LFO, if present, returning LFO PM value
	int32_t clock_noise_and_lfo();

	// reset the LFO
	void reset_lfo() { m_lfo_counter = 0; }

	// return the AM offset from LFO for the given channel
	uint32_t lfo_am_offset(uint32_t choffs) const;

	// return the current noise state, gated by the noise clock
	uint32_t noise_state() const { return 0; }

	// caching helpers
	void cache_operator_data(uint32_t choffs, uint32_t opoffs, opdata_cache &cache);

	// compute the phase step, given a PM value
	uint32_t compute_phase_step(uint32_t choffs, uint32_t opoffs, opdata_cache const &cache, int32_t lfo_raw_pm);

	// log a key-on event
	std::string log_keyon(uint32_t choffs, uint32_t opoffs);

	// system-wide registers
	uint32_t timer_a_value() const              { return 0; }
	uint32_t timer_b_value() const              { return byte(0x03, 0, 8); }
	uint32_t csm() const                        { return 0; }
	uint32_t reset_timer_b() const              { return byte(0x14, 5, 1); }
	uint32_t reset_timer_a() const              { return 0; }
	uint32_t enable_timer_b() const             { return byte(0x14, 3, 1); }
	uint32_t enable_timer_a() const             { return 0; }
	uint32_t load_timer_b() const               { return byte(0x14, 1, 1); }
	uint32_t load_timer_a() const               { return 0; }
	uint32_t lfo_enable() const                 { return byte(0x04, 4, 1) ^ 1; }
	uint32_t lfo_rate() const                   { return byte(0x04, 0, 3); }

	// per-channel registers
	uint32_t ch_output_any(uint32_t choffs) const    { return byte(0x10, 6, 2, choffs); }
	uint32_t ch_output_0(uint32_t choffs) const      { return byte(0x10, 6, 1, choffs); }
	uint32_t ch_output_1(uint32_t choffs) const      { return byte(0x10, 7, 1, choffs); }
	uint32_t ch_output_2(uint32_t choffs) const      { return 0; }
	uint32_t ch_output_3(uint32_t choffs) const      { return 0; }
	uint32_t ch_feedback(uint32_t choffs) const      { return byte(0x10, 3, 3, choffs); }
	uint32_t ch_algorithm(uint32_t choffs) const     { return byte(0x10, 0, 3, choffs); }
	uint32_t ch_echo(uint32_t choffs) const          { return byte(0x18, 7, 1, choffs); }
	uint32_t ch_lfo_pm_sens(uint32_t choffs) const   { return byte(0x18, 4, 3, choffs); }
	uint32_t ch_lfo_am_sens(uint32_t choffs) const   { return byte(0x18, 0, 2, choffs); }
	uint32_t ch_block_freq_24(uint32_t choffs) const { return word(0x20, 0, 6, 0x30, 0, 8, choffs); }
	uint32_t ch_block_freq_13(uint32_t choffs) const { return word(0x28, 0, 6, 0x30, 0, 8, choffs); }

	// per-operator registers
	uint32_t op_detune(uint32_t opoffs) const        { return byte(0x40, 0, 6, opoffs); }
	uint32_t op_multiple(uint32_t opoffs) const      { return byte(0x100, 0, 4, opoffs); }
	uint32_t op_total_level(uint32_t opoffs) const   { return byte(0x60, 0, 7, opoffs); }
	uint32_t op_ksr(uint32_t opoffs) const           { return byte(0x80, 5, 3, opoffs); }
	uint32_t op_attack_rate(uint32_t opoffs) const   { return byte(0x80, 0, 5, opoffs); }
	uint32_t op_lfo_am_enable(uint32_t opoffs) const { return byte(0xa0, 7, 1, opoffs); }
	uint32_t op_waveform(uint32_t opoffs) const      { return byte(0xa0, 6, 1, opoffs); }
	uint32_t op_decay_rate(uint32_t opoffs) const    { return byte(0xa0, 0, 5, opoffs); }
	uint32_t op_sustain_rate(uint32_t opoffs) const  { return byte(0xc0, 0, 5, opoffs); }
	uint32_t op_sustain_level(uint32_t opoffs) const { return byte(0xe0, 4, 4, opoffs); }
	uint32_t op_release_rate(uint32_t opoffs) const  { return byte(0xe0, 0, 4, opoffs); }

protected:
	// return a bitfield extracted from a byte
	uint32_t byte(uint32_t offset, uint32_t start, uint32_t count, uint32_t extra_offset = 0) const
	{
		return bitfield(m_regdata[offset + extra_offset], start, count);
	}

	// return a bitfield extracted from a pair of bytes, MSBs listed first
	uint32_t word(uint32_t offset1, uint32_t start1, uint32_t count1, uint32_t offset2, uint32_t start2, uint32_t count2, uint32_t extra_offset = 0) const
	{
		return (byte(offset1, start1, count1, extra_offset) << count2) | byte(offset2, start2, count2, extra_offset);
	}

	// internal state
	uint32_t m_lfo_counter;               // LFO counter
	uint8_t m_lfo_am;                     // current LFO AM value
	uint8_t m_regdata[REGISTERS];         // register data
	uint16_t m_waveform[WAVEFORMS][WAVEFORM_LENGTH]; // waveforms
};


//*********************************************************
//  CORE ENGINE CLASSES
//*********************************************************

// three different keyon sources; actual keyon is an OR over all of these
enum keyon_type : uint32_t
{
	KEYON_NORMAL = 0,
	KEYON_RHYTHM = 1,
	KEYON_CSM = 2
};


// ======================> fm_operator

// fm_operator represents an FM operator (or "slot" in FM parlance), which
// produces an output sine wave modulated by an envelope
template<class RegisterType>
class fm_operator
{
	// "quiet" value, used to optimize when we can skip doing working
	static constexpr uint32_t EG_QUIET = 0x200;

public:
	// constructor
	fm_operator(fm_engine_base<RegisterType> &owner, uint32_t opoffs);

	// register for save states
	void save(fm_interface &intf, uint32_t index);

	// reset the operator state
	void reset();

	// return the operator/channel offset
	uint32_t opoffs() const { return m_opoffs; }
	uint32_t choffs() const { return m_choffs; }

	// set the current channel
	void set_choffs(uint32_t choffs) { m_choffs = choffs; }

	// prepare prior to clocking
	bool prepare();

	// master clocking function
	void clock(uint32_t env_counter, int32_t lfo_raw_pm);

	// return the current phase value
	uint32_t phase() const { return m_phase >> 10; }

	// compute operator volume
	int32_t compute_volume(uint32_t phase, uint32_t am_offset) const;

	// compute volume for the OPM noise channel
	int32_t compute_noise_volume(uint32_t am_offset) const;

	// key state control
	void keyonoff(uint32_t on, keyon_type type);

	// return a reference to our registers
	RegisterType &regs() { return m_regs; }

private:
	// start the attack phase
	void start_attack();

	// start the release phase
	void start_release();

	// clock phases
	void clock_keystate(uint32_t keystate);
	void clock_ssg_eg_state();
	void clock_envelope(uint32_t env_counter);
	void clock_phase(int32_t lfo_raw_pm);

	// return effective attenuation of the envelope
	uint32_t envelope_attenuation(uint32_t am_offset) const;

	// internal state
	uint32_t m_choffs;                     // channel offset in registers
	uint32_t m_opoffs;                     // operator offset in registers
	uint32_t m_phase;                      // current phase value (10.10 format)
	uint16_t m_env_attenuation;            // computed envelope attenuation (4.6 format)
	envelope_state m_env_state;            // current envelope state
	uint8_t m_ssg_inverted;                // non-zero if the output should be inverted (bit 0)
	uint8_t m_key_state;                   // current key state: on or off (bit 0)
	uint8_t m_keyon_live;                  // live key on state (bit 0 = direct, bit 1 = rhythm, bit 2 = CSM)
	opdata_cache m_cache;                  // cached values for performance
	RegisterType &m_regs;                  // direct reference to registers
	fm_engine_base<RegisterType> &m_owner; // reference to the owning engine
};


// ======================> fm_channel

// fm_channel represents an FM channel which combines the output of 2 or 4
// operators into a final result
template<class RegisterType>
class fm_channel
{
public:
	// constructor
	fm_channel(fm_engine_base<RegisterType> &owner, uint32_t choffs);

	// register for save states
	void save(fm_interface &intf, uint32_t index);

	// reset the channel state
	void reset();

	// return the channel offset
	uint32_t choffs() const { return m_choffs; }

	// assign operators
	void assign(int index, fm_operator<RegisterType> *op)
	{
		assert(index < std::size(m_op));
		m_op[index] = op;
		if (op != nullptr)
			op->set_choffs(m_choffs);
	}

	// signal key on/off to our operators
	void keyonoff(uint32_t states, keyon_type type);

	// prepare prior to clocking
	bool prepare();

	// master clocking function
	void clock(uint32_t env_counter, int32_t lfo_raw_pm);

	// specific 2-operator and 4-operator output handlers
	void output_2op(int32_t outputs[RegisterType::OUTPUTS], uint32_t rshift, int32_t clipmax) const;
	void output_4op(int32_t outputs[RegisterType::OUTPUTS], uint32_t rshift, int32_t clipmax) const;

	// compute the special OPL rhythm channel outputs
	void output_rhythm_ch6(int32_t outputs[RegisterType::OUTPUTS], uint32_t rshift, int32_t clipmax) const;
	void output_rhythm_ch7(uint32_t phase_select, int32_t outputs[RegisterType::OUTPUTS], uint32_t rshift, int32_t clipmax) const;
	void output_rhythm_ch8(uint32_t phase_select, int32_t outputs[RegisterType::OUTPUTS], uint32_t rshift, int32_t clipmax) const;

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
	void add_to_output(uint32_t choffs, int32_t *outputs, int32_t value) const
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
	uint32_t m_choffs;                     // channel offset in registers
	int16_t m_feedback[2];                 // feedback memory for operator 1
	mutable int16_t m_feedback_in;         // next input value for op 1 feedback (set in output)
	fm_operator<RegisterType> *m_op[4];    // up to 4 operators
	RegisterType &m_regs;                  // direct reference to registers
	fm_engine_base<RegisterType> &m_owner; // reference to the owning engine
};


// ======================> fm_engine_base

// fm_engine_base represents a set of operators and channels which together
// form a Yamaha FM core; chips that implement other engines (ADPCM, wavetable,
// etc) take this output and combine it with the others externally
template<class RegisterType>
class fm_engine_base : public fm_engine_callbacks
{
public:
	// expose some constants from the registers
	static constexpr uint32_t OUTPUTS = RegisterType::OUTPUTS;
	static constexpr uint32_t CHANNELS = RegisterType::CHANNELS;
	static constexpr uint32_t ALL_CHANNELS = RegisterType::ALL_CHANNELS;
	static constexpr uint32_t OPERATORS = RegisterType::OPERATORS;

	// also expose status flags for consumers that inject additional bits
	static constexpr uint8_t STATUS_TIMERA = RegisterType::STATUS_TIMERA;
	static constexpr uint8_t STATUS_TIMERB = RegisterType::STATUS_TIMERB;
	static constexpr uint8_t STATUS_BUSY = RegisterType::STATUS_BUSY;
	static constexpr uint8_t STATUS_IRQ = RegisterType::STATUS_IRQ;

	// constructor
	fm_engine_base(fm_interface &intf);

	// register for save states
	void save();

	// reset the overall state
	void reset();

	// master clocking function
	uint32_t clock(uint32_t chanmask);

	// compute sum of channel outputs
	void output(int32_t outputs[RegisterType::OUTPUTS], uint32_t rshift, int32_t clipmax, uint32_t chanmask) const;

	// write to the OPN registers
	void write(uint16_t regnum, uint8_t data);

	// return the current status
	uint8_t status() const;

	// set/reset bits in the status register, updating the IRQ status
	uint8_t set_reset_status(uint8_t set, uint8_t reset)
	{
		m_status = (m_status | set) & ~reset;
		m_intf.synchronized_check_interrupts();
		return m_status;
	}

	// set the IRQ mask
	void set_irq_mask(uint8_t mask) { m_irq_mask = mask; m_intf.synchronized_check_interrupts(); }

	// return the current clock prescale
	uint32_t clock_prescale() const { return m_clock_prescale; }

	// set prescale factor (2/3/6)
	void set_clock_prescale(uint32_t prescale) { m_clock_prescale = prescale; }

	// compute sample rate
	uint32_t sample_rate(uint32_t baseclock) const { return baseclock / (m_clock_prescale * OPERATORS); }

	// reset the LFO state
	void reset_lfo() { m_regs.reset_lfo(); }

	// return the owning device
	fm_interface &intf() const { return m_intf; }

	// return a reference to our registers
	RegisterType &regs() { return m_regs; }

	// timer callback; called by the interface when a timer fires
	virtual void intf_timer_expired(uint32_t tnum) override;

	// check interrupts; called by the interface after synchronization
	virtual void intf_check_interrupts() override;

	// mode register write; called by the interface after synchronization
	virtual void intf_mode_write(uint8_t data) override;

protected:
	// assign the current set of operators to channels
	void assign_operators();

	// update the state of the given timer
	void update_timer(uint32_t which, uint32_t enable);

	// internal state
	fm_interface &m_intf;            // reference to the system interface
	uint32_t m_env_counter;          // envelope counter; low 2 bits are sub-counter
	uint8_t m_status;                // current status register
	uint8_t m_clock_prescale;        // prescale factor (2/3/6)
	uint8_t m_irq_mask;              // mask of which bits signal IRQs
	uint8_t m_irq_state;             // current IRQ state
	uint8_t m_timer_running[2];      // current timer running state
	uint32_t m_active_channels;      // mask of active channels (computed by prepare)
	uint32_t m_modified_channels;    // mask of channels that have been modified
	uint32_t m_prepare_count;        // counter to do periodic prepare sweeps
	RegisterType m_regs;             // register accessor
	std::unique_ptr<fm_channel<RegisterType>> m_channel[CHANNELS]; // channel pointers
	std::unique_ptr<fm_operator<RegisterType>> m_operator[OPERATORS]; // operator pointers
};

// ======================> template instantiations

extern template class fm_engine_base<opm_registers>;
extern template class fm_engine_base<opn_registers>;
extern template class fm_engine_base<opna_registers>;
extern template class fm_engine_base<opl_registers>;
extern template class fm_engine_base<opl2_registers>;
extern template class fm_engine_base<opl3_registers>;

// pull these out explicitly
using opm_engine = fm_engine_base<opm_registers>;
using opn_engine = fm_engine_base<opn_registers>;
using opna_engine = fm_engine_base<opna_registers>;
using opl_engine = fm_engine_base<opl_registers>;
using opl2_engine = fm_engine_base<opl2_registers>;
using opl3_engine = fm_engine_base<opl3_registers>;
using opl4_engine = fm_engine_base<opl4_registers>;


// ======================> ymopll_engine

// ymopll_engine is a special case because instrument data needs to be
// provided from an external source
class opll_engine : public fm_engine_base<opll_registers>
{
public:
	// constructor
	opll_engine(fm_interface &intf) :
		fm_engine_base(intf)
	{
	}

	// set the instrument data
	void set_instrument_data(uint8_t const *data)
	{
		m_regs.set_instrument_data(data);
	}
};

}


#endif // MAME_SOUND_YMFM_H
