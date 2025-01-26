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

#include "ymfm_opq.h"
#include "ymfm_fm.ipp"

#define TEMPORARY_DEBUG_PRINTS (0)

//
// OPQ (aka YM3806/YM3533)
//
// This chip is not officially documented as far as I know. What I have
// comes from Jari Kangas' work on reverse engineering the PSR70:
//
//    https://github.com/JKN0/PSR70-reverse
//
// OPQ appears be bsaically a mixture of OPM and OPN.
//

namespace ymfm
{

//*********************************************************
//  OPQ SPECIFICS
//*********************************************************

//-------------------------------------------------
//  opq_registers - constructor
//-------------------------------------------------

opq_registers::opq_registers() :
	m_lfo_counter(0),
	m_lfo_am(0)
{
	// create the waveforms
	for (uint32_t index = 0; index < WAVEFORM_LENGTH; index++)
		m_waveform[0][index] = abs_sin_attenuation(index) | (bitfield(index, 9) << 15);

	uint16_t zeroval = m_waveform[0][0];
	for (uint32_t index = 0; index < WAVEFORM_LENGTH; index++)
		m_waveform[1][index] = bitfield(index, 9) ? zeroval : m_waveform[0][index];
}


//-------------------------------------------------
//  reset - reset to initial state
//-------------------------------------------------

void opq_registers::reset()
{
	std::fill_n(&m_regdata[0], REGISTERS, 0);

	// enable output on both channels by default
	m_regdata[0x10] = m_regdata[0x11] = m_regdata[0x12] = m_regdata[0x13] = 0xc0;
	m_regdata[0x14] = m_regdata[0x15] = m_regdata[0x16] = m_regdata[0x17] = 0xc0;
}


//-------------------------------------------------
//  save_restore - save or restore the data
//-------------------------------------------------

void opq_registers::save_restore(ymfm_saved_state &state)
{
	state.save_restore(m_lfo_counter);
	state.save_restore(m_lfo_am);
	state.save_restore(m_regdata);
}


//-------------------------------------------------
//  operator_map - return an array of operator
//  indices for each channel; for OPM this is fixed
//-------------------------------------------------

void opq_registers::operator_map(operator_mapping &dest) const
{
	// seems like the operators are not swizzled like they are on OPM/OPN?
	static const operator_mapping s_fixed_map =
	{ {
		operator_list(  0,  8, 16, 24 ),  // Channel 0 operators
		operator_list(  1,  9, 17, 25 ),  // Channel 1 operators
		operator_list(  2, 10, 18, 26 ),  // Channel 2 operators
		operator_list(  3, 11, 19, 27 ),  // Channel 3 operators
		operator_list(  4, 12, 20, 28 ),  // Channel 4 operators
		operator_list(  5, 13, 21, 29 ),  // Channel 5 operators
		operator_list(  6, 14, 22, 30 ),  // Channel 6 operators
		operator_list(  7, 15, 23, 31 ),  // Channel 7 operators
	} };
	dest = s_fixed_map;
}


//-------------------------------------------------
//  write - handle writes to the register array
//-------------------------------------------------

bool opq_registers::write(uint16_t index, uint8_t data, uint32_t &channel, uint32_t &opmask)
{
	assert(index < REGISTERS);

	// detune/multiple share a register based on the MSB of what is written
	// remap the multiple values to 100-11F
	if ((index & 0xe0) == 0x40 && bitfield(data, 7) != 0)
		index += 0xc0;

	m_regdata[index] = data;

	// handle writes to the key on index
	if (index == 0x05)
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

int32_t opq_registers::clock_noise_and_lfo()
{
	// OPQ LFO is not well-understood, but the enable and rate values
	// look a lot like OPN, so we'll crib from there as a starting point

	// if LFO not enabled (not present on OPN), quick exit with 0s
	if (!lfo_enable())
	{
		m_lfo_counter = 0;
		m_lfo_am = 0;
		return 0;
	}

	// this table is based on converting the frequencies in the applications
	// manual to clock dividers, based on the assumption of a 7-bit LFO value
	static uint8_t const lfo_max_count[8] = { 109, 78, 72, 68, 63, 45, 9, 6 };
	uint32_t subcount = uint8_t(m_lfo_counter++);

	// when we cross the divider count, add enough to zero it and cause an
	// increment at bit 8; the 7-bit value lives from bits 8-14
	if (subcount >= lfo_max_count[lfo_rate()])
		m_lfo_counter += 0x101 - subcount;

	// AM value is 7 bits, staring at bit 8; grab the low 6 directly
	m_lfo_am = bitfield(m_lfo_counter, 8, 6);

	// first half of the AM period (bit 6 == 0) is inverted
	if (bitfield(m_lfo_counter, 8+6) == 0)
		m_lfo_am ^= 0x3f;

	// PM value is 5 bits, starting at bit 10; grab the low 3 directly
	int32_t pm = bitfield(m_lfo_counter, 10, 3);

	// PM is reflected based on bit 3
	if (bitfield(m_lfo_counter, 10+3))
		pm ^= 7;

	// PM is negated based on bit 4
	return bitfield(m_lfo_counter, 10+4) ? -pm : pm;
}


//-------------------------------------------------
//  lfo_am_offset - return the AM offset from LFO
//  for the given channel
//-------------------------------------------------

uint32_t opq_registers::lfo_am_offset(uint32_t choffs) const
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

void opq_registers::cache_operator_data(uint32_t choffs, uint32_t opoffs, opdata_cache &cache)
{
	// set up the easy stuff
	cache.waveform = &m_waveform[op_waveform(opoffs)][0];

	// get frequency from the appropriate registers
	uint32_t block_freq = cache.block_freq = (opoffs & 8) ? ch_block_freq_24(choffs) : ch_block_freq_13(choffs);

	// compute the keycode: block_freq is:
	//
	//     BBBFFFFFFFFFFFF
	//     ^^^^???
	//
	// keycode is not understood, so just guessing it is like OPN:
	// the 5-bit keycode uses the top 4 bits plus a magic formula
	// for the final bit
	uint32_t keycode = bitfield(block_freq, 11, 4) << 1;

	// lowest bit is determined by a mix of next lower FNUM bits
	// according to this equation from the YM2608 manual:
	//
	//   (F11 & (F10 | F9 | F8)) | (!F11 & F10 & F9 & F8)
	//
	// for speed, we just look it up in a 16-bit constant
	keycode |= bitfield(0xfe80, bitfield(block_freq, 8, 4));

	// detune adjustment: the detune values supported by the OPQ are
	// a much larger range (6 bits vs 3 bits) compared to any other
	// known FM chip; based on experiments, it seems that the extra
	// bits provide a bigger detune range rather than finer control,
	// so until we get true measurements just assemble a net detune
	// value by summing smaller detunes
	int32_t detune = int32_t(op_detune(opoffs)) - 0x20;
	int32_t abs_detune = std::abs(detune);
	int32_t adjust = (abs_detune / 3) * detune_adjustment(3, keycode) + detune_adjustment(abs_detune % 3, keycode);
	cache.detune = (detune >= 0) ? adjust : -adjust;

	// multiple value, as an x.1 value (0 means 0.5)
	static const uint8_t s_multiple_map[16] = { 1,2,4,6,8,10,12,14,16,18,20,24,30,32,34,36 };
	cache.multiple = s_multiple_map[op_multiple(opoffs)];

	// phase step, or PHASE_STEP_DYNAMIC if PM is active; this depends on
	// block_freq, detune, and multiple, so compute it after we've done those
	if (lfo_enable() == 0 || ch_lfo_pm_sens(choffs) == 0)
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
	cache.eg_rate[EG_REVERB] = (ch_reverb(choffs) != 0) ? 5*4 : cache.eg_rate[EG_RELEASE];
	cache.eg_shift = 0;
}


//-------------------------------------------------
//  compute_phase_step - compute the phase step
//-------------------------------------------------

uint32_t opq_registers::compute_phase_step(uint32_t choffs, uint32_t opoffs, opdata_cache const &cache, int32_t lfo_raw_pm)
{
	// OPN phase calculation has only a single detune parameter
	// and uses FNUMs instead of keycodes

	// extract frequency number (low 12 bits of block_freq)
	uint32_t fnum = bitfield(cache.block_freq, 0, 12);

	// if there's a non-zero PM sensitivity, compute the adjustment
	uint32_t pm_sensitivity = ch_lfo_pm_sens(choffs);
	if (pm_sensitivity != 0)
	{
		// apply the phase adjustment based on the upper 7 bits
		// of FNUM and the PM depth parameters
		fnum += opn_lfo_pm_phase_adjustment(bitfield(cache.block_freq, 5, 7), pm_sensitivity, lfo_raw_pm);

		// keep fnum to 12 bits
		fnum &= 0xfff;
	}

	// apply block shift to compute phase step
	uint32_t block = bitfield(cache.block_freq, 12, 3);
	uint32_t phase_step = (fnum << block) >> 2;

	// apply detune based on the keycode
	phase_step += cache.detune;

	// clamp to 17 bits in case detune overflows
	// QUESTION: is this specific to the YM2612/3438?
	phase_step &= 0x1ffff;

	// apply frequency multiplier (which is cached as an x.1 value)
	return (phase_step * cache.multiple) >> 1;
}


//-------------------------------------------------
//  log_keyon - log a key-on event
//-------------------------------------------------

std::string opq_registers::log_keyon(uint32_t choffs, uint32_t opoffs)
{
	uint32_t chnum = choffs;
	uint32_t opnum = opoffs;

	char buffer[256];
	int end = 0;

	end += snprintf(&buffer[end], sizeof(buffer) - end, "%u.%02u freq=%04X dt=%+2d fb=%u alg=%X mul=%X tl=%02X ksr=%u adsr=%02X/%02X/%02X/%X sl=%X out=%c%c",
		chnum, opnum,
		(opoffs & 1) ? ch_block_freq_24(choffs) : ch_block_freq_13(choffs),
		int32_t(op_detune(opoffs)) - 0x20,
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

	bool am = (lfo_enable() && op_lfo_am_enable(opoffs) && ch_lfo_am_sens(choffs) != 0);
	if (am)
		end += snprintf(&buffer[end], sizeof(buffer) - end, " am=%u", ch_lfo_am_sens(choffs));
	bool pm = (lfo_enable() && ch_lfo_pm_sens(choffs) != 0);
	if (pm)
		end += snprintf(&buffer[end], sizeof(buffer) - end, " pm=%u", ch_lfo_pm_sens(choffs));
	if (am || pm)
		end += snprintf(&buffer[end], sizeof(buffer) - end, " lfo=%02X", lfo_rate());
	if (ch_reverb(choffs))
		end += snprintf(&buffer[end], sizeof(buffer) - end, " reverb");

	return buffer;
}



//*********************************************************
//  YM3806
//*********************************************************

//-------------------------------------------------
//  ym3806 - constructor
//-------------------------------------------------

ym3806::ym3806(ymfm_interface &intf) :
	m_fm(intf)
{
}


//-------------------------------------------------
//  reset - reset the system
//-------------------------------------------------

void ym3806::reset()
{
	// reset the engines
	m_fm.reset();
}


//-------------------------------------------------
//  save_restore - save or restore the data
//-------------------------------------------------

void ym3806::save_restore(ymfm_saved_state &state)
{
	m_fm.save_restore(state);
}


//-------------------------------------------------
//  read_status - read the status register
//-------------------------------------------------

uint8_t ym3806::read_status()
{
	uint8_t result = m_fm.status();
	if (m_fm.intf().ymfm_is_busy())
		result |= fm_engine::STATUS_BUSY;
	return result;
}


//-------------------------------------------------
//  read - handle a read from the device
//-------------------------------------------------

uint8_t ym3806::read(uint32_t offset)
{
	uint8_t result = 0xff;
	switch (offset)
	{
		case 0: // status port
			result = read_status();
			break;

		default: // unknown
			debug::log_unexpected_read_write("Unexpected read from YM3806 offset %02X\n", offset);
			break;
	}
if (TEMPORARY_DEBUG_PRINTS && offset != 0) printf("Read %02X = %02X\n", offset, result);
	return result;
}


//-------------------------------------------------
//  write - handle a write to the register
//  interface
//-------------------------------------------------

void ym3806::write(uint32_t offset, uint8_t data)
{
if (TEMPORARY_DEBUG_PRINTS && (offset != 3 || data != 0x71)) printf("Write %02X = %02X\n", offset, data);
	// write the FM register
	m_fm.write(offset, data);
}


//-------------------------------------------------
//  generate - generate one sample of sound
//-------------------------------------------------

void ym3806::generate(output_data *output, uint32_t numsamples)
{
	for (uint32_t samp = 0; samp < numsamples; samp++, output++)
	{
		// clock the system
		m_fm.clock(fm_engine::ALL_CHANNELS);

		// update the FM content; YM3806 is full 14-bit with no intermediate clipping
		m_fm.output(output->clear(), 0, 32767, fm_engine::ALL_CHANNELS);

		// YM3608 appears to go through a YM3012 DAC, which means we want to apply
		// the FP truncation logic to the outputs
		output->roundtrip_fp();
	}
}

}
