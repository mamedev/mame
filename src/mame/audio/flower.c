/* Clarue Flower sound driver.
Initial version was based on the Wiping sound driver, which was based on the old namco.c sound driver.

TODO:
- timing (see main driver file), but also of samplerate and effects counter
- what do the unknown bits in soundregs do?
- Are channel effects correct? It's currently mostly guesswork, the pitch effects sound pretty convincing though.
  Considering that the game sound hardware isn't complicated (no dedicated soundchip) these bits are possibly
  for something way simpler, such as a length counter. PCB sound recordings would be useful!

*/

#include "emu.h"
#include "includes/flower.h"

#define FLOWER_VERBOSE		0		// show register writes

#define MIXER_SAMPLERATE	48000	/* ? (native freq is probably in the MHz range) */
#define MIXER_DEFGAIN		48


/* this structure defines the parameters for a channel */
struct sound_channel 
{
	UINT32 start;
	UINT32 pos;
	UINT16 freq;
	UINT8 volume;
	UINT8 voltab;
	UINT8 oneshot;
	UINT8 active;
	UINT8 effect;
	UINT32 ecount;

};


struct flower_sound_state
{
	emu_timer *m_effect_timer;

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

	return (flower_sound_state *)downcast<flower_sound_device *>(device)->token();
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
		state->m_mixer_lookup[-i] =-val;
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
		int f = voice->freq;
		int v = voice->volume;

		if (!voice->active)
			continue;

		// effects
		// bit 0: volume slide down?
		if (voice->effect & 1 && !voice->oneshot)
		{
			// note: one-shot samples are fixed volume
			v -= (voice->ecount >> 4);
			if (v < 0) v = 0;
		}
		// bit 1: used often, but hard to figure out what for
		// bit 2: probably pitch slide
		if (voice->effect & 4)
		{
			f -= (voice->ecount << 7);
			if (f < 0) f = 0;
		}
		// bit 3: not used much, maybe pitch slide the other way?

		v |= voice->voltab;
		mix = state->m_mixer_buffer;

		for (i = 0; i < samples; i++)
		{
			// add sample
			if (voice->oneshot)
			{
				UINT8 sample = state->m_sample_rom[(voice->start + voice->pos) >> 7 & 0x7fff];
				if (sample == 0xff)
				{
					voice->active = 0;
					break;
				}
				else
					*mix++ += state->m_volume_rom[v << 8 | sample] - 0x80;
			}
			else
			{
				UINT8 sample = state->m_sample_rom[(voice->start >> 7 & 0x7e00) | (voice->pos >> 7 & 0x1ff)];
				*mix++ += state->m_volume_rom[v << 8 | sample] - 0x80;
			}

			// update counter
			voice->pos += f;
		}
	}

	/* mix it down */
	mix = state->m_mixer_buffer;
	for (i = 0; i < samples; i++)
		*buffer++ = state->m_mixer_lookup[*mix++];
}

/* clock sound channel effect counters */
static TIMER_CALLBACK( flower_clock_effect )
{
	flower_sound_state *state = (flower_sound_state *)ptr;
	sound_channel *voice;
	state->m_stream->update();

	for (voice = state->m_channel_list; voice < state->m_last_channel; voice++)
		voice->ecount += (voice->ecount < (1<<22));
}



static DEVICE_START( flower_sound )
{
	flower_sound_state *state = get_safe_token(device);
	running_machine &machine = device->machine();
	sound_channel *voice;
	int i;

	state->m_effect_timer = machine.scheduler().timer_alloc(FUNC(flower_clock_effect), state);
	state->m_stream = device->machine().sound().stream_alloc(*device, 0, 1, MIXER_SAMPLERATE, 0, flower_update_mono);
	state->m_mixer_buffer = auto_alloc_array(device->machine(), short, MIXER_SAMPLERATE);
	make_mixer_table(device, 8, MIXER_DEFGAIN);

	/* extract globals from the interface */
	state->m_last_channel = state->m_channel_list + 8;

	state->m_sample_rom = machine.root_device().memregion("sound1")->base();
	state->m_volume_rom = machine.root_device().memregion("sound2")->base();

	/* register for savestates */
	for (i = 0; i < 8; i++)
	{
		voice = &state->m_channel_list[i];

		device->save_item(NAME(voice->freq), i+1);
		device->save_item(NAME(voice->pos), i+1);
		device->save_item(NAME(voice->volume), i+1);
		device->save_item(NAME(voice->voltab), i+1);
		device->save_item(NAME(voice->effect), i+1);
		device->save_item(NAME(voice->ecount), i+1);
		device->save_item(NAME(voice->oneshot), i+1);
		device->save_item(NAME(voice->active), i+1);
		device->save_item(NAME(voice->start), i+1);
	}
}

static DEVICE_RESET( flower_sound )
{
	flower_sound_state *state = get_safe_token(device);
	sound_channel *voice;
	attotime period;
	int i;

	/* reset effect timer, period is unknown/guessed */
	period = attotime::from_hz(MIXER_SAMPLERATE / 256);
	state->m_effect_timer->adjust(period, 0, period);

	/* reset all the voices */
	for (i = 0; i < 8; i++)
	{
		voice = &state->m_channel_list[i];

		voice->freq = 0;
		voice->pos = 0;
		voice->volume = 0;
		voice->voltab = 0;
		voice->effect = 0;
		voice->ecount = 0;
		voice->oneshot = 1;
		voice->active = 0;
		voice->start = 0;
	}
}

/********************************************************************************/

#if FLOWER_VERBOSE
static int r_numwrites[2][8] = {{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0}};
static void show_soundregs(device_t *device)
{
	flower_sound_state *state = get_safe_token(device);
	int set,reg,chan;
	char text[0x100];
	char message[0x1000] = {0};
	UINT8 *base = state->m_soundregs1;

	for (set=0;set<2;set++)
	{
		for (reg=0;reg<8;reg++)
		{
			sprintf(text,"R%d%d:",set+1,reg);
			strcat(message,text);

			for (chan=0;chan<8;chan++)
			{
				sprintf(text," %02X",base[reg + 8*chan]);
				strcat(message,text);
			}
			sprintf(text," - %07d\n",r_numwrites[set][reg]);
			strcat(message,text);
		}
		strcat(message,"\n");
		base = state->m_soundregs2;
	}
	popmessage("%s",message);
}
#endif // FLOWER_VERBOSE


/* register functions (preliminary):
offset: cccrrr      c=channel, r=register

set 1:
R  76543210
0  xxxxxxxx         frequency (which nibble?)
1  xxxxxxxx         *
2  xxxxxxxx         *
3  xxxxxxxx         *
4  ...x....         one-shot sample
5  ...x....         ??? same as R4?
6  ........         unused
7  xxxx....         volume

set 2:
R  76543210
0  ....xxxx         start address
1  ....xxxx         *
2  ....xxxx         *
3  ....xxxx         *
4  xxxx             assume it's channel pitch/volume effects
       xxxx         start address
5  x...             ???
       xxxx         start address
6  ........         unused
7  ......xx         volume table + start trigger

*/

WRITE8_DEVICE_HANDLER( flower_sound1_w )
{
	flower_sound_state *state = get_safe_token(device);
	sound_channel *voice = &state->m_channel_list[offset >> 3 & 7];
	int c = offset & 0xf8;
	UINT8 *base1 = state->m_soundregs1;
//  UINT8 *base2 = state->m_soundregs2;

	state->m_stream->update();
	base1[offset] = data;
#if FLOWER_VERBOSE
	r_numwrites[0][offset & 7]++;
	show_soundregs(device);
#endif

	// recompute voice parameters
	voice->freq = (base1[c+2] & 0xf) << 12 | (base1[c+3] & 0xf) << 8 | (base1[c+0] & 0xf) << 4 | (base1[c+1] & 0xf);
	voice->volume = base1[c+7] >> 4;
}

WRITE8_DEVICE_HANDLER( flower_sound2_w )
{
	flower_sound_state *state = get_safe_token(device);
	sound_channel *voice = &state->m_channel_list[offset >> 3 & 7];
	int i, c = offset & 0xf8;
	UINT8 *base1 = state->m_soundregs1;
	UINT8 *base2 = state->m_soundregs2;

	state->m_stream->update();
	base2[offset] = data;
#if FLOWER_VERBOSE
	r_numwrites[1][offset & 7]++;
	show_soundregs(device);
#endif

	// reg 7 is start trigger!
	if ((offset & 7) != 7)
		return;

	voice->voltab = (base2[c+7] & 3) << 4;
	voice->oneshot = (~base1[c+4] & 0x10) >> 4;
	voice->effect = base2[c+4] >> 4;
	voice->ecount = 0;
	voice->pos = 0;
	voice->active = 1;

	// full start address is 6 nibbles
	voice->start = 0;
	for (i = 5; i >= 0; i--)
		voice->start = (voice->start << 4) | (base2[c+i] & 0xf);
}


const device_type FLOWER = &device_creator<flower_sound_device>;

flower_sound_device::flower_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, FLOWER, "Flower Custom", tag, owner, clock),
	  device_sound_interface(mconfig, *this)
{
	m_token = global_alloc_array_clear(UINT8, sizeof(flower_sound_state));
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void flower_sound_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void flower_sound_device::device_start()
{
	DEVICE_START_NAME( flower_sound )(this);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void flower_sound_device::device_reset()
{
	DEVICE_RESET_NAME( flower_sound )(this);
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void flower_sound_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// should never get here
	fatalerror("sound_stream_update called; not applicable to legacy sound devices\n");
}


