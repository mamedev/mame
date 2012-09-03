/***************************************************************************

    Gomoku sound driver (quick hack of the Wiping sound driver)

    used by wiping.c

***************************************************************************/

#include "emu.h"
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


typedef struct _gomoku_sound_state gomoku_sound_state;
struct _gomoku_sound_state
{
	/* data about the sound system */
	sound_channel m_channel_list[MAX_VOICES];
	sound_channel *m_last_channel;

	/* global sound parameters */
	const UINT8 *m_sound_rom;
	int m_num_voices;
	int m_sound_enable;
	sound_stream *m_stream;

	/* mixer tables and internal buffers */
	INT16 *m_mixer_table;
	INT16 *m_mixer_lookup;
	short *m_mixer_buffer;
	short *m_mixer_buffer_2;

	UINT8 m_soundregs1[0x20];
	UINT8 m_soundregs2[0x20];
};

INLINE gomoku_sound_state *get_safe_token( device_t *device )
{
	assert(device != NULL);
	assert(device->type() == GOMOKU);

	return (gomoku_sound_state *)downcast<gomoku_sound_device *>(device)->token();
}


/* build a table to divide by the number of voices; gain is specified as gain*16 */
static void make_mixer_table(device_t *device, int voices, int gain)
{
	gomoku_sound_state *state = get_safe_token(device);
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
static STREAM_UPDATE( gomoku_update_mono )
{
	gomoku_sound_state *state = get_safe_token(device);
	stream_sample_t *buffer = outputs[0];
	sound_channel *voice;
	short *mix;
	int i, ch;

	/* if no sound, we're done */
	if (state->m_sound_enable == 0)
	{
		memset(buffer, 0, samples * sizeof(*buffer));
		return;
	}

	/* zap the contents of the mixer buffer */
	memset(state->m_mixer_buffer, 0, samples * sizeof(short));

	/* loop over each voice and add its contribution */
	for (ch = 0, voice = state->m_channel_list; voice < state->m_last_channel; ch++, voice++)
	{
		int f = 16 * voice->frequency;
		int v = voice->volume;

		/* only update if we have non-zero volume and frequency */
		if (v && f)
		{
			int w_base;
			int c = voice->counter;

			if (ch < 3)
				w_base = 0x20 * (state->m_soundregs1[0x06 + (ch * 8)] & 0x0f);
			else
				w_base = 0x100 * (state->m_soundregs2[0x1d] & 0x0f);

			mix = state->m_mixer_buffer;

			/* add our contribution */
			for (i = 0; i < samples; i++)
			{
				c += f;

				if (ch < 3)
				{
					int offs = w_base | ((c >> 16) & 0x1f);

					/* use full byte, first the high 4 bits, then the low 4 bits */
					if (c & 0x8000)
						*mix++ += ((state->m_sound_rom[offs] & 0x0f) - 8) * v;
					else
						*mix++ += (((state->m_sound_rom[offs]>>4) & 0x0f) - 8) * v;
				}
				else
				{
					int offs = (w_base + (c >> 16)) & 0x0fff;

					if (state->m_sound_rom[offs] == 0xff)
					{
						voice->oneshotplaying = 0;
					}

					if (voice->oneshotplaying)
					{
						/* use full byte, first the high 4 bits, then the low 4 bits */
						if (c & 0x8000)
							*mix++ += ((state->m_sound_rom[offs] & 0x0f) - 8) * v;
						else
							*mix++ += (((state->m_sound_rom[offs]>>4) & 0x0f) - 8) * v;
					}
				}

				/* update the counter for this voice */
				voice->counter = c;
			}
		}
	}

	/* mix it down */
	mix = state->m_mixer_buffer;
	for (i = 0; i < samples; i++)
		*buffer++ = state->m_mixer_lookup[*mix++];
}



static DEVICE_START( gomoku_sound )
{
	gomoku_sound_state *state = get_safe_token(device);
	running_machine &machine = device->machine();
	sound_channel *voice;
	int ch;

	/* get stream channels */
	state->m_stream = device->machine().sound().stream_alloc(*device, 0, 1, samplerate, NULL, gomoku_update_mono);

	/* allocate a pair of buffers to mix into - 1 second's worth should be more than enough */
	state->m_mixer_buffer = auto_alloc_array(machine, short, 2 * samplerate);
	state->m_mixer_buffer_2 = state->m_mixer_buffer + samplerate;

	/* build the mixer table */
	make_mixer_table(device, 8, defgain);

	/* extract globals from the interface */
	state->m_num_voices = MAX_VOICES;
	state->m_last_channel = state->m_channel_list + state->m_num_voices;

	state->m_sound_rom = machine.root_device().memregion("gomoku")->base();

	/* start with sound enabled, many games don't have a sound enable register */
	state->m_sound_enable = 1;

	/* reset all the voices */
	for (ch = 0, voice = state->m_channel_list; voice < state->m_last_channel; ch++, voice++)
	{
		voice->channel = ch;
		voice->frequency = 0;
		voice->counter = 0;
		voice->volume = 0;
		voice->oneshotplaying = 0;
	}
}


/********************************************************************************/

WRITE8_DEVICE_HANDLER( gomoku_sound1_w )
{
	gomoku_sound_state *state = get_safe_token(device);
	sound_channel *voice;
	int base;
	int ch;

	/* update the streams */
	state->m_stream->update();

	/* set the register */
	state->m_soundregs1[offset] = data;

	/* recompute all the voice parameters */
	for (ch = 0, base = 0, voice = state->m_channel_list; voice < state->m_channel_list + 3; ch++, voice++, base += 8)
	{
		voice->channel = ch;
		voice->frequency = state->m_soundregs1[0x02 + base] & 0x0f;
		voice->frequency = voice->frequency * 16 + ((state->m_soundregs1[0x01 + base]) & 0x0f);
		voice->frequency = voice->frequency * 16 + ((state->m_soundregs1[0x00 + base]) & 0x0f);
	}
}

WRITE8_DEVICE_HANDLER( gomoku_sound2_w )
{
	gomoku_sound_state *state = get_safe_token(device);
	sound_channel *voice;
	int base;
	int ch;

	/* update the streams */
	state->m_stream->update();

	/* set the register */
	state->m_soundregs2[offset] = data;

	/* recompute all the voice parameters */
	for (ch = 0, base = 0, voice = state->m_channel_list; voice < state->m_channel_list + 3; ch++, voice++, base += 8)
	{
		voice->channel = ch;
		voice->volume = state->m_soundregs2[0x06 + base] & 0x0f;
		voice->oneshotplaying = 0;
	}

	if (offset == 0x1d)
	{
		voice = &state->m_channel_list[3];
		voice->channel = 3;

		// oneshot frequency is hand tune...
		if ((state->m_soundregs2[0x1d] & 0x0f) < 0x0c)
			voice->frequency = 3000 / 16;			// ichi, ni, san, yon, go
		else
			voice->frequency = 8000 / 16;			// shoot

		voice->volume = 8;
		voice->counter = 0;

		if (state->m_soundregs2[0x1d] & 0x0f)
			voice->oneshotplaying = 1;
		else
			voice->oneshotplaying = 0;
	}
}


const device_type GOMOKU = &device_creator<gomoku_sound_device>;

gomoku_sound_device::gomoku_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, GOMOKU, "Gomoku Custom", tag, owner, clock),
	  device_sound_interface(mconfig, *this)
{
	m_token = global_alloc_array_clear(UINT8, sizeof(gomoku_sound_state));
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void gomoku_sound_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void gomoku_sound_device::device_start()
{
	DEVICE_START_NAME( gomoku_sound )(this);
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void gomoku_sound_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// should never get here
	fatalerror("sound_stream_update called; not applicable to legacy sound devices\n");
}


