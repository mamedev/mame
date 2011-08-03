/***************************************************************************

    Flower sound driver (quick hack of the Wiping sound driver)

***************************************************************************/

#include "emu.h"
#include "includes/flower.h"

#define FLOWER_VERBOSE		0		// show register writes

#define MIXER_SAMPLERATE	48000	/* ? */
#define MIXER_DEFGAIN		48


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
	sound_channel m_channel_list[8];
	sound_channel *m_last_channel;

	/* global sound parameters */
	const UINT8 *m_sample_rom;
	const UINT8 *m_volume_rom;
	sound_stream * m_stream;

	/* mixer tables and internal buffers */
	INT16 *m_mixer_table;
	INT16 *m_mixer_lookup;
	short *m_mixer_buffer;

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
			const UINT8 *w = &state->m_sample_rom[voice->rom_offset];
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
							*mix++ += state->m_volume_rom[v*256 + w[offs]] - 0x80;
						}
					}
				}
				else
				{
					offs = (c >> 15) & 0x1ff;

					*mix++ += state->m_volume_rom[v*256 + w[offs]] - 0x80;
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

	state->m_stream = device->machine().sound().stream_alloc(*device, 0, 1, MIXER_SAMPLERATE, 0, flower_update_mono);
	state->m_mixer_buffer = auto_alloc_array(device->machine(), short, MIXER_SAMPLERATE);
	make_mixer_table(device, 8, MIXER_DEFGAIN);

	/* extract globals from the interface */
	state->m_last_channel = state->m_channel_list + 8;

	state->m_sample_rom = machine.region("sound1")->base();
	state->m_volume_rom = machine.region("sound2")->base();

	/* register for savestates */
	for (i = 0; i < 8; i++)
	{
		voice = &state->m_channel_list[i];

		device->save_item(NAME(voice->frequency), i+1);
		device->save_item(NAME(voice->counter), i+1);
		device->save_item(NAME(voice->volume), i+1);
		device->save_item(NAME(voice->oneshot), i+1);
		device->save_item(NAME(voice->oneshotplaying), i+1);
		device->save_item(NAME(voice->rom_offset), i+1);
	}
}

static DEVICE_RESET( flower_sound )
{
	flower_sound_state *state = get_safe_token(device);
	sound_channel *voice;
	int i;

	/* reset all the voices */
	for (i = 0; i < 8; i++)
	{
		voice = &state->m_channel_list[i];

		voice->frequency = 0;
		voice->counter = 0;
		voice->volume = 0;
		voice->oneshot = 1;
		voice->oneshotplaying = 0;
		voice->rom_offset = 0;
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
		case DEVINFO_FCT_RESET:							info->start = DEVICE_RESET_NAME(flower_sound);	break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "Flower Custom");				break;
		case DEVINFO_STR_SOURCE_FILE:					strcpy(info->s, __FILE__);						break;
	}
}


/********************************************************************************/

#if FLOWER_VERBOSE
static void show_soundregs(device_t *device)
{
	flower_sound_state *state = get_safe_token(device);
	int set,reg,chan;
	char text[0x100];
	char message[0x1000] = {0};
	UINT8 *sregs = state->m_soundregs1;

	for (set=0;set<2;set++)
	{
		for (reg=0;reg<8;reg++)
		{
			sprintf(text,"R%d%d:",set+1,reg);
			strcat(message,text);
		
			for (chan=0;chan<8;chan++)
			{
				sprintf(text," %02X",sregs[reg + 8*chan]);
				strcat(message,text);
			}
			strcat(message,"\n");
		}
		strcat(message,"\n");
		sregs = state->m_soundregs2;
	}
	popmessage("%s",message);
}
#endif // FLOWER_VERBOSE


/* register functions (preliminary):
offset: cccrrr		c=channel, r=register

set 1:
R  76543210
0  xxxxxxxx			frequency (which nibble?)
1  xxxxxxxx			*
2  xxxxxxxx			*
3  xxxxxxxx			*
4  ...x....			one-shot sample
5  ...x....			??? same as R4?
6  ........			unused?
7  xxxx....			volume

set 2:
R  76543210
0  ....xxxx			start address?
1  ....xxxx			start address?
2  ....xxxx			start address
3  ....xxxx			start address
4  xxxx    			??? effect? (volume/pitch slide) -- these bits are used by the stuck notes
       xxxx			start address
5  x...    			??? loop/one-shot related?
       xxxx			start address
6  ........			unused?
7  ......xx			volume

*/

WRITE8_DEVICE_HANDLER( flower_sound1_w )
{
	flower_sound_state *state = get_safe_token(device);
	sound_channel *voice;
	int base;

	state->m_stream->update();
	state->m_soundregs1[offset] = data;
#if FLOWER_VERBOSE
	show_soundregs(device);
#endif

	/* recompute all the voice parameters */
	for (base = 0, voice = state->m_channel_list; voice < state->m_last_channel; voice++, base += 8)
	{
		voice->frequency = state->m_soundregs1[2 + base] & 0x0f;
		voice->frequency = voice->frequency * 16 + ((state->m_soundregs1[3 + base]) & 0x0f);
		voice->frequency = voice->frequency * 16 + ((state->m_soundregs1[0 + base]) & 0x0f);
		voice->frequency = voice->frequency * 16 + ((state->m_soundregs1[1 + base]) & 0x0f);

		voice->volume = (state->m_soundregs1[7 + base] >> 4) | ((state->m_soundregs2[7 + base] & 0x03) << 4);

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

	state->m_stream->update();
	state->m_soundregs2[offset] = data;
#if FLOWER_VERBOSE
	show_soundregs(device);
#endif

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
