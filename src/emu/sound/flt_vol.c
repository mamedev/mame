#include "emu.h"
#include "streams.h"
#include "flt_vol.h"


typedef struct _filter_volume_state filter_volume_state;
struct _filter_volume_state
{
	sound_stream *	stream;
	int				gain;
};

INLINE filter_volume_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == FILTER_VOLUME);
	return (filter_volume_state *)downcast<legacy_device_base *>(device)->token();
}



static STREAM_UPDATE( filter_volume_update )
{
	stream_sample_t *src = inputs[0];
	stream_sample_t *dst = outputs[0];
	filter_volume_state *info = (filter_volume_state *)param;

	while (samples--)
		*dst++ = (*src++ * info->gain) >> 8;
}


static DEVICE_START( filter_volume )
{
	filter_volume_state *info = get_safe_token(device);

	info->gain = 0x100;
	info->stream = stream_create(device, 1, 1, device->machine->sample_rate, info, filter_volume_update);
}


void flt_volume_set_volume(device_t *device, float volume)
{
	filter_volume_state *info = get_safe_token(device);
	info->gain = (int)(volume * 256);
}



/**************************************************************************
 * Generic get_info
 **************************************************************************/

DEVICE_GET_INFO( filter_volume )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:					info->i = sizeof(filter_volume_state);			break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME( filter_volume );			break;
		case DEVINFO_FCT_STOP:							/* Nothing */											break;
		case DEVINFO_FCT_RESET:							/* Nothing */											break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "Volume Filter");						break;
		case DEVINFO_STR_FAMILY:					strcpy(info->s, "Filters");								break;
		case DEVINFO_STR_VERSION:					strcpy(info->s, "1.0");									break;
		case DEVINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);								break;
		case DEVINFO_STR_CREDITS:					strcpy(info->s, "Copyright Nicola Salmoria and the MAME Team"); break;
	}
}


DEFINE_LEGACY_SOUND_DEVICE(FILTER_VOLUME, filter_volume);
