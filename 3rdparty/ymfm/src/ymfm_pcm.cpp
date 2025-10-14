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

#include "ymfm_pcm.h"
#include "ymfm_fm.h"
#include "ymfm_fm.ipp"

namespace ymfm
{

//*********************************************************
// PCM REGISTERS
//*********************************************************

//-------------------------------------------------
//  reset - reset the register state
//-------------------------------------------------

void pcm_registers::reset()
{
	std::fill_n(&m_regdata[0], REGISTERS, 0);
	m_regdata[0xf8] = 0x1b;
}


//-------------------------------------------------
//  save_restore - save or restore the data
//-------------------------------------------------

void pcm_registers::save_restore(ymfm_saved_state &state)
{
	state.save_restore(m_regdata);
}


//-------------------------------------------------
//  cache_channel_data - update the cache with
//  data from the registers
//-------------------------------------------------

void pcm_registers::cache_channel_data(uint32_t choffs, pcm_cache &cache)
{
	// compute step from octave and fnumber; the math here implies
	// a .18 fraction but .16 should be perfectly fine
	int32_t octave = int8_t(ch_octave(choffs) << 4) >> 4;
	uint32_t fnum = ch_fnumber(choffs);
	cache.step = ((0x400 | fnum) << (octave + 7)) >> 2;

	// total level is computed as a .10 value for interpolation
	cache.total_level = ch_total_level(choffs) << 10;

	// compute panning values in terms of envelope attenuation
	int32_t panpot = int8_t(ch_panpot(choffs) << 4) >> 4;
	if (panpot >= 0)
	{
		cache.pan_left = (panpot == 7) ? 0x3ff : 0x20 * panpot;
		cache.pan_right = 0;
	}
	else if (panpot >= -7)
	{
		cache.pan_left = 0;
		cache.pan_right = (panpot == -7) ? 0x3ff : -0x20 * panpot;
	}
	else
		cache.pan_left = cache.pan_right = 0x3ff;

	// determine the LFO stepping value; this how much to add to a running
	// x.18 value for the LFO; steps were derived from frequencies in the
	// manual and come out very close with these values
	static const uint8_t s_lfo_steps[8] = { 1, 12, 19, 25, 31, 35, 37, 42 };
	cache.lfo_step = s_lfo_steps[ch_lfo_speed(choffs)];

	// AM LFO depth values, derived from the manual; note each has at most
	// 2 bits to make the "multiply" easy in hardware
	static const uint8_t s_am_depth[8] = { 0, 0x14, 0x20, 0x28, 0x30, 0x40, 0x50, 0x80 };
	cache.am_depth = s_am_depth[ch_am_depth(choffs)];

	// PM LFO depth values; these are converted from the manual's cents values
	// into f-numbers; the computations come out quite cleanly so pretty sure
	// these are correct
	static const uint8_t s_pm_depth[8] = { 0, 2, 3, 4, 6, 12, 24, 48 };
	cache.pm_depth = s_pm_depth[ch_vibrato(choffs)];

	// 4-bit sustain level, but 15 means 31 so effectively 5 bits
	cache.eg_sustain = ch_sustain_level(choffs);
	cache.eg_sustain |= (cache.eg_sustain + 1) & 0x10;
	cache.eg_sustain <<= 5;

	// compute the key scaling correction factor; 15 means don't do any correction
	int32_t correction = ch_rate_correction(choffs);
	if (correction == 15)
		correction = 0;
	else
		correction = (octave + correction) * 2 + bitfield(fnum, 9);

	// compute the envelope generator rates
	cache.eg_rate[EG_ATTACK] = effective_rate(ch_attack_rate(choffs), correction);
	cache.eg_rate[EG_DECAY] = effective_rate(ch_decay_rate(choffs), correction);
	cache.eg_rate[EG_SUSTAIN] = effective_rate(ch_sustain_rate(choffs), correction);
	cache.eg_rate[EG_RELEASE] = effective_rate(ch_release_rate(choffs), correction);
	cache.eg_rate[EG_REVERB] = 5;

	// if damping is on, override some things; essentially decay at a hardcoded
	// rate of 48 until -12db (0x80), then at maximum rate for the rest
	if (ch_damp(choffs) != 0)
	{
		cache.eg_rate[EG_DECAY] = 48;
		cache.eg_rate[EG_SUSTAIN] = 63;
		cache.eg_rate[EG_RELEASE] = 63;
		cache.eg_sustain = 0x80;
	}
}


//-------------------------------------------------
//  effective_rate - return the effective rate,
//  clamping and applying corrections as needed
//-------------------------------------------------

uint32_t pcm_registers::effective_rate(uint32_t raw, uint32_t correction)
{
	// raw rates of 0 and 15 just pin to min/max
	if (raw == 0)
		return 0;
	if (raw == 15)
		return 63;

	// otherwise add the correction and clamp to range
	return clamp(raw * 4 + correction, 0, 63);
}



//*********************************************************
// PCM CHANNEL
//*********************************************************

//-------------------------------------------------
//  pcm_channel - constructor
//-------------------------------------------------

pcm_channel::pcm_channel(pcm_engine &owner, uint32_t choffs) :
	m_choffs(choffs),
	m_baseaddr(0),
	m_endpos(0),
	m_looppos(0),
	m_curpos(0),
	m_nextpos(0),
	m_lfo_counter(0),
	m_eg_state(EG_RELEASE),
	m_env_attenuation(0x3ff),
	m_total_level(0x7f << 10),
	m_format(0),
	m_key_state(0),
	m_regs(owner.regs()),
	m_owner(owner)
{
}


//-------------------------------------------------
//  reset - reset the channel state
//-------------------------------------------------

void pcm_channel::reset()
{
	m_baseaddr = 0;
	m_endpos = 0;
	m_looppos = 0;
	m_curpos = 0;
	m_nextpos = 0;
	m_lfo_counter = 0;
	m_eg_state = EG_RELEASE;
	m_env_attenuation = 0x3ff;
	m_total_level = 0x7f << 10;
	m_format = 0;
	m_key_state = 0;
}


//-------------------------------------------------
//  save_restore - save or restore the data
//-------------------------------------------------

void pcm_channel::save_restore(ymfm_saved_state &state)
{
	state.save_restore(m_baseaddr);
	state.save_restore(m_endpos);
	state.save_restore(m_looppos);
	state.save_restore(m_curpos);
	state.save_restore(m_nextpos);
	state.save_restore(m_lfo_counter);
	state.save_restore(m_eg_state);
	state.save_restore(m_env_attenuation);
	state.save_restore(m_total_level);
	state.save_restore(m_format);
	state.save_restore(m_key_state);
}


//-------------------------------------------------
//  prepare - prepare for clocking
//-------------------------------------------------

bool pcm_channel::prepare()
{
	// cache the data
	m_regs.cache_channel_data(m_choffs, m_cache);

	// clock the key state
	if ((m_key_state & KEY_PENDING) != 0)
	{
		uint8_t oldstate = m_key_state;
		m_key_state = (m_key_state >> 1) & KEY_ON;
		if (((oldstate ^ m_key_state) & KEY_ON) != 0)
		{
			if ((m_key_state & KEY_ON) != 0)
				start_attack();
			else
				start_release();
		}
	}

	// set the total level directly if not interpolating
	if (m_regs.ch_level_direct(m_choffs))
		m_total_level = m_cache.total_level;

	// we're active until we're quiet after the release
	return (m_eg_state < EG_RELEASE || m_env_attenuation < EG_QUIET);
}


//-------------------------------------------------
//  clock - master clocking function
//-------------------------------------------------

void pcm_channel::clock(uint32_t env_counter)
{
	// clock the LFO, which is an x.18 value incremented based on the
	// LFO speed value
	m_lfo_counter += m_cache.lfo_step;

	// clock the envelope
	clock_envelope(env_counter);

	// determine the step after applying vibrato
	uint32_t step = m_cache.step;
	if (m_cache.pm_depth != 0)
	{
		// shift the LFO by 1/4 cycle for PM so that it starts at 0
		uint32_t lfo_shifted = m_lfo_counter + (1 << 16);
		int32_t lfo_value = bitfield(lfo_shifted, 10, 7);
		if (bitfield(lfo_shifted, 17) != 0)
			lfo_value ^= 0x7f;
		lfo_value -= 0x40;
		step += (lfo_value * int32_t(m_cache.pm_depth)) >> 7;
	}

	// advance the sample step and loop as needed
	m_curpos = m_nextpos;
	m_nextpos = m_curpos + step;
	if (m_nextpos >= m_endpos)
		m_nextpos += m_looppos - m_endpos;

	// interpolate total level if needed
	if (m_total_level != m_cache.total_level)
	{
		// max->min volume takes 156.4ms, or pretty close to 19/1024 per 44.1kHz sample
		// min->max volume is half that, so advance by 38/1024 per sample
		if (m_total_level < m_cache.total_level)
			m_total_level = std::min<int32_t>(m_total_level + 19, m_cache.total_level);
		else
			m_total_level = std::max<int32_t>(m_total_level - 38, m_cache.total_level);
	}
}


//-------------------------------------------------
//  output - return the computed output value, with
//  panning applied
//-------------------------------------------------

void pcm_channel::output(output_data &output) const
{
	// early out if the envelope is effectively off
	uint32_t envelope = m_env_attenuation;
	if (envelope > EG_QUIET)
		return;

	// add in LFO AM modulation
	if (m_cache.am_depth != 0)
	{
		uint32_t lfo_value = bitfield(m_lfo_counter, 10, 7);
		if (bitfield(m_lfo_counter, 17) != 0)
			lfo_value ^= 0x7f;
		envelope += (lfo_value * m_cache.am_depth) >> 7;
	}

	// add in the current interpolated total level value, which is a .10
	// value shifted left by 2
	envelope += m_total_level >> 8;

	// add in panning effect and clamp
	uint32_t lenv = std::min<uint32_t>(envelope + m_cache.pan_left, 0x3ff);
	uint32_t renv = std::min<uint32_t>(envelope + m_cache.pan_right, 0x3ff);

	// convert to volume as a .11 fraction
	int32_t lvol = attenuation_to_volume(lenv << 2);
	int32_t rvol = attenuation_to_volume(renv << 2);

	// fetch current sample and add
	int16_t sample = fetch_sample();
	uint32_t outnum = m_regs.ch_output_channel(m_choffs) * 2;
	output.data[outnum + 0] += (lvol * sample) >> 15;
	output.data[outnum + 1] += (rvol * sample) >> 15;
}


//-------------------------------------------------
//  keyonoff - signal key on/off
//-------------------------------------------------

void pcm_channel::keyonoff(bool on)
{
	// mark the key state as pending
	m_key_state |= KEY_PENDING | (on ? KEY_PENDING_ON : 0);

	// don't log masked channels
	if ((m_key_state & (KEY_PENDING_ON | KEY_ON)) == KEY_PENDING_ON && ((debug::GLOBAL_PCM_CHANNEL_MASK >> m_choffs) & 1) != 0)
	{
		debug::log_keyon("KeyOn PCM-%02d: num=%3d oct=%2d fnum=%03X level=%02X%c ADSR=%X/%X/%X/%X SL=%X",
			m_choffs,
			m_regs.ch_wave_table_num(m_choffs),
			int8_t(m_regs.ch_octave(m_choffs) << 4) >> 4,
			m_regs.ch_fnumber(m_choffs),
			m_regs.ch_total_level(m_choffs),
			m_regs.ch_level_direct(m_choffs) ? '!' : '/',
			m_regs.ch_attack_rate(m_choffs),
			m_regs.ch_decay_rate(m_choffs),
			m_regs.ch_sustain_rate(m_choffs),
			m_regs.ch_release_rate(m_choffs),
			m_regs.ch_sustain_level(m_choffs));

		if (m_regs.ch_rate_correction(m_choffs) != 15)
			debug::log_keyon(" RC=%X", m_regs.ch_rate_correction(m_choffs));

		if (m_regs.ch_pseudo_reverb(m_choffs) != 0)
			debug::log_keyon(" %s", "REV");
		if (m_regs.ch_damp(m_choffs) != 0)
			debug::log_keyon(" %s", "DAMP");

		if (m_regs.ch_vibrato(m_choffs) != 0 || m_regs.ch_am_depth(m_choffs) != 0)
		{
			if (m_regs.ch_vibrato(m_choffs) != 0)
				debug::log_keyon(" VIB=%d", m_regs.ch_vibrato(m_choffs));
			if (m_regs.ch_am_depth(m_choffs) != 0)
				debug::log_keyon(" AM=%d", m_regs.ch_am_depth(m_choffs));
			debug::log_keyon(" LFO=%d", m_regs.ch_lfo_speed(m_choffs));
		}
		debug::log_keyon("%s", "\n");
	}
}


//-------------------------------------------------
//  load_wavetable - load a wavetable by fetching
//  its data from external memory
//-------------------------------------------------

void pcm_channel::load_wavetable()
{
	// determine the address of the wave table header
	uint32_t wavnum = m_regs.ch_wave_table_num(m_choffs);
	uint32_t wavheader = 12 * wavnum;

	// above 384 it may be in a different bank
	if (wavnum >= 384)
	{
		uint32_t bank = m_regs.wave_table_header();
		if (bank != 0)
			wavheader = 512*1024 * bank + (wavnum - 384) * 12;
	}

	// fetch the 22-bit base address and 2-bit format
	uint8_t byte = read_pcm(wavheader + 0);
	m_format = bitfield(byte, 6, 2);
	m_baseaddr = bitfield(byte, 0, 6) << 16;
	m_baseaddr |= read_pcm(wavheader + 1) << 8;
	m_baseaddr |= read_pcm(wavheader + 2) << 0;

	// fetch the 16-bit loop position
	m_looppos = read_pcm(wavheader + 3) << 8;
	m_looppos |= read_pcm(wavheader + 4);
	m_looppos <<= 16;

	// fetch the 16-bit end position, which is stored as a negative value
	// for some reason that is unclear
	m_endpos = read_pcm(wavheader + 5) << 8;
	m_endpos |= read_pcm(wavheader + 6);
	m_endpos = -int32_t(m_endpos) << 16;

	// remaining data values set registers
	m_owner.write(0x80 + m_choffs, read_pcm(wavheader + 7));
	m_owner.write(0x98 + m_choffs, read_pcm(wavheader + 8));
	m_owner.write(0xb0 + m_choffs, read_pcm(wavheader + 9));
	m_owner.write(0xc8 + m_choffs, read_pcm(wavheader + 10));
	m_owner.write(0xe0 + m_choffs, read_pcm(wavheader + 11));

	// reset the envelope so we don't continue playing mid-sample from previous key ons
	m_env_attenuation = 0x3ff;
}


//-------------------------------------------------
//  read_pcm - read a byte from the external PCM
//  memory interface
//-------------------------------------------------

uint8_t pcm_channel::read_pcm(uint32_t address) const
{
	return m_owner.intf().ymfm_external_read(ACCESS_PCM, address);
}


//-------------------------------------------------
//  start_attack - start the attack phase
//-------------------------------------------------

void pcm_channel::start_attack()
{
	// don't change anything if already in attack state
	if (m_eg_state == EG_ATTACK)
		return;
	m_eg_state = EG_ATTACK;

	// reset the LFO if requested
	if (m_regs.ch_lfo_reset(m_choffs))
		m_lfo_counter = 0;

	// if the attack rate == 63 then immediately go to max attenuation
	if (m_cache.eg_rate[EG_ATTACK] == 63)
		m_env_attenuation = 0;

	// reset the positions
	m_curpos = m_nextpos = 0;
}


//-------------------------------------------------
//  start_release - start the release phase
//-------------------------------------------------

void pcm_channel::start_release()
{
	// don't change anything if already in release or reverb state
	if (m_eg_state >= EG_RELEASE)
		return;
	m_eg_state = EG_RELEASE;
}


//-------------------------------------------------
//  clock_envelope - clock the envelope generator
//-------------------------------------------------

void pcm_channel::clock_envelope(uint32_t env_counter)
{
	// handle attack->decay transitions
	if (m_eg_state == EG_ATTACK && m_env_attenuation == 0)
		m_eg_state = EG_DECAY;

	// handle decay->sustain transitions
	if (m_eg_state == EG_DECAY && m_env_attenuation >= m_cache.eg_sustain)
		m_eg_state = EG_SUSTAIN;

	// fetch the appropriate 6-bit rate value from the cache
	uint32_t rate = m_cache.eg_rate[m_eg_state];

	// compute the rate shift value; this is the shift needed to
	// apply to the env_counter such that it becomes a 5.11 fixed
	// point number
	uint32_t rate_shift = rate >> 2;
	env_counter <<= rate_shift;

	// see if the fractional part is 0; if not, it's not time to clock
	if (bitfield(env_counter, 0, 11) != 0)
		return;

	// determine the increment based on the non-fractional part of env_counter
	uint32_t relevant_bits = bitfield(env_counter, (rate_shift <= 11) ? 11 : rate_shift, 3);
	uint32_t increment = attenuation_increment(rate, relevant_bits);

	// attack is the only one that increases
	if (m_eg_state == EG_ATTACK)
		m_env_attenuation += (~m_env_attenuation * increment) >> 4;

	// all other cases are similar
	else
	{
		// apply the increment
		m_env_attenuation += increment;

		// clamp the final attenuation
		if (m_env_attenuation >= 0x400)
			m_env_attenuation = 0x3ff;

		// transition to reverb at -18dB if enabled
		if (m_env_attenuation >= 0xc0 && m_eg_state < EG_REVERB && m_regs.ch_pseudo_reverb(m_choffs))
			m_eg_state = EG_REVERB;
	}
}


//-------------------------------------------------
//  fetch_sample - fetch a sample at the current
//  position
//-------------------------------------------------

int16_t pcm_channel::fetch_sample() const
{
	uint32_t addr = m_baseaddr;
	uint32_t pos = m_curpos >> 16;

	// 8-bit PCM: shift up by 8
	if (m_format == 0)
		return read_pcm(addr + pos) << 8;

	// 16-bit PCM: assemble from 2 halves
	if (m_format == 2)
	{
		addr += pos * 2;
		return (read_pcm(addr) << 8) | read_pcm(addr + 1);
	}

	// 12-bit PCM: assemble out of half of 3 bytes
	addr += (pos / 2) * 3;
	if ((pos & 1) == 0)
		return (read_pcm(addr + 0) << 8) | ((read_pcm(addr + 1) << 4) & 0xf0);
	else
		return (read_pcm(addr + 2) << 8) | ((read_pcm(addr + 1) << 0) & 0xf0);
}



//*********************************************************
// PCM ENGINE
//*********************************************************

//-------------------------------------------------
//  pcm_engine - constructor
//-------------------------------------------------

pcm_engine::pcm_engine(ymfm_interface &intf) :
	m_intf(intf),
	m_env_counter(0),
	m_modified_channels(ALL_CHANNELS),
	m_active_channels(ALL_CHANNELS)
{
	// create the channels
	for (int chnum = 0; chnum < CHANNELS; chnum++)
		m_channel[chnum] = std::make_unique<pcm_channel>(*this, chnum);
}


//-------------------------------------------------
//  reset - reset the engine state
//-------------------------------------------------

void pcm_engine::reset()
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

void pcm_engine::save_restore(ymfm_saved_state &state)
{
	// save our data
	state.save_restore(m_env_counter);

	// save channel state
	for (int chnum = 0; chnum < CHANNELS; chnum++)
		m_channel[chnum]->save_restore(state);
}


//-------------------------------------------------
//  clock - master clocking function
//-------------------------------------------------

void pcm_engine::clock(uint32_t chanmask)
{
	// if something was modified, prepare
	// also prepare every 4k samples to catch ending notes
	if (m_modified_channels != 0 || m_prepare_count++ >= 4096)
	{
		// call each channel to prepare
		m_active_channels = 0;
		for (int chnum = 0; chnum < CHANNELS; chnum++)
			if (bitfield(chanmask, chnum))
				if (m_channel[chnum]->prepare())
					m_active_channels |= 1 << chnum;

		// reset the modified channels and prepare count
		m_modified_channels = m_prepare_count = 0;
	}

	// increment the envelope counter; the envelope generator
	// only clocks every other sample in order to make the PCM
	// envelopes line up with the FM envelopes (after taking into
	// account the different FM sampling rate)
	m_env_counter++;

	// now update the state of all the channels and operators
	for (int chnum = 0; chnum < CHANNELS; chnum++)
		if (bitfield(chanmask, chnum))
			m_channel[chnum]->clock(m_env_counter >> 1);
}


//-------------------------------------------------
//  update - master update function
//-------------------------------------------------

void pcm_engine::output(output_data &output, uint32_t chanmask)
{
	// mask out some channels for debug purposes
	chanmask &= debug::GLOBAL_PCM_CHANNEL_MASK;

	// compute the output of each channel
	for (int chnum = 0; chnum < CHANNELS; chnum++)
		if (bitfield(chanmask, chnum))
			m_channel[chnum]->output(output);
}


//-------------------------------------------------
//  read - handle reads from the PCM registers
//-------------------------------------------------

uint8_t pcm_engine::read(uint32_t regnum)
{
	// handle reads from the data register
	if (regnum == 0x06 && m_regs.memory_access_mode() != 0)
		return m_intf.ymfm_external_read(ACCESS_PCM, m_regs.memory_address_autoinc());

	return m_regs.read(regnum);
}


//-------------------------------------------------
//  write - handle writes to the PCM registers
//-------------------------------------------------

void pcm_engine::write(uint32_t regnum, uint8_t data)
{
	// handle reads to the data register
	if (regnum == 0x06 && m_regs.memory_access_mode() != 0)
	{
		m_intf.ymfm_external_write(ACCESS_PCM, m_regs.memory_address_autoinc(), data);
		return;
	}

	// for now just mark all channels as modified
	m_modified_channels = ALL_CHANNELS;

	// most writes are passive, consumed only when needed
	m_regs.write(regnum, data);

	// however, process keyons immediately
	if (regnum >= 0x68 && regnum <= 0x7f)
		m_channel[regnum - 0x68]->keyonoff(bitfield(data, 7));

	// and also wavetable writes
	else if (regnum >= 0x08 && regnum <= 0x1f)
		m_channel[regnum - 0x08]->load_wavetable();
}

}
