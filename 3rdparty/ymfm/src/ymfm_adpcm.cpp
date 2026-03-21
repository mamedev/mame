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

#include "ymfm_adpcm.h"

namespace ymfm
{

//*********************************************************
// ADPCM "A" REGISTERS
//*********************************************************

//-------------------------------------------------
//  reset - reset the register state
//-------------------------------------------------

void adpcm_a_registers::reset()
{
	std::fill_n(&m_regdata[0], REGISTERS, 0);

	// initialize the pans to on by default, and max instrument volume;
	// some neogeo homebrews (for example ffeast) rely on this
	m_regdata[0x08] = m_regdata[0x09] = m_regdata[0x0a] =
	m_regdata[0x0b] = m_regdata[0x0c] = m_regdata[0x0d] = 0xdf;
}


//-------------------------------------------------
//  save_restore - save or restore the data
//-------------------------------------------------

void adpcm_a_registers::save_restore(ymfm_saved_state &state)
{
	state.save_restore(m_regdata);
}


//*********************************************************
// ADPCM "A" CHANNEL
//*********************************************************

//-------------------------------------------------
//  adpcm_a_channel - constructor
//-------------------------------------------------

adpcm_a_channel::adpcm_a_channel(adpcm_a_engine &owner, uint32_t choffs, uint32_t addrshift) :
	m_choffs(choffs),
	m_address_shift(addrshift),
	m_playing(0),
	m_curnibble(0),
	m_curbyte(0),
	m_curaddress(0),
	m_accumulator(0),
	m_step_index(0),
	m_regs(owner.regs()),
	m_owner(owner)
{
}


//-------------------------------------------------
//  reset - reset the channel state
//-------------------------------------------------

void adpcm_a_channel::reset()
{
	m_playing = 0;
	m_curnibble = 0;
	m_curbyte = 0;
	m_curaddress = 0;
	m_accumulator = 0;
	m_step_index = 0;
}


//-------------------------------------------------
//  save_restore - save or restore the data
//-------------------------------------------------

void adpcm_a_channel::save_restore(ymfm_saved_state &state)
{
	state.save_restore(m_playing);
	state.save_restore(m_curnibble);
	state.save_restore(m_curbyte);
	state.save_restore(m_curaddress);
	state.save_restore(m_accumulator);
	state.save_restore(m_step_index);
}


//-------------------------------------------------
//  keyonoff - signal key on/off
//-------------------------------------------------

void adpcm_a_channel::keyonoff(bool on)
{
	// QUESTION: repeated key ons restart the sample?
	m_playing = on;
	if (m_playing)
	{
		m_curaddress = m_regs.ch_start(m_choffs) << m_address_shift;
		m_curnibble = 0;
		m_curbyte = 0;
		m_accumulator = 0;
		m_step_index = 0;

		// don't log masked channels
		if (((debug::GLOBAL_ADPCM_A_CHANNEL_MASK >> m_choffs) & 1) != 0)
			debug::log_keyon("KeyOn ADPCM-A%d: pan=%d%d start=%04X end=%04X level=%02X\n",
				m_choffs,
				m_regs.ch_pan_left(m_choffs),
				m_regs.ch_pan_right(m_choffs),
				m_regs.ch_start(m_choffs),
				m_regs.ch_end(m_choffs),
				m_regs.ch_instrument_level(m_choffs));
	}
}


//-------------------------------------------------
//  clock - master clocking function
//-------------------------------------------------

bool adpcm_a_channel::clock()
{
	// if not playing, just output 0
	if (m_playing == 0)
	{
		m_accumulator = 0;
		return false;
	}

	// if we're about to read nibble 0, fetch the data
	uint8_t data;
	if (m_curnibble == 0)
	{
		// stop when we hit the end address; apparently only low 20 bits are used for
		// comparison on the YM2610: this affects sample playback in some games, for
		// example twinspri character select screen music will skip some samples if
		// this is not correct
		//
		// note also: end address is inclusive, so wait until we are about to fetch
		// the sample just after the end before stopping; this is needed for nitd's
		// jump sound, for example
		uint32_t end = (m_regs.ch_end(m_choffs) + 1) << m_address_shift;
		if (((m_curaddress ^ end) & 0xfffff) == 0)
		{
			m_playing = m_accumulator = 0;
			return true;
		}

		m_curbyte = m_owner.intf().ymfm_external_read(ACCESS_ADPCM_A, m_curaddress++);
		data = m_curbyte >> 4;
		m_curnibble = 1;
	}

	// otherwise just extract from the previosuly-fetched byte
	else
	{
		data = m_curbyte & 0xf;
		m_curnibble = 0;
	}

	// compute the ADPCM delta
	static uint16_t const s_steps[49] =
	{
		 16,  17,   19,   21,   23,   25,   28,
		 31,  34,   37,   41,   45,   50,   55,
		 60,  66,   73,   80,   88,   97,  107,
		118, 130,  143,  157,  173,  190,  209,
		230, 253,  279,  307,  337,  371,  408,
		449, 494,  544,  598,  658,  724,  796,
		876, 963, 1060, 1166, 1282, 1411, 1552
	};
	int32_t delta = (2 * bitfield(data, 0, 3) + 1) * s_steps[m_step_index] / 8;
	if (bitfield(data, 3))
		delta = -delta;

	// the 12-bit accumulator wraps on the ym2610 and ym2608 (like the msm5205)
	m_accumulator = (m_accumulator + delta) & 0xfff;

	// adjust ADPCM step
	static int8_t const s_step_inc[8] = { -1, -1, -1, -1, 2, 5, 7, 9 };
	m_step_index = clamp(m_step_index + s_step_inc[bitfield(data, 0, 3)], 0, 48);

	return false;
}


//-------------------------------------------------
//  output - return the computed output value, with
//  panning applied
//-------------------------------------------------

template<int NumOutputs>
void adpcm_a_channel::output(ymfm_output<NumOutputs> &output) const
{
	// volume combines instrument and total levels
	int vol = (m_regs.ch_instrument_level(m_choffs) ^ 0x1f) + (m_regs.total_level() ^ 0x3f);

	// if combined is maximum, don't add to outputs
	if (vol >= 63)
		return;

	// convert into a shift and a multiplier
	// QUESTION: verify this from other sources
	int8_t mul = 15 - (vol & 7);
	uint8_t shift = 4 + 1 + (vol >> 3);

	// m_accumulator is a 12-bit value; shift up to sign-extend;
	// the downshift is incorporated into 'shift'
	int16_t value = ((int16_t(m_accumulator << 4) * mul) >> shift) & ~3;

	// apply to left/right as appropriate
	if (NumOutputs == 1 || m_regs.ch_pan_left(m_choffs))
		output.data[0] += value;
	if (NumOutputs > 1 && m_regs.ch_pan_right(m_choffs))
		output.data[1] += value;
}



//*********************************************************
// ADPCM "A" ENGINE
//*********************************************************

//-------------------------------------------------
//  adpcm_a_engine - constructor
//-------------------------------------------------

adpcm_a_engine::adpcm_a_engine(ymfm_interface &intf, uint32_t addrshift) :
	m_intf(intf)
{
	// create the channels
	for (int chnum = 0; chnum < CHANNELS; chnum++)
		m_channel[chnum] = std::make_unique<adpcm_a_channel>(*this, chnum, addrshift);
}


//-------------------------------------------------
//  reset - reset the engine state
//-------------------------------------------------

void adpcm_a_engine::reset()
{
	// reset register state
	m_regs.reset();

	// reset each channel
	for (auto &chan : m_channel)
		chan->reset();
}


//-------------------------------------------------
//  save_restore - save or restore the data
//-------------------------------------------------

void adpcm_a_engine::save_restore(ymfm_saved_state &state)
{
	// save register state
	m_regs.save_restore(state);

	// save channel state
	for (int chnum = 0; chnum < CHANNELS; chnum++)
		m_channel[chnum]->save_restore(state);
}


//-------------------------------------------------
//  clock - master clocking function
//-------------------------------------------------

uint32_t adpcm_a_engine::clock(uint32_t chanmask)
{
	// clock each channel, setting a bit in result if it finished
	uint32_t result = 0;
	for (int chnum = 0; chnum < CHANNELS; chnum++)
		if (bitfield(chanmask, chnum))
			if (m_channel[chnum]->clock())
				result |= 1 << chnum;

	// return the bitmask of completed samples
	return result;
}


//-------------------------------------------------
//  update - master update function
//-------------------------------------------------

template<int NumOutputs>
void adpcm_a_engine::output(ymfm_output<NumOutputs> &output, uint32_t chanmask)
{
	// mask out some channels for debug purposes
	chanmask &= debug::GLOBAL_ADPCM_A_CHANNEL_MASK;

	// compute the output of each channel
	for (int chnum = 0; chnum < CHANNELS; chnum++)
		if (bitfield(chanmask, chnum))
			m_channel[chnum]->output(output);
}

template void adpcm_a_engine::output<1>(ymfm_output<1> &output, uint32_t chanmask);
template void adpcm_a_engine::output<2>(ymfm_output<2> &output, uint32_t chanmask);


//-------------------------------------------------
//  write - handle writes to the ADPCM-A registers
//-------------------------------------------------

void adpcm_a_engine::write(uint32_t regnum, uint8_t data)
{
	// store the raw value to the register array;
	// most writes are passive, consumed only when needed
	m_regs.write(regnum, data);

	// actively handle writes to the control register
	if (regnum == 0x00)
		for (int chnum = 0; chnum < CHANNELS; chnum++)
			if (bitfield(data, chnum))
				m_channel[chnum]->keyonoff(bitfield(~data, 7));
}



//*********************************************************
// ADPCM "B" REGISTERS
//*********************************************************

//-------------------------------------------------
//  reset - reset the register state
//-------------------------------------------------

void adpcm_b_registers::reset()
{
	std::fill_n(&m_regdata[0], REGISTERS, 0);

	// default limit to wide open
	m_regdata[0x0c] = m_regdata[0x0d] = 0xff;
}


//-------------------------------------------------
//  save_restore - save or restore the data
//-------------------------------------------------

void adpcm_b_registers::save_restore(ymfm_saved_state &state)
{
	state.save_restore(m_regdata);
}



//*********************************************************
// ADPCM "B" CHANNEL
//*********************************************************

//-------------------------------------------------
//  adpcm_b_channel - constructor
//-------------------------------------------------

adpcm_b_channel::adpcm_b_channel(adpcm_b_engine &owner, uint32_t addrshift) :
	m_address_shift(addrshift),
	m_status(STATUS_BRDY),
	m_buffer(0),
	m_nibbles(0),
	m_position(0),
	m_curaddress(0),
	m_accumulator(0),
	m_output(0),
	m_prev_output(0),
	m_adpcm_step(STEP_MIN),
	m_regs(owner.regs()),
	m_owner(owner)
{
}


//-------------------------------------------------
//  reset - reset the channel state
//-------------------------------------------------

void adpcm_b_channel::reset()
{
	m_status = STATUS_BRDY;
	m_buffer = 0;
	m_nibbles = 0;
	m_position = 0;
	m_curaddress = 0;
	m_accumulator = 0;
	m_output = 0;
	m_prev_output = 0;
	m_adpcm_step = STEP_MIN;
}


//-------------------------------------------------
//  save_restore - save or restore the data
//-------------------------------------------------

void adpcm_b_channel::save_restore(ymfm_saved_state &state)
{
	state.save_restore(m_status);
	state.save_restore(m_buffer);
	state.save_restore(m_nibbles);
	state.save_restore(m_position);
	state.save_restore(m_curaddress);
	state.save_restore(m_accumulator);
	state.save_restore(m_output);
	state.save_restore(m_prev_output);
	state.save_restore(m_adpcm_step);
}


//-------------------------------------------------
//  clock - master clocking function
//-------------------------------------------------

void adpcm_b_channel::clock()
{
	// only process if active and not recording (which we don't support)
	if (!m_regs.execute() || m_regs.record() || (m_status & STATUS_INTERNAL_PLAYING) == 0)
	{
		m_prev_output = m_output;
		m_position = 0;
		set_reset_status(0, STATUS_INTERNAL_PLAYING);
		return;
	}

	// otherwise, advance the step
	uint32_t position = m_position + m_regs.delta_n();
	m_position = uint16_t(position);
	if (position < 0x10000)
		return;

	// if we have nibbles available, process them
	if (m_nibbles != 0)
	{
		// fetch the next nibble
		uint8_t data = consume_nibbles(1);

		// forecast to next forecast: 1/8, 3/8, 5/8, 7/8, 9/8, 11/8, 13/8, 15/8
		int32_t delta = (2 * bitfield(data, 0, 3) + 1) * m_adpcm_step / 8;
		if (bitfield(data, 3))
			delta = -delta;

		// add and clamp to 16 bits
		m_accumulator = clamp(m_accumulator + delta, -32768, 32767);

		// scale the ADPCM step: 0.9, 0.9, 0.9, 0.9, 1.2, 1.6, 2.0, 2.4
		static uint8_t const s_step_scale[8] = { 57, 57, 57, 57, 77, 102, 128, 153 };
		m_adpcm_step = clamp((m_adpcm_step * s_step_scale[bitfield(data, 0, 3)]) / 64, STEP_MIN, STEP_MAX);

		// make the new output equal to the accumulator
		m_prev_output = m_output;
		m_output = m_accumulator;

		// if we've drained all the nibbles, that means we're at a repeat point or end of sample
		if (m_nibbles == 0)
		{
			// reset the ADPCM state (but leave output alone)
			m_accumulator = 0;
			m_adpcm_step = STEP_MIN;

			// always set EOS bit, even if repeating
			set_reset_status(STATUS_EOS);
			debug::log_keyon("%s\n", "ADPCM EOS");

			// clear playing flag if we're not repeating
			if (!m_regs.repeat())
				set_reset_status(0, STATUS_INTERNAL_PLAYING);
		}
	}

	// if we don't have at least 3 nibbles in the buffer, request more data
	if ((m_status & STATUS_INTERNAL_PLAYING) != 0 && m_nibbles < 3)
	{
		// if we hit the end address after this fetch, handle looping/ending
		if (request_data())
		{
			// the final 3 samples are not played; chop them from the stream
			consume_nibbles(3);

			// this should always end up as 1; the logic above assumes we will hit
			// 0 nibbles after processing the next one
			assert(m_nibbles == 1);

			// if repeating, set the current address back to start for next fetch
			if (m_regs.repeat())
				latch_addresses();
		}
	}
}


//-------------------------------------------------
//  output - return the computed output value, with
//  panning applied
//-------------------------------------------------

template<int NumOutputs>
void adpcm_b_channel::output(ymfm_output<NumOutputs> &output, uint32_t rshift) const
{
	// mask out some channels for debug purposes
	if ((debug::GLOBAL_ADPCM_B_CHANNEL_MASK & 1) == 0)
		return;

	// do a linear interpolation between samples
	int32_t result = (m_prev_output * int32_t((m_position ^ 0xffff) + 1) + m_output * int32_t(m_position)) >> 16;

	// apply volume (level) in a linear fashion and reduce
	result = (result * int32_t(m_regs.level())) >> (8 + rshift);

	// apply to left/right
	if (NumOutputs == 1 || m_regs.pan_left())
		output.data[0] += result;
	if (NumOutputs > 1 && m_regs.pan_right())
		output.data[1] += result;
}


//-------------------------------------------------
//  read - handle special register reads
//-------------------------------------------------

uint8_t adpcm_b_channel::read(uint32_t regnum)
{
	uint8_t result = 0;

	// register 8 reads over the bus under some conditions
	if (regnum == 0x08 && !m_regs.execute() && !m_regs.record() && m_regs.external())
		result = read_ram();

	return result;
}


//-------------------------------------------------
//  write - handle special register writes
//-------------------------------------------------

void adpcm_b_channel::write(uint32_t regnum, uint8_t value)
{
	// register 0 can do a reset; also use writes here to reset the
	// dummy read counter
	if (regnum == 0x00)
	{
		// reset flag stops playback and holds output, but does not clear the
		// externally-visible playing flag
		if (m_regs.resetflag())
			set_reset_status(STATUS_BRDY | (((m_status & STATUS_INTERNAL_PLAYING) != 0) ? STATUS_EOS : 0), STATUS_INTERNAL_PLAYING);

		// all other modes set up for an operation
		else
		{
			// initialize the core state; appears to leave EOS flag alone until next execute
			set_reset_status(STATUS_BRDY, STATUS_PLAYING | STATUS_INTERNAL_DRAIN | STATUS_INTERNAL_PLAYING | STATUS_INTERNAL_SUPPRESS_WRITE);

			// flag the address to be latched at the next access; this is necessary
			// because it is allowed to program the start/stop addresses after this
			// command byte is written
			m_curaddress = LATCH_ADDRESS;

			// if playing, set the playing status
			if (m_regs.execute())
			{
				m_buffer = 0;
				m_nibbles = 0;
				m_position = 0;
				m_accumulator = 0;
				m_adpcm_step = STEP_MIN;
				m_output = 0;

				set_reset_status(STATUS_PLAYING | STATUS_INTERNAL_PLAYING, STATUS_EOS);

				// don't log masked channels
				if ((debug::GLOBAL_ADPCM_B_CHANNEL_MASK & 1) != 0)
					debug::log_keyon("KeyOn ADPCM-B: rep=%d spk=%d pan=%d%d dac=%d 8b=%d rom=%d ext=%d rec=%d start=%04X end=%04X pre=%04X dn=%04X lvl=%02X lim=%04X\n",
						m_regs.repeat(),
						m_regs.speaker(),
						m_regs.pan_left(),
						m_regs.pan_right(),
						m_regs.dac_enable(),
						m_regs.dram_8bit(),
						m_regs.rom_ram(),
						m_regs.external(),
						m_regs.record(),
						m_regs.start(),
						m_regs.end(),
						m_regs.prescale(),
						m_regs.delta_n(),
						m_regs.level(),
						m_regs.limit());
			}
		}
	}

	// register 8 writes over the bus under some conditions
	else if (regnum == 0x08)
	{
		// writing during execute
		if (m_regs.execute())
		{
			// if writing from the CPU during execute, clear the ready flag; data will be picked
			// up on next fetch
			if (!m_regs.record() && !m_regs.external())
				set_reset_status(0, STATUS_BRDY);
		}

		// if writing to external data, process record mode, which writes data to RAM
		else if (m_regs.external() && m_regs.record())
			write_ram(value);

		// writes in external non-record mode appear to behave like a read in that it will advance
		// the address and consume a nibble, but the last written value will still be present
		else
			read_ram();
	}
}


//-------------------------------------------------
//  address_shift - compute the current address
//  shift amount based on register settings
//-------------------------------------------------

uint32_t adpcm_b_channel::address_shift() const
{
	// if a constant address shift, just provide that
	if (m_address_shift != 0)
		return m_address_shift;

	// if ROM or 8-bit DRAM, shift is 5 bits
	if (m_regs.rom_ram())
		return 5;
	if (m_regs.dram_8bit())
		return 5;

	// otherwise, shift is 2 bits
	return 2;
}


//-------------------------------------------------
//  advance_address - advance the address, checking
//  for end/limit values at programmed boundaries;
//  returns true if the end is hit
//-------------------------------------------------

bool adpcm_b_channel::advance_address()
{
	// if we're fetching the last byte of a unit, check ending conditions
	auto shift = address_shift();
	auto mask = (1 << shift) - 1;

	// should never get here with an uninitialized current address
	assert(m_curaddress != LATCH_ADDRESS);

	// if at the end of a unit, check for end/limit
	if ((m_curaddress & mask) == mask)
	{
		// shift off the low address bits and check against the end
		uint32_t unitaddr = m_curaddress >> shift;
		if (unitaddr == m_regs.end())
			return true;

		// wrap at the limit address; this does not report any status
		else if (unitaddr == m_regs.limit())
		{
			m_curaddress = 0;
			return false;
		}
	}

	// advance the address
	m_curaddress = (m_curaddress + 1) & 0xffffff;
	return false;
}


//-------------------------------------------------
//  request_data - request another byte of data
//  for the buffer; used by both playback and
//  data reading code
//-------------------------------------------------

bool adpcm_b_channel::request_data()
{
	// pick up the current address if this is the first read
	if (m_curaddress == LATCH_ADDRESS)
		latch_addresses();

	// if CPU-driven, just set the flag and return true
	if (!m_regs.external())
	{
		// if data was written, consume it
		if ((m_status & STATUS_BRDY) == 0)
			append_buffer_byte(m_regs.cpudata());
		set_reset_status(STATUS_BRDY);
		return false;
	}

	// append the new byte to our buffer; also write to the cpudata register to match
	// behavior of dummy reads from real chip
	uint8_t data = m_owner.intf().ymfm_external_read(ACCESS_ADPCM_B, m_curaddress);
	append_buffer_byte(data);
	m_regs.write(0x08, data);

	// advance the address, returning true if we hit the end
	return advance_address();
}


//-------------------------------------------------
//  read_ram - perform a read cycle from RAM/ROM
//-------------------------------------------------

uint8_t adpcm_b_channel::read_ram()
{
	// if this is the first read, ensure there is at least 2 bytes of data available,
	// padding with the cpudata register if needed
	if (m_curaddress == LATCH_ADDRESS)
	{
		set_reset_status(0, STATUS_INTERNAL_DRAIN);
		while (m_nibbles < 4)
			append_buffer_byte(m_regs.cpudata());
	}

	// if we have the nibbles, return them
	uint8_t result = consume_nibbles(2);
	set_reset_status(STATUS_BRDY);

	// if we previously hit the end and we're draining, see if we're out
	if ((m_status & STATUS_INTERNAL_DRAIN) != 0)
	{
		// if we run out of nibbles, mark end of sample and reset the address
		if (m_nibbles == 0)
		{
			set_reset_status(STATUS_EOS, STATUS_INTERNAL_DRAIN);

			// if repeating, add one dummy sample and issue a fetch of the first byte
			if (m_regs.repeat())
			{
				append_buffer_byte(m_regs.cpudata());
				m_curaddress = m_regs.start() << address_shift();
				request_data();
			}

			// otherwise, reset the address
			else
				m_curaddress = LATCH_ADDRESS;
		}
	}

	// if not draining, then request more data and start draining if we hit the end
	else if (request_data())
		set_reset_status(STATUS_INTERNAL_DRAIN);

	return result;
}


//-------------------------------------------------
//  write_ram - perform a write cycle to RAM
//-------------------------------------------------

void adpcm_b_channel::write_ram(uint8_t value)
{
	// normal write case, unsuppressed
	if ((m_status & STATUS_INTERNAL_SUPPRESS_WRITE) == 0)
	{
		// latch the current address if this is the first write
		if (m_curaddress == LATCH_ADDRESS)
			latch_addresses();

		// write the data
		m_owner.intf().ymfm_external_write(ACCESS_ADPCM_B, m_curaddress, value);
		set_reset_status(STATUS_BRDY);

		// advance; if we hit the end, signal EOS and put ourselves back in the latching state
		if (advance_address())
		{
			set_reset_status(STATUS_EOS);
			m_curaddress = LATCH_ADDRESS;

			// in the repeat case, suppress further writes
			if (m_regs.repeat())
				set_reset_status(STATUS_INTERNAL_SUPPRESS_WRITE);
		}
	}

	// suppressed writes after reaching stop address in repeat mode; note that this runs
	// immediately after the EOS condition above, as well as on subsequent writes
	if ((m_status & STATUS_INTERNAL_SUPPRESS_WRITE) != 0)
	{
		// reset the buffer to 4 nibbles with 0 and the value written, then trigger a read
		// cycle which will consume the 0 and clock in the next byte, leaving the value
		// written as the next byte to consume
		m_buffer = value << 16;
		m_nibbles = 4;
		read_ram();
	}
}



//*********************************************************
// ADPCM "B" ENGINE
//*********************************************************

//-------------------------------------------------
//  adpcm_b_engine - constructor
//-------------------------------------------------

adpcm_b_engine::adpcm_b_engine(ymfm_interface &intf, uint32_t addrshift) :
	m_intf(intf)
{
	// create the channel (only one supported for now, but leaving possibilities open)
	m_channel = std::make_unique<adpcm_b_channel>(*this, addrshift);
}


//-------------------------------------------------
//  reset - reset the engine state
//-------------------------------------------------

void adpcm_b_engine::reset()
{
	// reset registers
	m_regs.reset();

	// reset each channel
	m_channel->reset();
}


//-------------------------------------------------
//  save_restore - save or restore the data
//-------------------------------------------------

void adpcm_b_engine::save_restore(ymfm_saved_state &state)
{
	// save our state
	m_regs.save_restore(state);

	// save channel state
	m_channel->save_restore(state);
}


//-------------------------------------------------
//  clock - master clocking function
//-------------------------------------------------

void adpcm_b_engine::clock()
{
	// clock each channel, setting a bit in result if it finished
	m_channel->clock();
}


//-------------------------------------------------
//  output - master output function
//-------------------------------------------------

template<int NumOutputs>
void adpcm_b_engine::output(ymfm_output<NumOutputs> &output, uint32_t rshift)
{
	// compute the output of each channel
	m_channel->output(output, rshift);
}

template void adpcm_b_engine::output<1>(ymfm_output<1> &output, uint32_t rshift);
template void adpcm_b_engine::output<2>(ymfm_output<2> &output, uint32_t rshift);


//-------------------------------------------------
//  write - handle writes to the ADPCM-B registers
//-------------------------------------------------

void adpcm_b_engine::write(uint32_t regnum, uint8_t data)
{
	// store the raw value to the register array;
	// most writes are passive, consumed only when needed
	m_regs.write(regnum, data);

	// let the channel handle any special writes
	m_channel->write(regnum, data);
}

}
