/***************************************************************************

    Flower sound driver (quick hack of the Wiping sound driver)

***************************************************************************/

#include "emu.h"
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
	sound_channel m_channel_list[MAX_VOICES];
	sound_channel *m_last_channel;

	/* global sound parameters */
	const UINT8 *m_sound_rom1;
	const UINT8 *m_sound_rom2;
	UINT8 m_num_voices;
	UINT8 m_sound_enable;
	sound_stream * m_stream;

	/* mixer tables and internal buffers */
	INT16 *m_mixer_table;
	INT16 *m_mixer_lookup;
	short *m_mixer_buffer;
	short *m_mixer_buffer_2;

	UINT8 m_soundregs1[0x40];
	UINT8 m_soundregs2[0x40];
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
	state->m_mixer_table = auto_alloc_array(device->machine(), INT16, 256 * voices);

	/* find the middle of the table */
	state->m_mixer_lookup = state->m_mixer_table + (128 * voices);

	/* fill in the table - 16 bit case */
	for (i = 0; i < count; i++)
	{
		int val = i * gain * 16 / voices;
		if (val > 32767) val = 32767;
		state->m_mixer_lookup[ i] = val;
		state->m_mixer_lookup[-i] = -val;
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
	if (state->m_sound_enable == 0)
	{
		memset(buffer, 0, samples * sizeof(*buffer));
		return;
	}

	/* zap the contents of the mixer buffer */
	memset(state->m_mixer_buffer, 0, samples * sizeof(short));

	/* loop over each voice and add its contribution */
	for (voice = state->m_channel_list; voice < state->m_last_channel; voice++)
	{
		int f = 256*voice->frequency;
		int v = voice->volume;

		/* only update if we have non-zero volume and frequency */
		if (v && f)
		{
			const UINT8 *w = &state->m_sound_rom1[voice->rom_offset];
			int c = voice->counter;

			mix = state->m_mixer_buffer;

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
							*mix++ += state->m_sound_rom2[v*256 + w[offs]] - 0x80;
						}
					}
				}
				else
				{
					offs = (c >> 15) & 0x1ff;

//                  *mix++ += ((w[offs] - 0x80) * v) / 16;
					*mix++ += state->m_sound_rom2[v*256 + w[offs]] - 0x80;
				}
			}

			/* update the counter for this voice */
			voice->counter = c;
		}
	}

	/* mix it down */
	mix = state->m_mixer_buffer;
	for (i = 0; i < samples; i++)
		*buffer++ = state->m_mixer_lookup[*mix++];
}



static DEVICE_START( flower_sound )
{
	flower_sound_state *state = get_safe_token(device);
	running_machine &machine = device->machine();
	sound_channel *voice;
	int i;

	/* get stream channels */
	state->m_stream = device->machine().sound().stream_alloc(*device, 0, 1, samplerate, 0, flower_update_mono);

	/* allocate a pair of buffers to mix into - 1 second's worth should be more than enough */
	state->m_mixer_buffer = auto_alloc_array(device->machine(), short, 2 * samplerate);
	state->m_mixer_buffer_2 = state->m_mixer_buffer + samplerate;

	/* build the mixer table */
	make_mixer_table(device, 8, defgain);

	/* extract globals from the interface */
	state->m_num_voices = 8;
	state->m_last_channel = state->m_channel_list + state->m_num_voices;

	state->m_sound_rom1 = machine.region("sound1")->base();
	state->m_sound_rom2 = machine.region("sound2")->base();

	/* start with sound enabled, many games don't have a sound enable register */
	state->m_sound_enable = 1;

	/* save globals */
	device->save_item(NAME(state->m_num_voices));
	device->save_item(NAME(state->m_sound_enable));

	/* reset all the voices */
	for (i = 0; i < state->m_num_voices; i++)
	{
		voice = &state->m_channel_list[i];

		voice->frequency = 0;
		voice->volume = 0;
		voice->counter = 0;
		voice->rom_offset = 0;

		device->save_item(NAME(voice->frequency), i+1);
		device->save_item(NAME(voice->counter), i+1);
		device->save_item(NAME(voice->volume), i+1);
		device->save_item(NAME(voice->oneshot), i+1);
		device->save_item(NAME(voice->oneshotplaying), i+1);
		device->save_item(NAME(voice->rom_offset), i+1);
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
	state->m_stream->update();

	/* set the register */
	state->m_soundregs1[offset] = data;

	/* recompute all the voice parameters */
	for (base = 0, voice = state->m_channel_list; voice < state->m_last_channel; voice++, base += 8)
	{
		voice->frequency = state->m_soundregs1[2 + base] & 0x0f;
		voice->frequency = voice->frequency * 16 + ((state->m_soundregs1[3 + base]) & 0x0f);
		voice->frequency = voice->frequency * 16 + ((state->m_soundregs1[0 + base]) & 0x0f);
		voice->frequency = voice->frequency * 16 + ((state->m_soundregs1[1 + base]) & 0x0f);

		voice->volume = (state->m_soundregs1[7 + base] >> 4) | ((state->m_soundregs2[7 + base] & 0x03) << 4);
// the following would fix the hanging notes...
//if ((state->m_soundregs2[7 + base] & 0x01) == 0)
//  voice->volume = 0;

		if (state->m_soundregs1[4 + base] & 0x10)
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
        state->m_soundregs2[7 + 8*0],state->m_soundregs1[7 + 8*0],
        state->m_soundregs2[7 + 8*1],state->m_soundregs1[7 + 8*1],
        state->m_soundregs2[7 + 8*2],state->m_soundregs1[7 + 8*2],
        state->m_soundregs2[7 + 8*3],state->m_soundregs1[7 + 8*3],
        state->m_soundregs2[7 + 8*4],state->m_soundregs1[7 + 8*4],
        state->m_soundregs2[7 + 8*5],state->m_soundregs1[7 + 8*5],
        state->m_soundregs2[7 + 8*6],state->m_soundregs1[7 + 8*6],
        state->m_soundregs2[7 + 8*7],state->m_soundregs1[7 + 8*7]
    );
*/

	/* update the streams */
	state->m_stream->update();

	/* set the register */
	state->m_soundregs2[offset] = data;

	/* recompute all the voice parameters */
	voice = &state->m_channel_list[offset/8];
	if (voice->oneshot)
	{
		int start;

		start = state->m_soundregs2[5 + base] & 0x0f;
		start = start * 16 + ((state->m_soundregs2[4 + base]) & 0x0f);
		start = start * 16 + ((state->m_soundregs2[3 + base]) & 0x0f);
		start = start * 16 + ((state->m_soundregs2[2 + base]) & 0x0f);
		start = start * 16 + ((state->m_soundregs2[1 + base]) & 0x0f);
		start = start * 16 + ((state->m_soundregs2[0 + base]) & 0x0f);

		voice->rom_offset = (start >> 7) & 0x7fff;

		voice->counter = 0;
		voice->oneshotplaying = 1;
	}
	else
	{
		int start;

		start = state->m_soundregs2[5 + base] & 0x0f;
		start = start * 16 + ((state->m_soundregs2[4 + base]) & 0x0f);

		voice->rom_offset = (start << 9) & 0x7fff;	// ???
		voice->oneshot = 0;
		voice->oneshotplaying = 0;
	}
}


DEFINE_LEGACY_SOUND_DEVICE(FLOWER, flower_sound);
