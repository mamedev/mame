// license:BSD-3-Clause
// copyright-holders:Aaron Giles

#ifndef MAME_SOUND_YMADPCM_H
#define MAME_SOUND_YMADPCM_H

#pragma once

#include "dirom.h"

// forward declarations
class ymadpcm_a_engine;
class ymadpcm_b_engine;


// ======================> ymadpcm_a_registers

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
class ymadpcm_a_registers
{
public:
	// constants
	static constexpr u32 OUTPUTS = 2;
	static constexpr u32 CHANNELS = 6;
	static constexpr u32 REGISTERS = 0x30;
	static constexpr u32 ALL_CHANNELS = (1 << CHANNELS) - 1;

	// constructor
	ymadpcm_a_registers() { }

	// register for save states
	void save(device_t &device);

	// reset to initial state
	void reset();

	// map channel number to register offset
	static constexpr u32 channel_offset(u32 chnum)
	{
		assert(chnum < CHANNELS);
		return chnum;
	}

	// direct read/write access
	void write(u32 index, u8 data) { m_regdata[index] = data; }

	// system-wide registers
	u32 dump() const                          { return BIT(m_regdata[0x00], 7); }
	u32 dump_mask() const                     { return BIT(m_regdata[0x00], 0, 6); }
	u32 total_level() const                   { return BIT(m_regdata[0x01], 0, 6); }
	u32 test() const                          { return m_regdata[0x02]; }

	// per-channel registers
	u32 ch_pan_left(u32 choffs) const         { return BIT(m_regdata[choffs + 0x08], 7); }
	u32 ch_pan_right(u32 choffs) const        { return BIT(m_regdata[choffs + 0x08], 6); }
	u32 ch_instrument_level(u32 choffs) const { return BIT(m_regdata[choffs + 0x08], 0, 5); }
	u32 ch_start(u32 choffs) const            { return m_regdata[choffs + 0x10] | (m_regdata[choffs + 0x18] << 8); }
	u32 ch_end(u32 choffs) const              { return m_regdata[choffs + 0x20] | (m_regdata[choffs + 0x28] << 8); }

	// per-channel writes
	void write_start(u32 choffs, u32 address)
	{
		write(choffs + 0x10, address);
		write(choffs + 0x18, address >> 8);
	}
	void write_end(u32 choffs, u32 address)
	{
		write(choffs + 0x20, address);
		write(choffs + 0x28, address >> 8);
	}

private:
	// internal state
	u8 m_regdata[REGISTERS];         // register data
};


// ======================> ymadpcm_a_channel

class ymadpcm_a_channel
{
public:
	// constructor
	ymadpcm_a_channel(ymadpcm_a_engine &owner, u32 choffs, read8sm_delegate reader, u32 addrshift);

	// register for save states
	void save(device_t &device, u32 index);

	// reset the channel state
	void reset();

	// signal key on/off
	void keyonoff(bool on);

	// master clockingfunction
	bool clock();

	// return the computed output value, with panning applied
	void output(s32 outputs[ymadpcm_a_registers::OUTPUTS]) const;

private:
	// internal state
	u32 const m_choffs;              // channel offset
	u32 const m_address_shift;       // address bits shift-left
	u32 m_playing;                   // currently playing?
	u32 m_curnibble;                 // index of the current nibble
	u32 m_curbyte;                   // current byte of data
	u32 m_curaddress;                // current address
	s32 m_accumulator;               // accumulator
	s32 m_step_index;                // index in the stepping table
	read8sm_delegate const m_reader; // read delegate
	ymadpcm_a_registers &m_regs;     // reference to registers
};


// ======================> ymadpcm_a_engine

class ymadpcm_a_engine
{
public:
	static constexpr int OUTPUTS = ymadpcm_a_registers::OUTPUTS;
	static constexpr int CHANNELS = ymadpcm_a_registers::CHANNELS;

	// constructor
	ymadpcm_a_engine(device_t &device, read8sm_delegate reader, u32 addrshift);

	// save state handling
	void save(device_t &device);

	// reset our status
	void reset();

	// master clocking function
	u32 clock(u32 chanmask);

	// compute sum of channel outputs
	void output(s32 outputs[ymadpcm_a_registers::OUTPUTS], u32 chanmask);

	// write to the ADPCM-A registers
	void write(u32 regnum, u8 data);

	// set the start/end address for a channel (for hardcoded YM2608 percussion)
	void set_start_end(u8 chnum, u16 start, u16 end)
	{
		u32 choffs = ymadpcm_a_registers::channel_offset(chnum);
		m_regs.write_start(choffs, start);
		m_regs.write_end(choffs, end);
	}

	// return a reference to our registers
	ymadpcm_a_registers &regs() { return m_regs; }

private:
	// internal state
	std::unique_ptr<ymadpcm_a_channel> m_channel[CHANNELS]; // array of channels
	ymadpcm_a_registers m_regs;      // register accessor
};


// ======================> ymadpcm_b_registers

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
class ymadpcm_b_registers
{
public:
	// constants
	static constexpr u32 OUTPUTS = 2;
	static constexpr u32 CHANNELS = 1;
	static constexpr u32 REGISTERS = 0x11;
	static constexpr u32 ALL_CHANNELS = (1 << CHANNELS) - 1;

	// constructor
	ymadpcm_b_registers() { }

	// register for save states
	void save(device_t &device);

	// reset to initial state
	void reset();

	// direct read/write access
	void write(u32 index, u8 data) { m_regdata[index] = data; }

	// system-wide registers
	u32 execute() const          { return BIT(m_regdata[0x00], 7); }
	u32 record() const           { return BIT(m_regdata[0x00], 6); }
	u32 external() const         { return BIT(m_regdata[0x00], 5); }
	u32 repeat() const           { return BIT(m_regdata[0x00], 4); }
	u32 speaker() const          { return BIT(m_regdata[0x00], 3); }
	u32 resetflag() const        { return BIT(m_regdata[0x00], 0); }
	u32 pan_left() const         { return BIT(m_regdata[0x01], 7); }
	u32 pan_right() const        { return BIT(m_regdata[0x01], 6); }
	u32 start_conversion() const { return BIT(m_regdata[0x01], 3); }
	u32 dac_enable() const       { return BIT(m_regdata[0x01], 2); }
	u32 dram_8bit() const        { return BIT(m_regdata[0x01], 1); }
	u32 rom_ram() const          { return BIT(m_regdata[0x01], 0); }
	u32 start() const            { return m_regdata[0x02] | (m_regdata[0x03] << 8); }
	u32 end() const              { return m_regdata[0x04] | (m_regdata[0x05] << 8); }
	u32 prescale() const         { return m_regdata[0x06] | (BIT(m_regdata[0x07], 0, 3) << 8); }
	u32 cpudata() const          { return m_regdata[0x08]; }
	u32 delta_n() const          { return m_regdata[0x09] | (m_regdata[0x0a] << 8); }
	u32 level() const            { return m_regdata[0x0b]; }
	u32 limit() const            { return m_regdata[0x0c] | (m_regdata[0x0d] << 8); }
	u32 dac() const              { return m_regdata[0x0e]; }
	u32 pcm() const              { return m_regdata[0x0f]; }

private:
	// internal state
	u8 m_regdata[REGISTERS];         // register data
};


// ======================> ymadpcm_b_channel

class ymadpcm_b_channel
{
	static constexpr s32 STEP_MIN = 127;
	static constexpr s32 STEP_MAX = 24576;

public:
	static constexpr u8 STATUS_EOS = 0x01;
	static constexpr u8 STATUS_BRDY = 0x02;
	static constexpr u8 STATUS_PLAYING = 0x04;

	// constructor
	ymadpcm_b_channel(ymadpcm_b_engine &owner, read8sm_delegate reader, write8sm_delegate writer, u32 addrshift);

	// register for save states
	void save(device_t &device, u32 index);

	// reset the channel state
	void reset();

	// signal key on/off
	void keyonoff(bool on);

	// master clocking function
	void clock();

	// return the computed output value, with panning applied
	void output(s32 outputs[ymadpcm_b_registers::OUTPUTS], u32 rshift) const;

	// return the status register
	u8 status() const { return m_status; }

	// handle special register reads
	u8 read(u32 regnum);

	// handle special register writes
	void write(u32 regnum, u8 value);

private:
	// helper - return the current address shift
	u32 address_shift() const;

	// load the start address
	void load_start();

	// limit checker
	bool at_limit() const { return (m_curaddress >> address_shift()) >= m_regs.limit(); }

	// end checker
	bool at_end() const { return (m_curaddress >> address_shift()) > m_regs.end(); }

	// internal state
	u32 const m_address_shift;       // address bits shift-left
	u32 m_status;                    // currently playing?
	u32 m_curnibble;                 // index of the current nibble
	u32 m_curbyte;                   // current byte of data
	u32 m_dummy_read;                // dummy read tracker
	u32 m_position;                  // current fractional position
	u32 m_curaddress;                // current address
	s32 m_accumulator;               // accumulator
	s32 m_prev_accum;                // previous accumulator (for linear interp)
	s32 m_adpcm_step;                // next forecast
	read8sm_delegate const m_reader; // read delegate
	write8sm_delegate const m_writer;// write delegate
	ymadpcm_b_registers &m_regs;     // reference to registers
};


// ======================> ymadpcm_b_engine

class ymadpcm_b_engine
{
public:
	static constexpr int OUTPUTS = ymadpcm_b_registers::OUTPUTS;
	static constexpr int CHANNELS = ymadpcm_b_registers::CHANNELS;

	// constructor
	ymadpcm_b_engine(device_t &device, read8sm_delegate reader, write8sm_delegate writer, u32 addrshift = 0);

	// save state handling
	void save(device_t &device);

	// reset our status
	void reset();

	// master clocking function
	void clock(u32 chanmask);

	// compute sum of channel outputs
	void output(s32 outputs[2], u32 rshift, u32 chanmask);

	// read from the ADPCM-B registers
	u32 read(u32 regnum) { return m_channel[0]->read(regnum); }

	// write to the ADPCM-B registers
	void write(u32 regnum, u8 data);

	// status
	u8 status() const { return m_channel[0]->status(); }

	// return a reference to our registers
	ymadpcm_b_registers &regs() { return m_regs; }

private:
	// internal state
	std::unique_ptr<ymadpcm_b_channel> m_channel[CHANNELS]; // array of channels
	ymadpcm_b_registers m_regs;      // register accessor
};

#endif // MAME_SOUND_YMADPCM_H
