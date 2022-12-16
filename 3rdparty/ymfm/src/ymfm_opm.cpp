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

#include "ymfm_opm.h"
#include "ymfm_fm.ipp"

namespace ymfm
{

//*********************************************************
//  OPM REGISTERS
//*********************************************************

//-------------------------------------------------
//  opm_registers - constructor
//-------------------------------------------------

opm_registers::opm_registers() :
	m_lfo_counter(0),
	m_noise_lfsr(1),
	m_noise_counter(0),
	m_noise_state(0),
	m_noise_lfo(0),
	m_lfo_am(0)
{
	// create the waveforms
	for (uint32_t index = 0; index < WAVEFORM_LENGTH; index++)
		m_waveform[0][index] = abs_sin_attenuation(index) | (bitfield(index, 9) << 15);

	// create the LFO waveforms; AM in the low 8 bits, PM in the upper 8
	// waveforms are adjusted to match the pictures in the application manual
	for (uint32_t index = 0; index < LFO_WAVEFORM_LENGTH; index++)
	{
		// waveform 0 is a sawtooth
		uint8_t am = index ^ 0xff;
		int8_t pm = int8_t(index);
		m_lfo_waveform[0][index] = am | (pm << 8);

		// waveform 1 is a square wave
		am = bitfield(index, 7) ? 0 : 0xff;
		pm = int8_t(am ^ 0x80);
		m_lfo_waveform[1][index] = am | (pm << 8);

		// waveform 2 is a triangle wave
		am = bitfield(index, 7) ? (index << 1) : ((index ^ 0xff) << 1);
		pm = int8_t(bitfield(index, 6) ? am : ~am);
		m_lfo_waveform[2][index] = am | (pm << 8);

		// waveform 3 is noise; it is filled in dynamically
		m_lfo_waveform[3][index] = 0;
	}
}


//-------------------------------------------------
//  reset - reset to initial state
//-------------------------------------------------

void opm_registers::reset()
{
	std::fill_n(&m_regdata[0], REGISTERS, 0);

	// enable output on both channels by default
	m_regdata[0x20] = m_regdata[0x21] = m_regdata[0x22] = m_regdata[0x23] = 0xc0;
	m_regdata[0x24] = m_regdata[0x25] = m_regdata[0x26] = m_regdata[0x27] = 0xc0;
}


//-------------------------------------------------
//  save_restore - save or restore the data
//-------------------------------------------------

void opm_registers::save_restore(ymfm_saved_state &state)
{
	state.save_restore(m_lfo_counter);
	state.save_restore(m_lfo_am);
	state.save_restore(m_noise_lfsr);
	state.save_restore(m_noise_counter);
	state.save_restore(m_noise_state);
	state.save_restore(m_noise_lfo);
	state.save_restore(m_regdata);
}


//-------------------------------------------------
//  operator_map - return an array of operator
//  indices for each channel; for OPM this is fixed
//-------------------------------------------------

void opm_registers::operator_map(operator_mapping &dest) const
{
	// Note that the channel index order is 0,2,1,3, so we bitswap the index.
	//
	// This is because the order in the map is:
	//    carrier 1, carrier 2, modulator 1, modulator 2
	//
	// But when wiring up the connections, the more natural order is:
	//    carrier 1, modulator 1, carrier 2, modulator 2
	static const operator_mapping s_fixed_map =
	{ {
		operator_list(  0, 16,  8, 24 ),  // Channel 0 operators
		operator_list(  1, 17,  9, 25 ),  // Channel 1 operators
		operator_list(  2, 18, 10, 26 ),  // Channel 2 operators
		operator_list(  3, 19, 11, 27 ),  // Channel 3 operators
		operator_list(  4, 20, 12, 28 ),  // Channel 4 operators
		operator_list(  5, 21, 13, 29 ),  // Channel 5 operators
		operator_list(  6, 22, 14, 30 ),  // Channel 6 operators
		operator_list(  7, 23, 15, 31 ),  // Channel 7 operators
	} };
	dest = s_fixed_map;
}


//-------------------------------------------------
//  write - handle writes to the register array
//-------------------------------------------------

bool opm_registers::write(uint16_t index, uint8_t data, uint32_t &channel, uint32_t &opmask)
{
	assert(index < REGISTERS);

	// LFO AM/PM depth are written to the same register (0x19);
	// redirect the PM depth to an unused neighbor (0x1a)
	if (index == 0x19)
		m_regdata[index + bitfield(data, 7)] = data;
	else if (index != 0x1a)
		m_regdata[index] = data;

	// handle writes to the key on index
	if (index == 0x08)
	{
		channel = bitfield(data, 0, 3);
		opmask = bitfield(data, 3, 4);
		return true;
	}
	return false;
}


//-------------------------------------------------
//  clock_noise_and_lfo - clock the noise and LFO,
//  handling clock division, depth, and waveform
//  computations
//-------------------------------------------------

int32_t opm_registers::clock_noise_and_lfo()
{
	// base noise frequency is measured at 2x 1/2 FM frequency; this
	// means each tick counts as two steps against the noise counter
	uint32_t freq = noise_frequency();
	for (int rep = 0; rep < 2; rep++)
	{
		// evidence seems to suggest the LFSR is clocked continually and just
		// sampled at the noise frequency for output purposes; note that the
		// low 8 bits are the most recent 8 bits of history while bits 8-24
		// contain the 17 bit LFSR state
		m_noise_lfsr <<= 1;
		m_noise_lfsr |= bitfield(m_noise_lfsr, 17) ^ bitfield(m_noise_lfsr, 14) ^ 1;

		// compare against the frequency and latch when we exceed it
		if (m_noise_counter++ >= freq)
		{
			m_noise_counter = 0;
			m_noise_state = bitfield(m_noise_lfsr, 17);
		}
	}

	// treat the rate as a 4.4 floating-point step value with implied
	// leading 1; this matches exactly the frequencies in the application
	// manual, though it might not be implemented exactly this way on chip
	uint32_t rate = lfo_rate();
	m_lfo_counter += (0x10 | bitfield(rate, 0, 4)) << bitfield(rate, 4, 4);

	// bit 1 of the test register is officially undocumented but has been
	// discovered to hold the LFO in reset while active
	if (lfo_reset())
		m_lfo_counter = 0;

	// now pull out the non-fractional LFO value
	uint32_t lfo = bitfield(m_lfo_counter, 22, 8);

	// fill in the noise entry 1 ahead of our current position; this
	// ensures the current value remains stable for a full LFO clock
	// and effectively latches the running value when the LFO advances
	uint32_t lfo_noise = bitfield(m_noise_lfsr, 17, 8);
	m_lfo_waveform[3][(lfo + 1) & 0xff] = lfo_noise | (lfo_noise << 8);

	// fetch the AM/PM values based on the waveform; AM is unsigned and
	// encoded in the low 8 bits, while PM signed and encoded in the upper
	// 8 bits
	int32_t ampm = m_lfo_waveform[lfo_waveform()][lfo];

	// apply depth to the AM value and store for later
	m_lfo_am = ((ampm & 0xff) * lfo_am_depth()) >> 7;

	// apply depth to the PM value and return it
	return ((ampm >> 8) * int32_t(lfo_pm_depth())) >> 7;
}


//-------------------------------------------------
//  lfo_am_offset - return the AM offset from LFO
//  for the given channel
//-------------------------------------------------

uint32_t opm_registers::lfo_am_offset(uint32_t choffs) const
{
	// OPM maps AM quite differently from OPN

	// shift value for AM sensitivity is [*, 0, 1, 2],
	// mapping to values of [0, 23.9, 47.8, and 95.6dB]
	uint32_t am_sensitivity = ch_lfo_am_sens(choffs);
	if (am_sensitivity == 0)
		return 0;

	// QUESTION: see OPN note below for the dB range mapping; it applies
	// here as well

	// raw LFO AM value on OPM is 0-FF, which is already a factor of 2
	// larger than the OPN below, putting our staring point at 2x theirs;
	// this works out since our minimum is 2x their maximum
	return m_lfo_am << (am_sensitivity - 1);
}


//-------------------------------------------------
//  cache_operator_data - fill the operator cache
//  with prefetched data
//-------------------------------------------------

void opm_registers::cache_operator_data(uint32_t choffs, uint32_t opoffs, opdata_cache &cache)
{
	// set up the easy stuff
	cache.waveform = &m_waveform[0][0];

	// get frequency from the channel
	uint32_t block_freq = cache.block_freq = ch_block_freq(choffs);

	// compute the keycode: block_freq is:
	//
	//     BBBCCCCFFFFFF
	//     ^^^^^
	//
	// the 5-bit keycode is just the top 5 bits (block + top 2 bits
	// of the key code)
	uint32_t keycode = bitfield(block_freq, 8, 5);

	// detune adjustment
	cache.detune = detune_adjustment(op_detune(opoffs), keycode);

	// multiple value, as an x.1 value (0 means 0.5)
	cache.multiple = op_multiple(opoffs) * 2;
	if (cache.multiple == 0)
		cache.multiple = 1;

	// phase step, or PHASE_STEP_DYNAMIC if PM is active; this depends on
	// block_freq, detune, and multiple, so compute it after we've done those
	if (lfo_pm_depth() == 0 || ch_lfo_pm_sens(choffs) == 0)
		cache.phase_step = compute_phase_step(choffs, opoffs, cache, 0);
	else
		cache.phase_step = opdata_cache::PHASE_STEP_DYNAMIC;

	// total level, scaled by 8
	cache.total_level = op_total_level(opoffs) << 3;

	// 4-bit sustain level, but 15 means 31 so effectively 5 bits
	cache.eg_sustain = op_sustain_level(opoffs);
	cache.eg_sustain |= (cache.eg_sustain + 1) & 0x10;
	cache.eg_sustain <<= 5;

	// determine KSR adjustment for enevlope rates
	uint32_t ksrval = keycode >> (op_ksr(opoffs) ^ 3);
	cache.eg_rate[EG_ATTACK] = effective_rate(op_attack_rate(opoffs) * 2, ksrval);
	cache.eg_rate[EG_DECAY] = effective_rate(op_decay_rate(opoffs) * 2, ksrval);
	cache.eg_rate[EG_SUSTAIN] = effective_rate(op_sustain_rate(opoffs) * 2, ksrval);
	cache.eg_rate[EG_RELEASE] = effective_rate(op_release_rate(opoffs) * 4 + 2, ksrval);
}


//-------------------------------------------------
//  compute_phase_step - compute the phase step
//-------------------------------------------------

uint32_t opm_registers::compute_phase_step(uint32_t choffs, uint32_t opoffs, opdata_cache const &cache, int32_t lfo_raw_pm)
{
	// OPM logic is rather unique here, due to extra detune
	// and the use of key codes (not to be confused with keycode)

	// start with coarse detune delta; table uses cents value from
	// manual, converted into 1/64ths
	static const int16_t s_detune2_delta[4] = { 0, (600*64+50)/100, (781*64+50)/100, (950*64+50)/100 };
	int32_t delta = s_detune2_delta[op_detune2(opoffs)];

	// add in the PM delta
	uint32_t pm_sensitivity = ch_lfo_pm_sens(choffs);
	if (pm_sensitivity != 0)
	{
		// raw PM value is -127..128 which is +/- 200 cents
		// manual gives these magnitudes in cents:
		//    0, +/-5, +/-10, +/-20, +/-50, +/-100, +/-400, +/-700
		// this roughly corresponds to shifting the 200-cent value:
		//    0  >> 5,  >> 4,  >> 3,  >> 2,  >> 1,   << 1,   << 2
		if (pm_sensitivity < 6)
			delta += lfo_raw_pm >> (6 - pm_sensitivity);
		else
			delta += lfo_raw_pm << (pm_sensitivity - 5);
	}

	// apply delta and convert to a frequency number
	uint32_t phase_step = opm_key_code_to_phase_step(cache.block_freq, delta);

	// apply detune based on the keycode
	phase_step += cache.detune;

	// apply frequency multiplier (which is cached as an x.1 value)
	return (phase_step * cache.multiple) >> 1;
}


//-------------------------------------------------
//  log_keyon - log a key-on event
//-------------------------------------------------

std::string opm_registers::log_keyon(uint32_t choffs, uint32_t opoffs)
{
	uint32_t chnum = choffs;
	uint32_t opnum = opoffs;

	char buffer[256];
	char *end = &buffer[0];

	end += sprintf(end, "%u.%02u freq=%04X dt2=%u dt=%u fb=%u alg=%X mul=%X tl=%02X ksr=%u adsr=%02X/%02X/%02X/%X sl=%X out=%c%c",
		chnum, opnum,
		ch_block_freq(choffs),
		op_detune2(opoffs),
		op_detune(opoffs),
		ch_feedback(choffs),
		ch_algorithm(choffs),
		op_multiple(opoffs),
		op_total_level(opoffs),
		op_ksr(opoffs),
		op_attack_rate(opoffs),
		op_decay_rate(opoffs),
		op_sustain_rate(opoffs),
		op_release_rate(opoffs),
		op_sustain_level(opoffs),
		ch_output_0(choffs) ? 'L' : '-',
		ch_output_1(choffs) ? 'R' : '-');

	bool am = (lfo_am_depth() != 0 && ch_lfo_am_sens(choffs) != 0 && op_lfo_am_enable(opoffs) != 0);
	if (am)
		end += sprintf(end, " am=%u/%02X", ch_lfo_am_sens(choffs), lfo_am_depth());
	bool pm = (lfo_pm_depth() != 0 && ch_lfo_pm_sens(choffs) != 0);
	if (pm)
		end += sprintf(end, " pm=%u/%02X", ch_lfo_pm_sens(choffs), lfo_pm_depth());
	if (am || pm)
		end += sprintf(end, " lfo=%02X/%c", lfo_rate(), "WQTN"[lfo_waveform()]);
	if (noise_enable() && opoffs == 31)
		end += sprintf(end, " noise=1");

	return buffer;
}



//*********************************************************
//  YM2151
//*********************************************************

//-------------------------------------------------
//  ym2151 - constructor
//-------------------------------------------------

ym2151::ym2151(ymfm_interface &intf, opm_variant variant) :
	m_variant(variant),
	m_address(0),
	m_fm(intf)
{
}


//-------------------------------------------------
//  reset - reset the system
//-------------------------------------------------

void ym2151::reset()
{
	// reset the engines
	m_fm.reset();
}


//-------------------------------------------------
//  save_restore - save or restore the data
//-------------------------------------------------

void ym2151::save_restore(ymfm_saved_state &state)
{
	m_fm.save_restore(state);
	state.save_restore(m_address);
}


//-------------------------------------------------
//  read_status - read the status register
//-------------------------------------------------

uint8_t ym2151::read_status()
{
	uint8_t result = m_fm.status();
	if (m_fm.intf().ymfm_is_busy())
		result |= fm_engine::STATUS_BUSY;
	return result;
}


//-------------------------------------------------
//  read - handle a read from the device
//-------------------------------------------------

uint8_t ym2151::read(uint32_t offset)
{
	uint8_t result = 0xff;
	switch (offset & 1)
	{
		case 0: // data port (unused)
			debug::log_unexpected_read_write("Unexpected read from YM2151 offset %d\n", offset & 3);
			break;

		case 1: // status port, YM2203 compatible
			result = read_status();
			break;
	}
	return result;
}


//-------------------------------------------------
//  write_address - handle a write to the address
//  register
//-------------------------------------------------

void ym2151::write_address(uint8_t data)
{
	// just set the address
	m_address = data;
}


//-------------------------------------------------
//  write - handle a write to the register
//  interface
//-------------------------------------------------

void ym2151::write_data(uint8_t data)
{
	// write the FM register
	m_fm.write(m_address, data);

	// special cases
	if (m_address == 0x1b)
	{
		// writes to register 0x1B send the upper 2 bits to the output lines
		m_fm.intf().ymfm_external_write(ACCESS_IO, 0, data >> 6);
	}

	// mark busy for a bit
	m_fm.intf().ymfm_set_busy_end(32 * m_fm.clock_prescale());
}


//-------------------------------------------------
//  write - handle a write to the register
//  interface
//-------------------------------------------------

void ym2151::write(uint32_t offset, uint8_t data)
{
	switch (offset & 1)
	{
		case 0: // address port
			write_address(data);
			break;

		case 1: // data port
			write_data(data);
			break;
	}
}


//-------------------------------------------------
//  generate - generate one sample of sound
//-------------------------------------------------

void ym2151::generate(output_data *output, uint32_t numsamples)
{
	for (uint32_t samp = 0; samp < numsamples; samp++, output++)
	{
		// clock the system
		m_fm.clock(fm_engine::ALL_CHANNELS);

		// update the FM content; OPM is full 14-bit with no intermediate clipping
		m_fm.output(output->clear(), 0, 32767, fm_engine::ALL_CHANNELS);

		// YM2151 uses an external DAC (YM3012) with mantissa/exponent format
		// convert to 10.3 floating point value and back to simulate truncation
		output->roundtrip_fp();
	}
}

}
