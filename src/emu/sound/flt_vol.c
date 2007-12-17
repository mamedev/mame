#include "sndintrf.h"
#include "streams.h"
#include "flt_vol.h"


struct filter_volume_info
{
	sound_stream *	stream;
	int				gain;
};



static void filter_volume_update(void *param, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	stream_sample_t *src = inputs[0];
	stream_sample_t *dst = outputs[0];
	struct filter_volume_info *info = param;

	while (samples--)
		*dst++ = (*src++ * info->gain) >> 8;
}


static void *filter_volume_start(int sndindex, int clock, const void *config)
{
	struct filter_volume_info *info;

	info = auto_malloc(sizeof(*info));
	memset(info, 0, sizeof(*info));

	info->gain = 0x100;
	info->stream = stream_create(1, 1, Machine->sample_rate, info, filter_volume_update);

	return info;
}


void flt_volume_set_volume(int num, float volume)
{
	struct filter_volume_info *info = sndti_token(SOUND_FILTER_VOLUME, num);
	info->gain = (int)(volume * 256);
}



/**************************************************************************
 * Generic get_info
 **************************************************************************/

static void filter_volume_set_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		/* no parameters to set */
	}
}


void filter_volume_get_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case SNDINFO_PTR_SET_INFO:						info->set_info = filter_volume_set_info;break;
		case SNDINFO_PTR_START:							info->start = filter_volume_start;		break;
		case SNDINFO_PTR_STOP:							/* Nothing */							break;
		case SNDINFO_PTR_RESET:							/* Nothing */							break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case SNDINFO_STR_NAME:							info->s = "Volume Filter";				break;
		case SNDINFO_STR_CORE_FAMILY:					info->s = "Filters";					break;
		case SNDINFO_STR_CORE_VERSION:					info->s = "1.0";						break;
		case SNDINFO_STR_CORE_FILE:						info->s = __FILE__;						break;
		case SNDINFO_STR_CORE_CREDITS:					info->s = "Copyright (c) 2004, The MAME Team"; break;
	}
}

