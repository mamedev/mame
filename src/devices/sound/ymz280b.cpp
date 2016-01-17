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

    Some other drivers (eg. bishi.c, bfm_sc4/5.c) also use ROM readback.

*/


#include "emu.h"
#include "ymz280b.h"


#define MAX_SAMPLE_CHUNK    10000

#define FRAC_BITS           14
#define FRAC_ONE            (1 << FRAC_BITS)
#define FRAC_MASK           (FRAC_ONE - 1)

#define INTERNAL_BUFFER_SIZE    (1 << 15)
#define INTERNAL_SAMPLE_RATE    (m_master_clock * 2.0)

#if MAKE_WAVS
#include "sound/wavwrite.h"
#endif



/* step size index shift table */
static const int index_scale[8] = { 0x0e6, 0x0e6, 0x0e6, 0x0e6, 0x133, 0x199, 0x200, 0x266 };

/* lookup table for the precomputed difference */
static int diff_lookup[16];


UINT8 ymz280b_device::ymz280b_read_memory(UINT32 offset)
{
	if (m_ext_read_handler.isnull())
	{
		if (offset < m_mem_size)
			return m_mem_base[offset];

		/* 16MB chip limit (shouldn't happen) */
		else if (offset > 0xffffff)
			return m_mem_base[offset & 0xffffff];

		else
			return 0;
	}
	else
		return m_ext_read_handler(offset);
}


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
		if (!m_irq_handler.isnull())
			m_irq_handler(1);
		else logerror("YMZ280B: IRQ generated, but no callback specified!\n");
	}
	else if (!irq_bits && m_irq_state)
	{
		m_irq_state = 0;
		if (!m_irq_handler.isnull())
			m_irq_handler(0);
		else logerror("YMZ280B: IRQ generated, but no callback specified!\n");
	}
}


void ymz280b_device::update_step(struct YMZ280BVoice *voice)
{
	double frequency;

	/* compute the frequency */
	if (voice->mode == 1)
		frequency = m_master_clock * (double)((voice->fnum & 0x0ff) + 1) * (1.0 / 256.0);
	else
		frequency = m_master_clock * (double)((voice->fnum & 0x1ff) + 1) * (1.0 / 256.0);
	voice->output_step = (UINT32)(frequency * (double)FRAC_ONE / INTERNAL_SAMPLE_RATE);
}


void ymz280b_device::update_volumes(struct YMZ280BVoice *voice)
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
	for (auto & elem : m_voice)
	{
		struct YMZ280BVoice *voice = &elem;
		update_step(voice);
		if(voice->irq_schedule)
			voice->timer->adjust(attotime::zero);
	}
}


void ymz280b_device::update_irq_state_timer_common(int voicenum)
{
	struct YMZ280BVoice *voice = &m_voice[voicenum];

	if(!voice->irq_schedule) return;

	voice->playing = 0;
	m_status_register |= 1 << voicenum;
	update_irq_state();
	voice->irq_schedule = 0;
}

/**********************************************************************************************

     compute_tables -- compute the difference tables

***********************************************************************************************/

static void compute_tables(void)
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

int ymz280b_device::generate_adpcm(struct YMZ280BVoice *voice, INT16 *buffer, int samples)
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
			val = ymz280b_read_memory(position / 2) >> ((~position & 1) << 2);
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
			val = ymz280b_read_memory(position / 2) >> ((~position & 1) << 2);
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

int ymz280b_device::generate_pcm8(struct YMZ280BVoice *voice, INT16 *buffer, int samples)
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
			val = ymz280b_read_memory(position / 2);

			/* output to the buffer, scaling by the volume */
			*buffer++ = (INT8)val * 256;
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
			val = ymz280b_read_memory(position / 2);

			/* output to the buffer, scaling by the volume */
			*buffer++ = (INT8)val * 256;
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

int ymz280b_device::generate_pcm16(struct YMZ280BVoice *voice, INT16 *buffer, int samples)
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
			val = (INT16)((ymz280b_read_memory(position / 2 + 1) << 8) + ymz280b_read_memory(position / 2 + 0));

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
			val = (INT16)((ymz280b_read_memory(position / 2 + 1) << 8) + ymz280b_read_memory(position / 2 + 0));

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

void ymz280b_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	stream_sample_t *lacc = outputs[0];
	stream_sample_t *racc = outputs[1];
	int v;

	/* clear out the accumulator */
	memset(lacc, 0, samples * sizeof(lacc[0]));
	memset(racc, 0, samples * sizeof(racc[0]));

	/* loop over voices */
	for (v = 0; v < 8; v++)
	{
		struct YMZ280BVoice *voice = &m_voice[v];
		INT16 prev = voice->last_sample;
		INT16 curr = voice->curr_sample;
		INT16 *curr_data = m_scratch.get();
		INT32 *ldest = lacc;
		INT32 *rdest = racc;
		UINT32 new_samples, samples_left;
		UINT32 final_pos;
		int remaining = samples;
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
			int interp_sample = (((INT32)prev * (FRAC_ONE - voice->output_pos)) + ((INT32)curr * voice->output_pos)) >> FRAC_BITS;
			*ldest++ += interp_sample * lvol;
			*rdest++ += interp_sample * rvol;
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
				voice->timer->adjust(attotime::zero);
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
				int interp_sample = (((INT32)prev * (FRAC_ONE - voice->output_pos)) + ((INT32)curr * voice->output_pos)) >> FRAC_BITS;
				*ldest++ += interp_sample * lvol;
				*rdest++ += interp_sample * rvol;
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

	for (v = 0; v < samples; v++)
	{
		outputs[0][v] /= 256;
		outputs[1][v] /= 256;
	}
}



//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ymz280b_device::device_start()
{
	m_ext_read_handler.resolve();
	m_ext_write_handler.resolve();

	/* compute ADPCM tables */
	compute_tables();

	/* initialize the rest of the structure */
	m_master_clock = (double)clock() / 384.0;
	m_mem_base = region()->base();
	m_mem_size = region()->bytes();
	m_irq_handler.resolve();

	for (int i = 0; i < 8; i++)
	{
		m_voice[i].timer = timer_alloc(i);
	}

	/* create the stream */
	m_stream = machine().sound().stream_alloc(*this, 0, 2, INTERNAL_SAMPLE_RATE);

	/* allocate memory */
	assert(MAX_SAMPLE_CHUNK < 0x10000);
	m_scratch = std::make_unique<INT16[]>(MAX_SAMPLE_CHUNK);

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
	for (int j = 0; j < 8; j++)
	{
		save_item(NAME(m_voice[j].playing), j);
		save_item(NAME(m_voice[j].ended), j);
		save_item(NAME(m_voice[j].keyon), j);
		save_item(NAME(m_voice[j].looping), j);
		save_item(NAME(m_voice[j].mode), j);
		save_item(NAME(m_voice[j].fnum), j);
		save_item(NAME(m_voice[j].level), j);
		save_item(NAME(m_voice[j].pan), j);
		save_item(NAME(m_voice[j].start), j);
		save_item(NAME(m_voice[j].stop), j);
		save_item(NAME(m_voice[j].loop_start), j);
		save_item(NAME(m_voice[j].loop_end), j);
		save_item(NAME(m_voice[j].position), j);
		save_item(NAME(m_voice[j].signal), j);
		save_item(NAME(m_voice[j].step), j);
		save_item(NAME(m_voice[j].loop_signal), j);
		save_item(NAME(m_voice[j].loop_step), j);
		save_item(NAME(m_voice[j].loop_count), j);
		save_item(NAME(m_voice[j].output_left), j);
		save_item(NAME(m_voice[j].output_right), j);
		save_item(NAME(m_voice[j].output_pos), j);
		save_item(NAME(m_voice[j].last_sample), j);
		save_item(NAME(m_voice[j].curr_sample), j);
		save_item(NAME(m_voice[j].irq_schedule), j);
	}

#if MAKE_WAVS
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
	for (auto & elem : m_voice)
	{
		struct YMZ280BVoice *voice = &elem;

		voice->curr_sample = 0;
		voice->last_sample = 0;
		voice->output_pos = FRAC_ONE;
		voice->playing = 0;
	}
}


void ymz280b_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	if (id < 8)
		update_irq_state_timer_common( id );
	else
		assert_always(FALSE, "Unknown id in ymz280b_device::device_timer");
}


/**********************************************************************************************

     write_to_register -- handle a write to the current register

***********************************************************************************************/

void ymz280b_device::write_to_register(int data)
{
	struct YMZ280BVoice *voice;
	int i;

	/* lower registers follow a pattern */
	if (m_current_register < 0x80)
	{
		voice = &m_voice[(m_current_register >> 2) & 7];

		switch (m_current_register & 0xe3)
		{
			case 0x00:      /* pitch low 8 bits */
				voice->fnum = (voice->fnum & 0x100) | (data & 0xff);
				update_step(voice);
				break;

			case 0x01:      /* pitch upper 1 bit, loop, key on, mode */
				voice->fnum = (voice->fnum & 0xff) | ((data & 0x01) << 8);
				voice->looping = (data & 0x10) >> 4;
				if ((data & 0x60) == 0) data &= 0x7f; /* ignore mode setting and set to same state as KON=0 */
				else voice->mode = (data & 0x60) >> 5;
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
					m_ext_readlatch = ymz280b_read_memory(m_ext_mem_address);
				break;

			case 0x87:      /* RAM write */
				if (m_ext_mem_enable)
				{
					if (!m_ext_write_handler.isnull())
						m_ext_write_handler(m_ext_mem_address, data);
					else
						logerror("YMZ280B attempted RAM write to %X\n", m_ext_mem_address);
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
					for (i = 0; i < 8; i++)
					{
						m_voice[i].playing = 0;

						/* if update_irq_state_timer is set, cancel it. */
						m_voice[i].irq_schedule = 0;
					}
				}
				else if (!m_keyon_enable && (data & 0x80))
				{
					for (i = 0; i < 8; i++)
					{
						if (m_voice[i].keyon && m_voice[i].looping)
							m_voice[i].playing = 1;
					}
				}
				m_keyon_enable = (data & 0x80) >> 7;
				break;

			default:
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
	UINT8 result;

	/* force an update */
	m_stream->update();

	result = m_status_register;

	/* clear the IRQ state */
	m_status_register = 0;
	update_irq_state();

	return result;
}



/**********************************************************************************************

     ymz280b_r/ymz280b_w -- handle external accesses

***********************************************************************************************/

READ8_MEMBER( ymz280b_device::read )
{
	if ((offset & 1) == 0)
	{
		if (!m_ext_mem_enable)
			return 0xff;

		/* read from external memory */
		UINT8 ret = m_ext_readlatch;
		m_ext_readlatch = ymz280b_read_memory(m_ext_mem_address);
		m_ext_mem_address = (m_ext_mem_address + 1) & 0xffffff;
		return ret;
	}
	else
		return compute_status();
}


WRITE8_MEMBER( ymz280b_device::write )
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


const device_type YMZ280B = &device_creator<ymz280b_device>;

ymz280b_device::ymz280b_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, YMZ280B, "YMZ280B", tag, owner, clock, "ymz280b", __FILE__),
		device_sound_interface(mconfig, *this),
		m_current_register(0),
		m_status_register(0),
		m_irq_state(0),
		m_irq_mask(0),
		m_irq_enable(0),
		m_keyon_enable(0),
		m_ext_mem_enable(0),
		m_ext_readlatch(0),
		m_ext_mem_address_hi(0),
		m_ext_mem_address_mid(0),
		m_ext_mem_address(0),
		m_irq_handler(*this),
		m_ext_read_handler(*this),
		m_ext_write_handler(*this)
{
	memset(m_voice, 0, sizeof(m_voice));
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void ymz280b_device::device_config_complete()
{
}
