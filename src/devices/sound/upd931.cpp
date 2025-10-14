// license:BSD-3-Clause
// copyright-holders:Devin Acker

/***************************************************************************
    NEC/Casio uPD931 synthesis chip

    Many details of this implementation are based on research and notes by Robin Whittle:
    https://www.firstpr.com.au/rwi/casio/Casio-931-2006-06-17.txt
    Any references to MT-65 behavior are based on this document.

    TODO:
    - implement vibrato register (CT-8000 doesn't use it)
    - a few other unknown/unclear bits in the flags shift register
***************************************************************************/

#include "emu.h"
#include "upd931.h"

#include <algorithm>
#include <cmath>

DEFINE_DEVICE_TYPE(UPD931, upd931_device, "upd931", "NEC uPD931")

upd931_device::upd931_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, UPD931, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, device_memory_interface(mconfig, *this)
	, m_io_config("io", ENDIANNESS_LITTLE, 8, 8, 0, address_map_constructor(FUNC(upd931_device::io_map), this))
	, m_retrig_timer(nullptr)
	, m_filter_cb(*this)
	, m_sync_cb(*this)
	, m_master(true)
{
}

/**************************************************************************/
device_memory_interface::space_config_vector upd931_device::memory_space_config() const
{
	return space_config_vector{std::make_pair(AS_IO, &m_io_config)};
}

/**************************************************************************/
void upd931_device::io_map(address_map &map)
{
	map(0x20, 0x27).w(FUNC(upd931_device::note_w));
	map(0x30, 0x37).w(FUNC(upd931_device::octave_w));
	// waveform write position: ct8000 uses 40, mt65 uses 60
	map(0x40, 0x40).mirror(0x20).w(FUNC(upd931_device::wave_pos_w));
	map(0xa7, 0xa7).w(FUNC(upd931_device::wave_data_w));
	map(0xb0, 0xb7).w(FUNC(upd931_device::flags_w)); // lower address bits are ignored
	map(0xb8, 0xbf).w(FUNC(upd931_device::status_latch_w));
	// vibrato/sustain: ct8000 uses c0-c7, mt65 uses d0-d7 (lower two bits are also ignored)
	map(0xc0, 0xc3).mirror(0x10).w(FUNC(upd931_device::vibrato_w));
	map(0xc4, 0xc7).mirror(0x10).w(FUNC(upd931_device::sustain_w));
	map(0xe0, 0xe7).w(FUNC(upd931_device::note_on_w));
	map(0xf4, 0xf4).lw8(NAME([this] (offs_t offset, u8 data) { m_filter_cb(data); }));
}

/**************************************************************************/
void upd931_device::device_start()
{
	space(AS_IO).specific(m_io);

	m_stream = stream_alloc(0, 1, clock() / CLOCKS_PER_SAMPLE);

	if (m_master)
	{
		m_retrig_timer = timer_alloc(FUNC(upd931_device::timer_tick), this);
		reset_timer();
	}
	m_last_clock = clock();

	m_db = 0xf;
	m_i1 = m_i2 = m_i3 = 1;
	m_status = 0xf;

	// generate note frequencies based on CT-8000 crystal frequency (probably not entirely accurate)
	const unsigned sample_rate = 4'946'864 / CLOCKS_PER_SAMPLE;
	for (int i = 0; i < 73; i++)
	{
		// A4 is note 33, 442 Hz
		const double freq = 442.0 * pow(2, (i - 33) / 12.0);
		m_pitch[i] = (1 << PITCH_SHIFT) * (freq * 16 / sample_rate);
	}

	save_item(NAME(m_db));
	save_item(NAME(m_i1));
	save_item(NAME(m_i2));
	save_item(NAME(m_i3));

	save_item(NAME(m_addr));
	save_item(NAME(m_data));
	save_item(NAME(m_status));

	save_item(NAME(m_wave));
	save_item(NAME(m_wave_pos));
	save_item(NAME(m_vibrato));
	save_item(NAME(m_sustain));
	save_item(NAME(m_reverb));

	save_item(NAME(m_flags));
	save_item(NAME(m_last_clock));

	save_item(STRUCT_MEMBER(m_voice, m_note));
	save_item(STRUCT_MEMBER(m_voice, m_octave));
	save_item(STRUCT_MEMBER(m_voice, m_pitch));
	save_item(STRUCT_MEMBER(m_voice, m_pitch_counter));
	save_item(STRUCT_MEMBER(m_voice, m_timbre_shift));

	save_item(STRUCT_MEMBER(m_voice, m_wave_pos));
	save_item(STRUCT_MEMBER(m_voice, m_wave_out));

	save_item(STRUCT_MEMBER(m_voice, m_env_state));
	save_item(STRUCT_MEMBER(m_voice, m_env_counter));
	save_item(STRUCT_MEMBER(m_voice, m_env_level));
	save_item(STRUCT_MEMBER(m_voice, m_force_release));
}

/**************************************************************************/
TIMER_CALLBACK_MEMBER(upd931_device::timer_tick)
{
	m_sync_cb(1);
	sync_w(1);
}

/**************************************************************************/
void upd931_device::device_reset()
{
	std::fill(std::begin(m_wave[0]), std::end(m_wave[0]), 0);
	std::fill(std::begin(m_wave[1]), std::end(m_wave[1]), 0);
	m_wave_pos = 0;
	m_vibrato = 0;
	m_sustain = 0;
	m_reverb = 0;

	m_flags = 0;

	std::fill(m_voice.begin(), m_voice.end(), voice_t());

	m_filter_cb(0);
	m_sync_cb(0);
}

/**************************************************************************/
void upd931_device::device_clock_changed()
{
	m_stream->set_sample_rate(clock() / CLOCKS_PER_SAMPLE);

	if (m_retrig_timer)
	{
		const u64 ticks_left = m_retrig_timer->remaining().as_ticks(m_last_clock);
		const attotime remaining = attotime::from_ticks(ticks_left, clock());
		const attotime period = attotime::from_ticks(RETRIG_RATE, clock());

		m_retrig_timer->adjust(remaining, 0, period);
	}

	m_last_clock = clock();
}

/**************************************************************************/
void upd931_device::sound_stream_update(sound_stream &stream)
{
	for (int i = 0; i < stream.samples(); i++)
	{
		s32 sample = 0;
		for (voice_t &voice : m_voice)
		{
			update_env(voice);
			update_wave(voice);

			sample += voice.m_wave_out[0] * (voice.m_env_level[0] >> VOLUME_SHIFT);
			sample += voice.m_wave_out[1] * (voice.m_env_level[1] >> VOLUME_SHIFT);
		}
		stream.put_int_clamp(0, i, sample, 1 << 16);
	}
}

/**************************************************************************/
void upd931_device::i1_w(int state)
{
	if (!m_i3 && m_i1 && !state)
	{
		// address high nibble on I1 falling edge
		m_addr &= 0x0f;
		m_addr |= (m_db << 4);
	}

	m_i1 = state;
}

/**************************************************************************/
void upd931_device::i2_w(int state)
{
	if (!m_i3)
	{
		if (!m_i2 && state)
		{
			// address low nibble on I2 rising edge
			m_addr &= 0xf0;
			m_addr |= (m_db & 0xf);

			// apply register write
			m_stream->update();
			m_io.write_byte(m_addr, m_data & 0xf);
		}
		else if (m_i2 && !state)
		{
			// data on I2 falling edge
			m_data = m_db;
		}
	}

	m_i2 = state;
}

/**************************************************************************/
void upd931_device::i3_w(int state)
{
	m_i3 = state;
}

/**************************************************************************/
void upd931_device::db_w(u8 data)
{
	m_db = data;
}

/**************************************************************************/
u8 upd931_device::db_r()
{
	if (m_i1 && m_i2 && !m_i3)
		return m_status;
	return 0xf;
}

/**************************************************************************/
void upd931_device::sync_w(int state)
{
	if (BIT(m_flags, FLAG_RETRIGGER))
	{
		m_stream->update();

		for (voice_t &voice : m_voice)
		{
			if (voice.m_env_state == ENV_DECAY1 || voice.m_env_state == ENV_DECAY2)
			{
				voice.m_env_state = ENV_ATTACK1;
				voice.m_env_counter = 0;
			}
		}
	}
}

/**************************************************************************/
void upd931_device::note_w(offs_t offset, u8 data)
{
	m_voice[offset].m_note = data;
}

/**************************************************************************/
void upd931_device::octave_w(offs_t offset, u8 data)
{
	m_voice[offset].m_octave = m_data;
}

/**************************************************************************/
void upd931_device::wave_pos_w(u8 data)
{
	m_wave_pos = data;
}

/**************************************************************************/
void upd931_device::wave_data_w(u8 data)
{
	const u8 sel = BIT(m_flags, FLAG_WAVE_SEL);
	m_wave[sel][m_wave_pos & 0xf] = data;
}

/**************************************************************************/
void upd931_device::flags_w(u8 data)
{
	m_flags >>= 1;
	m_flags |= ((data & 1) << FLAG_WAVE_SEL);
}

/**************************************************************************/
void upd931_device::status_latch_w(offs_t offset, u8 data)
{
	// TODO: more details. ct8000 only checks if bits 1-3 are all zero or not
	if (m_voice[offset].m_env_state == ENV_IDLE)
		m_status = 0xf;
	else
		m_status = 0;
}

/**************************************************************************/
void upd931_device::vibrato_w(u8 data)
{
	// TODO: implement this. ct8000 always writes 0 since it uses the external VCO vibrato instead
	m_vibrato = data & 3;
}

/**************************************************************************/
void upd931_device::sustain_w(u8 data)
{
	m_sustain = data & 3;
	m_reverb = BIT(data, 1, 2) ? 1 : 0;
}

/**************************************************************************/
void upd931_device::note_on_w(offs_t offset, u8 data)
{
	voice_t &voice = m_voice[offset];
	if (BIT(data, 0))
		note_on(voice);
	else
		voice.m_env_state = ENV_RELEASE;

	// mt65 turns off sustain and reverb when changing tones, ct8000 does this instead
	// (and also when playing a new note that is already playing)
	voice.m_force_release = BIT(data, 3);
}

/**************************************************************************/
void upd931_device::note_on(voice_t &voice)
{
	if (voice.m_note >= 0x2 && voice.m_note <= 0xe)
	{
		const u8 note = voice.m_note - 2;
		u8 octave = voice.m_octave & 7;

		if (octave >= 2)
			octave -= 2; // octave values 0-1 are the same as 2-3

		/*
		setting bit 3 of the octave reduces the duty cycle of individual notes, which is
		implemented here by changing which part of the phase counter to use as the sample address.
		ct8000 uses this for a few of its presets to produce a simple key-scaling effect.
		*/
		if (BIT(voice.m_octave, 3))
			voice.m_timbre_shift = 3 - octave;
		else
			voice.m_timbre_shift = 0;

		voice.m_pitch = m_pitch[octave * 12 + note];
	}
	else
	{
		voice.m_pitch = 0;
	}

	voice.m_pitch_counter = 0;
	voice.m_wave_pos = 0xff;
	voice.m_wave_out[0] = voice.m_wave_out[1] = 0;
	voice.m_env_state = ENV_ATTACK1;
	voice.m_env_counter = 0;

	if (m_master)
		reset_timer();
}

/**************************************************************************/
void upd931_device::reset_timer()
{
	const attotime period = attotime::from_ticks(RETRIG_RATE, clock());
	m_retrig_timer->adjust(period, 0, period);
}

/**************************************************************************/
void upd931_device::update_env(voice_t &voice)
{
	const unsigned shift = BIT(m_flags, FLAG_ENV_SHIFT, 2);

	switch (voice.m_env_state)
	{
	case ENV_IDLE:
		return;

	case ENV_ATTACK1:
	{
		static const u16 rates[] =
		{
			0, 2048, 512, 256, 160, 80, 32, 8
		};

		const u8 val = BIT(m_flags, FLAG_ATTACK1, 3);
		u32 rate;

		if (val == 0)
			rate = VOLUME_MAX; // value 0 = instant
		else if (val < 4 && voice.m_env_counter >= (0xe0 << VOLUME_SHIFT))
			rate = 160 << shift; // values 1-3 slow down at 7/8 of max volume
		else
			rate = rates[val] << shift;
		voice.m_env_counter = std::min(voice.m_env_counter + rate, VOLUME_MAX);

		// only increase wave A level if it isn't set to rise during attack2 instead
		if (BIT(m_flags, FLAG_ATTACK2_A))
			voice.m_env_level[0] = 0;
		else
			voice.m_env_level[0] = voice.m_env_counter;
		voice.m_env_level[1] = voice.m_env_counter;

		if (voice.m_env_counter >= VOLUME_MAX)
		{
			voice.m_env_counter = 0;
			voice.m_env_state = ENV_ATTACK2;
		}
		break;
	}

	case ENV_ATTACK2:
	{
		static const u32 rates[] =
		{
			0, 2048, 256, 128, 64, 32, 16, 8
		};

		const u8 val = BIT(m_flags, FLAG_ATTACK2, 3);
		u32 rate;

		if (val == 0)
			rate = VOLUME_MAX;
		else
			rate = rates[val] << shift;
		voice.m_env_counter = std::min(voice.m_env_counter + rate, VOLUME_MAX);

		// fade wave A in, if specified
		if (BIT(m_flags, FLAG_ATTACK2_A))
			voice.m_env_level[0] = voice.m_env_counter;

		// fade wave B out, if specified
		if (BIT(m_flags, FLAG_ATTACK2_B))
			voice.m_env_level[1] = VOLUME_MAX - voice.m_env_counter;

		if (voice.m_env_counter >= VOLUME_MAX)
			voice.m_env_state = ENV_DECAY1;

		break;
	}

	case ENV_DECAY1:
	{
		static const u32 rates[] =
		{
			2048, 640, 160, 32, 16, 8, 2, 0
		};

		const u8 val = BIT(m_flags, FLAG_DECAY1, 3);
		const u32 rate = rates[val] << shift;

		if (voice.m_env_counter < rate)
		{
			voice.m_env_counter = 0;
			voice.m_env_state = ENV_IDLE;
		}
		else
		{
			voice.m_env_counter -= rate;
		}

		voice.m_env_level[0] = voice.m_env_counter;
		// only fade wave B if it didn't already fade out during attack2
		if (voice.m_env_level[1])
			voice.m_env_level[1] = voice.m_env_counter;

		if (!BIT(m_flags, FLAG_DECAY2_DISABLE))
		{
			// transition to decay2 at 1/2 or 1/4 of max volume, if enabled
			const u8 decay2_level = BIT(m_flags, FLAG_DECAY2_LEVEL) ? 0x40 : 0x80;
			if (voice.m_env_counter < (decay2_level << VOLUME_SHIFT))
				voice.m_env_state = ENV_DECAY2;
		}

		break;
	}

	case ENV_DECAY2:
	{
		// apply reverb rate (below 1/8 max volume) or decay2 rate
		u16 rate;

		if (m_reverb && voice.m_env_counter < (0x20 << VOLUME_SHIFT))
			rate = 1 << shift;
		else if (BIT(m_flags, FLAG_DECAY2))
			rate = 3 << shift;
		else
			rate = 6 << shift;

		if (voice.m_env_counter < rate)
		{
			voice.m_env_counter = 0;
			voice.m_env_state = ENV_IDLE;
		}
		else
		{
			voice.m_env_counter -= rate;
		}

		voice.m_env_level[0] = voice.m_env_counter;
		if (voice.m_env_level[1])
			voice.m_env_level[1] = voice.m_env_counter;

		break;
	}

	case ENV_RELEASE:
	{
		// apply reverb rate (below 1/8 max volume), sustain rate (if enabled) or default release rate
		u16 rate = 512 << shift;

		if (!voice.m_force_release)
		{
			if (m_reverb && voice.m_env_counter < (0x20 << VOLUME_SHIFT))
				rate = 1 << shift;
			else if (m_sustain == 1)
				rate = 16 << shift;
			else if (m_sustain == 2)
				rate = 12 << shift;
		}

		if (voice.m_env_counter < rate)
		{
			voice.m_env_counter = 0;
			voice.m_env_state = ENV_IDLE;
		}
		else
		{
			voice.m_env_counter -= rate;
		}

		// fade each wave individually because if keyed off during attack, they may be different from each other
		voice.m_env_level[0] = std::min(voice.m_env_level[0], voice.m_env_counter);
		voice.m_env_level[1] = std::min(voice.m_env_level[1], voice.m_env_counter);

		break;
	}

	}
}

/**************************************************************************/
void upd931_device::update_wave(voice_t &voice)
{
	voice.m_pitch_counter += voice.m_pitch;

	const u8 cycle = BIT(voice.m_pitch_counter, PITCH_SHIFT + 4, 2);
	/*
	the part of the counter which is treated as the sample address depends on if key scaling is active.
	if it is active, then the voice alternates between a narrowed waveform and silence, which alters
	the timbre without affecting pitch
	*/
	u8 pos = BIT(voice.m_pitch_counter, PITCH_SHIFT - voice.m_timbre_shift, 4 + voice.m_timbre_shift);
	if (pos == voice.m_wave_pos || pos >= 0x10)
		return;

	voice.m_wave_pos = pos;

	// play every other cycle backwards, if enabled
	if (BIT(m_flags, FLAG_MIRROR) && BIT(cycle, 0))
		pos ^= 0xf;

	// flag bits determine which of 4 consecutive cycles to apply waveforms A and B
	const unsigned cycle_mode[] =
	{
		BIT(m_flags, FLAG_MODE_A, 2),
		BIT(m_flags, FLAG_MODE_B, 2)
	};

	static const u8 cycle_mask[] =
	{
		0xf, // always on
		0x5, // on, off, on, off
		0x1, // on 1x, off 3x
		0x3  // on 2x, off 2x
	};

	static const s8 steps[] =
	{
		0,  1,  2,  2,  4,  4,  8,  8,
		0, -1, -2, -2, -4, -4, -8, -8
	};

	for (int i = 0; i < 2; i++)
	{
		// check if this waveform is enabled for this cycle
		if (!BIT(cycle_mask[cycle_mode[i]], cycle))
			continue;

		s8 step = steps[m_wave[i][pos] & 0xf];
		// invert waveform on every other cycle, if enabled
		if (BIT(m_flags, FLAG_INVERT) && BIT(cycle, 0))
			step = -step;

		voice.m_wave_out[i] += step;
		voice.m_wave_out[i] = util::sext(voice.m_wave_out[i] & 0x3f, 6);
	}
}
