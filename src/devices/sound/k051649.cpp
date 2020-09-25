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
#include <algorithm>

#define DEF_GAIN    8

void k051649_device::scc_map(address_map &map)
{
	map(0x00, 0x7f).rw(FUNC(k051649_device::k051649_waveform_r), FUNC(k051649_device::k051649_waveform_w));
	map(0x80, 0x89).mirror(0x10).w(FUNC(k051649_device::k051649_frequency_w));
	map(0x8a, 0x8e).mirror(0x10).w(FUNC(k051649_device::k051649_volume_w));
	map(0x8f, 0x8f).mirror(0x10).w(FUNC(k051649_device::k051649_keyonoff_w));
	map(0xe0, 0xe0).mirror(0x1f).rw(FUNC(k051649_device::k051649_test_r), FUNC(k051649_device::k051649_test_w));
}

// device type definition
DEFINE_DEVICE_TYPE(K051649, k051649_device, "k051649", "K051649 SCC1")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  k051649_device - constructor
//-------------------------------------------------

k051649_device::k051649_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, K051649, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, m_stream(nullptr)
	, m_rate(0)
	, m_mixer_lookup(nullptr)
	, m_test(0)
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

	// allocate a buffer to mix into - 1 second's worth should be more than enough
	m_mixer_buffer.resize(2 * m_rate);

	// build the mixer table
	make_mixer_table(5);

	// save states
	save_item(STRUCT_MEMBER(m_channel_list, counter));
	save_item(STRUCT_MEMBER(m_channel_list, clock));
	save_item(STRUCT_MEMBER(m_channel_list, frequency));
	save_item(STRUCT_MEMBER(m_channel_list, volume));
	save_item(STRUCT_MEMBER(m_channel_list, key));
	save_item(STRUCT_MEMBER(m_channel_list, waveram));
	save_item(NAME(m_test));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void k051649_device::device_reset()
{
	// reset all the voices
	for (sound_channel &voice : m_channel_list)
	{
		voice.frequency = 0;
		voice.volume = 0xf;
		voice.counter = 0;
		voice.clock = 0;
		voice.key = false;
	}

	// other parameters
	m_test = 0;
}


//-------------------------------------------------
//  device_post_load - device-specific post-load
//-------------------------------------------------

void k051649_device::device_post_load()
{
	device_clock_changed();
}


//-------------------------------------------------
//  device_clock_changed - called if the clock
//  changes
//-------------------------------------------------

void k051649_device::device_clock_changed()
{
	const u32 old_rate = m_rate;
	m_rate = clock()/16;

	if (old_rate < m_rate)
	{
		m_mixer_buffer.resize(2 * m_rate, 0);
	}
	m_stream->set_sample_rate(m_rate);
}


//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void k051649_device::sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs)
{
	// zap the contents of the mixer buffer
	std::fill(m_mixer_buffer.begin(), m_mixer_buffer.end(), 0);

	for (sound_channel &voice : m_channel_list)
	{
		// channel is halted for freq < 9
		if (voice.frequency > 8)
		{
			const int v = voice.volume * voice.key;
			int a = voice.counter;
			int c = voice.clock;
			const int step = voice.frequency;

			// add our contribution
			for (int i = 0; i < outputs[0].samples(); i++)
			{
				c += 32;
				while (c > step)
				{
					a = (a + 1) & 0x1f;
					c -= step+1;
				}
				m_mixer_buffer[i] += (voice.waveram[a] * v) >> 3;
			}

			// update the counter for this voice
			voice.counter = a;
			voice.clock = c;
		}
	}

	// mix it down
	auto &buffer = outputs[0];
	for (int i = 0; i < buffer.samples(); i++)
		buffer.put(i, m_mixer_lookup[m_mixer_buffer[i]]);
}


/********************************************************************************/


void k051649_device::k051649_waveform_w(offs_t offset, u8 data)
{
	// waveram is read-only?
	if (m_test & 0x40 || (m_test & 0x80 && offset >= 0x60))
		return;

	m_stream->update();

	if (offset >= 0x60)
	{
		// channel 5 shares waveram with channel 4
		m_channel_list[3].waveram[offset & 0x1f] = data;
		m_channel_list[4].waveram[offset & 0x1f] = data;
	}
	else
		m_channel_list[offset >> 5].waveram[offset & 0x1f] = data;
}


u8 k051649_device::k051649_waveform_r(offs_t offset)
{
	// test-register bits 6/7 expose the internal counter
	if (m_test & 0xc0)
	{
		m_stream->update();

		if (offset >= 0x60)
			offset += m_channel_list[3 + (m_test >> 6 & 1)].counter;
		else if (m_test & 0x40)
			offset += m_channel_list[offset >> 5].counter;
	}
	return m_channel_list[offset >> 5].waveram[offset & 0x1f];
}


void k051649_device::k052539_waveform_w(offs_t offset, u8 data)
{
	// waveram is read-only?
	if (m_test & 0x40)
		return;

	m_stream->update();
	m_channel_list[offset >> 5].waveram[offset & 0x1f] = data;
}


u8 k051649_device::k052539_waveform_r(offs_t offset)
{
	// test-register bit 6 exposes the internal counter
	if (m_test & 0x40)
	{
		m_stream->update();
		offset += m_channel_list[offset >> 5].counter;
	}
	return m_channel_list[offset >> 5].waveram[offset & 0x1f];
}


void k051649_device::k051649_volume_w(offs_t offset, u8 data)
{
	m_stream->update();
	m_channel_list[offset & 0x7].volume = data & 0xf;
}


void k051649_device::k051649_frequency_w(offs_t offset, u8 data)
{
	const int freq_hi = offset & 1;
	offset >>= 1;

	m_stream->update();

	// test-register bit 5 resets the internal counter
	if (m_test & 0x20)
	{
		m_channel_list[offset].counter = 0;
		m_channel_list[offset].clock = 0;
	}
	// TODO: correct?
	else if (m_channel_list[offset].frequency < 9)
		m_channel_list[offset].clock = 0;

	// update frequency
	if (freq_hi)
		m_channel_list[offset].frequency = (m_channel_list[offset].frequency & 0x0ff) | (data << 8 & 0xf00);
	else
		m_channel_list[offset].frequency = (m_channel_list[offset].frequency & 0xf00) | data;
}


void k051649_device::k051649_keyonoff_w(u8 data)
{
	m_stream->update();

	for (int i = 0; i < 5; i++)
	{
		m_channel_list[i].key = BIT(data, i);
	}
}


void k051649_device::k051649_test_w(u8 data)
{
	m_test = data;
}


u8 k051649_device::k051649_test_r()
{
	// reading the test register sets it to $ff!
	if (!machine().side_effects_disabled())
		k051649_test_w(0xff);
	return 0xff;
}


//-------------------------------------------------
// build a table to divide by the number of voices
//-------------------------------------------------

void k051649_device::make_mixer_table(int voices)
{
	// allocate memory
	m_mixer_table.resize(512 * voices);

	// find the middle of the table
	m_mixer_lookup = &m_mixer_table[256 * voices];

	// fill in the table - 16 bit case
	for (int i = 0; i < (voices * 256); i++)
	{
		const int val = std::min(32767, i * DEF_GAIN * 16 / voices);
		stream_buffer::sample_t fval = stream_buffer::sample_t(val) / 32768.0;
		m_mixer_lookup[ i] = fval;
		m_mixer_lookup[-i] = -fval;
	}
}
