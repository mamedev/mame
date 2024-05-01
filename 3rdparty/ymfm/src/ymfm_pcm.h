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

#ifndef YMFM_PCM_H
#define YMFM_PCM_H

#pragma once

#include "ymfm.h"

namespace ymfm
{

/*
Note to self: Sega "Multi-PCM" is almost identical to this

28 channels

Writes:
00 = data reg, causes write
01 = target slot = data - (data / 8)
02 = address (clamped to 7)

Slot data (registers with ADSR/KSR seem to be inaccessible):
0: xxxx---- panpot
1: xxxxxxxx wavetable low
2: xxxxxx-- pitch low
   -------x wavetable high
3: xxxx---- octave
   ----xxxx pitch hi
4: x------- key on
5: xxxxxxx- total level
   -------x level direct (0=interpolate)
6: --xxx--- LFO frequency
   -----xxx PM sensitivity
7: -----xxx AM sensitivity

Sample data:
+00: start hi
+01: start mid
+02: start low
+03: loop hi
+04: loop low
+05: -end hi
+06: -end low
+07: vibrato (reg 6)
+08: attack/decay
+09: sustain level/rate
+0A: ksr/release
+0B: LFO amplitude (reg 7)

*/

//*********************************************************
//  INTERFACE CLASSES
//*********************************************************

class pcm_engine;


// ======================> pcm_cache

// this class holds data that is computed once at the start of clocking
// and remains static during subsequent sound generation
struct pcm_cache
{
	uint32_t step;                    // sample position step, as a .16 value
	uint32_t total_level;             // target total level, as a .10 value
	uint32_t pan_left;                // left panning attenuation
	uint32_t pan_right;               // right panning attenuation
	uint32_t eg_sustain;              // sustain level, shifted up to envelope values
	uint8_t eg_rate[EG_STATES];       // envelope rate, including KSR
	uint8_t lfo_step;                 // stepping value for LFO
	uint8_t am_depth;                 // scale value for AM LFO
	uint8_t pm_depth;                 // scale value for PM LFO
};


// ======================> pcm_registers

//
// PCM register map:
//
//      System-wide registers:
//        00-01 xxxxxxxx LSI Test
//           02 -------x Memory access mode (0=sound gen, 1=read/write)
//              ------x- Memory type (0=ROM, 1=ROM+SRAM)
//              ---xxx-- Wave table header
//              xxx----- Device ID (=1 for YMF278B)
//           03 --xxxxxx Memory address high
//           04 xxxxxxxx Memory address mid
//           05 xxxxxxxx Memory address low
//           06 xxxxxxxx Memory data
//           F8 --xxx--- Mix control (FM_R)
//              -----xxx Mix control (FM_L)
//           F9 --xxx--- Mix control (PCM_R)
//              -----xxx Mix control (PCM_L)
//
//      Channel-specific registers:
//        08-1F xxxxxxxx Wave table number low
//        20-37 -------x Wave table number high
//              xxxxxxx- F-number low
//        38-4F -----xxx F-number high
//              ----x--- Pseudo-reverb
//              xxxx---- Octave
//        50-67 xxxxxxx- Total level
//              -------x Level direct
//        68-7F x------- Key on
//              -x------ Damp
//              --x----- LFO reset
//              ---x---- Output channel
//              ----xxxx Panpot
//        80-97 --xxx--- LFO speed
//              -----xxx Vibrato
//        98-AF xxxx---- Attack rate
//              ----xxxx Decay rate
//        B0-C7 xxxx---- Sustain level
//              ----xxxx Sustain rate
//        C8-DF xxxx---- Rate correction
//              ----xxxx Release rate
//        E0-F7 -----xxx AM depth

class pcm_registers
{
public:
	// constants
	static constexpr uint32_t OUTPUTS = 4;
	static constexpr uint32_t CHANNELS = 24;
	static constexpr uint32_t REGISTERS = 0x100;
	static constexpr uint32_t ALL_CHANNELS = (1 << CHANNELS) - 1;

	// constructor
	pcm_registers() { }

	// save/restore
	void save_restore(ymfm_saved_state &state);

	// reset to initial state
	void reset();

	// update cache information
	void cache_channel_data(uint32_t choffs, pcm_cache &cache);

	// direct read/write access
	uint8_t read(uint32_t index ) { return m_regdata[index]; }
	void write(uint32_t index, uint8_t data) { m_regdata[index] = data; }

	// system-wide registers
	uint32_t memory_access_mode() const                 { return bitfield(m_regdata[0x02], 0); }
	uint32_t memory_type() const                        { return bitfield(m_regdata[0x02], 1); }
	uint32_t wave_table_header() const                  { return bitfield(m_regdata[0x02], 2, 3); }
	uint32_t device_id() const                          { return bitfield(m_regdata[0x02], 5, 3); }
	uint32_t memory_address() const                     { return (bitfield(m_regdata[0x03], 0, 6) << 16) | (m_regdata[0x04] << 8) | m_regdata[0x05]; }
	uint32_t memory_data() const                        { return m_regdata[0x06]; }
	uint32_t mix_fm_r() const                           { return bitfield(m_regdata[0xf8], 3, 3); }
	uint32_t mix_fm_l() const                           { return bitfield(m_regdata[0xf8], 0, 3); }
	uint32_t mix_pcm_r() const                          { return bitfield(m_regdata[0xf9], 3, 3); }
	uint32_t mix_pcm_l() const                          { return bitfield(m_regdata[0xf9], 0, 3); }

	// per-channel registers
	uint32_t ch_wave_table_num(uint32_t choffs) const   { return m_regdata[choffs + 0x08] | (bitfield(m_regdata[choffs + 0x20], 0) << 8); }
	uint32_t ch_fnumber(uint32_t choffs) const          { return bitfield(m_regdata[choffs + 0x20], 1, 7) | (bitfield(m_regdata[choffs + 0x38], 0, 3) << 7); }
	uint32_t ch_pseudo_reverb(uint32_t choffs) const    { return bitfield(m_regdata[choffs + 0x38], 3); }
	uint32_t ch_octave(uint32_t choffs) const           { return bitfield(m_regdata[choffs + 0x38], 4, 4); }
	uint32_t ch_total_level(uint32_t choffs) const      { return bitfield(m_regdata[choffs + 0x50], 1, 7); }
	uint32_t ch_level_direct(uint32_t choffs) const     { return bitfield(m_regdata[choffs + 0x50], 0); }
	uint32_t ch_keyon(uint32_t choffs) const            { return bitfield(m_regdata[choffs + 0x68], 7); }
	uint32_t ch_damp(uint32_t choffs) const             { return bitfield(m_regdata[choffs + 0x68], 6); }
	uint32_t ch_lfo_reset(uint32_t choffs) const        { return bitfield(m_regdata[choffs + 0x68], 5); }
	uint32_t ch_output_channel(uint32_t choffs) const   { return bitfield(m_regdata[choffs + 0x68], 4); }
	uint32_t ch_panpot(uint32_t choffs) const           { return bitfield(m_regdata[choffs + 0x68], 0, 4); }
	uint32_t ch_lfo_speed(uint32_t choffs) const        { return bitfield(m_regdata[choffs + 0x80], 3, 3); }
	uint32_t ch_vibrato(uint32_t choffs) const          { return bitfield(m_regdata[choffs + 0x80], 0, 3); }
	uint32_t ch_attack_rate(uint32_t choffs) const      { return bitfield(m_regdata[choffs + 0x98], 4, 4); }
	uint32_t ch_decay_rate(uint32_t choffs) const       { return bitfield(m_regdata[choffs + 0x98], 0, 4); }
	uint32_t ch_sustain_level(uint32_t choffs) const    { return bitfield(m_regdata[choffs + 0xb0], 4, 4); }
	uint32_t ch_sustain_rate(uint32_t choffs) const     { return bitfield(m_regdata[choffs + 0xb0], 0, 4); }
	uint32_t ch_rate_correction(uint32_t choffs) const  { return bitfield(m_regdata[choffs + 0xc8], 4, 4); }
	uint32_t ch_release_rate(uint32_t choffs) const     { return bitfield(m_regdata[choffs + 0xc8], 0, 4); }
	uint32_t ch_am_depth(uint32_t choffs) const         { return bitfield(m_regdata[choffs + 0xe0], 0, 3); }

	// return the memory address and increment it
	uint32_t memory_address_autoinc()
	{
		uint32_t result = memory_address();
		uint32_t newval = result + 1;
		m_regdata[0x05] = newval >> 0;
		m_regdata[0x04] = newval >> 8;
		m_regdata[0x03] = (newval >> 16) & 0x3f;
		return result;
	}

private:
	// internal helpers
	uint32_t effective_rate(uint32_t raw, uint32_t correction);

	// internal state
	uint8_t m_regdata[REGISTERS];         // register data
};


// ======================> pcm_channel

class pcm_channel
{
	static constexpr uint8_t KEY_ON = 0x01;
	static constexpr uint8_t KEY_PENDING_ON = 0x02;
	static constexpr uint8_t KEY_PENDING = 0x04;

	// "quiet" value, used to optimize when we can skip doing working
	static constexpr uint32_t EG_QUIET = 0x200;

public:
	using output_data = ymfm_output<pcm_registers::OUTPUTS>;

	// constructor
	pcm_channel(pcm_engine &owner, uint32_t choffs);

	// save/restore
	void save_restore(ymfm_saved_state &state);

	// reset the channel state
	void reset();

	// return the channel offset
	uint32_t choffs() const { return m_choffs; }

	// prepare prior to clocking
	bool prepare();

	// master clocking function
	void clock(uint32_t env_counter);

	// return the computed output value, with panning applied
	void output(output_data &output) const;

	// signal key on/off
	void keyonoff(bool on);

	// load a new wavetable entry
	void load_wavetable();

private:
	// internal helpers
	void start_attack();
	void start_release();
	void clock_envelope(uint32_t env_counter);
	int16_t fetch_sample() const;
	uint8_t read_pcm(uint32_t address) const;

	// internal state
	uint32_t const m_choffs;              // channel offset
	uint32_t m_baseaddr;                  // base address
	uint32_t m_endpos;                    // ending position
	uint32_t m_looppos;                   // loop position
	uint32_t m_curpos;                    // current position
	uint32_t m_nextpos;                   // next position
	uint32_t m_lfo_counter;               // LFO counter
	envelope_state m_eg_state;            // envelope state
	uint16_t m_env_attenuation;           // computed envelope attenuation
	uint32_t m_total_level;               // total level with as 7.10 for interp
	uint8_t m_format;                     // sample format
	uint8_t m_key_state;                  // current key state
	pcm_cache m_cache;                    // cached data
	pcm_registers &m_regs;                // reference to registers
	pcm_engine &m_owner;                  // reference to our owner
};


// ======================> pcm_engine

class pcm_engine
{
public:
	static constexpr int OUTPUTS = pcm_registers::OUTPUTS;
	static constexpr int CHANNELS = pcm_registers::CHANNELS;
	static constexpr uint32_t ALL_CHANNELS = pcm_registers::ALL_CHANNELS;
	using output_data = pcm_channel::output_data;

	// constructor
	pcm_engine(ymfm_interface &intf);

	// reset our status
	void reset();

	// save/restore
	void save_restore(ymfm_saved_state &state);

	// master clocking function
	void clock(uint32_t chanmask);

	// compute sum of channel outputs
	void output(output_data &output, uint32_t chanmask);

	// read from the PCM registers
	uint8_t read(uint32_t regnum);

	// write to the PCM registers
	void write(uint32_t regnum, uint8_t data);

	// return a reference to our interface
	ymfm_interface &intf() { return m_intf; }

	// return a reference to our registers
	pcm_registers &regs() { return m_regs; }

private:
	// internal state
	ymfm_interface &m_intf;                           // reference to the interface
	uint32_t m_env_counter;                           // envelope counter
	uint32_t m_modified_channels;                     // bitmask of modified channels
	uint32_t m_active_channels;                       // bitmask of active channels
	uint32_t m_prepare_count;                         // counter to do periodic prepare sweeps
	std::unique_ptr<pcm_channel> m_channel[CHANNELS]; // array of channels
	pcm_registers m_regs;                             // registers
};

}

#endif // YMFM_PCM_H
