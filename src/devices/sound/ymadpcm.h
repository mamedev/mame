// license:BSD-3-Clause
// copyright-holders:Aaron Giles

#ifndef MAME_SOUND_YMADPCM_H
#define MAME_SOUND_YMADPCM_H

#pragma once

#include "dirom.h"


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
	// private constructor to directly specify channel base
	ymadpcm_a_registers(ymadpcm_a_registers const &src, u8 chbase) :
		m_chbase(chbase),
		m_regdata(src.m_regdata)
	{
	}

public:
	// constructor
	ymadpcm_a_registers(std::vector<u8> &regdata) :
		m_chbase(0),
		m_regdata(regdata)
	{
	}

	u8 chbase() const { return m_chbase; }

	// direct read/write access
	u8 read(u8 index) { return m_regdata[index]; }
	void write(u8 index, u8 data) { m_regdata[index] = data; }

	// create a new version of ourself with a different channel/operator base
	ymadpcm_a_registers channel_registers(u8 chnum) { return ymadpcm_a_registers(*this, chnum); }

	// system-wide registers
	u8 dump() const             /*  1 bit  */ { return BIT(m_regdata[0x00], 7); }
	u8 dump_mask() const        /*  6 bits */ { return BIT(m_regdata[0x00], 0, 6); }
	u8 total_level() const      /*  6 bits */ { return BIT(m_regdata[0x01], 0, 6); }
	u8 test() const             /*  8 bits */ { return m_regdata[0x02]; }

	// per-channel registers
	u8 pan_left() const         /*  1 bit  */ { return BIT(m_regdata[m_chbase + 0x08], 7); }
	u8 pan_right() const        /*  1 bit  */ { return BIT(m_regdata[m_chbase + 0x08], 6); }
	u8 instrument_level() const /*  5 bits */ { return BIT(m_regdata[m_chbase + 0x08], 0, 5); }
	u16 start() const           /* 16 bits */ { return m_regdata[m_chbase + 0x10] | (m_regdata[m_chbase + 0x18] << 8); }
	u16 end() const             /* 16 bits */ { return m_regdata[m_chbase + 0x20] | (m_regdata[m_chbase + 0x28] << 8); }

	// per-channel writes
	void write_start(u16 address) { write(m_chbase + 0x10, address); write(m_chbase + 0x18, address >> 8); }
	void write_end(u16 address) { write(m_chbase + 0x20, address); write(m_chbase + 0x28, address >> 8); }

private:
	// internal state
	u8 m_chbase;                   // base offset for channel-specific data
	std::vector<u8> &m_regdata;    // reference to the raw data
};


// ======================> ymadpcm_a_channel

class ymadpcm_a_channel
{
public:
	// constructor
	ymadpcm_a_channel(ymadpcm_a_registers regs, read8sm_delegate reader, u8 addrshift);

	// register for save states
	void save(device_t &device, u8 index);

	// reset the channel state
	void reset();

	// signal key on/off
	void keyonoff(bool on);

	// master clockingfunction
	bool clock();

	// return the computed output value, with panning applied
	void output(s32 &lsum, s32 &rsum) const;

	// direct parameter setting for YM2608 ROM-based samples
	void set_start_end(u16 start, u16 end) { m_regs.write_start(start); m_regs.write_end(end); }

private:
	// internal state
	u8 const m_address_shift;        // address bits shift-left
	read8sm_delegate const m_reader; // read delegate
	u8 m_playing;                    // currently playing?
	u8 m_curnibble;                  // index of the current nibble
	u8 m_curbyte;                    // current byte of data
	u32 m_curaddress;                // current address
	s16 m_accumulator;               // accumulator
	s8 m_step_index;                 // index in the stepping table
	ymadpcm_a_registers m_regs;      // register accessor
};


// ======================> ymadpcm_a_engine

class ymadpcm_a_engine
{
	static constexpr int CHANNELS = 6;

public:
	// constructor
	ymadpcm_a_engine(device_t &device, read8sm_delegate reader, u8 addrshift);

	// save state handling
	void save(device_t &device);

	// reset our status
	void reset();

	// master clocking function
	u8 clock(u8 chanmask);

	// compute sum of channel outputs
	void output(s32 &lsum, s32 &rsum, u8 chanmask);

	// write to the ADPCM-A registers
	void write(u8 regnum, u8 data);

	// set the start/end address for a channel (for hardcoded YM2608 percussion)
	void set_start_end(u8 chnum, u16 start, u16 end) { m_channel[chnum]->set_start_end(start, end); }

private:
	// internal state
	std::unique_ptr<ymadpcm_a_channel> m_channel[CHANNELS]; // array of channels
	std::vector<u8> m_regdata;       // raw register data
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
//           0e xxxxxxxx DAC data
//           0f xxxxxxxx PCM data
//
class ymadpcm_b_registers
{
public:
	// constructor
	ymadpcm_b_registers(std::vector<u8> &regdata) :
		m_regdata(regdata)
	{
	}

	// direct read/write access
	u8 read(u8 index) { return m_regdata[index]; }
	void write(u8 index, u8 data) { m_regdata[index] = data; }

	// system-wide registers
	u8 execute() const          /*  1 bit  */ { return BIT(m_regdata[0x00], 7); }
	u8 record() const           /*  1 bit  */ { return BIT(m_regdata[0x00], 6); }
	u8 external() const         /*  1 bit  */ { return BIT(m_regdata[0x00], 5); }
	u8 repeat() const           /*  1 bit  */ { return BIT(m_regdata[0x00], 4); }
	u8 speaker() const          /*  1 bit  */ { return BIT(m_regdata[0x00], 3); }
	u8 reset() const            /*  1 bit  */ { return BIT(m_regdata[0x00], 0); }
	u8 pan_left() const         /*  1 bit  */ { return BIT(m_regdata[0x01], 7); }
	u8 pan_right() const        /*  1 bit  */ { return BIT(m_regdata[0x01], 6); }
	u8 start_conversion() const /*  1 bit  */ { return BIT(m_regdata[0x01], 3); }
	u8 dac_enable() const       /*  1 bit  */ { return BIT(m_regdata[0x01], 2); }
	u8 dram_8bit() const        /*  1 bit  */ { return BIT(m_regdata[0x01], 1); }
	u8 rom_ram() const          /*  1 bit  */ { return BIT(m_regdata[0x01], 0); }
	u16 start() const           /* 16 bits */ { return m_regdata[0x02] | (m_regdata[0x03] << 8); }
	u16 end() const             /* 16 bits */ { return m_regdata[0x04] | (m_regdata[0x05] << 8); }
	u16 prescale() const        /* 11 bits */ { return m_regdata[0x06] | (BIT(m_regdata[0x07], 0, 3) << 8); }
	u8 cpudata() const          /*  8 bits */ { return m_regdata[0x08]; }
	u16 delta_n() const         /* 16 bits */ { return m_regdata[0x09] | (m_regdata[0x0a] << 8); }
	u8 level() const            /*  8 bits */ { return m_regdata[0x0b]; }
	u16 limit() const           /* 16 bits */ { return m_regdata[0x0c] | (m_regdata[0x0d] << 8); }
	u8 dac() const              /*  8 bits */ { return m_regdata[0x0e]; }
	u8 pcm() const              /*  8 bits */ { return m_regdata[0x0f]; }

private:
	// internal state
	std::vector<u8> &m_regdata;    // reference to the raw data
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
	ymadpcm_b_channel(ymadpcm_b_registers regs, read8sm_delegate reader, write8sm_delegate writer, u8 addrshift);

	// register for save states
	void save(device_t &device, u8 index);

	// reset the channel state
	void reset();

	// signal key on/off
	void keyonoff(bool on);

	// master clocking function
	void clock();

	// return the computed output value, with panning applied
	void output(s32 &lsum, s32 &rsum, u8 rshift) const;

	// return the status register
	u8 status() const { return m_status; }

	// handle special register reads
	u8 read(u8 regnum);

	// handle special register writes
	void write(u8 regnum, u8 value);

private:
	// helper - return the current address shift
	u8 address_shift() const;

	// load the start address
	void load_start();

	// limit checker
	bool at_limit() const { return (m_curaddress >> address_shift()) >= m_regs.limit(); }

	// end checker
	bool at_end() const { return (m_curaddress >> address_shift()) > m_regs.end(); }

	// internal state
	read8sm_delegate const m_reader; // read delegate
	write8sm_delegate const m_writer;// write delegate
	u8 const m_address_shift;        // address bits shift-left
	u8 m_status;                     // currently playing?
	u8 m_curnibble;                  // index of the current nibble
	u8 m_curbyte;                    // current byte of data
	u8 m_dummy_read;                 // dummy read tracker
	u16 m_position;                  // current fractional position
	u32 m_curaddress;                // current address
	s32 m_accumulator;               // accumulator
	s32 m_prev_accum;                // previous accumulator (for linear interp)
	s32 m_adpcm_step;                // next forecast
	ymadpcm_b_registers m_regs;      // register accessor
};


// ======================> ymadpcm_b_engine

class ymadpcm_b_engine
{
	static constexpr int CHANNELS = 1;

public:
	// constructor
	ymadpcm_b_engine(device_t &device, read8sm_delegate reader, write8sm_delegate writer, u8 addrshift = 0);

	// save state handling
	void save(device_t &device);

	// reset our status
	void reset();

	// master clocking function
	void clock(u8 chanmask);

	// compute sum of channel outputs
	void output(s32 &lsum, s32 &rsum, u8 rshift, u8 chanmask);

	// read from the ADPCM-B registers
	u8 read(u8 regnum) { return m_channel[0]->read(regnum); }

	// write to the ADPCM-B registers
	void write(u8 regnum, u8 data);

	// status
	u8 status(u8 chnum = 0) const { return m_channel[chnum]->status(); }

private:
	// internal state
	std::unique_ptr<ymadpcm_b_channel> m_channel[CHANNELS]; // array of channels
	std::vector<u8> m_regdata;       // raw register data
	ymadpcm_b_registers m_regs;      // register accessor
};

#endif // MAME_SOUND_YMADPCM_H
