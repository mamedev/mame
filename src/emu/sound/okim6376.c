/**********************************************************************************************
 *
 *   OKI MSM6376 ADPCM
 *   by Mirko Buffoni, J. Wallace
 *
 *   TODO:
 *     add BEEP tone generator
 *     confirm echo
 *     sample divisor in ROM table not implemented (no documentation)
 *     modernise
 **********************************************************************************************/


#include "emu.h"
#include "okim6376.h"

#define MAX_SAMPLE_CHUNK    10000
//#define MAX_WORDS           111

#define OKIVERBOSE 0
#define MSM6376LOG(x) do { if (OKIVERBOSE) logerror x; } while (0)

/* struct describing a single playing ADPCM voice */
struct ADPCMVoice
{
	UINT8 playing;          /* 1 if we are actively playing */

	UINT32 base_offset;     /* pointer to the base memory location */
	UINT32 sample;          /* current sample number */
	UINT32 count;           /* total samples to play */

	UINT32 volume;          /* output volume */
	INT32 signal;
	INT32 step;
};

struct okim6376_state
{
	#define OKIM6376_VOICES     2
	struct ADPCMVoice voice[OKIM6376_VOICES];
	INT32 command[OKIM6376_VOICES];
	INT32 latch;            /* Command data is held before transferring to either channel */
	UINT8 stage[OKIM6376_VOICES];/* If a sample is playing, flag that we have a command staged */
	UINT8 *region_base;     /* pointer to the base of the region */
	sound_stream *stream;   /* which stream are we playing on? */
	UINT32 master_clock;    /* master clock frequency */
	UINT8 divisor;          /* can be 8,10,16, and is read out of ROM data */
	UINT8 channel;
	UINT8 nar;              /* Next Address Ready */
	UINT8 nartimer;
	UINT8 busy;
	UINT8 ch2;              /* 2CH pin - enables Channel 2 operation */
	UINT8 st;               /* STart */
	UINT8 st_pulses;        /* Keep track of attenuation */
	UINT8 ch2_update;       /* Pulse shape */
	UINT8 st_update;
};

/* step size index shift table */
static const int index_shift[8] = { -1, -1, -1, -1, 2, 4, 6, 8 };

/* lookup table for the precomputed difference */
static int diff_lookup[49*16];

/* volume lookup table. Upon configuration, the number of ST pulses determine how much
   attenuation to apply to the sound signal. However, this only applies to the second
   channel*/
static const int volume_table[3] =
{
	0x20,   //   0 dB
	0x10,   //  -6.0 dB
	0x08,   // -12.0 dB
};

/* divisor lookup table. When an individual word is selected, it can be assigned one of three different 'rates'.
   These are implemented as clock divisors, and are looked up in the ROM header. More often than not, this value is 0,
   relating to a division by 8, or nominally 8KHz sampling (based on the datasheet example of a 64KHz clock).*/
static const int divisor_table[3] =
{
	8,
	10,
	16,
};

/* tables computed? */
static int tables_computed = 0;


INLINE okim6376_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == OKIM6376);
	return (okim6376_state *)downcast<okim6376_device *>(device)->token();
}



/**********************************************************************************************

     compute_tables -- compute the difference tables

***********************************************************************************************/

static void compute_tables(void)
{
	/* nibble to bit map */
	static const int nbl2bit[16][4] =
	{
		{ 1, 0, 0, 0}, { 1, 0, 0, 1}, { 1, 0, 1, 0}, { 1, 0, 1, 1},
		{ 1, 1, 0, 0}, { 1, 1, 0, 1}, { 1, 1, 1, 0}, { 1, 1, 1, 1},
		{-1, 0, 0, 0}, {-1, 0, 0, 1}, {-1, 0, 1, 0}, {-1, 0, 1, 1},
		{-1, 1, 0, 0}, {-1, 1, 0, 1}, {-1, 1, 1, 0}, {-1, 1, 1, 1}
	};

	int step, nib;

	/* loop over all possible steps */
	for (step = 0; step <= 48; step++)
	{
		/* compute the step value */
		int stepval = floor(16.0 * pow(11.0 / 10.0, (double)step));

		/* loop over all nibbles and compute the difference */
		for (nib = 0; nib < 16; nib++)
		{
			diff_lookup[step*16 + nib] = nbl2bit[nib][0] *
				(stepval   * nbl2bit[nib][1] +
					stepval/2 * nbl2bit[nib][2] +
					stepval/4 * nbl2bit[nib][3] +
					stepval/8);
		}
	}

	tables_computed = 1;
}



/**********************************************************************************************

     reset_adpcm -- reset the ADPCM stream

***********************************************************************************************/

static void reset_adpcm(struct ADPCMVoice *voice)
{
	/* make sure we have our tables */
	if (!tables_computed)
		compute_tables();

	/* reset the signal/step */
	voice->signal = -2;
	voice->step = 0;
}



/**********************************************************************************************

     clock_adpcm -- clock the next ADPCM byte

***********************************************************************************************/

static INT16 clock_adpcm(struct ADPCMVoice *voice, UINT8 nibble)
{
	voice->signal += diff_lookup[voice->step * 16 + (nibble & 15)];

	/* clamp to the maximum 12bit */
	if (voice->signal > 2047)
		voice->signal = 2047;
	else if (voice->signal < -2048)
		voice->signal = -2048;

	/* adjust the step size and clamp */
	voice->step += index_shift[nibble & 7];
	if (voice->step > 48)
		voice->step = 48;
	else if (voice->step < 0)
		voice->step = 0;

	/* return the signal */
	return voice->signal;
}


static void oki_process(okim6376_state *info, int channel, int command)
{
	/* if a command is pending, process the second half */
	if ((command != -1) && (command != 0)) //process silence separately
	{
		int start;
		unsigned char *base/*, *base_end*/;
		/* update the stream */
		info->stream->update();

		/* determine which voice(s) (voice is set by the state of 2CH) */
		{
			struct ADPCMVoice *voice = &info->voice[channel];

			/* determine the start position, max address space is 16Mbit */
			base = &info->region_base[info->command[channel] * 4];
			//base_end = &info->region_base[(MAX_WORDS+1) * 4];
			start = ((base[0] << 16) + (base[1] << 8) + base[2]) & 0x1fffff;

				if (start == 0)
				{
					voice->playing = 0;
				}
				else
				{
					/* set up the voice to play this sample */
					if (!voice->playing)
					{
						voice->playing = 1;
						voice->base_offset = start;
						voice->sample = 0;
						voice->count = 0;

						/* also reset the ADPCM parameters */
						reset_adpcm(voice);
						if (channel == 0)
						{
							/* We set channel 2's audio separately */
							voice->volume = volume_table[0];
						}
					}
					else
					{
						if (((info->nar)&&(channel == 0))||(channel == 1))//Store the request, for later processing (channel 2 ignores NAR)
						{
							info->stage[channel] = 1;
						}
					}
				}
			}
		}
		/* otherwise, see if this is a silence command */
		else
		{
			/* update the stream, then turn it off */
			info->stream->update();

			if (command ==0)
			{
				int i;
				for (i = 0; i < OKIM6376_VOICES; i++)
				{
					struct ADPCMVoice *voice = &info->voice[i];
					voice->playing = 0;
			}
		}
	}
}


/**********************************************************************************************

     generate_adpcm -- general ADPCM decoding routine

***********************************************************************************************/

static void generate_adpcm(okim6376_state *chip, struct ADPCMVoice *voice, INT16 *buffer, int samples,int channel)
{
	/* if this voice is active */
	if (voice->playing)
	{
		UINT8 *base = chip->region_base + voice->base_offset;
		int sample = voice->sample;
		int count = voice->count;

		/* loop while we still have samples to generate */
		while (samples)
		{
			int nibble;

			if (count == 0)
			{
				/* get the number of samples to play */
				count = (base[sample / 2] & 0x7f) << 1;

				/* end of voice marker */
				if (count == 0)
				{
					voice->playing = 0;
					break;
				}
				else
				{
					/* step past the count byte */
					sample += 2;
				}
			}

			/* compute the new amplitude and update the current step */
			nibble = base[sample / 2] >> (((sample & 1) << 2) ^ 4);

			/* output to the buffer, scaling by the volume */
			/* signal in range -4096..4095, volume in range 2..16 => signal * volume / 2 in range -32768..32767 */
			*buffer++ = clock_adpcm(voice, nibble) * voice->volume / 2;

			++sample;
			--count;
			--samples;
		}

		/* update the parameters */
		voice->sample = sample;
		voice->count = count;
	}

	/* fill the rest with silence */
	while (samples--)
		*buffer++ = 0;

	if ((!voice->playing)&&(chip->stage[channel]))//end of samples, load anything staged in
	{
		chip->stage[channel] = 0;
		oki_process(chip,channel,chip->command[channel]);
	}
}



/**********************************************************************************************

     okim6376_update -- update the sound chip so that it is in sync with CPU execution

***********************************************************************************************/

static STREAM_UPDATE( okim6376_update )
{
	okim6376_state *chip = (okim6376_state *)param;
	int i;

	memset(outputs[0], 0, samples * sizeof(*outputs[0]));

	for (i = 0; i < OKIM6376_VOICES; i++)
	{
		struct ADPCMVoice *voice = &chip->voice[i];
		stream_sample_t *buffer = outputs[0];
		INT16 sample_data[MAX_SAMPLE_CHUNK];
		int remaining = samples;
		if (i == 0) //channel 1 is the only channel to affect NAR
		{
			if (chip->nartimer > 0)
			{
				chip->nartimer--;
				if (!chip->nartimer)
				{
					chip->nar =1;
				}
			}
		}

		/* loop while we have samples remaining */
		while (remaining)
		{
			int samples = (remaining > MAX_SAMPLE_CHUNK) ? MAX_SAMPLE_CHUNK : remaining;
			int samp;

			generate_adpcm(chip, voice, sample_data, samples,i);
			for (samp = 0; samp < samples; samp++)
				*buffer++ += sample_data[samp];

			remaining -= samples;
		}
	}
}



/**********************************************************************************************

     state save support for MAME

***********************************************************************************************/

static void okim6376_postload(device_t *device)
{
	okim6376_state *info = get_safe_token(device);

	okim6376_set_frequency(device, info->master_clock);
}

static void adpcm_state_save_register(struct ADPCMVoice *voice, device_t *device, int index)
{
	device->save_item(NAME(voice->playing), index);
	device->save_item(NAME(voice->sample), index);
	device->save_item(NAME(voice->count), index);
	device->save_item(NAME(voice->signal), index);
	device->save_item(NAME(voice->step), index);
	device->save_item(NAME(voice->volume), index);
	device->save_item(NAME(voice->base_offset), index);
}

static void okim6376_state_save_register(okim6376_state *info, device_t *device)
{
	int j;
	for (j = 0; j < OKIM6376_VOICES; j++)
	{
		adpcm_state_save_register(&info->voice[j], device, j);
	}
		device->machine().save().register_postload(save_prepost_delegate(FUNC(okim6376_postload), device));
		device->save_item(NAME(info->command[0]));
		device->save_item(NAME(info->command[1]));
		device->save_item(NAME(info->stage[0]));
		device->save_item(NAME(info->stage[1]));
		device->save_item(NAME(info->latch));
		device->save_item(NAME(info->divisor));
		device->save_item(NAME(info->nar));
		device->save_item(NAME(info->nartimer));
		device->save_item(NAME(info->busy));
		device->save_item(NAME(info->st));
		device->save_item(NAME(info->st_pulses));
		device->save_item(NAME(info->st_update));
		device->save_item(NAME(info->ch2));
		device->save_item(NAME(info->ch2_update));
		device->save_item(NAME(info->master_clock));
}

/**********************************************************************************************

     DEVICE_START( okim6376 ) -- start emulation of an OKIM6376-compatible chip

***********************************************************************************************/

static DEVICE_START( okim6376 )
{
	okim6376_state *info = get_safe_token(device);
	int voice;
	compute_tables();

	info->command[0] = -1;
	info->command[1] = -1;
	info->stage[0] = 0;
	info->stage[1] = 0;
	info->latch = 0;
	info->divisor = divisor_table[0];
	info->region_base = *device->region();
	info->master_clock = device->clock();
	info->nar = 1;
	info->nartimer = 0;
	info->busy = 1;
	info->st = 1;
	info->ch2 = 1;
	info->st_update = 0;
	info->ch2_update = 0;
	info->st_pulses = 0;
	/* generate the name and create the stream */
	info->stream = device->machine().sound().stream_alloc(*device, 0, 1, device->clock()/ info->divisor, info, okim6376_update);

	/* initialize the voices */
	for (voice = 0; voice < OKIM6376_VOICES; voice++)
	{
		/* initialize the rest of the structure */
		info->voice[voice].volume = 0;
		reset_adpcm(&info->voice[voice]);
	}

	okim6376_state_save_register(info, device);
}

void okim6376_set_frequency(device_t *device, int frequency)
{
	okim6376_state *info = get_safe_token(device);

	info->master_clock = frequency;
	info->stream->set_sample_rate(info->master_clock / info->divisor);
}

/**********************************************************************************************

     DEVICE_RESET( okim6376 ) -- stop emulation of an OKIM6376-compatible chip

***********************************************************************************************/

static DEVICE_RESET( okim6376 )
{
	okim6376_state *info = get_safe_token(device);
	int i;

	info->stream->update();
	for (i = 0; i < OKIM6376_VOICES; i++)
		info->voice[i].playing = 0;
}




/**********************************************************************************************

     okim6376_status_r -- read the status port of an OKIM6376-compatible chip

***********************************************************************************************/

READ_LINE_DEVICE_HANDLER( okim6376_busy_r )
{
	okim6376_state *info = get_safe_token(device);

	struct ADPCMVoice *voice0 = &info->voice[0];
	struct ADPCMVoice *voice1 = &info->voice[1];

	/* set the bit low if it's playing */
	if ((voice0->playing)||(voice1->playing))
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

READ_LINE_DEVICE_HANDLER( okim6376_nar_r )
{
	okim6376_state *info = get_safe_token(device);
	MSM6376LOG(("OKIM6376:'%s' NAR %x\n",device->tag(),info->nar));
	return info->nar;
}

WRITE_LINE_DEVICE_HANDLER( okim6376_ch2_w )
{
	okim6376_state *info = get_safe_token(device);
	info->ch2_update = 0;//Clear flag
	MSM6376LOG(("OKIM6376:'%s' CH2 %x\n",device->tag(),state));

	if (info->ch2 != state)
	{
		info->ch2 = state;
		info->ch2_update = 1;
	}
	if((!info->ch2)&&(info->ch2_update))
	{
		info->st_pulses = 0;
		struct ADPCMVoice *voice0 = &info->voice[0];
		struct ADPCMVoice *voice1 = &info->voice[1];
		// We set to channel 2
		MSM6376LOG(("OKIM6376:'%s' Channel 1\n",device->tag()));
		info->channel = 1;

		if ((voice0->playing)&&(info->st))
		{
			//Echo functions when Channel 1 is playing, and ST is still high
			info->command[1] = info->command[0];//copy sample over
			voice1->volume = volume_table[1]; //echo is 6dB attenuated
		}
	}

	if((info->ch2)&&(info->ch2_update))
	{
		info->stage[1]=0;
		oki_process(info, 1, info->command[1]);
		MSM6376LOG(("OKIM6376:'%s' Channel 0\n",device->tag()));
		info->channel = 0;
	}
}


WRITE_LINE_DEVICE_HANDLER( okim6376_st_w )
{
	//As in STart, presumably, this triggers everything

	okim6376_state *info = get_safe_token(device);
	info->st_update = 0;//Clear flag
	MSM6376LOG(("OKIM6376:'%s' ST %x\n",device->tag(),state));

	if (info->st != state)
	{
		info->st = state;
		info->st_update = 1;

		if ((info->channel == 1) & !info->st)//ST acts as attenuation for Channel 2 when low, and stays at that level until the channel is reset
		{
			struct ADPCMVoice *voice = &info->voice[info->channel];
			{
				info->st_pulses ++;
				MSM6376LOG(("OKIM6376:'%s' ST pulses %x\n",device->tag(),info->st_pulses));
				if (info->st_pulses > 3)
				{
					info->st_pulses = 3; //undocumented behaviour beyond 3 pulses
				}

				voice->volume = volume_table[info->st_pulses - 1];
			}
		}
		if (info->st && info->st_update)
		{
			info->command[info->channel] = info->latch;
			if (info->channel ==0 && info->nar)
			{
				info->stage[info->channel]=0;
				oki_process(info, 0, info->command[0]);
				info->nar = 0;
				info->nartimer = 4;
				/*According to datasheet, NAR timing is ~375 us at 8KHz, and is inversely proportional to sample rate, effectively 6 stream updates. */
			}
		}
	}
}

/**********************************************************************************************

     okim6376_data_w -- write to the data port of an OKIM6376-compatible chip

***********************************************************************************************/

WRITE8_DEVICE_HANDLER( okim6376_w )
{
	// The data port is purely used to set the latch, everything else is started by an ST pulse

	okim6376_state *info = get_safe_token(device);
	info->latch = data & 0x7f;
	// FIX: maximum adpcm words are 111, there are other 8 commands to generate BEEP tone (0x70 to 0x77),
	// and others for internal testing, that manual explicitly says not to use (0x78 to 0x7f)
	// We aren't doing anything with the BEEP at the moment, as we'd need to mix the ADPCM stream with beep.c
}


const device_type OKIM6376 = &device_creator<okim6376_device>;

okim6376_device::okim6376_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, OKIM6376, "OKI6376", tag, owner, clock),
		device_sound_interface(mconfig, *this)
{
	m_token = global_alloc_clear(okim6376_state);
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void okim6376_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void okim6376_device::device_start()
{
	DEVICE_START_NAME( okim6376 )(this);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void okim6376_device::device_reset()
{
	DEVICE_RESET_NAME( okim6376 )(this);
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void okim6376_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// should never get here
	fatalerror("sound_stream_update called; not applicable to legacy sound devices\n");
}
