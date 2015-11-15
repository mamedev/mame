// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
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

#define FREQ_BITS   16
#define DEF_GAIN    8


// device type definition
const device_type K051649 = &device_creator<k051649_device>;


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  k051649_device - constructor
//-------------------------------------------------

k051649_device::k051649_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, K051649, "K051649 SCC1", tag, owner, clock, "k051649", __FILE__),
		device_sound_interface(mconfig, *this),
		m_stream(NULL),
		m_mclock(0),
		m_rate(0),
		m_mixer_table(NULL),
		m_mixer_lookup(NULL),
		m_mixer_buffer(NULL),
		m_test(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void k051649_device::device_start()
{
	// get stream channels
	m_rate = clock()/16;
	m_stream = stream_alloc(0, 1, m_rate);
	m_mclock = clock();

	// allocate a buffer to mix into - 1 second's worth should be more than enough
	m_mixer_buffer = auto_alloc_array(machine(), short, 2 * m_rate);

	// build the mixer table
	make_mixer_table(5);
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void k051649_device::device_reset()
{
	k051649_sound_channel *voice = m_channel_list;
	int i;

	// reset all the voices
	for (i = 0; i < 5; i++)
	{
		voice[i].frequency = 0;
		voice[i].volume = 0xf;
		voice[i].counter = 0;
		voice[i].key = 0;
	}

	// other parameters
	m_test = 0;
}


//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void k051649_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	k051649_sound_channel *voice=m_channel_list;
	stream_sample_t *buffer = outputs[0];
	short *mix;
	int i,j;

	// zap the contents of the mixer buffer
	memset(m_mixer_buffer, 0, samples * sizeof(short));

	for (j = 0; j < 5; j++)
	{
		// channel is halted for freq < 9
		if (voice[j].frequency > 8)
		{
			const signed char *w = voice[j].waveram;
			int v=voice[j].volume * voice[j].key;
			int c=voice[j].counter;
			int step = ((INT64)m_mclock * (1 << FREQ_BITS)) / (float)((voice[j].frequency + 1) * 16 * (m_rate / 32)) + 0.5f;

			mix = m_mixer_buffer;

			// add our contribution
			for (i = 0; i < samples; i++)
			{
				int offs;

				c += step;
				offs = (c >> FREQ_BITS) & 0x1f;
				*mix++ += (w[offs] * v)>>3;
			}

			// update the counter for this voice
			voice[j].counter = c;
		}
	}

	// mix it down
	mix = m_mixer_buffer;
	for (i = 0; i < samples; i++)
		*buffer++ = m_mixer_lookup[*mix++];
}


/********************************************************************************/


WRITE8_MEMBER( k051649_device::k051649_waveform_w )
{
	// waveram is read-only?
	if (m_test & 0x40 || (m_test & 0x80 && offset >= 0x60))
		return;

	m_stream->update();

	if (offset >= 0x60)
	{
		// channel 5 shares waveram with channel 4
		m_channel_list[3].waveram[offset&0x1f]=data;
		m_channel_list[4].waveram[offset&0x1f]=data;
	}
	else
		m_channel_list[offset>>5].waveram[offset&0x1f]=data;
}


READ8_MEMBER ( k051649_device::k051649_waveform_r )
{
	// test-register bits 6/7 expose the internal counter
	if (m_test & 0xc0)
	{
		m_stream->update();

		if (offset >= 0x60)
			offset += (m_channel_list[3 + (m_test >> 6 & 1)].counter >> FREQ_BITS);
		else if (m_test & 0x40)
			offset += (m_channel_list[offset>>5].counter >> FREQ_BITS);
	}
	return m_channel_list[offset>>5].waveram[offset&0x1f];
}


WRITE8_MEMBER( k051649_device::k052539_waveform_w )
{
	// waveram is read-only?
	if (m_test & 0x40)
		return;

	m_stream->update();
	m_channel_list[offset>>5].waveram[offset&0x1f]=data;
}


READ8_MEMBER ( k051649_device::k052539_waveform_r )
{
	// test-register bit 6 exposes the internal counter
	if (m_test & 0x40)
	{
		m_stream->update();
		offset += (m_channel_list[offset>>5].counter >> FREQ_BITS);
	}
	return m_channel_list[offset>>5].waveram[offset&0x1f];
}


WRITE8_MEMBER( k051649_device::k051649_volume_w )
{
	m_stream->update();
	m_channel_list[offset&0x7].volume=data&0xf;
}


WRITE8_MEMBER( k051649_device::k051649_frequency_w )
{
	int freq_hi = offset & 1;
	offset >>= 1;

	m_stream->update();

	// test-register bit 5 resets the internal counter
	if (m_test & 0x20)
		m_channel_list[offset].counter = ~0;
	else if (m_channel_list[offset].frequency < 9)
		m_channel_list[offset].counter |= ((1 << FREQ_BITS) - 1);

	// update frequency
	if (freq_hi)
		m_channel_list[offset].frequency = (m_channel_list[offset].frequency & 0x0ff) | (data << 8 & 0xf00);
	else
		m_channel_list[offset].frequency = (m_channel_list[offset].frequency & 0xf00) | data;
}


WRITE8_MEMBER( k051649_device::k051649_keyonoff_w )
{
	int i;
	m_stream->update();

	for (i = 0; i < 5; i++)
	{
		m_channel_list[i].key=data&1;
		data >>= 1;
	}
}


WRITE8_MEMBER( k051649_device::k051649_test_w )
{
	m_test = data;
}


READ8_MEMBER ( k051649_device::k051649_test_r )
{
	// reading the test register sets it to $ff!
	k051649_test_w(space, offset, 0xff);
	return 0xff;
}


//-------------------------------------------------
// build a table to divide by the number of voices
//-------------------------------------------------

void k051649_device::make_mixer_table(int voices)
{
	int i;

	// allocate memory
	m_mixer_table = auto_alloc_array(machine(), INT16, 512 * voices);

	// find the middle of the table
	m_mixer_lookup = m_mixer_table + (256 * voices);

	// fill in the table - 16 bit case
	for (i = 0; i < (voices * 256); i++)
	{
		int val = i * DEF_GAIN * 16 / voices;
		if (val > 32767) val = 32767;
		m_mixer_lookup[ i] = val;
		m_mixer_lookup[-i] = -val;
	}
}
