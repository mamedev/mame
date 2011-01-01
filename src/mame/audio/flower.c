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


typedef struct _flower_sound_state flower_sound_state;
struct _flower_sound_state
{
	/* data about the sound system */
	sound_channel channel_list[MAX_VOICES];
	sound_channel *last_channel;

	/* global sound parameters */
	const UINT8 *sound_rom1, *sound_rom2;
	UINT8 num_voices;
	UINT8 sound_enable;
	sound_stream * stream;

	/* mixer tables and internal buffers */
	INT16 *mixer_table;
	INT16 *mixer_lookup;
	short *mixer_buffer;
	short *mixer_buffer_2;

	UINT8 soundregs1[0x40];
	UINT8 soundregs2[0x40];
};

INLINE flower_sound_state *get_safe_token( device_t *device )
{
	assert(device != NULL);
	assert(device->type() == FLOWER);

	return (flower_sound_state *)downcast<legacy_device_base *>(device)->token();
}

/* build a table to divide by the number of voices; gain is specified as gain*16 */
static void make_mixer_table(device_t *device, int voices, int gain)
{
	flower_sound_state *state = get_safe_token(device);
	int count = voices * 128;
	int i;

	/* allocate memory */
	state->mixer_table = auto_alloc_array(device->machine, INT16, 256 * voices);

	/* find the middle of the table */
	state->mixer_lookup = state->mixer_table + (128 * voices);

	/* fill in the table - 16 bit case */
	for (i = 0; i < count; i++)
	{
		int val = i * gain * 16 / voices;
		if (val > 32767) val = 32767;
		state->mixer_lookup[ i] = val;
		state->mixer_lookup[-i] = -val;
	}
}


/* generate sound to the mix buffer in mono */
static STREAM_UPDATE( flower_update_mono )
{
	flower_sound_state *state = get_safe_token(device);
	stream_sample_t *buffer = outputs[0];
	sound_channel *voice;
	short *mix;
	int i;

	/* if no sound, we're done */
	if (state->sound_enable == 0)
	{
		memset(buffer, 0, samples * sizeof(*buffer));
		return;
	}

	/* zap the contents of the mixer buffer */
	memset(state->mixer_buffer, 0, samples * sizeof(short));

	/* loop over each voice and add its contribution */
	for (voice = state->channel_list; voice < state->last_channel; voice++)
	{
		int f = 256*voice->frequency;
		int v = voice->volume;

		/* only update if we have non-zero volume and frequency */
		if (v && f)
		{
			const UINT8 *w = &state->sound_rom1[voice->rom_offset];
			int c = voice->counter;

			mix = state->mixer_buffer;

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

						else
						{
//                          *mix++ += ((w[offs] - 0x80) * v) / 16;
							*mix++ += state->sound_rom2[v*256 + w[offs]] - 0x80;
						}
					}
				}
				else
				{
					offs = (c >> 15) & 0x1ff;

//                  *mix++ += ((w[offs] - 0x80) * v) / 16;
					*mix++ += state->sound_rom2[v*256 + w[offs]] - 0x80;
				}
			}

			/* update the counter for this voice */
			voice->counter = c;
		}
	}

	/* mix it down */
	mix = state->mixer_buffer;
	for (i = 0; i < samples; i++)
		*buffer++ = state->mixer_lookup[*mix++];
}



static DEVICE_START( flower_sound )
{
	flower_sound_state *state = get_safe_token(device);
	running_machine *machine = device->machine;
	sound_channel *voice;
	int i;

	/* get stream channels */
	state->stream = stream_create(device, 0, 1, samplerate, 0, flower_update_mono);

	/* allocate a pair of buffers to mix into - 1 second's worth should be more than enough */
	state->mixer_buffer = auto_alloc_array(device->machine, short, 2 * samplerate);
	state->mixer_buffer_2 = state->mixer_buffer + samplerate;

	/* build the mixer table */
	make_mixer_table(device, 8, defgain);

	/* extract globals from the interface */
	state->num_voices = 8;
	state->last_channel = state->channel_list + state->num_voices;

	state->sound_rom1 = machine->region("sound1")->base();
	state->sound_rom2 = machine->region("sound2")->base();

	/* start with sound enabled, many games don't have a sound enable register */
	state->sound_enable = 1;

	/* save globals */
	state_save_register_device_item(device, 0, state->num_voices);
	state_save_register_device_item(device, 0, state->sound_enable);

	/* reset all the voices */
	for (i = 0; i < state->num_voices; i++)
	{
		voice = &state->channel_list[i];

		voice->frequency = 0;
		voice->volume = 0;
		voice->counter = 0;
		voice->rom_offset = 0;

		state_save_register_device_item(device, i+1, voice->frequency);
		state_save_register_device_item(device, i+1, voice->counter);
		state_save_register_device_item(device, i+1, voice->volume);
		state_save_register_device_item(device, i+1, voice->oneshot);
		state_save_register_device_item(device, i+1, voice->oneshotplaying);
		state_save_register_device_item(device, i+1, voice->rom_offset);
	}
}


DEVICE_GET_INFO( flower_sound )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:					info->i = sizeof(flower_sound_state);			break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME(flower_sound);	break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "Flower Custom");				break;
		case DEVINFO_STR_SOURCE_FILE:					strcpy(info->s, __FILE__);						break;
	}
}


/********************************************************************************/

WRITE8_DEVICE_HANDLER( flower_sound1_w )
{
	flower_sound_state *state = get_safe_token(device);
	sound_channel *voice;
	int base;

	/* update the streams */
	stream_update(state->stream);

	/* set the register */
	state->soundregs1[offset] = data;

	/* recompute all the voice parameters */
	for (base = 0, voice = state->channel_list; voice < state->last_channel; voice++, base += 8)
	{
		voice->frequency = state->soundregs1[2 + base] & 0x0f;
		voice->frequency = voice->frequency * 16 + ((state->soundregs1[3 + base]) & 0x0f);
		voice->frequency = voice->frequency * 16 + ((state->soundregs1[0 + base]) & 0x0f);
		voice->frequency = voice->frequency * 16 + ((state->soundregs1[1 + base]) & 0x0f);

		voice->volume = (state->soundregs1[7 + base] >> 4) | ((state->soundregs2[7 + base] & 0x03) << 4);
// the following would fix the hanging notes...
//if ((state->soundregs2[7 + base] & 0x01) == 0)
//  voice->volume = 0;

		if (state->soundregs1[4 + base] & 0x10)
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

WRITE8_DEVICE_HANDLER( flower_sound2_w )
{
	flower_sound_state *state = get_safe_token(device);
	sound_channel *voice;
	int base = offset & 0xf8;

/*
popmessage("%02x%02x %02x%02x %02x%02x %02x%02x %02x%02x %02x%02x %02x%02x %02x%02x",
        state->soundregs2[7 + 8*0],state->soundregs1[7 + 8*0],
        state->soundregs2[7 + 8*1],state->soundregs1[7 + 8*1],
        state->soundregs2[7 + 8*2],state->soundregs1[7 + 8*2],
        state->soundregs2[7 + 8*3],state->soundregs1[7 + 8*3],
        state->soundregs2[7 + 8*4],state->soundregs1[7 + 8*4],
        state->soundregs2[7 + 8*5],state->soundregs1[7 + 8*5],
        state->soundregs2[7 + 8*6],state->soundregs1[7 + 8*6],
        state->soundregs2[7 + 8*7],state->soundregs1[7 + 8*7]
    );
*/

	/* update the streams */
	stream_update(state->stream);

	/* set the register */
	state->soundregs2[offset] = data;

	/* recompute all the voice parameters */
	voice = &state->channel_list[offset/8];
	if (voice->oneshot)
	{
		int start;

		start = state->soundregs2[5 + base] & 0x0f;
		start = start * 16 + ((state->soundregs2[4 + base]) & 0x0f);
		start = start * 16 + ((state->soundregs2[3 + base]) & 0x0f);
		start = start * 16 + ((state->soundregs2[2 + base]) & 0x0f);
		start = start * 16 + ((state->soundregs2[1 + base]) & 0x0f);
		start = start * 16 + ((state->soundregs2[0 + base]) & 0x0f);

		voice->rom_offset = (start >> 7) & 0x7fff;

		voice->counter = 0;
		voice->oneshotplaying = 1;
	}
	else
	{
		int start;

		start = state->soundregs2[5 + base] & 0x0f;
		start = start * 16 + ((state->soundregs2[4 + base]) & 0x0f);

		voice->rom_offset = (start << 9) & 0x7fff;	// ???
		voice->oneshot = 0;
		voice->oneshotplaying = 0;
	}
}


DEFINE_LEGACY_SOUND_DEVICE(FLOWER, flower_sound);
