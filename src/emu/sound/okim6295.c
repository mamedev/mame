/**********************************************************************************************
 *
 *   streaming ADPCM driver
 *   by Aaron Giles
 *
 *   Library to transcode from an ADPCM source to raw PCM.
 *   Written by Buffoni Mirko in 08/06/97
 *   References: various sources and documents.
 *
 *   HJB 08/31/98
 *   modified to use an automatically selected oversampling factor
 *   for the current sample rate
 *
 *   Mish 21/7/99
 *   Updated to allow multiple OKI chips with different sample rates
 *
 *   R. Belmont 31/10/2003
 *   Updated to allow a driver to use both MSM6295s and "raw" ADPCM voices (gcpinbal)
 *   Also added some error trapping for MAME_DEBUG builds
 *
 **********************************************************************************************/


#include <math.h>

#include "sndintrf.h"
#include "streams.h"
#include "okim6295.h"

#define MAX_SAMPLE_CHUNK	10000


/* struct describing a single playing ADPCM voice */
struct ADPCMVoice
{
	UINT8 playing;			/* 1 if we are actively playing */

	UINT32 base_offset;		/* pointer to the base memory location */
	UINT32 sample;			/* current sample number */
	UINT32 count;			/* total samples to play */

	struct adpcm_state adpcm;/* current ADPCM state */
	UINT32 volume;			/* output volume */
};

struct okim6295
{
	#define OKIM6295_VOICES		4
	struct ADPCMVoice voice[OKIM6295_VOICES];
	INT32 command;
	INT32 bank_offset;
	UINT8 *region_base;		/* pointer to the base of the region */
	sound_stream *stream;	/* which stream are we playing on? */
	UINT32 master_clock;	/* master clock frequency */
};

/* step size index shift table */
static const int index_shift[8] = { -1, -1, -1, -1, 2, 4, 6, 8 };

/* lookup table for the precomputed difference */
static int diff_lookup[49*16];

/* volume lookup table. The manual lists only 9 steps, ~3dB per step. Given the dB values,
   that seems to map to a 5-bit volume control. What happens for steps after the 9th is not
	known, we arbitrarily assign a "1" to all those steps. */
static int volume_table[16] =
{
	0x20,	//   0 dB
	0x16,	//  -3.2 dB
	0x10,	//  -6.0 dB
	0x0b,	//  -9.2 dB
	0x08,	// -12.0 dB
	0x06,	// -14.5 dB
	0x04,	// -18.0 dB
	0x03,	// -20.5 dB
	0x02,	// -24.0 dB
	0x01,	// unknown
	0x01,	// unknown
	0x01,	// unknown
	0x01,	// unknown
	0x01,	// unknown
	0x01,	// unknown
	0x01,	// unknown
};

/* tables computed? */
static int tables_computed = 0;

/* useful interfaces */
const okim6295_interface okim6295_interface_pin7high = { 1 };
const okim6295_interface okim6295_interface_pin7low = { 0 };



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

void reset_adpcm(struct adpcm_state *state)
{
	/* make sure we have our tables */
	if (!tables_computed)
		compute_tables();

	/* reset the signal/step */
	state->signal = -2;
	state->step = 0;
}



/**********************************************************************************************

     clock_adpcm -- clock the next ADPCM byte

***********************************************************************************************/

INT16 clock_adpcm(struct adpcm_state *state, UINT8 nibble)
{
	state->signal += diff_lookup[state->step * 16 + (nibble & 15)];

	/* clamp to the maximum */
	if (state->signal > 2047)
		state->signal = 2047;
	else if (state->signal < -2048)
		state->signal = -2048;

	/* adjust the step size and clamp */
	state->step += index_shift[nibble & 7];
	if (state->step > 48)
		state->step = 48;
	else if (state->step < 0)
		state->step = 0;

	/* return the signal */
	return state->signal;
}



/**********************************************************************************************

     generate_adpcm -- general ADPCM decoding routine

***********************************************************************************************/

static void generate_adpcm(struct okim6295 *chip, struct ADPCMVoice *voice, INT16 *buffer, int samples)
{
	/* if this voice is active */
	if (voice->playing)
	{
		UINT8 *base = chip->region_base + chip->bank_offset + voice->base_offset;
		int sample = voice->sample;
		int count = voice->count;

		/* loop while we still have samples to generate */
		while (samples)
		{
			/* compute the new amplitude and update the current step */
			int nibble = base[sample / 2] >> (((sample & 1) << 2) ^ 4);

			/* output to the buffer, scaling by the volume */
			/* signal in range -2048..2047, volume in range 2..32 => signal * volume / 2 in range -32768..32767 */
			*buffer++ = clock_adpcm(&voice->adpcm, nibble) * voice->volume / 2;
			samples--;

			/* next! */
			if (++sample >= count)
			{
				voice->playing = 0;
				break;
			}
		}

		/* update the parameters */
		voice->sample = sample;
	}

	/* fill the rest with silence */
	while (samples--)
		*buffer++ = 0;
}



/**********************************************************************************************
 *
 *  OKIM 6295 ADPCM chip:
 *
 *  Command bytes are sent:
 *
 *      1xxx xxxx = start of 2-byte command sequence, xxxxxxx is the sample number to trigger
 *      abcd vvvv = second half of command; one of the abcd bits is set to indicate which voice
 *                  the v bits seem to be volumed
 *
 *      0abc d000 = stop playing; one or more of the abcd bits is set to indicate which voice(s)
 *
 *  Status is read:
 *
 *      ???? abcd = one bit per voice, set to 0 if nothing is playing, or 1 if it is active
 *
***********************************************************************************************/


/**********************************************************************************************

     okim6295_update -- update the sound chip so that it is in sync with CPU execution

***********************************************************************************************/

static void okim6295_update(void *param, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	struct okim6295 *chip = param;
	int i;

	memset(outputs[0], 0, samples * sizeof(*outputs[0]));

	for (i = 0; i < OKIM6295_VOICES; i++)
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

static void adpcm_state_save_register(struct ADPCMVoice *voice, const char *tag, int index)
{
	state_save_register_item("okim6295", tag, index, voice->playing);
	state_save_register_item("okim6295", tag, index, voice->sample);
	state_save_register_item("okim6295", tag, index, voice->count);
	state_save_register_item("okim6295", tag, index, voice->adpcm.signal);
	state_save_register_item("okim6295", tag, index, voice->adpcm.step);
	state_save_register_item("okim6295", tag, index, voice->volume);
	state_save_register_item("okim6295", tag, index, voice->base_offset);
}

static void okim6295_state_save_register(struct okim6295 *info, const char *tag)
{
	int j;

	state_save_register_item("okim6295", tag, 0, info->command);
	state_save_register_item("okim6295", tag, 0, info->bank_offset);
	for (j = 0; j < OKIM6295_VOICES; j++)
		adpcm_state_save_register(&info->voice[j], tag, j);
}



/**********************************************************************************************

     SND_START( okim6295 ) -- start emulation of an OKIM6295-compatible chip

***********************************************************************************************/

static SND_START( okim6295 )
{
	const okim6295_interface *intf = config;
	struct okim6295 *info;
	int voice;
	int divisor = intf->pin7 ? 132 : 165;

	info = auto_malloc(sizeof(*info));
	memset(info, 0, sizeof(*info));

	compute_tables();

	info->command = -1;
	info->bank_offset = 0;
	info->region_base = memory_region(device->machine, (intf->rgnoverride != NULL) ? intf->rgnoverride : tag);

	info->master_clock = clock;

	/* generate the name and create the stream */
	info->stream = stream_create(0, 1, clock/divisor, info, okim6295_update);

	/* initialize the voices */
	for (voice = 0; voice < OKIM6295_VOICES; voice++)
	{
		/* initialize the rest of the structure */
		info->voice[voice].volume = 0;
		reset_adpcm(&info->voice[voice].adpcm);
	}

	okim6295_state_save_register(info, tag);

	/* success */
	return info;
}



/**********************************************************************************************

     SND_RESET( okim6295 ) -- stop emulation of an OKIM6295-compatible chip

***********************************************************************************************/

static SND_RESET( okim6295 )
{
	struct okim6295 *info = device->token;
	int i;

	stream_update(info->stream);
	for (i = 0; i < OKIM6295_VOICES; i++)
		info->voice[i].playing = 0;
}



/**********************************************************************************************

     okim6295_set_bank_base -- set the base of the bank for a given voice on a given chip

***********************************************************************************************/

void okim6295_set_bank_base(int which, int base)
{
	struct okim6295 *info = sndti_token(SOUND_OKIM6295, which);
	stream_update(info->stream);
	info->bank_offset = base;
}



/**********************************************************************************************

     okim6295_set_pin7 -- adjust pin 7, which controls the internal clock division

***********************************************************************************************/

void okim6295_set_pin7(int which, int pin7)
{
	struct okim6295 *info = sndti_token(SOUND_OKIM6295, which);
	int divisor = pin7 ? 132 : 165;

	stream_set_sample_rate(info->stream, info->master_clock/divisor);
}


/**********************************************************************************************

     okim6295_status_r -- read the status port of an OKIM6295-compatible chip

***********************************************************************************************/

static int okim6295_status_r(int num)
{
	struct okim6295 *info = sndti_token(SOUND_OKIM6295, num);
	int i, result;

	result = 0xf0;	/* naname expects bits 4-7 to be 1 */

	/* set the bit to 1 if something is playing on a given channel */
	stream_update(info->stream);
	for (i = 0; i < OKIM6295_VOICES; i++)
	{
		struct ADPCMVoice *voice = &info->voice[i];

		/* set the bit if it's playing */
		if (voice->playing)
			result |= 1 << i;
	}

	return result;
}



/**********************************************************************************************

     okim6295_data_w -- write to the data port of an OKIM6295-compatible chip

***********************************************************************************************/

static void okim6295_data_w(int num, int data)
{
	struct okim6295 *info = sndti_token(SOUND_OKIM6295, num);

	/* if a command is pending, process the second half */
	if (info->command != -1)
	{
		int temp = data >> 4, i, start, stop;
		unsigned char *base;


		/* the manual explicitly says that it's not possible to start multiple voices at the same time */
		if (temp != 1 && temp != 2 && temp != 4 && temp != 8)
			popmessage("OKI6295 start %x contact MAMEDEV", temp);

		/* update the stream */
		stream_update(info->stream);

		/* determine which voice(s) (voice is set by a 1 bit in the upper 4 bits of the second byte) */
		for (i = 0; i < OKIM6295_VOICES; i++, temp >>= 1)
		{
			if (temp & 1)
			{
				struct ADPCMVoice *voice = &info->voice[i];

				/* determine the start/stop positions */
				base = &info->region_base[info->bank_offset + info->command * 8];
				start = ((base[0] << 16) + (base[1] << 8) + base[2]) & 0x3ffff;
				stop  = ((base[3] << 16) + (base[4] << 8) + base[5]) & 0x3ffff;

				/* set up the voice to play this sample */
				if (start < stop)
				{
					if (!voice->playing) /* fixes Got-cha and Steel Force */
					{
						voice->playing = 1;
						voice->base_offset = start;
						voice->sample = 0;
						voice->count = 2 * (stop - start + 1);

						/* also reset the ADPCM parameters */
						reset_adpcm(&voice->adpcm);
						voice->volume = volume_table[data & 0x0f];

						/* the manual only lists 9 volume steps */
						if ((data & 0x0f) >= 9)
							popmessage("OKI6295 volume %x contact MAMEDEV", data & 0x0f);
					}
					else
					{
						logerror("OKIM6295:%d requested to play sample %02x on non-stopped voice\n",num,info->command);
					}
				}
				/* invalid samples go here */
				else
				{
					logerror("OKIM6295:%d requested to play invalid sample %02x\n",num,info->command);
					voice->playing = 0;
				}
			}
		}

		/* reset the command */
		info->command = -1;
	}

	/* if this is the start of a command, remember the sample number for next time */
	else if (data & 0x80)
	{
		info->command = data & 0x7f;
	}

	/* otherwise, see if this is a silence command */
	else
	{
		int temp = data >> 3, i;

		/* update the stream, then turn it off */
		stream_update(info->stream);

		/* determine which voice(s) (voice is set by a 1 bit in bits 3-6 of the command */
		for (i = 0; i < 4; i++, temp >>= 1)
		{
			if (temp & 1)
			{
				struct ADPCMVoice *voice = &info->voice[i];

				voice->playing = 0;
			}
		}
	}
}



/**********************************************************************************************

     okim6295_status_0_r -- generic status read functions
     okim6295_status_1_r

***********************************************************************************************/

READ8_HANDLER( okim6295_status_0_r )
{
	return okim6295_status_r(0);
}

READ8_HANDLER( okim6295_status_1_r )
{
	return okim6295_status_r(1);
}

READ8_HANDLER( okim6295_status_2_r )
{
	return okim6295_status_r(2);
}

READ16_HANDLER( okim6295_status_0_lsb_r )
{
	return okim6295_status_r(0);
}

READ16_HANDLER( okim6295_status_1_lsb_r )
{
	return okim6295_status_r(1);
}

READ16_HANDLER( okim6295_status_2_lsb_r )
{
	return okim6295_status_r(2);
}

READ16_HANDLER( okim6295_status_0_msb_r )
{
	return okim6295_status_r(0) << 8;
}

READ16_HANDLER( okim6295_status_1_msb_r )
{
	return okim6295_status_r(1) << 8;
}

READ16_HANDLER( okim6295_status_2_msb_r )
{
	return okim6295_status_r(2) << 8;
}



/**********************************************************************************************

     okim6295_data_0_w -- generic data write functions
     okim6295_data_1_w

***********************************************************************************************/

WRITE8_HANDLER( okim6295_data_0_w )
{
	okim6295_data_w(0, data);
}

WRITE8_HANDLER( okim6295_data_1_w )
{
	okim6295_data_w(1, data);
}

WRITE8_HANDLER( okim6295_data_2_w )
{
	okim6295_data_w(2, data);
}

WRITE16_HANDLER( okim6295_data_0_lsb_w )
{
	if (ACCESSING_BITS_0_7)
		okim6295_data_w(0, data & 0xff);
}

WRITE16_HANDLER( okim6295_data_1_lsb_w )
{
	if (ACCESSING_BITS_0_7)
		okim6295_data_w(1, data & 0xff);
}

WRITE16_HANDLER( okim6295_data_2_lsb_w )
{
	if (ACCESSING_BITS_0_7)
		okim6295_data_w(2, data & 0xff);
}

WRITE16_HANDLER( okim6295_data_0_msb_w )
{
	if (ACCESSING_BITS_8_15)
		okim6295_data_w(0, data >> 8);
}

WRITE16_HANDLER( okim6295_data_1_msb_w )
{
	if (ACCESSING_BITS_8_15)
		okim6295_data_w(1, data >> 8);
}

WRITE16_HANDLER( okim6295_data_2_msb_w )
{
	if (ACCESSING_BITS_8_15)
		okim6295_data_w(2, data >> 8);
}



/**************************************************************************
 * Generic get_info
 **************************************************************************/

static SND_SET_INFO( okim6295 )
{
	switch (state)
	{
		/* no parameters to set */
	}
}


SND_GET_INFO( okim6295 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case SNDINFO_PTR_SET_INFO:						info->set_info = SND_SET_INFO_NAME( okim6295 );		break;
		case SNDINFO_PTR_START:							info->start = SND_START_NAME( okim6295 );			break;
		case SNDINFO_PTR_STOP:							/* nothing */							break;
		case SNDINFO_PTR_RESET:							info->reset = SND_RESET_NAME( okim6295 );			break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case SNDINFO_STR_NAME:							info->s = "OKI6295";					break;
		case SNDINFO_STR_CORE_FAMILY:					info->s = "OKI ADPCM";					break;
		case SNDINFO_STR_CORE_VERSION:					info->s = "1.0";						break;
		case SNDINFO_STR_CORE_FILE:						info->s = __FILE__;						break;
		case SNDINFO_STR_CORE_CREDITS:					info->s = "Copyright Nicola Salmoria and the MAME Team"; break;
	}
}

