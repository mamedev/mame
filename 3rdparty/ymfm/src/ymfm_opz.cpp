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

#include "ymfm_opz.h"
#include "ymfm_fm.ipp"

#define TEMPORARY_DEBUG_PRINTS (0)

//
// OPZ (aka YM2414)
//
// This chip is not officially documented as far as I know. What I have
// comes from this site:
//
//    http://sr4.sakura.ne.jp/fmsound/opz.html
//
// and from reading the TX81Z operator manual, which describes how a number
// of these new features work.
//
// OPZ appears be bsaically OPM with a bunch of extra features.
//
// For starters, there are two LFO generators. I have presumed that they
// operate identically since identical parameters are offered for each. I
// have also presumed the effects are additive between them. The LFOs on
// the OPZ have an extra "sync" option which apparently causes the LFO to
// reset whenever a key on is received.
//
// At the channel level, there is an additional 8-bit volume control. This
// might work as an addition to total level, or some other way. Completely
// unknown, and unimplemented.
//
// At the operator level, there are a number of extra features. First, there
// are 8 different waveforms to choose from. These are different than the
// waveforms introduced in the OPL2 and later chips.
//
// Second, there is an additional "reverb" stage added to the envelope
// generator, which kicks in when the envelope reaches -18dB. It specifies
// a slower decay rate to produce a sort of faux reverb effect.
//
// The envelope generator also supports a 2-bit shift value, which can be
// used to reduce the effect of the envelope attenuation.
//
// OPZ supports a "fixed frequency" mode for each operator, with a 3-bit
// range and 4-bit frequency value, plus a 1-bit enable. Not sure how that
// works at all, so it's not implemented.
//
// There are also several mystery fields in the operators which I have no
// clue about: "fine" (4 bits), "eg_shift" (2 bits), and "rev" (3 bits).
// eg_shift is some kind of envelope generator effect, but how it works is
// unknown.
//
// Also, according to the site above, the panning controls are changed from
// OPM, with a "mono" bit and only one control bit for the right channel.
// Current implementation is just a guess.
//

namespace ymfm
{

//*********************************************************
//  OPZ REGISTERS
//*********************************************************

//-------------------------------------------------
//  opz_registers - constructor
//-------------------------------------------------

opz_registers::opz_registers() :
	m_lfo_counter{ 0, 0 },
	m_noise_lfsr(1),
	m_noise_counter(0),
	m_noise_state(0),
	m_noise_lfo(0),
	m_lfo_am{ 0, 0 }
{
	// create the waveforms
	for (uint32_t index = 0; index < WAVEFORM_LENGTH; index++)
		m_waveform[0][index] = abs_sin_attenuation(index) | (bitfield(index, 9) << 15);

	// we only have the diagrams to judge from, but suspecting waveform 1 (and
	// derived waveforms) are sin^2, based on OPX description of similar wave-
	// forms; since our sin table is logarithmic, this ends up just being
	// 2*existing value
	uint16_t zeroval = m_waveform[0][0];
	for (uint32_t index = 0; index < WAVEFORM_LENGTH; index++)
		m_waveform[1][index] = std::min<uint16_t>(2 * (m_waveform[0][index] & 0x7fff), zeroval) | (bitfield(index, 9) << 15);

	// remaining waveforms are just derivations of the 2 main ones
	for (uint32_t index = 0; index < WAVEFORM_LENGTH; index++)
	{
		m_waveform[2][index] = bitfield(index, 9) ? zeroval : m_waveform[0][index];
		m_waveform[3][index] = bitfield(index, 9) ? zeroval : m_waveform[1][index];
		m_waveform[4][index] = bitfield(index, 9) ? zeroval : m_waveform[0][index * 2];
		m_waveform[5][index] = bitfield(index, 9) ? zeroval : m_waveform[1][index * 2];
		m_waveform[6][index] = bitfield(index, 9) ? zeroval : m_waveform[0][(index * 2) & 0x1ff];
		m_waveform[7][index] = bitfield(index, 9) ? zeroval : m_waveform[1][(index * 2) & 0x1ff];
	}

	// create the LFO waveforms; AM in the low 8 bits, PM in the upper 8
	// waveforms are adjusted to match the pictures in the application manual
	for (uint32_t index = 0; index < LFO_WAVEFORM_LENGTH; index++)
	{
		// waveform 0 is a sawtooth
		uint8_t am = index ^ 0xff;
		uint8_t pm = index;
		m_lfo_waveform[0][index] = am | (pm << 8);

		// waveform 1 is a square wave
		am = bitfield(index, 7) ? 0 : 0xff;
		pm = am ^ 0x80;
		m_lfo_waveform[1][index] = am | (pm << 8);

		// waveform 2 is a triangle wave
		am = bitfield(index, 7) ? (index << 1) : ((index ^ 0xff) << 1);
		pm = bitfield(index, 6) ? am : ~am;
		m_lfo_waveform[2][index] = am | (pm << 8);

		// waveform 3 is noise; it is filled in dynamically
	}
}


//-------------------------------------------------
//  reset - reset to initial state
//-------------------------------------------------

void opz_registers::reset()
{
	std::fill_n(&m_regdata[0], REGISTERS, 0);

	// enable output on both channels by default
	m_regdata[0x30] = m_regdata[0x31] = m_regdata[0x32] = m_regdata[0x33] = 0x01;
	m_regdata[0x34] = m_regdata[0x35] = m_regdata[0x36] = m_regdata[0x37] = 0x01;
}


//-------------------------------------------------
//  save_restore - save or restore the data
//-------------------------------------------------

void opz_registers::save_restore(ymfm_saved_state &state)
{
	state.save_restore(m_lfo_counter);
	state.save_restore(m_lfo_am);
	state.save_restore(m_noise_lfsr);
	state.save_restore(m_noise_counter);
	state.save_restore(m_noise_state);
	state.save_restore(m_noise_lfo);
	state.save_restore(m_regdata);
	state.save_restore(m_phase_substep);
}


//-------------------------------------------------
//  operator_map - return an array of operator
//  indices for each channel; for OPZ this is fixed
//-------------------------------------------------

void opz_registers::operator_map(operator_mapping &dest) const
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

bool opz_registers::write(uint16_t index, uint8_t data, uint32_t &channel, uint32_t &opmask)
{
	assert(index < REGISTERS);

	// special mappings:
	//   0x16 -> 0x188 if bit 7 is set
	//   0x19 -> 0x189 if bit 7 is set
	//   0x38..0x3F -> 0x180..0x187 if bit 7 is set
	//   0x40..0x5F -> 0x100..0x11F if bit 7 is set
	//   0xC0..0xDF -> 0x120..0x13F if bit 5 is set
	if (index == 0x17 && bitfield(data, 7) != 0)
		m_regdata[0x188] = data;
	else if (index == 0x19 && bitfield(data, 7) != 0)
		m_regdata[0x189] = data;
	else if ((index & 0xf8) == 0x38 && bitfield(data, 7) != 0)
		m_regdata[0x180 + (index & 7)] = data;
	else if ((index & 0xe0) == 0x40 && bitfield(data, 7) != 0)
		m_regdata[0x100 + (index & 0x1f)] = data;
	else if ((index & 0xe0) == 0xc0 && bitfield(data, 5) != 0)
		m_regdata[0x120 + (index & 0x1f)] = data;
	else if (index < 0x100)
		m_regdata[index] = data;

	// preset writes restore some values from a preset memory; not sure
	// how this really works but the TX81Z will overwrite the sustain level/
	// release rate register and the envelope shift/reverb rate register to
	// dampen sound, then write the preset number to register 8 to restore them
	if (index == 0x08)
	{
		int chan = bitfield(data, 0, 3);
		if (TEMPORARY_DEBUG_PRINTS)
			printf("Loading preset %d\n", chan);
		m_regdata[0xe0 + chan + 0] = m_regdata[0x140 + chan + 0];
		m_regdata[0xe0 + chan + 8] = m_regdata[0x140 + chan + 8];
		m_regdata[0xe0 + chan + 16] = m_regdata[0x140 + chan + 16];
		m_regdata[0xe0 + chan + 24] = m_regdata[0x140 + chan + 24];
		m_regdata[0x120 + chan + 0] = m_regdata[0x160 + chan + 0];
		m_regdata[0x120 + chan + 8] = m_regdata[0x160 + chan + 8];
		m_regdata[0x120 + chan + 16] = m_regdata[0x160 + chan + 16];
		m_regdata[0x120 + chan + 24] = m_regdata[0x160 + chan + 24];
	}

	// store the presets under some unknown condition; the pattern of writes
	// when setting a new preset is:
	//
	//   08 (0-7), 80-9F, A0-BF, C0-DF, C0-DF (alt), 20-27, 40-5F, 40-5F (alt),
	//   C0-DF (alt -- again?), 38-3F, 1B, 18, E0-FF
	//
	// So it writes 0-7 to 08 to either reset all presets or to indicate
	// that we're going to be loading them. Immediately after all the writes
	// above, the very next write will be temporary values to blow away the
	// values loaded into E0-FF, so somehow it also knows that anything after
	// that point is not part of the preset.
	//
	// For now, try using the 40-5F (alt) writes as flags that presets are
	// being loaded until the E0-FF writes happen.
	bool is_setting_preset = (bitfield(m_regdata[0x100 + (index & 0x1f)], 7) != 0);
	if (is_setting_preset)
	{
		if ((index & 0xe0) == 0xe0)
		{
			m_regdata[0x140 + (index & 0x1f)] = data;
			m_regdata[0x100 + (index & 0x1f)] &= 0x7f;
		}
		else if ((index & 0xe0) == 0xc0 && bitfield(data, 5) != 0)
			m_regdata[0x160 + (index & 0x1f)] = data;
	}

	// handle writes to the key on index
	if ((index & 0xf8) == 0x20 && bitfield(index, 0, 3) == bitfield(m_regdata[0x08], 0, 3))
	{
		channel = bitfield(index, 0, 3);
		opmask = ch_key_on(channel) ? 0xf : 0;

		// according to the TX81Z manual, the sync option causes the LFOs
		// to reset at each note on
		if (opmask != 0)
		{
			if (lfo_sync())
				m_lfo_counter[0] = 0;
			if (lfo2_sync())
				m_lfo_counter[1] = 0;
		}
		return true;
	}
	return false;
}


//-------------------------------------------------
//  clock_noise_and_lfo - clock the noise and LFO,
//  handling clock division, depth, and waveform
//  computations
//-------------------------------------------------

int32_t opz_registers::clock_noise_and_lfo()
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
	uint32_t rate0 = lfo_rate();
	uint32_t rate1 = lfo2_rate();
	m_lfo_counter[0] += (0x10 | bitfield(rate0, 0, 4)) << bitfield(rate0, 4, 4);
	m_lfo_counter[1] += (0x10 | bitfield(rate1, 0, 4)) << bitfield(rate1, 4, 4);
	uint32_t lfo0 = bitfield(m_lfo_counter[0], 22, 8);
	uint32_t lfo1 = bitfield(m_lfo_counter[1], 22, 8);

	// fill in the noise entry 1 ahead of our current position; this
	// ensures the current value remains stable for a full LFO clock
	// and effectively latches the running value when the LFO advances
	uint32_t lfo_noise = bitfield(m_noise_lfsr, 17, 8);
	m_lfo_waveform[3][(lfo0 + 1) & 0xff] = lfo_noise | (lfo_noise << 8);
	m_lfo_waveform[3][(lfo1 + 1) & 0xff] = lfo_noise | (lfo_noise << 8);

	// fetch the AM/PM values based on the waveform; AM is unsigned and
	// encoded in the low 8 bits, while PM signed and encoded in the upper
	// 8 bits
	int32_t ampm0 = m_lfo_waveform[lfo_waveform()][lfo0];
	int32_t ampm1 = m_lfo_waveform[lfo2_waveform()][lfo1];

	// apply depth to the AM values and store for later
	m_lfo_am[0] = ((ampm0 & 0xff) * lfo_am_depth()) >> 7;
	m_lfo_am[1] = ((ampm1 & 0xff) * lfo2_am_depth()) >> 7;

	// apply depth to the PM values and return them combined into two
	int32_t pm0 = ((ampm0 >> 8) * int32_t(lfo_pm_depth())) >> 7;
	int32_t pm1 = ((ampm1 >> 8) * int32_t(lfo2_pm_depth())) >> 7;
	return (pm0 & 0xff) | (pm1 << 8);
}


//-------------------------------------------------
//  lfo_am_offset - return the AM offset from LFO
//  for the given channel
//-------------------------------------------------

uint32_t opz_registers::lfo_am_offset(uint32_t choffs) const
{
	// not sure how this works for real, but just adding the two
	// AM LFOs together
	uint32_t result = 0;

	// shift value for AM sensitivity is [*, 0, 1, 2],
	// mapping to values of [0, 23.9, 47.8, and 95.6dB]
	uint32_t am_sensitivity = ch_lfo_am_sens(choffs);
	if (am_sensitivity != 0)
		result = m_lfo_am[0] << (am_sensitivity - 1);

	// QUESTION: see OPN note below for the dB range mapping; it applies
	// here as well

	// raw LFO AM value on OPZ is 0-FF, which is already a factor of 2
	// larger than the OPN below, putting our staring point at 2x theirs;
	// this works out since our minimum is 2x their maximum
	uint32_t am_sensitivity2 = ch_lfo2_am_sens(choffs);
	if (am_sensitivity2 != 0)
		result += m_lfo_am[1] << (am_sensitivity2 - 1);

	return result;
}


//-------------------------------------------------
//  cache_operator_data - fill the operator cache
//  with prefetched data
//-------------------------------------------------

void opz_registers::cache_operator_data(uint32_t choffs, uint32_t opoffs, opdata_cache &cache)
{
	// TODO: how does fixed frequency mode work? appears to be enabled by
	// op_fix_mode(), and controlled by op_fix_range(), op_fix_frequency()

	// TODO: what is op_rev()?

	// set up the easy stuff
	cache.waveform = &m_waveform[op_waveform(opoffs)][0];

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

	// multiple value, as an x.4 value (0 means 0.5)
	// the "fine" control provides the fractional bits
	cache.multiple = op_multiple(opoffs) << 4;
	if (cache.multiple == 0)
		cache.multiple = 0x08;
	cache.multiple |= op_fine(opoffs);

	// phase step, or PHASE_STEP_DYNAMIC if PM is active; this depends on
	// block_freq, detune, and multiple, so compute it after we've done those;
	// note that fix frequency mode is also treated as dynamic
	if (!op_fix_mode(opoffs) && (lfo_pm_depth() == 0 || ch_lfo_pm_sens(choffs) == 0) && (lfo2_pm_depth() == 0 || ch_lfo2_pm_sens(choffs) == 0))
		cache.phase_step = compute_phase_step(choffs, opoffs, cache, 0);
	else
		cache.phase_step = opdata_cache::PHASE_STEP_DYNAMIC;

	// total level, scaled by 8
	// TODO: how does ch_volume() fit into this?
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
	cache.eg_rate[EG_REVERB] = cache.eg_rate[EG_RELEASE];
	uint32_t reverb = op_reverb_rate(opoffs);
	if (reverb != 0)
		cache.eg_rate[EG_REVERB] = std::min<uint32_t>(effective_rate(reverb * 4 + 2, ksrval), cache.eg_rate[EG_REVERB]);

	// set the envelope shift; TX81Z manual says operator 1 shift is fixed at "off"
	cache.eg_shift = ((opoffs & 0x18) == 0) ? 0 : op_eg_shift(opoffs);
}


//-------------------------------------------------
//  compute_phase_step - compute the phase step
//-------------------------------------------------

uint32_t opz_registers::compute_phase_step(uint32_t choffs, uint32_t opoffs, opdata_cache const &cache, int32_t lfo_raw_pm)
{
	// OPZ has a fixed frequency mode; it is unclear whether the
	// detune and multiple parameters affect things

	uint32_t phase_step;
	if (op_fix_mode(opoffs))
	{
		// the baseline frequency in hz comes from the fix frequency and fine
		// registers, which can specify values 8-255Hz in 1Hz increments; that
		// value is then shifted up by the 3-bit range
		uint32_t freq = op_fix_frequency(opoffs) << 4;
		if (freq == 0)
			freq = 8;
		freq |= op_fine(opoffs);
		freq <<= op_fix_range(opoffs);

		// there is not enough resolution in the plain phase step to track the
		// full range of frequencies, so we keep a per-operator sub step with an
		// additional 12 bits of resolution; this calculation gives us, for
		// example, a frequency of 8.0009Hz when 8Hz is requested
		uint32_t substep = m_phase_substep[opoffs];
		substep += 75 * freq;
		phase_step = substep >> 12;
		m_phase_substep[opoffs] = substep & 0xfff;

		// detune/multiple occupy the same space as fix_range/fix_frequency so
		// don't apply them in addition
		return phase_step;
	}
	else
	{
		// start with coarse detune delta; table uses cents value from
		// manual, converted into 1/64ths
		static const int16_t s_detune2_delta[4] = { 0, (600*64+50)/100, (781*64+50)/100, (950*64+50)/100 };
		int32_t delta = s_detune2_delta[op_detune2(opoffs)];

		// add in the PM deltas
		uint32_t pm_sensitivity = ch_lfo_pm_sens(choffs);
		if (pm_sensitivity != 0)
		{
			// raw PM value is -127..128 which is +/- 200 cents
			// manual gives these magnitudes in cents:
			//    0, +/-5, +/-10, +/-20, +/-50, +/-100, +/-400, +/-700
			// this roughly corresponds to shifting the 200-cent value:
			//    0  >> 5,  >> 4,  >> 3,  >> 2,  >> 1,   << 1,   << 2
			if (pm_sensitivity < 6)
				delta += int8_t(lfo_raw_pm) >> (6 - pm_sensitivity);
			else
				delta += int8_t(lfo_raw_pm) << (pm_sensitivity - 5);
		}
		uint32_t pm_sensitivity2 = ch_lfo2_pm_sens(choffs);
		if (pm_sensitivity2 != 0)
		{
			// raw PM value is -127..128 which is +/- 200 cents
			// manual gives these magnitudes in cents:
			//    0, +/-5, +/-10, +/-20, +/-50, +/-100, +/-400, +/-700
			// this roughly corresponds to shifting the 200-cent value:
			//    0  >> 5,  >> 4,  >> 3,  >> 2,  >> 1,   << 1,   << 2
			if (pm_sensitivity2 < 6)
				delta += int8_t(lfo_raw_pm >> 8) >> (6 - pm_sensitivity2);
			else
				delta += int8_t(lfo_raw_pm >> 8) << (pm_sensitivity2 - 5);
		}

		// apply delta and convert to a frequency number; this translation is
		// the same as OPM so just re-use that helper
		phase_step = opm_key_code_to_phase_step(cache.block_freq, delta);

		// apply detune based on the keycode
		phase_step += cache.detune;

		// apply frequency multiplier (which is cached as an x.4 value)
		return (phase_step * cache.multiple) >> 4;
	}
}


//-------------------------------------------------
//  log_keyon - log a key-on event
//-------------------------------------------------

std::string opz_registers::log_keyon(uint32_t choffs, uint32_t opoffs)
{
	uint32_t chnum = choffs;
	uint32_t opnum = opoffs;

	char buffer[256];
	int end = 0;

	end += snprintf(&buffer[end], sizeof(buffer) - end, "%u.%02u", chnum, opnum);

	if (op_fix_mode(opoffs))
		end += snprintf(&buffer[end], sizeof(buffer) - end, " fixfreq=%X fine=%X shift=%X", op_fix_frequency(opoffs), op_fine(opoffs), op_fix_range(opoffs));
	else
		end += snprintf(&buffer[end], sizeof(buffer) - end, " freq=%04X dt2=%u fine=%X", ch_block_freq(choffs), op_detune2(opoffs), op_fine(opoffs));

	end += snprintf(&buffer[end], sizeof(buffer) - end, " dt=%u fb=%u alg=%X mul=%X tl=%02X ksr=%u adsr=%02X/%02X/%02X/%X sl=%X out=%c%c",
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

	if (op_eg_shift(opoffs) != 0)
		end += snprintf(&buffer[end], sizeof(buffer) - end, " egshift=%u", op_eg_shift(opoffs));

	bool am = (lfo_am_depth() != 0 && ch_lfo_am_sens(choffs) != 0 && op_lfo_am_enable(opoffs) != 0);
	if (am)
		end += snprintf(&buffer[end], sizeof(buffer) - end, " am=%u/%02X", ch_lfo_am_sens(choffs), lfo_am_depth());
	bool pm = (lfo_pm_depth() != 0 && ch_lfo_pm_sens(choffs) != 0);
	if (pm)
		end += snprintf(&buffer[end], sizeof(buffer) - end, " pm=%u/%02X", ch_lfo_pm_sens(choffs), lfo_pm_depth());
	if (am || pm)
		end += snprintf(&buffer[end], sizeof(buffer) - end, " lfo=%02X/%c", lfo_rate(), "WQTN"[lfo_waveform()]);

	bool am2 = (lfo2_am_depth() != 0 && ch_lfo2_am_sens(choffs) != 0 && op_lfo_am_enable(opoffs) != 0);
	if (am2)
		end += snprintf(&buffer[end], sizeof(buffer) - end, " am2=%u/%02X", ch_lfo2_am_sens(choffs), lfo2_am_depth());
	bool pm2 = (lfo2_pm_depth() != 0 && ch_lfo2_pm_sens(choffs) != 0);
	if (pm2)
		end += snprintf(&buffer[end], sizeof(buffer) - end, " pm2=%u/%02X", ch_lfo2_pm_sens(choffs), lfo2_pm_depth());
	if (am2 || pm2)
		end += snprintf(&buffer[end], sizeof(buffer) - end, " lfo2=%02X/%c", lfo2_rate(), "WQTN"[lfo2_waveform()]);

	if (op_reverb_rate(opoffs) != 0)
		end += snprintf(&buffer[end], sizeof(buffer) - end, " rev=%u", op_reverb_rate(opoffs));
	if (op_waveform(opoffs) != 0)
		end += snprintf(&buffer[end], sizeof(buffer) - end, " wf=%u", op_waveform(opoffs));
	if (noise_enable() && opoffs == 31)
		end += snprintf(&buffer[end], sizeof(buffer) - end, " noise=1");

	return buffer;
}



//*********************************************************
//  YM2414
//*********************************************************

//-------------------------------------------------
//  ym2414 - constructor
//-------------------------------------------------

ym2414::ym2414(ymfm_interface &intf) :
	m_address(0),
	m_fm(intf)
{
}


//-------------------------------------------------
//  reset - reset the system
//-------------------------------------------------

void ym2414::reset()
{
	// reset the engines
	m_fm.reset();
}


//-------------------------------------------------
//  save_restore - save or restore the data
//-------------------------------------------------

void ym2414::save_restore(ymfm_saved_state &state)
{
	m_fm.save_restore(state);
	state.save_restore(m_address);
}


//-------------------------------------------------
//  read_status - read the status register
//-------------------------------------------------

uint8_t ym2414::read_status()
{
	uint8_t result = m_fm.status();
	if (m_fm.intf().ymfm_is_busy())
		result |= fm_engine::STATUS_BUSY;
	return result;
}


//-------------------------------------------------
//  read - handle a read from the device
//-------------------------------------------------

uint8_t ym2414::read(uint32_t offset)
{
	uint8_t result = 0xff;
	switch (offset & 1)
	{
		case 0: // data port (unused)
			debug::log_unexpected_read_write("Unexpected read from YM2414 offset %d\n", offset & 3);
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

void ym2414::write_address(uint8_t data)
{
	// just set the address
	m_address = data;
}


//-------------------------------------------------
//  write - handle a write to the register
//  interface
//-------------------------------------------------

void ym2414::write_data(uint8_t data)
{
	// write the FM register
	m_fm.write(m_address, data);
	if (TEMPORARY_DEBUG_PRINTS)
	{
		switch (m_address & 0xe0)
		{
			case 0x00:
				printf("CTL %02X = %02X\n", m_address, data);
				break;

			case 0x20:
				switch (m_address & 0xf8)
				{
					case 0x20:	printf("R/FBL/ALG %d = %02X\n", m_address & 7, data);	break;
					case 0x28:	printf("KC %d = %02X\n", m_address & 7, data);	break;
					case 0x30:	printf("KF/M %d = %02X\n", m_address & 7, data);	break;
					case 0x38:	printf("PMS/AMS %d = %02X\n", m_address & 7, data); break;
				}
				break;

			case 0x40:
				if (bitfield(data, 7) == 0)
					printf("DT1/MUL %d.%d = %02X\n", m_address & 7, (m_address >> 3) & 3, data);
				else
					printf("OW/FINE %d.%d = %02X\n", m_address & 7, (m_address >> 3) & 3, data);
				break;

			case 0x60:
				printf("TL %d.%d = %02X\n", m_address & 7, (m_address >> 3) & 3, data);
				break;

			case 0x80:
				printf("KRS/FIX/AR %d.%d = %02X\n", m_address & 7, (m_address >> 3) & 3, data);
				break;

			case 0xa0:
				printf("A/D1R %d.%d = %02X\n", m_address & 7, (m_address >> 3) & 3, data);
				break;

			case 0xc0:
				if (bitfield(data, 5) == 0)
					printf("DT2/D2R %d.%d = %02X\n", m_address & 7, (m_address >> 3) & 3, data);
				else
					printf("EGS/REV %d.%d = %02X\n", m_address & 7, (m_address >> 3) & 3, data);
				break;

			case 0xe0:
				printf("D1L/RR %d.%d = %02X\n", m_address & 7, (m_address >> 3) & 3, data);
				break;
		}
	}

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

void ym2414::write(uint32_t offset, uint8_t data)
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

void ym2414::generate(output_data *output, uint32_t numsamples)
{
	for (uint32_t samp = 0; samp < numsamples; samp++, output++)
	{
		// clock the system
		m_fm.clock(fm_engine::ALL_CHANNELS);

		// update the FM content; YM2414 is full 14-bit with no intermediate clipping
		m_fm.output(output->clear(), 0, 32767, fm_engine::ALL_CHANNELS);

		// unsure about YM2414 outputs; assume it is like YM2151
		output->roundtrip_fp();
	}
}

}
