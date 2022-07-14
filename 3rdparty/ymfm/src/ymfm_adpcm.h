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

#ifndef YMFM_ADPCM_H
#define YMFM_ADPCM_H

#pragma once

#include "ymfm.h"

namespace ymfm
{

//*********************************************************
//  INTERFACE CLASSES
//*********************************************************

// forward declarations
class adpcm_a_engine;
class adpcm_b_engine;


// ======================> adpcm_a_registers

//
// ADPCM-A register map:
//
//      System-wide registers:
//           00 x------- Dump (disable=1) or keyon (0) control
//              --xxxxxx Mask of channels to dump or keyon
//           01 --xxxxxx Total level
//           02 xxxxxxxx Test register
//        08-0D x------- Pan left
//              -x------ Pan right
//              ---xxxxx Instrument level
//        10-15 xxxxxxxx Start address (low)
//        18-1D xxxxxxxx Start address (high)
//        20-25 xxxxxxxx End address (low)
//        28-2D xxxxxxxx End address (high)
//
class adpcm_a_registers
{
public:
	// constants
	static constexpr uint32_t OUTPUTS = 2;
	static constexpr uint32_t CHANNELS = 6;
	static constexpr uint32_t REGISTERS = 0x30;
	static constexpr uint32_t ALL_CHANNELS = (1 << CHANNELS) - 1;

	// constructor
	adpcm_a_registers() { }

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

	// direct read/write access
	void write(uint32_t index, uint8_t data) { m_regdata[index] = data; }

	// system-wide registers
	uint32_t dump() const                               { return bitfield(m_regdata[0x00], 7); }
	uint32_t dump_mask() const                          { return bitfield(m_regdata[0x00], 0, 6); }
	uint32_t total_level() const                        { return bitfield(m_regdata[0x01], 0, 6); }
	uint32_t test() const                               { return m_regdata[0x02]; }

	// per-channel registers
	uint32_t ch_pan_left(uint32_t choffs) const         { return bitfield(m_regdata[choffs + 0x08], 7); }
	uint32_t ch_pan_right(uint32_t choffs) const        { return bitfield(m_regdata[choffs + 0x08], 6); }
	uint32_t ch_instrument_level(uint32_t choffs) const { return bitfield(m_regdata[choffs + 0x08], 0, 5); }
	uint32_t ch_start(uint32_t choffs) const            { return m_regdata[choffs + 0x10] | (m_regdata[choffs + 0x18] << 8); }
	uint32_t ch_end(uint32_t choffs) const              { return m_regdata[choffs + 0x20] | (m_regdata[choffs + 0x28] << 8); }

	// per-channel writes
	void write_start(uint32_t choffs, uint32_t address)
	{
		write(choffs + 0x10, address);
		write(choffs + 0x18, address >> 8);
	}
	void write_end(uint32_t choffs, uint32_t address)
	{
		write(choffs + 0x20, address);
		write(choffs + 0x28, address >> 8);
	}

private:
	// internal state
	uint8_t m_regdata[REGISTERS];         // register data
};


// ======================> adpcm_a_channel

class adpcm_a_channel
{
public:
	// constructor
	adpcm_a_channel(adpcm_a_engine &owner, uint32_t choffs, uint32_t addrshift);

	// reset the channel state
	void reset();

	// save/restore
	void save_restore(ymfm_saved_state &state);

	// signal key on/off
	void keyonoff(bool on);

	// master clockingfunction
	bool clock();

	// return the computed output value, with panning applied
	template<int NumOutputs>
	void output(ymfm_output<NumOutputs> &output) const;

private:
	// internal state
	uint32_t const m_choffs;              // channel offset
	uint32_t const m_address_shift;       // address bits shift-left
	uint32_t m_playing;                   // currently playing?
	uint32_t m_curnibble;                 // index of the current nibble
	uint32_t m_curbyte;                   // current byte of data
	uint32_t m_curaddress;                // current address
	int32_t m_accumulator;                // accumulator
	int32_t m_step_index;                 // index in the stepping table
	adpcm_a_registers &m_regs;            // reference to registers
	adpcm_a_engine &m_owner;              // reference to our owner
};


// ======================> adpcm_a_engine

class adpcm_a_engine
{
public:
	static constexpr int CHANNELS = adpcm_a_registers::CHANNELS;

	// constructor
	adpcm_a_engine(ymfm_interface &intf, uint32_t addrshift);

	// reset our status
	void reset();

	// save/restore
	void save_restore(ymfm_saved_state &state);

	// master clocking function
	uint32_t clock(uint32_t chanmask);

	// compute sum of channel outputs
	template<int NumOutputs>
	void output(ymfm_output<NumOutputs> &output, uint32_t chanmask);

	// write to the ADPCM-A registers
	void write(uint32_t regnum, uint8_t data);

	// set the start/end address for a channel (for hardcoded YM2608 percussion)
	void set_start_end(uint8_t chnum, uint16_t start, uint16_t end)
	{
		uint32_t choffs = adpcm_a_registers::channel_offset(chnum);
		m_regs.write_start(choffs, start);
		m_regs.write_end(choffs, end);
	}

	// return a reference to our interface
	ymfm_interface &intf() { return m_intf; }

	// return a reference to our registers
	adpcm_a_registers &regs() { return m_regs; }

private:
	// internal state
	ymfm_interface &m_intf;                                 // reference to the interface
	std::unique_ptr<adpcm_a_channel> m_channel[CHANNELS]; // array of channels
	adpcm_a_registers m_regs;                             // registers
};


// ======================> adpcm_b_registers

//
// ADPCM-B register map:
//
//      System-wide registers:
//           00 x------- Start of synthesis/analysis
//              -x------ Record
//              --x----- External/manual driving
//              ---x---- Repeat playback
//              ----x--- Speaker off
//              -------x Reset
//           01 x------- Pan left
//              -x------ Pan right
//              ----x--- Start conversion
//              -----x-- DAC enable
//              ------x- DRAM access (1=8-bit granularity; 0=1-bit)
//              -------x RAM/ROM (1=ROM, 0=RAM)
//           02 xxxxxxxx Start address (low)
//           03 xxxxxxxx Start address (high)
//           04 xxxxxxxx End address (low)
//           05 xxxxxxxx End address (high)
//           06 xxxxxxxx Prescale value (low)
//           07 -----xxx Prescale value (high)
//           08 xxxxxxxx CPU data/buffer
//           09 xxxxxxxx Delta-N frequency scale (low)
//           0a xxxxxxxx Delta-N frequency scale (high)
//           0b xxxxxxxx Level control
//           0c xxxxxxxx Limit address (low)
//           0d xxxxxxxx Limit address (high)
//           0e xxxxxxxx DAC data [YM2608/10]
//           0f xxxxxxxx PCM data [YM2608/10]
//           0e xxxxxxxx DAC data high [Y8950]
//           0f xx------ DAC data low [Y8950]
//           10 -----xxx DAC data exponent [Y8950]
//
class adpcm_b_registers
{
public:
	// constants
	static constexpr uint32_t REGISTERS = 0x11;

	// constructor
	adpcm_b_registers() { }

	// reset to initial state
	void reset();

	// save/restore
	void save_restore(ymfm_saved_state &state);

	// direct read/write access
	void write(uint32_t index, uint8_t data) { m_regdata[index] = data; }

	// system-wide registers
	uint32_t execute() const          { return bitfield(m_regdata[0x00], 7); }
	uint32_t record() const           { return bitfield(m_regdata[0x00], 6); }
	uint32_t external() const         { return bitfield(m_regdata[0x00], 5); }
	uint32_t repeat() const           { return bitfield(m_regdata[0x00], 4); }
	uint32_t speaker() const          { return bitfield(m_regdata[0x00], 3); }
	uint32_t resetflag() const        { return bitfield(m_regdata[0x00], 0); }
	uint32_t pan_left() const         { return bitfield(m_regdata[0x01], 7); }
	uint32_t pan_right() const        { return bitfield(m_regdata[0x01], 6); }
	uint32_t start_conversion() const { return bitfield(m_regdata[0x01], 3); }
	uint32_t dac_enable() const       { return bitfield(m_regdata[0x01], 2); }
	uint32_t dram_8bit() const        { return bitfield(m_regdata[0x01], 1); }
	uint32_t rom_ram() const          { return bitfield(m_regdata[0x01], 0); }
	uint32_t start() const            { return m_regdata[0x02] | (m_regdata[0x03] << 8); }
	uint32_t end() const              { return m_regdata[0x04] | (m_regdata[0x05] << 8); }
	uint32_t prescale() const         { return m_regdata[0x06] | (bitfield(m_regdata[0x07], 0, 3) << 8); }
	uint32_t cpudata() const          { return m_regdata[0x08]; }
	uint32_t delta_n() const          { return m_regdata[0x09] | (m_regdata[0x0a] << 8); }
	uint32_t level() const            { return m_regdata[0x0b]; }
	uint32_t limit() const            { return m_regdata[0x0c] | (m_regdata[0x0d] << 8); }
	uint32_t dac() const              { return m_regdata[0x0e]; }
	uint32_t pcm() const              { return m_regdata[0x0f]; }

private:
	// internal state
	uint8_t m_regdata[REGISTERS];         // register data
};


// ======================> adpcm_b_channel

class adpcm_b_channel
{
	static constexpr int32_t STEP_MIN = 127;
	static constexpr int32_t STEP_MAX = 24576;

public:
	static constexpr uint8_t STATUS_EOS = 0x01;
	static constexpr uint8_t STATUS_BRDY = 0x02;
	static constexpr uint8_t STATUS_PLAYING = 0x04;

	// constructor
	adpcm_b_channel(adpcm_b_engine &owner, uint32_t addrshift);

	// reset the channel state
	void reset();

	// save/restore
	void save_restore(ymfm_saved_state &state);

	// signal key on/off
	void keyonoff(bool on);

	// master clocking function
	void clock();

	// return the computed output value, with panning applied
	template<int NumOutputs>
	void output(ymfm_output<NumOutputs> &output, uint32_t rshift) const;

	// return the status register
	uint8_t status() const { return m_status; }

	// handle special register reads
	uint8_t read(uint32_t regnum);

	// handle special register writes
	void write(uint32_t regnum, uint8_t value);

private:
	// helper - return the current address shift
	uint32_t address_shift() const;

	// load the start address
	void load_start();

	// limit checker; stops at the last byte of the chunk described by address_shift()
	bool at_limit() const { return (m_curaddress == (((m_regs.limit() + 1) << address_shift()) - 1)); }

	// end checker; stops at the last byte of the chunk described by address_shift()
	bool at_end() const { return (m_curaddress == (((m_regs.end() + 1) << address_shift()) - 1)); }

	// internal state
	uint32_t const m_address_shift; // address bits shift-left
	uint32_t m_status;              // currently playing?
	uint32_t m_curnibble;           // index of the current nibble
	uint32_t m_curbyte;             // current byte of data
	uint32_t m_dummy_read;          // dummy read tracker
	uint32_t m_position;            // current fractional position
	uint32_t m_curaddress;          // current address
	int32_t m_accumulator;          // accumulator
	int32_t m_prev_accum;           // previous accumulator (for linear interp)
	int32_t m_adpcm_step;           // next forecast
	adpcm_b_registers &m_regs;      // reference to registers
	adpcm_b_engine &m_owner;        // reference to our owner
};


// ======================> adpcm_b_engine

class adpcm_b_engine
{
public:
	// constructor
	adpcm_b_engine(ymfm_interface &intf, uint32_t addrshift = 0);

	// reset our status
	void reset();

	// save/restore
	void save_restore(ymfm_saved_state &state);

	// master clocking function
	void clock();

	// compute sum of channel outputs
	template<int NumOutputs>
	void output(ymfm_output<NumOutputs> &output, uint32_t rshift);

	// read from the ADPCM-B registers
	uint32_t read(uint32_t regnum) { return m_channel->read(regnum); }

	// write to the ADPCM-B registers
	void write(uint32_t regnum, uint8_t data);

	// status
	uint8_t status() const { return m_channel->status(); }

	// return a reference to our interface
	ymfm_interface &intf() { return m_intf; }

	// return a reference to our registers
	adpcm_b_registers &regs() { return m_regs; }

private:
	// internal state
	ymfm_interface &m_intf;                     // reference to our interface
	std::unique_ptr<adpcm_b_channel> m_channel; // channel pointer
	adpcm_b_registers m_regs;                   // registers
};

}

#endif // YMFM_ADPCM_H
