// license:BSD-3-Clause
// copyright-holders:Devin Acker

/***************************************************************************
    NEC/Casio uPD933 "Phase Distortion" synthesis chip
***************************************************************************/

#include "emu.h"
#include "upd933.h"

#include <algorithm>
#include <climits>
#include <cmath>

DEFINE_DEVICE_TYPE(UPD933, upd933_device, "upd933", "NEC uPD933")

upd933_device::upd933_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, UPD933, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, m_irq_cb(*this)
{
}

/**************************************************************************/
void upd933_device::device_start()
{
	m_stream = stream_alloc(0, 1, clock() / CLOCKS_PER_SAMPLE);

	m_irq_timer = timer_alloc(FUNC(upd933_device::timer_tick), this);

	for (int i = 0; i < 0x800; i++)
		m_cosine[i] = 0xfff * (1 - cos(2.0 * M_PI * i / 0x7ff)) / 2;

	for (int i = 0; i < 0x80; i++)
	{
		// A4 is note 62, 442 Hz
		const double freq = 442.0 * pow(2, (i - 62) / 12.0);
		m_pitch[i] = (1 << PITCH_SHIFT) * (freq * 0x800 / 40000);
	}

	for (int i = 0; i < 0x200; i++)
		m_pitch_fine[i] = (1 << PITCH_FINE_SHIFT) * (pow(2, (double)i / (12.0 * 0x200)) - 1);

	// logarithmic volume curve, also scales 12-bit waveform to 13-bit range
	// (also allows pitch modulation to cover the same spectrum with no extra scaling)
	for (int i = 1; i < 0x200; i++)
		m_volume[i] = pow(2 << VOLUME_SHIFT, (double)i / 0x1ff);
	m_volume[0] = 0;

	m_cs = m_id = 1;

	save_item(NAME(m_irq_pending));
	save_item(NAME(m_irq_state));
	save_item(NAME(m_cs));
	save_item(NAME(m_id));
	save_item(NAME(m_sound_data));
	save_item(NAME(m_sound_data_pos));
	save_item(NAME(m_sound_regs));
	save_item(NAME(m_sample_count));
	save_item(NAME(m_last_sample));

	save_item(STRUCT_MEMBER(m_voice, m_wave));
	save_item(STRUCT_MEMBER(m_voice, m_window));
	save_item(STRUCT_MEMBER(m_voice, m_ring_mod));
	save_item(STRUCT_MEMBER(m_voice, m_pitch_mod));
	save_item(STRUCT_MEMBER(m_voice, m_mute_other));
	save_item(STRUCT_MEMBER(m_voice, m_pitch));
	save_item(STRUCT_MEMBER(m_voice, m_position));
	save_item(STRUCT_MEMBER(m_voice, m_pitch_step));
	save_item(STRUCT_MEMBER(m_voice, m_dcw_limit));
	save_item(STRUCT_MEMBER(m_voice, m_pm_level));

	save_item(STRUCT_MEMBER(m_dca, m_direction));
	save_item(STRUCT_MEMBER(m_dca, m_sustain));
	save_item(STRUCT_MEMBER(m_dca, m_irq));
	save_item(STRUCT_MEMBER(m_dca, m_rate));
	save_item(STRUCT_MEMBER(m_dca, m_target));
	save_item(STRUCT_MEMBER(m_dca, m_current));

	save_item(STRUCT_MEMBER(m_dcw, m_direction));
	save_item(STRUCT_MEMBER(m_dcw, m_sustain));
	save_item(STRUCT_MEMBER(m_dcw, m_irq));
	save_item(STRUCT_MEMBER(m_dcw, m_rate));
	save_item(STRUCT_MEMBER(m_dcw, m_target));
	save_item(STRUCT_MEMBER(m_dcw, m_current));

	save_item(STRUCT_MEMBER(m_dco, m_direction));
	save_item(STRUCT_MEMBER(m_dco, m_sustain));
	save_item(STRUCT_MEMBER(m_dco, m_irq));
	save_item(STRUCT_MEMBER(m_dco, m_rate));
	save_item(STRUCT_MEMBER(m_dco, m_target));
	save_item(STRUCT_MEMBER(m_dco, m_current));
}

/**************************************************************************/
TIMER_CALLBACK_MEMBER(upd933_device::timer_tick)
{
	m_irq_pending = 1;
	update_irq();
}

/**************************************************************************/
void upd933_device::device_reset()
{
	m_irq_pending = m_irq_state = 0;

	m_sound_data[0] = m_sound_data[1] = 0;
	m_sound_data_pos = 0;
	std::fill(m_sound_regs.begin(), m_sound_regs.end(), 0);

	std::fill(m_voice.begin(), m_voice.end(), voice_t());
	std::fill(m_dca.begin(), m_dca.end(), env_t());
	std::fill(m_dco.begin(), m_dco.end(), env_t());
	std::fill(m_dcw.begin(), m_dcw.end(), env_t());

	m_sample_count = 0;
	m_last_sample = 0;

	m_irq_timer->adjust(attotime::never);
	m_irq_cb(0);
}

/**************************************************************************/
void upd933_device::device_clock_changed()
{
	m_stream->set_sample_rate(clock() / CLOCKS_PER_SAMPLE);
}

/**************************************************************************/
int upd933_device::rq_r()
{
	if (!machine().side_effects_disabled())
		m_stream->update();

	return m_irq_state;
}

/**************************************************************************/
void upd933_device::cs_w(int state)
{
	m_stream->update();

	if (!m_cs && state)
		update_pending_irq();
	m_cs = state;
	update_irq();
}

/**************************************************************************/
void upd933_device::id_w(int state)
{
	m_stream->update();

	m_id = state;
	update_irq();
}

/**************************************************************************/
u8 upd933_device::irq_data()
{
	// TODO: do these have the correct priority?
	for (int i = 0; i < 8; i++)
	{
		if (m_dco[i].m_irq)
		{
			if (!machine().side_effects_disabled())
				m_dco[i].m_irq = false;
			return 4 | (i << 3);
		}
	}
	for (int i = 0; i < 8; i++)
	{
		if (m_dcw[i].m_irq)
		{
			if (!machine().side_effects_disabled())
				m_dcw[i].m_irq = false;
			return 2 | (i << 2);
		}
	}
	for (int i = 0; i < 8; i++)
	{
		if (m_dca[i].m_irq)
		{
			if (!machine().side_effects_disabled())
				m_dca[i].m_irq = false;
			return 1 | (i << 1);
		}
	}
	return 0;
}

/**************************************************************************/
void upd933_device::update_pending_irq()
{
	m_irq_pending = 0;
	bool env_active = false;
	unsigned new_time = UINT_MAX;

	for (int i = 0; i < 8; i++)
	{
		env_active |= (m_dca[i].calc_timeout(new_time)
				   ||  m_dco[i].calc_timeout(new_time)
				   ||  m_dcw[i].calc_timeout(new_time));
	}

	if (env_active)
		m_irq_timer->adjust(clocks_to_attotime((u64)new_time * CLOCKS_PER_SAMPLE));
	else
		m_irq_timer->adjust(attotime::never);
}

/**************************************************************************/
void upd933_device::update_irq()
{
	u8 const irq_state = m_cs & m_id & m_irq_pending;
	if (irq_state != m_irq_state)
	{
		m_irq_state = irq_state;
		m_irq_cb(m_irq_state);
	}
}

/**************************************************************************/
u8 upd933_device::read()
{
	if (!machine().side_effects_disabled())
		m_stream->update();

	return m_cs ? 0xff : irq_data();
}

/**************************************************************************/
void upd933_device::write(u8 data)
{
	if (m_cs) return;

	if (m_sound_data_pos >= 2)
	{
		m_stream->update();

		bool ok = true;
		const u8 reg = m_sound_data[0];
		const u16 value = m_sound_regs[reg] = (m_sound_data[1] << 8) | data;

		// the low 3 bits of the register number determine which voice is controlled by per-voice registers...
		const int vnum = reg & 7;
		voice_t &voice = m_voice[vnum];
		// ...except for registers 68-6f, which control waveform for voice 'n', but modulation for voice 'n-2'
		// (even though those two voices don't actually modulate each other...)
		voice_t &mod_voice = m_voice[(vnum + 6) & 7];

		m_sound_data_pos = 0;
		switch (reg >> 3)
		{
		case 0x0: // 00-07: DCA step (volume envelope)
			/*
			msb      lsb
			n-------          - direction (0 = up, 1 = down)
			-nnnnnnn          - rate
			         n------- - sustain flag
			         -nnnnnnn - level
			*/
		{
			env_t &dca = m_dca[vnum];
			dca.m_direction = BIT(value, 15);
			dca.m_rate      = env_rate(BIT(value, 8, 7));
			dca.m_sustain   = BIT(value, 7);
			dca.m_target    = BIT(value, 0, 7) << (ENV_DCA_SHIFT + 2);
			dca.m_irq = false;
		}
			break;

		case 0x2: // 10-17: DCO step (pitch envelope)
			/*
			msb      lsb
			n-------          - direction (0 = up, 1 = down)
			-nnnnnnn          - rate
			         n------- - sustain flag
			         -n------ - level units (1 = 2-semitone intervals, 0 = 6.25-cent intervals)
			         --nnnnnn - level
			*/
		{
			env_t &dco = m_dco[vnum];
			dco.m_direction = BIT(value, 15);
			dco.m_rate      = env_rate(BIT(value, 8, 7));
			dco.m_sustain   = BIT(value, 7);
			dco.m_target    = BIT(value, 0, 6) << (ENV_DCO_SHIFT + 5);
			if (BIT(value, 6))
				dco.m_target <<= 5;
			dco.m_irq = false;
		}
			break;

		case 0x4: // 20-27: DCW step (waveform envelope)
			// same bits as DCA step
		{
			env_t &dcw = m_dcw[vnum];
			dcw.m_direction = BIT(value, 15);
			dcw.m_rate      = env_rate(BIT(value, 8, 7));
			dcw.m_sustain   = BIT(value, 7);
			dcw.m_target    = BIT(value, 0, 7) << (ENV_DCW_SHIFT + 3);
			dcw.m_irq = false;
		}
			break;

		case 0xc: // 60-67: pitch (in semitones, as 7.9 fixed point)
			voice.m_pitch = value;
			update_pitch_step(vnum);
			break;

		case 0xd: // 68-6f: waveform
			/*
			msb      lsb
			nnn-----          - first waveform
			---nnn--          - second waveform
			------n-          - enable second
			-------n nn------ - window function
			         --n----- - ring modulation enable
			         ---n---- - pitch modulation enable
			         ----n--- - pitch modulation source (0 = other voice, 1 = noise)
			         -----n-- - output (0 = normal, 1 = mute previous voice)
			*/
			voice.m_wave[0]         = BIT(value, 13, 3);
			if (BIT(value, 9))
				voice.m_wave[1]     = BIT(value, 10, 3);
			else
				voice.m_wave[1]     = voice.m_wave[0];
			voice.m_window          = BIT(value, 6, 3);
			if (!BIT(vnum, 0))
			{
				// see earlier comment - these bits actually control a different voice
				mod_voice.m_ring_mod    = BIT(value, 5);
				mod_voice.m_pitch_mod   = BIT(value, 3, 2);
				mod_voice.m_mute_other  = BIT(value, 2);
			}
			break;

		case 0x13: // 98-9f: phase counter
			/*
			cz101 sets these to zero when starting a note to reset the oscillator.
			cz1 writes 0x0000, 0x0080, 0x0100, or 0x0180 for up to four voices of a tone instead
			*/
			voice.m_position = value << (PITCH_SHIFT - 4);
			break;

		case 0x17: // b8-bb: pitch modulator (probably - cz1 sets to zero when disabling noise)
			if (vnum < 4)
				m_voice[vnum << 1].m_pm_level = (s16)value;
			else
				ok = false;
			break;

		default:
			ok = false;
			break;
		}

		if (!ok)
			logerror("%s: unknown sound reg write: %02x %04x\n", machine().describe_context(), reg, value);
	}
	else
	{
		m_sound_data[m_sound_data_pos++] = data;
	}
}

/**************************************************************************/
u32 upd933_device::env_rate(u8 data) const
{
	return (8 | (data & 7)) << (data >> 3);
}

/**************************************************************************/
void upd933_device::sound_stream_update(sound_stream &stream)
{
	for (int i = 0; i < stream.samples(); i++)
	{
		s32 sample = 0;

		/*
		Voices need to be processed in a certain order for modulation to work correctly,
		i.e. to match each odd-numbered voice ("line 1") with the corresponding even-numbered one ("line 2").
		*/
		static const int voice_map[] = {5, 0, 7, 2, 1, 4, 3, 6};
		for (int j : voice_map)
			sample += update(j);

		stream.put_int_clamp(0, i, sample, 1 << 15);
		m_sample_count++;
	}
}

/**************************************************************************/
s16 upd933_device::update(int vnum)
{
	voice_t &voice = m_voice[vnum];
	s16 sample = 0;

	const u16 pos = BIT(voice.m_position, PITCH_SHIFT, 11);
	const u8 wave = BIT(voice.m_position, PITCH_SHIFT + 11);

	const u16 dcw = std::min(u16(m_dcw[vnum].m_current >> ENV_DCW_SHIFT), voice.m_dcw_limit);
	const u16 pivot = 0x400 - dcw;
	u16 phase = 0;
	u16 window = 0;

	//
	// apply transfer function
	//
	switch (voice.m_wave[wave] & 7)
	{
	case 0: // sawtooth - rises from [0, pivot) and falls from [pivot, 800)
		if (pos < pivot)
			phase = pos * 0x400 / pivot;
		else
			phase = 0x400 + (pos - pivot) * 0x400 / (0x800 - pivot);
		break;

	case 1: // square - rises from [0, pivot), stays high from [pivot, 400), then inverts
		if ((pos & 0x3ff) < pivot)
			phase = (pos & 0x3ff) * 0x400 / pivot;
		else
			phase = 0x3ff;

		phase |= (pos & 0x400);
		break;

	case 2: // pulse - rises & falls from [0, pivot*2), then stays low
		if (pos < pivot * 2)
			phase = pos * 0x800 / (pivot * 2);
		else
			phase = 0x7ff;
		break;

	case 3: // silent (undocumented)
		break;

	case 4: // double sine - rises & falls from [0, pivot), then again from [pivot, 800)
		if (pos < pivot)
			phase = pos * 0x800 / pivot;
		else
			phase = (pos - pivot) * 0x800 / (0x800 - pivot);
		break;

	case 5: // saw pulse - rises from [0, 400), falls from [400, 400+pivot), then stays low
		if (pos < 0x400)
			phase = pos;
		else if (pos < (pivot + 0x400))
			phase = 0x400 + (pos & 0x3ff) * 0x400 / pivot;
		else
			phase = 0x7ff;
		break;

	case 6: // resonance
		// this is a special case that just multiplies the frequency by the DCW level...
		phase = pos + ((pos * dcw) >> 6);
		// ...and hardsyncs to the fundamental frequency
		phase &= 0x7ff;
		break;

	case 7: // double pulse (undocumented) - same as regular pulse but double frequency
		if ((pos & 0x3ff) < pivot)
			phase = (pos & 0x3ff) * 0x400 / pivot;
		else
			phase = 0x7ff;
		break;
	}

	//
	// apply window function
	//
	switch (voice.m_window & 7)
	{
	case 0: // none
		break;

	case 1: // sawtooth - falls from [0, 800)
		window = pos;
		break;

	case 2: // triangle - rises from [0, 400), falls from [400, 800)
		window = (pos & 0x3ff) * 2;
		if (pos < 0x400)
			window ^= 0x7fe;
		break;

	case 3: // trapezoid - falls from [400, 800)
		if (pos >= 0x400)
			window = (pos & 0x3ff) * 2;
		break;

	case 4: // pulse (undocumented) - falls from [0, 400)
		if (pos < 0x400)
			window = pos * 2;
		else
			window = 0x7ff;
		break;

	default: // double saw (undocumented) - rises from [0, 400) and [400, 800)
		window = (0x3ff ^ (pos & 0x3ff)) * 2;
		break;
	}

	sample = m_cosine[phase];
	if (window)
		sample = ((s32)sample * (0x800 - window)) / 0x800;

	// center sample around zero, apply volume and ring mod
	const u16 volume = m_dca[vnum].m_current >> ENV_DCA_SHIFT;
	sample = ((s32)sample * m_volume[volume]) >> VOLUME_SHIFT;
	sample -= m_volume[volume] / 2;

	if (voice.m_ring_mod)
		sample = ((s32)sample * m_last_sample) / 0x1000;

	// 'mute' actually negates the other voice in a modulating pair
	if (voice.m_mute_other)
		sample -= m_last_sample;

	//
	// update envelopes and pitch modulation, recalculate DCO pitch step if needed
	//
	const u32 old_dco = m_dco[vnum].m_current;
	const s16 old_pm = voice.m_pm_level;

	m_dca[vnum].update();
	m_dcw[vnum].update();
	m_dco[vnum].update();

	// pitch/noise modulation latches a new pitch multiplier every 8 samples
	if (!(m_sample_count & 7))
	{
		switch (voice.m_pitch_mod & 3)
		{
		default:
			voice.m_pm_level = 0;
			break;

		case 2:
			// pitch modulated by other voice (normally unused, up to about +/- 7.5 semitones)
			voice.m_pm_level = m_last_sample;
			break;

		case 3:
			// pitch modulated by noise (0 or 32 semitones above base pitch)
			voice.m_pm_level = machine().rand() & (32 << NOTE_SHIFT);
			break;
		}
	}

	if ((old_dco ^ m_dco[vnum].m_current) >> ENV_DCO_SHIFT
		|| old_pm != voice.m_pm_level)
		update_pitch_step(vnum);

	voice.m_position += voice.m_pitch_step;

	m_last_sample = sample;
	return sample;
}

/**************************************************************************/
void upd933_device::env_t::update()
{
	if (m_current != m_target)
	{
		if (!m_direction) // increasing
		{
			if (m_current > m_target
				|| m_target - m_current <= m_rate)
				m_current = m_target;
			else
				m_current += m_rate;
		}
		else // decreasing
		{
			if (m_current < m_target
				|| m_current - m_target <= m_rate)
				m_current = m_target;
			else
				m_current -= m_rate;
		}
	}

	if (!m_sustain && (m_current == m_target))
		m_irq = m_sustain = true; // set sustain too to make sure this only causes an interrupt once
}

/**************************************************************************/
bool upd933_device::env_t::calc_timeout(unsigned &samples)
{
	if (m_irq)
	{
		samples = 0;
	}
	else if (m_sustain || !m_rate)
	{
		return false;
	}
	else
	{
		const unsigned remaining = m_direction ? (m_current - m_target) : (m_target - m_current);
		unsigned new_time = remaining / m_rate;
		if (remaining % m_rate)
			new_time++;
		if (new_time < samples)
			samples = new_time;
	}

	return true;
}

/**************************************************************************/
void upd933_device::update_pitch_step(int vnum)
{
	voice_t &voice = m_voice[vnum];
	const s32 pitch = s32(voice.m_pitch + (m_dco[vnum].m_current >> ENV_DCO_SHIFT)) + voice.m_pm_level;
	u32 step = 0;

	if (pitch > 0 && pitch < (1 << 16))
	{
		const u8 note = pitch >> NOTE_SHIFT;
		const u16 fine = pitch & ((1 << NOTE_SHIFT) - 1);
		step = m_pitch[note];
		if (fine)
			step += (step >> PITCH_FINE_SHIFT) * m_pitch_fine[fine];
	}

	voice.m_pitch_step = step;

	/*
	The effective DCW envelope value is limited for higher pitch values.
	This allows e.g. narrow pulse waves to remain correctly audible
	and also prevents aliasing noise for extremely high pitch values.
	*/
	voice.m_dcw_limit = 0x400 - std::min(0x400U, (step >> (PITCH_SHIFT - 2)));
}
