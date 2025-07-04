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

#define DEFAULT_SAMPLE_RATE         (48000)


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
	initialize_state();

	/* allocate a stream channel */
	m_channel = stream_alloc(0, 1, DEFAULT_SAMPLE_RATE);

	/* register with the save state system */
	save_item(NAME(m_bufin));
	save_item(NAME(m_bufout));
	save_item(NAME(m_volume));
	save_item(NAME(m_enabled));
	save_item(NAME(m_buffer));
}

void dmadac_sound_device::initialize_state()
{
	/* reset the state */
	m_volume = 1.0;
	m_bufin = m_bufout = 0;
	m_enabled = 0;

	std::fill_n(m_buffer.begin(), BUFFER_SIZE, 0);
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
	m_volume = sound_stream::sample_t(volume) * (1.0 / 256.0);
}

DEFINE_DEVICE_TYPE(DMADAC, dmadac_sound_device, "dmadac", "DMA-driven DAC")

dmadac_sound_device::dmadac_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, DMADAC, tag, owner, clock),
		device_sound_interface(mconfig, *this),
		m_buffer(BUFFER_SIZE),
		m_bufin(0),
		m_bufout(0),
		m_volume(0),
		m_enabled(0)
{
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void dmadac_sound_device::sound_stream_update(sound_stream &stream)
{
	uint32_t curout = m_bufout;
	uint32_t curin = m_bufin;

	/* feed as much as we can, leave the rest as silence */
	int sampindex;
	for (sampindex = 0; curout != curin && sampindex < stream.samples(); sampindex++)
	{
		stream.put(0, sampindex, sound_stream::sample_t(m_buffer[curout]) * m_volume);
		curout = (curout + 1) % BUFFER_SIZE;
	}

	/* save the new output pointer */
	m_bufout = curout;
}
