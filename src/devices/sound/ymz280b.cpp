// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*

 Yamaha YMZ280B driver
  by Aaron Giles

  YMZ280B 8-Channel PCMD8 PCM/ADPCM Decoder

 Features as listed in LSI-4MZ280B3 data sheet:
  Voice data stored in external memory can be played back simultaneously for up to eight voices
  Voice data format can be selected from 4-bit ADPCM, 8-bit PCM and 16-bit PCM
  Control of voice data external memory
   Up to 16M bytes of ROM or SRAM (x 8 bits, access time 150ms max) can be connected
   Continuous access is possible
   Loop playback between selective addresses is possible
  Voice data playback frequency control
   4-bit ADPCM ................ 0.172 to 44.1kHz in 256 steps
   8-bit PCM, 16-bit PCM ...... 0.172 to 88.2kHz in 512 steps
  256 steps total level and 16 steps panpot can be set
  Voice signal is output in stereo 16-bit 2's complement MSB-first format

  TODO:
  - Is memory handling 100% correct? At the moment, Konami firebeat.c is the only
    hardware currently emulated that uses external handlers.
    It also happens to be the only one using 16-bit PCM.

    Some other drivers (eg. bishi.cpp, bfm_sc4/5.cpp) also use ROM readback.

*/


#include "emu.h"
#include "ymz280b.h"

#if YMZ280B_MAKE_WAVS
#include "sound/wavwrite.h"
#endif


#define MAX_SAMPLE_CHUNK    10000

static constexpr unsigned FRAC_BITS = 9;
static constexpr s32      FRAC_ONE = 1 << FRAC_BITS;

//#define INTERNAL_BUFFER_SIZE    (1 << 15)
#define INTERNAL_SAMPLE_RATE    (m_master_clock * 2.0)



/* step size index shift table */
static constexpr int index_scale[8] = { 0x0e6, 0x0e6, 0x0e6, 0x0e6, 0x133, 0x199, 0x200, 0x266 };

/* lookup table for the precomputed difference */
static int diff_lookup[16];


void ymz280b_device::update_irq_state()
{
	int irq_bits = m_status_register & m_irq_mask;

	/* always off if the enable is off */
	if (!m_irq_enable)
		irq_bits = 0;

	/* update the state if changed */
	if (irq_bits && !m_irq_state)
	{
		m_irq_state = 1;
		if (!m_irq_handler.isunset())
			m_irq_handler(1);
		else
			logerror("YMZ280B: IRQ generated, but no callback specified!\n");
	}
	else if (!irq_bits && m_irq_state)
	{
		m_irq_state = 0;
		if (!m_irq_handler.isunset())
			m_irq_handler(0);
		else
			logerror("YMZ280B: IRQ generated, but no callback specified!\n");
	}
}


void ymz280b_device::update_step(YMZ280BVoice *voice)
{
	int frequency;

	/* compute the frequency */
	if (voice->mode == 1)
		frequency = voice->fnum & 0x0ff;
	else
		frequency = voice->fnum & 0x1ff;
	voice->output_step = frequency + 1; // ((fnum + 1) * (input clock / 384)) / 256
}


void ymz280b_device::update_volumes(YMZ280BVoice *voice)
{
	if (voice->pan == 8)
	{
		voice->output_left = voice->level;
		voice->output_right = voice->level;
	}
	else if (voice->pan < 8)
	{
		voice->output_left = voice->level;

		/* pan 1 is hard-left, what's pan 0? for now assume same as pan 1 */
		voice->output_right = (voice->pan == 0) ? 0 : voice->level * (voice->pan - 1) / 7;
	}
	else
	{
		voice->output_left = voice->level * (15 - voice->pan) / 7;
		voice->output_right = voice->level;
	}
}


void ymz280b_device::device_post_load()
{
	for (size_t i = 0; i < std::size(m_voice); i++)
	{
		YMZ280BVoice *voice = &m_voice[i];
		update_step(voice);
		if(voice->irq_schedule)
			voice->timer->adjust(attotime::zero, i);
	}
}


TIMER_CALLBACK_MEMBER(ymz280b_device::update_irq_state_timer_common)
{
	YMZ280BVoice *voice = &m_voice[param];

	if(!voice->irq_schedule) return;

	voice->playing = 0;
	m_status_register |= 1 << param;
	update_irq_state();
	voice->irq_schedule = 0;
}

/**********************************************************************************************

     compute_tables -- compute the difference tables

***********************************************************************************************/

static void compute_tables()
{
	/* loop over all nibbles and compute the difference */
	for (int nib = 0; nib < 16; nib++)
	{
		int value = (nib & 0x07) * 2 + 1;
		diff_lookup[nib] = (nib & 0x08) ? -value : value;
	}
}



/**********************************************************************************************

     generate_adpcm -- general ADPCM decoding routine

***********************************************************************************************/

int ymz280b_device::generate_adpcm(YMZ280BVoice *voice, s16 *buffer, int samples)
{
	int position = voice->position;
	int signal = voice->signal;
	int step = voice->step;
	int val;

	/* two cases: first cases is non-looping */
	if (!voice->looping)
	{
		/* loop while we still have samples to generate */
		while (samples)
		{
			/* compute the new amplitude and update the current step */
			val = read_byte(position / 2) >> ((~position & 1) << 2);
			signal += (step * diff_lookup[val & 15]) / 8;

			/* clamp to the maximum */
			if (signal > 32767)
				signal = 32767;
			else if (signal < -32768)
				signal = -32768;

			/* adjust the step size and clamp */
			step = (step * index_scale[val & 7]) >> 8;
			if (step > 0x6000)
				step = 0x6000;
			else if (step < 0x7f)
				step = 0x7f;

			/* output to the buffer, scaling by the volume */
			*buffer++ = signal;
			samples--;

			/* next! */
			position++;
			if (position >= voice->stop)
			{
				voice->ended = true;
				break;
			}
		}
	}

	/* second case: looping */
	else
	{
		/* loop while we still have samples to generate */
		while (samples)
		{
			/* compute the new amplitude and update the current step */
			val = read_byte(position / 2) >> ((~position & 1) << 2);
			signal += (step * diff_lookup[val & 15]) / 8;

			/* clamp to the maximum */
			if (signal > 32767)
				signal = 32767;
			else if (signal < -32768)
				signal = -32768;

			/* adjust the step size and clamp */
			step = (step * index_scale[val & 7]) >> 8;
			if (step > 0x6000)
				step = 0x6000;
			else if (step < 0x7f)
				step = 0x7f;

			/* output to the buffer, scaling by the volume */
			*buffer++ = signal;
			samples--;

			/* next! */
			position++;
			if (position == voice->loop_start && voice->loop_count == 0)
			{
				voice->loop_signal = signal;
				voice->loop_step = step;
			}
			if (position >= voice->loop_end)
			{
				if (voice->keyon)
				{
					position = voice->loop_start;
					signal = voice->loop_signal;
					step = voice->loop_step;
					voice->loop_count++;
				}
			}
			if (position >= voice->stop)
			{
				voice->ended = true;
				break;
			}
		}
	}

	/* update the parameters */
	voice->position = position;
	voice->signal = signal;
	voice->step = step;

	return samples;
}



/**********************************************************************************************

     generate_pcm8 -- general 8-bit PCM decoding routine

***********************************************************************************************/

int ymz280b_device::generate_pcm8(YMZ280BVoice *voice, s16 *buffer, int samples)
{
	int position = voice->position;
	int val;

	/* two cases: first cases is non-looping */
	if (!voice->looping)
	{
		/* loop while we still have samples to generate */
		while (samples)
		{
			/* fetch the current value */
			val = read_byte(position / 2);

			/* output to the buffer, scaling by the volume */
			*buffer++ = (s8)val * 256;
			samples--;

			/* next! */
			position += 2;
			if (position >= voice->stop)
			{
				voice->ended = true;
				break;
			}
		}
	}

	/* second case: looping */
	else
	{
		/* loop while we still have samples to generate */
		while (samples)
		{
			/* fetch the current value */
			val = read_byte(position / 2);

			/* output to the buffer, scaling by the volume */
			*buffer++ = (s8)val * 256;
			samples--;

			/* next! */
			position += 2;
			if (position >= voice->loop_end)
			{
				if (voice->keyon)
					position = voice->loop_start;
			}
			if (position >= voice->stop)
			{
				voice->ended = true;
				break;
			}
		}
	}

	/* update the parameters */
	voice->position = position;

	return samples;
}



/**********************************************************************************************

     generate_pcm16 -- general 16-bit PCM decoding routine

***********************************************************************************************/

int ymz280b_device::generate_pcm16(YMZ280BVoice *voice, s16 *buffer, int samples)
{
	int position = voice->position;
	int val;

	/* two cases: first cases is non-looping */
	if (!voice->looping)
	{
		/* loop while we still have samples to generate */
		while (samples)
		{
			/* fetch the current value */
			val = (s16)((read_byte(position / 2 + 1) << 8) + read_byte(position / 2 + 0));

			/* output to the buffer, scaling by the volume */
			*buffer++ = val;
			samples--;

			/* next! */
			position += 4;
			if (position >= voice->stop)
			{
				voice->ended = true;
				break;
			}
		}
	}

	/* second case: looping */
	else
	{
		/* loop while we still have samples to generate */
		while (samples)
		{
			/* fetch the current value */
			val = (s16)((read_byte(position / 2 + 1) << 8) + read_byte(position / 2 + 0));

			/* output to the buffer, scaling by the volume */
			*buffer++ = val;
			samples--;

			/* next! */
			position += 4;
			if (position >= voice->loop_end)
			{
				if (voice->keyon)
					position = voice->loop_start;
			}
			if (position >= voice->stop)
			{
				voice->ended = true;
				break;
			}
		}
	}

	/* update the parameters */
	voice->position = position;

	return samples;
}




//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void ymz280b_device::sound_stream_update(sound_stream &stream)
{
	int v;

	/* loop over voices */
	for (v = 0; v < 8; v++)
	{
		YMZ280BVoice *voice = &m_voice[v];
		s16 prev = voice->last_sample;
		s16 curr = voice->curr_sample;
		s16 *curr_data = m_scratch.get();
		s32 sampindex = 0;
		u32 new_samples, samples_left;
		u32 final_pos;
		int remaining = stream.samples();
		int lvol = voice->output_left;
		int rvol = voice->output_right;

		/* quick out if we're not playing and we're at 0 */
		if (!voice->playing && curr == 0 && prev == 0)
		{
			/* make sure next sound plays immediately */
			voice->output_pos = FRAC_ONE;
			continue;
		}

		/* finish off the current sample */
		/* interpolate */
		while (remaining > 0 && voice->output_pos < FRAC_ONE)
		{
			int interp_sample = ((s32(prev) * (FRAC_ONE - voice->output_pos)) + (s32(curr) * voice->output_pos)) >> FRAC_BITS;
			stream.add_int(0, sampindex, interp_sample * lvol / 2, 32768 * 256);
			stream.add_int(1, sampindex, interp_sample * rvol / 2, 32768 * 256);
			sampindex++;
			voice->output_pos += voice->output_step;
			remaining--;
		}

		/* if we're over, continue; otherwise, we're done */
		if (voice->output_pos >= FRAC_ONE)
			voice->output_pos -= FRAC_ONE;
		else
			continue;

		/* compute how many new samples we need */
		final_pos = voice->output_pos + remaining * voice->output_step;
		new_samples = (final_pos + FRAC_ONE) >> FRAC_BITS;
		if (new_samples > MAX_SAMPLE_CHUNK)
			new_samples = MAX_SAMPLE_CHUNK;
		samples_left = new_samples;

		/* generate them into our buffer */
		switch (voice->playing << 7 | voice->mode)
		{
			case 0x81:  samples_left = generate_adpcm(voice, m_scratch.get(), new_samples); break;
			case 0x82:  samples_left = generate_pcm8(voice, m_scratch.get(), new_samples); break;
			case 0x83:  samples_left = generate_pcm16(voice, m_scratch.get(), new_samples); break;
			default:    samples_left = 0; memset(m_scratch.get(), 0, new_samples * sizeof(m_scratch[0])); break;
		}

		if (samples_left || voice->ended)
		{
			voice->ended = false;

			/* if there are leftovers, ramp back to 0 */
			int base = new_samples - samples_left;
			int i, t = (base == 0) ? curr : m_scratch[base - 1];
			for (i = 0; i < samples_left; i++)
			{
				if (t < 0) t = -((-t * 15) >> 4);
				else if (t > 0) t = (t * 15) >> 4;
				m_scratch[base + i] = t;
			}

			/* if we hit the end and IRQs are enabled, signal it */
			if (base != 0)
			{
				voice->playing = 0;

				/* set update_irq_state_timer. IRQ is signaled on next CPU execution. */
				voice->timer->adjust(attotime::zero, v);
				voice->irq_schedule = 1;
			}
		}

		/* advance forward one sample */
		prev = curr;
		curr = *curr_data++;

		/* then sample-rate convert with linear interpolation */
		while (remaining > 0)
		{
			/* interpolate */
			while (remaining > 0 && voice->output_pos < FRAC_ONE)
			{
				int interp_sample = ((s32(prev) * (FRAC_ONE - voice->output_pos)) + (s32(curr) * voice->output_pos)) >> FRAC_BITS;
				stream.add_int(0, sampindex, interp_sample * lvol / 2, 32768 * 256);
				stream.add_int(1, sampindex, interp_sample * rvol / 2, 32768 * 256);
				sampindex++;
				voice->output_pos += voice->output_step;
				remaining--;
			}

			/* if we're over, grab the next samples */
			if (voice->output_pos >= FRAC_ONE)
			{
				voice->output_pos -= FRAC_ONE;
				prev = curr;
				curr = *curr_data++;
			}
		}

		/* remember the last samples */
		voice->last_sample = prev;
		voice->curr_sample = curr;
	}
}



//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ymz280b_device::device_start()
{
	/* compute ADPCM tables */
	compute_tables();

	/* initialize the rest of the structure */
	m_master_clock = (double)clock() / 384.0;

	for (int i = 0; i < 8; i++)
	{
		m_voice[i].timer = timer_alloc(FUNC(ymz280b_device::update_irq_state_timer_common), this);
	}

	/* create the stream */
	m_stream = stream_alloc(0, 2, INTERNAL_SAMPLE_RATE);

	/* allocate memory */
	assert(MAX_SAMPLE_CHUNK < 0x10000);
	m_scratch = std::make_unique<s16[]>(MAX_SAMPLE_CHUNK);

	/* state save */
	save_item(NAME(m_current_register));
	save_item(NAME(m_status_register));
	save_item(NAME(m_irq_state));
	save_item(NAME(m_irq_mask));
	save_item(NAME(m_irq_enable));
	save_item(NAME(m_keyon_enable));
	save_item(NAME(m_ext_mem_enable));
	save_item(NAME(m_ext_mem_address));
	save_item(NAME(m_ext_readlatch));
	save_item(NAME(m_ext_mem_address_hi));
	save_item(NAME(m_ext_mem_address_mid));

	save_item(STRUCT_MEMBER(m_voice, playing));
	save_item(STRUCT_MEMBER(m_voice, ended));
	save_item(STRUCT_MEMBER(m_voice, keyon));
	save_item(STRUCT_MEMBER(m_voice, looping));
	save_item(STRUCT_MEMBER(m_voice, mode));
	save_item(STRUCT_MEMBER(m_voice, fnum));
	save_item(STRUCT_MEMBER(m_voice, level));
	save_item(STRUCT_MEMBER(m_voice, pan));
	save_item(STRUCT_MEMBER(m_voice, start));
	save_item(STRUCT_MEMBER(m_voice, stop));
	save_item(STRUCT_MEMBER(m_voice, loop_start));
	save_item(STRUCT_MEMBER(m_voice, loop_end));
	save_item(STRUCT_MEMBER(m_voice, position));
	save_item(STRUCT_MEMBER(m_voice, signal));
	save_item(STRUCT_MEMBER(m_voice, step));
	save_item(STRUCT_MEMBER(m_voice, loop_signal));
	save_item(STRUCT_MEMBER(m_voice, loop_step));
	save_item(STRUCT_MEMBER(m_voice, loop_count));
	save_item(STRUCT_MEMBER(m_voice, output_left));
	save_item(STRUCT_MEMBER(m_voice, output_right));
	save_item(STRUCT_MEMBER(m_voice, output_pos));
	save_item(STRUCT_MEMBER(m_voice, last_sample));
	save_item(STRUCT_MEMBER(m_voice, curr_sample));
	save_item(STRUCT_MEMBER(m_voice, irq_schedule));

#if YMZ280B_MAKE_WAVS
	m_wavresample = wav_open("resamp.wav", INTERNAL_SAMPLE_RATE, 2);
#endif
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void ymz280b_device::device_reset()
{
	/* initial clear registers */
	for (int i = 0xff; i >= 0; i--)
	{
		m_current_register = i;
		write_to_register(0);
	}

	m_current_register = 0;
	m_status_register = 0;
	m_ext_mem_address = 0;

	/* clear other voice parameters */
	for (auto &elem : m_voice)
	{
		YMZ280BVoice *voice = &elem;

		voice->curr_sample = 0;
		voice->last_sample = 0;
		voice->output_pos = FRAC_ONE;
		voice->playing = 0;
	}
}


void ymz280b_device::device_clock_changed()
{
	m_master_clock = (double)clock() / 384.0;
	m_stream->set_sample_rate(INTERNAL_SAMPLE_RATE);
}


void ymz280b_device::rom_bank_pre_change()
{
	m_stream->update();
}


/**********************************************************************************************

     write_to_register -- handle a write to the current register

***********************************************************************************************/

void ymz280b_device::write_to_register(int data)
{
	/* lower registers follow a pattern */
	if (m_current_register < 0x80)
	{
		YMZ280BVoice *voice = &m_voice[(m_current_register >> 2) & 7];

		switch (m_current_register & 0xe3)
		{
			case 0x00:      /* pitch low 8 bits */
				voice->fnum = (voice->fnum & 0x100) | (data & 0xff);
				update_step(voice);
				break;

			case 0x01:      /* pitch upper 1 bit, loop, key on, mode */
				voice->fnum = (voice->fnum & 0xff) | ((data & 0x01) << 8);
				voice->looping = (data & 0x10) >> 4;
				if ((data & 0x60) == 0)
					data &= 0x7f; /* ignore mode setting and set to same state as KON=0 */
				else
					voice->mode = (data & 0x60) >> 5;
				if (!voice->keyon && (data & 0x80) && m_keyon_enable)
				{
					voice->playing = 1;
					voice->position = voice->start;
					voice->signal = voice->loop_signal = 0;
					voice->step = voice->loop_step = 0x7f;
					voice->loop_count = 0;

					/* if update_irq_state_timer is set, cancel it. */
					voice->irq_schedule = 0;
				}
				else if (voice->keyon && !(data & 0x80))
				{
					voice->playing = 0;

					/* if update_irq_state_timer is set, cancel it. */
					voice->irq_schedule = 0;
				}
				voice->keyon = (data & 0x80) >> 7;
				update_step(voice);
				break;

			case 0x02:      /* total level */
				voice->level = data;
				update_volumes(voice);
				break;

			case 0x03:      /* pan */
				voice->pan = data & 0x0f;
				update_volumes(voice);
				break;

			case 0x20:      /* start address high */
				voice->start = (voice->start & (0x00ffff << 1)) | (data << 17);
				break;

			case 0x21:      /* loop start address high */
				voice->loop_start = (voice->loop_start & (0x00ffff << 1)) | (data << 17);
				break;

			case 0x22:      /* loop end address high */
				voice->loop_end = (voice->loop_end & (0x00ffff << 1)) | (data << 17);
				break;

			case 0x23:      /* stop address high */
				voice->stop = (voice->stop & (0x00ffff << 1)) | (data << 17);
				break;

			case 0x40:      /* start address middle */
				voice->start = (voice->start & (0xff00ff << 1)) | (data << 9);
				break;

			case 0x41:      /* loop start address middle */
				voice->loop_start = (voice->loop_start & (0xff00ff << 1)) | (data << 9);
				break;

			case 0x42:      /* loop end address middle */
				voice->loop_end = (voice->loop_end & (0xff00ff << 1)) | (data << 9);
				break;

			case 0x43:      /* stop address middle */
				voice->stop = (voice->stop & (0xff00ff << 1)) | (data << 9);
				break;

			case 0x60:      /* start address low */
				voice->start = (voice->start & (0xffff00 << 1)) | (data << 1);
				break;

			case 0x61:      /* loop start address low */
				voice->loop_start = (voice->loop_start & (0xffff00 << 1)) | (data << 1);
				break;

			case 0x62:      /* loop end address low */
				voice->loop_end = (voice->loop_end & (0xffff00 << 1)) | (data << 1);
				break;

			case 0x63:      /* stop address low */
				voice->stop = (voice->stop & (0xffff00 << 1)) | (data << 1);
				break;

			default:
				if (data != 0)
					logerror("YMZ280B: unknown register write %02X = %02X\n", m_current_register, data);
				break;
		}
	}

	/* upper registers are special */
	else
	{
		switch (m_current_register)
		{
			/* DSP related (not implemented yet) */
			case 0x80: // d0-2: DSP Rch, d3: enable Rch (0: yes, 1: no), d4-6: DSP Lch, d7: enable Lch (0: yes, 1: no)
			case 0x81: // d0: enable control of $82 (0: yes, 1: no)
			case 0x82: // DSP data
				logerror("YMZ280B: DSP register write %02X = %02X\n", m_current_register, data);
				break;

			case 0x84:      /* ROM readback / RAM write (high) */
				m_ext_mem_address_hi = data << 16;
				break;

			case 0x85:      /* ROM readback / RAM write (middle) */
				m_ext_mem_address_mid = data << 8;
				break;

			case 0x86:      /* ROM readback / RAM write (low) -> update latch */
				m_ext_mem_address = m_ext_mem_address_hi | m_ext_mem_address_mid | data;
				if (m_ext_mem_enable)
					m_ext_readlatch = read_byte(m_ext_mem_address);
				break;

			case 0x87:      /* RAM write */
				if (m_ext_mem_enable)
				{
					space(0).write_byte(m_ext_mem_address, data);
					m_ext_mem_address = (m_ext_mem_address + 1) & 0xffffff;
				}
				break;

			case 0xfe:      /* IRQ mask */
				m_irq_mask = data;
				update_irq_state();
				break;

			case 0xff:      /* IRQ enable, test, etc */
				m_ext_mem_enable = (data & 0x40) >> 6;
				m_irq_enable = (data & 0x10) >> 4;
				update_irq_state();

				if (m_keyon_enable && !(data & 0x80))
				{
					for (int i = 0; i < 8; i++)
					{
						m_voice[i].playing = 0;

						/* if update_irq_state_timer is set, cancel it. */
						m_voice[i].irq_schedule = 0;
					}
				}
				else if (!m_keyon_enable && (data & 0x80))
				{
					for (int i = 0; i < 8; i++)
					{
						if (m_voice[i].keyon && m_voice[i].looping)
							m_voice[i].playing = 1;
					}
				}
				m_keyon_enable = (data & 0x80) >> 7;
				break;

			default:
				if (data != 0)
					logerror("YMZ280B: unknown register write %02X = %02X\n", m_current_register, data);
				break;
		}
	}
}



/**********************************************************************************************

     compute_status -- determine the status bits

***********************************************************************************************/

int ymz280b_device::compute_status()
{
	u8 result;

	/* force an update */
	m_stream->update();

	result = m_status_register;

	if (!machine().side_effects_disabled())
	{
		/* clear the IRQ state */
		m_status_register = 0;
		update_irq_state();
	}
	return result;
}



/**********************************************************************************************

     read/write -- handle external accesses

***********************************************************************************************/

u8 ymz280b_device::read(offs_t offset)
{
	if ((offset & 1) == 0)
	{
		if (!m_ext_mem_enable)
			return 0xff;

		/* read from external memory */
		u8 ret = m_ext_readlatch;
		m_ext_readlatch = read_byte(m_ext_mem_address);
		if (!machine().side_effects_disabled())
			m_ext_mem_address = (m_ext_mem_address + 1) & 0xffffff;
		return ret;
	}
	else
		return compute_status();
}


void ymz280b_device::write(offs_t offset, u8 data)
{
	if ((offset & 1) == 0)
		m_current_register = data;
	else
	{
		/* force an update */
		m_stream->update();

		write_to_register(data);
	}
}


DEFINE_DEVICE_TYPE(YMZ280B, ymz280b_device, "ymz280b", "Yamaha YMZ280B PCMD8")

ymz280b_device::ymz280b_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, YMZ280B, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, device_rom_interface(mconfig, *this)
	, m_current_register(0)
	, m_status_register(0)
	, m_irq_state(0)
	, m_irq_mask(0)
	, m_irq_enable(0)
	, m_keyon_enable(0)
	, m_ext_mem_enable(0)
	, m_ext_readlatch(0)
	, m_ext_mem_address_hi(0)
	, m_ext_mem_address_mid(0)
	, m_ext_mem_address(0)
	, m_irq_handler(*this)
{
	memset(m_voice, 0, sizeof(m_voice));
}
