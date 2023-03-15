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

#ifndef YMFM_OPN_H
#define YMFM_OPN_H

#pragma once

#include "ymfm.h"
#include "ymfm_adpcm.h"
#include "ymfm_fm.h"
#include "ymfm_ssg.h"

namespace ymfm
{

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
	static constexpr uint32_t WAVEFORMS = 1;
	static constexpr uint32_t REGISTERS = IsOpnA ? 0x200 : 0x100;
	static constexpr uint32_t REG_MODE = 0x27;
	static constexpr uint32_t DEFAULT_PRESCALE = 6;
	static constexpr uint32_t EG_CLOCK_DIVIDER = 3;
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
	void save_restore(ymfm_saved_state &state);

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

	// read a register value
	uint8_t read(uint16_t index) const { return m_regdata[index]; }

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
//  OPN IMPLEMENTATION CLASSES
//*********************************************************

// A note about prescaling and sample rates.
//
// YM2203, YM2608, and YM2610 contain an onboard SSG (basically, a YM2149).
// In order to properly generate sound at fully fidelity, the output sample
// rate of the YM2149 must be input_clock / 8. This is much higher than the
// FM needs, but in the interest of keeping things simple, the OPN generate
// functions will output at the higher rate and just replicate the last FM
// sample as many times as needed.
//
// To make things even more complicated, the YM2203 and YM2608 allow for
// software-controlled prescaling, which affects the FM and SSG clocks in
// different ways. There are three settings: divide by 6/4 (FM/SSG); divide
// by 3/2; and divide by 2/1.
//
// Thus, the minimum output sample rate needed by each part of the chip
// varies with the prescale as follows:
//
//             ---- YM2203 -----    ---- YM2608 -----    ---- YM2610 -----
// Prescale    FM rate  SSG rate    FM rate  SSG rate    FM rate  SSG rate
//     6         /72      /16         /144     /32          /144    /32
//     3         /36      /8          /72      /16
//     2         /24      /4          /48      /8
//
// If we standardized on the fastest SSG rate, we'd end up with the following
// (ratios are output_samples:source_samples):
//
//             ---- YM2203 -----    ---- YM2608 -----    ---- YM2610 -----
//              rate = clock/4       rate = clock/8       rate = clock/16
// Prescale    FM rate  SSG rate    FM rate  SSG rate    FM rate  SSG rate
//     6         18:1     4:1         18:1     4:1          9:1    2:1
//     3          9:1     2:1          9:1     2:1
//     2          6:1     1:1          6:1     1:1
//
// However, that's a pretty big performance hit for minimal gain. Going to
// the other extreme, we could standardize on the fastest FM rate, but then
// at least one prescale case (3) requires the FM to be smeared across two
// output samples:
//
//             ---- YM2203 -----    ---- YM2608 -----    ---- YM2610 -----
//              rate = clock/24      rate = clock/48      rate = clock/144
// Prescale    FM rate  SSG rate    FM rate  SSG rate    FM rate  SSG rate
//     6          3:1     2:3          3:1     2:3          1:1    2:9
//     3        1.5:1     1:3        1.5:1     1:3
//     2          1:1     1:6          1:1     1:6
//
// Stepping back one factor of 2 addresses that issue:
//
//             ---- YM2203 -----    ---- YM2608 -----    ---- YM2610 -----
//              rate = clock/12      rate = clock/24      rate = clock/144
// Prescale    FM rate  SSG rate    FM rate  SSG rate    FM rate  SSG rate
//     6          6:1     4:3          6:1     4:3          1:1    2:9
//     3          3:1     2:3          3:1     2:3
//     2          2:1     1:3          2:1     1:3
//
// This gives us three levels of output fidelity:
//    OPN_FIDELITY_MAX -- highest sample rate, using fastest SSG rate
//    OPN_FIDELITY_MIN -- lowest sample rate, using fastest FM rate
//    OPN_FIDELITY_MED -- medium sample rate such that FM is never smeared
//
// At the maximum clocks for YM2203/YM2608 (4Mhz/8MHz), these rates will
// end up as:
//    OPN_FIDELITY_MAX = 1000kHz
//    OPN_FIDELITY_MIN =  166kHz
//    OPN_FIEDLITY_MED =  333kHz


// ======================> opn_fidelity

enum opn_fidelity : uint8_t
{
	OPN_FIDELITY_MAX,
	OPN_FIDELITY_MIN,
	OPN_FIDELITY_MED,

	OPN_FIDELITY_DEFAULT = OPN_FIDELITY_MAX
};


// ======================> ssg_resampler

template<typename OutputType, int FirstOutput, bool MixTo1>
class ssg_resampler
{
private:
	// helper to add the last computed value to the sums, applying the given scale
	void add_last(int32_t &sum0, int32_t &sum1, int32_t &sum2, int32_t scale = 1);

	// helper to clock a new value and then add it to the sums, applying the given scale
	void clock_and_add(int32_t &sum0, int32_t &sum1, int32_t &sum2, int32_t scale = 1);

	// helper to write the sums to the appropriate outputs, applying the given
	// divisor to the final result
	void write_to_output(OutputType *output, int32_t sum0, int32_t sum1, int32_t sum2, int32_t divisor = 1);

public:
	// constructor
	ssg_resampler(ssg_engine &ssg);

	// save/restore
	void save_restore(ymfm_saved_state &state);

	// get the current sample index
	uint32_t sampindex() const { return m_sampindex; }

	// configure the ratio
	void configure(uint8_t outsamples, uint8_t srcsamples);

	// resample
	void resample(OutputType *output, uint32_t numsamples)
	{
		(this->*m_resampler)(output, numsamples);
	}

private:
	// resample SSG output to the target at a rate of 1 SSG sample
	// to every n output samples
	template<int Multiplier>
	void resample_n_1(OutputType *output, uint32_t numsamples);

	// resample SSG output to the target at a rate of n SSG samples
	// to every 1 output sample
	template<int Divisor>
	void resample_1_n(OutputType *output, uint32_t numsamples);

	// resample SSG output to the target at a rate of 9 SSG samples
	// to every 2 output samples
	void resample_2_9(OutputType *output, uint32_t numsamples);

	// resample SSG output to the target at a rate of 3 SSG samples
	// to every 1 output sample
	void resample_1_3(OutputType *output, uint32_t numsamples);

	// resample SSG output to the target at a rate of 3 SSG samples
	// to every 2 output samples
	void resample_2_3(OutputType *output, uint32_t numsamples);

	// resample SSG output to the target at a rate of 3 SSG samples
	// to every 4 output samples
	void resample_4_3(OutputType *output, uint32_t numsamples);

	// no-op resampler
	void resample_nop(OutputType *output, uint32_t numsamples);

	// define a pointer type
	using resample_func = void (ssg_resampler::*)(OutputType *output, uint32_t numsamples);

	// internal state
	ssg_engine &m_ssg;
	uint32_t m_sampindex;
	resample_func m_resampler;
	ssg_engine::output_data m_last;
};


// ======================> ym2203

class ym2203
{
public:
	using fm_engine = fm_engine_base<opn_registers>;
	static constexpr uint32_t FM_OUTPUTS = fm_engine::OUTPUTS;
	static constexpr uint32_t SSG_OUTPUTS = ssg_engine::OUTPUTS;
	static constexpr uint32_t OUTPUTS = FM_OUTPUTS + SSG_OUTPUTS;
	using output_data = ymfm_output<OUTPUTS>;

	// constructor
	ym2203(ymfm_interface &intf);

	// configuration
	void ssg_override(ssg_override &intf) { m_ssg.override(intf); }
	void set_fidelity(opn_fidelity fidelity) { m_fidelity = fidelity; update_prescale(m_fm.clock_prescale()); }

	// reset
	void reset();

	// save/restore
	void save_restore(ymfm_saved_state &state);

	// pass-through helpers
	uint32_t sample_rate(uint32_t input_clock) const
	{
		switch (m_fidelity)
		{
			case OPN_FIDELITY_MIN:	return input_clock / 24;
			case OPN_FIDELITY_MED:	return input_clock / 12;
			default:
			case OPN_FIDELITY_MAX:	return input_clock / 4;
		}
	}
	uint32_t ssg_effective_clock(uint32_t input_clock) const { uint32_t scale = m_fm.clock_prescale() * 2 / 3; return input_clock * 2 / scale; }
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
	void generate(output_data *output, uint32_t numsamples = 1);

protected:
	// internal helpers
	void update_prescale(uint8_t prescale);
	void clock_fm();

	// internal state
	opn_fidelity m_fidelity;            // configured fidelity
	uint8_t m_address;                  // address register
	uint8_t m_fm_samples_per_output;    // how many samples to repeat
	fm_engine::output_data m_last_fm;   // last FM output
	fm_engine m_fm;                     // core FM engine
	ssg_engine m_ssg;                   // SSG engine
	ssg_resampler<output_data, 1, false> m_ssg_resampler; // SSG resampler helper
};



//*********************************************************
//  OPNA IMPLEMENTATION CLASSES
//*********************************************************

// ======================> ym2608

class ym2608
{
	static constexpr uint8_t STATUS_ADPCM_B_EOS = 0x04;
	static constexpr uint8_t STATUS_ADPCM_B_BRDY = 0x08;
	static constexpr uint8_t STATUS_ADPCM_B_ZERO = 0x10;
	static constexpr uint8_t STATUS_ADPCM_B_PLAYING = 0x20;

public:
	using fm_engine = fm_engine_base<opna_registers>;
	static constexpr uint32_t FM_OUTPUTS = fm_engine::OUTPUTS;
	static constexpr uint32_t SSG_OUTPUTS = 1;
	static constexpr uint32_t OUTPUTS = FM_OUTPUTS + SSG_OUTPUTS;
	using output_data = ymfm_output<OUTPUTS>;

	// constructor
	ym2608(ymfm_interface &intf);

	// configuration
	void ssg_override(ssg_override &intf) { m_ssg.override(intf); }
	void set_fidelity(opn_fidelity fidelity) { m_fidelity = fidelity; update_prescale(m_fm.clock_prescale()); }

	// reset
	void reset();

	// save/restore
	void save_restore(ymfm_saved_state &state);

	// pass-through helpers
	uint32_t sample_rate(uint32_t input_clock) const
	{
		switch (m_fidelity)
		{
			case OPN_FIDELITY_MIN:	return input_clock / 48;
			case OPN_FIDELITY_MED:	return input_clock / 24;
			default:
			case OPN_FIDELITY_MAX:	return input_clock / 8;
		}
	}
	uint32_t ssg_effective_clock(uint32_t input_clock) const { uint32_t scale = m_fm.clock_prescale() * 2 / 3; return input_clock / scale; }
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
	void generate(output_data *output, uint32_t numsamples = 1);

protected:
	// internal helpers
	void update_prescale(uint8_t prescale);
	void clock_fm_and_adpcm();

	// internal state
	opn_fidelity m_fidelity;            // configured fidelity
	uint16_t m_address;                 // address register
	uint8_t m_fm_samples_per_output;    // how many samples to repeat
	uint8_t m_irq_enable;               // IRQ enable register
	uint8_t m_flag_control;             // flag control register
	fm_engine::output_data m_last_fm;   // last FM output
	fm_engine m_fm;                     // core FM engine
	ssg_engine m_ssg;                   // SSG engine
	ssg_resampler<output_data, 2, true> m_ssg_resampler; // SSG resampler helper
	adpcm_a_engine m_adpcm_a;           // ADPCM-A engine
	adpcm_b_engine m_adpcm_b;           // ADPCM-B engine
};


// ======================> ymf288

class ymf288
{
public:
	using fm_engine = fm_engine_base<opna_registers>;
	static constexpr uint32_t FM_OUTPUTS = fm_engine::OUTPUTS;
	static constexpr uint32_t SSG_OUTPUTS = 1;
	static constexpr uint32_t OUTPUTS = FM_OUTPUTS + SSG_OUTPUTS;
	using output_data = ymfm_output<OUTPUTS>;

	// constructor
	ymf288(ymfm_interface &intf);

	// configuration
	void ssg_override(ssg_override &intf) { m_ssg.override(intf); }
	void set_fidelity(opn_fidelity fidelity) { m_fidelity = fidelity; update_prescale(); }

	// reset
	void reset();

	// save/restore
	void save_restore(ymfm_saved_state &state);

	// pass-through helpers
	uint32_t sample_rate(uint32_t input_clock) const
	{
		switch (m_fidelity)
		{
			case OPN_FIDELITY_MIN:	return input_clock / 144;
			case OPN_FIDELITY_MED:	return input_clock / 144;
			default:
			case OPN_FIDELITY_MAX:	return input_clock / 16;
		}
	}
	uint32_t ssg_effective_clock(uint32_t input_clock) const { return input_clock / 4; }
	void invalidate_caches() { m_fm.invalidate_caches(); }

	// read access
	uint8_t read_status();
	uint8_t read_data();
	uint8_t read_status_hi();
	uint8_t read(uint32_t offset);

	// write access
	void write_address(uint8_t data);
	void write_data(uint8_t data);
	void write_address_hi(uint8_t data);
	void write_data_hi(uint8_t data);
	void write(uint32_t offset, uint8_t data);

	// generate one sample of sound
	void generate(output_data *output, uint32_t numsamples = 1);

protected:
	// internal helpers
	bool ymf288_mode() { return ((m_fm.regs().read(0x20) & 0x02) != 0); }
	void update_prescale();
	void clock_fm_and_adpcm();

	// internal state
	opn_fidelity m_fidelity;            // configured fidelity
	uint16_t m_address;                 // address register
	uint8_t m_fm_samples_per_output;    // how many samples to repeat
	uint8_t m_irq_enable;               // IRQ enable register
	uint8_t m_flag_control;             // flag control register
	fm_engine::output_data m_last_fm;   // last FM output
	fm_engine m_fm;                     // core FM engine
	ssg_engine m_ssg;                   // SSG engine
	ssg_resampler<output_data, 2, true> m_ssg_resampler; // SSG resampler helper
	adpcm_a_engine m_adpcm_a;           // ADPCM-A engine
};


// ======================> ym2610/ym2610b

class ym2610
{
	static constexpr uint8_t EOS_FLAGS_MASK = 0xbf;

public:
	using fm_engine = fm_engine_base<opna_registers>;
	static constexpr uint32_t FM_OUTPUTS = fm_engine::OUTPUTS;
	static constexpr uint32_t SSG_OUTPUTS = 1;
	static constexpr uint32_t OUTPUTS = FM_OUTPUTS + SSG_OUTPUTS;
	using output_data = ymfm_output<OUTPUTS>;

	// constructor
	ym2610(ymfm_interface &intf, uint8_t channel_mask = 0x36);

	// configuration
	void ssg_override(ssg_override &intf) { m_ssg.override(intf); }
	void set_fidelity(opn_fidelity fidelity) { m_fidelity = fidelity; update_prescale(); }

	// reset
	void reset();

	// save/restore
	void save_restore(ymfm_saved_state &state);

	// pass-through helpers
	uint32_t sample_rate(uint32_t input_clock) const
	{
		switch (m_fidelity)
		{
			case OPN_FIDELITY_MIN:	return input_clock / 144;
			case OPN_FIDELITY_MED:	return input_clock / 144;
			default:
			case OPN_FIDELITY_MAX:	return input_clock / 16;
		}
	}
	uint32_t ssg_effective_clock(uint32_t input_clock) const { return input_clock / 4; }
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
	void generate(output_data *output, uint32_t numsamples = 1);

protected:
	// internal helpers
	void update_prescale();
	void clock_fm_and_adpcm();

	// internal state
	opn_fidelity m_fidelity;            // configured fidelity
	uint16_t m_address;                 // address register
	uint8_t const m_fm_mask;            // FM channel mask
	uint8_t m_fm_samples_per_output;    // how many samples to repeat
	uint8_t m_eos_status;               // end-of-sample signals
	uint8_t m_flag_mask;                // flag mask control
	fm_engine::output_data m_last_fm;   // last FM output
	fm_engine m_fm;                     // core FM engine
	ssg_engine m_ssg;                   // core FM engine
	ssg_resampler<output_data, 2, true> m_ssg_resampler; // SSG resampler helper
	adpcm_a_engine m_adpcm_a;           // ADPCM-A engine
	adpcm_b_engine m_adpcm_b;           // ADPCM-B engine
};

class ym2610b : public ym2610
{
public:
	// constructor
	ym2610b(ymfm_interface &intf) : ym2610(intf, 0x3f) { }
};


// ======================> ym2612

class ym2612
{
public:
	using fm_engine = fm_engine_base<opna_registers>;
	static constexpr uint32_t OUTPUTS = fm_engine::OUTPUTS;
	using output_data = fm_engine::output_data;

	// constructor
	ym2612(ymfm_interface &intf);

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
	void write_address_hi(uint8_t data);
	void write_data_hi(uint8_t data);
	void write(uint32_t offset, uint8_t data);

	// generate one sample of sound
	void generate(output_data *output, uint32_t numsamples = 1);

protected:
	// simulate the DAC discontinuity
	constexpr int32_t dac_discontinuity(int32_t value) const { return (value < 0) ? (value - 3) : (value + 4); }

	// internal state
	uint16_t m_address;              // address register
	uint16_t m_dac_data;             // 9-bit DAC data
	uint8_t m_dac_enable;            // DAC enabled?
	fm_engine m_fm;                  // core FM engine
};


// ======================> ym3438

class ym3438 : public ym2612
{
public:
	ym3438(ymfm_interface &intf) : ym2612(intf) { }

	// generate one sample of sound
	void generate(output_data *output, uint32_t numsamples = 1);
};


// ======================> ymf276

class ymf276 : public ym2612
{
public:
	ymf276(ymfm_interface &intf) : ym2612(intf) { }

	// generate one sample of sound
	void generate(output_data *output, uint32_t numsamples);
};

}


#endif // YMFM_OPN_H
