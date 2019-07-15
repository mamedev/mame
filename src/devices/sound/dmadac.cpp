// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    DMA-driven DAC driver
    by Aaron Giles

***************************************************************************/

#include "emu.h"
#include "dmadac.h"

//#define VERBOSE 1
#include "logmacro.h"



/*************************************
 *
 *  Constants
 *
 *************************************/

#define DEFAULT_SAMPLE_RATE         (44100)

#define BUFFER_SIZE                 32768


/*************************************
 *
 *  Sound hardware init
 *
 *************************************/

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void dmadac_sound_device::device_start()
{
	/* allocate a clear a buffer */
	m_buffer = make_unique_clear<int16_t[]>(BUFFER_SIZE);

	/* reset the state */
	m_volume = 0x100;

	/* allocate a stream channel */
	m_channel = machine().sound().stream_alloc(*this, 0, 1, DEFAULT_SAMPLE_RATE);

	/* register with the save state system */
	save_item(NAME(m_bufin));
	save_item(NAME(m_bufout));
	save_item(NAME(m_volume));
	save_item(NAME(m_enabled));
	save_item(NAME(m_frequency));
	save_pointer(NAME(m_buffer), BUFFER_SIZE);
}



/*************************************
 *
 *  Primary transfer routine
 *
 *************************************/

void dmadac_transfer(dmadac_sound_device **devlist, uint8_t num_channels, offs_t channel_spacing, offs_t frame_spacing, offs_t total_frames, int16_t *data)
{
	int i;

	/* flush out as much data as we can */
	for (i = 0; i < num_channels; i++)
	{
		devlist[i]->flush();
	}

	/* loop over all channels and accumulate the data */
	for (i = 0; i < num_channels; i++)
	{
		devlist[i]->transfer(i, channel_spacing, frame_spacing, total_frames, data);
	}
}

void dmadac_sound_device::flush()
{
	m_channel->update();
}

void dmadac_sound_device::transfer(int channel, offs_t channel_spacing, offs_t frame_spacing, offs_t total_frames, int16_t *data)
{
	int j;

	/* loop over all channels and accumulate the data */
	if (m_enabled)
	{
		int maxin = (m_bufout + BUFFER_SIZE - 1) % BUFFER_SIZE;
		int16_t *src = data + channel * channel_spacing;
		int curin = m_bufin;

		/* copy the data */
		for (j = 0; j < total_frames && curin != maxin; j++)
		{
			m_buffer[curin] = *src;
			curin = (curin + 1) % BUFFER_SIZE;
			src += frame_spacing;
		}
		m_bufin = curin;

		/* log overruns */
		if (j != total_frames)
			logerror("dmadac_transfer: buffer overrun (short %d frames)\n", total_frames - j);
	}

	// FIXME: this line has rotted and can no longer compile - it should be fixed and uncommented or removed
	//LOG("dmadac_transfer - %d samples, %d effective, %d in buffer\n", total_frames, int(total_frames * double(DEFAULT_SAMPLE_RATE) / dmadac[first_channel].frequency), dmadac[first_channel].curinpos - dmadac[first_channel].curoutpos);
}



/*************************************
 *
 *  Enable/disable DMA channel(s)
 *
 *************************************/

void dmadac_enable(dmadac_sound_device **devlist, uint8_t num_channels, uint8_t enable)
{
	int i;

	/* flush out as much data as we can */
	for (i = 0; i < num_channels; i++)
	{
		if (devlist[i])
			devlist[i]->enable(enable);
	}
}


void dmadac_sound_device::enable(uint8_t enable)
{
	m_channel->update();
	m_enabled = enable;
	if (!enable)
		m_bufin = m_bufout = 0;
}

/*************************************
 *
 *  Set the frequency on DMA channel(s)
 *
 *************************************/

void dmadac_set_frequency(dmadac_sound_device **devlist, uint8_t num_channels, double frequency)
{
	int i;

	/* set the sample rate on each channel */
	for (i = 0; i < num_channels; i++)
	{
		devlist[i]->set_frequency(frequency);
	}
}


void dmadac_sound_device::set_frequency(double frequency)
{
	m_channel->set_sample_rate(frequency);
}

/*************************************
 *
 *  Set the volume on DMA channel(s)
 *
 *************************************/

void dmadac_set_volume(dmadac_sound_device **devlist, uint8_t num_channels, uint16_t volume)
{
	int i;

	/* flush out as much data as we can */
	for (i = 0; i < num_channels; i++)
	{
		devlist[i]->set_volume(volume);
	}
}

void dmadac_sound_device::set_volume(uint16_t volume)
{
	m_channel->update();
	m_volume = volume;
}

DEFINE_DEVICE_TYPE(DMADAC, dmadac_sound_device, "dmadac", "DMA-driven DAC")

dmadac_sound_device::dmadac_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, DMADAC, tag, owner, clock),
		device_sound_interface(mconfig, *this),
		m_buffer(nullptr),
		m_bufin(0),
		m_bufout(0),
		m_volume(0),
		m_enabled(0),
		m_frequency(0)
{
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void dmadac_sound_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	stream_sample_t *output = outputs[0];
	int16_t *source = m_buffer.get();
	uint32_t curout = m_bufout;
	uint32_t curin = m_bufin;
	int volume = m_volume;

	/* feed as much as we can */
	while (curout != curin && samples-- > 0)
	{
		*output++ = (source[curout] * volume) >> 8;
		curout = (curout + 1) % BUFFER_SIZE;
	}

	/* fill the rest with silence */
	while (samples-- > 0)
		*output++ = 0;

	/* save the new output pointer */
	m_bufout = curout;
}
