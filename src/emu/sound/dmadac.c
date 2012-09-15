/***************************************************************************

    DMA-driven DAC driver
    by Aaron Giles

***************************************************************************/

#include "emu.h"
#include "dmadac.h"



/*************************************
 *
 *  Debugging
 *
 *************************************/

#define VERBOSE		0

#define LOG(x) do { if (VERBOSE) logerror x; } while (0)


/*************************************
 *
 *  Constants
 *
 *************************************/

#define DEFAULT_SAMPLE_RATE			(44100)

#define BUFFER_SIZE					32768



/*************************************
 *
 *  Types
 *
 *************************************/

struct dmadac_state
{
	/* sound stream and buffers */
	sound_stream *	channel;
	INT16 *			buffer;
	UINT32			bufin;
	UINT32			bufout;

	/* per-channel parameters */
	INT16			volume;
	UINT8			enabled;
	double			frequency;
};


INLINE dmadac_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == DMADAC);
	return (dmadac_state *)downcast<dmadac_sound_device *>(device)->token();
}


/*************************************
 *
 *  Stream callback
 *
 *************************************/

static STREAM_UPDATE( dmadac_update )
{
	dmadac_state *ch = (dmadac_state *)param;
	stream_sample_t *output = outputs[0];
	INT16 *source = ch->buffer;
	UINT32 curout = ch->bufout;
	UINT32 curin = ch->bufin;
	int volume = ch->volume;

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
	ch->bufout = curout;
}



/*************************************
 *
 *  Sound hardware init
 *
 *************************************/

static DEVICE_START( dmadac )
{
	dmadac_state *info = get_safe_token(device);

	/* allocate a clear a buffer */
	info->buffer = auto_alloc_array_clear(device->machine(), INT16, BUFFER_SIZE);

	/* reset the state */
	info->volume = 0x100;

	/* allocate a stream channel */
	info->channel = device->machine().sound().stream_alloc(*device, 0, 1, DEFAULT_SAMPLE_RATE, info, dmadac_update);

	/* register with the save state system */
	device->save_item(NAME(info->bufin));
	device->save_item(NAME(info->bufout));
	device->save_item(NAME(info->volume));
	device->save_item(NAME(info->enabled));
	device->save_item(NAME(info->frequency));
	device->save_pointer(NAME(info->buffer), BUFFER_SIZE);
}



/*************************************
 *
 *  Primary transfer routine
 *
 *************************************/

void dmadac_transfer(dmadac_sound_device **devlist, UINT8 num_channels, offs_t channel_spacing, offs_t frame_spacing, offs_t total_frames, INT16 *data)
{
	int i, j;

	/* flush out as much data as we can */
	for (i = 0; i < num_channels; i++)
	{
		dmadac_state *info = get_safe_token(devlist[i]);
		info->channel->update();
	}

	/* loop over all channels and accumulate the data */
	for (i = 0; i < num_channels; i++)
	{
		dmadac_state *ch = get_safe_token(devlist[i]);
		if (ch->enabled)
		{
			int maxin = (ch->bufout + BUFFER_SIZE - 1) % BUFFER_SIZE;
			INT16 *src = data + i * channel_spacing;
			int curin = ch->bufin;

			/* copy the data */
			for (j = 0; j < total_frames && curin != maxin; j++)
			{
				ch->buffer[curin] = *src;
				curin = (curin + 1) % BUFFER_SIZE;
				src += frame_spacing;
			}
			ch->bufin = curin;

			/* log overruns */
			if (j != total_frames)
				logerror("dmadac_transfer: buffer overrun (short %d frames)\n", total_frames - j);
		}
	}

	//LOG(("dmadac_transfer - %d samples, %d effective, %d in buffer\n", total_frames, (int)(total_frames * (double)DEFAULT_SAMPLE_RATE / dmadac[first_channel].frequency), dmadac[first_channel].curinpos - dmadac[first_channel].curoutpos));
}



/*************************************
 *
 *  Enable/disable DMA channel(s)
 *
 *************************************/

void dmadac_enable(dmadac_sound_device **devlist, UINT8 num_channels, UINT8 enable)
{
	int i;

	/* flush out as much data as we can */
	for (i = 0; i < num_channels; i++)
	{
		dmadac_state *info = get_safe_token(devlist[i]);
		info->channel->update();
		info->enabled = enable;
		if (!enable)
			info->bufin = info->bufout = 0;
	}
}



/*************************************
 *
 *  Set the frequency on DMA channel(s)
 *
 *************************************/

void dmadac_set_frequency(dmadac_sound_device **devlist, UINT8 num_channels, double frequency)
{
	int i;

	/* set the sample rate on each channel */
	for (i = 0; i < num_channels; i++)
	{
		dmadac_state *info = get_safe_token(devlist[i]);
		info->channel->set_sample_rate(frequency);
	}
}



/*************************************
 *
 *  Set the volume on DMA channel(s)
 *
 *************************************/

void dmadac_set_volume(dmadac_sound_device **devlist, UINT8 num_channels, UINT16 volume)
{
	int i;

	/* flush out as much data as we can */
	for (i = 0; i < num_channels; i++)
	{
		dmadac_state *info = get_safe_token(devlist[i]);
		info->channel->update();
		info->volume = volume;
	}
}

const device_type DMADAC = &device_creator<dmadac_sound_device>;

dmadac_sound_device::dmadac_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, DMADAC, "DMA-driven DAC", tag, owner, clock),
	  device_sound_interface(mconfig, *this)
{
	m_token = global_alloc_array_clear(UINT8, sizeof(dmadac_state));
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void dmadac_sound_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void dmadac_sound_device::device_start()
{
	DEVICE_START_NAME( dmadac )(this);
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void dmadac_sound_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// should never get here
	fatalerror("sound_stream_update called; not applicable to legacy sound devices\n");
}


