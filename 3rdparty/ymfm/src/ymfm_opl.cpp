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

#include "ymfm_opl.h"
#include "ymfm_fm.ipp"

namespace ymfm
{

//-------------------------------------------------
//  opl_key_scale_atten - converts an
//  OPL concatenated block (3 bits) and fnum
//  (10 bits) into an attenuation offset; values
//  here are for 6dB/octave, in 0.75dB units
//  (matching total level LSB)
//-------------------------------------------------

inline uint32_t opl_key_scale_atten(uint32_t block, uint32_t fnum_4msb)
{
	// this table uses the top 4 bits of FNUM and are the maximal values
	// (for when block == 7). Values for other blocks can be computed by
	// subtracting 8 for each block below 7.
	static uint8_t const fnum_to_atten[16] = { 0,24,32,37,40,43,45,47,48,50,51,52,53,54,55,56 };
	int32_t result = fnum_to_atten[fnum_4msb] - 8 * (block ^ 7);
	return std::max<int32_t>(0, result);
}


//*********************************************************
//  OPL REGISTERS
//*********************************************************

//-------------------------------------------------
//  opl_registers_base - constructor
//-------------------------------------------------

template<int Revision>
opl_registers_base<Revision>::opl_registers_base() :
	m_lfo_am_counter(0),
	m_lfo_pm_counter(0),
	m_noise_lfsr(1),
	m_lfo_am(0)
{
	// create these pointers to appease overzealous compilers checking array
	// bounds in unreachable code (looking at you, clang)
	uint16_t *wf0 = &m_waveform[0][0];
	uint16_t *wf1 = &m_waveform[1 % WAVEFORMS][0];
	uint16_t *wf2 = &m_waveform[2 % WAVEFORMS][0];
	uint16_t *wf3 = &m_waveform[3 % WAVEFORMS][0];
	uint16_t *wf4 = &m_waveform[4 % WAVEFORMS][0];
	uint16_t *wf5 = &m_waveform[5 % WAVEFORMS][0];
	uint16_t *wf6 = &m_waveform[6 % WAVEFORMS][0];
	uint16_t *wf7 = &m_waveform[7 % WAVEFORMS][0];

	// create the waveforms
	for (uint32_t index = 0; index < WAVEFORM_LENGTH; index++)
		wf0[index] = abs_sin_attenuation(index) | (bitfield(index, 9) << 15);

	if (WAVEFORMS >= 4)
	{
		uint16_t zeroval = wf0[0];
		for (uint32_t index = 0; index < WAVEFORM_LENGTH; index++)
		{
			wf1[index] = bitfield(index, 9) ? zeroval : wf0[index];
			wf2[index] = wf0[index] & 0x7fff;
			wf3[index] = bitfield(index, 8) ? zeroval : (wf0[index] & 0x7fff);
			if (WAVEFORMS >= 8)
			{
				wf4[index] = bitfield(index, 9) ? zeroval : wf0[index * 2];
				wf5[index] = bitfield(index, 9) ? zeroval : wf0[(index * 2) & 0x1ff];
				wf6[index] = bitfield(index, 9) << 15;
				wf7[index] = (bitfield(index, 9) ? (index ^ 0x13ff) : index) << 3;
			}
		}
	}

	// OPL3/OPL4 have dynamic operators, so initialize the fourop_enable value here
	// since operator_map() is called right away, prior to reset()
	if (Revision > 2)
		m_regdata[0x104 % REGISTERS] = 0;
}


//-------------------------------------------------
//  reset - reset to initial state
//-------------------------------------------------

template<int Revision>
void opl_registers_base<Revision>::reset()
{
	std::fill_n(&m_regdata[0], REGISTERS, 0);
}


//-------------------------------------------------
//  save_restore - save or restore the data
//-------------------------------------------------

template<int Revision>
void opl_registers_base<Revision>::save_restore(ymfm_saved_state &state)
{
	state.save_restore(m_lfo_am_counter);
	state.save_restore(m_lfo_pm_counter);
	state.save_restore(m_lfo_am);
	state.save_restore(m_noise_lfsr);
	state.save_restore(m_regdata);
}


//-------------------------------------------------
//  operator_map - return an array of operator
//  indices for each channel; for OPL this is fixed
//-------------------------------------------------

template<int Revision>
void opl_registers_base<Revision>::operator_map(operator_mapping &dest) const
{
	if (Revision <= 2)
	{
		// OPL/OPL2 has a fixed map, all 2 operators
		static const operator_mapping s_fixed_map =
		{ {
			operator_list(  0,  3 ),  // Channel 0 operators
			operator_list(  1,  4 ),  // Channel 1 operators
			operator_list(  2,  5 ),  // Channel 2 operators
			operator_list(  6,  9 ),  // Channel 3 operators
			operator_list(  7, 10 ),  // Channel 4 operators
			operator_list(  8, 11 ),  // Channel 5 operators
			operator_list( 12, 15 ),  // Channel 6 operators
			operator_list( 13, 16 ),  // Channel 7 operators
			operator_list( 14, 17 ),  // Channel 8 operators
		} };
		dest = s_fixed_map;
	}
	else
	{
		// OPL3/OPL4 can be configured for 2 or 4 operators
		uint32_t fourop = fourop_enable();

		dest.chan[ 0] = bitfield(fourop, 0) ? operator_list(  0,  3,  6,  9 ) : operator_list(  0,  3 );
		dest.chan[ 1] = bitfield(fourop, 1) ? operator_list(  1,  4,  7, 10 ) : operator_list(  1,  4 );
		dest.chan[ 2] = bitfield(fourop, 2) ? operator_list(  2,  5,  8, 11 ) : operator_list(  2,  5 );
		dest.chan[ 3] = bitfield(fourop, 0) ? operator_list() : operator_list(  6,  9 );
		dest.chan[ 4] = bitfield(fourop, 1) ? operator_list() : operator_list(  7, 10 );
		dest.chan[ 5] = bitfield(fourop, 2) ? operator_list() : operator_list(  8, 11 );
		dest.chan[ 6] = operator_list( 12, 15 );
		dest.chan[ 7] = operator_list( 13, 16 );
		dest.chan[ 8] = operator_list( 14, 17 );

		dest.chan[ 9] = bitfield(fourop, 3) ? operator_list( 18, 21, 24, 27 ) : operator_list( 18, 21 );
		dest.chan[10] = bitfield(fourop, 4) ? operator_list( 19, 22, 25, 28 ) : operator_list( 19, 22 );
		dest.chan[11] = bitfield(fourop, 5) ? operator_list( 20, 23, 26, 29 ) : operator_list( 20, 23 );
		dest.chan[12] = bitfield(fourop, 3) ? operator_list() : operator_list( 24, 27 );
		dest.chan[13] = bitfield(fourop, 4) ? operator_list() : operator_list( 25, 28 );
		dest.chan[14] = bitfield(fourop, 5) ? operator_list() : operator_list( 26, 29 );
		dest.chan[15] = operator_list( 30, 33 );
		dest.chan[16] = operator_list( 31, 34 );
		dest.chan[17] = operator_list( 32, 35 );
	}
}


//-------------------------------------------------
//  write - handle writes to the register array
//-------------------------------------------------

template<int Revision>
bool opl_registers_base<Revision>::write(uint16_t index, uint8_t data, uint32_t &channel, uint32_t &opmask)
{
	assert(index < REGISTERS);

	// writes to the mode register with high bit set ignore the low bits
	if (index == REG_MODE && bitfield(data, 7) != 0)
		m_regdata[index] |= 0x80;
	else
		m_regdata[index] = data;

	// handle writes to the rhythm keyons
	if (index == 0xbd)
	{
		channel = RHYTHM_CHANNEL;
		opmask = bitfield(data, 5) ? bitfield(data, 0, 5) : 0;
		return true;
	}

	// handle writes to the channel keyons
	if ((index & 0xf0) == 0xb0)
	{
		channel = index & 0x0f;
		if (channel < 9)
		{
			if (IsOpl3Plus)
				channel += 9 * bitfield(index, 8);
			opmask = bitfield(data, 5) ? 15 : 0;
			return true;
		}
	}
	return false;
}


//-------------------------------------------------
//  clock_noise_and_lfo - clock the noise and LFO,
//  handling clock division, depth, and waveform
//  computations
//-------------------------------------------------

static int32_t opl_clock_noise_and_lfo(uint32_t &noise_lfsr, uint16_t &lfo_am_counter, uint16_t &lfo_pm_counter, uint8_t &lfo_am, uint32_t am_depth, uint32_t pm_depth)
{
	// OPL has a 23-bit noise generator for the rhythm section, running at
	// a constant rate, used only for percussion input
	noise_lfsr <<= 1;
	noise_lfsr |= bitfield(noise_lfsr, 23) ^ bitfield(noise_lfsr, 9) ^ bitfield(noise_lfsr, 8) ^ bitfield(noise_lfsr, 1);

	// OPL has two fixed-frequency LFOs, one for AM, one for PM

	// the AM LFO has 210*64 steps; at a nominal 50kHz output,
	// this equates to a period of 50000/(210*64) = 3.72Hz
	uint32_t am_counter = lfo_am_counter++;
	if (am_counter >= 210*64 - 1)
		lfo_am_counter = 0;

	// low 8 bits are fractional; depth 0 is divided by 2, while depth 1 is times 2
	int shift = 9 - 2 * am_depth;

	// AM value is the upper bits of the value, inverted across the midpoint
	// to produce a triangle
	lfo_am = ((am_counter < 105*64) ? am_counter : (210*64+63 - am_counter)) >> shift;

	// the PM LFO has 8192 steps, or a nominal period of 6.1Hz
	uint32_t pm_counter = lfo_pm_counter++;

	// PM LFO is broken into 8 chunks, each lasting 1024 steps; the PM value
	// depends on the upper bits of FNUM, so this value is a fraction and
	// sign to apply to that value, as a 1.3 value
	static int8_t const pm_scale[8] = { 8, 4, 0, -4, -8, -4, 0, 4 };
	return pm_scale[bitfield(pm_counter, 10, 3)] >> (pm_depth ^ 1);
}

template<int Revision>
int32_t opl_registers_base<Revision>::clock_noise_and_lfo()
{
	return opl_clock_noise_and_lfo(m_noise_lfsr, m_lfo_am_counter, m_lfo_pm_counter, m_lfo_am, lfo_am_depth(), lfo_pm_depth());
}


//-------------------------------------------------
//  cache_operator_data - fill the operator cache
//  with prefetched data; note that this code is
//  also used by ymopna_registers, so it must
//  handle upper channels cleanly
//-------------------------------------------------

template<int Revision>
void opl_registers_base<Revision>::cache_operator_data(uint32_t choffs, uint32_t opoffs, opdata_cache &cache)
{
	// set up the easy stuff
	cache.waveform = &m_waveform[op_waveform(opoffs) % WAVEFORMS][0];

	// get frequency from the channel
	uint32_t block_freq = cache.block_freq = ch_block_freq(choffs);

	// compute the keycode: block_freq is:
	//
	//     111  |
	//     21098|76543210
	//     BBBFF|FFFFFFFF
	//     ^^^??
	//
	// the 4-bit keycode uses the top 3 bits plus one of the next two bits
	uint32_t keycode = bitfield(block_freq, 10, 3) << 1;

	// lowest bit is determined by note_select(); note that it is
	// actually reversed from what the manual says, however
	keycode |= bitfield(block_freq, 9 - note_select(), 1);

	// no detune adjustment on OPL
	cache.detune = 0;

	// multiple value, as an x.1 value (0 means 0.5)
	// replace the low bit with a table lookup to give 0,1,2,3,4,5,6,7,8,9,10,10,12,12,15,15
	uint32_t multiple = op_multiple(opoffs);
	cache.multiple = ((multiple & 0xe) | bitfield(0xc2aa, multiple)) * 2;
	if (cache.multiple == 0)
		cache.multiple = 1;

	// phase step, or PHASE_STEP_DYNAMIC if PM is active; this depends on block_freq, detune,
	// and multiple, so compute it after we've done those
	if (op_lfo_pm_enable(opoffs) == 0)
		cache.phase_step = compute_phase_step(choffs, opoffs, cache, 0);
	else
		cache.phase_step = opdata_cache::PHASE_STEP_DYNAMIC;

	// total level, scaled by 8
	cache.total_level = op_total_level(opoffs) << 3;

	// pre-add key scale level
	uint32_t ksl = op_ksl(opoffs);
	if (ksl != 0)
		cache.total_level += opl_key_scale_atten(bitfield(block_freq, 10, 3), bitfield(block_freq, 6, 4)) << ksl;

	// 4-bit sustain level, but 15 means 31 so effectively 5 bits
	cache.eg_sustain = op_sustain_level(opoffs);
	cache.eg_sustain |= (cache.eg_sustain + 1) & 0x10;
	cache.eg_sustain <<= 5;

	// determine KSR adjustment for enevlope rates
	uint32_t ksrval = keycode >> (2 * (op_ksr(opoffs) ^ 1));
	cache.eg_rate[EG_ATTACK] = effective_rate(op_attack_rate(opoffs) * 4, ksrval);
	cache.eg_rate[EG_DECAY] = effective_rate(op_decay_rate(opoffs) * 4, ksrval);
	cache.eg_rate[EG_SUSTAIN] = op_eg_sustain(opoffs) ? 0 : effective_rate(op_release_rate(opoffs) * 4, ksrval);
	cache.eg_rate[EG_RELEASE] = effective_rate(op_release_rate(opoffs) * 4, ksrval);
	cache.eg_rate[EG_DEPRESS] = 0x3f;
}


//-------------------------------------------------
//  compute_phase_step - compute the phase step
//-------------------------------------------------

static uint32_t opl_compute_phase_step(uint32_t block_freq, uint32_t multiple, int32_t lfo_raw_pm)
{
	// OPL phase calculation has no detuning, but uses FNUMs like
	// the OPN version, and computes PM a bit differently

	// extract frequency number as a 12-bit fraction
	uint32_t fnum = bitfield(block_freq, 0, 10) << 2;

	// apply the phase adjustment based on the upper 3 bits
	// of FNUM and the PM depth parameters
	fnum += (lfo_raw_pm * bitfield(block_freq, 7, 3)) >> 1;

	// keep fnum to 12 bits
	fnum &= 0xfff;

	// apply block shift to compute phase step
	uint32_t block = bitfield(block_freq, 10, 3);
	uint32_t phase_step = (fnum << block) >> 2;

	// apply frequency multiplier (which is cached as an x.1 value)
	return (phase_step * multiple) >> 1;
}

template<int Revision>
uint32_t opl_registers_base<Revision>::compute_phase_step(uint32_t choffs, uint32_t opoffs, opdata_cache const &cache, int32_t lfo_raw_pm)
{
	return opl_compute_phase_step(cache.block_freq, cache.multiple, op_lfo_pm_enable(opoffs) ? lfo_raw_pm : 0);
}


//-------------------------------------------------
//  log_keyon - log a key-on event
//-------------------------------------------------

template<int Revision>
std::string opl_registers_base<Revision>::log_keyon(uint32_t choffs, uint32_t opoffs)
{
	uint32_t chnum = (choffs & 15) + 9 * bitfield(choffs, 8);
	uint32_t opnum = (opoffs & 31) - 2 * ((opoffs & 31) / 8) + 18 * bitfield(opoffs, 8);

	char buffer[256];
	int end = 0;

	end += snprintf(&buffer[end], sizeof(buffer) - end, "%2u.%02u freq=%04X fb=%u alg=%X mul=%X tl=%02X ksr=%u ns=%u ksl=%u adr=%X/%X/%X sl=%X sus=%u",
		chnum, opnum,
		ch_block_freq(choffs),
		ch_feedback(choffs),
		ch_algorithm(choffs),
		op_multiple(opoffs),
		op_total_level(opoffs),
		op_ksr(opoffs),
		note_select(),
		op_ksl(opoffs),
		op_attack_rate(opoffs),
		op_decay_rate(opoffs),
		op_release_rate(opoffs),
		op_sustain_level(opoffs),
		op_eg_sustain(opoffs));

	if (OUTPUTS > 1)
		end += snprintf(&buffer[end], sizeof(buffer) - end, " out=%c%c%c%c",
			ch_output_0(choffs) ? 'L' : '-',
			ch_output_1(choffs) ? 'R' : '-',
			ch_output_2(choffs) ? '0' : '-',
			ch_output_3(choffs) ? '1' : '-');
	if (op_lfo_am_enable(opoffs) != 0)
		end += snprintf(&buffer[end], sizeof(buffer) - end, " am=%u", lfo_am_depth());
	if (op_lfo_pm_enable(opoffs) != 0)
		end += snprintf(&buffer[end], sizeof(buffer) - end, " pm=%u", lfo_pm_depth());
	if (waveform_enable() && op_waveform(opoffs) != 0)
		end += snprintf(&buffer[end], sizeof(buffer) - end, " wf=%u", op_waveform(opoffs));
	if (is_rhythm(choffs))
		end += snprintf(&buffer[end], sizeof(buffer) - end, " rhy=1");
	if (DYNAMIC_OPS)
	{
		operator_mapping map;
		operator_map(map);
		if (bitfield(map.chan[chnum], 16, 8) != 0xff)
			end += snprintf(&buffer[end], sizeof(buffer) - end, " 4op");
	}

	return buffer;
}


//*********************************************************
//  OPLL SPECIFICS
//*********************************************************

//-------------------------------------------------
//  opll_registers - constructor
//-------------------------------------------------

opll_registers::opll_registers() :
	m_lfo_am_counter(0),
	m_lfo_pm_counter(0),
	m_noise_lfsr(1),
	m_lfo_am(0)
{
	// create the waveforms
	for (uint32_t index = 0; index < WAVEFORM_LENGTH; index++)
		m_waveform[0][index] = abs_sin_attenuation(index) | (bitfield(index, 9) << 15);

	uint16_t zeroval = m_waveform[0][0];
	for (uint32_t index = 0; index < WAVEFORM_LENGTH; index++)
		m_waveform[1][index] = bitfield(index, 9) ? zeroval : m_waveform[0][index];

	// initialize the instruments to something sane
	for (uint32_t choffs = 0; choffs < CHANNELS; choffs++)
		m_chinst[choffs] = &m_regdata[0];
	for (uint32_t opoffs = 0; opoffs < OPERATORS; opoffs++)
		m_opinst[opoffs] = &m_regdata[bitfield(opoffs, 0)];
}


//-------------------------------------------------
//  reset - reset to initial state
//-------------------------------------------------

void opll_registers::reset()
{
	std::fill_n(&m_regdata[0], REGISTERS, 0);
}


//-------------------------------------------------
//  save_restore - save or restore the data
//-------------------------------------------------

void opll_registers::save_restore(ymfm_saved_state &state)
{
	state.save_restore(m_lfo_am_counter);
	state.save_restore(m_lfo_pm_counter);
	state.save_restore(m_lfo_am);
	state.save_restore(m_noise_lfsr);
	state.save_restore(m_regdata);
}


//-------------------------------------------------
//  operator_map - return an array of operator
//  indices for each channel; for OPLL this is fixed
//-------------------------------------------------

void opll_registers::operator_map(operator_mapping &dest) const
{
	static const operator_mapping s_fixed_map =
	{ {
		operator_list(  0,  1 ),  // Channel 0 operators
		operator_list(  2,  3 ),  // Channel 1 operators
		operator_list(  4,  5 ),  // Channel 2 operators
		operator_list(  6,  7 ),  // Channel 3 operators
		operator_list(  8,  9 ),  // Channel 4 operators
		operator_list( 10, 11 ),  // Channel 5 operators
		operator_list( 12, 13 ),  // Channel 6 operators
		operator_list( 14, 15 ),  // Channel 7 operators
		operator_list( 16, 17 ),  // Channel 8 operators
	} };
	dest = s_fixed_map;
}


//-------------------------------------------------
//  write - handle writes to the register array;
//  note that this code is also used by
//  ymopl3_registers, so it must handle upper
//  channels cleanly
//-------------------------------------------------

bool opll_registers::write(uint16_t index, uint8_t data, uint32_t &channel, uint32_t &opmask)
{
	// unclear the address is masked down to 6 bits or if writes above
	// the register top are ignored; assuming the latter for now
	if (index >= REGISTERS)
		return false;

	// write the new data
	m_regdata[index] = data;

	// handle writes to the rhythm keyons
	if (index == 0x0e)
	{
		channel = RHYTHM_CHANNEL;
		opmask = bitfield(data, 5) ? bitfield(data, 0, 5) : 0;
		return true;
	}

	// handle writes to the channel keyons
	if ((index & 0xf0) == 0x20)
	{
		channel = index & 0x0f;
		if (channel < CHANNELS)
		{
			opmask = bitfield(data, 4) ? 3 : 0;
			return true;
		}
	}
	return false;
}


//-------------------------------------------------
//  clock_noise_and_lfo - clock the noise and LFO,
//  handling clock division, depth, and waveform
//  computations
//-------------------------------------------------

int32_t opll_registers::clock_noise_and_lfo()
{
	// implementation is the same as OPL with fixed depths
	return opl_clock_noise_and_lfo(m_noise_lfsr, m_lfo_am_counter, m_lfo_pm_counter, m_lfo_am, 1, 1);
}


//-------------------------------------------------
//  cache_operator_data - fill the operator cache
//  with prefetched data; note that this code is
//  also used by ymopna_registers, so it must
//  handle upper channels cleanly
//-------------------------------------------------

void opll_registers::cache_operator_data(uint32_t choffs, uint32_t opoffs, opdata_cache &cache)
{
	// first set up the instrument data
	uint32_t instrument = ch_instrument(choffs);
	if (rhythm_enable() && choffs >= 6)
		m_chinst[choffs] = &m_instdata[8 * (15 + (choffs - 6))];
	else
		m_chinst[choffs] = (instrument == 0) ? &m_regdata[0] : &m_instdata[8 * (instrument - 1)];
	m_opinst[opoffs] = m_chinst[choffs] + bitfield(opoffs, 0);

	// set up the easy stuff
	cache.waveform = &m_waveform[op_waveform(opoffs) % WAVEFORMS][0];

	// get frequency from the channel
	uint32_t block_freq = cache.block_freq = ch_block_freq(choffs);

	// compute the keycode: block_freq is:
	//
	//     11  |
	//     1098|76543210
	//     BBBF|FFFFFFFF
	//     ^^^^
	//
	// the 4-bit keycode uses the top 4 bits
	uint32_t keycode = bitfield(block_freq, 8, 4);

	// no detune adjustment on OPLL
	cache.detune = 0;

	// multiple value, as an x.1 value (0 means 0.5)
	// replace the low bit with a table lookup to give 0,1,2,3,4,5,6,7,8,9,10,10,12,12,15,15
	uint32_t multiple = op_multiple(opoffs);
	cache.multiple = ((multiple & 0xe) | bitfield(0xc2aa, multiple)) * 2;
	if (cache.multiple == 0)
		cache.multiple = 1;

	// phase step, or PHASE_STEP_DYNAMIC if PM is active; this depends on
	// block_freq, detune, and multiple, so compute it after we've done those
	if (op_lfo_pm_enable(opoffs) == 0)
		cache.phase_step = compute_phase_step(choffs, opoffs, cache, 0);
	else
		cache.phase_step = opdata_cache::PHASE_STEP_DYNAMIC;

	// total level, scaled by 8; for non-rhythm operator 0, this is the total
	// level from the instrument data; for other operators it is 4*volume
	if (bitfield(opoffs, 0) == 1 || (rhythm_enable() && choffs >= 7))
		cache.total_level = op_volume(opoffs) * 4;
	else
		cache.total_level = ch_total_level(choffs);
	cache.total_level <<= 3;

	// pre-add key scale level
	uint32_t ksl = op_ksl(opoffs);
	if (ksl != 0)
		cache.total_level += opl_key_scale_atten(bitfield(block_freq, 9, 3), bitfield(block_freq, 5, 4)) << ksl;

	// 4-bit sustain level, but 15 means 31 so effectively 5 bits
	cache.eg_sustain = op_sustain_level(opoffs);
	cache.eg_sustain |= (cache.eg_sustain + 1) & 0x10;
	cache.eg_sustain <<= 5;

	// The envelope diagram in the YM2413 datasheet gives values for these
	// in ms from 0->48dB. The attack/decay tables give values in ms from
	// 0->96dB, so to pick an equivalent decay rate, we want to find the
	// closest match that is 2x the 0->48dB value:
	//
	//     DP =   10ms (0->48db) ->   20ms (0->96db); decay of 12 gives   19.20ms
	//     RR =  310ms (0->48db) ->  620ms (0->96db); decay of  7 gives  613.76ms
	//     RS = 1200ms (0->48db) -> 2400ms (0->96db); decay of  5 gives 2455.04ms
	//
	// The envelope diagram for percussive sounds (eg_sustain() == 0) also uses
	// "RR" to mean both the constant RR above and the Release Rate specified in
	// the instrument data. In this case, Relief Pitcher's credit sound bears out
	// that the Release Rate is used during sustain, and that the constant RR
	// (or RS) is used during the release phase.
	constexpr uint8_t DP = 12 * 4;
	constexpr uint8_t RR = 7 * 4;
	constexpr uint8_t RS = 5 * 4;

	// determine KSR adjustment for envelope rates
	uint32_t ksrval = keycode >> (2 * (op_ksr(opoffs) ^ 1));
	cache.eg_rate[EG_DEPRESS] = DP;
	cache.eg_rate[EG_ATTACK] = effective_rate(op_attack_rate(opoffs) * 4, ksrval);
	cache.eg_rate[EG_DECAY] = effective_rate(op_decay_rate(opoffs) * 4, ksrval);
	if (op_eg_sustain(opoffs))
	{
		cache.eg_rate[EG_SUSTAIN] = 0;
		cache.eg_rate[EG_RELEASE] = ch_sustain(choffs) ? RS : effective_rate(op_release_rate(opoffs) * 4, ksrval);
	}
	else
	{
		cache.eg_rate[EG_SUSTAIN] = effective_rate(op_release_rate(opoffs) * 4, ksrval);
		cache.eg_rate[EG_RELEASE] = ch_sustain(choffs) ? RS : RR;
	}
}


//-------------------------------------------------
//  compute_phase_step - compute the phase step
//-------------------------------------------------

uint32_t opll_registers::compute_phase_step(uint32_t choffs, uint32_t opoffs, opdata_cache const &cache, int32_t lfo_raw_pm)
{
	// phase step computation is the same as OPL but the block_freq has one
	// more bit, which we shift in
	return opl_compute_phase_step(cache.block_freq << 1, cache.multiple, op_lfo_pm_enable(opoffs) ? lfo_raw_pm : 0);
}


//-------------------------------------------------
//  log_keyon - log a key-on event
//-------------------------------------------------

std::string opll_registers::log_keyon(uint32_t choffs, uint32_t opoffs)
{
	uint32_t chnum = choffs;
	uint32_t opnum = opoffs;

	char buffer[256];
	int end = 0;

	end += snprintf(&buffer[end], sizeof(buffer) - end, "%u.%02u freq=%04X inst=%X fb=%u mul=%X",
		chnum, opnum,
		ch_block_freq(choffs),
		ch_instrument(choffs),
		ch_feedback(choffs),
		op_multiple(opoffs));

	if (bitfield(opoffs, 0) == 1 || (is_rhythm(choffs) && choffs >= 6))
		end += snprintf(&buffer[end], sizeof(buffer) - end, " vol=%X", op_volume(opoffs));
	else
		end += snprintf(&buffer[end], sizeof(buffer) - end, " tl=%02X", ch_total_level(choffs));

	end += snprintf(&buffer[end], sizeof(buffer) - end, " ksr=%u ksl=%u adr=%X/%X/%X sl=%X sus=%u/%u",
		op_ksr(opoffs),
		op_ksl(opoffs),
		op_attack_rate(opoffs),
		op_decay_rate(opoffs),
		op_release_rate(opoffs),
		op_sustain_level(opoffs),
		op_eg_sustain(opoffs),
		ch_sustain(choffs));

	if (op_lfo_am_enable(opoffs))
		end += snprintf(&buffer[end], sizeof(buffer) - end, " am=1");
	if (op_lfo_pm_enable(opoffs))
		end += snprintf(&buffer[end], sizeof(buffer) - end, " pm=1");
	if (op_waveform(opoffs) != 0)
		end += snprintf(&buffer[end], sizeof(buffer) - end, " wf=1");
	if (is_rhythm(choffs))
		end += snprintf(&buffer[end], sizeof(buffer) - end, " rhy=1");

	return buffer;
}



//*********************************************************
//  YM3526
//*********************************************************

//-------------------------------------------------
//  ym3526 - constructor
//-------------------------------------------------

ym3526::ym3526(ymfm_interface &intf) :
	m_address(0),
	m_fm(intf)
{
}


//-------------------------------------------------
//  reset - reset the system
//-------------------------------------------------

void ym3526::reset()
{
	// reset the engines
	m_fm.reset();
}


//-------------------------------------------------
//  save_restore - save or restore the data
//-------------------------------------------------

void ym3526::save_restore(ymfm_saved_state &state)
{
	state.save_restore(m_address);
	m_fm.save_restore(state);
}


//-------------------------------------------------
//  read_status - read the status register
//-------------------------------------------------

uint8_t ym3526::read_status()
{
	return m_fm.status() | 0x06;
}


//-------------------------------------------------
//  read - handle a read from the device
//-------------------------------------------------

uint8_t ym3526::read(uint32_t offset)
{
	uint8_t result = 0xff;
	switch (offset & 1)
	{
		case 0: // status port
			result = read_status();
			break;

		case 1: // when A0=1 datasheet says "the data on the bus are not guaranteed"
			break;
	}
	return result;
}


//-------------------------------------------------
//  write_address - handle a write to the address
//  register
//-------------------------------------------------

void ym3526::write_address(uint8_t data)
{
	// YM3526 doesn't expose a busy signal, and the datasheets don't indicate
	// delays, but all other OPL chips need 12 cycles for address writes
	m_fm.intf().ymfm_set_busy_end(12 * m_fm.clock_prescale());

	// just set the address
	m_address = data;
}


//-------------------------------------------------
//  write - handle a write to the register
//  interface
//-------------------------------------------------

void ym3526::write_data(uint8_t data)
{
	// YM3526 doesn't expose a busy signal, and the datasheets don't indicate
	// delays, but all other OPL chips need 84 cycles for data writes
	m_fm.intf().ymfm_set_busy_end(84 * m_fm.clock_prescale());

	// write to FM
	m_fm.write(m_address, data);
}


//-------------------------------------------------
//  write - handle a write to the register
//  interface
//-------------------------------------------------

void ym3526::write(uint32_t offset, uint8_t data)
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
//  generate - generate samples of sound
//-------------------------------------------------

void ym3526::generate(output_data *output, uint32_t numsamples)
{
	for (uint32_t samp = 0; samp < numsamples; samp++, output++)
	{
		// clock the system
		m_fm.clock(fm_engine::ALL_CHANNELS);

		// update the FM content; mixing details for YM3526 need verification
		m_fm.output(output->clear(), 1, 32767, fm_engine::ALL_CHANNELS);

		// YM3526 uses an external DAC (YM3014) with mantissa/exponent format
		// convert to 10.3 floating point value and back to simulate truncation
		output->roundtrip_fp();
	}
}



//*********************************************************
//  Y8950
//*********************************************************

//-------------------------------------------------
//  y8950 - constructor
//-------------------------------------------------

y8950::y8950(ymfm_interface &intf) :
	m_address(0),
	m_io_ddr(0),
	m_fm(intf),
	m_adpcm_b(intf)
{
}


//-------------------------------------------------
//  reset - reset the system
//-------------------------------------------------

void y8950::reset()
{
	// reset the engines
	m_fm.reset();
	m_adpcm_b.reset();
}


//-------------------------------------------------
//  save_restore - save or restore the data
//-------------------------------------------------

void y8950::save_restore(ymfm_saved_state &state)
{
	state.save_restore(m_address);
	state.save_restore(m_io_ddr);
	m_fm.save_restore(state);
}


//-------------------------------------------------
//  read_status - read the status register
//-------------------------------------------------

uint8_t y8950::read_status()
{
	// start with current FM status, masking out bits we might set
	uint8_t status = m_fm.status() & ~(STATUS_ADPCM_B_EOS | STATUS_ADPCM_B_BRDY | STATUS_ADPCM_B_PLAYING);

	// insert the live ADPCM status bits
	uint8_t adpcm_status = m_adpcm_b.status();
	if ((adpcm_status & adpcm_b_channel::STATUS_EOS) != 0)
		status |= STATUS_ADPCM_B_EOS;
	if ((adpcm_status & adpcm_b_channel::STATUS_BRDY) != 0)
		status |= STATUS_ADPCM_B_BRDY;
	if ((adpcm_status & adpcm_b_channel::STATUS_PLAYING) != 0)
		status |= STATUS_ADPCM_B_PLAYING;

	// run it through the FM engine to handle interrupts for us
	return m_fm.set_reset_status(status, ~status);
}


//-------------------------------------------------
//  read_data - read the data port
//-------------------------------------------------

uint8_t y8950::read_data()
{
	uint8_t result = 0xff;
	switch (m_address)
	{
		case 0x05:  // keyboard in
			result = m_fm.intf().ymfm_external_read(ACCESS_IO, 1);
			break;

		case 0x09:  // ADPCM data
		case 0x1a:
			result = m_adpcm_b.read(m_address - 0x07);
			break;

		case 0x19:  // I/O data
			result = m_fm.intf().ymfm_external_read(ACCESS_IO, 0);
			break;

		default:
			debug::log_unexpected_read_write("Unexpected read from Y8950 data port %02X\n", m_address);
			break;
	}
	return result;
}


//-------------------------------------------------
//  read - handle a read from the device
//-------------------------------------------------

uint8_t y8950::read(uint32_t offset)
{
	uint8_t result = 0xff;
	switch (offset & 1)
	{
		case 0: // status port
			result = read_status();
			break;

		case 1: // when A0=1 datasheet says "the data on the bus are not guaranteed"
			result = read_data();
			break;
	}
	return result;
}


//-------------------------------------------------
//  write_address - handle a write to the address
//  register
//-------------------------------------------------

void y8950::write_address(uint8_t data)
{
	// Y8950 doesn't expose a busy signal, but it does indicate that
	// address writes should be no faster than every 12 clocks
	m_fm.intf().ymfm_set_busy_end(12 * m_fm.clock_prescale());

	// just set the address
	m_address = data;
}


//-------------------------------------------------
//  write - handle a write to the register
//  interface
//-------------------------------------------------

void y8950::write_data(uint8_t data)
{
	// Y8950 doesn't expose a busy signal, but it does indicate that
	// data writes should be no faster than every 12 clocks for
	// registers 00-1A, or every 84 clocks for other registers
	m_fm.intf().ymfm_set_busy_end(((m_address <= 0x1a) ? 12 : 84) * m_fm.clock_prescale());

	// handle special addresses
	switch (m_address)
	{
		case 0x04:  // IRQ control
			m_fm.write(m_address, data);
			read_status();
			break;

		case 0x06:  // keyboard out
			m_fm.intf().ymfm_external_write(ACCESS_IO, 1, data);
			break;

		case 0x08:  // split FM/ADPCM-B
			m_adpcm_b.write(m_address - 0x07, (data & 0x0f) | 0x80);
			m_fm.write(m_address, data & 0xc0);
			break;

		case 0x07:  // ADPCM-B registers
		case 0x09:
		case 0x0a:
		case 0x0b:
		case 0x0c:
		case 0x0d:
		case 0x0e:
		case 0x0f:
		case 0x10:
		case 0x11:
		case 0x12:
		case 0x15:
		case 0x16:
		case 0x17:
			m_adpcm_b.write(m_address - 0x07, data);
			break;

		case 0x18:  // I/O direction
			m_io_ddr = data & 0x0f;
			break;

		case 0x19:  // I/O data
			m_fm.intf().ymfm_external_write(ACCESS_IO, 0, data & m_io_ddr);
			break;

		default:    // everything else to FM
			m_fm.write(m_address, data);
			break;
	}
}


//-------------------------------------------------
//  write - handle a write to the register
//  interface
//-------------------------------------------------

void y8950::write(uint32_t offset, uint8_t data)
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
//  generate - generate samples of sound
//-------------------------------------------------

void y8950::generate(output_data *output, uint32_t numsamples)
{
	for (uint32_t samp = 0; samp < numsamples; samp++, output++)
	{
		// clock the system
		m_fm.clock(fm_engine::ALL_CHANNELS);
		m_adpcm_b.clock();

		// update the FM content; clipping need verification
		m_fm.output(output->clear(), 1, 32767, fm_engine::ALL_CHANNELS);

		// mix in the ADPCM; ADPCM-B is stereo, but only one channel
		// not sure how it's wired up internally
		m_adpcm_b.output(*output, 3);

		// Y8950 uses an external DAC (YM3014) with mantissa/exponent format
		// convert to 10.3 floating point value and back to simulate truncation
		output->roundtrip_fp();
	}
}



//*********************************************************
//  YM3812
//*********************************************************

//-------------------------------------------------
//  ym3812 - constructor
//-------------------------------------------------

ym3812::ym3812(ymfm_interface &intf) :
	m_address(0),
	m_fm(intf)
{
}


//-------------------------------------------------
//  reset - reset the system
//-------------------------------------------------

void ym3812::reset()
{
	// reset the engines
	m_fm.reset();
}


//-------------------------------------------------
//  save_restore - save or restore the data
//-------------------------------------------------

void ym3812::save_restore(ymfm_saved_state &state)
{
	state.save_restore(m_address);
	m_fm.save_restore(state);
}


//-------------------------------------------------
//  read_status - read the status register
//-------------------------------------------------

uint8_t ym3812::read_status()
{
	return m_fm.status() | 0x06;
}


//-------------------------------------------------
//  read - handle a read from the device
//-------------------------------------------------

uint8_t ym3812::read(uint32_t offset)
{
	uint8_t result = 0xff;
	switch (offset & 1)
	{
		case 0: // status port
			result = read_status();
			break;

		case 1: // "inhibit" according to datasheet
			break;
	}
	return result;
}


//-------------------------------------------------
//  write_address - handle a write to the address
//  register
//-------------------------------------------------

void ym3812::write_address(uint8_t data)
{
	// YM3812 doesn't expose a busy signal, but it does indicate that
	// address writes should be no faster than every 12 clocks
	m_fm.intf().ymfm_set_busy_end(12 * m_fm.clock_prescale());

	// just set the address
	m_address = data;
}


//-------------------------------------------------
//  write - handle a write to the register
//  interface
//-------------------------------------------------

void ym3812::write_data(uint8_t data)
{
	// YM3812 doesn't expose a busy signal, but it does indicate that
	// data writes should be no faster than every 84 clocks
	m_fm.intf().ymfm_set_busy_end(84 * m_fm.clock_prescale());

	// write to FM
	m_fm.write(m_address, data);
}


//-------------------------------------------------
//  write - handle a write to the register
//  interface
//-------------------------------------------------

void ym3812::write(uint32_t offset, uint8_t data)
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
//  generate - generate samples of sound
//-------------------------------------------------

void ym3812::generate(output_data *output, uint32_t numsamples)
{
	for (uint32_t samp = 0; samp < numsamples; samp++, output++)
	{
		// clock the system
		m_fm.clock(fm_engine::ALL_CHANNELS);

		// update the FM content; mixing details for YM3812 need verification
		m_fm.output(output->clear(), 1, 32767, fm_engine::ALL_CHANNELS);

		// YM3812 uses an external DAC (YM3014) with mantissa/exponent format
		// convert to 10.3 floating point value and back to simulate truncation
		output->roundtrip_fp();
	}
}



//*********************************************************
//  YMF262
//*********************************************************

//-------------------------------------------------
//  ymf262 - constructor
//-------------------------------------------------

ymf262::ymf262(ymfm_interface &intf) :
	m_address(0),
	m_fm(intf)
{
}


//-------------------------------------------------
//  reset - reset the system
//-------------------------------------------------

void ymf262::reset()
{
	// reset the engines
	m_fm.reset();
}


//-------------------------------------------------
//  save_restore - save or restore the data
//-------------------------------------------------

void ymf262::save_restore(ymfm_saved_state &state)
{
	state.save_restore(m_address);
	m_fm.save_restore(state);
}


//-------------------------------------------------
//  read_status - read the status register
//-------------------------------------------------

uint8_t ymf262::read_status()
{
	return m_fm.status();
}


//-------------------------------------------------
//  read - handle a read from the device
//-------------------------------------------------

uint8_t ymf262::read(uint32_t offset)
{
	uint8_t result = 0xff;
	switch (offset & 3)
	{
		case 0: // status port
			result = read_status();
			break;

		case 1:
		case 2:
		case 3:
			debug::log_unexpected_read_write("Unexpected read from YMF262 offset %d\n", offset & 3);
			break;
	}
	return result;
}


//-------------------------------------------------
//  write_address - handle a write to the address
//  register
//-------------------------------------------------

void ymf262::write_address(uint8_t data)
{
	// YMF262 doesn't expose a busy signal, but it does indicate that
	// address writes should be no faster than every 32 clocks
	m_fm.intf().ymfm_set_busy_end(32 * m_fm.clock_prescale());

	// just set the address
	m_address = data;
}


//-------------------------------------------------
//  write_data - handle a write to the data
//  register
//-------------------------------------------------

void ymf262::write_data(uint8_t data)
{
	// YMF262 doesn't expose a busy signal, but it does indicate that
	// data writes should be no faster than every 32 clocks
	m_fm.intf().ymfm_set_busy_end(32 * m_fm.clock_prescale());

	// write to FM
	m_fm.write(m_address, data);
}


//-------------------------------------------------
//  write_address_hi - handle a write to the upper
//  address register
//-------------------------------------------------

void ymf262::write_address_hi(uint8_t data)
{
	// YMF262 doesn't expose a busy signal, but it does indicate that
	// address writes should be no faster than every 32 clocks
	m_fm.intf().ymfm_set_busy_end(32 * m_fm.clock_prescale());

	// just set the address
	m_address = data | 0x100;

	// tests reveal that in compatibility mode, upper bit is masked
	// except for register 0x105
	if (m_fm.regs().newflag() == 0 && m_address != 0x105)
		m_address &= 0xff;
}


//-------------------------------------------------
//  write - handle a write to the register
//  interface
//-------------------------------------------------

void ymf262::write(uint32_t offset, uint8_t data)
{
	switch (offset & 3)
	{
		case 0: // address port
			write_address(data);
			break;

		case 1: // data port
			write_data(data);
			break;

		case 2: // address port
			write_address_hi(data);
			break;

		case 3: // data port
			write_data(data);
			break;
	}
}


//-------------------------------------------------
//  generate - generate samples of sound
//-------------------------------------------------

void ymf262::generate(output_data *output, uint32_t numsamples)
{
	for (uint32_t samp = 0; samp < numsamples; samp++, output++)
	{
		// clock the system
		m_fm.clock(fm_engine::ALL_CHANNELS);

		// update the FM content; mixing details for YMF262 need verification
		m_fm.output(output->clear(), 0, 32767, fm_engine::ALL_CHANNELS);

		// YMF262 output is 16-bit offset serial via YAC512 DAC
		output->clamp16();
	}
}



//*********************************************************
//  YMF289B
//*********************************************************

// YMF289B is a YMF262 with the following changes:
//   * "Power down" mode added
//   * Bulk register clear added
//   * Busy flag added to the status register
//   * Shorter busy times
//   * All registers can be read
//   * Only 2 outputs exposed

//-------------------------------------------------
//  ymf289b - constructor
//-------------------------------------------------

ymf289b::ymf289b(ymfm_interface &intf) :
	m_address(0),
	m_fm(intf)
{
}


//-------------------------------------------------
//  reset - reset the system
//-------------------------------------------------

void ymf289b::reset()
{
	// reset the engines
	m_fm.reset();
}


//-------------------------------------------------
//  save_restore - save or restore the data
//-------------------------------------------------

void ymf289b::save_restore(ymfm_saved_state &state)
{
	state.save_restore(m_address);
	m_fm.save_restore(state);
}


//-------------------------------------------------
//  read_status - read the status register
//-------------------------------------------------

uint8_t ymf289b::read_status()
{
	uint8_t result = m_fm.status();

	// YMF289B adds a busy flag
	if (ymf289b_mode() && m_fm.intf().ymfm_is_busy())
		result |= STATUS_BUSY_FLAGS;
	return result;
}


//-------------------------------------------------
//  read_data - read the data register
//-------------------------------------------------

uint8_t ymf289b::read_data()
{
	uint8_t result = 0xff;

	// YMF289B can read register data back
	if (ymf289b_mode())
		result = m_fm.regs().read(m_address);
	return result;
}


//-------------------------------------------------
//  read - handle a read from the device
//-------------------------------------------------

uint8_t ymf289b::read(uint32_t offset)
{
	uint8_t result = 0xff;
	switch (offset & 3)
	{
		case 0: // status port
			result = read_status();
			break;

		case 1:	// data port
			result = read_data();
			break;

		case 2:
		case 3:
			debug::log_unexpected_read_write("Unexpected read from YMF289B offset %d\n", offset & 3);
			break;
	}
	return result;
}


//-------------------------------------------------
//  write_address - handle a write to the address
//  register
//-------------------------------------------------

void ymf289b::write_address(uint8_t data)
{
	m_address = data;

	// count busy time
	m_fm.intf().ymfm_set_busy_end(56);
}


//-------------------------------------------------
//  write_data - handle a write to the data
//  register
//-------------------------------------------------

void ymf289b::write_data(uint8_t data)
{
	// write to FM
	m_fm.write(m_address, data);

	// writes to 0x108 with the CLR flag set clear the registers
	if (m_address == 0x108 && bitfield(data, 2) != 0)
		m_fm.regs().reset();

	// count busy time
	m_fm.intf().ymfm_set_busy_end(56);
}


//-------------------------------------------------
//  write_address_hi - handle a write to the upper
//  address register
//-------------------------------------------------

void ymf289b::write_address_hi(uint8_t data)
{
	// just set the address
	m_address = data | 0x100;

	// tests reveal that in compatibility mode, upper bit is masked
	// except for register 0x105
	if (m_fm.regs().newflag() == 0 && m_address != 0x105)
		m_address &= 0xff;

	// count busy time
	m_fm.intf().ymfm_set_busy_end(56);
}


//-------------------------------------------------
//  write - handle a write to the register
//  interface
//-------------------------------------------------

void ymf289b::write(uint32_t offset, uint8_t data)
{
	switch (offset & 3)
	{
		case 0: // address port
			write_address(data);
			break;

		case 1: // data port
			write_data(data);
			break;

		case 2: // address port
			write_address_hi(data);
			break;

		case 3: // data port
			write_data(data);
			break;
	}
}


//-------------------------------------------------
//  generate - generate samples of sound
//-------------------------------------------------

void ymf289b::generate(output_data *output, uint32_t numsamples)
{
	for (uint32_t samp = 0; samp < numsamples; samp++, output++)
	{
		// clock the system
		m_fm.clock(fm_engine::ALL_CHANNELS);

		// update the FM content; mixing details for YMF262 need verification
		fm_engine::output_data full;
		m_fm.output(full.clear(), 0, 32767, fm_engine::ALL_CHANNELS);

		// YMF278B output is 16-bit offset serial via YAC512 DAC, but
		// only 2 of the 4 outputs are exposed
		output->data[0] = full.data[0];
		output->data[1] = full.data[1];
		output->clamp16();
	}
}



//*********************************************************
//  YMF278B
//*********************************************************

//-------------------------------------------------
//  ymf278b - constructor
//-------------------------------------------------

ymf278b::ymf278b(ymfm_interface &intf) :
	m_address(0),
	m_fm_pos(0),
	m_load_remaining(0),
	m_next_status_id(false),
	m_fm(intf),
	m_pcm(intf)
{
}


//-------------------------------------------------
//  reset - reset the system
//-------------------------------------------------

void ymf278b::reset()
{
	// reset the engines
	m_fm.reset();
	m_pcm.reset();

	// next status read will return ID
	m_next_status_id = true;
}


//-------------------------------------------------
//  save_restore - save or restore the data
//-------------------------------------------------

void ymf278b::save_restore(ymfm_saved_state &state)
{
	state.save_restore(m_address);
	state.save_restore(m_fm_pos);
	state.save_restore(m_load_remaining);
	state.save_restore(m_next_status_id);
	m_fm.save_restore(state);
	m_pcm.save_restore(state);
}


//-------------------------------------------------
//  read_status - read the status register
//-------------------------------------------------

uint8_t ymf278b::read_status()
{
	uint8_t result;

	// first status read after initialization returns a chip ID, which
	// varies based on the "new" flags, indicating the mode
	if (m_next_status_id)
	{
		if (m_fm.regs().new2flag())
			result = 0x02;
		else if (m_fm.regs().newflag())
			result = 0x00;
		else
			result = 0x06;
		m_next_status_id = false;
	}
	else
	{
		result = m_fm.status();
		if (m_fm.intf().ymfm_is_busy())
			result |= STATUS_BUSY;
		if (m_load_remaining != 0)
			result |= STATUS_LD;

		// if new2 flag is not set, we're in OPL2 or OPL3 mode
		if (!m_fm.regs().new2flag())
			result &= ~(STATUS_BUSY | STATUS_LD);
	}
	return result;
}


//-------------------------------------------------
//  write_data_pcm - handle a write to the PCM data
//  register
//-------------------------------------------------

uint8_t ymf278b::read_data_pcm()
{
	// read from PCM
	if (bitfield(m_address, 9) != 0)
	{
		uint8_t result = m_pcm.read(m_address & 0xff);
		if ((m_address & 0xff) == 0x02)
			result |= 0x20;

		return result;
	}
	return 0;
}


//-------------------------------------------------
//  read - handle a read from the device
//-------------------------------------------------

uint8_t ymf278b::read(uint32_t offset)
{
	uint8_t result = 0xff;
	switch (offset & 7)
	{
		case 0: // status port
			result = read_status();
			break;

		case 5: // PCM data port
			result = read_data_pcm();
			break;

		default:
			debug::log_unexpected_read_write("Unexpected read from ymf278b offset %d\n", offset & 3);
			break;
	}
	return result;
}


//-------------------------------------------------
//  write_address - handle a write to the address
//  register
//-------------------------------------------------

void ymf278b::write_address(uint8_t data)
{
	// just set the address
	m_address = data;
}


//-------------------------------------------------
//  write_data - handle a write to the data
//  register
//-------------------------------------------------

void ymf278b::write_data(uint8_t data)
{
	// write to FM
	if (bitfield(m_address, 9) == 0)
	{
		uint8_t old = m_fm.regs().new2flag();
		m_fm.write(m_address, data);

		// changing NEW2 from 0->1 causes the next status read to
		// return the chip ID
		if (old == 0 && m_fm.regs().new2flag() != 0)
			m_next_status_id = true;
	}

	// BUSY goes for 56 clocks on FM writes
	m_fm.intf().ymfm_set_busy_end(56);
}


//-------------------------------------------------
//  write_address_hi - handle a write to the upper
//  address register
//-------------------------------------------------

void ymf278b::write_address_hi(uint8_t data)
{
	// just set the address
	m_address = data | 0x100;

	// YMF262, in compatibility mode, treats the upper bit as masked
	// except for register 0x105; assuming YMF278B works the same way?
	if (m_fm.regs().newflag() == 0 && m_address != 0x105)
		m_address &= 0xff;
}


//-------------------------------------------------
//  write_address_pcm - handle a write to the upper
//  address register
//-------------------------------------------------

void ymf278b::write_address_pcm(uint8_t data)
{
	// just set the address
	m_address = data | 0x200;
}


//-------------------------------------------------
//  write_data_pcm - handle a write to the PCM data
//  register
//-------------------------------------------------

void ymf278b::write_data_pcm(uint8_t data)
{
	// ignore data writes if new2 is not yet set
	if (m_fm.regs().new2flag() == 0)
		return;

	// write to FM
	if (bitfield(m_address, 9) != 0)
	{
		uint8_t addr = m_address & 0xff;
		m_pcm.write(addr, data);

		// writes to the waveform number cause loads to happen for "about 300usec"
		// which is ~13 samples at the nominal output frequency of 44.1kHz
		if (addr >= 0x08 && addr <= 0x1f)
			m_load_remaining = 13;
	}

	// BUSY goes for 88 clocks on PCM writes
	m_fm.intf().ymfm_set_busy_end(88);
}


//-------------------------------------------------
//  write - handle a write to the register
//  interface
//-------------------------------------------------

void ymf278b::write(uint32_t offset, uint8_t data)
{
	switch (offset & 7)
	{
		case 0: // address port
			write_address(data);
			break;

		case 1: // data port
			write_data(data);
			break;

		case 2: // address port
			write_address_hi(data);
			break;

		case 3: // data port
			write_data(data);
			break;

		case 4: // PCM address port
			write_address_pcm(data);
			break;

		case 5: // PCM address port
			write_data_pcm(data);
			break;

		default:
			debug::log_unexpected_read_write("Unexpected write to ymf278b offset %d\n", offset & 7);
			break;
	}
}


//-------------------------------------------------
//  generate - generate one sample of sound
//-------------------------------------------------

void ymf278b::generate(output_data *output, uint32_t numsamples)
{
	static const int16_t s_mix_scale[8] = { 0x7fa, 0x5a4, 0x3fd, 0x2d2, 0x1fe, 0x169, 0xff, 0 };
	int32_t const pcm_l = s_mix_scale[m_pcm.regs().mix_pcm_l()];
	int32_t const pcm_r = s_mix_scale[m_pcm.regs().mix_pcm_r()];
	int32_t const fm_l = s_mix_scale[m_pcm.regs().mix_fm_l()];
	int32_t const fm_r = s_mix_scale[m_pcm.regs().mix_fm_r()];
	for (uint32_t samp = 0; samp < numsamples; samp++, output++)
	{
		// clock the system
		m_fm_pos += FM_EXTRA_SAMPLE_STEP;
		if (m_fm_pos >= FM_EXTRA_SAMPLE_THRESH)
		{
			m_fm.clock(fm_engine::ALL_CHANNELS);
			m_fm_pos -= FM_EXTRA_SAMPLE_THRESH;
		}
		m_fm.clock(fm_engine::ALL_CHANNELS);
		m_pcm.clock(pcm_engine::ALL_CHANNELS);

		// update the FM content; mixing details for YMF278B need verification
		fm_engine::output_data fmout;
		m_fm.output(fmout.clear(), 0, 32767, fm_engine::ALL_CHANNELS);

		// update the PCM content
		pcm_engine::output_data pcmout;
		m_pcm.output(pcmout.clear(), pcm_engine::ALL_CHANNELS);

		// DO0 output: FM channels 2+3 only
		output->data[0] = fmout.data[2];
		output->data[1] = fmout.data[3];

		// DO1 output: wavetable channels 2+3 only
		output->data[2] = pcmout.data[2];
		output->data[3] = pcmout.data[3];

		// DO2 output: mixed FM channels 0+1 and wavetable channels 0+1
		output->data[4] = (fmout.data[0] * fm_l + pcmout.data[0] * pcm_l) >> 11;
		output->data[5] = (fmout.data[1] * fm_r + pcmout.data[1] * pcm_r) >> 11;

		// YMF278B output is 16-bit 2s complement serial
		output->clamp16();
	}

	// decrement the load waiting count
	if (m_load_remaining > 0)
		m_load_remaining -= std::min(m_load_remaining, numsamples);
}



//*********************************************************
//  OPLL BASE
//*********************************************************

//-------------------------------------------------
//  opll_base - constructor
//-------------------------------------------------

opll_base::opll_base(ymfm_interface &intf, uint8_t const *instrument_data) :
	m_address(0),
	m_fm(intf)
{
	m_fm.regs().set_instrument_data(instrument_data);
}


//-------------------------------------------------
//  reset - reset the system
//-------------------------------------------------

void opll_base::reset()
{
	// reset the engines
	m_fm.reset();
}


//-------------------------------------------------
//  save_restore - save or restore the data
//-------------------------------------------------

void opll_base::save_restore(ymfm_saved_state &state)
{
	state.save_restore(m_address);
	m_fm.save_restore(state);
}


//-------------------------------------------------
//  write_address - handle a write to the address
//  register
//-------------------------------------------------

void opll_base::write_address(uint8_t data)
{
	// OPLL doesn't expose a busy signal, but datasheets are pretty consistent
	// in indicating that address writes should be no faster than every 12 clocks
	m_fm.intf().ymfm_set_busy_end(12);

	// just set the address
	m_address = data;
}


//-------------------------------------------------
//  write - handle a write to the register
//  interface
//-------------------------------------------------

void opll_base::write_data(uint8_t data)
{
	// OPLL doesn't expose a busy signal, but datasheets are pretty consistent
	// in indicating that address writes should be no faster than every 84 clocks
	m_fm.intf().ymfm_set_busy_end(84);

	// write to FM
	m_fm.write(m_address, data);
}


//-------------------------------------------------
//  write - handle a write to the register
//  interface
//-------------------------------------------------

void opll_base::write(uint32_t offset, uint8_t data)
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

void opll_base::generate(output_data *output, uint32_t numsamples)
{
	for (uint32_t samp = 0; samp < numsamples; samp++, output++)
	{
		// clock the system
		m_fm.clock(fm_engine::ALL_CHANNELS);

		// update the FM content; OPLL has a built-in 9-bit DAC
		m_fm.output(output->clear(), 5, 256, fm_engine::ALL_CHANNELS);

		// final output is multiplexed; we don't simulate that here except
		// to average over everything
		output->data[0] = (output->data[0] * 128) / 9;
		output->data[1] = (output->data[1] * 128) / 9;
	}
}



//*********************************************************
//  YM2413
//*********************************************************

//-------------------------------------------------
//  ym2413 - constructor
//-------------------------------------------------

ym2413::ym2413(ymfm_interface &intf, uint8_t const *instrument_data) :
	opll_base(intf, (instrument_data != nullptr) ? instrument_data : s_default_instruments)
{
};

// table below taken from https://github.com/plgDavid/misc/wiki/Copyright-free-OPLL(x)-ROM-patches
uint8_t const ym2413::s_default_instruments[] =
{
	//April 2015 David Viens, tweaked May 19-21th 2015 Hubert Lamontagne
	0x71, 0x61, 0x1E, 0x17, 0xEF, 0x7F, 0x00, 0x17, //Violin
	0x13, 0x41, 0x1A, 0x0D, 0xF8, 0xF7, 0x23, 0x13, //Guitar
	0x13, 0x01, 0x99, 0x00, 0xF2, 0xC4, 0x11, 0x23, //Piano
	0x31, 0x61, 0x0E, 0x07, 0x98, 0x64, 0x70, 0x27, //Flute
	0x22, 0x21, 0x1E, 0x06, 0xBF, 0x76, 0x00, 0x28, //Clarinet
	0x31, 0x22, 0x16, 0x05, 0xE0, 0x71, 0x0F, 0x18, //Oboe
	0x21, 0x61, 0x1D, 0x07, 0x82, 0x8F, 0x10, 0x07, //Trumpet
	0x23, 0x21, 0x2D, 0x14, 0xFF, 0x7F, 0x00, 0x07, //Organ
	0x41, 0x61, 0x1B, 0x06, 0x64, 0x65, 0x10, 0x17, //Horn
	0x61, 0x61, 0x0B, 0x18, 0x85, 0xFF, 0x81, 0x07, //Synthesizer
	0x13, 0x01, 0x83, 0x11, 0xFA, 0xE4, 0x10, 0x04, //Harpsichord
	0x17, 0x81, 0x23, 0x07, 0xF8, 0xF8, 0x22, 0x12, //Vibraphone
	0x61, 0x50, 0x0C, 0x05, 0xF2, 0xF5, 0x29, 0x42, //Synthesizer Bass
	0x01, 0x01, 0x54, 0x03, 0xC3, 0x92, 0x03, 0x02, //Acoustic Bass
	0x41, 0x41, 0x89, 0x03, 0xF1, 0xE5, 0x11, 0x13, //Electric Guitar
	0x01, 0x01, 0x18, 0x0F, 0xDF, 0xF8, 0x6A, 0x6D, //rhythm 1
	0x01, 0x01, 0x00, 0x00, 0xC8, 0xD8, 0xA7, 0x48, //rhythm 2
	0x05, 0x01, 0x00, 0x00, 0xF8, 0xAA, 0x59, 0x55  //rhythm 3
};



//*********************************************************
//  YM2423
//*********************************************************

//-------------------------------------------------
//  ym2423 - constructor
//-------------------------------------------------

ym2423::ym2423(ymfm_interface &intf, uint8_t const *instrument_data) :
	opll_base(intf, (instrument_data != nullptr) ? instrument_data : s_default_instruments)
{
};

// table below taken from https://github.com/plgDavid/misc/wiki/Copyright-free-OPLL(x)-ROM-patches
uint8_t const ym2423::s_default_instruments[] =
{
	// May 4-6 2016 Hubert Lamontagne
	// Doesn't seem to have any diff between opllx-x and opllx-y
	// Drums seem identical to regular opll
	0x61, 0x61, 0x1B, 0x07, 0x94, 0x5F, 0x10, 0x06, //1	Strings	Saw wave with vibrato Violin
	0x93, 0xB1, 0x51, 0x04, 0xF3, 0xF2, 0x70, 0xFB, //2	Guitar	Jazz GuitarPiano
	0x41, 0x21, 0x11, 0x85, 0xF2, 0xF2, 0x70, 0x75, //3	Electric Guitar	Same as OPLL No.15 Synth
	0x93, 0xB2, 0x28, 0x07, 0xF3, 0xF2, 0x70, 0xB4, //4	Electric Piano 2	Slow attack, tremoloDing-a-ling
	0x72, 0x31, 0x97, 0x05, 0x51, 0x6F, 0x60, 0x09, //5 	Flute	Same as OPLL No.4Clarinet
	0x13, 0x30, 0x18, 0x06, 0xF7, 0xF4, 0x50, 0x85, //6	Marimba 	Also be used as steel drumXyophone
	0x51, 0x31, 0x1C, 0x07, 0x51, 0x71, 0x20, 0x26, //7	Trumpet 	Same as OPLL No.7Trumpet
	0x41, 0xF4, 0x1B, 0x07, 0x74, 0x34, 0x00, 0x06, //8	Harmonica Harmonica synth
	0x50, 0x30, 0x4D, 0x03, 0x42, 0x65, 0x20, 0x06, //9	Tuba Tuba
	0x40, 0x20, 0x10, 0x85, 0xF3, 0xF5, 0x20, 0x04, //10 	Synth Brass 2 Synth sweep
	0x61, 0x61, 0x1B, 0x07, 0xC5, 0x96, 0xF3, 0xF6, //11 	Short Saw	Saw wave with short envelopeSynth hit
	0xF9, 0xF1, 0xDC, 0x00, 0xF5, 0xF3, 0x77, 0xF2, //12 	Vibraphone	Bright vibraphoneVibes
	0x60, 0xA2, 0x91, 0x03, 0x94, 0xC1, 0xF7, 0xF7, //13 	Electric Guitar 2	Clean guitar with feedbackHarmonic bass
	0x30, 0x30, 0x17, 0x06, 0xF3, 0xF1, 0xB7, 0xFC, //14 	Synth Bass 2Snappy bass
	0x31, 0x36, 0x0D, 0x05, 0xF2, 0xF4, 0x27, 0x9C, //15 	Sitar	Also be used as ShamisenBanjo
	0x01, 0x01, 0x18, 0x0F, 0xDF, 0xF8, 0x6A, 0x6D, //rhythm 1
	0x01, 0x01, 0x00, 0x00, 0xC8, 0xD8, 0xA7, 0x48, //rhythm 2
	0x05, 0x01, 0x00, 0x00, 0xF8, 0xAA, 0x59, 0x55  //rhythm 3
};



//*********************************************************
//  YMF281
//*********************************************************

//-------------------------------------------------
//  ymf281 - constructor
//-------------------------------------------------

ymf281::ymf281(ymfm_interface &intf, uint8_t const *instrument_data) :
	opll_base(intf, (instrument_data != nullptr) ? instrument_data : s_default_instruments)
{
};

// table below taken from https://github.com/plgDavid/misc/wiki/Copyright-free-OPLL(x)-ROM-patches
uint8_t const ymf281::s_default_instruments[] =
{
	// May 14th 2015 Hubert Lamontagne
	0x72, 0x21, 0x1A, 0x07, 0xF6, 0x64, 0x01, 0x16, // Clarinet ~~ Electric String 	Square wave with vibrato
	0x00, 0x10, 0x45, 0x00, 0xF6, 0x83, 0x73, 0x63, // Synth Bass ~~ Bow wow 	Triangular wave
	0x13, 0x01, 0x96, 0x00, 0xF1, 0xF4, 0x31, 0x23, // Piano ~~ Electric Guitar 	Despite of its name, same as Piano of YM2413.
	0x71, 0x21, 0x0B, 0x0F, 0xF9, 0x64, 0x70, 0x17, // Flute ~~ Organ 	Sine wave
	0x02, 0x21, 0x1E, 0x06, 0xF9, 0x76, 0x00, 0x28, // Square Wave ~~ Clarinet 	Same as ones of YM2413.
	0x00, 0x61, 0x82, 0x0E, 0xF9, 0x61, 0x20, 0x27, // Space Oboe ~~ Saxophone 	Saw wave with vibrato
	0x21, 0x61, 0x1B, 0x07, 0x84, 0x8F, 0x10, 0x07, // Trumpet ~~ Trumpet 	Same as ones of YM2413.
	0x37, 0x32, 0xCA, 0x02, 0x66, 0x64, 0x47, 0x29, // Wow Bell ~~ Street Organ 	Calliope
	0x41, 0x41, 0x07, 0x03, 0xF5, 0x70, 0x51, 0xF5, // Electric Guitar ~~ Synth Brass 	Same as Synthesizer of YM2413.
	0x36, 0x01, 0x5E, 0x07, 0xF2, 0xF3, 0xF7, 0xF7, // Vibes ~~ Electric Piano 	Simulate of Rhodes Piano
	0x00, 0x00, 0x18, 0x06, 0xC5, 0xF3, 0x20, 0xF2, // Bass ~~ Bass 	Electric bass
	0x17, 0x81, 0x25, 0x07, 0xF7, 0xF3, 0x21, 0xF7, // Vibraphone ~~ Vibraphone	Same as ones of YM2413.
	0x35, 0x64, 0x00, 0x00, 0xFF, 0xF3, 0x77, 0xF5, // Vibrato Bell ~~ Chime 	Bell
	0x11, 0x31, 0x00, 0x07, 0xDD, 0xF3, 0xFF, 0xFB, // Click Sine ~~ Tom Tom II 	Tom
	0x3A, 0x21, 0x00, 0x07, 0x95, 0x84, 0x0F, 0xF5, // Noise and Tone ~~ Noise 	for S.E.
	0x01, 0x01, 0x18, 0x0F, 0xDF, 0xF8, 0x6A, 0x6D, //rhythm 1
	0x01, 0x01, 0x00, 0x00, 0xC8, 0xD8, 0xA7, 0x48, //rhythm 2
	0x05, 0x01, 0x00, 0x00, 0xF8, 0xAA, 0x59, 0x55  //rhythm 3
};



//*********************************************************
//  DS1001
//*********************************************************

//-------------------------------------------------
//  ds1001 - constructor
//-------------------------------------------------

ds1001::ds1001(ymfm_interface &intf, uint8_t const *instrument_data) :
	opll_base(intf, (instrument_data != nullptr) ? instrument_data : s_default_instruments)
{
};

// table below taken from https://github.com/plgDavid/misc/wiki/Copyright-free-OPLL(x)-ROM-patches
uint8_t const ds1001::s_default_instruments[] =
{
	// May 15th 2015 Hubert Lamontagne & David Viens
	0x03, 0x21, 0x05, 0x06, 0xC8, 0x81, 0x42, 0x27, // Buzzy Bell
	0x13, 0x41, 0x14, 0x0D, 0xF8, 0xF7, 0x23, 0x12, // Guitar
	0x31, 0x11, 0x08, 0x08, 0xFA, 0xC2, 0x28, 0x22, // Wurly
	0x31, 0x61, 0x0C, 0x07, 0xF8, 0x64, 0x60, 0x27, // Flute
	0x22, 0x21, 0x1E, 0x06, 0xFF, 0x76, 0x00, 0x28, // Clarinet
	0x02, 0x01, 0x05, 0x00, 0xAC, 0xF2, 0x03, 0x02, // Synth
	0x21, 0x61, 0x1D, 0x07, 0x82, 0x8F, 0x10, 0x07, // Trumpet
	0x23, 0x21, 0x22, 0x17, 0xFF, 0x73, 0x00, 0x17, // Organ
	0x15, 0x11, 0x25, 0x00, 0x41, 0x71, 0x00, 0xF1, // Bells
	0x95, 0x01, 0x10, 0x0F, 0xB8, 0xAA, 0x50, 0x02, // Vibes
	0x17, 0xC1, 0x5E, 0x07, 0xFA, 0xF8, 0x22, 0x12, // Vibraphone
	0x71, 0x23, 0x11, 0x06, 0x65, 0x74, 0x10, 0x16, // Tutti
	0x01, 0x02, 0xD3, 0x05, 0xF3, 0x92, 0x83, 0xF2, // Fretless
	0x61, 0x63, 0x0C, 0x00, 0xA4, 0xFF, 0x30, 0x06, // Synth Bass
	0x21, 0x62, 0x0D, 0x00, 0xA1, 0xFF, 0x50, 0x08, // Sweep
	0x01, 0x01, 0x18, 0x0F, 0xDF, 0xF8, 0x6A, 0x6D, //rhythm 1
	0x01, 0x01, 0x00, 0x00, 0xC8, 0xD8, 0xA7, 0x48, //rhythm 2
	0x05, 0x01, 0x00, 0x00, 0xF8, 0xAA, 0x59, 0x55  //rhythm 3
};


//*********************************************************
//  EXPLICIT INSTANTIATION
//*********************************************************

template class opl_registers_base<4>;
template class fm_engine_base<opl_registers_base<4>>;

}
