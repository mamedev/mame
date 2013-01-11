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

*/


#include "emu.h"
#include "ymz280b.h"


#define MAX_SAMPLE_CHUNK    10000
#define MAKE_WAVS           0

#define FRAC_BITS           14
#define FRAC_ONE            (1 << FRAC_BITS)
#define FRAC_MASK           (FRAC_ONE - 1)

#define INTERNAL_BUFFER_SIZE    (1 << 15)
#define INTERNAL_SAMPLE_RATE    (chip->master_clock * 2.0)

#if MAKE_WAVS
#include "wavwrite.h"
#endif


/* struct describing a single playing ADPCM voice */
struct YMZ280BVoice
{
	UINT8 playing;          /* 1 if we are actively playing */

	UINT8 keyon;            /* 1 if the key is on */
	UINT8 looping;          /* 1 if looping is enabled */
	UINT8 mode;             /* current playback mode */
	UINT16 fnum;            /* frequency */
	UINT8 level;            /* output level */
	UINT8 pan;              /* panning */

	UINT32 start;           /* start address, in nibbles */
	UINT32 stop;            /* stop address, in nibbles */
	UINT32 loop_start;      /* loop start address, in nibbles */
	UINT32 loop_end;        /* loop end address, in nibbles */
	UINT32 position;        /* current position, in nibbles */

	INT32 signal;           /* current ADPCM signal */
	INT32 step;             /* current ADPCM step */

	INT32 loop_signal;      /* signal at loop start */
	INT32 loop_step;        /* step at loop start */
	UINT32 loop_count;      /* number of loops so far */

	INT32 output_left;      /* output volume (left) */
	INT32 output_right;     /* output volume (right) */
	INT32 output_step;      /* step value for frequency conversion */
	INT32 output_pos;       /* current fractional position */
	INT16 last_sample;      /* last sample output */
	INT16 curr_sample;      /* current sample target */
	UINT8 irq_schedule;     /* 1 if the IRQ state is updated by timer */
};

struct ymz280b_state
{
	sound_stream * stream;          /* which stream are we using */
	UINT8 *region_base;             /* pointer to the base of the region */
	UINT32 region_size;
	UINT8 current_register;         /* currently accessible register */
	UINT8 status_register;          /* current status register */
	UINT8 irq_state;                /* current IRQ state */
	UINT8 irq_mask;                 /* current IRQ mask */
	UINT8 irq_enable;               /* current IRQ enable */
	UINT8 keyon_enable;             /* key on enable */
	UINT8 ext_mem_enable;           /* external memory enable */
	double master_clock;            /* master clock frequency */
	void (*irq_callback)(device_t *, int);  /* IRQ callback */
	struct YMZ280BVoice voice[8];   /* the 8 voices */
	UINT32 rom_addr_hi;
	UINT32 rom_addr_mid;
	UINT32 rom_readback_addr;       /* where the CPU can read the ROM */
	devcb_resolved_read8 ext_ram_read;      /* external RAM read handler */
	devcb_resolved_write8 ext_ram_write;    /* external RAM write handler */

#if MAKE_WAVS
	void * wavresample;             /* resampled waveform */
#endif

	INT16 *scratch;
	device_t *device;
};

static void write_to_register(ymz280b_state *, int);


/* step size index shift table */
static const int index_scale[8] = { 0x0e6, 0x0e6, 0x0e6, 0x0e6, 0x133, 0x199, 0x200, 0x266 };

/* lookup table for the precomputed difference */
static int diff_lookup[16];

/* timer callback */
static TIMER_CALLBACK( update_irq_state_timer_0 );
static TIMER_CALLBACK( update_irq_state_timer_1 );
static TIMER_CALLBACK( update_irq_state_timer_2 );
static TIMER_CALLBACK( update_irq_state_timer_3 );
static TIMER_CALLBACK( update_irq_state_timer_4 );
static TIMER_CALLBACK( update_irq_state_timer_5 );
static TIMER_CALLBACK( update_irq_state_timer_6 );
static TIMER_CALLBACK( update_irq_state_timer_7 );

static const struct { timer_expired_func func; const char *name; } update_irq_state_cb[] =
{
	{ FUNC(update_irq_state_timer_0) },
	{ FUNC(update_irq_state_timer_1) },
	{ FUNC(update_irq_state_timer_2) },
	{ FUNC(update_irq_state_timer_3) },
	{ FUNC(update_irq_state_timer_4) },
	{ FUNC(update_irq_state_timer_5) },
	{ FUNC(update_irq_state_timer_6) },
	{ FUNC(update_irq_state_timer_7) }
};


INLINE ymz280b_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == YMZ280B);
	return (ymz280b_state *)downcast<ymz280b_device *>(device)->token();
}


INLINE UINT8 ymz280b_read_memory(ymz280b_state *chip, UINT32 offset)
{
	if (chip->ext_ram_read.isnull())
	{
		if (offset < chip->region_size)
			return chip->region_base[offset];

		/* 16MB chip limit (shouldn't happen) */
		else if (offset > 0xffffff)
			return chip->region_base[offset & 0xffffff];

		else
			return 0;
	}
	else
		return chip->ext_ram_read(offset);
}


INLINE void update_irq_state(ymz280b_state *chip)
{
	int irq_bits = chip->status_register & chip->irq_mask;

	/* always off if the enable is off */
	if (!chip->irq_enable)
		irq_bits = 0;

	/* update the state if changed */
	if (irq_bits && !chip->irq_state)
	{
		chip->irq_state = 1;
		if (chip->irq_callback)
			(*chip->irq_callback)(chip->device, 1);
		else logerror("YMZ280B: IRQ generated, but no callback specified!");
	}
	else if (!irq_bits && chip->irq_state)
	{
		chip->irq_state = 0;
		if (chip->irq_callback)
			(*chip->irq_callback)(chip->device, 0);
		else logerror("YMZ280B: IRQ generated, but no callback specified!");
	}
}


INLINE void update_step(ymz280b_state *chip, struct YMZ280BVoice *voice)
{
	double frequency;

	/* compute the frequency */
	if (voice->mode == 1)
		frequency = chip->master_clock * (double)((voice->fnum & 0x0ff) + 1) * (1.0 / 256.0);
	else
		frequency = chip->master_clock * (double)((voice->fnum & 0x1ff) + 1) * (1.0 / 256.0);
	voice->output_step = (UINT32)(frequency * (double)FRAC_ONE / INTERNAL_SAMPLE_RATE);
}


INLINE void update_volumes(struct YMZ280BVoice *voice)
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


static void YMZ280B_state_save_update_step(ymz280b_state *chip)
{
	for (int j = 0; j < 8; j++)
	{
		struct YMZ280BVoice *voice = &chip->voice[j];
		update_step(chip, voice);
		if(voice->irq_schedule)
			chip->device->machine().scheduler().timer_set(attotime::zero, update_irq_state_cb[j].func, update_irq_state_cb[j].name, 0, chip);
	}
}


static void update_irq_state_timer_common(void *param, int voicenum)
{
	ymz280b_state *chip = (ymz280b_state *)param;
	struct YMZ280BVoice *voice = &chip->voice[voicenum];

	if(!voice->irq_schedule) return;

	voice->playing = 0;
	chip->status_register |= 1 << voicenum;
	update_irq_state(chip);
	voice->irq_schedule = 0;
}

static TIMER_CALLBACK( update_irq_state_timer_0 ) { update_irq_state_timer_common(ptr, 0); }
static TIMER_CALLBACK( update_irq_state_timer_1 ) { update_irq_state_timer_common(ptr, 1); }
static TIMER_CALLBACK( update_irq_state_timer_2 ) { update_irq_state_timer_common(ptr, 2); }
static TIMER_CALLBACK( update_irq_state_timer_3 ) { update_irq_state_timer_common(ptr, 3); }
static TIMER_CALLBACK( update_irq_state_timer_4 ) { update_irq_state_timer_common(ptr, 4); }
static TIMER_CALLBACK( update_irq_state_timer_5 ) { update_irq_state_timer_common(ptr, 5); }
static TIMER_CALLBACK( update_irq_state_timer_6 ) { update_irq_state_timer_common(ptr, 6); }
static TIMER_CALLBACK( update_irq_state_timer_7 ) { update_irq_state_timer_common(ptr, 7); }


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

static int generate_adpcm(ymz280b_state *chip, struct YMZ280BVoice *voice, INT16 *buffer, int samples)
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
			val = ymz280b_read_memory(chip, position / 2) >> ((~position & 1) << 2);
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
				if (!samples)
					samples |= 0x10000;

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
			val = ymz280b_read_memory(chip, position / 2) >> ((~position & 1) << 2);
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
				if (!samples)
					samples |= 0x10000;

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

static int generate_pcm8(ymz280b_state *chip, struct YMZ280BVoice *voice, INT16 *buffer, int samples)
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
			val = ymz280b_read_memory(chip, position / 2);

			/* output to the buffer, scaling by the volume */
			*buffer++ = (INT8)val * 256;
			samples--;

			/* next! */
			position += 2;
			if (position >= voice->stop)
			{
				if (!samples)
					samples |= 0x10000;

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
			val = ymz280b_read_memory(chip, position / 2);

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
				if (!samples)
					samples |= 0x10000;

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

static int generate_pcm16(ymz280b_state *chip, struct YMZ280BVoice *voice, INT16 *buffer, int samples)
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
			val = (INT16)((ymz280b_read_memory(chip, position / 2 + 0) << 8) + ymz280b_read_memory(chip, position / 2 + 1));

			/* output to the buffer, scaling by the volume */
			*buffer++ = val;
			samples--;

			/* next! */
			position += 4;
			if (position >= voice->stop)
			{
				if (!samples)
					samples |= 0x10000;

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
			val = (INT16)((ymz280b_read_memory(chip, position / 2 + 0) << 8) + ymz280b_read_memory(chip, position / 2 + 1));

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
				if (!samples)
					samples |= 0x10000;

				break;
			}
		}
	}

	/* update the parameters */
	voice->position = position;

	return samples;
}



/**********************************************************************************************

     ymz280b_update -- update the sound chip so that it is in sync with CPU execution

***********************************************************************************************/

static STREAM_UPDATE( ymz280b_update )
{
	ymz280b_state *chip = (ymz280b_state *)param;
	stream_sample_t *lacc = outputs[0];
	stream_sample_t *racc = outputs[1];
	int v;

	/* clear out the accumulator */
	memset(lacc, 0, samples * sizeof(lacc[0]));
	memset(racc, 0, samples * sizeof(racc[0]));

	/* loop over voices */
	for (v = 0; v < 8; v++)
	{
		struct YMZ280BVoice *voice = &chip->voice[v];
		INT16 prev = voice->last_sample;
		INT16 curr = voice->curr_sample;
		INT16 *curr_data = chip->scratch;
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
			case 0x81:  samples_left = generate_adpcm(chip, voice, chip->scratch, new_samples); break;
			case 0x82:  samples_left = generate_pcm8(chip, voice, chip->scratch, new_samples); break;
			case 0x83:  samples_left = generate_pcm16(chip, voice, chip->scratch, new_samples); break;
			default:    samples_left = 0; memset(chip->scratch, 0, new_samples * sizeof(chip->scratch[0])); break;
		}

		/* if there are leftovers, ramp back to 0 */
		if (samples_left)
		{
			/* note: samples_left bit 16 is set if the voice was finished at the same time the function ended */
			int base = new_samples - (samples_left & 0xffff);
			int i, t = (base == 0) ? curr : chip->scratch[base - 1];
			for (i = 0; i < (samples_left & 0xffff); i++)
			{
				if (t < 0) t = -((-t * 15) >> 4);
				else if (t > 0) t = (t * 15) >> 4;
				chip->scratch[base + i] = t;
			}

			/* if we hit the end and IRQs are enabled, signal it */
			if (base != 0)
			{
				voice->playing = 0;

				/* set update_irq_state_timer. IRQ is signaled on next CPU execution. */
				chip->device->machine().scheduler().timer_set(attotime::zero, update_irq_state_cb[v].func, update_irq_state_cb[v].name, 0, chip);
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



/**********************************************************************************************

     DEVICE_START/RESET( ymz280b ) -- start/reset emulation of the YMZ280B

***********************************************************************************************/

static DEVICE_START( ymz280b )
{
	static const ymz280b_interface defintrf = { 0 };
	const ymz280b_interface *intf = (device->static_config() != NULL) ? (const ymz280b_interface *)device->static_config() : &defintrf;
	ymz280b_state *chip = get_safe_token(device);

	chip->device = device;
	chip->ext_ram_read.resolve(intf->ext_read, *device);
	chip->ext_ram_write.resolve(intf->ext_write, *device);

	/* compute ADPCM tables */
	compute_tables();

	/* initialize the rest of the structure */
	chip->master_clock = (double)device->clock() / 384.0;
	chip->region_base = *device->region();
	chip->region_size = device->region()->bytes();
	chip->irq_callback = intf->irq_callback;

	/* create the stream */
	chip->stream = device->machine().sound().stream_alloc(*device, 0, 2, INTERNAL_SAMPLE_RATE, chip, ymz280b_update);

	/* allocate memory */
	assert(MAX_SAMPLE_CHUNK < 0x10000);
	chip->scratch = auto_alloc_array(device->machine(), INT16, MAX_SAMPLE_CHUNK);

	/* state save */
	{
		int j;
		device->save_item(NAME(chip->current_register));
		device->save_item(NAME(chip->status_register));
		device->save_item(NAME(chip->irq_state));
		device->save_item(NAME(chip->irq_mask));
		device->save_item(NAME(chip->irq_enable));
		device->save_item(NAME(chip->keyon_enable));
		device->save_item(NAME(chip->ext_mem_enable));
		device->save_item(NAME(chip->rom_readback_addr));
		device->save_item(NAME(chip->rom_addr_hi));
		device->save_item(NAME(chip->rom_addr_mid));
		for (j = 0; j < 8; j++)
		{
			device->save_item(NAME(chip->voice[j].playing), j);
			device->save_item(NAME(chip->voice[j].keyon), j);
			device->save_item(NAME(chip->voice[j].looping), j);
			device->save_item(NAME(chip->voice[j].mode), j);
			device->save_item(NAME(chip->voice[j].fnum), j);
			device->save_item(NAME(chip->voice[j].level), j);
			device->save_item(NAME(chip->voice[j].pan), j);
			device->save_item(NAME(chip->voice[j].start), j);
			device->save_item(NAME(chip->voice[j].stop), j);
			device->save_item(NAME(chip->voice[j].loop_start), j);
			device->save_item(NAME(chip->voice[j].loop_end), j);
			device->save_item(NAME(chip->voice[j].position), j);
			device->save_item(NAME(chip->voice[j].signal), j);
			device->save_item(NAME(chip->voice[j].step), j);
			device->save_item(NAME(chip->voice[j].loop_signal), j);
			device->save_item(NAME(chip->voice[j].loop_step), j);
			device->save_item(NAME(chip->voice[j].loop_count), j);
			device->save_item(NAME(chip->voice[j].output_left), j);
			device->save_item(NAME(chip->voice[j].output_right), j);
			device->save_item(NAME(chip->voice[j].output_pos), j);
			device->save_item(NAME(chip->voice[j].last_sample), j);
			device->save_item(NAME(chip->voice[j].curr_sample), j);
			device->save_item(NAME(chip->voice[j].irq_schedule), j);
		}
	}

	device->machine().save().register_postload(save_prepost_delegate(FUNC(YMZ280B_state_save_update_step), chip));

#if MAKE_WAVS
	chip->wavresample = wav_open("resamp.wav", INTERNAL_SAMPLE_RATE, 2);
#endif
}

static DEVICE_RESET( ymz280b )
{
	int i;
	ymz280b_state *chip = get_safe_token(device);

	/* initial clear registers */
	for (i = 0xff; i >= 0; i--)
	{
		chip->current_register = i;
		write_to_register(chip, 0);
	}

	chip->current_register = 0;
	chip->status_register = 0;
	chip->rom_readback_addr = 0;

	/* clear other voice parameters */
	for (i = 0; i < 8; i++)
	{
		struct YMZ280BVoice *voice = &chip->voice[i];

		voice->curr_sample = 0;
		voice->last_sample = 0;
		voice->output_pos = FRAC_ONE;
		voice->playing = 0;
	}
}



/**********************************************************************************************

     write_to_register -- handle a write to the current register

***********************************************************************************************/

static void write_to_register(ymz280b_state *chip, int data)
{
	struct YMZ280BVoice *voice;
	int i;

	/* lower registers follow a pattern */
	if (chip->current_register < 0x80)
	{
		voice = &chip->voice[(chip->current_register >> 2) & 7];

		switch (chip->current_register & 0xe3)
		{
			case 0x00:      /* pitch low 8 bits */
				voice->fnum = (voice->fnum & 0x100) | (data & 0xff);
				update_step(chip, voice);
				break;

			case 0x01:      /* pitch upper 1 bit, loop, key on, mode */
				voice->fnum = (voice->fnum & 0xff) | ((data & 0x01) << 8);
				voice->looping = (data & 0x10) >> 4;
				if ((data & 0x60) == 0) data &= 0x7f; /* ignore mode setting and set to same state as KON=0 */
				else voice->mode = (data & 0x60) >> 5;
				if (!voice->keyon && (data & 0x80) && chip->keyon_enable)
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
				update_step(chip, voice);
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
				logerror("YMZ280B: unknown register write %02X = %02X\n", chip->current_register, data);
				break;
		}
	}

	/* upper registers are special */
	else
	{
		switch (chip->current_register)
		{
			/* DSP related (not implemented yet) */
			case 0x80: // d0-2: DSP Rch, d3: enable Rch (0: yes, 1: no), d4-6: DSP Lch, d7: enable Lch (0: yes, 1: no)
			case 0x81: // d0: enable control of $82 (0: yes, 1: no)
			case 0x82: // DSP data
				logerror("YMZ280B: DSP register write %02X = %02X\n", chip->current_register, data);
				break;

			case 0x84:      /* ROM readback / RAM write (high) */
				chip->rom_addr_hi = data << 16;
				break;

			case 0x85:      /* ROM readback / RAM write (middle) */
				chip->rom_addr_mid = data << 8;
				break;

			case 0x86:      /* ROM readback / RAM write (low) -> update latch */
				chip->rom_readback_addr = chip->rom_addr_hi | chip->rom_addr_mid | data;
				break;

			case 0x87:      /* RAM write */
				if (chip->ext_mem_enable)
				{
					if (!chip->ext_ram_write.isnull())
						chip->ext_ram_write(chip->rom_readback_addr, data);
					else
						logerror("YMZ280B attempted RAM write to %X\n", chip->rom_readback_addr);
					chip->rom_readback_addr = (chip->rom_readback_addr + 1) & 0xffffff;
				}
				break;

			case 0xfe:      /* IRQ mask */
				chip->irq_mask = data;
				update_irq_state(chip);
				break;

			case 0xff:      /* IRQ enable, test, etc */
				chip->ext_mem_enable = (data & 0x40) >> 6;
				chip->irq_enable = (data & 0x10) >> 4;
				update_irq_state(chip);

				if (chip->keyon_enable && !(data & 0x80))
				{
					for (i = 0; i < 8; i++)
					{
						chip->voice[i].playing = 0;

						/* if update_irq_state_timer is set, cancel it. */
						chip->voice[i].irq_schedule = 0;
					}
				}
				else if (!chip->keyon_enable && (data & 0x80))
				{
					for (i = 0; i < 8; i++)
					{
						if (chip->voice[i].keyon && chip->voice[i].looping)
							chip->voice[i].playing = 1;
					}
				}
				chip->keyon_enable = (data & 0x80) >> 7;
				break;

			default:
				logerror("YMZ280B: unknown register write %02X = %02X\n", chip->current_register, data);
				break;
		}
	}
}



/**********************************************************************************************

     compute_status -- determine the status bits

***********************************************************************************************/

static int compute_status(ymz280b_state *chip)
{
	UINT8 result;

	/* force an update */
	chip->stream->update();

	result = chip->status_register;

	/* clear the IRQ state */
	chip->status_register = 0;
	update_irq_state(chip);

	return result;
}



/**********************************************************************************************

     ymz280b_r/ymz280b_w -- handle external accesses

***********************************************************************************************/

READ8_DEVICE_HANDLER( ymz280b_r )
{
	ymz280b_state *chip = get_safe_token(device);

	if ((offset & 1) == 0)
	{
		if (!chip->ext_mem_enable)
			return 0xff;

		/* read from external memory */
		UINT8 result = ymz280b_read_memory(chip, chip->rom_readback_addr);
		chip->rom_readback_addr = (chip->rom_readback_addr + 1) & 0xffffff;
		return result;
	}
	else
		return compute_status(chip);
}


WRITE8_DEVICE_HANDLER( ymz280b_w )
{
	ymz280b_state *chip = get_safe_token(device);

	if ((offset & 1) == 0)
		chip->current_register = data;
	else
	{
		/* force an update */
		chip->stream->update();

		write_to_register(chip, data);
	}
}


const device_type YMZ280B = &device_creator<ymz280b_device>;

ymz280b_device::ymz280b_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, YMZ280B, "YMZ280B", tag, owner, clock),
		device_sound_interface(mconfig, *this)
{
	m_token = global_alloc_clear(ymz280b_state);
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void ymz280b_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ymz280b_device::device_start()
{
	DEVICE_START_NAME( ymz280b )(this);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void ymz280b_device::device_reset()
{
	DEVICE_RESET_NAME( ymz280b )(this);
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void ymz280b_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// should never get here
	fatalerror("sound_stream_update called; not applicable to legacy sound devices\n");
}
