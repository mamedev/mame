#include "emu.h"
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
	return (filter_volume_state *)downcast<filter_volume_device *>(device)->token();
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
	info->stream = device->machine().sound().stream_alloc(*device, 1, 1, device->machine().sample_rate(), info, filter_volume_update);
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


const device_type FILTER_VOLUME = &device_creator<filter_volume_device>;

filter_volume_device::filter_volume_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, FILTER_VOLUME, "Volume Filter", tag, owner, clock),
	  device_sound_interface(mconfig, *this)
{
	m_token = global_alloc_array_clear(UINT8, sizeof(filter_volume_state));
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void filter_volume_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void filter_volume_device::device_start()
{
	DEVICE_START_NAME( filter_volume )(this);
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void filter_volume_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// should never get here
	fatalerror("sound_stream_update called; not applicable to legacy sound devices\n");
}


