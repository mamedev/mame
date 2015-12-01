// license:BSD-3-Clause
// copyright-holders:Miguel Angel Horna
/*
 * Sega System 32 Multi/Model 1/Model 2 custom PCM chip (315-5560) emulation.
 *
 * by Miguel Angel Horna (ElSemi) for Model 2 Emulator and MAME.
 * Information by R. Belmont and the YMF278B (OPL4) manual.
 *
 * voice registers:
 * 0: Pan
 * 1: Index of sample
 * 2: LSB of pitch (low 2 bits seem unused so)
 * 3: MSB of pitch (ooooppppppppppxx) (o=octave (4 bit signed), p=pitch (10 bits), x=unused?
 * 4: voice control: top bit = 1 for key on, 0 for key off
 * 5: bit 0: 0: interpolate volume changes, 1: direct set volume,
      bits 1-7 = volume attenuate (0=max, 7f=min)
 * 6: LFO frequency + Phase LFO depth
 * 7: Amplitude LFO size
 *
 * The first sample ROM contains a variable length table with 12
 * bytes per instrument/sample. This is very similar to the YMF278B.
 *
 * The first 3 bytes are the offset into the file (big endian). (0, 1, 2)
 * The next 2 are the loop start offset into the file (big endian) (3, 4)
 * The next 2 are the 2's complement of the total sample size (big endian) (5, 6)
 * The next byte is LFO freq + depth (copied to reg 6 ?) (7, 8)
 * The next 3 are envelope params (Attack, Decay1 and 2, sustain level, release, Key Rate Scaling) (9, 10, 11)
 * The next byte is Amplitude LFO size (copied to reg 7 ?)
 *
 * TODO
 * - The YM278B manual states that the chip supports 512 instruments. The MultiPCM probably supports them
 * too but the high bit position is unknown (probably reg 2 low bit). Any game use more than 256?
 *
 */

#include "emu.h"
#include "multipcm.h"

ALLOW_SAVE_TYPE(multipcm_device::state_t); // allow save_item on a non-fundamental type

/*******************************
        ENVELOPE SECTION
*******************************/

//Times are based on a 44100Hz timebase. It's adjusted to the actual sampling rate on startup

const double multipcm_device::BASE_TIMES[64] = {
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

const INT32 multipcm_device::VALUE_TO_CHANNEL[32] =
{
	0, 1, 2, 3, 4, 5, 6 , -1,
	7, 8, 9, 10,11,12,13, -1,
	14,15,16,17,18,19,20, -1,
	21,22,23,24,25,26,27, -1,
};

const UINT32 multipcm_device::TL_SHIFT = 12;
const UINT32 multipcm_device::EG_SHIFT = 16;

INT32 multipcm_device::envelope_generator_update(slot_t *slot)
{
	switch(slot->m_envelope_gen.m_state)
	{
		case ATTACK:
			slot->m_envelope_gen.m_volume += slot->m_envelope_gen.m_attack_rate;
			if (slot->m_envelope_gen.m_volume >= (0x3ff << EG_SHIFT))
			{
				slot->m_envelope_gen.m_state = DECAY1;
				if (slot->m_envelope_gen.m_decay1_rate >= (0x400 << EG_SHIFT)) //Skip DECAY1, go directly to DECAY2
				{
					slot->m_envelope_gen.m_state = DECAY2;
				}
				slot->m_envelope_gen.m_volume = 0x3ff << EG_SHIFT;
			}
			break;
		case DECAY1:
			slot->m_envelope_gen.m_volume -= slot->m_envelope_gen.m_decay1_rate;
			if (slot->m_envelope_gen.m_volume <= 0)
			{
				slot->m_envelope_gen.m_volume = 0;
			}
			if (slot->m_envelope_gen.m_volume >> EG_SHIFT <= (slot->m_envelope_gen.m_decay_level << 6))
			{
				slot->m_envelope_gen.m_state = DECAY2;
			}
			break;
		case DECAY2:
			slot->m_envelope_gen.m_volume -= slot->m_envelope_gen.m_decay2_rate;
			if (slot->m_envelope_gen.m_volume <= 0)
			{
				slot->m_envelope_gen.m_volume = 0;
			}
			break;
		case RELEASE:
			slot->m_envelope_gen.m_volume -= slot->m_envelope_gen.m_release_rate;
			if (slot->m_envelope_gen.m_volume <= 0)
			{
				slot->m_envelope_gen.m_volume = 0;
				slot->m_playing = false;
			}
			break;
		default:
			return 1 << TL_SHIFT;
	}

	return m_linear_to_exp_volume[slot->m_envelope_gen.m_volume >> EG_SHIFT];
}

UINT32 multipcm_device::get_rate(UINT32 *steps, UINT32 rate, UINT32 val)
{
	INT32 r = 4 * val + rate;
	if (val == 0)
	{
		return steps[0];
	}
	if (val == 0xf)
	{
		return steps[0x3f];
	}
	if (r > 0x3f)
	{
		r = 0x3f;
	}
	return steps[r];
}

void multipcm_device::envelope_generator_calc(slot_t *slot)
{
	INT32 octave = ((slot->m_regs[3] >> 4) - 1) & 0xf;
	if (octave & 8) {
		octave = octave - 16;
	}

	INT32 rate;
	if (slot->m_sample->m_key_rate_scale != 0xf)
	{
		rate = (octave + slot->m_sample->m_key_rate_scale) * 2 + ((slot->m_regs[3] >> 3) & 1);
	}
	else
	{
		rate = 0;
	}

	slot->m_envelope_gen.m_attack_rate = get_rate(m_attack_step, rate, slot->m_sample->m_attack_reg);
	slot->m_envelope_gen.m_decay1_rate = get_rate(m_decay_release_step, rate, slot->m_sample->m_decay1_reg);
	slot->m_envelope_gen.m_decay2_rate = get_rate(m_decay_release_step, rate, slot->m_sample->m_decay2_reg);
	slot->m_envelope_gen.m_release_rate = get_rate(m_decay_release_step, rate, slot->m_sample->m_release_reg);
	slot->m_envelope_gen.m_decay_level = 0xf - slot->m_sample->m_decay_level;

}

/*****************************
        LFO  SECTION
*****************************/

const UINT32 multipcm_device::LFO_SHIFT = 8;

const float multipcm_device::LFO_FREQ[8] = // In Hertz
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

const float multipcm_device::PHASE_SCALE_LIMIT[8] = // In Cents
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

const float multipcm_device::AMPLITUDE_SCALE_LIMIT[8] = // In Decibels
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

void multipcm_device::lfo_init()
{
	m_pitch_table = auto_alloc_array_clear(machine(), INT32, 256);
	m_amplitude_table = auto_alloc_array_clear(machine(), INT32, 256);
	for (INT32 i = 0; i < 256; ++i)
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

	m_pitch_scale_tables = auto_alloc_array_clear(machine(), INT32*, 8);
	m_amplitude_scale_tables = auto_alloc_array_clear(machine(), INT32*, 8);
	for (INT32 table = 0; table < 8; ++table)
	{
		float limit = PHASE_SCALE_LIMIT[table];
		m_pitch_scale_tables[table] = auto_alloc_array_clear(machine(), INT32, 256);
		for(INT32 i = -128; i < 128; ++i)
		{
			const float value = (limit * (float)i) / 128.0f;
			const float converted = powf(2.0f, value / 1200.0f);
			m_pitch_scale_tables[table][i + 128] = value_to_fixed(LFO_SHIFT, converted);
		}

		limit = -AMPLITUDE_SCALE_LIMIT[table];
		m_amplitude_scale_tables[table] = auto_alloc_array_clear(machine(), INT32, 256);
		for(INT32 i = 0; i < 256; ++i)
		{
			const float value = (limit * (float)i) / 256.0f;
			const float converted = powf(10.0f, value / 20.0f);
			m_amplitude_scale_tables[table][i] = value_to_fixed(LFO_SHIFT, converted);
		}
	}
}

UINT32 multipcm_device::value_to_fixed(const UINT32 bits, const float value)
{
	const float float_shift = (float)(1 << bits);
	return (UINT32)(float_shift * value);
}

INT32 multipcm_device::pitch_lfo_step(lfo_t *lfo)
{
	lfo->m_phase += lfo->m_phase_step;
	INT32 p = lfo->m_table[(lfo->m_phase >> LFO_SHIFT) & 0xff];
	p = lfo->m_scale[p];
	return p << (TL_SHIFT - LFO_SHIFT);
}

INT32 multipcm_device::amplitude_lfo_step(lfo_t *lfo)
{
	lfo->m_phase += lfo->m_phase_step;
	INT32 p = lfo->m_table[(lfo->m_phase >> LFO_SHIFT) & 0xff];
	p = lfo->m_scale[p];
	return p << (TL_SHIFT - LFO_SHIFT);
}

void multipcm_device::lfo_compute_step(lfo_t *lfo, UINT32 lfo_frequency, UINT32 lfo_scale, INT32 amplitude_lfo)
{
	float step = (float)LFO_FREQ[lfo_frequency] * 256.0f / (float)m_rate;
	lfo->m_phase_step = (UINT32)((float)(1 << LFO_SHIFT) * step);
	if (amplitude_lfo)
	{
		lfo->m_table = m_amplitude_table;
		lfo->m_scale = m_amplitude_scale_tables[lfo_scale];
	}
	else
	{
		lfo->m_table = m_pitch_table;
		lfo->m_scale = m_pitch_scale_tables[lfo_scale];
	}
}

void multipcm_device::write_slot(slot_t *slot, INT32 reg, UINT8 data)
{
	slot->m_regs[reg] = data;

	switch(reg)
	{
		case 0: // PANPOT
			slot->m_pan = (data >> 4) & 0xf;
			break;
		case 1: // Sample
		{
			//according to YMF278 sample write causes some base params written to the regs (envelope+lfos)
			//the game should never change the sample while playing.
			sample_t *sample = m_samples + slot->m_regs[1];
			write_slot(slot, 6, sample->m_lfo_vibrato_reg);
			write_slot(slot, 7, sample->m_lfo_amplitude_reg);
			break;
		}
		case 2: //Pitch
		case 3:
			{
				UINT32 oct = ((slot->m_regs[3] >> 4) - 1) & 0xf;
				UINT32 pitch = ((slot->m_regs[3] & 0xf) << 6) | (slot->m_regs[2] >> 2);
				pitch = m_freq_step_table[pitch];
				if (oct & 0x8)
				{
					pitch >>= (16 - oct);
				}
				else
				{
					pitch <<= oct;
				}
				slot->m_step = pitch / m_rate;
			}
			break;
		case 4:     //KeyOn/Off (and more?)
			if (data & 0x80)       //KeyOn
			{
				slot->m_sample = m_samples + slot->m_regs[1];
				slot->m_playing = true;
				slot->m_base = slot->m_sample->m_start;
				slot->m_offset = 0;
				slot->m_prev_sample = 0;
				slot->m_total_level = slot->m_dest_total_level << TL_SHIFT;

				envelope_generator_calc(slot);
				slot->m_envelope_gen.m_state = ATTACK;
				slot->m_envelope_gen.m_volume = 0;

				if (slot->m_base >= 0x100000)
				{
					if (slot->m_pan & 8)
					{
						slot->m_base = (slot->m_base & 0xfffff) | m_bank_left;
					}
					else
					{
						slot->m_base = (slot->m_base & 0xfffff) | m_bank_right;
					}
				}

			}
			else
			{
				if (slot->m_playing)
				{
					if (slot->m_sample->m_release_reg != 0xf)
					{
						slot->m_envelope_gen.m_state = RELEASE;
					}
					else
					{
						slot->m_playing = false;
					}
				}
			}
			break;
		case 5: // TL + Interpolation
			slot->m_dest_total_level = (data >> 1) & 0x7f;
			if (!(data & 1))   //Interpolate TL
			{
				if ((slot->m_total_level >> TL_SHIFT) > slot->m_dest_total_level)
				{
					slot->m_total_level_step = m_total_level_steps[0]; // decrease
				}
				else
				{
					slot->m_total_level_step = m_total_level_steps[1]; // increase
				}
			}
			else
			{
				slot->m_total_level = slot->m_dest_total_level << TL_SHIFT;
			}
			break;
		case 6: // LFO frequency + Pitch LFO
			if (data)
			{
				lfo_compute_step(&(slot->m_pitch_lfo), (slot->m_regs[6] >> 3) & 7, slot->m_regs[6] & 7, 0);
				lfo_compute_step(&(slot->m_amplitude_lfo), (slot->m_regs[6] >> 3) & 7, slot->m_regs[7] & 7, 1);
			}
			break;
		case 7: // Amplitude LFO
			if (data)
			{
				lfo_compute_step(&(slot->m_pitch_lfo), (slot->m_regs[6] >> 3) & 7, slot->m_regs[6] & 7, 0);
				lfo_compute_step(&(slot->m_amplitude_lfo), (slot->m_regs[6] >> 3) & 7, slot->m_regs[7] & 7, 1);
			}
			break;
	}
}

READ8_MEMBER( multipcm_device::read )
{
	return 0;
}


WRITE8_MEMBER( multipcm_device::write )
{
	switch(offset)
	{
		case 0:     //Data write
			write_slot(m_slots + m_cur_slot, m_address, data);
			break;
		case 1:
			m_cur_slot = VALUE_TO_CHANNEL[data & 0x1f];
			break;

		case 2:
			m_address = (data > 7) ? 7 : data;
			break;
	}
}

/* MAME/M1 access functions */

void multipcm_device::set_bank(UINT32 leftoffs, UINT32 rightoffs)
{
	m_bank_left = leftoffs;
	m_bank_right = rightoffs;
}

const device_type MULTIPCM = &device_creator<multipcm_device>;

// default address map
static ADDRESS_MAP_START( multipcm, AS_0, 8, multipcm_device )
	AM_RANGE(0x000000, 0x3fffff) AM_ROM
ADDRESS_MAP_END

multipcm_device::multipcm_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, MULTIPCM, "Sega/Yamaha 315-5560", tag, owner, clock, "multipcm", __FILE__),
		device_sound_interface(mconfig, *this),
		device_memory_interface(mconfig, *this),
		m_space_config("mpcm_samples", ENDIANNESS_LITTLE, 8, 24, 0, NULL),
		m_stream(NULL),
		m_samples(NULL),
		m_slots(NULL),
		m_cur_slot(0),
		m_address(0),
		m_bank_right(0),
		m_bank_left(0),
		m_rate(0),
		m_attack_step(NULL),
		m_decay_release_step(NULL),
		m_freq_step_table(NULL),
		m_direct(NULL),
		m_left_pan_table(NULL),
		m_right_pan_table(NULL),
		m_linear_to_exp_volume(NULL),
		m_total_level_steps(NULL),
		m_pitch_scale_tables(NULL),
		m_amplitude_scale_tables(NULL)
{
	m_address_map[0] = *ADDRESS_MAP_NAME(multipcm);
}

//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

const address_space_config *multipcm_device::memory_space_config(address_spacenum spacenum) const
{
	return (spacenum == 0) ? &m_space_config : NULL;
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void multipcm_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void multipcm_device::device_start()
{
	// find our direct access
	m_direct = &space().direct();

	const float clock_divider = 180.0f;
	m_rate = (float)clock() / clock_divider;

	m_stream = machine().sound().stream_alloc(*this, 0, 2, m_rate);

	// Volume + pan table
	m_left_pan_table = auto_alloc_array_clear(machine(), INT32, 0x800);
	m_right_pan_table = auto_alloc_array_clear(machine(), INT32, 0x800);
	for (INT32 level = 0; level < 0x80; ++level)
	{
		const float vol_db = (float)level * (-24.0f) / 64.0f;
		const float total_level = powf(10.0f, vol_db / 20.0f) / 4.0f;

		for (INT32 pan = 0; pan < 0x10; ++pan)
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

				const INT32 inverted_pan = 0x10 - pan;
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

	//Pitch steps
	m_freq_step_table = auto_alloc_array_clear(machine(), UINT32, 0x400);
	for (INT32 i = 0; i < 0x400; ++i)
	{
		const float fcent = m_rate * (1024.0f + (float)i) / 1024.0f;
		m_freq_step_table[i] = value_to_fixed(TL_SHIFT, fcent);
	}

	// Envelope steps
	m_attack_step = auto_alloc_array_clear(machine(), UINT32, 0x40);
	m_decay_release_step = auto_alloc_array_clear(machine(), UINT32, 0x40);
	const double attack_rate_to_decay_rate = 14.32833;
	for (INT32 i = 0; i < 0x40; ++i)
	{
		// Times are based on 44100Hz clock, adjust to real chip clock
		m_attack_step[i] = (float)(0x400 << EG_SHIFT) / (float)(BASE_TIMES[i] * 44100.0 / 1000.0);
		m_decay_release_step[i] = (float)(0x400 << EG_SHIFT) / (float)(BASE_TIMES[i] * attack_rate_to_decay_rate * 44100.0 / 1000.0);
	}
	m_attack_step[0] = m_attack_step[1] = m_attack_step[2] = m_attack_step[3] = 0;
	m_attack_step[0x3f] = 0x400 << EG_SHIFT;
	m_decay_release_step[0] = m_decay_release_step[1] = m_decay_release_step[2] = m_decay_release_step[3] = 0;

	// Total level interpolation steps
	m_total_level_steps = auto_alloc_array_clear(machine(), INT32, 2);
	m_total_level_steps[0] = -(float)(0x80 << TL_SHIFT) / (78.2f * 44100.0f / 1000.0f); // lower
	m_total_level_steps[1] = (float)(0x80 << TL_SHIFT) / (78.2f * 2 * 44100.0f / 1000.0f); // raise

	// build the linear->exponential ramps
	m_linear_to_exp_volume = auto_alloc_array_clear(machine(), INT32, 0x400);
	for(INT32 i = 0; i < 0x400; ++i)
	{
		const float db = -(96.0f - (96.0f * (float)i / (float)0x400));
		const float exp_volume = powf(10.0f, db / 20.0f);
		m_linear_to_exp_volume[i] = value_to_fixed(TL_SHIFT, exp_volume);
	}

	// Samples
	m_samples = auto_alloc_array_clear(machine(), sample_t, 0x200);
	for(INT32 sample = 0; sample < 0x200; ++sample)
	{
		UINT8 data[12];

		for (INT32 sample_byte = 0; sample_byte < 12; sample_byte++)
		{
			data[sample_byte] = (UINT8)m_direct->read_byte((sample * 12) + sample_byte);
		}

		m_samples[sample].m_start = (data[0] << 16) | (data[1] << 8) | (data[2] << 0);
		m_samples[sample].m_loop = (data[3] << 8) | (data[4] << 0);
		m_samples[sample].m_end = 0xffff - ((data[5] << 8) | (data[6] << 0));
		m_samples[sample].m_lfo_vibrato_reg = data[7];
		m_samples[sample].m_decay1_reg = data[8] & 0xf;
		m_samples[sample].m_attack_reg = (data[8] >> 4) & 0xf;
		m_samples[sample].m_decay2_reg = data[9] & 0xf;
		m_samples[sample].m_decay_level = (data[9] >> 4) & 0xf;
		m_samples[sample].m_release_reg = data[10] & 0xf;
		m_samples[sample].m_key_rate_scale = (data[10] >> 4) & 0xf;
		m_samples[sample].m_lfo_amplitude_reg = data[11];
	}

	save_item(NAME(m_cur_slot));
	save_item(NAME(m_address));
	save_item(NAME(m_bank_left));
	save_item(NAME(m_bank_right));

	// Slots
	m_slots = auto_alloc_array_clear(machine(), slot_t, 28);
	for (INT32 slot = 0; slot < 28; ++slot)
	{
		m_slots[slot].m_slot_index = slot;
		m_slots[slot].m_playing = false;

		save_item(NAME(m_slots[slot].m_slot_index), slot);
		save_item(NAME(m_slots[slot].m_regs), slot);
		save_item(NAME(m_slots[slot].m_playing), slot);
		save_item(NAME(m_slots[slot].m_base), slot);
		save_item(NAME(m_slots[slot].m_offset), slot);
		save_item(NAME(m_slots[slot].m_step), slot);
		save_item(NAME(m_slots[slot].m_pan), slot);
		save_item(NAME(m_slots[slot].m_total_level), slot);
		save_item(NAME(m_slots[slot].m_dest_total_level), slot);
		save_item(NAME(m_slots[slot].m_total_level_step), slot);
		save_item(NAME(m_slots[slot].m_prev_sample), slot);
		save_item(NAME(m_slots[slot].m_envelope_gen.m_volume), slot);
		save_item(NAME(m_slots[slot].m_envelope_gen.m_state), slot);
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

//-----------------------------------------------------
//  clamp_to_int16 - clamp a 32-bit value to 16 bits
//-----------------------------------------------------

INT16 multipcm_device::clamp_to_int16(INT32 value)
{
	if (value < -32768)
	{
		return -32768;
	}
	else if (value > 32767)
	{
		return 32767;
	}
	return (INT16)value;
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void multipcm_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, INT32 samples)
{
	stream_sample_t  *datap[2];

	datap[0] = outputs[0];
	datap[1] = outputs[1];

	memset(datap[0], 0, sizeof(*datap[0]) * samples);
	memset(datap[1], 0, sizeof(*datap[1]) * samples);

	for (INT32 i = 0; i < samples; ++i)
	{
		INT32 smpl = 0;
		INT32 smpr = 0;
		for (INT32 sl = 0; sl < 28; ++sl)
		{
			slot_t *slot = m_slots + sl;
			if (slot->m_playing)
			{
				UINT32 vol = (slot->m_total_level >> TL_SHIFT) | (slot->m_pan << 7);
				UINT32 adr = slot->m_offset >> TL_SHIFT;
				UINT32 step = slot->m_step;
				INT32 csample = (INT16) (m_direct->read_byte(slot->m_base + adr) << 8);
				INT32 fpart = slot->m_offset & ((1 << TL_SHIFT) - 1);
				INT32 sample = (csample * fpart + slot->m_prev_sample * ((1 << TL_SHIFT) - fpart)) >> TL_SHIFT;

				if (slot->m_regs[6] & 7) // Vibrato enabled
				{
					step = step * pitch_lfo_step(&(slot->m_pitch_lfo));
					step >>= TL_SHIFT;
				}

				slot->m_offset += step;
				if (slot->m_offset >= (slot->m_sample->m_end << TL_SHIFT))
				{
					slot->m_offset = slot->m_sample->m_loop << TL_SHIFT;
				}

				if (adr ^ (slot->m_offset >> TL_SHIFT))
				{
					slot->m_prev_sample = csample;
				}

				if ((slot->m_total_level >> TL_SHIFT) != slot->m_dest_total_level)
				{
					slot->m_total_level += slot->m_total_level_step;
				}

				if (slot->m_regs[7] & 7) // Tremolo enabled
				{
					sample = sample * amplitude_lfo_step(&(slot->m_amplitude_lfo));
					sample >>= TL_SHIFT;
				}

				sample = (sample * envelope_generator_update(slot)) >> 10;

				smpl += (m_left_pan_table[vol] * sample) >> TL_SHIFT;
				smpr += (m_right_pan_table[vol] * sample) >> TL_SHIFT;
			}
		}

		datap[0][i] = clamp_to_int16(smpl);
		datap[1][i] = clamp_to_int16(smpr);
	}
}
