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

#ifndef YMFM_OPX_H
#define YMFM_OPX_H

#pragma once

#include "ymfm.h"
#include "ymfm_fm.h"

namespace ymfm
{

//*********************************************************
//  REGISTER CLASSES
//*********************************************************

// ======================> opx_registers

//
// OPX register map:
//
//      System-wide registers:
//
//     Per-channel registers (channel in address bits 0-2)
//
//     Per-operator registers (4 banks):
//        00-0F x------- Enable
//              -xxxx--- EXT out
//              -------x Key on
//        10-1F xxxxxxxx LFO frequency
//        20-2F xx------ AM sensitivity (0-3)
//              --xxx--- PM sensitivity (0-7)
//              ------xx LFO waveform (0=disable, 1=saw, 2=
//        30-3F -xxx---- Detune (0-7)
//              ----xxxx Multiple (0-15)
//        40-4F -xxxxxxx Total level (0-127)
//        50-5F xxx----- Key scale (0-7)
//              ---xxxxx Attack rate (0-31)
//        60-6F ---xxxxx Decay rate (0-31)
//        70-7F ---xxxxx Sustain rate (0-31)
//        80-8F xxxx---- Sustain level (0-15)
//              ----xxxx Release rate (0-15)
//        90-9F xxxxxxxx Frequency number (low 8 bits)
//        A0-AF xxxx---- Block (0-15)
//              ----xxxx Frequency number (high 4 bits)
//        B0-BF x------- Acc on
//              -xxx---- Feedback level (0-7)
//              -----xxx Waveform (0-7, 7=PCM)
//        C0-CF ----xxxx Algorithm (0-15)
//        D0-DF xxxx---- CH0 level (0-15)
//              ----xxxx CH1 level (0-15)
//        E0-EF xxxx---- CH2 level (0-15)
//              ----xxxx CH3 level (0-15)
//

class opx_registers : public fm_registers_base
{
	// LFO waveforms are 256 entries long
	static constexpr uint32_t LFO_WAVEFORM_LENGTH = 256;

public:
	// constants
	static constexpr uint32_t OUTPUTS = 8;
	static constexpr uint32_t CHANNELS = 24;
	static constexpr uint32_t ALL_CHANNELS = (1 << CHANNELS) - 1;
	static constexpr uint32_t OPERATORS = CHANNELS * 2;
	static constexpr uint32_t WAVEFORMS = 8;
	static constexpr uint32_t REGISTERS = 0x800;
	static constexpr uint32_t DEFAULT_PRESCALE = 8;
	static constexpr uint32_t EG_CLOCK_DIVIDER = 2;
	static constexpr uint32_t CSM_TRIGGER_MASK = ALL_CHANNELS;
	static constexpr uint32_t REG_MODE = 0x14;
	static constexpr uint8_t STATUS_TIMERA = 0x01;
	static constexpr uint8_t STATUS_TIMERB = 0x02;
	static constexpr uint8_t STATUS_BUSY = 0x80;
	static constexpr uint8_t STATUS_IRQ = 0;

	// constructor
	opx_registers();

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
	uint32_t noise_frequency() const                 { return byte(0x0f, 0, 5); }
	uint32_t noise_enable() const                    { return byte(0x0f, 7, 1); }
	uint32_t timer_a_value() const                   { return word(0x10, 0, 8, 0x11, 0, 2); }
	uint32_t timer_b_value() const                   { return byte(0x12, 0, 8); }
	uint32_t csm() const                             { return byte(0x14, 7, 1); }
	uint32_t reset_timer_b() const                   { return byte(0x14, 5, 1); }
	uint32_t reset_timer_a() const                   { return byte(0x14, 4, 1); }
	uint32_t enable_timer_b() const                  { return byte(0x14, 3, 1); }
	uint32_t enable_timer_a() const                  { return byte(0x14, 2, 1); }
	uint32_t load_timer_b() const                    { return byte(0x14, 1, 1); }
	uint32_t load_timer_a() const                    { return byte(0x14, 0, 1); }
	uint32_t lfo2_pm_depth() const                   { return byte(0x148, 0, 7); } // fake
	uint32_t lfo2_rate() const                       { return byte(0x16, 0, 8); }
	uint32_t lfo2_am_depth() const                   { return byte(0x17, 0, 7); }
	uint32_t lfo_rate() const                        { return byte(0x18, 0, 8); }
	uint32_t lfo_am_depth() const                    { return byte(0x19, 0, 7); }
	uint32_t lfo_pm_depth() const                    { return byte(0x149, 0, 7); } // fake
	uint32_t output_bits() const                     { return byte(0x1b, 6, 2); }
	uint32_t lfo2_sync() const                       { return byte(0x1b, 5, 1); }
	uint32_t lfo_sync() const                        { return byte(0x1b, 4, 1); }
	uint32_t lfo2_waveform() const                   { return byte(0x1b, 2, 2); }
	uint32_t lfo_waveform() const                    { return byte(0x1b, 0, 2); }

	// per-channel registers
	uint32_t ch_volume(uint32_t choffs) const        { return byte(0x00, 0, 8, choffs); }
	uint32_t ch_output_any(uint32_t choffs) const    { return byte(0x20, 7, 1, choffs) | byte(0x30, 0, 1, choffs); }
	uint32_t ch_output_0(uint32_t choffs) const      { return byte(0x30, 0, 1, choffs); }
	uint32_t ch_output_1(uint32_t choffs) const      { return byte(0x20, 7, 1, choffs) | byte(0x30, 0, 1, choffs); }
	uint32_t ch_output_2(uint32_t choffs) const      { return 0; }
	uint32_t ch_output_3(uint32_t choffs) const      { return 0; }
	uint32_t ch_key_on(uint32_t choffs) const        { return byte(0x20, 6, 1, choffs); }
	uint32_t ch_feedback(uint32_t choffs) const      { return byte(0x20, 3, 3, choffs); }
	uint32_t ch_algorithm(uint32_t choffs) const     { return byte(0x20, 0, 3, choffs); }
	uint32_t ch_block_freq(uint32_t choffs) const    { return word(0x28, 0, 7, 0x30, 2, 6, choffs); }
	uint32_t ch_lfo_pm_sens(uint32_t choffs) const   { return byte(0x38, 4, 3, choffs); }
	uint32_t ch_lfo_am_sens(uint32_t choffs) const   { return byte(0x38, 0, 2, choffs); }
	uint32_t ch_lfo2_pm_sens(uint32_t choffs) const  { return byte(0x140, 4, 3, choffs); } // fake
	uint32_t ch_lfo2_am_sens(uint32_t choffs) const  { return byte(0x140, 0, 2, choffs); } // fake

	// per-operator registers
	uint32_t op_detune(uint32_t opoffs) const        { return byte(0x40, 4, 3, opoffs); }
	uint32_t op_multiple(uint32_t opoffs) const      { return byte(0x40, 0, 4, opoffs); }
	uint32_t op_fix_range(uint32_t opoffs) const     { return byte(0x40, 4, 3, opoffs); }
	uint32_t op_fix_frequency(uint32_t opoffs) const { return byte(0x40, 0, 4, opoffs); }
	uint32_t op_waveform(uint32_t opoffs) const      { return byte(0x100, 4, 3, opoffs); } // fake
	uint32_t op_fine(uint32_t opoffs) const          { return byte(0x100, 0, 4, opoffs); } // fake
	uint32_t op_total_level(uint32_t opoffs) const   { return byte(0x60, 0, 7, opoffs); }
	uint32_t op_ksr(uint32_t opoffs) const           { return byte(0x80, 6, 2, opoffs); }
	uint32_t op_fix_mode(uint32_t opoffs) const      { return byte(0x80, 5, 1, opoffs); }
	uint32_t op_attack_rate(uint32_t opoffs) const   { return byte(0x80, 0, 5, opoffs); }
	uint32_t op_lfo_am_enable(uint32_t opoffs) const { return byte(0xa0, 7, 1, opoffs); }
	uint32_t op_decay_rate(uint32_t opoffs) const    { return byte(0xa0, 0, 5, opoffs); }
	uint32_t op_detune2(uint32_t opoffs) const       { return byte(0xc0, 6, 2, opoffs); }
	uint32_t op_sustain_rate(uint32_t opoffs) const  { return byte(0xc0, 0, 5, opoffs); }
	uint32_t op_eg_shift(uint32_t opoffs) const      { return byte(0x120, 6, 2, opoffs); } // fake
	uint32_t op_reverb_rate(uint32_t opoffs) const   { return byte(0x120, 0, 3, opoffs); } // fake
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
	uint32_t m_lfo_counter[2];            // LFO counter
	uint32_t m_noise_lfsr;                // noise LFSR state
	uint8_t m_noise_counter;              // noise counter
	uint8_t m_noise_state;                // latched noise state
	uint8_t m_noise_lfo;                  // latched LFO noise value
	uint8_t m_lfo_am[2];                  // current LFO AM value
	uint8_t m_regdata[REGISTERS];         // register data
	uint16_t m_phase_substep[OPERATORS];  // phase substep for fixed frequency
	int16_t m_lfo_waveform[4][LFO_WAVEFORM_LENGTH]; // LFO waveforms; AM in low 8, PM in upper 8
	uint16_t m_waveform[WAVEFORMS][WAVEFORM_LENGTH]; // waveforms
};



//*********************************************************
//  IMPLEMENTATION CLASSES
//*********************************************************

// ======================> ymf271

class ymf271
{
public:
	using fm_engine = fm_engine_base<opx_registers>;
	static constexpr uint32_t OUTPUTS = fm_engine::OUTPUTS;
	using output_data = fm_engine::output_data;

	// constructor
	ymf271(ymfm_interface &intf);

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
	void write_address(uint8_t data);
	void write_data(uint8_t data);
	void write(uint32_t offset, uint8_t data);

	// generate one sample of sound
	void generate(output_data *output, uint32_t numsamples = 1);

protected:
	// internal state
	uint8_t m_address;               // address register
	fm_engine m_fm;                  // core FM engine
};

}


#endif // YMFM_OPX_H
