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

#define FREQBASEBITS    16


// device type definition
const device_type K005289 = &device_creator<k005289_device>;


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  k005289_device - constructor
//-------------------------------------------------

k005289_device::k005289_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, K005289, "K005289", tag, owner, clock),
		device_sound_interface(mconfig, *this),
	m_sound_prom(NULL),
	m_stream(NULL),
	m_mclock(0),
	m_rate(0),
	m_mixer_table(NULL),
	m_mixer_lookup(NULL),
	m_mixer_buffer(NULL),
	m_k005289_A_frequency(0),
	m_k005289_B_frequency(0),
	m_k005289_A_volume(0),
	m_k005289_B_volume(0),
	m_k005289_A_waveform(0),
	m_k005289_B_waveform(0),
	m_k005289_A_latch(0),
	m_k005289_B_latch(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void k005289_device::device_start()
{
	k005289_sound_channel *voice;

	voice = m_channel_list;

	/* get stream channels */
	m_rate = clock()/16;
	m_stream = stream_alloc(0, 1, m_rate);
	m_mclock = clock();

	/* allocate a pair of buffers to mix into - 1 second's worth should be more than enough */
	m_mixer_buffer = auto_alloc_array(machine(), short, 2 * m_rate);

	/* build the mixer table */
	make_mixer_table(2);

	m_sound_prom = m_region->base();

	/* reset all the voices */
	voice[0].frequency = 0;
	voice[0].volume = 0;
	voice[0].wave = &m_sound_prom[0];
	voice[0].counter = 0;
	voice[1].frequency = 0;
	voice[1].volume = 0;
	voice[1].wave = &m_sound_prom[0x100];
	voice[1].counter = 0;
}


//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void k005289_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	k005289_sound_channel *voice=m_channel_list;
	stream_sample_t *buffer = outputs[0];
	short *mix;
	int i,v,f;

	/* zap the contents of the mixer buffer */
	memset(m_mixer_buffer, 0, samples * sizeof(INT16));

	v=voice[0].volume;
	f=voice[0].frequency;
	if (v && f)
	{
		const unsigned char *w = voice[0].wave;
		int c = voice[0].counter;

		mix = m_mixer_buffer;

		/* add our contribution */
		for (i = 0; i < samples; i++)
		{
			int offs;

			c+=(long)((((float)m_mclock / (float)(f * 16))*(float)(1<<FREQBASEBITS)) / (float)(m_rate / 32));
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

		mix = m_mixer_buffer;

		/* add our contribution */
		for (i = 0; i < samples; i++)
		{
			int offs;

			c+=(long)((((float)m_mclock / (float)(f * 16))*(float)(1<<FREQBASEBITS)) / (float)(m_rate / 32));
			offs = (c >> 16) & 0x1f;
			*mix++ += ((w[offs] & 0x0f) - 8) * v;
		}

		/* update the counter for this voice */
		voice[1].counter = c;
	}

	/* mix it down */
	mix = m_mixer_buffer;
	for (i = 0; i < samples; i++)
		*buffer++ = m_mixer_lookup[*mix++];
}




/********************************************************************************/

/* build a table to divide by the number of voices */
void k005289_device::make_mixer_table(int voices)
{
	int count = voices * 128;
	int i;
	int gain = 16;

	/* allocate memory */
	m_mixer_table = auto_alloc_array(machine(), INT16, 256 * voices);

	/* find the middle of the table */
	m_mixer_lookup = m_mixer_table + (128 * voices);

	/* fill in the table - 16 bit case */
	for (i = 0; i < count; i++)
	{
		int val = i * gain * 16 / voices;
		if (val > 32767) val = 32767;
		m_mixer_lookup[ i] = val;
		m_mixer_lookup[-i] = -val;
	}
}


void k005289_device::k005289_recompute()
{
	k005289_sound_channel *voice = m_channel_list;

	m_stream->update(); /* update the streams */

	voice[0].frequency = m_k005289_A_frequency;
	voice[1].frequency = m_k005289_B_frequency;
	voice[0].volume = m_k005289_A_volume;
	voice[1].volume = m_k005289_B_volume;
	voice[0].wave = &m_sound_prom[32 * m_k005289_A_waveform];
	voice[1].wave = &m_sound_prom[32 * m_k005289_B_waveform + 0x100];
}


WRITE8_MEMBER( k005289_device::k005289_control_A_w )
{
	m_k005289_A_volume=data&0xf;
	m_k005289_A_waveform=data>>5;
	k005289_recompute();
}


WRITE8_MEMBER( k005289_device::k005289_control_B_w )
{
	m_k005289_B_volume=data&0xf;
	m_k005289_B_waveform=data>>5;
	k005289_recompute();
}


WRITE8_MEMBER( k005289_device::k005289_pitch_A_w )
{
	m_k005289_A_latch = 0x1000 - offset;
}


WRITE8_MEMBER( k005289_device::k005289_pitch_B_w )
{
	m_k005289_B_latch = 0x1000 - offset;
}


WRITE8_MEMBER( k005289_device::k005289_keylatch_A_w )
{
	m_k005289_A_frequency = m_k005289_A_latch;
	k005289_recompute();
}


WRITE8_MEMBER( k005289_device::k005289_keylatch_B_w )
{
	m_k005289_B_frequency = m_k005289_B_latch;
	k005289_recompute();
}
