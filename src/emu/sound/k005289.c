/***************************************************************************

    Konami 005289 - SCC sound as used in Bubblesystem

    This file is pieced together by Bryan McPhail from a combination of
    Namco Sound, Amuse by Cab, Nemesis schematics and whoever first
    figured out SCC!

    The 005289 is a 2 channel sound generator, each channel gets it's
    waveform from a prom (4 bits wide).

    (From Nemesis schematics)

    Address lines A0-A4 of the prom run to the 005289, giving 32 bytes
    per waveform.  Address lines A5-A7 of the prom run to PA5-PA7 of
    the AY8910 control port A, giving 8 different waveforms. PA0-PA3
    of the AY8910 control volume.

    The second channel is the same as above except port B is used.

    The 005289 has no data bus, so data values written don't matter.

    There are 4 unknown pins, LD1, LD2, TG1, TG2.  Two of them look to be
    the selector for changing frequency.  The other two seem unused.

***************************************************************************/

#include "sndintrf.h"
#include "streams.h"
#include "k005289.h"

#define FREQBASEBITS	16

/* this structure defines the parameters for a channel */
typedef struct
{
	int frequency;
	int counter;
	int volume;
	const unsigned char *wave;
} k005289_sound_channel;

struct k005289_info
{
	k005289_sound_channel channel_list[2];

	/* global sound parameters */
	const unsigned char *sound_prom;
	sound_stream * stream;
	int mclock,rate;

	/* mixer tables and internal buffers */
	INT16 *mixer_table;
	INT16 *mixer_lookup;
	short *mixer_buffer;

	int k005289_A_frequency,k005289_B_frequency;
	int k005289_A_volume,k005289_B_volume;
	int k005289_A_waveform,k005289_B_waveform;
	int k005289_A_latch,k005289_B_latch;
};

/* build a table to divide by the number of voices */
static int make_mixer_table(struct k005289_info *info, int voices)
{
	int count = voices * 128;
	int i;
	int gain = 16;

	/* allocate memory */
	info->mixer_table = auto_malloc(256 * voices * sizeof(INT16));

	/* find the middle of the table */
	info->mixer_lookup = info->mixer_table + (128 * voices);

	/* fill in the table - 16 bit case */
	for (i = 0; i < count; i++)
	{
		int val = i * gain * 16 / voices;
		if (val > 32767) val = 32767;
		info->mixer_lookup[ i] = val;
		info->mixer_lookup[-i] = -val;
	}

	return 0;
}


/* generate sound to the mix buffer */
static void K005289_update(void *param, stream_sample_t **inputs, stream_sample_t **_buffer, int length)
{
	struct k005289_info *info = param;
	k005289_sound_channel *voice=info->channel_list;
	stream_sample_t *buffer = _buffer[0];
	short *mix;
	int i,v,f;

	/* zap the contents of the mixer buffer */
	memset(info->mixer_buffer, 0, length * sizeof(INT16));

	v=voice[0].volume;
	f=voice[0].frequency;
	if (v && f)
	{
		const unsigned char *w = voice[0].wave;
		int c = voice[0].counter;

		mix = info->mixer_buffer;

		/* add our contribution */
		for (i = 0; i < length; i++)
		{
			int offs;

			c+=(long)((((float)info->mclock / (float)(f * 16))*(float)(1<<FREQBASEBITS)) / (float)(info->rate / 32));
			offs = (c >> 16) & 0x1f;
			*mix++ += ((w[offs] & 0x0f) - 8) * v;
		}

		/* update the counter for this voice */
		voice[0].counter = c;
	}

	v=voice[1].volume;
	f=voice[1].frequency;
	if (v && f)
	{
		const unsigned char *w = voice[1].wave;
		int c = voice[1].counter;

		mix = info->mixer_buffer;

		/* add our contribution */
		for (i = 0; i < length; i++)
		{
			int offs;

			c+=(long)((((float)info->mclock / (float)(f * 16))*(float)(1<<FREQBASEBITS)) / (float)(info->rate / 32));
			offs = (c >> 16) & 0x1f;
			*mix++ += ((w[offs] & 0x0f) - 8) * v;
		}

		/* update the counter for this voice */
		voice[1].counter = c;
	}

	/* mix it down */
	mix = info->mixer_buffer;
	for (i = 0; i < length; i++)
		*buffer++ = info->mixer_lookup[*mix++];
}

static void *k005289_start(int sndindex, int clock, const void *config)
{
	k005289_sound_channel *voice;
	const struct k005289_interface *intf = config;
	struct k005289_info *info;

	info = auto_malloc(sizeof(*info));
	memset(info, 0, sizeof(*info));
	voice = info->channel_list;

	/* get stream channels */
	info->rate = clock/16;
	info->stream = stream_create(0, 1, info->rate, info, K005289_update);
	info->mclock = clock;

	/* allocate a pair of buffers to mix into - 1 second's worth should be more than enough */
	info->mixer_buffer = auto_malloc(2 * sizeof(short) * info->rate);

	/* build the mixer table */
	if (make_mixer_table(info, 2))
		return NULL;

	info->sound_prom = memory_region(intf->region);

	/* reset all the voices */
	voice[0].frequency = 0;
	voice[0].volume = 0;
	voice[0].wave = &info->sound_prom[0];
	voice[0].counter = 0;
	voice[1].frequency = 0;
	voice[1].volume = 0;
	voice[1].wave = &info->sound_prom[0x100];
	voice[1].counter = 0;

	return info;
}


/********************************************************************************/

static void k005289_recompute(struct k005289_info *info)
{
	k005289_sound_channel *voice = info->channel_list;

	stream_update(info->stream); 	/* update the streams */

	voice[0].frequency = info->k005289_A_frequency;
	voice[1].frequency = info->k005289_B_frequency;
	voice[0].volume = info->k005289_A_volume;
	voice[1].volume = info->k005289_B_volume;
	voice[0].wave = &info->sound_prom[32 * info->k005289_A_waveform];
	voice[1].wave = &info->sound_prom[32 * info->k005289_B_waveform + 0x100];
}

WRITE8_HANDLER( k005289_control_A_w )
{
	struct k005289_info *info = sndti_token(SOUND_K005289, 0);
	info->k005289_A_volume=data&0xf;
	info->k005289_A_waveform=data>>5;
	k005289_recompute(info);
}

WRITE8_HANDLER( k005289_control_B_w )
{
	struct k005289_info *info = sndti_token(SOUND_K005289, 0);
	info->k005289_B_volume=data&0xf;
	info->k005289_B_waveform=data>>5;
	k005289_recompute(info);
}

WRITE8_HANDLER( k005289_pitch_A_w )
{
	struct k005289_info *info = sndti_token(SOUND_K005289, 0);
	info->k005289_A_latch = 0x1000 - offset;
}

WRITE8_HANDLER( k005289_pitch_B_w )
{
	struct k005289_info *info = sndti_token(SOUND_K005289, 0);
	info->k005289_B_latch = 0x1000 - offset;
}

WRITE8_HANDLER( k005289_keylatch_A_w )
{
	struct k005289_info *info = sndti_token(SOUND_K005289, 0);
	info->k005289_A_frequency = info->k005289_A_latch;
	k005289_recompute(info);
}

WRITE8_HANDLER( k005289_keylatch_B_w )
{
	struct k005289_info *info = sndti_token(SOUND_K005289, 0);
	info->k005289_B_frequency = info->k005289_B_latch;
	k005289_recompute(info);
}




/**************************************************************************
 * Generic get_info
 **************************************************************************/

static void k005289_set_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		/* no parameters to set */
	}
}


void k005289_get_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case SNDINFO_PTR_SET_INFO:						info->set_info = k005289_set_info;		break;
		case SNDINFO_PTR_START:							info->start = k005289_start;			break;
		case SNDINFO_PTR_STOP:							/* nothing */							break;
		case SNDINFO_PTR_RESET:							/* nothing */							break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case SNDINFO_STR_NAME:							info->s = "K005289";					break;
		case SNDINFO_STR_CORE_FAMILY:					info->s = "Konami custom";				break;
		case SNDINFO_STR_CORE_VERSION:					info->s = "1.0";						break;
		case SNDINFO_STR_CORE_FILE:						info->s = __FILE__;						break;
		case SNDINFO_STR_CORE_CREDITS:					info->s = "Copyright (c) 2004, The MAME Team"; break;
	}
}

