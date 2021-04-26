// license:BSD-3-Clause
// copyright-holders:Aaron Giles

#ifndef MAME_SOUND_YMFM_OPM_H
#define MAME_SOUND_YMFM_OPM_H

#pragma once

#include "ymfm.h"

namespace ymfm
{

//*********************************************************
//  REGISTER CLASSES
//*********************************************************

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



//*********************************************************
//  IMPLEMENTATION CLASSES
//*********************************************************

// ======================> ym2151

class ym2151
{
public:
	using fm_engine = fm_engine_base<opm_registers>;
	static constexpr uint32_t OUTPUTS = fm_engine::OUTPUTS;

	// constructor
	ym2151(fm_interface &intf) : ym2151(intf, VARIANT_YM2151) { }

	// reset
	void reset();

	// save/restore
	void save_restore(fm_saved_state &state);
#ifdef MAME_EMU_SAVE_H
	void register_save(device_t &device);
#endif

	// sample rate computer
	uint32_t sample_rate(uint32_t input_clock) const { return m_fm.sample_rate(input_clock); }

	// read access
	uint8_t read_status();
	uint8_t read(uint32_t offset);

	// write access
	void write_address(uint8_t data);
	void write_data(uint8_t data);
	void write(uint32_t offset, uint8_t data);

	// generate one sample of sound
	void generate(int32_t output[fm_engine::OUTPUTS]);

protected:
	// variants
	enum opm_variant
	{
		VARIANT_YM2151,
		VARIANT_YM2164,
		VARIANT_YM2414
	};

	// internal constructor
	ym2151(fm_interface &intf, opm_variant variant);

	// internal state
	opm_variant m_variant;           // chip variant
	uint8_t m_address;               // address register
	fm_engine m_fm;                  // core FM engine
};


// ======================> ym2164

class ym2164 : public ym2151
{
public:
	// constructor
	ym2164(fm_interface &intf) : ym2151(intf, VARIANT_YM2164) { }
};


// ======================> ym2414

class ym2414 : public ym2151
{
public:
	// constructor
	ym2414(fm_interface &intf) : ym2151(intf, VARIANT_YM2414) { }
};

}


#endif // MAME_SOUND_YMFM_OPM_H
