// license:BSD-3-Clause
// copyright-holders:Miguel Angel Horna

#include "emu.h"
#include "gew.h"
#include "wavwrite.h"

ALLOW_SAVE_TYPE(gew_pcm_device::state_t); // allow save_item on a non-fundamental type

/*******************************
        ENVELOPE SECTION
*******************************/

// Times are based on a 44100Hz timebase. It's adjusted to the actual sampling rate on startup

const double gew_pcm_device::BASE_TIMES[64] = {
	0,          0,          0,          0,
	6222.95,    4978.37,    4148.66,    3556.01,
	3111.47,    2489.21,    2074.33,    1778.00,
	1555.74,    1244.63,    1037.19,    889.02,
	777.87,     622.31,     518.59,     444.54,
	388.93,     311.16,     259.32,     222.27,
	194.47,     155.60,     129.66,     111.16,
	97.23,      77.82,      64.85,      55.60,
	48.62,      38.91,      32.43,      27.80,
	24.31,      19.46,      16.24,      13.92,
	12.15,      9.75,       8.12,       6.98,
	6.08,       4.90,       4.08,       3.49,
	3.04,       2.49,       2.13,       1.90,
	1.72,       1.41,       1.18,       1.04,
	0.91,       0.73,       0.59,       0.50,
	0.45,       0.45,       0.45,       0.45
};

constexpr uint32_t gew_pcm_device::TL_SHIFT;
constexpr uint32_t gew_pcm_device::EG_SHIFT;

void gew_pcm_device::retrigger_sample(slot_t &slot)
{
	slot.m_offset = 0;
	slot.m_prev_sample = 0;
	slot.m_total_level = slot.m_dest_total_level << TL_SHIFT;

	envelope_generator_calc(slot);
	slot.m_envelope_gen.m_state = state_t::ATTACK;
	slot.m_envelope_gen.m_volume = 0;

#if MULTIPCM_LOG_SAMPLES
	dump_sample(slot);
#endif
}

void  gew_pcm_device::update_step(slot_t &slot)
{
	const uint8_t oct = (slot.m_octave - 1) & 0xf;
	uint32_t pitch = m_freq_step_table[slot.m_pitch];
	if (oct & 0x8)
	{
		pitch >>= (16 - oct);
	}
	else
	{
		pitch <<= oct;
	}
	slot.m_step = pitch / m_rate;
}

void gew_pcm_device::envelope_generator_init(const double (&rates)[64], double attack_decay_ratio)
{
	for (int32_t i = 4; i < 0x40; ++i)
	{
		// Times are based on 44100Hz clock, adjust to real chip clock
		m_attack_step[i] = (float)(0x400 << EG_SHIFT) / (float)(rates[i] * 44100.0 / 1000.0);
		m_decay_release_step[i] = (float)(0x400 << EG_SHIFT) / (float)(rates[i] * attack_decay_ratio * 44100.0 / 1000.0);
	}
	m_attack_step[0] = m_attack_step[1] = m_attack_step[2] = m_attack_step[3] = 0;
	m_attack_step[0x3f] = 0x400 << EG_SHIFT;
	m_decay_release_step[0] = m_decay_release_step[1] = m_decay_release_step[2] = m_decay_release_step[3] = 0;
}

int32_t gew_pcm_device::envelope_generator_update(slot_t &slot)
{
	switch (slot.m_envelope_gen.m_state)
	{
	case state_t::ATTACK:
		slot.m_envelope_gen.m_volume += slot.m_envelope_gen.m_attack_rate;
		if (slot.m_envelope_gen.m_volume >= (0x3ff << EG_SHIFT))
		{
			slot.m_envelope_gen.m_state = state_t::DECAY1;
			if (slot.m_envelope_gen.m_decay1_rate >= (0x400 << EG_SHIFT)) //Skip DECAY1, go directly to DECAY2
			{
				slot.m_envelope_gen.m_state = state_t::DECAY2;
			}
			slot.m_envelope_gen.m_volume = 0x3ff << EG_SHIFT;
		}
		break;
	case state_t::DECAY1:
		slot.m_envelope_gen.m_volume -= slot.m_envelope_gen.m_decay1_rate;
		if (slot.m_envelope_gen.m_volume <= 0)
		{
			slot.m_envelope_gen.m_volume = 0;
		}
		if (slot.m_envelope_gen.m_volume >> (EG_SHIFT + 6) <= slot.m_envelope_gen.m_decay_level)
		{
			slot.m_envelope_gen.m_state = state_t::DECAY2;
		}
		break;
	case state_t::DECAY2:
		slot.m_envelope_gen.m_volume -= slot.m_envelope_gen.m_decay2_rate;
		if (slot.m_envelope_gen.m_volume <= 0)
		{
			slot.m_envelope_gen.m_volume = 0;
		}
		break;
	case state_t::RELEASE:
		slot.m_envelope_gen.m_volume -= slot.m_envelope_gen.m_release_rate;
		if (slot.m_envelope_gen.m_volume <= 0)
		{
			slot.m_envelope_gen.m_volume = 0;
			slot.m_playing = false;
		}
		break;
	default:
		return 1 << TL_SHIFT;
	}

	// TODO: this is currently only implemented for GEW7, it's probably not accurate
	if (slot.m_envelope_gen.m_reverb && slot.m_envelope_gen.m_state != state_t::ATTACK
		&& (slot.m_envelope_gen.m_volume >> EG_SHIFT) <= 0x300)
	{
		slot.m_envelope_gen.m_decay1_rate  = m_decay_release_step[17];
		slot.m_envelope_gen.m_decay2_rate  = m_decay_release_step[17];
		slot.m_envelope_gen.m_release_rate = m_decay_release_step[17];
	}

	return m_linear_to_exp_volume[slot.m_envelope_gen.m_volume >> EG_SHIFT];
}

uint32_t gew_pcm_device::get_rate(uint32_t *steps, int32_t rate, uint32_t val)
{
	if (val == 0)
	{
		return steps[0];
	}
	if (val == 0xf)
	{
		return steps[0x3f];
	}

	const int r = std::clamp(4 * (int)val + rate, 0, 0x3f);
	return steps[r];
}

void gew_pcm_device::envelope_generator_calc(slot_t &slot)
{
	int32_t octave = slot.m_octave;
	if (octave & 8) {
		octave = octave - 16;
	}

	int32_t rate;
	if (slot.m_sample.m_key_rate_scale != 0xf)
	{
		rate = (octave + slot.m_sample.m_key_rate_scale) * 2 + BIT(slot.m_pitch, 9);
	}
	else
	{
		rate = 0;
	}

	slot.m_envelope_gen.m_attack_rate = get_rate(m_attack_step.get(), rate, slot.m_sample.m_attack_reg);
	slot.m_envelope_gen.m_decay1_rate = get_rate(m_decay_release_step.get(), rate, slot.m_sample.m_decay1_reg);
	slot.m_envelope_gen.m_decay2_rate = get_rate(m_decay_release_step.get(), rate, slot.m_sample.m_decay2_reg);
	slot.m_envelope_gen.m_release_rate = get_rate(m_decay_release_step.get(), rate, slot.m_sample.m_release_reg);
	slot.m_envelope_gen.m_decay_level = 0xf - slot.m_sample.m_decay_level;
	slot.m_envelope_gen.m_reverb = false;
}

/*****************************
        LFO  SECTION
*****************************/

constexpr uint32_t gew_pcm_device::LFO_SHIFT;

const float gew_pcm_device::LFO_FREQ[8] = // In Hertz
{
	0.168f,
	2.019f,
	3.196f,
	4.206f,
	5.215f,
	5.888f,
	6.224f,
	7.066f
};

const float gew_pcm_device::PHASE_SCALE_LIMIT[8] = // In Cents
{
	0.0f,
	3.378f,
	5.065f,
	6.750f,
	10.114f,
	20.170f,
	40.180f,
	79.307f
};

const float gew_pcm_device::AMPLITUDE_SCALE_LIMIT[8] = // In Decibels
{
	0.0f,
	0.4f,
	0.8f,
	1.5f,
	3.0f,
	6.0f,
	12.0f,
	24.0f
};

void gew_pcm_device::lfo_init()
{
	m_pitch_table = make_unique_clear<int32_t[]>(256);
	m_amplitude_table = make_unique_clear<int32_t[]>(256);
	for (int32_t i = 0; i < 256; ++i)
	{
		if (i < 64)
		{
			m_pitch_table[i] = i * 2 + 128;
		}
		else if (i < 128)
		{
			m_pitch_table[i] = 383 - i * 2;
		}
		else if (i < 192)
		{
			m_pitch_table[i] = 384 - i * 2;
		}
		else
		{
			m_pitch_table[i] = i * 2 - 383;
		}

		if (i < 128)
		{
			m_amplitude_table[i] = 255 - (i * 2);
		}
		else
		{
			m_amplitude_table[i] = (i * 2) - 256;
		}
	}

	for (int32_t table = 0; table < 8; ++table)
	{
		float limit = PHASE_SCALE_LIMIT[table];
		m_pitch_scale_tables[table] = make_unique_clear<int32_t[]>(256);
		for (int32_t i = -128; i < 128; ++i)
		{
			const float value = (limit * (float)i) / 128.0f;
			const float converted = powf(2.0f, value / 1200.0f);
			m_pitch_scale_tables[table][i + 128] = value_to_fixed(LFO_SHIFT, converted);
		}

		limit = -AMPLITUDE_SCALE_LIMIT[table];
		m_amplitude_scale_tables[table] = make_unique_clear<int32_t[]>(256);
		for (int32_t i = 0; i < 256; ++i)
		{
			const float value = (limit * (float)i) / 256.0f;
			const float converted = powf(10.0f, value / 20.0f);
			m_amplitude_scale_tables[table][i] = value_to_fixed(LFO_SHIFT, converted);
		}
	}
}

uint32_t gew_pcm_device::value_to_fixed(const uint32_t bits, const float value)
{
	const float float_shift = float(1 << bits);
	return uint32_t(float_shift * value);
}

int32_t gew_pcm_device::pitch_lfo_step(lfo_t &lfo)
{
	lfo.m_phase += lfo.m_phase_step;
	int32_t p = lfo.m_table[(lfo.m_phase >> LFO_SHIFT) & 0xff];
	p = lfo.m_scale[p];
	return p << (TL_SHIFT - LFO_SHIFT);
}

int32_t gew_pcm_device::amplitude_lfo_step(lfo_t &lfo)
{
	lfo.m_phase += lfo.m_phase_step;
	int32_t p = lfo.m_table[(lfo.m_phase >> LFO_SHIFT) & 0xff];
	p = lfo.m_scale[p];
	return p << (TL_SHIFT - LFO_SHIFT);
}

void gew_pcm_device::lfo_compute_step(lfo_t &lfo, uint32_t lfo_frequency, uint32_t lfo_scale, int32_t amplitude_lfo)
{
	float step = (float)LFO_FREQ[lfo_frequency] * 256.0f / (float)m_rate;
	lfo.m_phase_step = uint32_t(float(1 << LFO_SHIFT) * step);
	if (amplitude_lfo)
	{
		lfo.m_table = m_amplitude_table.get();
		lfo.m_scale = m_amplitude_scale_tables[lfo_scale].get();
	}
	else
	{
		lfo.m_table = m_pitch_table.get();
		lfo.m_scale = m_pitch_scale_tables[lfo_scale].get();
	}
}

/* MAME access functions */

gew_pcm_device::gew_pcm_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock,
		uint32_t voices, uint32_t clock_divider) :
	device_t(mconfig, type, tag, owner, clock),
	device_sound_interface(mconfig, *this),
	device_rom_interface(mconfig, *this),
	m_stream(nullptr),
	m_slots(nullptr),
	m_rate(0),
	m_voices(voices),
	m_clock_divider(clock_divider),
	m_attack_step(nullptr),
	m_decay_release_step(nullptr),
	m_freq_step_table(nullptr),
	m_left_pan_table(nullptr),
	m_right_pan_table(nullptr),
	m_linear_to_exp_volume(nullptr),
	m_total_level_steps(nullptr)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void gew_pcm_device::device_start()
{
	m_rate = (float)clock() / m_clock_divider;

	m_stream = stream_alloc(0, 2, m_rate);

	// Volume + pan table
	m_left_pan_table = make_unique_clear<int32_t[]>(0x800);
	m_right_pan_table = make_unique_clear<int32_t[]>(0x800);
	for (int32_t level = 0; level < 0x80; ++level)
	{
		const float vol_db = (float)level * (-24.0f) / 64.0f;
		const float total_level = powf(10.0f, vol_db / 20.0f) / 4.0f;

		for (int32_t pan = 0; pan < 0x10; ++pan)
		{
			float pan_left, pan_right;
			if (pan == 0x8)
			{
				pan_left = 0.0;
				pan_right = 0.0;
			}
			else if (pan == 0x0)
			{
				pan_left = 1.0;
				pan_right = 1.0;
			}
			else if (pan & 0x8)
			{
				pan_left = 1.0;

				const int32_t inverted_pan = 0x10 - pan;
				const float pan_vol_db = (float)inverted_pan * (-12.0f) / 4.0f;

				pan_right = pow(10.0f, pan_vol_db / 20.0f);

				if ((inverted_pan & 0x7) == 7)
				{
					pan_right = 0.0;
				}
			}
			else
			{
				pan_right = 1.0;

				const float pan_vol_db = (float)pan * (-12.0f) / 4.0f;

				pan_left = pow(10.0f, pan_vol_db / 20.0f);

				if ((pan & 0x7) == 7)
				{
					pan_left = 0.0;
				}
			}

			m_left_pan_table[(pan << 7) | level] = value_to_fixed(TL_SHIFT, pan_left * total_level);
			m_right_pan_table[(pan << 7) | level] = value_to_fixed(TL_SHIFT, pan_right * total_level);
		}
	}

	// Pitch steps
	m_freq_step_table = make_unique_clear<uint32_t[]>(0x400);
	for (int32_t i = 0; i < 0x400; ++i)
	{
		const float fcent = m_rate * (1024.0f + (float)i) / 1024.0f;
		m_freq_step_table[i] = value_to_fixed(TL_SHIFT, fcent);
	}

	// Envelope steps
	m_attack_step = make_unique_clear<uint32_t[]>(0x40);
	m_decay_release_step = make_unique_clear<uint32_t[]>(0x40);
	envelope_generator_init(BASE_TIMES, 14.32833);

	// Total level interpolation steps
	m_total_level_steps = make_unique_clear<int32_t[]>(2);
	m_total_level_steps[0] = -(float)(0x80 << TL_SHIFT) / (78.2f * 44100.0f / 1000.0f); // lower
	m_total_level_steps[1] = (float)(0x80 << TL_SHIFT) / (78.2f * 2 * 44100.0f / 1000.0f); // raise

	// build the linear->exponential ramps
	m_linear_to_exp_volume = make_unique_clear<int32_t[]>(0x400);
	for (int32_t i = 0; i < 0x400; ++i)
	{
		const float db = -(96.0f - (96.0f * (float)i / (float)0x400));
		const float exp_volume = powf(10.0f, db / 20.0f);
		m_linear_to_exp_volume[i] = value_to_fixed(TL_SHIFT, exp_volume);
	}

	// Slots
	m_slots = std::make_unique<slot_t[]>(m_voices);

	save_pointer(STRUCT_MEMBER(m_slots, m_regs), m_voices);
	save_pointer(STRUCT_MEMBER(m_slots, m_playing), m_voices);
	save_pointer(STRUCT_MEMBER(m_slots, m_offset), m_voices);
	save_pointer(STRUCT_MEMBER(m_slots, m_octave), m_voices);
	save_pointer(STRUCT_MEMBER(m_slots, m_pitch), m_voices);
	save_pointer(STRUCT_MEMBER(m_slots, m_step), m_voices);
	save_pointer(STRUCT_MEMBER(m_slots, m_reverse), m_voices);
	save_pointer(STRUCT_MEMBER(m_slots, m_pan), m_voices);
	save_pointer(STRUCT_MEMBER(m_slots, m_total_level), m_voices);
	save_pointer(STRUCT_MEMBER(m_slots, m_dest_total_level), m_voices);
	save_pointer(STRUCT_MEMBER(m_slots, m_total_level_step), m_voices);
	save_pointer(STRUCT_MEMBER(m_slots, m_prev_sample), m_voices);
	save_pointer(STRUCT_MEMBER(m_slots, m_lfo_frequency), m_voices);
	save_pointer(STRUCT_MEMBER(m_slots, m_vibrato), m_voices);
	save_pointer(STRUCT_MEMBER(m_slots, m_tremolo), m_voices);

	for (int32_t slot = 0; slot < m_voices; ++slot)
	{
		save_item(NAME(m_slots[slot].m_sample.m_start), slot);
		save_item(NAME(m_slots[slot].m_sample.m_loop), slot);
		save_item(NAME(m_slots[slot].m_sample.m_end), slot);
		save_item(NAME(m_slots[slot].m_sample.m_attack_reg), slot);
		save_item(NAME(m_slots[slot].m_sample.m_decay1_reg), slot);
		save_item(NAME(m_slots[slot].m_sample.m_decay2_reg), slot);
		save_item(NAME(m_slots[slot].m_sample.m_decay_level), slot);
		save_item(NAME(m_slots[slot].m_sample.m_release_reg), slot);
		save_item(NAME(m_slots[slot].m_sample.m_key_rate_scale), slot);
		save_item(NAME(m_slots[slot].m_sample.m_lfo_vibrato_reg), slot);
		save_item(NAME(m_slots[slot].m_sample.m_lfo_amplitude_reg), slot);
		save_item(NAME(m_slots[slot].m_sample.m_format), slot);

		save_item(NAME(m_slots[slot].m_envelope_gen.m_volume), slot);
		save_item(NAME(m_slots[slot].m_envelope_gen.m_state), slot);
		save_item(NAME(m_slots[slot].m_envelope_gen.m_reverb), slot);
		save_item(NAME(m_slots[slot].m_envelope_gen.step), slot);
		save_item(NAME(m_slots[slot].m_envelope_gen.m_attack_rate), slot);
		save_item(NAME(m_slots[slot].m_envelope_gen.m_decay1_rate), slot);
		save_item(NAME(m_slots[slot].m_envelope_gen.m_decay2_rate), slot);
		save_item(NAME(m_slots[slot].m_envelope_gen.m_release_rate), slot);
		save_item(NAME(m_slots[slot].m_envelope_gen.m_decay_level), slot);

		save_item(NAME(m_slots[slot].m_pitch_lfo.m_phase), slot);
		save_item(NAME(m_slots[slot].m_pitch_lfo.m_phase_step), slot);
		save_item(NAME(m_slots[slot].m_amplitude_lfo.m_phase), slot);
		save_item(NAME(m_slots[slot].m_amplitude_lfo.m_phase_step), slot);
	}

	lfo_init();
}

void gew_pcm_device::device_reset()
{
	for (int32_t slot = 0; slot < m_voices; ++slot)
	{
		m_slots[slot].m_playing = false;
	}
}

//-------------------------------------------------
//  device_clock_changed - called if the clock
//  changes
//-------------------------------------------------

void gew_pcm_device::device_clock_changed()
{
	m_rate = (float)clock() / m_clock_divider;
	m_stream->set_sample_rate(m_rate);

	for (int32_t i = 0; i < 0x400; ++i)
	{
		const float fcent = m_rate * (1024.0f + (float)i) / 1024.0f;
		m_freq_step_table[i] = value_to_fixed(TL_SHIFT, fcent);
	}
}

//-----------------------------------------------------
//  dump_sample - dump current sample to WAV file
//-----------------------------------------------------

#if MULTIPCM_LOG_SAMPLES
void gew_pcm_device::dump_sample(slot_t &slot)
{
	if (m_logged_map[slot.m_sample.m_start])
		return;

	m_logged_map[slot.m_sample.m_start] = true;

	char filebuf[256];
	snprintf(filebuf, 256, "multipcm%08x.wav", slot.m_sample.m_start);
	util::wav_file_ptr file = util::wav_open(filebuf, m_stream->sample_rate(), 1);
	if (file == nullptr)
		return;

	uint32_t offset = slot.m_offset;
	bool done = false;
	while (!done)
	{
		int16_t sample = (int16_t)(read_byte(slot.m_sample.m_start + (offset >> TL_SHIFT)) << 8);
		util::wav_add_data_16(*file.get(), &sample, 1);

		offset += 1 << TL_SHIFT;
		if (offset >= (slot.m_sample.m_end << TL_SHIFT))
		{
			done = true;
		}
	}

	util::wav_close(file.get());
}
#endif

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void gew_pcm_device::sound_stream_update(sound_stream &stream)
{
	for (int32_t i = 0; i < stream.samples(); ++i)
	{
		int32_t smpl = 0;
		int32_t smpr = 0;
		for (int32_t sl = 0; sl < m_voices; ++sl)
		{
			slot_t& slot = m_slots[sl];
			if (slot.m_playing)
			{
				uint32_t vol = (slot.m_total_level >> TL_SHIFT) | (slot.m_pan << 7);
				uint32_t spos = slot.m_offset >> TL_SHIFT;
				uint32_t step = slot.m_step;
				int32_t csample = 0;
				int32_t fpart = slot.m_offset & ((1 << TL_SHIFT) - 1);

				if (slot.m_reverse)
				{
					spos = slot.m_sample.m_end - spos - 1;
				}

				if (slot.m_sample.m_format & 4)  // 12-bit linear
				{
					offs_t adr = slot.m_sample.m_start + (spos >> 1) * 3;
					if (!(spos & 1))
					{ // ab.c ..
						s16 w0 = read_byte(adr) << 8 | ((read_byte(adr + 1) & 0xf) << 4);
						csample = w0;
					}
					else
					{ // ..C. AB
						s16 w0 = (read_byte(adr + 2) << 8) | (read_byte(adr + 1) & 0xf0);
						csample = w0;
					}
				}
				else
				{
					csample = (int16_t)(read_byte(slot.m_sample.m_start + spos) << 8);
				}

				int32_t sample = (csample * fpart + slot.m_prev_sample * ((1 << TL_SHIFT) - fpart)) >> TL_SHIFT;

				if (slot.m_vibrato) // Vibrato enabled
				{
					step = step * pitch_lfo_step(slot.m_pitch_lfo);
					step >>= TL_SHIFT;
				}

				slot.m_offset += step;

				if (spos ^ (slot.m_offset >> TL_SHIFT))
				{
					slot.m_prev_sample = csample;
				}

				if (slot.m_offset >= (slot.m_sample.m_end << TL_SHIFT))
				{
					slot.m_offset -= (slot.m_sample.m_end - slot.m_sample.m_loop) << TL_SHIFT;
					// DD-9 expects the looped silence at the end of some samples to be the same whether reversed or not
					slot.m_reverse = false;
				}

				if ((slot.m_total_level >> TL_SHIFT) != slot.m_dest_total_level)
				{
					slot.m_total_level += slot.m_total_level_step;
				}

				if (slot.m_tremolo) // Tremolo enabled
				{
					sample = sample * amplitude_lfo_step(slot.m_amplitude_lfo);
					sample >>= TL_SHIFT;
				}

				sample = (sample * envelope_generator_update(slot)) >> 10;

				smpl += (m_left_pan_table[vol] * sample) >> TL_SHIFT;
				smpr += (m_right_pan_table[vol] * sample) >> TL_SHIFT;
			}
		}

		stream.put_int_clamp(0, i, smpl, 32768);
		stream.put_int_clamp(1, i, smpr, 32768);
	}
}


//-------------------------------------------------
//  rom_bank_pre_change - refresh the stream if the
//  ROM banking changes
//-------------------------------------------------

void gew_pcm_device::rom_bank_pre_change()
{
	m_stream->update();
}
