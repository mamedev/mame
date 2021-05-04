// license:BSD-3-Clause
// copyright-holders:Aaron Giles

#ifndef MAME_SOUND_YMFM_H
#define MAME_SOUND_YMFM_H

#pragma once

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <algorithm>
#include <array>
#include <memory>
#include <string>
#include <vector>

namespace ymfm
{

//*********************************************************
//  GLOBAL ENUMERATORS
//*********************************************************

enum envelope_state : uint32_t
{
	EG_DEPRESS = 0,		// OPLL only; set EG_HAS_DEPRESS to enable
	EG_ATTACK = 1,
	EG_DECAY = 2,
	EG_SUSTAIN = 3,
	EG_RELEASE = 4,
	EG_REVERB = 5,		// OPZ only; set EG_HAS_REVERB to enable
	EG_STATES = 6
};


// three different keyon sources; actual keyon is an OR over all of these
enum keyon_type : uint32_t
{
	KEYON_NORMAL = 0,
	KEYON_RHYTHM = 1,
	KEYON_CSM = 2
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
//  INTERFACE CLASSES
//*********************************************************

// forward declarations
template<class RegisterType> class fm_engine_base;


// ======================> ymfm_saved_state

// this class contains a managed vector of bytes that is used to save and
// restore state
class ymfm_saved_state
{
public:
	// construction
	ymfm_saved_state(bool saving) :
		m_offset(saving ? -1 : 0)
	{
	}

	// are we saving or restoring?
	bool saving() const { return (m_offset < 0); }

	// retrieve the buffer
	std::vector<uint8_t> &buffer() { return m_buffer; }

	// generic save/restore
	template<typename DataType>
	void save_restore(DataType &data)
	{
		if (saving())
			save(data);
		else
			restore(data);
	}

public:
	// save data to the buffer
	void save(bool &data) { write(data ? 1 : 0); }
	void save(int8_t &data) { write(data); }
	void save(uint8_t &data) { write(data); }
	void save(int16_t &data) { write(data).write(data >> 8); }
	void save(uint16_t &data) { write(data).write(data >> 8); }
	void save(int32_t &data) { write(data).write(data >> 8).write(data >> 16).write(data >> 24); }
	void save(uint32_t &data) { write(data).write(data >> 8).write(data >> 16).write(data >> 24); }
	void save(envelope_state &data) { write(uint8_t(data)); }
	template<typename DataType, int Count>
	void save(DataType (&data)[Count]) { for (int index = 0; index < Count; index++) save(data[index]); }

	// restore data from the buffer
	void restore(bool &data) { data = read() ? true : false; }
	void restore(int8_t &data) { data = read(); }
	void restore(uint8_t &data) { data = read(); }
	void restore(int16_t &data) { data = read(); data |= read() << 8; }
	void restore(uint16_t &data) { data = read(); data |= read() << 8; }
	void restore(int32_t &data) { data = read(); data |= read() << 8; data |= read() << 16; data |= read() << 24; }
	void restore(uint32_t &data) { data = read(); data |= read() << 8; data |= read() << 16; data |= read() << 24; }
	void restore(envelope_state &data) { data = envelope_state(read()); }
	template<typename DataType, int Count>
	void restore(DataType (&data)[Count]) { for (int index = 0; index < Count; index++) restore(data[index]); }

	// internal helper
	ymfm_saved_state &write(uint8_t data) { m_buffer.push_back(data); return *this; }
	uint8_t read() { return (m_offset < m_buffer.size()) ? m_buffer[m_offset++] : 0; }

	// internal state
	std::vector<uint8_t> m_buffer;
	int32_t m_offset;
};


// ======================> ymfm_engine_callbacks

// this class represents functions in the engine that the ymfm_interface
// needs to be able to call; it is represented here as a separate interface
// that is independent of the actual engine implementation
class ymfm_engine_callbacks
{
public:
	// timer callback; called by the interface when a timer fires
	virtual void engine_timer_expired(uint32_t tnum) = 0;

	// check interrupts; called by the interface after synchronization
	virtual void engine_check_interrupts() = 0;

	// mode register write; called by the interface after synchronization
	virtual void engine_mode_write(uint8_t data) = 0;
};


// ======================> ymfm_interface

// this class represents the interface between the fm_engine and the outside
// world; it provides hooks for timers, synchronization, and IRQ signaling
class ymfm_interface
{
	// the engine is our friend
	template<typename RegisterType> friend class fm_engine_base;

public:
	// constructor
	ymfm_interface() { }

	// destructor
	virtual ~ymfm_interface() { }

	// logging helper
	template<typename... Params>
	void log(Params &&... args)
	{
		char buffer[256];
		snprintf(buffer, sizeof(buffer), std::forward<Params>(args)...);
		buffer[sizeof(buffer) - 1] = 0;
		ymfm_log(buffer);
	}

	// the following functions must be implemented by any derived classes; the
	// default implementations are sufficient for some minimal operation, but will
	// likely need to be overridden to integrate with the outside world; they are
	// all prefixed with ymfm_ to reduce the likelihood of namespace collisions

	// the chip implementation calls this when a write happens to the mode
	// register, which could affect timers and interrupts; our responsibility
	// is to ensure the system is up to date before calling the engine's
	// engine_mode_write() method
	virtual void ymfm_sync_mode_write(uint8_t data) { m_engine->engine_mode_write(data); }

	// the chip implementation calls this when the chip's status has changed,
	// which may affect the interrupt state; our responsibility is to ensure
	// the system is up to date before calling the engine's
	// engine_check_interrupts() method
	virtual void ymfm_sync_check_interrupts() { m_engine->engine_check_interrupts(); }

	// the chip implementation calls this when one of the two internal timers
	// has changed state; our responsibility is to arrange to call the engine's
	// engine_timer_expired() method after the provided number of clocks; if
	// duration_in_clocks is negative, we should cancel any outstanding timers
	virtual void ymfm_set_timer(uint32_t tnum, int32_t duration_in_clocks) { }

	// the chip implementation calls this when the state of the IRQ signal has
	// changed due to a status change; our responsibility is to respons as
	// needed to the change in IRQ state, signaling any consumers
	virtual void ymfm_update_irq(bool asserted) { }

	// the chip implementation calls this to indicate that the chip should be
	// considered in a busy state until the given number of clocks has passed;
	// our responsibility is to compute and remember the ending time based on
	// the chip's clock for later checking
	virtual void ymfm_set_busy_end(uint32_t clocks) { }

	// the chip implementation calls this to see if the chip is still currently
	// is a busy state, as specified by a previous call to ymfm_set_busy_end();
	// our responsibility is to compare the current time against the previously
	// noted busy end time and return true if we haven't yet passed it
	virtual bool ymfm_is_busy() { return false; }

	// the chip implementation calls this whenever the internal clock prescaler
	// changes; our responsibility is to adjust our clocking of the chip in
	// response to produce the correct output rate
	virtual void ymfm_prescale_changed() { }

	// the chip implementation calls this whenever a new value is written to
	// one of the chip's output ports (only applies to some chip types); our
	// responsibility is to pass the written data on to any consumers
	virtual void ymfm_io_write(uint8_t port, uint8_t data) { }

	// the chip implementation calls this whenever an on-chip register is read
	// which returns data from one of the chip's input ports; our responsibility
	// is to produce the current input value so that it can be reflected by the
	// read operation
	virtual uint8_t ymfm_io_read(uint8_t port) { return 0; }

	// the chip implementation calls this whenever the ADPCM-A engine needs to
	// fetch data for sound generation; our responsibility is to read the data
	// from the appropriate ROM/RAM at the given offset and return it
	virtual uint8_t ymfm_adpcm_a_read(uint32_t offset) { return 0; }

	// the chip implementation calls this whenever the ADPCM-B engine needs to
	// fetch data for sound generation; our responsibility is to read the data
	// from the appropriate ROM/RAM at the given offset and return it
	virtual uint8_t ymfm_adpcm_b_read(uint32_t offset) { return 0; }

	// the chip implementation calls this whenever the ADPCM-B engine requests
	// a write to the sound data; our responsibility is to write the data to
	// the appropriate RAM at the given offset
	virtual void ymfm_adpcm_b_write(uint32_t offset, uint8_t data) { }

	// the chip implementation calls this to log warnings or other information;
	// our responsibility is to either ignore it or surface it for debugging
	virtual void ymfm_log(char const *string) { }

protected:
	// pointer to engine callbacks -- this is set directly by the engine at
	// construction time
	ymfm_engine_callbacks *m_engine;
};



//*********************************************************
//  CORE IMPLEMENTATION
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
	int32_t detune;                   // detuning value (used to compute phase_step)
	uint32_t multiple;                // multiple value (x.1, used to compute phase_step)
	uint32_t eg_sustain;              // sustain level, shifted up to envelope values
	uint8_t eg_rate[EG_STATES];       // envelope rate, including KSR
	uint8_t eg_shift = 0;             // envelope shift amount
};


// ======================> fm_registers_base

// base class for family-specific register classes; this provides a few
// constants, common defaults, and helpers, but mostly each derived class is
// responsible for defining all commonly-called methods
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
	//        uint32_t WAVEFORMS: The number of waveforms offered
	//        uint32_t REGISTERS: The number of 8-bit registers allocated
	// uint32_t DEFAULT_PRESCALE: The starting clock prescale
	// uint32_t EG_CLOCK_DIVIDER: The clock divider of the envelope generator
	// uint32_t CSM_TRIGGER_MASK: Mask of channels to trigger in CSM mode
	//         uint32_t REG_MODE: The address of the "mode" register controlling timers
	//     uint8_t STATUS_TIMERA: Status bit to set when timer A fires
	//     uint8_t STATUS_TIMERB: Status bit to set when tiemr B fires
	//       uint8_t STATUS_BUSY: Status bit to set when the chip is busy
	//        uint8_t STATUS_IRQ: Status bit to set when an IRQ is signalled
	//
	// the following constants are uncommon:
	//          bool DYNAMIC_OPS: True if ops/channel can be changed at runtime (OPL3+)
	//       bool EG_HAS_DEPRESS: True if the chip has a DP ("depress"?) envelope stage (OPLL)
	//        bool EG_HAS_REVERB: True if the chip has a faux reverb envelope stage (OPZ)
	//           bool EG_HAS_SSG: True if the chip has SSG envelope support (OPN)
	//      bool MODULATOR_DELAY: True if the modulator is delayed by 1 sample (OPL pre-OPL3)
	//
	static constexpr bool DYNAMIC_OPS = false;
	static constexpr bool EG_HAS_DEPRESS = false;
	static constexpr bool EG_HAS_REVERB = false;
	static constexpr bool EG_HAS_SSG = false;
	static constexpr bool MODULATOR_DELAY = false;

	// system-wide register defaults
	uint32_t status_mask() const                     { return 0; } // OPL only
	uint32_t irq_reset() const                       { return 0; } // OPL only
	uint32_t noise_enable() const                    { return 0; } // OPM only
	uint32_t rhythm_enable() const                   { return 0; } // OPL only

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

	// save/restore
	void save_restore(ymfm_saved_state &state);
#ifdef MAME_EMU_SAVE_H
	void register_save(device_t &device);
#endif

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

	// save/restore
	void save_restore(ymfm_saved_state &state);
#ifdef MAME_EMU_SAVE_H
	void register_save(device_t &device);
#endif

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
class fm_engine_base : public ymfm_engine_callbacks
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
	fm_engine_base(ymfm_interface &intf);

	// save/restore
	void save_restore(ymfm_saved_state &state);
#ifdef MAME_EMU_SAVE_H
	void register_save(device_t &device);
#endif

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
		m_status = (m_status | set) & ~(reset | STATUS_BUSY);
		m_intf.ymfm_sync_check_interrupts();
		return m_status;
	}

	// set the IRQ mask
	void set_irq_mask(uint8_t mask) { m_irq_mask = mask; m_intf.ymfm_sync_check_interrupts(); }

	// return the current clock prescale
	uint32_t clock_prescale() const { return m_clock_prescale; }

	// set prescale factor (2/3/6)
	void set_clock_prescale(uint32_t prescale) { m_clock_prescale = prescale; m_intf.ymfm_prescale_changed(); }

	// compute sample rate
	uint32_t sample_rate(uint32_t baseclock) const { return baseclock / (m_clock_prescale * OPERATORS); }

	// return the owning device
	ymfm_interface &intf() const { return m_intf; }

	// return a reference to our registers
	RegisterType &regs() { return m_regs; }

	// invalidate any caches
	void invalidate_caches() { m_modified_channels = RegisterType::ALL_CHANNELS; }

	// timer callback; called by the interface when a timer fires
	virtual void engine_timer_expired(uint32_t tnum) override;

	// check interrupts; called by the interface after synchronization
	virtual void engine_check_interrupts() override;

	// mode register write; called by the interface after synchronization
	virtual void engine_mode_write(uint8_t data) override;

protected:
	// assign the current set of operators to channels
	void assign_operators();

	// update the state of the given timer
	void update_timer(uint32_t which, uint32_t enable);

	// internal state
	ymfm_interface &m_intf;            // reference to the system interface
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

#endif // MAME_SOUND_YMFM_H
