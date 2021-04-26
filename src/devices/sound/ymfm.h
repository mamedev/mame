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


// ======================> fm_engine_callbacks

// this class represents functions in the engine that the fm_interface
// needs to be able to call; it is represented here as a separate
// interface that is independent of the actual engine implementation
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


// ======================> fm_interface

// this class represents the interface between the fm_engine and the
// outside world; it provides hooks for timers, synchronization, and
// IRQ signaling
class fm_interface
{
	// the engine is our friend
	template<typename RegisterType> friend class fm_engine_base;

public:
	// constructor
	fm_interface() { }

	// destructor
	virtual ~fm_interface() { }

	virtual void save()
	{
	}

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

	// the following functions must be implemented by any derived classes

	// the engine calls this to schedule a synchronized write to the mode
	// register with the provided data; in response, the interface should
	// clock through any unprocessed stream data and then call the
	// m_callbacks->intf_mode_write() method with the provided data
	virtual void synchronized_mode_write(uint8_t data) = 0;

	// the engine calls this to schedule a synchronized interrupt check;
	// in response, the interface should clock through any unprocessed
	// stream data and then call the m_callbacks->intf_check_interrupts() method
	virtual void synchronized_check_interrupts() = 0;

	// the engine calls this to set one of two timers which should fire after
	// the given number of clocks; when the timer fires, it must call the
	// m_callbacks->intf_timer_handler() method; a duration_in_clocks that is
	// negative means to disable the timer
	virtual void set_timer(uint32_t tnum, int32_t duration_in_clocks) = 0;

	// the engine calls this to set the time when the busy signal will next
	// be dropped, which is the current time plus the given number of clocks
	virtual void set_busy_end(uint32_t clocks) = 0;

	// the engine calls this to query whether the current time is before the
	// last set end-of-busy time
	virtual bool is_busy() = 0;

	// the engine calls this when the external IRQ state changes; in
	// response, the interface should perform any IRQ signaling that needs
	// to happen as a result
	virtual void update_irq(bool asserted) { }

	// the engine calls this when a write to an output port is issued; only
	// the OPM supports this at this time
	virtual void output_port(uint8_t data) { }

protected:
	// pointer to engine callbacks -- this is set direclty by the engine at
	// construction time
	fm_engine_callbacks *m_callbacks;
};


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


}



// ======================> mame_fm_interface

// this class abstracts out system-dependent behaviors and interfaces
#ifndef NOT_MAME
class mame_fm_interface : public ymfm::fm_interface
{
public:
	// construction
	mame_fm_interface(device_t &device) :
		m_device(device),
		m_update_irq(device),
		m_output_port(device)
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
		m_update_irq.resolve();
		m_output_port.resolve_safe();
	}

	// configuration helpers
	auto update_irq_handler() { return m_update_irq.bind(); }
	auto output_port_handler() { return m_output_port.bind(); }

	virtual void save() override
	{
		m_device.save_item(NAME(m_busy_end));
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

	virtual void set_timer(uint32_t tnum, int32_t duration_in_clocks) override
	{
		if (duration_in_clocks >= 0)
			m_timer[tnum]->adjust(attotime::from_ticks(duration_in_clocks, m_device.clock()), tnum);
		else
			m_timer[tnum]->enable(false);
	}

	virtual void set_busy_end(uint32_t clocks) override
	{
		m_busy_end = m_device.machine().time() + attotime::from_ticks(clocks, m_device.clock());
	}

	virtual bool is_busy() override
	{
		return (m_device.machine().time() < m_busy_end);
	}

	virtual void update_irq(bool asserted) override
	{
		if (!m_update_irq.isnull())
			m_update_irq(asserted ? ASSERT_LINE : CLEAR_LINE);
	}

	virtual void output_port(uint8_t data) override
	{
		if (!m_output_port.isnull())
			m_output_port(data);
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
	// timer callbacks
	void timer_handler(void *ptr, int param) { m_callbacks->intf_timer_expired(param); }
	void mode_write(void *ptr, int param) { m_callbacks->intf_mode_write(param); }
	void check_interrupts(void *ptr, int param) { m_callbacks->intf_check_interrupts(); }

	// internal state
	device_t &m_device;
	attotime m_busy_end;
	emu_timer *m_timer[2];
	devcb_write_line m_update_irq;
	devcb_write8 m_output_port;
};
#endif


#endif // MAME_SOUND_YMFM_H
