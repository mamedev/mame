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

#include "ymfm_opn.h"
#include "ymfm_fm.ipp"

namespace ymfm
{

//*********************************************************
//  OPN/OPNA REGISTERS
//*********************************************************

//-------------------------------------------------
//  opn_registers_base - constructor
//-------------------------------------------------

template<bool IsOpnA>
opn_registers_base<IsOpnA>::opn_registers_base() :
	m_lfo_counter(0),
	m_lfo_am(0)
{
	// create the waveforms
	for (uint32_t index = 0; index < WAVEFORM_LENGTH; index++)
		m_waveform[0][index] = abs_sin_attenuation(index) | (bitfield(index, 9) << 15);
}


//-------------------------------------------------
//  reset - reset to initial state
//-------------------------------------------------

template<bool IsOpnA>
void opn_registers_base<IsOpnA>::reset()
{
	std::fill_n(&m_regdata[0], REGISTERS, 0);
	if (IsOpnA)
	{
		// enable output on both channels by default
		m_regdata[0xb4] = m_regdata[0xb5] = m_regdata[0xb6] = 0xc0;
		m_regdata[0x1b4] = m_regdata[0x1b5] = m_regdata[0x1b6] = 0xc0;
	}
}


//-------------------------------------------------
//  save_restore - save or restore the data
//-------------------------------------------------

template<bool IsOpnA>
void opn_registers_base<IsOpnA>::save_restore(ymfm_saved_state &state)
{
	if (IsOpnA)
	{
		state.save_restore(m_lfo_counter);
		state.save_restore(m_lfo_am);
	}
	state.save_restore(m_regdata);
}


//-------------------------------------------------
//  operator_map - return an array of operator
//  indices for each channel; for OPN this is fixed
//-------------------------------------------------

template<>
void opn_registers_base<false>::operator_map(operator_mapping &dest) const
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
		operator_list(  0,  6,  3,  9 ),  // Channel 0 operators
		operator_list(  1,  7,  4, 10 ),  // Channel 1 operators
		operator_list(  2,  8,  5, 11 ),  // Channel 2 operators
	} };
	dest = s_fixed_map;
}

template<>
void opn_registers_base<true>::operator_map(operator_mapping &dest) const
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
		operator_list(  0,  6,  3,  9 ),  // Channel 0 operators
		operator_list(  1,  7,  4, 10 ),  // Channel 1 operators
		operator_list(  2,  8,  5, 11 ),  // Channel 2 operators
		operator_list( 12, 18, 15, 21 ),  // Channel 3 operators
		operator_list( 13, 19, 16, 22 ),  // Channel 4 operators
		operator_list( 14, 20, 17, 23 ),  // Channel 5 operators
	} };
	dest = s_fixed_map;
}


//-------------------------------------------------
//  write - handle writes to the register array
//-------------------------------------------------

template<bool IsOpnA>
bool opn_registers_base<IsOpnA>::write(uint16_t index, uint8_t data, uint32_t &channel, uint32_t &opmask)
{
	assert(index < REGISTERS);

	// writes in the 0xa0-af/0x1a0-af region are handled as latched pairs
	// borrow unused registers 0xb8-bf as temporary holding locations
	if ((index & 0xf0) == 0xa0)
	{
		if (bitfield(index, 0, 2) == 3)
			return false;

		uint32_t latchindex = 0xb8 | bitfield(index, 3);

		// writes to the upper half just latch (only low 6 bits matter)
		if (bitfield(index, 2))
			m_regdata[latchindex] = data & 0x3f;

		// writes to the lower half also apply said latch
		else
		{
			m_regdata[index] = data;
			m_regdata[index | 4] = m_regdata[latchindex];
		}
		return false;
	}
	else if ((index & 0xf8) == 0xb8)
	{
		// registers 0xb8-0xbf are used internally
		return false;
	}

	// everything else is normal
	m_regdata[index] = data;

	// handle writes to the key on index
	if (index == 0x28)
	{
		channel = bitfield(data, 0, 2);
		if (channel == 3)
			return false;
		if (IsOpnA)
			channel += bitfield(data, 2, 1) * 3;
		opmask = bitfield(data, 4, 4);
		return true;
	}
	return false;
}


//-------------------------------------------------
//  clock_noise_and_lfo - clock the noise and LFO,
//  handling clock division, depth, and waveform
//  computations
//-------------------------------------------------

template<bool IsOpnA>
int32_t opn_registers_base<IsOpnA>::clock_noise_and_lfo()
{
	// OPN has no noise generation

	// if LFO not enabled (not present on OPN), quick exit with 0s
	if (!IsOpnA || !lfo_enable())
	{
		m_lfo_counter = 0;

		// special case: if LFO is disabled on OPNA, it basically just keeps the counter
		// at 0; since position 0 gives an AM value of 0x3f, it is important to reflect
		// that here; for example, MegaDrive Venom plays some notes with LFO globally
		// disabled but enabling LFO on the operators, and it expects this added attenutation
		m_lfo_am = IsOpnA ? 0x3f : 0x00;
		return 0;
	}

	// this table is based on converting the frequencies in the applications
	// manual to clock dividers, based on the assumption of a 7-bit LFO value
	static uint8_t const lfo_max_count[8] = { 109, 78, 72, 68, 63, 45, 9, 6 };
	uint32_t subcount = uint8_t(m_lfo_counter++);

	// when we cross the divider count, add enough to zero it and cause an
	// increment at bit 8; the 7-bit value lives from bits 8-14
	if (subcount >= lfo_max_count[lfo_rate()])
	{
		// note: to match the published values this should be 0x100 - subcount;
		// however, tests on the hardware and nuked bear out an off-by-one
		// error exists that causes the max LFO rate to be faster than published
		m_lfo_counter += 0x101 - subcount;
	}

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

template<bool IsOpnA>
uint32_t opn_registers_base<IsOpnA>::lfo_am_offset(uint32_t choffs) const
{
	// shift value for AM sensitivity is [7, 3, 1, 0],
	// mapping to values of [0, 1.4, 5.9, and 11.8dB]
	uint32_t am_shift = (1 << (ch_lfo_am_sens(choffs) ^ 3)) - 1;

	// QUESTION: max sensitivity should give 11.8dB range, but this value
	// is directly added to an x.8 attenuation value, which will only give
	// 126/256 or ~4.9dB range -- what am I missing? The calculation below
	// matches several other emulators, including the Nuked implemenation.

	// raw LFO AM value on OPN is 0-3F, scale that up by a factor of 2
	// (giving 7 bits) before applying the final shift
	return (m_lfo_am << 1) >> am_shift;
}


//-------------------------------------------------
//  cache_operator_data - fill the operator cache
//  with prefetched data
//-------------------------------------------------

template<bool IsOpnA>
void opn_registers_base<IsOpnA>::cache_operator_data(uint32_t choffs, uint32_t opoffs, opdata_cache &cache)
{
	// set up the easy stuff
	cache.waveform = &m_waveform[0][0];

	// get frequency from the channel
	uint32_t block_freq = cache.block_freq = ch_block_freq(choffs);

	// if multi-frequency mode is enabled and this is channel 2,
	// fetch one of the special frequencies
	if (multi_freq() && choffs == 2)
	{
		if (opoffs == 2)
			block_freq = cache.block_freq = multi_block_freq(1);
		else if (opoffs == 10)
			block_freq = cache.block_freq = multi_block_freq(2);
		else if (opoffs == 6)
			block_freq = cache.block_freq = multi_block_freq(0);
	}

	// compute the keycode: block_freq is:
	//
	//     BBBFFFFFFFFFFF
	//     ^^^^???
	//
	// the 5-bit keycode uses the top 4 bits plus a magic formula
	// for the final bit
	uint32_t keycode = bitfield(block_freq, 10, 4) << 1;

	// lowest bit is determined by a mix of next lower FNUM bits
	// according to this equation from the YM2608 manual:
	//
	//   (F11 & (F10 | F9 | F8)) | (!F11 & F10 & F9 & F8)
	//
	// for speed, we just look it up in a 16-bit constant
	keycode |= bitfield(0xfe80, bitfield(block_freq, 7, 4));

	// detune adjustment
	cache.detune = detune_adjustment(op_detune(opoffs), keycode);

	// multiple value, as an x.1 value (0 means 0.5)
	cache.multiple = op_multiple(opoffs) * 2;
	if (cache.multiple == 0)
		cache.multiple = 1;

	// phase step, or PHASE_STEP_DYNAMIC if PM is active; this depends on
	// block_freq, detune, and multiple, so compute it after we've done those
	if (!IsOpnA || lfo_enable() == 0 || ch_lfo_pm_sens(choffs) == 0)
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

template<bool IsOpnA>
uint32_t opn_registers_base<IsOpnA>::compute_phase_step(uint32_t choffs, uint32_t opoffs, opdata_cache const &cache, int32_t lfo_raw_pm)
{
	// OPN phase calculation has only a single detune parameter
	// and uses FNUMs instead of keycodes

	// extract frequency number (low 11 bits of block_freq)
	uint32_t fnum = bitfield(cache.block_freq, 0, 11) << 1;

	// if there's a non-zero PM sensitivity, compute the adjustment
	uint32_t pm_sensitivity = ch_lfo_pm_sens(choffs);
	if (pm_sensitivity != 0)
	{
		// apply the phase adjustment based on the upper 7 bits
		// of FNUM and the PM depth parameters
		fnum += opn_lfo_pm_phase_adjustment(bitfield(cache.block_freq, 4, 7), pm_sensitivity, lfo_raw_pm);

		// keep fnum to 12 bits
		fnum &= 0xfff;
	}

	// apply block shift to compute phase step
	uint32_t block = bitfield(cache.block_freq, 11, 3);
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

template<bool IsOpnA>
std::string opn_registers_base<IsOpnA>::log_keyon(uint32_t choffs, uint32_t opoffs)
{
	uint32_t chnum = (choffs & 3) + 3 * bitfield(choffs, 8);
	uint32_t opnum = (opoffs & 15) - ((opoffs & 15) / 4) + 12 * bitfield(opoffs, 8);

	uint32_t block_freq = ch_block_freq(choffs);
	if (multi_freq() && choffs == 2)
	{
		if (opoffs == 2)
			block_freq = multi_block_freq(1);
		else if (opoffs == 10)
			block_freq = multi_block_freq(2);
		else if (opoffs == 6)
			block_freq = multi_block_freq(0);
	}

	char buffer[256];
	int end = 0;

	end += snprintf(&buffer[end], sizeof(buffer) - end, "%u.%02u freq=%04X dt=%u fb=%u alg=%X mul=%X tl=%02X ksr=%u adsr=%02X/%02X/%02X/%X sl=%X",
		chnum, opnum,
		block_freq,
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
		op_sustain_level(opoffs));

	if (OUTPUTS > 1)
		end += snprintf(&buffer[end], sizeof(buffer) - end, " out=%c%c",
			ch_output_0(choffs) ? 'L' : '-',
			ch_output_1(choffs) ? 'R' : '-');
	if (op_ssg_eg_enable(opoffs))
		end += snprintf(&buffer[end], sizeof(buffer) - end, " ssg=%X", op_ssg_eg_mode(opoffs));
	bool am = (op_lfo_am_enable(opoffs) && ch_lfo_am_sens(choffs) != 0);
	if (am)
		end += snprintf(&buffer[end], sizeof(buffer) - end, " am=%u", ch_lfo_am_sens(choffs));
	bool pm = (ch_lfo_pm_sens(choffs) != 0);
	if (pm)
		end += snprintf(&buffer[end], sizeof(buffer) - end, " pm=%u", ch_lfo_pm_sens(choffs));
	if (am || pm)
		end += snprintf(&buffer[end], sizeof(buffer) - end, " lfo=%02X", lfo_rate());
	if (multi_freq() && choffs == 2)
		end += snprintf(&buffer[end], sizeof(buffer) - end, " multi=1");

	return buffer;
}



//*********************************************************
//  SSG RESAMPLER
//*********************************************************

//-------------------------------------------------
//  add_last - helper to add the last computed
//  value to the sums, applying the given scale
//-------------------------------------------------

template<typename OutputType, int FirstOutput, bool MixTo1>
void ssg_resampler<OutputType, FirstOutput, MixTo1>::add_last(int32_t &sum0, int32_t &sum1, int32_t &sum2, int32_t scale)
{
	sum0 += m_last.data[0] * scale;
	sum1 += m_last.data[1] * scale;
	sum2 += m_last.data[2] * scale;
}


//-------------------------------------------------
//  clock_and_add - helper to clock a new value
//  and then add it to the sums, applying the
//  given scale
//-------------------------------------------------

template<typename OutputType, int FirstOutput, bool MixTo1>
void ssg_resampler<OutputType, FirstOutput, MixTo1>::clock_and_add(int32_t &sum0, int32_t &sum1, int32_t &sum2, int32_t scale)
{
	m_ssg.clock();
	m_ssg.output(m_last);
	add_last(sum0, sum1, sum2, scale);
}


//-------------------------------------------------
//  write_to_output - helper to write the sums to
//  the appropriate outputs, applying the given
//  divisor to the final result
//-------------------------------------------------

template<typename OutputType, int FirstOutput, bool MixTo1>
void ssg_resampler<OutputType, FirstOutput, MixTo1>::write_to_output(OutputType *output, int32_t sum0, int32_t sum1, int32_t sum2, int32_t divisor)
{
	if (MixTo1)
	{
		// mixing to one, apply a 2/3 factor to prevent overflow
		output->data[FirstOutput] = (sum0 + sum1 + sum2) * 2 / (3 * divisor);
	}
	else
	{
		// write three outputs in a row
		output->data[FirstOutput + 0] = sum0 / divisor;
		output->data[FirstOutput + 1] = sum1 / divisor;
		output->data[FirstOutput + 2] = sum2 / divisor;
	}

	// track the sample index here
	m_sampindex++;
}


//-------------------------------------------------
//  ssg_resampler - constructor
//-------------------------------------------------

template<typename OutputType, int FirstOutput, bool MixTo1>
ssg_resampler<OutputType, FirstOutput, MixTo1>::ssg_resampler(ssg_engine &ssg) :
	m_ssg(ssg),
	m_sampindex(0),
	m_resampler(&ssg_resampler::resample_nop)
{
	m_last.clear();
}


//-------------------------------------------------
//  save_restore - save or restore the data
//-------------------------------------------------

template<typename OutputType, int FirstOutput, bool MixTo1>
void ssg_resampler<OutputType, FirstOutput, MixTo1>::save_restore(ymfm_saved_state &state)
{
	state.save_restore(m_sampindex);
	state.save_restore(m_last.data);
}


//-------------------------------------------------
//  configure - configure a new ratio
//-------------------------------------------------

template<typename OutputType, int FirstOutput, bool MixTo1>
void ssg_resampler<OutputType, FirstOutput, MixTo1>::configure(uint8_t outsamples, uint8_t srcsamples)
{
	switch (outsamples * 10 + srcsamples)
	{
		case 4*10 + 1:	/* 4:1 */	m_resampler = &ssg_resampler::resample_n_1<4>;	break;
		case 2*10 + 1:	/* 2:1 */	m_resampler = &ssg_resampler::resample_n_1<2>;	break;
		case 4*10 + 3:	/* 4:3 */	m_resampler = &ssg_resampler::resample_4_3;		break;
		case 1*10 + 1:	/* 1:1 */	m_resampler = &ssg_resampler::resample_n_1<1>;	break;
		case 2*10 + 3:	/* 2:3 */	m_resampler = &ssg_resampler::resample_2_3;		break;
		case 1*10 + 3:	/* 1:3 */	m_resampler = &ssg_resampler::resample_1_n<3>;	break;
		case 2*10 + 9:	/* 2:9 */	m_resampler = &ssg_resampler::resample_2_9;		break;
		case 1*10 + 6:	/* 1:6 */	m_resampler = &ssg_resampler::resample_1_n<6>;	break;
		case 0*10 + 0:	/* 0:0 */	m_resampler = &ssg_resampler::resample_nop;		break;
		default: assert(false); break;
	}
}


//-------------------------------------------------
//  resample_n_1 - resample SSG output to the
//  target at a rate of 1 SSG sample to every
//  n output sample
//-------------------------------------------------

template<typename OutputType, int FirstOutput, bool MixTo1>
template<int Multiplier>
void ssg_resampler<OutputType, FirstOutput, MixTo1>::resample_n_1(OutputType *output, uint32_t numsamples)
{
	for (uint32_t samp = 0; samp < numsamples; samp++, output++)
	{
		if (m_sampindex % Multiplier == 0)
		{
			m_ssg.clock();
			m_ssg.output(m_last);
		}
		write_to_output(output, m_last.data[0], m_last.data[1], m_last.data[2]);
	}
}


//-------------------------------------------------
//  resample_1_n - resample SSG output to the
//  target at a rate of n SSG samples to every
//  1 output sample
//-------------------------------------------------

template<typename OutputType, int FirstOutput, bool MixTo1>
template<int Divisor>
void ssg_resampler<OutputType, FirstOutput, MixTo1>::resample_1_n(OutputType *output, uint32_t numsamples)
{
	for (uint32_t samp = 0; samp < numsamples; samp++, output++)
	{
		int32_t sum0 = 0, sum1 = 0, sum2 = 0;
		for (int rep = 0; rep < Divisor; rep++)
			clock_and_add(sum0, sum1, sum2);
		write_to_output(output, sum0, sum1, sum2, Divisor);
	}
}


//-------------------------------------------------
//  resample_2_9 - resample SSG output to the
//  target at a rate of 9 SSG samples to every
//  2 output samples
//-------------------------------------------------

template<typename OutputType, int FirstOutput, bool MixTo1>
void ssg_resampler<OutputType, FirstOutput, MixTo1>::resample_2_9(OutputType *output, uint32_t numsamples)
{
	for (uint32_t samp = 0; samp < numsamples; samp++, output++)
	{
		int32_t sum0 = 0, sum1 = 0, sum2 = 0;
		if (bitfield(m_sampindex, 0) != 0)
			add_last(sum0, sum1, sum2, 1);
		clock_and_add(sum0, sum1, sum2, 2);
		clock_and_add(sum0, sum1, sum2, 2);
		clock_and_add(sum0, sum1, sum2, 2);
		clock_and_add(sum0, sum1, sum2, 2);
		if (bitfield(m_sampindex, 0) == 0)
			clock_and_add(sum0, sum1, sum2, 1);
		write_to_output(output, sum0, sum1, sum2, 9);
	}
}


//-------------------------------------------------
//  resample_2_3 - resample SSG output to the
//  target at a rate of 3 SSG samples to every
//  2 output samples
//-------------------------------------------------

template<typename OutputType, int FirstOutput, bool MixTo1>
void ssg_resampler<OutputType, FirstOutput, MixTo1>::resample_2_3(OutputType *output, uint32_t numsamples)
{
	for (uint32_t samp = 0; samp < numsamples; samp++, output++)
	{
		int32_t sum0 = 0, sum1 = 0, sum2 = 0;
		if (bitfield(m_sampindex, 0) == 0)
		{
			clock_and_add(sum0, sum1, sum2, 2);
			clock_and_add(sum0, sum1, sum2, 1);
		}
		else
		{
			add_last(sum0, sum1, sum2, 1);
			clock_and_add(sum0, sum1, sum2, 2);
		}
		write_to_output(output, sum0, sum1, sum2, 3);
	}
}


//-------------------------------------------------
//  resample_4_3 - resample SSG output to the
//  target at a rate of 3 SSG samples to every
//  4 output samples
//-------------------------------------------------

template<typename OutputType, int FirstOutput, bool MixTo1>
void ssg_resampler<OutputType, FirstOutput, MixTo1>::resample_4_3(OutputType *output, uint32_t numsamples)
{
	for (uint32_t samp = 0; samp < numsamples; samp++, output++)
	{
		int32_t sum0 = 0, sum1 = 0, sum2 = 0;
		int32_t step = bitfield(m_sampindex, 0, 2);
		add_last(sum0, sum1, sum2, step);
		if (step != 3)
			clock_and_add(sum0, sum1, sum2, 3 - step);
		write_to_output(output, sum0, sum1, sum2, 3);
	}
}


//-------------------------------------------------
//  resample_nop - no-op resampler
//-------------------------------------------------

template<typename OutputType, int FirstOutput, bool MixTo1>
void ssg_resampler<OutputType, FirstOutput, MixTo1>::resample_nop(OutputType *output, uint32_t numsamples)
{
	// nothing to do except increment the sample index
	m_sampindex += numsamples;
}



//*********************************************************
//  YM2203
//*********************************************************

//-------------------------------------------------
//  ym2203 - constructor
//-------------------------------------------------

ym2203::ym2203(ymfm_interface &intf) :
	m_fidelity(OPN_FIDELITY_MAX),
	m_address(0),
	m_fm(intf),
	m_ssg(intf),
	m_ssg_resampler(m_ssg)
{
	m_last_fm.clear();
	update_prescale(m_fm.clock_prescale());
}


//-------------------------------------------------
//  reset - reset the system
//-------------------------------------------------

void ym2203::reset()
{
	// reset the engines
	m_fm.reset();
	m_ssg.reset();
}


//-------------------------------------------------
//  save_restore - save or restore the data
//-------------------------------------------------

void ym2203::save_restore(ymfm_saved_state &state)
{
	state.save_restore(m_address);
	state.save_restore(m_last_fm.data);

	m_fm.save_restore(state);
	m_ssg.save_restore(state);
	m_ssg_resampler.save_restore(state);

	update_prescale(m_fm.clock_prescale());
}


//-------------------------------------------------
//  read_status - read the status register
//-------------------------------------------------

uint8_t ym2203::read_status()
{
	uint8_t result = m_fm.status();
	if (m_fm.intf().ymfm_is_busy())
		result |= fm_engine::STATUS_BUSY;
	return result;
}


//-------------------------------------------------
//  read_data - read the data register
//-------------------------------------------------

uint8_t ym2203::read_data()
{
	uint8_t result = 0;
	if (m_address < 0x10)
	{
		// 00-0F: Read from SSG
		result = m_ssg.read(m_address & 0x0f);
	}
	return result;
}


//-------------------------------------------------
//  read - handle a read from the device
//-------------------------------------------------

uint8_t ym2203::read(uint32_t offset)
{
	uint8_t result = 0xff;
	switch (offset & 1)
	{
		case 0: // status port
			result = read_status();
			break;

		case 1: // data port (only SSG)
			result = read_data();
			break;
	}
	return result;
}


//-------------------------------------------------
//  write_address - handle a write to the address
//  register
//-------------------------------------------------

void ym2203::write_address(uint8_t data)
{
	// just set the address
	m_address = data;

	// special case: update the prescale
	if (m_address >= 0x2d && m_address <= 0x2f)
	{
		// 2D-2F: prescaler select
		if (m_address == 0x2d)
			update_prescale(6);
		else if (m_address == 0x2e && m_fm.clock_prescale() == 6)
			update_prescale(3);
		else if (m_address == 0x2f)
			update_prescale(2);
	}
}


//-------------------------------------------------
//  write - handle a write to the register
//  interface
//-------------------------------------------------

void ym2203::write_data(uint8_t data)
{
	if (m_address < 0x10)
	{
		// 00-0F: write to SSG
		m_ssg.write(m_address & 0x0f, data);
	}
	else
	{
		// 10-FF: write to FM
		m_fm.write(m_address, data);
	}

	// mark busy for a bit
	m_fm.intf().ymfm_set_busy_end(32 * m_fm.clock_prescale());
}


//-------------------------------------------------
//  write - handle a write to the register
//  interface
//-------------------------------------------------

void ym2203::write(uint32_t offset, uint8_t data)
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

void ym2203::generate(output_data *output, uint32_t numsamples)
{
	// FM output is just repeated the prescale number of times; note that
	// 0 is a special 1.5 case
	if (m_fm_samples_per_output != 0)
	{
		for (uint32_t samp = 0; samp < numsamples; samp++, output++)
		{
			if ((m_ssg_resampler.sampindex() + samp) % m_fm_samples_per_output == 0)
				clock_fm();
			output->data[0] = m_last_fm.data[0];
		}
	}
	else
	{
		for (uint32_t samp = 0; samp < numsamples; samp++, output++)
		{
			uint32_t step = (m_ssg_resampler.sampindex() + samp) % 3;
			if (step == 0)
				clock_fm();
			output->data[0] = m_last_fm.data[0];
			if (step == 1)
			{
				clock_fm();
				output->data[0] = (output->data[0] + m_last_fm.data[0]) / 2;
			}
		}
	}

	// resample the SSG as configured
	m_ssg_resampler.resample(output - numsamples, numsamples);
}


//-------------------------------------------------
//  update_prescale - update the prescale value,
//  recomputing derived values
//-------------------------------------------------

void ym2203::update_prescale(uint8_t prescale)
{
	// tell the FM engine
	m_fm.set_clock_prescale(prescale);
	m_ssg.prescale_changed();

	// Fidelity:   ---- minimum ----    ---- medium -----    ---- maximum-----
	//              rate = clock/24      rate = clock/12      rate = clock/4
	// Prescale    FM rate  SSG rate    FM rate  SSG rate    FM rate  SSG rate
	//     6          3:1     2:3          6:1     4:3         18:1     4:1
	//     3        1.5:1     1:3          3:1     2:3          9:1     2:1
	//     2          1:1     1:6          2:1     1:3          6:1     1:1

	// compute the number of FM samples per output sample, and select the
	// resampler function
	if (m_fidelity == OPN_FIDELITY_MIN)
	{
		switch (prescale)
		{
			default:
			case 6:	m_fm_samples_per_output = 3;	m_ssg_resampler.configure(2, 3);	break;
			case 3: m_fm_samples_per_output = 0;	m_ssg_resampler.configure(1, 3);	break;
			case 2: m_fm_samples_per_output = 1;	m_ssg_resampler.configure(1, 6);	break;
		}
	}
	else if (m_fidelity == OPN_FIDELITY_MED)
	{
		switch (prescale)
		{
			default:
			case 6:	m_fm_samples_per_output = 6;	m_ssg_resampler.configure(4, 3);	break;
			case 3: m_fm_samples_per_output = 3;	m_ssg_resampler.configure(2, 3);	break;
			case 2: m_fm_samples_per_output = 2;	m_ssg_resampler.configure(1, 3);	break;
		}
	}
	else
	{
		switch (prescale)
		{
			default:
			case 6:	m_fm_samples_per_output = 18;	m_ssg_resampler.configure(4, 1);	break;
			case 3: m_fm_samples_per_output = 9;	m_ssg_resampler.configure(2, 1);	break;
			case 2: m_fm_samples_per_output = 6;	m_ssg_resampler.configure(1, 1);	break;
		}
	}

	// if overriding the SSG, override the configuration with the nop
	// resampler to at least keep the sample index moving forward
	if (m_ssg.overridden())
		m_ssg_resampler.configure(0, 0);
}


//-------------------------------------------------
//  clock_fm - clock FM state
//-------------------------------------------------

void ym2203::clock_fm()
{
	// clock the system
	m_fm.clock(fm_engine::ALL_CHANNELS);

	// update the FM content; OPN is full 14-bit with no intermediate clipping
	m_fm.output(m_last_fm.clear(), 0, 32767, fm_engine::ALL_CHANNELS);

	// convert to 10.3 floating point value for the DAC and back
	m_last_fm.roundtrip_fp();
}



//*********************************************************
//  YM2608
//*********************************************************

//-------------------------------------------------
//  ym2608 - constructor
//-------------------------------------------------

ym2608::ym2608(ymfm_interface &intf) :
	m_fidelity(OPN_FIDELITY_MAX),
	m_address(0),
	m_irq_enable(0x1f),
	m_flag_control(0x1c),
	m_fm(intf),
	m_ssg(intf),
	m_ssg_resampler(m_ssg),
	m_adpcm_a(intf, 0),
	m_adpcm_b(intf)
{
	m_last_fm.clear();
	update_prescale(m_fm.clock_prescale());
}


//-------------------------------------------------
//  reset - reset the system
//-------------------------------------------------

void ym2608::reset()
{
	// reset the engines
	m_fm.reset();
	m_ssg.reset();
	m_adpcm_a.reset();
	m_adpcm_b.reset();

	// configure ADPCM percussion sounds; these are present in an embedded ROM
	m_adpcm_a.set_start_end(0, 0x0000, 0x01bf); // bass drum
	m_adpcm_a.set_start_end(1, 0x01c0, 0x043f); // snare drum
	m_adpcm_a.set_start_end(2, 0x0440, 0x1b7f); // top cymbal
	m_adpcm_a.set_start_end(3, 0x1b80, 0x1cff); // high hat
	m_adpcm_a.set_start_end(4, 0x1d00, 0x1f7f); // tom tom
	m_adpcm_a.set_start_end(5, 0x1f80, 0x1fff); // rim shot

	// initialize our special interrupt states, then read the upper status
	// register, which updates the IRQs
	m_irq_enable = 0x1f;
	m_flag_control = 0x1c;
	read_status_hi();
}


//-------------------------------------------------
//  save_restore - save or restore the data
//-------------------------------------------------

void ym2608::save_restore(ymfm_saved_state &state)
{
	state.save_restore(m_address);
	state.save_restore(m_irq_enable);
	state.save_restore(m_flag_control);
	state.save_restore(m_last_fm.data);

	m_fm.save_restore(state);
	m_ssg.save_restore(state);
	m_ssg_resampler.save_restore(state);
	m_adpcm_a.save_restore(state);
	m_adpcm_b.save_restore(state);
}


//-------------------------------------------------
//  read_status - read the status register
//-------------------------------------------------

uint8_t ym2608::read_status()
{
	uint8_t result = m_fm.status() & (fm_engine::STATUS_TIMERA | fm_engine::STATUS_TIMERB);
	if (m_fm.intf().ymfm_is_busy())
		result |= fm_engine::STATUS_BUSY;
	return result;
}


//-------------------------------------------------
//  read_data - read the data register
//-------------------------------------------------

uint8_t ym2608::read_data()
{
	uint8_t result = 0;
	if (m_address < 0x10)
	{
		// 00-0F: Read from SSG
		result = m_ssg.read(m_address & 0x0f);
	}
	else if (m_address == 0xff)
	{
		// FF: ID code
		result = 1;
	}
	return result;
}


//-------------------------------------------------
//  read_status_hi - read the extended status
//  register
//-------------------------------------------------

uint8_t ym2608::read_status_hi()
{
	// fetch regular status
	uint8_t status = m_fm.status() & ~(STATUS_ADPCM_B_EOS | STATUS_ADPCM_B_BRDY | STATUS_ADPCM_B_PLAYING);

	// fetch ADPCM-B status, and merge in the bits
	uint8_t adpcm_status = m_adpcm_b.status();
	if ((adpcm_status & adpcm_b_channel::STATUS_EOS) != 0)
		status |= STATUS_ADPCM_B_EOS;
	if ((adpcm_status & adpcm_b_channel::STATUS_BRDY) != 0)
		status |= STATUS_ADPCM_B_BRDY;
	if ((adpcm_status & adpcm_b_channel::STATUS_PLAYING) != 0)
		status |= STATUS_ADPCM_B_PLAYING;

	// turn off any bits that have been requested to be masked
	status &= ~(m_flag_control & 0x1f);

	// update the status so that IRQs are propagated
	m_fm.set_reset_status(status, ~status);

	// merge in the busy flag
	if (m_fm.intf().ymfm_is_busy())
		status |= fm_engine::STATUS_BUSY;
	return status;
}


//-------------------------------------------------
//  read_data_hi - read the upper data register
//-------------------------------------------------

uint8_t ym2608::read_data_hi()
{
	uint8_t result = 0;
	if ((m_address & 0xff) < 0x10)
	{
		// 00-0F: Read from ADPCM-B
		result = m_adpcm_b.read(m_address & 0x0f);
	}
	return result;
}


//-------------------------------------------------
//  read - handle a read from the device
//-------------------------------------------------

uint8_t ym2608::read(uint32_t offset)
{
	uint8_t result = 0;
	switch (offset & 3)
	{
		case 0: // status port, YM2203 compatible
			result = read_status();
			break;

		case 1: // data port (only SSG)
			result = read_data();
			break;

		case 2: // status port, extended
			result = read_status_hi();
			break;

		case 3: // ADPCM-B data
			result = read_data_hi();
			break;
	}
	return result;
}


//-------------------------------------------------
//  write_address - handle a write to the address
//  register
//-------------------------------------------------

void ym2608::write_address(uint8_t data)
{
	// just set the address
	m_address = data;

	// special case: update the prescale
	if (m_address >= 0x2d && m_address <= 0x2f)
	{
		// 2D-2F: prescaler select
		if (m_address == 0x2d)
			update_prescale(6);
		else if (m_address == 0x2e && m_fm.clock_prescale() == 6)
			update_prescale(3);
		else if (m_address == 0x2f)
			update_prescale(2);
	}
}


//-------------------------------------------------
//  write - handle a write to the data register
//-------------------------------------------------

void ym2608::write_data(uint8_t data)
{
	// ignore if paired with upper address
	if (bitfield(m_address, 8))
		return;

	if (m_address < 0x10)
	{
		// 00-0F: write to SSG
		m_ssg.write(m_address & 0x0f, data);
	}
	else if (m_address < 0x20)
	{
		// 10-1F: write to ADPCM-A
		m_adpcm_a.write(m_address & 0x0f, data);
	}
	else if (m_address == 0x29)
	{
		// 29: special IRQ mask register
		m_irq_enable = data;
		m_fm.set_irq_mask(m_irq_enable & ~m_flag_control & 0x1f);
	}
	else
	{
		// 20-28, 2A-FF: write to FM
		m_fm.write(m_address, data);
	}

	// mark busy for a bit
	m_fm.intf().ymfm_set_busy_end(32 * m_fm.clock_prescale());
}


//-------------------------------------------------
//  write_address_hi - handle a write to the upper
//  address register
//-------------------------------------------------

void ym2608::write_address_hi(uint8_t data)
{
	// just set the address
	m_address = 0x100 | data;
}


//-------------------------------------------------
//  write_data_hi - handle a write to the upper
//  data register
//-------------------------------------------------

void ym2608::write_data_hi(uint8_t data)
{
	// ignore if paired with upper address
	if (!bitfield(m_address, 8))
		return;

	if (m_address < 0x110)
	{
		// 100-10F: write to ADPCM-B
		m_adpcm_b.write(m_address & 0x0f, data);
	}
	else if (m_address == 0x110)
	{
		// 110: IRQ flag control
		if (bitfield(data, 7))
			m_fm.set_reset_status(0, 0xff);
		else
		{
			m_flag_control = data;
			m_fm.set_irq_mask(m_irq_enable & ~m_flag_control & 0x1f);
		}
	}
	else
	{
		// 111-1FF: write to FM
		m_fm.write(m_address, data);
	}

	// mark busy for a bit
	m_fm.intf().ymfm_set_busy_end(32 * m_fm.clock_prescale());
}


//-------------------------------------------------
//  write - handle a write to the register
//  interface
//-------------------------------------------------

void ym2608::write(uint32_t offset, uint8_t data)
{
	switch (offset & 3)
	{
		case 0: // address port
			write_address(data);
			break;

		case 1: // data port
			write_data(data);
			break;

		case 2: // upper address port
			write_address_hi(data);
			break;

		case 3: // upper data port
			write_data_hi(data);
			break;
	}
}


//-------------------------------------------------
//  generate - generate one sample of sound
//-------------------------------------------------

void ym2608::generate(output_data *output, uint32_t numsamples)
{
	// FM output is just repeated the prescale number of times; note that
	// 0 is a special 1.5 case
	if (m_fm_samples_per_output != 0)
	{
		for (uint32_t samp = 0; samp < numsamples; samp++, output++)
		{
			if ((m_ssg_resampler.sampindex() + samp) % m_fm_samples_per_output == 0)
				clock_fm_and_adpcm();
			output->data[0] = m_last_fm.data[0];
			output->data[1] = m_last_fm.data[1];
		}
	}
	else
	{
		for (uint32_t samp = 0; samp < numsamples; samp++, output++)
		{
			uint32_t step = (m_ssg_resampler.sampindex() + samp) % 3;
			if (step == 0)
				clock_fm_and_adpcm();
			output->data[0] = m_last_fm.data[0];
			output->data[1] = m_last_fm.data[1];
			if (step == 1)
			{
				clock_fm_and_adpcm();
				output->data[0] = (output->data[0] + m_last_fm.data[0]) / 2;
				output->data[1] = (output->data[1] + m_last_fm.data[1]) / 2;
			}
		}
	}

	// resample the SSG as configured
	m_ssg_resampler.resample(output - numsamples, numsamples);
}


//-------------------------------------------------
//  update_prescale - update the prescale value,
//  recomputing derived values
//-------------------------------------------------

void ym2608::update_prescale(uint8_t prescale)
{
	// tell the FM engine
	m_fm.set_clock_prescale(prescale);
	m_ssg.prescale_changed();

	// Fidelity:   ---- minimum ----    ---- medium -----    ---- maximum-----
	//              rate = clock/48      rate = clock/24      rate = clock/8
	// Prescale    FM rate  SSG rate    FM rate  SSG rate    FM rate  SSG rate
	//     6          3:1     2:3          6:1     4:3         18:1     4:1
	//     3        1.5:1     1:3          3:1     2:3          9:1     2:1
	//     2          1:1     1:6          2:1     1:3          6:1     1:1

	// compute the number of FM samples per output sample, and select the
	// resampler function
	if (m_fidelity == OPN_FIDELITY_MIN)
	{
		switch (prescale)
		{
			default:
			case 6:	m_fm_samples_per_output = 3;	m_ssg_resampler.configure(2, 3);	break;
			case 3: m_fm_samples_per_output = 0;	m_ssg_resampler.configure(1, 3);	break;
			case 2: m_fm_samples_per_output = 1;	m_ssg_resampler.configure(1, 6);	break;
		}
	}
	else if (m_fidelity == OPN_FIDELITY_MED)
	{
		switch (prescale)
		{
			default:
			case 6:	m_fm_samples_per_output = 6;	m_ssg_resampler.configure(4, 3);	break;
			case 3: m_fm_samples_per_output = 3;	m_ssg_resampler.configure(2, 3);	break;
			case 2: m_fm_samples_per_output = 2;	m_ssg_resampler.configure(1, 3);	break;
		}
	}
	else
	{
		switch (prescale)
		{
			default:
			case 6:	m_fm_samples_per_output = 18;	m_ssg_resampler.configure(4, 1);	break;
			case 3: m_fm_samples_per_output = 9;	m_ssg_resampler.configure(2, 1);	break;
			case 2: m_fm_samples_per_output = 6;	m_ssg_resampler.configure(1, 1);	break;
		}
	}

	// if overriding the SSG, override the configuration with the nop
	// resampler to at least keep the sample index moving forward
	if (m_ssg.overridden())
		m_ssg_resampler.configure(0, 0);
}


//-------------------------------------------------
//  clock_fm_and_adpcm - clock FM and ADPCM state
//-------------------------------------------------

void ym2608::clock_fm_and_adpcm()
{
	// top bit of the IRQ enable flags controls 3-channel vs 6-channel mode
	uint32_t fmmask = bitfield(m_irq_enable, 7) ? 0x3f : 0x07;

	// clock the system
	uint32_t env_counter = m_fm.clock(fm_engine::ALL_CHANNELS);

	// clock the ADPCM-A engine on every envelope cycle
	// (channels 4 and 5 clock every 2 envelope clocks)
	if (bitfield(env_counter, 0, 2) == 0)
		m_adpcm_a.clock(bitfield(env_counter, 2) ? 0x0f : 0x3f);

	// clock the ADPCM-B engine every cycle
	m_adpcm_b.clock();

	// update the FM content; OPNA is 13-bit with no intermediate clipping
	m_fm.output(m_last_fm.clear(), 1, 32767, fmmask);

	// mix in the ADPCM and clamp
	m_adpcm_a.output(m_last_fm, 0x3f);
	m_adpcm_b.output(m_last_fm, 1);
	m_last_fm.clamp16();
}


//*********************************************************
//  YMF288
//*********************************************************

// YMF288 is a YM2608 with the following changes:
//   * ADPCM-B part removed
//   * prescaler removed (fixed at 6)
//   * CSM removed
//   * Low power mode added
//   * SSG tone frequency is altered in some way? (explicitly DC for Tp 0-7, also double volume in some cases)
//   * I/O ports removed
//   * Shorter busy times
//   * All registers can be read

//-------------------------------------------------
//  ymf288 - constructor
//-------------------------------------------------

ymf288::ymf288(ymfm_interface &intf) :
	m_fidelity(OPN_FIDELITY_MAX),
	m_address(0),
	m_irq_enable(0x03),
	m_flag_control(0x03),
	m_fm(intf),
	m_ssg(intf),
	m_ssg_resampler(m_ssg),
	m_adpcm_a(intf, 0)
{
	m_last_fm.clear();
	update_prescale();
}


//-------------------------------------------------
//  reset - reset the system
//-------------------------------------------------

void ymf288::reset()
{
	// reset the engines
	m_fm.reset();
	m_ssg.reset();
	m_adpcm_a.reset();

	// configure ADPCM percussion sounds; these are present in an embedded ROM
	m_adpcm_a.set_start_end(0, 0x0000, 0x01bf); // bass drum
	m_adpcm_a.set_start_end(1, 0x01c0, 0x043f); // snare drum
	m_adpcm_a.set_start_end(2, 0x0440, 0x1b7f); // top cymbal
	m_adpcm_a.set_start_end(3, 0x1b80, 0x1cff); // high hat
	m_adpcm_a.set_start_end(4, 0x1d00, 0x1f7f); // tom tom
	m_adpcm_a.set_start_end(5, 0x1f80, 0x1fff); // rim shot

	// initialize our special interrupt states, then read the upper status
	// register, which updates the IRQs
	m_irq_enable = 0x03;
	m_flag_control = 0x00;
	read_status_hi();
}


//-------------------------------------------------
//  save_restore - save or restore the data
//-------------------------------------------------

void ymf288::save_restore(ymfm_saved_state &state)
{
	state.save_restore(m_address);
	state.save_restore(m_irq_enable);
	state.save_restore(m_flag_control);
	state.save_restore(m_last_fm.data);

	m_fm.save_restore(state);
	m_ssg.save_restore(state);
	m_ssg_resampler.save_restore(state);
	m_adpcm_a.save_restore(state);
}


//-------------------------------------------------
//  read_status - read the status register
//-------------------------------------------------

uint8_t ymf288::read_status()
{
	uint8_t result = m_fm.status() & (fm_engine::STATUS_TIMERA | fm_engine::STATUS_TIMERB);
	if (m_fm.intf().ymfm_is_busy())
		result |= fm_engine::STATUS_BUSY;
	return result;
}


//-------------------------------------------------
//  read_data - read the data register
//-------------------------------------------------

uint8_t ymf288::read_data()
{
	uint8_t result = 0;
	if (m_address < 0x0e)
	{
		// 00-0D: Read from SSG
		result = m_ssg.read(m_address & 0x0f);
	}
	else if (m_address < 0x10)
	{
		// 0E-0F: I/O ports not supported
		result = 0xff;
	}
	else if (m_address == 0xff)
	{
		// FF: ID code
		result = 2;
	}
	else if (ymf288_mode())
	{
		// registers are readable in YMF288 mode
		result = m_fm.regs().read(m_address);
	}
	return result;
}


//-------------------------------------------------
//  read_status_hi - read the extended status
//  register
//-------------------------------------------------

uint8_t ymf288::read_status_hi()
{
	// fetch regular status
	uint8_t status = m_fm.status() & (fm_engine::STATUS_TIMERA | fm_engine::STATUS_TIMERB);

	// turn off any bits that have been requested to be masked
	status &= ~(m_flag_control & 0x03);

	// update the status so that IRQs are propagated
	m_fm.set_reset_status(status, ~status);

	// merge in the busy flag
	if (m_fm.intf().ymfm_is_busy())
		status |= fm_engine::STATUS_BUSY;
	return status;
}


//-------------------------------------------------
//  read - handle a read from the device
//-------------------------------------------------

uint8_t ymf288::read(uint32_t offset)
{
	uint8_t result = 0;
	switch (offset & 3)
	{
		case 0: // status port, YM2203 compatible
			result = read_status();
			break;

		case 1: // data port
			result = read_data();
			break;

		case 2: // status port, extended
			result = read_status_hi();
			break;

		case 3: // unmapped
			debug::log_unexpected_read_write("Unexpected read from YMF288 offset %d\n", offset & 3);
			break;
	}
	return result;
}


//-------------------------------------------------
//  write_address - handle a write to the address
//  register
//-------------------------------------------------

void ymf288::write_address(uint8_t data)
{
	// just set the address
	m_address = data;

	// in YMF288 mode, busy is signaled after address writes too
	if (ymf288_mode())
		m_fm.intf().ymfm_set_busy_end(16);
}


//-------------------------------------------------
//  write - handle a write to the data register
//-------------------------------------------------

void ymf288::write_data(uint8_t data)
{
	// ignore if paired with upper address
	if (bitfield(m_address, 8))
		return;

	// wait times are shorter in YMF288 mode
	int busy_cycles = ymf288_mode() ? 16 : 32 * m_fm.clock_prescale();
	if (m_address < 0x0e)
	{
		// 00-0D: write to SSG
		m_ssg.write(m_address & 0x0f, data);
	}
	else if (m_address < 0x10)
	{
		// 0E-0F: I/O ports not supported
	}
	else if (m_address < 0x20)
	{
		// 10-1F: write to ADPCM-A
		m_adpcm_a.write(m_address & 0x0f, data);
		busy_cycles = 32 * m_fm.clock_prescale();
	}
	else if (m_address == 0x27)
	{
		// 27: mode register; CSM isn't supported so disable it
		data &= 0x7f;
		m_fm.write(m_address, data);
	}
	else if (m_address == 0x29)
	{
		// 29: special IRQ mask register
		m_irq_enable = data;
		m_fm.set_irq_mask(m_irq_enable & ~m_flag_control & 0x03);
	}
	else
	{
		// 20-27, 2A-FF: write to FM
		m_fm.write(m_address, data);
	}

	// mark busy for a bit
	m_fm.intf().ymfm_set_busy_end(busy_cycles);
}


//-------------------------------------------------
//  write_address_hi - handle a write to the upper
//  address register
//-------------------------------------------------

void ymf288::write_address_hi(uint8_t data)
{
	// just set the address
	m_address = 0x100 | data;

	// in YMF288 mode, busy is signaled after address writes too
	if (ymf288_mode())
		m_fm.intf().ymfm_set_busy_end(16);
}


//-------------------------------------------------
//  write_data_hi - handle a write to the upper
//  data register
//-------------------------------------------------

void ymf288::write_data_hi(uint8_t data)
{
	// ignore if paired with upper address
	if (!bitfield(m_address, 8))
		return;

	// wait times are shorter in YMF288 mode
	int busy_cycles = ymf288_mode() ? 16 : 32 * m_fm.clock_prescale();
	if (m_address == 0x110)
	{
		// 110: IRQ flag control
		if (bitfield(data, 7))
			m_fm.set_reset_status(0, 0xff);
		else
		{
			m_flag_control = data;
			m_fm.set_irq_mask(m_irq_enable & ~m_flag_control & 0x03);
		}
	}
	else
	{
		// 100-10F,111-1FF: write to FM
		m_fm.write(m_address, data);
	}

	// mark busy for a bit
	m_fm.intf().ymfm_set_busy_end(busy_cycles);
}


//-------------------------------------------------
//  write - handle a write to the register
//  interface
//-------------------------------------------------

void ymf288::write(uint32_t offset, uint8_t data)
{
	switch (offset & 3)
	{
		case 0: // address port
			write_address(data);
			break;

		case 1: // data port
			write_data(data);
			break;

		case 2: // upper address port
			write_address_hi(data);
			break;

		case 3: // upper data port
			write_data_hi(data);
			break;
	}
}


//-------------------------------------------------
//  generate - generate one sample of sound
//-------------------------------------------------

void ymf288::generate(output_data *output, uint32_t numsamples)
{
	// FM output is just repeated the prescale number of times; note that
	// 0 is a special 1.5 case
	if (m_fm_samples_per_output != 0)
	{
		for (uint32_t samp = 0; samp < numsamples; samp++, output++)
		{
			if ((m_ssg_resampler.sampindex() + samp) % m_fm_samples_per_output == 0)
				clock_fm_and_adpcm();
			output->data[0] = m_last_fm.data[0];
			output->data[1] = m_last_fm.data[1];
		}
	}
	else
	{
		for (uint32_t samp = 0; samp < numsamples; samp++, output++)
		{
			uint32_t step = (m_ssg_resampler.sampindex() + samp) % 3;
			if (step == 0)
				clock_fm_and_adpcm();
			output->data[0] = m_last_fm.data[0];
			output->data[1] = m_last_fm.data[1];
			if (step == 1)
			{
				clock_fm_and_adpcm();
				output->data[0] = (output->data[0] + m_last_fm.data[0]) / 2;
				output->data[1] = (output->data[1] + m_last_fm.data[1]) / 2;
			}
		}
	}

	// resample the SSG as configured
	m_ssg_resampler.resample(output - numsamples, numsamples);
}


//-------------------------------------------------
//  update_prescale - update the prescale value,
//  recomputing derived values
//-------------------------------------------------

void ymf288::update_prescale()
{
	// Fidelity:   ---- minimum ----    ---- medium -----    ---- maximum-----
	//              rate = clock/144     rate = clock/144     rate = clock/16
	// Prescale    FM rate  SSG rate    FM rate  SSG rate    FM rate  SSG rate
	//     6          1:1     2:9          1:1     2:9         9:1     2:1

	// compute the number of FM samples per output sample, and select the
	// resampler function
	if (m_fidelity == OPN_FIDELITY_MIN || m_fidelity == OPN_FIDELITY_MED)
	{
		m_fm_samples_per_output = 1;
		m_ssg_resampler.configure(2, 9);
	}
	else
	{
		m_fm_samples_per_output = 9;
		m_ssg_resampler.configure(2, 1);
	}

	// if overriding the SSG, override the configuration with the nop
	// resampler to at least keep the sample index moving forward
	if (m_ssg.overridden())
		m_ssg_resampler.configure(0, 0);
}


//-------------------------------------------------
//  clock_fm_and_adpcm - clock FM and ADPCM state
//-------------------------------------------------

void ymf288::clock_fm_and_adpcm()
{
	// top bit of the IRQ enable flags controls 3-channel vs 6-channel mode
	uint32_t fmmask = bitfield(m_irq_enable, 7) ? 0x3f : 0x07;

	// clock the system
	uint32_t env_counter = m_fm.clock(fm_engine::ALL_CHANNELS);

	// clock the ADPCM-A engine on every envelope cycle
	// (channels 4 and 5 clock every 2 envelope clocks)
	if (bitfield(env_counter, 0, 2) == 0)
		m_adpcm_a.clock(bitfield(env_counter, 2) ? 0x0f : 0x3f);

	// update the FM content; OPNA is 13-bit with no intermediate clipping
	m_fm.output(m_last_fm.clear(), 1, 32767, fmmask);

	// mix in the ADPCM
	m_adpcm_a.output(m_last_fm, 0x3f);
}



//*********************************************************
//  YM2610
//*********************************************************

//-------------------------------------------------
//  ym2610 - constructor
//-------------------------------------------------

ym2610::ym2610(ymfm_interface &intf, uint8_t channel_mask) :
	m_fidelity(OPN_FIDELITY_MAX),
	m_address(0),
	m_fm_mask(channel_mask),
	m_eos_status(0x00),
	m_flag_mask(EOS_FLAGS_MASK),
	m_fm(intf),
	m_ssg(intf),
	m_ssg_resampler(m_ssg),
	m_adpcm_a(intf, 8),
	m_adpcm_b(intf, 8)
{
	update_prescale();
}


//-------------------------------------------------
//  reset - reset the system
//-------------------------------------------------

void ym2610::reset()
{
	// reset the engines
	m_fm.reset();
	m_ssg.reset();
	m_adpcm_a.reset();
	m_adpcm_b.reset();

	// initialize our special interrupt states
	m_eos_status = 0x00;
	m_flag_mask = EOS_FLAGS_MASK;
}


//-------------------------------------------------
//  save_restore - save or restore the data
//-------------------------------------------------

void ym2610::save_restore(ymfm_saved_state &state)
{
	state.save_restore(m_address);
	state.save_restore(m_eos_status);
	state.save_restore(m_flag_mask);

	m_fm.save_restore(state);
	m_ssg.save_restore(state);
	m_ssg_resampler.save_restore(state);
	m_adpcm_a.save_restore(state);
	m_adpcm_b.save_restore(state);
}


//-------------------------------------------------
//  read_status - read the status register
//-------------------------------------------------

uint8_t ym2610::read_status()
{
	uint8_t result = m_fm.status() & (fm_engine::STATUS_TIMERA | fm_engine::STATUS_TIMERB);
	if (m_fm.intf().ymfm_is_busy())
		result |= fm_engine::STATUS_BUSY;
	return result;
}


//-------------------------------------------------
//  read_data - read the data register
//-------------------------------------------------

uint8_t ym2610::read_data()
{
	uint8_t result = 0;
	if (m_address < 0x0e)
	{
		// 00-0D: Read from SSG
		result = m_ssg.read(m_address & 0x0f);
	}
	else if (m_address < 0x10)
	{
		// 0E-0F: I/O ports not supported
		result = 0xff;
	}
	else if (m_address == 0xff)
	{
		// FF: ID code
		result = 1;
	}
	return result;
}


//-------------------------------------------------
//  read_status_hi - read the extended status
//  register
//-------------------------------------------------

uint8_t ym2610::read_status_hi()
{
	return m_eos_status & m_flag_mask;
}


//-------------------------------------------------
//  read_data_hi - read the upper data register
//-------------------------------------------------

uint8_t ym2610::read_data_hi()
{
	uint8_t result = 0;
	return result;
}


//-------------------------------------------------
//  read - handle a read from the device
//-------------------------------------------------

uint8_t ym2610::read(uint32_t offset)
{
	uint8_t result = 0;
	switch (offset & 3)
	{
		case 0: // status port, YM2203 compatible
			result = read_status();
			break;

		case 1: // data port (only SSG)
			result = read_data();
			break;

		case 2: // status port, extended
			result = read_status_hi();
			break;

		case 3: // ADPCM-B data
			result = read_data_hi();
			break;
	}
	return result;
}


//-------------------------------------------------
//  write_address - handle a write to the address
//  register
//-------------------------------------------------

void ym2610::write_address(uint8_t data)
{
	// just set the address
	m_address = data;
}


//-------------------------------------------------
//  write - handle a write to the data register
//-------------------------------------------------

void ym2610::write_data(uint8_t data)
{
	// ignore if paired with upper address
	if (bitfield(m_address, 8))
		return;

	if (m_address < 0x0e)
	{
		// 00-0D: write to SSG
		m_ssg.write(m_address & 0x0f, data);
	}
	else if (m_address < 0x10)
	{
		// 0E-0F: I/O ports not supported
	}
	else if (m_address < 0x1c)
	{
		// 10-1B: write to ADPCM-B
		// YM2610 effectively forces external mode on, and disables recording
		if (m_address == 0x10)
			data = (data | 0x20) & ~0x40;
		m_adpcm_b.write(m_address & 0x0f, data);
	}
	else if (m_address == 0x1c)
	{
		// 1C: EOS flag reset
		m_flag_mask = ~data & EOS_FLAGS_MASK;
		m_eos_status &= ~(data & EOS_FLAGS_MASK);
	}
	else
	{
		// 1D-FF: write to FM
		m_fm.write(m_address, data);
	}

	// mark busy for a bit
	m_fm.intf().ymfm_set_busy_end(32 * m_fm.clock_prescale());
}


//-------------------------------------------------
//  write_address_hi - handle a write to the upper
//  address register
//-------------------------------------------------

void ym2610::write_address_hi(uint8_t data)
{
	// just set the address
	m_address = 0x100 | data;
}


//-------------------------------------------------
//  write_data_hi - handle a write to the upper
//  data register
//-------------------------------------------------

void ym2610::write_data_hi(uint8_t data)
{
	// ignore if paired with upper address
	if (!bitfield(m_address, 8))
		return;

	if (m_address < 0x130)
	{
		// 100-12F: write to ADPCM-A
		m_adpcm_a.write(m_address & 0x3f, data);
	}
	else
	{
		// 130-1FF: write to FM
		m_fm.write(m_address, data);
	}

	// mark busy for a bit
	m_fm.intf().ymfm_set_busy_end(32 * m_fm.clock_prescale());
}


//-------------------------------------------------
//  write - handle a write to the register
//  interface
//-------------------------------------------------

void ym2610::write(uint32_t offset, uint8_t data)
{
	switch (offset & 3)
	{
		case 0: // address port
			write_address(data);
			break;

		case 1: // data port
			write_data(data);
			break;

		case 2: // upper address port
			write_address_hi(data);
			break;

		case 3: // upper data port
			write_data_hi(data);
			break;
	}
}


//-------------------------------------------------
//  generate - generate one sample of sound
//-------------------------------------------------

void ym2610::generate(output_data *output, uint32_t numsamples)
{
	// FM output is just repeated the prescale number of times
	for (uint32_t samp = 0; samp < numsamples; samp++, output++)
	{
		if ((m_ssg_resampler.sampindex() + samp) % m_fm_samples_per_output == 0)
			clock_fm_and_adpcm();
		output->data[0] = m_last_fm.data[0];
		output->data[1] = m_last_fm.data[1];
	}

	// resample the SSG as configured
	m_ssg_resampler.resample(output - numsamples, numsamples);
}


//-------------------------------------------------
//  update_prescale - update the prescale value,
//  recomputing derived values
//-------------------------------------------------

void ym2610::update_prescale()
{
	// Fidelity:   ---- minimum ----    ---- medium -----    ---- maximum-----
	//              rate = clock/144     rate = clock/144     rate = clock/16
	// Prescale    FM rate  SSG rate    FM rate  SSG rate    FM rate  SSG rate
	//     6          1:1     2:9          1:1     2:9         9:1     2:1

	// compute the number of FM samples per output sample, and select the
	// resampler function
	if (m_fidelity == OPN_FIDELITY_MIN || m_fidelity == OPN_FIDELITY_MED)
	{
		m_fm_samples_per_output = 1;
		m_ssg_resampler.configure(2, 9);
	}
	else
	{
		m_fm_samples_per_output = 9;
		m_ssg_resampler.configure(2, 1);
	}

	// if overriding the SSG, override the configuration with the nop
	// resampler to at least keep the sample index moving forward
	if (m_ssg.overridden())
		m_ssg_resampler.configure(0, 0);
}


//-------------------------------------------------
//  clock_fm_and_adpcm - clock FM and ADPCM state
//-------------------------------------------------

void ym2610::clock_fm_and_adpcm()
{
	// clock the system
	uint32_t env_counter = m_fm.clock(m_fm_mask);

	// clock the ADPCM-A engine on every envelope cycle
	if (bitfield(env_counter, 0, 2) == 0)
		m_eos_status |= m_adpcm_a.clock(0x3f);

	// clock the ADPCM-B engine every cycle
	m_adpcm_b.clock();

	// we track the last ADPCM-B EOS value in bit 6 (which is hidden from callers);
	// if it changed since the last sample, update the visible EOS state in bit 7
	uint8_t live_eos = ((m_adpcm_b.status() & adpcm_b_channel::STATUS_EOS) != 0) ? 0x40 : 0x00;
	if (((live_eos ^ m_eos_status) & 0x40) != 0)
		m_eos_status = (m_eos_status & ~0xc0) | live_eos | (live_eos << 1);

	// update the FM content; OPNB is 13-bit with no intermediate clipping
	m_fm.output(m_last_fm.clear(), 1, 32767, m_fm_mask);

	// mix in the ADPCM and clamp
	m_adpcm_a.output(m_last_fm, 0x3f);
	m_adpcm_b.output(m_last_fm, 1);
	m_last_fm.clamp16();
}



//*********************************************************
//  YM2612
//*********************************************************

//-------------------------------------------------
//  ym2612 - constructor
//-------------------------------------------------

ym2612::ym2612(ymfm_interface &intf) :
	m_address(0),
	m_dac_data(0),
	m_dac_enable(0),
	m_fm(intf)
{
}


//-------------------------------------------------
//  reset - reset the system
//-------------------------------------------------

void ym2612::reset()
{
	// reset the engines
	m_fm.reset();
}


//-------------------------------------------------
//  save_restore - save or restore the data
//-------------------------------------------------

void ym2612::save_restore(ymfm_saved_state &state)
{
	state.save_restore(m_address);
	state.save_restore(m_dac_data);
	state.save_restore(m_dac_enable);
	m_fm.save_restore(state);
}


//-------------------------------------------------
//  read_status - read the status register
//-------------------------------------------------

uint8_t ym2612::read_status()
{
	uint8_t result = m_fm.status();
	if (m_fm.intf().ymfm_is_busy())
		result |= fm_engine::STATUS_BUSY;
	return result;
}


//-------------------------------------------------
//  read - handle a read from the device
//-------------------------------------------------

uint8_t ym2612::read(uint32_t offset)
{
	uint8_t result = 0;
	switch (offset & 3)
	{
		case 0: // status port, YM2203 compatible
			result = read_status();
			break;

		case 1: // data port (unused)
		case 2: // status port, extended
		case 3: // data port (unused)
			debug::log_unexpected_read_write("Unexpected read from YM2612 offset %d\n", offset & 3);
			break;
	}
	return result;
}


//-------------------------------------------------
//  write_address - handle a write to the address
//  register
//-------------------------------------------------

void ym2612::write_address(uint8_t data)
{
	// just set the address
	m_address = data;
}


//-------------------------------------------------
//  write_data - handle a write to the data
//  register
//-------------------------------------------------

void ym2612::write_data(uint8_t data)
{
	// ignore if paired with upper address
	if (bitfield(m_address, 8))
		return;

	if (m_address == 0x2a)
	{
		// 2A: DAC data (most significant 8 bits)
		m_dac_data = (m_dac_data & ~0x1fe) | ((data ^ 0x80) << 1);
	}
	else if (m_address == 0x2b)
	{
		// 2B: DAC enable (bit 7)
		m_dac_enable = bitfield(data, 7);
	}
	else if (m_address == 0x2c)
	{
		// 2C: test/low DAC bit
		m_dac_data = (m_dac_data & ~1) | bitfield(data, 3);
	}
	else
	{
		// 00-29, 2D-FF: write to FM
		m_fm.write(m_address, data);
	}

	// mark busy for a bit
	m_fm.intf().ymfm_set_busy_end(32 * m_fm.clock_prescale());
}


//-------------------------------------------------
//  write_address_hi - handle a write to the upper
//  address register
//-------------------------------------------------

void ym2612::write_address_hi(uint8_t data)
{
	// just set the address
	m_address = 0x100 | data;
}


//-------------------------------------------------
//  write_data_hi - handle a write to the upper
//  data register
//-------------------------------------------------

void ym2612::write_data_hi(uint8_t data)
{
	// ignore if paired with upper address
	if (!bitfield(m_address, 8))
		return;

	// 100-1FF: write to FM
	m_fm.write(m_address, data);

	// mark busy for a bit
	m_fm.intf().ymfm_set_busy_end(32 * m_fm.clock_prescale());
}


//-------------------------------------------------
//  write - handle a write to the register
//  interface
//-------------------------------------------------

void ym2612::write(uint32_t offset, uint8_t data)
{
	switch (offset & 3)
	{
		case 0: // address port
			write_address(data);
			break;

		case 1: // data port
			write_data(data);
			break;

		case 2: // upper address port
			write_address_hi(data);
			break;

		case 3: // upper data port
			write_data_hi(data);
			break;
	}
}


//-------------------------------------------------
//  generate - generate one sample of sound
//-------------------------------------------------

void ym2612::generate(output_data *output, uint32_t numsamples)
{
	for (uint32_t samp = 0; samp < numsamples; samp++, output++)
	{
		// clock the system
		m_fm.clock(fm_engine::ALL_CHANNELS);

		// sum individual channels to apply DAC discontinuity on each
		output->clear();
		output_data temp;

		// first do FM-only channels; OPN2 is 9-bit with intermediate clipping
		int const last_fm_channel = m_dac_enable ? 5 : 6;
		for (int chan = 0; chan < last_fm_channel; chan++)
		{
			m_fm.output(temp.clear(), 5, 256, 1 << chan);
			output->data[0] += dac_discontinuity(temp.data[0]);
			output->data[1] += dac_discontinuity(temp.data[1]);
		}

		// add in DAC
		if (m_dac_enable)
		{
			// DAC enabled: start with DAC value then add the first 5 channels only
			int32_t dacval = dac_discontinuity(int16_t(m_dac_data << 7) >> 7);
			output->data[0] += m_fm.regs().ch_output_0(0x102) ? dacval : dac_discontinuity(0);
			output->data[1] += m_fm.regs().ch_output_1(0x102) ? dacval : dac_discontinuity(0);
		}

		// output is technically multiplexed rather than mixed, but that requires
		// a better sound mixer than we usually have, so just average over the six
		// channels; also apply a 64/65 factor to account for the discontinuity
		// adjustment above
		output->data[0] = (output->data[0] * 128) * 64 / (6 * 65);
		output->data[1] = (output->data[1] * 128) * 64 / (6 * 65);
	}
}


//-------------------------------------------------
//  generate - generate one sample of sound
//-------------------------------------------------

void ym3438::generate(output_data *output, uint32_t numsamples)
{
	for (uint32_t samp = 0; samp < numsamples; samp++, output++)
	{
		// clock the system
		m_fm.clock(fm_engine::ALL_CHANNELS);

		// first do FM-only channels; OPN2C is 9-bit with intermediate clipping
		if (!m_dac_enable)
		{
			// DAC disabled: all 6 channels sum together
			m_fm.output(output->clear(), 5, 256, fm_engine::ALL_CHANNELS);
		}
		else
		{
			// DAC enabled: start with DAC value then add the first 5 channels only
			int32_t dacval = int16_t(m_dac_data << 7) >> 7;
			output->data[0] = m_fm.regs().ch_output_0(0x102) ? dacval : 0;
			output->data[1] = m_fm.regs().ch_output_1(0x102) ? dacval : 0;
			m_fm.output(*output, 5, 256, fm_engine::ALL_CHANNELS ^ (1 << 5));
		}

		// YM3438 doesn't have the same DAC discontinuity, though its output is
		// multiplexed like the YM2612
		output->data[0] = (output->data[0] * 128) / 6;
		output->data[1] = (output->data[1] * 128) / 6;
	}
}


//-------------------------------------------------
//  generate - generate one sample of sound
//-------------------------------------------------

void ymf276::generate(output_data *output, uint32_t numsamples)
{
	for (uint32_t samp = 0; samp < numsamples; samp++, output++)
	{
		// clock the system
		m_fm.clock(fm_engine::ALL_CHANNELS);

		// first do FM-only channels; OPN2L is 14-bit with intermediate clipping
		if (!m_dac_enable)
		{
			// DAC disabled: all 6 channels sum together
			m_fm.output(output->clear(), 0, 8191, fm_engine::ALL_CHANNELS);
		}
		else
		{
			// DAC enabled: start with DAC value then add the first 5 channels only
			int32_t dacval = int16_t(m_dac_data << 7) >> 7;
			output->data[0] = m_fm.regs().ch_output_0(0x102) ? dacval : 0;
			output->data[1] = m_fm.regs().ch_output_1(0x102) ? dacval : 0;
			m_fm.output(*output, 0, 8191, fm_engine::ALL_CHANNELS ^ (1 << 5));
		}

		// YMF276 is properly mixed; it shifts down 1 bit before clamping
		output->data[0] = clamp(output->data[0] >> 1, -32768, 32767);
		output->data[1] = clamp(output->data[1] >> 1, -32768, 32767);
	}
}

}
