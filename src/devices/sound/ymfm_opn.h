// license:BSD-3-Clause
// copyright-holders:Aaron Giles

#ifndef MAME_SOUND_YMFM_OPN_H
#define MAME_SOUND_YMFM_OPN_H

#pragma once

#include "ymfm.h"
#include "ymadpcm.h"

namespace ymfm
{

//*********************************************************
//  SSG ENGINE
//*********************************************************

// ======================> ssg_engine

// this class represents a built-in overridable SSG implementation; at this
// time it is not implemented, so you will have to add your own, or else live
// with no SSG audio
class ssg_engine
{
public:
	static constexpr uint32_t OUTPUTS = ssg_interface::OUTPUTS;

	// constructor: if you pass an ssg_interface, that will override
	ssg_engine() : m_intf(&m_default_intf) { }

	// engine override
	void override(ssg_interface &intf) { m_intf = &intf; }

	// reset
	void reset() { m_intf->ssg_reset(); }

	// save/restore
	void save_restore(fm_saved_state &state) { m_intf->ssg_save_restore(state); }
#ifdef MAME_EMU_SAVE_H
	void register_save(device_t &device) { m_intf->ssg_register_save(device); }
#endif

	// set the clock prescale value
	void set_clock_prescale(uint8_t clock_divider) { m_intf->ssg_set_clock_prescale(clock_divider); }

	// read access
	uint8_t read(uint8_t offset) { return m_intf->ssg_read(offset); }

	// write access
	void write(uint8_t offset, uint8_t data) { m_intf->ssg_write(offset, data); }

	// generate one sample
	void generate(int32_t output[OUTPUTS]) { m_intf->ssg_generate(output); }

private:
	// internal state
	ssg_interface *m_intf;
	ssg_interface m_default_intf;
};


//*********************************************************
//  REGISTER CLASSES
//*********************************************************

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

	// reset to initial state
	void reset();

	// save/restore
	void save_restore(fm_saved_state &state);
#ifdef MAME_EMU_SAVE_H
	void register_save(device_t &save);
#endif

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



//*********************************************************
//  IMPLEMENTATION CLASSES
//*********************************************************

// ======================> ym2203

class ym2203
{
public:
	using fm_engine = fm_engine_base<opn_registers>;
	static constexpr uint32_t OUTPUTS = fm_engine::OUTPUTS;
	static constexpr uint32_t SSG_OUTPUTS = ssg_engine::OUTPUTS;

	// constructor
	ym2203(fm_interface &intf);

	// configuration
	void ssg_override(ssg_interface &intf) { m_ssg.override(intf); }

	// reset
	void reset();

	// save/restore
	void save_restore(fm_saved_state &state);
#ifdef MAME_EMU_SAVE_H
	void register_save(device_t &device);
#endif

	// pass-through helpers
	uint32_t sample_rate(uint32_t input_clock) const { return m_fm.sample_rate(input_clock); }
	uint32_t sample_rate_ssg(uint32_t input_clock) const { uint32_t scale = m_fm.clock_prescale() * 2 / 3; return input_clock * 2 / scale; }
	void invalidate_caches() { m_fm.invalidate_caches(); }

	// read access
	uint8_t read_status();
	uint8_t read_data();
	uint8_t read(uint32_t offset);

	// write access
	void write_address(uint8_t data);
	void write_data(uint8_t data);
	void write(uint32_t offset, uint8_t data);

	// generate one sample of sound
	void generate(int32_t output[fm_engine::OUTPUTS]);
	void generate_ssg(int32_t output[ssg_engine::OUTPUTS]) { m_ssg.generate(output); }

protected:
	// internal updates
	void update_prescale(uint8_t prescale);

	// internal state
	uint8_t m_address;               // address register
	fm_engine m_fm;                  // core FM engine
	ssg_engine m_ssg;                // SSG engine
};


// ======================> ym2608

class ym2608
{
	static constexpr uint8_t STATUS_ADPCM_B_EOS = 0x04;
	static constexpr uint8_t STATUS_ADPCM_B_BRDY = 0x08;
	static constexpr uint8_t STATUS_ADPCM_B_ZERO = 0x10;
	static constexpr uint8_t STATUS_ADPCM_B_PLAYING = 0x20;

public:
	using fm_engine = fm_engine_base<opna_registers>;
	static constexpr uint32_t OUTPUTS = fm_engine::OUTPUTS;
	static constexpr uint32_t SSG_OUTPUTS = ssg_engine::OUTPUTS;

	// constructor
	ym2608(fm_interface &intf);

	// configuration
	void ssg_override(ssg_interface &intf) { m_ssg.override(intf); }

	// reset
	void reset();

	// save/restore
	void save_restore(fm_saved_state &state);
#ifdef MAME_EMU_SAVE_H
	void register_save(device_t &device);
#endif

	// pass-through helpers
	uint32_t sample_rate(uint32_t input_clock) const { return m_fm.sample_rate(input_clock); }
	uint32_t sample_rate_ssg(uint32_t input_clock) const { uint32_t scale = m_fm.clock_prescale() * 2 / 3; return input_clock * 2 / scale; }
	void invalidate_caches() { m_fm.invalidate_caches(); }

	// read access
	uint8_t read_status();
	uint8_t read_data();
	uint8_t read_status_hi();
	uint8_t read_data_hi();
	uint8_t read(uint32_t offset);

	// write access
	void write_address(uint8_t data);
	void write_data(uint8_t data);
	void write_address_hi(uint8_t data);
	void write_data_hi(uint8_t data);
	void write(uint32_t offset, uint8_t data);

	// generate one sample of sound
	void generate(int32_t output[fm_engine::OUTPUTS]);
	void generate_ssg(int32_t output[ssg_engine::OUTPUTS]) { m_ssg.generate(output); }

protected:
	// internal updates
	void update_prescale(uint8_t prescale);

	// internal state
	uint16_t m_address;            // address register
	uint8_t m_irq_enable;          // IRQ enable register
	uint8_t m_flag_control;        // flag control register
	fm_engine m_fm;                // core FM engine
	ssg_engine m_ssg;              // SSG engine
	adpcm_a_engine m_adpcm_a;      // ADPCM-A engine
	adpcm_b_engine m_adpcm_b;      // ADPCM-B engine
};


// ======================> ym2610/ym2610b

class ym2610
{
public:
	using fm_engine = fm_engine_base<opna_registers>;
	static constexpr uint32_t OUTPUTS = fm_engine::OUTPUTS;
	static constexpr uint32_t SSG_OUTPUTS = ssg_engine::OUTPUTS;

	// constructor
	ym2610(fm_interface &intf, uint8_t channel_mask = 0x36);

	// configuration
	void ssg_override(ssg_interface &intf) { m_ssg.override(intf); }

	// reset
	void reset();

	// save/restore
	void save_restore(fm_saved_state &state);
#ifdef MAME_EMU_SAVE_H
	void register_save(device_t &device);
#endif

	// pass-through helpers
	uint32_t sample_rate(uint32_t input_clock) const { return m_fm.sample_rate(input_clock); }
	uint32_t sample_rate_ssg(uint32_t input_clock) const { return input_clock / 4; }
	void invalidate_caches() { m_fm.invalidate_caches(); }

	// read access
	uint8_t read_status();
	uint8_t read_data();
	uint8_t read_status_hi();
	uint8_t read_data_hi();
	uint8_t read(uint32_t offset);

	// write access
	void write_address(uint8_t data);
	void write_data(uint8_t data);
	void write_address_hi(uint8_t data);
	void write_data_hi(uint8_t data);
	void write(uint32_t offset, uint8_t data);

	// generate one sample of sound
	void generate(int32_t output[fm_engine::OUTPUTS]);
	void generate_ssg(int32_t output[ssg_engine::OUTPUTS]) { m_ssg.generate(output); }

protected:
	// internal state
	uint16_t m_address;            // address register
	uint8_t const m_fm_mask;       // FM channel mask
	uint8_t m_eos_status;          // end-of-sample signals
	uint8_t m_flag_mask;           // flag mask control
	fm_engine m_fm;                // core FM engine
	ssg_engine m_ssg;              // core FM engine
	adpcm_a_engine m_adpcm_a;      // ADPCM-A engine
	adpcm_b_engine m_adpcm_b;      // ADPCM-B engine
};

class ym2610b : public ym2610
{
public:
	// constructor
	ym2610b(fm_interface &intf) : ym2610(intf, 0x3f) { }
};


// ======================> ym2612/ym3438/ymf276

class ym2612
{
public:
	using fm_engine = fm_engine_base<opna_registers>;
	static constexpr uint32_t OUTPUTS = fm_engine::OUTPUTS;

	// constructor
	ym2612(fm_interface &intf);

	// reset
	void reset();

	// save/restore
	void save_restore(fm_saved_state &state);
#ifdef MAME_EMU_SAVE_H
	void register_save(device_t &device);
#endif

	// pass-through helpers
	uint32_t sample_rate(uint32_t input_clock) const { return m_fm.sample_rate(input_clock); }
	void invalidate_caches() { m_fm.invalidate_caches(); }

	// read access
	uint8_t read_status();
	uint8_t read(uint32_t offset);

	// write access
	void write_address(uint8_t data);
	void write_data(uint8_t data);
	void write_address_hi(uint8_t data);
	void write_data_hi(uint8_t data);
	void write(uint32_t offset, uint8_t data);

	// generate one sample of sound
	void generate(int32_t output[fm_engine::OUTPUTS]);

protected:
	// internal state
	uint16_t m_address;              // address register
	uint16_t m_dac_data;             // 9-bit DAC data
	uint8_t m_dac_enable;            // DAC enabled?
	fm_engine m_fm;                  // core FM engine
};

class ym3438 : public ym2612
{
public:
	ym3438(fm_interface &intf) : ym2612(intf) { }

	// generate one sample of sound
	void generate(int32_t output[fm_engine::OUTPUTS]);
};

class ymf276 : public ym2612
{
public:
	ymf276(fm_interface &intf) : ym2612(intf) { }

	// generate one sample of sound
	void generate(int32_t output[fm_engine::OUTPUTS]);
};

}


#endif // MAME_SOUND_YMFM_OPN_H
