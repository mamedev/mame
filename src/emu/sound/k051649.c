/***************************************************************************

    Konami 051649 - SCC1 sound as used in Haunted Castle, City Bomber

    This file is pieced together by Bryan McPhail from a combination of
    Namco Sound, Amuse by Cab, Haunted Castle schematics and whoever first
    figured out SCC!

    The 051649 is a 5 channel sound generator, each channel gets its
    waveform from RAM (32 bytes per waveform, 8 bit signed data).

    This sound chip is the same as the sound chip in some Konami
    megaROM cartridges for the MSX. It is actually well researched
    and documented:

        http://bifi.msxnet.org/msxnet/tech/scc.html

    Thanks to Sean Young (sean@mess.org) for some bugfixes.

    K052539 is more or less equivalent to this chip except channel 5
    does not share waveram with channel 4.

***************************************************************************/

#include "emu.h"
#include "k051649.h"

#define FREQ_BITS	16
#define DEF_GAIN	8


/* this structure defines the parameters for a channel */
typedef struct
{
	unsigned long counter;
	int frequency;
	int volume;
	int key;
	signed char waveram[32];
} k051649_sound_channel;

struct k051649_state
{
	k051649_sound_channel channel_list[5];

	/* global sound parameters */
	sound_stream * stream;
	int mclock,rate;

	/* mixer tables and internal buffers */
	INT16 *mixer_table;
	INT16 *mixer_lookup;
	short *mixer_buffer;

	/* chip registers */
	UINT8 test;
};

INLINE k051649_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == K051649);
	return (k051649_state *)downcast<k051649_device *>(device)->token();
}

/* build a table to divide by the number of voices */
static void make_mixer_table(running_machine &machine, k051649_state *info, int voices)
{
	int i;

	/* allocate memory */
	info->mixer_table = auto_alloc_array(machine, INT16, 512 * voices);

	/* find the middle of the table */
	info->mixer_lookup = info->mixer_table + (256 * voices);

	/* fill in the table - 16 bit case */
	for (i = 0; i < (voices * 256); i++)
	{
		int val = i * DEF_GAIN * 16 / voices;
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
	int i,j;

	/* zap the contents of the mixer buffer */
	memset(info->mixer_buffer, 0, samples * sizeof(short));

	for (j = 0; j < 5; j++) {
		/* channel is halted for freq < 9 */
		if (voice[j].frequency > 8)
		{
			const signed char *w = voice[j].waveram;
			int v=voice[j].volume * voice[j].key;
			int c=voice[j].counter;
			int step = ((INT64)info->mclock * (1 << FREQ_BITS)) / (float)((voice[j].frequency + 1) * 16 * (info->rate / 32)) + 0.5;

			mix = info->mixer_buffer;

			/* add our contribution */
			for (i = 0; i < samples; i++)
			{
				int offs;

				c += step;
				offs = (c >> FREQ_BITS) & 0x1f;
				*mix++ += (w[offs] * v)>>3;
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
}

/********************************************************************************/

WRITE8_DEVICE_HANDLER( k051649_waveform_w )
{
	k051649_state *info = get_safe_token(device);

	/* waveram is read-only? */
	if (info->test & 0x40 || (info->test & 0x80 && offset >= 0x60))
		return;

	info->stream->update();

    if (offset >= 0x60)
    {
		/* channel 5 shares waveram with channel 4 */
		info->channel_list[3].waveram[offset&0x1f]=data;
		info->channel_list[4].waveram[offset&0x1f]=data;
	}
	else
		info->channel_list[offset>>5].waveram[offset&0x1f]=data;
}

READ8_DEVICE_HANDLER ( k051649_waveform_r )
{
	k051649_state *info = get_safe_token(device);

	/* test-register bits 6/7 expose the internal counter */
	if (info->test & 0xc0)
	{
		info->stream->update();

		if (offset >= 0x60)
			offset += (info->channel_list[3 + (info->test >> 6 & 1)].counter >> FREQ_BITS);
		else if (info->test & 0x40)
			offset += (info->channel_list[offset>>5].counter >> FREQ_BITS);
	}
	return info->channel_list[offset>>5].waveram[offset&0x1f];
}

WRITE8_DEVICE_HANDLER( k052539_waveform_w )
{
	k051649_state *info = get_safe_token(device);

	/* waveram is read-only? */
	if (info->test & 0x40)
		return;

	info->stream->update();
	info->channel_list[offset>>5].waveram[offset&0x1f]=data;
}

READ8_DEVICE_HANDLER ( k052539_waveform_r )
{
	k051649_state *info = get_safe_token(device);

	/* test-register bit 6 exposes the internal counter */
	if (info->test & 0x40)
	{
		info->stream->update();
		offset += (info->channel_list[offset>>5].counter >> FREQ_BITS);
	}
	return info->channel_list[offset>>5].waveram[offset&0x1f];
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
	int freq_hi = offset & 1;
	offset >>= 1;

	info->stream->update();

	/* test-register bit 5 resets the internal counter */
	if (info->test & 0x20)
		info->channel_list[offset].counter = ~0;
	else if (info->channel_list[offset].frequency < 9)
		info->channel_list[offset].counter |= ((1 << FREQ_BITS) - 1);

	/* update frequency */
	if (freq_hi)
		info->channel_list[offset].frequency = (info->channel_list[offset].frequency & 0x0ff) | (data << 8 & 0xf00);
	else
		info->channel_list[offset].frequency = (info->channel_list[offset].frequency & 0xf00) | data;
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
	/* reading the test register sets it to $ff! */
	k051649_test_w(device, offset, 0xff);
	return 0xff;
}

const device_type K051649 = &device_creator<k051649_device>;

k051649_device::k051649_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, K051649, "K051649", tag, owner, clock),
	  device_sound_interface(mconfig, *this)
{
	m_token = global_alloc_array_clear(UINT8, sizeof(k051649_state));
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void k051649_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void k051649_device::device_start()
{
	DEVICE_START_NAME( k051649 )(this);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void k051649_device::device_reset()
{
	DEVICE_RESET_NAME( k051649 )(this);
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void k051649_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// should never get here
	fatalerror("sound_stream_update called; not applicable to legacy sound devices\n");
}


