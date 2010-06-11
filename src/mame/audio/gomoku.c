/***************************************************************************

    Gomoku sound driver (quick hack of the Wiping sound driver)

    used by wiping.c

***************************************************************************/

#include "emu.h"
#include "streams.h"
#include "includes/gomoku.h"


/* 4 voices max */
#define MAX_VOICES 4


static const int samplerate = 48000;
static const int defgain = 48;


/* this structure defines the parameters for a channel */
typedef struct
{
	int channel;
	int frequency;
	int counter;
	int volume;
	int oneshotplaying;
} sound_channel;


/* globals available to everyone */
UINT8 *gomoku_soundregs1;
UINT8 *gomoku_soundregs2;

/* data about the sound system */
static sound_channel channel_list[MAX_VOICES];
static sound_channel *last_channel;

/* global sound parameters */
static const UINT8 *sound_rom;
static int num_voices;
static int sound_enable;
static sound_stream *stream;

/* mixer tables and internal buffers */
static INT16 *mixer_table;
static INT16 *mixer_lookup;
static short *mixer_buffer;
static short *mixer_buffer_2;



/* build a table to divide by the number of voices; gain is specified as gain*16 */
static void make_mixer_table(running_machine *machine, int voices, int gain)
{
	int count = voices * 128;
	int i;

	/* allocate memory */
	mixer_table = auto_alloc_array(machine, INT16, 256 * voices);

	/* find the middle of the table */
	mixer_lookup = mixer_table + (128 * voices);

	/* fill in the table - 16 bit case */
	for (i = 0; i < count; i++)
	{
		int val = i * gain * 16 / voices;
		if (val > 32767) val = 32767;
		mixer_lookup[ i] = val;
		mixer_lookup[-i] = -val;
	}
}


/* generate sound to the mix buffer in mono */
static STREAM_UPDATE( gomoku_update_mono )
{
	stream_sample_t *buffer = outputs[0];
	sound_channel *voice;
	short *mix;
	int i, ch;

	/* if no sound, we're done */
	if (sound_enable == 0)
	{
		memset(buffer, 0, samples * sizeof(*buffer));
		return;
	}

	/* zap the contents of the mixer buffer */
	memset(mixer_buffer, 0, samples * sizeof(short));

	/* loop over each voice and add its contribution */
	for (ch = 0, voice = channel_list; voice < last_channel; ch++, voice++)
	{
		int f = 16 * voice->frequency;
		int v = voice->volume;

		/* only update if we have non-zero volume and frequency */
		if (v && f)
		{
			int w_base;
			int c = voice->counter;

			if (ch < 3)
				w_base = 0x20 * (gomoku_soundregs1[0x06 + (ch * 8)] & 0x0f);
			else
				w_base = 0x100 * (gomoku_soundregs2[0x1d] & 0x0f);

			mix = mixer_buffer;

			/* add our contribution */
			for (i = 0; i < samples; i++)
			{
				c += f;

				if (ch < 3)
				{
					int offs = w_base | ((c >> 16) & 0x1f);

					/* use full byte, first the high 4 bits, then the low 4 bits */
					if (c & 0x8000)
						*mix++ += ((sound_rom[offs] & 0x0f) - 8) * v;
					else
						*mix++ += (((sound_rom[offs]>>4) & 0x0f) - 8) * v;
				}
				else
				{
					int offs = (w_base + (c >> 16)) & 0x0fff;

					if (sound_rom[offs] == 0xff)
					{
						voice->oneshotplaying = 0;
					}

					if (voice->oneshotplaying)
					{
						/* use full byte, first the high 4 bits, then the low 4 bits */
						if (c & 0x8000)
							*mix++ += ((sound_rom[offs] & 0x0f) - 8) * v;
						else
							*mix++ += (((sound_rom[offs]>>4) & 0x0f) - 8) * v;
					}
				}

				/* update the counter for this voice */
				voice->counter = c;
			}
		}
	}

	/* mix it down */
	mix = mixer_buffer;
	for (i = 0; i < samples; i++)
		*buffer++ = mixer_lookup[*mix++];
}



static DEVICE_START( gomoku_sound )
{
	running_machine *machine = device->machine;
	sound_channel *voice;
	int ch;

	/* get stream channels */
	stream = stream_create(device, 0, 1, samplerate, NULL, gomoku_update_mono);

	/* allocate a pair of buffers to mix into - 1 second's worth should be more than enough */
	mixer_buffer = auto_alloc_array(machine, short, 2 * samplerate);
	mixer_buffer_2 = mixer_buffer + samplerate;

	/* build the mixer table */
	make_mixer_table(machine, 8, defgain);

	/* extract globals from the interface */
	num_voices = MAX_VOICES;
	last_channel = channel_list + num_voices;

	sound_rom = memory_region(machine, "gomoku");

	/* start with sound enabled, many games don't have a sound enable register */
	sound_enable = 1;

	/* reset all the voices */
	for (ch = 0, voice = channel_list; voice < last_channel; ch++, voice++)
	{
		voice->channel = ch;
		voice->frequency = 0;
		voice->counter = 0;
		voice->volume = 0;
		voice->oneshotplaying = 0;
	}
}


DEVICE_GET_INFO( gomoku_sound )
{
	switch (state)
	{
		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME(gomoku_sound);	break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "Gomoku Custom");				break;
		case DEVINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);						break;
	}
}


/********************************************************************************/

WRITE8_HANDLER( gomoku_sound1_w )
{
	sound_channel *voice;
	int base;
	int ch;

	/* update the streams */
	stream_update(stream);

	/* set the register */
	gomoku_soundregs1[offset] = data;

	/* recompute all the voice parameters */
	for (ch = 0, base = 0, voice = channel_list; voice < channel_list + 3; ch++, voice++, base += 8)
	{
		voice->channel = ch;
		voice->frequency = gomoku_soundregs1[0x02 + base] & 0x0f;
		voice->frequency = voice->frequency * 16 + ((gomoku_soundregs1[0x01 + base]) & 0x0f);
		voice->frequency = voice->frequency * 16 + ((gomoku_soundregs1[0x00 + base]) & 0x0f);
	}
}

WRITE8_HANDLER( gomoku_sound2_w )
{
	sound_channel *voice;
	int base;
	int ch;

	/* update the streams */
	stream_update(stream);

	/* set the register */
	gomoku_soundregs2[offset] = data;

	/* recompute all the voice parameters */
	for (ch = 0, base = 0, voice = channel_list; voice < channel_list + 3; ch++, voice++, base += 8)
	{
		voice->channel = ch;
		voice->volume = gomoku_soundregs2[0x06 + base] & 0x0f;
		voice->oneshotplaying = 0;
	}

	if (offset == 0x1d)
	{
		voice = &channel_list[3];
		voice->channel = 3;

		// oneshot frequency is hand tune...
		if ((gomoku_soundregs2[0x1d] & 0x0f) < 0x0c)
			voice->frequency = 3000 / 16;			// ichi, ni, san, yon, go
		else
			voice->frequency = 8000 / 16;			// shoot

		voice->volume = 8;
		voice->counter = 0;

		if (gomoku_soundregs2[0x1d] & 0x0f)
			voice->oneshotplaying = 1;
		else
			voice->oneshotplaying = 0;
	}
}


DEFINE_LEGACY_SOUND_DEVICE(GOMOKU, gomoku_sound);
