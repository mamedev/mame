// BSD 3-Clause License
//
// Copyright (c) 2021, Aaron Giles
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef YMFM_OPQ_H
#define YMFM_OPQ_H

#pragma once

#include "ymfm.h"
#include "ymfm_fm.h"

namespace ymfm
{

//*********************************************************
//  REGISTER CLASSES
//*********************************************************

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
//        18-1F x------- Reverb
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
//        80-9F xx------ Key scale rate (0-3)
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
//  - retrigger disable
//  - 2 waveforms
//  - uses FNUM
//  - reverb behavior
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
	static constexpr uint32_t WAVEFORMS = 2;
	static constexpr uint32_t REGISTERS = 0x120;
	static constexpr uint32_t REG_MODE = 0x03;
	static constexpr uint32_t DEFAULT_PRESCALE = 2;
	static constexpr uint32_t EG_CLOCK_DIVIDER = 3;
	static constexpr bool EG_HAS_REVERB = true;
	static constexpr bool MODULATOR_DELAY = false;
	static constexpr uint32_t CSM_TRIGGER_MASK = ALL_CHANNELS;
	static constexpr uint8_t STATUS_TIMERA = 0;
	static constexpr uint8_t STATUS_TIMERB = 0x04;
	static constexpr uint8_t STATUS_BUSY = 0x80;
	static constexpr uint8_t STATUS_IRQ = 0;

	// constructor
	opq_registers();

	// reset to initial state
	void reset();

	// save/restore
	void save_restore(ymfm_saved_state &state);

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
	uint32_t timer_a_value() const                   { return 0; }
	uint32_t timer_b_value() const                   { return byte(0x03, 2, 6) | 0xc0; } // ???
	uint32_t csm() const                             { return 0; }
	uint32_t reset_timer_b() const                   { return byte(0x03, 0, 1); } // ???
	uint32_t reset_timer_a() const                   { return 0; }
	uint32_t enable_timer_b() const                  { return byte(0x03, 0, 1); } // ???
	uint32_t enable_timer_a() const                  { return 0; }
	uint32_t load_timer_b() const                    { return byte(0x03, 0, 1); } // ???
	uint32_t load_timer_a() const                    { return 0; }
	uint32_t lfo_enable() const                      { return byte(0x04, 3, 1) ^ 1; }
	uint32_t lfo_rate() const                        { return byte(0x04, 0, 3); }

	// per-channel registers
	uint32_t ch_output_any(uint32_t choffs) const    { return byte(0x10, 6, 2, choffs); }
	uint32_t ch_output_0(uint32_t choffs) const      { return byte(0x10, 6, 1, choffs); }
	uint32_t ch_output_1(uint32_t choffs) const      { return byte(0x10, 7, 1, choffs); }
	uint32_t ch_output_2(uint32_t choffs) const      { return 0; }
	uint32_t ch_output_3(uint32_t choffs) const      { return 0; }
	uint32_t ch_feedback(uint32_t choffs) const      { return byte(0x10, 3, 3, choffs); }
	uint32_t ch_algorithm(uint32_t choffs) const     { return byte(0x10, 0, 3, choffs); }
	uint32_t ch_reverb(uint32_t choffs) const        { return byte(0x18, 7, 1, choffs); }
	uint32_t ch_lfo_pm_sens(uint32_t choffs) const   { return byte(0x18, 4, 3, choffs); }
	uint32_t ch_lfo_am_sens(uint32_t choffs) const   { return byte(0x18, 0, 2, choffs); }
	uint32_t ch_block_freq_24(uint32_t choffs) const { return word(0x20, 0, 7, 0x30, 0, 8, choffs); }
	uint32_t ch_block_freq_13(uint32_t choffs) const { return word(0x28, 0, 7, 0x38, 0, 8, choffs); }

	// per-operator registers
	uint32_t op_detune(uint32_t opoffs) const        { return byte(0x40, 0, 6, opoffs); }
	uint32_t op_multiple(uint32_t opoffs) const      { return byte(0x100, 0, 4, opoffs); }
	uint32_t op_total_level(uint32_t opoffs) const   { return byte(0x60, 0, 7, opoffs); }
	uint32_t op_ksr(uint32_t opoffs) const           { return byte(0x80, 6, 2, opoffs); }
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
//  IMPLEMENTATION CLASSES
//*********************************************************

// ======================> ym3806

class ym3806
{
public:
	using fm_engine = fm_engine_base<opq_registers>;
	static constexpr uint32_t OUTPUTS = fm_engine::OUTPUTS;
	using output_data = fm_engine::output_data;

	// constructor
	ym3806(ymfm_interface &intf);

	// reset
	void reset();

	// save/restore
	void save_restore(ymfm_saved_state &state);

	// pass-through helpers
	uint32_t sample_rate(uint32_t input_clock) const { return m_fm.sample_rate(input_clock); }
	void invalidate_caches() { m_fm.invalidate_caches(); }

	// read access
	uint8_t read_status();
	uint8_t read(uint32_t offset);

	// write access
	void write_address(uint8_t data) { /* not supported; only direct writes */ }
	void write_data(uint8_t data) { /* not supported; only direct writes */ }
	void write(uint32_t offset, uint8_t data);

	// generate one sample of sound
	void generate(output_data *output, uint32_t numsamples = 1);

protected:
	// internal state
	fm_engine m_fm;                  // core FM engine
};


// ======================> ym3533

class ym3533 : public ym3806
{
public:
	// constructor
	ym3533(ymfm_interface &intf) :
		ym3806(intf) { }
};

}


#endif // YMFM_OPQ_H
