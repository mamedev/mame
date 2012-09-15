/***************************************************************************

    Konami 005289 - SCC sound as used in Bubblesystem

    This file is pieced together by Bryan McPhail from a combination of
    Namco Sound, Amuse by Cab, Nemesis schematics and whoever first
    figured out SCC!

    The 005289 is a 2 channel sound generator, each channel gets it's
    waveform from a prom (4 bits wide).

    (From Nemesis schematics)

    Address lines A0-A4 of the prom run to the 005289, giving 32 bytes
    per waveform.  Address lines A5-A7 of the prom run to PA5-PA7 of
    the AY8910 control port A, giving 8 different waveforms. PA0-PA3
    of the AY8910 control volume.

    The second channel is the same as above except port B is used.

    The 005289 has no data bus, so data values written don't matter.

    There are 4 unknown pins, LD1, LD2, TG1, TG2.  Two of them look to be
    the selector for changing frequency.  The other two seem unused.

***************************************************************************/

#include "emu.h"
#include "k005289.h"

#define FREQBASEBITS	16

/* this structure defines the parameters for a channel */
typedef struct
{
	int frequency;
	int counter;
	int volume;
	const unsigned char *wave;
} k005289_sound_channel;

struct k005289_state
{
	k005289_sound_channel channel_list[2];

	/* global sound parameters */
	const unsigned char *sound_prom;
	sound_stream * stream;
	int mclock,rate;

	/* mixer tables and internal buffers */
	INT16 *mixer_table;
	INT16 *mixer_lookup;
	short *mixer_buffer;

	int k005289_A_frequency,k005289_B_frequency;
	int k005289_A_volume,k005289_B_volume;
	int k005289_A_waveform,k005289_B_waveform;
	int k005289_A_latch,k005289_B_latch;
};

INLINE k005289_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == K005289);
	return (k005289_state *)downcast<k005289_device *>(device)->token();
}

/* build a table to divide by the number of voices */
static void make_mixer_table(running_machine &machine, k005289_state *info, int voices)
{
	int count = voices * 128;
	int i;
	int gain = 16;

	/* allocate memory */
	info->mixer_table = auto_alloc_array(machine, INT16, 256 * voices);

	/* find the middle of the table */
	info->mixer_lookup = info->mixer_table + (128 * voices);

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
static STREAM_UPDATE( K005289_update )
{
	k005289_state *info = (k005289_state *)param;
	k005289_sound_channel *voice=info->channel_list;
	stream_sample_t *buffer = outputs[0];
	short *mix;
	int i,v,f;

	/* zap the contents of the mixer buffer */
	memset(info->mixer_buffer, 0, samples * sizeof(INT16));

	v=voice[0].volume;
	f=voice[0].frequency;
	if (v && f)
	{
		const unsigned char *w = voice[0].wave;
		int c = voice[0].counter;

		mix = info->mixer_buffer;

		/* add our contribution */
		for (i = 0; i < samples; i++)
		{
			int offs;

			c+=(long)((((float)info->mclock / (float)(f * 16))*(float)(1<<FREQBASEBITS)) / (float)(info->rate / 32));
			offs = (c >> 16) & 0x1f;
			*mix++ += ((w[offs] & 0x0f) - 8) * v;
		}

		/* update the counter for this voice */
		voice[0].counter = c;
	}

	v=voice[1].volume;
	f=voice[1].frequency;
	if (v && f)
	{
		const unsigned char *w = voice[1].wave;
		int c = voice[1].counter;

		mix = info->mixer_buffer;

		/* add our contribution */
		for (i = 0; i < samples; i++)
		{
			int offs;

			c+=(long)((((float)info->mclock / (float)(f * 16))*(float)(1<<FREQBASEBITS)) / (float)(info->rate / 32));
			offs = (c >> 16) & 0x1f;
			*mix++ += ((w[offs] & 0x0f) - 8) * v;
		}

		/* update the counter for this voice */
		voice[1].counter = c;
	}

	/* mix it down */
	mix = info->mixer_buffer;
	for (i = 0; i < samples; i++)
		*buffer++ = info->mixer_lookup[*mix++];
}

static DEVICE_START( k005289 )
{
	k005289_sound_channel *voice;
	k005289_state *info = get_safe_token(device);

	voice = info->channel_list;

	/* get stream channels */
	info->rate = device->clock()/16;
	info->stream = device->machine().sound().stream_alloc(*device, 0, 1, info->rate, info, K005289_update);
	info->mclock = device->clock();

	/* allocate a pair of buffers to mix into - 1 second's worth should be more than enough */
	info->mixer_buffer = auto_alloc_array(device->machine(), short, 2 * info->rate);

	/* build the mixer table */
	make_mixer_table(device->machine(), info, 2);

	info->sound_prom = *device->region();

	/* reset all the voices */
	voice[0].frequency = 0;
	voice[0].volume = 0;
	voice[0].wave = &info->sound_prom[0];
	voice[0].counter = 0;
	voice[1].frequency = 0;
	voice[1].volume = 0;
	voice[1].wave = &info->sound_prom[0x100];
	voice[1].counter = 0;
}


/********************************************************************************/

static void k005289_recompute(k005289_state *info)
{
	k005289_sound_channel *voice = info->channel_list;

	info->stream->update();	/* update the streams */

	voice[0].frequency = info->k005289_A_frequency;
	voice[1].frequency = info->k005289_B_frequency;
	voice[0].volume = info->k005289_A_volume;
	voice[1].volume = info->k005289_B_volume;
	voice[0].wave = &info->sound_prom[32 * info->k005289_A_waveform];
	voice[1].wave = &info->sound_prom[32 * info->k005289_B_waveform + 0x100];
}

WRITE8_DEVICE_HANDLER( k005289_control_A_w )
{
	k005289_state *info = get_safe_token(device);
	info->k005289_A_volume=data&0xf;
	info->k005289_A_waveform=data>>5;
	k005289_recompute(info);
}

WRITE8_DEVICE_HANDLER( k005289_control_B_w )
{
	k005289_state *info = get_safe_token(device);
	info->k005289_B_volume=data&0xf;
	info->k005289_B_waveform=data>>5;
	k005289_recompute(info);
}

WRITE8_DEVICE_HANDLER( k005289_pitch_A_w )
{
	k005289_state *info = get_safe_token(device);
	info->k005289_A_latch = 0x1000 - offset;
}

WRITE8_DEVICE_HANDLER( k005289_pitch_B_w )
{
	k005289_state *info = get_safe_token(device);
	info->k005289_B_latch = 0x1000 - offset;
}

WRITE8_DEVICE_HANDLER( k005289_keylatch_A_w )
{
	k005289_state *info = get_safe_token(device);
	info->k005289_A_frequency = info->k005289_A_latch;
	k005289_recompute(info);
}

WRITE8_DEVICE_HANDLER( k005289_keylatch_B_w )
{
	k005289_state *info = get_safe_token(device);
	info->k005289_B_frequency = info->k005289_B_latch;
	k005289_recompute(info);
}

const device_type K005289 = &device_creator<k005289_device>;

k005289_device::k005289_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, K005289, "K005289", tag, owner, clock),
	  device_sound_interface(mconfig, *this)
{
	m_token = global_alloc_array_clear(UINT8, sizeof(k005289_state));
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void k005289_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void k005289_device::device_start()
{
	DEVICE_START_NAME( k005289 )(this);
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void k005289_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// should never get here
	fatalerror("sound_stream_update called; not applicable to legacy sound devices\n");
}


