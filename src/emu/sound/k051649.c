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

#include "emu.h"
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

typedef struct _k051649_state k051649_state;
struct _k051649_state
{
	k051649_sound_channel channel_list[5];

	/* global sound parameters */
	sound_stream * stream;
	int mclock,rate;

	/* mixer tables and internal buffers */
	INT16 *mixer_table;
	INT16 *mixer_lookup;
	short *mixer_buffer;

	/* misc */
	UINT8 test;
	int f[10];
};

INLINE k051649_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == K051649);
	return (k051649_state *)downcast<legacy_device_base *>(device)->token();
}

/* build a table to divide by the number of voices */
static void make_mixer_table(running_machine &machine, k051649_state *info, int voices)
{
	int count = voices * 256;
	int i;
	int gain = 8;

	/* allocate memory */
	info->mixer_table = auto_alloc_array(machine, INT16, 512 * voices);

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
}


/* generate sound to the mix buffer */
static STREAM_UPDATE( k051649_update )
{
	k051649_state *info = (k051649_state *)param;
	k051649_sound_channel *voice=info->channel_list;
	stream_sample_t *buffer = outputs[0];
	short *mix;
	int i,v,f,j,k;

	/* zap the contents of the mixer buffer */
	memset(info->mixer_buffer, 0, samples * sizeof(short));

	for (j=0; j<5; j++) {
		v=voice[j].volume;
		f=voice[j].frequency;
		k=voice[j].key;
		/* SY 20040109: the SCC produces no sound for freq < 9 (channel is halted) */
		if (f > 8)
		{
			const signed char *w = voice[j].waveform;			/* 19991207.CAB */
			int c=voice[j].counter;

			mix = info->mixer_buffer;

			/* add our contribution */
			for (i = 0; i < samples; i++)
			{
				int offs;

				/* Amuse source:  Cab suggests this method gives greater resolution */
				/* Sean Young 20010417: the formula is really: f = clock/(16*(f+1))*/
				c+=(long)((((float)info->mclock / (float)((f+1) * 16))*(float)(1<<FREQBASEBITS)) / (float)(info->rate / 32));
				offs = (c >> FREQBASEBITS) & 0x1f;
				*mix++ += (w[offs] * v * k)>>3;
			}

			/* update the counter for this voice */
			voice[j].counter = c;
		}
	}

	/* mix it down */
	mix = info->mixer_buffer;
	for (i = 0; i < samples; i++)
		*buffer++ = info->mixer_lookup[*mix++];
}

static DEVICE_START( k051649 )
{
	k051649_state *info = get_safe_token(device);

	/* get stream channels */
	info->rate = device->clock()/16;
	info->stream = device->machine().sound().stream_alloc(*device, 0, 1, info->rate, info, k051649_update);
	info->mclock = device->clock();

	/* allocate a buffer to mix into - 1 second's worth should be more than enough */
	info->mixer_buffer = auto_alloc_array(device->machine(), short, 2 * info->rate);

	/* build the mixer table */
	make_mixer_table(device->machine(), info, 5);
}

static DEVICE_RESET( k051649 )
{
	k051649_state *info = get_safe_token(device);
	k051649_sound_channel *voice = info->channel_list;
	int i;

	/* reset all the voices */
	for (i = 0; i < 5; i++) {
		voice[i].frequency = 0;
		voice[i].volume = 0xf;
		voice[i].counter = 0;
		voice[i].key = 0;
	}

	/* other parameters */
	info->test = 0;
	for (i = 0; i < 5; i++)
		info->f[i] = 0;
}

/********************************************************************************/

WRITE8_DEVICE_HANDLER( k051649_waveform_w )
{
	k051649_state *info = get_safe_token(device);
	if (info->test & 0x40)
		return;

	info->stream->update();

    if (offset >= 0x60)
    {
		if (info->test & 0x80)
			return;

		/* SY 20001114: Channel 5 shares the waveform with channel 4 */
		info->channel_list[3].waveform[offset&0x1f]=data;
		info->channel_list[4].waveform[offset&0x1f]=data;
	}
	else
		info->channel_list[offset>>5].waveform[offset&0x1f]=data;
}

READ8_DEVICE_HANDLER ( k051649_waveform_r )
{
	k051649_state *info = get_safe_token(device);

	if (offset >= 0x60)
	{
		if (info->test & 0xc0)
		{
			info->stream->update();
			offset += (info->channel_list[3 + (info->test >> 6 & 1)].counter >> FREQBASEBITS);
		}
	}
	else
	{
		if (info->test & 0x40)
		{
			info->stream->update();
			offset += (info->channel_list[offset>>5].counter >> FREQBASEBITS);
		}
	}
	return info->channel_list[offset>>5].waveform[offset&0x1f];
}

WRITE8_DEVICE_HANDLER( k052539_waveform_w )
{
	k051649_state *info = get_safe_token(device);
	if (info->test & 0x40)
		return;

	info->stream->update();

	/* SY 20001114: Channel 5 doesn't share the waveform with channel 4 on this chip */
	info->channel_list[offset>>5].waveform[offset&0x1f]=data;
}

READ8_DEVICE_HANDLER ( k052539_waveform_r )
{
	k051649_state *info = get_safe_token(device);

	if (info->test & 0x40)
	{
		info->stream->update();
		offset += (info->channel_list[offset>>5].counter >> FREQBASEBITS);
	}
	return info->channel_list[offset>>5].waveform[offset&0x1f];
}

WRITE8_DEVICE_HANDLER( k051649_volume_w )
{
	k051649_state *info = get_safe_token(device);
	info->stream->update();
	info->channel_list[offset&0x7].volume=data&0xf;
}

WRITE8_DEVICE_HANDLER( k051649_frequency_w )
{
	k051649_state *info = get_safe_token(device);
	info->f[offset]=data;

	info->stream->update();

	if (info->test & 0x20)
		info->channel_list[offset>>1].counter = ~0;
	else if (info->channel_list[offset>>1].frequency < 9)
		info->channel_list[offset>>1].counter |= ((1 << FREQBASEBITS) - 1);

	info->channel_list[offset>>1].frequency=(info->f[offset&0xe] + (info->f[offset|1]<<8))&0xfff;
}

WRITE8_DEVICE_HANDLER( k051649_keyonoff_w )
{
	int i;
	k051649_state *info = get_safe_token(device);
	info->stream->update();

	for (i = 0; i < 5; i++) {
		info->channel_list[i].key=data&1;
		data >>= 1;
	}
}

WRITE8_DEVICE_HANDLER( k051649_test_w )
{
	k051649_state *info = get_safe_token(device);
	info->test = data;
}

READ8_DEVICE_HANDLER ( k051649_test_r )
{
	k051649_state *info = get_safe_token(device);

	/* reading the test register sets it to $ff! */
	info->test = 0xff;
	return 0xff;
}




/**************************************************************************
 * Generic get_info
 **************************************************************************/

DEVICE_GET_INFO( k051649 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:					info->i = sizeof(k051649_state);				break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME( k051649 );		break;
		case DEVINFO_FCT_STOP:							/* nothing */									break;
		case DEVINFO_FCT_RESET:							info->reset = DEVICE_RESET_NAME( k051649 );		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "K051649");						break;
		case DEVINFO_STR_FAMILY:						strcpy(info->s, "Konami custom");				break;
		case DEVINFO_STR_VERSION:						strcpy(info->s, "1.0");							break;
		case DEVINFO_STR_SOURCE_FILE:					strcpy(info->s, __FILE__);						break;
		case DEVINFO_STR_CREDITS:						strcpy(info->s, "Copyright Nicola Salmoria and the MAME Team"); break;
	}
}


DEFINE_LEGACY_SOUND_DEVICE(K051649, k051649);
