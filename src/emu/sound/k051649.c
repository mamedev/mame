/***************************************************************************

    Konami 051649 - SCC1 sound as used in Haunted Castle, City Bomber

    This file is pieced together by Bryan McPhail from a combination of
    Namco Sound, Amuse by Cab, Haunted Castle schematics and whoever first
    figured out SCC!

    The 051649 is a 5 channel sound generator, each channel gets it's
    waveform from RAM (32 bytes per waveform, 8 bit signed data).

    This sound chip is the same as the sound chip in some Konami
    megaROM cartridges for the MSX. It is actually well researched
    and documented:

        http://www.msxnet.org/tech/scc

    Thanks to Sean Young (sean@mess.org) for some bugfixes.

    K052539 is equivalent to this chip except channel 5 does not share
    waveforms with channel 4.

***************************************************************************/

#include "sndintrf.h"
#include "streams.h"
#include "k051649.h"

#define FREQBASEBITS	16

/* this structure defines the parameters for a channel */
typedef struct
{
	unsigned long counter;
	int frequency;
	int volume;
	int key;
	signed char waveform[32];		/* 19991207.CAB */
} k051649_sound_channel;

struct k051649_info
{
	k051649_sound_channel channel_list[5];

	/* global sound parameters */
	sound_stream * stream;
	int mclock,rate;

	/* mixer tables and internal buffers */
	INT16 *mixer_table;
	INT16 *mixer_lookup;
	short *mixer_buffer;

	int f[10];
};

/* build a table to divide by the number of voices */
static int make_mixer_table(struct k051649_info *info, int voices)
{
	int count = voices * 256;
	int i;
	int gain = 8;

	/* allocate memory */
	info->mixer_table = auto_malloc(512 * voices * sizeof(INT16));

	/* find the middle of the table */
	info->mixer_lookup = info->mixer_table + (256 * voices);

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
static void K051649_update(void *param, stream_sample_t **inputs, stream_sample_t **_buffer, int length)
{
	struct k051649_info *info = param;
	k051649_sound_channel *voice=info->channel_list;
	stream_sample_t *buffer = _buffer[0];
	short *mix;
	int i,v,f,j,k;

	/* zap the contents of the mixer buffer */
	memset(info->mixer_buffer, 0, length * sizeof(short));

	for (j=0; j<5; j++) {
		v=voice[j].volume;
		f=voice[j].frequency;
		k=voice[j].key;
		/* SY 20040109: the SCC produces no sound for freq < 9 */
		if (v && f > 8 && k)
		{
			const signed char *w = voice[j].waveform;			/* 19991207.CAB */
			int c=voice[j].counter;

			mix = info->mixer_buffer;

			/* add our contribution */
			for (i = 0; i < length; i++)
			{
				int offs;

				/* Amuse source:  Cab suggests this method gives greater resolution */
				/* Sean Young 20010417: the formula is really: f = clock/(16*(f+1))*/
				c+=(long)((((float)info->mclock / (float)((f+1) * 16))*(float)(1<<FREQBASEBITS)) / (float)(info->rate / 32));
				offs = (c >> 16) & 0x1f;
				*mix++ += (w[offs] * v)>>3;
			}

			/* update the counter for this voice */
			voice[j].counter = c;
		}
	}

	/* mix it down */
	mix = info->mixer_buffer;
	for (i = 0; i < length; i++)
		*buffer++ = info->mixer_lookup[*mix++];
}

static void *k051649_start(int sndindex, int clock, const void *config)
{
	struct k051649_info *info;

	info = auto_malloc(sizeof(*info));
	memset(info, 0, sizeof(*info));

	/* get stream channels */
	info->rate = clock/16;
	info->stream = stream_create(0, 1, info->rate, info, K051649_update);
	info->mclock = clock;

	/* allocate a buffer to mix into - 1 second's worth should be more than enough */
	info->mixer_buffer = auto_malloc(2 * sizeof(short) * info->rate);

	/* build the mixer table */
	if (make_mixer_table(info, 5))
		return NULL;

	return info;
}

static void k051649_reset(void *chip)
{
	struct k051649_info *info = chip;
	k051649_sound_channel *voice = info->channel_list;
	int i;

	/* reset all the voices */
	for (i=0; i>5; i++) {
		voice[i].frequency = 0;
		voice[i].volume = 0;
		voice[i].counter = 0;
	}
}

/********************************************************************************/

WRITE8_HANDLER( K051649_waveform_w )
{
	struct k051649_info *info = sndti_token(SOUND_K051649, 0);
	stream_update(info->stream);
	info->channel_list[offset>>5].waveform[offset&0x1f]=data;
	/* SY 20001114: Channel 5 shares the waveform with channel 4 */
    if (offset >= 0x60)
		info->channel_list[4].waveform[offset&0x1f]=data;
}

READ8_HANDLER ( K051649_waveform_r )
{
	struct k051649_info *info = sndti_token(SOUND_K051649, 0);
	return info->channel_list[offset>>5].waveform[offset&0x1f];
}

/* SY 20001114: Channel 5 doesn't share the waveform with channel 4 on this chip */
WRITE8_HANDLER( K052539_waveform_w )
{
	struct k051649_info *info = sndti_token(SOUND_K051649, 0);
	stream_update(info->stream);
	info->channel_list[offset>>5].waveform[offset&0x1f]=data;
}

WRITE8_HANDLER( K051649_volume_w )
{
	struct k051649_info *info = sndti_token(SOUND_K051649, 0);
	stream_update(info->stream);
	info->channel_list[offset&0x7].volume=data&0xf;
}

WRITE8_HANDLER( K051649_frequency_w )
{
	struct k051649_info *info = sndti_token(SOUND_K051649, 0);
	info->f[offset]=data;

	stream_update(info->stream);
	info->channel_list[offset>>1].frequency=(info->f[offset&0xe] + (info->f[offset|1]<<8))&0xfff;
}

WRITE8_HANDLER( K051649_keyonoff_w )
{
	struct k051649_info *info = sndti_token(SOUND_K051649, 0);
	stream_update(info->stream);
	info->channel_list[0].key=data&1;
	info->channel_list[1].key=data&2;
	info->channel_list[2].key=data&4;
	info->channel_list[3].key=data&8;
	info->channel_list[4].key=data&16;
}




/**************************************************************************
 * Generic get_info
 **************************************************************************/

static void k051649_set_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		/* no parameters to set */
	}
}


void k051649_get_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case SNDINFO_PTR_SET_INFO:						info->set_info = k051649_set_info;		break;
		case SNDINFO_PTR_START:							info->start = k051649_start;			break;
		case SNDINFO_PTR_STOP:							/* nothing */							break;
		case SNDINFO_PTR_RESET:							info->reset = k051649_reset;			break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case SNDINFO_STR_NAME:							info->s = "K051649";					break;
		case SNDINFO_STR_CORE_FAMILY:					info->s = "Konami custom";				break;
		case SNDINFO_STR_CORE_VERSION:					info->s = "1.0";						break;
		case SNDINFO_STR_CORE_FILE:						info->s = __FILE__;						break;
		case SNDINFO_STR_CORE_CREDITS:					info->s = "Copyright (c) 2004, The MAME Team"; break;
	}
}

