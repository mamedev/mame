/**********************************************************************************************
 *
 *   OKI MSM6376 ADPCM
 *   by Mirko Buffoni
 *
 *   TODO:
 *     add BEEP tone generator
 *     add 2ch handling (for echoing and continuous speech)
 *
 **********************************************************************************************/


#include <math.h>

#include "sndintrf.h"
#include "streams.h"
#include "okim6376.h"

#define MAX_SAMPLE_CHUNK	10000
#define MAX_WORDS           111


/* struct describing a single playing ADPCM voice */
struct ADPCMVoice
{
	UINT8 playing;			/* 1 if we are actively playing */

	UINT32 base_offset;		/* pointer to the base memory location */
	UINT32 sample;			/* current sample number */
	UINT32 count;			/* total samples to play */

	UINT32 volume;			/* output volume */
	INT32 signal;
	INT32 step;
};

typedef struct _okim6376_state okim6376_state;
struct _okim6376_state
{
	#define OKIM6376_VOICES		2
	struct ADPCMVoice voice[OKIM6376_VOICES];
	INT32 command;
	UINT8 *region_base;		/* pointer to the base of the region */
	sound_stream *stream;	/* which stream are we playing on? */
	UINT32 master_clock;	/* master clock frequency */
};

/* step size index shift table */
static const int index_shift[8] = { -1, -1, -1, -1, 2, 4, 6, 8 };

/* lookup table for the precomputed difference */
static int diff_lookup[49*16];

/* volume lookup table. Upon configuration, the number of ST pulses determine how much
   attenuation to apply to the sound signal. */
static const int volume_table[4] =
{
	0x20,	//   0 dB
	0x10,	//  -6.0 dB
	0x08,	// -12.0 dB
	0x04,	// -24.0 dB
};

/* tables computed? */
static int tables_computed = 0;


INLINE okim6376_state *get_safe_token(const device_config *device)
{
	assert(device != NULL);
	assert(device->token != NULL);
	assert(device->type == SOUND);
	assert(sound_get_type(device) == SOUND_OKIM6376);
	return (okim6376_state *)device->token;
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



/**********************************************************************************************

     generate_adpcm -- general ADPCM decoding routine

***********************************************************************************************/

static void generate_adpcm(okim6376_state *chip, struct ADPCMVoice *voice, INT16 *buffer, int samples)
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

		/* loop while we have samples remaining */
		while (remaining)
		{
			int samples = (remaining > MAX_SAMPLE_CHUNK) ? MAX_SAMPLE_CHUNK : remaining;
			int samp;

			generate_adpcm(chip, voice, sample_data, samples);
			for (samp = 0; samp < samples; samp++)
				*buffer++ += sample_data[samp];

			remaining -= samples;
		}
	}
}



/**********************************************************************************************

     state save support for MAME

***********************************************************************************************/

static void adpcm_state_save_register(struct ADPCMVoice *voice, const device_config *device, int index)
{
	state_save_register_device_item(device, index, voice->playing);
	state_save_register_device_item(device, index, voice->sample);
	state_save_register_device_item(device, index, voice->count);
	state_save_register_device_item(device, index, voice->signal);
	state_save_register_device_item(device, index, voice->step);
	state_save_register_device_item(device, index, voice->volume);
	state_save_register_device_item(device, index, voice->base_offset);
}

static void okim6376_state_save_register(okim6376_state *info, const device_config *device)
{
	int j;

	state_save_register_device_item(device, 0, info->command);
	for (j = 0; j < OKIM6376_VOICES; j++)
		adpcm_state_save_register(&info->voice[j], device, j);
}



/**********************************************************************************************

     DEVICE_START( okim6376 ) -- start emulation of an OKIM6376-compatible chip

***********************************************************************************************/

static DEVICE_START( okim6376 )
{
	okim6376_state *info = get_safe_token(device);
	int voice;
	int divisor = 165;

	compute_tables();

	info->command = -1;
	info->region_base = device->region;
	info->master_clock = device->clock;

	/* generate the name and create the stream */
	info->stream = stream_create(device, 0, 1, device->clock/divisor, info, okim6376_update);

	/* initialize the voices */
	for (voice = 0; voice < OKIM6376_VOICES; voice++)
	{
		/* initialize the rest of the structure */
		info->voice[voice].volume = 0;
		reset_adpcm(&info->voice[voice]);
	}

	okim6376_state_save_register(info, device);
}



/**********************************************************************************************

     DEVICE_RESET( okim6376 ) -- stop emulation of an OKIM6376-compatible chip

***********************************************************************************************/

static DEVICE_RESET( okim6376 )
{
	okim6376_state *info = get_safe_token(device);
	int i;

	stream_update(info->stream);
	for (i = 0; i < OKIM6376_VOICES; i++)
		info->voice[i].playing = 0;
}




/**********************************************************************************************

     okim6376_status_r -- read the status port of an OKIM6376-compatible chip

***********************************************************************************************/

READ8_DEVICE_HANDLER( okim6376_r )
{
	okim6376_state *info = get_safe_token(device);
	int i, result;

	result = 0xff;

	/* set the bit to 1 if something is playing on a given channel */
	stream_update(info->stream);
	for (i = 0; i < OKIM6376_VOICES; i++)
	{
		struct ADPCMVoice *voice = &info->voice[i];

		/* set the bit if it's playing */
		if (!voice->playing)
			result ^= 1 << i;
	}

	return result;
}



/**********************************************************************************************

     okim6376_data_w -- write to the data port of an OKIM6376-compatible chip

***********************************************************************************************/

WRITE8_DEVICE_HANDLER( okim6376_w )
{
	okim6376_state *info = get_safe_token(device);

	/* if a command is pending, process the second half */
	if (info->command != -1)
	{
		int temp = data >> 4, i, start;
		unsigned char *base, *base_end;


		/* FIX: Check if it's possible to start multiple voices at the same time */
		if (temp != 0 && temp != 1 && temp != 2)
			popmessage("OKI6376 start %x contact MAMEDEV", temp);

		/* update the stream */
		stream_update(info->stream);

		/* determine which voice(s) (voice is set by a 1 bit in the upper 4 bits of the second byte) */
		for (i = 0; i < OKIM6376_VOICES; i++, temp >>= 1)
		{
			if (temp & 1)
			{
				struct ADPCMVoice *voice = &info->voice[i];

				/* determine the start position, max address space is 16Mbit */
				base = &info->region_base[info->command * 4];
				base_end = &info->region_base[(MAX_WORDS+1) * 4];
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
						/* FIX: no attenuation for now */
						voice->volume = volume_table[0];
					}
					else
					{
						logerror("OKIM6376:'%s' requested to play sample %02x on non-stopped voice\n",device->tag,info->command);
					}
				}
			}
		}

		/* reset the command */
		info->command = -1;
	}

	/* if this is the start of a command, remember the sample number for next time */
	else if (data & 0x80)
	{
		// FIX: maximum adpcm words are 111, there are other 8 commands to generate BEEP tone (0x70 to 0x77),
		// and others for internal testing, that manual explicitly says not to use (0x78 to 0x7f)
		info->command = data & 0x7f;
	}

	/* otherwise, see if this is a silence command */
	else
	{
		int temp = data >> 3, i;

		/* update the stream, then turn it off */
		stream_update(info->stream);

		/* determine which voice(s) (voice is set by a 1 bit in bits 3-6 of the command */
		for (i = 0; i < OKIM6376_VOICES; i++, temp >>= 1)
		{
			if (temp & 1)
			{
				struct ADPCMVoice *voice = &info->voice[i];

				voice->playing = 0;
			}
		}
	}
}



/**************************************************************************
 * Generic get_info
 **************************************************************************/

DEVICE_GET_INFO( okim6376 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:					info->i = sizeof(okim6376_state);					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME( okim6376 );			break;
		case DEVINFO_FCT_STOP:							/* nothing */										break;
		case DEVINFO_FCT_RESET:							info->reset = DEVICE_RESET_NAME( okim6376 );			break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "OKI6376");							break;
		case DEVINFO_STR_FAMILY:					strcpy(info->s, "OKI ADPCM");						break;
		case DEVINFO_STR_VERSION:					strcpy(info->s, "1.0");								break;
		case DEVINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);							break;
		case DEVINFO_STR_CREDITS:					strcpy(info->s, "Copyright Nicola Salmoria and the MAME Team"); break;
	}
}

