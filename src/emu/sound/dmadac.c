/***************************************************************************

    DMA-driven DAC driver
    by Aaron Giles

***************************************************************************/

#include "emu.h"
#include "streams.h"
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

typedef struct _dmadac_state dmadac_state;
struct _dmadac_state
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


INLINE dmadac_state *get_safe_token(running_device *device)
{
	assert(device != NULL);
	assert(device->token != NULL);
	assert(device->type == SOUND);
	assert(sound_get_type(device) == SOUND_DMADAC);
	return (dmadac_state *)device->token;
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
	info->buffer = auto_alloc_array_clear(device->machine, INT16, BUFFER_SIZE);

	/* reset the state */
	info->volume = 0x100;

	/* allocate a stream channel */
	info->channel = stream_create(device, 0, 1, DEFAULT_SAMPLE_RATE, info, dmadac_update);

	/* register with the save state system */
	state_save_register_device_item(device, 0, info->bufin);
	state_save_register_device_item(device, 0, info->bufout);
	state_save_register_device_item(device, 0, info->volume);
	state_save_register_device_item(device, 0, info->enabled);
	state_save_register_device_item(device, 0, info->frequency);
	state_save_register_device_item_pointer(device, 0, info->buffer, BUFFER_SIZE);
}



/*************************************
 *
 *  Primary transfer routine
 *
 *************************************/

void dmadac_transfer(running_device **devlist, UINT8 num_channels, offs_t channel_spacing, offs_t frame_spacing, offs_t total_frames, INT16 *data)
{
	int i, j;

	/* flush out as much data as we can */
	for (i = 0; i < num_channels; i++)
	{
		dmadac_state *info = get_safe_token(devlist[i]);
		stream_update(info->channel);
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

void dmadac_enable(running_device **devlist, UINT8 num_channels, UINT8 enable)
{
	int i;

	/* flush out as much data as we can */
	for (i = 0; i < num_channels; i++)
	{
		dmadac_state *info = get_safe_token(devlist[i]);
		stream_update(info->channel);
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

void dmadac_set_frequency(running_device **devlist, UINT8 num_channels, double frequency)
{
	int i;

	/* set the sample rate on each channel */
	for (i = 0; i < num_channels; i++)
	{
		dmadac_state *info = get_safe_token(devlist[i]);
		stream_set_sample_rate(info->channel, frequency);
	}
}



/*************************************
 *
 *  Set the volume on DMA channel(s)
 *
 *************************************/

void dmadac_set_volume(running_device **devlist, UINT8 num_channels, UINT16 volume)
{
	int i;

	/* flush out as much data as we can */
	for (i = 0; i < num_channels; i++)
	{
		dmadac_state *info = get_safe_token(devlist[i]);
		stream_update(info->channel);
		info->volume = volume;
	}
}



/**************************************************************************
 * Generic get_info
 **************************************************************************/

DEVICE_GET_INFO( dmadac )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:					info->i = sizeof(dmadac_state);					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME( dmadac );		break;
		case DEVINFO_FCT_STOP:							/* nothing */									break;
		case DEVINFO_FCT_RESET:							/* nothing */									break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "DMA-driven DAC");				break;
		case DEVINFO_STR_FAMILY:					strcpy(info->s, "DAC");							break;
		case DEVINFO_STR_VERSION:					strcpy(info->s, "1.0");							break;
		case DEVINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);						break;
		case DEVINFO_STR_CREDITS:					strcpy(info->s, "Copyright Nicola Salmoria and the MAME Team"); break;
	}
}

