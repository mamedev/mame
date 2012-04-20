/***************************************************************************

    Wiping sound driver (quick hack of the Namco sound driver)

    used by wiping.c and clshroad.c

***************************************************************************/

#include "emu.h"
#include "audio/wiping.h"


/* 8 voices max */
#define MAX_VOICES 8


static const int samplerate = 48000;
static const int defgain = 48;


/* this structure defines the parameters for a channel */
typedef struct
{
	int frequency;
	int counter;
	int volume;
	const UINT8 *wave;
	int oneshot;
	int oneshotplaying;
} sound_channel;




typedef struct _wiping_sound_state wiping_sound_state;
struct _wiping_sound_state
{
	/* data about the sound system */
	sound_channel m_channel_list[MAX_VOICES];
	sound_channel *m_last_channel;

	/* global sound parameters */
	const UINT8 *m_sound_prom;
	const UINT8 *m_sound_rom;
	int m_num_voices;
	int m_sound_enable;
	sound_stream *m_stream;

	/* mixer tables and internal buffers */
	INT16 *m_mixer_table;
	INT16 *m_mixer_lookup;
	short *m_mixer_buffer;
	short *m_mixer_buffer_2;

	UINT8 m_soundregs[0x4000];
};


INLINE wiping_sound_state *get_safe_token( device_t *device )
{
	assert(device != NULL);
	assert(device->type() == WIPING);

	return (wiping_sound_state *)downcast<legacy_device_base *>(device)->token();
}

/* build a table to divide by the number of voices; gain is specified as gain*16 */
static void make_mixer_table(device_t *device, int voices, int gain)
{
	wiping_sound_state *state = get_safe_token(device);
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
static STREAM_UPDATE( wiping_update_mono )
{
	wiping_sound_state *state = get_safe_token(device);
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
		int f = 16*voice->frequency;
		int v = voice->volume;

		/* only update if we have non-zero volume and frequency */
		if (v && f)
		{
			const UINT8 *w = voice->wave;
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
						if (w[offs>>1] == 0xff)
						{
							voice->oneshotplaying = 0;
						}

						else
						{
							/* use full byte, first the high 4 bits, then the low 4 bits */
							if (offs & 1)
								*mix++ += ((w[offs>>1] & 0x0f) - 8) * v;
							else
								*mix++ += (((w[offs>>1]>>4) & 0x0f) - 8) * v;
						}
					}
				}
				else
				{
					offs = (c >> 15) & 0x1f;

					/* use full byte, first the high 4 bits, then the low 4 bits */
					if (offs & 1)
						*mix++ += ((w[offs>>1] & 0x0f) - 8) * v;
					else
						*mix++ += (((w[offs>>1]>>4) & 0x0f) - 8) * v;
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



static DEVICE_START( wiping_sound )
{
	wiping_sound_state *state = get_safe_token(device);
	running_machine &machine = device->machine();
	sound_channel *voice;

	/* get stream channels */
	state->m_stream = device->machine().sound().stream_alloc(*device, 0, 1, samplerate, NULL, wiping_update_mono);

	/* allocate a pair of buffers to mix into - 1 second's worth should be more than enough */
	state->m_mixer_buffer = auto_alloc_array(machine, short, 2 * samplerate);
	state->m_mixer_buffer_2 = state->m_mixer_buffer + samplerate;

	/* build the mixer table */
	make_mixer_table(device, 8, defgain);

	/* extract globals from the interface */
	state->m_num_voices = 8;
	state->m_last_channel = state->m_channel_list + state->m_num_voices;

	state->m_sound_rom = machine.root_device().memregion("samples")->base();
	state->m_sound_prom = machine.root_device().memregion("soundproms")->base();

	/* start with sound enabled, many games don't have a sound enable register */
	state->m_sound_enable = 1;

	/* reset all the voices */
	for (voice = state->m_channel_list; voice < state->m_last_channel; voice++)
	{
		voice->frequency = 0;
		voice->volume = 0;
		voice->wave = &state->m_sound_prom[0];
		voice->counter = 0;
	}
}


DEVICE_GET_INFO( wiping_sound )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:					info->i = sizeof(wiping_sound_state);			break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME(wiping_sound);	break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "Wiping Custom");				break;
		case DEVINFO_STR_SOURCE_FILE:					strcpy(info->s, __FILE__);						break;
	}
}



/********************************************************************************/

WRITE8_DEVICE_HANDLER( wiping_sound_w )
{
	wiping_sound_state *state = get_safe_token(device);
	sound_channel *voice;
	int base;

	/* update the streams */
	state->m_stream->update();

	/* set the register */
	state->m_soundregs[offset] = data;

	/* recompute all the voice parameters */
	if (offset <= 0x3f)
	{
		for (base = 0, voice = state->m_channel_list; voice < state->m_last_channel; voice++, base += 8)
		{
			voice->frequency = state->m_soundregs[0x02 + base] & 0x0f;
			voice->frequency = voice->frequency * 16 + ((state->m_soundregs[0x01 + base]) & 0x0f);
			voice->frequency = voice->frequency * 16 + ((state->m_soundregs[0x00 + base]) & 0x0f);

			voice->volume = state->m_soundregs[0x07 + base] & 0x0f;
			if (state->m_soundregs[0x5 + base] & 0x0f)
			{
				voice->wave = &state->m_sound_rom[128 * (16 * (state->m_soundregs[0x5 + base] & 0x0f)
						+ (state->m_soundregs[0x2005 + base] & 0x0f))];
				voice->oneshot = 1;
			}
			else
			{
				voice->wave = &state->m_sound_rom[16 * (state->m_soundregs[0x3 + base] & 0x0f)];
				voice->oneshot = 0;
				voice->oneshotplaying = 0;
			}
		}
	}
	else if (offset >= 0x2000)
	{
		voice = &state->m_channel_list[(offset & 0x3f)/8];
		if (voice->oneshot)
		{
			voice->counter = 0;
			voice->oneshotplaying = 1;
		}
	}
}


DEFINE_LEGACY_SOUND_DEVICE(WIPING, wiping_sound);
