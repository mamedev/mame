/***************************************************************************

    Flower sound driver (quick hack of the Wiping sound driver)

***************************************************************************/

#include "emu.h"
#include "streams.h"
#include "includes/flower.h"


/* 8 voices max */
#define MAX_VOICES 8


static const int samplerate = 48000;
static const int defgain = 48;


/* this structure defines the parameters for a channel */
typedef struct
{
	UINT32 frequency;
	UINT32 counter;
	UINT16 volume;
	UINT8 oneshot;
	UINT8 oneshotplaying;
	UINT16 rom_offset;

} sound_channel;


/* globals available to everyone */
UINT8 *flower_soundregs1,*flower_soundregs2;

/* data about the sound system */
static sound_channel channel_list[MAX_VOICES];
static sound_channel *last_channel;

/* global sound parameters */
static const UINT8 *sound_rom1, *sound_rom2;
static UINT8 num_voices;
static UINT8 sound_enable;
static sound_stream * stream;

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
static STREAM_UPDATE( flower_update_mono )
{
	stream_sample_t *buffer = outputs[0];
	sound_channel *voice;
	short *mix;
	int i;

	/* if no sound, we're done */
	if (sound_enable == 0)
	{
		memset(buffer, 0, samples * sizeof(*buffer));
		return;
	}

	/* zap the contents of the mixer buffer */
	memset(mixer_buffer, 0, samples * sizeof(short));

	/* loop over each voice and add its contribution */
	for (voice = channel_list; voice < last_channel; voice++)
	{
		int f = 256*voice->frequency;
		int v = voice->volume;

		/* only update if we have non-zero volume and frequency */
		if (v && f)
		{
			const UINT8 *w = &sound_rom1[voice->rom_offset];
			int c = voice->counter;

			mix = mixer_buffer;

			/* add our contribution */
			for (i = 0; i < samples; i++)
			{
				int offs;

				c += f;

				if (voice->oneshot)
				{
					if (voice->oneshotplaying)
					{
						offs = (c >> 15);
						if (w[offs] == 0xff)
						{
							voice->oneshotplaying = 0;
						}

						if (voice->oneshotplaying)
						{
//                          *mix++ += ((w[offs] - 0x80) * v) / 16;
							*mix++ += sound_rom2[v*256 + w[offs]] - 0x80;
						}
					}
				}
				else
				{
					offs = (c >> 15) & 0x1ff;

//                  *mix++ += ((w[offs] - 0x80) * v) / 16;
					*mix++ += sound_rom2[v*256 + w[offs]] - 0x80;
				}
			}

			/* update the counter for this voice */
			voice->counter = c;
		}
	}

	/* mix it down */
	mix = mixer_buffer;
	for (i = 0; i < samples; i++)
		*buffer++ = mixer_lookup[*mix++];
}



static DEVICE_START( flower_sound )
{
	running_machine *machine = device->machine;
	sound_channel *voice;
	int i;

	/* get stream channels */
	stream = stream_create(device, 0, 1, samplerate, 0, flower_update_mono);

	/* allocate a pair of buffers to mix into - 1 second's worth should be more than enough */
	mixer_buffer = auto_alloc_array(device->machine, short, 2 * samplerate);
	mixer_buffer_2 = mixer_buffer + samplerate;

	/* build the mixer table */
	make_mixer_table(machine, 8, defgain);

	/* extract globals from the interface */
	num_voices = 8;
	last_channel = channel_list + num_voices;

	sound_rom1 = memory_region(machine, "sound1");
	sound_rom2 = memory_region(machine, "sound2");

	/* start with sound enabled, many games don't have a sound enable register */
	sound_enable = 1;

	/* save globals */
	state_save_register_item(machine, "flower_custom", NULL, 0, num_voices);
	state_save_register_item(machine, "flower_custom", NULL, 0, sound_enable);

	/* reset all the voices */
	for (i = 0; i < num_voices; i++)
	{
		voice = &channel_list[i];

		voice->frequency = 0;
		voice->volume = 0;
		voice->counter = 0;
		voice->rom_offset = 0;

		state_save_register_item(machine, "flower_custom", NULL, i+1, voice->frequency);
		state_save_register_item(machine, "flower_custom", NULL, i+1, voice->counter);
		state_save_register_item(machine, "flower_custom", NULL, i+1, voice->volume);
		state_save_register_item(machine, "flower_custom", NULL, i+1, voice->oneshot);
		state_save_register_item(machine, "flower_custom", NULL, i+1, voice->oneshotplaying);
		state_save_register_item(machine, "flower_custom", NULL, i+1, voice->rom_offset);
	}
}


DEVICE_GET_INFO( flower_sound )
{
	switch (state)
	{
		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME(flower_sound);	break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "Flower Custom");				break;
		case DEVINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);						break;
	}
}


/********************************************************************************/

WRITE8_HANDLER( flower_sound1_w )
{
	sound_channel *voice;
	int base;

	/* update the streams */
	stream_update(stream);

	/* set the register */
	flower_soundregs1[offset] = data;

	/* recompute all the voice parameters */
	for (base = 0, voice = channel_list; voice < last_channel; voice++, base += 8)
	{
		voice->frequency = flower_soundregs1[2 + base] & 0x0f;
		voice->frequency = voice->frequency * 16 + ((flower_soundregs1[3 + base]) & 0x0f);
		voice->frequency = voice->frequency * 16 + ((flower_soundregs1[0 + base]) & 0x0f);
		voice->frequency = voice->frequency * 16 + ((flower_soundregs1[1 + base]) & 0x0f);

		voice->volume = (flower_soundregs1[7 + base] >> 4) | ((flower_soundregs2[7 + base] & 0x03) << 4);
// the following would fix the hanging notes...
//if ((flower_soundregs2[7 + base] & 0x01) == 0)
//  voice->volume = 0;

		if (flower_soundregs1[4 + base] & 0x10)
		{
			voice->oneshot = 0;
			voice->oneshotplaying = 0;
		}
		else
		{
			voice->oneshot = 1;
		}
	}
}

WRITE8_HANDLER( flower_sound2_w )
{
	sound_channel *voice;
	int base = offset & 0xf8;

/*
popmessage("%02x%02x %02x%02x %02x%02x %02x%02x %02x%02x %02x%02x %02x%02x %02x%02x",
        flower_soundregs2[7 + 8*0],flower_soundregs1[7 + 8*0],
        flower_soundregs2[7 + 8*1],flower_soundregs1[7 + 8*1],
        flower_soundregs2[7 + 8*2],flower_soundregs1[7 + 8*2],
        flower_soundregs2[7 + 8*3],flower_soundregs1[7 + 8*3],
        flower_soundregs2[7 + 8*4],flower_soundregs1[7 + 8*4],
        flower_soundregs2[7 + 8*5],flower_soundregs1[7 + 8*5],
        flower_soundregs2[7 + 8*6],flower_soundregs1[7 + 8*6],
        flower_soundregs2[7 + 8*7],flower_soundregs1[7 + 8*7]
    );
*/

	/* update the streams */
	stream_update(stream);

	/* set the register */
	flower_soundregs2[offset] = data;

	/* recompute all the voice parameters */
	voice = &channel_list[offset/8];
	if (voice->oneshot)
	{
		int start;

		start = flower_soundregs2[5 + base] & 0x0f;
		start = start * 16 + ((flower_soundregs2[4 + base]) & 0x0f);
		start = start * 16 + ((flower_soundregs2[3 + base]) & 0x0f);
		start = start * 16 + ((flower_soundregs2[2 + base]) & 0x0f);
		start = start * 16 + ((flower_soundregs2[1 + base]) & 0x0f);
		start = start * 16 + ((flower_soundregs2[0 + base]) & 0x0f);

		voice->rom_offset = (start >> 7) & 0x7fff;

		voice->counter = 0;
		voice->oneshotplaying = 1;
	}
	else
	{
		int start;

		start = flower_soundregs2[5 + base] & 0x0f;
		start = start * 16 + ((flower_soundregs2[4 + base]) & 0x0f);

		voice->rom_offset = (start << 9) & 0x7fff;	// ???
		voice->oneshot = 0;
		voice->oneshotplaying = 0;
	}
}
