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
	m_curnibble(0),
	m_curbyte(0),
	m_dummy_read(0),
	m_position(0),
	m_curaddress(0),
	m_accumulator(0),
	m_prev_accum(0),
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
	m_curnibble = 0;
	m_curbyte = 0;
	m_dummy_read = 0;
	m_position = 0;
	m_curaddress = 0;
	m_accumulator = 0;
	m_prev_accum = 0;
	m_adpcm_step = STEP_MIN;
}


//-------------------------------------------------
//  save_restore - save or restore the data
//-------------------------------------------------

void adpcm_b_channel::save_restore(ymfm_saved_state &state)
{
	state.save_restore(m_status);
	state.save_restore(m_curnibble);
	state.save_restore(m_curbyte);
	state.save_restore(m_dummy_read);
	state.save_restore(m_position);
	state.save_restore(m_curaddress);
	state.save_restore(m_accumulator);
	state.save_restore(m_prev_accum);
	state.save_restore(m_adpcm_step);
}


//-------------------------------------------------
//  clock - master clocking function
//-------------------------------------------------

void adpcm_b_channel::clock()
{
	// only process if active and not recording (which we don't support)
	if (!m_regs.execute() || m_regs.record() || (m_status & STATUS_PLAYING) == 0)
	{
		m_status &= ~STATUS_PLAYING;
		return;
	}

	// otherwise, advance the step
	uint32_t position = m_position + m_regs.delta_n();
	m_position = uint16_t(position);
	if (position < 0x10000)
		return;

	// if we're about to process nibble 0, fetch sample
	if (m_curnibble == 0)
	{
		// playing from RAM/ROM
		if (m_regs.external())
			m_curbyte = m_owner.intf().ymfm_external_read(ACCESS_ADPCM_B, m_curaddress);
	}

	// extract the nibble from our current byte
	uint8_t data = uint8_t(m_curbyte << (4 * m_curnibble)) >> 4;
	m_curnibble ^= 1;

	// we just processed the last nibble
	if (m_curnibble == 0)
	{
		// if playing from RAM/ROM, check the end/limit address or advance
		if (m_regs.external())
		{
			// handle the sample end, either repeating or stopping
			if (at_end())
			{
				// if repeating, go back to the start
				if (m_regs.repeat())
					load_start();

				// otherwise, done; set the EOS bit
				else
				{
					m_accumulator = 0;
					m_prev_accum = 0;
					m_status = (m_status & ~STATUS_PLAYING) | STATUS_EOS;
					debug::log_keyon("%s\n", "ADPCM EOS");
					return;
				}
			}

			// wrap at the limit address
			else if (at_limit())
				m_curaddress = 0;

			// otherwise, advance the current address
			else
			{
				m_curaddress++;
				m_curaddress &= 0xffffff;
			}
		}

		// if CPU-driven, copy the next byte and request more
		else
		{
			m_curbyte = m_regs.cpudata();
			m_status |= STATUS_BRDY;
		}
	}

	// remember previous value for interpolation
	m_prev_accum = m_accumulator;

	// forecast to next forecast: 1/8, 3/8, 5/8, 7/8, 9/8, 11/8, 13/8, 15/8
	int32_t delta = (2 * bitfield(data, 0, 3) + 1) * m_adpcm_step / 8;
	if (bitfield(data, 3))
		delta = -delta;

	// add and clamp to 16 bits
	m_accumulator = clamp(m_accumulator + delta, -32768, 32767);

	// scale the ADPCM step: 0.9, 0.9, 0.9, 0.9, 1.2, 1.6, 2.0, 2.4
	static uint8_t const s_step_scale[8] = { 57, 57, 57, 57, 77, 102, 128, 153 };
	m_adpcm_step = clamp((m_adpcm_step * s_step_scale[bitfield(data, 0, 3)]) / 64, STEP_MIN, STEP_MAX);
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
	int32_t result = (m_prev_accum * int32_t((m_position ^ 0xffff) + 1) + m_accumulator * int32_t(m_position)) >> 16;

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
	{
		// two dummy reads are consumed first
		if (m_dummy_read != 0)
		{
			load_start();
			m_dummy_read--;
		}

		// read the data
		else
		{
			// read from outside of the chip
			result = m_owner.intf().ymfm_external_read(ACCESS_ADPCM_B, m_curaddress++);

			// did we hit the end? if so, signal EOS
			if (at_end())
			{
				m_status = STATUS_EOS | STATUS_BRDY;
				debug::log_keyon("%s\n", "ADPCM EOS");
			}
			else
			{
				// signal ready
				m_status = STATUS_BRDY;
			}

			// wrap at the limit address
			if (at_limit())
				m_curaddress = 0;
		}
	}
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
		if (m_regs.execute())
		{
			load_start();

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
		else
			m_status &= ~STATUS_EOS;
		if (m_regs.resetflag())
			reset();
		if (m_regs.external())
			m_dummy_read = 2;
	}

	// register 8 writes over the bus under some conditions
	else if (regnum == 0x08)
	{
		// if writing from the CPU during execute, clear the ready flag
		if (m_regs.execute() && !m_regs.record() && !m_regs.external())
			m_status &= ~STATUS_BRDY;

		// if writing during "record", pass through as data
		else if (!m_regs.execute() && m_regs.record() && m_regs.external())
		{
			// clear out dummy reads and set start address
			if (m_dummy_read != 0)
			{
				load_start();
				m_dummy_read = 0;
			}

			// did we hit the end? if so, signal EOS
			if (at_end())
			{
				debug::log_keyon("%s\n", "ADPCM EOS");
				m_status = STATUS_EOS | STATUS_BRDY;
			}

			// otherwise, write the data and signal ready
			else
			{
				m_owner.intf().ymfm_external_write(ACCESS_ADPCM_B, m_curaddress++, value);
				m_status = STATUS_BRDY;
			}
		}
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
//  load_start - load the start address and
//  initialize the state
//-------------------------------------------------

void adpcm_b_channel::load_start()
{
	m_status = (m_status & ~STATUS_EOS) | STATUS_PLAYING;
	m_curaddress = m_regs.external() ? (m_regs.start() << address_shift()) : 0;
	m_curnibble = 0;
	m_curbyte = 0;
	m_position = 0;
	m_accumulator = 0;
	m_prev_accum = 0;
	m_adpcm_step = STEP_MIN;
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
