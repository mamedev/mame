/***************************************************************************

    DMA-driven DAC driver
    by Aaron Giles

***************************************************************************/

#include "sndintrf.h"
#include "streams.h"
#include "dmadac.h"



/*************************************
 *
 *  Debugging
 *
 *************************************/

#define VERBOSE		0

#if VERBOSE
#define LOG(x) logerror x
#else
#define LOG(x)
#endif



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

struct dmadac_channel_data
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



/*************************************
 *
 *  Stream callback
 *
 *************************************/

static void dmadac_update(void *param, stream_sample_t **inputs, stream_sample_t **_buffer, int length)
{
	struct dmadac_channel_data *ch = param;
	stream_sample_t *output = _buffer[0];
	INT16 *source = ch->buffer;
	UINT32 curout = ch->bufout;
	UINT32 curin = ch->bufin;
	int volume = ch->volume;

	/* feed as much as we can */
	while (curout != curin && length-- > 0)
		*output++ = (source[curout++ % BUFFER_SIZE] * volume) >> 8;

	/* fill the rest with silence */
	while (length-- > 0)
		*output++ = 0;

	/* save the new output pointer */
	ch->bufout = curout % BUFFER_SIZE;
}



/*************************************
 *
 *  Sound hardware init
 *
 *************************************/

static void *dmadac_start(int sndindex, int clock, const void *config)
{
	struct dmadac_channel_data *info;

	info = auto_malloc(sizeof(*info));
	memset(info, 0, sizeof(*info));

	/* allocate a clear a buffer */
	info->buffer = auto_malloc(sizeof(info->buffer[0]) * BUFFER_SIZE);
	memset(info->buffer, 0, sizeof(info->buffer[0]) * BUFFER_SIZE);

	/* reset the state */
	info->volume = 0x100;

	/* allocate a stream channel */
	info->channel = stream_create(0, 1, DEFAULT_SAMPLE_RATE, info, dmadac_update);

	/* register with the save state system */
	state_save_register_item("dmadac", sndindex, info->bufin);
	state_save_register_item("dmadac", sndindex, info->bufout);
	state_save_register_item("dmadac", sndindex, info->volume);
	state_save_register_item("dmadac", sndindex, info->enabled);
	state_save_register_item("dmadac", sndindex, info->frequency);
	state_save_register_item_pointer("dmadac", sndindex, info->buffer, BUFFER_SIZE);

	return info;
}



/*************************************
 *
 *  Primary transfer routine
 *
 *************************************/

void dmadac_transfer(UINT8 first_channel, UINT8 num_channels, offs_t channel_spacing, offs_t frame_spacing, offs_t total_frames, INT16 *data)
{
	int i, j;

	/* flush out as much data as we can */
	for (i = 0; i < num_channels; i++)
	{
		struct dmadac_channel_data *info = sndti_token(SOUND_DMADAC, first_channel + i);
		stream_update(info->channel);
	}

	/* loop over all channels and accumulate the data */
	for (i = 0; i < num_channels; i++)
	{
		struct dmadac_channel_data *ch = sndti_token(SOUND_DMADAC, first_channel + i);
		if (ch->enabled)
		{
			int maxin = (ch->bufout + BUFFER_SIZE - 1) % BUFFER_SIZE;
			INT16 *src = data + i * channel_spacing;
			int curin = ch->bufin;

			/* copy the data */
			for (j = 0; j < total_frames && curin != maxin; j++)
			{
				ch->buffer[curin++ % BUFFER_SIZE] = *src;
				src += frame_spacing;
			}
			ch->bufin = curin;

			/* log overruns */
			if (j != total_frames)
				logerror("dmadac_transfer: buffer overrun (short %d frames)\n", total_frames - j);
		}
	}

	LOG(("dmadac_transfer - %d samples, %d effective, %d in buffer\n", total_frames, (int)(total_frames * (double)DEFAULT_SAMPLE_RATE / dmadac[first_channel].frequency), dmadac[first_channel].curinpos - dmadac[first_channel].curoutpos));
}



/*************************************
 *
 *  Enable/disable DMA channel(s)
 *
 *************************************/

void dmadac_enable(UINT8 first_channel, UINT8 num_channels, UINT8 enable)
{
	int i;

	/* flush out as much data as we can */
	for (i = 0; i < num_channels; i++)
	{
		struct dmadac_channel_data *info = sndti_token(SOUND_DMADAC, first_channel + i);
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

void dmadac_set_frequency(UINT8 first_channel, UINT8 num_channels, double frequency)
{
	int i;

	/* set the sample rate on each channel */
	for (i = 0; i < num_channels; i++)
	{
		struct dmadac_channel_data *info = sndti_token(SOUND_DMADAC, first_channel + i);
		stream_set_sample_rate(info->channel, frequency);
	}
}



/*************************************
 *
 *  Set the volume on DMA channel(s)
 *
 *************************************/

void dmadac_set_volume(UINT8 first_channel, UINT8 num_channels, UINT16 volume)
{
	int i;

	/* flush out as much data as we can */
	for (i = 0; i < num_channels; i++)
	{
		struct dmadac_channel_data *info = sndti_token(SOUND_DMADAC, first_channel + i);
		stream_update(info->channel);
		info->volume = volume;
	}
}



/**************************************************************************
 * Generic get_info
 **************************************************************************/

static void dmadac_set_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		/* no parameters to set */
	}
}


void dmadac_get_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case SNDINFO_PTR_SET_INFO:						info->set_info = dmadac_set_info;		break;
		case SNDINFO_PTR_START:							info->start = dmadac_start;				break;
		case SNDINFO_PTR_STOP:							/* nothing */							break;
		case SNDINFO_PTR_RESET:							/* nothing */							break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case SNDINFO_STR_NAME:							info->s = "DMA-driven DAC";				break;
		case SNDINFO_STR_CORE_FAMILY:					info->s = "DAC";						break;
		case SNDINFO_STR_CORE_VERSION:					info->s = "1.0";						break;
		case SNDINFO_STR_CORE_FILE:						info->s = __FILE__;						break;
		case SNDINFO_STR_CORE_CREDITS:					info->s = "Copyright (c) 2004, The MAME Team"; break;
	}
}

